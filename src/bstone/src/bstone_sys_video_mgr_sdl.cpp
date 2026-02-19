/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video manager (SDL)

#include "bstone_sys_video_mgr_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include "bstone_string_builder.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_gl_current_context_sdl.h"
#include "bstone_sys_limits_sdl.h"
#include "bstone_sys_video_mgr_null.h"
#include "bstone_sys_mouse_mgr_sdl.h"
#include "bstone_sys_vulkan_mgr_sdl.h"
#include "bstone_sys_window_mgr_sdl.h"
#include "bstone_sys_sdl_subsystem.h"
#include <algorithm>
#include <exception>
#include <vector>
#include "SDL3/SDL_video.h"

namespace bstone::sys {

namespace {

class VideoMgrSdl final : public VideoMgr
{
public:
	explicit VideoMgrSdl(Logger& logger);
	VideoMgrSdl(const VideoMgrSdl&) = delete;
	VideoMgrSdl& operator=(const VideoMgrSdl&) = delete;
	~VideoMgrSdl() override;

	bool is_initialized() const override;
	Logger& get_logger() override;
	DisplayMode get_current_display_mode() override;
	std::span<const DisplayMode> get_display_modes() override;
	GlCurrentContext& get_gl_current_context() override;
	VulkanMgr& get_vulkan_mgr() override;
	MouseMgr& get_mouse_mgr() override;
	WindowMgr& get_window_mgr() override;

private:
	using DisplayModeCache = std::vector<DisplayMode>;

	Logger& logger_;
	SdlSubsystem sdl_subsystem_{};
	MouseMgrUPtr mouse_mgr_{};
	WindowMgrUPtr window_mgr_{};
	DisplayModeCache display_mode_cache_{};
	GlCurrentContextUPtr gl_current_context_{};
	VulkanMgrUPtr vulkan_mgr_{};

	static void log_sdl_error(StringBuilder& formatter);
	static void log_drivers(StringBuilder& formatter);
	static void log_display_bounds(SDL_DisplayID sdl_display_id, StringBuilder& formatter);
	static void log_display_fullscreen_modes(SDL_DisplayID sdl_display_id, StringBuilder& formatter);
	static void log_displays(StringBuilder& formatter);
	void log_info();
	static DisplayMode map_display_mode(const SDL_DisplayMode& sdl_display_mode);
};

// --------------------------------------

VideoMgrSdl::VideoMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL video manager.");
	SdlSubsystem sdl_subsystem{SDL_INIT_VIDEO};
	log_info();
	mouse_mgr_ = make_mouse_mgr_sdl(logger);
	window_mgr_ = make_window_mgr_sdl(logger);
	sdl_subsystem_.swap(sdl_subsystem);
	logger_.log_information("SDL video manager has started.");
}

VideoMgrSdl::~VideoMgrSdl()
{
	logger_.log_information("Shut down SDL video manager.");
	gl_current_context_ = nullptr;
	vulkan_mgr_ = nullptr;
	window_mgr_ = nullptr;
	mouse_mgr_ = nullptr;
}

bool VideoMgrSdl::is_initialized() const
{
	return true;
}

Logger& VideoMgrSdl::get_logger()
{
	return logger_;
}

DisplayMode VideoMgrSdl::get_current_display_mode()
{
	const SDL_DisplayID sdl_display_id = SDL_GetPrimaryDisplay();
	if (sdl_display_id == 0)
	{
		sdl::fail("SDL_GetPrimaryDisplay");
	}
	const SDL_DisplayMode* sdl_display_mode = SDL_GetCurrentDisplayMode(sdl_display_id);
	if (sdl_display_mode == nullptr)
	{
		sdl::fail("SDL_GetCurrentDisplayMode");
	}
	return map_display_mode(*sdl_display_mode);
}

std::span<const DisplayMode> VideoMgrSdl::get_display_modes()
{
	int sdl_mode_count;
	SDL_DisplayMode** const sdl_display_mode_ptrs = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &sdl_mode_count);
	if (sdl_display_mode_ptrs == nullptr)
	{
		sdl::fail("SDL_GetFullscreenDisplayModes");
	}
	const auto scope_exit = make_scope_exit(
		[sdl_display_mode_ptrs]()
		{
			SDL_free(sdl_display_mode_ptrs);
		});
	if (static_cast<int>(display_mode_cache_.size()) < sdl_mode_count)
	{
		display_mode_cache_.resize(static_cast<std::size_t>(sdl_mode_count));
	}
	for (int i_mode = 0; i_mode < sdl_mode_count; ++i_mode)
	{
		display_mode_cache_[i_mode] = map_display_mode(*(sdl_display_mode_ptrs[i_mode]));
	}
	return std::span<const DisplayMode>{display_mode_cache_.data(), static_cast<std::size_t>(sdl_mode_count)};
}

GlCurrentContext& VideoMgrSdl::get_gl_current_context()
{
	if (gl_current_context_ == nullptr)
	{
		gl_current_context_ = make_gl_current_context_sdl(logger_);
	}
	return *gl_current_context_;
}

VulkanMgr& VideoMgrSdl::get_vulkan_mgr()
{
	if (vulkan_mgr_ == nullptr)
	{
		vulkan_mgr_ = make_vulkan_mgr_sdl(logger_);
	}
	return *vulkan_mgr_;
}

MouseMgr& VideoMgrSdl::get_mouse_mgr()
{
	return *mouse_mgr_;
}

