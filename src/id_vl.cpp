/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "id_heads.h"
#include "bstone_sprite.h"
#include "bstone_sprite_cache.h"


static const int palette_color_count = 256;


extern bool is_full_menu_active;


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

VgaBuffer sdl_ui_buffer;
UiMaskBuffer sdl_mask_buffer;


std::string sdl_error_message;

VgaPalette sdl_vga_palette;
VgaBuffer sdl_vga_buffer;

bool sdl_use_custom_window_position = false;
int sdl_window_x = 0;
int sdl_window_y = 0;

SDL_DisplayMode display_mode;
bool sdl_is_windowed = false;
SDL_Window* sdl_window = nullptr;
SDL_Renderer* sdl_renderer = nullptr;
SDL_PixelFormat* sdl_texture_pixel_format = nullptr;
SDL_Texture* sdl_screen_texture = nullptr;
SDL_Texture* sdl_ui_texture = nullptr;
SdlPalette sdl_palette;
SDL_Rect sdl_ui_whole_src_rect;
SDL_Rect sdl_ui_whole_dst_rect;
SDL_Rect sdl_ui_top_src_rect;
SDL_Rect sdl_ui_top_dst_rect;
SDL_Rect sdl_ui_wide_middle_src_rect;
SDL_Rect sdl_ui_wide_middle_dst_rect;
SDL_Rect sdl_ui_bottom_src_rect;
SDL_Rect sdl_ui_bottom_dst_rect;
std::array<SDL_Rect, 2> sdl_filler_ui_rects;
std::array<SDL_Rect, 4> sdl_filler_hud_rects;
const auto sdl_ref_filler_color = SDL_Color{0x00, 0x28, 0x50, 0xFF, };
auto sdl_filler_color = SDL_Color{};

const auto filler_color_index = 0xE8;


void sdl_initialize_vga_buffer()
{
	const auto vga_area = 2 * ::vga_width * ::vga_height;

	::sdl_vga_buffer.resize(
		vga_area);

	::vga_memory = ::sdl_vga_buffer.data();
}

void sdl_initialize_ui_buffer()
{
	const auto area = ::vga_ref_width * ::vga_ref_height;

	::sdl_ui_buffer.resize(
		area);
}

bool sdl_initialize_window()
{
	bstone::Log::write("VID: Creating a window...");


	if (!::sdl_use_custom_window_position)
	{
		::sdl_window_x = SDL_WINDOWPOS_CENTERED;
		::sdl_window_y = SDL_WINDOWPOS_CENTERED;
	}

	if (::sdl_window_x < 0)
	{
		::sdl_window_x = 0;
	}

	if (::sdl_window_y < 0)
	{
		::sdl_window_y = 0;
	}

	auto window_flags = Uint32{
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_HIDDEN |
		0};

	if (!::sdl_is_windowed)
	{
		window_flags |=
			SDL_WINDOW_BORDERLESS |
			SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

#ifdef __vita__
	window_flags = SDL_WINDOW_SHOWN;
#endif

	const auto& assets_info = AssetsInfo{};

	auto title = std::string{"Blake Stone"};

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

	::sdl_window = ::SDL_CreateWindow(
		title.c_str(),
		::sdl_window_x,
		::sdl_window_y,
		::window_width,
		::window_height,
		window_flags);

	if (!::sdl_window)
	{
		::sdl_error_message = "VID: Failed to create a window: ";
		::sdl_error_message += ::SDL_GetError();

		bstone::Log::write_error(::SDL_GetError());

		return false;
	}

	auto hint_result = ::SDL_SetHint("SDL_HINT_RENDER_DRIVER", "opengl");

	return true;
}

bool sdl_initialize_renderer()
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

			bstone::Log::write(
				"{}. {}",
				i + 1,
				info.name);
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
				bstone::Log::write_warning(
					"VID: Unsupported renderer: {}",
					ren_string);
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

		::sdl_renderer = ::SDL_CreateRenderer(
			::sdl_window,
			-1,
			renderer_flags);

		if (!::sdl_renderer)
		{
			is_succeed = false;

			::sdl_error_message = "VID: Failed to create a renderer: ";
			::sdl_error_message += ::SDL_GetError();

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
			::sdl_renderer,
			&renderer_info);

		if (sdl_result != 0)
		{
			is_succeed = false;

			::sdl_error_message = "VID: Failed to query the renderer: ";
			::sdl_error_message += ::SDL_GetError();

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
				bstone::Log::write_warning(
					"VID: Unexpected renderer is selected: {0}.",
					renderer_info.name);
			}
		}
		else
		{
			bstone::Log::write(
				"VID: Current renderer: {0}.",
				renderer_info.name);
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

		::sdl_texture_pixel_format = ::SDL_AllocFormat(
			pixel_format);

		if (!::sdl_texture_pixel_format)
		{
			is_succeed = false;

			::sdl_error_message = "VID: Failed to allocate a texture pixel format: ";
			::sdl_error_message += ::SDL_GetError();

			bstone::Log::write_error(
				::SDL_GetError());
		}
	}

	return is_succeed;
}

