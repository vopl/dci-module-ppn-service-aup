/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "catalog.hpp"
#include <dci/aup/instance/io.hpp>
#include <dci/aup/instance/notifiers.hpp>

namespace dci::module::ppn::service::aup::supplier
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Catalog::Catalog(api::SupplierCatalog<>::Opposite sc)
        : _sc{std::move(sc)}
    {
        _releases = instance::io::bufferMostReleases();

        //in getReleases() -> set<oid>;
        _sc->getReleases() += _sbsOwner * [this]
        {
            return cmt::readyFuture(_releases);
        };

        //out newRelease(oid);
        instance::notifiers::onBufferMostReleases() += _sbsOwner * [this](const Set<Oid>&)
        {
            auto releases = instance::io::bufferMostReleases();
            releases.swap(_releases);

            for(const Oid& oid : _releases)
            {
                if(!releases.count(oid))
                {
                    _sc->newRelease(oid);
                }
            }
        };

        //in startBlobTransfer(Oid, BlobTransfer);
        _sc->startBlobTransfer() += _sbsOwner * [this](const Oid& oid, api::BlobTransfer<>::Opposite&& bt)
        {
            return Base::startOne(oid, std::move(bt));
        };

        //out statusChanged(BlobStatus);
        instance::notifiers::onBufferCatalogIncomplete() += _sbsOwner * [this](const Oid& oid)
        {
            auto iter = _transfers.find(oid);
            if(_transfers.end() != iter)
            {
                iter->second.updateStatus();
            }
        };

        instance::notifiers::onBufferCatalogComplete() += _sbsOwner * [this](const Oid& oid)
        {
            auto iter = _transfers.find(oid);
            if(_transfers.end() != iter)
            {
                iter->second.updateStatus();
            }
        };

        instance::notifiers::onBufferMostReleases() += _sbsOwner * [this](const Set<Oid>&)
        {
            for(auto iter {_transfers.begin()}; iter!=_transfers.end(); ++iter)
            {
                iter->second.updateStatus();
            }
        };
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Catalog::~Catalog()
    {
        _sbsOwner.flush();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Bytes Catalog::getPiece(const Oid& oid, uint32 offset, uint32 size)
    {
        auto obj = instance::io::getCatalogObject(oid);
        if(!obj)
        {
            return Bytes{};
        }

        Bytes blob = std::move(*obj);

        uint32 last = offset + size;
        if(blob.size() > last)
        {
            bytes::Alter a = blob.begin();
            a.advance(static_cast<int32>(last));
            a.remove();
        }

        if(offset)
        {
            blob.begin().remove(offset);
        }

        return blob;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    api::BlobStatus Catalog::getStatus(const Oid& oid)
    {
        if(instance::io::bufferCatalogComplete().count(oid))
        {
            return api::BlobStatus::present;
        }

        if(instance::io::bufferCatalogIncomplete().count(oid))
        {
            return api::BlobStatus::missingAndWanted;
        }

        return api::BlobStatus::missingAndUnwanted;
    }

}

