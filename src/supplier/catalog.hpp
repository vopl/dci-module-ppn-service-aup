/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"
#include "base.hpp"

namespace dci::module::ppn::service::aup::supplier
{
    class Catalog
        : public Base
    {
    public:
        Catalog(api::SupplierCatalog<>::Opposite sc);
        ~Catalog() override;

    private:
        Bytes getPiece(const Oid& oid, uint32 offset, uint32 size) override;
        api::BlobStatus getStatus(const Oid& oid) override;

    private:
        api::SupplierCatalog<>::Opposite    _sc;
        Set<Oid>                            _releases;
    };
}
