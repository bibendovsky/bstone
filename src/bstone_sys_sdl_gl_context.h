/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_GL_CONTEXT_INCLUDED)
#define BSTONE_SYS_SDL_GL_CONTEXT_INCLUDED

#include "SDL_video.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_gl_context.h"

namespace bstone {
namespace sys {

GlContextUPtr make_sdl_gl_context(Logger& logger, SDL_Window& sdl_window);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_GL_CONTEXT_INCLUDED
