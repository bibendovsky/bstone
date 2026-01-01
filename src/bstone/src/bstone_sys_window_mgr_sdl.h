/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window manager (SDL)

#ifndef BSTONE_SYS_WINDOW_MGR_SDL_INCLUDED
#define BSTONE_SYS_WINDOW_MGR_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_window_mgr.h"

namespace bstone::sys {

WindowMgrUPtr make_window_mgr_sdl(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_WINDOW_MGR_SDL_INCLUDED
