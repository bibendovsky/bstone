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
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include "glm/gtc/matrix_transform.hpp"
#include "id_heads.h"
#include "id_ca.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"
#include "bstone_fixed_point.h"
#include "bstone_hw_texture_manager.h"
#include "bstone_logger.h"
#include "bstone_renderer_manager.h"
#include "bstone_renderer_shader_registry.h"
#include "bstone_sdl_types.h"
#include "bstone_sprite.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_text_writer.h"


extern bool is_full_menu_active;


bool vid_is_hw_ = false;

int bufferofs;

bool screenfaded;

std::uint8_t palette1[bstone::RgbPalette::get_max_color_count()][3];
std::uint8_t palette2[bstone::RgbPalette::get_max_color_count()][3];
std::uint8_t* vga_memory = nullptr;

double vga_height_scale = 0.0;
double vga_width_scale = 0.0;
double vga_wide_scale = 0.0;
int vga_width = 0;
int vga_height = 0;
int vga_area = 0;
int vga_3d_view_top_y = 0;
int vga_3d_view_bottom_y = 0;

bool vid_has_vsync = false;
bool vid_is_hud = false;
bool vid_is_3d = false;
bool vid_is_fizzle_fade = false;
bool vid_is_movie = false;

VidConfiguration vid_configuration_;

bool vid_hw_is_draw_3d_ = false;

bstone::SpriteCache vid_sprite_cache;


// BBi
namespace
{


/*

Window elements.

A--B------------------------------------------------------------------------C
|                                                                           |
|                                                                           |
D  E------------F-----------------------------------------G--------------H  |
|  | FILLER (1) |             TOP STATUS BAR              |  FILLER (1)  |  |
|  I- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  J- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  |            |                                         |              |  |
|  |   FILLER   |                                         |    FILLER    |  |
|  |     OR     |                 3D VIEW                 |      OR      |  |
|  |  3D VIEW   |                                         |   3D VIEW*   |  |
|  |     (2)    |                                         |      (2)     |  |
|  K- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  L- - - - - - +-----------------------------------------+ - - - - - - -+  |
|  |            |                                         |              |  |
|  |   FILLER   |            BOTTOM STATUS BAR            |    FILLER    |  |
|  |     (3)    |                                         |      (3)     |  |
|  |            |                                         |              |  |
|  M------------+-----------------------------------------+--------------+  |
|                                                                           |
|                                                                           |
N---------------------------------------------------------------------------+

Legend:
    - AC - window width;
    - AN - window height;
    - AB - window viewport left width;
    - AD - window viewport top height;
    - EH - screen width;
    - EM - screen height;
    - EF - screen left filler width;
    - GH - screen right filler width;
    - EI - screen top filler height;
    - LM - screen bottom filler height;
    - FG - screen width (4x3);
    - EJ - screen viewport top height;
    - KM - screen viewport bottom height;
    - JK - screen viewport height;
    - IJ/KL - margin height.

Notes:
    (1) Top status bar if stretch is on or filler otherwise.
    (2) 3D view if widescreen is on or filler otherwise.
    (3) Bottom status bar if stretch is on or filler otherwise.

*/


constexpr int default_window_width = 640;
constexpr int default_window_height = 480;


struct WindowElementsDimensions
{
	int screen_width_;
	int screen_height_;

	int window_width_;
	int window_height_;

	int window_viewport_left_width_;
	int window_viewport_right_width_;
	int window_viewport_top_height_;
	int window_viewport_bottom_height_;

	int screen_left_filler_width_;
	int screen_right_filler_width_;
	int screen_top_filler_height_;
	int screen_bottom_filler_height_;
	int screen_width_4x3_;

	int screen_viewport_left_width_;
	int screen_viewport_right_width_;
	int screen_viewport_width_;

	int screen_viewport_top_height_;
	int screen_viewport_bottom_height_;
	int screen_viewport_height_;
}; // WindowElementsDimensions


using SdlPalette = std::array<std::uint32_t, bstone::RgbPalette::get_max_color_count()>;


VgaBuffer vid_ui_buffer;
UiMaskBuffer vid_mask_buffer;


std::string sw_error_message;

bstone::VgaPalette vid_vga_palette;
VgaBuffer sw_vga_buffer;

WindowElementsDimensions downscale_;

SDL_DisplayMode display_mode;
bstone::SdlWindowUPtr sw_window = nullptr;
bstone::SdlRendererUPtr sw_renderer = nullptr;
bstone::SdlPixelFormatUPtr sw_texture_pixel_format = nullptr;
bstone::SdlTextureUPtr sw_screen_texture = nullptr;
bstone::SdlTextureUPtr sw_ui_texture = nullptr;
SdlPalette sw_palette;
SDL_Rect sw_ui_whole_src_rect;
SDL_Rect sw_ui_whole_dst_rect;
SDL_Rect sw_ui_stretched_dst_rect;
SDL_Rect sw_ui_top_src_rect;
SDL_Rect sw_ui_top_dst_rect;
SDL_Rect sw_ui_wide_middle_src_rect;
SDL_Rect sw_ui_wide_middle_dst_rect;
SDL_Rect sw_ui_bottom_src_rect;
SDL_Rect sw_ui_bottom_dst_rect;
std::array<SDL_Rect, 2> sw_filler_ui_rects;
std::array<SDL_Rect, 4> sw_filler_hud_rects;
SDL_Rect sw_screen_dst_rect;
const auto sw_ref_filler_color = SDL_Color{0x00, 0x28, 0x50, 0xFF, };
auto sw_filler_color = SDL_Color{};

const auto filler_color_index = 0xE8;


struct CalculateScreenSizeInputParam
{
	bool is_widescreen_;

	int window_width_;
	int window_height_;
}; // CalculateScreenSizeInputParam


const std::string& vid_get_empty_string()
{
	static const auto& result = std::string{};

	return result;
}

const std::string& vid_get_nearest_value_string()
{
	static const auto& result = std::string{"nearest"};

	return result;
}

const std::string& vid_get_linear_value_string()
{
	static const auto& result = std::string{"linear"};

	return result;
}

const std::string& vid_get_none_value_string()
{
	static const auto& result = std::string{"none"};

	return result;
}

const std::string& vid_get_msaa_value_string()
{
	static const auto& result = std::string{"msaa"};

	return result;
}

const std::string& vid_get_auto_detect_value_string()
{
	static const auto& result = std::string{"auto-detect"};

	return result;
}

const std::string& vid_get_software_value_string()
{
	static const auto& result = std::string{"software"};

	return result;
}

const std::string& vid_get_ogl_2_x_value_string()
{
	static const auto& result = std::string{"ogl_2_x"};

	return result;
}

const std::string& vid_get_renderer_kind_key_name()
{
	static const auto& result = std::string{"vid_renderer_kind"};

	return result;
}

const std::string& vid_get_is_windowed_key_name()
{
	static const auto& result = std::string{"vid_is_windowed"};

	return result;
}

const std::string& vid_get_x_key_name()
{
	static const auto& result = std::string{"vid_x"};

	return result;
}

const std::string& vid_get_y_key_name()
{
	static const auto& result = std::string{"vid_y"};

	return result;
}

const std::string& vid_get_width_key_name()
{
	static const auto& result = std::string{"vid_width"};

	return result;
}

const std::string& vid_get_height_key_name()
{
	static const auto& result = std::string{"vid_height"};

	return result;
}

const std::string& vid_get_is_widescreen_key_name()
{
	static const auto& result = std::string{"vid_is_widescreen"};

	return result;
}

const std::string& vid_get_vsync_key_name()
{
	static const auto& result = std::string{"vid_vsync"};

	return result;
}

const std::string& vid_get_is_ui_stretched_key_name()
{
	static const auto& result = std::string{"vid_is_ui_stretched"};

	return result;
}

const std::string& vid_get_hw_2d_texture_filter_key_name()
{
	static const auto& result = std::string{"vid_hw_2d_texture_filter"};

	return result;
}

const std::string& vid_get_hw_3d_texture_image_filter_key_name()
{
	static const auto& result = std::string{"vid_hw_3d_texture_image_filter"};

	return result;
}

const std::string& vid_get_hw_3d_texture_mipmap_filter_key_name()
{
	static const auto& result = std::string{"vid_hw_3d_texture_mipmap_filter"};

	return result;
}

const std::string& vid_get_hw_3d_texture_anisotropy_key_name()
{
	static const auto& result = std::string{"vid_hw_3d_texture_anisotropy"};

	return result;
}

const std::string& vid_get_hw_3d_texture_anisotropy_value_key_name()
{
	static const auto& result = std::string{"vid_hw_3d_texture_anisotropy_value"};

	return result;
}

const std::string& vid_get_hw_aa_kind_key_name()
{
	static const auto& result = std::string{"vid_hw_aa_kind"};

	return result;
}

const std::string& vid_get_hw_aa_value_key_name()
{
	static const auto& result = std::string{"vid_hw_aa_value"};

	return result;
}

const std::string& vid_get_is_downscale_key_name()
{
	static const auto& result = std::string{"vid_is_downscale"};

	return result;
}

const std::string& vid_get_downscale_width_key_name()
{
	static const auto& result = std::string{"vid_downscale_width"};

	return result;
}

const std::string& vid_get_downscale_height_key_name()
{
	static const auto& result = std::string{"vid_downscale_height"};

	return result;
}

const std::string& vid_get_hw_downscale_blit_filter_key_name()
{
	static const auto& result = std::string{"vid_hw_downscale_blit_filter"};

	return result;
}

const std::string& vid_get_hw_dbg_draw_all_key_name()
{
	static const auto& result = std::string{"vid_hw_dbg_draw_all"};

	return result;
}


int vid_align_dimension(
	const int dimension)
{
	const auto alignment = 2;

	return ((dimension + (alignment - 1)) / alignment) * alignment;
}

void vid_configuration_fix_window_dimension(
	VidConfiguration::IntModValue& dimension,
	const int min_value,
	const int default_value)
{
	if (dimension <= 0)
	{
		dimension = default_value;
	}

	if (dimension < min_value)
	{
		dimension = min_value;
	}
}

void vid_configuration_fix_window_width()
{
	::vid_configuration_fix_window_dimension(
		::vid_configuration_.width_,
		::vga_ref_width,
		::display_mode.w
	);
}

void vid_configuration_fix_window_height()
{
	::vid_configuration_fix_window_dimension(
		::vid_configuration_.height_,
		::vga_ref_height_4x3,
		::display_mode.h
	);
}

void vid_configuration_fix_window_size()
{
	::vid_configuration_fix_window_width();
	::vid_configuration_fix_window_height();
}

void vid_configuration_adjust_window_position()
{
	auto window_x = *::vid_configuration_.x_;
	auto window_y = *::vid_configuration_.y_;

	if (window_x < 0)
	{
		window_x = 0;
	}

	if (window_y < 0)
	{
		window_y = 0;
	}

	::vid_configuration_.x_ = window_x;
	::vid_configuration_.y_ = window_y;
}

void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	WindowElementsDimensions& dst_param)
{
	const auto window_width = src_param.window_width_;
	const auto window_height = src_param.window_height_;

	assert(window_width >= ::vga_ref_width);
	assert(window_height >= ::vga_ref_height_4x3);

	auto screen_width = (window_width / 2) * 2;
	auto screen_height = (window_height / 2) * 2;

	const auto min_r_ratio = 3.0 / 4.0;
	const auto r_ratio = static_cast<double>(window_height) / static_cast<double>(window_width);

	if (r_ratio > min_r_ratio)
	{
		screen_height = (((screen_width * 3) / 4) / 2) * 2;
	}

	//
	const auto window_viewport_left_width = (window_width - screen_width) / 2;
	const auto window_viewport_right_width = window_width - screen_width - window_viewport_left_width;
	const auto window_viewport_top_height = (window_height - screen_height) / 2;
	const auto window_viewport_bottom_height = window_height - screen_height - window_viewport_top_height;

	//
	const auto screen_width_4x3 = (((screen_height * 4) / 3) / 2) * 2;

	const auto screen_left_filler_width = (screen_width - screen_width_4x3) / 2;
	const auto screen_right_filler_width = screen_width - screen_width_4x3 - screen_left_filler_width;

	const auto screen_top_filler_height = (::ref_top_bar_height * screen_height) / ::vga_ref_height;
	const auto screen_bottom_filler_height = (::ref_bottom_bar_height * screen_height) / ::vga_ref_height;

	auto screen_viewport_left_width = 0;
	auto screen_viewport_right_width = 0;
	auto screen_viewport_width = 0;

	if (src_param.is_widescreen_)
	{
		screen_viewport_width = screen_width;
	}
	else
	{
		screen_viewport_left_width = screen_left_filler_width;
		screen_viewport_right_width = screen_width - screen_width_4x3 - screen_left_filler_width;
		screen_viewport_width = screen_width_4x3;
	}

	const auto screen_viewport_top_height = ((::ref_top_bar_height + ::ref_3d_margin) * screen_height) / ::vga_ref_height;
	const auto screen_viewport_bottom_height = ((::ref_bottom_bar_height + ::ref_3d_margin) * screen_height) / ::vga_ref_height;
	const auto screen_viewport_height = screen_height - screen_viewport_top_height - screen_viewport_bottom_height;

	//
	dst_param.window_width_ = window_width;
	dst_param.window_height_ = window_height;

	dst_param.window_viewport_left_width_ = window_viewport_left_width;
	dst_param.window_viewport_right_width_ = window_viewport_right_width;
	dst_param.window_viewport_top_height_ = window_viewport_top_height;
	dst_param.window_viewport_bottom_height_ = window_viewport_bottom_height;

	dst_param.screen_width_ = screen_width;
	dst_param.screen_height_ = screen_height;

	dst_param.screen_width_4x3_ = screen_width_4x3;

	dst_param.screen_left_filler_width_ = screen_left_filler_width;
	dst_param.screen_right_filler_width_ = screen_right_filler_width;

	dst_param.screen_top_filler_height_ = screen_top_filler_height;
	dst_param.screen_bottom_filler_height_ = screen_bottom_filler_height;

	dst_param.screen_viewport_left_width_ = screen_viewport_left_width;
	dst_param.screen_viewport_right_width_ = screen_viewport_right_width;
	dst_param.screen_viewport_width_ = screen_viewport_width;

	dst_param.screen_viewport_top_height_ = screen_viewport_top_height;
	dst_param.screen_viewport_bottom_height_ = screen_viewport_bottom_height;
	dst_param.screen_viewport_height_ = screen_viewport_height;
}

CalculateScreenSizeInputParam vid_create_downscale_param()
{
	auto downscale_window_width = static_cast<int>(::vid_configuration_.downscale_width_);
	auto downscale_window_height = static_cast<int>(::vid_configuration_.downscale_height_);

	if (!::vid_configuration_.is_downscale_)
	{
		downscale_window_width = ::vid_configuration_.width_;
	}
	else if (downscale_window_width <= 0)
	{
		downscale_window_width = ::vid_configuration_.width_;
	}
	else if (downscale_window_width < ::vga_ref_width)
	{
		downscale_window_width = ::vga_ref_width;
	}
	else if (downscale_window_width > ::vid_configuration_.width_)
	{
		downscale_window_width = ::vid_configuration_.width_;
	}

	if (!::vid_configuration_.is_downscale_)
	{
		downscale_window_height = ::vid_configuration_.height_;
	}
	else if (downscale_window_height <= 0)
	{
		downscale_window_height = ::vid_configuration_.height_;
	}
	else if (downscale_window_height < ::vga_ref_height_4x3)
	{
		downscale_window_height = ::vga_ref_height_4x3;
	}
	else if (downscale_window_height > ::vid_configuration_.height_)
	{
		downscale_window_height = ::vid_configuration_.height_;
	}


	const auto ref_absolute_epsilon = 1E-6;

	const auto width_ratio = static_cast<double>(::vid_configuration_.width_) / static_cast<double>(downscale_window_width);
	const auto height_ratio = static_cast<double>(::vid_configuration_.height_) / static_cast<double>(downscale_window_height);

	const auto absolute_epsilon = std::abs(width_ratio - height_ratio);

	if (absolute_epsilon > ref_absolute_epsilon)
	{
		downscale_window_width = ::vid_configuration_.width_;
		downscale_window_height = ::vid_configuration_.height_;
	}

	auto result = CalculateScreenSizeInputParam{};
	result.is_widescreen_ = ::vid_configuration_.is_widescreen_;
	result.window_width_ = downscale_window_width;
	result.window_height_ = downscale_window_height;

	return result;
}

void vid_dimensions_vga_calculate()
{
	::vga_width = ::downscale_.screen_viewport_width_;
	::vga_height = ::vid_align_dimension((10 * ::downscale_.screen_height_) / 12);

	::vga_width_scale = static_cast<double>(::vga_width) / static_cast<double>(::vga_ref_width);
	::vga_height_scale = static_cast<double>(::vga_height) / static_cast<double>(::vga_ref_height);

	::vga_wide_scale =
		static_cast<double>(::vga_ref_height * ::vga_width) /
		static_cast<double>(::vga_ref_width * ::vga_height);

	::vga_area = ::vga_width * ::vga_height;
}

std::string vid_get_window_title()
{
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

	return title;
}

void vid_read_is_windowed_cl_configuration()
{
	// "vid_is_windowed"
	//
	const auto is_windowed_str = ::g_args.get_option_value(::vid_get_is_windowed_key_name());

	auto is_windowed_value = 0;

	if (!bstone::StringHelper::string_to_int(is_windowed_str, is_windowed_value))
	{
		return;
	}

	::vid_configuration_.is_windowed_ = (is_windowed_value != 0);
}

void vid_read_window_offset_cl_configuration(
	const std::string& option_name,
	VidConfiguration::IntModValue& offset)
{
	const auto& offset_str = ::g_args.get_option_value(option_name);

	auto offset_value = 0;

	if (!bstone::StringHelper::string_to_int(offset_str, offset_value))
	{
		return;
	}

	::vid_configuration_.is_custom_position_ = true;

	offset = offset_value;
}

void vid_read_window_offset_x_cl_configuration()
{
	::vid_read_window_offset_cl_configuration(::vid_get_x_key_name(), ::vid_configuration_.x_);
}

void vid_read_window_offset_y_cl_configuration()
{
	::vid_read_window_offset_cl_configuration(::vid_get_y_key_name(), ::vid_configuration_.y_);
}

void vid_read_window_dimension_cl_configuration(
	const std::string& option_name,
	VidConfiguration::IntModValue& dimension)
{
	const auto& dimension_str = ::g_args.get_option_value(option_name);

	auto dimension_value = 0;

	if (bstone::StringHelper::string_to_int(dimension_str, dimension_value))
	{
		dimension = dimension_value;
	}
}

void vid_read_window_width_cl_configuration()
{
	::vid_read_window_dimension_cl_configuration(::vid_get_width_key_name(), ::vid_configuration_.width_);
}

void vid_read_window_height_cl_configuration()
{
	::vid_read_window_dimension_cl_configuration(::vid_get_height_key_name(), ::vid_configuration_.height_);
}

void vid_read_vsync_cl_configuration()
{
	const auto vsync_str = ::g_args.get_option_value(::vid_get_vsync_key_name());

	auto vsync_value = 0;

	if (!bstone::StringHelper::string_to_int(vsync_str, vsync_value))
	{
		return;
	}

	::vid_configuration_.is_vsync_ = (vsync_value != 0);
}

const std::string& vid_get_vid_string()
{
	static const auto result = std::string{"[VID]"};

	return result;
}

const std::string& vid_get_hw_string()
{
	static const auto result = std::string{"[HW]"};

	return result;
}

const std::string& vid_get_dbg_string()
{
	static const auto result = std::string{"[DBG]"};

	return result;
}

void vid_quit(
	const std::string& error_message)
{
	::Quit(::vid_get_vid_string() + ' ' + error_message);
}

void vid_quit_with_sdl_error(
	const std::string& error_message)
{
	auto message = std::string{};
	message += ::vid_get_vid_string() + ' ' + error_message;

	const auto sdl_error_message = ::SDL_GetError();

	if (sdl_error_message != nullptr)
	{
		message += ' ';
		message += sdl_error_message;
	}

	::Quit(message);
}

void vid_log()
{
	bstone::logger_->write();
}

void vid_log(
	const bstone::LoggerMessageKind message_kind,
	const std::string& message)
{
	bstone::logger_->write(message_kind, ::vid_get_vid_string() + ' ' + message);
}

void vid_log(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::information,
		::vid_get_vid_string() + ' ' + message
	);
}

void vid_log_warning(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::warning,
		::vid_get_vid_string() + ' ' + message
	);
}

void vid_log_error(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::error,
		::vid_get_vid_string() + ' ' + message
	);
}

void vid_log_error(
	const std::string& required_message,
	const std::string& optional_message)
{
	::vid_log_error(required_message);

	if (!optional_message.empty())
	{
		::vid_log_error(optional_message);
	}
}

void vid_hw_log(
	const std::string& message)
{
	bstone::logger_->write(::vid_get_vid_string() + ::vid_get_hw_string() + ' ' + message);
}

void vid_hw_dbg_log(
	const std::string& message)
{
	bstone::logger_->write(::vid_get_vid_string() + ::vid_get_hw_string() + ::vid_get_dbg_string() + ' ' + message);
}

void vid_get_current_display_mode()
{
	const auto sdl_result = ::SDL_GetDesktopDisplayMode(0, &::display_mode);

	if (sdl_result != 0)
	{
		::vid_quit_with_sdl_error("Failed to get a display mode.");
	}
}

void vid_read_cl_configuration()
{
	::vid_configuration_.is_custom_position_ = false;


	// "vid_is_windowed"
	//
	::vid_read_is_windowed_cl_configuration();

	// "vid_x"
	//
	::vid_read_window_offset_x_cl_configuration();

	// "vid_y"
	//
	::vid_read_window_offset_y_cl_configuration();

	// "vid_width"
	//
	::vid_read_window_width_cl_configuration();

	// "vid_height"
	//
	::vid_read_window_height_cl_configuration();

	// "vid_vsync"
	//
	::vid_read_vsync_cl_configuration();
}

void vid_hw_read_cl_configuration()
{
	// Option "vid_hw_dbg_draw_all"
	//
	::vid_configuration_.hw_dbg_draw_all_ = ::g_args.has_option(::vid_get_hw_dbg_draw_all_key_name());
}

const std::string& vid_to_string(
	const bool value)
{
	static const auto false_string = std::string{"false"};
	static const auto true_string = std::string{"true"};

	return value ? true_string : false_string;
}

std::string vid_to_string(
	const int value)
{
	return std::to_string(value);
}

const std::string& vid_to_string(
	const bstone::RendererFilterKind filter_kind)
{
	static const auto invalid_string = std::string{"?"};

	switch (filter_kind)
	{
		case bstone::RendererFilterKind::nearest:
			return vid_get_nearest_value_string();

		case bstone::RendererFilterKind::linear:
			return vid_get_linear_value_string();

		default:
			return invalid_string;
	}
}

const std::string& vid_to_string(
	const bstone::RendererAaKind aa_kind)
{
	static const auto invalid_string = std::string{"?"};

	switch (aa_kind)
	{
		case bstone::RendererAaKind::ms:
			return vid_get_msaa_value_string();

		case bstone::RendererAaKind::none:
			return vid_get_none_value_string();

		default:
			return invalid_string;
	}
}

const std::string& vid_to_string(
	const bstone::RendererPath renderer_kind)
{
	static const auto invalid_string = std::string{"?"};
	static const auto ogl_2_x_string = std::string{"OpenGL 2.x"};

	switch (renderer_kind)
	{
		case bstone::RendererPath::none:
			return vid_get_none_value_string();

		case bstone::RendererPath::auto_detect:
			return vid_get_auto_detect_value_string();

		case bstone::RendererPath::software:
			return vid_get_software_value_string();

		case bstone::RendererPath::ogl_2_x:
			return ogl_2_x_string;


		default:
			return invalid_string;
	}
}

void vid_log_common_configuration()
{
	::vid_log();
	::vid_log("--------------------");
	::vid_log("Common configuration");
	::vid_log("--------------------");

	::vid_log("Renderer: " + ::vid_to_string(::vid_configuration_.renderer_kind_));

	::vid_log("Is windowed: " + ::vid_to_string(::vid_configuration_.is_windowed_));
	::vid_log("Window offset by X: " + ::vid_to_string(::vid_configuration_.x_));
	::vid_log("Window offset by Y: " + ::vid_to_string(::vid_configuration_.y_));
	::vid_log("Window width: " + ::vid_to_string(::vid_configuration_.width_));
	::vid_log("Window height: " + ::vid_to_string(::vid_configuration_.height_));

	::vid_log("Is UI stretched: " + ::vid_to_string(::vid_configuration_.is_ui_stretched_));
	::vid_log("Is widescreen: " + ::vid_to_string(::vid_configuration_.is_widescreen_));

	::vid_hw_dbg_log("Draw all: " + ::vid_to_string(::vid_configuration_.hw_dbg_draw_all_));

	::vid_log("Is downscale: " + ::vid_to_string(::vid_configuration_.is_downscale_));
	::vid_log("Downscale width: " + ::vid_to_string(::vid_configuration_.downscale_width_));
	::vid_log("Downscale height: " + ::vid_to_string(::vid_configuration_.downscale_height_));
	::vid_log("Downscale blit filter: " + ::vid_to_string(::vid_configuration_.hw_downscale_blit_filter_));

	::vid_hw_log("2D texture filter: " + ::vid_to_string(::vid_configuration_.hw_2d_texture_filter_));

	::vid_hw_log("3D texture image filter: " + ::vid_to_string(::vid_configuration_.hw_3d_texture_image_filter_));
	::vid_hw_log("3D texture mipmap filter: " + ::vid_to_string(::vid_configuration_.hw_3d_texture_mipmap_filter_));

	::vid_hw_log("Texture anisotropy: " + ::vid_to_string(::vid_configuration_.hw_3d_texture_anisotropy_));
	::vid_hw_log("Texture anisotropy value: " + ::vid_to_string(::vid_configuration_.hw_3d_texture_anisotropy_value_));

	::vid_hw_log("Anti-aliasing kind: " + ::vid_to_string(::vid_configuration_.hw_aa_kind_));
	::vid_hw_log("Anti-aliasing value: " + ::vid_to_string(::vid_configuration_.hw_aa_value_));

	::vid_log("--------------------");
}

void vid_sw_log_sdl_error(
	const std::string& message)
{
	auto error_message = std::string{};

	error_message = "[VID] ";
	error_message += message;
	bstone::logger_->write_error(error_message);

	error_message = "[VID] ";
	error_message += ::SDL_GetError();
	bstone::logger_->write_error(error_message);
}

void vid_common_initialize()
{
	::vid_get_current_display_mode();
	::vid_read_cl_configuration();
	::vid_hw_read_cl_configuration();
	::vid_configuration_adjust_window_position();
	::vid_configuration_fix_window_size();

	::vid_log_common_configuration();
}


void sw_initialize_vga_buffer()
{
	const auto area = 2 * ::vga_width * ::vga_height;

	::sw_vga_buffer.resize(area);

	::vga_memory = ::sw_vga_buffer.data();
}

void sw_initialize_ui_buffer()
{
	const auto area = ::vga_ref_width * ::vga_ref_height;

	::vid_ui_buffer.resize(area);
}

bool sw_initialize_window()
{
	::vid_log("Creating a window.");


	auto window_x = *::vid_configuration_.x_;
	auto window_y = *::vid_configuration_.y_;

	if (!::vid_configuration_.is_custom_position_)
	{
		window_x = SDL_WINDOWPOS_CENTERED;
		window_y = SDL_WINDOWPOS_CENTERED;
	}

	auto window_flags = Uint32{
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_HIDDEN |
		0};

	if (!::vid_configuration_.is_windowed_)
	{
		window_flags |=
			SDL_WINDOW_BORDERLESS |
			SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

#ifdef __vita__
	window_flags = SDL_WINDOW_SHOWN;
#endif

	const auto title = ::vid_get_window_title();

	::sw_window = bstone::SdlWindowUPtr{::SDL_CreateWindow(
		title.c_str(),
		window_x,
		window_y,
		::vid_configuration_.width_,
		::vid_configuration_.height_,
		window_flags
	)};

	if (!::sw_window)
	{
		::vid_sw_log_sdl_error("Failed to create a window.");

		return false;
	}

	return true;
}

bool sw_initialize_renderer()
{
	::vid_log();
	::vid_log("Initializing renderer.");


	bool is_succeed = true;


	if (is_succeed)
	{
		::vid_log("Available renderer drivers:");

		const auto driver_count = ::SDL_GetNumRenderDrivers();

		for (int i = 0; i < driver_count; ++i)
		{
			SDL_RendererInfo info;

			const auto sdl_result = ::SDL_GetRenderDriverInfo(i, &info);

			::vid_log(std::to_string(i + 1) + ". " + info.name);
		}
	}


	std::uint32_t renderer_flags = 0;
	const char* renderer_driver = nullptr;

	if (is_succeed)
	{
		if (!vid_configuration_.is_vsync_)
		{
			::vid_log("Skipping VSync.");
		}
		else
		{
			renderer_flags = SDL_RENDERER_PRESENTVSYNC;

			::vid_log("Using VSync.");
		}
	}

	if (is_succeed)
	{
		::vid_log("Creating renderer.");

		::sw_renderer = bstone::SdlRendererUPtr{::SDL_CreateRenderer(
			::sw_window.get(),
			-1,
			renderer_flags
		)};

		if (!::sw_renderer)
		{
			is_succeed = false;

			::vid_sw_log_sdl_error("Failed to create a renderer.");
		}
	}


	auto renderer_info = SDL_RendererInfo{};

	if (is_succeed)
	{
		::vid_log("Quering renderer for info.");

		auto sdl_result = ::SDL_GetRendererInfo(
			::sw_renderer.get(),
			&renderer_info);

		if (sdl_result != 0)
		{
			is_succeed = false;

			::vid_sw_log_sdl_error("Failed to query the renderer.");
		}
	}


	if (is_succeed)
	{
		if (renderer_driver)
		{
			if (::SDL_strcasecmp(renderer_driver, renderer_info.name) != 0)
			{
				::vid_log_warning("Unexpected renderer is selected: \"" + std::string{renderer_info.name} + "\".");
			}
		}
		else
		{
			::vid_log("Current renderer: \"" + std::string{renderer_info.name} + "\".");
		}
	}


	std::uint32_t pixel_format = SDL_PIXELFORMAT_UNKNOWN;

	if (is_succeed)
	{
		::vid_log("Looking up for a texture pixel format.");

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
			::vid_log_warning("Falling back to a predefined pixel format.");

			pixel_format = SDL_PIXELFORMAT_ARGB8888;
		}


		::vid_log("Allocating a texture pixel format.");

		::sw_texture_pixel_format = bstone::SdlPixelFormatUPtr{::SDL_AllocFormat(pixel_format)};

		if (!::sw_texture_pixel_format)
		{
			is_succeed = false;

			::vid_sw_log_sdl_error("Failed to allocate a texture pixel format.");
		}
	}

	return is_succeed;
}

bool sw_initialize_screen_texture()
{
	::vid_log("Creating screen texture.");

	::sw_screen_texture = bstone::SdlTextureUPtr{::SDL_CreateTexture(
		::sw_renderer.get(),
		::sw_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_width,
		::vga_height
	)};

	if (!::sw_screen_texture)
	{
		::vid_sw_log_sdl_error("Failed to create a screen texture.");

		return false;
	}

	return true;
}

bool sw_initialize_ui_texture()
{
	::vid_log("Creating UI texture.");

	::sw_ui_texture = bstone::SdlTextureUPtr{::SDL_CreateTexture(
		::sw_renderer.get(),
		::sw_texture_pixel_format->format,
		SDL_TEXTUREACCESS_STREAMING,
		::vga_ref_width,
		::vga_ref_height
	)};

	if (!::sw_ui_texture)
	{
		::vid_sw_log_sdl_error("Failed to create an UI texture.");

		return false;
	}

	return true;
}

bool sw_initialize_textures()
{
	::vid_log();
	::vid_log("Initializing textures.");


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
			::sw_texture_pixel_format.get(),
			(255 * vga_color.r) / 63,
			(255 * vga_color.g) / 63,
			(255 * vga_color.b) / 63);
	}
}

void sw_update_viewport()
{
	auto sdl_result = ::SDL_RenderSetLogicalSize(
		::sw_renderer.get(),
		::downscale_.window_width_,
		::downscale_.window_height_);

	if (sdl_result != 0)
	{
		::vid_sw_log_sdl_error("Failed to update a viewport.");
	}
}

void sw_initialize_palette()
{
	::vid_vga_palette.fill({});

	::sw_update_palette(
		0,
		bstone::RgbPalette::get_max_color_count());
}

void sw_calculate_dimensions()
{
	auto src_param = ::vid_create_downscale_param();

	::vid_calculate_window_elements_dimensions(src_param, ::downscale_);

	::vid_dimensions_vga_calculate();


	// UI whole rect
	//
	::sw_ui_whole_src_rect = SDL_Rect
	{
		0,
		0,
		::vga_ref_width,
		::vga_ref_height,
	};

	::sw_ui_whole_dst_rect = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_left_filler_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_width_4x3_,
		::downscale_.screen_height_,
	};


	// UI stretched rect
	//
	::sw_ui_stretched_dst_rect = SDL_Rect
	{
		::downscale_.window_viewport_left_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_width_,
		::downscale_.screen_height_,
	};


	// UI top rect
	//
	::sw_ui_top_src_rect = SDL_Rect
	{
		0,
		0,
		::vga_ref_width,
		::ref_top_bar_height,
	};

	::sw_ui_top_dst_rect = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_left_filler_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_width_4x3_,
		::downscale_.screen_top_filler_height_,
	};


	// UI middle rect (stretched to full width)
	//
	::sw_ui_wide_middle_src_rect = SDL_Rect
	{
		0,
		::ref_view_top_y,
		::vga_ref_width,
		::ref_view_height,
	};

	::sw_ui_wide_middle_dst_rect = SDL_Rect
	{
		::downscale_.window_viewport_left_width_,
		::downscale_.window_viewport_top_height_ + ::downscale_.screen_top_filler_height_,
		::downscale_.screen_width_,
		::downscale_.screen_height_,
	};


	// UI bottom rect
	//
	::sw_ui_bottom_src_rect = SDL_Rect
	{
		0,
		::ref_view_bottom_y + 1,
		::vga_ref_width,
		::ref_bottom_bar_height,
	};

	::sw_ui_bottom_dst_rect = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_left_filler_width_,
		::downscale_.window_viewport_top_height_ + ::downscale_.screen_height_ - ::downscale_.screen_bottom_filler_height_,
		::downscale_.screen_width_4x3_,
		::downscale_.screen_bottom_filler_height_,
	};


	// UI left bar
	::sw_filler_ui_rects[0] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_left_filler_width_,
		::downscale_.screen_height_,
	};

	// UI right bar
	::sw_filler_ui_rects[1] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_width_ - ::downscale_.screen_left_filler_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_left_filler_width_,
		::downscale_.screen_height_,
	};

	// HUD upper left rect
	::sw_filler_hud_rects[0] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_left_filler_width_,
		::downscale_.screen_top_filler_height_,
	};

	// HUD upper right rect
	::sw_filler_hud_rects[1] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_width_ - ::downscale_.screen_right_filler_width_,
		::downscale_.window_viewport_top_height_,
		::downscale_.screen_right_filler_width_,
		::downscale_.screen_top_filler_height_,
	};

	// HUD lower left rect
	::sw_filler_hud_rects[2] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_,
		::downscale_.window_viewport_top_height_ + ::downscale_.screen_height_ - ::downscale_.screen_bottom_filler_height_,
		::downscale_.screen_left_filler_width_,
		::downscale_.screen_bottom_filler_height_,
	};

	// HUD lower right rect
	::sw_filler_hud_rects[3] = SDL_Rect
	{
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_width_ - ::downscale_.screen_right_filler_width_,
		::downscale_.window_viewport_top_height_ + ::downscale_.screen_height_ - ::downscale_.screen_bottom_filler_height_,
		::downscale_.screen_right_filler_width_,
		::downscale_.screen_bottom_filler_height_,
	};

	// Filler color.
	::sw_filler_color = SDL_Color
	{
		::vgapal[(filler_color_index * 3) + 0],
		::vgapal[(filler_color_index * 3) + 1],
		::vgapal[(filler_color_index * 3) + 2],
		0xFF,
	};

	// Screen destination rect.

	const auto screen_left = (
		::vid_configuration_.is_widescreen_ ?
		0 :
		::downscale_.window_viewport_left_width_ + ::downscale_.screen_left_filler_width_);

	const auto screen_top = ::downscale_.window_viewport_top_height_;
	const auto screen_width = (::vid_configuration_.is_widescreen_ ? ::downscale_.screen_width_ : ::downscale_.screen_width_4x3_);
	const auto screen_height = ::downscale_.screen_height_;

	::sw_screen_dst_rect = SDL_Rect
	{
		screen_left,
		screen_top,
		screen_width,
		screen_height,
	};
}

void sw_initialize_video()
{
	::vid_log();
	::vid_log("Software video system");
	::vid_log("---------------------");

	::vid_common_initialize();
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
		::SDL_ShowWindow(::sw_window.get());
		::in_grab_mouse(true);
	}
	else
	{
		::Quit(::sw_error_message);
	}
}

void sw_uninitialize_screen_texture()
{
	::sw_screen_texture = nullptr;
}

void sw_uninitialize_ui_texture()
{
	::sw_ui_texture = nullptr;
}

void sw_uninitialize_vga_buffer()
{
	::sw_vga_buffer.clear();
	::sw_vga_buffer.shrink_to_fit();

	::vga_memory = nullptr;
}

