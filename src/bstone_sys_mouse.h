/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Mouse.

#ifndef BSTONE_SYS_MOUSE_INCLUDED
#define BSTONE_SYS_MOUSE_INCLUDED

namespace bstone {
namespace sys {

enum class MouseWheelDirection
{
	none,
	normal,
	flipped,
};

struct MouseButtonIndex
{
	static constexpr auto left = 0;
	static constexpr auto right = 1;
	static constexpr auto middle = 2;
	static constexpr auto x1 = 3;
	static constexpr auto x2 = 4;
};

struct MouseButtonMask
{
	static constexpr auto none = 0U;
	static constexpr auto left = 1U << MouseButtonIndex::left;
	static constexpr auto right = 1U << MouseButtonIndex::right;
	static constexpr auto middle = 1U << MouseButtonIndex::middle;
	static constexpr auto x1 = 1U << MouseButtonIndex::x1;
	static constexpr auto x2 = 1U << MouseButtonIndex::x2;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_MOUSE_INCLUDED
