/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_GL_SHARED_LIBRARY_INCLUDED)
#define BSTONE_SYS_SDL_GL_SHARED_LIBRARY_INCLUDED

#include <memory>
#include "bstone_sys_gl_shared_library.h"

namespace bstone {
namespace sys {

GlSharedLibraryUPtr make_sdl_gl_shared_library(const char* path = nullptr);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_GL_SHARED_LIBRARY_INCLUDED