void sw_uninitialize_video()
{
	::sw_texture_pixel_format = nullptr;
	::sw_uninitialize_screen_texture();
	::sw_uninitialize_ui_texture();
	::sw_renderer = nullptr;
	::sw_window = nullptr;

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
			::sw_screen_texture.get(),
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to lock a screen texture.");
		}

		auto dst_pixels = static_cast<std::uint32_t*>(dst_raw_pixels);

		for (int y = 0; y < ::vga_height; ++y)
		{
			const auto src_line = &src_pixels[y * src_pitch];
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < ::vga_width; ++x)
			{
				dst_line[x] = sw_palette[src_line[x]];
			}
		}

		::SDL_UnlockTexture(::sw_screen_texture.get());
	}


	// 2D stuff
	//
	{
		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = ::SDL_LockTexture(
			::sw_ui_texture.get(),
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to lock an UI texture.");
		}

		const auto alpha_0_mask = ~sw_texture_pixel_format->Amask;

		auto dst_pixels = static_cast<std::uint32_t*>(dst_raw_pixels);

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

		::SDL_UnlockTexture(::sw_ui_texture.get());
	}


	// Clear all
	//
	sdl_result = ::SDL_RenderClear(sw_renderer.get());

	if (sdl_result != 0)
	{
		::vid_quit_with_sdl_error("Failed to clear a render target.");
	}


	// Copy HUD+3D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_RenderCopy(
			sw_renderer.get(),
			sw_screen_texture.get(),
			nullptr,
			&::sw_screen_dst_rect);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to copy a screen texture on a render target.");
		}
	}


	// Use filler if necessary
	//
	if (!::vid_configuration_.is_ui_stretched_)
	{
		const auto is_hud = ::vid_is_hud;

		auto fill_color = SDL_Color{};

		if (!::vid_is_movie)
		{
			fill_color = ::sw_filler_color;
		}

		sdl_result = ::SDL_SetRenderDrawColor(
			sw_renderer.get(),
			fill_color.r,
			fill_color.g,
			fill_color.b,
			0xFF);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to set draw color.");
		}

		if (is_hud)
		{
			sdl_result = ::SDL_RenderFillRects(sw_renderer.get(), ::sw_filler_hud_rects.data(), 4);
		}
		else
		{
			sdl_result = ::SDL_RenderFillRects(sw_renderer.get(), ::sw_filler_ui_rects.data(), 2);
		}

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to draw fillers.");
		}
	}


	// Copy 2D stuff
	//
	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sw_ui_texture.get(),
			SDL_BLENDMODE_BLEND);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to set blend mode for an UI texture.");
		}
	}

	if (!::vid_configuration_.is_ui_stretched_)
	{
		if (::vid_is_fizzle_fade)
		{
			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer.get(),
					::sw_ui_texture.get(),
					&::sw_ui_top_src_rect,
					&::sw_ui_top_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer.get(),
					::sw_ui_texture.get(),
					&::sw_ui_wide_middle_src_rect,
					&::sw_ui_wide_middle_dst_rect);
			}

			if (sdl_result == 0)
			{
				sdl_result = ::SDL_RenderCopy(
					::sw_renderer.get(),
					::sw_ui_texture.get(),
					&::sw_ui_bottom_src_rect,
					&::sw_ui_bottom_dst_rect);
			}
		}
		else
		{
			sdl_result = ::SDL_RenderCopy(
				::sw_renderer.get(),
				::sw_ui_texture.get(),
				nullptr,
				&::sw_ui_whole_dst_rect);
		}
	}
	else
	{
		sdl_result = ::SDL_RenderCopy(
			::sw_renderer.get(),
			::sw_ui_texture.get(),
			nullptr,
			&::sw_ui_stretched_dst_rect);
	}

	if (sdl_result != 0)
	{
		::vid_quit_with_sdl_error("Failed to copy an UI texture on render target.");
	}

	if (::vid_is_hud)
	{
		sdl_result = ::SDL_SetTextureBlendMode(
			::sw_ui_texture.get(),
			SDL_BLENDMODE_NONE);

		if (sdl_result != 0)
		{
			::vid_quit_with_sdl_error("Failed to set blend mode for an UI texture.");
		}
	}


	// Present
	//
	::SDL_RenderPresent(sw_renderer.get());
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
// Hardware accelerated renderer (HW).
//

enum class HwVertexAttributeLocationId :
	unsigned char
{
	none,
	position,
	color,
	texture_coordinates,
}; // HwVertexAttributeLocationId


const auto hw_3d_map_dimension_f = static_cast<float>(MAPSIZE);
const auto hw_3d_map_dimension_d = static_cast<double>(MAPSIZE);
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

constexpr auto hw_3d_sides_per_wall = 4;
constexpr auto hw_3d_indices_per_wall_side = 6;
constexpr auto hw_3d_vertices_per_wall_side = 4;

constexpr auto hw_3d_max_wall_sides_indices = 0x10000;

constexpr auto hw_3d_sides_per_door = 2;
constexpr auto hw_3d_halves_per_side = 2;
constexpr auto hw_3d_halves_per_door = ::hw_3d_sides_per_door * ::hw_3d_halves_per_side;
constexpr auto hw_3d_vertices_per_door_half = 4;
constexpr auto hw_3d_vertices_per_door = ::hw_3d_sides_per_door * ::hw_3d_vertices_per_door_half;

constexpr auto hw_3d_indices_per_door_half = 6;
constexpr auto hw_3d_indices_per_door = 2 * hw_3d_indices_per_door_half;
constexpr auto hw_3d_indices_per_door_side = 2 * hw_3d_indices_per_door;

constexpr auto hw_3d_max_door_sides_vertices = MAXDOORS * hw_3d_vertices_per_door;
constexpr auto hw_3d_max_door_sides_indices = MAXDOORS * ::hw_3d_indices_per_door_side;


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

constexpr auto hw_3d_cloaked_actor_alpha_u8 = std::uint8_t{0x50};

constexpr auto hw_common_command_buffer_initial_size = 1'024;
constexpr auto hw_common_command_buffer_resize_delta_size = 1'024;

constexpr auto hw_2d_command_buffer_initial_size = 4'096;
constexpr auto hw_2d_command_buffer_resize_delta_size = 4'096;

constexpr auto hw_3d_command_buffer_initial_size = 16'384;
constexpr auto hw_3d_command_buffer_resize_delta_size = 16'384;


template<
	typename T,
	HwVertexAttributeLocationId TLocationId,
	typename = int>
struct HwVertexHasAttribute :
	std::false_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttributeLocationId::position,
	decltype(static_cast<void>(T::xyz_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttributeLocationId::color,
	decltype(static_cast<void>(T::rgba_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttributeLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute


template<
	typename T,
	HwVertexAttributeLocationId TLocationId,
	typename = int>
struct HwVertexAttributeTraits
{
	static constexpr auto is_valid = false;
	static constexpr auto offset = -1;
	static constexpr auto stride = -1;
}; // HwVertexAttributeTraits

template<typename T>
struct HwVertexAttributeTraits<
	T,
	HwVertexAttributeLocationId::position,
	decltype(static_cast<void>(T::xyz_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, xyz_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits

template<typename T>
struct HwVertexAttributeTraits<
	T,
	HwVertexAttributeLocationId::color,
	decltype(static_cast<void>(T::rgba_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, rgba_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits

template<typename T>
struct HwVertexAttributeTraits<
	T,
	HwVertexAttributeLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, uv_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits


using HwVertexPosition = glm::vec3;
using HwVertexColor = bstone::R8g8b8a8;
using HwVertexTextureCoordinates = glm::vec2;

struct HwVertexXyzUv
{
	HwVertexPosition xyz_;
	HwVertexTextureCoordinates uv_;
}; // HwVertexXyzUv

struct HwVertexXyzRgbaUv
{
	HwVertexPosition xyz_;
	HwVertexColor rgba_;
	HwVertexTextureCoordinates uv_;
}; // HwVertexXyzRgbaUv


struct HwVertex : HwVertexXyzRgbaUv {};
struct Hw3dWallVertex : HwVertexXyzUv {};
struct Hw3dPushwallVertex : HwVertexXyzRgbaUv {};
struct Hw3dFlooringVertex : HwVertexXyzUv {};
struct Hw3dCeilingVertex : HwVertexXyzUv {};
struct Hw3dDoorVertex : HwVertexXyzUv {};
struct Hw3dSpriteVertex : HwVertexXyzRgbaUv {};
struct Hw3dPlayerWeaponVertex : HwVertexXyzUv {};
struct Hw3dFadeVertex : HwVertexXyzUv {};


struct Hw3dQuadFlags
{
	using Value = unsigned char;


	Value is_vertical_ : 1;
	Value is_back_face_ : 1;
}; // Hw3dQuadFlags


struct Hw3dWall;
using Hw3dWallPtr = Hw3dWall*;
using Hw3dWallCPtr = const Hw3dWall*;

struct Hw3dWallSideFlags
{
	using Type = unsigned char;


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
using Hw3dWallsToRenderList = std::unordered_set<int>;

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
	Hw3dDoorPtr hw_door_;

	bool is_back_face_;
	bstone::RendererTexture2dPtr texture_2d_;
}; // Hw3dDoorSide

using Hw3dDoorSidePtr = Hw3dDoorSide*;
using Hw3dDoorSideCPtr = const Hw3dDoorSide*;


struct Hw3dDoor
{
	using Sides = std::array<Hw3dDoorSide, ::hw_3d_sides_per_door>;

	int bs_door_index_;
	int vertex_index_;
	Sides sides_;
}; // Hw3dDoor


struct Hw3dDoorDrawItem
{
	bstone::RendererTexture2dPtr texture_2d_;
	Hw3dDoorSideCPtr hw_door_side_;
}; // Hw3dDoorDrawItem

using Hw3dXyDoorMap = std::unordered_map<int, Hw3dDoor>;
using Hw3dDoorsToRenderList = std::unordered_set<int>;
using Hw3dDoorDrawItems = std::vector<Hw3dDoorDrawItem>;

using Hw3dWallSideDrawItems = std::vector<Hw3dWallSideDrawItem>;

using Hw3dWallSideIndexBuffer = std::vector<std::uint16_t>;
using Hw3dDoorIndexBuffer = std::vector<std::uint16_t>;


enum class Hw3dSpriteKind
{
	stat,
	actor,
}; // Hw3dSpriteKind

struct Hw3dSpriteFlags
{
	using Value = unsigned char;


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
	int tile_x_;
	int tile_y_;
	int bs_sprite_id_;
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

using Hw3dSprites = std::vector<Hw3dSprite>;
using Hw3dSpritesPtr = Hw3dSprites*;

using Hw3dStaticsToRenderList = std::unordered_set<int>;
using Hw3dActorsToRenderList = std::unordered_set<int>;
using Hw3dSpritesDrawList = std::vector<Hw3dSpriteDrawItem>;

using Hw3dSpritesIndexBuffer = std::vector<std::uint16_t>;


template<typename TVertex>
using HwVertexBufferImageT = std::vector<TVertex>;

using HwVbi = HwVertexBufferImageT<HwVertex>;
using Hw3dWallsVbi = HwVertexBufferImageT<Hw3dWallVertex>;
using Hw3dPushwallsVbi = HwVertexBufferImageT<Hw3dPushwallVertex>;
using Hw3dDoorsVbi = HwVertexBufferImageT<Hw3dDoorVertex>;
using Hw3dSpritesVbi = HwVertexBufferImageT<Hw3dSpriteVertex>;
using Hw3dPlayerWeaponVbi = HwVertexBufferImageT<Hw3dPlayerWeaponVertex>;
using Hw3dFadeVbi = HwVertexBufferImageT<Hw3dFadeVertex>;


using HwShadingModeMod = bstone::ModValue<int>;
HwShadingModeMod hw_shading_mode_;

using HwBsShadeMaxMod = bstone::ModValue<int>;
HwBsShadeMaxMod hw_bs_shade_max_;

using HwBsNormalShadeMod = bstone::ModValue<int>;
HwBsNormalShadeMod hw_bs_normal_shade_;

using HwBsHeightNumeratorMod = bstone::ModValue<int>;
HwBsHeightNumeratorMod hw_bs_height_numerator_;

using HwBsLightingMod = bstone::ModValue<int>;
HwBsLightingMod hw_bs_lighting_;

using HwBsViewDirectionMod = bstone::ModValue<glm::vec2>;
HwBsViewDirectionMod hw_bs_view_direction_;

using HwBsViewPositionMod = bstone::ModValue<glm::vec2>;
HwBsViewPositionMod hw_bs_view_position_;


using HwMatrixTextureMod = bstone::ModValue<glm::mat4>;
HwMatrixTextureMod hw_matrix_texture_;

glm::mat4 hw_2d_matrix_model_ = glm::mat4{};
glm::mat4 hw_2d_matrix_view_ = glm::mat4{};
glm::mat4 hw_2d_matrix_projection_ = glm::mat4{};


constexpr auto hw_2d_quad_count = 2;

constexpr auto hw_2d_index_count_ = hw_2d_quad_count * 6;
constexpr auto hw_2d_stretched_index_offset_ = 0;
constexpr auto hw_2d_non_stretched_index_offset_ = 6;

constexpr auto hw_2d_vertex_count_ = hw_2d_quad_count * 4;
constexpr auto hw_2d_stretched_vertex_offset_ = 0;
constexpr auto hw_2d_non_stretched_vertex_offset_ = 4;

using Hw2dVbi = std::array<HwVertex, hw_2d_vertex_count_>;


constexpr auto hw_2d_fillers_ui_quad_count = 2;
constexpr auto hw_2d_fillers_hud_quad_count = 4;
constexpr auto hw_2d_fillers_quad_count = ::hw_2d_fillers_ui_quad_count + ::hw_2d_fillers_hud_quad_count;

constexpr auto hw_2d_fillers_index_count_ = ::hw_2d_fillers_quad_count * 6;
constexpr auto hw_2d_fillers_ui_index_offset_ = 0;
constexpr auto hw_2d_fillers_hud_index_offset_ = 6 * ::hw_2d_fillers_ui_quad_count;

constexpr auto hw_2d_fillers_vertex_count_ = hw_2d_fillers_quad_count * 4;



// Reference horizontal FOV of the camera (degrees).
// (Found out empirical.)
const double hw_3d_ref_camera_hfov_deg = 49.0;

// Vertical FOV in radians of the camera.
double hw_3d_camera_vfov_rad = 0.0;

// Vertical FOV in degrees of the camera.
double hw_3d_camera_vfov_deg = 0.0;

// Distance to the near plane of the camera.
double hw_3d_camera_near_distance = 0.0;

// Distance to the far plane of the camera.
double hw_3d_camera_far_distance = 0.0;


bstone::RendererSamplerState hw_2d_ui_s_state_;
bstone::RendererSamplerPtr hw_2d_ui_s_;

bstone::RendererSamplerState hw_3d_wall_s_state_;
bstone::RendererSamplerPtr hw_3d_wall_s_;

bstone::RendererSamplerState hw_3d_sprite_s_state_;
bstone::RendererSamplerPtr hw_3d_sprite_s_;

bstone::RendererSamplerState hw_3d_player_weapon_s_state_;
bstone::RendererSamplerPtr hw_3d_player_weapon_s_;

bstone::RendererSamplerPtr hw_fade_s_;


Hw2dVbi hw_2d_vertices_;

bstone::RendererDeviceFeatures hw_device_features_;
bstone::RendererManagerUPtr hw_renderer_manager_ = nullptr;
bstone::RendererPtr hw_renderer_ = nullptr;

bstone::HwTextureManagerUPtr hw_texture_manager_ = nullptr;

bstone::R8g8b8a8Palette hw_palette_;
bstone::R8g8b8a8Palette hw_default_palette_;

bstone::RendererCommandManagerUPtr hw_command_manager_;
bstone::RendererCommandBufferPtr hw_common_command_buffer_;
bstone::RendererCommandBufferPtr hw_2d_command_buffer_;
bstone::RendererCommandBufferPtr hw_3d_command_buffer_;

bstone::RendererTexture2dPtr hw_2d_ui_t2d_ = nullptr;
bstone::RendererIndexBufferPtr hw_2d_ui_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_2d_ui_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_2d_ui_vi_ = nullptr;

bstone::RendererTexture2dPtr hw_2d_black_t2d_1x1_ = nullptr;
bstone::RendererTexture2dPtr hw_2d_white_t2d_1x1_ = nullptr;

bstone::RendererIndexBufferPtr hw_2d_fillers_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_2d_fillers_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_2d_fillers_vi_ = nullptr;

bool hw_2d_fade_is_enabled_ = false;
HwVertexColor hw_2d_fade_color_ = bstone::R8g8b8a8{};
bstone::RendererTexture2dPtr hw_2d_fade_t2d_ = nullptr;


auto hw_3d_matrix_bs_to_r_ = glm::mat4{};
auto hw_3d_matrix_model_ = glm::mat4{};
auto hw_3d_matrix_view_ = glm::mat4{};
auto hw_3d_matrix_projection_ = glm::mat4{};


bstone::RendererIndexBufferPtr hw_3d_flooring_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_flooring_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_flooring_vi_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_flooring_solid_t2d_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_flooring_textured_t2d_ = nullptr;

bstone::RendererIndexBufferPtr hw_3d_ceiling_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_ceiling_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_ceiling_vi_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_ceiling_solid_t2d_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_ceiling_textured_t2d_ = nullptr;


auto hw_3d_player_angle_rad_ = double{};
auto hw_3d_view_direction_ = glm::dvec2{};
auto hw_3d_player_position_ = glm::dvec2{};
auto hw_3d_view_position_ = glm::dvec3{};


bool hw_3d_has_active_pushwall_ = false;
int hw_3d_active_pushwall_next_x_ = 0;
int hw_3d_active_pushwall_next_y_ = 0;

int hw_3d_wall_count_ = 0;
int hw_3d_wall_side_count_ = 0;
int hw_3d_wall_vertex_count_ = 0;
int hw_3d_wall_last_xy_to_render_at_ = 0;
Hw3dXyWallMap hw_3d_xy_wall_map_;
Hw3dWallsToRenderList hw_3d_walls_to_render_;

int hw_3d_wall_side_draw_item_count_ = 0;
Hw3dWallSideDrawItems hw_3d_wall_side_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_wall_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_wall_sides_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_wall_sides_vi_ = nullptr;

Hw3dWallSideIndexBuffer hw_3d_wall_sides_ibi_;


int hw_3d_pushwall_count_ = 0;
int hw_3d_pushwall_side_count_ = 0;
int hw_3d_pushwall_last_xy_to_render_at_ = 0;
Hw3dXyWallMap hw_3d_xy_pushwall_map_;
Hw3dWallsVbi hw_3d_pushwall_to_wall_vbi_;
Hw3dWallsToRenderList hw_3d_pushwalls_to_render_;

int hw_3d_pushwall_side_draw_item_count_ = 0;
Hw3dWallSideDrawItems hw_3d_pushwall_side_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_pushwall_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_pushwall_sides_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_pushwall_sides_vi_ = nullptr;

Hw3dWallSideIndexBuffer hw_3d_pushwall_sides_ibi_;
Hw3dPushwallsVbi hw_3d_pushwalls_vbi_;


int hw_3d_door_count_ = 0;

Hw3dXyDoorMap hw_3d_xy_door_map_;

int hw_3d_door_draw_item_count_ = 0;
int hw_3d_door_last_xy_to_render_at_ = 0;
Hw3dDoorsToRenderList hw_3d_doors_to_render_;
Hw3dDoorDrawItems hw_3d_door_draw_items_;

bstone::RendererIndexBufferPtr hw_3d_door_sides_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_door_sides_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_door_sides_vi_ = nullptr;

Hw3dDoorIndexBuffer hw_3d_door_sides_ibi_;
Hw3dDoorsVbi hw_3d_doors_vbi_;


Hw3dSprites hw_3d_statics_;
Hw3dStaticsToRenderList hw_3d_statics_to_render_;

using Hw3dActorsToReposition = std::vector<Hw3dSprite>;
using Hw3dActorsToRenderList = std::unordered_set<int>;
Hw3dSprites hw_3d_actors_;

int hw_3d_sprites_draw_count_ = 0;
Hw3dActorsToRenderList hw_3d_actors_to_render_;
Hw3dSpritesDrawList hw_3d_sprites_draw_list_;

bstone::RendererIndexBufferPtr hw_3d_sprites_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_sprites_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_sprites_vi_ = nullptr;

Hw3dSpritesIndexBuffer hw_3d_sprites_ibi_;
HwVbi hw_3d_sprites_vbi_;


bstone::RendererIndexBufferPtr hw_3d_player_weapon_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_player_weapon_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_player_weapon_vi_ = nullptr;
glm::mat4 hw_3d_player_weapon_model_matrix_;
glm::mat4 hw_3d_player_weapon_view_matrix_;
glm::mat4 hw_3d_player_weapon_projection_matrix_;


bool hw_3d_fade_is_enabled_ = false;
bstone::RendererIndexBufferPtr hw_3d_fade_ib_ = nullptr;
bstone::RendererVertexBufferPtr hw_3d_fade_vb_ = nullptr;
bstone::RendererVertexInputPtr hw_3d_fade_vi_ = nullptr;
bstone::RendererTexture2dPtr hw_3d_fade_t2d_ = nullptr;

bool hw_3d_fizzle_fx_is_enabled_ = false;
bool hw_3d_fizzle_fx_is_fading_ = false;
int hw_3d_fizzle_fx_color_index_ = 0;
float hw_3d_fizzle_fx_ratio_ = 0.0F;

bstone::RendererShaderPtr hw_shader_fragment_;
bstone::RendererShaderPtr hw_shader_vertex_;
bstone::RendererShaderStagePtr hw_shader_stage_;

bstone::RendererShaderVariableMat4Ptr hw_shader_variable_model_mat_;
bstone::RendererShaderVariableMat4Ptr hw_shader_variable_view_mat_;
bstone::RendererShaderVariableMat4Ptr hw_shader_variable_projection_mat_;
bstone::RendererShaderVariableMat4Ptr hw_shader_variable_texture_mat_;

bstone::RendererShaderVariableInt32Ptr hw_shader_variable_shading_mode_;
bstone::RendererShaderVariableFloat32Ptr hw_shader_variable_shade_max_;
bstone::RendererShaderVariableFloat32Ptr hw_shader_variable_normal_shade_;
bstone::RendererShaderVariableFloat32Ptr hw_shader_variable_height_numerator_;
bstone::RendererShaderVariableFloat32Ptr hw_shader_variable_extra_lighting_;
bstone::RendererShaderVariableVec2Ptr hw_shader_variable_view_direction_;
bstone::RendererShaderVariableVec2Ptr hw_shader_variable_view_position_;



bool hw_device_reset();
void hw_dbg_3d_orient_all_sprites();

bool hw_3d_player_weapon_initialize();
void hw_3d_player_weapon_model_matrix_update();


void vid_log_hw_renderer_manager_error(
	const std::string& message)
{
	::vid_log_error(message, ::hw_renderer_manager_->get_error_message());
}

void vid_log_hw_renderer_error(
	const std::string& message)
{
	::vid_log_error(message, ::hw_renderer_->get_error_message());
}

void vid_log_hw_texture_manager_error(
	const std::string& message)
{
	::vid_log_error(message, ::hw_texture_manager_->get_error_message());
}

bstone::RendererMipmapMode hw_config_texture_mipmap_filter_to_renderer(
	const bstone::RendererFilterKind filter_kind)
{
	switch (filter_kind)
	{
		case bstone::RendererFilterKind::nearest:
			return bstone::RendererMipmapMode::nearest;

		case bstone::RendererFilterKind::linear:
			return bstone::RendererMipmapMode::linear;

		default:
			::Quit("Invalid mipmap mode.");
			return bstone::RendererMipmapMode::none;
	}
}

int hw_config_texture_anisotropy_to_renderer(
	const bool is_enabled,
	const int value)
{
	if (!is_enabled || value <= bstone::RendererSampler::anisotropy_min)
	{
		return bstone::RendererSampler::anisotropy_min;
	}

	return value;
}

int hw_get_static_index(
	const statobj_t& bs_static)
{
	return static_cast<int>(&bs_static - ::statobjlist);
}

Hw3dSprite& hw_get_static(
	const statobj_t& bs_static)
{
	const auto bs_static_index = ::hw_get_static_index(bs_static);

	return ::hw_3d_statics_[bs_static_index];
}

int hw_get_actor_index(
	const objtype& bs_actor)
{
	return static_cast<int>(&bs_actor - ::objlist);
}

Hw3dSprite& hw_get_actor(
	const objtype& bs_actor)
{
	const auto bs_actor_index = ::hw_get_actor_index(bs_actor);

	return ::hw_3d_actors_[bs_actor_index];
}

constexpr int hw_encode_xy(
	const int x,
	const int y)
{
	return (x << 8) | y;
}

constexpr void hw_decode_xy(
	const int xy,
	int& x,
	int& y)
{
	x = (xy >> 8) & 0xFF;
	y = xy & 0xFF;
}

HwVertexColor hw_vga_color_to_r8g8b8a8(
	const int vga_red,
	const int vga_green,
	const int vga_blue)
{
	return HwVertexColor
	{
		static_cast<std::uint8_t>((255 * vga_red) / 63),
		static_cast<std::uint8_t>((255 * vga_green) / 63),
		static_cast<std::uint8_t>((255 * vga_blue) / 63),
		255
	};
}

void hw_sampler_destroy(
	bstone::RendererSamplerPtr& sampler)
{
	if (sampler == nullptr)
	{
		return;
	}

	::hw_renderer_->sampler_destroy(sampler);
	sampler = nullptr;
}

void hw_index_buffer_destroy(
	bstone::RendererIndexBufferPtr& index_buffer)
{
	if (index_buffer == nullptr)
	{
		return;
	}

	::hw_renderer_->index_buffer_destroy(index_buffer);
	index_buffer = nullptr;
}

bstone::RendererIndexBufferPtr hw_index_buffer_create(
	const bstone::RendererBufferUsageKind usage_kind,
	const int byte_depth,
	const int index_count)
{
	const auto index_buffer_size = index_count * byte_depth;

	auto param = bstone::RendererIndexBufferCreateParam{};
	param.usage_kind_ = usage_kind;
	param.byte_depth_ = byte_depth;
	param.size_ = index_buffer_size;

	return ::hw_renderer_->index_buffer_create(param);
}

template<typename TIndex>
void hw_index_buffer_update(
	bstone::RendererIndexBufferPtr index_buffer,
	const int index_offset,
	const int index_count,
	const TIndex* const indices)
{
	const auto byte_depth = static_cast<int>(sizeof(TIndex));
	const auto offset = index_offset * byte_depth;
	const auto size = index_count * byte_depth;

	auto param = bstone::RendererIndexBufferUpdateParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = indices;

	index_buffer->update(param);
}

void hw_vertex_buffer_destroy(
	bstone::RendererVertexBufferPtr& vertex_buffer)
{
	if (vertex_buffer == nullptr)
	{
		return;
	}

	::hw_renderer_->vertex_buffer_destroy(vertex_buffer);
	vertex_buffer = nullptr;
}

template<typename TVertex>
bstone::RendererVertexBufferPtr hw_vertex_buffer_create(
	const bstone::RendererBufferUsageKind usage_kind,
	const int vertex_count)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto vertex_buffer_size = vertex_count * vertex_size;

	auto param = bstone::RendererVertexBufferCreateParam{};
	param.usage_kind_ = usage_kind;
	param.size_ = vertex_buffer_size;

	return ::hw_renderer_->vertex_buffer_create(param);
}

template<typename TVertex>
void hw_vertex_buffer_update(
	bstone::RendererVertexBufferPtr vertex_buffer,
	const int vertex_offset,
	const int vertex_count,
	const TVertex* const vertices)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto offset = vertex_offset * vertex_size;
	const auto size = vertex_count * vertex_size;

	auto param = bstone::RendererVertexBufferUpdateParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = vertices;

	vertex_buffer->update(param);
}

void hw_vertex_input_destroy(
	bstone::RendererVertexInputPtr& vertex_input)
{
	if (vertex_input == nullptr)
	{
		return;
	}

	::hw_renderer_->vertex_input_destroy(vertex_input);
	vertex_input = nullptr;
}

void hw_texture_2d_destroy(
	bstone::RendererTexture2dPtr& texture_2d)
{
	if (texture_2d == nullptr)
	{
		return;
	}

	::hw_renderer_->texture_2d_destroy(texture_2d);
	texture_2d = nullptr;
}

template<
	typename TVertex,
	bool TIsExist = false
>
struct HwVertexInputAddAttributeDescription
{
	void operator()(
		const int location,
		const bstone::RendererVertexAttributeFormat format,
		const int offset,
		const int stride,
		const glm::vec4& default_value,
		bstone::RendererVertexBufferPtr vertex_buffer,
		bstone::RendererVertexAttributeDescriptions& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = true;
		description.location_ = location;
		description.format_ = bstone::RendererVertexAttributeFormat::none;
		description.vertex_buffer_ = nullptr;
		description.offset_ = -1;
		description.stride_ = -1;
		description.default_value_ = default_value;
	}
}; // HwVertexInputAddAttributeDescription

template<
	typename TVertex>
struct HwVertexInputAddAttributeDescription<TVertex, true>
{
	void operator()(
		const int location,
		const bstone::RendererVertexAttributeFormat format,
		const int offset,
		const int stride,
		const glm::vec4& default_value,
		bstone::RendererVertexBufferPtr vertex_buffer,
		bstone::RendererVertexAttributeDescriptions& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = false;
		description.location_ = location;
		description.format_ = format;
		description.vertex_buffer_ = vertex_buffer;
		description.offset_ = offset;
		description.stride_ = stride;
		description.default_value_ = default_value;
	}
}; // HwVertexInputAddAttributeDescription

template<
	typename TVertex,
	HwVertexAttributeLocationId TLocationId>
void hw_vertex_input_add_attribute_description(
	const bstone::RendererVertexAttributeFormat format,
	const glm::vec4& default_value,
	bstone::RendererVertexBufferPtr vertex_buffer,
	bstone::RendererVertexAttributeDescriptions& attribute_descriptions)
{
	const auto& traits = HwVertexAttributeTraits<TVertex, TLocationId>{};
	const auto& add_attribute = HwVertexInputAddAttributeDescription<TVertex, traits.is_valid>{};

	auto location = 0;

	switch (TLocationId)
	{
		case HwVertexAttributeLocationId::position:
			location = bstone::RendererShaderRegistry::get_a_position_location();
			break;

		case HwVertexAttributeLocationId::color:
			location = bstone::RendererShaderRegistry::get_a_color_location();
			break;

		case HwVertexAttributeLocationId::texture_coordinates:
			location = bstone::RendererShaderRegistry::get_a_tx_coords_location();
			break;
	}

	add_attribute(
		location,
		format,
		traits.offset,
		traits.stride,
		default_value,
		vertex_buffer,
		attribute_descriptions
	);
}

template<typename TVertex>
bool hw_vertex_input_create(
	bstone::RendererIndexBufferPtr index_buffer,
	bstone::RendererVertexBufferPtr vertex_buffer,
	bstone::RendererVertexInputPtr& vertex_input)
{
	auto param = bstone::RendererVertexInputCreateParam{};
	param.index_buffer_ = index_buffer;

	auto& descriptions = param.attribute_descriptions_;
	descriptions.reserve(3);

	::hw_vertex_input_add_attribute_description<TVertex, HwVertexAttributeLocationId::position>(
		bstone::RendererVertexAttributeFormat::r32g32b32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	::hw_vertex_input_add_attribute_description<TVertex, HwVertexAttributeLocationId::color>(
		bstone::RendererVertexAttributeFormat::r8g8b8a8_unorm,
		glm::vec4{1.0F, 1.0F, 1.0F, 1.0F},
		vertex_buffer,
		descriptions
	);

	::hw_vertex_input_add_attribute_description<TVertex, HwVertexAttributeLocationId::texture_coordinates>(
		bstone::RendererVertexAttributeFormat::r32g32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	vertex_input = ::hw_renderer_->vertex_input_create(param);

	if (!vertex_input)
	{
		return false;
	}

	return true;
}

void hw_3d_player_update_direction()
{
	::hw_3d_player_angle_rad_ = ::player->angle * (m_pi() / 180.0);

	::hw_3d_view_direction_.x = std::cos(::hw_3d_player_angle_rad_);
	::hw_3d_view_direction_.y = -std::sin(::hw_3d_player_angle_rad_);

	::hw_bs_view_direction_ = ::hw_3d_view_direction_;
}

void hw_3d_player_update_position()
{
	::hw_3d_player_position_.x = bstone::FixedPoint{::player->x}.to_double();
	::hw_3d_player_position_.y = bstone::FixedPoint{::player->y}.to_double();
}

void hw_3d_player_update_view_position()
{
	const auto focal_length = bstone::FixedPoint{::focallength}.to_double();

	const auto focal_delta = glm::dvec2
	{
		::hw_3d_view_direction_.x * focal_length,
		::hw_3d_view_direction_.y * focal_length,
	};

	::hw_3d_view_position_ = glm::dvec3{::hw_3d_player_position_ - focal_delta, 0.5};

	::hw_bs_view_position_ = ::hw_3d_view_position_;
}

void hw_3d_player_update()
{
	::hw_3d_player_update_direction();
	::hw_3d_player_update_position();
	::hw_3d_player_update_view_position();
}

void hw_ui_buffer_initialize()
{
	::sw_initialize_ui_buffer();
}

void hw_shader_destroy(
	bstone::RendererShaderPtr& shader)
{
	if (shader == nullptr)
	{
		return;
	}

	::hw_renderer_->shader_destroy(shader);

	shader = nullptr;
}

bool hw_shader_create(
	const bstone::RendererShader::Kind kind,
	bstone::RendererShaderPtr& shader)
{
	auto param = bstone::RendererShader::CreateParam{};
	param.kind_ = kind;

	const auto renderer_kind = ::hw_renderer_->get_path();

	switch (kind)
	{
		case bstone::RendererShader::Kind::fragment:
			param.source_ = bstone::RendererShaderRegistry::get_fragment(renderer_kind);
			break;

		case bstone::RendererShader::Kind::vertex:
			param.source_ = bstone::RendererShaderRegistry::get_vertex(renderer_kind);
			break;

		default:
			::Quit("Unsupported shader kind.");

			return false;
	}

	shader = ::hw_renderer_->shader_create(param);

	return shader != nullptr;
}

void hw_shader_fragment_destroy()
{
	::hw_shader_destroy(::hw_shader_fragment_);
}

bool hw_shader_fragment_create()
{
	return ::hw_shader_create(bstone::RendererShader::Kind::fragment, ::hw_shader_fragment_);
}

void hw_shader_vertex_destroy()
{
	::hw_shader_destroy(::hw_shader_vertex_);
}

bool hw_shader_vertex_create()
{
	return ::hw_shader_create(bstone::RendererShader::Kind::vertex, ::hw_shader_vertex_);
}

void hw_shader_stage_destroy()
{
	if (::hw_shader_stage_ == nullptr)
	{
		return;
	}

	::hw_renderer_->shader_stage_destroy(::hw_shader_stage_);

	::hw_shader_stage_ = nullptr;
}

bool hw_shader_stage_create()
{
	static const auto input_bindings = bstone::RendererShaderStage::InputBindings
	{
		{0, bstone::RendererShaderRegistry::get_a_position_name()},
		{1, bstone::RendererShaderRegistry::get_a_color_name()},
		{2, bstone::RendererShaderRegistry::get_a_tx_coords_name()},
	};

	auto param = bstone::RendererShaderStage::CreateParam{};
	param.fragment_shader_ = ::hw_shader_fragment_;
	param.vertex_shader_ = ::hw_shader_vertex_;
	param.input_bindings_ = input_bindings;

	::hw_shader_stage_ = ::hw_renderer_->shader_stage_create(param);

	if (::hw_shader_stage_ == nullptr)
	{
		return false;
	}

	return true;
}

template<typename T>
void hw_program_uninitialize_variable(
	T*& variable)
{
	variable = nullptr;
}

void hw_program_uninitialize_variable_model_mat()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_model_mat_);
}

struct HwProgramInitializeVariableInt32Tag{};
struct HwProgramInitializeVariableFloat32Tag{};
struct HwProgramInitializeVariableVec2Tag{};
struct HwProgramInitializeVariableMat4Tag{};

void hw_program_initialize_variable(
	const std::string& name,
	bstone::RendererShaderVariableInt32Ptr& variable,
	const HwProgramInitializeVariableInt32Tag)
{
	variable = ::hw_shader_stage_->find_variable_int32(name);
}

void hw_program_initialize_variable(
	const std::string& name,
	bstone::RendererShaderVariableFloat32Ptr& variable,
	const HwProgramInitializeVariableFloat32Tag)
{
	variable = ::hw_shader_stage_->find_variable_float32(name);
}

void hw_program_initialize_variable(
	const std::string& name,
	bstone::RendererShaderVariableVec2Ptr& variable,
	const HwProgramInitializeVariableVec2Tag)
{
	variable = ::hw_shader_stage_->find_variable_vec2(name);
}

void hw_program_initialize_variable(
	const std::string& name,
	bstone::RendererShaderVariableMat4Ptr& variable,
	const HwProgramInitializeVariableMat4Tag)
{
	variable = ::hw_shader_stage_->find_variable_mat4(name);
}

template<typename T>
bool hw_program_initialize_variable(
	const std::string& name,
	T*& variable)
{
	using Tag = std::conditional_t<
		std::is_same<T, bstone::RendererShaderVariableInt32>::value,
		HwProgramInitializeVariableInt32Tag,
		std::conditional_t<
			std::is_same<T, bstone::RendererShaderVariableFloat32>::value,
			HwProgramInitializeVariableFloat32Tag,
			std::conditional_t<
				std::is_same<T, bstone::RendererShaderVariableVec2>::value,
				HwProgramInitializeVariableVec2Tag,
				std::conditional_t<
					std::is_same<T, bstone::RendererShaderVariableMat4>::value,
					HwProgramInitializeVariableMat4Tag,
					void
				>
			>
		>
	>;

	static_assert(!std::is_same<Tag, void>::value, "Unsupported type.");

	hw_program_initialize_variable(name, variable, Tag{});

	if (variable == nullptr)
	{
		bstone::logger_->write_error("Shader variable \"" + name + "\" not found.");

		return false;
	}

	return true;
}

bool hw_program_initialize_variable_model_mat()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_model_mat_name(),
		::hw_shader_variable_model_mat_
	);
}

void hw_program_uninitialize_variable_view_mat()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_view_mat_);
}

bool hw_program_initialize_variable_view_mat()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_view_mat_name(),
		::hw_shader_variable_view_mat_
	);
}

void hw_program_uninitialize_variable_projection_mat()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_projection_mat_);
}

bool hw_program_initialize_variable_projection_mat()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_projection_mat_name(),
		::hw_shader_variable_projection_mat_
	);
}

void hw_program_uninitialize_variable_texture_mat()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_texture_mat_);
}

bool hw_program_initialize_variable_texture_mat()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_texture_mat_name(),
		::hw_shader_variable_texture_mat_
	);
}

void hw_program_uninitialize_variable_shading_mode()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_shading_mode_);
}

bool hw_program_initialize_variable_shading_mode()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_shading_mode_name(),
		::hw_shader_variable_shading_mode_
	);
}

void hw_program_uninitialize_variable_shade_max()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_shade_max_);
}

bool hw_program_initialize_variable_shade_max()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_shade_max_name(),
		::hw_shader_variable_shade_max_
	);
}

void hw_program_uninitialize_variable_normal_shade()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_normal_shade_);
}

bool hw_program_initialize_variable_normal_shade()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_normal_shade_name(),
		::hw_shader_variable_normal_shade_
	);
}

void hw_program_uninitialize_variable_height_numerator()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_height_numerator_);
}

bool hw_program_initialize_variable_height_numerator()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_height_numerator_name(),
		::hw_shader_variable_height_numerator_
	);
}

