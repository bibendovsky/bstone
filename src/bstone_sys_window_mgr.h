/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window manager.

#if !defined(BSTONE_SYS_WINDOW_MGR_INCLUDED)
#define BSTONE_SYS_WINDOW_MGR_INCLUDED

#include <memory>

#include "bstone_sys_window.h"

namespace bstone {
namespace sys {

class WindowMgr
{
public:
	WindowMgr() = default;
	virtual ~WindowMgr() = default;

	WindowUPtr make_window(const WindowInitParam& param);

private:
	virtual WindowUPtr do_make_window(const WindowInitParam& param) = 0;
};

// ==========================================================================

using WindowMgrUPtr = std::unique_ptr<WindowMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_WINDOW_MGR_INCLUDED
