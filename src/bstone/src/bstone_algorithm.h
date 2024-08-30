/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Functions for a variety of purposes that operate on ranges of elements.

#ifndef BSTONE_ALGORITHM_INCLUDED
#define BSTONE_ALGORITHM_INCLUDED

namespace bstone {

template<typename T, typename TPredicate>
inline constexpr const T& clamp(const T& x, const T& x_min, const T& x_max, TPredicate predicate)
{
	return predicate(x, x_min) ? x_min : (predicate(x_max, x) ? x_max : x);
}

template<typename T>
inline constexpr const T& clamp(const T& x, const T& x_min, const T& x_max)
{
	return x < x_min ? x_min : (x > x_max ? x_max : x);
}

} // namespace bstone

#endif // BSTONE_ALGORITHM_INCLUDED