void hw_program_uninitialize_variable_extra_lighting()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_extra_lighting_);
}

bool hw_program_initialize_variable_extra_lighting()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_extra_lighting_name(),
		::hw_shader_variable_extra_lighting_
	);
}

void hw_program_uninitialize_variable_view_direction()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_view_direction_);
}

bool hw_program_initialize_variable_view_direction()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_view_direction_name(),
		::hw_shader_variable_view_direction_
	);
}

void hw_program_uninitialize_variable_view_position()
{
	::hw_program_uninitialize_variable(::hw_shader_variable_view_position_);
}

bool hw_program_initialize_variable_view_position()
{
	return hw_program_initialize_variable(
		bstone::RendererShaderRegistry::get_u_view_position_name(),
		::hw_shader_variable_view_position_
	);
}

void hw_program_uninitialize_variables_shading()
{
	::hw_program_uninitialize_variable_shading_mode();
	::hw_program_uninitialize_variable_shade_max();
	::hw_program_uninitialize_variable_normal_shade();
	::hw_program_uninitialize_variable_height_numerator();
	::hw_program_uninitialize_variable_extra_lighting();
	::hw_program_uninitialize_variable_view_direction();
	::hw_program_uninitialize_variable_view_position();
}

bool hw_program_initialize_variables_shading()
{
	if (!::hw_program_initialize_variable_shading_mode())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_shade_max())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_normal_shade())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_height_numerator())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_extra_lighting())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_view_direction())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_view_position())
	{
		return false;
	}

	return true;
}

void hw_program_uninitialize_variables()
{
	::hw_program_uninitialize_variable_model_mat();
	::hw_program_uninitialize_variable_view_mat();
	::hw_program_uninitialize_variable_projection_mat();
	::hw_program_uninitialize_variable_texture_mat();
	::hw_program_uninitialize_variables_shading();
}

bool hw_program_initialize_variables()
{
	if (!::hw_program_initialize_variable_model_mat())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_view_mat())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_projection_mat())
	{
		return false;
	}

	if (!::hw_program_initialize_variable_texture_mat())
	{
		return false;
	}

	if (!::hw_program_initialize_variables_shading())
	{
		return false;
	}

	return true;
}

void hw_program_uninitialize()
{
	::hw_program_uninitialize_variables();
	::hw_shader_stage_destroy();
	::hw_shader_fragment_destroy();
	::hw_shader_vertex_destroy();
}

bool hw_program_initialize()
{
	if (!::hw_shader_fragment_create())
	{
		return false;
	}

	if (!::hw_shader_vertex_create())
	{
		return false;
	}

	if (!::hw_shader_stage_create())
	{
		return false;
	}

	if (!::hw_program_initialize_variables())
	{
		return false;
	}

	return true;
}

bool hw_renderer_initialize()
{
	::vid_log();
	::vid_log("Initializing renderer.");


	const auto title = ::vid_get_window_title();


	// Initialization parameter.
	//
	const auto& probe = ::hw_renderer_manager_->renderer_probe_get();

	auto param = bstone::RendererInitializeParam{};
	param.renderer_path_ = probe.path_;

	param.aa_kind_ = ::vid_configuration_.hw_aa_kind_;
	param.aa_value_ = ::vid_configuration_.hw_aa_value_;

	param.is_vsync_ = ::vid_configuration_.is_vsync_;

#ifdef __vita__
	param.window_.is_visible_ = true;
#endif // __vita__

	if (!::vid_configuration_.is_windowed_)
	{
		param.window_.is_borderless_ = true;
		param.window_.is_fullscreen_desktop_ = true;
	}

	param.window_.is_positioned_ = ::vid_configuration_.is_custom_position_;
	param.window_.x_ = ::vid_configuration_.x_;
	param.window_.y_ = ::vid_configuration_.y_;

	param.window_.width_ = ::vid_configuration_.width_;
	param.window_.height_ = ::vid_configuration_.height_;

	param.downscale_width_ = ::downscale_.window_width_;
	param.downscale_height_ = ::downscale_.window_height_;

	param.window_.title_utf8_ = title;

	hw_renderer_ = hw_renderer_manager_->renderer_initialize(param);

	if (!hw_renderer_)
	{
		::vid_log_hw_renderer_manager_error("Failed to initialize renderer.");

		return false;
	}

	return true;
}

void hw_2d_ui_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_2d_ui_ib_);
}

bool hw_2d_ui_ib_create()
{
	::hw_2d_ui_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::static_draw,
		1,
		::hw_2d_index_count_
	);

	if (!::hw_2d_ui_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create UI index buffer.");

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

	::hw_index_buffer_update(
		::hw_2d_ui_ib_,
		0,
		::hw_2d_index_count_,
		indices.data());

	return true;
}

void hw_2d_ui_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_2d_ui_vi_);
}

bool hw_2d_ui_vi_create()
{
	if (!::hw_vertex_input_create<HwVertex>(
		::hw_2d_ui_ib_,
		::hw_2d_ui_vb_,
		::hw_2d_ui_vi_))
	{
		::vid_log_hw_renderer_error("Failed to create UI vertex input.");

		return false;
	}

	return true;
}

void hw_2d_vb_fill_x_stretched(
	const float left_f,
	const float right_f,
	const float width_f,
	const int vertex_offset)
{
	static_cast<void>(width_f);

	auto vertex_index = vertex_offset;
	auto& vertices = ::hw_2d_vertices_;

	const auto height_f = static_cast<float>(::downscale_.screen_height_);

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_f, 0.0F, 0.0F};
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_f, 0.0F, 0.0F};
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Upper right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_f, height_f, 0.0F};
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Upper left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_f, height_f, 0.0F};
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}
}

void hw_2d_vb_fill_stretched()
{
	const auto left_f = 0.0F;
	const auto right_f = static_cast<float>(::downscale_.screen_width_);
	const auto width_f = static_cast<float>(::downscale_.screen_width_);

	hw_2d_vb_fill_x_stretched(left_f, right_f, width_f, ::hw_2d_stretched_vertex_offset_);
}

void hw_2d_vb_fill_non_stretched()
{
	const auto left_f = static_cast<float>(::downscale_.screen_left_filler_width_);
	const auto right_f = static_cast<float>(::downscale_.screen_width_ - ::downscale_.screen_right_filler_width_);
	const auto width_f = static_cast<float>(::downscale_.screen_width_4x3_);

	hw_2d_vb_fill_x_stretched(left_f, right_f, width_f, ::hw_2d_non_stretched_vertex_offset_);
}

void hw_2d_ui_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_2d_ui_vb_);
}

bool hw_2d_ui_vb_create()
{
	::hw_2d_ui_vb_ = ::hw_vertex_buffer_create<HwVertex>(
		bstone::RendererBufferUsageKind::static_draw,
		::hw_2d_vertex_count_
	);

	if (!::hw_2d_ui_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create UI vertex buffer.");

		return false;
	}

	hw_2d_vb_fill_stretched();
	hw_2d_vb_fill_non_stretched();

	::hw_vertex_buffer_update(
		::hw_2d_ui_vb_,
		0,
		::hw_2d_vertex_count_,
		::hw_2d_vertices_.data()
	);

	return true;
}

void hw_2d_fillers_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_2d_fillers_ib_);
}

bool hw_2d_fillers_ib_create()
{
	::hw_2d_fillers_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::static_draw,
		1,
		::hw_2d_fillers_index_count_
	);

	if (!::hw_2d_fillers_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create fillers index buffer.");

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

	::hw_index_buffer_update(
		::hw_2d_fillers_ib_,
		0,
		::hw_2d_fillers_index_count_,
		indices.data());

	return true;
}

void hw_2d_fillers_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_2d_fillers_vb_);
}

bool hw_2d_fillers_vb_create()
{
	::hw_2d_fillers_vb_ = ::hw_vertex_buffer_create<HwVertex>(
		bstone::RendererBufferUsageKind::static_draw,
		::hw_2d_fillers_vertex_count_
	);

	if (!::hw_2d_fillers_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create fillers vertex buffer.");

		return false;
	}

	const auto& filler_color = ::hw_vga_color_to_r8g8b8a8(
		::vgapal[(::filler_color_index * 3) + 0],
		::vgapal[(::filler_color_index * 3) + 1],
		::vgapal[(::filler_color_index * 3) + 2]
	);

	const auto left_left_f = static_cast<float>(0.0F);
	const auto left_right_f = static_cast<float>(::downscale_.screen_left_filler_width_);

	const auto right_left_f = static_cast<float>(::downscale_.screen_width_ - ::downscale_.screen_right_filler_width_);
	const auto right_right_f = static_cast<float>(::downscale_.screen_width_);

	const auto top_top_f = static_cast<float>(::downscale_.screen_height_);
	const auto top_bottom_f = static_cast<float>(::downscale_.screen_height_ - ::downscale_.screen_top_filler_height_);

	const auto bottom_top_f = static_cast<float>(::downscale_.screen_bottom_filler_height_);
	const auto bottom_bottom_f = static_cast<float>(0.0F);

	auto vertex_index = 0;

	using Hw2dFillersVertices = std::array<HwVertex, hw_2d_fillers_vertex_count_>;
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
		vertex.xyz_ = HwVertexPosition{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
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
		vertex.xyz_ = HwVertexPosition{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
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
		vertex.xyz_ = HwVertexPosition{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
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
		vertex.xyz_ = HwVertexPosition{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, bottom_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
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
		vertex.xyz_ = HwVertexPosition{right_left_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
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
		vertex.xyz_ = HwVertexPosition{left_left_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_bottom_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Top left.
	// ----------------------------------------------------------------------

	//
	// HUD fillers.
	// ======================================================================


	::hw_vertex_buffer_update(
		::hw_2d_fillers_vb_,
		0,
		::hw_2d_fillers_vertex_count_,
		vertices.data()
	);

	return true;
}

void hw_2d_fillers_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_2d_fillers_vi_);
}

bool hw_2d_fillers_vi_create()
{
	const auto result = ::hw_vertex_input_create<HwVertex>(
		::hw_2d_fillers_ib_,
		::hw_2d_fillers_vb_,
		::hw_2d_fillers_vi_
	);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create fillers vertex input.");
	}

	return result;
}

void hw_2d_texture_1x1_black_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::black);
	}

	::hw_2d_black_t2d_1x1_ = nullptr;
}

bool hw_2d_texture_1x1_black_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::black))
	{
		::vid_log_hw_texture_manager_error("Failed to create 1x1 black texture.");

		return false;
	}

	::hw_2d_black_t2d_1x1_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::black);

	if (::hw_2d_black_t2d_1x1_ == nullptr)
	{
		::vid_log_hw_texture_manager_error("Failed to get 1x1 black texture.");

		return false;
	}

	return true;
}

void hw_2d_texture_1x1_white_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::white);
	}

	::hw_2d_white_t2d_1x1_ = nullptr;
}

bool hw_2d_texture_1x1_white_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::white))
	{
		::vid_log_hw_texture_manager_error("Failed to create 1x1 white texture.");

		return false;
	}

	::hw_2d_white_t2d_1x1_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::white);

	if (::hw_2d_white_t2d_1x1_ == nullptr)
	{
		::vid_log_hw_texture_manager_error("Failed to get 1x1 white texture.");

		return false;
	}

	return true;
}

void hw_2d_texture_1x1_fade_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::fade_2d);
	}

	::hw_2d_fade_t2d_ = nullptr;
}

bool hw_2d_texture_1x1_fade_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::fade_2d))
	{
		::vid_log_hw_texture_manager_error("Failed to create 1x1 2d fade texture.");

		return false;
	}

	::hw_2d_fade_t2d_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::fade_2d);

	if (::hw_2d_fade_t2d_ == nullptr)
	{
		::vid_log_hw_texture_manager_error("Failed to get 1x1 2d fade texture.");

		return false;
	}

	return true;
}

void hw_2d_ui_texture_destroy()
{
	if (::hw_2d_ui_t2d_ == nullptr)
	{
		return;
	}

	::hw_texture_manager_->ui_destroy();
	::hw_2d_ui_t2d_ = nullptr;
}

bool hw_2d_ui_texture_create()
{
	if (!::hw_texture_manager_->ui_create(::vid_ui_buffer.data(), ::vid_mask_buffer.data(), &::hw_palette_))
	{
		::vid_log_hw_texture_manager_error("Failed to create UI texture.");

		return false;
	}

	::hw_2d_ui_t2d_ = ::hw_texture_manager_->ui_get();

	if (::hw_2d_ui_t2d_ == nullptr)
	{
		::vid_log_hw_texture_manager_error("Failed to get UI texture.");

		return false;
	}

	return true;
}

void hw_2d_uninitialize()
{
	::hw_2d_ui_texture_destroy();

	::hw_2d_ui_vi_destroy();
	::hw_2d_ui_ib_destroy();
	::hw_2d_ui_vb_destroy();

	::hw_2d_fillers_vi_destroy();
	::hw_2d_fillers_ib_destroy();
	::hw_2d_fillers_vb_destroy();

	::hw_2d_texture_1x1_black_destroy();
	::hw_2d_texture_1x1_white_destroy();
	::hw_2d_texture_1x1_fade_destroy();
}

bool hw_2d_initialize()
{
	::vid_log();
	::vid_log("Initializing 2D stuff.");

	if (!::hw_2d_ui_ib_create())
	{
		return false;
	}

	if (!::hw_2d_ui_vb_create())
	{
		return false;
	}

	if (!::hw_2d_ui_vi_create())
	{
		return false;
	}

	if (!::hw_2d_fillers_ib_create())
	{
		return false;
	}

	if (!::hw_2d_fillers_vb_create())
	{
		return false;
	}

	if (!::hw_2d_fillers_vi_create())
	{
		return false;
	}

	if (!::hw_2d_ui_texture_create())
	{
		return false;
	}

	if (!::hw_2d_texture_1x1_black_create())
	{
		return false;
	}

	if (!::hw_2d_texture_1x1_white_create())
	{
		return false;
	}

	if (!::hw_2d_texture_1x1_fade_create())
	{
		return false;
	}

	return true;
}

void hw_3d_flooring_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_flooring_ib_);
}

bool hw_3d_flooring_ib_create()
{
	const auto index_count = 6;

	{
		::hw_3d_flooring_ib_ = ::hw_index_buffer_create(
			bstone::RendererBufferUsageKind::static_draw,
			1,
			index_count
		);

		if (!::hw_3d_flooring_ib_)
		{
			::vid_log_hw_renderer_error("Failed to create flooring index buffer.");

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

		::hw_index_buffer_update(
			::hw_3d_flooring_ib_,
			0,
			index_count,
			indices.data()
		);
	}

	return true;
}

void hw_3d_flooring_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_flooring_vb_);
}

bool hw_3d_flooring_vb_create()
{
	const auto vertex_count = 4;

	{
		::hw_3d_flooring_vb_ = ::hw_vertex_buffer_create<Hw3dFlooringVertex>(
			bstone::RendererBufferUsageKind::static_draw,
			vertex_count
		);

		if (!::hw_3d_flooring_vb_)
		{
			::vid_log_hw_renderer_error("Failed to create flooring vertex buffer.");

			return false;
		}
	}

	{
		const auto map_dimension_f = static_cast<float>(MAPSIZE);

		using Vertices = std::array<Hw3dFlooringVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, 0.0F, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, map_dimension_f, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{map_dimension_f, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{map_dimension_f, map_dimension_f, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{map_dimension_f, map_dimension_f};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{map_dimension_f, 0.0F, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, map_dimension_f};
		}

		::hw_vertex_buffer_update(
			::hw_3d_flooring_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}

	return true;
}

void hw_3d_flooring_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_flooring_vi_);
}

bool hw_3d_flooring_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dFlooringVertex>(
		::hw_3d_flooring_ib_,
		::hw_3d_flooring_vb_,
		::hw_3d_flooring_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create flooring vertex input.");

		return false;
	}

	return true;
}

void hw_3d_flooring_texture_2d_solid_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::flooring);
	}

	::hw_3d_flooring_solid_t2d_ = nullptr;
}

bool hw_3d_flooring_texture_2d_solid_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::flooring))
	{
		return false;
	}

	::hw_3d_flooring_solid_t2d_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::flooring);

	if (::hw_3d_flooring_solid_t2d_ == nullptr)
	{
		return false;
	}

	return true;
}

void hw_3d_flooring_uninitialize()
{
	::hw_3d_flooring_vi_destroy();
	::hw_3d_flooring_ib_destroy();
	::hw_3d_flooring_vb_destroy();

	::hw_3d_flooring_texture_2d_solid_destroy();

	// Managed by texture manager. No need to destroy.
	::hw_3d_flooring_textured_t2d_ = nullptr;
}

bool hw_3d_flooring_initialize()
{
	::vid_log();
	::vid_log("Initializing flooring.");

	if (!::hw_3d_flooring_ib_create())
	{
		return false;
	}

	if (!::hw_3d_flooring_vb_create())
	{
		return false;
	}

	if (!::hw_3d_flooring_vi_create())
	{
		return false;
	}

	if (!::hw_3d_flooring_texture_2d_solid_create())
	{
		return false;
	}

	return true;
}

void hw_3d_ceiling_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_ceiling_ib_);
}

bool hw_3d_ceiling_ib_create()
{
	const auto index_count = 6;

	{
		::hw_3d_ceiling_ib_ = ::hw_index_buffer_create(
			bstone::RendererBufferUsageKind::static_draw,
			1,
			index_count
		);

		if (!::hw_3d_ceiling_ib_)
		{
			::vid_log_hw_renderer_error("Failed to create ceiling index buffer.");

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

		::hw_index_buffer_update(
			::hw_3d_ceiling_ib_,
			0,
			index_count,
			indices.data()
		);
	}

	return true;
}

void hw_3d_ceiling_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_ceiling_vb_);
}

bool hw_3d_ceiling_vb_create()
{
	const auto vertex_count = 4;

	{
		::hw_3d_ceiling_vb_ = ::hw_vertex_buffer_create<Hw3dCeilingVertex>(
			bstone::RendererBufferUsageKind::static_draw,
			vertex_count
		);

		if (!::hw_3d_ceiling_vb_)
		{
			::vid_log_hw_renderer_error("Failed to create ceiling vertex buffer.");

			return false;
		}
	}

	{
		using Vertices = std::array<Hw3dCeilingVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, 0.0F, ::hw_3d_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, ::hw_3d_map_dimension_f, ::hw_3d_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{::hw_3d_map_dimension_f, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{::hw_3d_map_dimension_f, ::hw_3d_map_dimension_f, ::hw_3d_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{::hw_3d_map_dimension_f, ::hw_3d_map_dimension_f};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{::hw_3d_map_dimension_f, 0.0F, ::hw_3d_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, ::hw_3d_map_dimension_f};
		}

		::hw_vertex_buffer_update(
			::hw_3d_ceiling_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}

	return true;
}

void hw_3d_ceiling_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_ceiling_vi_);
}

bool hw_3d_ceiling_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dCeilingVertex>(
		::hw_3d_ceiling_ib_,
		::hw_3d_ceiling_vb_,
		::hw_3d_ceiling_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create ceiling vertex input.");
	}

	return result;
}

void hw_3d_ceiling_texture_2d_solid_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::ceiling);
	}

	::hw_3d_ceiling_solid_t2d_ = nullptr;
}

bool hw_3d_ceiling_texture_2d_solid_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::ceiling))
	{
		::vid_log_hw_texture_manager_error("Failed to create 1x1 solid ceiling texture.");

		return false;
	}

	::hw_3d_ceiling_solid_t2d_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::ceiling);

	if (::hw_3d_ceiling_solid_t2d_ == nullptr)
	{
		::vid_log_hw_texture_manager_error("Failed to get 1x1 solid ceiling texture.");

		return false;
	}

	return true;
}

bool hw_3d_ceiling_initialize()
{
	::vid_log();
	::vid_log("Initializing ceiling.");

	if (!::hw_3d_ceiling_ib_create())
	{
		return false;
	}

	if (!::hw_3d_ceiling_vb_create())
	{
		return false;
	}

	if (!::hw_3d_ceiling_vi_create())
	{
		return false;
	}

	if (!::hw_3d_ceiling_texture_2d_solid_create())
	{
		return false;
	}

	return true;
}

void hw_3d_ceiling_uninitialize()
{
	::hw_3d_ceiling_vi_destroy();
	::hw_3d_ceiling_ib_destroy();
	::hw_3d_ceiling_vb_destroy();

	::hw_3d_ceiling_texture_2d_solid_destroy();

	// Managed by texture manager. No need to destroy.
	::hw_3d_ceiling_textured_t2d_ = nullptr;
}

bool hw_3d_walls_ib_create()
{
	const auto index_count = ::hw_3d_wall_side_count_ * ::hw_3d_indices_per_wall_side;

	::hw_3d_wall_sides_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::stream_draw,
		2,
		index_count
	);

	if (!::hw_3d_wall_sides_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create walls index buffer.");

		return false;
	}

	::hw_3d_wall_sides_ibi_.clear();
	::hw_3d_wall_sides_ibi_.resize(index_count);

	return true;
}

void hw_3d_walls_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_wall_sides_ib_);
	::hw_3d_wall_sides_ibi_.clear();
}

bool hw_3d_walls_vb_create()
{
	const auto vertex_count = ::hw_3d_wall_side_count_ * ::hw_3d_vertices_per_wall_side;

	::hw_3d_wall_sides_vb_ = ::hw_vertex_buffer_create<Hw3dWallVertex>(
		bstone::RendererBufferUsageKind::static_draw,
		vertex_count
	);

	if (!::hw_3d_wall_sides_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create walls vertex buffer.");

		return false;
	}

	return true;
}

void hw_3d_walls_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_wall_sides_vi_);
}

bool hw_3d_walls_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dWallVertex>(
		::hw_3d_wall_sides_ib_,
		::hw_3d_wall_sides_vb_,
		::hw_3d_wall_sides_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create walls vertex input.");

		return false;
	}

	return true;
}

void hw_3d_walls_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_wall_sides_vb_);
}

bool hw_3d_walls_initialize()
{
	::vid_log();
	::vid_log("Initializing walls.");

	::hw_3d_xy_wall_map_.reserve(::hw_3d_wall_count_);

	::hw_3d_walls_to_render_.clear();

	::hw_3d_wall_side_draw_item_count_ = 0;
	::hw_3d_wall_side_draw_items_.clear();
	::hw_3d_wall_side_draw_items_.resize(::hw_3d_wall_side_count_);

	if (!::hw_3d_walls_ib_create())
	{
		return false;
	}

	if (!::hw_3d_walls_vb_create())
	{
		return false;
	}

	if (!::hw_3d_walls_vi_create())
	{
		return false;
	}

	return true;
}

void hw_3d_walls_uninitialize()
{
	::hw_3d_wall_count_ = 0;
	::hw_3d_wall_side_count_ = 0;
	::hw_3d_xy_wall_map_.clear();

	::hw_3d_wall_side_draw_item_count_ = 0;
	::hw_3d_wall_side_draw_items_.clear();

	::hw_3d_walls_ib_destroy();
	::hw_3d_walls_vb_destroy();
	::hw_3d_walls_vi_destroy();
}

bool hw_3d_pushwalls_ibi_create()
{
	const auto index_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_indices_per_wall_side;

	::hw_3d_pushwall_sides_ibi_.clear();
	::hw_3d_pushwall_sides_ibi_.resize(index_count);

	return true;
}

bool hw_3d_pushwalls_ib_create()
{
	const auto index_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_indices_per_wall_side;

	::hw_3d_pushwall_sides_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::stream_draw,
		2,
		index_count
	);

	if (!::hw_3d_pushwall_sides_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create pushwalls index buffer.");

		return false;
	}

	return true;
}

void hw_3d_pushwalls_ibi_destroy()
{
	::hw_3d_pushwall_sides_ibi_.clear();
}

void hw_3d_pushwalls_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_pushwall_sides_ib_);
}

bool hw_3d_pushwalls_vb_create()
{
	const auto vertex_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_vertices_per_wall_side;

	::hw_3d_pushwall_sides_vb_ = ::hw_vertex_buffer_create<Hw3dPushwallVertex>(
		bstone::RendererBufferUsageKind::dynamic_draw,
		vertex_count
	);

	if (!::hw_3d_pushwall_sides_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create pushwalls vertex buffer.");

		return false;
	}

	return true;
}

void hw_3d_pushwalls_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_pushwall_sides_vi_);
}

bool hw_3d_pushwalls_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dPushwallVertex>(
		::hw_3d_pushwall_sides_ib_,
		::hw_3d_pushwall_sides_vb_,
		::hw_3d_pushwall_sides_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create pushwalls vertex input.");

		return false;
	}

	return true;
}

void hw_3d_pushwalls_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_pushwall_sides_vb_);
}

bool hw_3d_pushwalls_initialize()
{
	::vid_log();
	::vid_log("Initializing pushwalls.");

	::hw_3d_xy_pushwall_map_.reserve(::hw_3d_pushwall_count_);

	::hw_3d_pushwall_to_wall_vbi_.clear();
	::hw_3d_pushwall_to_wall_vbi_.resize(::hw_3d_sides_per_wall * ::hw_3d_vertices_per_wall_side);

	::hw_3d_pushwalls_to_render_.clear();

	::hw_3d_pushwall_side_draw_item_count_ = 0;
	::hw_3d_pushwall_side_draw_items_.clear();
	::hw_3d_pushwall_side_draw_items_.resize(::hw_3d_pushwall_side_count_);

	if (!::hw_3d_pushwalls_ibi_create())
	{
		return false;
	}

	if (!::hw_3d_pushwalls_ib_create())
	{
		return false;
	}

	if (!::hw_3d_pushwalls_vb_create())
	{
		return false;
	}

	if (!::hw_3d_pushwalls_vi_create())
	{
		return false;
	}

	return true;
}

void hw_3d_pushwalls_uninitialize()
{
	::hw_3d_pushwall_count_ = 0;
	::hw_3d_pushwall_side_count_ = 0;
	::hw_3d_xy_pushwall_map_.clear();

	::hw_3d_pushwall_side_draw_item_count_ = 0;
	::hw_3d_pushwall_side_draw_items_.clear();

	::hw_3d_pushwalls_vi_destroy();

	::hw_3d_pushwalls_ibi_destroy();
	::hw_3d_pushwalls_ib_destroy();

	::hw_3d_pushwalls_vb_destroy();
}

bool hw_3d_door_sides_ibi_create()
{
	const auto index_count = ::hw_3d_door_count_ * ::hw_3d_indices_per_door_side;

	::hw_3d_door_sides_ibi_.clear();
	::hw_3d_door_sides_ibi_.resize(index_count);

	return true;
}

bool hw_3d_door_sides_ib_create()
{
	const auto index_count = ::hw_3d_door_count_ * ::hw_3d_indices_per_door_side;

	::hw_3d_door_sides_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::stream_draw,
		2,
		index_count
	);

	if (!::hw_3d_door_sides_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create doors index buffer.");

		return false;
	}

	return true;
}

void hw_3d_door_sides_ibi_destroy()
{
	::hw_3d_door_sides_ibi_.clear();
}

void hw_3d_door_sides_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_door_sides_ib_);

	::hw_3d_door_sides_ibi_.clear();
}

bool hw_3d_door_sides_vb_create()
{
	const auto vertex_count = ::hw_3d_door_count_ * ::hw_3d_indices_per_door_side;

	::hw_3d_door_sides_vb_ = ::hw_vertex_buffer_create<Hw3dDoorVertex>(
		bstone::RendererBufferUsageKind::dynamic_draw,
		vertex_count
	);

	if (!::hw_3d_door_sides_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create doors vertex buffer.");

		return false;
	}

	return true;
}

void hw_3d_door_sides_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_door_sides_vb_);
}

void hw_3d_door_sides_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_door_sides_vi_);
}

bool hw_3d_door_sides_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dDoorVertex>(
		::hw_3d_door_sides_ib_,
		::hw_3d_door_sides_vb_,
		::hw_3d_door_sides_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create doors vertex input.");

		return false;
	}

	return true;
}

bool hw_3d_door_sides_initialize()
{
	::vid_log();
	::vid_log("Initializing doors.");

	::hw_3d_xy_door_map_.reserve(::hw_3d_door_count_);

	const auto max_draw_item_count = ::hw_3d_door_count_ * hw_3d_halves_per_door;

	::hw_3d_door_draw_item_count_ = 0;
	::hw_3d_door_draw_items_.clear();
	::hw_3d_door_draw_items_.resize(max_draw_item_count);

	if (!::hw_3d_door_sides_ibi_create())
	{
		return false;
	}

	if (!::hw_3d_door_sides_ib_create())
	{
		return false;
	}

	if (!::hw_3d_door_sides_vb_create())
	{
		return false;
	}

	if (!::hw_3d_door_sides_vi_create())
	{
		return false;
	}

	return true;
}

void hw_3d_door_sides_uninitialize()
{
	::hw_3d_xy_door_map_.clear();

	::hw_3d_door_draw_item_count_ = 0;
	::hw_3d_door_draw_items_.clear();

	::hw_3d_door_sides_vi_destroy();

	::hw_3d_door_sides_ibi_destroy();
	::hw_3d_door_sides_ib_destroy();

	::hw_3d_door_sides_vb_destroy();
}

void hw_palette_update(
	const int first_index,
	const int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = ::vid_vga_palette[color_index];
		auto& hw_color = ::hw_palette_[color_index];

		hw_color = ::hw_vga_color_to_r8g8b8a8(
			vga_color.r,
			vga_color.g,
			vga_color.b
		);
	}
}

void hw_palette_initialize()
{
	::hw_palette_ = {};

	::hw_palette_update(0, bstone::RgbPalette::get_max_color_count());

	::hw_default_palette_ = bstone::R8g8b8a8Palette{};

	for (int i = 0; i < bstone::RgbPalette::get_max_color_count(); ++i)
	{
		const auto vga_color = ::vgapal + (i * 3);
		auto& hw_color = ::hw_default_palette_[i];

		hw_color = ::hw_vga_color_to_r8g8b8a8(vga_color[0], vga_color[1], vga_color[2]);
	}
}

void hw_dimensions_calculate()
{
	auto src_param = ::vid_create_downscale_param();

	::vid_calculate_window_elements_dimensions(src_param, ::downscale_);

	::vid_dimensions_vga_calculate();
}

void hw_2d_matrix_model_build()
{
	::hw_2d_matrix_model_ = glm::identity<glm::mat4>();
}

void hw_2d_matrix_view_build()
{
	::hw_2d_matrix_view_ = glm::identity<glm::mat4>();
}

void hw_2d_matrix_projection_build()
{
	::hw_2d_matrix_projection_ = glm::orthoRH_NO(
		0.0, // left
		static_cast<double>(::downscale_.screen_width_), // right
		0.0, // bottom
		static_cast<double>(::downscale_.screen_height_), // top
		0.0, // zNear
		1.0 // zFar
	);
}

void hw_2d_matrices_build()
{
	::hw_2d_matrix_model_build();
	::hw_2d_matrix_view_build();
	::hw_2d_matrix_projection_build();
}

void hw_3d_camera_parameters_calculate()
{
	// Vertical FOV.
	//
	// V = 2 * arctan(tan(H / 2) * (h / w))
	//

	const auto ref_r_ratio = static_cast<double>(::vga_ref_height_4x3) / static_cast<double>(::vga_ref_width);

	const auto half_hfov_deg = ::hw_3d_ref_camera_hfov_deg / 2.0;
	const auto half_hfov_rad = (::m_pi() / 180.0) * half_hfov_deg;
	const auto tan_half_hfov_rad = std::tan(half_hfov_rad);
	const auto half_vfov_rad = tan_half_hfov_rad * ref_r_ratio;

	// Radians.
	const auto vfov_rad = 2.0 * half_vfov_rad;
	::hw_3d_camera_vfov_rad = vfov_rad;

	// Degrees.
	const auto half_vfov_deg = half_vfov_rad * (180.0 / ::m_pi());
	const auto vfov_deg = 2.0 * half_vfov_deg;
	::hw_3d_camera_vfov_deg = vfov_deg;


	// Distances to the planes.
	//
	::hw_3d_camera_near_distance = 0.05;
	::hw_3d_camera_far_distance = (std::sqrt(2.0) * ::hw_3d_map_dimension_d) + 0.5;
}

void hw_matrix_texture_build()
{
	::hw_matrix_texture_ = ::hw_renderer_->csc_get_texture();
}

void hw_3d_matrix_bs_to_r_build()
{
	//
	// |  0 y   0   0 |
	// |  0 0 z*1.2 0 |
	// | -x 0   0   0 |
	// |  0 0   0   1 |
	//

	const auto m_11 = 0.0F;
	const auto m_12 = 1.0F;
	const auto m_13 = 0.0F;
	const auto m_14 = 0.0F;

	const auto m_21 = 0.0F;
	const auto m_22 = 0.0F;
	const auto m_23 = 1.2F;
	const auto m_24 = 0.0F;

	const auto m_31 = -1.0F;
	const auto m_32 = 0.0F;
	const auto m_33 = 0.0F;
	const auto m_34 = 0.0F;

	const auto m_41 = 0.0F;
	const auto m_42 = 0.0F;
	const auto m_43 = 0.0F;
	const auto m_44 = 1.0F;

	::hw_3d_matrix_bs_to_r_ = glm::mat4
	{
		m_11, m_21, m_31, m_41,
		m_12, m_22, m_32, m_42,
		m_13, m_23, m_33, m_43,
		m_14, m_24, m_34, m_44,
	};
}

void hw_3d_matrix_model_build()
{
	::hw_3d_matrix_model_ = glm::identity<glm::mat4>();
}

void hw_3d_matrix_view_build()
{
	if (!::player)
	{
		::hw_3d_matrix_view_ = glm::identity<glm::mat4>();

		return;
	}

	auto view_matrix = glm::identity<glm::dmat4>();

	view_matrix = glm::rotate(view_matrix, ::hw_3d_player_angle_rad_, glm::dvec3{0.0, 0.0, 1.0});
	view_matrix = glm::translate(view_matrix, -::hw_3d_view_position_);

	::hw_3d_matrix_view_ = view_matrix;
}

void hw_3d_matrix_projection_build()
{
	const auto perspective = glm::perspectiveFovRH_NO(
		static_cast<float>(::hw_3d_camera_vfov_rad),
		static_cast<float>(::downscale_.screen_viewport_width_),
		static_cast<float>(::downscale_.screen_viewport_height_),
		static_cast<float>(::hw_3d_camera_near_distance),
		static_cast<float>(::hw_3d_camera_far_distance)
	);

	const auto& correction = ::hw_renderer_->csc_get_projection();

	::hw_3d_matrix_projection_ = correction * perspective * ::hw_3d_matrix_bs_to_r_;
}

void hw_3d_matrices_build()
{
	::hw_3d_camera_parameters_calculate();

	::hw_3d_matrix_bs_to_r_build();
	::hw_3d_matrix_model_build();
	::hw_3d_matrix_view_build();
	::hw_3d_matrix_projection_build();
}

void hw_matrices_build()
{
	::hw_matrix_texture_build();
	::hw_2d_matrices_build();
	::hw_3d_matrices_build();
}

void hw_2d_sampler_ui_set_default_state()
{
	::hw_2d_ui_s_state_.min_filter_ = bstone::RendererFilterKind::nearest;
	::hw_2d_ui_s_state_.mag_filter_ = bstone::RendererFilterKind::nearest;
	::hw_2d_ui_s_state_.mipmap_mode_ = bstone::RendererMipmapMode::none;
	::hw_2d_ui_s_state_.address_mode_u_ = bstone::RendererAddressMode::clamp;
	::hw_2d_ui_s_state_.address_mode_v_ = bstone::RendererAddressMode::clamp;
	::hw_2d_ui_s_state_.anisotropy_ = bstone::RendererSampler::anisotropy_min;
}

void hw_2d_sampler_ui_update()
{
	::hw_2d_ui_s_state_.min_filter_ = ::vid_configuration_.hw_2d_texture_filter_;
	::hw_2d_ui_s_state_.mag_filter_ = ::vid_configuration_.hw_2d_texture_filter_;

	if (hw_2d_ui_s_ != nullptr)
	{
		auto param = bstone::RendererSamplerUpdateParam{};
		param.state_ = ::hw_2d_ui_s_state_;
		::hw_2d_ui_s_->update(param);
	}
}

void hw_2d_sampler_ui_destroy()
{
	if (::hw_2d_ui_s_)
	{
		::hw_renderer_->sampler_destroy(::hw_2d_ui_s_);
		::hw_2d_ui_s_ = nullptr;
	}
}

bool hw_2d_sampler_ui_create()
{
	auto param = bstone::RendererSamplerCreateParam{};
	param.state_ = ::hw_2d_ui_s_state_;

	::hw_2d_ui_s_ = ::hw_renderer_->sampler_create(param);

	if (::hw_2d_ui_s_ == nullptr)
	{
		::vid_log_hw_renderer_error("Failed to create UI sampler.");

		return false;
	}

	return true;
}

void hw_3d_sampler_sprite_set_default_state()
{
	::hw_3d_sprite_s_state_.min_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_sprite_s_state_.mag_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_sprite_s_state_.mipmap_mode_ = bstone::RendererMipmapMode::nearest;
	::hw_3d_sprite_s_state_.address_mode_u_ = bstone::RendererAddressMode::clamp;
	::hw_3d_sprite_s_state_.address_mode_v_ = bstone::RendererAddressMode::clamp;
	::hw_3d_sprite_s_state_.anisotropy_ = bstone::RendererSampler::anisotropy_min;
}

void hw_3d_sampler_sprite_update()
{
	::hw_3d_sprite_s_state_.min_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;
	::hw_3d_sprite_s_state_.mag_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;

	::hw_3d_sprite_s_state_.mipmap_mode_ = ::hw_config_texture_mipmap_filter_to_renderer(
		::vid_configuration_.hw_3d_texture_mipmap_filter_);

	::hw_3d_sprite_s_state_.anisotropy_ = ::hw_config_texture_anisotropy_to_renderer(
		::vid_configuration_.hw_3d_texture_anisotropy_,
		::vid_configuration_.hw_3d_texture_anisotropy_value_
	);

	if (::hw_3d_sprite_s_ != nullptr)
	{
		auto param = bstone::RendererSamplerUpdateParam{};
		param.state_ = ::hw_3d_sprite_s_state_;
		::hw_3d_sprite_s_->update(param);
	}
}

void hw_3d_sampler_sprite_destroy()
{
	::hw_sampler_destroy(::hw_3d_sprite_s_);
}

bool hw_3d_sampler_sprite_create()
{
	auto param = bstone::RendererSamplerCreateParam{};
	param.state_ = ::hw_3d_sprite_s_state_;

	::hw_3d_sprite_s_ = ::hw_renderer_->sampler_create(param);

	if (::hw_3d_sprite_s_ == nullptr)
	{
		::vid_log_hw_renderer_error("Failed to create sprite sampler.");

		return false;
	}

	return true;
}

