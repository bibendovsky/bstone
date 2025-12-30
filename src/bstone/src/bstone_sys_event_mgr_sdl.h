/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Event manager (SDL)

#ifndef BSTONE_SYS_EVENT_MGR_SDL_INCLUDED
#define BSTONE_SYS_EVENT_MGR_SDL_INCLUDED

#include "bstone_sys_event_mgr.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

EventMgrUPtr make_event_mgr_sdl(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_EVENT_MGR_SDL_INCLUDED
