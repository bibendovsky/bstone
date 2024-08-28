/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Mouse manager.

#ifndef BSTONE_SYS_MOUSE_MGR_INCLUDED
#define BSTONE_SYS_MOUSE_MGR_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class MouseMgr
{
public:
	MouseMgr();
	virtual ~MouseMgr();

	void set_relative_mode(bool is_relative);

private:
	virtual void do_set_relative_mode(bool is_relative) = 0;
};

// ==========================================================================

using MouseMgrUPtr = std::unique_ptr<MouseMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_MOUSE_MGR_INCLUDED

