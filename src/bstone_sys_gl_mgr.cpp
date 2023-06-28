/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_gl_mgr.h"

namespace bstone {
namespace sys {

GlSharedLibraryUPtr GlMgr::make_shared_library(const char* path)
BSTONE_BEGIN_FUNC_TRY
	return do_make_shared_library(path);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlMgr::get_swap_interval() noexcept
{
	return do_get_swap_interval();
}

void GlMgr::set_swap_interval(int swap_interval)
BSTONE_BEGIN_FUNC_TRY
	do_set_swap_interval(swap_interval);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
