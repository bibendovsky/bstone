/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_video_mgr_null.h"

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

namespace {

class NullVideoMgr final : public VideoMgr
{
public:
	NullVideoMgr(Logger& logger);
	~NullVideoMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;

private:
	bool do_is_initialized() const noexcept override;

	DisplayMode do_get_current_display_mode() override;
	Span<const DisplayMode> do_get_display_modes() override;

	GlCurrentContext& do_get_gl_current_context() override;
	MouseMgr& do_get_mouse_mgr() override;
	WindowMgr& do_get_window_mgr() override;

private:
	[[noreturn]] static void not_initialized();
};

// ==========================================================================

using NullVideoMgrPool = SinglePoolResource<NullVideoMgr>;
NullVideoMgrPool null_video_mgr_pool{};

// ==========================================================================

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

void* NullVideoMgr::operator new(std::size_t size)
{
	return null_video_mgr_pool.allocate(size);
}

void NullVideoMgr::operator delete(void* ptr) noexcept
{
	null_video_mgr_pool.deallocate(ptr);
}

bool NullVideoMgr::do_is_initialized() const noexcept
{
	return false;
}

DisplayMode NullVideoMgr::do_get_current_display_mode()
{
	not_initialized();
}

Span<const DisplayMode> NullVideoMgr::do_get_display_modes()
{
	not_initialized();
}

GlCurrentContext& NullVideoMgr::do_get_gl_current_context()
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

// ==========================================================================

VideoMgrUPtr make_null_video_mgr(Logger& logger)
try {
	return std::make_unique<NullVideoMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
