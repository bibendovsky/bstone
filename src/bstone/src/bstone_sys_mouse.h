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
	static constexpr int left = 0;
	static constexpr int right = 1;
	static constexpr int middle = 2;
	static constexpr int x1 = 3;
	static constexpr int x2 = 4;
};

struct MouseButtonMask
{
	static constexpr unsigned int none = 0U;
	static constexpr unsigned int left = 1U << MouseButtonIndex::left;
	static constexpr unsigned int right = 1U << MouseButtonIndex::right;
	static constexpr unsigned int middle = 1U << MouseButtonIndex::middle;
	static constexpr unsigned int x1 = 1U << MouseButtonIndex::x1;
	static constexpr unsigned int x2 = 1U << MouseButtonIndex::x2;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_MOUSE_INCLUDED
