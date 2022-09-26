/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// C-string operations.

#ifndef BSTONE_C_STR_INCLUDED
#define BSTONE_C_STR_INCLUDED

#include <cassert>
#include "bstone_int.h"

namespace bstone {
namespace c_str {

template<typename TChar>
inline constexpr Int get_size(const TChar* chars) noexcept
{
	assert(chars != nullptr);
	auto size = Int{};

	while (chars[size] != TChar{})
	{
		size += 1;
	}

	return size;
}

template<typename TChar>
inline constexpr Int get_size_with_null(const TChar* chars) noexcept
{
	return get_size(chars) + 1;
}

} // namespace c_str
} // namespace bstone

#endif // !BSTONE_C_STR_INCLUDED
