/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Mouse manager.

#include "bstone_sys_mouse_mgr.h"

namespace bstone {
namespace sys {

MouseMgr::MouseMgr() = default;

MouseMgr::~MouseMgr() = default;

void MouseMgr::set_relative_mode(bool is_relative)
{
	do_set_relative_mode(is_relative);
}

} // namespace sys
} // namespace bstone
