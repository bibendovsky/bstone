/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context (SDL).

#ifndef BSTONE_SYS_GL_CURRENT_CONTEXT_SDL_INCLUDED
#define BSTONE_SYS_GL_CURRENT_CONTEXT_SDL_INCLUDED

#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

GlCurrentContextUPtr make_gl_current_context_sdl(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_GL_CURRENT_CONTEXT_SDL_INCLUDED
