/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer

#ifndef BSTONE_VK_R3R_INCLUDED
#define BSTONE_VK_R3R_INCLUDED

#include "bstone_r3r.h"
#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"

// ======================================

namespace bstone {

R3rUPtr make_vk_r3r(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_INCLUDED
