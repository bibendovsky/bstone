/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Software accelerated video (SW).

#ifndef BSTONE_SW_VIDEO_INCLUDED
#define BSTONE_SW_VIDEO_INCLUDED

#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"
#include "bstone_video.h"

namespace bstone {

VideoUPtr make_sw_video(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);

} // namespace bstone

#endif // BSTONE_SW_VIDEO_INCLUDED
