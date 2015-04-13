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
#include "jm_cio.h"
#include "jm_lzh.h"


#if DEMOS_EXTERN

// --------------------------------------------------------------------------
// IO_WriteFile()
// --------------------------------------------------------------------------
boolean IO_WriteFile(
    char* filename,
    void* ptr,
    Sint32 length)
{
    Sint16 handle;
    Sint32 size;

    handle = open(filename, O_CREAT | O_BINARY | O_WRONLY,
                  S_IREAD | S_IWRITE | S_IFREG);

    if (handle == -1) {
        return false;
    }

    if (!IO_FarWrite(handle, ptr, length)) {
        close(handle);
        return false;
    }
    close(handle);
    return true;
}

#endif

// --------------------------------------------------------------------------
// IO_LoadFile()
// --------------------------------------------------------------------------
int IO_LoadFile(
    const char* filename,
    void** dst)
{
    char buffer[5] = { 0, 0, 0, 0, 0 };
    bstone::FileStream handle;
    Sint32 size = 0;

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
                std::auto_ptr<Uint8> buffer(new Uint8[head.CompressLen]);

                ::LZH_Decompress(buffer.get(), *dst,
                                 size, head.CompressLen);
            }

            LZH_Shutdown();
            break;

        case ct_LZW:
            IO_ERROR(IO_LOADFILE_NO_LZW);
            break;

        default:
            IO_ERROR(IO_LOADFILE_UNKNOWN);
            break;
        }
    } else {
        handle.set_position(0);
        size = static_cast<Sint32>(handle.get_size());
        *dst = new char[size];
        if (handle.read(*dst, size) != size) {
            return size;
        }
    }

    return size;
}

#if 0

// --------------------------------------------------------------------------
// IO_CopyFile()
// --------------------------------------------------------------------------
void IO_CopyFile(
    char* sFilename,
    char* dFilename)
{
    Sint16 sHandle, dHandle;
    Uint16 length;

// Allocate memory for buffer.
//
    if ((sHandle = open(sFilename, O_RDONLY | O_BINARY, S_IREAD)) == -1) {
        IO_ERROR(IO_COPYFILE_OPEN_SRC);
    }

    if ((dHandle = open(dFilename, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE)) == -1) {
        IO_ERROR(IO_COPYFILE_OPEN_DEST);
    }

// Copy that file!
//
    IO_CopyHandle(sHandle, dHandle, -1);

// Close files.
//
    close(sHandle);
    close(dHandle);
}

#endif
