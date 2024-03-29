/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"

namespace dci::module::ppn::service::aup::consumer::base
{
    class Downloader;
    class Remote;

    namespace remote
    {
        using Api = api::Supplier<>;
        using Ptr = std::unique_ptr<Remote>;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    class Remote
    {
    public:
        Remote(remote::Api&& api);
        ~Remote();

        sbs::Owner& sol();
        const remote::Api& api();

        void uninvolved();
        void involve(Downloader* d);
        void uninvolve(Downloader* d);

    private:
        sbs::Owner              _sbsOwner;
        remote::Api             _api;
        Set<Downloader*>   _downloaders;
    };
}
