/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// The size of a rectangular region.
//


#ifndef BSTONE_EXTENT_2D_INCLUDED
#define BSTONE_EXTENT_2D_INCLUDED


namespace bstone
{


// ==========================================================================
// Extent2d
//

struct Extent2d
{
	int width;
	int height;
}; // Extent2d

//
// Extent2d
// ==========================================================================


bool operator==(
	const Extent2d& lhs,
	const Extent2d& rhs);

bool operator!=(
	const Extent2d& lhs,
	const Extent2d& rhs);


} // bstone


#endif // !BSTONE_EXTENT_2D_INCLUDED
