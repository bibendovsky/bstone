/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver (SDL).

#include <SDL_video.h>

#include "bstone_sys_gl_symbol_resolver_sdl2.h"

namespace bstone {
namespace sys {

Sdl2GlSymbolResolver::Sdl2GlSymbolResolver() = default;

Sdl2GlSymbolResolver::~Sdl2GlSymbolResolver() = default;

void* Sdl2GlSymbolResolver::do_find_symbol(const char* symbol_name) const noexcept
{
	return SDL_GL_GetProcAddress(symbol_name);
}

} // namespace sys
} // namespace bstone
