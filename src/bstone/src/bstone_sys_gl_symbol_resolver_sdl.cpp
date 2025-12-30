/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver (SDL).

#include "bstone_sys_gl_symbol_resolver_sdl.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

GlSymbolResolverSymbolFunc GlSymbolResolverSdl::do_find_symbol(const char* symbol_name) const noexcept
{
	return reinterpret_cast<GlSymbolResolverSymbolFunc>(SDL_GL_GetProcAddress(symbol_name));
}

} // namespace bstone::sys
