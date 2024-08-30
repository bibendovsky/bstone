/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context (SDL).

#ifndef BSTONE_SYS_GL_CURRENT_CONTEXT_SDL2_INCLUDED
#define BSTONE_SYS_GL_CURRENT_CONTEXT_SDL2_INCLUDED

#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

GlCurrentContextUPtr make_sdl2_gl_current_context(Logger& logger);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_CURRENT_CONTEXT_SDL2_INCLUDED
