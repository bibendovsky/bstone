/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Mouse

#ifndef BSTONE_SYS_MOUSE_INCLUDED
#define BSTONE_SYS_MOUSE_INCLUDED

namespace bstone::sys {

enum class MouseWheelDirection
{
	none,
	normal,
	flipped,
};

struct MouseButtonIndex
{
	constexpr static int left = 0;
	constexpr static int right = 1;
	constexpr static int middle = 2;
	constexpr static int x1 = 3;
	constexpr static int x2 = 4;
};

struct MouseButtonMask
{
	constexpr static unsigned int none = 0U;
	constexpr static unsigned int left = 1U << MouseButtonIndex::left;
	constexpr static unsigned int right = 1U << MouseButtonIndex::right;
	constexpr static unsigned int middle = 1U << MouseButtonIndex::middle;
	constexpr static unsigned int x1 = 1U << MouseButtonIndex::x1;
	constexpr static unsigned int x2 = 1U << MouseButtonIndex::x2;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_MOUSE_INCLUDED