void hw_3d_sampler_wall_set_default_state()
{
	::hw_3d_wall_s_state_.min_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_wall_s_state_.mag_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_wall_s_state_.mipmap_mode_ = bstone::RendererMipmapMode::nearest;
	::hw_3d_wall_s_state_.address_mode_u_ = bstone::RendererAddressMode::repeat;
	::hw_3d_wall_s_state_.address_mode_v_ = bstone::RendererAddressMode::repeat;
	::hw_3d_wall_s_state_.anisotropy_ = bstone::RendererSampler::anisotropy_min;
}

void hw_3d_sampler_wall_update()
{
	::hw_3d_wall_s_state_.min_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;
	::hw_3d_wall_s_state_.mag_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;

	::hw_3d_wall_s_state_.mipmap_mode_ = ::hw_config_texture_mipmap_filter_to_renderer(
		::vid_configuration_.hw_3d_texture_mipmap_filter_);

	::hw_3d_wall_s_state_.anisotropy_ = ::hw_config_texture_anisotropy_to_renderer(
		::vid_configuration_.hw_3d_texture_anisotropy_,
		::vid_configuration_.hw_3d_texture_anisotropy_value_
	);

	if (::hw_3d_wall_s_ != nullptr)
	{
		auto param = bstone::RendererSamplerUpdateParam{};
		param.state_ = ::hw_3d_wall_s_state_;
		::hw_3d_wall_s_->update(param);
	}
}

void hw_3d_sampler_wall_destroy()
{
	::hw_sampler_destroy(::hw_3d_wall_s_);
}

bool hw_3d_sampler_wall_create()
{
	auto param = bstone::RendererSamplerCreateParam{};
	param.state_ = ::hw_3d_wall_s_state_;

	::hw_3d_wall_s_ = ::hw_renderer_->sampler_create(param);

	if (::hw_3d_wall_s_ == nullptr)
	{
		::vid_log_hw_renderer_error("Failed to create wall sampler.");

		return false;
	}

	return true;
}

void hw_3d_player_weapon_vb_update()
{
	auto vertices = Hw3dPlayerWeaponVbi{};
	vertices.resize(::hw_3d_vertices_per_sprite);

	const auto dimension = static_cast<float>(bstone::Sprite::dimension);
	const auto half_dimension = 0.5F * dimension;

	auto vertex_index = 0;

	// Bottom-left.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{-half_dimension, 0.0F, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{half_dimension, 0.0F, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{half_dimension, dimension, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = vertices[vertex_index];
		vertex.xyz_ = HwVertexPosition{-half_dimension, dimension, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}

	// Update vertex buffer.
	//
	::hw_vertex_buffer_update(
		::hw_3d_player_weapon_vb_,
		0,
		::hw_3d_vertices_per_sprite,
		vertices.data()
	);
}

void hw_3d_player_weapon_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_player_weapon_ib_);
}

bool hw_3d_player_weapon_ib_create()
{
	::hw_3d_player_weapon_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::static_draw,
		1,
		::hw_3d_indices_per_sprite
	);

	if (!::hw_3d_player_weapon_ib_)
	{
		::vid_log_hw_renderer_error("Failed to create weapon index buffer.");

		return false;
	}

	return true;
}

void hw_3d_player_weapon_ib_update()
{
	using Indices = std::array<std::uint8_t, ::hw_3d_indices_per_sprite>;

	auto indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	::hw_index_buffer_update(
		::hw_3d_player_weapon_ib_,
		0,
		::hw_3d_indices_per_sprite,
		indices.data()
	);
}

void hw_3d_player_weapon_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_player_weapon_vb_);
}

bool hw_3d_player_weapon_vb_create()
{
	::hw_3d_player_weapon_vb_ = ::hw_vertex_buffer_create<Hw3dPlayerWeaponVertex>(
		bstone::RendererBufferUsageKind::static_draw,
		::hw_3d_vertices_per_sprite
	);

	if (!::hw_3d_player_weapon_vb_)
	{
		::vid_log_hw_renderer_error("Failed to create weapon vertex buffer.");

		return false;
	}

	return true;
}

void hw_3d_player_weapon_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_player_weapon_vi_);
}

bool hw_3d_player_weapon_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dPlayerWeaponVertex>(
		::hw_3d_player_weapon_ib_,
		::hw_3d_player_weapon_vb_,
		::hw_3d_player_weapon_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to create weapon vertex input.");

		return false;
	}

	return true;
}

void hw_3d_player_weapon_model_matrix_update()
{
	const auto& assets_info = AssetsInfo{};

	const auto aog_scale = 25.0 / 9.0;
	const auto ps_scale = 91.0 / 45.0;

	const auto game_scalar = (assets_info.is_ps() ? ps_scale : aog_scale);
	const auto scalar = game_scalar * ::vga_height_scale;

	const auto translate_x = 0.5 * static_cast<double>(::downscale_.screen_viewport_width_);

	const auto bounce_offset = (assets_info.is_aog() ? 0 : ::player_get_weapon_bounce_offset());
	const auto translate_y = ::vga_height_scale * bstone::FixedPoint{-bounce_offset}.to_double();

	const auto translate_v = glm::vec3
	{
		static_cast<float>(translate_x),
		static_cast<float>(translate_y),
		0.0
	};

	const auto& identity = glm::identity<glm::mat4>();
	const auto& translate = glm::translate(identity, translate_v);

	const auto& scale = glm::scale(identity, glm::vec3{scalar, scalar, 0.0F});

	::hw_3d_player_weapon_model_matrix_ = translate * scale;
}

void hw_3d_player_weapon_view_matrix_update()
{
	::hw_3d_player_weapon_view_matrix_ = glm::identity<glm::mat4>();
}

void hw_3d_player_weapon_projection_matrix_build()
{
	const auto ortho = glm::orthoRH_NO(
		0.0F, // left
		static_cast<float>(::downscale_.screen_viewport_width_), // right
		0.0F, // bottom
		static_cast<float>(::downscale_.screen_viewport_height_), // top
		0.0F, // zNear
		1.0F // zFar
	);

	const auto& correction = ::hw_renderer_->csc_get_projection();

	::hw_3d_player_weapon_projection_matrix_ = correction * ortho;
}

void hw_3d_player_weapon_sampler_set_default_state()
{
	::hw_3d_player_weapon_s_state_.min_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_player_weapon_s_state_.mag_filter_ = bstone::RendererFilterKind::nearest;
	::hw_3d_player_weapon_s_state_.mipmap_mode_ = bstone::RendererMipmapMode::none;
	::hw_3d_player_weapon_s_state_.address_mode_u_ = bstone::RendererAddressMode::clamp;
	::hw_3d_player_weapon_s_state_.address_mode_v_ = bstone::RendererAddressMode::clamp;
	::hw_3d_player_weapon_s_state_.anisotropy_ = bstone::RendererSampler::anisotropy_min;
}

void hw_3d_player_weapon_sampler_update()
{
	::hw_3d_player_weapon_s_state_.min_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;
	::hw_3d_player_weapon_s_state_.mag_filter_ = ::vid_configuration_.hw_3d_texture_image_filter_;

	if (::hw_3d_player_weapon_s_ != nullptr)
	{
		auto param = bstone::RendererSamplerUpdateParam{};
		param.state_ = ::hw_3d_player_weapon_s_state_;
		::hw_3d_player_weapon_s_->update(param);
	}
}

void hw_3d_player_weapon_sampler_destroy()
{
	if (::hw_3d_player_weapon_s_)
	{
		::hw_renderer_->sampler_destroy(::hw_3d_player_weapon_s_);
		::hw_3d_player_weapon_s_ = nullptr;
	}
}

bool hw_3d_player_weapon_sampler_create()
{
	auto param = bstone::RendererSamplerCreateParam{};
	param.state_ = ::hw_3d_player_weapon_s_state_;

	::hw_3d_player_weapon_s_ = ::hw_renderer_->sampler_create(param);

	if (!::hw_3d_player_weapon_s_)
	{
		::vid_log_hw_renderer_error("Failed to create weapon sampler.");

		return false;
	}

	return true;
}

void hw_3d_player_weapon_uninitialize()
{
	::hw_3d_player_weapon_vi_destroy();
	::hw_3d_player_weapon_ib_destroy();
	::hw_3d_player_weapon_vb_destroy();
	::hw_3d_player_weapon_sampler_destroy();
}

bool hw_3d_player_weapon_initialize()
{
	::vid_log();
	::vid_log("Initializing player's weapon.");

	if (!::hw_3d_player_weapon_ib_create())
	{
		return false;
	}

	if (!::hw_3d_player_weapon_vb_create())
	{
		return false;
	}

	if (!::hw_3d_player_weapon_vi_create())
	{
		return false;
	}

	if (!::hw_3d_player_weapon_sampler_create())
	{
		return false;
	}

	::hw_3d_player_weapon_ib_update();
	::hw_3d_player_weapon_vb_update();

	::hw_3d_player_weapon_model_matrix_update();
	::hw_3d_player_weapon_view_matrix_update();
	::hw_3d_player_weapon_projection_matrix_build();

	return true;
}

void hw_fade_sampler_destroy()
{
	::hw_sampler_destroy(::hw_fade_s_);
}

bool hw_fade_sampler_create()
{
	auto param = bstone::RendererSamplerCreateParam{};
	param.state_.min_filter_ = bstone::RendererFilterKind::nearest;
	param.state_.mag_filter_ = bstone::RendererFilterKind::nearest;
	param.state_.mipmap_mode_ = bstone::RendererMipmapMode::none;
	param.state_.address_mode_u_ = bstone::RendererAddressMode::repeat;
	param.state_.address_mode_v_ = bstone::RendererAddressMode::repeat;
	param.state_.anisotropy_ = bstone::RendererSampler::anisotropy_min;

	::hw_fade_s_ = ::hw_renderer_->sampler_create(param);

	if (!::hw_fade_s_)
	{
		::vid_log_hw_renderer_error("Failed to create fade sampler.");

		return false;
	}

	return true;
}

void hw_samplers_set_default_states()
{
	::hw_2d_sampler_ui_set_default_state();
	::hw_3d_sampler_sprite_set_default_state();
	::hw_3d_sampler_wall_set_default_state();
}

void hw_samplers_uninitialize()
{
	::hw_2d_sampler_ui_destroy();
	::hw_3d_sampler_sprite_destroy();
	::hw_3d_sampler_wall_destroy();
	::hw_fade_sampler_destroy();
}

bool hw_samplers_initialize()
{
	::vid_log();
	::vid_log("Initializing samplers.");

	if (!::hw_2d_sampler_ui_create())
	{
		return false;
	}

	if (!::hw_3d_sampler_sprite_create())
	{
		return false;
	}

	if (!::hw_3d_sampler_wall_create())
	{
		return false;
	}

	if (!::hw_fade_sampler_create())
	{
		return false;
	}

	return true;
}

void hw_command_manager_destroy()
{
	::hw_command_manager_ = nullptr;
}

bool hw_command_manager_create()
{
	::hw_command_manager_ = bstone::RendererCommandManagerFactory::create();

	if (!::hw_command_manager_)
	{
		return false;
	}

	return true;
}

void hw_command_buffer_common_destroy()
{
	if (::hw_common_command_buffer_ == nullptr)
	{
		return;
	}

	::hw_command_manager_->buffer_remove(::hw_common_command_buffer_);
	::hw_common_command_buffer_ = nullptr;
}

bool hw_command_buffer_common_create()
{
	auto param = bstone::RendererCommandManagerBufferAddParam{};
	param.initial_size_ = ::hw_common_command_buffer_initial_size;
	param.resize_delta_size_ = ::hw_common_command_buffer_resize_delta_size;

	::hw_common_command_buffer_ = ::hw_command_manager_->buffer_add(param);

	if (!::hw_common_command_buffer_)
	{
		::vid_log_hw_renderer_error("Failed to create common command buffer.");

		return false;
	}

	return true;
}

void hw_command_buffer_2d_destroy()
{
	if (::hw_2d_command_buffer_ == nullptr)
	{
		return;
	}

	::hw_command_manager_->buffer_remove(::hw_2d_command_buffer_);
	::hw_2d_command_buffer_ = nullptr;
}

bool hw_command_buffer_2d_create()
{
	auto param = bstone::RendererCommandManagerBufferAddParam{};
	param.initial_size_ = ::hw_2d_command_buffer_initial_size;
	param.resize_delta_size_ = ::hw_2d_command_buffer_resize_delta_size;

	::hw_2d_command_buffer_ = ::hw_command_manager_->buffer_add(param);

	if (!::hw_2d_command_buffer_)
	{
		::vid_log_hw_renderer_error("Failed to create 2D command buffer.");

		return false;
	}

	return true;
}

void hw_command_buffer_3d_destroy()
{
	if (::hw_3d_command_buffer_ == nullptr)
	{
		return;
	}

	::hw_command_manager_->buffer_remove(::hw_3d_command_buffer_);
	::hw_3d_command_buffer_ = nullptr;
}

bool hw_command_buffer_3d_create()
{
	auto param = bstone::RendererCommandManagerBufferAddParam{};
	param.initial_size_ = ::hw_3d_command_buffer_initial_size;
	param.resize_delta_size_ = ::hw_3d_command_buffer_resize_delta_size;

	::hw_3d_command_buffer_ = ::hw_command_manager_->buffer_add(param);

	if (!::hw_3d_command_buffer_)
	{
		::vid_log_hw_renderer_error("Failed to create 3D command buffer.");

		return false;
	}

	return true;
}

void hw_command_manager_uninitialize()
{
	::hw_command_buffer_3d_destroy();
	::hw_command_buffer_2d_destroy();
	::hw_command_buffer_common_destroy();
	::hw_command_manager_destroy();
}

bool hw_command_manager_initialize()
{
	::vid_log();
	::vid_log("Initializing command managers.");

	if (!::hw_command_manager_create())
	{
		return false;
	}

	if (!::hw_command_buffer_common_create())
	{
		return false;
	}

	if (!::hw_command_buffer_3d_create())
	{
		return false;
	}

	if (!::hw_command_buffer_2d_create())
	{
		return false;
	}

	return true;
}

void hw_3d_fade_ib_destroy()
{
	::hw_index_buffer_destroy(::hw_3d_fade_ib_);
}

bool hw_3d_fade_ib_create()
{
	::hw_3d_fade_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::static_draw,
		1,
		6
	);

	if (!::hw_3d_fade_ib_)
	{
		::vid_log_hw_renderer_error("Failed to 3D fade index buffer.");

		return false;
	}

	return true;
}

void hw_3d_fade_vb_destroy()
{
	::hw_vertex_buffer_destroy(::hw_3d_fade_vb_);
}

bool hw_3d_fade_vb_create()
{
	::hw_3d_fade_vb_ = ::hw_vertex_buffer_create<Hw3dFadeVertex>(
		bstone::RendererBufferUsageKind::static_draw,
		4
	);

	if (!::hw_3d_fade_vb_)
	{
		::vid_log_hw_renderer_error("Failed to 3D fade vertex buffer.");

		return false;
	}

	return true;
}

void hw_3d_fade_vi_destroy()
{
	::hw_vertex_input_destroy(::hw_3d_fade_vi_);
}

bool hw_3d_fade_vi_create()
{
	const auto result = ::hw_vertex_input_create<Hw3dFadeVertex>(
		::hw_3d_fade_ib_,
		::hw_3d_fade_vb_,
		::hw_3d_fade_vi_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to 3D fade vertex input.");

		return false;
	}

	return true;
}

void hw_3d_fade_ib_update()
{
	using Indices = std::array<std::uint8_t, 6>;

	const auto& indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	::hw_index_buffer_update(
		::hw_3d_fade_ib_,
		0,
		6,
		indices.data()
	);
}

void hw_3d_fade_vb_update()
{
	auto vertices = Hw3dFadeVbi{};
	vertices.resize(4);

	auto vertex_index = 0;

	const auto width_f = ::downscale_.screen_viewport_width_;
	const auto height_f = ::downscale_.screen_viewport_height_;

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{0.0F, 0.0F, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{width_f, 0.0F, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{width_f, height_f, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{0.0F, height_f, 0.0F};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}

	::hw_vertex_buffer_update<Hw3dFadeVertex>(
		::hw_3d_fade_vb_,
		0,
		4,
		vertices.data()
	);
}

void hw_3d_fade_texture_2d_destroy()
{
	if (::hw_texture_manager_ != nullptr)
	{
		::hw_texture_manager_->solid_1x1_destroy(bstone::HwTextureManagerSolid1x1Id::fade_3d);
	}

	::hw_3d_fade_t2d_ = nullptr;
}

bool hw_3d_fade_texture_2d_create()
{
	if (!::hw_texture_manager_->solid_1x1_create(bstone::HwTextureManagerSolid1x1Id::fade_3d))
	{
		::vid_log_hw_renderer_error("Failed to create 1x1 3D fade texture.");

		return false;
	}

	::hw_3d_fade_t2d_ = ::hw_texture_manager_->solid_1x1_get(bstone::HwTextureManagerSolid1x1Id::fade_3d);

	if (::hw_3d_fade_t2d_ == nullptr)
	{
		::vid_log_hw_renderer_error("Failed to get 1x1 3D fade texture.");

		return false;
	}

	return true;
}

void hw_3d_fade_uninitialize()
{
	::hw_3d_fade_vi_destroy();
	::hw_3d_fade_vb_destroy();
	::hw_3d_fade_ib_destroy();
	::hw_3d_fade_texture_2d_destroy();
}

void hw_texture_manager_destroy()
{
	if (::hw_texture_manager_ == nullptr)
	{
		return;
	}

	::hw_texture_manager_->uninitialize();
	::hw_texture_manager_ = nullptr;
}

bool hw_texture_manager_create()
{
	::hw_texture_manager_ = bstone::HwTextureManagerFactory::create(
		::hw_renderer_,
		&::vid_sprite_cache
	);

	if (!::hw_texture_manager_->is_initialized())
	{
		::vid_log_hw_texture_manager_error("Failed to initialize texture manager.");

		return false;
	}

	return true;
}

bool hw_3d_fade_initialize()
{
	::vid_log();
	::vid_log("Initializing 3D fade.");

	if (!::hw_3d_fade_ib_create())
	{
		return false;
	}

	if (!::hw_3d_fade_vb_create())
	{
		return false;
	}

	if (!::hw_3d_fade_vi_create())
	{
		return false;
	}

	if (!::hw_3d_fade_texture_2d_create())
	{
		return false;
	}

	::hw_3d_fade_ib_update();
	::hw_3d_fade_vb_update();

	return true;
}

void hw_screen_common_refresh()
{
	::hw_shading_mode_ = 0;
	::hw_bs_shade_max_ = ::shade_max;
	::hw_bs_normal_shade_ = ::normalshade;
	::hw_bs_height_numerator_ = ::heightnumerator;
	::hw_bs_lighting_ = 0;


	auto command_buffer = ::hw_common_command_buffer_;

	command_buffer->enable(true);

	command_buffer->write_begin();

	// Build commands.
	//

	// Set shader stage.
	//
	{
		auto& command = *command_buffer->write_shader_stage();
		command.shader_stage_ = ::hw_shader_stage_;
	}

	// Set texture matrix.
	//
	if (::hw_matrix_texture_.is_modified())
	{
		::hw_matrix_texture_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_texture_mat_;
		command.value_ = ::hw_matrix_texture_;
	}

	// Set shading mode.
	//
	if (::hw_shading_mode_.is_modified())
	{
		::hw_shading_mode_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_int32();
		command.variable_ = ::hw_shader_variable_shading_mode_;
		command.value_ = ::hw_shading_mode_;
	}

	// Set shade_max.
	//
	if (::hw_bs_shade_max_.is_modified())
	{
		::hw_bs_shade_max_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_float32();
		command.variable_ = ::hw_shader_variable_shade_max_;
		command.value_ = static_cast<float>(::hw_bs_shade_max_);
	}

	// Set normal_shade.
	//
	if (::hw_bs_normal_shade_.is_modified())
	{
		::hw_bs_normal_shade_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_float32();
		command.variable_ = ::hw_shader_variable_normal_shade_;
		command.value_ = static_cast<float>(::hw_bs_normal_shade_);
	}

	// Set height_numerator.
	//
	if (::hw_bs_height_numerator_.is_modified())
	{
		::hw_bs_height_numerator_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_float32();
		command.variable_ = ::hw_shader_variable_height_numerator_;
		command.value_ = bstone::FixedPoint{::hw_bs_height_numerator_}.to_float();
	}

	// Set extra_lighting.
	//
	if (::hw_bs_lighting_.is_modified())
	{
		::hw_bs_lighting_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_float32();
		command.variable_ = ::hw_shader_variable_extra_lighting_;
		command.value_ = static_cast<float>(::hw_bs_lighting_);
	}

	// Set view_direction.
	//
	if (::vid_hw_is_draw_3d_ && ::hw_bs_view_direction_.is_modified())
	{
		::hw_bs_view_direction_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_vec2();
		command.variable_ = ::hw_shader_variable_view_direction_;
		command.value_ = ::hw_bs_view_direction_;
	}

	// Set view_position.
	//
	if (::vid_hw_is_draw_3d_ && ::hw_bs_view_position_.is_modified())
	{
		::hw_bs_view_position_.set_is_modified(false);

		auto& command = *command_buffer->write_shader_variable_vec2();
		command.variable_ = ::hw_shader_variable_view_position_;
		command.value_ = ::hw_bs_view_position_;
	}

	// Finalize.
	//
	command_buffer->write_end();
}

void hw_screen_2d_refresh()
{
	// Update 2D texture.
	//
	{
		::hw_texture_manager_->ui_update();
	}

	// Update fade color.
	//
	if (::hw_2d_fade_is_enabled_)
	{
		::hw_texture_manager_->solid_1x1_update(bstone::HwTextureManagerSolid1x1Id::fade_2d, ::hw_2d_fade_color_);
	}


	auto command_buffer = ::hw_2d_command_buffer_;

	command_buffer->enable(true);

	command_buffer->write_begin();

	// Build commands.
	//

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_culling();
		command.is_enabled_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_depth_test();
		command.is_enabled_ = false;
	}

	// Set viewport.
	//
	{
		auto& command = *command_buffer->write_viewport();
		command.x_ = ::downscale_.window_viewport_left_width_;
		command.y_ = ::downscale_.window_viewport_bottom_height_;
		command.width_ = ::downscale_.screen_width_;
		command.height_ = ::downscale_.screen_height_;
		command.min_depth_ = 0.0F;
		command.max_depth_ = 0.0F;
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_sampler();
		command.sampler_ = ::hw_2d_ui_s_;
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_model_mat_;
		command.value_ = ::hw_2d_matrix_model_;
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_view_mat_;
		command.value_ = ::hw_2d_matrix_view_;
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_projection_mat_;
		command.value_ = ::hw_2d_matrix_projection_;
	}

	// Fillers.
	//
	if (!::vid_configuration_.is_ui_stretched_)
	{
		{
			const auto texture_2d = ::hw_2d_white_t2d_1x1_;
			auto& command = *command_buffer->write_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_vertex_input();
			command.vertex_input_ = ::hw_2d_fillers_vi_;
		}

		{
			auto count = 0;
			auto index_offset = 0;

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

			auto& command = *command_buffer->write_draw_quads();
			command.count_ = count;
			command.index_offset_ = index_offset;
		}
	}

	// Draw 2D (UI, menu, etc.).
	//
	{
		if (::vid_is_hud)
		{
			{
				auto& command = *command_buffer->write_blending();
				command.is_enabled_ = true;
			}

			// Set blending function.
			//
			{
				auto& command = *command_buffer->write_blending_function();
				command.src_factor_ = bstone::RendererBlendingFactor::src_alpha;
				command.dst_factor_ = bstone::RendererBlendingFactor::one_minus_src_alpha;
			}
		}

		{
			auto& command = *command_buffer->write_texture();
			command.texture_2d_ = ::hw_2d_ui_t2d_;
		}

		{
			auto& command = *command_buffer->write_vertex_input();
			command.vertex_input_ = ::hw_2d_ui_vi_;
		}

		{
			const auto index_offset = (::vid_configuration_.is_ui_stretched_
				?
				::hw_2d_stretched_index_offset_
				:
				::hw_2d_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_quads();
			command.count_ = 1;
			command.index_offset_ = index_offset;
		}

		if (::vid_is_hud)
		{
			auto& command = *command_buffer->write_blending();
			command.is_enabled_ = false;
		}
	}

	// 2D fade in or out.
	//
	if (::hw_2d_fade_is_enabled_)
	{
		// Enable blending.
		//
		{
			auto& command = *command_buffer->write_blending();
			command.is_enabled_ = true;
		}

		// Set blending function.
		//
		{
			auto& command = *command_buffer->write_blending_function();
			command.src_factor_ = bstone::RendererBlendingFactor::src_alpha;
			command.dst_factor_ = bstone::RendererBlendingFactor::one_minus_src_alpha;
		}

		// Set texture.
		//
		{
			auto& command = *command_buffer->write_texture();
			command.texture_2d_ = ::hw_2d_fade_t2d_;
		}

		// Set sampler.
		//
		{
			auto& command = *command_buffer->write_sampler();
			command.sampler_ = ::hw_fade_s_;
		}

		// Set vertex input.
		//
		{
			auto& command = *command_buffer->write_vertex_input();
			command.vertex_input_ = ::hw_2d_ui_vi_;
		}

		// Draw the quad.
		//
		{
			const auto index_offset = (::vid_configuration_.is_ui_stretched_
				?
				::hw_2d_stretched_index_offset_
				:
				::hw_2d_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_quads();
			command.count_ = 1;
			command.index_offset_ = index_offset;
		}

		// Disable blending.
		//
		{
			auto& command = *command_buffer->write_blending();
			command.is_enabled_ = false;
		}
	}

	// Finalize.
	//
	command_buffer->write_end();
}

bool hw_3d_dbg_is_tile_vertex_visible(
	const int x,
	const int y)
{
	const auto& wall_direction = glm::dvec2
	{
		::hw_3d_player_position_[0] - static_cast<double>(x),
		::hw_3d_player_position_[1] - static_cast<double>(y)
	};

	const auto cosine_between_directions = glm::dot(
		wall_direction, ::hw_3d_view_direction_);

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

void hw_3d_walls_render()
{
	if (::hw_3d_wall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_wall_side_draw_items_;

	if (::vid_configuration_.hw_dbg_draw_all_)
	{
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
	}
	else
	{
		if (::hw_3d_walls_to_render_.empty())
		{
			return;
		}

		const auto wall_map_end_it = ::hw_3d_xy_wall_map_.cend();

		for (const auto wall_xy : ::hw_3d_walls_to_render_)
		{
			const auto wall_map_it = ::hw_3d_xy_wall_map_.find(wall_xy);

			if (wall_map_it == wall_map_end_it)
			{
				continue;
			}

			for (const auto& side : wall_map_it->second.sides_)
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
		auto& ib_buffer = ::hw_3d_wall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 3);
		}

		::hw_index_buffer_update(
			::hw_3d_wall_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto command_buffer = ::hw_3d_command_buffer_;

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
			{
				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = last_texture;
			}

			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_wall_sides_vi_;
			}

			{
				auto& command = *command_buffer->write_draw_quads();
				command.count_ = draw_quad_count;
				command.index_offset_ = draw_index_offset_;

				draw_index_offset_ += ::hw_3d_indices_per_wall_side * draw_quad_count;
			}
		}
	}

	::hw_3d_wall_side_draw_item_count_ = draw_side_index;
}

void hw_3d_pushwalls_render()
{
	if (::hw_3d_pushwall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_pushwall_side_draw_items_;

	if (::vid_configuration_.hw_dbg_draw_all_)
	{
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
	}
	else
	{
		if (::hw_3d_pushwalls_to_render_.empty())
		{
			return;
		}

		const auto pushwall_map_end_it = ::hw_3d_xy_pushwall_map_.cend();

		for (const auto pushwall_xy : ::hw_3d_pushwalls_to_render_)
		{
			const auto pushwall_map_it = ::hw_3d_xy_pushwall_map_.find(pushwall_xy);

			if (pushwall_map_it == pushwall_map_end_it)
			{
				continue;
			}

			for (const auto& side : pushwall_map_it->second.sides_)
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
		auto& ib_buffer = ::hw_3d_pushwall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index_ + 3);
		}

		::hw_index_buffer_update(
			::hw_3d_pushwall_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto command_buffer = ::hw_3d_command_buffer_;

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
			{
				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = last_texture;
			}

			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_pushwall_sides_vi_;
			}

			{
				auto& command = *command_buffer->write_draw_quads();
				command.count_ = draw_quad_count;
				command.index_offset_ = draw_index_offset_;

				draw_index_offset_ += ::hw_3d_indices_per_wall_side * draw_quad_count;
			}
		}
	}

	::hw_3d_pushwall_side_draw_item_count_ = draw_side_index;
}

bool hw_3d_dbg_is_door_vertex_visible(
	const double x,
	const double y)
{
	const auto& wall_direction = glm::dvec2
	{
		::hw_3d_player_position_[0] - x,
		::hw_3d_player_position_[1] - y
	};

	const auto cosine_between_directions = glm::dot(
		wall_direction, ::hw_3d_view_direction_);

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

void hw_3d_doors_render()
{
	if (::hw_3d_door_count_ == 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = ::hw_3d_door_draw_items_;

	if (::vid_configuration_.hw_dbg_draw_all_)
	{
		for (const auto& xy_door_item : ::hw_3d_xy_door_map_)
		{
			const auto& hw_door = xy_door_item.second;
			const auto bs_door_index = hw_door.bs_door_index_;
			const auto door_position = ::doorposition[bs_door_index];

			if (door_position == 0xFFFF)
			{
				// Skip fully open door.
				continue;
			}

			const auto& bs_door = ::doorobjlist[bs_door_index];

			if (!::hw_3d_dbg_is_door_visible(bs_door))
			{
				continue;
			}

			for (const auto& side : xy_door_item.second.sides_)
			{
				auto& draw_item = draw_items[draw_side_index++];

				draw_item.texture_2d_ = side.texture_2d_;
				draw_item.hw_door_side_ = &side;
			}
		}
	}
	else
	{
		if (::hw_3d_doors_to_render_.empty())
		{
			return;
		}

		const auto door_map_end_it = ::hw_3d_xy_door_map_.cend();

		for (const auto door_xy : ::hw_3d_doors_to_render_)
		{
			const auto door_map_it = ::hw_3d_xy_door_map_.find(door_xy);

			if (door_map_it == door_map_end_it)
			{
				continue;
			}

			for (const auto& side : door_map_it->second.sides_)
			{
				auto& draw_item = draw_items[draw_side_index++];

				draw_item.texture_2d_ = side.texture_2d_;
				draw_item.hw_door_side_ = &side;
			}
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
		auto& ib_buffer = ::hw_3d_door_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& hw_door_side = *draw_items[i].hw_door_side_;
			const auto& hw_door = *hw_door_side.hw_door_;

			if (hw_door_side.is_back_face_)
			{
				auto vertex_index = hw_door.vertex_index_ + 4;

				for (int i_quad = 0; i_quad < 2; ++i_quad)
				{
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);

					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);

					vertex_index -= 4;
				}
			}
			else
			{
				auto vertex_index = hw_door.vertex_index_;

				for (int i_quad = 0; i_quad < 2; ++i_quad)
				{
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);

					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);

					vertex_index += 4;
				}
			}
		}

		::hw_index_buffer_update(
			::hw_3d_door_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset = 0;
	auto command_buffer = ::hw_3d_command_buffer_;

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
			{
				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = last_texture;
			}

			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_door_sides_vi_;
			}

			{
				auto& command = *command_buffer->write_draw_quads();
				command.count_ = draw_quad_count;
				command.index_offset_ = draw_index_offset;

				draw_index_offset += 6 * draw_quad_count;
			}
		}
	}

	::hw_3d_door_draw_item_count_ = draw_side_index;
}

bool hw_3d_fog_calculate(
	const int sprite_lighting)
{
	if (::gp_no_shading_)
	{
		return false;
	}

	if (sprite_lighting == NO_SHADING)
	{
		return false;
	}

	const auto shade_index = ::shade_max + sprite_lighting;

	if (shade_index <= 0)
	{
		return false;
	}

	const auto start_wall_height = (::normalshade * shade_index) / (63.0 * ::vga_height_scale);

	if (start_wall_height <= 1.0)
	{
		return false;
	}

	const auto height_num = bstone::FixedPoint{::heightnumerator}.to_double() / ::vga_height_scale;
	const auto start_wall_distance = (32.0 * height_num) / start_wall_height;
	const auto wall_height_step = ::normalshade / (8.0 * ::vga_height_scale);
	const auto fog_delta = static_cast<float>(wall_height_step * ::normalshade_div);

	return true;
}

void hw_3d_actor_cloak_update(
	const Hw3dSprite& sprite)
{
	if (!sprite.flags_.is_visible_)
	{
		return;
	}

	if (sprite.kind_ != Hw3dSpriteKind::actor)
	{
		return;
	}

	const auto& actor = *sprite.bs_object_.actor_;

	const auto is_cloaked = ((actor.flags2 & (FL2_CLOAKED | FL2_DAMAGE_CLOAK)) == FL2_CLOAKED);

	const auto vertex_color = (
		is_cloaked
		?
		HwVertexColor{0x00, 0x00, 0x00, ::hw_3d_cloaked_actor_alpha_u8}
		:
		HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF}
	);


	auto vertex_index = sprite.vertex_index_;

	// Bottom-left.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = vertex_color;
	}

	// Bottom-right.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = vertex_color;
	}

	// Top-right.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = vertex_color;
	}

	// Top-left.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = vertex_color;
	}
}

int hw_3d_actor_anim_rotation_calculate(
	const objtype& bs_actor)
{
	auto dir = bs_actor.dir;

	const auto view_dir_x = static_cast<double>(bs_actor.x - ::player->x);
	const auto view_dir_y = static_cast<double>(-bs_actor.y + ::player->y);

	const auto view_angle_rad = std::atan2(view_dir_y, view_dir_x);
	const auto view_angle = static_cast<int>((180.0 * view_angle_rad) / m_pi());

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(bs_actor.trydir & 127);
	}

	auto target_angle = (view_angle - 180) - ::dirangle[dir];

	target_angle += ANGLES / 16;

	while (target_angle >= ANGLES)
	{
		target_angle -= ANGLES;
	}

	while (target_angle < 0)
	{
		target_angle += ANGLES;
	}

	if ((bs_actor.state->flags & SF_PAINFRAME) != 0)
	{
		// 2 rotation pain frame
		return 4 * (target_angle / (ANGLES / 2)); // seperated by 3 (art layout...)

	}

	return target_angle / (ANGLES / 8);
}

int hw_3d_bs_actor_sprite_get_id(
	const objtype& bs_actor)
{
	assert(bs_actor.state);

	auto result = bs_actor.state->shapenum;

	if ((bs_actor.flags & FL_OFFSET_STATES) != 0)
	{
		result += bs_actor.temp1;
	}

	if (result == -1)
	{
		result = bs_actor.temp1;
	}

	if ((bs_actor.state->flags & SF_ROTATE) != 0)
	{
		result += ::hw_3d_actor_anim_rotation_calculate(bs_actor);
	}

	return result;
}

void hw_actor_update(
	const std::intptr_t bs_actor_index)
{
	auto& hw_actor = ::hw_3d_actors_[bs_actor_index];

	const auto& bs_actor = ::objlist[bs_actor_index];
	const auto new_bs_sprite_id = ::hw_3d_bs_actor_sprite_get_id(bs_actor);

	if (hw_actor.bs_sprite_id_ == 0 || hw_actor.bs_sprite_id_ != new_bs_sprite_id)
	{
		hw_actor.bs_sprite_id_ = new_bs_sprite_id;

		if (hw_actor.bs_sprite_id_ > 0)
		{
			hw_actor.texture_2d_ = ::hw_texture_manager_->sprite_get(hw_actor.bs_sprite_id_);
		}
		else
		{
			hw_actor.texture_2d_ = nullptr;
		}
	}

	if (hw_actor.x_ != bs_actor.x || hw_actor.y_ != bs_actor.y)
	{
		hw_actor.x_ = bs_actor.x;
		hw_actor.y_ = bs_actor.y;
	}
}

void hw_3d_sprite_orient(
	Hw3dSprite& sprite)
{
	sprite.flags_.is_visible_ = false;

	if (!sprite.texture_2d_)
	{
		return;
	}

	if (sprite.kind_ == Hw3dSpriteKind::actor)
	{
		if (sprite.bs_sprite_id_ <= 0)
		{
			return;
		}

		if (sprite.bs_object_.actor_->obclass == nothing)
		{
			return;
		}
	}

	auto sprite_origin = glm::dvec2{};

	if (sprite.kind_ == Hw3dSpriteKind::actor)
	{
		sprite_origin[0] = bstone::FixedPoint{sprite.x_}.to_double();
		sprite_origin[1] = bstone::FixedPoint{sprite.y_}.to_double();
	}
	else
	{
		sprite_origin[0] = static_cast<double>(sprite.tile_x_) + 0.5;
		sprite_origin[1] = static_cast<double>(sprite.tile_y_) + 0.5;
	};

	auto direction = ::hw_3d_player_position_ - sprite_origin;

	const auto cosinus_between_directions = glm::dot(
		::hw_3d_view_direction_,
		direction
	);

	if (::vid_configuration_.hw_dbg_draw_all_ && cosinus_between_directions >= 0.0)
	{
		return;
	}

	sprite.flags_.is_visible_ = true;


	auto bottom_left_vertex = sprite_origin;
	auto bottom_right_vertex = sprite_origin;

	const auto square_distance = glm::dot(direction, direction);

	sprite.square_distance_ = square_distance;

	// Orient the sprite along the player's line of sight (inverted).
	//
	direction[0] = -::hw_3d_view_direction_[0];
	direction[1] = -::hw_3d_view_direction_[1];

	const auto perpendicular_dx = ::hw_3d_tile_half_dimension_d * direction[1];
	const auto perpendicular_dy = ::hw_3d_tile_half_dimension_d * direction[0];

	bottom_left_vertex[0] += -perpendicular_dx;
	bottom_left_vertex[1] += +perpendicular_dy;

	bottom_right_vertex[0] += +perpendicular_dx;
	bottom_right_vertex[1] += -perpendicular_dy;


	auto vertex_index = sprite.vertex_index_;

	// Bottom-left.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			0.0F
		};
	}

	// Bottom-right.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			0.0F
		};
	}

	// Top-right.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			::hw_3d_tile_dimension_f
		};
	}

	// Top-left.
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			::hw_3d_tile_dimension_f
		};
	}

	::hw_3d_actor_cloak_update(sprite);
}

