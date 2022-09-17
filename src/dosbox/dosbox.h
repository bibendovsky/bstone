/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_INCLUDED
#define BSTONE_DOSBOX_INCLUDED


#include <cassert>
#include <cmath>
#include <cstdint>


#define INLINE
#define GCC_UNLIKELY(x) (x)


using Bit32u = std::uint32_t;
using Bit32s = std::int32_t;
using Bit16u = std::uint16_t;
using Bit16s = std::int16_t;
using Bit8u = std::uint8_t;
using Bit8s = std::int8_t;

using Bitu = std::uintptr_t;
using Bits = std::intptr_t;


#endif // BSTONE_DOSBOX_INCLUDED
