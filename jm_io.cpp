/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
============================================================== */


#include "id_heads.h"
#include "jm_io.h"
#include "jm_lzh.h"


int IO_LoadFile(
    const char* filename,
    void** dst)
{
    char buffer[5] = { 0, 0, 0, 0, 0 };
    bstone::FileStream handle;
    int32_t size = 0;

    handle.open(filename);

    if (!handle.is_open()) {
        return size;
    }

    handle.read(buffer, 4);

    if (!strcmp(buffer, "JAMP")) {
        struct JAMPHeader head;

        handle.read(&head, sizeof(head));
        size = head.OriginalLen;
        switch (head.CompType) {
        case ct_LZH:
            LZH_Startup();

            *dst = new char[head.OriginalLen];

            {
                std::auto_ptr<uint8_t> buffer(new uint8_t[head.CompressLen]);

                ::LZH_Decompress(buffer.get(), *dst,
                                 size, head.CompressLen);
            }

            LZH_Shutdown();
            break;

        case ct_LZW:
            ::Quit("No code for LZW compression.");
            break;

        default:
            ::Quit("Unknown compression type.");
            break;
        }
    } else {
        handle.set_position(0);
        size = static_cast<int32_t>(handle.get_size());
        *dst = new char[size];
        if (handle.read(*dst, size) != size) {
            return size;
        }
    }

    return size;
}