void hw_3d_sprites_render()
{
	// Build draw list.
	//
	auto draw_sprite_index = 0;
	auto& draw_items = ::hw_3d_sprites_draw_list_;

	if (::vid_configuration_.hw_dbg_draw_all_)
	{
		for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
		{
			if (bs_static->shapenum == -1)
			{
				continue;
			}

			const auto bs_static_index = bs_static - ::statobjlist;

			const auto& hw_static = ::hw_3d_statics_[bs_static_index];

			if (!hw_static.flags_.is_visible_)
			{
				continue;
			}

			auto& draw_item = draw_items[draw_sprite_index++];

			draw_item.texture_2d_ = hw_static.texture_2d_;
			draw_item.sprite_ = &hw_static;
		}

		for (auto bs_actor = ::player->next; bs_actor; bs_actor = bs_actor->next)
		{
			const auto bs_actor_index = bs_actor - ::objlist;

			const auto& hw_actor = ::hw_3d_actors_[bs_actor_index];

			if (!hw_actor.flags_.is_visible_)
			{
				continue;
			}

			auto& draw_item = draw_items[draw_sprite_index++];

			draw_item.texture_2d_ = hw_actor.texture_2d_;
			draw_item.sprite_ = &hw_actor;
		}

		if (draw_sprite_index == 0)
		{
			return;
		}
	}
	else
	{
		auto min_vertex_index = ::hw_3d_max_sprites_vertices;
		auto max_vertex_index = 0;

		for (const auto bs_static_index : ::hw_3d_statics_to_render_)
		{
			auto& sprite = ::hw_3d_statics_[bs_static_index];
			::hw_3d_sprite_orient(sprite);

			const auto& hw_static = ::hw_3d_statics_[bs_static_index];

			auto& draw_item = draw_items[draw_sprite_index++];
			draw_item.texture_2d_ = hw_static.texture_2d_;
			draw_item.sprite_ = &hw_static;

			min_vertex_index = std::min(hw_static.vertex_index_, min_vertex_index);
			max_vertex_index = std::max(hw_static.vertex_index_, max_vertex_index);
		}

		for (const auto bs_actor_index : ::hw_3d_actors_to_render_)
		{
			::hw_actor_update(bs_actor_index);

			auto& hw_actor = ::hw_3d_actors_[bs_actor_index];
			::hw_3d_sprite_orient(hw_actor);

			auto& draw_item = draw_items[draw_sprite_index++];
			draw_item.texture_2d_ = hw_actor.texture_2d_;
			draw_item.sprite_ = &hw_actor;

			min_vertex_index = std::min(hw_actor.vertex_index_, min_vertex_index);
			max_vertex_index = std::max(hw_actor.vertex_index_, max_vertex_index);
		}

		if (draw_sprite_index == 0)
		{
			return;
		}

		const auto vertex_count = max_vertex_index - min_vertex_index + ::hw_3d_vertices_per_sprite;

		::hw_vertex_buffer_update(
			::hw_3d_sprites_vb_,
			min_vertex_index,
			vertex_count,
			&::hw_3d_sprites_vbi_[min_vertex_index]
		);
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
		auto& ib_buffer = ::hw_3d_sprites_ibi_;

		for (int i = 0; i < draw_sprite_index; ++i)
		{
			const auto& sprite = *draw_items[i].sprite_;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index_ + 3);
		}

		::hw_index_buffer_update(
			::hw_3d_sprites_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto command_buffer = ::hw_3d_command_buffer_;

	// Disable depth write.
	//
	{
		auto& command = *command_buffer->write_depth_write();
		command.is_enabled_ = false;
	}

	// Enable blending.
	//
	{
		auto& command = *command_buffer->write_blending();
		command.is_enabled_ = true;
	}

	using CurrentTexture = bstone::ModValue<bstone::RendererTexture2dPtr>;

	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto current_texture = CurrentTexture{};

	while (draw_index < draw_sprite_index)
	{
		auto is_first = true;
		auto last_texture = bstone::RendererTexture2dPtr{};
		auto last_lighting = 0;

		draw_quad_count = 0;

		while (draw_index < draw_sprite_index)
		{
			const auto& draw_item = draw_items[draw_index];

			const auto texture = draw_item.texture_2d_;

			auto lighting = 0;

			// TODO Lighting
			if (!::gp_no_shading_)
			{
				const auto& hw_sprite = *draw_item.sprite_;

				switch (hw_sprite.kind_)
				{
					case Hw3dSpriteKind::actor:
						lighting = hw_sprite.bs_object_.actor_->lighting;
						break;

					case Hw3dSpriteKind::stat:
						lighting = hw_sprite.bs_object_.stat_->lighting;
						break;

					default:
						assert(!"Invalid sprite kind.");
						break;
				}

				if (lighting > 0)
				{
					lighting = 0;
				}
			}

			if (is_first)
			{
				is_first = false;

				last_texture = texture;
				last_lighting = lighting;
			}
			else if (last_texture == texture && last_lighting == lighting)
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
			// Set extra lighting.
			//
			if (!::gp_no_shading_)
			{
				::hw_bs_lighting_ = last_lighting;

				if (::hw_bs_lighting_.is_modified())
				{
					::hw_bs_lighting_.set_is_modified(false);

					auto& command = *command_buffer->write_shader_variable_float32();
					command.variable_ = ::hw_shader_variable_extra_lighting_;
					command.value_ = static_cast<float>(::hw_bs_lighting_);
				}
			}

			current_texture = last_texture;

			if (current_texture.is_modified())
			{
				current_texture.set_is_modified(false);

				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = current_texture;
			}

			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_sprites_vi_;
			}

			{
				auto& command = *command_buffer->write_draw_quads();
				command.count_ = draw_quad_count;
				command.index_offset_ = draw_index_offset_;

				draw_index_offset_ += ::hw_3d_indices_per_sprite * draw_quad_count;
			}
		}
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_depth_write();
		command.is_enabled_ = true;
	}

	// Disable blending.
	//
	{
		auto& command = *command_buffer->write_blending();
		command.is_enabled_ = false;
	}

	::hw_3d_sprites_draw_count_ = draw_sprite_index;
}

void hw_widescreen_update()
{
	::hw_dimensions_calculate();
	::SetViewSize();
	::hw_3d_matrix_projection_build();
	::hw_3d_player_weapon_projection_matrix_build();
	::hw_3d_fade_vb_update();
}

void hw_downscale_update()
{
	::hw_dimensions_calculate();
	::SetViewSize();
	::hw_2d_matrix_projection_build();
	::hw_3d_matrix_projection_build();
	::hw_3d_player_weapon_projection_matrix_build();
	::hw_3d_fade_vb_update();


	auto blit_filter = *::vid_configuration_.hw_downscale_blit_filter_;

	switch (blit_filter)
	{
		case bstone::RendererFilterKind::nearest:
		case bstone::RendererFilterKind::linear:
			break;

		default:
			blit_filter = bstone::RendererFilterKind::nearest;
			break;
	}

	const auto downscale_set_result = ::hw_renderer_->downscale_set(
		::downscale_.window_width_,
		::downscale_.window_height_,
		blit_filter
	);

	if (!downscale_set_result)
	{
		::vid_quit("Failed to set downsample parameters. " + ::hw_renderer_->get_error_message());

		return;
	}
}

void vid_apply_hw_is_ui_stretched_configuration()
{
	if (!::vid_configuration_.is_ui_stretched_.is_modified())
	{
		return;
	}

	::vid_configuration_.is_ui_stretched_.set_is_modified(false);
}

void vid_apply_hw_is_widescreen_configuration()
{
	if (!::vid_configuration_.is_widescreen_.is_modified())
	{
		return;
	}

	::vid_configuration_.is_widescreen_.set_is_modified(false);

	::hw_widescreen_update();
}

void vid_apply_hw_2d_texture_filter_configuration()
{
	if (!::vid_configuration_.hw_2d_texture_filter_.is_modified())
	{
		return;
	}

	::vid_configuration_.hw_2d_texture_filter_.set_is_modified(false);

	::hw_2d_sampler_ui_update();
}

void vid_apply_hw_3d_texture_filter_configuration()
{
	if (!::vid_configuration_.hw_3d_texture_image_filter_.is_modified() &&
		!::vid_configuration_.hw_3d_texture_mipmap_filter_.is_modified() &&
		!::vid_configuration_.hw_3d_texture_anisotropy_.is_modified() &&
		!::vid_configuration_.hw_3d_texture_anisotropy_value_.is_modified())
	{
		return;
	}

	::vid_configuration_.hw_3d_texture_image_filter_.set_is_modified(false);
	::vid_configuration_.hw_3d_texture_mipmap_filter_.set_is_modified(false);
	::vid_configuration_.hw_3d_texture_anisotropy_.set_is_modified(false);
	::vid_configuration_.hw_3d_texture_anisotropy_value_.set_is_modified(false);

	::hw_3d_sampler_sprite_update();
	::hw_3d_sampler_wall_update();
	::hw_3d_player_weapon_sampler_update();
}

void vid_apply_hw_downscale_configuration()
{
	if (!::vid_configuration_.is_downscale_.is_modified() &&
		!::vid_configuration_.downscale_width_.is_modified() &&
		!::vid_configuration_.downscale_height_.is_modified() &&
		!::vid_configuration_.hw_downscale_blit_filter_.is_modified())
	{
		return;
	}

	::vid_configuration_.is_downscale_.set_is_modified(false);
	::vid_configuration_.downscale_width_.set_is_modified(false);
	::vid_configuration_.downscale_height_.set_is_modified(false);
	::vid_configuration_.hw_downscale_blit_filter_.set_is_modified(false);

	::hw_downscale_update();
}

void vid_apply_hw_aa_configuration()
{
	if (!::vid_configuration_.hw_aa_kind_.is_modified() &&
		!::vid_configuration_.hw_aa_value_.is_modified())
	{
		return;
	}

	::vid_configuration_.hw_aa_kind_.set_is_modified(false);
	::vid_configuration_.hw_aa_value_.set_is_modified(false);

	const auto aa_result = ::hw_renderer_->aa_set(
		::vid_configuration_.hw_aa_kind_,
		::vid_configuration_.hw_aa_value_);
}

void vid_apply_hw_vsync_configuration()
{
	if (!::vid_configuration_.is_vsync_.is_modified())
	{
		return;
	}

	::vid_configuration_.is_vsync_.set_is_modified(false);

	if (!::hw_device_features_.vsync_is_available_)
	{
		return;
	}

	if (::hw_device_features_.vsync_is_requires_restart_)
	{
		// FIXME
	}

	const auto result = ::hw_renderer_->vsync_set(::vid_configuration_.is_vsync_);

	if (!result)
	{
		::vid_log_hw_renderer_error("Failed to update V-Sync.");
	}
}

void hw_3d_fade_update()
{
	::hw_3d_fade_is_enabled_ = false;

	auto r_f = 1.0F;
	auto g_f = 1.0F;
	auto b_f = 1.0F;
	auto a_f = 1.0F;

	const auto& palette_shift_info = ::palette_shift_get_info();

	if (::hw_3d_fizzle_fx_is_enabled_)
	{
		::hw_3d_fade_is_enabled_ = true;

		auto ratio = ::hw_3d_fizzle_fx_ratio_;

		if (!::hw_3d_fizzle_fx_is_fading_)
		{
			ratio = 1.0F - ratio;
		}

		const auto vga_color = ::vgapal + (3 * ::hw_3d_fizzle_fx_color_index_);
		const auto& color_32 = ::hw_vga_color_to_r8g8b8a8(vga_color[0], vga_color[1], vga_color[2]);

		r_f = static_cast<float>(color_32.r) / 255.0F;
		g_f = static_cast<float>(color_32.g) / 255.0F;
		b_f = static_cast<float>(color_32.b) / 255.0F;
		a_f = ratio;
	}
	else if (palette_shift_info.is_bonus_shifted_ || palette_shift_info.is_damage_shifted_)
	{
		::hw_3d_fade_is_enabled_ = true;

		if (palette_shift_info.is_bonus_shifted_)
		{
			r_f *= palette_shift_info.bonus_r_ / 255.0F;
			g_f *= palette_shift_info.bonus_g_ / 255.0F;
			b_f *= palette_shift_info.bonus_b_ / 255.0F;
			a_f *= palette_shift_info.bonus_a_ / 255.0F;
		}

		if (palette_shift_info.is_damage_shifted_)
		{
			r_f *= palette_shift_info.damage_r_ / 255.0F;
			g_f *= palette_shift_info.damage_g_ / 255.0F;
			b_f *= palette_shift_info.damage_b_ / 255.0F;
			a_f *= palette_shift_info.damage_a_ / 255.0F;
		}
	}

	if (!::hw_3d_fade_is_enabled_)
	{
		return;
	}

	const auto r = static_cast<std::uint8_t>(r_f * 255.0F);
	const auto g = static_cast<std::uint8_t>(g_f * 255.0F);
	const auto b = static_cast<std::uint8_t>(b_f * 255.0F);
	const auto a = static_cast<std::uint8_t>(a_f * 255.0F);

	const auto r8g8b8a8_unorm = bstone::R8g8b8a8{r, g, b, a};

	::hw_texture_manager_->solid_1x1_update(bstone::HwTextureManagerSolid1x1Id::fade_3d, r8g8b8a8_unorm);
}

void hw_screen_3d_refresh()
{
	auto command_buffer = ::hw_3d_command_buffer_;

	command_buffer->enable(false);

	if (!::vid_hw_is_draw_3d_)
	{
		return;
	}

	const auto is_shading = (!::gp_no_shading_);

	const auto& assets_info = AssetsInfo{};

	::hw_3d_fade_update();

	command_buffer->enable(true);

	command_buffer->write_begin();

	// Set viewport.
	//
	{
		auto& command = *command_buffer->write_viewport();
		command.x_ = ::downscale_.screen_viewport_left_width_;
		command.y_ = ::downscale_.window_viewport_bottom_height_ + ::downscale_.screen_viewport_bottom_height_;
		command.width_ = ::downscale_.screen_viewport_width_;
		command.height_ = ::downscale_.screen_viewport_height_;
		command.min_depth_ = 0.0F;
		command.max_depth_ = 1.0F;
	}

	// Enable back-face culling.
	//
	{
		auto& command = *command_buffer->write_culling();
		command.is_enabled_ = true;
	}

	// Enable depth test.
	//
	{
		auto& command = *command_buffer->write_depth_test();
		command.is_enabled_ = true;
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_depth_write();
		command.is_enabled_ = true;
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_model_mat_;
		command.value_ = ::hw_3d_matrix_model_;
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_view_mat_;
		command.value_ = ::hw_3d_matrix_view_;
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_shader_variable_mat4();
		command.variable_ = ::hw_shader_variable_projection_mat_;
		command.value_ = ::hw_3d_matrix_projection_;
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_sampler();
		command.sampler_ = ::hw_3d_wall_s_;
	}

	// Set shading mode.
	//
	{
		::hw_shading_mode_ = is_shading;

		if (::hw_shading_mode_.is_modified())
		{
			::hw_shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_shader_variable_int32();
			command.variable_ = ::hw_shader_variable_shading_mode_;
			command.value_ = ::hw_shading_mode_;
		}
	}

	// Draw solid walls.
	//
	::hw_3d_walls_render();

	// Draw pushwalls.
	//
	::hw_3d_pushwalls_render();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_sampler();
		command.sampler_ = ::hw_3d_sprite_s_;
	}

	// Draw doors.
	//
	::hw_3d_doors_render();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_sampler();
		command.sampler_ = ::hw_3d_wall_s_;
	}

	// Draw flooring.
	//
	{
		auto texture_2d = (!::gp_is_flooring_solid_
			?
			::hw_3d_flooring_textured_t2d_
			:
			::hw_3d_flooring_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_vertex_input();
			command.vertex_input_ = ::hw_3d_flooring_vi_;
		}

		{
			auto& command = *command_buffer->write_draw_quads();
			command.count_ = 1;
			command.index_offset_ = 0;
		}
	}

	// Draw ceiling.
	//
	{
		auto texture_2d = (!::gp_is_ceiling_solid_
			?
			::hw_3d_ceiling_textured_t2d_
			:
			::hw_3d_ceiling_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_vertex_input();
			command.vertex_input_ = ::hw_3d_ceiling_vi_;
		}

		{
			auto& command = *command_buffer->write_draw_quads();
			command.count_ = 1;
			command.index_offset_ = 0;
		}
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_sampler();
		command.sampler_ = ::hw_3d_sprite_s_;
	}

	// Draw statics and actors.
	//
	::hw_3d_sprites_render();

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_culling();
		command.is_enabled_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_depth_test();
		command.is_enabled_ = false;
	}

	// Disable shading mode.
	//
	if (is_shading)
	{
		::hw_shading_mode_ = 0;

		if (::hw_shading_mode_.is_modified())
		{
			::hw_shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_shader_variable_int32();
			command.variable_ = ::hw_shader_variable_shading_mode_;
			command.value_ = ::hw_shading_mode_;
		}
	}

	// Draw player's weapon.
	//
	if (::vid_is_hud)
	{
		const auto player_weapon_sprite_id = ::player_get_weapon_sprite_id();

		if (player_weapon_sprite_id > 0 || ::hw_3d_fade_is_enabled_)
		{
			// Set projection matrix.
			//
			{
				auto& command = *command_buffer->write_shader_variable_mat4();
				command.variable_ = ::hw_shader_variable_projection_mat_;
				command.value_ = ::hw_3d_player_weapon_projection_matrix_;
			}
		}

		if (player_weapon_sprite_id > 0)
		{
			const auto player_weapon_texture = ::hw_texture_manager_->sprite_get(player_weapon_sprite_id);

			if (assets_info.is_ps())
			{
				::hw_3d_player_weapon_model_matrix_update();
			}

			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_shader_variable_mat4();
				command.variable_ = ::hw_shader_variable_model_mat_;
				command.value_ = ::hw_3d_player_weapon_model_matrix_;
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_shader_variable_mat4();
				command.variable_ = ::hw_shader_variable_view_mat_;
				command.value_ = ::hw_3d_player_weapon_view_matrix_;
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = player_weapon_texture;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_sampler();
				command.sampler_ = ::hw_3d_player_weapon_s_;
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_player_weapon_vi_;
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_blending();
				command.is_enabled_ = true;
			}

			// Set blending function.
			//
			{
				auto& command = *command_buffer->write_blending_function();
				command.src_factor_ = bstone::RendererBlendingFactor::src_alpha;
				command.dst_factor_ = bstone::RendererBlendingFactor::one_minus_src_alpha;
			}

			// Draw the weapon.
			//
			{
				auto& command = *command_buffer->write_draw_quads();
				command.index_offset_ = 0;
				command.count_ = 1;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_blending();
				command.is_enabled_ = false;
			}
		}


		// 3D fade (bonus, damage, death, etc).
		//
		if (::hw_3d_fade_is_enabled_)
		{
			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_shader_variable_mat4();
				command.variable_ = ::hw_shader_variable_model_mat_;
				command.value_ = glm::identity<glm::mat4>();
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_shader_variable_mat4();
				command.variable_ = ::hw_shader_variable_view_mat_;
				command.value_ = glm::identity<glm::mat4>();
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_blending();
				command.is_enabled_ = true;
			}

			// Set blending function.
			//
			{
				auto& command = *command_buffer->write_blending_function();
				command.src_factor_ = bstone::RendererBlendingFactor::src_alpha;
				command.dst_factor_ = bstone::RendererBlendingFactor::one_minus_src_alpha;
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_texture();
				command.texture_2d_ = ::hw_3d_fade_t2d_;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_sampler();
				command.sampler_ = ::hw_fade_s_;
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_vertex_input();
				command.vertex_input_ = ::hw_3d_fade_vi_;
			}

			// Draw the quad.
			//
			{
				auto& command = *command_buffer->write_draw_quads();
				command.count_ = 1;
				command.index_offset_ = 0;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_blending();
				command.is_enabled_ = false;
			}
		}
	}

	// Finalize.
	//
	command_buffer->write_end();
}

void hw_screen_refresh()
{
	if (::hw_renderer_ == nullptr)
	{
		return;
	}

#if 0
	if (::hw_device_features_.is_losable_ &&
		::hw_renderer_->device_is_lost())
	{
		if (!::hw_renderer_->device_is_ready_to_reset())
		{
			return;
		}

		if (!::hw_device_reset())
		{
			::vid_quit("Failed to reset a lost device.");
			return;
		}
	}
#else
	static bool test_reset_is_reset_tested_ = false;
	static int test_reset_counter_ = 0;

	if (!test_reset_is_reset_tested_ && test_reset_counter_ == 11)
	{
		test_reset_is_reset_tested_ = true;

		if (!::hw_device_reset())
		{
			::vid_quit("Failed to reset a lost device.");
			return;
		}
	}

	++test_reset_counter_;
#endif

	if (::vid_is_hud && ::player != nullptr)
	{
		::vid_hw_is_draw_3d_ = true;

		::hw_3d_player_update();
		::hw_3d_matrix_view_build();
		::hw_dbg_3d_orient_all_sprites();
	}

	::hw_renderer_->clear_buffers();

	::hw_screen_common_refresh();
	::hw_screen_3d_refresh();
	::hw_screen_2d_refresh();

	::hw_renderer_->execute_commands(::hw_command_manager_.get());
	::hw_renderer_->present();

	::vid_hw_is_draw_3d_ = false;
}

void hw_vsync_check()
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
		::hw_screen_refresh();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	::vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}

void hw_precache_flooring()
{
	if (!::hw_texture_manager_->wall_cache(::FloorTile))
	{
		::vid_quit("Failed to cache a floor #" + std::to_string(::FloorTile) + ".");
	}

	::hw_3d_flooring_textured_t2d_ = ::hw_texture_manager_->wall_get(::FloorTile);

	const auto vga_index = ::BottomColor & 0xFF;
	const auto vga_color = ::vgapal + (3 * vga_index);

	const auto renderer_color = ::hw_vga_color_to_r8g8b8a8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	::hw_texture_manager_->solid_1x1_update(bstone::HwTextureManagerSolid1x1Id::flooring, renderer_color);
}

void hw_precache_ceiling()
{
	if (!::hw_texture_manager_->wall_cache(::CeilingTile))
	{
		::vid_quit("Failed to cache a ceiling #" + std::to_string(::CeilingTile) + ".");
	}

	::hw_3d_ceiling_textured_t2d_ = ::hw_texture_manager_->wall_get(::CeilingTile);

	const auto vga_index = ::TopColor & 0xFF;
	const auto vga_color = ::vgapal + (3 * vga_index);

	const auto renderer_color = ::hw_vga_color_to_r8g8b8a8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	::hw_texture_manager_->solid_1x1_update(bstone::HwTextureManagerSolid1x1Id::ceiling, renderer_color);
}

bool hw_tile_is_activated_pushwall(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_flags;
}

bool hw_tile_is_door(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_flag;
}

bool hw_tile_is_door_track(
	const int tile)
{
	return (tile & ::tilemap_door_flags) == ::tilemap_door_track_flag;
}

bool hw_tile_is_solid_wall(
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

bool hw_tile_is_pushwall(
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

bool hw_tile_is_solid_wall(
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
		::vid_quit("Failed to cache a wall #" + std::to_string(wall_id) + ".");
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

	const auto& bs_door = ::doorobjlist[tile_wall];

	if (bs_door.tilex != x || bs_door.tiley != y)
	{
		::vid_quit("Expected a door at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
	}

	const auto wall_id = ::door_get_track_texture_id(bs_door);

	::hw_precache_wall(wall_id);
}

void hw_precache_walls()
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
		::vid_quit("Failed to cache a door side #" + std::to_string(page_number) + ".");
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

	for (auto bs_door = ::doorobjlist; bs_door != ::lastdoorobj; ++bs_door)
	{
		++::hw_3d_door_count_;

		::hw_precache_door(*bs_door);
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
	auto bs_door_x = x;
	auto bs_door_y = y;

	switch (direction)
	{
	case di_north:
		bs_door_y -= 1;
		break;

	case di_east:
		bs_door_x += 1;
		break;

	case di_south:
		bs_door_y += 1;
		break;

	case di_west:
		bs_door_x -= 1;
		break;

	default:
		::vid_quit("Invalid direction.");
	}

	if (bs_door_x < 0 || bs_door_x >= MAPSIZE || bs_door_y < 0 || bs_door_y >= MAPSIZE)
	{
		return -1;
	}

	const auto bs_door_tile = ::tilemap[bs_door_x][bs_door_y];

	if (!::hw_tile_is_door(bs_door_tile))
	{
		return -1;
	}

	const auto door_index = bs_door_tile & ::tilemap_wall_mask;
	const auto& door = ::doorobjlist[door_index];

	return ::door_get_track_texture_id(door);
}

template<typename TVertex, bool TIsExists = false>
struct HwUpdateVertexXyz
{
	void operator()(
		TVertex& vertex,
		const HwVertexPosition& xyz) const
	{
		static_cast<void>(xyz);
	}
}; // HwUpdateVertexXyz

template<typename TVertex>
struct HwUpdateVertexXyz<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const HwVertexPosition& xyz) const
	{
		vertex.xyz_ = xyz;
	}
}; // HwUpdateVertexXyz

template<typename TVertex>
void hw_update_vertex_xyz(
	TVertex& vertex,
	const HwVertexPosition& xyz)
{
	const auto& traits = HwVertexAttributeTraits<TVertex, HwVertexAttributeLocationId::position>{};

	HwUpdateVertexXyz<TVertex, traits.is_valid>{}(vertex, xyz);
}

template<typename TVertex, bool TIsExists = false>
struct HwUpdateVertexRgba
{
	void operator()(
		TVertex& vertex,
		const HwVertexColor& r8g8b8a8_unorm) const
	{
		static_cast<void>(vertex);
		static_cast<void>(r8g8b8a8_unorm);
	}
}; // HwUpdateVertexRgba

template<typename TVertex>
struct HwUpdateVertexRgba<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const HwVertexColor& r8g8b8a8_unorm) const
	{
		vertex.rgba_ = r8g8b8a8_unorm;
	}
}; // HwUpdateVertexRgba

template<typename TVertex>
void hw_update_vertex_rgba(
	TVertex& vertex,
	const HwVertexColor& r8g8b8a8_unorm)
{
	const auto& traits = HwVertexAttributeTraits<TVertex, HwVertexAttributeLocationId::color>{};

	HwUpdateVertexRgba<TVertex, traits.is_valid>{}(vertex, r8g8b8a8_unorm);
}

template<typename TVertex, bool TIsExists = false>
struct HwUpdateVertexUv
{
	void operator()(
		TVertex& vertex,
		const HwVertexTextureCoordinates& uv) const
	{
		static_cast<void>(uv);
	}
}; // HwUpdateVertexUv

template<typename TVertex>
struct HwUpdateVertexUv<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const HwVertexTextureCoordinates& uv) const
	{
		vertex.uv_ = uv;
	}
}; // HwUpdateVertexUv

template<typename TVertex>
void hw_update_vertex_uv(
	TVertex& vertex,
	const HwVertexTextureCoordinates& uv)
{
	const auto& traits = HwVertexAttributeTraits<TVertex, HwVertexAttributeLocationId::texture_coordinates>{};

	HwUpdateVertexUv<TVertex, traits.is_valid>{}(vertex, uv);
}

template<typename TVertex>
void hw_3d_map_wall_side(
	const controldir_t side_direction,
	Hw3dWall& wall,
	int& vertex_index,
	HwVertexBufferImageT<TVertex>& vb_buffer)
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
		::vid_quit("Invalid direction.");
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

	const auto& r8g8b8a8_unorm = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			0.0F,
		};

		const auto& uv = HwVertexTextureCoordinates{0.0F, 0.0F};

		::hw_update_vertex_xyz(vertex, xyz);
		::hw_update_vertex_rgba(vertex, r8g8b8a8_unorm);
		::hw_update_vertex_uv(vertex, uv);
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			0.0F,
		};

		const auto& uv = HwVertexTextureCoordinates{1.0F, 0.0F};

		::hw_update_vertex_xyz(vertex, xyz);
		::hw_update_vertex_rgba(vertex, r8g8b8a8_unorm);
		::hw_update_vertex_uv(vertex, uv);
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			::hw_3d_map_height_f,
		};

		const auto& uv = HwVertexTextureCoordinates{1.0F, 1.0F};

		::hw_update_vertex_xyz(vertex, xyz);
		::hw_update_vertex_rgba(vertex, r8g8b8a8_unorm);
		::hw_update_vertex_uv(vertex, uv);
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			::hw_3d_map_height_f,
		};

		const auto& uv = HwVertexTextureCoordinates{0.0F, 1.0F};

		::hw_update_vertex_xyz(vertex, xyz);
		::hw_update_vertex_rgba(vertex, r8g8b8a8_unorm);
		::hw_update_vertex_uv(vertex, uv);
	}
}

template<typename TVertex>
void hw_3d_map_xy_to_xwall(
	const Hw3dXyWallKind wall_kind,
	const int x,
	const int y,
	Hw3dXyWallMap& map,
	int& vertex_index,
	HwVertexBufferImageT<TVertex>& vb_buffer)
{
	switch (wall_kind)
	{
	case Hw3dXyWallKind::solid:
	case Hw3dXyWallKind::push:
		break;

	default:
		::vid_quit("Invalid wall kind.");
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

void hw_3d_walls_build()
{
	::hw_3d_walls_uninitialize();

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
	::hw_3d_wall_count_ += ::hw_3d_pushwall_count_;
	::hw_3d_wall_side_count_ += ::hw_3d_pushwall_count_ * ::hw_3d_sides_per_wall;

	const auto index_count = ::hw_3d_wall_side_count_ * ::hw_3d_indices_per_wall_side;

	if (index_count == 0)
	{
		return;
	}

	if (index_count > ::hw_3d_max_wall_sides_indices)
	{
		::vid_quit("Too many wall indices.");
	}

	// Create index an vertex buffers.
	//
	if (!::hw_3d_walls_initialize())
	{
		::vid_quit("Failed to initialize walls.");
	}

	// Build the map (XY to wall).
	//
	const auto vertex_count = ::hw_3d_wall_side_count_ * ::hw_3d_vertices_per_wall_side;

	auto vb_buffer = Hw3dWallsVbi{};
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

	::hw_3d_wall_vertex_count_ = vertex_index;

	// Update vertex buffer.
	//
	::hw_vertex_buffer_update(
		::hw_3d_wall_sides_vb_,
		0,
		vertex_count,
		vb_buffer.data()
	);

	::hw_3d_walls_to_render_.reserve(::hw_3d_wall_count_);
}

void hw_3d_pushwall_side_translate(
	const float translate_x,
	const float translate_y,
	const controldir_t side_direction,
	const Hw3dWall& wall,
	int& vertex_index,
	Hw3dPushwallsVbi& vb_buffer)
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
		::vid_quit("Invalid direction.");
	}

	const auto& vertex_offsets = all_vertex_offsets[side_direction];

	const auto x_f = static_cast<float>(wall.x_) + translate_x;
	const auto y_f = static_cast<float>(wall.y_) + translate_y;

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			0.0F,
		};
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			0.0F,
		};
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			::hw_3d_map_height_f,
		};
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			::hw_3d_map_height_f,
		};
	}
}

void hw_3d_pushwall_translate(
	const Hw3dWall& wall,
	int& vertex_index,
	Hw3dPushwallsVbi& vb_buffer)
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
		::vid_quit("Invalid direction.");
		break;
	}

	::hw_3d_pushwall_side_translate(
		translate_x,
		translate_y,
		di_north,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_pushwall_side_translate(
		translate_x,
		translate_y,
		di_east,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_pushwall_side_translate(
		translate_x,
		translate_y,
		di_south,
		wall,
		vertex_index,
		vb_buffer
	);

	::hw_3d_pushwall_side_translate(
		translate_x,
		translate_y,
		di_west,
		wall,
		vertex_index,
		vb_buffer
	);
}

void hw_3d_pushwall_translate()
{
	const auto xy = ::hw_encode_xy(::pwallx, ::pwally);

	const auto wall_item_it = ::hw_3d_xy_pushwall_map_.find(xy);

	if (wall_item_it == ::hw_3d_xy_pushwall_map_.cend())
	{
		::vid_quit("Pushwall mapping not found.");
	}

	const auto& wall = wall_item_it->second;

	const auto first_vertex_index = wall.sides_.front().vertex_index_;

	auto vertex_index = first_vertex_index;

	::hw_3d_pushwall_translate(wall, vertex_index, ::hw_3d_pushwalls_vbi_);

	const auto vertex_count = vertex_index - first_vertex_index;

	::hw_vertex_buffer_update(
		::hw_3d_pushwall_sides_vb_,
		first_vertex_index,
		vertex_count,
		&::hw_3d_pushwalls_vbi_[first_vertex_index]
	);
}

void hw_3d_pushwall_step(
	const int old_x,
	const int old_y)
{
	const auto old_xy = ::hw_encode_xy(old_x, old_y);

	const auto old_wall_item_it = ::hw_3d_xy_pushwall_map_.find(old_xy);

	if (old_wall_item_it == ::hw_3d_xy_pushwall_map_.cend())
	{
		::vid_quit("Pushwall mapping not found.");
	}

	const auto new_xy = ::hw_encode_xy(::pwallx, ::pwally);

	auto wall = old_wall_item_it->second;
	wall.x_ = ::pwallx;
	wall.y_ = ::pwally;

	static_cast<void>(::hw_3d_xy_pushwall_map_.erase(old_xy));
	::hw_3d_xy_pushwall_map_[new_xy] = wall;

	::hw_3d_pushwall_translate();
}

void hw_3d_pushwalls_build()
{
	::hw_3d_pushwalls_uninitialize();

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

	::hw_3d_pushwall_side_count_ = ::hw_3d_pushwall_count_ * ::hw_3d_sides_per_wall;

	// Check for maximums.
	//
	const auto index_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_indices_per_wall_side;

	if (index_count > ::hw_3d_max_wall_sides_indices)
	{
		::vid_quit("Too many pushwall indices.");
	}

	if (::hw_3d_pushwall_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	if (!::hw_3d_pushwalls_initialize())
	{
		::vid_quit("Failed to initialize pushwalls.");
	}

	// Build the map (XY to pushwall).
	//
	const auto vertex_count = ::hw_3d_pushwall_side_count_ * ::hw_3d_vertices_per_wall_side;

	::hw_3d_pushwalls_vbi_.clear();
	::hw_3d_pushwalls_vbi_.resize(vertex_count);

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
				::hw_3d_pushwalls_vbi_
			);
		}
	}

	// Update vertex buffer.
	//
	::hw_vertex_buffer_update(
		::hw_3d_pushwall_sides_vb_,
		0,
		vertex_count,
		::hw_3d_pushwalls_vbi_.data()
	);


	::hw_3d_pushwalls_to_render_.reserve(::hw_3d_pushwall_count_);
}

template<typename TVertex>
void hw_3d_update_quad_vertices(
	const Hw3dQuadFlags flags,
	const HwVertexPosition& origin,
	const glm::vec2& size,
	int& vertex_index,
	HwVertexBufferImageT<TVertex>& vb_buffer)
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

