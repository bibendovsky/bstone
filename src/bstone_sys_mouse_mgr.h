/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_MOUSE_MGR_INCLUDED)
#define BSTONE_SYS_MOUSE_MGR_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class MouseMgr
{
public:
	MouseMgr() = default;
	virtual ~MouseMgr() = default;

	void set_relative_mode(bool is_enable);

private:
	virtual void do_set_relative_mode(bool is_enable) = 0;
};

// ==========================================================================

using MouseMgrUPtr = std::unique_ptr<MouseMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_MOUSE_MGR_INCLUDED

