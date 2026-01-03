/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window decoration manager

#ifndef BSTONE_SYS_WINDOW_DECORATION_MGR_INCLUDED
#define BSTONE_SYS_WINDOW_DECORATION_MGR_INCLUDED

#include "bstone_sys_window.h"
#include <memory>

namespace bstone::sys {

class WindowDecorationMgr
{
public:
	WindowDecorationMgr() = default;
	virtual ~WindowDecorationMgr() = default;

	void set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type);

private:
	virtual void do_set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type) = 0;
};

// ======================================

using WindowDecorationMgrUPtr = std::unique_ptr<WindowDecorationMgr>;

WindowDecorationMgrUPtr make_window_decoration_mgr();

} // namespace bstone::sys

#endif // BSTONE_SYS_WINDOW_DECORATION_MGR_INCLUDED
