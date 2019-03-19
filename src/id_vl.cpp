/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#define BSTONE_DBG_FORCE_SW (0)


#include <cassert>
#include <chrono>
#include "SDL_hints.h"
#include "SDL_render.h"
#include "id_heads.h"
#include "id_ca.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"
#include "bstone_fixed_point.h"
#include "bstone_log.h"
#include "bstone_sprite.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_renderer_manager.h"
#include "bstone_renderer_texture_manager.h"


using namespace std::string_literals;


static const int palette_color_count = 256;


extern bool is_full_menu_active;


bool vid_is_hw_ = false;

int bufferofs;

bool screenfaded;

std::uint8_t palette1[palette_color_count][3];
std::uint8_t palette2[palette_color_count][3];
std::uint8_t* vga_memory = nullptr;

int vga_scale = 0;
float vga_height_scale = 0.0F;
float vga_width_scale = 0.0F;
int vga_width = 0;
int vga_height = 0;
int vga_area = 0;
int vga_3d_view_top_y = 0;
int vga_3d_view_bottom_y = 0;

int screen_x = 0;
int screen_y = 0;

int screen_width = 0;
int screen_height = 0;

int filler_width = 0;

bool vid_has_vsync = false;
bool vid_widescreen = default_vid_widescreen;
bool vid_is_hud = false;
bool vid_is_3d = false;
bool vid_is_fizzle_fade = false;
bool vid_is_movie = false;
bool vid_is_ui_stretched = false;

bool vid_hw_is_draw_3d_ = false;

bstone::SpriteCache vid_sprite_cache;


// BBi
namespace
{


constexpr int default_window_width = 640;
constexpr int default_window_height = 480;


class VgaColor
{
public:
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
}; // VgaColor

using VgaPalette = std::array<VgaColor, palette_color_count>;
using SdlPalette = std::array<std::uint32_t, palette_color_count>;


int window_width = 0;
int window_height = 0;

VgaBuffer vid_ui_buffer;
UiMaskBuffer vid_mask_buffer;


std::string sw_error_message;

VgaPalette vid_vga_palette;
VgaBuffer sw_vga_buffer;

bool vid_use_custom_window_position = false;
int vid_window_x = 0;
int vid_window_y = 0;

SDL_DisplayMode display_mode;
bool vid_is_windowed = false;
SDL_Window* sw_window = nullptr;
SDL_Renderer* sw_renderer = nullptr;
SDL_PixelFormat* sw_texture_pixel_format = nullptr;
SDL_Texture* sw_screen_texture = nullptr;
SDL_Texture* sw_ui_texture = nullptr;
SdlPalette sw_palette;
SDL_Rect sw_ui_whole_src_rect;
SDL_Rect sw_ui_whole_dst_rect;
SDL_Rect sw_ui_top_src_rect;
SDL_Rect sw_ui_top_dst_rect;
SDL_Rect sw_ui_wide_middle_src_rect;
SDL_Rect sw_ui_wide_middle_dst_rect;
SDL_Rect sw_ui_bottom_src_rect;
SDL_Rect sw_ui_bottom_dst_rect;
std::array<SDL_Rect, 2> sw_filler_ui_rects;
std::array<SDL_Rect, 4> sw_filler_hud_rects;
const auto sw_ref_filler_color = SDL_Color{0x00, 0x28, 0x50, 0xFF, };
auto sw_filler_color = SDL_Color{};

const auto filler_color_index = 0xE8;


void sw_initialize_vga_buffer()
{
	const auto vga_area = 2 * ::vga_width * ::vga_height;

	::sw_vga_buffer.resize(
		vga_area);

	::vga_memory = ::sw_vga_buffer.data();
}

void sw_initialize_ui_buffer()
{
	const auto area = ::vga_ref_width * ::vga_ref_height;

	::vid_ui_buffer.resize(
		area);
}

bool sw_initialize_window()
{
	bstone::Log::write("VID: Creating a window...");


	if (!::vid_use_custom_window_position)
	{
		::vid_window_x = SDL_WINDOWPOS_CENTERED;
		::vid_window_y = SDL_WINDOWPOS_CENTERED;
	}

	if (::vid_window_x < 0)
	{
		::vid_window_x = 0;
	}

	if (::vid_window_y < 0)
	{
		::vid_window_y = 0;
	}

	auto window_flags = Uint32{
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_HIDDEN |
		0};

	if (!::vid_is_windowed)
	{
		window_flags |=
			SDL_WINDOW_BORDERLESS |
			SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

#ifdef __vita__
	window_flags = SDL_WINDOW_SHOWN;
#endif

	const auto& assets_info = AssetsInfo{};

	auto title = "Blake Stone"s;

	if (assets_info.is_aog())
	{
		auto version_string = std::string{};

		if (assets_info.is_aog_full_v1_0() || assets_info.is_aog_sw_v1_0())
		{
			version_string = "v1.0";
		}
		else if (assets_info.is_aog_full_v2_0() || assets_info.is_aog_sw_v2_0())
		{
			version_string = "v2.0";
		}
		else if (assets_info.is_aog_full_v2_1() || assets_info.is_aog_sw_v2_1())
		{
			version_string = "v2.1";
		}
		else if (assets_info.is_aog_full_v3_0() || assets_info.is_aog_sw_v3_0())
		{
			version_string = "v3.0";
		}

		auto type = std::string{};

		if (assets_info.is_aog_full())
		{
			type = "full";
		}
		else if (assets_info.is_aog_sw())
		{
			type = "shareware";
		}

		const auto has_type_or_version = (!version_string.empty() || !type.empty());

		title += ": Aliens of Gold";

		if (has_type_or_version)
		{
			title += " (";

			if (!type.empty())
			{
				title += type;
			}

			if (!version_string.empty())
			{
				if (!type.empty())
				{
					title += ", ";
				}

				title += version_string;
			}

			title += ')';
		}
	}
	else if (assets_info.is_ps())
	{
		title += ": Planet Strike";
	}

	::sw_window = ::SDL_CreateWindow(
		title.c_str(),
		::vid_window_x,
		::vid_window_y,
		::window_width,
		::window_height,
		window_flags);

	if (!::sw_window)
	{
		::sw_error_message = "VID: Failed to create a window: ";
		::sw_error_message += ::SDL_GetError();

		bstone::Log::write_error(::SDL_GetError());

		return false;
	}

	auto hint_result = ::SDL_SetHint("SDL_HINT_RENDER_DRIVER", "opengl");

	return true;
}

bool sw_initialize_renderer()
{
	bstone::Log::write(
		"VID: Initializing a renderer...");


	bool is_succeed = true;


	if (is_succeed)
	{
		bstone::Log::write(
			"VID: Available renderer drivers:");

		const auto driver_count = ::SDL_GetNumRenderDrivers();

		for (int i = 0; i < driver_count; ++i)
		{
			SDL_RendererInfo info;

			const auto sdl_result = ::SDL_GetRenderDriverInfo(
				i,
				&info);

			bstone::Log::write(std::to_string(i + 1) + ". "s + info.name);
		}
	}


	std::uint32_t renderer_flags = 0;
	const char* renderer_driver = nullptr;

	if (is_succeed)
	{
		const auto is_vsync_disabled = ::g_args.has_option(
			"vid_no_vsync");

		if (is_vsync_disabled)
		{
			bstone::Log::write(
				"VID: Skipping VSync...");
		}
		else
		{
			renderer_flags = SDL_RENDERER_PRESENTVSYNC;

			bstone::Log::write(
				"VID: Using VSync...");
		}


		auto& ren_string = ::g_args.get_option_value(
			"vid_renderer");

		if (!ren_string.empty())
		{
			bstone::Log::write(
				"VID: Setting preferred renderer...");


			if (ren_string == "d3d")
			{
				renderer_driver = "direct3d";

				bstone::Log::write(
					"VID: Forcing Direct3D renderer.");
			}
			else if (ren_string == "ogl")
			{
				renderer_driver = "opengl";

				bstone::Log::write(
					"VID: Forcing OpenGL renderer.");
			}
			else if (ren_string == "ogles")
			{
				renderer_driver = "opengles";

				bstone::Log::write(
					"VID: Forcing OpenGL ES renderer.");
			}
			else if (ren_string == "ogles2")
			{
				renderer_driver = "opengles2";

				bstone::Log::write(
					"VID: Forcing OpenGL ES 2 renderer.");
			}
			else if (ren_string == "soft")
			{
				renderer_driver = "software";

				bstone::Log::write(
					"VID: Forcing software renderer.");
			}
			else
			{
				bstone::Log::write_warning("VID: Unsupported renderer: \"" + ren_string + "\".");
			}


			auto hint_result = ::SDL_SetHint(
				SDL_HINT_RENDER_DRIVER,
				renderer_driver);

			if (hint_result == SDL_FALSE)
			{
				bstone::Log::write_warning(
					::SDL_GetError());
			}
		}
	}

	if (is_succeed)
	{
		bstone::Log::write(
			"VID: Creating a renderer...");

		::sw_renderer = ::SDL_CreateRenderer(
			::sw_window,
			-1,
			renderer_flags);

		if (!::sw_renderer)
		{
			is_succeed = false;

			::sw_error_message = "VID: Failed to create a renderer: ";
			::sw_error_message += ::SDL_GetError();

			bstone::Log::write_error(
				::SDL_GetError());
		}
	}


	SDL_RendererInfo renderer_info;

	if (is_succeed)
	{
		bstone::Log::write(
			"VID: Quering renderer for info...");

		auto sdl_result = ::SDL_GetRendererInfo(
			::sw_renderer,
			&renderer_info);

		if (sdl_result != 0)
		{
			is_succeed = false;

			::sw_error_message = "VID: Failed to query the renderer: ";
			::sw_error_message += ::SDL_GetError();

			bstone::Log::write_error(
				::SDL_GetError());
		}
	}


	if (is_succeed)
	{
		if (renderer_driver)
		{
			if (::SDL_strcasecmp(
				renderer_driver,
				renderer_info.name) != 0)
			{
				bstone::Log::write_warning("VID: Unexpected renderer is selected: \""s + renderer_info.name + "\"."s);
			}
		}
		else
		{
			bstone::Log::write("VID: Current renderer: \""s + renderer_info.name + "\"."s);
		}
	}


	std::uint32_t pixel_format = SDL_PIXELFORMAT_UNKNOWN;

	if (is_succeed)
	{
		bstone::Log::write(
			"VID: Looking up for a texture pixel format...");

		const auto format_count = renderer_info.num_texture_formats;

		for (auto i = decltype(format_count){}; i < format_count; ++i)
		{
			const auto format = renderer_info.texture_formats[i];

			if (
				SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED32 &&
				SDL_PIXELLAYOUT(format) == SDL_PACKEDLAYOUT_8888 &&
				SDL_ISPIXELFORMAT_ALPHA(format))
			{
				pixel_format = format;
				break;
			}
		}

		if (pixel_format == SDL_PIXELFORMAT_UNKNOWN)
		{
			bstone::Log::write_warning(
				"Falling back to a predefined pixel format.");

			pixel_format = SDL_PIXELFORMAT_ARGB8888;
		}


		bstone::Log::write(
			"VID: Allocating a texture pixel format...");

		::sw_texture_pixel_format = ::SDL_AllocFormat(
			pixel_format);

		if (!::sw_texture_pixel_format)
		{
			is_succeed = false;

			::sw_error_message = "VID: Failed to allocate a texture pixel format: ";
			::sw_error_message += ::SDL_GetError();

			bstone::Log::write_error(
				::SDL_GetError());
		}
	}

	return is_succeed;
}

bool sw_initialize_screen_texture()
{
	bstone::Log::write(
		"VID: Creating a screen texture...");

	::sw_screen_texture = ::SDL_CreateTexture(
		::sw_renderer,
		::sw_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_width,
		::vga_height);

	if (!::sw_screen_texture)
	{
		::sw_error_message = "VID: Failed to create a screen texture: ";
		::sw_error_message += ::SDL_GetError();

		bstone::Log::write(
			::SDL_GetError());

		return false;
	}

	return true;
}

bool sw_initialize_ui_texture()
{
	bstone::Log::write(
		"VID: Creating an UI texture...");

	::sw_ui_texture = ::SDL_CreateTexture(
		::sw_renderer,
		::sw_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_ref_width,
		::vga_ref_height);

	if (!::sw_ui_texture)
	{
		::sw_error_message = "VID: Failed to create an UI texture: ";
		::sw_error_message += ::SDL_GetError();

		bstone::Log::write(
			::SDL_GetError());

		return false;
	}

	return true;
}

bool sw_initialize_textures()
{
	bstone::Log::write(
		"VID: Initializing textures...");


	auto is_succeed = true;

	if (is_succeed)
	{
		is_succeed = sw_initialize_screen_texture();
	}

	if (is_succeed)
	{
		is_succeed = sw_initialize_ui_texture();
	}

	return is_succeed;
}

void sw_update_palette(
	int first_index,
	int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = ::vid_vga_palette[color_index];
		auto& sdl_color = ::sw_palette[color_index];

		sdl_color = ::SDL_MapRGB(
			::sw_texture_pixel_format,
			(255 * vga_color.r) / 63,
			(255 * vga_color.g) / 63,
			(255 * vga_color.b) / 63);
	}
}

void sw_update_viewport()
{
	auto sdl_result = ::SDL_RenderSetLogicalSize(
		::sw_renderer,
		::screen_width,
		::screen_height);

	if (sdl_result != 0)
	{
		bstone::Log::write_error(
			"VID: Failed to update a viewport.");
	}
}

void sw_initialize_palette()
{
	::vid_vga_palette.fill({});

	::sw_update_palette(
		0,
		palette_color_count);
}

void sw_calculate_dimensions()
{
	const auto alignment = 2;

	// Decrease by 20% to compensate vanilla VGA stretch.
	::vga_height = (10 * window_height) / 12;
	::vga_height += alignment - 1;
	::vga_height /= alignment;
	::vga_height *= alignment;

	if (::vid_widescreen)
	{
		::vga_width = ::window_width;
	}
	else
	{
		::vga_width = (::vga_ref_width * ::vga_height) / ::vga_ref_height;
	}

	::vga_width += alignment - 1;
	::vga_width /= alignment;
	::vga_width *= alignment;

	::vga_width_scale = static_cast<float>(::vga_width) / static_cast<float>(::vga_ref_width);
	::vga_height_scale = static_cast<float>(::vga_height) / static_cast<float>(::vga_ref_height_4x3);

	::vga_area = ::vga_width * ::vga_height;

	::screen_width = ::vga_width;

	::screen_height = (12 * ::vga_height) / 10;
	::screen_height += alignment - 1;
	::screen_height /= alignment;
	::screen_height *= alignment;

	::filler_width = (::screen_width * ::vga_ref_height_4x3) - (::screen_height * ::vga_ref_width);
	::filler_width /= 2 * ::vga_ref_height_4x3;

#ifdef __vita__
	const auto upper_filler_height = (::screen_height * ref_top_bar_height) / ::vga_ref_height + 1; //todo: double check then just hardcode values
	const auto lower_filler_height = (::screen_height * ref_bottom_bar_height) / ::vga_ref_height + 1;
#else  
	const auto upper_filler_height = (::screen_height * ref_top_bar_height) / ::vga_ref_height;
	const auto lower_filler_height = (::screen_height * ref_bottom_bar_height) / ::vga_ref_height;
#endif
	const auto middle_filler_height = ::screen_height - (upper_filler_height + lower_filler_height);

	// UI whole rect
	//
	::sw_ui_whole_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::vga_ref_height,
	};

