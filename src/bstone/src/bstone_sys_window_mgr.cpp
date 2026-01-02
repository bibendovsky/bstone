/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window manager

#include "bstone_sys_window_mgr.h"

namespace bstone::sys {

WindowUPtr WindowMgr::make_window(const WindowInitParam& param)
{
	return do_make_window(param);
}

} // namespace bstone::sys
