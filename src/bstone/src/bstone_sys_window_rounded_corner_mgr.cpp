/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager.

#include "bstone_sys_window_rounded_corner_mgr.h"

namespace bstone {
namespace sys {

WindowRoundedCornerMgr::WindowRoundedCornerMgr() = default;

WindowRoundedCornerMgr::~WindowRoundedCornerMgr() = default;

void WindowRoundedCornerMgr::set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type)
{
	do_set_round_corner_type(window, round_corner_type);
}

} // namespace sys
} // namespace bstone
