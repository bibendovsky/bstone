/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// General-purpose functionality.

#if !defined(BSTONE_UTILITY_INCLUDED)
#define BSTONE_UTILITY_INCLUDED

#include <cstddef>

#include <type_traits>
#include <utility>

namespace bstone {

class Utility
{
public:
	template<typename T>
	static constexpr void swap(T& lhs, T& rhs)
		noexcept(noexcept(
			std::is_nothrow_move_constructible<T>::value &&
			std::is_nothrow_move_assignable<T>::value));

	template<typename TLshIter, typename TRhsIter>
	static constexpr TRhsIter swap_ranges(TLshIter lhs_begin, TLshIter lhs_end, TRhsIter rhs_begin);

	template<typename T, std::size_t TSize>
	static constexpr void swap(T (&lhs)[TSize], T (&rhs)[TSize]) noexcept(noexcept(swap(*lhs, *rhs)))
	{
		for (auto i = decltype(TSize){}; i < TSize; ++i)
		{
			swap(lhs[i], rhs[i]);
		}
	}

	// ======================================================================

	template<typename T>
	static constexpr std::add_const_t<T>& as_const(T& x) noexcept;

	template<typename T>
	static void as_const(const T&&) = delete;
};

// --------------------------------------------------------------------------

template<typename T>
constexpr void Utility::swap(T& lhs, T& rhs)
	noexcept(noexcept(
		std::is_nothrow_move_constructible<T>::value &&
		std::is_nothrow_move_assignable<T>::value))
{
	auto temp = std::move(lhs);
	lhs = std::move(rhs);
	rhs = std::move(temp);
}

template<typename TLshIter, typename TRhsIter>
constexpr TRhsIter Utility::swap_ranges(TLshIter lhs_begin, TLshIter lhs_end, TRhsIter rhs_begin)
{
	while (lhs_begin != lhs_end)
	{
		swap(*lhs_begin, *rhs_begin);
		++lhs_begin;
		++rhs_begin;
	}

	return rhs_begin;
}

// ======================================================================

template<typename T>
constexpr std::add_const_t<T>& Utility::as_const(T& x) noexcept
{
	return x;
}

} // namespace bstone

#endif // BSTONE_UTILITY_INCLUDED
