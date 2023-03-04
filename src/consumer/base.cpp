/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "base.hpp"
#include "../aup.hpp"

namespace dci::module::ppn::service::aup::consumer
{
    using namespace base;

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::Base(std::string_view name, Quota* quota)
        : _name{name}
        , _quota{quota}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Base::~Base()
    {
        _sbsOwner.flush();
        _downloaders.clear();
        _remotes.clear();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Remote* Base::involve(remote::Api&& remoteApi)
    {
        remote::Ptr remote = std::make_unique<Remote>(std::move(remoteApi));
        Remote* raw = remote.get();
        _remotes.emplace(raw, std::move(remote));

        raw->api().involvedChanged() += raw->sol() * [=,this](bool b) mutable
        {
            if(!b)
            {
                auto rn = _remotes.extract(raw);
                raw->uninvolved();
            }
        };

        for(auto&[oid, d] : _downloaders)
        {
            d.involve(raw);
            raw->involve(&d);
        }

        return raw;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Base::addIncomplete(const Oid& oid, int priority)
    {
        auto entry = _downloaders.try_emplace(
                         oid,
                         this, oid, priority);

        if(entry.second)
        {
            for(auto&[r,rp] : _remotes)
            {
                entry.first->second.involve(r);
                r->involve(&entry.first->second);
            }

            return true;
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    base::Quota& Base::quota()
    {
        return *_quota;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Base::ready(base::Downloader* d, RecvBuffer& recvBuffer)
    {
        try
        {
            return onComplete(d->oid(), recvBuffer);
        }
        catch(...)
        {
            LOGW(_name<<": blob placing failure: "<<exception::toString(std::current_exception()));
        }

        return false;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Base::done(base::Downloader* d)
    {
        _downloaders.erase(d->oid());
    }
}
