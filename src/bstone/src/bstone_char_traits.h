/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Character traits.

#ifndef BSTONE_CHAR_TRAITS_INCLUDED
#define BSTONE_CHAR_TRAITS_INCLUDED

#include <cstdint>

#include <algorithm>
#include <type_traits>

#include "bstone_assert.h"

namespace bstone {
namespace char_traits {

template<typename TChar>
constexpr std::intptr_t get_size(const TChar* chars) noexcept;

template<typename TChar>
constexpr int compare(
	const TChar* lhs_chars,
	std::intptr_t lhs_size,
	const TChar* rhs_chars,
	std::intptr_t rhs_size) noexcept;

// ==========================================================================

template<typename TChar>
inline constexpr std::intptr_t get_size(const TChar* chars) noexcept
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported type.");

	BSTONE_ASSERT(chars != nullptr);

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
	std::intptr_t rhs_size) noexcept
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported type.");

	BSTONE_ASSERT((lhs_chars != nullptr && lhs_size >= 0) || (lhs_chars == nullptr && lhs_size == 0));
	BSTONE_ASSERT((rhs_chars != nullptr && rhs_size >= 0) || (rhs_chars == nullptr && rhs_size == 0));

	const auto size = std::min(lhs_size, rhs_size);

	for (auto i = std::intptr_t{}; i < size; ++i)
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
