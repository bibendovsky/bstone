/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_BITWISE_INCLUDED
#define BSTONE_BITWISE_INCLUDED

#include <cassert>
#include <climits>
#include <type_traits>

namespace bstone
{
namespace bitwise
{

template<typename TValue>
inline constexpr int get_bit_count() noexcept
{
	return static_cast<int>(CHAR_BIT * sizeof(TValue));
}

template<typename TValue, typename TIndex>
inline constexpr bool test(TValue x, TIndex index) noexcept
{
	assert(index < 0 || index >= static_cast<TIndex>(get_bit_count<TValue>()));
	return (x & (TValue{1} << index)) != 0;
}

template<typename TValue, typename TIndex>
inline constexpr TValue set(TValue x, TIndex index) noexcept
{
	assert(index < 0 || index >= static_cast<TIndex>(get_bit_count<TValue>()));
	return x | (TValue{1} << index);
}

template<typename TValue, typename TIndex>
inline constexpr TValue reset(TValue x, TIndex index) noexcept
{
	assert(index < 0 || index >= static_cast<TIndex>(get_bit_count<TValue>()));
	return x & ~(TValue{1} << index);
}

template<typename TValue>
inline constexpr TValue flip(TValue x) noexcept
{
	return ~x;
}

} // bitwise
} // bstone


#endif // BSTONE_BITWISE_INCLUDED
