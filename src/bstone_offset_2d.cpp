/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// The location of a rectangular region.
//


#include "bstone_offset_2d.h"


namespace bstone
{


bool operator==(
	const Offset2d& lhs,
	const Offset2d& rhs) noexcept
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(
	const Offset2d& lhs,
	const Offset2d& rhs) noexcept
{
	return !(lhs == rhs);
}


} // bstone
