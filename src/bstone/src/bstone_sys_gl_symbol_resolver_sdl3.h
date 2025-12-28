/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver (SDL).

#ifndef BSTONE_SYS_GL_SYMBOL_RESOLVER_SDL3_INCLUDED
#define BSTONE_SYS_GL_SYMBOL_RESOLVER_SDL3_INCLUDED

#include "bstone_sys_gl_symbol_resolver.h"

namespace bstone {
namespace sys {

class Sdl3GlSymbolResolver final : public GlSymbolResolver
{
public:
	Sdl3GlSymbolResolver();
	~Sdl3GlSymbolResolver() override;

private:
	GlSymbolResolverSymbolFunc do_find_symbol(const char* symbol_name) const noexcept override;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_SYMBOL_RESOLVER_SDL3_INCLUDED
