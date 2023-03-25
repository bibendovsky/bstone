/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_GL_MGR_INCLUDED)
#define BSTONE_SYS_GL_MGR_INCLUDED

#include <memory>

#include "bstone_sys_gl_shared_library.h"

namespace bstone {
namespace sys {

class GlMgr
{
public:
	GlMgr() = default;
	virtual ~GlMgr() = default;

	GlSharedLibraryUPtr make_shared_library(const char* path = nullptr);

	int get_swap_interval() noexcept;
	void set_swap_interval(int swap_interval);

private:
	virtual GlSharedLibraryUPtr do_make_shared_library(const char* path) = 0;

	virtual bool do_has_extension(const char* extension_name) = 0;

	virtual int do_get_swap_interval() noexcept = 0;
	virtual void do_set_swap_interval(int swap_interval) = 0;
};

// ==========================================================================

using GlMgrUPtr = std::unique_ptr<GlMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_MGR_INCLUDED
