/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include "SDL.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_sdl_exception.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_sdl_mouse_mgr.h"
#include "bstone_sys_sdl_video_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlVideoMgr final : public VideoMgr
{
public:
	SdlVideoMgr(Logger& logger);
	SdlVideoMgr(const SdlVideoMgr&) = delete;
	SdlVideoMgr& operator=(const SdlVideoMgr&) = delete;
	~SdlVideoMgr() override;

private:
	Logger& logger_;

private:
	MouseMgrUPtr do_make_mouse_mgr() override;

private:
	static void log_int(int value, std::string& message);
	static void log_rect(const SDL_Rect& rect, std::string& message);

	void log_drivers();
	static void log_display_mode(const SDL_DisplayMode& mode, std::string& message);
	void log_displays();
	void log_info() noexcept;
};

// ==========================================================================

SdlVideoMgr::SdlVideoMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information();
	logger_.log_information("<<< Start up SDL video manager.");

	sdl_ensure_result(SDL_InitSubSystem(SDL_INIT_VIDEO));
	log_info();

	logger_.log_information(">>> SDL video manager started up.");
}
BSTONE_FUNC_STATIC_THROW_NESTED

SdlVideoMgr::~SdlVideoMgr()
{
	logger_.log_information();
	logger_.log_information("Shut down SDL video manager.");

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

MouseMgrUPtr SdlVideoMgr::do_make_mouse_mgr()
{
	return make_sdl_mouse_mgr(logger_);
}

void SdlVideoMgr::log_int(int value, std::string& message)
{
	char chars[11];
	const auto digit_count = char_conv::to_chars(value, make_span(chars), 10);
	message.append(chars, static_cast<std::size_t>(digit_count));
}

void SdlVideoMgr::log_rect(const SDL_Rect& rect, std::string& message)
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

void SdlVideoMgr::log_drivers()
{
	auto message = std::string{};
	message.reserve(256);

	//
	logger_.log_information();
	const auto current_driver = SDL_GetCurrentVideoDriver();

	message.clear();
	message += "Current driver: \"";
	message += (current_driver != nullptr ? current_driver : "???");
	message += '"';
	logger_.log_information(message);

	//
	logger_.log_information();
	const auto driver_count = SDL_GetNumVideoDrivers();

	if (driver_count == 0)
	{
		message = "No built-in drivers.";
		logger_.log_information(message);
		return;
	}

	message = "Built-in drivers:";
	logger_.log_information(message);

	for (auto i = decltype(driver_count){}; i < driver_count; ++i)
	{
		const auto sdl_driver_name = SDL_GetVideoDriver(i);

		message.clear();
		message += "  \"";
		message += (sdl_driver_name != nullptr ? sdl_driver_name : "???");
		message += '"';
		logger_.log_information(message);
	}
}

void SdlVideoMgr::log_display_mode(const SDL_DisplayMode& mode, std::string& message)
{
	log_int(mode.w, message);
	message += 'x';
	log_int(mode.h, message);
	message += ' ';
	log_int(mode.refresh_rate, message);
	message += " Hz";
}

void SdlVideoMgr::log_displays()
{
	auto message = std::string{};
	message.reserve(256);

	//
	logger_.log_information();
	const auto display_count = SDL_GetNumVideoDisplays();

	if (display_count == 0)
	{
		message = "No displays.";
		logger_.log_information(message);
		return;
	}

	message = "Displays:";
	logger_.log_information(message);

	for (auto i = decltype(display_count){}; i < display_count; ++i)
	{
		auto sdl_spec = SDL_AudioSpec{};
		const auto sdl_display_name = SDL_GetDisplayName(i);

		if (i != 0)
		{
			logger_.log_information();
		}

		//
		message.clear();
		message += "  ";
		log_int(i + 1, message);
		message += ". \"";
		message += (sdl_display_name != nullptr ? sdl_display_name : "???");
		message += '"';
		logger_.log_information(message);

		//
		message.clear();
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

		logger_.log_information(message);

#if SDL_VERSION_ATLEAST(2, 0, 5)
		//
		message.clear();
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

		logger_.log_information(message);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 9)
		//
		message.clear();
		message += "  Orientation: ";

		switch (SDL_GetDisplayOrientation(i))
		{
			case SDL_ORIENTATION_LANDSCAPE: message += "landscape"; break;
			case SDL_ORIENTATION_LANDSCAPE_FLIPPED: message += "landscape flipped"; break;
			case SDL_ORIENTATION_PORTRAIT: message += "portrait"; break;
			case SDL_ORIENTATION_PORTRAIT_FLIPPED: message += "portrait flipped"; break;
			default: message += "???"; break;
		}

		logger_.log_information(message);
#endif

		//
		auto current_mode = SDL_DisplayMode{};

		message.clear();
		message += "  Current mode: ";

		if (SDL_GetCurrentDisplayMode(i, &current_mode) == 0)
		{
			log_display_mode(current_mode, message);
		}
		else
		{
			message += "???";
		}

		logger_.log_information(message);

		//
		const auto mode_count = SDL_GetNumDisplayModes(i);

		if (mode_count > 0)
		{
			message = "  Modes:";
			logger_.log_information(message);

			for (auto j = decltype(mode_count){}; j < mode_count; ++j)
			{
				auto mode = SDL_DisplayMode{};

				if (SDL_GetDisplayMode(i, j, &mode) == 0)
				{
					message.clear();
					message += "    ";
					log_display_mode(mode, message);
					logger_.log_information(message);
				}
			}
		}
		else
		{
			message = "  No modes.";
			logger_.log_information(message);
		}
	}

	logger_.log_information();
}

void SdlVideoMgr::log_info() noexcept
try
{
	log_drivers();
	log_displays();
}
catch (...) {}

} // namespace

// ==========================================================================

VideoMgrUPtr make_sdl_video_mgr(Logger& logger)
try
{
	return std::make_unique<SdlVideoMgr>(logger);
}
BSTONE_FUNC_STATIC_THROW_NESTED

} // namespace sys
} // namespace bstone