WindowMgr& VideoMgrSdl::get_window_mgr()
{
	return *window_mgr_;
}

void VideoMgrSdl::log_sdl_error(StringBuilder& formatter)
{
	formatter.add("ERROR: {}", SDL_GetError());
}

void VideoMgrSdl::log_drivers(StringBuilder& formatter)
{
	//
	const char* const current_driver_name = SDL_GetCurrentVideoDriver();
	formatter.reset_indent();
	formatter.add("Current driver: ");
	if (current_driver_name != nullptr)
	{
		formatter.add(current_driver_name);
	}
	else
	{
		log_sdl_error(formatter);
	}
	formatter.add_line();
	//
	formatter.reset_indent();
	formatter.add_line("Built-in drivers:");
	formatter.increase_indent();
	const int driver_count = SDL_GetNumVideoDrivers();
	for (int i_driver = 0; i_driver < driver_count; ++i_driver)
	{
		const char* const sdl_driver_name = SDL_GetVideoDriver(i_driver);
		formatter.add_indented_line(sdl_driver_name != nullptr ? sdl_driver_name : "???");
	}
}

void VideoMgrSdl::log_display_bounds(SDL_DisplayID sdl_display_id, StringBuilder& formatter)
{
	formatter.add_indented("Bounds: ");
	SDL_Rect sdl_rect;
	if (SDL_GetDisplayBounds(sdl_display_id, &sdl_rect))
	{
		formatter.add("({}, {}, {}, {})", sdl_rect.x, sdl_rect.y, sdl_rect.w, sdl_rect.h);
	}
	else
	{
		log_sdl_error(formatter);
	}
	formatter.add_line();
}

void VideoMgrSdl::log_display_fullscreen_modes(SDL_DisplayID sdl_display_id, StringBuilder& formatter)
{
	formatter.add_indented_line("Fullscreen modes:");
	formatter.increase_indent();
	int mode_count;
	if (SDL_DisplayMode** const sdl_display_mode_ptrs = SDL_GetFullscreenDisplayModes(sdl_display_id, &mode_count);
		sdl_display_mode_ptrs != nullptr)
	{
		const auto scope_exit = make_scope_exit(
			[sdl_display_mode_ptrs]()
			{
				SDL_free(sdl_display_mode_ptrs);
			});
		for (int i_mode = 0; i_mode < mode_count; ++i_mode)
		{
			const SDL_DisplayMode& sdl_display_mode = *(sdl_display_mode_ptrs[i_mode]);
			formatter.add_indented("{}. ", i_mode + 1);
			if (sdl_display_mode.pixel_density != 1.0F)
			{
				formatter.add("{:.2f}x", sdl_display_mode.pixel_density);
			}
			formatter.add("{}x{}", sdl_display_mode.w, sdl_display_mode.h);
			formatter.add(" {:.2f}", sdl_display_mode.refresh_rate);
			if (sdl_display_mode.refresh_rate_denominator != 1)
			{
				formatter.add(" ({}/{})", sdl_display_mode.refresh_rate_numerator, sdl_display_mode.refresh_rate_denominator);
			}
			formatter.add(" Hz");
			formatter.add_line();
		}
	}
	else
	{
		formatter.add_indent();
		log_sdl_error(formatter);
		formatter.add_line();
	}
	formatter.decrease_indent();
}

void VideoMgrSdl::log_displays(StringBuilder& formatter)
{
	formatter.reset_indent();
	formatter.add_line("Displays:");
	formatter.increase_indent();
	int display_count;
	SDL_DisplayID* const sdl_display_ids = SDL_GetDisplays(&display_count);
	if (sdl_display_ids != nullptr)
	{
		const auto scope_exit = make_scope_exit(
			[sdl_display_ids]()
			{
				SDL_free(sdl_display_ids);
			});
		for (int i_display = 0; i_display < display_count; ++i_display)
		{
			const SDL_DisplayID sdl_display_id = sdl_display_ids[i_display];
			const char* const sdl_name = SDL_GetDisplayName(sdl_display_id);
			formatter.add_indented("{}. ", i_display + 1);
			if (sdl_name != nullptr)
			{
				formatter.add(sdl_name);
			}
			else
			{
				log_sdl_error(formatter);
			}
			formatter.add_line();
			formatter.increase_indent();
			log_display_bounds(sdl_display_id, formatter);
			log_display_fullscreen_modes(sdl_display_id, formatter);
		}
	}
	else
	{
		log_sdl_error(formatter);
	}
}

void VideoMgrSdl::log_info()
{
	StringBuilder formatter{};
	formatter.reserve(1024);
	log_drivers(formatter);
	log_displays(formatter);
	logger_.log_information(formatter.get_string().c_str());
}

DisplayMode VideoMgrSdl::map_display_mode(const SDL_DisplayMode& sdl_display_mode)
{
	return DisplayMode{
		.width = sdl_display_mode.w,
		.height = sdl_display_mode.h,
		.refresh_rate = sdl_display_mode.refresh_rate,
	};
}

} // namespace

// ======================================

VideoMgrUPtr make_video_mgr_sdl(Logger& logger)
try
{
	return std::make_unique<VideoMgrSdl>(logger);
}
catch (const std::exception& exception)
{
	logger.log_error(exception.what());
	return make_video_mgr_null(logger);
}

} // namespace bstone::sys
