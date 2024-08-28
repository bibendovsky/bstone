/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A rectangle with the origin at the upper-left.

#ifndef BSTONE_RECTANGLE_INCLUDED
#define BSTONE_RECTANGLE_INCLUDED

namespace bstone {
namespace sys {

struct Rectangle
{
	int x;
	int y;

	int width;
	int height;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_RECTANGLE_INCLUDED
