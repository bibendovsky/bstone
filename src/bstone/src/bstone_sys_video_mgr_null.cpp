/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager (NULL)

#include "bstone_sys_video_mgr_null.h"
#include "bstone_exception.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

namespace {

class NullVideoMgr final : public VideoMgr
{
public:
	NullVideoMgr(Logger& logger);
	~NullVideoMgr() override;

private:
	Logger& logger_;

	bool do_is_initialized() const override;
	Logger& do_get_logger() override;
	DisplayMode do_get_current_display_mode() override;
	std::span<const DisplayMode> do_get_display_modes() override;
	GlCurrentContext& do_get_gl_current_context() override;
	VulkanMgr& do_get_vulkan_mgr() override;
	MouseMgr& do_get_mouse_mgr() override;
	WindowMgr& do_get_window_mgr() override;

	[[noreturn]] static void not_initialized();
};

// ======================================

NullVideoMgr::NullVideoMgr(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up NULL video manager.");
}

NullVideoMgr::~NullVideoMgr()
{
	logger_.log_information("Shut down NULL video manager.");
}

bool NullVideoMgr::do_is_initialized() const
{
	return false;
}

Logger& NullVideoMgr::do_get_logger()
{
	return logger_;
}

DisplayMode NullVideoMgr::do_get_current_display_mode()
{
	not_initialized();
}

std::span<const DisplayMode> NullVideoMgr::do_get_display_modes()
{
	not_initialized();
}

GlCurrentContext& NullVideoMgr::do_get_gl_current_context()
{
	not_initialized();
}

VulkanMgr& NullVideoMgr::do_get_vulkan_mgr()
{
	not_initialized();
}

MouseMgr& NullVideoMgr::do_get_mouse_mgr()
{
	not_initialized();
}

WindowMgr& NullVideoMgr::do_get_window_mgr()
{
	not_initialized();
}

[[noreturn]] void NullVideoMgr::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ======================================

VideoMgrUPtr make_video_mgr_null(Logger& logger)
{
	return std::make_unique<NullVideoMgr>(logger);
}

} // namespace bstone::sys
