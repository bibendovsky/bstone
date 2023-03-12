/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_event_mgr.h"

namespace bstone {
namespace sys {

bool EventMgr::poll_event(Event& e)
try
{
	return do_poll_event(e);
}
BSTONE_FUNC_STATIC_THROW_NESTED

} // namespace sys
} // namespace bstone
