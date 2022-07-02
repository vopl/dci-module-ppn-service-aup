/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "storage.hpp"
#include "../aup.hpp"
#include <dci/aup/instance/io.hpp>
#include <dci/aup/instance/notifiers.hpp>

namespace dci::module::ppn::service::aup::supplier
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Storage::Storage(api::SupplierStorage<>::Opposite ss)
        : _ss{std::move(ss)}
    {
        //in startBlobTransfer(Oid, BlobTransfer);
        _ss->startBlobTransfer() += _sbsOwner * [this](const Oid& oid, api::BlobTransfer<>::Opposite&& bt)
        {
            return Base::startOne(oid, std::move(bt));
        };

        //out statusChanged(BlobStatus);
        instance::notifiers::onBufferStorageIncomplete() += _sbsOwner * [this](const Oid& oid)
        {
            auto iter = _transfers.find(oid);
            if(_transfers.end() != iter)
            {
                iter->second.updateStatus();
            }
        };

        instance::notifiers::onBufferStorageComplete() += _sbsOwner * [this](const Oid& oid)
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
    Storage::~Storage()
    {
        _sbsOwner.flush();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Bytes Storage::getPiece(const Oid& oid, uint32 offset, uint32 size)
    {
        auto obj = instance::io::getStorageObject(oid, offset, size);
        if(!obj)
        {
            return Bytes{};
        }

        return std::move(*obj);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    api::BlobStatus Storage::getStatus(const Oid& oid)
    {
        if(instance::io::bufferStorageComplete().count(oid))
        {
            return api::BlobStatus::present;
        }

        if(instance::io::bufferStorageIncomplete().count(oid))
        {
            return api::BlobStatus::missingAndWanted;
        }

        return api::BlobStatus::missingAndUnwanted;
    }

}

