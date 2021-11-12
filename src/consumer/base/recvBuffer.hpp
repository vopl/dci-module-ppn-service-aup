/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "pch.hpp"

namespace dci::module::ppn::service::aup::consumer::base
{
    class RecvBuffer
    {
        RecvBuffer(const RecvBuffer&) = delete;
        RecvBuffer(RecvBuffer&&) = delete;

        void operator=(const RecvBuffer&) = delete;
        void operator=(RecvBuffer&&) = delete;

    public:
        RecvBuffer(uint32 ramBound);
        ~RecvBuffer();

        bool                        push(Bytes&& payload);
        uint32                      payloadSize();
        bool                        hasFile();
        Bytes                       detachBytes();
        instance::io::StdFilePtr    detachFile();
        void                        reset();

    private:
        bool pushFs(Bytes&& payload);

    private:
        uint32      _ramBound;
        uint32      _payloadSize{};
        Bytes       _ramPayload;
        std::FILE * _fsPayload{};
    };
}
