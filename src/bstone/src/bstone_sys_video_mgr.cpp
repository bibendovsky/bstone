/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager.

#include "bstone_sys_video_mgr.h"

namespace bstone {
namespace sys {

VideoMgr::VideoMgr() = default;

VideoMgr::~VideoMgr() = default;

bool VideoMgr::is_initialized() const noexcept
{
	return do_is_initialized();
}

DisplayMode VideoMgr::get_current_display_mode()
{
	return do_get_current_display_mode();
}

Span<const DisplayMode> VideoMgr::get_display_modes()
{
	return do_get_display_modes();
}

GlCurrentContext& VideoMgr::get_gl_current_context()
{
	return do_get_gl_current_context();
}

MouseMgr& VideoMgr::get_mouse_mgr()
{
	return do_get_mouse_mgr();
}

WindowMgr& VideoMgr::get_window_mgr()
{
	return do_get_window_mgr();
}

} // namespace sys
} // namespace bstone
