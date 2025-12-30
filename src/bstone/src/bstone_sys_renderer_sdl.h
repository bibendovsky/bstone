/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_RENDERER_SDL_INCLUDED
#define BSTONE_SYS_RENDERER_SDL_INCLUDED

// 2D renderer (SDL)

#include "bstone_sys_logger.h"
#include "bstone_sys_renderer.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

RendererUPtr make_renderer_sdl(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param);

} // namespace bstone::sys

#endif // BSTONE_SYS_RENDERER_SDL_INCLUDED
