/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// 3D renderer manager.
//

#if !defined(BSTONE_REN_3D_MGR_INCLUDED)
#define BSTONE_REN_3D_MGR_INCLUDED

#include "bstone_ren_3d.h"
#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"

namespace bstone {

class Ren3dMgr
{
public:
	Ren3dMgr() noexcept = default;
	virtual ~Ren3dMgr() = default;

	virtual Ren3dPtr renderer_initialize(const Ren3dCreateParam& param) = 0;
};

// ==========================================================================

using Ren3dMgrUPtr = std::unique_ptr<Ren3dMgr>;

struct Ren3dMgrFactory
{
	static Ren3dMgrUPtr create(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);
};

} // namespace bstone

#endif // BSTONE_REN_3D_MGR_INCLUDED
