/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_gl_mgr.h"

namespace bstone {
namespace sys {

void GlMgr::load_default_library()
try
{
	load_library(nullptr);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void GlMgr::load_library(const char* path)
try
{
	do_load_library(path);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void GlMgr::unload_library() noexcept
{
	do_unload_library();
}

void* GlMgr::get_symbol(const char* symbol_name)
try
{
	return do_get_symbol(symbol_name);
}
BSTONE_STATIC_THROW_NESTED_FUNC

int GlMgr::get_swap_interval() noexcept
{
	return do_get_swap_interval();
}

void GlMgr::set_swap_interval(int swap_interval)
try
{
	do_set_swap_interval(swap_interval);
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
