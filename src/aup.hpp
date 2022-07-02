/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"
#include "supplier/catalog.hpp"
#include "supplier/storage.hpp"
#include "consumer/catalog.hpp"
#include "consumer/storage.hpp"

namespace dci::module::ppn::service
{
    class Aup
        : public idl::ppn::service::Aup<>::Opposite
        , public host::module::ServiceBase<Aup>
    {
    public:
        Aup();
        ~Aup();

    private:
        void joined(link::Remote<> r);

    private:
        struct Io
        {
            api::SupplierCatalog<>::Opposite _supplierCatalogApi;
            api::SupplierStorage<>::Opposite _supplierStorageApi;

            aup::supplier::Catalog _supplierCatalog;
            aup::supplier::Storage _supplierStorage;

            aup::consumer::base::Quota   _consumerQuota;
            aup::consumer::Catalog       _consumerCatalog;
            aup::consumer::Storage       _consumerStorage;

            Io();
        };
        std::unique_ptr<Io> _io;
    };
}
