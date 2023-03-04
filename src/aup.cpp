/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "aup.hpp"
#include <dci/aup/instance/io.hpp>

namespace dci::module::ppn::service
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Aup::Aup()
        : idl::ppn::service::Aup<>::Opposite{idl::interface::Initializer{}}
    {
        if(!instance::io::instanceInitialized())
        {
            return;
        }

        _io = std::make_unique<Io>();

        {
            link::Feature<>::Opposite op = *this;

            op->setup() += sol() * [this](link::feature::Service<> srv)
            {
                srv->addPayload(*this);

                srv->joinedByConnect() += sol() * [this](const link::Id&, link::Remote<> r)
                {
                    joined(r);
                };

                srv->joinedByAccept() += sol() * [this](const link::Id&, link::Remote<> r)
                {
                    joined(r);
                };
            };
        }

        {
            link::feature::Payload<>::Opposite op = *this;

            //in ids() -> set<ilid>;
            op->ids() += sol() * []()
            {
                return cmt::readyFuture(Set<idl::interface::Lid>{
                                            api::SupplierCatalog<>::lid(),
                                            api::SupplierStorage<>::lid()});
            };

            //in getInstance(Id requestorId, Remote requestor, ilid) -> interface;
            op->getInstance() += sol() * [this](const link::Id&, const link::Remote<>&, idl::interface::Lid ilid)
            {
                if(api::SupplierCatalog<>::lid() == ilid)
                {
                    return cmt::readyFuture(idl::Interface{_io->_supplierCatalogApi.opposite()});
                }
                if(api::SupplierStorage<>::lid() == ilid)
                {
                    return cmt::readyFuture(idl::Interface{_io->_supplierStorageApi.opposite()});
                }

                dbgWarn("crazy link?");

                return cmt::readyFuture<idl::Interface>(exception::buildInstance<api::Error>("bad instance ilid requested"));
            };
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Aup::~Aup()
    {
        sol().flush();
        _io.reset();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Aup::joined(link::Remote<> r)
    {
        r->getInstance(api::SupplierCatalog<>::lid()).then() += sol() * [this](cmt::Future<idl::Interface> in)
        {
            if(in.resolvedValue())
            {
                _io->_consumerCatalog.involve(in.value());
            }
        };
        r->getInstance(api::SupplierStorage<>::lid()).then() += sol() * [this](cmt::Future<idl::Interface> in)
        {
            if(in.resolvedValue())
            {
                _io->_consumerStorage.involve(in.value());
            }
        };
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Aup::Io::Io()
        : _supplierCatalogApi{idl::interface::Initializer{}}
        , _supplierStorageApi{idl::interface::Initializer{}}
        , _supplierCatalog{_supplierCatalogApi}
        , _supplierStorage{_supplierStorageApi}
        , _consumerQuota{100}
        , _consumerCatalog{&_consumerQuota}
        , _consumerStorage{&_consumerQuota}
    {
    }
}
