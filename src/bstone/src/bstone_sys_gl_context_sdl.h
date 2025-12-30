/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context (SDL)

#ifndef BSTONE_SYS_GL_CONTEXT_SDL_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_gl_context.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

GlContextUPtr make_gl_context_sdl(Logger& logger, SDL_Window& sdl_window);

} // namespace bstone::sys

#endif // BSTONE_SYS_GL_CONTEXT_SDL_INCLUDED
