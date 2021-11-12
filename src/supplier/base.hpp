/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"

namespace dci::module::ppn::service::aup::supplier
{
    class Base
    {
    public:
        Base();
        virtual ~Base();

        void startOne(const Oid& oid, api::BlobTransfer<>::Opposite&& bt);

    protected:
        virtual Bytes getPiece(const Oid& oid, uint32 offset, uint32 size) = 0;
        virtual api::BlobStatus getStatus(const Oid& oid) = 0;

    private:
        void transferBecomesEmpty(const Oid& oid);

    protected:
        sbs::Owner                              _sbsOwner;

    protected:
        class Transfer
        {
        public:
            Transfer(Base* base, const Oid& oid);
            ~Transfer();

            void startOne(api::BlobTransfer<>::Opposite&& bt);
            void updateStatus();

        private:
            Base *                              _base;
            const Oid                           _oid;
            api::BlobStatus                     _status{api::BlobStatus::present};
            Set<api::BlobTransfer<>::Opposite>  _ifaces;

            sbs::Owner                          _sbsOwner;
        };

        Map<Oid, Transfer> _transfers;
    };
}
