/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context

#ifndef BSTONE_SYS_GL_CURRENT_CONTEXT_INCLUDED
#define BSTONE_SYS_GL_CURRENT_CONTEXT_INCLUDED

#include "bstone_sys_gl_symbol_resolver.h"
#include "bstone_sys_swap_interval_type.h"
#include <memory>

namespace bstone::sys {

class GlCurrentContext
{
public:
	GlCurrentContext() = default;
	virtual ~GlCurrentContext() = default;

	virtual bool has_extension(const char* extension_name) const = 0;
	virtual SwapIntervalType get_swap_interval() const = 0;
	virtual void set_swap_interval(SwapIntervalType swap_interval_type) = 0;
	virtual const GlSymbolResolver& get_symbol_resolver() const = 0;
};

// ======================================

using GlCurrentContextUPtr = std::unique_ptr<GlCurrentContext>;

} // namespace bstone::sys

#endif // BSTONE_SYS_GL_CURRENT_CONTEXT_INCLUDED
