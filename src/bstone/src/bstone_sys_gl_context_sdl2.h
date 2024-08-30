/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_GL_CONTEXT_SDL2_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_SDL2_INCLUDED

#include "SDL_video.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_gl_context.h"

namespace bstone {
namespace sys {

GlContextUPtr make_sdl2_gl_context(Logger& logger, SDL_Window& sdl_window);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_CONTEXT_SDL2_INCLUDED
