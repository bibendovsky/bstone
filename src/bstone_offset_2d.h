/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// The location of a rectangular region.
//


#ifndef BSTONE_OFFSET_2D_INCLUDED
#define BSTONE_OFFSET_2D_INCLUDED


namespace bstone
{


// ==========================================================================
// Offset2d
//

struct Offset2d
{
	int x;
	int y;
}; // Offset2d

//
// Offset2d
// ==========================================================================


bool operator==(
	const Offset2d& lhs,
	const Offset2d& rhs) noexcept;

bool operator!=(
	const Offset2d& lhs,
	const Offset2d& rhs) noexcept;


} // bstone


#endif // !BSTONE_OFFSET_2D_INCLUDED
