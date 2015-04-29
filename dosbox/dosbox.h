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


//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_INCLUDED
#define BSTONE_DOSBOX_INCLUDED


#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdint.h>


#define INLINE
#define GCC_UNLIKELY(x) (x)


typedef uint32_t Bit32u;
typedef int32_t Bit32s;
typedef uint16_t Bit16u;
typedef int16_t Bit16s;
typedef uint8_t Bit8u;
typedef int8_t Bit8s;

typedef uintptr_t Bitu;
typedef intptr_t Bits;


#endif // BSTONE_DOSBOX_INCLUDED
