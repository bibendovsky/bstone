/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_video_mgr_sdl3.h"

#include <algorithm>
#include <iterator>

#include "SDL3/SDL.h"

#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_sys_logger.h"
#include "bstone_sys_detail_sdl3.h"
#include "bstone_sys_exception_sdl3.h"
#include "bstone_sys_gl_current_context_sdl3.h"
#include "bstone_sys_limits_sdl3.h"
#include "bstone_sys_video_mgr_null.h"
#include "bstone_sys_mouse_mgr_sdl3.h"
#include "bstone_sys_vulkan_mgr_sdl3.h"
#include "bstone_sys_window_mgr_sdl3.h"
#include "bstone_sys_sdl3_subsystem.h"

namespace bstone {
namespace sys {

namespace {

class Sdl3VideoMgr final : public VideoMgr
{
public:
	Sdl3VideoMgr(Logger& logger);
	Sdl3VideoMgr(const Sdl3VideoMgr&) = delete;
	Sdl3VideoMgr& operator=(const Sdl3VideoMgr&) = delete;
	~Sdl3VideoMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	using DisplayModeCache = DisplayMode[limits::max_display_modes];

private:
	Logger& logger_;
	Sdl3Subsystem sdl3_subsystem_{};
	bool is_initialized_{};
	MouseMgrUPtr mouse_mgr_{};
	WindowMgrUPtr window_mgr_{};
	DisplayModeCache display_mode_cache_{};
	GlCurrentContextUPtr gl_current_context_{};
	VulkanMgrUPtr vulkan_mgr_{};

private:
	bool do_is_initialized() const noexcept override;

	Logger& do_get_logger() override;

	DisplayMode do_get_current_display_mode() override;
	std::span<const DisplayMode> do_get_display_modes() override;

	GlCurrentContext& do_get_gl_current_context() override;
	VulkanMgr& do_get_vulkan_mgr() override;
	MouseMgr& do_get_mouse_mgr() override;
	WindowMgr& do_get_window_mgr() override;

private:
	static MemoryResource& get_memory_resource();

	static void log_int(int value, std::string& message);
	static void log_rect(const SDL_Rect& rect, std::string& message);

	void log_drivers(std::string& message);
	static void log_display_mode(const SDL_DisplayMode& mode, std::string& message);
	void log_displays(std::string& message);
	void log_info() noexcept;

	static DisplayMode map_display_mode(const SDL_DisplayMode& sdl_display_mode) noexcept;
};

// ==========================================================================

Sdl3VideoMgr::Sdl3VideoMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL video manager.");

	auto sdl3_subsystem = Sdl3Subsystem{SDL_INIT_VIDEO};
	log_info();

	mouse_mgr_ = make_sdl3_mouse_mgr(logger);
	window_mgr_ = make_sdl3_window_mgr(logger);
	gl_current_context_ = make_sdl3_gl_current_context(logger_);
	sdl3_subsystem_.swap(sdl3_subsystem);
	is_initialized_ = true;

