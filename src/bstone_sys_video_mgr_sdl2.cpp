/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_video_mgr_sdl2.h"

#include <algorithm>
#include <iterator>

#include "SDL.h"

#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_sys_logger.h"
#include "bstone_sys_detail_sdl2.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_gl_current_context_sdl2.h"
#include "bstone_sys_limits_sdl2.h"
#include "bstone_sys_video_mgr_null.h"
#include "bstone_sys_mouse_mgr_sdl2.h"
#include "bstone_sys_window_mgr_sdl2.h"
#include "bstone_sys_sdl2_subsystem.h"

namespace bstone {
namespace sys {

namespace {

class Sdl2VideoMgr final : public VideoMgr
{
public:
	Sdl2VideoMgr(Logger& logger);
	Sdl2VideoMgr(const Sdl2VideoMgr&) = delete;
	Sdl2VideoMgr& operator=(const Sdl2VideoMgr&) = delete;
	~Sdl2VideoMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	using DisplayModeCache = DisplayMode[limits::max_display_modes];

private:
	Logger& logger_;
	Sdl2Subsystem sdl2_subsystem_{};
	bool is_initialized_{};
	MouseMgrUPtr mouse_mgr_{};
	WindowMgrUPtr window_mgr_{};
	DisplayModeCache display_mode_cache_{};
	GlCurrentContextUPtr gl_current_context_{};

private:
	bool do_is_initialized() const noexcept override;

	DisplayMode do_get_current_display_mode() override;
	Span<const DisplayMode> do_get_display_modes() override;

	GlCurrentContext& do_get_gl_current_context() override;
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

Sdl2VideoMgr::Sdl2VideoMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL video manager.");

	auto sdl2_subsystem = Sdl2Subsystem{SDL_INIT_VIDEO};
	log_info();

	mouse_mgr_ = make_sdl2_mouse_mgr(logger);
	window_mgr_ = make_sdl2_window_mgr(logger);
	gl_current_context_ = make_sdl2_gl_current_context(logger_);
	sdl2_subsystem_.swap(sdl2_subsystem);
	is_initialized_ = true;