	::sw_ui_whole_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		::screen_height,
	};


	// UI top rect
	//
	::sw_ui_top_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::ref_top_bar_height,
	};

	::sw_ui_top_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		upper_filler_height,
	};


	// UI middle rect (stretched to full width)
	//
	::sw_ui_wide_middle_src_rect = SDL_Rect{
		0,
		::ref_view_top_y,
		::vga_ref_width,
		::ref_view_height,
	};

	::sw_ui_wide_middle_dst_rect = SDL_Rect{
		0,
		upper_filler_height,
		::screen_width,
		middle_filler_height,
	};


	// UI bottom rect
	//
	::sw_ui_bottom_src_rect = SDL_Rect{
		0,
		::ref_view_bottom_y + 1,
		::vga_ref_width,
		::ref_bottom_bar_height,
	};

	::sw_ui_bottom_dst_rect = SDL_Rect{
		::filler_width,
		::screen_height - lower_filler_height,
		::screen_width - (2 * ::filler_width),
		lower_filler_height,
	};


	// UI left bar
	::sw_filler_ui_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		::screen_height,
	};

	// UI right bar
	::sw_filler_ui_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		::screen_height,
	};

	// HUD upper left rect
	::sw_filler_hud_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		upper_filler_height
	};

	// HUD upper right rect
	::sw_filler_hud_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		upper_filler_height,
	};

	// HUD lower left rect
	::sw_filler_hud_rects[2] = SDL_Rect{
		0,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	// HUD lower right rect
	::sw_filler_hud_rects[3] = SDL_Rect{
		::screen_width - ::filler_width,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	::sw_filler_color = SDL_Color{
		::vgapal[(filler_color_index * 3) + 0],
		::vgapal[(filler_color_index * 3) + 1],
		::vgapal[(filler_color_index * 3) + 2],
		0xFF, };
}

void sw_initialize_video()
{
	bstone::Log::write(
		"VID: Initializing a system...");

	bool is_custom_scale = false;

	//
	// Option "vid_windowed"
	//

	::vid_is_windowed = ::g_args.has_option(
		"vid_windowed");

	::vid_use_custom_window_position = false;


	//
	// Option "vid_window_x"
	//

	const auto& vid_window_x_str = ::g_args.get_option_value(
		"vid_window_x");

	if (bstone::StringHelper::string_to_int(vid_window_x_str, ::vid_window_x))
	{
		::vid_use_custom_window_position = true;
	}


	//
	// Option "vid_window_y"
	//

	const auto& vid_window_y_str = ::g_args.get_option_value(
		"vid_window_y");

	if (bstone::StringHelper::string_to_int(vid_window_y_str, ::vid_window_y))
	{
		::vid_use_custom_window_position = true;
	}


	//
	// Option "vid_mode"
	//

	std::string width_str;
	std::string height_str;

	::g_args.get_option_values(
		"vid_mode",
		width_str,
		height_str);

	static_cast<void>(bstone::StringHelper::string_to_int(width_str, ::window_width));
	static_cast<void>(bstone::StringHelper::string_to_int(height_str, ::window_height));

	if (::window_width == 0)
	{
		::window_width = ::default_window_width;
	}

	if (::window_height == 0)
	{
		::window_height = ::default_window_height;
	}

	if (::window_width < ::vga_ref_width)
	{
		::window_width = ::vga_ref_width;
	}

	if (::window_height < ::vga_ref_height_4x3)
	{
		::window_height = ::vga_ref_height_4x3;
	}


	//
	// Option "vid_scale"
	//

	const auto& vid_scale_str = ::g_args.get_option_value(
		"vid_scale");

	if (!vid_scale_str.empty())
	{
		int scale_value;

		if (bstone::StringHelper::string_to_int(vid_scale_str, scale_value))
		{
			is_custom_scale = true;

			if (scale_value < 1)
			{
				scale_value = 1;
			}

			::vga_scale = scale_value;
		}
	}


	int sdl_result = 0;

	sdl_result = ::SDL_GetDesktopDisplayMode(
		0,
		&::display_mode);

	if (sdl_result != 0)
	{
		::Quit("VID: Failed to get a display mode.");
	}

	if (!::vid_is_windowed)
	{
		::window_width = ::display_mode.w;
		::window_height = ::display_mode.h;
	}


	::sw_calculate_dimensions();

	bool is_succeed = true;

	if (is_succeed)
	{
		is_succeed = ::sw_initialize_window();
	}

	if (is_succeed)
	{
		is_succeed = ::sw_initialize_renderer();
	}

	if (is_succeed)
	{
		is_succeed = ::sw_initialize_textures();
	}

	if (is_succeed)
	{
		::sw_initialize_palette();
	}

	if (is_succeed)
	{
		::sw_initialize_vga_buffer();
		::sw_initialize_ui_buffer();
	}

	if (is_succeed)
	{
		::SDL_ShowWindow(
			::sw_window);

		::in_grab_mouse(
			true);
	}
	else
	{
		::Quit(
			::sw_error_message);
	}
}

void sw_uninitialize_screen_texture()
{
	if (::sw_screen_texture)
	{
		::SDL_DestroyTexture(
			::sw_screen_texture);

		::sw_screen_texture = nullptr;
	}
}

void sw_uninitialize_ui_texture()
{
	if (::sw_ui_texture)
	{
		::SDL_DestroyTexture(
			::sw_ui_texture);

		::sw_ui_texture = nullptr;
	}
}

void sw_uninitialize_vga_buffer()
{
	::sw_vga_buffer.clear();
	::sw_vga_buffer.shrink_to_fit();

	::vga_memory = nullptr;
}

void sw_uninitialize_video()
{
	if (::sw_texture_pixel_format)
	{
		::SDL_FreeFormat(
			::sw_texture_pixel_format);

		::sw_texture_pixel_format = nullptr;
	}

	::sw_uninitialize_screen_texture();
	::sw_uninitialize_ui_texture();

	if (::sw_renderer)
	{
		::SDL_DestroyRenderer(
			::sw_renderer);

		::sw_renderer = nullptr;
	}

	if (::sw_window)
	{
		::SDL_DestroyWindow(
			::sw_window);

		::sw_window = nullptr;
	}

	::sw_uninitialize_vga_buffer();
}

void sw_refresh_screen()
{
	int sdl_result = 0;

	// HUD+3D stuff
	//
	if (::vid_is_hud)
	{
		const auto src_pixels = ::sw_vga_buffer.data();
		const auto src_pitch = ::vga_width;

		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = ::SDL_LockTexture(
			::sw_screen_texture,
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::Quit("VID: Failed to lock a screen texture: "s + ::SDL_GetError());
		}

		auto dst_pixels = static_cast<std::uint32_t*>(
			dst_raw_pixels);

		for (int y = 0; y < ::vga_height; ++y)
		{
			const auto src_line = &src_pixels[y * src_pitch];
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < ::vga_width; ++x)
			{
				dst_line[x] = sw_palette[src_line[x]];
			}
		}

		::SDL_UnlockTexture(
			::sw_screen_texture);
	}


	// 2D stuff
	//
	{
		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = ::SDL_LockTexture(
			::sw_ui_texture,
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::Quit("VID: Failed to lock an UI texture: "s + ::SDL_GetError());
		}

		const auto alpha_0_mask = ~sw_texture_pixel_format->Amask;

		auto dst_pixels = static_cast<std::uint32_t*>(
			dst_raw_pixels);

		for (int y = 0; y < ::vga_ref_height; ++y)
		{
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < ::vga_ref_width; ++x)
			{
				const auto src_offset = (y * ::vga_ref_width) + x;
				auto dst_color = ::sw_palette[::vid_ui_buffer[src_offset]];

				if (::vid_is_hud)
				{
					if (!::vid_mask_buffer[src_offset])
					{
						dst_color &= alpha_0_mask;
					}
				}

				dst_line[x] = dst_color;
			}
		}

		::SDL_UnlockTexture(
			::sw_ui_texture);
	}


	// Clear all
	//
	sdl_result = ::SDL_RenderClear(
		sw_renderer);

	if (sdl_result != 0)
	{
		::Quit("VID: Failed to clear a render target: "s + ::SDL_GetError());
	}


	// Copy HUD+3D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_RenderCopy(
			sw_renderer,
			sw_screen_texture,
			nullptr,
			nullptr);

		if (sdl_result != 0)
		{
			::Quit("VID: Failed to copy a screen texture on a render target: "s + ::SDL_GetError());
		}
	}


	// Use filler if necessary
	//
	if (!::vid_is_ui_stretched)
	{
		const auto is_hud = ::vid_is_hud;

		auto fill_color = SDL_Color{};

		if (!::vid_is_movie)
		{
			fill_color = ::sw_filler_color;
		}

		::SDL_SetRenderDrawColor(
			sw_renderer,
			fill_color.r,
			fill_color.g,
			fill_color.b,
			0xFF);

		if (is_hud)
		{
			::SDL_RenderFillRects(sw_renderer, ::sw_filler_hud_rects.data(), 4);
		}
		else
		{
			::SDL_RenderFillRects(sw_renderer, ::sw_filler_ui_rects.data(), 2);
		}
	}


	// Copy 2D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sw_ui_texture,
			SDL_BLENDMODE_BLEND);

		if (sdl_result != 0)
		{
			::Quit("VID: Failed to set blend mode for an UI texture: "s + ::SDL_GetError());
		}
	}

	if (!::vid_is_ui_stretched)
	{
		if (::vid_is_fizzle_fade)
		{
			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer,
					::sw_ui_texture,
					&::sw_ui_top_src_rect,
					&::sw_ui_top_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer,
					::sw_ui_texture,
					&::sw_ui_wide_middle_src_rect,
					&::sw_ui_wide_middle_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer,
					::sw_ui_texture,
					&::sw_ui_bottom_src_rect,
					&::sw_ui_bottom_dst_rect);
			}
		}
		else
		{
			sdl_result = ::SDL_RenderCopy(
				::sw_renderer,
				::sw_ui_texture,
				nullptr,
				&::sw_ui_whole_dst_rect);
		}
	}
	else
	{
		sdl_result = ::SDL_RenderCopy(
			::sw_renderer,
			::sw_ui_texture,
			nullptr,
			nullptr);
	}

	if (sdl_result != 0)
	{
		::Quit("VID: Failed to copy an UI texture on a render target: "s + ::SDL_GetError());
	}

	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sw_ui_texture,
			SDL_BLENDMODE_NONE);

		if (sdl_result != 0)
		{
			::Quit("VID: Failed to set blend mode for an UI texture: "s + ::SDL_GetError());
		}
	}


	// Present
	//
	::SDL_RenderPresent(
		sw_renderer);

	if (sdl_result != 0)
	{
		::Quit("VID: Failed to present a render target: "s + ::SDL_GetError());
	}
}

void sw_check_vsync()
{
	using Clock = std::chrono::system_clock;

	constexpr int draw_count = 10;

	constexpr int duration_tolerance_pct = 25;

	const int expected_duration_ms =
		(1000 * draw_count) / ::display_mode.refresh_rate;

	const int min_expected_duration_ms =
		((100 - duration_tolerance_pct) * expected_duration_ms) / 100;

	const auto before_timestamp = Clock::now();

	for (int i = 0; i < draw_count; ++i)
	{
		::sw_refresh_screen();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	::vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}

void sw_update_widescreen()
{
	::sw_uninitialize_screen_texture();
	::sw_uninitialize_vga_buffer();
	::sw_calculate_dimensions();
	::sw_initialize_vga_buffer();
	::sw_initialize_screen_texture();
	::sw_update_viewport();
}


// ==========================================================================
// Accelerated renderer.
//

const auto hw_3d_map_dimension_f = static_cast<float>(MAPSIZE);
const auto hw_3d_map_height_f = 1.0F;

template<typename T>
constexpr auto hw_3d_tile_dimension = static_cast<T>(1);

constexpr auto hw_3d_tile_dimension_i = ::hw_3d_tile_dimension<int>;
constexpr auto hw_3d_tile_dimension_f = ::hw_3d_tile_dimension<float>;
constexpr auto hw_3d_tile_dimension_d = ::hw_3d_tile_dimension<double>;

template<typename T>
constexpr auto hw_3d_tile_half_dimension = hw_3d_tile_dimension<T> / static_cast<T>(2);

constexpr auto hw_3d_tile_half_dimension_f = ::hw_3d_tile_half_dimension<float>;
constexpr auto hw_3d_tile_half_dimension_d = ::hw_3d_tile_half_dimension<double>;


template<int TCount>
struct HwIbItem
{
	using Type = std::conditional_t<
		TCount <= 0x100,
		std::uint8_t,
		std::conditional_t<
			TCount <= 0x10000,
			std::uint16_t,
			std::uint32_t
		>
	>;
}; // HwIbItem

template<int TCount>
struct HwMapDimensionBitDepth
{
	static constexpr int Value = std::numeric_limits<typename HwIbItem<TCount>::Type>::digits;
}; // HwMapDimensionBitDepth

template<int TCount>
struct HwMapDimensionMask
{
	static constexpr int Value = (TCount == 8 ? 0xFF : (TCount == 16 ? 0xFFFF : 0xFFFFFFFF));
}; // HwMapDimensionMask

constexpr auto hw_map_dimension_bit_depth = HwMapDimensionBitDepth<MAPSIZE>::Value;
constexpr auto hw_map_dimension_mask = HwMapDimensionMask<hw_map_dimension_bit_depth>::Value;

constexpr auto hw_3d_max_sides_per_wall = 4;
constexpr auto hw_3d_max_indices_per_wall_side = 6;
constexpr auto hw_3d_max_vertices_per_wall_side = 4;

constexpr auto hw_3d_max_wall_sides_indices = 0x10000;

constexpr auto hw_3d_max_sides_per_door = 2;
constexpr auto hw_3d_max_halves_per_door = hw_3d_max_sides_per_door * 2;
constexpr auto hw_3d_max_vertices_per_door_half = 4;
constexpr auto hw_3d_vertices_per_door = ::hw_3d_max_sides_per_door * hw_3d_max_vertices_per_door_half;
constexpr auto hw_3d_vertices_per_door_sides = 2 * hw_3d_vertices_per_door;

constexpr auto hw_3d_max_indices_per_door_half = 6;
constexpr auto hw_3d_max_indices_per_door = 2 * hw_3d_max_indices_per_door_half;
constexpr auto hw_3d_max_indices_per_door_side = 2 * hw_3d_max_indices_per_door;

constexpr auto hw_3d_max_door_sides_vertices = MAXDOORS * hw_3d_vertices_per_door_sides;
constexpr auto hw_3d_max_door_sides_indices = MAXDOORS * ::hw_3d_max_indices_per_door_side;


constexpr auto hw_3d_max_sprites = MAXSTATS + MAXACTORS;

constexpr auto hw_3d_vertices_per_sprite = 4;
constexpr auto hw_3d_indices_per_sprite = 6;

constexpr auto hw_3d_max_statics_vertices = MAXSTATS * ::hw_3d_vertices_per_sprite;
constexpr auto hw_3d_max_actors_vertices = MAXACTORS * ::hw_3d_vertices_per_sprite;
constexpr auto hw_3d_max_sprites_vertices = ::hw_3d_max_statics_vertices + ::hw_3d_max_actors_vertices;
constexpr auto hw_3d_statics_base_vertex_index = 0;
constexpr auto hw_3d_actors_base_vertex_index = ::hw_3d_max_statics_vertices;

constexpr auto hw_3d_max_statics_indices = MAXSTATS * ::hw_3d_indices_per_sprite;
constexpr auto hw_3d_max_actor_indices = MAXACTORS * ::hw_3d_indices_per_sprite;
constexpr auto hw_3d_max_sprites_indices = ::hw_3d_max_statics_indices + ::hw_3d_max_actor_indices;

constexpr auto hw_min_2d_commands = 16;
constexpr auto hw_min_3d_commands = 4096;


struct Hw3dQuadFlags
{
	using Value = unsigned int;


	Value is_vertical_ : 1;
	Value is_back_face_ : 1;
}; // Hw3dQuadFlags


struct Hw3dWall;
using Hw3dWallPtr = Hw3dWall*;
using Hw3dWallCPtr = const Hw3dWall*;

struct Hw3dWallSideFlags
{
	using Type = unsigned int;


	Type is_active_ : 1;
	Type is_vertical_ : 1;
	Type is_door_track_ : 1;
}; // Hw3dWallSideFlags

struct Hw3dWallSide
{
	Hw3dWallCPtr wall_;

	Hw3dWallSideFlags flags_;
	int vertex_index_;
	bstone::RendererTexture2dPtr texture_2d_;
}; // Hw3dWallSide

using Hw3dWallSidePtr = Hw3dWallSide*;
using Hw3dWallSideCPtr = const Hw3dWallSide*;


struct Hw3dWall
{
	static constexpr auto max_sides = 4;

	using Sides = std::array<Hw3dWallSide, max_sides>;


	int x_;
	int y_;

	Sides sides_;
}; // Hw3dWall

using Hw3dXyWallMap = std::unordered_map<int, Hw3dWall>;

enum Hw3dXyWallKind
{
	solid,
	push,
}; // Hw3dXyWallKind

struct Hw3dWallSideDrawItem
{
	bstone::RendererTexture2dPtr texture_2d_;
	Hw3dWallSideCPtr wall_side_;
}; // Hw3dWallSideDrawItem


struct Hw3dDoor;
using Hw3dDoorPtr = Hw3dDoor*;

struct Hw3dDoorSide
{
	Hw3dDoorPtr door_;

	bool is_back_face_;

	int vertex_index_;
	bstone::RendererTexture2dPtr texture_2d_;
}; // Hw3dDoorSide

using Hw3dDoorSidePtr = Hw3dDoorSide*;
using Hw3dDoorSideCPtr = const Hw3dDoorSide*;


struct Hw3dDoor
{
	using Sides = std::array<Hw3dDoorSide, ::hw_3d_max_sides_per_door>;

	const doorobj_t* door_;

	Sides sides_;
}; // Hw3dDoor


struct Hw3dDoorDrawItem
{
	bstone::RendererTexture2dPtr texture_2d_;
	Hw3dDoorSideCPtr door_side_;
}; // Hw3dDoorDrawItem

using Hw3dXyDoorMap = std::unordered_map<int, Hw3dDoor>;

using Hw3dDoorDrawItems = std::vector<Hw3dDoorDrawItem>;

using Hw3dWallSideDrawItems = std::vector<Hw3dWallSideDrawItem>;

using Hw3dWallSideIndexBufferItem = HwIbItem<hw_3d_max_wall_sides_indices>::Type;
using Hw3dWallSideIndexBuffer = std::vector<Hw3dWallSideIndexBufferItem>;

using Hw3dDoorIndexBufferItem = HwIbItem<::hw_3d_max_door_sides_indices>::Type;
using Hw3dDoorIndexBuffer = std::vector<Hw3dDoorIndexBufferItem>;


enum class Hw3dSpriteKind
{
	stat,
	actor,
}; // Hw3dSpriteKind

struct Hw3dSpriteFlags
{
	using Value = unsigned int;


	Value is_visible_ : 1;
}; // Hw3dSpriteFlags

struct Hw3dSprite
{
	union BsObject
	{
		const statobj_t* stat_;
		const objtype* actor_;
	}; // BsObject


	int x_;
	int y_;
	double square_distance_;

	Hw3dSpriteKind kind_;
	Hw3dSpriteFlags flags_;
	int vertex_index_;
	BsObject bs_object_;
	bstone::RendererTexture2dPtr texture_2d_;
}; // Hw3dSprite


using Hw3dSpritePtr = Hw3dSprite*;
using Hw3dSpriteCPtr = const Hw3dSprite*;

struct Hw3dSpriteDrawItem
{
	bstone::RendererTexture2dPtr texture_2d_;
	Hw3dSpriteCPtr sprite_;
}; // Hw3dSpriteDrawItem

using Hw3dXySpriteMap = std::unordered_map<int, Hw3dSprite>;
using Hw3dXySpriteMapPtr = Hw3dXySpriteMap*;

using Hw3dSpritesDrawList = std::vector<Hw3dSpriteDrawItem>;

using Hw3dSpritesIndexBufferItem = HwIbItem<::hw_3d_max_sprites_indices>::Type;
using Hw3dSpritesIndexBuffer = std::vector<Hw3dSpritesIndexBufferItem>;


using HwVbBuffer = std::vector<bstone::RendererVertex>;


constexpr auto hw_2d_quad_count = 2;

constexpr auto hw_2d_index_count_ = hw_2d_quad_count * 6;
constexpr auto hw_2d_stretched_index_offset_ = 0;
constexpr auto hw_2d_non_stretched_index_offset_ = 6;

constexpr auto hw_2d_vertex_count_ = hw_2d_quad_count * 4;
constexpr auto hw_2d_stretched_vertex_offset_ = 0;
constexpr auto hw_2d_non_stretched_vertex_offset_ = 4;

using Hw2dVertices = std::array<bstone::RendererVertex, hw_2d_vertex_count_>;


constexpr auto hw_2d_fillers_ui_quad_count = 2;
constexpr auto hw_2d_fillers_hud_quad_count = 4;
constexpr auto hw_2d_fillers_quad_count = ::hw_2d_fillers_ui_quad_count + ::hw_2d_fillers_hud_quad_count;

constexpr auto hw_2d_fillers_index_count_ = ::hw_2d_fillers_quad_count * 6;
constexpr auto hw_2d_fillers_ui_index_offset_ = 0;
constexpr auto hw_2d_fillers_hud_index_offset_ = 6 * ::hw_2d_fillers_ui_quad_count;

constexpr auto hw_2d_fillers_vertex_count_ = hw_2d_fillers_quad_count * 4;


int hw_2d_width_4x3_ = 0;

int hw_2d_left_filler_width_4x3_ = 0;
int hw_2d_right_filler_width_4x3_ = 0;

int hw_2d_top_filler_height_4x3_ = 0;
int hw_2d_bottom_filler_height_4x3_ = 0;

int hw_3d_viewport_x_ = 0;
int hw_3d_viewport_y_ = 0;
int hw_3d_viewport_width_ = 0;
int hw_3d_viewport_height_ = 0;


Hw2dVertices hw_2d_vertices_;

bstone::RendererManagerUPtr hw_renderer_manager_ = nullptr;
bstone::RendererPtr hw_renderer_ = nullptr;

bstone::RendererTextureManagerUPtr hw_texture_manager_ = nullptr;

bstone::RendererPalette hw_palette_;

bstone::RendererCommandSets hw_command_sets_;
bstone::RendererCommandSet* hw_2d_command_set_;
bstone::RendererCommandSet* hw_3d_command_set_;

bstone::RendererTexture2dPtr hw_2d_texture_ = nullptr;
bstone::RendererIndexBufferPtr hw_2d_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_2d_vb_ = nullptr;

bstone::RendererTexture2dPtr hw_2d_black_t2d_1x1_ = nullptr;
bstone::RendererTexture2dPtr hw_2d_white_t2d_1x1_ = nullptr;

bstone::RendererIndexBufferPtr hw_2d_fillers_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_2d_fillers_vb_ = nullptr;

bool hw_2d_fade_is_enabled_ = false;
bstone::RendererColor32 hw_2d_fade_color_ = bstone::RendererColor32{};
bstone::RendererTexture2dPtr hw_2d_fade_t2d_ = nullptr;


bstone::RendererIndexBufferPtr hw_3d_flooring_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_flooring_vb_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_flooring_solid_t2d_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_flooring_textured_t2d_ = nullptr;

bstone::RendererIndexBufferPtr hw_3d_ceiling_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_ceiling_vb_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_ceiling_solid_t2d_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_ceiling_textured_t2d_ = nullptr;


auto hw_3d_player_direction = bstone::Vec2D{};
auto hw_3d_player_position = bstone::Vec2D{};


bool hw_3d_has_active_pushwall_ = false;
int hw_3d_active_pushwall_next_x_ = 0;
int hw_3d_active_pushwall_next_y_ = 0;

int hw_3d_wall_count_ = 0;
int hw_3d_wall_side_count_ = 0;
Hw3dXyWallMap hw_3d_xy_wall_map_;

int hw_3d_wall_side_draw_item_count_ = 0;
Hw3dWallSideDrawItems hw_3d_wall_side_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_wall_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_wall_sides_vb_ = nullptr;

Hw3dWallSideIndexBuffer hw_3d_wall_sides_ib_buffer_;


int hw_3d_pushwall_count_ = 0;
int hw_3d_pushwall_side_count_ = 0;
Hw3dXyWallMap hw_3d_xy_pushwall_map_;

int hw_3d_pushwall_side_draw_item_count_ = 0;
Hw3dWallSideDrawItems hw_3d_pushwall_side_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_pushwall_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_pushwall_sides_vb_ = nullptr;

Hw3dWallSideIndexBuffer hw_3d_pushwall_sides_ib_buffer_;
HwVbBuffer hw_3d_pushwalls_vb_buffer_;


int hw_3d_door_count_ = 0;

Hw3dXyDoorMap hw_3d_xy_door_map_;

int hw_3d_door_draw_item_count_ = 0;
Hw3dDoorDrawItems hw_3d_door_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_door_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_door_sides_vb_ = nullptr;

Hw3dDoorIndexBuffer hw_3d_door_sides_ib_buffer_;
HwVbBuffer hw_3d_doors_vb_buffer_;


Hw3dXySpriteMap hw_3d_xy_static_map_;

int hw_3d_sprites_draw_count_ = 0;
Hw3dSpritesDrawList hw_3d_sprites_draw_list_;

bstone::RendererIndexBufferPtr hw_3d_sprites_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_sprites_vb_ = nullptr;

Hw3dSpritesIndexBuffer hw_3d_sprites_ib_buffer_;
HwVbBuffer hw_3d_sprites_vb_buffer_;


void hw_dbg_3d_orient_all_sprites();


constexpr int hw_encode_xy(
	const int x,
	const int y)
{
	return (x << ::hw_map_dimension_bit_depth) | y;
}

constexpr void hw_decode_xy(
	const int xy,
	int& x,
	int& y)
{
	x = (xy >> ::hw_map_dimension_bit_depth) & ::hw_map_dimension_mask;
	y = xy & ::hw_map_dimension_mask;
}

bstone::RendererColor32 hw_vga_color_to_color_32(
	const int vga_red,
	const int vga_green,
	const int vga_blue)
{
	return bstone::RendererColor32
	{
		static_cast<std::uint8_t>((0xFF * vga_red) / 0x3F),
		static_cast<std::uint8_t>((0xFF * vga_green) / 0x3F),
		static_cast<std::uint8_t>((0xFF * vga_blue) / 0x3F),
		0xFF
	};
}

void hw_3d_update_player_direction()
{
	const auto direction_angle = (::player->angle * m_pi()) / 180.0;

	::hw_3d_player_direction[0] = std::cos(direction_angle);
	::hw_3d_player_direction[1] = -std::sin(direction_angle);
}

void hw_3d_update_player_position()
{
	::hw_3d_player_position[0] = bstone::FixedPoint{::player->x}.to_double();
	::hw_3d_player_position[1] = bstone::FixedPoint{::player->y}.to_double();
}

void hw_3d_update_player()
{
	::hw_3d_update_player_direction();
	::hw_3d_update_player_position();
}

void hw_initialize_vga_buffer()
{
	::sw_initialize_vga_buffer();
}

void hw_initialize_ui_buffer()
{
	::sw_initialize_ui_buffer();
}

bool hw_initialize_renderer()
{
	bstone::Log::write("VID: Initializing HW renderer...");

	// Custom position.
	//
	if (::vid_use_custom_window_position)
	{
		if (::vid_window_x < 0)
		{
			::vid_window_x = 0;
		}

		if (::vid_window_y < 0)
		{
			::vid_window_y = 0;
		}
	}

	// Title.
	//
	const auto& assets_info = AssetsInfo{};

	auto title = "Blake Stone"s;

	if (assets_info.is_aog())
	{
		auto version_string = std::string{};

		if (assets_info.is_aog_full_v1_0() || assets_info.is_aog_sw_v1_0())
		{
			version_string = "v1.0";
		}
		else if (assets_info.is_aog_full_v2_0() || assets_info.is_aog_sw_v2_0())
		{
			version_string = "v2.0";
		}
		else if (assets_info.is_aog_full_v2_1() || assets_info.is_aog_sw_v2_1())
		{
			version_string = "v2.1";
		}
		else if (assets_info.is_aog_full_v3_0() || assets_info.is_aog_sw_v3_0())
		{
			version_string = "v3.0";
		}

		auto type = std::string{};

		if (assets_info.is_aog_full())
		{
			type = "full";
		}
		else if (assets_info.is_aog_sw())
		{
			type = "shareware";
		}

		const auto has_type_or_version = (!version_string.empty() || !type.empty());

		title += ": Aliens of Gold";

		if (has_type_or_version)
		{
			title += " (";

			if (!type.empty())
			{
				title += type;
			}

			if (!version_string.empty())
			{
				if (!type.empty())
				{
					title += ", ";
				}

				title += version_string;
			}

			title += ')';
		}
	}
	else if (assets_info.is_ps())
	{
		title += ": Planet Strike";
	}


	// Initialization parameter.
	//
	auto param = bstone::RendererInitializeParam{};
	param.renderer_path_ = hw_renderer_manager_->renderer_get_probe_path();

#ifdef __vita__
	param.window_.is_visible_ = true;
#endif // __vita__

	if (!::vid_is_windowed)
	{
		param.window_.is_borderless_ = true;
		param.window_.is_fullscreen_desktop_ = true;
	}

	param.window_.is_positioned_ = ::vid_use_custom_window_position;
	param.window_.x_ = ::vid_window_x;
	param.window_.y_ = ::vid_window_y;

	param.window_.width_ = ::window_width;
	param.window_.height_ = ::window_height;

	param.window_.title_utf8_ = title;

	hw_renderer_ = hw_renderer_manager_->renderer_initialize(param);

	if (!hw_renderer_)
	{
		bstone::Log::write_error("VID: Failed to initialize HW renderer.");

		return false;
	}

	return true;
}

bool hw_2d_create_ib()
{
	auto ib_create_param = bstone::RendererIndexBufferCreateParam{};
	ib_create_param.index_count_ = ::hw_2d_index_count_;

	::hw_2d_ib_ = ::hw_renderer_->index_buffer_create(ib_create_param);

	if (!::hw_2d_ib_)
	{
		return false;
	}


	using Indices = std::array<std::uint8_t, ::hw_2d_index_count_>;

	const auto indices = Indices
	{
		// Stretched quad.
		//
		(4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2,
		(4 * 0) + 0, (4 * 0) + 2, (4 * 0) + 3,

		// Non-stretched quad.
		//
		(4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2,
		(4 * 1) + 0, (4 * 1) + 2, (4 * 1) + 3,
	};

	auto ib_update_param = bstone::RendererIndexBufferUpdateParam{};
	ib_update_param.offset_ = 0;
	ib_update_param.count_ = ::hw_2d_index_count_;
	ib_update_param.indices_ = indices.data();

	::hw_2d_ib_->update(ib_update_param);

	return true;
}

void hw_2d_fill_x_stretched_vb(
	const float left_f,
	const float right_f,
	const float width_f,
	const int vertex_offset)
{
	auto vertex_index = vertex_offset;
	auto& vertices = ::hw_2d_vertices_;

	const auto height_f = static_cast<float>(::window_height);

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_f, 0.0F, 0.0F};
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_f, 0.0F, 0.0F};
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Upper right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_f, height_f, 0.0F};
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Upper left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_f, height_f, 0.0F};
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}
}

