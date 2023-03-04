/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "remote.hpp"
#include "downloader.hpp"

namespace dci::module::ppn::service::aup::consumer::base
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Remote::Remote(remote::Api&& api)
        : _api{std::move(api)}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Remote::~Remote()
    {
        uninvolved();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Owner& Remote::sol()
    {
        return _sbsOwner;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    const remote::Api& Remote::api()
    {
        return _api;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Remote::uninvolved()
    {
        _sbsOwner.flush();
        for(Downloader* d : std::exchange(_downloaders, {}))
        {
            d->uninvolve(this);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Remote::involve(Downloader* d)
    {
        _downloaders.insert(d);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Remote::uninvolve(Downloader* d)
    {
        _downloaders.erase(d);
    }
}
