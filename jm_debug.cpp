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


#include "3d_def.h"

#include "jm_error.h"
#include "jm_debug.h"

#ifdef MSVC
#pragma hdrstop
#endif

#ifdef __MPRINTF__

static char x = 0;
static char y = 0;
static char* video = MK_FP(0xb000, 0x0000);

// -------------------------------------------------------------------------
// mclear()
// -------------------------------------------------------------------------
void mclear()
{
    int16_t length = 80 * 25 * 2;

    video = MK_FP(0xb000, 0x0000);
    while (length--) {
        *(int16_t*)video++ = 0x0f20;
    }

    x = y = 0;
    video = MK_FP(0xb000, 0x0000);
}

// -------------------------------------------------------------------------
// mprintf()
// -------------------------------------------------------------------------
void mprintf(
    char* msg,
    ...)
{
    char buffer[100], * ptr;

    va_list(ap);

    va_start(ap, msg);

    vsprintf(buffer, msg, ap);

    ptr = buffer;
    while (*ptr) {
        switch (*ptr) {
        case '\n':
            if (y >= 23) {
                video -= (x << 1);
                _fmemcpy(MK_FP(0xb000, 0x0000), MK_FP(0xb000, 0x00a0), 3840);
            } else {
                y++;
                video += ((80 - x) << 1);
            }
            x = 0;
            break;

        default:
            *video = *ptr;
            video[1] = 15;
            video += 2;
            x++;
            break;
        }
        ptr++;
    }

    va_end(ap);
}

// -------------------------------------------------------------------------
// fmprint()
// -------------------------------------------------------------------------
void fmprint(
    char* text)
{
    while (*text) {
        mprintf("%c", *text++);
    }
}

#endif
