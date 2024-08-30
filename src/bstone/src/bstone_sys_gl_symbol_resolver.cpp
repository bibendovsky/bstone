/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver.

#include "bstone_sys_gl_symbol_resolver.h"

namespace bstone {
namespace sys {

GlSymbolResolver::GlSymbolResolver() = default;

GlSymbolResolver::~GlSymbolResolver() = default;

void* GlSymbolResolver::find_symbol(const char* symbol_name) const noexcept
{
	return do_find_symbol(symbol_name);
}

} // namespace sys
} // namespace bstone
