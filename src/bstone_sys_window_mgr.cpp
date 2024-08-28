/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window manager.

#include "bstone_sys_window_mgr.h"

namespace bstone {
namespace sys {

WindowMgr::WindowMgr() = default;

WindowMgr::~WindowMgr() = default;

WindowUPtr WindowMgr::make_window(const WindowInitParam& param)
{
	return do_make_window(param);
}

} // namespace sys
} // namespace bstone
