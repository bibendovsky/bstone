/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window (SDL)

#include "bstone_sys_window_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_string_builder.h"
#include "bstone_sys_gl_context_sdl.h"
#include "bstone_sys_renderer_sdl.h"
#include <format>
#include <string>
#include <type_traits>
#include "SDL3/SDL_video.h"

namespace bstone::sys {

void* WindowSdlInternal::get_native_handle() const
{
	return do_get_native_handle();
}

void* WindowSdlInternal::get_sdl_window() const
{
	return do_get_sdl_window();
}

// ======================================

namespace {

class WindowSdl final : public WindowSdlInternal
{
public:
	WindowSdl(
		Logger& logger,
		WindowRoundedCornerMgr& rounded_corner_mgr,
		const WindowInitParam& param);

	~WindowSdl() override;

private:
	Logger& logger_;
	WindowRoundedCornerMgr& rounded_corner_mgr_;
	SDL_Window* sdl_window_{};
	Uint32 sdl_window_id_{};
	void* native_window_handle_{};

	void* do_get_native_handle() const override;
	void* do_get_sdl_window() const override;

	const char* do_get_title() override;
	void do_set_title(const char* title) override;
	WindowPosition do_get_position() override;
	void do_set_position(WindowPosition position) override;
	WindowSize do_get_size() override;
	void do_set_size(WindowSize size) override;
	DisplayMode do_get_display_mode() override;
	void do_set_display_mode(const DisplayMode& display_mode) override;
	void do_show(bool is_visible) override;
	void do_set_rounded_corner_type(WindowRoundedCornerType value) override;
	WindowFullscreenType do_get_fullscreen_mode() override;
	void do_set_fullscreen_mode(WindowFullscreenType fullscreen_mode) override;
	GlContextUPtr do_make_gl_context() override;
	WindowSize do_gl_get_drawable_size() override;
	void do_gl_swap_buffers() override;
	RendererUPtr do_make_renderer(const RendererInitParam& param) override;

