/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context

#include "bstone_sys_gl_current_context.h"

namespace bstone::sys {

bool GlCurrentContext::has_extension(const char* extension_name) const
{
	return do_has_extension(extension_name);
}

SwapIntervalType GlCurrentContext::get_swap_interval() const
{
	return do_get_swap_interval();
}

void GlCurrentContext::set_swap_interval(SwapIntervalType swap_interval_type)
{
	do_set_swap_interval(swap_interval_type);
}

const GlSymbolResolver& GlCurrentContext::get_symbol_resolver() const
{
	return do_get_symbol_resolver();
}

} // namespace bstone::sys
