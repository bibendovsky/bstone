/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A stub for DOSBox OPL2 emulator.

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