bool sdl_initialize_screen_texture()
{
	bstone::Log::write(
		"VID: Creating a screen texture...");

	::sdl_screen_texture = ::SDL_CreateTexture(
		::sdl_renderer,
		::sdl_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_width,
		::vga_height);

	if (!::sdl_screen_texture)
	{
		::sdl_error_message = "VID: Failed to create a screen texture: ";
		::sdl_error_message += ::SDL_GetError();

		bstone::Log::write(
			::SDL_GetError());

		return false;
	}

	return true;
}

bool sdl_initialize_ui_texture()
{
	bstone::Log::write(
		"VID: Creating an UI texture...");

	::sdl_ui_texture = ::SDL_CreateTexture(
		::sdl_renderer,
		::sdl_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_ref_width,
		::vga_ref_height);

	if (!::sdl_ui_texture)
	{
		::sdl_error_message = "VID: Failed to create an UI texture: ";
		::sdl_error_message += ::SDL_GetError();

		bstone::Log::write(
			::SDL_GetError());

		return false;
	}

	return true;
}

bool sdl_initialize_textures()
{
	bstone::Log::write(
		"VID: Initializing textures...");


	auto is_succeed = true;

	if (is_succeed)
	{
		is_succeed = sdl_initialize_screen_texture();
	}

	if (is_succeed)
	{
		is_succeed = sdl_initialize_ui_texture();
	}

	return is_succeed;
}

void sdl_update_palette(
	int first_index,
	int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = ::sdl_vga_palette[color_index];
		auto& sdl_color = ::sdl_palette[color_index];

		sdl_color = ::SDL_MapRGB(
			::sdl_texture_pixel_format,
			(255 * vga_color.r) / 63,
			(255 * vga_color.g) / 63,
			(255 * vga_color.b) / 63);
	}
}

void sdl_update_viewport()
{
	auto sdl_result = ::SDL_RenderSetLogicalSize(
		::sdl_renderer,
		::screen_width,
		::screen_height);

	if (sdl_result != 0)
	{
		bstone::Log::write_error(
			"VID: Failed to update a viewport.");
	}
}

void sdl_initialize_palette()
{
	::sdl_vga_palette.fill({});

	::sdl_update_palette(
		0,
		palette_color_count);
}

void sdl_calculate_dimensions()
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
	::sdl_ui_whole_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::vga_ref_height,
	};

	::sdl_ui_whole_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		::screen_height,
	};


	// UI top rect
	//
	::sdl_ui_top_src_rect = SDL_Rect{
		0,
		0,
		::vga_ref_width,
		::ref_top_bar_height,
	};

	::sdl_ui_top_dst_rect = SDL_Rect{
		::filler_width,
		0,
		::screen_width - (2 * ::filler_width),
		upper_filler_height,
	};


	// UI middle rect (stretched to full width)
	//
	::sdl_ui_wide_middle_src_rect = SDL_Rect{
		0,
		::ref_view_top_y,
		::vga_ref_width,
		::ref_view_height,
	};

	::sdl_ui_wide_middle_dst_rect = SDL_Rect{
		0,
		upper_filler_height,
		::screen_width,
		middle_filler_height,
	};


	// UI bottom rect
	//
	::sdl_ui_bottom_src_rect = SDL_Rect{
		0,
		::ref_view_bottom_y,
		::vga_ref_width,
		::ref_bottom_bar_height,
	};

	::sdl_ui_bottom_dst_rect = SDL_Rect{
		::filler_width,
		::screen_height - lower_filler_height,
		::screen_width - (2 * ::filler_width),
		lower_filler_height,
	};


	// UI left bar
	::sdl_filler_ui_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		::screen_height,
	};

	// UI right bar
	::sdl_filler_ui_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		::screen_height,
	};

	// HUD upper left rect
	::sdl_filler_hud_rects[0] = SDL_Rect{
		0,
		0,
		::filler_width,
		upper_filler_height
	};

	// HUD upper right rect
	::sdl_filler_hud_rects[1] = SDL_Rect{
		::screen_width - ::filler_width,
		0,
		::filler_width,
		upper_filler_height,
	};

	// HUD lower left rect
	::sdl_filler_hud_rects[2] = SDL_Rect{
		0,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	// HUD lower right rect
	::sdl_filler_hud_rects[3] = SDL_Rect{
		::screen_width - ::filler_width,
		::screen_height - lower_filler_height,
		::filler_width,
		lower_filler_height,
	};

	::sdl_filler_color = SDL_Color{
		::vgapal[(filler_color_index * 3) + 0],
		::vgapal[(filler_color_index * 3) + 1],
		::vgapal[(filler_color_index * 3) + 2],
		0xFF, };
}