void hw_3d_door_side_map(
	Hw3dDoorSide& door_side,
	int& vertex_index,
	Hw3dDoorsVbi& vb_buffer)
{
	const auto& hw_door = *door_side.hw_door_;
	const auto bs_door_index = hw_door.bs_door_index_;
	const auto& bs_door = ::doorobjlist[bs_door_index];
	const auto door_offset = (0.5F * static_cast<float>(::doorposition[bs_door_index])) / 65'535.0F;

	auto flags = Hw3dQuadFlags{};
	flags.is_back_face_ = door_side.is_back_face_;
	flags.is_vertical_ = bs_door.vertical;

	const auto origin_axis_index = (flags.is_vertical_ ? 1 : 0);

	const auto size = glm::vec2{0.5F, 1.0F};

	const auto tile_center = HwVertexPosition{bs_door.tilex + 0.5F, bs_door.tiley + 0.5F, 0.0F};

	const auto left_offset = -(0.5F + door_offset);
	const auto right_offset = door_offset;

	for (int i = 0; i < 2; ++i)
	{
		auto origin = tile_center;
		auto offset = 0.0F;

		if (i == 0)
		{
			offset = left_offset;
		}
		else
		{
			offset = right_offset;
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

void hw_3d_xy_to_door_map(
	const doorobj_t& bs_door,
	int& vertex_index,
	Hw3dDoorsVbi& vb_buffer)
{
	const auto xy = ::hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = ::hw_3d_xy_door_map_.find(xy);

	if (map_it != ::hw_3d_xy_door_map_.cend())
	{
		::vid_quit("Door already mapped.");
	}

	::hw_3d_xy_door_map_[xy] = Hw3dDoor{};
	auto& hw_door = ::hw_3d_xy_door_map_[xy];

	const auto bs_door_index = static_cast<int>(&bs_door - ::doorobjlist);
	hw_door.bs_door_index_ = bs_door_index;
	hw_door.vertex_index_ = vertex_index;

	for (int i = 0; i < ::hw_3d_halves_per_side; ++i)
	{
		auto u_0 = 0.0F;
		auto u_1 = 0.0F;

		switch (i)
		{
		case 0:
			u_0 = 0.0F;
			u_1 = 0.5F;
			break;

		case 1:
			u_0 = 0.5F;
			u_1 = 1.0F;
			break;
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv_ = HwVertexTextureCoordinates{u_0, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv_ = HwVertexTextureCoordinates{u_1, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv_ = HwVertexTextureCoordinates{u_1, 1.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv_ = HwVertexTextureCoordinates{u_0, 1.0F};
		}
	}

	auto is_back_face = false;

	for (auto& hw_door_side : hw_door.sides_)
	{
		hw_door_side.hw_door_ = &hw_door;
		hw_door_side.is_back_face_ = is_back_face;

		is_back_face = !is_back_face;
	}

	vertex_index = hw_door.vertex_index_;

	::hw_3d_door_side_map(hw_door.sides_.front(), vertex_index, vb_buffer);

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	::door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

	const auto front_face_texture_2d = ::hw_texture_manager_->wall_get(front_face_page_number);
	const auto back_face_texture_2d = ::hw_texture_manager_->wall_get(back_face_page_number);

	assert(front_face_texture_2d);
	assert(back_face_texture_2d);

	hw_door.sides_[0].texture_2d_ = front_face_texture_2d;
	hw_door.sides_[1].texture_2d_ = back_face_texture_2d;
}

void hw_3d_doors_build()
{
	::hw_3d_door_sides_uninitialize();

	if (::hw_3d_door_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	if (!::hw_3d_door_sides_initialize())
	{
		::vid_quit("Failed to initialize door sides.");
	}

	// Build the map (XY to door).
	//
	const auto vertex_count = ::hw_3d_vertices_per_door * ::hw_3d_door_count_;

	::hw_3d_doors_vbi_.clear();
	::hw_3d_doors_vbi_.resize(vertex_count);

	::hw_3d_xy_door_map_.clear();

	auto vertex_index = 0;

	for (auto bs_door = ::doorobjlist; bs_door != ::lastdoorobj; ++bs_door)
	{
		::hw_3d_xy_to_door_map(
			*bs_door,
			vertex_index,
			::hw_3d_doors_vbi_
		);
	}

	// Update vertex buffer.
	//
	::hw_vertex_buffer_update(
		::hw_3d_door_sides_vb_,
		0,
		vertex_count,
		::hw_3d_doors_vbi_.data()
	);

	::hw_3d_doors_to_render_.reserve(::hw_3d_door_count_);
}

bool hw_sprites_ib_initialize()
{
	const auto index_count = ::hw_3d_max_sprites_indices;

	::hw_3d_sprites_ib_ = ::hw_index_buffer_create(
		bstone::RendererBufferUsageKind::stream_draw,
		2,
		index_count
	);

	if (!::hw_3d_sprites_ib_)
	{
		return false;
	}

	::hw_3d_sprites_ibi_.clear();
	::hw_3d_sprites_ibi_.resize(index_count);

	return true;
}

void hw_3d_sprites_ib_uninitialize()
{
	if (::hw_3d_sprites_ib_)
	{
		::hw_renderer_->index_buffer_destroy(::hw_3d_sprites_ib_);
		::hw_3d_sprites_ib_ = nullptr;
	}

	::hw_3d_sprites_ibi_.clear();
}

bool hw_sprites_vb_initialize()
{
	const auto vertex_count = ::hw_3d_max_sprites_vertices;

	::hw_3d_sprites_vb_ = ::hw_vertex_buffer_create<Hw3dSpriteVertex>(
		bstone::RendererBufferUsageKind::stream_draw,
		vertex_count
	);

	if (!::hw_3d_sprites_vb_)
	{
		return false;
	}

	::hw_3d_sprites_vbi_.resize(vertex_count);

	return true;
}

void hw_3d_sprites_vb_uninitialize()
{
	if (::hw_3d_sprites_vb_)
	{
		::hw_renderer_->vertex_buffer_destroy(::hw_3d_sprites_vb_);
		::hw_3d_sprites_vb_ = nullptr;
	}

	::hw_3d_sprites_vbi_.clear();
}

void hw_3d_sprites_vi_uninitialize()
{
	::hw_vertex_input_destroy(::hw_3d_sprites_vi_);
}

bool hw_3d_sprites_vi_initialize()
{
	if (!::hw_vertex_input_create<Hw3dSpriteVertex>(
		::hw_3d_sprites_ib_,
		::hw_3d_sprites_vb_,
		::hw_3d_sprites_vi_))
	{
		return false;
	}

	return true;
}

bool hw_3d_statics_initialize()
{
	::hw_3d_statics_.resize(MAXSTATS);

	::hw_3d_statics_to_render_.clear();
	::hw_3d_statics_to_render_.reserve(MAXSTATS);

	return true;
}

bool hw_3d_actors_initialize()
{
	::hw_3d_actors_.resize(MAXACTORS);

	::hw_3d_actors_to_render_.clear();
	::hw_3d_actors_to_render_.reserve(MAXACTORS);

	return true;
}

bool hw_3d_sprites_initialize()
{
	::hw_3d_sprites_draw_count_ = 0;
	::hw_3d_sprites_draw_list_.clear();
	::hw_3d_sprites_draw_list_.resize(::hw_3d_max_sprites);

	if (!::hw_sprites_ib_initialize())
	{
		return false;
	}

	if (!::hw_sprites_vb_initialize())
	{
		return false;
	}

	if (!::hw_3d_sprites_vi_initialize())
	{
		return false;
	}

	if (!::hw_3d_statics_initialize())
	{
		return false;
	}

	if (!::hw_3d_actors_initialize())
	{
		return false;
	}

	return true;
}

void hw_3d_statics_uninitialize()
{
	::hw_3d_statics_.clear();
}

void hw_3d_actors_uninitialize()
{
	::hw_3d_actors_.clear();
}

void hw_3d_sprites_uninitialize()
{
	::hw_3d_statics_uninitialize();
	::hw_3d_actors_uninitialize();

	::hw_3d_sprites_draw_count_ = 0;
	::hw_3d_sprites_draw_list_.clear();

	::hw_3d_sprites_vi_uninitialize();
	::hw_3d_sprites_ib_uninitialize();
	::hw_3d_sprites_vb_uninitialize();
}

void hw_dbg_3d_update_actors()
{
	for (auto bs_actor = ::player->next; bs_actor; bs_actor = bs_actor->next)
	{
		const auto bs_actor_index = bs_actor - ::objlist;

		::hw_actor_update(bs_actor_index);
	}
}

void hw_dbg_3d_orient_all_sprites()
{
	if (!::vid_configuration_.hw_dbg_draw_all_)
	{
		return;
	}

	for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
	{
		const auto bs_static_index = bs_static - ::statobjlist;

		auto& sprite = ::hw_3d_statics_[bs_static_index];

		::hw_3d_sprite_orient(sprite);
	}

	::hw_dbg_3d_update_actors();

	for (auto bs_actor = ::player->next; bs_actor; bs_actor = bs_actor->next)
	{
		const auto bs_actor_index = bs_actor - ::objlist;

		auto& sprite = ::hw_3d_actors_[bs_actor_index];

		::hw_3d_sprite_orient(sprite);
	}

	::hw_vertex_buffer_update(
		::hw_3d_sprites_vb_,
		0,
		::hw_3d_max_sprites_vertices,
		::hw_3d_sprites_vbi_.data()
	);
}

void hw_3d_sprite_map(
	const Hw3dSpriteKind sprite_kind,
	int vertex_index,
	Hw3dSprite& sprite)
{
	sprite.kind_ = sprite_kind;
	sprite.vertex_index_ = vertex_index;

	// Bottom-left.
	//
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index++];
		vertex.rgba_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = ::hw_3d_sprites_vbi_[vertex_index];
		vertex.rgba_ = bstone::R8g8b8a8{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}
}

void hw_3d_static_map(
	const statobj_t& bs_static)
{
	const auto bs_static_index = static_cast<int>(&bs_static - ::statobjlist);

	auto vertex_index = ::hw_3d_statics_base_vertex_index;
	vertex_index += (bs_static_index * ::hw_3d_vertices_per_sprite);

	auto& sprite = ::hw_3d_statics_[bs_static_index];
	sprite.tile_x_ = bs_static.tilex;
	sprite.tile_y_ = bs_static.tiley;
	sprite.bs_object_.stat_ = &bs_static;
	sprite.texture_2d_ = ::hw_texture_manager_->sprite_get(bs_static.shapenum);

	::hw_3d_sprite_map(Hw3dSpriteKind::stat, vertex_index, sprite);
}

void hw_3d_static_add(
	const statobj_t& bs_static)
{
	::hw_3d_static_map(bs_static);
}

void hw_3d_sprite_texture_change(
	const Hw3dSpriteKind sprite_kind,
	Hw3dSprite& sprite)
{
	auto sprite_id = 0;

	switch (sprite_kind)
	{
	case Hw3dSpriteKind::stat:
		sprite_id = sprite.bs_object_.stat_->shapenum;;
		break;

	case Hw3dSpriteKind::actor:
		sprite_id = ::hw_3d_bs_actor_sprite_get_id(*sprite.bs_object_.actor_);
		break;

	default:
		::vid_quit("Invalid sprite kind.");

		return;
	}

	sprite.texture_2d_ = ::hw_texture_manager_->sprite_get(sprite_id);
}

void hw_3d_static_texture_change(
	const statobj_t& bs_static)
{
	auto& hw_static = ::hw_get_static(bs_static);

	::hw_3d_sprite_texture_change(Hw3dSpriteKind::stat, hw_static);
}

void hw_3d_static_precache(
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

void hw_3d_statics_precache()
{
	for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		::hw_3d_static_precache(*bs_static);
	}
}

void hw_sprite_cache(
	const int bs_sprite_id)
{
	if (!::hw_texture_manager_->sprite_cache(bs_sprite_id))
	{
		const auto& error_message = "Failed to cache a sprite #" + std::to_string(bs_sprite_id) + ".";

		::vid_quit(error_message);
	}
}

void hw_3d_actor_map(
	const objtype& bs_actor)
{
	const auto bs_actor_index = ::hw_get_actor_index(bs_actor);

	auto vertex_index = ::hw_3d_actors_base_vertex_index;
	vertex_index += (bs_actor_index * ::hw_3d_vertices_per_sprite);

	auto& sprite = ::hw_3d_actors_[bs_actor_index];

	::hw_3d_sprite_map(Hw3dSpriteKind::actor, vertex_index, sprite);

	sprite.x_ = bs_actor.x;
	sprite.y_ = bs_actor.y;
	sprite.tile_x_ = bs_actor.tilex;
	sprite.tile_y_ = bs_actor.tiley;
	sprite.bs_sprite_id_ = ::hw_3d_bs_actor_sprite_get_id(bs_actor);

	sprite.bs_object_.actor_ = &bs_actor;

	if (sprite.bs_sprite_id_ > 0)
	{
		sprite.texture_2d_ = ::hw_texture_manager_->sprite_get(sprite.bs_sprite_id_);
	}
}

void hw_3d_actor_add(
	const objtype& bs_actor)
{
	::hw_3d_actor_map(bs_actor);
}

// Explosion.
void hw_precache_explosion()
{
	::hw_sprite_cache(::SPR_EXPLOSION_1);
	::hw_sprite_cache(::SPR_EXPLOSION_2);
	::hw_sprite_cache(::SPR_EXPLOSION_3);
	::hw_sprite_cache(::SPR_EXPLOSION_4);
	::hw_sprite_cache(::SPR_EXPLOSION_5);
}

// Clip Explosion.
void hw_precache_clip_explosion()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::hw_sprite_cache(::SPR_CLIP_EXP1);
		::hw_sprite_cache(::SPR_CLIP_EXP2);
		::hw_sprite_cache(::SPR_CLIP_EXP3);
		::hw_sprite_cache(::SPR_CLIP_EXP4);
		::hw_sprite_cache(::SPR_CLIP_EXP5);
		::hw_sprite_cache(::SPR_CLIP_EXP6);
		::hw_sprite_cache(::SPR_CLIP_EXP7);
		::hw_sprite_cache(::SPR_CLIP_EXP8);
	}
}

// Grenade explosion.
void hw_precache_grenade_explosion()
{
	::hw_sprite_cache(::SPR_GRENADE_EXPLODE1);
	::hw_sprite_cache(::SPR_GRENADE_EXPLODE2);
	::hw_sprite_cache(::SPR_GRENADE_EXPLODE3);
	::hw_sprite_cache(::SPR_GRENADE_EXPLODE4);
	::hw_sprite_cache(::SPR_GRENADE_EXPLODE5);
}

// Flying grenade.
void hw_precache_flying_grenade()
{
	::hw_sprite_cache(::SPR_GRENADE_FLY1);
	::hw_sprite_cache(::SPR_GRENADE_FLY2);
	::hw_sprite_cache(::SPR_GRENADE_FLY3);
	::hw_sprite_cache(::SPR_GRENADE_FLY4);

	::hw_precache_grenade_explosion();
}

void hw_precache_plasma_detonator_explosion()
{
	::hw_sprite_cache(::SPR_DETONATOR_EXP1);
	::hw_sprite_cache(::SPR_DETONATOR_EXP2);
	::hw_sprite_cache(::SPR_DETONATOR_EXP3);
	::hw_sprite_cache(::SPR_DETONATOR_EXP4);
	::hw_sprite_cache(::SPR_DETONATOR_EXP5);
	::hw_sprite_cache(::SPR_DETONATOR_EXP6);
	::hw_sprite_cache(::SPR_DETONATOR_EXP7);
	::hw_sprite_cache(::SPR_DETONATOR_EXP8);
}

void hw_precache_plasma_detonator()
{
	::hw_sprite_cache(::SPR_DOORBOMB);
	::hw_sprite_cache(::SPR_ALT_DOORBOMB);

	::hw_precache_plasma_detonator_explosion();
}

void hw_precache_anti_plasma_cannon_explosion()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::hw_sprite_cache(::SPR_BFG_EXP1);
		::hw_sprite_cache(::SPR_BFG_EXP2);
		::hw_sprite_cache(::SPR_BFG_EXP3);
		::hw_sprite_cache(::SPR_BFG_EXP4);
		::hw_sprite_cache(::SPR_BFG_EXP5);
		::hw_sprite_cache(::SPR_BFG_EXP6);
		::hw_sprite_cache(::SPR_BFG_EXP7);
		::hw_sprite_cache(::SPR_BFG_EXP8);
	}
}

void hw_precache_anti_plasma_cannon_shot()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::hw_sprite_cache(::SPR_BFG_WEAPON_SHOT1);
		::hw_sprite_cache(::SPR_BFG_WEAPON_SHOT2);
		::hw_sprite_cache(::SPR_BFG_WEAPON_SHOT3);


		::hw_precache_anti_plasma_cannon_explosion();
	}
}

// A rubble.
void hw_precache_rubble()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::hw_sprite_cache(::SPR_RUBBLE);
	}
}

// Toxic waste (green #1).
void hw_precache_toxic_waste_green_1()
{
	::hw_sprite_cache(::SPR_GREEN_OOZE1);
	::hw_sprite_cache(::SPR_GREEN_OOZE2);
	::hw_sprite_cache(::SPR_GREEN_OOZE3);
}

// Toxic waste (green #2).
void hw_precache_toxic_waste_green_2()
{
	::hw_sprite_cache(::SPR_GREEN2_OOZE1);
	::hw_sprite_cache(::SPR_GREEN2_OOZE2);
	::hw_sprite_cache(::SPR_GREEN2_OOZE3);
}

// Toxic waste (black #1).
void hw_precache_toxic_waste_black_1()
{
	::hw_sprite_cache(::SPR_BLACK_OOZE1);
	::hw_sprite_cache(::SPR_BLACK_OOZE2);
	::hw_sprite_cache(::SPR_BLACK_OOZE3);
}

// Toxic waste (black #2).
void hw_precache_toxic_waste_black_2()
{
	::hw_sprite_cache(::SPR_BLACK2_OOZE1);
	::hw_sprite_cache(::SPR_BLACK2_OOZE2);
	::hw_sprite_cache(::SPR_BLACK2_OOZE3);
}

// Coin (1).
void hw_precache_coin_1()
{
	::hw_sprite_cache(::SPR_STAT_77);
}

// Red Access Card.
void hw_precache_red_access_card()
{
	::hw_sprite_cache(::SPR_STAT_32);
}

// Yellow Access Card.
void hw_precache_yellow_access_card()
{
	::hw_sprite_cache(::SPR_STAT_33);
}

// Green Access Card (AOG).
void hw_precache_green_access_card()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog())
	{
		::hw_sprite_cache(::SPR_STAT_34);
	}
}

// Blue Access Card.
void hw_precache_blue_access_card()
{
	::hw_sprite_cache(::SPR_STAT_35);
}

// Golden Access Card (AOG).
void hw_precache_golden_access_card()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog())
	{
		::hw_sprite_cache(::SPR_STAT_36);
	}
}

// Partial Charge Pack.
void hw_precache_partial_charge_pack()
{
	::hw_sprite_cache(::SPR_STAT_26);
}

// Slow Fire Protector.
void hw_precache_slow_fire_protector()
{
	::hw_sprite_cache(::SPR_STAT_24);
}

// Rapid Assault Weapon.
void hw_precache_rapid_assault_weapon()
{
	::hw_sprite_cache(::SPR_STAT_27);
}

// Generic alien spit (#1).
void hw_precache_generic_alien_spit_1()
{
	::hw_sprite_cache(::SPR_SPIT1_1);
	::hw_sprite_cache(::SPR_SPIT1_2);
	::hw_sprite_cache(::SPR_SPIT1_3);

	::hw_sprite_cache(::SPR_SPIT_EXP1_1);
	::hw_sprite_cache(::SPR_SPIT_EXP1_2);
	::hw_sprite_cache(::SPR_SPIT_EXP1_3);
}

// Generic alien spit (#2).
void hw_precache_generic_alien_spit_2()
{
	::hw_sprite_cache(::SPR_SPIT2_1);
	::hw_sprite_cache(::SPR_SPIT2_2);
	::hw_sprite_cache(::SPR_SPIT2_3);

	::hw_sprite_cache(::SPR_SPIT_EXP2_1);
	::hw_sprite_cache(::SPR_SPIT_EXP2_2);
	::hw_sprite_cache(::SPR_SPIT_EXP2_3);
}

// Generic alien spit (#3).
void hw_precache_generic_alien_spit_3()
{
	::hw_sprite_cache(::SPR_SPIT3_1);
	::hw_sprite_cache(::SPR_SPIT3_2);
	::hw_sprite_cache(::SPR_SPIT3_3);

	::hw_sprite_cache(::SPR_SPIT_EXP3_1);
	::hw_sprite_cache(::SPR_SPIT_EXP3_2);
	::hw_sprite_cache(::SPR_SPIT_EXP3_3);
}

// Electrical Shot.
void hw_precache_electrical_shot()
{
	::hw_sprite_cache(::SPR_ELEC_SHOT1);
	::hw_sprite_cache(::SPR_ELEC_SHOT2);
	::hw_sprite_cache(::SPR_ELEC_SHOT_EXP1);
	::hw_sprite_cache(::SPR_ELEC_SHOT_EXP2);
}

// Sector Patrol (AOG) / Sector Guard (PS).
void hw_precache_sector_patrol_or_sector_guard()
{
	::hw_sprite_cache(::SPR_RENT_S_1);
	::hw_sprite_cache(::SPR_RENT_S_2);
	::hw_sprite_cache(::SPR_RENT_S_3);
	::hw_sprite_cache(::SPR_RENT_S_4);
	::hw_sprite_cache(::SPR_RENT_S_5);
	::hw_sprite_cache(::SPR_RENT_S_6);
	::hw_sprite_cache(::SPR_RENT_S_7);
	::hw_sprite_cache(::SPR_RENT_S_8);

	::hw_sprite_cache(::SPR_RENT_W1_1);
	::hw_sprite_cache(::SPR_RENT_W1_2);
	::hw_sprite_cache(::SPR_RENT_W1_3);
	::hw_sprite_cache(::SPR_RENT_W1_4);
	::hw_sprite_cache(::SPR_RENT_W1_5);
	::hw_sprite_cache(::SPR_RENT_W1_6);
	::hw_sprite_cache(::SPR_RENT_W1_7);
	::hw_sprite_cache(::SPR_RENT_W1_8);

	::hw_sprite_cache(::SPR_RENT_W2_1);
	::hw_sprite_cache(::SPR_RENT_W2_2);
	::hw_sprite_cache(::SPR_RENT_W2_3);
	::hw_sprite_cache(::SPR_RENT_W2_4);
	::hw_sprite_cache(::SPR_RENT_W2_5);
	::hw_sprite_cache(::SPR_RENT_W2_6);
	::hw_sprite_cache(::SPR_RENT_W2_7);
	::hw_sprite_cache(::SPR_RENT_W2_8);

	::hw_sprite_cache(::SPR_RENT_W3_1);
	::hw_sprite_cache(::SPR_RENT_W3_2);
	::hw_sprite_cache(::SPR_RENT_W3_3);
	::hw_sprite_cache(::SPR_RENT_W3_4);
	::hw_sprite_cache(::SPR_RENT_W3_5);
	::hw_sprite_cache(::SPR_RENT_W3_6);
	::hw_sprite_cache(::SPR_RENT_W3_7);
	::hw_sprite_cache(::SPR_RENT_W3_8);

	::hw_sprite_cache(::SPR_RENT_W4_1);
	::hw_sprite_cache(::SPR_RENT_W4_2);
	::hw_sprite_cache(::SPR_RENT_W4_3);
	::hw_sprite_cache(::SPR_RENT_W4_4);
	::hw_sprite_cache(::SPR_RENT_W4_5);
	::hw_sprite_cache(::SPR_RENT_W4_6);
	::hw_sprite_cache(::SPR_RENT_W4_7);
	::hw_sprite_cache(::SPR_RENT_W4_8);

	::hw_sprite_cache(::SPR_RENT_DIE_1);
	::hw_sprite_cache(::SPR_RENT_DIE_2);
	::hw_sprite_cache(::SPR_RENT_DIE_3);
	::hw_sprite_cache(::SPR_RENT_DIE_4);
	::hw_sprite_cache(::SPR_RENT_PAIN_1);
	::hw_sprite_cache(::SPR_RENT_DEAD);

	::hw_sprite_cache(::SPR_RENT_SHOOT1);
	::hw_sprite_cache(::SPR_RENT_SHOOT2);
	::hw_sprite_cache(::SPR_RENT_SHOOT3);


	// Goodies.
	//
	::hw_precache_slow_fire_protector();
	::hw_precache_partial_charge_pack();
	::hw_precache_coin_1();
}

// Robot Turret.
void hw_precache_robot_turret()
{
	::hw_sprite_cache(::SPR_TERROT_1);
	::hw_sprite_cache(::SPR_TERROT_2);
	::hw_sprite_cache(::SPR_TERROT_3);
	::hw_sprite_cache(::SPR_TERROT_4);
	::hw_sprite_cache(::SPR_TERROT_5);
	::hw_sprite_cache(::SPR_TERROT_6);
	::hw_sprite_cache(::SPR_TERROT_7);
	::hw_sprite_cache(::SPR_TERROT_8);

	::hw_sprite_cache(::SPR_TERROT_FIRE_1);
	::hw_sprite_cache(::SPR_TERROT_FIRE_2);
	::hw_sprite_cache(::SPR_TERROT_DIE_1);
	::hw_sprite_cache(::SPR_TERROT_DIE_2);
	::hw_sprite_cache(::SPR_TERROT_DIE_3);
	::hw_sprite_cache(::SPR_TERROT_DIE_4);
	::hw_sprite_cache(::SPR_TERROT_DEAD);
}

// Bio-Technician.
void hw_precache_bio_technician()
{
	::hw_sprite_cache(::SPR_OFC_S_1);
	::hw_sprite_cache(::SPR_OFC_S_2);
	::hw_sprite_cache(::SPR_OFC_S_3);
	::hw_sprite_cache(::SPR_OFC_S_4);
	::hw_sprite_cache(::SPR_OFC_S_5);
	::hw_sprite_cache(::SPR_OFC_S_6);
	::hw_sprite_cache(::SPR_OFC_S_7);
	::hw_sprite_cache(::SPR_OFC_S_8);

	::hw_sprite_cache(::SPR_OFC_W1_1);
	::hw_sprite_cache(::SPR_OFC_W1_2);
	::hw_sprite_cache(::SPR_OFC_W1_3);
	::hw_sprite_cache(::SPR_OFC_W1_4);
	::hw_sprite_cache(::SPR_OFC_W1_5);
	::hw_sprite_cache(::SPR_OFC_W1_6);
	::hw_sprite_cache(::SPR_OFC_W1_7);
	::hw_sprite_cache(::SPR_OFC_W1_8);

	::hw_sprite_cache(::SPR_OFC_W2_1);
	::hw_sprite_cache(::SPR_OFC_W2_2);
	::hw_sprite_cache(::SPR_OFC_W2_3);
	::hw_sprite_cache(::SPR_OFC_W2_4);
	::hw_sprite_cache(::SPR_OFC_W2_5);
	::hw_sprite_cache(::SPR_OFC_W2_6);
	::hw_sprite_cache(::SPR_OFC_W2_7);
	::hw_sprite_cache(::SPR_OFC_W2_8);

	::hw_sprite_cache(::SPR_OFC_W3_1);
	::hw_sprite_cache(::SPR_OFC_W3_2);
	::hw_sprite_cache(::SPR_OFC_W3_3);
	::hw_sprite_cache(::SPR_OFC_W3_4);
	::hw_sprite_cache(::SPR_OFC_W3_5);
	::hw_sprite_cache(::SPR_OFC_W3_6);
	::hw_sprite_cache(::SPR_OFC_W3_7);
	::hw_sprite_cache(::SPR_OFC_W3_8);

	::hw_sprite_cache(::SPR_OFC_W4_1);
	::hw_sprite_cache(::SPR_OFC_W4_2);
	::hw_sprite_cache(::SPR_OFC_W4_3);
	::hw_sprite_cache(::SPR_OFC_W4_4);
	::hw_sprite_cache(::SPR_OFC_W4_5);
	::hw_sprite_cache(::SPR_OFC_W4_6);
	::hw_sprite_cache(::SPR_OFC_W4_7);
	::hw_sprite_cache(::SPR_OFC_W4_8);

	::hw_sprite_cache(::SPR_OFC_PAIN_1);
	::hw_sprite_cache(::SPR_OFC_DIE_1);
	::hw_sprite_cache(::SPR_OFC_DIE_2);
	::hw_sprite_cache(::SPR_OFC_DIE_3);
	::hw_sprite_cache(::SPR_OFC_PAIN_2);
	::hw_sprite_cache(::SPR_OFC_DIE_4);
	::hw_sprite_cache(::SPR_OFC_DEAD);

	::hw_sprite_cache(::SPR_OFC_SHOOT1);
	::hw_sprite_cache(::SPR_OFC_SHOOT2);
	::hw_sprite_cache(::SPR_OFC_SHOOT3);


	// Goodies.
	//
	::hw_precache_partial_charge_pack();
	::hw_precache_coin_1();
}

// Pod Alien.
void hw_precache_pod_alien()
{
	::hw_sprite_cache(::SPR_POD_WALK1);
	::hw_sprite_cache(::SPR_POD_WALK2);
	::hw_sprite_cache(::SPR_POD_WALK3);
	::hw_sprite_cache(::SPR_POD_WALK4);
	::hw_sprite_cache(::SPR_POD_ATTACK1);
	::hw_sprite_cache(::SPR_POD_ATTACK2);
	::hw_sprite_cache(::SPR_POD_ATTACK3);
	::hw_sprite_cache(::SPR_POD_OUCH);
	::hw_sprite_cache(::SPR_POD_DIE1);
	::hw_sprite_cache(::SPR_POD_DIE2);
	::hw_sprite_cache(::SPR_POD_DIE3);
	::hw_sprite_cache(::SPR_POD_SPIT1);
	::hw_sprite_cache(::SPR_POD_SPIT2);
	::hw_sprite_cache(::SPR_POD_SPIT3);


	::hw_precache_generic_alien_spit_3();
}

// Pod Alien Egg.
void hw_precache_pod_alien_egg()
{
	::hw_sprite_cache(::SPR_POD_EGG);
	::hw_sprite_cache(::SPR_POD_HATCH1);
	::hw_sprite_cache(::SPR_POD_HATCH2);
	::hw_sprite_cache(::SPR_POD_HATCH3);


	::hw_precache_pod_alien();
}

// High Energy Plasma Alien.
void hw_precache_high_energy_plasma_alien()
{
	::hw_sprite_cache(::SPR_ELEC_APPEAR1);
	::hw_sprite_cache(::SPR_ELEC_APPEAR2);
	::hw_sprite_cache(::SPR_ELEC_APPEAR3);
	::hw_sprite_cache(::SPR_ELEC_WALK1);
	::hw_sprite_cache(::SPR_ELEC_WALK2);
	::hw_sprite_cache(::SPR_ELEC_WALK3);
	::hw_sprite_cache(::SPR_ELEC_WALK4);
	::hw_sprite_cache(::SPR_ELEC_OUCH);
	::hw_sprite_cache(::SPR_ELEC_SHOOT1);
	::hw_sprite_cache(::SPR_ELEC_SHOOT2);
	::hw_sprite_cache(::SPR_ELEC_SHOOT3);
	::hw_sprite_cache(::SPR_ELEC_DIE1);
	::hw_sprite_cache(::SPR_ELEC_DIE2);
	::hw_sprite_cache(::SPR_ELEC_DIE3);


	::hw_precache_electrical_shot();
}

// High Energy Plasma Alien.
void hw_precache_plasma_sphere()
{
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_ROAM1);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_ROAM2);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_ROAM3);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_OUCH);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_DIE1);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_DIE2);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_DIE3);
	::hw_sprite_cache(::SPR_ELECTRO_SPHERE_DIE4);
}

// Star Sentinel (AOG) / Tech Warrior (PS).
void hw_precache_star_sentinel_or_tech_warrior()
{
	::hw_sprite_cache(::SPR_PRO_S_1);
	::hw_sprite_cache(::SPR_PRO_S_2);
	::hw_sprite_cache(::SPR_PRO_S_3);
	::hw_sprite_cache(::SPR_PRO_S_4);
	::hw_sprite_cache(::SPR_PRO_S_5);
	::hw_sprite_cache(::SPR_PRO_S_6);
	::hw_sprite_cache(::SPR_PRO_S_7);
	::hw_sprite_cache(::SPR_PRO_S_8);

	::hw_sprite_cache(::SPR_PRO_W1_1);
	::hw_sprite_cache(::SPR_PRO_W1_2);
	::hw_sprite_cache(::SPR_PRO_W1_3);
	::hw_sprite_cache(::SPR_PRO_W1_4);
	::hw_sprite_cache(::SPR_PRO_W1_5);
	::hw_sprite_cache(::SPR_PRO_W1_6);
	::hw_sprite_cache(::SPR_PRO_W1_7);
	::hw_sprite_cache(::SPR_PRO_W1_8);

	::hw_sprite_cache(::SPR_PRO_W2_1);
	::hw_sprite_cache(::SPR_PRO_W2_2);
	::hw_sprite_cache(::SPR_PRO_W2_3);
	::hw_sprite_cache(::SPR_PRO_W2_4);
	::hw_sprite_cache(::SPR_PRO_W2_5);
	::hw_sprite_cache(::SPR_PRO_W2_6);
	::hw_sprite_cache(::SPR_PRO_W2_7);
	::hw_sprite_cache(::SPR_PRO_W2_8);

	::hw_sprite_cache(::SPR_PRO_W3_1);
	::hw_sprite_cache(::SPR_PRO_W3_2);
	::hw_sprite_cache(::SPR_PRO_W3_3);
	::hw_sprite_cache(::SPR_PRO_W3_4);
	::hw_sprite_cache(::SPR_PRO_W3_5);
	::hw_sprite_cache(::SPR_PRO_W3_6);
	::hw_sprite_cache(::SPR_PRO_W3_7);
	::hw_sprite_cache(::SPR_PRO_W3_8);

	::hw_sprite_cache(::SPR_PRO_W4_1);
	::hw_sprite_cache(::SPR_PRO_W4_2);
	::hw_sprite_cache(::SPR_PRO_W4_3);
	::hw_sprite_cache(::SPR_PRO_W4_4);
	::hw_sprite_cache(::SPR_PRO_W4_5);
	::hw_sprite_cache(::SPR_PRO_W4_6);
	::hw_sprite_cache(::SPR_PRO_W4_7);
	::hw_sprite_cache(::SPR_PRO_W4_8);

	::hw_sprite_cache(::SPR_PRO_PAIN_1);
	::hw_sprite_cache(::SPR_PRO_DIE_1);
	::hw_sprite_cache(::SPR_PRO_DIE_2);
	::hw_sprite_cache(::SPR_PRO_DIE_3);
	::hw_sprite_cache(::SPR_PRO_PAIN_2);
	::hw_sprite_cache(::SPR_PRO_DIE_4);
	::hw_sprite_cache(::SPR_PRO_DEAD);

	::hw_sprite_cache(::SPR_PRO_SHOOT1);
	::hw_sprite_cache(::SPR_PRO_SHOOT2);
	::hw_sprite_cache(::SPR_PRO_SHOOT3);


	// Goodies.
	//
	::hw_precache_rapid_assault_weapon();
	::hw_precache_partial_charge_pack();
	::hw_precache_coin_1();
}

// High-Security Genetic Guard.
void hw_precache_high_security_genetic_guard()
{
	::hw_sprite_cache(::SPR_GENETIC_W1);
	::hw_sprite_cache(::SPR_GENETIC_W2);
	::hw_sprite_cache(::SPR_GENETIC_W3);
	::hw_sprite_cache(::SPR_GENETIC_W4);
	::hw_sprite_cache(::SPR_GENETIC_SWING1);
	::hw_sprite_cache(::SPR_GENETIC_SWING2);
	::hw_sprite_cache(::SPR_GENETIC_SWING3);
	::hw_sprite_cache(::SPR_GENETIC_DEAD);
	::hw_sprite_cache(::SPR_GENETIC_DIE1);
	::hw_sprite_cache(::SPR_GENETIC_DIE2);
	::hw_sprite_cache(::SPR_GENETIC_DIE3);
	::hw_sprite_cache(::SPR_GENETIC_DIE4);
	::hw_sprite_cache(::SPR_GENETIC_OUCH);
	::hw_sprite_cache(::SPR_GENETIC_SHOOT1);
	::hw_sprite_cache(::SPR_GENETIC_SHOOT2);
	::hw_sprite_cache(::SPR_GENETIC_SHOOT3);


	// Goodies.
	//
	::hw_precache_slow_fire_protector();
	::hw_precache_partial_charge_pack();
}

// Experimental Mech-Sentinel.
void hw_precache_experimental_mech_sentinel()
{
	::hw_sprite_cache(::SPR_MUTHUM1_W1);
	::hw_sprite_cache(::SPR_MUTHUM1_W2);
	::hw_sprite_cache(::SPR_MUTHUM1_W3);
	::hw_sprite_cache(::SPR_MUTHUM1_W4);
	::hw_sprite_cache(::SPR_MUTHUM1_SWING1);
	::hw_sprite_cache(::SPR_MUTHUM1_SWING2);
	::hw_sprite_cache(::SPR_MUTHUM1_SWING3);
	::hw_sprite_cache(::SPR_MUTHUM1_DEAD);
	::hw_sprite_cache(::SPR_MUTHUM1_DIE1);
	::hw_sprite_cache(::SPR_MUTHUM1_DIE2);
	::hw_sprite_cache(::SPR_MUTHUM1_DIE3);
	::hw_sprite_cache(::SPR_MUTHUM1_DIE4);
	::hw_sprite_cache(::SPR_MUTHUM1_OUCH);
	::hw_sprite_cache(::SPR_MUTHUM1_SPIT1);
	::hw_sprite_cache(::SPR_MUTHUM1_SPIT2);
	::hw_sprite_cache(::SPR_MUTHUM1_SPIT3);

	::hw_precache_electrical_shot();


	// Goodies.
	//
	::hw_precache_partial_charge_pack();
}

// Experimental Mutant Human.
void hw_precache_experimental_mutant_human()
{
	::hw_sprite_cache(::SPR_MUTHUM2_W1);
	::hw_sprite_cache(::SPR_MUTHUM2_W2);
	::hw_sprite_cache(::SPR_MUTHUM2_W3);
	::hw_sprite_cache(::SPR_MUTHUM2_W4);
	::hw_sprite_cache(::SPR_MUTHUM2_SWING1);
	::hw_sprite_cache(::SPR_MUTHUM2_SWING2);
	::hw_sprite_cache(::SPR_MUTHUM2_SWING3);
	::hw_sprite_cache(::SPR_MUTHUM2_DEAD);
	::hw_sprite_cache(::SPR_MUTHUM2_DIE1);
	::hw_sprite_cache(::SPR_MUTHUM2_DIE2);
	::hw_sprite_cache(::SPR_MUTHUM2_DIE3);
	::hw_sprite_cache(::SPR_MUTHUM2_DIE4);
	::hw_sprite_cache(::SPR_MUTHUM2_OUCH);
	::hw_sprite_cache(::SPR_MUTHUM2_SPIT1);
	::hw_sprite_cache(::SPR_MUTHUM2_SPIT2);
	::hw_sprite_cache(::SPR_MUTHUM2_SPIT3);
}

// Morphing Experimental Mutant Human.
void hw_precache_experimental_mutant_human_morphing()
{
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH1);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH2);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH3);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH4);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH5);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH6);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH7);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH8);
	::hw_sprite_cache(::SPR_MUTHUM2_MORPH9);


	::hw_precache_experimental_mutant_human();
}

// Large Experimental Genetic Alien.
void hw_precache_large_experimental_genetic_alien()
{
	::hw_sprite_cache(::SPR_LCAN_ALIEN_W1);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_W2);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_W3);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_W4);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SWING1);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SWING2);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SWING3);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_DEAD);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_DIE1);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_DIE2);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_DIE3);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_DIE4);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_OUCH);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SPIT1);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SPIT2);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_SPIT3);


	::hw_precache_generic_alien_spit_3();
}

// A canister with large Experimental Genetic Alien.
void hw_precache_canister_with_large_experimental_genetic_alien()
{
	::hw_sprite_cache(::SPR_LCAN_ALIEN_READY);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_B1);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_B2);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_B3);
	::hw_sprite_cache(::SPR_LCAN_ALIEN_EMPTY);

	::hw_precache_large_experimental_genetic_alien();
}

// Small Experimental Genetic Alien.
void hw_precache_experimental_genetic_alien_small()
{
	::hw_sprite_cache(::SPR_SCAN_ALIEN_W1);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_W2);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_W3);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_W4);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SWING1);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SWING2);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SWING3);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_DEAD);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_DIE1);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_DIE2);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_DIE3);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_DIE4);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_OUCH);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SPIT1);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SPIT2);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_SPIT3);


	::hw_precache_generic_alien_spit_1();
}

// A canister with small Experimental Genetic Alien.
void hw_precache_canister_with_small_experimental_genetic_alien()
{
	::hw_sprite_cache(::SPR_SCAN_ALIEN_READY);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_B1);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_B2);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_B3);
	::hw_sprite_cache(::SPR_SCAN_ALIEN_EMPTY);


	::hw_precache_experimental_genetic_alien_small();
}

// Mutated Guard.
void hw_precache_mutated_guard()
{
	::hw_sprite_cache(::SPR_GURNEY_MUT_W1);
	::hw_sprite_cache(::SPR_GURNEY_MUT_W2);
	::hw_sprite_cache(::SPR_GURNEY_MUT_W3);
	::hw_sprite_cache(::SPR_GURNEY_MUT_W4);
	::hw_sprite_cache(::SPR_GURNEY_MUT_SWING1);
	::hw_sprite_cache(::SPR_GURNEY_MUT_SWING2);
	::hw_sprite_cache(::SPR_GURNEY_MUT_SWING3);
	::hw_sprite_cache(::SPR_GURNEY_MUT_DEAD);
	::hw_sprite_cache(::SPR_GURNEY_MUT_DIE1);
	::hw_sprite_cache(::SPR_GURNEY_MUT_DIE2);
	::hw_sprite_cache(::SPR_GURNEY_MUT_DIE3);
	::hw_sprite_cache(::SPR_GURNEY_MUT_DIE4);
	::hw_sprite_cache(::SPR_GURNEY_MUT_OUCH);


	// Goodies.
	//
	::hw_precache_slow_fire_protector();
	::hw_precache_partial_charge_pack();
}

