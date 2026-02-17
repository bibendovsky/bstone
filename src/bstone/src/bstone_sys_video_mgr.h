/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager

#ifndef BSTONE_SYS_VIDEO_MGR_INCLUDED
#define BSTONE_SYS_VIDEO_MGR_INCLUDED

#include "bstone_sys_display_mode.h"
#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_mouse_mgr.h"
#include "bstone_sys_vulkan_mgr.h"
#include "bstone_sys_window_mgr.h"
#include <memory>
#include <span>

namespace bstone::sys {

class VideoMgr
{
public:
	VideoMgr() = default;
	virtual ~VideoMgr() = default;

	virtual bool is_initialized() const = 0;
	virtual Logger& get_logger() = 0;
	virtual DisplayMode get_current_display_mode() = 0;
	// Notes:
	//   - The returned data valid until the next invocation.
	virtual std::span<const DisplayMode> get_display_modes() = 0;
	virtual GlCurrentContext& get_gl_current_context() = 0;
	virtual VulkanMgr& get_vulkan_mgr() = 0;
	virtual MouseMgr& get_mouse_mgr() = 0;
	virtual WindowMgr& get_window_mgr() = 0;
};

// ======================================

using VideoMgrUPtr = std::unique_ptr<VideoMgr>;

} // namespace bstone::sys

#endif // BSTONE_SYS_VIDEO_MGR_INCLUDED