void sdl_initialize_video()
{
	bstone::Log::write(
		"VID: Initializing a system...");

	const auto& string_helper = bstone::StringHelper{};

	bool is_custom_scale = false;

	//
	// Option "vid_windowed"
	//

	::sdl_is_windowed = ::g_args.has_option(
		"vid_windowed");

	::sdl_use_custom_window_position = false;


	//
	// Option "vid_window_x"
	//

	const auto& vid_window_x_str = ::g_args.get_option_value(
		"vid_window_x");

	if (string_helper.lexical_cast(vid_window_x_str, ::sdl_window_x))
	{
		::sdl_use_custom_window_position = true;
	}


	//
	// Option "vid_window_y"
	//

	const auto& vid_window_y_str = ::g_args.get_option_value(
		"vid_window_y");

	if (string_helper.lexical_cast(vid_window_y_str, ::sdl_window_y))
	{
		::sdl_use_custom_window_position = true;
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

	static_cast<void>(string_helper.lexical_cast(width_str, ::window_width));
	static_cast<void>(string_helper.lexical_cast(height_str, ::window_height));

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
		int scale_value = 0;

		if (string_helper.lexical_cast(vid_scale_str, scale_value))
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

	if (!::sdl_is_windowed)
	{
		::window_width = ::display_mode.w;
		::window_height = ::display_mode.h;
	}


	::sdl_calculate_dimensions();

	bool is_succeed = true;

	if (is_succeed)
	{
		is_succeed = ::sdl_initialize_window();
	}

	if (is_succeed)
	{
		is_succeed = ::sdl_initialize_renderer();
	}

	if (is_succeed)
	{
		is_succeed = ::sdl_initialize_textures();
	}

	if (is_succeed)
	{
		::sdl_initialize_palette();
	}

	if (is_succeed)
	{
		::sdl_initialize_vga_buffer();
		::sdl_initialize_ui_buffer();
	}

	if (is_succeed)
	{
		::SDL_ShowWindow(
			::sdl_window);

		::in_grab_mouse(
			true);
	}
	else
	{
		::Quit(
			::sdl_error_message);
	}
}

void sdl_uninitialize_screen_texture()
{
	if (::sdl_screen_texture)
	{
		::SDL_DestroyTexture(
			::sdl_screen_texture);

		::sdl_screen_texture = nullptr;
	}
}

void sdl_uninitialize_ui_texture()
{
	if (::sdl_ui_texture)
	{
		::SDL_DestroyTexture(
			::sdl_ui_texture);

		::sdl_ui_texture = nullptr;
	}
}

void sdl_uninitialize_vga_buffer()
{
	::sdl_vga_buffer.clear();
	::sdl_vga_buffer.shrink_to_fit();

	::vga_memory = nullptr;
}

void sdl_uninitialize_video()
{
	if (::sdl_texture_pixel_format)
	{
		::SDL_FreeFormat(
			::sdl_texture_pixel_format);

		::sdl_texture_pixel_format = nullptr;
	}

	::sdl_uninitialize_screen_texture();
	::sdl_uninitialize_ui_texture();

	if (::sdl_renderer)
	{
		::SDL_DestroyRenderer(
			::sdl_renderer);

		::sdl_renderer = nullptr;
	}

	if (::sdl_window)
	{
		::SDL_DestroyWindow(
			::sdl_window);

		::sdl_window = nullptr;
	}

	::sdl_uninitialize_vga_buffer();
}

void sdl_refresh_screen()
{
	int sdl_result = 0;

	// HUD+3D stuff
	//
	if (::vid_is_hud)
	{
		const auto src_pixels = ::sdl_vga_buffer.data();
		const auto src_pitch = ::vga_width;

		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = ::SDL_LockTexture(
			::sdl_screen_texture,
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::Quit(
				"VID: Failed to lock a screen texture: {}",
				::SDL_GetError());
		}

		auto dst_pixels = static_cast<std::uint32_t*>(
			dst_raw_pixels);

		for (int y = 0; y < ::vga_height; ++y)
		{
			const auto src_line = &src_pixels[y * src_pitch];
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < ::vga_width; ++x)
			{
				dst_line[x] = sdl_palette[src_line[x]];
			}
		}

		::SDL_UnlockTexture(
			::sdl_screen_texture);
	}


	// 2D stuff
	//
	{
		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = ::SDL_LockTexture(
			::sdl_ui_texture,
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::Quit(
				"VID: Failed to lock an UI texture: {}",
				::SDL_GetError());
		}

		const auto alpha_0_mask = ~sdl_texture_pixel_format->Amask;

		auto dst_pixels = static_cast<std::uint32_t*>(
			dst_raw_pixels);

		for (int y = 0; y < ::vga_ref_height; ++y)
		{
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < ::vga_ref_width; ++x)
			{
				const auto src_offset = (y * ::vga_ref_width) + x;
				auto dst_color = ::sdl_palette[::sdl_ui_buffer[src_offset]];

				if (::vid_is_hud)
				{
					if (!::sdl_mask_buffer[src_offset])
					{
						dst_color &= alpha_0_mask;
					}
				}

				dst_line[x] = dst_color;
			}
		}

		::SDL_UnlockTexture(
			::sdl_ui_texture);
	}


	// Clear all
	//
	sdl_result = ::SDL_RenderClear(
		sdl_renderer);

	if (sdl_result != 0)
	{
		::Quit(
			"VID: Failed to clear a render target: {}",
			::SDL_GetError());
	}


	// Copy HUD+3D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_RenderCopy(
			sdl_renderer,
			sdl_screen_texture,
			nullptr,
			nullptr);

		if (sdl_result != 0)
		{
			::Quit(
				"VID: Failed to copy a screen texture on a render target: {}",
				::SDL_GetError());
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
			fill_color = ::sdl_filler_color;
		}

		::SDL_SetRenderDrawColor(
			sdl_renderer,
			fill_color.r,
			fill_color.g,
			fill_color.b,
			0xFF);

		if (is_hud)
		{
			::SDL_RenderFillRects(sdl_renderer, ::sdl_filler_hud_rects.data(), 4);
		}
		else
		{
			::SDL_RenderFillRects(sdl_renderer, ::sdl_filler_ui_rects.data(), 2);
		}
	}


	// Copy 2D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sdl_ui_texture,
			SDL_BLENDMODE_BLEND);

		if (sdl_result != 0)
		{
			::Quit(
				"VID: Failed to set blend mode for an UI texture: {}",
				::SDL_GetError());
		}
	}

	if (!::vid_is_ui_stretched)
	{
		if (::vid_is_fizzle_fade)
		{
			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sdl_renderer,
					::sdl_ui_texture,
					&::sdl_ui_top_src_rect,
					&::sdl_ui_top_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sdl_renderer,
					::sdl_ui_texture,
					&::sdl_ui_wide_middle_src_rect,
					&::sdl_ui_wide_middle_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sdl_renderer,
					::sdl_ui_texture,
					&::sdl_ui_bottom_src_rect,
					&::sdl_ui_bottom_dst_rect);
			}
		}
		else
		{
			sdl_result = ::SDL_RenderCopy(
				::sdl_renderer,
				::sdl_ui_texture,
				nullptr,
				&::sdl_ui_whole_dst_rect);
		}
	}
	else
	{
		sdl_result = ::SDL_RenderCopy(
			::sdl_renderer,
			::sdl_ui_texture,
			nullptr,
			nullptr);
	}

	if (sdl_result != 0)
	{
		::Quit(
			"VID: Failed to copy an UI texture on a render target: {}",
			::SDL_GetError());
	}

	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sdl_ui_texture,
			SDL_BLENDMODE_NONE);

		if (sdl_result != 0)
		{
			::Quit(
				"VID: Failed to set blend mode for an UI texture: {}",
				::SDL_GetError());
		}
	}


	// Present
	//
	::SDL_RenderPresent(
		sdl_renderer);

	if (sdl_result != 0)
	{
		::Quit(
			"VID: Failed to present a render target: {}",
			::SDL_GetError());
	}
}