void hw_2d_fill_stretched_vb()
{
	const auto left_f = 0.0F;
	const auto right_f = static_cast<float>(::window_width);
	const auto width_f = static_cast<float>(::window_width);

	hw_2d_fill_x_stretched_vb(left_f, right_f, width_f, ::hw_2d_stretched_vertex_offset_);
}

void hw_2d_fill_non_stretched_vb()
{
	const auto left_f = static_cast<float>(::hw_2d_left_filler_width_4x3_);
	const auto right_f = static_cast<float>(::hw_2d_left_filler_width_4x3_ + ::hw_2d_width_4x3_);
	const auto width_f = static_cast<float>(::hw_2d_width_4x3_);

	hw_2d_fill_x_stretched_vb(left_f, right_f, width_f, ::hw_2d_non_stretched_vertex_offset_);
}

bool hw_2d_create_vb()
{
	auto vb_create_param = bstone::RendererVertexBufferCreateParam{};
	vb_create_param.vertex_count_ = ::hw_2d_vertex_count_;

	::hw_2d_vb_ = ::hw_renderer_->vertex_buffer_create(vb_create_param);

	if (!::hw_2d_vb_)
	{
		return false;
	}

	hw_2d_fill_stretched_vb();
	hw_2d_fill_non_stretched_vb();

	auto vb_update_param = bstone::RendererVertexBufferUpdateParam{};
	vb_update_param.offset_ = 0;
	vb_update_param.count_ = ::hw_2d_vertex_count_;
	vb_update_param.vertices_ = ::hw_2d_vertices_.data();

	::hw_2d_vb_->update(vb_update_param);

	return true;
}

bool hw_2d_fillers_create_ib()
{
	auto ib_create_param = bstone::RendererIndexBufferCreateParam{};
	ib_create_param.index_count_ = ::hw_2d_fillers_index_count_;

	::hw_2d_fillers_ib_ = ::hw_renderer_->index_buffer_create(ib_create_param);

	if (!::hw_2d_fillers_ib_)
	{
		return false;
	}

	using Indices = std::array<std::uint8_t, ::hw_2d_fillers_index_count_>;

	const auto& indices = Indices
	{
		// Vertical left.
		//
		(4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2,
		(4 * 0) + 0, (4 * 0) + 2, (4 * 0) + 3,

		// Vertical right.
		//
		(4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2,
		(4 * 1) + 0, (4 * 1) + 2, (4 * 1) + 3,

		// Bottom left.
		//
		(4 * 2) + 0, (4 * 2) + 1, (4 * 2) + 2,
		(4 * 2) + 0, (4 * 2) + 2, (4 * 2) + 3,

		// Bottom right.
		//
		(4 * 3) + 0, (4 * 3) + 1, (4 * 3) + 2,
		(4 * 3) + 0, (4 * 3) + 2, (4 * 3) + 3,

		// Top right.
		//
		(4 * 4) + 0, (4 * 4) + 1, (4 * 4) + 2,
		(4 * 4) + 0, (4 * 4) + 2, (4 * 4) + 3,

		// Top left.
		//
		(4 * 5) + 0, (4 * 5) + 1, (4 * 5) + 2,
		(4 * 5) + 0, (4 * 5) + 2, (4 * 5) + 3,
	};

	auto ib_update_param = bstone::RendererIndexBufferUpdateParam{};
	ib_update_param.offset_ = 0;
	ib_update_param.count_ = ::hw_2d_fillers_index_count_;
	ib_update_param.indices_ = indices.data();

	::hw_2d_fillers_ib_->update(ib_update_param);

	return true;
}

bool hw_2d_fillers_create_vb()
{
	auto vb_create_param = bstone::RendererVertexBufferCreateParam{};
	vb_create_param.vertex_count_ = ::hw_2d_fillers_vertex_count_;

	::hw_2d_fillers_vb_ = ::hw_renderer_->vertex_buffer_create(vb_create_param);

	if (!::hw_2d_fillers_vb_)
	{
		return false;
	}

	const auto& filler_color = ::hw_vga_color_to_color_32(
		::vgapal[(::filler_color_index * 3) + 0],
		::vgapal[(::filler_color_index * 3) + 1],
		::vgapal[(::filler_color_index * 3) + 2]
	);

	const auto left_left_f = static_cast<float>(0.0F);
	const auto left_right_f = static_cast<float>(::hw_2d_left_filler_width_4x3_);

	const auto right_left_f = static_cast<float>(::hw_2d_left_filler_width_4x3_ + ::hw_2d_width_4x3_);
	const auto right_right_f = static_cast<float>(::window_width);

	const auto top_top_f = static_cast<float>(::window_height);
	const auto top_bottom_f = static_cast<float>(::window_height - ::hw_2d_top_filler_height_4x3_);

	const auto bottom_top_f = static_cast<float>(::hw_2d_bottom_filler_height_4x3_);
	const auto bottom_bottom_f = static_cast<float>(0.0F);

	auto vertex_index = 0;

	using Hw2dFillersVertices = std::array<bstone::RendererVertex, hw_2d_fillers_vertex_count_>;
	auto vertices = Hw2dFillersVertices{};


	// ======================================================================
	// UI fillers.
	//

	// ----------------------------------------------------------------------
	// Vertical left.
	//
	// +--+============+--+
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Vertical left.
	// ----------------------------------------------------------------------


	// ----------------------------------------------------------------------
	// Vertical right.
	//
	// +--+============+--+
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Vertical right.
	// ----------------------------------------------------------------------

	//
	// UI fillers.
	// ======================================================================


	// ======================================================================
	// HUD fillers.
	//

	// ----------------------------------------------------------------------
	// Bottom left.
	//
	// +--+============+--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |xx|            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Bottom left.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Bottom right.
	//
	// +--+============+--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |xx|
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Bottom right.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Top right.
	//
	// +--+============+--+
	// |  |            |xx|
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{right_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Top right.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Top left.
	//
	// +--+============+--+
	// |xx|            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = bstone::Vec3F{left_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	//
	// Top left.
	// ----------------------------------------------------------------------

	//
	// HUD fillers.
	// ======================================================================


	auto vb_update_param = bstone::RendererVertexBufferUpdateParam{};
	vb_update_param.offset_ = 0;
	vb_update_param.count_ = ::hw_2d_fillers_vertex_count_;
	vb_update_param.vertices_ = vertices.data();

	::hw_2d_fillers_vb_->update(vb_update_param);

	return true;
}

bool hw_create_solid_texture_1x1(
	const bstone::RendererColor32& color,
	const bool has_alpha,
	bstone::RendererTexture2dPtr& texture_2d)
{
	auto t2d_create_param = bstone::RendererTexture2dCreateParam{};
	t2d_create_param.width_ = 1;
	t2d_create_param.height_ = 1;
	t2d_create_param.has_rgba_alpha_ = has_alpha;
	t2d_create_param.rgba_pixels_ = &color;

	texture_2d = ::hw_renderer_->texture_2d_create(t2d_create_param);

	if (!texture_2d)
	{
		return false;
	}

	return true;
}

bool hw_update_solid_texture_1x1(
	const bstone::RendererColor32& color,
	bstone::RendererTexture2dPtr texture_2d)
{
	if (!texture_2d)
	{
		return false;
	}

	auto param = bstone::RendererTexture2dUpdateParam{};
	param.rgba_pixels_ = &color;

	texture_2d->update(param);

	return true;
}

bool hw_2d_create_black_texture_1x1()
{
	const auto& black_color = bstone::RendererColor32{0x00, 0x00, 0x00, 0xFF};

	return hw_create_solid_texture_1x1(black_color, false, ::hw_2d_black_t2d_1x1_);
}

bool hw_2d_create_white_texture_1x1()
{
	const auto& white_color = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

	return hw_create_solid_texture_1x1(white_color, false, ::hw_2d_white_t2d_1x1_);
}

bool hw_2d_create_fade_texture_1x1()
{
	const auto& color = bstone::RendererColor32{};

	return hw_create_solid_texture_1x1(color, true, ::hw_2d_fade_t2d_);
}

bool hw_initialize_ui_texture()
{
	// Index buffer.
	//
	if (!::hw_2d_create_ib())
	{
		return false;
	}

	if (!::hw_2d_fillers_create_ib())
	{
		return false;
	}

	
	// Vertex buffers.
	//
	if (!::hw_2d_create_vb())
	{
		return false;
	}

	if (!::hw_2d_fillers_create_vb())
	{
		return false;
	}


	// Texture.
	//
	auto param = bstone::RendererTexture2dCreateParam{};
	param.width_ = ::vga_ref_width;
	param.height_ = ::vga_ref_height;
	param.indexed_pixels_ = ::vid_ui_buffer.data();
	param.indexed_alphas_ = ::vid_mask_buffer.data();

	::hw_2d_texture_ = hw_renderer_->texture_2d_create(param);

	if (!::hw_2d_texture_)
	{
		return false;
	}

	if (!hw_2d_create_black_texture_1x1())
	{
		return false;
	}

	if (!hw_2d_create_white_texture_1x1())
	{
		return false;
	}

	if (!hw_2d_create_fade_texture_1x1())
	{
		return false;
	}

	return true;
}

bool hw_initialize_textures()
{
	if (!::hw_initialize_ui_texture())
	{
		return false;
	}

	return true;
}

bool hw_3d_initialize_flooring_ib()
{
	const auto index_count = 6;

	{
		auto param = bstone::RendererIndexBufferCreateParam{};
		param.index_count_ = index_count;

		::hw_3d_flooring_ib_ = ::hw_renderer_->index_buffer_create(param);

		if (!::hw_3d_flooring_ib_)
		{
			return false;
		}
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 1, 2,
			0, 2, 3,
		};

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.count_ = index_count;
		param.offset_ = 0;
		param.indices_ = indices.data();

		::hw_3d_flooring_ib_->update(param);
	}

	return true;
}

bool hw_3d_initialize_flooring_vb()
{
	const auto vertex_count = 4;

	{
		auto param = bstone::RendererVertexBufferCreateParam{};
		param.vertex_count_ = vertex_count;

		::hw_3d_flooring_vb_ = ::hw_renderer_->vertex_buffer_create(param);

		if (!::hw_3d_flooring_vb_)
		{
			return false;
		}
	}

	{
		const auto map_dimension_f = static_cast<float>(MAPSIZE);

		using Vertices = std::array<bstone::RendererVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{0.0F, 0.0F, 0.0F};
			//vertex.xyz_ = bstone::Vec3F{-1.0F, -1.0F, 1.0F};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{0.0F, map_dimension_f, 0.0F};
			//vertex.xyz_ = bstone::Vec3F{1.0F, -1.0F, 1.0F};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{map_dimension_f, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{map_dimension_f, map_dimension_f, 0.0F};
			//vertex.xyz_ = bstone::Vec3F{1.0F, -1.0F, -1.0F};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{map_dimension_f, map_dimension_f};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{map_dimension_f, 0.0F, 0.0F};
			//vertex.xyz_ = bstone::Vec3F{-1.0F, -1.0F, -1.0F};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{0.0F, map_dimension_f};
		}

		auto param = bstone::RendererVertexBufferUpdateParam{};
		param.count_ = vertex_count;
		param.offset_ = 0;
		param.vertices_ = vertices.data();

		::hw_3d_flooring_vb_->update(param);
	}

	return true;
}

bool hw_3d_initialize_flooring_solid_texture_2d()
{
	return ::hw_create_solid_texture_1x1(
		bstone::RendererColor32{},
		false,
		::hw_3d_flooring_solid_t2d_
	);
}

bool hw_3d_initialize_flooring()
{
	if (!::hw_3d_initialize_flooring_ib())
	{
		return false;
	}

	if (!::hw_3d_initialize_flooring_vb())
	{
		return false;
	}

	if (!::hw_3d_initialize_flooring_solid_texture_2d())
	{
		return false;
	}

	return true;
}

void hw_uninitialize_flooring()
{
	if (::hw_3d_flooring_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_flooring_ib_);
		::hw_3d_flooring_ib_ = nullptr;
	}

	if (::hw_3d_flooring_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_flooring_vb_);
		::hw_3d_flooring_vb_ = nullptr;
	}

	if (::hw_3d_flooring_solid_t2d_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_3d_flooring_solid_t2d_);
		::hw_3d_flooring_solid_t2d_ = nullptr;
	}

	::hw_3d_flooring_textured_t2d_ = nullptr;
}

