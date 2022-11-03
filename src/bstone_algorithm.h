/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ALGORITHM_INCLUDED
#define BSTONE_ALGORITHM_INCLUDED

#include <functional>

namespace bstone {
namespace algorithm {

template<typename T, typename TPredicate>
inline constexpr const T& clamp(const T& x, const T& x_min, const T& x_max, TPredicate predicate)
{
	return predicate(x, x_min) ? x_min : (predicate(x_max, x) ? x_max : x);
}

template<typename T>
inline constexpr const T& clamp(const T& x, const T& x_min, const T& x_max)
{
	return clamp(x, x_min, x_max, std::less<T>{});
}

} // namespace algorithm
} // namespace bstone

#endif // !BSTONE_ALGORITHM_INCLUDED
