/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_r2_extent.h"

#if !defined(BSTONE_SYS_WINDOW_SIZE_INCLUDED)
#define BSTONE_SYS_WINDOW_SIZE_INCLUDED

namespace bstone {
namespace sys {

struct WindowSize : R2Extent
{
	using R2Extent::R2Extent;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_WINDOW_SIZE_INCLUDED
