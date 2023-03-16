/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_RENDERER_INCLUDED)
#define BSTONE_SYS_SDL_RENDERER_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_renderer.h"
#include "bstone_sys_sdl_window.h"

struct SDL_Window;

namespace bstone {
namespace sys {

RendererUPtr make_sdl_renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_RENDERER_INCLUDED