bool hw_3d_initialize_ceiling_ib()
{
	const auto index_count = 6;

	{
		auto param = bstone::RendererIndexBufferCreateParam{};
		param.index_count_ = index_count;

		::hw_3d_ceiling_ib_ = ::hw_renderer_->index_buffer_create(param);

		if (!::hw_3d_ceiling_ib_)
		{
			return false;
		}
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 2, 1,
			0, 3, 2,
		};

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.count_ = index_count;
		param.offset_ = 0;
		param.indices_ = indices.data();

		::hw_3d_ceiling_ib_->update(param);
	}

	return true;
}

bool hw_3d_initialize_ceiling_vb()
{
	const auto vertex_count = 4;

	{
		auto param = bstone::RendererVertexBufferCreateParam{};
		param.vertex_count_ = vertex_count;

		::hw_3d_ceiling_vb_ = ::hw_renderer_->vertex_buffer_create(param);

		if (!::hw_3d_ceiling_vb_)
		{
			return false;
		}
	}

	{
		using Vertices = std::array<bstone::RendererVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{0.0F, 0.0F, ::hw_3d_map_height_f};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{0.0F, ::hw_3d_map_dimension_f, ::hw_3d_map_height_f};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{::hw_3d_map_dimension_f, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{::hw_3d_map_dimension_f, ::hw_3d_map_dimension_f, ::hw_3d_map_height_f};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{::hw_3d_map_dimension_f, ::hw_3d_map_dimension_f};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = bstone::Vec3F{::hw_3d_map_dimension_f, 0.0F, ::hw_3d_map_height_f};
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{0.0F, ::hw_3d_map_dimension_f};
		}

		auto param = bstone::RendererVertexBufferUpdateParam{};
		param.count_ = vertex_count;
		param.offset_ = 0;
		param.vertices_ = vertices.data();

		::hw_3d_ceiling_vb_->update(param);
	}

	return true;
}

bool hw_3d_initialize_ceiling_solid_texture_2d()
{
	return ::hw_create_solid_texture_1x1(
		bstone::RendererColor32{},
		false,
		::hw_3d_ceiling_solid_t2d_
	);
}

bool hw_3d_initialize_ceiling()
{
	if (!::hw_3d_initialize_ceiling_ib())
	{
		return false;
	}

	if (!::hw_3d_initialize_ceiling_vb())
	{
		return false;
	}

	if (!::hw_3d_initialize_ceiling_solid_texture_2d())
	{
		return false;
	}

	return true;
}

void hw_uninitialize_ceiling()
{
	if (::hw_3d_ceiling_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_ceiling_ib_);
		::hw_3d_ceiling_ib_ = nullptr;
	}

	if (::hw_3d_ceiling_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_ceiling_vb_);
		::hw_3d_ceiling_vb_ = nullptr;
	}

	if (::hw_3d_ceiling_solid_t2d_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_3d_ceiling_solid_t2d_);
		::hw_3d_ceiling_solid_t2d_ = nullptr;
	}

	::hw_3d_ceiling_textured_t2d_ = nullptr;
}

bool hw_initialize_solid_walls_ib()
{
	const auto index_count = ::hw_3d_wall_side_count_ * ::hw_3d_max_indices_per_wall_side;

	auto param = bstone::RendererIndexBufferCreateParam{};
	param.index_count_ = index_count;

	::hw_3d_wall_sides_ib_ = ::hw_renderer_->index_buffer_create(param);

	if (!::hw_3d_wall_sides_ib_)
	{
		return false;
	}

	::hw_3d_wall_sides_ib_buffer_.clear();
	::hw_3d_wall_sides_ib_buffer_.resize(index_count);

	return true;
}

void hw_3d_uninitialize_walls_ib()
{
	if (::hw_3d_wall_sides_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_wall_sides_ib_);
		::hw_3d_wall_sides_ib_ = nullptr;
	}

	::hw_3d_wall_sides_ib_buffer_.clear();
}

bool hw_initialize_solid_walls_vb()
{
	auto param = bstone::RendererVertexBufferCreateParam{};
	param.vertex_count_ = ::hw_3d_wall_side_count_ * ::hw_3d_max_vertices_per_wall_side;

	::hw_3d_wall_sides_vb_ = ::hw_renderer_->vertex_buffer_create(param);

	if (!::hw_3d_wall_sides_vb_)
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_walls_vb()
{
	if (::hw_3d_wall_sides_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_wall_sides_vb_);
		::hw_3d_wall_sides_vb_ = nullptr;
	}
}

bool hw_3d_initialize_solid_walls()
{
	::hw_3d_xy_wall_map_.reserve(::hw_3d_wall_count_);

	::hw_3d_wall_side_draw_item_count_ = 0;
	::hw_3d_wall_side_draw_items_.clear();
	::hw_3d_wall_side_draw_items_.resize(::hw_3d_wall_side_count_);

	if (!::hw_initialize_solid_walls_ib())
	{
		return false;
	}

	if (!::hw_initialize_solid_walls_vb())
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_solid_walls()
{
	::hw_3d_wall_count_ = 0;
	::hw_3d_wall_side_count_ = 0;
	::hw_3d_xy_wall_map_.clear();

	::hw_3d_wall_side_draw_item_count_ = 0;
	::hw_3d_wall_side_draw_items_.clear();

	::hw_3d_uninitialize_walls_ib();
	::hw_3d_uninitialize_walls_vb();
}

bool hw_initialize_pushwalls_ib()
{
	const auto index_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_max_indices_per_wall_side;

	auto param = bstone::RendererIndexBufferCreateParam{};
	param.index_count_ = index_count;

	::hw_3d_pushwall_sides_ib_ = ::hw_renderer_->index_buffer_create(param);

	if (!::hw_3d_pushwall_sides_ib_)
	{
		return false;
	}

	::hw_3d_pushwall_sides_ib_buffer_.clear();
	::hw_3d_pushwall_sides_ib_buffer_.resize(index_count);

	return true;
}

void hw_3d_uninitialize_pushwalls_ib()
{
	if (::hw_3d_pushwall_sides_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_pushwall_sides_ib_);
		::hw_3d_pushwall_sides_ib_ = nullptr;
	}

	::hw_3d_pushwall_sides_ib_buffer_.clear();
}

bool hw_initialize_pushwalls_vb()
{
	auto param = bstone::RendererVertexBufferCreateParam{};
	param.vertex_count_ = ::hw_3d_pushwall_side_count_ * ::hw_3d_max_vertices_per_wall_side;

	::hw_3d_pushwall_sides_vb_ = ::hw_renderer_->vertex_buffer_create(param);

	if (!::hw_3d_pushwall_sides_vb_)
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_pushwalls_vb()
{
	if (::hw_3d_pushwall_sides_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_pushwall_sides_vb_);
		::hw_3d_pushwall_sides_vb_ = nullptr;
	}
}

bool hw_3d_initialize_pushwalls()
{
	::hw_3d_xy_pushwall_map_.reserve(::hw_3d_pushwall_count_);

	::hw_3d_pushwall_side_draw_item_count_ = 0;
	::hw_3d_pushwall_side_draw_items_.clear();
	::hw_3d_pushwall_side_draw_items_.resize(::hw_3d_pushwall_side_count_);

	if (!::hw_initialize_pushwalls_ib())
	{
		return false;
	}

	if (!::hw_initialize_pushwalls_vb())
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_pushwalls()
{
	::hw_3d_pushwall_count_ = 0;
	::hw_3d_pushwall_side_count_ = 0;
	::hw_3d_xy_pushwall_map_.clear();

	::hw_3d_pushwall_side_draw_item_count_ = 0;
	::hw_3d_pushwall_side_draw_items_.clear();

	::hw_3d_uninitialize_pushwalls_ib();
	::hw_3d_uninitialize_pushwalls_vb();
}

bool hw_initialize_door_sides_ib()
{
	const auto index_count = ::hw_3d_door_count_ * ::hw_3d_max_indices_per_door_side;

	auto param = bstone::RendererIndexBufferCreateParam{};
	param.index_count_ = index_count;

	::hw_3d_door_sides_ib_ = ::hw_renderer_->index_buffer_create(param);

	if (!::hw_3d_door_sides_ib_)
	{
		return false;
	}

	::hw_3d_door_sides_ib_buffer_.clear();
	::hw_3d_door_sides_ib_buffer_.resize(index_count);

	return true;
}

void hw_3d_uninitialize_door_sides_ib()
{
	if (::hw_3d_door_sides_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_door_sides_ib_);
		::hw_3d_door_sides_ib_ = nullptr;
	}

	::hw_3d_door_sides_ib_buffer_.clear();
}

bool hw_initialize_door_sides_vb()
{
	auto param = bstone::RendererVertexBufferCreateParam{};
	param.vertex_count_ = ::hw_3d_door_count_ * ::hw_3d_max_indices_per_door_side;

	::hw_3d_door_sides_vb_ = ::hw_renderer_->vertex_buffer_create(param);

	if (!::hw_3d_door_sides_vb_)
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_door_sides_vb()
{
	if (::hw_3d_door_sides_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_door_sides_vb_);
		::hw_3d_door_sides_vb_ = nullptr;
	}
}

bool hw_3d_initialize_door_sides()
{
	::hw_3d_xy_door_map_.reserve(::hw_3d_door_count_);

	const auto max_draw_item_count = ::hw_3d_door_count_ * hw_3d_max_halves_per_door;

	::hw_3d_door_draw_item_count_ = 0;
	::hw_3d_door_draw_items_.clear();
	::hw_3d_door_draw_items_.resize(max_draw_item_count);

	if (!::hw_initialize_door_sides_ib())
	{
		return false;
	}

	if (!::hw_initialize_door_sides_vb())
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_door_sides()
{
	::hw_3d_xy_door_map_.clear();

	::hw_3d_door_draw_item_count_ = 0;
	::hw_3d_door_draw_items_.clear();

	::hw_3d_uninitialize_door_sides_ib();
	::hw_3d_uninitialize_door_sides_vb();
}

void hw_update_palette(
	const int first_index,
	const int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = ::vid_vga_palette[color_index];
		auto& hw_color = ::hw_palette_[color_index];

		hw_color = ::hw_vga_color_to_color_32(
			vga_color.r,
			vga_color.g,
			vga_color.b
		);
	}
}
void hw_initialize_palette()
{
	::hw_palette_ = {};

	::hw_update_palette(0, palette_color_count);

	auto default_palette = bstone::RendererPalette{};

	for (int i = 0; i < palette_color_count; ++i)
	{
		const auto vga_color = ::vgapal + (i * 3);
		auto& hw_color = default_palette[i];

		hw_color = ::hw_vga_color_to_color_32(vga_color[0], vga_color[1], vga_color[2]);

	}

	::hw_renderer_->palette_update(default_palette);
}

void hw_calculate_dimensions()
{
	const auto alignment = 2;

	// Decrease by 20% to compensate vanilla VGA stretch.
	::vga_height = (10 * window_height) / 12;
	::vga_height += alignment - 1;
	::vga_height /= alignment;
	::vga_height *= alignment;

	if (::vid_widescreen)
	{
		::vga_width = ::window_width;
	}
	else
	{
		::vga_width = (::vga_ref_width * ::vga_height) / ::vga_ref_height;
	}

	::vga_width += alignment - 1;
	::vga_width /= alignment;
	::vga_width *= alignment;

	::vga_width_scale = static_cast<float>(::vga_width) / static_cast<float>(::vga_ref_width);
	::vga_height_scale = static_cast<float>(::vga_height) / static_cast<float>(::vga_ref_height_4x3);

	::vga_area = ::vga_width * ::vga_height;

	::screen_width = ::vga_width;

	::screen_height = (12 * ::vga_height) / 10;
	::screen_height += alignment - 1;
	::screen_height /= alignment;
	::screen_height *= alignment;

	::filler_width = (::screen_width * ::vga_ref_height_4x3) - (::screen_height * ::vga_ref_width);
	::filler_width /= 2 * ::vga_ref_height_4x3;

#ifdef __vita__
	const auto upper_filler_height = (::screen_height * ref_top_bar_height) / ::vga_ref_height + 1; //todo: double check then just hardcode values
	const auto lower_filler_height = (::screen_height * ref_bottom_bar_height) / ::vga_ref_height + 1;
#else  
	const auto upper_filler_height = (::screen_height * ref_top_bar_height) / ::vga_ref_height;
	const auto lower_filler_height = (::screen_height * ref_bottom_bar_height) / ::vga_ref_height;
#endif
	const auto middle_filler_height = ::screen_height - (upper_filler_height + lower_filler_height);

	// UI whole rect
	//
	::sw_ui_whole_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::vga_ref_height,
	};

	::sw_ui_whole_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		::screen_height,
	};


	// UI top rect
	//
	::sw_ui_top_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::ref_top_bar_height,
	};

	::sw_ui_top_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		upper_filler_height,
	};


	// UI middle rect (stretched to full width)
	//
	::sw_ui_wide_middle_src_rect = SDL_Rect{
		0,
		::ref_view_top_y,
		::vga_ref_width,
		::ref_view_height,
	};

	::sw_ui_wide_middle_dst_rect = SDL_Rect{
		0,
		upper_filler_height,
		::screen_width,
		middle_filler_height,
	};


	// UI bottom rect
	//
	::sw_ui_bottom_src_rect = SDL_Rect{
		0,
		::ref_view_bottom_y + 1,
		::vga_ref_width,
		::ref_bottom_bar_height,
	};

	::sw_ui_bottom_dst_rect = SDL_Rect{
		::filler_width,
		::screen_height - lower_filler_height,
		::screen_width - (2 * ::filler_width),
		lower_filler_height,
	};


	// UI left bar
	::sw_filler_ui_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		::screen_height,
	};

	// UI right bar
	::sw_filler_ui_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		::screen_height,
	};

	// HUD upper left rect
	::sw_filler_hud_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		upper_filler_height
	};

	// HUD upper right rect
	::sw_filler_hud_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		upper_filler_height,
	};

	// HUD lower left rect
	::sw_filler_hud_rects[2] = SDL_Rect{
		0,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	// HUD lower right rect
	::sw_filler_hud_rects[3] = SDL_Rect{
		::screen_width - ::filler_width,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	::sw_filler_color = SDL_Color{
		::vgapal[(filler_color_index * 3) + 0],
		::vgapal[(filler_color_index * 3) + 1],
		::vgapal[(filler_color_index * 3) + 2],
		0xFF,
	};


	::hw_2d_width_4x3_ = (::window_height * ::vga_ref_width) / ::vga_ref_height_4x3;

	::hw_2d_left_filler_width_4x3_ = (::window_width - ::hw_2d_width_4x3_) / 2;
	::hw_2d_right_filler_width_4x3_ = ::window_width - ::hw_2d_width_4x3_ - ::hw_2d_left_filler_width_4x3_;

	::hw_2d_top_filler_height_4x3_ = (::ref_top_bar_height * ::window_height) / ::vga_ref_height_4x3;
	::hw_2d_bottom_filler_height_4x3_ = (::ref_bottom_bar_height * ::window_height) / ::vga_ref_height_4x3;


	::hw_3d_viewport_x_ = 0;
	::hw_3d_viewport_y_ = ((::ref_bottom_bar_height + ::ref_3d_margin) * ::window_height) / ::vga_ref_height;

	::hw_3d_viewport_width_ = ::window_width;
	::hw_3d_viewport_height_ = (::ref_3d_view_height * ::window_height) / ::vga_ref_height;
}

bool hw_initialize_video()
{
	::vid_is_hw_ = false;

	bstone::Log::write("VID: Probing for hardware accelerated renderer...");

	hw_renderer_manager_ = bstone::RendererManagerFactory::create();

	if (!hw_renderer_manager_->initialize())
	{
		bstone::Log::write_warning("VID: Failed to initialize renderer manager.");

		return false;
	}

	if (!hw_renderer_manager_->renderer_probe(bstone::RendererPath::autodetect))
	{
		bstone::Log::write_warning("VID: No renderer path was found.");

		return false;
	}

	bool is_custom_scale = false;

	//
	// Option "vid_windowed"
	//

	::vid_is_windowed = ::g_args.has_option("vid_windowed");

	::vid_use_custom_window_position = false;


	//
	// Option "vid_window_x"
	//

	const auto& vid_window_x_str = ::g_args.get_option_value("vid_window_x");

	if (bstone::StringHelper::string_to_int(vid_window_x_str, ::vid_window_x))
	{
		::vid_use_custom_window_position = true;
	}


	//
	// Option "vid_window_y"
	//

	const auto& vid_window_y_str = ::g_args.get_option_value("vid_window_y");

	if (bstone::StringHelper::string_to_int(vid_window_y_str, ::vid_window_y))
	{
		::vid_use_custom_window_position = true;
	}


	//
	// Option "vid_mode"
	//

	std::string width_str;
	std::string height_str;

	::g_args.get_option_values("vid_mode", width_str, height_str);

	static_cast<void>(bstone::StringHelper::string_to_int(width_str, ::window_width));
	static_cast<void>(bstone::StringHelper::string_to_int(height_str, ::window_height));

	if (::window_width == 0)
	{
		::window_width = ::default_window_width;
	}

	if (::window_height == 0)
	{
		::window_height = ::default_window_height;
	}

	if (::window_width < ::vga_ref_width)
	{
		::window_width = ::vga_ref_width;
	}

	if (::window_height < ::vga_ref_height_4x3)
	{
		::window_height = ::vga_ref_height_4x3;
	}


	//
	// Option "vid_scale"
	//

	const auto& vid_scale_str = ::g_args.get_option_value("vid_scale");

	if (!vid_scale_str.empty())
	{
		int scale_value;

		if (bstone::StringHelper::string_to_int(vid_scale_str, scale_value))
		{
			is_custom_scale = true;

			if (scale_value < 1)
			{
				scale_value = 1;
			}

			::vga_scale = scale_value;
		}
	}


	int sdl_result = 0;

	sdl_result = ::SDL_GetDesktopDisplayMode(0, &::display_mode);

	if (sdl_result != 0)
	{
		//::Quit("VID: Failed to get a display mode.");
		return false;
	}

	if (!::vid_is_windowed)
	{
		::window_width = ::display_mode.w;
		::window_height = ::display_mode.h;
	}


	::hw_calculate_dimensions();

	bool is_succeed = true;

	if (is_succeed)
	{
		is_succeed = ::hw_initialize_renderer();
	}

	if (is_succeed)
	{
		::hw_initialize_vga_buffer();
		::hw_initialize_ui_buffer();
	}

	if (is_succeed)
	{
		::hw_initialize_palette();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_initialize_textures();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_3d_initialize_flooring();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_3d_initialize_ceiling();
	}

	if (is_succeed)
	{
		::hw_texture_manager_ = bstone::RendererTextureManagerFactory::create(
			::hw_renderer_,
			&::vid_sprite_cache
		);

		if (!::hw_texture_manager_->is_initialized())
		{
			is_succeed = false;
		}
	}

	if (is_succeed)
	{
		::hw_renderer_->set_2d_projection_matrix(::window_width, ::window_height);

		::hw_renderer_->set_3d_projection_matrix(
			::hw_3d_viewport_width_,
			::hw_3d_viewport_height_,
			45,
			0.05F,
			100.0F
		);

		::hw_command_sets_.resize(2);

		// 2D
		//
		::hw_2d_command_set_ = &::hw_command_sets_[1];

		::hw_2d_command_set_->count_ = 0;
		::hw_2d_command_set_->commands_.resize(::hw_min_2d_commands);

		// 3D
		//
		::hw_3d_command_set_ = &::hw_command_sets_[0];

		::hw_3d_command_set_->count_ = 0;
		::hw_3d_command_set_->commands_.resize(::hw_min_3d_commands);
	}

	if (is_succeed)
	{
		::vid_is_hw_ = true;

		hw_renderer_->color_buffer_set_clear_color(bstone::RendererColor32{});

		hw_renderer_->window_show(true);

		::in_grab_mouse(true);
	}

	return is_succeed;
}

void hw_uninitialize_ui_texture()
{
	if (::hw_2d_texture_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_2d_texture_);
		::hw_2d_texture_ = nullptr;
	}

	if (::hw_2d_black_t2d_1x1_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_2d_black_t2d_1x1_);
		::hw_2d_black_t2d_1x1_ = nullptr;
	}

	if (::hw_2d_white_t2d_1x1_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_2d_white_t2d_1x1_);
		::hw_2d_white_t2d_1x1_ = nullptr;
	}

	if (::hw_2d_fade_t2d_)
	{
		::hw_renderer_->texture_2d_destroy(::hw_2d_fade_t2d_);
		::hw_2d_fade_t2d_ = nullptr;
	}

	if (::hw_2d_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_2d_ib_);
		::hw_2d_ib_ = nullptr;
	}

	if (::hw_2d_fillers_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_2d_fillers_ib_);
		::hw_2d_fillers_ib_ = nullptr;
	}

	if (::hw_2d_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_2d_vb_);
		::hw_2d_vb_ = nullptr;
	}

	if (::hw_2d_fillers_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_2d_fillers_vb_);
		::hw_2d_fillers_vb_ = nullptr;
	}
}

