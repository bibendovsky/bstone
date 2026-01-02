/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Event manager (NULL)

#ifndef BSTONE_SYS_EVENT_MGR_NULL_INCLUDED
#define BSTONE_SYS_EVENT_MGR_NULL_INCLUDED

#include "bstone_sys_event_mgr.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

EventMgrUPtr make_event_mgr_null(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_EVENT_MGR_NULL_INCLUDED
