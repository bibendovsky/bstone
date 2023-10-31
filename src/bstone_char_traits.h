/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Character traits.

#if !defined(BSTONE_CHAR_TRAITS_INCLUDED)
#define BSTONE_CHAR_TRAITS_INCLUDED

#include <cassert>
#include <cstdint>

namespace bstone {
namespace char_traits {

template<typename TChar>
inline constexpr std::intptr_t get_size(const TChar* chars)
{
	assert(chars != nullptr);

	auto size = std::intptr_t{};

	while (chars[size] != TChar{})
	{
		++size;
	}

	return size;
}

template<typename TChar>
inline constexpr int compare(
	const TChar* lhs_chars,
	std::intptr_t lhs_size,
	const TChar* rhs_chars,
	std::intptr_t rhs_size)
{
	assert((lhs_chars == nullptr && lhs_size == 0) || (lhs_chars != nullptr && lhs_size >= 0));
	assert((rhs_chars == nullptr && rhs_size == 0) || (rhs_chars != nullptr && rhs_size >= 0));

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
