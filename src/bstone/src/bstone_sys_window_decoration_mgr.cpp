/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window decoration manager

#include "bstone_sys_window_decoration_mgr.h"

namespace bstone::sys {

void WindowDecorationMgr::set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type)
{
	do_set_round_corner_type(window, round_corner_type);
}

} // namespace bstone::sys
