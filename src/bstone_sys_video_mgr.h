/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager.

#ifndef BSTONE_SYS_VIDEO_MGR_INCLUDED
#define BSTONE_SYS_VIDEO_MGR_INCLUDED

#include <memory>

#include "bstone_span.h"

#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_mouse_mgr.h"
#include "bstone_sys_window_mgr.h"

namespace bstone {
namespace sys {

struct DisplayMode
{
	int width;
	int height;
	int refresh_rate;
};

class VideoMgr
{
public:
	VideoMgr();
	virtual ~VideoMgr();

	bool is_initialized() const noexcept;

	DisplayMode get_current_display_mode();

	// Notes:
	//   - The returned data valid until next invocation.
	Span<const DisplayMode> get_display_modes();

	GlCurrentContext& get_gl_current_context();
	MouseMgr& get_mouse_mgr();
	WindowMgr& get_window_mgr();

private:
	virtual bool do_is_initialized() const noexcept = 0;

	virtual DisplayMode do_get_current_display_mode() = 0;
	virtual Span<const DisplayMode> do_get_display_modes() = 0;

	virtual GlCurrentContext& do_get_gl_current_context() = 0;
	virtual MouseMgr& do_get_mouse_mgr() = 0;
	virtual WindowMgr& do_get_window_mgr() = 0;
};

// ==========================================================================

using VideoMgrUPtr = std::unique_ptr<VideoMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_VIDEO_MGR_INCLUDED
