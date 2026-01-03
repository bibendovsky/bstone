/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Rectangle with the origin at the upper-left

#ifndef BSTONE_RECTANGLE_INCLUDED
#define BSTONE_RECTANGLE_INCLUDED

namespace bstone::sys {

struct Rect
{
	int x;
	int y;
	int width;
	int height;
};

struct FRect
{
	float x;
	float y;
	float width;
	float height;
};

} // namespace bstone::sys

#endif // BSTONE_RECTANGLE_INCLUDED
