/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager (SDL)

#ifndef BSTONE_SYS_VIDEO_MGR_SDL_INCLUDED
#define BSTONE_SYS_VIDEO_MGR_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_video_mgr.h"

namespace bstone::sys {

VideoMgrUPtr make_video_mgr_sdl(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_VIDEO_MGR_SDL_INCLUDED
