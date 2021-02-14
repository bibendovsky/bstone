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
	int width_;
	int height_;
}; // Extent2d

//
// Extent2d
// ==========================================================================


bool operator==(
	const Extent2d& lhs,
	const Extent2d& rhs) noexcept
{
	return lhs.width_ == rhs.width_ && lhs.width_ == rhs.height_;
}

bool operator!=(
	const Extent2d& lhs,
	const Extent2d& rhs) noexcept
{
	return !(lhs == rhs);
}


} // bstone


#endif // !BSTONE_EXTENT_2D_INCLUDED
