/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_video_mgr.h"

namespace bstone {
namespace sys {

DisplayMode VideoMgr::get_current_display_mode()
try
{
	return do_get_current_display_mode();
}
BSTONE_STATIC_THROW_NESTED_FUNC

Span<const DisplayMode> VideoMgr::get_display_modes()
try
{
	return do_get_display_modes();
}
BSTONE_STATIC_THROW_NESTED_FUNC

GlMgrUPtr VideoMgr::make_gl_mgr()
try
{
	return do_make_gl_mgr();
}
BSTONE_STATIC_THROW_NESTED_FUNC

MouseMgrUPtr VideoMgr::make_mouse_mgr()
try
{
	return do_make_mouse_mgr();
}
BSTONE_STATIC_THROW_NESTED_FUNC

WindowMgrUPtr VideoMgr::make_window_mgr()
try
{
	return do_make_window_mgr();
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