// Mutated Guard (waiting).
void hw_precache_mutated_guard_waiting()
{
	::hw_sprite_cache(::SPR_GURNEY_MUT_READY);
	::hw_sprite_cache(::SPR_GURNEY_MUT_B1);
	::hw_sprite_cache(::SPR_GURNEY_MUT_B2);
	::hw_sprite_cache(::SPR_GURNEY_MUT_B3);
	::hw_sprite_cache(::SPR_GURNEY_MUT_EMPTY);


	::hw_precache_mutated_guard();
}

// Fluid Alien Shot.
void hw_precache_fluid_alien_shot()
{
	::hw_sprite_cache(::SPR_LIQUID_SHOT_FLY_1);
	::hw_sprite_cache(::SPR_LIQUID_SHOT_FLY_2);
	::hw_sprite_cache(::SPR_LIQUID_SHOT_FLY_3);
	::hw_sprite_cache(::SPR_LIQUID_SHOT_BURST_1);
	::hw_sprite_cache(::SPR_LIQUID_SHOT_BURST_2);
	::hw_sprite_cache(::SPR_LIQUID_SHOT_BURST_3);
}

// Fluid Alien.
void hw_precache_fluid_alien()
{
	::hw_sprite_cache(::SPR_LIQUID_M1);
	::hw_sprite_cache(::SPR_LIQUID_M2);
	::hw_sprite_cache(::SPR_LIQUID_M3);
	::hw_sprite_cache(::SPR_LIQUID_R1);
	::hw_sprite_cache(::SPR_LIQUID_R2);
	::hw_sprite_cache(::SPR_LIQUID_R3);
	::hw_sprite_cache(::SPR_LIQUID_R4);
	::hw_sprite_cache(::SPR_LIQUID_S1);
	::hw_sprite_cache(::SPR_LIQUID_S2);
	::hw_sprite_cache(::SPR_LIQUID_S3);
	::hw_sprite_cache(::SPR_LIQUID_OUCH);
	::hw_sprite_cache(::SPR_LIQUID_DIE_1);
	::hw_sprite_cache(::SPR_LIQUID_DIE_2);
	::hw_sprite_cache(::SPR_LIQUID_DIE_3);
	::hw_sprite_cache(::SPR_LIQUID_DIE_4);
	::hw_sprite_cache(::SPR_LIQUID_DEAD);


	::hw_precache_fluid_alien_shot();
}

// Star Trooper (AOG) / Alien Protector (PS).
void hw_precache_star_trooper_or_alien_protector()
{
	::hw_sprite_cache(::SPR_SWAT_S_1);
	::hw_sprite_cache(::SPR_SWAT_S_2);
	::hw_sprite_cache(::SPR_SWAT_S_3);
	::hw_sprite_cache(::SPR_SWAT_S_4);
	::hw_sprite_cache(::SPR_SWAT_S_5);
	::hw_sprite_cache(::SPR_SWAT_S_6);
	::hw_sprite_cache(::SPR_SWAT_S_7);
	::hw_sprite_cache(::SPR_SWAT_S_8);

	::hw_sprite_cache(::SPR_SWAT_W1_1);
	::hw_sprite_cache(::SPR_SWAT_W1_2);
	::hw_sprite_cache(::SPR_SWAT_W1_3);
	::hw_sprite_cache(::SPR_SWAT_W1_4);
	::hw_sprite_cache(::SPR_SWAT_W1_5);
	::hw_sprite_cache(::SPR_SWAT_W1_6);
	::hw_sprite_cache(::SPR_SWAT_W1_7);
	::hw_sprite_cache(::SPR_SWAT_W1_8);

	::hw_sprite_cache(::SPR_SWAT_W2_1);
	::hw_sprite_cache(::SPR_SWAT_W2_2);
	::hw_sprite_cache(::SPR_SWAT_W2_3);
	::hw_sprite_cache(::SPR_SWAT_W2_4);
	::hw_sprite_cache(::SPR_SWAT_W2_5);
	::hw_sprite_cache(::SPR_SWAT_W2_6);
	::hw_sprite_cache(::SPR_SWAT_W2_7);
	::hw_sprite_cache(::SPR_SWAT_W2_8);

	::hw_sprite_cache(::SPR_SWAT_W3_1);
	::hw_sprite_cache(::SPR_SWAT_W3_2);
	::hw_sprite_cache(::SPR_SWAT_W3_3);
	::hw_sprite_cache(::SPR_SWAT_W3_4);
	::hw_sprite_cache(::SPR_SWAT_W3_5);
	::hw_sprite_cache(::SPR_SWAT_W3_6);
	::hw_sprite_cache(::SPR_SWAT_W3_7);
	::hw_sprite_cache(::SPR_SWAT_W3_8);

	::hw_sprite_cache(::SPR_SWAT_W4_1);
	::hw_sprite_cache(::SPR_SWAT_W4_2);
	::hw_sprite_cache(::SPR_SWAT_W4_3);
	::hw_sprite_cache(::SPR_SWAT_W4_4);
	::hw_sprite_cache(::SPR_SWAT_W4_5);
	::hw_sprite_cache(::SPR_SWAT_W4_6);
	::hw_sprite_cache(::SPR_SWAT_W4_7);
	::hw_sprite_cache(::SPR_SWAT_W4_8);

	::hw_sprite_cache(::SPR_SWAT_PAIN_1);
	::hw_sprite_cache(::SPR_SWAT_DIE_1);
	::hw_sprite_cache(::SPR_SWAT_DIE_2);
	::hw_sprite_cache(::SPR_SWAT_DIE_3);
	::hw_sprite_cache(::SPR_SWAT_PAIN_2);
	::hw_sprite_cache(::SPR_SWAT_DIE_4);
	::hw_sprite_cache(::SPR_SWAT_DEAD);

	::hw_sprite_cache(::SPR_SWAT_SHOOT1);
	::hw_sprite_cache(::SPR_SWAT_SHOOT2);
	::hw_sprite_cache(::SPR_SWAT_SHOOT3);

	::hw_sprite_cache(::SPR_SWAT_WOUNDED1);
	::hw_sprite_cache(::SPR_SWAT_WOUNDED2);
	::hw_sprite_cache(::SPR_SWAT_WOUNDED3);
	::hw_sprite_cache(::SPR_SWAT_WOUNDED4);


	// Goodies.
	//
	::hw_precache_rapid_assault_weapon();
	::hw_precache_partial_charge_pack();
	::hw_precache_coin_1();
}

// Dr. Goldfire.
void hw_precache_dr_goldfire()
{
	::hw_sprite_cache(::SPR_GOLD_S_1);
	::hw_sprite_cache(::SPR_GOLD_S_2);
	::hw_sprite_cache(::SPR_GOLD_S_3);
	::hw_sprite_cache(::SPR_GOLD_S_4);
	::hw_sprite_cache(::SPR_GOLD_S_5);
	::hw_sprite_cache(::SPR_GOLD_S_6);
	::hw_sprite_cache(::SPR_GOLD_S_7);
	::hw_sprite_cache(::SPR_GOLD_S_8);

	::hw_sprite_cache(::SPR_GOLD_W1_1);
	::hw_sprite_cache(::SPR_GOLD_W1_2);
	::hw_sprite_cache(::SPR_GOLD_W1_3);
	::hw_sprite_cache(::SPR_GOLD_W1_4);
	::hw_sprite_cache(::SPR_GOLD_W1_5);
	::hw_sprite_cache(::SPR_GOLD_W1_6);
	::hw_sprite_cache(::SPR_GOLD_W1_7);
	::hw_sprite_cache(::SPR_GOLD_W1_8);

	::hw_sprite_cache(::SPR_GOLD_W2_1);
	::hw_sprite_cache(::SPR_GOLD_W2_2);
	::hw_sprite_cache(::SPR_GOLD_W2_3);
	::hw_sprite_cache(::SPR_GOLD_W2_4);
	::hw_sprite_cache(::SPR_GOLD_W2_5);
	::hw_sprite_cache(::SPR_GOLD_W2_6);
	::hw_sprite_cache(::SPR_GOLD_W2_7);
	::hw_sprite_cache(::SPR_GOLD_W2_8);

	::hw_sprite_cache(::SPR_GOLD_W3_1);
	::hw_sprite_cache(::SPR_GOLD_W3_2);
	::hw_sprite_cache(::SPR_GOLD_W3_3);
	::hw_sprite_cache(::SPR_GOLD_W3_4);
	::hw_sprite_cache(::SPR_GOLD_W3_5);
	::hw_sprite_cache(::SPR_GOLD_W3_6);
	::hw_sprite_cache(::SPR_GOLD_W3_7);
	::hw_sprite_cache(::SPR_GOLD_W3_8);

	::hw_sprite_cache(::SPR_GOLD_W4_1);
	::hw_sprite_cache(::SPR_GOLD_W4_2);
	::hw_sprite_cache(::SPR_GOLD_W4_3);
	::hw_sprite_cache(::SPR_GOLD_W4_4);
	::hw_sprite_cache(::SPR_GOLD_W4_5);
	::hw_sprite_cache(::SPR_GOLD_W4_6);
	::hw_sprite_cache(::SPR_GOLD_W4_7);
	::hw_sprite_cache(::SPR_GOLD_W4_8);

	::hw_sprite_cache(::SPR_GOLD_PAIN_1);

	::hw_sprite_cache(::SPR_GOLD_WRIST_1);
	::hw_sprite_cache(::SPR_GOLD_WRIST_2);

	::hw_sprite_cache(::SPR_GOLD_SHOOT1);
	::hw_sprite_cache(::SPR_GOLD_SHOOT2);
	::hw_sprite_cache(::SPR_GOLD_SHOOT3);

	::hw_sprite_cache(::SPR_GOLD_WARP1);
	::hw_sprite_cache(::SPR_GOLD_WARP2);
	::hw_sprite_cache(::SPR_GOLD_WARP3);
	::hw_sprite_cache(::SPR_GOLD_WARP4);
	::hw_sprite_cache(::SPR_GOLD_WARP5);


	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::hw_sprite_cache(::SPR_GOLD_DEATH1);
		::hw_sprite_cache(::SPR_GOLD_DEATH2);
		::hw_sprite_cache(::SPR_GOLD_DEATH3);
		::hw_sprite_cache(::SPR_GOLD_DEATH4);
		::hw_sprite_cache(::SPR_GOLD_DEATH5);
	}


	// Goodies.
	//
	::hw_precache_golden_access_card();
}

// Morphed Dr. Goldfire Shot.
void hw_precache_morphed_dr_goldfire_shot()
{
	::hw_sprite_cache(::SPR_MGOLD_SHOT1);
	::hw_sprite_cache(::SPR_MGOLD_SHOT2);
	::hw_sprite_cache(::SPR_MGOLD_SHOT3);
	::hw_sprite_cache(::SPR_MGOLD_SHOT_EXP1);
	::hw_sprite_cache(::SPR_MGOLD_SHOT_EXP2);
	::hw_sprite_cache(::SPR_MGOLD_SHOT_EXP3);
}

// Morphed Dr. Goldfire.
void hw_precache_morphed_dr_goldfire()
{
	::hw_sprite_cache(::SPR_MGOLD_OUCH);

	::hw_sprite_cache(::SPR_GOLD_MORPH1);
	::hw_sprite_cache(::SPR_GOLD_MORPH2);
	::hw_sprite_cache(::SPR_GOLD_MORPH3);
	::hw_sprite_cache(::SPR_GOLD_MORPH4);
	::hw_sprite_cache(::SPR_GOLD_MORPH5);
	::hw_sprite_cache(::SPR_GOLD_MORPH6);
	::hw_sprite_cache(::SPR_GOLD_MORPH7);
	::hw_sprite_cache(::SPR_GOLD_MORPH8);

	::hw_sprite_cache(::SPR_MGOLD_WALK1);
	::hw_sprite_cache(::SPR_MGOLD_WALK2);
	::hw_sprite_cache(::SPR_MGOLD_WALK3);
	::hw_sprite_cache(::SPR_MGOLD_WALK4);
	::hw_sprite_cache(::SPR_MGOLD_ATTACK1);
	::hw_sprite_cache(::SPR_MGOLD_ATTACK2);
	::hw_sprite_cache(::SPR_MGOLD_ATTACK3);
	::hw_sprite_cache(::SPR_MGOLD_ATTACK4);


	::hw_precache_morphed_dr_goldfire_shot();


	// Goodies.
	//
	::hw_precache_golden_access_card();
}

// Volatile Material Transport.
void hw_precache_volatile_material_transport()
{
	::hw_sprite_cache(::SPR_GSCOUT_W1_1);
	::hw_sprite_cache(::SPR_GSCOUT_W1_2);
	::hw_sprite_cache(::SPR_GSCOUT_W1_3);
	::hw_sprite_cache(::SPR_GSCOUT_W1_4);
	::hw_sprite_cache(::SPR_GSCOUT_W1_5);
	::hw_sprite_cache(::SPR_GSCOUT_W1_6);
	::hw_sprite_cache(::SPR_GSCOUT_W1_7);
	::hw_sprite_cache(::SPR_GSCOUT_W1_8);

	::hw_sprite_cache(::SPR_GSCOUT_W2_1);
	::hw_sprite_cache(::SPR_GSCOUT_W2_2);
	::hw_sprite_cache(::SPR_GSCOUT_W2_3);
	::hw_sprite_cache(::SPR_GSCOUT_W2_4);
	::hw_sprite_cache(::SPR_GSCOUT_W2_5);
	::hw_sprite_cache(::SPR_GSCOUT_W2_6);
	::hw_sprite_cache(::SPR_GSCOUT_W2_7);
	::hw_sprite_cache(::SPR_GSCOUT_W2_8);

	::hw_sprite_cache(::SPR_GSCOUT_W3_1);
	::hw_sprite_cache(::SPR_GSCOUT_W3_2);
	::hw_sprite_cache(::SPR_GSCOUT_W3_3);
	::hw_sprite_cache(::SPR_GSCOUT_W3_4);
	::hw_sprite_cache(::SPR_GSCOUT_W3_5);
	::hw_sprite_cache(::SPR_GSCOUT_W3_6);
	::hw_sprite_cache(::SPR_GSCOUT_W3_7);
	::hw_sprite_cache(::SPR_GSCOUT_W3_8);

	::hw_sprite_cache(::SPR_GSCOUT_W4_1);
	::hw_sprite_cache(::SPR_GSCOUT_W4_2);
	::hw_sprite_cache(::SPR_GSCOUT_W4_3);
	::hw_sprite_cache(::SPR_GSCOUT_W4_4);
	::hw_sprite_cache(::SPR_GSCOUT_W4_5);
	::hw_sprite_cache(::SPR_GSCOUT_W4_6);
	::hw_sprite_cache(::SPR_GSCOUT_W4_7);
	::hw_sprite_cache(::SPR_GSCOUT_W4_8);

	::hw_sprite_cache(::SPR_GSCOUT_DIE1);
	::hw_sprite_cache(::SPR_GSCOUT_DIE2);
	::hw_sprite_cache(::SPR_GSCOUT_DIE3);
	::hw_sprite_cache(::SPR_GSCOUT_DIE4);
	::hw_sprite_cache(::SPR_GSCOUT_DIE5);
	::hw_sprite_cache(::SPR_GSCOUT_DIE6);
	::hw_sprite_cache(::SPR_GSCOUT_DIE7);
	::hw_sprite_cache(::SPR_GSCOUT_DIE8);


	//
	::hw_precache_explosion();
	::hw_precache_toxic_waste_green_1();
}

void hw_precache_perscan_drone()
{
	::hw_sprite_cache(::SPR_FSCOUT_W1_1);
	::hw_sprite_cache(::SPR_FSCOUT_W1_2);
	::hw_sprite_cache(::SPR_FSCOUT_W1_3);
	::hw_sprite_cache(::SPR_FSCOUT_W1_4);
	::hw_sprite_cache(::SPR_FSCOUT_W1_5);
	::hw_sprite_cache(::SPR_FSCOUT_W1_6);
	::hw_sprite_cache(::SPR_FSCOUT_W1_7);
	::hw_sprite_cache(::SPR_FSCOUT_W1_8);

	::hw_sprite_cache(::SPR_FSCOUT_W2_1);
	::hw_sprite_cache(::SPR_FSCOUT_W2_2);
	::hw_sprite_cache(::SPR_FSCOUT_W2_3);
	::hw_sprite_cache(::SPR_FSCOUT_W2_4);
	::hw_sprite_cache(::SPR_FSCOUT_W2_5);
	::hw_sprite_cache(::SPR_FSCOUT_W2_6);
	::hw_sprite_cache(::SPR_FSCOUT_W2_7);
	::hw_sprite_cache(::SPR_FSCOUT_W2_8);

	::hw_sprite_cache(::SPR_FSCOUT_W3_1);
	::hw_sprite_cache(::SPR_FSCOUT_W3_2);
	::hw_sprite_cache(::SPR_FSCOUT_W3_3);
	::hw_sprite_cache(::SPR_FSCOUT_W3_4);
	::hw_sprite_cache(::SPR_FSCOUT_W3_5);
	::hw_sprite_cache(::SPR_FSCOUT_W3_6);
	::hw_sprite_cache(::SPR_FSCOUT_W3_7);
	::hw_sprite_cache(::SPR_FSCOUT_W3_8);

	::hw_sprite_cache(::SPR_FSCOUT_W4_1);
	::hw_sprite_cache(::SPR_FSCOUT_W4_2);
	::hw_sprite_cache(::SPR_FSCOUT_W4_3);
	::hw_sprite_cache(::SPR_FSCOUT_W4_4);
	::hw_sprite_cache(::SPR_FSCOUT_W4_5);
	::hw_sprite_cache(::SPR_FSCOUT_W4_6);
	::hw_sprite_cache(::SPR_FSCOUT_W4_7);
	::hw_sprite_cache(::SPR_FSCOUT_W4_8);

	::hw_sprite_cache(::SPR_FSCOUT_DIE1);
	::hw_sprite_cache(::SPR_FSCOUT_DIE2);
	::hw_sprite_cache(::SPR_FSCOUT_DIE3);
	::hw_sprite_cache(::SPR_FSCOUT_DIE4);
	::hw_sprite_cache(::SPR_FSCOUT_DIE5);
	::hw_sprite_cache(::SPR_FSCOUT_DIE6);
	::hw_sprite_cache(::SPR_FSCOUT_DIE7);
	::hw_sprite_cache(::SPR_FSCOUT_DEAD);

	//
	::hw_precache_explosion();
}

// Security Cube Explosion.
void hw_precache_security_cube_explosion()
{
	::hw_sprite_cache(::SPR_CUBE_EXP1);
	::hw_sprite_cache(::SPR_CUBE_EXP2);
	::hw_sprite_cache(::SPR_CUBE_EXP3);
	::hw_sprite_cache(::SPR_CUBE_EXP4);
	::hw_sprite_cache(::SPR_CUBE_EXP5);
	::hw_sprite_cache(::SPR_CUBE_EXP6);
	::hw_sprite_cache(::SPR_CUBE_EXP7);
	::hw_sprite_cache(::SPR_CUBE_EXP8);
}

// Security Cube.
void hw_precache_security_cube_or_projection_generator()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog())
	{
		::hw_sprite_cache(::SPR_VITAL_STAND);
		::hw_sprite_cache(::SPR_VITAL_DIE_1);
		::hw_sprite_cache(::SPR_VITAL_DIE_2);
		::hw_sprite_cache(::SPR_VITAL_DIE_3);
		::hw_sprite_cache(::SPR_VITAL_DIE_4);
		::hw_sprite_cache(::SPR_VITAL_DIE_5);
		::hw_sprite_cache(::SPR_VITAL_DIE_6);
		::hw_sprite_cache(::SPR_VITAL_DIE_7);
		::hw_sprite_cache(::SPR_VITAL_DIE_8);
		::hw_sprite_cache(::SPR_VITAL_DEAD_1);
		::hw_sprite_cache(::SPR_VITAL_DEAD_2);
		::hw_sprite_cache(::SPR_VITAL_DEAD_3);
		::hw_sprite_cache(::SPR_VITAL_OUCH);


		::hw_precache_explosion();
	}
	else
	{
		::hw_sprite_cache(::SPR_CUBE1);
		::hw_sprite_cache(::SPR_CUBE2);
		::hw_sprite_cache(::SPR_CUBE3);
		::hw_sprite_cache(::SPR_CUBE4);
		::hw_sprite_cache(::SPR_CUBE5);
		::hw_sprite_cache(::SPR_CUBE6);
		::hw_sprite_cache(::SPR_CUBE7);
		::hw_sprite_cache(::SPR_CUBE8);
		::hw_sprite_cache(::SPR_CUBE9);
		::hw_sprite_cache(::SPR_CUBE10);
		::hw_sprite_cache(::SPR_DEAD_CUBE);


		::hw_precache_security_cube_explosion();
	}
}

// Spider Mutant Shot.
void hw_precache_spider_mutant_shot()
{
	::hw_sprite_cache(::SPR_BOSS1_PROJ1);
	::hw_sprite_cache(::SPR_BOSS1_PROJ2);
	::hw_sprite_cache(::SPR_BOSS1_PROJ3);
	::hw_sprite_cache(::SPR_BOSS1_EXP1);
	::hw_sprite_cache(::SPR_BOSS1_EXP2);
	::hw_sprite_cache(::SPR_BOSS1_EXP3);
}

// Spider Mutant.
void hw_precache_spider_mutant()
{
	::hw_sprite_cache(::SPR_BOSS1_W1);
	::hw_sprite_cache(::SPR_BOSS1_W2);
	::hw_sprite_cache(::SPR_BOSS1_W3);
	::hw_sprite_cache(::SPR_BOSS1_W4);
	::hw_sprite_cache(::SPR_BOSS1_SWING1);
	::hw_sprite_cache(::SPR_BOSS1_SWING2);
	::hw_sprite_cache(::SPR_BOSS1_SWING3);
	::hw_sprite_cache(::SPR_BOSS1_DEAD);
	::hw_sprite_cache(::SPR_BOSS1_DIE1);
	::hw_sprite_cache(::SPR_BOSS1_DIE2);
	::hw_sprite_cache(::SPR_BOSS1_DIE3);
	::hw_sprite_cache(::SPR_BOSS1_DIE4);
	::hw_sprite_cache(::SPR_BOSS1_OUCH);
	::hw_sprite_cache(::SPR_BOSS1_PROJ1);
	::hw_sprite_cache(::SPR_BOSS1_PROJ2);
	::hw_sprite_cache(::SPR_BOSS1_PROJ3);
	::hw_sprite_cache(::SPR_BOSS1_EXP1);
	::hw_sprite_cache(::SPR_BOSS1_EXP2);
	::hw_sprite_cache(::SPR_BOSS1_EXP3);


	::hw_precache_spider_mutant_shot();
}

// Morphing Spider Mutant.
void hw_precache_spider_mutant_morphing()
{
	::hw_sprite_cache(::SPR_BOSS1_MORPH1);
	::hw_sprite_cache(::SPR_BOSS1_MORPH2);
	::hw_sprite_cache(::SPR_BOSS1_MORPH3);
	::hw_sprite_cache(::SPR_BOSS1_MORPH4);
	::hw_sprite_cache(::SPR_BOSS1_MORPH5);
	::hw_sprite_cache(::SPR_BOSS1_MORPH6);
	::hw_sprite_cache(::SPR_BOSS1_MORPH7);
	::hw_sprite_cache(::SPR_BOSS1_MORPH8);
	::hw_sprite_cache(::SPR_BOSS1_MORPH9);


	::hw_precache_spider_mutant();
}

// Breather Beast.
void hw_precache_breather_beast()
{
	::hw_sprite_cache(::SPR_BOSS2_W1);
	::hw_sprite_cache(::SPR_BOSS2_W2);
	::hw_sprite_cache(::SPR_BOSS2_W3);
	::hw_sprite_cache(::SPR_BOSS2_W4);
	::hw_sprite_cache(::SPR_BOSS2_SWING1);
	::hw_sprite_cache(::SPR_BOSS2_SWING2);
	::hw_sprite_cache(::SPR_BOSS2_SWING3);
	::hw_sprite_cache(::SPR_BOSS2_DEAD);
	::hw_sprite_cache(::SPR_BOSS2_DIE1);
	::hw_sprite_cache(::SPR_BOSS2_DIE2);
	::hw_sprite_cache(::SPR_BOSS2_DIE3);
	::hw_sprite_cache(::SPR_BOSS2_DIE4);
	::hw_sprite_cache(::SPR_BOSS2_OUCH);
}

// Cyborg Warrior.
void hw_precache_cyborg_warrior()
{
	::hw_sprite_cache(::SPR_BOSS3_W1);
	::hw_sprite_cache(::SPR_BOSS3_W2);
	::hw_sprite_cache(::SPR_BOSS3_W3);
	::hw_sprite_cache(::SPR_BOSS3_W4);
	::hw_sprite_cache(::SPR_BOSS3_SWING1);
	::hw_sprite_cache(::SPR_BOSS3_SWING2);
	::hw_sprite_cache(::SPR_BOSS3_SWING3);
	::hw_sprite_cache(::SPR_BOSS3_DEAD);
	::hw_sprite_cache(::SPR_BOSS3_DIE1);
	::hw_sprite_cache(::SPR_BOSS3_DIE2);
	::hw_sprite_cache(::SPR_BOSS3_DIE3);
	::hw_sprite_cache(::SPR_BOSS3_DIE4);
	::hw_sprite_cache(::SPR_BOSS3_OUCH);
}

// Reptilian Warrior.
void hw_precache_reptilian_warrior()
{
	::hw_sprite_cache(::SPR_BOSS4_W1);
	::hw_sprite_cache(::SPR_BOSS4_W2);
	::hw_sprite_cache(::SPR_BOSS4_W3);
	::hw_sprite_cache(::SPR_BOSS4_W4);
	::hw_sprite_cache(::SPR_BOSS4_SWING1);
	::hw_sprite_cache(::SPR_BOSS4_SWING2);
	::hw_sprite_cache(::SPR_BOSS4_SWING3);
	::hw_sprite_cache(::SPR_BOSS4_DEAD);
	::hw_sprite_cache(::SPR_BOSS4_DIE1);
	::hw_sprite_cache(::SPR_BOSS4_DIE2);
	::hw_sprite_cache(::SPR_BOSS4_DIE3);
	::hw_sprite_cache(::SPR_BOSS4_DIE4);
	::hw_sprite_cache(::SPR_BOSS4_OUCH);
}

// Reptilian Warrior (morphing).
void hw_precache_reptilian_warrior_morphing()
{
	::hw_sprite_cache(::SPR_BOSS4_MORPH1);
	::hw_sprite_cache(::SPR_BOSS4_MORPH2);
	::hw_sprite_cache(::SPR_BOSS4_MORPH3);
	::hw_sprite_cache(::SPR_BOSS4_MORPH4);
	::hw_sprite_cache(::SPR_BOSS4_MORPH5);
	::hw_sprite_cache(::SPR_BOSS4_MORPH6);
	::hw_sprite_cache(::SPR_BOSS4_MORPH7);
	::hw_sprite_cache(::SPR_BOSS4_MORPH8);
	::hw_sprite_cache(::SPR_BOSS4_MORPH9);


	::hw_precache_reptilian_warrior();
}

// Acid Dragon Shot.
void hw_precache_acid_dragon_shot()
{
	::hw_sprite_cache(::SPR_BOSS5_PROJ1);
	::hw_sprite_cache(::SPR_BOSS5_PROJ2);
	::hw_sprite_cache(::SPR_BOSS5_PROJ3);
	::hw_sprite_cache(::SPR_BOSS5_EXP1);
	::hw_sprite_cache(::SPR_BOSS5_EXP2);
	::hw_sprite_cache(::SPR_BOSS5_EXP3);
}

// Acid Dragon.
void hw_precache_acid_dragon()
{
	::hw_sprite_cache(::SPR_BOSS5_W1);
	::hw_sprite_cache(::SPR_BOSS5_W2);
	::hw_sprite_cache(::SPR_BOSS5_W3);
	::hw_sprite_cache(::SPR_BOSS5_W4);
	::hw_sprite_cache(::SPR_BOSS5_SWING1);
	::hw_sprite_cache(::SPR_BOSS5_SWING2);
	::hw_sprite_cache(::SPR_BOSS5_SWING3);
	::hw_sprite_cache(::SPR_BOSS5_DEAD);
	::hw_sprite_cache(::SPR_BOSS5_DIE1);
	::hw_sprite_cache(::SPR_BOSS5_DIE2);
	::hw_sprite_cache(::SPR_BOSS5_DIE3);
	::hw_sprite_cache(::SPR_BOSS5_DIE4);
	::hw_sprite_cache(::SPR_BOSS5_OUCH);


	::hw_precache_acid_dragon_shot();
}

// Bio-Mech Guardian.
void hw_precache_bio_mech_guardian()
{
	::hw_sprite_cache(::SPR_BOSS6_W1);
	::hw_sprite_cache(::SPR_BOSS6_W2);
	::hw_sprite_cache(::SPR_BOSS6_W3);
	::hw_sprite_cache(::SPR_BOSS6_W4);
	::hw_sprite_cache(::SPR_BOSS6_SWING1);
	::hw_sprite_cache(::SPR_BOSS6_SWING2);
	::hw_sprite_cache(::SPR_BOSS6_SWING3);
	::hw_sprite_cache(::SPR_BOSS6_DEAD);
	::hw_sprite_cache(::SPR_BOSS6_DIE1);
	::hw_sprite_cache(::SPR_BOSS6_DIE2);
	::hw_sprite_cache(::SPR_BOSS6_DIE3);
	::hw_sprite_cache(::SPR_BOSS6_DIE4);
	::hw_sprite_cache(::SPR_BOSS6_OUCH);
}

// The Giant Stalker.
void hw_precache_the_giant_stalker()
{
	::hw_sprite_cache(::SPR_BOSS7_W1);
	::hw_sprite_cache(::SPR_BOSS7_W2);
	::hw_sprite_cache(::SPR_BOSS7_W3);
	::hw_sprite_cache(::SPR_BOSS7_W4);
	::hw_sprite_cache(::SPR_BOSS7_SHOOT1);
	::hw_sprite_cache(::SPR_BOSS7_SHOOT2);
	::hw_sprite_cache(::SPR_BOSS7_SHOOT3);
	::hw_sprite_cache(::SPR_BOSS7_DEAD);
	::hw_sprite_cache(::SPR_BOSS7_DIE1);
	::hw_sprite_cache(::SPR_BOSS7_DIE2);
	::hw_sprite_cache(::SPR_BOSS7_DIE3);
	::hw_sprite_cache(::SPR_BOSS7_DIE4);
	::hw_sprite_cache(::SPR_BOSS7_OUCH);
}

// The Spector Demon.
void hw_precache_the_spector_demon()
{
	::hw_sprite_cache(::SPR_BOSS8_W1);
	::hw_sprite_cache(::SPR_BOSS8_W2);
	::hw_sprite_cache(::SPR_BOSS8_W3);
	::hw_sprite_cache(::SPR_BOSS8_W4);
	::hw_sprite_cache(::SPR_BOSS8_SHOOT1);
	::hw_sprite_cache(::SPR_BOSS8_SHOOT2);
	::hw_sprite_cache(::SPR_BOSS8_SHOOT3);
	::hw_sprite_cache(::SPR_BOSS8_DIE1);
	::hw_sprite_cache(::SPR_BOSS8_DIE2);
	::hw_sprite_cache(::SPR_BOSS8_DIE3);
	::hw_sprite_cache(::SPR_BOSS8_DIE4);
	::hw_sprite_cache(::SPR_BOSS8_DEAD);
	::hw_sprite_cache(::SPR_BOSS8_OUCH);


	//
	::hw_precache_morphed_dr_goldfire_shot();
}

// The Armored Stalker.
void hw_precache_the_armored_stalker()
{
	::hw_sprite_cache(::SPR_BOSS9_W1);
	::hw_sprite_cache(::SPR_BOSS9_W2);
	::hw_sprite_cache(::SPR_BOSS9_W3);
	::hw_sprite_cache(::SPR_BOSS9_W4);
	::hw_sprite_cache(::SPR_BOSS9_SHOOT1);
	::hw_sprite_cache(::SPR_BOSS9_SHOOT2);
	::hw_sprite_cache(::SPR_BOSS9_SHOOT3);
	::hw_sprite_cache(::SPR_BOSS9_DIE1);
	::hw_sprite_cache(::SPR_BOSS9_DIE2);
	::hw_sprite_cache(::SPR_BOSS9_DIE3);
	::hw_sprite_cache(::SPR_BOSS9_DIE4);
	::hw_sprite_cache(::SPR_BOSS9_DEAD);
	::hw_sprite_cache(::SPR_BOSS9_OUCH);
}

// The Crawler Beast Shot.
void hw_precache_the_crawler_beast_shot()
{
	::hw_sprite_cache(::SPR_BOSS10_SPIT1);
	::hw_sprite_cache(::SPR_BOSS10_SPIT2);
	::hw_sprite_cache(::SPR_BOSS10_SPIT3);

	::hw_sprite_cache(::SPR_BOSS10_SPIT_EXP1);
	::hw_sprite_cache(::SPR_BOSS10_SPIT_EXP2);
	::hw_sprite_cache(::SPR_BOSS10_SPIT_EXP3);
}

// The Crawler Beast.
void hw_precache_the_crawler_beast()
{
	::hw_sprite_cache(::SPR_BOSS10_W1);
	::hw_sprite_cache(::SPR_BOSS10_W2);
	::hw_sprite_cache(::SPR_BOSS10_W3);
	::hw_sprite_cache(::SPR_BOSS10_W4);
	::hw_sprite_cache(::SPR_BOSS10_SHOOT1);
	::hw_sprite_cache(::SPR_BOSS10_SHOOT2);
	::hw_sprite_cache(::SPR_BOSS10_SHOOT3);
	::hw_sprite_cache(::SPR_BOSS10_DEAD);
	::hw_sprite_cache(::SPR_BOSS10_DIE1);
	::hw_sprite_cache(::SPR_BOSS10_DIE2);
	::hw_sprite_cache(::SPR_BOSS10_DIE3);
	::hw_sprite_cache(::SPR_BOSS10_DIE4);
	::hw_sprite_cache(::SPR_BOSS10_OUCH);


	::hw_precache_the_crawler_beast_shot();
}

// Blake Stone.
void hw_precache_blake_stone()
{
	::hw_sprite_cache(::SPR_BLAKE_W1);
	::hw_sprite_cache(::SPR_BLAKE_W2);
	::hw_sprite_cache(::SPR_BLAKE_W3);
	::hw_sprite_cache(::SPR_BLAKE_W4);
}

void hw_precache_vent_and_dripping_blood()
{
	::hw_sprite_cache(::SPR_BLOOD_DRIP1);
	::hw_sprite_cache(::SPR_BLOOD_DRIP2);
	::hw_sprite_cache(::SPR_BLOOD_DRIP3);
	::hw_sprite_cache(::SPR_BLOOD_DRIP4);
}

void hw_precache_vent_and_dripping_water()
{
	::hw_sprite_cache(::SPR_WATER_DRIP1);
	::hw_sprite_cache(::SPR_WATER_DRIP2);
	::hw_sprite_cache(::SPR_WATER_DRIP3);
	::hw_sprite_cache(::SPR_WATER_DRIP4);
}

void hw_precache_flicker_light()
{
	::hw_sprite_cache(::SPR_DECO_ARC_1);
	::hw_sprite_cache(::SPR_DECO_ARC_2);
	::hw_sprite_cache(::SPR_DECO_ARC_3);
}

void hw_precache_crate_content()
{
	const auto& assets_info = AssetsInfo{};

	::hw_sprite_cache(::SPR_STAT_24); // PISTOL SPR4V
	::hw_sprite_cache(::SPR_STAT_31); // Charge Unit
	::hw_sprite_cache(::SPR_STAT_27); // Auto-Burst Rifle
	::hw_sprite_cache(::SPR_STAT_28); // Particle Charged ION

	::hw_sprite_cache(::SPR_STAT_32); // Red Key SPR5V
	::hw_sprite_cache(::SPR_STAT_33); // Yellow Key
	::hw_sprite_cache(::SPR_STAT_35); // Blue Key

	if (assets_info.is_aog())
	{
		::hw_sprite_cache(::SPR_STAT_34); // Green Key
		::hw_sprite_cache(::SPR_STAT_36); // Gold Key
	}

	::hw_sprite_cache(::SPR_STAT_42); // Chicken Leg
	::hw_sprite_cache(::SPR_STAT_44); // Ham
	::hw_sprite_cache(::SPR_STAT_46); // Grande Launcher

	::hw_sprite_cache(::SPR_STAT_48); // money bag
	::hw_sprite_cache(::SPR_STAT_49); // loot
	::hw_sprite_cache(::SPR_STAT_50); // gold
	::hw_sprite_cache(::SPR_STAT_51); // bonus

	::hw_sprite_cache(::SPR_STAT_57); // Body Parts
}

void hw_precache_crate_1()
{
	::hw_sprite_cache(::SPR_CRATE_1);


	// Goodies.
	//
	::hw_precache_grenade_explosion();
	::hw_precache_crate_content();
}

void hw_precache_crate_2()
{
	::hw_sprite_cache(::SPR_CRATE_2);


	// Goodies.
	//
	::hw_precache_grenade_explosion();
	::hw_precache_crate_content();
}

void hw_precache_crate_3()
{
	::hw_sprite_cache(::SPR_CRATE_3);


	// Goodies.
	//
	::hw_precache_grenade_explosion();
	::hw_precache_crate_content();
}

void hw_precache_electrical_post_barrier()
{
	::hw_sprite_cache(::SPR_ELEC_POST1);
	::hw_sprite_cache(::SPR_ELEC_POST2);
	::hw_sprite_cache(::SPR_ELEC_POST3);
	::hw_sprite_cache(::SPR_ELEC_POST4);
}

void hw_precache_electrical_arc_barrier()
{
	::hw_sprite_cache(::SPR_ELEC_ARC1);
	::hw_sprite_cache(::SPR_ELEC_ARC2);
	::hw_sprite_cache(::SPR_ELEC_ARC3);
	::hw_sprite_cache(::SPR_ELEC_ARC4);
}

void hw_precache_vertical_post_barrier()
{
	::hw_sprite_cache(::SPR_VPOST1);
	::hw_sprite_cache(::SPR_VPOST2);
	::hw_sprite_cache(::SPR_VPOST3);
	::hw_sprite_cache(::SPR_VPOST4);
	::hw_sprite_cache(::SPR_VPOST5);
	::hw_sprite_cache(::SPR_VPOST6);
	::hw_sprite_cache(::SPR_VPOST7);
	::hw_sprite_cache(::SPR_VPOST8);
}