void sdl_check_vsync()
{
	constexpr int draw_count = 10;

	constexpr int duration_tolerance_pct = 25;

	const int expected_duration_ms =
		(1000 * draw_count) / ::display_mode.refresh_rate;

	const int min_expected_duration_ms =
		((100 - duration_tolerance_pct) * expected_duration_ms) / 100;

	const auto before_timestamp = Clock::now();

	for (int i = 0; i < draw_count; ++i)
	{
		::sdl_refresh_screen();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	::vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}

void sdl_update_widescreen()
{
	::sdl_uninitialize_screen_texture();
	::sdl_uninitialize_vga_buffer();
	::sdl_calculate_dimensions();
	::sdl_initialize_vga_buffer();
	::sdl_initialize_screen_texture();
	::sdl_update_viewport();
}


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
	::sdl_initialize_video();
	::sdl_refresh_screen();

	::in_handle_events();

	::sdl_check_vsync();
}
// BBi

void VL_Shutdown()
{
	::sdl_uninitialize_video();
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
	for (auto& vga_color : ::sdl_vga_palette)
	{
		vga_color.r = red;
		vga_color.g = green;
		vga_color.b = blue;
	}

	::sdl_update_palette(
		0,
		palette_color_count);
}

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		auto& vga_color = ::sdl_vga_palette[first + i];

		vga_color.r = palette[(3 * i) + 0];
		vga_color.g = palette[(3 * i) + 1];
		vga_color.b = palette[(3 * i) + 2];
	}

	::sdl_update_palette(
		first,
		count);
}

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		const auto& vga_color = ::sdl_vga_palette[first + i];

		palette[(3 * i) + 0] = vga_color.r;
		palette[(3 * i) + 1] = vga_color.g;
		palette[(3 * i) + 2] = vga_color.b;
	}
}

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps)
{
	int orig;
	int delta;

	::VL_GetPalette(
		0,
		256,
		&::palette1[0][0]);

	std::uninitialized_copy_n(
		&::palette1[0][0],
		768,
		&::palette2[0][0]);

	//
	// fade through intermediate frames
	//
	for (int i = 0; i < steps; ++i)
	{
		auto origptr = &::palette1[start][0];
		auto newptr = &::palette2[start][0];

		for (int j = start; j <= end; ++j)
		{
			orig = *origptr++;
			delta = red - orig;
			*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

			orig = *origptr++;
			delta = green - orig;
			*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

			orig = *origptr++;
			delta = blue - orig;
			*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));
		}

		::sdl_filler_color.r = ::palette2[filler_color_index][0];
		::sdl_filler_color.g = ::palette2[filler_color_index][1];
		::sdl_filler_color.b = ::palette2[filler_color_index][2];

		::VL_SetPalette(
			0,
			256,
			&::palette2[0][0]);

		::VL_RefreshScreen();

		if (!::vid_has_vsync)
		{
			::VL_WaitVBL(1);
		}
	}

	::sdl_filler_color = SDL_Color{
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue),
		0xFF,
	};

	//
	// final color
	//
	::VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	if (!::vid_has_vsync)
	{
		::VL_WaitVBL(1);
	}

	::screenfaded = true;
}

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps)
{
	::VL_GetPalette(
		0,
		256,
		&::palette1[0][0]);

	std::uninitialized_copy_n(
		&::palette1[0][0],
		768,
		&::palette2[0][0]);

	start *= 3;
	end = (end * 3) + 2;

	//
	// fade through intermediate frames
	//
	auto delta = 0;

	for (int i = 0; i < steps; ++i)
	{
		for (int j = start; j <= end; ++j)
		{
			const int delta = palette[j] - ::palette1[0][j];

			::palette2[0][j] =
				static_cast<std::uint8_t>(::palette1[0][j] + ((delta * i) / steps));
		}

		::sdl_filler_color.r = ::palette2[filler_color_index][0];
		::sdl_filler_color.g = ::palette2[filler_color_index][1];
		::sdl_filler_color.b = ::palette2[filler_color_index][2];

		::VL_SetPalette(
			0,
			256,
			&::palette2[0][0]);

		::VL_RefreshScreen();

		if (!::vid_has_vsync)
		{
			::VL_WaitVBL(1);
		}
	}

	::sdl_filler_color.r = palette[(filler_color_index * 3) + 0];
	::sdl_filler_color.g = palette[(filler_color_index * 3) + 1];
	::sdl_filler_color.b = palette[(filler_color_index * 3) + 2];

	//
	// final color
	//
	::VL_SetPalette(0, 256, palette);

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

	::sdl_ui_buffer[offset] = color;
	::sdl_mask_buffer[offset] = !is_transparent;
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
			::sdl_ui_buffer.begin() + offset,
			::sdl_ui_buffer.begin() + offset + count,
			color);

		std::uninitialized_fill(
			::sdl_mask_buffer.begin() + offset,
			::sdl_mask_buffer.begin() + offset + count,
			!is_transparent);
	}
	else
	{
		for (int i = 0; i < height; ++i)
		{
			const auto offset = ((y + i) * ::vga_ref_width) + x;

			std::uninitialized_fill(
				::sdl_ui_buffer.begin() + offset,
				::sdl_ui_buffer.begin() + offset + width,
				color);

			std::uninitialized_fill(
				::sdl_mask_buffer.begin() + offset,
				::sdl_mask_buffer.begin() + offset + width,
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
			::sdl_ui_buffer.begin() + dst_offset);

		std::uninitialized_fill(
			::sdl_mask_buffer.begin() + dst_offset,
			::sdl_mask_buffer.begin() + dst_offset + width,
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
			::sdl_ui_buffer.cbegin() + src_offset,
			::sdl_ui_buffer.cbegin() + src_offset + width,
			::sdl_ui_buffer.begin() + dst_offset);

		std::uninitialized_fill(
			::sdl_mask_buffer.begin() + dst_offset,
			::sdl_mask_buffer.begin() + dst_offset + width,
			true);
	}
}