	logger_.log_information(">>> SDL video manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2VideoMgr::~Sdl2VideoMgr()
{
	logger_.log_information("Shut down SDL video manager.");

	gl_current_context_ = nullptr;
	window_mgr_ = nullptr;
	mouse_mgr_ = nullptr;
}

void* Sdl2VideoMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2VideoMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

bool Sdl2VideoMgr::do_is_initialized() const noexcept
{
	return is_initialized_;
}

DisplayMode Sdl2VideoMgr::do_get_current_display_mode()
try {
	BSTONE_ASSERT(is_initialized_);

	auto sdl_display_mode = SDL_DisplayMode{};
	sdl2_ensure_result(SDL_GetCurrentDisplayMode(0, &sdl_display_mode));
	return map_display_mode(sdl_display_mode);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Span<const DisplayMode> Sdl2VideoMgr::do_get_display_modes()
try {
	BSTONE_ASSERT(is_initialized_);

	const auto count = std::min(SDL_GetNumDisplayModes(0), limits::max_display_modes);

	auto sdl_display_mode = SDL_DisplayMode{};

	for (auto i = 0; i < count; ++i)
	{
		sdl2_ensure_result(SDL_GetDisplayMode(0, i, &sdl_display_mode));
		display_mode_cache_[i] = map_display_mode(sdl_display_mode);
	}

	return Span<const DisplayMode>{display_mode_cache_, count};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlCurrentContext& Sdl2VideoMgr::do_get_gl_current_context()
{
	BSTONE_ASSERT(is_initialized_);

	return *gl_current_context_;
}

MouseMgr& Sdl2VideoMgr::do_get_mouse_mgr()
{
	BSTONE_ASSERT(is_initialized_);

	return *mouse_mgr_;
}

WindowMgr& Sdl2VideoMgr::do_get_window_mgr()
{
	BSTONE_ASSERT(is_initialized_);

	return *window_mgr_;
}

MemoryResource& Sdl2VideoMgr::get_memory_resource()
{
	static SinglePoolResource<Sdl2VideoMgr> memory_pool{};

	return memory_pool;
}

void Sdl2VideoMgr::log_int(int value, std::string& message)
{
	char chars[11];
	const auto digit_count = to_chars(value, std::begin(chars), std::end(chars)) - chars;
	message.append(chars, static_cast<std::size_t>(digit_count));
}

void Sdl2VideoMgr::log_rect(const SDL_Rect& rect, std::string& message)
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

void Sdl2VideoMgr::log_drivers(std::string& message)
{
	//
	const auto current_driver = SDL_GetCurrentVideoDriver();

	message += "Current driver: \"";
	message += (current_driver != nullptr ? current_driver : "???");
	message += '"';
	detail::sdl2_log_eol(message);

	//
	const auto driver_count = SDL_GetNumVideoDrivers();

	if (driver_count == 0)
	{
		message = "No built-in drivers.";
		detail::sdl2_log_eol(message);
		return;
	}

	message = "Built-in drivers:";
	detail::sdl2_log_eol(message);

	for (auto i = decltype(driver_count){}; i < driver_count; ++i)
	{
		const auto sdl_driver_name = SDL_GetVideoDriver(i);

		message += "  \"";
		message += (sdl_driver_name != nullptr ? sdl_driver_name : "???");
		message += '"';
		detail::sdl2_log_eol(message);
	}
}

void Sdl2VideoMgr::log_display_mode(const SDL_DisplayMode& mode, std::string& message)
{
	log_int(mode.w, message);
	message += 'x';
	log_int(mode.h, message);
	message += ' ';
	log_int(mode.refresh_rate, message);
	message += " Hz";
}

void Sdl2VideoMgr::log_displays(std::string& message)
{
	//
	const auto display_count = SDL_GetNumVideoDisplays();

	if (display_count == 0)
	{
		message = "No displays.";
		detail::sdl2_log_eol(message);
		return;
	}

	message = "Displays:";
	detail::sdl2_log_eol(message);

	for (auto i = decltype(display_count){}; i < display_count; ++i)
	{
		const auto sdl_display_name = SDL_GetDisplayName(i);

		//
		message += "  ";
		log_int(i + 1, message);
		message += ". \"";
		message += (sdl_display_name != nullptr ? sdl_display_name : "???");
		message += '"';
		detail::sdl2_log_eol(message);

		//
		message += "  Bounds: ";
		auto bounds = SDL_Rect{};

		if (SDL_GetDisplayBounds(i, &bounds) == 0)
		{
			log_rect(bounds, message);
		}
		else
		{
			message += "???";
		}

		detail::sdl2_log_eol(message);

#if SDL_VERSION_ATLEAST(2, 0, 5)
		//
		message += "  Usable bounds: ";
		auto usable_bounds = SDL_Rect{};

		if (SDL_GetDisplayUsableBounds(i, &usable_bounds) == 0)
		{
			log_rect(usable_bounds, message);
		}
		else
		{
			message += "???";
		}

		detail::sdl2_log_eol(message);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 9)
		//
		message += "  Orientation: ";

		switch (SDL_GetDisplayOrientation(i))
		{
			case SDL_ORIENTATION_LANDSCAPE: message += "landscape"; break;
			case SDL_ORIENTATION_LANDSCAPE_FLIPPED: message += "landscape flipped"; break;
			case SDL_ORIENTATION_PORTRAIT: message += "portrait"; break;
			case SDL_ORIENTATION_PORTRAIT_FLIPPED: message += "portrait flipped"; break;
			default: message += "???"; break;
		}

		detail::sdl2_log_eol(message);
#endif

		//
		auto current_mode = SDL_DisplayMode{};

		message += "  Current mode: ";

		if (SDL_GetCurrentDisplayMode(i, &current_mode) == 0)
		{
			log_display_mode(current_mode, message);
		}
		else
		{
			message += "???";
		}

		detail::sdl2_log_eol(message);

		//
		const auto mode_count = SDL_GetNumDisplayModes(i);

		if (mode_count > 0)
		{
			message = "  Modes:";
			detail::sdl2_log_eol(message);

			for (auto j = decltype(mode_count){}; j < mode_count; ++j)
			{
				auto mode = SDL_DisplayMode{};

				if (SDL_GetDisplayMode(i, j, &mode) == 0)
				{
					message += "    ";
					log_display_mode(mode, message);
					detail::sdl2_log_eol(message);
				}
			}
		}
		else
		{
			message = "  No modes.";
			detail::sdl2_log_eol(message);
		}
	}
}

void Sdl2VideoMgr::log_info() noexcept
try
{
	auto message = std::string{};
	message.reserve(4096);

	log_drivers(message);
	log_displays(message);

	logger_.log_information(message.c_str());
}
catch (...) {}

DisplayMode Sdl2VideoMgr::map_display_mode(const SDL_DisplayMode& sdl_display_mode) noexcept
{
	auto display_mode = DisplayMode{};
	display_mode.width = sdl_display_mode.w;
	display_mode.height = sdl_display_mode.h;
	display_mode.refresh_rate = sdl_display_mode.refresh_rate;
	return display_mode;
}

} // namespace

// ==========================================================================

VideoMgrUPtr make_sdl2_video_mgr(Logger& logger)
try
{
	return std::make_unique<Sdl2VideoMgr>(logger);
}
catch (...)
{
	return make_null_video_mgr(logger);
}

} // namespace sys
} // namespace bstone
