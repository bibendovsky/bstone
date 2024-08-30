/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Manager

#ifndef BSTONE_R3R_MGR_INCLUDED
#define BSTONE_R3R_MGR_INCLUDED

#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"
#include "bstone_r3r.h"

namespace bstone {

class R3rMgr
{
public:
	R3rMgr() noexcept;
	virtual ~R3rMgr();

	// TODO Rename to "make...".
	R3r* renderer_initialize(const R3rInitParam& param);

private:
	virtual R3r* do_renderer_initialize(const R3rInitParam& param) = 0;
};

// ==========================================================================

using R3rMgrUPtr = std::unique_ptr<R3rMgr>;

R3rMgrUPtr make_r3r_mgr(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);

} // namespace bstone

#endif // BSTONE_R3R_MGR_INCLUDED
