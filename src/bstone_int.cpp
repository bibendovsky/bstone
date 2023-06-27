/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Fixed width integer types.

#include <climits>

#include <type_traits>

#include "bstone_int.h"

namespace bstone {

static_assert(
	std::is_integral<Int8>::value && std::is_signed<Int8>::value && sizeof(Int8) * CHAR_BIT == 8,
	"Invalid Int8 type size.");

static_assert(
	std::is_integral<UInt8>::value && std::is_unsigned<UInt8>::value && sizeof(UInt8) * CHAR_BIT == 8,
	"Invalid UInt8 type size.");

static_assert(
	std::is_integral<Int16>::value && std::is_signed<Int16>::value && sizeof(Int16) * CHAR_BIT == 16,
	"Invalid Int16 type size.");

static_assert(
	std::is_integral<UInt16>::value && std::is_unsigned<UInt16>::value && sizeof(UInt16) * CHAR_BIT == 16,
	"Invalid UInt16 type size.");

static_assert(
	std::is_integral<Int32>::value && std::is_signed<Int32>::value && sizeof(Int32) * CHAR_BIT == 32,
	"Invalid Int32 type size.");

static_assert(
	std::is_integral<UInt32>::value && std::is_unsigned<UInt32>::value && sizeof(UInt32) * CHAR_BIT == 32,
	"Invalid UInt32 type size.");

static_assert(
	std::is_integral<Int64>::value && std::is_signed<Int64>::value && sizeof(Int64) * CHAR_BIT == 64,
	"Invalid Int64 type size.");

static_assert(
	std::is_integral<UInt64>::value && std::is_unsigned<UInt64>::value && sizeof(UInt64) * CHAR_BIT == 64,
	"Invalid UInt64 type size.");

static_assert(
	std::is_integral<IntP>::value &&
		std::is_signed<IntP>::value &&
		(sizeof(IntP) * CHAR_BIT == 32 || sizeof(IntP) * CHAR_BIT == 64),
	"Invalid IntP type size.");

static_assert(
	std::is_integral<UIntP>::value &&
		std::is_unsigned<UIntP>::value &&
		(sizeof(UIntP) * CHAR_BIT == 32 || sizeof(UIntP) * CHAR_BIT == 64),
	"Invalid UIntP type size.");

} // namespace bstone
