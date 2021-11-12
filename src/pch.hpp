/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/host.hpp>
#include <dci/aup.hpp>
#include <dci/mm/heap/allocable.hpp>
#include <dci/poll/timer.hpp>
#include <dci/utils/b2h.hpp>
#include <dci/utils/atScopeExit.hpp>
#include <memory>
#include <queue>
#include <deque>
#include "ppn/service/aup.hpp"

namespace dci::module::ppn::service
{
    using namespace dci;
    using namespace dci::aup;

    namespace link      = idl::ppn::node::link;
    namespace api       = idl::ppn::service::aup;
}