	[[noreturn]] void fail_sdl_func(const char* sdl_func_name);
	static int map_offset(WindowOffset offset);
	static Uint32 map_flags(const WindowInitParam& param) noexcept;
	static SDL_GLProfile map_gl_context_profile(GlContextProfile context_profile);
	static GlContextAttributes make_default_gl_attributes() noexcept;
	static const char* get_gl_attribute_name(SDL_GLAttr sdl_gl_attr);
	static void set_gl_attribute(SDL_GLAttr sdl_gl_attr, int value);
	static void set_gl_attributes(const GlContextAttributes& gl_attributes);
};

// --------------------------------------

WindowSdl::WindowSdl(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param)
	:
	logger_{logger},
	rounded_corner_mgr_{rounded_corner_mgr}
{
	const int sdl_x = map_offset(param.x);
	const int sdl_y = map_offset(param.y);
	const Uint32 sdl_flags = map_flags(param);
	if (param.renderer_type == WindowRendererType::open_gl)
	{
		if (param.gl_attributes != nullptr)
		{
			set_gl_attributes(*param.gl_attributes);
		}
		else
		{
			const GlContextAttributes gl_attributes = make_default_gl_attributes();
			set_gl_attributes(gl_attributes);
		}
	}
	SDL_Window* sdl_window = SDL_CreateWindow(param.title, param.width, param.height, sdl_flags);
	if (sdl_window == nullptr)
	{
		fail_sdl_func("SDL_CreateWindow");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_window]()
		{
			if (sdl_window != nullptr)
			{
				SDL_DestroyWindow(sdl_window);
			}
		});
	if (!SDL_SetWindowPosition(sdl_window, sdl_x, sdl_y))
	{
		fail_sdl_func("SDL_SetWindowPosition");
	}
	sdl_window_id_ = SDL_GetWindowID(sdl_window);
	if (sdl_window_id_ == 0)
	{
		fail_sdl_func("SDL_GetWindowID");
	}
#ifdef _WIN32
	const SDL_PropertiesID sdl_window_properties_id = SDL_GetWindowProperties(sdl_window);
	if (sdl_window_properties_id == 0)
	{
		fail_sdl_func("SDL_GetWindowProperties");
	}
	native_window_handle_ = SDL_GetPointerProperty(sdl_window_properties_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	if (native_window_handle_ == nullptr)
	{
		fail_sdl_func("SDL_GetPointerProperty(SDL_PROP_WINDOW_WIN32_HWND_POINTER)");
	}
#endif
	rounded_corner_mgr_.set_round_corner_type(*this, param.rounded_corner_type);
	sdl_window_ = sdl_window;
	sdl_window = nullptr;
}

WindowSdl::~WindowSdl()
{
	SDL_DestroyWindow(sdl_window_);
}

void* WindowSdl::do_get_native_handle() const
{
#ifdef _WIN32
	return native_window_handle_;
#else
	return nullptr;
#endif
}

void* WindowSdl::do_get_sdl_window() const
{
	return sdl_window_;
}

const char* WindowSdl::do_get_title()
{
	return SDL_GetWindowTitle(sdl_window_);
}

void WindowSdl::do_set_title(const char* title)
{
	if (!SDL_SetWindowTitle(sdl_window_, title))
	{
		fail_sdl_func("SDL_SetWindowTitle");
	}
}

WindowPosition WindowSdl::do_get_position()
{
	int x;
	int y;
	if (!SDL_GetWindowPosition(sdl_window_, &x, &y))
	{
		fail_sdl_func("SDL_GetWindowPosition");
	}
	return WindowPosition{
		.x = WindowOffset{x},
		.y = WindowOffset{y}
	};
}

void WindowSdl::do_set_position(WindowPosition position)
{
	const int sdl_x = map_offset(position.x);
	const int sdl_y = map_offset(position.y);
	if (!SDL_SetWindowPosition(sdl_window_, sdl_x, sdl_y))
	{
		fail_sdl_func("SDL_SetWindowPosition");
	}
}

WindowSize WindowSdl::do_get_size()
{
	int width;
	int height;
	if (!SDL_GetWindowSize(sdl_window_, &width, &height))
	{
		fail_sdl_func("SDL_GetWindowSize");
	}
	return WindowSize{
		.width = width,
		.height = height,
	};
}

void WindowSdl::do_set_size(WindowSize size)
{
	if (!SDL_SetWindowSize(sdl_window_, size.width, size.height))
	{
		fail_sdl_func("SDL_SetWindowSize");
	}
}

DisplayMode WindowSdl::do_get_display_mode()
{
	SDL_Window* const sdl_window = sdl_window_;
	if (const Uint32 sdl_flags = SDL_GetWindowFlags(sdl_window);
		(sdl_flags & SDL_WINDOW_FULLSCREEN) != 0)
	{
		if (const SDL_DisplayMode* sdl_display_mode = SDL_GetWindowFullscreenMode(sdl_window);
			sdl_display_mode != nullptr)
		{
			return DisplayMode{
				.width = sdl_display_mode->w,
				.height = sdl_display_mode->h,
				.refresh_rate = static_cast<int>(sdl_display_mode->refresh_rate),
			};
		}
	}
	const SDL_DisplayID sdl_display_id = SDL_GetPrimaryDisplay();
	if (sdl_display_id == 0)
	{
		fail_sdl_func("SDL_GetPrimaryDisplay");
	}
	const SDL_DisplayMode* const sdl_display_mode = SDL_GetCurrentDisplayMode(sdl_display_id);
	if (sdl_display_mode == nullptr)
	{
		fail_sdl_func("SDL_GetCurrentDisplayMode");
	}
	int sdl_w;
	int sdl_h;
	if (!SDL_GetWindowSizeInPixels(sdl_window, &sdl_w, &sdl_h))
	{
		fail_sdl_func("SDL_GetWindowSizeInPixels");
	}
	return DisplayMode{
		.width = sdl_w,
		.height = sdl_h,
		.refresh_rate = static_cast<int>(sdl_display_mode->refresh_rate),
	};
}

void WindowSdl::do_set_display_mode(const DisplayMode& display_mode)
{
	const SDL_DisplayID sdl_display_id = SDL_GetDisplayForWindow(sdl_window_);
	if (sdl_display_id == 0)
	{
		fail_sdl_func("SDL_GetDisplayForWindow");
	}
	SDL_DisplayMode sdl_display_mode;
	if (!SDL_GetClosestFullscreenDisplayMode(
		sdl_display_id,
		display_mode.width,
		display_mode.height,
		display_mode.refresh_rate,
		false,
		&sdl_display_mode))
	{
		fail_sdl_func("SDL_GetClosestFullscreenDisplayMode");
	}
	if (!SDL_SetWindowFullscreenMode(sdl_window_, &sdl_display_mode))
	{
		fail_sdl_func("SDL_SetWindowFullscreenMode");
	}
}

void WindowSdl::do_show(bool is_visible)
{
	if (is_visible)
	{
		if (!SDL_ShowWindow(sdl_window_))
		{
			fail_sdl_func("SDL_ShowWindow");
		}
	}
	else
	{
		if (!SDL_HideWindow(sdl_window_))
		{
			fail_sdl_func("SDL_HideWindow");
		}
	}
}

void WindowSdl::do_set_rounded_corner_type(WindowRoundedCornerType value)
{
	rounded_corner_mgr_.set_round_corner_type(*this, value);
}

WindowFullscreenType WindowSdl::do_get_fullscreen_mode()
{
	const Uint32 sdl_flags = SDL_GetWindowFlags(sdl_window_);
	if ((sdl_flags & SDL_WINDOW_FULLSCREEN) != 0)
	{
		if (SDL_GetWindowFullscreenMode(sdl_window_) != nullptr)
		{
			return WindowFullscreenType::exclusive;
		}
		return WindowFullscreenType::fake;
	}
	return WindowFullscreenType::none;
}

void WindowSdl::do_set_fullscreen_mode(WindowFullscreenType fullscreen_mode)
{
	bool is_fullscreen = false;
	bool is_exclusive_fullscreen = false;
	switch (fullscreen_mode)
	{
		case WindowFullscreenType::none:
			break;
		case WindowFullscreenType::fake:
			is_fullscreen = true;
			break;
		case WindowFullscreenType::exclusive:
			is_fullscreen = true;
			is_exclusive_fullscreen = true;
			break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown fullscreen mode.");
	}
	if (!is_exclusive_fullscreen)
	{
		if (!SDL_SetWindowFullscreenMode(sdl_window_, nullptr))
		{
			fail_sdl_func("SDL_SetWindowFullscreenMode");
		}
	}
	if (!SDL_SetWindowFullscreen(sdl_window_, is_fullscreen))
	{
		fail_sdl_func("SDL_SetWindowFullscreen");
	}
}

GlContextUPtr WindowSdl::do_make_gl_context()
{
	return make_gl_context_sdl(logger_, *sdl_window_);
}

WindowSize WindowSdl::do_gl_get_drawable_size()
{
	int sdl_width;
	int sdl_height;
	if (!SDL_GetWindowSizeInPixels(sdl_window_, &sdl_width, &sdl_height))
	{
		fail_sdl_func("SDL_GetWindowSizeInPixels");
	}
	return WindowSize{
		.width = sdl_width,
		.height = sdl_height,
	};
}

void WindowSdl::do_gl_swap_buffers()
{
	if (!SDL_GL_SwapWindow(sdl_window_))
	{
		fail_sdl_func("SDL_GL_SwapWindow");
	}
}

RendererUPtr WindowSdl::do_make_renderer(const RendererInitParam& param)
{
	return make_renderer_sdl(logger_, *sdl_window_, param);
}

[[noreturn]] void WindowSdl::fail_sdl_func(const char* sdl_func_name)
{
	const std::string message = std::format("[{}] {}", sdl_func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

int WindowSdl::map_offset(WindowOffset offset)
{
	switch (offset.get_type())
	{
		case WindowOffsetType::centered:
			return SDL_WINDOWPOS_CENTERED;
		case WindowOffsetType::undefined:
			return SDL_WINDOWPOS_UNDEFINED;
		case WindowOffsetType::custom:
			{
				const int value = offset.get();
				if (value < window_min_position || value > window_max_position)
				{
					BSTONE_THROW_STATIC_SOURCE("Position out of range.");
				}
				return value;
			}
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown offset type.");
	}
}

Uint32 WindowSdl::map_flags(const WindowInitParam& param) noexcept
{
	Uint32 sdl_flags = 0;
	if (param.renderer_type == WindowRendererType::open_gl)
	{
		sdl_flags |= SDL_WINDOW_OPENGL;
	}
	else if (param.renderer_type == WindowRendererType::vulkan)
	{
		sdl_flags |= SDL_WINDOW_VULKAN;
	}
	if (!param.is_visible)
	{
		sdl_flags |= SDL_WINDOW_HIDDEN;
	}
	return sdl_flags;
}

SDL_GLProfile WindowSdl::map_gl_context_profile(GlContextProfile context_profile)
{
	switch (context_profile)
	{
		case GlContextProfile::compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
		case GlContextProfile::core: return SDL_GL_CONTEXT_PROFILE_CORE;
		case GlContextProfile::es: return SDL_GL_CONTEXT_PROFILE_ES;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown context profile.");
	}
}

GlContextAttributes WindowSdl::make_default_gl_attributes() noexcept
{
	return GlContextAttributes{
		.is_accelerated = true,
		.profile = GlContextProfile::compatibility,
		.major_version = 1,
		.minor_version = 1,
	};
}

const char* WindowSdl::get_gl_attribute_name(SDL_GLAttr sdl_gl_attr)
{
#define BSTONE_MACRO(x) case x: return #x
	switch (sdl_gl_attr)
	{
		BSTONE_MACRO(SDL_GL_ACCELERATED_VISUAL);
		BSTONE_MACRO(SDL_GL_CONTEXT_PROFILE_MASK);
		BSTONE_MACRO(SDL_GL_CONTEXT_MAJOR_VERSION);
		BSTONE_MACRO(SDL_GL_CONTEXT_MINOR_VERSION);
		BSTONE_MACRO(SDL_GL_MULTISAMPLEBUFFERS);
		BSTONE_MACRO(SDL_GL_MULTISAMPLESAMPLES);
		BSTONE_MACRO(SDL_GL_RED_SIZE);
		BSTONE_MACRO(SDL_GL_GREEN_SIZE);
		BSTONE_MACRO(SDL_GL_BLUE_SIZE);
		BSTONE_MACRO(SDL_GL_ALPHA_SIZE);
		BSTONE_MACRO(SDL_GL_DEPTH_SIZE);
		default: return nullptr;
	}
#undef BSTONE_MACRO
}

void WindowSdl::set_gl_attribute(SDL_GLAttr sdl_gl_attr, int value)
{
	if (SDL_GL_SetAttribute(sdl_gl_attr, value))
	{
		return;
	}
	const char* const attribute_name = get_gl_attribute_name(sdl_gl_attr);
	StringBuilder formatter{};
	formatter.reserve(128);
	formatter.add("[{}] {} (attr=", "SDL_GL_SetAttribute", SDL_GetError());
	if (attribute_name != nullptr)
	{
		formatter.add("{} ", attribute_name);
	}
	formatter.add("{}; value={})", static_cast<std::underlying_type_t<SDL_GLAttr>>(sdl_gl_attr), value);
	BSTONE_THROW_DYNAMIC_SOURCE(formatter.get_string().c_str());
}

void WindowSdl::set_gl_attributes(const GlContextAttributes& gl_attribs)
{
	const SDL_GLProfile sdl_profile = map_gl_context_profile(gl_attribs.profile);
	const int sdl_accelerated = gl_attribs.is_accelerated;
	SDL_GL_ResetAttributes();
	set_gl_attribute(SDL_GL_ACCELERATED_VISUAL, sdl_accelerated);
	set_gl_attribute(SDL_GL_CONTEXT_PROFILE_MASK, sdl_profile);
	set_gl_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_attribs.major_version);
	set_gl_attribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_attribs.minor_version);
	set_gl_attribute(SDL_GL_MULTISAMPLEBUFFERS, gl_attribs.multisample_buffer_count);
	set_gl_attribute(SDL_GL_MULTISAMPLESAMPLES, gl_attribs.multisample_sample_count);
	set_gl_attribute(SDL_GL_RED_SIZE, gl_attribs.red_bit_count);
	set_gl_attribute(SDL_GL_GREEN_SIZE, gl_attribs.green_bit_count);
	set_gl_attribute(SDL_GL_BLUE_SIZE, gl_attribs.blue_bit_count);
	set_gl_attribute(SDL_GL_ALPHA_SIZE, gl_attribs.alpha_bit_count);
	set_gl_attribute(SDL_GL_DEPTH_SIZE, gl_attribs.depth_bit_count);
}

} // namespace

// ======================================

WindowUPtr make_window_sdl(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param)
{
	return std::make_unique<WindowSdl>(logger, rounded_corner_mgr, param);
}

} // namespace bstone::sys
