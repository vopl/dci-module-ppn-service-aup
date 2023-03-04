/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "quota.hpp"
#include "downloader.hpp"

namespace dci::module::ppn::service::aup::consumer::base
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Quota::Quota(std::size_t slots_)
        : _slots{slots_}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Quota::~Quota()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Quota::clear()
    {
        _ready.clear();
        _inprogress.clear();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Quota::setSlots(std::size_t v)
    {
        _slots = v;
        update();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Quota::ready(Downloader* downloader)
    {
        _ready.emplace(downloader);
        _inprogress.erase(downloader);
        update();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Quota::done(Downloader* downloader)
    {
        _ready.erase(downloader);
        _inprogress.erase(downloader);
        update();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Quota::update()
    {
        while(_inprogress.size() < _slots && !_ready.empty())
        {
            Downloader* d = *_ready.begin();
            _ready.erase(_ready.begin());
            _inprogress.insert(d);
            d->canWork();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Quota::QueueCmp::operator()(Downloader*a, Downloader*b) const
    {
        return std::tuple{a->priority(), a} > std::tuple{b->priority(), b};
    }
}
