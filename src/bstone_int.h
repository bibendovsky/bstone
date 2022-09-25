/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Fundamental integer types.

#include <cstddef>
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

// --------------------------------------------------------------------------

using Int = std::ptrdiff_t;
using UInt = std::size_t;

} // bstone