	logger_.log_information(">>> SDL video manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3VideoMgr::~Sdl3VideoMgr()
{
	logger_.log_information("Shut down SDL video manager.");

	gl_current_context_ = nullptr;
	vulkan_mgr_ = nullptr;
	window_mgr_ = nullptr;
	mouse_mgr_ = nullptr;
}

void* Sdl3VideoMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3VideoMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

bool Sdl3VideoMgr::do_is_initialized() const noexcept
{
	return is_initialized_;
}

Logger& Sdl3VideoMgr::do_get_logger()
{
	return logger_;
}

DisplayMode Sdl3VideoMgr::do_get_current_display_mode()
try {
	BSTONE_ASSERT(is_initialized_);
	const SDL_DisplayMode* sdl_display_mode = sdl3_ensure_result(SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()));
	return map_display_mode(*sdl_display_mode);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::span<const DisplayMode> Sdl3VideoMgr::do_get_display_modes()
try {
	BSTONE_ASSERT(is_initialized_);
	int sdl_mode_count;
	SDL_DisplayMode** const sdl_display_modes =
		sdl3_ensure_result(SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &sdl_mode_count));
	const int mode_count = std::min(sdl_mode_count, limits::max_display_modes);
	for (int i_mode = 0; i_mode < mode_count; ++i_mode)
	{
		display_mode_cache_[i_mode] = map_display_mode(*(sdl_display_modes[i_mode]));
	}
	SDL_free(sdl_display_modes);
	return std::span<const DisplayMode>{display_mode_cache_, static_cast<std::size_t>(mode_count)};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlCurrentContext& Sdl3VideoMgr::do_get_gl_current_context()
{
	BSTONE_ASSERT(is_initialized_);

	return *gl_current_context_;
}

VulkanMgr& Sdl3VideoMgr::do_get_vulkan_mgr()
{
	BSTONE_ASSERT(is_initialized_);

	if (vulkan_mgr_ == nullptr)
	{
		vulkan_mgr_ = make_sdl3_vulkan_mgr(logger_);
	}

	return *vulkan_mgr_;
}

MouseMgr& Sdl3VideoMgr::do_get_mouse_mgr()
{
	BSTONE_ASSERT(is_initialized_);

	return *mouse_mgr_;
}

WindowMgr& Sdl3VideoMgr::do_get_window_mgr()
{
	BSTONE_ASSERT(is_initialized_);

	return *window_mgr_;
}

MemoryResource& Sdl3VideoMgr::get_memory_resource()
{
	static SinglePoolResource<Sdl3VideoMgr> memory_pool{};

	return memory_pool;
}

void Sdl3VideoMgr::log_int(int value, std::string& message)
{
	char chars[11];
	const auto digit_count = to_chars(value, std::begin(chars), std::end(chars)) - chars;
	message.append(chars, static_cast<std::size_t>(digit_count));
}

void Sdl3VideoMgr::log_rect(const SDL_Rect& rect, std::string& message)
{
	message += "(x: ";
	log_int(rect.x, message);
	message += "; y: ";
	log_int(rect.y, message);
	message += "; w: ";
	log_int(rect.w, message);
	message += "; h: ";
	log_int(rect.h, message);
	message += ')';
}

void Sdl3VideoMgr::log_drivers(std::string& message)
{
	//
	const auto current_driver = SDL_GetCurrentVideoDriver();

	message += "Current driver: \"";
	message += (current_driver != nullptr ? current_driver : "???");
	message += '"';
	detail::sdl3_log_eol(message);

	//
	const auto driver_count = SDL_GetNumVideoDrivers();

	if (driver_count == 0)
	{
		message = "No built-in drivers.";
		detail::sdl3_log_eol(message);
		return;
	}

	message = "Built-in drivers:";
	detail::sdl3_log_eol(message);

	for (auto i = decltype(driver_count){}; i < driver_count; ++i)
	{
		const auto sdl_driver_name = SDL_GetVideoDriver(i);

		message += "  \"";
		message += (sdl_driver_name != nullptr ? sdl_driver_name : "???");
		message += '"';
		detail::sdl3_log_eol(message);
	}
}

void Sdl3VideoMgr::log_display_mode(const SDL_DisplayMode& mode, std::string& message)
{
	log_int(mode.w, message);
	message += 'x';
	log_int(mode.h, message);
	message += ' ';
	log_int(mode.refresh_rate, message);
	message += " Hz";
}

void Sdl3VideoMgr::log_displays(std::string& message)
{
	message += "Displays:";
	detail::sdl3_log_eol(message);
	int display_count;
	SDL_DisplayID* const sdl_display_ids = SDL_GetDisplays(&display_count);
	if (sdl_display_ids != nullptr)
	{
		for (int i_display = 0; i_display < display_count; ++i_display)
		{
			const SDL_DisplayID sdl_display_id = sdl_display_ids[i_display];
			message += "  ";
			detail::sdl3_log_xint(i_display + 1, message);
			message += '.';
			detail::sdl3_log_eol(message);
			{
				const char* const sdl_name = SDL_GetDisplayName(sdl_display_id);
				message += "    Name: ";
				if (sdl_name != nullptr)
				{
					message += sdl_name;
				}
				else
				{
					message += SDL_GetError();
				}
				detail::sdl3_log_eol(message);
			}
		}
		SDL_free(sdl_display_ids);
	}
	else
	{
		message += "  [ERROR] ";
		message += SDL_GetError();
	}
	logger_.log_information(message.c_str());
}

void Sdl3VideoMgr::log_info() noexcept
try
{
	auto message = std::string{};
	message.reserve(4096);

	log_drivers(message);
	log_displays(message);

	logger_.log_information(message.c_str());
}
catch (...) {}

DisplayMode Sdl3VideoMgr::map_display_mode(const SDL_DisplayMode& sdl_display_mode) noexcept
{
	auto display_mode = DisplayMode{};
	display_mode.width = sdl_display_mode.w;
	display_mode.height = sdl_display_mode.h;
	display_mode.refresh_rate = sdl_display_mode.refresh_rate;
	return display_mode;
}

} // namespace

// ==========================================================================

VideoMgrUPtr make_sdl3_video_mgr(Logger& logger)
try
{
	return std::make_unique<Sdl3VideoMgr>(logger);
}
catch (...)
{
	return make_null_video_mgr(logger);
}

} // namespace sys
} // namespace bstone
