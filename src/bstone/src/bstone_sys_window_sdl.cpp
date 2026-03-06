/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window (SDL)

#include "bstone_sys_window_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include "bstone_string_builder.h"
#include "bstone_sys_gl_context_sdl.h"
#include "bstone_sys_renderer_sdl.h"
#include <type_traits>
#include "SDL3/SDL_video.h"

namespace bstone::sys {

namespace {

class WindowSdl final : public WindowSdlInternal
{
public:
	WindowSdl(
		Logger& logger,
		WindowDecorationMgr& decoration_mgr,
		const WindowInitParam& param);

	~WindowSdl() override;

	void* get_native_handle() const override;
	void* get_sdl_window() const override;

	const char* get_title() override;
	void set_title(const char* title) override;
	WindowPosition get_position() override;
	void set_position(WindowPosition position) override;
	WindowSize get_size() override;
	void set_size(WindowSize size) override;
	void show(bool is_visible) override;
	void set_rounded_corner_type(WindowRoundedCornerType value) override;
	WindowFullscreenType get_fullscreen_mode() override;
	void set_windowed_mode(WindowSize window_size) override;
	void set_exclusive_fullscreen_mode(DisplayMode display_mode) override;
	void set_fake_fullscreen_mode() override;
	GlContextUPtr gl_make_context() override;
	WindowSize get_size_in_pixels() override;
	void gl_swap_buffers() override;
	RendererUPtr make_renderer(const RendererInitParam& param) override;

private:
	Logger& logger_;
	WindowDecorationMgr& decoration_mgr_;
	SDL_Window* sdl_window_{};
	Uint32 sdl_window_id_{};
	void* native_window_handle_{};

