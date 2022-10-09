/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_TYPE_TRAITS_INCLUDED
#define BSTONE_TYPE_TRAITS_INCLUDED

#include <type_traits>

namespace bstone {

template<typename T>
constexpr std::add_const_t<T>& as_const(T& x) noexcept
{
	return x;
}

} // namespace bstone

#endif // !BSTONE_TYPE_TRAITS_INCLUDED