void hw_uninitialize_vga_buffer()
{
	::sw_vga_buffer.clear();
	::sw_vga_buffer.shrink_to_fit();

	::vga_memory = nullptr;
}

void hw_refresh_screen_2d()
{
	// Update 2D texture.
	//
	{
		auto param = bstone::RendererTexture2dUpdateParam{};
		param.indexed_pixels_ = ::vid_ui_buffer.data();
		param.indexed_palette_ = &::hw_palette_;
		param.indexed_alphas_ = nullptr;

		::hw_2d_texture_->update(param);
	}

	// Update fade color.
	//
	if (::hw_2d_fade_is_enabled_)
	{
		auto param = bstone::RendererTexture2dUpdateParam{};
		param.rgba_pixels_ = &::hw_2d_fade_color_;

		::hw_2d_fade_t2d_->update(param);
	}


	// Build commands.
	//
	auto command_index = 0;
	auto& commands = ::hw_2d_command_set_->commands_;


	// Enable 2D.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::set_2d;
	}

	// Set viewport.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::viewport_set;

		auto& viewport = command.viewport_set_;
		viewport.x_ = 0;
		viewport.y_ = 0;
		viewport.width_ = ::window_width;
		viewport.height_ = ::window_height;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 0.0F;
	}

	// Disable back-face culling.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::culling_set;
		command.culling_set_.is_enabled_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::depth_set_test;
		command.depth_set_test_.is_enabled_ = false;
	}

	// Fillers.
	//
	if (!::vid_is_ui_stretched)
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::draw_quads;

		auto count = 0;
		auto index_offset = 0;
		auto texture_2d = bstone::RendererTexture2dPtr{};

		if (::vid_is_hud)
		{
			count = ::hw_2d_fillers_hud_quad_count;
			index_offset = ::hw_2d_fillers_hud_index_offset_;
		}
		else
		{
			count = ::hw_2d_fillers_ui_quad_count;
			index_offset = ::hw_2d_fillers_ui_index_offset_;
		}

		if (::vid_is_movie)
		{
			texture_2d = ::hw_2d_white_t2d_1x1_;
		}
		else
		{
			texture_2d = ::hw_2d_black_t2d_1x1_;
		}

		auto& draw_quads = command.draw_quads_;
		draw_quads.count_ = count;
		draw_quads.index_buffer_ = ::hw_2d_fillers_ib_;
		draw_quads.index_offset_ = index_offset;
		draw_quads.texture_2d_ = texture_2d;
		draw_quads.vertex_buffer_ = ::hw_2d_fillers_vb_;
	}

	// Draw 2D (UI, menu, etc.).
	//
	{
		if (::vid_is_hud)
		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::blending_set;

			auto& blending_set = command.blending_set_;
			blending_set.is_enabled_ = true;
		}

		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			const auto index_offset = (::vid_is_ui_stretched
				?
				::hw_2d_stretched_index_offset_
				:
				::hw_2d_non_stretched_index_offset_
			);

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = 1;
			draw_quads.index_buffer_ = ::hw_2d_ib_;
			draw_quads.index_offset_ = index_offset;
			draw_quads.texture_2d_ = ::hw_2d_texture_;
			draw_quads.vertex_buffer_ = ::hw_2d_vb_;
		}

		if (::vid_is_hud)
		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::blending_set;

			auto& blending_set = command.blending_set_;
			blending_set.is_enabled_ = false;
		}
	}

	// 2D fade in or out.
	//
	if (::hw_2d_fade_is_enabled_)
	{
		// Enable blending.
		//
		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::blending_set;

			auto& blending_set = command.blending_set_;
			blending_set.is_enabled_ = true;
		}

		// Draw the quad.
		//
		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			const auto index_offset = (::vid_is_ui_stretched
				?
				::hw_2d_stretched_index_offset_
				:
				::hw_2d_non_stretched_index_offset_
			);

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = 1;
			draw_quads.index_buffer_ = ::hw_2d_ib_;
			draw_quads.index_offset_ = index_offset;
			draw_quads.texture_2d_ = ::hw_2d_fade_t2d_;
			draw_quads.vertex_buffer_ = ::hw_2d_vb_;
		}

		// Disable blending.
		//
		{
			auto& command = commands[command_index++];
			command.id_ = bstone::RendererCommandId::blending_set;

			auto& blending_set = command.blending_set_;
			blending_set.is_enabled_ = false;
		}
	}

	// Commit commands.
	//
	::hw_2d_command_set_->count_ = command_index;
}

bool hw_3d_dbg_is_tile_vertex_visible(
	const int x,
	const int y)
{
	const auto& wall_direction = bstone::Vec2D
	{
		::hw_3d_player_position[0] - static_cast<double>(x),
		::hw_3d_player_position[1] - static_cast<double>(y)
	};

	const auto cosine_between_directions = bstone::Vec2D::dot_product(
		wall_direction, ::hw_3d_player_direction);

	if (cosine_between_directions >= 0.0)
	{
		return false;
	}

	return true;
}

bool hw_3d_dbg_is_tile_visible(
	const int x,
	const int y)
{
	const auto delta = ::hw_3d_tile_dimension_i;

	if (::hw_3d_dbg_is_tile_vertex_visible(x + 0, y + 0))
	{
		return true;
	}

	if (::hw_3d_dbg_is_tile_vertex_visible(x + delta, y + 0))
	{
		return true;
	}

	if (::hw_3d_dbg_is_tile_vertex_visible(x + delta, y + delta))
	{
		return true;
	}

	if (::hw_3d_dbg_is_tile_vertex_visible(x + 0, y + delta))
	{
		return true;
	}

	return false;
}

void hw_3d_dbg_draw_all_solid_walls(
	int& command_index)
{
	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_wall_side_draw_items_;

	for (const auto& xy_wall_item : ::hw_3d_xy_wall_map_)
	{
		const auto& wall = xy_wall_item.second;

		if (!::hw_3d_dbg_is_tile_visible(wall.x_, wall.y_))
		{
			continue;
		}

		for (const auto& side : wall.sides_)
		{
			if (!side.flags_.is_active_)
			{
				continue;
			}

			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_2d_ = side.texture_2d_;
			draw_item.wall_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_2d_ < rhs.texture_2d_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = ::hw_3d_wall_sides_ib_buffer_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 3);
		}

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.offset_ = 0;
		param.count_ = ib_index;
		param.indices_ = ib_buffer.data();

		::hw_3d_wall_sides_ib_->update(param);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture = bstone::RendererTexture2dPtr{};

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture = draw_items[draw_index].texture_2d_;
			}
			else if (last_texture == draw_items[draw_index].texture_2d_)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			auto& command = ::hw_3d_command_set_->commands_[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = draw_quad_count;
			draw_quads.index_offset_ = draw_index_offset_;
			draw_quads.index_buffer_ = ::hw_3d_wall_sides_ib_;
			draw_quads.vertex_buffer_ = ::hw_3d_wall_sides_vb_;
			draw_quads.texture_2d_ = last_texture;

			draw_index_offset_ += ::hw_3d_max_indices_per_wall_side * draw_quad_count;
		}
	}

	::hw_3d_wall_side_draw_item_count_ = draw_side_index;
}

void hw_3d_dbg_draw_all_pushwalls(
	int& command_index)
{
	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_pushwall_side_draw_items_;

	for (const auto& xy_pushwall_item : ::hw_3d_xy_pushwall_map_)
	{
		const auto& pushwall = xy_pushwall_item.second;

		if (!::hw_3d_dbg_is_tile_visible(pushwall.x_, pushwall.y_))
		{
			continue;
		}

		for (const auto& side : pushwall.sides_)
		{
			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_2d_ = side.texture_2d_;
			draw_item.wall_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_2d_ < rhs.texture_2d_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = ::hw_3d_pushwall_sides_ib_buffer_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(wall_side.vertex_index_ + 3);
		}

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.offset_ = 0;
		param.count_ = ib_index;
		param.indices_ = ib_buffer.data();

		::hw_3d_pushwall_sides_ib_->update(param);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture = bstone::RendererTexture2dPtr{};

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture = draw_items[draw_index].texture_2d_;
			}
			else if (last_texture == draw_items[draw_index].texture_2d_)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			auto& command = ::hw_3d_command_set_->commands_[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = draw_quad_count;
			draw_quads.index_offset_ = draw_index_offset_;
			draw_quads.index_buffer_ = ::hw_3d_pushwall_sides_ib_;
			draw_quads.vertex_buffer_ = ::hw_3d_pushwall_sides_vb_;
			draw_quads.texture_2d_ = last_texture;

			draw_index_offset_ += ::hw_3d_max_indices_per_wall_side * draw_quad_count;
		}
	}

	::hw_3d_pushwall_side_draw_item_count_ = draw_side_index;
}

bool hw_3d_dbg_is_door_vertex_visible(
	const double x,
	const double y)
{
	const auto& wall_direction = bstone::Vec2D
	{
		::hw_3d_player_position[0] - x,
		::hw_3d_player_position[1] - y
	};

	const auto cosine_between_directions = bstone::Vec2D::dot_product(
		wall_direction, ::hw_3d_player_direction);

	if (cosine_between_directions >= 0.0)
	{
		return false;
	}

	return true;
}

bool hw_3d_dbg_is_door_visible(
	const doorobj_t& door)
{
	if (door.vertical)
	{
		const auto x = static_cast<double>(door.tilex) + ::hw_3d_tile_half_dimension_d;

		const auto y_0 = static_cast<double>(door.tiley);

		if (::hw_3d_dbg_is_door_vertex_visible(x, y_0))
		{
			return true;
		}

		const auto y_1 = y_0 + ::hw_3d_tile_dimension_d;

		if (::hw_3d_dbg_is_door_vertex_visible(x, y_1))
		{
			return true;
		}
	}
	else
	{
		const auto y = static_cast<double>(door.tiley) + ::hw_3d_tile_half_dimension_d;

		const auto x_0 = static_cast<double>(door.tilex);

		if (::hw_3d_dbg_is_door_vertex_visible(x_0, y))
		{
			return true;
		}

		const auto x_1 = x_0 + ::hw_3d_tile_dimension_d;

		if (::hw_3d_dbg_is_door_vertex_visible(x_1, y))
		{
			return true;
		}
	}

	return false;
}

void hw_3d_dbg_draw_all_doors(
	int& command_index)
{
	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_door_draw_items_;

	for (const auto& xy_door_item : ::hw_3d_xy_door_map_)
	{
		const auto& door = xy_door_item.second;
		const auto door_index = door.door_ - ::doorobjlist;
		const auto door_position = ::doorposition[door_index];

		if (door_position == 0xFFFF)
		{
			// Skip fully open door.
			continue;
		}

		if (!::hw_3d_dbg_is_door_visible(*door.door_))
		{
			continue;
		}

		for (const auto& side : xy_door_item.second.sides_)
		{
			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_2d_ = side.texture_2d_;
			draw_item.door_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_2d_ < rhs.texture_2d_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = ::hw_3d_door_sides_ib_buffer_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& door_side = *draw_items[i].door_side_;

			for (int i_quad = 0; i_quad < 2; ++i_quad)
			{
				auto base_vertex_index = door_side.vertex_index_ + (4 * i_quad);

				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 0);
				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 1);
				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 2);

				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 0);
				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 2);
				ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(base_vertex_index + 3);
			}
		}

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.offset_ = 0;
		param.count_ = ib_index;
		param.indices_ = ib_buffer.data();

		::hw_3d_door_sides_ib_->update(param);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset = 0;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture = bstone::RendererTexture2dPtr{};

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture = draw_items[draw_index].texture_2d_;
			}
			else if (last_texture == draw_items[draw_index].texture_2d_)
			{
				draw_quad_count += 2;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			auto& command = ::hw_3d_command_set_->commands_[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = draw_quad_count;
			draw_quads.index_offset_ = draw_index_offset;
			draw_quads.index_buffer_ = ::hw_3d_door_sides_ib_;
			draw_quads.vertex_buffer_ = ::hw_3d_door_sides_vb_;
			draw_quads.texture_2d_ = last_texture;

			draw_index_offset += 6 * draw_quad_count;
		}
	}

	::hw_3d_door_draw_item_count_ = draw_side_index;
}

void hw_3d_dbg_draw_all_sprites(
	int& command_index)
{
	// Build draw list.
	//
	auto draw_sprite_index = 0;
	auto& draw_items = ::hw_3d_sprites_draw_list_;

	for (const auto& xy_static_item : ::hw_3d_xy_static_map_)
	{
		const auto& hw_static = xy_static_item.second;

		if (!hw_static.flags_.is_visible_)
		{
			continue;
		}

		auto& draw_item = draw_items[draw_sprite_index++];

		draw_item.texture_2d_ = hw_static.texture_2d_;
		draw_item.sprite_ = &hw_static;
	}

	// Sort by distance (farthest -> nearest).
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_sprite_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.sprite_->square_distance_ > rhs.sprite_->square_distance_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = ::hw_3d_sprites_ib_buffer_;

		for (int i = 0; i < draw_sprite_index; ++i)
		{
			const auto& sprite = *draw_items[i].sprite_;

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<Hw3dWallSideIndexBufferItem>(sprite.vertex_index_ + 3);
		}

		auto param = bstone::RendererIndexBufferUpdateParam{};
		param.offset_ = 0;
		param.count_ = ib_index;
		param.indices_ = ib_buffer.data();

		::hw_3d_sprites_ib_->update(param);
	}

	// Add render commands.
	//

	// Disable depth write.
	//
	{
		auto& command = ::hw_3d_command_set_->commands_[command_index++];
		command.id_ = bstone::RendererCommandId::depth_set_write;
		command.depth_set_write_.is_enabled_ = false;
	}

	// Enable blending.
	//
	{
		auto& command = ::hw_3d_command_set_->commands_[command_index++];
		command.id_ = bstone::RendererCommandId::blending_set;
		command.blending_set_.is_enabled_ = true;
	}

	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;

	while (draw_index < draw_sprite_index)
	{
		auto is_first = true;
		auto last_texture = bstone::RendererTexture2dPtr{};

		draw_quad_count = 0;

		while (draw_index < draw_sprite_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture = draw_items[draw_index].texture_2d_;
			}
			else if (last_texture == draw_items[draw_index].texture_2d_)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			auto& command = ::hw_3d_command_set_->commands_[command_index++];
			command.id_ = bstone::RendererCommandId::draw_quads;

			auto& draw_quads = command.draw_quads_;
			draw_quads.count_ = draw_quad_count;
			draw_quads.index_offset_ = draw_index_offset_;
			draw_quads.index_buffer_ = ::hw_3d_sprites_ib_;
			draw_quads.vertex_buffer_ = ::hw_3d_sprites_vb_;
			draw_quads.texture_2d_ = last_texture;

			draw_index_offset_ += ::hw_3d_indices_per_sprite * draw_quad_count;
		}
	}

	// Enable depth write.
	//
	{
		auto& command = ::hw_3d_command_set_->commands_[command_index++];
		command.id_ = bstone::RendererCommandId::depth_set_write;
		command.depth_set_write_.is_enabled_ = true;
	}

	// Disable blending.
	//
	{
		auto& command = ::hw_3d_command_set_->commands_[command_index++];
		command.id_ = bstone::RendererCommandId::blending_set;
		command.blending_set_.is_enabled_ = false;
	}

	::hw_3d_sprites_draw_count_ = draw_sprite_index;
}

void hw_refresh_screen_3d()
{
	::hw_3d_command_set_->count_ = 0;

	if (!::vid_hw_is_draw_3d_)
	{
		return;
	}


	// Build commands.
	//
	auto command_index = 0;
	auto& commands = ::hw_3d_command_set_->commands_;


	// Enable 3D.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::set_3d;
	}

	// Set viewport.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::viewport_set;

		auto& viewport = command.viewport_set_;
		viewport.x_ = ::hw_3d_viewport_x_;
		viewport.y_ = ::hw_3d_viewport_y_;
		viewport.width_ = ::hw_3d_viewport_width_;
		viewport.height_ = ::hw_3d_viewport_height_;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 1.0F;
	}

	// Enable back-face culling.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::culling_set;
		command.culling_set_.is_enabled_ = true;
	}

	// Enable depth test.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::depth_set_test;
		command.depth_set_test_.is_enabled_ = true;
	}

	// Enable depth write.
	//
	{
		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::depth_set_write;
		command.depth_set_write_.is_enabled_ = true;
	}

	// Draw solid walls.
	//
	::hw_3d_dbg_draw_all_solid_walls(command_index);

	// Draw pushwalls.
	//
	::hw_3d_dbg_draw_all_pushwalls(command_index);

	// Draw doors.
	//
	::hw_3d_dbg_draw_all_doors(command_index);

	// Draw flooring.
	//
	{
		auto texture_2d = ((::gamestate.flags & GS_DRAW_FLOOR) != 0
			?
			::hw_3d_flooring_textured_t2d_
			:
			::hw_3d_flooring_solid_t2d_
		);

		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::draw_quads;

		auto& draw_quads = command.draw_quads_;
		draw_quads.count_ = 1;
		draw_quads.index_offset_ = 0;
		draw_quads.index_buffer_ = ::hw_3d_flooring_ib_;
		draw_quads.vertex_buffer_ = ::hw_3d_flooring_vb_;
		draw_quads.texture_2d_ = texture_2d;
	}

	// Draw ceiling.
	//
	{
		auto texture_2d = ((::gamestate.flags & GS_DRAW_CEILING) != 0
			?
			::hw_3d_ceiling_textured_t2d_
			:
			::hw_3d_ceiling_solid_t2d_
		);

		auto& command = commands[command_index++];
		command.id_ = bstone::RendererCommandId::draw_quads;

		auto& draw_quads = command.draw_quads_;
		draw_quads.count_ = 1;
		draw_quads.index_offset_ = 0;
		draw_quads.index_buffer_ = ::hw_3d_ceiling_ib_;
		draw_quads.vertex_buffer_ = ::hw_3d_ceiling_vb_;
		draw_quads.texture_2d_ = texture_2d;
	}

	// Draw statics.
	//
	::hw_3d_dbg_draw_all_sprites(command_index);

	// Commit commands.
	//
	::hw_3d_command_set_->count_ = command_index;
}