	static int map_offset(WindowOffset offset);
	static Uint32 map_flags(const WindowInitParam& param);
	static SDL_GLProfile map_gl_context_profile(GlContextProfile context_profile);
	static GlContextAttributes make_default_gl_attributes();
	static const char* get_gl_attribute_name(SDL_GLAttr sdl_gl_attr);
	static void set_gl_attribute(SDL_GLAttr sdl_gl_attr, int value);
	static void set_gl_attributes(const GlContextAttributes& gl_attributes);
};

// --------------------------------------

WindowSdl::WindowSdl(
	Logger& logger,
	WindowDecorationMgr& decoration_mgr,
	const WindowInitParam& param)
	:
	logger_{logger},
	decoration_mgr_{decoration_mgr}
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
		sdl::fail("SDL_CreateWindow");
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
		sdl::fail("SDL_SetWindowPosition");
	}
	sdl_window_id_ = SDL_GetWindowID(sdl_window);
	if (sdl_window_id_ == 0)
	{
		sdl::fail("SDL_GetWindowID");
	}
#ifdef _WIN32
	const SDL_PropertiesID sdl_window_properties_id = SDL_GetWindowProperties(sdl_window);
	if (sdl_window_properties_id == 0)
	{
		sdl::fail("SDL_GetWindowProperties");
	}
	native_window_handle_ = SDL_GetPointerProperty(sdl_window_properties_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	if (native_window_handle_ == nullptr)
	{
		sdl::fail("SDL_GetPointerProperty(SDL_PROP_WINDOW_WIN32_HWND_POINTER)");
	}
#endif
	decoration_mgr_.set_round_corner_type(*this, param.rounded_corner_type);
	sdl_window_ = sdl_window;
	sdl_window = nullptr;
}

WindowSdl::~WindowSdl()
{
	SDL_DestroyWindow(sdl_window_);
}

void* WindowSdl::get_native_handle() const
{
#ifdef _WIN32
	return native_window_handle_;
#else
	return nullptr;
#endif
}

void* WindowSdl::get_sdl_window() const
{
	return sdl_window_;
}

const char* WindowSdl::get_title()
{
	return SDL_GetWindowTitle(sdl_window_);
}

void WindowSdl::set_title(const char* title)
{
	if (!SDL_SetWindowTitle(sdl_window_, title))
	{
		sdl::fail("SDL_SetWindowTitle");
	}
}

WindowPosition WindowSdl::get_position()
{
	int x;
	int y;
	if (!SDL_GetWindowPosition(sdl_window_, &x, &y))
	{
		sdl::fail("SDL_GetWindowPosition");
	}
	return WindowPosition{
		.x = WindowOffset{x},
		.y = WindowOffset{y}
	};
}

void WindowSdl::set_position(WindowPosition position)
{
	const int sdl_x = map_offset(position.x);
	const int sdl_y = map_offset(position.y);
	if (!SDL_SetWindowPosition(sdl_window_, sdl_x, sdl_y))
	{
		sdl::fail("SDL_SetWindowPosition");
	}
}

WindowSize WindowSdl::get_size()
{
	int width;
	int height;
	if (!SDL_GetWindowSize(sdl_window_, &width, &height))
	{
		sdl::fail("SDL_GetWindowSize");
	}
	return WindowSize{
		.width = width,
		.height = height,
	};
}

void WindowSdl::set_size(WindowSize size)
{
	if (!SDL_SetWindowSize(sdl_window_, size.width, size.height))
	{
		sdl::fail("SDL_SetWindowSize");
	}
}

void WindowSdl::show(bool is_visible)
{
	if (is_visible)
	{
		if (!SDL_ShowWindow(sdl_window_))
		{
			sdl::fail("SDL_ShowWindow");
		}
	}
	else
	{
		if (!SDL_HideWindow(sdl_window_))
		{
			sdl::fail("SDL_HideWindow");
		}
	}
}

void WindowSdl::set_rounded_corner_type(WindowRoundedCornerType value)
{
	decoration_mgr_.set_round_corner_type(*this, value);
}

WindowFullscreenType WindowSdl::get_fullscreen_mode()
{
	const SDL_WindowFlags sdl_flags = SDL_GetWindowFlags(sdl_window_);
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

void WindowSdl::set_windowed_mode(WindowSize window_size)
{
	if (!SDL_SetWindowFullscreen(sdl_window_, false))
	{
		sdl::fail("SDL_SetWindowFullscreen");
	}
	if (!SDL_SetWindowSize(sdl_window_, window_size.width, window_size.height))
	{
		sdl::fail("SDL_SetWindowSize");
	}
}

void WindowSdl::set_exclusive_fullscreen_mode(DisplayMode display_mode)
{
	const SDL_DisplayID sdl_display_id = SDL_GetDisplayForWindow(sdl_window_);
	if (sdl_display_id == 0)
	{
		sdl::fail("SDL_GetDisplayForWindow");
	}
	SDL_DisplayMode sdl_display_mode;
	if (!SDL_GetClosestFullscreenDisplayMode(
		/* displayID                  */ sdl_display_id,
		/* w                          */ display_mode.width,
		/* h                          */ display_mode.height,
		/* refresh_rate               */ display_mode.refresh_rate,
		/* include_high_density_modes */ true,
		/* closest                    */ &sdl_display_mode))
	{
		sdl::fail("SDL_GetClosestFullscreenDisplayMode");
	}
	if (!SDL_SetWindowFullscreenMode(sdl_window_, &sdl_display_mode))
	{
		sdl::fail("SDL_SetWindowFullscreenMode");
	}
	if (!SDL_SetWindowFullscreen(sdl_window_, true))
	{
		sdl::fail("SDL_SetWindowFullscreen");
	}
}

void WindowSdl::set_fake_fullscreen_mode()
{
	if (!SDL_SetWindowFullscreenMode(sdl_window_, nullptr))
	{
		sdl::fail("SDL_SetWindowFullscreenMode");
	}
	if (!SDL_SetWindowFullscreen(sdl_window_, true))
	{
		sdl::fail("SDL_SetWindowFullscreen");
	}
}

GlContextUPtr WindowSdl::gl_make_context()
{
	return make_gl_context_sdl(logger_, *sdl_window_);
}

WindowSize WindowSdl::get_size_in_pixels()
{
	int sdl_width;
	int sdl_height;
	if (!SDL_GetWindowSizeInPixels(sdl_window_, &sdl_width, &sdl_height))
	{
		sdl::fail("SDL_GetWindowSizeInPixels");
	}
	return WindowSize{
		.width = sdl_width,
		.height = sdl_height,
	};
}

void WindowSdl::gl_swap_buffers()
{
	if (!SDL_GL_SwapWindow(sdl_window_))
	{
		sdl::fail("SDL_GL_SwapWindow");
	}
}

RendererUPtr WindowSdl::make_renderer(const RendererInitParam& param)
{
	return make_renderer_sdl(logger_, *sdl_window_, param);
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

Uint32 WindowSdl::map_flags(const WindowInitParam& param)
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

GlContextAttributes WindowSdl::make_default_gl_attributes()
{
	return GlContextAttributes{
		.is_accelerated = true,
		.profile = GlContextProfile::compatibility,
		.major_version = 1,
		.minor_version = 1,
		.multisample_buffer_count = 0,
		.multisample_sample_count = 0,
		.red_bit_count = 0,
		.green_bit_count = 0,
		.blue_bit_count = 0,
		.alpha_bit_count = 0,
		.depth_bit_count = 0,
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
	WindowDecorationMgr& decoration_mgr,
	const WindowInitParam& param)
{
	return std::make_unique<WindowSdl>(logger, decoration_mgr, param);
}

} // namespace bstone::sys
