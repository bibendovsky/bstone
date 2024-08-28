/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// RGBA color, 8-bit per component.

#ifndef BSTONE_SYS_COLOR_INCLUDED
#define BSTONE_SYS_COLOR_INCLUDED

#include <cstdint>

namespace bstone {
namespace sys {

struct Color
{
	std::uint8_t r{};
	std::uint8_t g{};
	std::uint8_t b{};
	std::uint8_t a{};

	Color() = default;

	constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) noexcept
		:
		r{r},
		g{g},
		b{b},
		a{a}
	{}
};

constexpr inline bool operator==(Color lhs, Color rhs) noexcept
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

constexpr inline bool operator!=(Color lhs, Color rhs) noexcept
{
	return !(lhs == rhs);
}

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_COLOR_INCLUDED
