/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Character traits.

#if !defined(BSTONE_CHAR_TRAITS_INCLUDED)
#define BSTONE_CHAR_TRAITS_INCLUDED

#include <cassert>

#include "bstone_int.h"

namespace bstone {

class CharTraits
{
public:
	template<typename TChar>
	static constexpr IntP get_size(const TChar* chars);

	template<typename TChar>
	static constexpr int compare(const TChar* lhs_chars, IntP lhs_size, const TChar* rhs_chars, IntP rhs_size);
};

// --------------------------------------------------------------------------

template<typename TChar>
constexpr IntP CharTraits::get_size(const TChar* chars)
{
	assert(chars != nullptr);

	auto size = IntP{};

	while (chars[size] != TChar{})
	{
		++size;
	}

	return size;
}

template<typename TChar>
constexpr int CharTraits::compare(const TChar* lhs_chars, IntP lhs_size, const TChar* rhs_chars, IntP rhs_size)
{
	assert(lhs_chars != nullptr);
	assert(rhs_chars != nullptr);

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

} // namespace bstone

#endif // BSTONE_CHAR_TRAITS_INCLUDED
