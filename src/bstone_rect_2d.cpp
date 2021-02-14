/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