void hw_refresh_screen()
{
	if (!::hw_renderer_)
	{
		return;
	}

	if (::vid_is_hud && ::player)
	{
		::vid_hw_is_draw_3d_ = true;

		::hw_3d_update_player();

		::hw_dbg_3d_orient_all_sprites();

		const auto player_x = bstone::FixedPoint{::player->x}.to_float();
		const auto player_y = bstone::FixedPoint{::player->y}.to_float();
		const auto view_position = bstone::Vec3F{player_x, player_y, 0.5F};

		::hw_renderer_->set_3d_view_matrix(::player->angle, view_position);
	}

	::hw_renderer_->clear_buffers();

	::hw_refresh_screen_3d();
	::hw_refresh_screen_2d();

	::hw_renderer_->execute_command_sets(::hw_command_sets_);
	::hw_renderer_->present();

	::vid_hw_is_draw_3d_ = false;
}

void hw_check_vsync()
{
	using Clock = std::chrono::system_clock;

	constexpr int draw_count = 10;

	constexpr int duration_tolerance_pct = 25;

	const int expected_duration_ms =
		(1000 * draw_count) / ::display_mode.refresh_rate;

	const int min_expected_duration_ms =
		((100 - duration_tolerance_pct) * expected_duration_ms) / 100;

	const auto before_timestamp = Clock::now();

	for (int i = 0; i < draw_count; ++i)
	{
		::hw_refresh_screen();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	::vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}

void hw_update_widescreen()
{
	::hw_uninitialize_vga_buffer();
	::hw_calculate_dimensions();
	::hw_initialize_vga_buffer();
}

void hw_precache_flooring()
{
	if (!::hw_texture_manager_->wall_cache(::FloorTile))
	{
		::Quit("Failed to cache a floor #" + std::to_string(::FloorTile) + ".");
	}

	::hw_3d_flooring_textured_t2d_ = ::hw_texture_manager_->wall_get(::FloorTile);

	const auto vga_index = ::BottomColor & 0xFF;
	const auto vga_color = ::vgapal + (3 * vga_index);

	const auto renderer_color = ::hw_vga_color_to_color_32(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	if (!::hw_update_solid_texture_1x1(renderer_color, ::hw_3d_flooring_solid_t2d_))
	{
		::Quit("Failed to update flooring solid texture.");
	}
}

void hw_precache_ceiling()
{
	if (!::hw_texture_manager_->wall_cache(::CeilingTile))
	{
		::Quit("Failed to cache a ceiling #" + std::to_string(::CeilingTile) + ".");
	}

	::hw_3d_ceiling_textured_t2d_ = ::hw_texture_manager_->wall_get(::CeilingTile);

	const auto vga_index = ::TopColor & 0xFF;
	const auto vga_color = ::vgapal + (3 * vga_index);

	const auto renderer_color = ::hw_vga_color_to_color_32(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	if (!::hw_update_solid_texture_1x1(renderer_color, ::hw_3d_ceiling_solid_t2d_))
	{
		::Quit("Failed to update ceiling solid texture.");
	}
}

constexpr bool hw_tile_is_activated_pushwall(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_flags;
}

constexpr bool hw_tile_is_door(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_flag;
}

constexpr bool hw_tile_is_door_track(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_track_flag;
}

constexpr bool hw_tile_is_solid_wall(
	const int tile)
{
	if (tile == 0)
	{
		return false;
	}

	if (::hw_tile_is_activated_pushwall(tile))
	{
		return false;
	}

	if (::hw_tile_is_door(tile))
	{
		return false;
	}

	return true;
}

constexpr bool hw_tile_is_pushwall(
	const int x,
	const int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return false;
	}

	const auto tile_wall = ::tilemap[x][y];

	if (tile_wall == 0)
	{
		return false;
	}

	if (::hw_tile_is_activated_pushwall(tile_wall))
	{
		return true;
	}

	const auto tile_object = ::mapsegs[1][(MAPSIZE * y) + x];

	if (tile_object == PUSHABLETILE)
	{
		return true;
	}

	return false;
}

constexpr bool hw_tile_is_solid_wall(
	const int x,
	const int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return true;
	}

	const auto tile_wall = ::tilemap[x][y];

	if (!::hw_tile_is_solid_wall(tile_wall))
	{
		return false;
	}

	if (::hw_tile_is_pushwall(x, y))
	{
		return false;
	}

	if (::hw_3d_has_active_pushwall_ &&
		::hw_3d_active_pushwall_next_x_ == x &&
		::hw_3d_active_pushwall_next_y_ == y)
	{
		return false;
	}

	return true;
}

void hw_precache_wall(
	const int wall_id)
{
	if (!::hw_texture_manager_->wall_cache(wall_id))
	{
		::Quit("Failed to cache a wall #" + std::to_string(wall_id) + ".");
	}
}

void hw_precache_horizontal_wall(
	const int tile_wall)
{
	const auto wall_id = ::horizwall[tile_wall];

	::hw_precache_wall(wall_id);
}

void hw_precache_vertical_wall(
	const int tile_wall)
{
	const auto wall_id = ::vertwall[tile_wall];

	::hw_precache_wall(wall_id);
}

void hw_precache_switches()
{
	::hw_precache_horizontal_wall(OFF_SWITCH);
	::hw_precache_vertical_wall(OFF_SWITCH);
	::hw_precache_horizontal_wall(ON_SWITCH);
	::hw_precache_vertical_wall(ON_SWITCH);
}

void hw_precache_door_track(
	const int x,
	const int y)
{
	const auto tile = ::tilemap[x][y];
	const auto tile_wall = tile & ::tilemap_wall_mask;

	const auto& door = ::doorobjlist[tile_wall];

	if (door.tilex != x || door.tiley != y)
	{
		::Quit("Expected a door at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
	}

	auto wall_id = ::door_get_track_texture_id(door);

	::hw_precache_wall(wall_id);
}

void hw_precache_solid_walls()
{
	auto has_switch = false;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			const auto tile = ::tilemap[x][y];
			const auto tile_wall = tile & ::tilemap_wall_mask;

			if (::hw_tile_is_door(tile))
			{
				::hw_precache_door_track(x, y);

				continue;
			}

			if (!::hw_tile_is_solid_wall(x, y))
			{
				continue;
			}

			if (tile_wall == ON_SWITCH || tile_wall == OFF_SWITCH)
			{
				has_switch = true;
			}

			::hw_precache_horizontal_wall(tile_wall);
			::hw_precache_vertical_wall(tile_wall);
		}
	}

	if (has_switch)
	{
		::hw_precache_switches();
	}
}

void hw_precache_pushwalls()
{
	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!::hw_tile_is_pushwall(x, y))
			{
				continue;
			}

			const auto tile = ::tilemap[x][y];
			const auto tile_wall = tile & ::tilemap_wall_mask;

			::hw_precache_horizontal_wall(tile_wall);
			::hw_precache_vertical_wall(tile_wall);
		}
	}
}

void hw_precache_door_side(
	const int page_number)
{
	if (!::hw_texture_manager_->wall_cache(page_number))
	{
		::Quit("Failed to cache a door side #" + std::to_string(page_number) + ".");
	}
}

void hw_precache_door(
	const doorobj_t& door)
{
	auto horizontal_locked_page_number = 0;
	auto horizontal_unlocked_page_number = 0;
	auto vertical_locked_page_number = 0;
	auto vertical_unlocked_page_number = 0;

	::door_get_page_numbers_for_caching(
		door,
		horizontal_locked_page_number,
		horizontal_unlocked_page_number,
		vertical_locked_page_number,
		vertical_unlocked_page_number
	);

	::hw_precache_door_side(horizontal_locked_page_number);
	::hw_precache_door_side(horizontal_unlocked_page_number);
	::hw_precache_door_side(vertical_locked_page_number);
	::hw_precache_door_side(vertical_unlocked_page_number);
}

void hw_precache_doors()
{
	::hw_3d_door_count_ = 0;

	for (auto door = ::doorobjlist; door != ::lastdoorobj; ++door)
	{
		++::hw_3d_door_count_;

		::hw_precache_door(*door);
	}
}

int hw_get_solid_wall_side_count(
	const int x,
	const int y)
{
	assert(x >= 0 && x < MAPSIZE);
	assert(y >= 0 && y < MAPSIZE);
	assert(::hw_tile_is_solid_wall(x, y));

	auto side_count = 4;

	side_count -= ::hw_tile_is_solid_wall(x + 0, y - 1); // north
	side_count -= ::hw_tile_is_solid_wall(x + 1, y + 0); // east
	side_count -= ::hw_tile_is_solid_wall(x + 0, y + 1); // south
	side_count -= ::hw_tile_is_solid_wall(x - 1, y + 0); // west

	return side_count;
}

int hw_tile_get_door_track_wall_id(
	const int x,
	const int y,
	const controldir_t direction)
{
	auto door_x = x;
	auto door_y = y;

	switch (direction)
	{
	case di_north:
		door_y -= 1;
		break;

	case di_east:
		door_x += 1;
		break;

	case di_south:
		door_y += 1;
		break;

	case di_west:
		door_x -= 1;
		break;

	default:
		::Quit("Invalid direction.");
	}

	if (door_x < 0 || door_x >= MAPSIZE || door_y < 0 || door_y >= MAPSIZE)
	{
		return -1;
	}

	const auto door_tile = ::tilemap[door_x][door_y];

	if (!::hw_tile_is_door(door_tile))
	{
		return -1;
	}

	const auto door_index = door_tile & ::tilemap_wall_mask;
	const auto& door = ::doorobjlist[door_index];

	return ::door_get_track_texture_id(door);
}

void hw_3d_map_wall_side(
	const controldir_t side_direction,
	Hw3dWall& wall,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{::hw_3d_tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, 0.0F,},
		{0.0F, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, ::hw_3d_tile_dimension_f,},
	};


	const auto x = wall.x_;
	const auto y = wall.y_;

	const auto tile = ::tilemap[x][y];
	const auto wall_id = tile & ::tilemap_wall_mask;
	const auto has_door_tracks = ::hw_tile_is_door_track(tile);

	auto is_vertical = false;
	auto wall_texture_id = 0;

	switch (side_direction)
	{
	case di_north:
	case di_south:
		wall_texture_id = ::horizwall[wall_id];
		break;

	case di_east:
	case di_west:
		is_vertical = true;
		wall_texture_id = ::vertwall[wall_id];
		break;

	default:
		::Quit("Invalid direction.");
	}

	auto is_door_track = false;

	if (has_door_tracks)
	{
		const auto door_track_wall_id = ::hw_tile_get_door_track_wall_id(x, y, side_direction);

		if (door_track_wall_id >= 0)
		{
			is_door_track = true;

			wall_texture_id = door_track_wall_id;
		}
	}

	const auto& vertex_offsets = all_vertex_offsets[side_direction];

	auto& side = wall.sides_[side_direction];

	side.flags_.is_active_ = true;
	side.flags_.is_vertical_ = is_vertical;
	side.flags_.is_door_track_ = is_door_track;
	side.vertex_index_ = vertex_index;
	side.texture_2d_ = ::hw_texture_manager_->wall_get(wall_texture_id);
	side.wall_ = &wall;

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			0.0F,
		};

		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			0.0F,
		};

		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			::hw_3d_map_height_f,
		};

		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			::hw_3d_map_height_f,
		};

		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}
}

void hw_3d_map_xy_to_xwall(
	const Hw3dXyWallKind wall_kind,
	const int x,
	const int y,
	Hw3dXyWallMap& map,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	switch (wall_kind)
	{
	case Hw3dXyWallKind::solid:
	case Hw3dXyWallKind::push:
		break;

	default:
		::Quit("Invalid kind.");
		break;
	}

	auto is_north_solid = false;
	auto is_east_solid = false;
	auto is_south_solid = false;
	auto is_west_solid = false;

	if (wall_kind == Hw3dXyWallKind::solid)
	{
		is_north_solid = ::hw_tile_is_solid_wall(x + 0, y - 1);
		is_east_solid = ::hw_tile_is_solid_wall(x + 1, y + 0);
		is_south_solid = ::hw_tile_is_solid_wall(x + 0, y + 1);
		is_west_solid = ::hw_tile_is_solid_wall(x - 1, y + 0);
	}

	if (is_north_solid && is_east_solid && is_south_solid && is_west_solid)
	{
		// Nothing to draw.
		// This solid wall is surrounded by other solid ones.

		return;
	}

	const auto xy = ::hw_encode_xy(x, y);

	if (map.find(xy) != map.cend())
	{
		::Quit("Wall mapping already exist.");
	}

	map[xy] = Hw3dWall{};
	auto& wall = map[xy];

	wall.x_ = x;
	wall.y_ = y;

	// A north side.
	if (!is_north_solid)
	{
		::hw_3d_map_wall_side(
			di_north,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An east side.
	if (!is_east_solid)
	{
		::hw_3d_map_wall_side(
			di_east,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An south side.
	if (!is_south_solid)
	{
		::hw_3d_map_wall_side(
			di_south,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// A west side.
	if (!is_west_solid)
	{
		::hw_3d_map_wall_side(
			di_west,
			wall,
			vertex_index,
			vb_buffer
		);
	}
}

void hw_3d_build_solid_walls()
{
	::hw_3d_uninitialize_solid_walls();

	// Check for moving pushwall.
	//
	::hw_3d_has_active_pushwall_ = (::pwallstate != 0);

	::hw_3d_active_pushwall_next_x_ = 0;
	::hw_3d_active_pushwall_next_y_ = 0;

	if (::hw_3d_has_active_pushwall_)
	{
		::hw_3d_active_pushwall_next_x_ = ::pwallx;
		::hw_3d_active_pushwall_next_y_ = ::pwally;

		switch (::pwalldir)
		{
		case di_north:
			--::hw_3d_active_pushwall_next_y_;
			break;

		case di_east:
			++::hw_3d_active_pushwall_next_x_;
			break;

		case di_south:
			++::hw_3d_active_pushwall_next_y_;
			break;

		case di_west:
			--::hw_3d_active_pushwall_next_x_;
			break;

		default:
			::Quit("Invalid direction.");
			break;
		}
	}

	// Count walls and their sides.
	//
	::hw_3d_wall_count_ = 0;
	::hw_3d_wall_side_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!::hw_tile_is_solid_wall(x, y))
			{
				continue;
			}

			if (::hw_3d_has_active_pushwall_ &&
				x == ::hw_3d_active_pushwall_next_x_ &&
				y == ::hw_3d_active_pushwall_next_y_)
			{
				continue;
			}

			::hw_3d_wall_count_ += 1;
			::hw_3d_wall_side_count_ += ::hw_get_solid_wall_side_count(x, y);
		}
	}

	// Check for maximums.
	//
	const auto index_count = ::hw_3d_wall_side_count_ * ::hw_3d_max_indices_per_wall_side;

	if (index_count > ::hw_3d_max_wall_sides_indices)
	{
		::Quit("Too many indices.");
	}

	// Create index an vertex buffers.
	//
	if (!::hw_3d_initialize_solid_walls())
	{
		::Quit("Failed to initialize walls.");
	}

	// Build the map (XY to wall).
	//
	const auto vertex_count = ::hw_3d_wall_side_count_ * ::hw_3d_max_vertices_per_wall_side;

	auto vb_buffer = HwVbBuffer{};
	vb_buffer.resize(vertex_count);

	::hw_3d_xy_wall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!::hw_tile_is_solid_wall(x, y))
			{
				continue;
			}

			if (::hw_3d_has_active_pushwall_ &&
				x == ::hw_3d_active_pushwall_next_x_ &&
				y == ::hw_3d_active_pushwall_next_y_)
			{
				continue;
			}

			::hw_3d_map_xy_to_xwall(
				Hw3dXyWallKind::solid,
				x,
				y,
				::hw_3d_xy_wall_map_,
				vertex_index,
				vb_buffer
			);
		}
	}

	// Update vertex buffer.
	//
	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = 0;
	param.count_ = vertex_count;
	param.vertices_ = vb_buffer.data();

	::hw_3d_wall_sides_vb_->update(param);
}

void hw_3d_translate_pushwall_side(
	const float translate_x,
	const float translate_y,
	const controldir_t side_direction,
	const Hw3dWall& wall,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{::hw_3d_tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, 0.0F,},
		{0.0F, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f, ::hw_3d_tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, ::hw_3d_tile_dimension_f,},
	};


	auto is_vertical = false;

	switch (side_direction)
	{
	case di_north:
	case di_south:
		break;

	case di_east:
	case di_west:
		is_vertical = true;
		break;

	default:
		::Quit("Invalid direction.");
	}

	const auto& vertex_offsets = all_vertex_offsets[side_direction];

	auto& side = wall.sides_[side_direction];

	const auto x_f = static_cast<float>(wall.x_) + translate_x;
	const auto y_f = static_cast<float>(wall.y_) + translate_y;

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			0.0F,
		};
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			0.0F,
		};
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			::hw_3d_map_height_f,
		};
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			::hw_3d_map_height_f,
		};
	}
}

void hw_3d_translate_pushwall(
	const Hw3dWall& wall,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	auto translate_distance = static_cast<float>(::pwallpos) / 63.0F;

	auto translate_x = 0.0F;
	auto translate_y = 0.0F;

	switch (::pwalldir)
	{
	case di_north:
		translate_y = -translate_distance;
		break;

	case di_east:
		translate_x = +translate_distance;
		break;

	case di_south:
		translate_y = +translate_distance;
		break;

	case di_west:
		translate_x = -translate_distance;
		break;

	default:
		::Quit("Invalid direction.");
		break;
	}

	::hw_3d_translate_pushwall_side(
		translate_x,
		translate_y,
		di_north,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_translate_pushwall_side(
		translate_x,
		translate_y,
		di_east,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_translate_pushwall_side(
		translate_x,
		translate_y,
		di_south,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_translate_pushwall_side(
		translate_x,
		translate_y,
		di_west,
		wall,
		vertex_index,
		vb_buffer
	);
}

void hw_3d_translate_pushwall()
{
	const auto xy = ::hw_encode_xy(::pwallx, ::pwally);

	const auto wall_item_it = ::hw_3d_xy_pushwall_map_.find(xy);

	if (wall_item_it == ::hw_3d_xy_pushwall_map_.cend())
	{
		::Quit("Pushwall mapping not found.");
	}

	const auto& wall = wall_item_it->second;

	const auto first_vertex_index = wall.sides_.front().vertex_index_;

	auto vertex_index = first_vertex_index;

	::hw_3d_translate_pushwall(wall, vertex_index, ::hw_3d_pushwalls_vb_buffer_);

	const auto vertex_count = vertex_index - first_vertex_index;

	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = first_vertex_index;
	param.count_ = vertex_count;
	param.vertices_ = ::hw_3d_pushwalls_vb_buffer_.data() + first_vertex_index;

	::hw_3d_pushwall_sides_vb_->update(param);
}

void hw_3d_step_pushwall(
	const int old_x,
	const int old_y)
{
	const auto old_xy = ::hw_encode_xy(old_x, old_y);

	const auto old_wall_item_it = ::hw_3d_xy_pushwall_map_.find(old_xy);

	if (old_wall_item_it == ::hw_3d_xy_pushwall_map_.cend())
	{
		::Quit("Pushwall mapping not found.");
	}

	const auto new_xy = ::hw_encode_xy(::pwallx, ::pwally);

	auto wall = old_wall_item_it->second;
	wall.x_ = ::pwallx;
	wall.y_ = ::pwally;

	static_cast<void>(::hw_3d_xy_pushwall_map_.erase(old_xy));
	::hw_3d_xy_pushwall_map_[new_xy] = wall;

	::hw_3d_translate_pushwall();
}

void hw_3d_build_pushwalls()
{
	::hw_3d_uninitialize_pushwalls();

	// Count pushwalls and their sides.
	//
	::hw_3d_pushwall_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!::hw_tile_is_pushwall(x, y))
			{
				continue;
			}

			::hw_3d_pushwall_count_ += 1;
		}
	}

	::hw_3d_pushwall_side_count_ = ::hw_3d_pushwall_count_ * ::hw_3d_max_sides_per_wall;

	// Check for maximums.
	//
	const auto index_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_max_indices_per_wall_side;

	if (index_count > ::hw_3d_max_wall_sides_indices)
	{
		::Quit("Too many indices.");
	}

	// Create index an vertex buffers.
	//
	if (!::hw_3d_initialize_pushwalls())
	{
		::Quit("Failed to initialize pushwalls.");
	}

	// Build the map (XY to pushwall).
	//
	const auto vertex_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_max_vertices_per_wall_side;

	::hw_3d_pushwalls_vb_buffer_.clear();
	::hw_3d_pushwalls_vb_buffer_.resize(vertex_count);

	::hw_3d_xy_pushwall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!::hw_tile_is_pushwall(x, y))
			{
				continue;
			}

			::hw_3d_map_xy_to_xwall(
				Hw3dXyWallKind::push,
				x,
				y,
				::hw_3d_xy_pushwall_map_,
				vertex_index,
				::hw_3d_pushwalls_vb_buffer_
			);
		}
	}

	// Update vertex buffer.
	//
	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = 0;
	param.count_ = vertex_count;
	param.vertices_ = ::hw_3d_pushwalls_vb_buffer_.data();

	::hw_3d_pushwall_sides_vb_->update(param);
}

