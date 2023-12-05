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
try {
	return do_get_current_display_mode();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Span<const DisplayMode> VideoMgr::get_display_modes()
try {
	return do_get_display_modes();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlCurrentContext& VideoMgr::get_gl_current_context()
{
	return do_get_gl_current_context();
}

MouseMgrUPtr VideoMgr::make_mouse_mgr()
try {
	return do_make_mouse_mgr();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowMgrUPtr VideoMgr::make_window_mgr()
try {
	return do_make_window_mgr();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
