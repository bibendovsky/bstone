/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_COLOR_INCLUDED)
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

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_COLOR_INCLUDED
