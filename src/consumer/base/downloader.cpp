/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "downloader.hpp"
#include "recvBuffer.hpp"
#include "../base.hpp"

namespace dci::module::ppn::service::aup::consumer::base
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Downloader::Downloader(Base* b, const Oid& oid, int priority)
        : _b{b}
        , _oid{oid}
        , _priority{priority}
    {
        cmt::spawn() += _taskOwner * [this]
        {
            worker();
        };
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Downloader::~Downloader()
    {
        for(Remote* r : std::exchange(_candidates, {}))
        {
            r->uninvolve(this);
        }

        _b->quota().done(this);
        _taskOwner.stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Downloader::involve(Remote* r)
    {
        _candidates.insert(r);
        _awaker.raise();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Downloader::uninvolve(Remote* r)
    {
        _candidates.erase(r);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    const Oid& Downloader::oid() const
    {
        return _oid;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    int Downloader::priority() const
    {
        return _priority;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Downloader::canWork()
    {
        _canWork = true;
        _awaker.raise();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Downloader::worker()
    {
        bool done = false;

        {
            struct Transfer : std::enable_shared_from_this<Transfer>, mm::heap::Allocable<Transfer>
            {
                sbs::Owner          _sbsOwner;
                api::BlobTransfer<> _api;

                Transfer()
                    : _api{idl::interface::Initializer{}}
                {
                }

                ~Transfer()
                {
                    close();
                }

                void close()
                {
                    _sbsOwner.flush();
                    _api.reset();
                }
            };
            using TransferPtr = std::shared_ptr<Transfer>;
            std::set<TransferPtr> transfersReady;
            std::set<TransferPtr> transfersWait;

            auto updateTransfer = [&](const TransferPtr& t, api::BlobStatus bs)
            {
                if(!t->_api)
                {
                    transfersReady.erase(t);
                    transfersWait.erase(t);
                    return;
                }

                switch(bs)
                {
                case api::BlobStatus::present:
                    transfersReady.insert(t);
                    transfersWait.erase(t);
                    break;
                case api::BlobStatus::missingAndWanted:
                    transfersReady.erase(t);
                    transfersWait.insert(t);
                    break;
                case api::BlobStatus::missingAndUnwanted:
                default:
                    t->close();
                    transfersReady.erase(t);
                    transfersWait.erase(t);
                    break;
                }
            };

            const uint32 granulaSize = 1024 * 128;
            RecvBuffer recvBuffer{granulaSize};

            auto workRaii = [this]
            {
                if(!_canWork)
                {
                    _b->quota().ready(this);
                }

                while(!_canWork)
                {
                    _awaker.wait();
                }

                _canWork = false;

                return utils::AtScopeExit{[this]
                {
                    _b->quota().done(this);
                }};
            };

            for(;;)
            {
                if(!transfersReady.empty())
                {
                    auto wl = workRaii();

                    if(!transfersReady.empty())
                    {
                        TransferPtr transfer = *transfersReady.begin();
                        dbgAssert(transfer->_api);

                        auto resf = transfer->_api->getPiece(recvBuffer.payloadSize(), granulaSize+0);
                        resf.wait();

                        if(resf.resolvedValue())
                        {
                            auto res = resf.detachValue();

                            api::BlobStatus bs = std::get<0>(res);
                            updateTransfer(transfer, bs);
                            if(api::BlobStatus::present != bs)
                            {
                                continue;
                            }

                            Bytes&& recv = std::move(res).get<1>();
                            uint32 recvSize = recv.size();

                            recvBuffer.push(std::move(recv));

                            if(recvSize != granulaSize)
                            {
                                utils::AtScopeExit se{[&]
                                {
                                    recvBuffer.reset();
                                }};

                                std::size_t recvBufferPayloadSize = recvBuffer.payloadSize();
                                if(_b->ready(this, recvBuffer))
                                {
                                    LOGI(_b->_name<<": blob transfer complete: "<<utils::b2h(_oid)<<", "<<recvBufferPayloadSize<<" bytes");
                                    done = true;
                                    break;
                                }

                                //ауп не принял полученный блоб, бросить его и попробовать еще раз
                                LOGW(_b->_name<<": blob unaccepted: "<<utils::b2h(_oid)<<", "<<recvBufferPayloadSize<<" bytes");
                            }
                        }
                        else if(resf.resolvedException())
                        {
                            recvBuffer.reset();
                            LOGW(_b->_name<<": blob transfer failed: "<<utils::b2h(_oid)<<", "<<exception::toString(resf.detachException()));
                            updateTransfer(transfer, api::BlobStatus::missingAndUnwanted);
                            continue;
                        }
                        else //if(resf.resolvedCancel())
                        {
                            recvBuffer.reset();
                            LOGW(_b->_name<<": blob transfer canceled: "<<utils::b2h(_oid));
                            updateTransfer(transfer, api::BlobStatus::missingAndUnwanted);
                            continue;
                        }
                    }

                    continue;
                }

                if(_candidates.empty())
                {
                    _awaker.wait();
                    continue;
                }

                auto wl = workRaii();

                if(!_candidates.empty())
                {

                    Remote* r = *_candidates.begin();
                    _candidates.erase(_candidates.begin());
                    r->uninvolve(this);


                    TransferPtr transfer {std::make_shared<Transfer>()};

                    transfer->_api.involvedChanged() += transfer->_sbsOwner * [this,&updateTransfer,raw=transfer.get()](bool v)
                    {
                        if(!v)
                        {
                            updateTransfer(raw->shared_from_this(), api::BlobStatus::missingAndUnwanted);
                            _awaker.raise();
                        }
                    };
                    transfer->_api->statusChanged() += transfer->_sbsOwner * [this,&updateTransfer,raw=transfer.get()](api::BlobStatus bs)
                    {
                        updateTransfer(raw->shared_from_this(), bs);
                        _awaker.raise();
                    };

                    updateTransfer(transfer, api::BlobStatus::present);
                    r->api()->startBlobTransfer(_oid, transfer->_api.opposite());
                }
            }
        }

        if(done)
        {
            cmt::task::currentTask().ownTo(nullptr);
            _b->done(this);
        }
    }
}