void JM_VGALinearFill(
	int start,
	int length,
	std::uint8_t fill)
{
	std::uninitialized_fill(
		::sdl_ui_buffer.begin() + start,
		::sdl_ui_buffer.begin() + start + length,
		fill);

	std::uninitialized_fill(
		::sdl_mask_buffer.begin() + start,
		::sdl_mask_buffer.begin() + start + length,
		true);
}

void VL_RefreshScreen()
{
	::sdl_refresh_screen();
}

void VH_UpdateScreen()
{
	::sdl_refresh_screen();
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
	return ::sdl_ui_buffer[(y * ::vga_ref_width) + x];
}

void vl_minimize_fullscreen_window(
	bool value)
{
	if (value)
	{
		::SDL_MinimizeWindow(
			::sdl_window);
	}
	else
	{
		::SDL_RestoreWindow(
			::sdl_window);
	}
}

void vl_update_widescreen()
{
	::sdl_update_widescreen();
}

void vid_set_ui_mask(
	bool value)
{
	::sdl_mask_buffer.fill(
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
			::sdl_mask_buffer.begin() + offset,
			::sdl_mask_buffer.begin() + offset + width,
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
		::sdl_vga_buffer.begin(),
		::sdl_vga_buffer.end(),
		0);
}

void vid_export_ui(
	VgaBuffer& dst_buffer)
{
	dst_buffer = ::sdl_ui_buffer;
}

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent)
{
	::sdl_ui_buffer = src_buffer;
	::vid_set_ui_mask(!is_transparent);
}

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer)
{
	dst_buffer = ::sdl_mask_buffer;
}

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer)
{
	::sdl_mask_buffer = src_buffer;
}

void vid_draw_ui_sprite(
	const int sprite_id,
	const int center_x,
	const int center_y,
	const int new_side)
{
	constexpr auto side = bstone::Sprite::side;

	const auto sprite_ptr = ::vid_sprite_cache.cache(
		sprite_id);

	const auto sprite_width = sprite_ptr->get_width();
	const auto sprite_height = sprite_ptr->get_height();

	const auto left = sprite_ptr->get_left();
	const auto x1 = center_x + ((new_side * (left - (side / 2))) / side);
	const auto x2 = x1 + ((sprite_width * new_side) / side);

	const auto top = sprite_ptr->get_top();
	const auto y1 = center_y + ((new_side * (top - (side / 2))) / side) - 2;
	const auto y2 = y1 + ((sprite_height * new_side) / side);

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
// BBi
