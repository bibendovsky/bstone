/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager.

#ifndef BSTONE_SYS_WINDOW_ROUNDED_CORNER_MGR_INCLUDED
#define BSTONE_SYS_WINDOW_ROUNDED_CORNER_MGR_INCLUDED

#include <memory>

#include "bstone_sys_window.h"

namespace bstone {
namespace sys {

class WindowRoundedCornerMgr
{
public:
	WindowRoundedCornerMgr();
	virtual ~WindowRoundedCornerMgr();

	void set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type);

private:
	virtual void do_set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type) = 0;
};

// ==========================================================================

using WindowRoundedCornerMgrUPtr = std::unique_ptr<WindowRoundedCornerMgr>;

WindowRoundedCornerMgrUPtr make_window_rounded_corner_mgr();

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_WINDOW_ROUNDED_CORNER_MGR_INCLUDED
