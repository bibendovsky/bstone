/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_mouse_mgr.h"

namespace bstone {
namespace sys {

void MouseMgr::set_relative_mode(bool is_enable)
BSTONE_BEGIN_FUNC_TRY
	do_set_relative_mode(is_enable);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
