/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// General-purpose functionality.

#ifndef BSTONE_UTILITY_INCLUDED
#define BSTONE_UTILITY_INCLUDED

#include <cstddef>

#include <type_traits>
#include <utility>

namespace bstone {

template<typename T>
inline constexpr void swop(T& lhs, T& rhs)
	noexcept(noexcept(
		std::is_nothrow_move_constructible<T>::value &&
		std::is_nothrow_move_assignable<T>::value))
{
	auto temp = std::move(lhs);
	lhs = std::move(rhs);
	rhs = std::move(temp);
}

template<typename TLshIter, typename TRhsIter>
inline constexpr TRhsIter swop_ranges(TLshIter lhs_begin, TLshIter lhs_end, TRhsIter rhs_begin)
{
	while (lhs_begin != lhs_end)
	{
		bstone::swop(*lhs_begin, *rhs_begin);
		++lhs_begin;
		++rhs_begin;
	}

	return rhs_begin;
}

template<typename T, std::size_t TSize>
static constexpr void swop(T (&lhs)[TSize], T (&rhs)[TSize]) noexcept(noexcept(bstone::swop(*lhs, *rhs)))
{
	for (auto i = decltype(TSize){}; i < TSize; ++i)
	{
		bstone::swop(lhs[i], rhs[i]);
	}
}

// ======================================================================

template<typename T>
inline constexpr std::add_const_t<T>& as_const(T& x) noexcept
{
	return x;
}

template<typename T>
inline void as_const(const T&&) = delete;

// ======================================================================

template<typename T>
inline constexpr T& as_mutable(const T& value) noexcept
{
    return const_cast<T&>(value);
}

template<typename T>
inline constexpr T* as_mutable(const T* value) noexcept
{
    return const_cast<T*>(value);
}

template<typename T>
inline constexpr T* as_mutable(T* value) noexcept
{
    return value;
}

template<typename T>
inline void as_mutable(const T&&) = delete;

} // namespace bstone

#endif // BSTONE_UTILITY_INCLUDED
