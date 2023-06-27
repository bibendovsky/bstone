/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Fixed width integer types.

#if !defined(BSTONE_INT_INCLUDED)
#define BSTONE_INT_INCLUDED

#include <cstdint>

namespace bstone {

using Int8 = std::int8_t;
using UInt8 = std::uint8_t;

using Int16 = std::int16_t;
using UInt16 = std::uint16_t;

using Int32 = std::int32_t;
using UInt32 = std::uint32_t;

using Int64 = std::int64_t;
using UInt64 = std::uint64_t;

using IntP = std::intptr_t;
using UIntP = std::uintptr_t;

} // namespace bstone

#endif // BSTONE_INT_INCLUDED
