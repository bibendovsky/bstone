/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// The rectangular region.
//


#ifndef BSTONE_RECT_2D_INCLUDED
#define BSTONE_RECT_2D_INCLUDED


#include "bstone_extent_2d.h"
#include "bstone_offset_2d.h"


namespace bstone
{


// ==========================================================================
// Rect2d
//

struct Rect2d
{
	Offset2d offset_;
	Extent2d extent_;
}; // Rect2d

//
// Rect2d
// ==========================================================================


bool operator==(
	const Rect2d& lhs,
	const Rect2d& rhs) noexcept
{
	return lhs.offset_ == rhs.offset_ && lhs.extent_ == rhs.extent_;
}

bool operator!=(
	const Rect2d& lhs,
	const Rect2d& rhs) noexcept
{
	return !(lhs == rhs);
}


} // bstone


#endif // !BSTONE_RECT_2D_INCLUDED
