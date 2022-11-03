/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_UTILITY_INCLUDED
#define BSTONE_UTILITY_INCLUDED

#include <cstddef>
#include <type_traits>
#include <utility>

namespace bstone {
namespace utility {

template<typename T>
inline static constexpr void swap(T& lhs, T& rhs)
	noexcept(
		noexcept(
			std::is_nothrow_move_constructible<T>::value &&
			std::is_nothrow_move_assignable<T>::value))
{
	auto temp = std::move(lhs);
	lhs = std::move(rhs);
	rhs = std::move(temp);
}

template<typename TLshIter, typename TRhsIter>
inline static constexpr TRhsIter swap_ranges(TLshIter lhs_begin, TLshIter lhs_end, TRhsIter rhs_begin)
{
	while (lhs_begin != lhs_end)
	{
		swap(*lhs_begin++, *rhs_begin++);
	}

	return rhs_begin;
}

template<typename T, std::size_t TSize>
inline static constexpr void swap(T (&lhs)[TSize], T (&rhs)[TSize]) noexcept(noexcept(swap(*lhs, *rhs)))
{
	swap_ranges(lhs, lhs + TSize, rhs);
}

} // namespace utility
} // namespace bstone

#endif // !BSTONE_UTILITY_INCLUDED
