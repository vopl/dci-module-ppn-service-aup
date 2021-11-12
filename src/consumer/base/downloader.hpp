/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"

namespace dci::module::ppn::service::aup::consumer
{
    class Base;
}

namespace dci::module::ppn::service::aup::consumer::base
{
    class Remote;
    namespace remote
    {
        using Ptr = std::unique_ptr<Remote>;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    class Downloader
    {
    public:
        Downloader(Base* b, const Oid& oid, int priority);
        ~Downloader();

        void involve(Remote* r);
        void uninvolve(Remote* r);

        const Oid& oid() const;
        int priority() const;
        void canWork();

    private:
        void worker();

    private:
        Base *  _b {};
        Oid     _oid {};
        int     _priority {};

        cmt::task::Owner    _taskOwner;

        std::set<Remote*>   _candidates;
        bool                _canWork {};

        cmt::Notifier       _awaker;

    private:
        std::set<Remote*> _remotesPresent;
        std::set<Remote*> _remotesMissingAndWanted;
        std::set<Remote*> _remotesMissingAndUnwanted;

    };
}
