/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_VIDEO_MGR_INCLUDED)
#define BSTONE_SYS_VIDEO_MGR_INCLUDED

#include <memory>
#include "bstone_span.h"
#include "bstone_sys_gl_mgr.h"
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
	VideoMgr() = default;
	virtual ~VideoMgr() = default;

	DisplayMode get_current_display_mode();

	// Notes:
	//   - The returned data valid until next call of the method.
	Span<const DisplayMode> get_display_modes();

	GlMgrUPtr make_gl_mgr();
	MouseMgrUPtr make_mouse_mgr();
	WindowMgrUPtr make_window_mgr();

private:
	virtual DisplayMode do_get_current_display_mode() = 0;
	virtual Span<const DisplayMode> do_get_display_modes() = 0;

	virtual GlMgrUPtr do_make_gl_mgr() = 0;
	virtual MouseMgrUPtr do_make_mouse_mgr() = 0;
	virtual WindowMgrUPtr do_make_window_mgr() = 0;
};

// ==========================================================================

using VideoMgrUPtr = std::unique_ptr<VideoMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_VIDEO_MGR_INCLUDED
