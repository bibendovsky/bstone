/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// The size of a rectangular region.
//


#include "bstone_extent_2d.h"


namespace bstone
{


bool operator==(
	const Extent2d& lhs,
	const Extent2d& rhs)
{
	return lhs.width == rhs.width && lhs.width == rhs.height;
}

bool operator!=(
	const Extent2d& lhs,
	const Extent2d& rhs)
{
	return !(lhs == rhs);
}


} // bstone