void hw_3d_update_quad_vertices(
	const Hw3dQuadFlags flags,
	const bstone::Vec3F& origin,
	const bstone::Vec2F& size,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	//
	// Front face order:
	//    bottom-left -> bottom-right -> top-right -> top-left
	//
	// Back face order:
	//    bottom-right -> bottom-left -> top-left -> top-right
	//

	using VertexOrder = std::array<int, 4>;

	const auto axis_index = (flags.is_vertical_ ? 1 : 0);

	for (int i = 0; i < 4; ++i)
	{
		auto& xyz = vb_buffer[vertex_index + i].xyz_;

		xyz = origin;

		switch (i)
		{
		case 0:
			if (flags.is_back_face_)
			{
				xyz[axis_index] += size[0];
			}

			break;

		case 1:
			if (!flags.is_back_face_)
			{
				xyz[axis_index] += size[0];
			}

			break;

		case 2:
			if (!flags.is_back_face_)
			{
				xyz[axis_index] += size[0];
			}

			xyz[2] = 1.0F;

			break;

		case 3:
			if (flags.is_back_face_)
			{
				xyz[axis_index] += size[0];
			}

			xyz[2] = 1.0F;

			break;
		}
	}

	vertex_index += 4;
}

void hw_3d_map_door_side(
	Hw3dDoorSide& door_side,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	const auto& door = *door_side.door_;
	const auto& bs_door = *door.door_;
	const auto door_index = door.door_ - ::doorobjlist;
	const auto door_offset = (0.5F * static_cast<float>(::doorposition[door_index])) / 65'535.0F;

	door_side.vertex_index_ = vertex_index;

	auto flags = Hw3dQuadFlags{};
	flags.is_back_face_ = door_side.is_back_face_;
	flags.is_vertical_ = bs_door.vertical;

	const auto origin_axis_index = (flags.is_vertical_ ? 1 : 0);

	const auto size = bstone::Vec2F{0.5F, 1.0F};

	const auto tile_center = bstone::Vec3F{bs_door.tilex + 0.5F, bs_door.tiley + 0.5F, 0.0F};

	const auto left_offset = -(0.5F + door_offset);
	const auto right_offset = door_offset;

	for (int i = 0; i < 2; ++i)
	{
		auto origin = tile_center;
		auto offset = 0.0F;

		if (i == 0)
		{
			if (flags.is_back_face_)
			{
				offset = right_offset;
			}
			else
			{
				offset = left_offset;
			}
		}
		else
		{
			if (flags.is_back_face_)
			{
				offset = left_offset;
			}
			else
			{
				offset = right_offset;
			}
		}

		origin[origin_axis_index] += offset;

		::hw_3d_update_quad_vertices(
			flags,
			origin,
			size,
			vertex_index,
			vb_buffer
		);
	}
}

void hw_3d_map_xy_to_door(
	const doorobj_t& door,
	int& vertex_index,
	HwVbBuffer& vb_buffer)
{
	const auto xy = ::hw_encode_xy(door.tilex, door.tiley);

	const auto map_it = ::hw_3d_xy_door_map_.find(xy);

	if (map_it != ::hw_3d_xy_door_map_.cend())
	{
		::Quit("XY map to door already exists.");
	}

	::hw_3d_xy_door_map_[xy] = Hw3dDoor{};
	auto& hw_door = ::hw_3d_xy_door_map_[xy];

	hw_door.door_ = &door;

	auto old_vertex_index = vertex_index;

	for (int i = 0; i < ::hw_3d_max_halves_per_door; ++i)
	{
		auto u_0 = 0.0F;
		auto u_1 = 0.0F;

		switch (i)
		{
		case 0:
		case 2:
			u_0 = 0.0F;
			u_1 = 0.5F;
			break;

		case 1:
		case 3:
			u_0 = 0.5F;
			u_1 = 1.0F;
			break;
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{u_0, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{u_1, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{u_1, 1.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
			vertex.uv_ = bstone::Vec2F{u_0, 1.0F};
		}
	}

	vertex_index = old_vertex_index;


	auto is_back_face = false;

	for (auto& hw_door_side : hw_door.sides_)
	{
		hw_door_side.door_ = &hw_door;
		hw_door_side.is_back_face_ = is_back_face;

		::hw_3d_map_door_side(hw_door_side, vertex_index, vb_buffer);

		is_back_face = !is_back_face;
	}

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	::door_get_page_numbers(door, front_face_page_number, back_face_page_number);

	const auto front_face_texture_2d = ::hw_texture_manager_->wall_get(front_face_page_number);
	const auto back_face_texture_2d = ::hw_texture_manager_->wall_get(back_face_page_number);

	assert(front_face_texture_2d);
	assert(back_face_texture_2d);

	hw_door.sides_[0].texture_2d_ = front_face_texture_2d;
	hw_door.sides_[1].texture_2d_ = back_face_texture_2d;
}

void hw_3d_build_doors()
{
	::hw_3d_uninitialize_door_sides();

	// Create index an vertex buffers.
	//
	if (!::hw_3d_initialize_door_sides())
	{
		::Quit("Failed to initialize door sides.");
	}

	// Build the map (XY to door).
	//
	const auto vertex_count = ::hw_3d_vertices_per_door_sides * ::hw_3d_door_count_;

	::hw_3d_doors_vb_buffer_.clear();
	::hw_3d_doors_vb_buffer_.resize(vertex_count);

	::hw_3d_xy_door_map_.clear();

	auto vertex_index = 0;

	for (auto door = ::doorobjlist; door != ::lastdoorobj; ++door)
	{
		::hw_3d_map_xy_to_door(
			*door,
			vertex_index,
			::hw_3d_doors_vb_buffer_
		);
	}

	// Update vertex buffer.
	//
	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = 0;
	param.count_ = vertex_count;
	param.vertices_ = ::hw_3d_doors_vb_buffer_.data();

	::hw_3d_door_sides_vb_->update(param);
}

bool hw_initialize_sprites_ib()
{
	const auto index_count = ::hw_3d_max_sprites_indices;

	auto param = bstone::RendererIndexBufferCreateParam{};
	param.index_count_ = index_count;

	::hw_3d_sprites_ib_ = ::hw_renderer_->index_buffer_create(param);

	if (!::hw_3d_sprites_ib_)
	{
		return false;
	}

	::hw_3d_sprites_ib_buffer_.clear();
	::hw_3d_sprites_ib_buffer_.resize(index_count);

	return true;
}

void hw_3d_uninitialize_sprites_ib()
{
	if (::hw_3d_sprites_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_sprites_ib_);
		::hw_3d_sprites_ib_ = nullptr;
	}

	::hw_3d_sprites_ib_buffer_.clear();
}

bool hw_initialize_sprites_vb()
{
	const auto vertex_count = ::hw_3d_max_sprites_vertices;

	auto param = bstone::RendererVertexBufferCreateParam{};
	param.vertex_count_ = vertex_count;

	::hw_3d_sprites_vb_ = ::hw_renderer_->vertex_buffer_create(param);

	if (!::hw_3d_sprites_vb_)
	{
		return false;
	}

	::hw_3d_sprites_vb_buffer_.resize(vertex_count);

	return true;
}

void hw_3d_uninitialize_sprites_vb()
{
	if (::hw_3d_sprites_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_sprites_vb_);
		::hw_3d_sprites_vb_ = nullptr;
	}

	::hw_3d_sprites_vb_buffer_.clear();
}

bool hw_3d_initialize_statics()
{
	::hw_3d_xy_static_map_.reserve(MAXSTATS);

	return true;
}

bool hw_3d_initialize_sprites()
{
	::hw_3d_sprites_draw_count_ = 0;
	::hw_3d_sprites_draw_list_.clear();
	::hw_3d_sprites_draw_list_.resize(::hw_3d_max_sprites);

	if (!::hw_initialize_sprites_ib())
	{
		return false;
	}

	if (!::hw_initialize_sprites_vb())
	{
		return false;
	}

	if (!::hw_3d_initialize_statics())
	{
		return false;
	}

	return true;
}

void hw_3d_uninitialize_statics()
{
	::hw_3d_xy_static_map_.clear();
}

void hw_3d_uninitialize_sprites()
{
	::hw_3d_uninitialize_statics();

	::hw_3d_sprites_draw_count_ = 0;
	::hw_3d_sprites_draw_list_.clear();

	::hw_3d_uninitialize_sprites_ib();
	::hw_3d_uninitialize_sprites_vb();
}

void hw_3d_orient_sprite(
	Hw3dSprite& sprite)
{
	const auto sprite_origin = bstone::Vec2D
	{
		static_cast<double>(sprite.x_) + 0.5,
		static_cast<double>(sprite.y_) + 0.5
	};

	auto direction = ::hw_3d_player_position - sprite_origin;

	const auto cosinus_between_directions = bstone::Vec2D::dot_product(
		::hw_3d_player_direction,
		direction
	);

	if (cosinus_between_directions >= 0.0)
	{
		sprite.flags_.is_visible_ = false;

		return;
	}
	else
	{
		sprite.flags_.is_visible_ = true;
	}


	auto bottom_left_vertex = sprite_origin;
	auto bottom_right_vertex = sprite_origin;
	auto r_distance = 1.0;

	const auto square_distance = direction.get_square_magnitude();

	sprite.square_distance_ = square_distance;

	constexpr auto min_square_distance = 1.0E-3;

	if (square_distance >= min_square_distance)
	{
		// Orient the sprite along a direction between the sprite and the player.
		//
		r_distance /= std::sqrt(square_distance);
	}
	else
	{
		// Orient the sprite along the player's line of sight (inverted).
		//
		direction[0] = -::hw_3d_player_direction[0];
		direction[1] = -::hw_3d_player_direction[1];
	}

	const auto perpendicular_dx = (::hw_3d_tile_half_dimension_d * direction[1]) * r_distance;
	const auto perpendicular_dy = (::hw_3d_tile_half_dimension_d * direction[0]) * r_distance;

	bottom_left_vertex[0] += -perpendicular_dx;
	bottom_left_vertex[1] += +perpendicular_dy;

	bottom_right_vertex[0] += +perpendicular_dx;
	bottom_right_vertex[1] += -perpendicular_dy;


	auto vertex_index = sprite.vertex_index_;

	// Bottom-left.
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			0.0F
		};
	}

	// Bottom-right.
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			0.0F
		};
	}

	// Top-right.
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			::hw_3d_tile_dimension_f
		};
	}

	// Top-left.
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];

		vertex.xyz_ = bstone::Vec3F
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			::hw_3d_tile_dimension_f
		};
	}
}

void hw_dbg_3d_orient_all_sprites()
{
	for (auto& map_item : ::hw_3d_xy_static_map_)
	{
		auto& sprite = map_item.second;

		::hw_3d_orient_sprite(sprite);
	}

	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = 0;
	param.count_ = ::hw_3d_max_sprites_vertices;
	param.vertices_ = ::hw_3d_sprites_vb_buffer_.data();

	::hw_3d_sprites_vb_->update(param);
}

Hw3dSprite& hw_3d_map_sprite(
	const Hw3dSpriteKind sprite_kind,
	const int x,
	const int y,
	int vertex_index,
	Hw3dXySpriteMap& map)
{
	const auto xy = ::hw_encode_xy(x, y);

	const auto map_it = map.find(xy);

	if (map_it != map.cend())
	{
		::Quit("Sprite already mapped.");
	}

	map[xy] = Hw3dSprite{};
	auto& sprite = map[xy];
	sprite.kind_ = sprite_kind;
	sprite.x_ = x;
	sprite.y_ = y;
	sprite.vertex_index_ = vertex_index;

	// Bottom-left.
	//
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index++];
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = ::hw_3d_sprites_vb_buffer_[vertex_index];
		vertex.rgba_ = bstone::RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = bstone::Vec2F{0.0F, 1.0F};
	}

	return sprite;
}

void hw_3d_map_static(
	const statobj_t& bs_static)
{
	const auto bs_static_index = static_cast<int>(&bs_static - ::statobjlist);

	auto vertex_index = ::hw_3d_statics_base_vertex_index;
	vertex_index += (bs_static_index * ::hw_3d_vertices_per_sprite);

	auto& sprite = ::hw_3d_map_sprite(
		Hw3dSpriteKind::stat,
		bs_static.tilex,
		bs_static.tiley,
		vertex_index,
		::hw_3d_xy_static_map_
	);

	sprite.bs_object_.stat_ = &bs_static;
	sprite.texture_2d_ = ::hw_texture_manager_->sprite_get(bs_static.shapenum);
}

void hw_3d_add_static(
	const statobj_t& bs_static)
{
	::hw_3d_map_static(bs_static);
}

void hw_3d_remove_static(
	const statobj_t& bs_static)
{
	const auto xy = ::hw_encode_xy(bs_static.tilex, bs_static.tiley);

	if (xy == 0)
	{
		// Reserved static.

		return;
	}

	const auto map_it = ::hw_3d_xy_static_map_.find(xy);

	if (map_it == ::hw_3d_xy_static_map_.cend())
	{
		::Quit("Static not mapped.");
	}

	static_cast<void>(::hw_3d_xy_static_map_.erase(map_it));
}

void hw_3d_change_sprite_texture(
	const Hw3dSpriteKind sprite_kind,
	const int x,
	const int y)
{
	auto map = Hw3dXySpriteMapPtr{};

	switch (sprite_kind)
	{
	case Hw3dSpriteKind::stat:
		map = &::hw_3d_xy_static_map_;
		break;

	case Hw3dSpriteKind::actor:
	default:
		::Quit("Invalid sprite kind.");

		return;
	}

	const auto xy = ::hw_encode_xy(x, y);

	const auto map_it = map->find(xy);

	if (map_it == map->cend())
	{
		::Quit("Sprite not mapped.");
	}

	auto& sprite = map_it->second;

	auto sprite_id = 0;

	switch (sprite_kind)
	{
	case Hw3dSpriteKind::stat:
		sprite_id = sprite.bs_object_.stat_->shapenum;;
		break;

	case Hw3dSpriteKind::actor:
	default:
		::Quit("Invalid sprite kind.");

		return;
	}

	sprite.texture_2d_ = ::hw_texture_manager_->sprite_get(sprite_id);
}

void hw_3d_change_static_texture(
	const statobj_t& bs_static)
{
	::hw_3d_change_sprite_texture(Hw3dSpriteKind::stat, bs_static.tilex, bs_static.tiley);
}

void hw_3d_precache_static(
	const statobj_t& bs_static)
{
	int sprite_0 = bs_static.shapenum;
	int sprite_1 = bs_static.shapenum;

	if (false)
	{
	}
	// Full water bowl.
	else if (sprite_0 == ::SPR_STAT_40)
	{
		// Empty water bowl.
		sprite_1 = ::SPR_STAT_41;
	}
	// Chicken leg.
	else if (sprite_0 == ::SPR_STAT_42)
	{
		// Chicken bone.
		sprite_1 = ::SPR_STAT_43;
	}
	// Ham.
	else if (sprite_0 == ::SPR_STAT_44)
	{
		// Ham bone.
		sprite_1 = ::SPR_STAT_45;
	}
	// Candy bar.
	else if (sprite_0 == ::SPR_CANDY_BAR)
	{
		// Candy bar wrapper.
		sprite_1 = ::SPR_CANDY_WRAPER;
	}
	// Sandwich.
	else if (sprite_0 == ::SPR_SANDWICH)
	{
		// Sandwich wrapper.
		sprite_1 = ::SPR_SANDWICH_WRAPER;
	}
	// Plasma detonator.
	else if (sprite_0 == ::SPR_DOORBOMB)
	{
		// Plasma detonator (blink).
		sprite_1 = ::SPR_ALT_DOORBOMB;
	}
	// Plasma detonator (blink).
	else if (sprite_0 == ::SPR_ALT_DOORBOMB)
	{
		// Plasma detonator.
		sprite_1 = ::SPR_DOORBOMB;
	}

	::hw_texture_manager_->sprite_cache(sprite_0);

	if (sprite_0 != sprite_1)
	{
		::hw_texture_manager_->sprite_cache(sprite_1);
	}
}

void hw_3d_precache_statics()
{
	for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		::hw_3d_precache_static(*bs_static);
	}
}

void hw_precache_sprites()
{
	::hw_3d_precache_statics();
}

void hw_3d_build_statics()
{
	::hw_3d_uninitialize_statics();

	if (!::hw_3d_initialize_statics())
	{
		::Quit("Failed to initialize statics.");
	}

	for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		::hw_3d_map_static(*bs_static);
	}
}

void hw_3d_build_sprites()
{
	::hw_3d_uninitialize_sprites();
	::hw_3d_initialize_sprites();

	::hw_3d_build_statics();
}

void hw_precache_resources()
{
	::hw_texture_manager_->cache_begin();

	::hw_precache_flooring();
	::hw_precache_ceiling();
	::hw_precache_solid_walls();
	::hw_precache_pushwalls();
	::hw_precache_doors();
	::hw_precache_sprites();

	::hw_texture_manager_->cache_end();

	::hw_texture_manager_->cache_purge();
}

void hw_uninitialize_video()
{
	::hw_command_sets_ .clear();
	::hw_2d_command_set_ = nullptr;

	::hw_3d_uninitialize_solid_walls();
	::hw_3d_uninitialize_pushwalls();
	::hw_3d_uninitialize_door_sides();
	::hw_3d_uninitialize_sprites();

	::hw_uninitialize_flooring();
	::hw_uninitialize_ceiling();

	::hw_uninitialize_ui_texture();
	::hw_uninitialize_vga_buffer();

	if (::hw_texture_manager_)
	{
		::hw_texture_manager_->uninitialize();
		::hw_texture_manager_ = nullptr;
	}

	if (::hw_renderer_manager_)
	{
		::hw_renderer_manager_->uninitialize();
		::hw_renderer_manager_ = nullptr;
	}
}

