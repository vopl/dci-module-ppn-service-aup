/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "base.hpp"
#include "../aup.hpp"

namespace dci::module::ppn::service::aup::supplier
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::Base()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::~Base()
    {
        _sbsOwner.flush();
        _transfers.clear();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Base::startOne(const Oid& oid, api::BlobTransfer<>::Opposite&& bt)
    {
        Transfer& transfer = _transfers.try_emplace(
                                 oid,
                                 this, oid).first->second;
        return transfer.startOne(std::move(bt));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Base::transferBecomesEmpty(const Oid& oid)
    {
        _transfers.erase(oid);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::Transfer::Transfer(Base* base, const Oid& oid)
        : _base{base}
        , _oid{oid}
        , _status{_base->getStatus(_oid)}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::Transfer::~Transfer()
    {
        _sbsOwner.flush();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Base::Transfer::startOne(api::BlobTransfer<>::Opposite&& bt)
    {
        dbgAssert(_ifaces.end() == _ifaces.find(bt));

        bt.involvedChanged() += _sbsOwner * [this, bt=bt.weak()](bool v)
        {
            if(!v)
            {
                _ifaces.erase(bt);
                if(_ifaces.empty())
                {
                    _base->transferBecomesEmpty(_oid);
                }
            }
        };

        //in getPiece(uint32 offset, uint32 size) -> tuple<BlobStatus, bytes>;
        bt->getPiece() += _sbsOwner * [this](uint32 offset, uint32 size)
        {
            if(api::BlobStatus::present == _status)
            {
                try
                {
                    return cmt::readyFuture(Tuple{_status, _base->getPiece(_oid, offset, size)});
                }
                catch(...)
                {
                    LOGW("supplier: unable to fetch blob piece: "<<exception::toString(std::current_exception()));
                    return cmt::readyFuture<Tuple<api::BlobStatus, Bytes>>(exception::buildInstance<api::Error>("unable to supply piece requested"));
                }
            }

            return cmt::readyFuture(Tuple{_status, Bytes{}});
        };

        _ifaces.emplace(std::move(bt));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Base::Transfer::updateStatus()
    {
        api::BlobStatus status = _base->getStatus(_oid);
        if(_status != status)
        {
            _status = status;

            //out statusChanged(BlobStatus);
            for(api::BlobTransfer<>::Opposite iface : _ifaces)
            {
                iface->statusChanged(_status);
            }
        }
    }
}

