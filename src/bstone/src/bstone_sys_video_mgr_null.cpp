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

	bool is_initialized() const override;
	Logger& get_logger() override;
	DisplayMode get_current_display_mode() override;
	std::span<const DisplayMode> get_display_modes() override;
	GlCurrentContext& get_gl_current_context() override;
	VulkanMgr& get_vulkan_mgr() override;
	MouseMgr& get_mouse_mgr() override;
	WindowMgr& get_window_mgr() override;

private:
	Logger& logger_;

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

bool NullVideoMgr::is_initialized() const
{
	return false;
}

Logger& NullVideoMgr::get_logger()
{
	return logger_;
}

DisplayMode NullVideoMgr::get_current_display_mode()
{
	not_initialized();
}

std::span<const DisplayMode> NullVideoMgr::get_display_modes()
{
	not_initialized();
}

GlCurrentContext& NullVideoMgr::get_gl_current_context()
{
	not_initialized();
}

VulkanMgr& NullVideoMgr::get_vulkan_mgr()
{
	not_initialized();
}

MouseMgr& NullVideoMgr::get_mouse_mgr()
{
	not_initialized();
}

WindowMgr& NullVideoMgr::get_window_mgr()
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