//
// Accelerated renderer.
// ==========================================================================

} // namespace
// BBi


// ===========================================================================

// asm

void VL_WaitVBL(
	std::uint32_t vbls)
{
	if (vbls == 0)
	{
		return;
	}

	::sys_sleep_for(1000 * vbls / TickBase);
}

// ===========================================================================


// BBi Moved from jm_free.cpp
void VL_Startup()
{
#if !BSTONE_DBG_FORCE_SW
	if (::hw_initialize_video())
	{
		::hw_refresh_screen();
	}
	else
	{
		::hw_uninitialize_video();
#endif // BSTONE_DBG_FORCE_SW

		::sw_initialize_video();
		::sw_refresh_screen();

#if !BSTONE_DBG_FORCE_SW
	}
#endif // BSTONE_DBG_FORCE_SW

	::in_handle_events();

	if (::vid_is_hw_)
	{
		::hw_check_vsync();
	}
	else
	{
		::sw_check_vsync();
	}
}
// BBi

void VL_Shutdown()
{
	if (::vid_is_hw_)
	{
		::hw_uninitialize_video();
	}
	else
	{
		::sw_uninitialize_video();
	}
}

// ===========================================================================

/*
=============================================================================

								PALETTE OPS

				To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

void VL_FillPalette(
	std::uint8_t red,
	std::uint8_t green,
	std::uint8_t blue)
{
	for (auto& vga_color : ::vid_vga_palette)
	{
		vga_color.r = red;
		vga_color.g = green;
		vga_color.b = blue;
	}

	if (::vid_is_hw_)
	{
		::hw_update_palette(0, palette_color_count);
	}
	else
	{
		::sw_update_palette(0, palette_color_count);
	}
}

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		auto& vga_color = ::vid_vga_palette[first + i];

		vga_color.r = palette[(3 * i) + 0];
		vga_color.g = palette[(3 * i) + 1];
		vga_color.b = palette[(3 * i) + 2];
	}

	if (::vid_is_hw_)
	{
		::hw_update_palette(0, palette_color_count);
	}
	else
	{
		::sw_update_palette(0, palette_color_count);
	}
}

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		const auto& vga_color = ::vid_vga_palette[first + i];

		palette[(3 * i) + 0] = vga_color.r;
		palette[(3 * i) + 1] = vga_color.g;
		palette[(3 * i) + 2] = vga_color.b;
	}
}

void vl_hw_fade_out(
	const int red,
	const int green,
	const int blue,
	const int step_count)
{
	::hw_2d_fade_is_enabled_ = true;

	::hw_2d_fade_color_ = ::hw_vga_color_to_color_32(red, green, blue);

	if (!::g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = (i * alpha) / step_count;

			::hw_2d_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

			::VL_RefreshScreen();

			if (!::vid_has_vsync)
			{
				::VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	::hw_2d_fade_color_.a_ = 0xFF;

	::VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	::VL_RefreshScreen();

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}

	::hw_2d_fade_is_enabled_ = false;

	::screenfaded = true;
}

// Fades the current palette to the given color in the given number of steps.
void VL_FadeOut(
	const int start,
	const int end,
	const int red,
	const int green,
	const int blue,
	const int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(red >= 0 && red <= 0xFF);
	assert(green >= 0 && green <= 0xFF);
	assert(blue >= 0 && blue <= 0xFF);
	assert(steps > 0);
	assert(start <= end);

	if (::vid_is_hw_)
	{
		::vl_hw_fade_out(red, green, blue, steps);

		return;
	}

	if (!::g_no_fade_in_or_out)
	{
		::VL_GetPalette(0, 256, &::palette1[0][0]);

		std::uninitialized_copy_n(&::palette1[0][0], 768, &::palette2[0][0]);

		//
		// fade through intermediate frames
		//
		for (int i = 0; i < steps; ++i)
		{
			auto origptr = &::palette1[start][0];
			auto newptr = &::palette2[start][0];

			for (int j = start; j <= end; ++j)
			{
				auto orig = *origptr++;
				auto delta = red - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = green - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = blue - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));
			}

			::sw_filler_color.r = ::palette2[filler_color_index][0];
			::sw_filler_color.g = ::palette2[filler_color_index][1];
			::sw_filler_color.b = ::palette2[filler_color_index][2];

			::VL_SetPalette(0, 256, &::palette2[0][0]);

			::VL_RefreshScreen();

			if (!::vid_has_vsync)
			{
				::VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	::sw_filler_color = SDL_Color
	{
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue),
		0xFF
	};

	::VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	::VL_RefreshScreen();

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}

	::screenfaded = true;
}

void vl_hw_fade_in(
	const std::uint8_t* const palette,
	const int step_count)
{
	::hw_2d_fade_is_enabled_ = true;

	::VL_SetPalette(0, 256, palette);

	::hw_2d_fade_color_.a_ = 0xFF;

	if (!::g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = ((step_count - 1 - i) * alpha) / step_count;

			::hw_2d_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

			::VL_RefreshScreen();

			if (!::vid_has_vsync)
			{
				::VL_WaitVBL(1);
			}
		}
	}

	::hw_2d_fade_color_.a_ = 0x00;

	::VL_RefreshScreen();

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}

	::hw_2d_fade_is_enabled_ = false;

	::screenfaded = false;
}

void VL_FadeIn(
	const int start,
	const int end,
	const std::uint8_t* const palette,
	const int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(palette != nullptr);
	assert(steps > 0);
	assert(start <= end);

	if (::vid_is_hw_)
	{
		::vl_hw_fade_in(palette, steps);

		return;
	}

	if (!::g_no_fade_in_or_out)
	{
		::VL_GetPalette(0, 256, &::palette1[0][0]);

		std::uninitialized_copy_n(&::palette1[0][0], 768, &::palette2[0][0]);

		const auto start_index = start * 3;
		const auto end_index = (end * 3) + 2;

		//
		// fade through intermediate frames
		//
		auto delta = 0;

		for (int i = 0; i < steps; ++i)
		{
			for (int j = start_index; j <= end_index; ++j)
			{
				const int delta = palette[j] - ::palette1[0][j];

				::palette2[0][j] =
					static_cast<std::uint8_t>(::palette1[0][j] + ((delta * i) / steps));
			}

			::sw_filler_color.r = ::palette2[filler_color_index][0];
			::sw_filler_color.g = ::palette2[filler_color_index][1];
			::sw_filler_color.b = ::palette2[filler_color_index][2];

			::VL_SetPalette(0, 256, &::palette2[0][0]);

			::VL_RefreshScreen();

			if (!::vid_has_vsync)
			{
				::VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	::sw_filler_color.r = palette[(filler_color_index * 3) + 0];
	::sw_filler_color.g = palette[(filler_color_index * 3) + 1];
	::sw_filler_color.b = palette[(filler_color_index * 3) + 2];

	::VL_SetPalette(0, 256, palette);

	::VL_RefreshScreen();

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}

	::screenfaded = false;
}

void VL_SetPaletteIntensity(
	int start,
	int end,
	const std::uint8_t* palette,
	int intensity)
{
	auto cmap = &::palette1[0][0] + (start * 3);

	intensity = 63 - intensity;

	for (int loop = start; loop <= end; ++loop)
	{
		int red = (*palette++) - intensity;

		if (red < 0)
		{
			red = 0;
		}

		*cmap++ = static_cast<std::uint8_t>(red);

		int green = *palette++ - intensity;

		if (green < 0)
		{
			green = 0;
		}

		*cmap++ = static_cast<std::uint8_t>(green);

		int blue = *palette++ - intensity;

		if (blue < 0)
		{
			blue = 0;
		}

		*cmap++ = static_cast<std::uint8_t>(blue);
	}

	::VL_SetPalette(
		start,
		end - start + 1,
		&::palette1[0][0]);
}

/*
=============================================================================

 PIXEL OPS

=============================================================================
*/

void VL_Plot(
	int x,
	int y,
	std::uint8_t color,
	const bool is_transparent)
{
	const auto offset = (y * ::vga_ref_width) + x;

	::vid_ui_buffer[offset] = color;
	::vid_mask_buffer[offset] = !is_transparent;
}

void VL_Hlin(
	int x,
	int y,
	int width,
	std::uint8_t color)
{
	::VL_Bar(x, y, width, 1, color);
}

void VL_Vlin(
	int x,
	int y,
	int height,
	std::uint8_t color)
{
	::VL_Bar(x, y, 1, height, color);
}

void VL_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	const bool is_transparent)
{
	if (x == 0 && width == ::vga_ref_width)
	{
		const auto offset = y * ::vga_ref_width;
		const auto count = height * ::vga_ref_width;

		std::uninitialized_fill(
			::vid_ui_buffer.begin() + offset,
			::vid_ui_buffer.begin() + offset + count,
			color);

		std::uninitialized_fill(
			::vid_mask_buffer.begin() + offset,
			::vid_mask_buffer.begin() + offset + count,
			!is_transparent);
	}
	else
	{
		for (int i = 0; i < height; ++i)
		{
			const auto offset = ((y + i) * ::vga_ref_width) + x;

			std::uninitialized_fill(
				::vid_ui_buffer.begin() + offset,
				::vid_ui_buffer.begin() + offset + width,
				color);

			std::uninitialized_fill(
				::vid_mask_buffer.begin() + offset,
				::vid_mask_buffer.begin() + offset + width,
				!is_transparent);
		}
	}
}

/*
============================================================================

 MEMORY OPS

============================================================================
*/

void VL_MemToLatch(
	const std::uint8_t* source,
	int width,
	int height,
	int dest)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				const auto pixel = *source++;
				const auto offset = dest + ((h * width) + w);

				::latches_cache[offset] = pixel;
			}
		}
	}
}

void VL_MemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				::VL_Plot(x + w, y + h, *source++);
			}
		}
	}
}

void VL_MaskMemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y,
	std::uint8_t mask)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				const auto color = *source++;

				if (color != mask)
				{
					::VL_Plot(x + w, y + h, color);
				}
			}
		}
	}
}

void VL_ScreenToMem(
	std::uint8_t* dest,
	int width,
	int height,
	int x,
	int y)
{
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				*dest++ = ::vl_get_pixel(::bufferofs, x + w, y + h);
			}
		}
	}
}

void VL_LatchToScreen(
	int source,
	int width,
	int height,
	int x,
	int y)
{
	for (int h = 0; h < height; ++h)
	{
		const auto src_offset = source + (h * width);
		const auto dst_offset = (::vga_ref_width * (y + h)) + x;

		std::uninitialized_copy(
			::latches_cache.cbegin() + src_offset,
			::latches_cache.cbegin() + src_offset + width,
			::vid_ui_buffer.begin() + dst_offset);

		std::uninitialized_fill(
			::vid_mask_buffer.begin() + dst_offset,
			::vid_mask_buffer.begin() + dst_offset + width,
			true);
	}
}

void VL_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height)
{
	for (int h = 0; h < height; ++h)
	{
		const auto src_offset = source + (h * ::vga_ref_width);
		const auto dst_offset = dest + (h * ::vga_ref_width);

		std::uninitialized_copy(
			::vid_ui_buffer.cbegin() + src_offset,
			::vid_ui_buffer.cbegin() + src_offset + width,
			::vid_ui_buffer.begin() + dst_offset);

		std::uninitialized_fill(
			::vid_mask_buffer.begin() + dst_offset,
			::vid_mask_buffer.begin() + dst_offset + width,
			true);
	}
}


void JM_VGALinearFill(
	int start,
	int length,
	std::uint8_t fill)
{
	std::uninitialized_fill(
		::vid_ui_buffer.begin() + start,
		::vid_ui_buffer.begin() + start + length,
		fill);

	std::uninitialized_fill(
		::vid_mask_buffer.begin() + start,
		::vid_mask_buffer.begin() + start + length,
		true);
}

void VL_RefreshScreen()
{
	if (::vid_is_hw_)
	{
		::hw_refresh_screen();
	}
	else
	{
		::sw_refresh_screen();
	}
}

void VH_UpdateScreen()
{
	if (::vid_is_hw_)
	{
		::hw_refresh_screen();
	}
	else
	{
		::sw_refresh_screen();
	}
}

int vl_get_offset(
	int base_offset,
	int x,
	int y)
{
	return base_offset + (y * ::vga_width) + x;
}

std::uint8_t vl_get_pixel(
	int base_offset,
	int x,
	int y)
{
	return ::vid_ui_buffer[(y * ::vga_ref_width) + x];
}

void vl_minimize_fullscreen_window(
	bool value)
{
	if (value)
	{
		::SDL_MinimizeWindow(
			::sw_window);
	}
	else
	{
		::SDL_RestoreWindow(
			::sw_window);
	}
}

void vl_update_widescreen()
{
	if (::vid_is_hw_)
	{
		::hw_update_widescreen();
	}
	else
	{
		::sw_update_widescreen();
	}
}

void vid_set_ui_mask(
	bool value)
{
	::vid_mask_buffer.fill(
		value);
}

void vid_set_ui_mask(
	int x,
	int y,
	int width,
	int height,
	bool value)
{
	for (int h = 0; h < height; ++h)
	{
		const auto offset = ((y + h) * ::vga_ref_width) + x;

		std::uninitialized_fill(
			::vid_mask_buffer.begin() + offset,
			::vid_mask_buffer.begin() + offset + width,
			value);
	}
}

void vid_set_ui_mask_3d(
	bool value)
{
	::vid_set_ui_mask(
		0,
		::ref_3d_view_top_y - ::ref_3d_margin,
		::vga_ref_width,
		::ref_3d_view_height + 2 * ::ref_3d_margin,
		value);
}

void vid_clear_3d()
{
	std::uninitialized_fill(
		::sw_vga_buffer.begin(),
		::sw_vga_buffer.end(),
		0);
}

void vid_export_ui(
	VgaBuffer& dst_buffer)
{
	dst_buffer = ::vid_ui_buffer;
}

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent)
{
	::vid_ui_buffer = src_buffer;
	::vid_set_ui_mask(!is_transparent);
}

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer)
{
	dst_buffer = ::vid_mask_buffer;
}

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer)
{
	::vid_mask_buffer = src_buffer;
}

void vid_draw_ui_sprite(
	const int sprite_id,
	const int center_x,
	const int center_y,
	const int new_side)
{
	constexpr auto dimension = bstone::Sprite::dimension;

	const auto sprite_ptr = ::vid_sprite_cache.cache(
		sprite_id);

	const auto sprite_width = sprite_ptr->get_width();
	const auto sprite_height = sprite_ptr->get_height();

	const auto left = sprite_ptr->get_left();
	const auto x1 = center_x + ((new_side * (left - (dimension / 2))) / dimension);
	const auto x2 = x1 + ((sprite_width * new_side) / dimension);

	const auto top = sprite_ptr->get_top();
	const auto y1 = center_y + ((new_side * (top - (dimension / 2))) / dimension) - 2;
	const auto y2 = y1 + ((sprite_height * new_side) / dimension);

	for (int x = x1; x < x2; ++x)
	{
		if (x < 0)
		{
			continue;
		}

		if (x >= ::vga_ref_width)
		{
			break;
		}

		const auto column_index = ((sprite_width - 1) * (x - x1)) / (x2 - x1 - 1);
		const auto column = sprite_ptr->get_column(column_index);

		for (int y = y1; y < y2; ++y)
		{
			if (y < 0)
			{
				continue;
			}

			if (y >= ::vga_ref_height)
			{
				break;
			}

			const auto row_index = ((sprite_height - 1) * (y - y1)) / (y2 - y1 - 1);
			const auto sprite_color = column[row_index];

			if (sprite_color < 0)
			{
				continue;
			}

			const auto color_index = static_cast<std::uint8_t>(sprite_color);

			::VL_Plot(x, y, color_index);
		}
	}
}

void vid_hw_on_level_load()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_precache_resources();

	::hw_3d_build_solid_walls();
	::hw_3d_build_pushwalls();
	::hw_3d_build_doors();
	::hw_3d_build_sprites();
}

void vid_hw_on_wall_switch_update(
	const int x,
	const int y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	assert(x >= 0 && x < MAPSIZE && y >= 0 && y < MAPSIZE);

	const auto xy = ::hw_encode_xy(x, y);

	auto wall_it = ::hw_3d_xy_wall_map_.find(xy);

	if (wall_it == ::hw_3d_xy_wall_map_.cend())
	{
		::Quit("Expected wall at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
	}

	const auto tile_wall = ::tilemap[x][y] & ::tilemap_wall_mask;

	assert(tile_wall == OFF_SWITCH || tile_wall == ON_SWITCH);

	const auto horizontal_wall_id = ::horizwall[tile_wall];
	const auto horizontal_texture_2d = ::hw_texture_manager_->wall_get(horizontal_wall_id);
	assert(horizontal_texture_2d);

	const auto vertical_wall_id = ::vertwall[tile_wall];
	const auto vertical_texture_2d = ::hw_texture_manager_->wall_get(vertical_wall_id);
	assert(vertical_texture_2d);

	auto& wall = wall_it->second;

	for (auto& side : wall.sides_)
	{
		const auto& flags = side.flags_;

		if (!flags.is_active_ || flags.is_door_track_)
		{
			continue;
		}

		if (flags.is_vertical_)
		{
			side.texture_2d_ = vertical_texture_2d;
		}
		else
		{
			side.texture_2d_ = horizontal_texture_2d;
		}
	}
}

void vid_hw_on_pushwall_move()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_translate_pushwall();
}

void vid_hw_on_pushwall_step(
	const int old_x,
	const int old_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_step_pushwall(old_x, old_y);
}

void vid_hw_on_door_move(
	const int door_index)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto& bs_door = ::doorobjlist[door_index];

	const auto xy = ::hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = ::hw_3d_xy_door_map_.find(xy);

	if (map_it == ::hw_3d_xy_door_map_.cend())
	{
		::Quit("Door mapping not found.");
	}

	auto& door = ::hw_3d_xy_door_map_[xy];

	auto vertex_index = door.sides_.front().vertex_index_;
	const auto old_vertex_index = vertex_index;

	::hw_3d_map_door_side(door.sides_[0], vertex_index, ::hw_3d_doors_vb_buffer_);
	::hw_3d_map_door_side(door.sides_[1], vertex_index, ::hw_3d_doors_vb_buffer_);

	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = old_vertex_index;
	param.count_ = ::hw_3d_vertices_per_door_sides;
	param.vertices_ = &::hw_3d_doors_vb_buffer_[old_vertex_index];

	::hw_3d_door_sides_vb_->update(param);
}

void vid_hw_on_door_lock_update(
	const int door_index)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto& bs_door = ::doorobjlist[door_index];

	const auto xy = ::hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = ::hw_3d_xy_door_map_.find(xy);

	if (map_it == ::hw_3d_xy_door_map_.cend())
	{
		::Quit("Door mapping not found.");
	}

	auto& door = ::hw_3d_xy_door_map_[xy];

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	::door_get_page_numbers(*door.door_, front_face_page_number, back_face_page_number);

	const auto front_face_texture_2d = ::hw_texture_manager_->wall_get(front_face_page_number);
	const auto back_face_texture_2d = ::hw_texture_manager_->wall_get(back_face_page_number);

	assert(front_face_texture_2d);
	assert(back_face_texture_2d);

	door.sides_[0].texture_2d_ = front_face_texture_2d;
	door.sides_[1].texture_2d_ = back_face_texture_2d;
}

void vid_hw_on_static_add(
	const statobj_t& bs_static)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_add_static(bs_static);
}

void vid_hw_on_static_remove(
	const statobj_t& bs_static)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_remove_static(bs_static);
}

void vid_hw_on_static_change_texture(
	const statobj_t& bs_static)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_change_static_texture(bs_static);
}
// BBi