void hw_precache_vertical_spike_barrier()
{
	::hw_sprite_cache(::SPR_VSPIKE1);
	::hw_sprite_cache(::SPR_VSPIKE2);
	::hw_sprite_cache(::SPR_VSPIKE3);
	::hw_sprite_cache(::SPR_VSPIKE4);
	::hw_sprite_cache(::SPR_VSPIKE5);
	::hw_sprite_cache(::SPR_VSPIKE6);
	::hw_sprite_cache(::SPR_VSPIKE7);
	::hw_sprite_cache(::SPR_VSPIKE8);
}

void hw_precache_security_light()
{
	::hw_sprite_cache(::SPR_SECURITY_NORMAL);
	::hw_sprite_cache(::SPR_SECURITY_ALERT);
}

void hw_precache_grate_and_steam()
{
	::hw_sprite_cache(::SPR_STEAM_1);
	::hw_sprite_cache(::SPR_STEAM_2);
	::hw_sprite_cache(::SPR_STEAM_3);
	::hw_sprite_cache(::SPR_STEAM_4);
}

void hw_precache_pipe_and_steam()
{
	::hw_sprite_cache(::SPR_PIPE_STEAM_1);
	::hw_sprite_cache(::SPR_PIPE_STEAM_2);
	::hw_sprite_cache(::SPR_PIPE_STEAM_3);
	::hw_sprite_cache(::SPR_PIPE_STEAM_4);
}

void hw_precache_special_stuff()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog())
	{
		if (::gamestate.mapon == 9)
		{
			::hw_precache_blake_stone();
			::hw_precache_dr_goldfire();
		}
	}

	if (::GoldsternInfo.SpawnCnt > 0)
	{
		::hw_precache_dr_goldfire();
	}
}

void hw_precache_access_cards()
{
	::hw_precache_red_access_card();
	::hw_precache_yellow_access_card();
	::hw_precache_green_access_card();
	::hw_precache_blue_access_card();
	::hw_precache_golden_access_card();
}

void hw_precache_player_weapon_auto_charge_pistol()
{
	::hw_sprite_cache(::SPR_KNIFEREADY);
	::hw_sprite_cache(::SPR_KNIFEATK1);
	::hw_sprite_cache(::SPR_KNIFEATK2);
	::hw_sprite_cache(::SPR_KNIFEATK3);
	::hw_sprite_cache(::SPR_KNIFEATK4);
}

void hw_precache_player_weapon_slow_fire_protector()
{
	::hw_sprite_cache(::SPR_PISTOLREADY);
	::hw_sprite_cache(::SPR_PISTOLATK1);
	::hw_sprite_cache(::SPR_PISTOLATK2);
	::hw_sprite_cache(::SPR_PISTOLATK3);
	::hw_sprite_cache(::SPR_PISTOLATK4);
}

void hw_precache_player_weapon_rapid_assault_weapon()
{
	::hw_sprite_cache(::SPR_MACHINEGUNREADY);
	::hw_sprite_cache(::SPR_MACHINEGUNATK1);
	::hw_sprite_cache(::SPR_MACHINEGUNATK2);
	::hw_sprite_cache(::SPR_MACHINEGUNATK3);
	::hw_sprite_cache(::SPR_MACHINEGUNATK4);
}

void hw_precache_player_weapon_dual_neutron_disruptor()
{
	::hw_sprite_cache(::SPR_CHAINREADY);
	::hw_sprite_cache(::SPR_CHAINATK1);
	::hw_sprite_cache(::SPR_CHAINATK2);
	::hw_sprite_cache(::SPR_CHAINATK3);
	::hw_sprite_cache(::SPR_CHAINATK4);
}

void hw_precache_player_weapon_plasma_discharge_unit()
{
	::hw_sprite_cache(::SPR_GRENADEREADY);
	::hw_sprite_cache(::SPR_GRENADEATK1);
	::hw_sprite_cache(::SPR_GRENADEATK2);
	::hw_sprite_cache(::SPR_GRENADEATK3);
	::hw_sprite_cache(::SPR_GRENADEATK4);

	::hw_precache_flying_grenade();
	::hw_precache_grenade_explosion();
}

void hw_precache_player_weapon_anti_plasma_cannon()
{
	const auto& assets_info = AssetsInfo{};

	if (!assets_info.is_ps())
	{
		return;
	}

	::hw_sprite_cache(::SPR_BFG_WEAPON1);
	::hw_sprite_cache(::SPR_BFG_WEAPON1);
	::hw_sprite_cache(::SPR_BFG_WEAPON2);
	::hw_sprite_cache(::SPR_BFG_WEAPON3);
	::hw_sprite_cache(::SPR_BFG_WEAPON4);
	::hw_sprite_cache(::SPR_BFG_WEAPON5);

	::hw_precache_anti_plasma_cannon_shot();
	::hw_precache_anti_plasma_cannon_explosion();

	::hw_precache_rubble();

	::hw_precache_explosion();
	::hw_precache_clip_explosion();
}

void hw_precache_player_weapons()
{
	::hw_precache_player_weapon_auto_charge_pistol();
	::hw_precache_player_weapon_slow_fire_protector();
	::hw_precache_player_weapon_rapid_assault_weapon();
	::hw_precache_player_weapon_dual_neutron_disruptor();
	::hw_precache_player_weapon_plasma_discharge_unit();
	::hw_precache_player_weapon_anti_plasma_cannon();
}

void hw_precache_actors()
{
	for (auto bs_actor = ::player->next; bs_actor; bs_actor = bs_actor->next)
	{
		switch (bs_actor->obclass)
		{
		case nothing:
			break;

		case rentacopobj:
			::hw_precache_sector_patrol_or_sector_guard();
			break;

		case hang_terrotobj:
			::hw_precache_robot_turret();
			break;

		case gen_scientistobj:
			::hw_precache_bio_technician();
			break;

		case podobj:
			::hw_precache_pod_alien();
			break;

		case electroobj:
			::hw_precache_high_energy_plasma_alien();
			break;

		case electrosphereobj:
			::hw_precache_plasma_sphere();
			break;

		case proguardobj:
			::hw_precache_star_sentinel_or_tech_warrior();
			break;

		case genetic_guardobj:
			::hw_precache_high_security_genetic_guard();
			break;

		case mutant_human1obj:
			::hw_precache_experimental_mech_sentinel();
			break;

		case mutant_human2obj:
			::hw_precache_experimental_mutant_human();
			break;

		case lcan_wait_alienobj:
			::hw_precache_canister_with_large_experimental_genetic_alien();
			break;

		case lcan_alienobj:
			::hw_precache_large_experimental_genetic_alien();
			break;

		case scan_wait_alienobj:
			::hw_precache_canister_with_small_experimental_genetic_alien();
			break;

		case scan_alienobj:
			::hw_precache_experimental_genetic_alien_small();
			break;

		case gurney_waitobj:
			::hw_precache_mutated_guard_waiting();
			break;

		case gurneyobj:
			::hw_precache_mutated_guard();
			break;

		case liquidobj:
			::hw_precache_fluid_alien();
			break;

		case swatobj:
			::hw_precache_star_trooper_or_alien_protector();
			break;

		case goldsternobj:
			::hw_precache_dr_goldfire();
			break;

		case gold_morphobj:
			::hw_precache_morphed_dr_goldfire();
			break;

		case volatiletransportobj:
			::hw_precache_volatile_material_transport();
			break;

		case floatingbombobj:
			::hw_precache_perscan_drone();
			break;

		case rotating_cubeobj:
			::hw_precache_security_cube_or_projection_generator();
			break;

		case spider_mutantobj:
			::hw_precache_spider_mutant();
			break;

		case breather_beastobj:
			::hw_precache_breather_beast();
			break;

		case cyborg_warriorobj:
			::hw_precache_cyborg_warrior();
			break;

		case reptilian_warriorobj:
			::hw_precache_reptilian_warrior();
			break;

		case acid_dragonobj:
			::hw_precache_acid_dragon();
			break;

		case mech_guardianobj:
			::hw_precache_bio_mech_guardian();
			break;

		case final_boss1obj:
			::hw_precache_the_giant_stalker();
			break;

		case final_boss2obj:
			::hw_precache_the_spector_demon();
			break;

		case final_boss3obj:
			::hw_precache_the_armored_stalker();
			break;

		case final_boss4obj:
			::hw_precache_the_crawler_beast();
			break;

		case blakeobj:
			::hw_precache_blake_stone();
			break;

		case crate1obj:
			::hw_precache_crate_1();
			break;

		case crate2obj:
			::hw_precache_crate_2();
			break;

		case crate3obj:
			::hw_precache_crate_3();
			break;

		case green_oozeobj:
			::hw_precache_toxic_waste_green_1();
			break;

		case black_oozeobj:
			::hw_precache_toxic_waste_black_1();
			break;

		case green2_oozeobj:
			::hw_precache_toxic_waste_green_2();
			break;

		case black2_oozeobj:
			::hw_precache_toxic_waste_black_2();
			break;

		case podeggobj:
			::hw_precache_pod_alien_egg();
			break;

		case morphing_spider_mutantobj:
			::hw_precache_spider_mutant_morphing();
			break;

		case morphing_reptilian_warriorobj:
			::hw_precache_reptilian_warrior_morphing();
			break;

		case morphing_mutanthuman2obj:
			::hw_precache_experimental_mutant_human_morphing();
			break;

		case electroshotobj:
			::hw_precache_electrical_shot();
			break;

		case post_barrierobj:
			::hw_precache_electrical_post_barrier();
			break;

		case arc_barrierobj:
			::hw_precache_electrical_arc_barrier();
			break;

		case vpost_barrierobj:
			::hw_precache_vertical_post_barrier();
			break;

		case vspike_barrierobj:
			::hw_precache_vertical_spike_barrier();
			break;

		case goldmorphshotobj:
			::hw_precache_morphed_dr_goldfire_shot();
			break;

		case security_lightobj:
			::hw_precache_security_light();
			break;

		case explosionobj:
			::hw_precache_explosion();
			::hw_precache_clip_explosion();
			break;

		case steamgrateobj:
			::hw_precache_grate_and_steam();
			break;

		case steampipeobj:
			::hw_precache_pipe_and_steam();
			break;

		case liquidshotobj:
			::hw_precache_fluid_alien_shot();
			break;

		case lcanshotobj:
			::hw_precache_generic_alien_spit_3();
			break;

		case podshotobj:
			::hw_precache_generic_alien_spit_3();
			break;

		case scanshotobj:
			::hw_precache_generic_alien_spit_1();
			break;

		case dogshotobj:
			::hw_precache_generic_alien_spit_1();
			break;

		case mut_hum1shotobj:
			::hw_precache_electrical_shot();
			break;

		case ventdripobj:
			::hw_precache_vent_and_dripping_blood();
			::hw_precache_vent_and_dripping_water();
			break;

		case playerspshotobj:
			break;

		case flickerlightobj:
			::hw_precache_flicker_light();
			break;

		case plasma_detonatorobj:
		case plasma_detonator_reserveobj:
			::hw_precache_plasma_detonator();
			break;

		case grenadeobj:
			::hw_precache_flying_grenade();
			break;

		case bfg_shotobj:
			::hw_precache_anti_plasma_cannon_shot();
			break;

		case bfg_explosionobj:
			::hw_precache_anti_plasma_cannon_explosion();
			break;

		case pd_explosionobj:
			::hw_precache_plasma_detonator_explosion();
			break;

		case spider_mutantshotobj:
			::hw_precache_spider_mutant_shot();
			break;

		case breather_beastshotobj:
			break;

		case cyborg_warriorshotobj:
			break;

		case reptilian_warriorshotobj:
			break;

		case acid_dragonshotobj:
			::hw_precache_acid_dragon_shot();
			break;

		case mech_guardianshotobj:
			break;

		case final_boss2shotobj:
			::hw_precache_morphed_dr_goldfire_shot();
			break;

		case final_boss4shotobj:
			::hw_precache_the_crawler_beast_shot();
			break;

		case doorexplodeobj:
			::hw_precache_explosion();
			::hw_precache_rubble();
			break;

		case gr_explosionobj:
			::hw_precache_explosion();
			::hw_precache_grenade_explosion();
			break;

		case gold_morphingobj:
			::hw_precache_morphed_dr_goldfire();
			break;

		default:
			break;
		}
	}

	::hw_precache_special_stuff();
	::hw_precache_access_cards();
	::hw_precache_player_weapons();
}

void hw_precache_sprites()
{
	::hw_3d_statics_precache();
	::hw_precache_actors();
}

void hw_3d_statics_build()
{
	::hw_3d_statics_uninitialize();

	if (!::hw_3d_statics_initialize())
	{
		::vid_quit("Failed to initialize statics.");
	}

	if (::statobjlist == nullptr || ::laststatobj == nullptr)
	{
		return;
	}

	for (auto bs_static = ::statobjlist; bs_static != ::laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		::hw_3d_static_map(*bs_static);
	}
}

void hw_3d_actors_build()
{
	::hw_3d_actors_uninitialize();

	if (!::hw_3d_actors_initialize())
	{
		::vid_quit("Failed to initialize actors.");
	}

	if (::player == nullptr)
	{
		return;
	}

	for (auto bs_actor = ::player->next; bs_actor != nullptr; bs_actor = bs_actor->next)
	{
		::hw_3d_actor_map(*bs_actor);
	}
}

void hw_3d_sprites_build()
{
	::hw_3d_sprites_uninitialize();
	::hw_3d_sprites_initialize();

	::hw_3d_statics_build();
	::hw_3d_actors_build();
}

void hw_precache_resources()
{
	::vid_log();
	::vid_log("Precaching resources.");

	::hw_texture_manager_->cache_begin();

	::hw_precache_flooring();
	::hw_precache_ceiling();
	::hw_precache_walls();
	::hw_precache_pushwalls();
	::hw_precache_doors();
	::hw_precache_sprites();

	::hw_texture_manager_->cache_end();

	::hw_texture_manager_->cache_purge();
}

void hw_device_reset_resources_destroy()
{
	::hw_2d_uninitialize();
	::hw_3d_flooring_uninitialize();
	::hw_3d_ceiling_uninitialize();
	::hw_3d_fade_uninitialize();
	::hw_3d_player_weapon_uninitialize();
	::hw_samplers_uninitialize();

	::hw_3d_walls_uninitialize();
	::hw_3d_pushwalls_uninitialize();
	::hw_3d_door_sides_uninitialize();
	::hw_3d_statics_uninitialize();
	::hw_3d_actors_uninitialize();

	::hw_texture_manager_destroy();
}

bool hw_device_reset_resources_create()
{
	if (!::hw_texture_manager_create())
	{
		return false;
	}

	if (!::hw_2d_initialize())
	{
		return false;
	}

	if (!::hw_3d_flooring_initialize())
	{
		return false;
	}

	if (!::hw_3d_ceiling_initialize())
	{
		return false;
	}

	if (!::hw_3d_fade_initialize())
	{
		return false;
	}

	if (!::hw_3d_player_weapon_initialize())
	{
		return false;
	}

	if (!::hw_samplers_initialize())
	{
		return false;
	}

	if (!::hw_texture_manager_create())
	{
		return false;
	}

	::hw_3d_walls_build();
	::hw_3d_pushwalls_build();
	::hw_3d_doors_build();
	::hw_3d_statics_build();
	::hw_3d_actors_build();

	return true;
}

bool hw_device_reset()
{
	::vid_log();
	::vid_log("Resetting device.");

	::hw_device_reset_resources_destroy();

	::hw_renderer_->device_reset();

	if (!hw_device_reset_resources_create())
	{
		return false;
	}

	return true;
}

void hw_video_uninitialize()
{
	::hw_command_manager_uninitialize();

	::hw_program_uninitialize();

	::hw_samplers_uninitialize();

	::hw_3d_walls_uninitialize();
	::hw_3d_pushwalls_uninitialize();
	::hw_3d_door_sides_uninitialize();
	::hw_3d_sprites_uninitialize();

	::hw_3d_flooring_uninitialize();
	::hw_3d_ceiling_uninitialize();

	::hw_3d_fade_uninitialize();

	::hw_3d_player_weapon_uninitialize();
	::hw_2d_uninitialize();

	::hw_texture_manager_destroy();

	if (::hw_renderer_manager_)
	{
		::hw_renderer_manager_->uninitialize();
		::hw_renderer_manager_ = nullptr;
	}
}

bool hw_video_initialize()
{
	::vid_is_hw_ = false;

	::hw_samplers_set_default_states();
	::hw_3d_player_weapon_sampler_set_default_state();

	::vid_log("Initializing hardware accelerated renderer.");

	hw_renderer_manager_ = bstone::RendererManagerFactory::create();

	if (!hw_renderer_manager_->initialize())
	{
		::vid_log_hw_renderer_manager_error("Failed to initialize renderer manager.");

		return false;
	}

	if (!hw_renderer_manager_->renderer_probe(bstone::RendererPath::auto_detect))
	{
		::vid_log_hw_renderer_manager_error("Failed to auto-detect compatible renderer.");

		return false;
	}

	::vid_common_initialize();
	::hw_dimensions_calculate();


	bool is_succeed = true;

	if (is_succeed)
	{
		is_succeed = ::hw_renderer_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_program_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_texture_manager_create();
	}

	if (is_succeed)
	{
		::hw_ui_buffer_initialize();
	}

	if (is_succeed)
	{
		::hw_palette_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_2d_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_3d_flooring_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_3d_ceiling_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_3d_fade_initialize();
	}

	if (is_succeed)
	{
		::hw_texture_manager_ = bstone::HwTextureManagerFactory::create(
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
		is_succeed = ::hw_3d_player_weapon_initialize();
	}

	if (is_succeed)
	{
		is_succeed = ::hw_command_manager_initialize();
	}

	if (is_succeed)
	{
		::hw_matrices_build();
	}

	if (is_succeed)
	{
		if (!::hw_samplers_initialize())
		{
			is_succeed = false;
		}
	}

	if (is_succeed)
	{
		::vid_is_hw_ = true;

		::hw_device_features_ = ::hw_renderer_->device_get_features();

		::vid_apply_hw_configuration();

		::hw_renderer_->color_buffer_set_clear_color(bstone::R8g8b8a8{});

		::hw_renderer_->window_show(true);

		::in_grab_mouse(true);
	}

	return is_succeed;
}

//
// Hardware accelerated renderer (HW).
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
	if (::hw_video_initialize())
	{
		::hw_screen_refresh();
	}
	else
	{
		::hw_video_uninitialize();
#endif // BSTONE_DBG_FORCE_SW

		::sw_initialize_video();
		::sw_refresh_screen();

#if !BSTONE_DBG_FORCE_SW
	}
#endif // BSTONE_DBG_FORCE_SW

	::in_handle_events();

	if (::vid_is_hw_)
	{
		::hw_vsync_check();
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
		::hw_video_uninitialize();
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
		::hw_palette_update(0, bstone::RgbPalette::get_max_color_count());
	}
	else
	{
		::sw_update_palette(0, bstone::RgbPalette::get_max_color_count());
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
		::hw_palette_update(0, bstone::RgbPalette::get_max_color_count());
	}
	else
	{
		::sw_update_palette(0, bstone::RgbPalette::get_max_color_count());
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

	::hw_2d_fade_color_ = ::hw_vga_color_to_r8g8b8a8(red, green, blue);

	if (!::g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = (i * alpha) / step_count;

			::hw_2d_fade_color_.a = static_cast<std::uint8_t>(new_alpha);

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
	::hw_2d_fade_color_.a = 0xFF;

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

	::hw_2d_fade_color_.a = 0xFF;

	if (!::g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = ((step_count - 1 - i) * alpha) / step_count;

			::hw_2d_fade_color_.a = static_cast<std::uint8_t>(new_alpha);

			::VL_RefreshScreen();

			if (!::vid_has_vsync)
			{
				::VL_WaitVBL(1);
			}
		}
	}

	::hw_2d_fade_color_.a = 0x00;

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
		::hw_screen_refresh();
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
		::hw_screen_refresh();
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
	static_cast<void>(base_offset);

	return ::vid_ui_buffer[(y * ::vga_ref_width) + x];
}

void vl_minimize_fullscreen_window(
	bool value)
{
	if (value)
	{
		::SDL_MinimizeWindow(
			::sw_window.get());
	}
	else
	{
		::SDL_RestoreWindow(
			::sw_window.get());
	}
}

void vl_update_widescreen()
{
	if (::vid_is_hw_)
	{
		::hw_widescreen_update();
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
		VgaBuffer::value_type{});
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

const std::string& vid_filter_to_string(
	const bstone::RendererFilterKind filter)
{
	switch (filter)
	{
		case bstone::RendererFilterKind::nearest:
			return vid_get_nearest_value_string();

		case bstone::RendererFilterKind::linear:
			return vid_get_linear_value_string();

		default:
			::Quit("Invalid filter.");

			return ::vid_get_empty_string();
	}
}

const std::string& vid_renderer_kind_to_string(
	const bstone::RendererPath kind)
{
	switch (kind)
	{
		case bstone::RendererPath::auto_detect:
		case bstone::RendererPath::none:
			return vid_get_auto_detect_value_string();

		case bstone::RendererPath::software:
			return vid_get_software_value_string();

		case bstone::RendererPath::ogl_2_x:
			return vid_get_ogl_2_x_value_string();

		default:
			::Quit("Invalid renderer kind.");

			return ::vid_get_empty_string();
	}
}

void vid_configuration_read_renderer_kind(
	const std::string& value_string)
{
	if (value_string == ::vid_get_none_value_string() ||
		value_string == ::vid_get_auto_detect_value_string())
	{
		::vid_configuration_.renderer_kind_ = bstone::RendererPath::auto_detect;
	}
	else if (value_string == ::vid_get_software_value_string())
	{
		::vid_configuration_.renderer_kind_ = bstone::RendererPath::software;
	}
	else if (value_string == ::vid_get_ogl_2_x_value_string())
	{
		::vid_configuration_.renderer_kind_ = bstone::RendererPath::ogl_2_x;
	}
}

void vid_configuration_read_width(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.width_ = value;
	}
}

void vid_configuration_read_height(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.height_ = value;
	}
}

void vid_configuration_read_is_windowed(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.is_windowed_ = (value != 0);
	}
}

void vid_configuration_read_vsync(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.is_vsync_ = (value != 0);
	}
}

void vid_configuration_read_is_widescreen(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.is_widescreen_ = (value != 0);
	}
}

void vid_configuration_read_is_ui_stretched(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.is_ui_stretched_ = (value != 0);
	}
}

void vid_configuration_read_hw_2d_texture_filter(
	const std::string& value_string)
{
	if (value_string == ::vid_get_nearest_value_string())
	{
		::vid_configuration_.hw_2d_texture_filter_ = bstone::RendererFilterKind::nearest;
	}
	else if (value_string == ::vid_get_linear_value_string())
	{
		::vid_configuration_.hw_2d_texture_filter_ = bstone::RendererFilterKind::linear;
	}
}

void vid_configuration_read_hw_3d_texture_image_filter(
	const std::string& value_string)
{
	if (value_string == ::vid_get_nearest_value_string())
	{
		::vid_configuration_.hw_3d_texture_image_filter_ = bstone::RendererFilterKind::nearest;
	}
	else if (value_string == ::vid_get_linear_value_string())
	{
		::vid_configuration_.hw_3d_texture_image_filter_ = bstone::RendererFilterKind::linear;
	}
}

void vid_configuration_read_hw_3d_texture_mipmap_filter(
	const std::string& value_string)
{
	if (value_string == ::vid_get_nearest_value_string())
	{
		::vid_configuration_.hw_3d_texture_mipmap_filter_ = bstone::RendererFilterKind::nearest;
	}
	else if (value_string == ::vid_get_linear_value_string())
	{
		::vid_configuration_.hw_3d_texture_mipmap_filter_ = bstone::RendererFilterKind::linear;
	}
}

void vid_configuration_read_hw_3d_texture_anisotropy(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.hw_3d_texture_anisotropy_ = (value != 0);
	}
}

void vid_configuration_read_hw_3d_texture_anisotropy_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.hw_3d_texture_anisotropy_value_ = value;
	}
}

void vid_configuration_read_hw_aa_kind(
	const std::string& value_string)
{
	if (value_string == ::vid_get_none_value_string())
	{
		::vid_configuration_.hw_aa_kind_ = bstone::RendererAaKind::none;
	}
	else if (value_string == ::vid_get_msaa_value_string())
	{
		::vid_configuration_.hw_aa_kind_ = bstone::RendererAaKind::ms;
	}
}

void vid_configuration_read_hw_aa_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.hw_aa_value_ = value;
	}
}

void vid_configuration_read_is_downscale_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.is_downscale_ = value;
	}
}

void vid_configuration_read_downscale_width_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.downscale_width_ = value;
	}
}

void vid_configuration_read_downscale_height_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		::vid_configuration_.downscale_height_ = value;
	}
}

void vid_configuration_read_hw_downscale_blit_filter_value(
	const std::string& value_string)
{
	if (value_string == ::vid_get_nearest_value_string())
	{
		::vid_configuration_.hw_downscale_blit_filter_ = bstone::RendererFilterKind::nearest;
	}
	else if (value_string == ::vid_get_linear_value_string())
	{
		::vid_configuration_.hw_downscale_blit_filter_ = bstone::RendererFilterKind::linear;
	}
}

bool vid_parse_configuration_key_value(
	const std::string& key_string,
	const std::string& value_string)
{
	if (false)
	{
	}
	else if (key_string == ::vid_get_renderer_kind_key_name())
	{
		::vid_configuration_read_renderer_kind(value_string);
	}
	else if (key_string == ::vid_get_is_windowed_key_name())
	{
		::vid_configuration_read_is_windowed(value_string);
	}
	else if (key_string == ::vid_get_vsync_key_name())
	{
		::vid_configuration_read_vsync(value_string);
	}
	else if (key_string == ::vid_get_width_key_name())
	{
		::vid_configuration_read_width(value_string);
	}
	else if (key_string == ::vid_get_height_key_name())
	{
		::vid_configuration_read_height(value_string);
	}
	else if (key_string == ::vid_get_is_widescreen_key_name())
	{
		::vid_configuration_read_is_widescreen(value_string);
	}
	else if (key_string == ::vid_get_is_ui_stretched_key_name())
	{
		::vid_configuration_read_is_ui_stretched(value_string);
	}
	else if (key_string == ::vid_get_hw_2d_texture_filter_key_name())
	{
		::vid_configuration_read_hw_2d_texture_filter(value_string);
	}
	else if (key_string == ::vid_get_hw_3d_texture_image_filter_key_name())
	{
		::vid_configuration_read_hw_3d_texture_image_filter(value_string);
	}
	else if (key_string == ::vid_get_hw_3d_texture_mipmap_filter_key_name())
	{
		::vid_configuration_read_hw_3d_texture_mipmap_filter(value_string);
	}
	else if (key_string == ::vid_get_hw_3d_texture_anisotropy_key_name())
	{
		::vid_configuration_read_hw_3d_texture_anisotropy(value_string);
	}
	else if (key_string == ::vid_get_hw_3d_texture_anisotropy_value_key_name())
	{
		::vid_configuration_read_hw_3d_texture_anisotropy_value(value_string);
	}
	else if (key_string == ::vid_get_hw_aa_kind_key_name())
	{
		::vid_configuration_read_hw_aa_kind(value_string);
	}
	else if (key_string == ::vid_get_hw_aa_value_key_name())
	{
		::vid_configuration_read_hw_aa_value(value_string);
	}
	else if (key_string == ::vid_get_is_downscale_key_name())
	{
		::vid_configuration_read_is_downscale_value(value_string);
	}
	else if (key_string == ::vid_get_downscale_width_key_name())
	{
		::vid_configuration_read_downscale_width_value(value_string);
	}
	else if (key_string == ::vid_get_downscale_height_key_name())
	{
		::vid_configuration_read_downscale_height_value(value_string);
	}
	else if (key_string == ::vid_get_hw_downscale_blit_filter_key_name())
	{
		::vid_configuration_read_hw_downscale_blit_filter_value(value_string);
	}
	else
	{
		return false;
	}

	return true;
}

void vid_write_hw_aa_kind_configuration(
	bstone::TextWriter& text_writer)
{
	switch (::vid_configuration_.hw_aa_kind_)
	{
		case bstone::RendererAaKind::ms:
			::write_configuration_entry(
				text_writer,
				::vid_get_hw_aa_kind_key_name(),
				::vid_get_msaa_value_string()
			);

			break;

		default:
			::write_configuration_entry(
				text_writer,
				::vid_get_hw_aa_kind_key_name(),
				::vid_get_none_value_string()
			);

			break;
	}
}

void vid_write_renderer_kind_configuration(
	bstone::TextWriter& text_writer)
{
	switch (::vid_configuration_.renderer_kind_)
	{
		case bstone::RendererPath::ogl_2_x:
			::write_configuration_entry(
				text_writer,
				::vid_get_renderer_kind_key_name(),
				::vid_get_ogl_2_x_value_string()
			);

			break;

		case bstone::RendererPath::auto_detect:
		case bstone::RendererPath::none:
		default:
			::write_configuration_entry(
				text_writer,
				::vid_get_renderer_kind_key_name(),
				::vid_get_auto_detect_value_string()
			);

			break;
	}
}

void vid_write_configuration(
	bstone::TextWriter& text_writer)
{
	text_writer.write("\n// Video\n");

	::vid_write_renderer_kind_configuration(text_writer);

	// vid_is_windowed
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_is_windowed_key_name(),
		std::to_string(::vid_configuration_.is_windowed_)
	);

	// vid_x
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_x_key_name(),
		std::to_string(::vid_configuration_.x_)
	);

	// vid_y
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_y_key_name(),
		std::to_string(::vid_configuration_.y_)
	);

	// vid_width
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_width_key_name(),
		std::to_string(::vid_configuration_.width_)
	);

	// vid_height
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_height_key_name(),
		std::to_string(::vid_configuration_.height_)
	);

	// vid_is_widescreen
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_is_widescreen_key_name(),
		std::to_string(::vid_configuration_.is_widescreen_)
	);

	// vid_vsync
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_vsync_key_name(),
		std::to_string(::vid_configuration_.is_vsync_)
	);

	// vid_is_ui_stretched
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_is_ui_stretched_key_name(),
		std::to_string(::vid_configuration_.is_ui_stretched_)
	);

	// vid_is_downscale
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_is_downscale_key_name(),
		std::to_string(::vid_configuration_.is_downscale_)
	);

	// vid_downscale_width
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_downscale_width_key_name(),
		std::to_string(::vid_configuration_.downscale_width_)
	);

	// vid_downscale_height
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_downscale_height_key_name(),
		std::to_string(::vid_configuration_.downscale_height_)
	);

	// vid_get_hw_dbg_draw_all_key_name
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_dbg_draw_all_key_name(),
		std::to_string(::vid_configuration_.hw_dbg_draw_all_)
	);

	// vid_hw_downscale_blit_filter
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_downscale_blit_filter_key_name(),
		::vid_filter_to_string(::vid_configuration_.hw_downscale_blit_filter_)
	);

	// vid_hw_aa_kind
	//
	vid_write_hw_aa_kind_configuration(text_writer);

	// vid_hw_aa_value
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_aa_value_key_name(),
		std::to_string(::vid_configuration_.hw_aa_value_)
	);

	// vid_hw_2d_texture_filter
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_2d_texture_filter_key_name(),
		::vid_filter_to_string(::vid_configuration_.hw_2d_texture_filter_)
	);

	// vid_hw_3d_texture_image_filter
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_3d_texture_image_filter_key_name(),
		::vid_filter_to_string(::vid_configuration_.hw_3d_texture_image_filter_)
	);

	// vid_hw_3d_texture_mipmap_filter
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_3d_texture_mipmap_filter_key_name(),
		::vid_filter_to_string(::vid_configuration_.hw_3d_texture_mipmap_filter_)
	);

	// vid_hw_3d_texture_anisotropy
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_3d_texture_anisotropy_key_name(),
		std::to_string(::vid_configuration_.hw_3d_texture_anisotropy_)
	);

	// vid_hw_3d_texture_anisotropy_value
	//
	::write_configuration_entry(
		text_writer,
		::vid_get_hw_3d_texture_anisotropy_value_key_name(),
		std::to_string(::vid_configuration_.hw_3d_texture_anisotropy_value_)
	);
}

VidConfiguration& vid_get_configuration()
{
	return vid_configuration_;
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

	::hw_3d_pushwalls_build();
	::hw_3d_walls_build();
	::hw_3d_doors_build();
	::hw_3d_sprites_build();
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
		const auto& assets_info = AssetsInfo{};

		if (assets_info.is_aog())
		{
			::Quit("Expected wall at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
		}
		else
		{
			// Operable non-directly.
			return;
		}
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

	::hw_3d_pushwall_translate();
}

void vid_hw_on_pushwall_step(
	const int old_x,
	const int old_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_pushwall_step(old_x, old_y);
}

void vid_hw_on_pushwall_to_wall(
	const int old_x,
	const int old_y,
	const int new_x,
	const int new_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	assert(old_x != new_x || old_y != new_y);

	const auto old_xy = ::hw_encode_xy(old_x, old_y);
	const auto old_pushwall_it = ::hw_3d_xy_pushwall_map_.find(old_xy);

	if (old_pushwall_it == ::hw_3d_xy_pushwall_map_.cend())
	{
		::Quit("Pushwall not found.");

		return;
	}

	::hw_3d_xy_pushwall_map_.erase(old_pushwall_it);

	auto vertex_index = 0;

	::hw_3d_map_xy_to_xwall(
		Hw3dXyWallKind::solid,
		new_x,
		new_y,
		::hw_3d_xy_wall_map_,
		vertex_index,
		::hw_3d_pushwall_to_wall_vbi_
	);

	// Adjust vertex indices.
	//
	const auto new_xy = ::hw_encode_xy(new_x, new_y);
	auto& wall = ::hw_3d_xy_wall_map_[new_xy];

	for (auto& wall_side : wall.sides_)
	{
		if (!wall_side.flags_.is_active_)
		{
			continue;
		}

		wall_side.vertex_index_ += ::hw_3d_wall_vertex_count_;
	}

	::hw_vertex_buffer_update(
		::hw_3d_wall_sides_vb_,
		::hw_3d_wall_vertex_count_,
		vertex_index,
		::hw_3d_pushwall_to_wall_vbi_.data()
	);

	::hw_3d_wall_vertex_count_ += vertex_index;
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

	auto& hw_door = ::hw_3d_xy_door_map_[xy];

	auto vertex_index = hw_door.vertex_index_;
	const auto old_vertex_index = vertex_index;

	::hw_3d_door_side_map(hw_door.sides_.front(), vertex_index, ::hw_3d_doors_vbi_);

	::hw_vertex_buffer_update(
		::hw_3d_door_sides_vb_,
		old_vertex_index,
		::hw_3d_vertices_per_door,
		&::hw_3d_doors_vbi_[old_vertex_index]
	);
}

void vid_hw_on_door_lock_update(
	const int bs_door_index)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto& bs_door = ::doorobjlist[bs_door_index];

	const auto xy = ::hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = ::hw_3d_xy_door_map_.find(xy);

	if (map_it == ::hw_3d_xy_door_map_.cend())
	{
		::Quit("Door mapping not found.");
	}

	auto& door = ::hw_3d_xy_door_map_[xy];

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	::door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

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

	::hw_3d_static_add(bs_static);
}

void vid_hw_on_static_change_texture(
	const statobj_t& bs_static)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_static_texture_change(bs_static);
}

void vid_hw_on_actor_add(
	const objtype& bs_actor)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_actor_add(bs_actor);
}

void vid_hw_fizzle_fx_set_is_enabled(
	const bool is_enabled)
{
	::hw_3d_fizzle_fx_is_enabled_ = is_enabled;
}

void vid_hw_fizzle_fx_set_is_fading(
	const bool is_fading)
{
	::hw_3d_fizzle_fx_is_fading_ = is_fading;
}

void vid_hw_fizzle_fx_set_color_index(
	const int color_index)
{
	::hw_3d_fizzle_fx_color_index_ = color_index;
}

void vid_hw_fizzle_fx_set_ratio(
	const float ratio)
{
	::hw_3d_fizzle_fx_ratio_ = ratio;
}

void vid_hw_walls_clear_render_list()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_wall_last_xy_to_render_at_ = -1;
	::hw_3d_walls_to_render_.clear();
}

void vid_hw_walls_add_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto xy = ::hw_encode_xy(tile_x, tile_y);

	if (::hw_3d_wall_last_xy_to_render_at_ == xy)
	{
		return;
	}

	::hw_3d_wall_last_xy_to_render_at_ = xy;

	const auto is_pushwall = ::hw_tile_is_pushwall(tile_x, tile_y);

	if (is_pushwall)
	{
		::hw_3d_pushwalls_to_render_.insert(xy);
	}
	else
	{
		::hw_3d_walls_to_render_.insert(xy);
	}
}

void vid_hw_pushwalls_clear_render_list()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_pushwall_last_xy_to_render_at_ = -1;
	::hw_3d_pushwalls_to_render_.clear();
}

void vid_hw_pushwalls_add_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto xy = ::hw_encode_xy(tile_x, tile_y);

	if (::hw_3d_pushwall_last_xy_to_render_at_ == xy)
	{
		return;
	}

	::hw_3d_pushwall_last_xy_to_render_at_ = xy;

	::hw_3d_pushwalls_to_render_.insert(xy);
}

void vid_hw_doors_clear_render_list()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_door_last_xy_to_render_at_ = -1;
	::hw_3d_doors_to_render_.clear();
}

void vid_hw_doors_add_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	const auto xy = ::hw_encode_xy(tile_x, tile_y);

	if (::hw_3d_door_last_xy_to_render_at_ == xy)
	{
		return;
	}

	::hw_3d_door_last_xy_to_render_at_ = xy;

	::hw_3d_doors_to_render_.insert(xy);
}

void vid_hw_statics_clear_render_list()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_statics_to_render_.clear();
}

void vid_hw_statics_add_render_item(
	const int bs_static_index)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_statics_to_render_.insert(bs_static_index);
}

void vid_hw_actors_clear_render_list()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_actors_to_render_.clear();
}

void vid_hw_actors_add_render_item(
	const int bs_actor_index)
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::hw_3d_actors_to_render_.insert(bs_actor_index);
}

void vid_apply_hw_configuration()
{
	if (!::vid_is_hw_)
	{
		return;
	}

	::vid_apply_hw_downscale_configuration();
	::vid_apply_hw_aa_configuration();
	::vid_apply_hw_is_ui_stretched_configuration();
	::vid_apply_hw_is_widescreen_configuration();
	::vid_apply_hw_2d_texture_filter_configuration();
	::vid_apply_hw_3d_texture_filter_configuration();
	::vid_apply_hw_vsync_configuration();
}

const bstone::R8g8b8a8Palette& vid_hw_get_default_palette()
{
	return ::hw_default_palette_;
}
// BBi
