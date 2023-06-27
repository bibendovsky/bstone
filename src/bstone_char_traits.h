/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_CHAR_TRAITS_INCLUDED)
#define BSTONE_CHAR_TRAITS_INCLUDED

#include "bstone_int.h"

namespace bstone {
namespace char_traits {

template<typename TChar>
inline constexpr IntP get_size(const TChar* chars)
{
	auto i_chars = chars;

	while (*i_chars != TChar{})
	{
		++i_chars;
	}

	return i_chars - chars;
}

// ==========================================================================

template<typename TChar>
inline constexpr int compare(
	const TChar* lhs_chars,
	IntP lhs_size,
	const TChar* rhs_chars,
	IntP rhs_size)
{
	const auto size = lhs_size < rhs_size ? lhs_size : rhs_size;

	for (auto i = decltype(size){}; i < size; ++i)
	{
		const auto& lhs_char = lhs_chars[i];
		const auto& rhs_char = rhs_chars[i];

		if (lhs_char < rhs_char)
		{
			return -1;
		}
		else if (lhs_char > rhs_char)
		{
			return 1;
		}
	}

	if (lhs_size < rhs_size)
	{
		return -1;
	}
	else if (lhs_size > rhs_size)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

} // namespace char_traits
} // namespace bstone

#endif // BSTONE_CHAR_TRAITS_INCLUDED
