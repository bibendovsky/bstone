/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include <cassert>

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

#include "glm/gtc/matrix_transform.hpp"

#include "id_heads.h"
#include "id_ca.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_atomic_flag.h"
#include "bstone_file_stream.h"
#include "bstone_file_system.h"
#include "bstone_hw_texture_mgr.h"
#include "bstone_logger.h"
#include "bstone_math.h"
#include "bstone_mod_value.h"
#include "bstone_mt_task_mgr.h"
#include "bstone_ren_3d_cmd_buffer.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_ren_3d_mgr.h"
#include "bstone_hw_shader_registry.h"
#include "bstone_sdl2_types.h"
#include "bstone_image_encoder.h"
#include "bstone_sprite.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_time.h"
#include "bstone_text_writer.h"
#include "bstone_version.h"

#include "bstone_detail_ren_3d_utils.h"


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

VidCfg vid_cfg_;

bool vid_hw_is_draw_3d_ = false;

bstone::SpriteCache vid_sprite_cache;


// BBi
namespace
{


void vid_log(
	const std::string& message);

void vid_log_error(
	const std::string& message);

void hw_refresh();
void sw_refresh_screen();

using ScreenshotBuffer = std::unique_ptr<std::uint8_t[]>;


auto vid_is_take_screenshot_scheduled = false;


void vid_take_screenshot();

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically);


// --------------------------------------------------------------------------

class SaveScreenshotMtTask final :
	public bstone::MtTask
{
public:
	// ----------------------------------------------------------------------
	// MtTask

	void execute() override;


	bool is_completed() const noexcept override;

	void set_completed() override;


	bool is_failed() const noexcept override;

	std::exception_ptr get_exception_ptr() const noexcept override;

	void set_failed(
		std::exception_ptr exception_ptr) override;

	// MtTask
	// ----------------------------------------------------------------------


	void reset(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888,
		bool is_flipped_vertically);


private:
	bstone::AtomicFlag is_completed_{true};
	bstone::AtomicFlag is_failed_{};
	std::exception_ptr exception_ptr_{};

	int width_{};
	int height_{};
	int stride_rgb_888_{};
	ScreenshotBuffer src_pixels_rgb_888_{};
	bool is_flipped_vertically_{};
}; // SaveScreenshotMtTask


void SaveScreenshotMtTask::execute()
try
{
	const auto date_time = bstone::make_local_date_time();

	const auto date_time_string = bstone::make_local_date_time_string(
		date_time,
		bstone::DateTimeStringFormat::screenshot_file_name
	);

	const auto& screenshot_dir = get_screenshot_dir();

	const auto& assets_info = get_assets_info();

	auto game_string = std::string{};

	if (assets_info.is_aog())
	{
		game_string = "aog";
	}
	else if (assets_info.is_aog_sw())
	{
		game_string = "aog_sw";
	}
	else if (assets_info.is_ps())
	{
		game_string = "ps";
	}

	const auto file_name = "bstone_" + game_string + "_sshot_" + date_time_string + ".png";

	const auto path = bstone::file_system::append_path(screenshot_dir, file_name);

	vid_log("Taking screenshot \"" + path + "\".");

	if (is_flipped_vertically_)
	{
		auto row_buffer = std::make_unique<std::uint8_t[]>(stride_rgb_888_);
		auto tmp_row = row_buffer.get();

		const auto half_height = height_ / 2;

		auto src_row = src_pixels_rgb_888_.get();
		auto dst_row = src_row + (stride_rgb_888_ * (height_ - 1));

		for (auto h = 0; h < half_height; ++h)
		{
			std::uninitialized_copy_n(src_row, stride_rgb_888_, tmp_row);
			std::uninitialized_copy_n(dst_row, stride_rgb_888_, src_row);
			std::uninitialized_copy_n(tmp_row, stride_rgb_888_, dst_row);

			src_row += stride_rgb_888_;
			dst_row -= stride_rgb_888_;
		}
	}

	const auto max_dst_buffer_size = stride_rgb_888_ * height_;
	auto dst_buffer = std::make_unique<std::uint8_t[]>(max_dst_buffer_size);
	auto image_encoder = bstone::make_image_encoder(bstone::ImageEncoderType::png);

	auto dst_buffer_size = 0;

	image_encoder->encode_24(
		src_pixels_rgb_888_.get(),
		width_,
		height_,
		dst_buffer.get(),
		max_dst_buffer_size,
		dst_buffer_size
	);

	{
		auto file_stream = bstone::FileStream{path, bstone::StreamOpenMode::write};

		if (!file_stream.is_open())
		{
			throw bstone::Exception{"Failed to open a file \"" + path + "\"."};
		}

		file_stream.write(dst_buffer.get(), dst_buffer_size);
	}
}
catch (const std::exception& ex)
{
	vid_log_error("Failed to save a screenshot.");
	vid_log_error(ex.what());
}

bool SaveScreenshotMtTask::is_completed() const noexcept
{
	return is_completed_;
}

void SaveScreenshotMtTask::set_completed()
{
	is_completed_ = true;
}

bool SaveScreenshotMtTask::is_failed() const noexcept
{
	return is_failed_;
}

std::exception_ptr SaveScreenshotMtTask::get_exception_ptr() const noexcept
{
	return exception_ptr_;
}

void SaveScreenshotMtTask::set_failed(
	std::exception_ptr exception_ptr)
{
	is_completed_ = true;
	is_failed_ = true;
	exception_ptr_ = exception_ptr;
}

void SaveScreenshotMtTask::reset(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically)
{
	is_completed_ = false;
	is_failed_ = false;
	exception_ptr_ = nullptr;

	width_ = width;
	height_ = height;
	stride_rgb_888_ = stride_rgb_888;
	src_pixels_rgb_888_ = std::move(src_pixels_rgb_888);
	is_flipped_vertically_ = is_flipped_vertically;
}

constexpr auto max_screenshot_tasks = 16;

using SaveScreenshotMtTasks = std::array<SaveScreenshotMtTask, max_screenshot_tasks>;

auto vid_save_screenshot_mt_tasks = SaveScreenshotMtTasks{};

// --------------------------------------------------------------------------


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


constexpr auto height_compensation_factor = 1.2;

constexpr int default_window_width = 640;
constexpr int default_window_height = 480;


struct WindowElementsDimensions
{
	bool is_windowed_;

	int windowed_width_;
	int windowed_height_;

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


VgaBuffer vid_ui_buffer_;
UiMaskBuffer vid_mask_buffer_;

bstone::Rgba8Palette vid_vga_palette_;
VgaBuffer sw_vga_buffer_;

WindowElementsDimensions vid_dimensions_;

SDL_DisplayMode vid_display_mode_;
bstone::SdlWindowUPtr sw_window_;
bstone::SdlRendererUPtr sw_renderer_;
bstone::SdlPixelFormatUPtr sw_texture_pixel_format_;
bstone::SdlTextureUPtr sw_screen_texture_;
bstone::SdlTextureUPtr sw_ui_texture_;
SdlPalette sw_palette_;
SDL_Rect sw_ui_whole_src_rect_;
SDL_Rect sw_ui_whole_dst_rect_;
SDL_Rect sw_ui_stretched_dst_rect_;
SDL_Rect sw_ui_top_src_rect_;
SDL_Rect sw_ui_top_dst_rect_;
SDL_Rect sw_ui_wide_middle_src_rect_;
SDL_Rect sw_ui_wide_middle_dst_rect_;
SDL_Rect sw_ui_bottom_src_rect_;
SDL_Rect sw_ui_bottom_dst_rect_;
std::array<SDL_Rect, 2> sw_filler_ui_rects_;
std::array<SDL_Rect, 4> sw_filler_hud_rects_;
SDL_Rect sw_screen_dst_rect_;
auto sw_filler_color_ = SDL_Color{};


struct CalculateScreenSizeInputParam
{
	bool is_widescreen_;

	int windowed_width_;
	int windowed_height_;

	int window_width_;
	int window_height_;
}; // CalculateScreenSizeInputParam


const std::string& vid_get_empty_string()
{
	static const auto result = std::string{};

	return result;
}

const std::string& vid_get_nearest_value_string()
{
	static const auto result = std::string{"nearest"};

	return result;
}

const std::string& vid_get_linear_value_string()
{
	static const auto result = std::string{"linear"};

	return result;
}

const std::string& vid_get_none_value_string()
{
	static const auto result = std::string{"none"};

	return result;
}

const std::string& vid_get_msaa_value_string()
{
	static const auto result = std::string{"msaa"};

	return result;
}

const std::string& vid_get_auto_detect_value_string()
{
	static const auto result = std::string{"auto-detect"};

	return result;
}

const std::string& vid_get_software_value_string()
{
	static const auto result = std::string{"software"};

	return result;
}

const std::string& vid_get_gl_2_0_value_string()
{
	static const auto result = std::string{"gl_2_0"};

	return result;
}

const std::string& vid_get_gl_3_2_c_value_string()
{
	static const auto result = std::string{"gl_3_2_c"};

	return result;
}

const std::string& vid_get_gles_2_0_value_string()
{
	static const auto result = std::string{"gles_2_0"};

	return result;
}

const std::string& vid_get_xbrz_value_string()
{
	static const auto result = std::string{"xbrz"};

	return result;
}

const std::string& vid_get_renderer_key_name()
{
	static const auto result = std::string{"vid_renderer"};

	return result;
}

const std::string& vid_get_is_windowed_key_name()
{
	static const auto result = std::string{"vid_is_windowed"};

	return result;
}

const std::string& vid_get_is_positioned_key_name()
{
	static const auto result = std::string{"vid_is_positioned"};

	return result;
}

const std::string& vid_get_windowed_x_key_name()
{
	static const auto result = std::string{"vid_windowed_x"};

	return result;
}

const std::string& vid_get_windowed_y_key_name()
{
	static const auto result = std::string{"vid_windowed_y"};

	return result;
}

const std::string& vid_get_windowed_width_key_name()
{
	static const auto result = std::string{"vid_windowed_width"};

	return result;
}

const std::string& vid_get_windowed_height_key_name()
{
	static const auto result = std::string{"vid_windowed_height"};

	return result;
}

const std::string& vid_get_is_vsync_key_name()
{
	static const auto result = std::string{"vid_is_vsync"};

	return result;
}

const std::string& vid_get_is_ui_stretched_key_name()
{
	static const auto result = std::string{"vid_is_ui_stretched"};

	return result;
}

const std::string& vid_get_is_widescreen_key_name()
{
	static const auto result = std::string{"vid_is_widescreen"};

	return result;
}

const std::string& vid_get_2d_texture_filter_key_name()
{
	static const auto result = std::string{"vid_2d_texture_filter"};

	return result;
}

const std::string& vid_get_3d_texture_image_filter_key_name()
{
	static const auto result = std::string{"vid_3d_texture_image_filter"};

	return result;
}

const std::string& vid_get_3d_texture_mipmap_filter_key_name()
{
	static const auto result = std::string{"vid_3d_texture_mipmap_filter"};

	return result;
}

const std::string& vid_get_3d_texture_anisotropy_key_name()
{
	static const auto result = std::string{"vid_3d_texture_anisotropy"};

	return result;
}

const std::string& vid_get_aa_kind_key_name()
{
	static const auto result = std::string{"vid_aa_kind"};

	return result;
}

const std::string& vid_get_aa_degree_key_name()
{
	static const auto result = std::string{"vid_aa_degree"};

	return result;
}

const std::string& vid_get_texture_upscale_filter_key_name()
{
	static const auto result = std::string{"vid_texture_upscale_filter"};

	return result;
}

const std::string& vid_get_texture_upscale_xbrz_degree_key_name()
{
	static const auto result = std::string{"vid_texture_upscale_xbrz_degree"};

	return result;
}

const std::string& vid_get_filler_color_index_name()
{
	static const auto result = std::string{"vid_filler_color_index"};

	return result;
}

const std::string& vid_get_external_textures_name()
{
	static const auto result = std::string{"vid_external_textures"};

	return result;
}

int vid_align_dimension(
	const int dimension)
{
	const auto alignment = 2;

	return ((dimension + (alignment - 1)) / alignment) * alignment;
}

void vid_cfg_fix_window_dimension(
	int& dimension,
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

void vid_cfg_fix_window_width()
{
	vid_cfg_fix_window_dimension(
		vid_cfg_.windowed_width_,
		vga_ref_width,
		vid_display_mode_.w
	);
}

void vid_cfg_fix_window_height()
{
	vid_cfg_fix_window_dimension(
		vid_cfg_.windowed_height_,
		vga_ref_height_4x3,
		vid_display_mode_.h
	);
}

void vid_cfg_fix_window_size()
{
	vid_cfg_fix_window_width();
	vid_cfg_fix_window_height();
}

void vid_cfg_adjust_window_position()
{
	auto window_x = vid_cfg_.windowed_x_;
	auto window_y = vid_cfg_.windowed_y_;

	if (window_x < 0)
	{
		window_x = 0;
	}

	if (window_y < 0)
	{
		window_y = 0;
	}

	vid_cfg_.windowed_x_ = window_x;
	vid_cfg_.windowed_y_ = window_y;
}

void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	WindowElementsDimensions& dst_param)
{
	dst_param.windowed_width_ = src_param.windowed_width_;
	dst_param.windowed_height_ = src_param.windowed_height_;

	const auto window_width = src_param.window_width_;
	const auto window_height = src_param.window_height_;

	assert(window_width >= vga_ref_width);
	assert(window_height >= vga_ref_height_4x3);

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

	const auto screen_top_filler_height = (ref_top_bar_height * screen_height) / vga_ref_height;
	const auto screen_bottom_filler_height = (ref_bottom_bar_height * screen_height) / vga_ref_height;

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

	const auto screen_viewport_top_height = ((ref_top_bar_height + ref_3d_margin) * screen_height) / vga_ref_height;
	const auto screen_viewport_bottom_height = ((ref_bottom_bar_height + ref_3d_margin) * screen_height) / vga_ref_height;
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

CalculateScreenSizeInputParam vid_create_screen_size_param()
{
	auto window_width = (
		vid_cfg_.is_windowed_ ?
		vid_cfg_.windowed_width_ :
		vid_display_mode_.w
	);

	auto window_height = (
		vid_cfg_.is_windowed_ ?
		vid_cfg_.windowed_height_ :
		vid_display_mode_.h
	);

	if (window_width < vga_ref_width)
	{
		window_width = vga_ref_width;
	}

	if (window_height < vga_ref_height_4x3)
	{
		window_height = vga_ref_height_4x3;
	}

	auto result = CalculateScreenSizeInputParam{};
	result.is_widescreen_ = vid_cfg_.is_widescreen_;
	result.windowed_width_ = vid_cfg_.windowed_width_;
	result.windowed_height_ = vid_cfg_.windowed_height_;
	result.window_width_ = window_width;
	result.window_height_ = window_height;

	return result;
}

void vid_calculate_vga_dimensions()
{
	vga_width = vid_dimensions_.screen_viewport_width_;
	vga_height = vid_align_dimension((10 * vid_dimensions_.screen_height_) / 12);

	vga_width_scale = static_cast<double>(vga_width) / static_cast<double>(vga_ref_width);
	vga_height_scale = static_cast<double>(vga_height) / static_cast<double>(vga_ref_height);

	vga_wide_scale =
		static_cast<double>(vga_ref_height * vga_width) /
		static_cast<double>(vga_ref_width * vga_height);

	vga_area = vga_width * vga_height;
}

std::string vid_get_game_name_and_game_version_string()
{
	const auto& assets_info = get_assets_info();

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

std::string vid_get_port_version_string()
{
	return "BStone v" + bstone::Version::get_string();
}

std::string vid_get_sw_renderer_name();
std::string vid_get_hw_renderer_name();
std::string vid_get_window_title_for_renderer();


void vid_cl_read_bool(
	const std::string& option_name,
	bool& value)
{
	int int_value;

	const auto& value_string = g_args.get_option_value(option_name);

	if (!bstone::StringHelper::string_to_int(value_string, int_value))
	{
		return;
	}

	value = (int_value != 0);
}

void vid_cl_read_int(
	const std::string& option_name,
	int& value)
{
	int int_value;

	const auto& value_string = g_args.get_option_value(option_name);

	if (!bstone::StringHelper::string_to_int(value_string, int_value))
	{
		return;
	}

	value = int_value;
}

void vid_cl_read_is_windowed()
{
	vid_cl_read_bool(vid_get_is_windowed_key_name(), vid_cfg_.is_windowed_);
}

void vid_cl_read_is_positioned()
{
	vid_cl_read_bool(vid_get_is_positioned_key_name(), vid_cfg_.is_positioned_);
}

void vid_cl_read_windowed_x()
{
	vid_cl_read_int(vid_get_windowed_x_key_name(), vid_cfg_.windowed_x_);
}

void vid_cl_read_windowed_y()
{
	vid_cl_read_int(vid_get_windowed_y_key_name(), vid_cfg_.windowed_y_);
}

void vid_cl_read_windowed_width()
{
	vid_cl_read_int(vid_get_windowed_width_key_name(), vid_cfg_.windowed_width_);
}

void vid_cl_read_windowed_height()
{
	vid_cl_read_int(vid_get_windowed_height_key_name(), vid_cfg_.windowed_height_);
}

void vid_cl_read_is_vsync()
{
	vid_cl_read_bool(vid_get_is_vsync_key_name(), vid_cfg_.is_vsync_);
}

void vid_cl_read_is_ui_stretched()
{
	vid_cl_read_bool(vid_get_is_ui_stretched_key_name(), vid_cfg_.is_ui_stretched_);
}

void vid_cl_read_is_widescreen()
{
	vid_cl_read_bool(vid_get_is_widescreen_key_name(), vid_cfg_.is_widescreen_);
}

void vid_cl_read_renderer_filter_kind(
	const std::string& value_string,
	bstone::Ren3dFilterKind& filter_kind)
{
	if (false)
	{
	}
	else if (value_string == vid_get_nearest_value_string())
	{
		filter_kind = bstone::Ren3dFilterKind::nearest;
	}
	else if (value_string == vid_get_linear_value_string())
	{
		filter_kind = bstone::Ren3dFilterKind::linear;
	}
}

void vid_cl_read_2d_texture_filter()
{
	const auto& value_string = g_args.get_option_value(vid_get_2d_texture_filter_key_name());

	vid_cl_read_renderer_filter_kind(value_string, vid_cfg_.d2_texture_filter_);
}

void vid_cl_read_3d_texture_image_filter()
{
	const auto& value_string = g_args.get_option_value(vid_get_3d_texture_image_filter_key_name());

	vid_cl_read_renderer_filter_kind(value_string, vid_cfg_.d3_texture_image_filter_);
}

void vid_cl_read_3d_texture_mipmap_filter()
{
	const auto& value_string = g_args.get_option_value(vid_get_3d_texture_mipmap_filter_key_name());

	vid_cl_read_renderer_filter_kind(value_string, vid_cfg_.d3_texture_mipmap_filter_);
}

void vid_cl_read_3d_texture_anisotropy()
{
	vid_cl_read_int(vid_get_3d_texture_anisotropy_key_name(), vid_cfg_.d3_texture_anisotropy_);
}

void vid_cl_read_aa_kind()
{
	const auto& value_string = g_args.get_option_value(vid_get_aa_kind_key_name());

	if (false)
	{
	}
	else if (value_string == vid_get_none_value_string())
	{
		vid_cfg_.aa_kind_ = bstone::Ren3dAaKind::none;
	}
	else if (value_string == vid_get_msaa_value_string())
	{
		vid_cfg_.aa_kind_ = bstone::Ren3dAaKind::ms;
	}
}

void vid_cl_read_aa_degree()
{
	vid_cl_read_int(vid_get_aa_degree_key_name(), vid_cfg_.aa_degree_);
}

void vid_cl_read_texture_upscale_filter()
{
	const auto& value_string = g_args.get_option_value(vid_get_texture_upscale_filter_key_name());

	if (false)
	{
	}
	else if (value_string == vid_get_none_value_string())
	{
		vid_cfg_.texture_upscale_kind_ = bstone::HwTextureMgrUpscaleFilterKind::none;
	}
	else if (value_string == vid_get_xbrz_value_string())
	{
		vid_cfg_.texture_upscale_kind_ = bstone::HwTextureMgrUpscaleFilterKind::xbrz;
	}
}

void vid_cl_read_texture_upscale_xbrz_degree()
{
	vid_cl_read_int(vid_get_texture_upscale_xbrz_degree_key_name(), vid_cfg_.texture_upscale_xbrz_degree_);
}

void vid_cl_read_filler_color_index()
{
	vid_cl_read_int(vid_get_filler_color_index_name(), vid_cfg_.filler_color_index);
	vid_cfg_.filler_color_index = vid_clamp_filler_color_index(vid_cfg_.filler_color_index);
}

void vid_cl_read_external_textures()
{
	auto int_value = -1;
	vid_cl_read_int(vid_get_external_textures_name(), int_value);

	if (int_value >= 0)
	{
		vid_cfg_.is_external_textures_enabled_ = (int_value != 0);
	}
}

const std::string& vid_get_vid_string()
{
	static const auto result = std::string{"[VID]"};

	return result;
}

[[noreturn]]
void vid_throw_sdl_error(
	const std::string& message_prefix)
{
	auto error_message = vid_get_vid_string();
	error_message += ' ';

	if (!message_prefix.empty())
	{
		error_message += message_prefix;
		error_message += ' ';
	}

	const auto sdl_error_message = SDL_GetError();

	if (sdl_error_message)
	{
		error_message = sdl_error_message;
	}
	else
	{
		error_message = "Generic SDL error.";
	}

	throw bstone::Exception{error_message};
}

[[noreturn]]
void vid_throw_sdl_error()
{
	vid_throw_sdl_error(vid_get_empty_string());
}

[[noreturn]]
void vid_quit(
	const std::string& error_message)
{
	Quit(vid_get_vid_string() + ' ' + error_message);
}

void vid_log()
{
	bstone::logger_->write();
}

void vid_log(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::information,
		vid_get_vid_string() + ' ' + message
	);
}

void vid_log_warning(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::warning,
		vid_get_vid_string() + ' ' + message
	);
}

void vid_log_error(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::error,
		vid_get_vid_string() + ' ' + message
	);
}

[[noreturn]]
void vid_hw_log_error_and_throw(
	const std::string& message)
{
	auto error_message = std::string{};

	error_message += vid_get_vid_string();
	error_message += ' ';
	error_message += message;

	bstone::logger_->write(error_message);

	throw bstone::Exception{error_message};
}

void vid_get_current_display_mode()
{
	vid_log("Getting display mode.");

	const auto sdl_result = SDL_GetDesktopDisplayMode(0, &vid_display_mode_);

	if (sdl_result != 0)
	{
		vid_throw_sdl_error();
	}
}

void vid_cl_read()
{
	static auto is_already_read = false;

	if (is_already_read)
	{
		return;
	}

	is_already_read = true;

	vid_cl_read_external_textures();
	vid_cl_read_is_windowed();
	vid_cl_read_is_positioned();
	vid_cl_read_windowed_x();
	vid_cl_read_windowed_y();
	vid_cl_read_windowed_width();
	vid_cl_read_windowed_height();
	vid_cl_read_is_vsync();
	vid_cl_read_is_ui_stretched();
	vid_cl_read_is_widescreen();
	vid_cl_read_2d_texture_filter();
	vid_cl_read_3d_texture_image_filter();
	vid_cl_read_3d_texture_mipmap_filter();
	vid_cl_read_3d_texture_anisotropy();
	vid_cl_read_aa_kind();
	vid_cl_read_aa_degree();
	vid_cl_read_texture_upscale_filter();
	vid_cl_read_texture_upscale_xbrz_degree();
	vid_cl_read_filler_color_index();
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
	const bstone::Ren3dFilterKind filter_kind)
{
	switch (filter_kind)
	{
		case bstone::Ren3dFilterKind::nearest:
			return vid_get_nearest_value_string();

		case bstone::Ren3dFilterKind::linear:
			return vid_get_linear_value_string();

		default:
			throw bstone::Exception{"Unsupported renderer filter kind."};
	}
}

const std::string& vid_to_string(
	const bstone::Ren3dAaKind aa_kind)
{
	switch (aa_kind)
	{
		case bstone::Ren3dAaKind::ms:
			return vid_get_msaa_value_string();

		case bstone::Ren3dAaKind::none:
			return vid_get_none_value_string();

		default:
			throw bstone::Exception{"Unsupported anti-aliasing kind."};
	}
}

const std::string& vid_to_string(
	const bstone::RendererKind renderer_kind)
{
	static const auto gl_2_0_string = std::string{"OpenGL 2.0"};
	static const auto gl_3_2_core_string = std::string{"OpenGL 3.2 core"};
	static const auto gles_2_0_string = std::string{"OpenGL ES 2.0"};

	switch (renderer_kind)
	{
		case bstone::RendererKind::auto_detect:
			return vid_get_auto_detect_value_string();

		case bstone::RendererKind::software:
			return vid_get_software_value_string();

		case bstone::RendererKind::gl_2_0:
			return gl_2_0_string;

		case bstone::RendererKind::gl_3_2_core:
			return gl_3_2_core_string;

		case bstone::RendererKind::gles_2_0:
			return gles_2_0_string;


		default:
			throw bstone::Exception{"Unsupported renderer kind."};
	}
}

const std::string& vid_to_string(
	const bstone::Ren3dKind renderer_kind)
{
	static const auto gl_2_0_string = std::string{"OpenGL 2.0"};
	static const auto gl_3_2_core_string = std::string{"OpenGL 3.2 core"};
	static const auto gles_2_0_string = std::string{"OpenGL ES 2.0"};

	switch (renderer_kind)
	{
		case bstone::Ren3dKind::gl_2_0:
			return gl_2_0_string;

		case bstone::Ren3dKind::gl_3_2_core:
			return gl_3_2_core_string;

		case bstone::Ren3dKind::gles_2_0:
			return gles_2_0_string;


		default:
			throw bstone::Exception{"Unsupported renderer kind."};
	}
}

const std::string& vid_to_string(
	const bstone::HwTextureMgrUpscaleFilterKind upscale_filter_kind)
{
	switch (upscale_filter_kind)
	{
		case bstone::HwTextureMgrUpscaleFilterKind::none:
			return vid_get_none_value_string();

		case bstone::HwTextureMgrUpscaleFilterKind::xbrz:
			return vid_get_xbrz_value_string();

		default:
			throw bstone::Exception{"Unsupported texture upscale filter kind."};
	}
}

void vid_initialize_vanilla_raycaster()
{
	SetupWalls();
	NewViewSize();
	SetPlaneViewSize();
}

void vid_log_common_configuration()
{
	vid_log();
	vid_log("--------------------");
	vid_log("Common configuration");
	vid_log("--------------------");

	vid_log("Renderer: " + vid_to_string(vid_cfg_.renderer_kind_));

	vid_log("Is windowed: " + vid_to_string(vid_cfg_.is_windowed_));
	vid_log("Window positioned: " + vid_to_string(vid_cfg_.is_positioned_));
	vid_log("Windowed x: " + vid_to_string(vid_cfg_.windowed_x_));
	vid_log("Windowed y: " + vid_to_string(vid_cfg_.windowed_y_));
	vid_log("Windowed width: " + vid_to_string(vid_cfg_.windowed_width_));
	vid_log("Windowed height: " + vid_to_string(vid_cfg_.windowed_height_));

	vid_log("UI stretched: " + vid_to_string(vid_cfg_.is_ui_stretched_));
	vid_log("Widescreen: " + vid_to_string(vid_cfg_.is_widescreen_));

	vid_log("2D texture filter: " + vid_to_string(vid_cfg_.d2_texture_filter_));

	vid_log("3D texture image filter: " + vid_to_string(vid_cfg_.d3_texture_image_filter_));
	vid_log("3D texture mipmap filter: " + vid_to_string(vid_cfg_.d3_texture_mipmap_filter_));

	vid_log("Texture anisotropy: " + vid_to_string(vid_cfg_.d3_texture_anisotropy_));

	vid_log("Texture upscale filter: " + vid_to_string(vid_cfg_.texture_upscale_kind_));
	vid_log("Texture upscale xBRZ factor: " + vid_to_string(vid_cfg_.texture_upscale_xbrz_degree_));

	vid_log("Anti-aliasing kind: " + vid_to_string(vid_cfg_.aa_kind_));
	vid_log("Anti-aliasing value: " + vid_to_string(vid_cfg_.aa_degree_));

	vid_log("--------------------");
}

void vid_initialize_common()
{
	vid_get_current_display_mode();
	vid_cfg_adjust_window_position();
	vid_cfg_fix_window_size();

	vid_log_common_configuration();
}

void vid_check_vsync()
{
	using Clock = std::chrono::steady_clock;

	constexpr int draw_count = 10;

	constexpr int duration_tolerance_pct = 25;

	const int expected_duration_ms =
		(1000 * draw_count) / vid_display_mode_.refresh_rate;

	const int min_expected_duration_ms =
		((100 - duration_tolerance_pct) * expected_duration_ms) / 100;

	const auto before_timestamp = Clock::now();

	for (int i = 0; i < draw_count; ++i)
	{
		if (vid_is_hw_)
		{
			hw_refresh();
		}
		else
		{
			sw_refresh_screen();
		}
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}


void sw_initialize_vga_buffer()
{
	const auto area = 2 * vga_width * vga_height;

	sw_vga_buffer_.resize(area);

	vga_memory = sw_vga_buffer_.data();
}

void sw_initialize_ui_buffer()
{
	const auto area = vga_ref_width * vga_ref_height;

	vid_ui_buffer_.resize(area);
}

void sw_create_window()
{
	vid_log("Creating window.");


	int window_x;
	int window_y;

	if (vid_cfg_.is_positioned_)
	{
		window_x = vid_cfg_.windowed_x_;
		window_y = vid_cfg_.windowed_y_;
	}
	else
	{
		window_x = SDL_WINDOWPOS_CENTERED;
		window_y = SDL_WINDOWPOS_CENTERED;
	}

	auto window_flags = Uint32{
		SDL_WINDOW_HIDDEN |
		SDL_WINDOW_ALLOW_HIGHDPI |
		0
	};

	if (!vid_cfg_.is_windowed_)
	{
		window_flags |=
			SDL_WINDOW_FULLSCREEN_DESKTOP |
			0;
	}

#ifdef __vita__
	window_flags = SDL_WINDOW_SHOWN;
#endif

	const auto title = vid_get_game_name_and_game_version_string();

	sw_window_ = bstone::SdlWindowUPtr{SDL_CreateWindow(
		title.c_str(),
		window_x,
		window_y,
		vid_dimensions_.window_width_,
		vid_dimensions_.window_height_,
		window_flags
	)};

	if (!sw_window_)
	{
		vid_throw_sdl_error();
	}
}

void sw_initialize_renderer()
{
	vid_log();
	vid_log("Initializing renderer.");

	{
		vid_log("Available renderer drivers:");

		const auto driver_count = SDL_GetNumRenderDrivers();

		for (int i = 0; i < driver_count; ++i)
		{
			SDL_RendererInfo info;

			const auto sdl_result = SDL_GetRenderDriverInfo(i, &info);

			if (sdl_result == 0)
			{
				vid_log(std::to_string(i + 1) + ". " + info.name);
			}
		}
	}


	std::uint32_t renderer_flags = 0;
	const char* renderer_driver = nullptr;

	{
		if (!vid_cfg_.is_vsync_)
		{
			vid_log("Skipping VSync.");
		}
		else
		{
			renderer_flags = SDL_RENDERER_PRESENTVSYNC;

			vid_log("Using VSync.");
		}
	}

	{
		vid_log("Creating renderer.");

		sw_renderer_ = bstone::SdlRendererUPtr{SDL_CreateRenderer(
			sw_window_.get(),
			-1,
			renderer_flags
		)};

		if (!sw_renderer_)
		{
			vid_throw_sdl_error();
		}
	}


	auto renderer_info = SDL_RendererInfo{};

	{
		vid_log("Quering renderer for information.");

		auto sdl_result = SDL_GetRendererInfo(
			sw_renderer_.get(),
			&renderer_info);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error();
		}
	}


	{
		if (renderer_driver)
		{
			if (SDL_strcasecmp(renderer_driver, renderer_info.name) != 0)
			{
				vid_log_warning("Mismatch renderer: \"" + std::string{renderer_info.name} + "\".");
			}
		}
		else
		{
			vid_log("Renderer: \"" + std::string{renderer_info.name} + "\".");
		}
	}


	std::uint32_t pixel_format = SDL_PIXELFORMAT_UNKNOWN;

	{
		vid_log("Looking up for a texture pixel format.");

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
			vid_log_warning("Falling back to a predefined pixel format.");

			pixel_format = SDL_PIXELFORMAT_ARGB8888;
		}

		const auto pixel_format_name = SDL_GetPixelFormatName(SDL_PIXELFORMAT_ARGB8888);
		vid_log(std::string{"Pixel format: \""} + pixel_format_name + '\"');

		vid_log("Allocating a texture pixel format.");

		sw_texture_pixel_format_ = bstone::SdlPixelFormatUPtr{SDL_AllocFormat(pixel_format)};

		if (!sw_texture_pixel_format_)
		{
			vid_throw_sdl_error();
		}
	}
}

void sw_create_screen_texture()
{
	vid_log("Creating screen texture.");

	sw_screen_texture_ = bstone::SdlTextureUPtr{SDL_CreateTexture(
		sw_renderer_.get(),
		sw_texture_pixel_format_->format,
		SDL_TEXTUREACCESS_STREAMING,
		vga_width,
		vga_height
	)};

	if (!sw_screen_texture_)
	{
		vid_throw_sdl_error();
	}
}

void sw_create_ui_texture()
{
	vid_log("Creating UI texture.");

	sw_ui_texture_ = bstone::SdlTextureUPtr{SDL_CreateTexture(
		sw_renderer_.get(),
		sw_texture_pixel_format_->format,
		SDL_TEXTUREACCESS_STREAMING,
		vga_ref_width,
		vga_ref_height
	)};

	if (!sw_ui_texture_)
	{
		vid_throw_sdl_error();
	}
}

void sw_initialize_textures()
{
	vid_log();
	vid_log("Initializing textures.");

	sw_create_screen_texture();
	sw_create_ui_texture();
}

void sw_update_palette(
	int first_index,
	int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = vid_vga_palette_[color_index];
		auto& sdl_color = sw_palette_[color_index];

		sdl_color = SDL_MapRGB(
			sw_texture_pixel_format_.get(),
			(255 * vga_color.r_) / 63,
			(255 * vga_color.g_) / 63,
			(255 * vga_color.b_) / 63);
	}
}

void sw_update_viewport()
{
	auto sdl_viewport = SDL_Rect{};
	sdl_viewport.w = vid_dimensions_.window_width_;
	sdl_viewport.h = vid_dimensions_.window_height_;

	auto sdl_result = SDL_RenderSetViewport(sw_renderer_.get(), &sdl_viewport);

	if (sdl_result != 0)
	{
		vid_throw_sdl_error("Failed to update a viewport.");
	}
}

void sw_initialize_palette()
{
	vid_vga_palette_.fill({});

	sw_update_palette(
		0,
		bstone::RgbPalette::get_max_color_count());
}

void sw_apply_filler_color_index()
{
	sw_filler_color_.r = static_cast<Uint8>((255 * vgapal[(vid_cfg_.filler_color_index * 3) + 0]) / 63);
	sw_filler_color_.g = static_cast<Uint8>((255 * vgapal[(vid_cfg_.filler_color_index * 3) + 1]) / 63);
	sw_filler_color_.b = static_cast<Uint8>((255 * vgapal[(vid_cfg_.filler_color_index * 3) + 2]) / 63);
	sw_filler_color_.a = 0xFF;
}

void sw_calculate_dimensions()
{
	auto src_param = vid_create_screen_size_param();

	vid_calculate_window_elements_dimensions(src_param, vid_dimensions_);

	vid_calculate_vga_dimensions();


	// UI whole rect
	//
	sw_ui_whole_src_rect_ = SDL_Rect
	{
		0,
		0,
		vga_ref_width,
		vga_ref_height,
	};

	sw_ui_whole_dst_rect_ = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_width_4x3_,
		vid_dimensions_.screen_height_,
	};


	// UI stretched rect
	//
	sw_ui_stretched_dst_rect_ = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_width_,
		vid_dimensions_.screen_height_,
	};


	// UI top rect
	//
	sw_ui_top_src_rect_ = SDL_Rect
	{
		0,
		0,
		vga_ref_width,
		ref_top_bar_height,
	};

	sw_ui_top_dst_rect_ = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_width_4x3_,
		vid_dimensions_.screen_top_filler_height_,
	};


	// UI middle rect (stretched to full width)
	//
	sw_ui_wide_middle_src_rect_ = SDL_Rect
	{
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
	};

	sw_ui_wide_middle_dst_rect_ = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_,
		vid_dimensions_.window_viewport_top_height_ + vid_dimensions_.screen_top_filler_height_,
		vid_dimensions_.screen_width_,
		vid_dimensions_.screen_height_,
	};


	// UI bottom rect
	//
	sw_ui_bottom_src_rect_ = SDL_Rect
	{
		0,
		ref_view_bottom_y + 1,
		vga_ref_width,
		ref_bottom_bar_height,
	};

	sw_ui_bottom_dst_rect_ = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.window_viewport_top_height_ + vid_dimensions_.screen_height_ - vid_dimensions_.screen_bottom_filler_height_,
		vid_dimensions_.screen_width_4x3_,
		vid_dimensions_.screen_bottom_filler_height_,
	};


	// UI left bar
	sw_filler_ui_rects_[0] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.screen_height_,
	};

	// UI right bar
	sw_filler_ui_rects_[1] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_width_ - vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.screen_height_,
	};

	// HUD upper left rect
	sw_filler_hud_rects_[0] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.screen_top_filler_height_,
	};

	// HUD upper right rect
	sw_filler_hud_rects_[1] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_width_ - vid_dimensions_.screen_right_filler_width_,
		vid_dimensions_.window_viewport_top_height_,
		vid_dimensions_.screen_right_filler_width_,
		vid_dimensions_.screen_top_filler_height_,
	};

	// HUD lower left rect
	sw_filler_hud_rects_[2] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_,
		vid_dimensions_.window_viewport_top_height_ + vid_dimensions_.screen_height_ - vid_dimensions_.screen_bottom_filler_height_,
		vid_dimensions_.screen_left_filler_width_,
		vid_dimensions_.screen_bottom_filler_height_,
	};

	// HUD lower right rect
	sw_filler_hud_rects_[3] = SDL_Rect
	{
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_width_ - vid_dimensions_.screen_right_filler_width_,
		vid_dimensions_.window_viewport_top_height_ + vid_dimensions_.screen_height_ - vid_dimensions_.screen_bottom_filler_height_,
		vid_dimensions_.screen_right_filler_width_,
		vid_dimensions_.screen_bottom_filler_height_,
	};

	sw_apply_filler_color_index();

	// Screen destination rect.

	const auto screen_left = (
		vid_cfg_.is_widescreen_ ?
		0 :
		vid_dimensions_.window_viewport_left_width_ + vid_dimensions_.screen_left_filler_width_);

	const auto screen_top = vid_dimensions_.window_viewport_top_height_;
	const auto screen_width = (vid_cfg_.is_widescreen_ ? vid_dimensions_.screen_width_ : vid_dimensions_.screen_width_4x3_);
	const auto screen_height = vid_dimensions_.screen_height_;

	sw_screen_dst_rect_ = SDL_Rect
	{
		screen_left,
		screen_top,
		screen_width,
		screen_height,
	};
}

void sw_initialize_video()
{
	vid_log();
	vid_log("Initializing software accelerated video system.");

	vid_initialize_common();
	sw_calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	sw_create_window();
	sw_initialize_renderer();
	sw_initialize_textures();
	sw_initialize_palette();
	sw_initialize_vga_buffer();
	sw_initialize_ui_buffer();

	const auto window_title = vid_get_window_title_for_renderer();
	SDL_SetWindowTitle(sw_window_.get(), window_title.c_str());

	SDL_ShowWindow(sw_window_.get());
	in_grab_mouse(true);
}

void sw_destroy_screen_texture()
{
	sw_screen_texture_ = nullptr;
}

void sw_destroy_ui_texture()
{
	sw_ui_texture_ = nullptr;
}

void sw_uninitialize_vga_buffer()
{
	sw_vga_buffer_ = VgaBuffer{};

	vga_memory = nullptr;
}

void sw_uninitialize_video()
{
	sw_texture_pixel_format_ = nullptr;
	sw_destroy_screen_texture();
	sw_destroy_ui_texture();
	sw_renderer_ = nullptr;
	sw_window_ = nullptr;

	sw_uninitialize_vga_buffer();
}

void sw_refresh_screen()
{
	int sdl_result = 0;

	// HUD+3D stuff
	//
	if (vid_is_hud)
	{
		const auto src_pixels = sw_vga_buffer_.data();
		const auto src_pitch = vga_width;

		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = SDL_LockTexture(
			sw_screen_texture_.get(),
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to lock a screen texture.");
		}

		auto dst_pixels = static_cast<std::uint32_t*>(dst_raw_pixels);

		for (int y = 0; y < vga_height; ++y)
		{
			const auto src_line = &src_pixels[y * src_pitch];
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < vga_width; ++x)
			{
				dst_line[x] = sw_palette_[src_line[x]];
			}
		}

		SDL_UnlockTexture(sw_screen_texture_.get());
	}


	// 2D stuff
	//
	{
		void* dst_raw_pixels = nullptr;
		int dst_pitch = 0;

		sdl_result = SDL_LockTexture(
			sw_ui_texture_.get(),
			nullptr,
			&dst_raw_pixels,
			&dst_pitch);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to lock an UI texture.");
		}

		const auto alpha_0_mask = ~sw_texture_pixel_format_->Amask;

		auto dst_pixels = static_cast<std::uint32_t*>(dst_raw_pixels);

		for (int y = 0; y < vga_ref_height; ++y)
		{
			auto dst_line = &dst_pixels[y * (dst_pitch / 4)];

			for (int x = 0; x < vga_ref_width; ++x)
			{
				const auto src_offset = (y * vga_ref_width) + x;
				auto dst_color = sw_palette_[vid_ui_buffer_[src_offset]];

				if (vid_is_hud)
				{
					if (!vid_mask_buffer_[src_offset])
					{
						dst_color &= alpha_0_mask;
					}
				}

				dst_line[x] = dst_color;
			}
		}

		SDL_UnlockTexture(sw_ui_texture_.get());
	}


	// Clear all
	//
	sdl_result = SDL_SetRenderDrawColor(
		sw_renderer_.get(),
		0x00,
		0x00,
		0x00,
		0xFF);

	if (sdl_result != 0)
	{
		vid_throw_sdl_error("Failed to set draw color.");
	}
	
	sdl_result = SDL_RenderClear(sw_renderer_.get());

	if (sdl_result != 0)
	{
		vid_throw_sdl_error("Failed to clear a render target.");
	}


	// Copy HUD+3D stuff
	//
	if (vid_is_hud)
	{
		sdl_result = SDL_RenderCopy(
			sw_renderer_.get(),
			sw_screen_texture_.get(),
			nullptr,
			&sw_screen_dst_rect_);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to copy a screen texture on a render target.");
		}
	}

	// Copy 2D stuff
	//
	if (vid_is_hud)
	{
		sdl_result = SDL_SetTextureBlendMode(
			sw_ui_texture_.get(),
			SDL_BLENDMODE_BLEND);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to set blend mode for an UI texture.");
		}
	}

	if (!vid_cfg_.is_ui_stretched_)
	{
		if (vid_is_fizzle_fade)
		{
			if (sdl_result == 0)
			{
				sdl_result = SDL_RenderCopy(
					sw_renderer_.get(),
					sw_ui_texture_.get(),
					&sw_ui_top_src_rect_,
					&sw_ui_top_dst_rect_);
			}

			if (sdl_result == 0)
			{
				sdl_result = SDL_RenderCopy(
					sw_renderer_.get(),
					sw_ui_texture_.get(),
					&sw_ui_wide_middle_src_rect_,
					&sw_ui_wide_middle_dst_rect_);
			}

			if (sdl_result == 0)
			{
				sdl_result = SDL_RenderCopy(
					sw_renderer_.get(),
					sw_ui_texture_.get(),
					&sw_ui_bottom_src_rect_,
					&sw_ui_bottom_dst_rect_);
			}
		}
		else
		{
			sdl_result = SDL_RenderCopy(
				sw_renderer_.get(),
				sw_ui_texture_.get(),
				nullptr,
				&sw_ui_whole_dst_rect_);
		}
	}
	else
	{
		sdl_result = SDL_RenderCopy(
			sw_renderer_.get(),
			sw_ui_texture_.get(),
			nullptr,
			&sw_ui_stretched_dst_rect_);
	}

	if (sdl_result != 0)
	{
		vid_throw_sdl_error("Failed to copy an UI texture on render target.");
	}

	if (vid_is_hud)
	{
		sdl_result = SDL_SetTextureBlendMode(
			sw_ui_texture_.get(),
			SDL_BLENDMODE_NONE);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to set blend mode for an UI texture.");
		}
	}


	// Use filler if necessary
	//
	if (!vid_cfg_.is_ui_stretched_)
	{
		const auto is_hud = vid_is_hud;

		auto fill_color = SDL_Color{};

		if (!vid_is_movie)
		{
			fill_color = sw_filler_color_;
		}

		sdl_result = SDL_SetRenderDrawColor(
			sw_renderer_.get(),
			fill_color.r,
			fill_color.g,
			fill_color.b,
			0xFF);

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to set draw color.");
		}

		if (is_hud)
		{
			sdl_result = SDL_RenderFillRects(sw_renderer_.get(), sw_filler_hud_rects_.data(), 4);
		}
		else
		{
			sdl_result = SDL_RenderFillRects(sw_renderer_.get(), sw_filler_ui_rects_.data(), 2);
		}

		if (sdl_result != 0)
		{
			vid_throw_sdl_error("Failed to draw fillers.");
		}
	}

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	// Present
	//
	SDL_RenderPresent(sw_renderer_.get());
}

void sw_apply_widescreen()
{
	sw_destroy_screen_texture();
	sw_uninitialize_vga_buffer();
	sw_calculate_dimensions();
	sw_initialize_vga_buffer();
	sw_create_screen_texture();
	sw_update_viewport();
}

void sw_take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
{
	const auto sdl_read_pixles_rgb_888_result = SDL_RenderReadPixels(
		sw_renderer_.get(),
		nullptr,
		SDL_PIXELFORMAT_RGB24,
		src_pixels_rgb_888.get(),
		stride_rgb_888
	);

	if (sdl_read_pixles_rgb_888_result != 0)
	{
		vid_throw_sdl_error();
	}

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		false
	);
}

// ==========================================================================
// Hardware accelerated renderer (HW).
//

enum class HwVertexAttribLocationId :
	unsigned char
{
	none,
	position,
	color,
	texture_coordinates,
}; // HwVertexAttribLocationId


template<typename T>
const T hw_map_dimension = static_cast<T>(MAPSIZE);

const auto hw_map_dimension_f = hw_map_dimension<float>;
const auto hw_map_dimension_d = hw_map_dimension<double>;
const auto hw_map_height_f = 1.0F;

template<typename T>
constexpr auto hw_tile_dimension = static_cast<T>(1);

constexpr auto hw_tile_dimension_f = hw_tile_dimension<float>;

template<typename T>
constexpr auto hw_tile_half_dimension = hw_tile_dimension<T> / static_cast<T>(2);

constexpr auto hw_tile_half_dimension_d = hw_tile_half_dimension<double>;


constexpr auto hw_vertices_per_triangle = 3;
constexpr auto hw_triangles_per_quad = 2;
constexpr auto hw_vertices_per_quad = hw_triangles_per_quad * hw_vertices_per_triangle;

constexpr auto hw_sides_per_wall = 4;
constexpr auto hw_indices_per_wall_side = 6;
constexpr auto hw_vertices_per_wall_side = 4;

constexpr auto hw_max_wall_sides_indices = 0x10000;

constexpr auto hw_sides_per_door = 2;
constexpr auto hw_door_halves_per_side = 2;
constexpr auto hw_door_halves_per_door = hw_sides_per_door * hw_door_halves_per_side;
constexpr auto hw_vertices_per_door_half = 4;
constexpr auto hw_vertices_per_door = hw_sides_per_door * hw_vertices_per_door_half;

constexpr auto hw_indices_per_door_half = 6;
constexpr auto hw_indices_per_door = 2 * hw_indices_per_door_half;
constexpr auto hw_indices_per_door_side = 2 * hw_indices_per_door;

constexpr auto hw_max_sprites = MAXSTATS + MAXACTORS;

constexpr auto hw_vertices_per_sprite = 4;
constexpr auto hw_indices_per_sprite = 6;

constexpr auto hw_max_statics_vertices = MAXSTATS * hw_vertices_per_sprite;
constexpr auto hw_max_actors_vertices = MAXACTORS * hw_vertices_per_sprite;
constexpr auto hw_max_sprites_vertices = hw_max_statics_vertices + hw_max_actors_vertices;
constexpr auto hw_statics_base_vertex_index = 0;
constexpr auto hw_actors_base_vertex_index = hw_max_statics_vertices;

constexpr auto hw_max_statics_indices = MAXSTATS * hw_indices_per_sprite;
constexpr auto hw_max_actor_indices = MAXACTORS * hw_indices_per_sprite;
constexpr auto hw_max_sprites_indices = hw_max_statics_indices + hw_max_actor_indices;

constexpr auto hw_cloaked_actor_alpha_u8 = std::uint8_t{0x50};

constexpr auto hw_common_command_buffer_initial_size = 1'024;
constexpr auto hw_common_command_buffer_resize_delta_size = 1'024;

constexpr auto hw_2d_command_buffer_initial_size = 4'096;
constexpr auto hw_2d_command_buffer_resize_delta_size = 4'096;

constexpr auto hw_3d_command_buffer_initial_size = 16'384;
constexpr auto hw_3d_command_buffer_resize_delta_size = 16'384;


template<
	typename T,
	HwVertexAttribLocationId TLocationId,
	typename = int>
struct HwVertexHasAttribute :
	std::false_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttribLocationId::position,
	decltype(static_cast<void>(T::xyz_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttribLocationId::color,
	decltype(static_cast<void>(T::rgba_8_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute

template<typename T>
struct HwVertexHasAttribute<
	T,
	HwVertexAttribLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv_), 0)>
	:
	std::true_type
{
}; // HwVertexHasAttribute


template<
	typename T,
	HwVertexAttribLocationId TLocationId,
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
	HwVertexAttribLocationId::position,
	decltype(static_cast<void>(T::xyz_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, xyz_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits

template<typename T>
struct HwVertexAttributeTraits<
	T,
	HwVertexAttribLocationId::color,
	decltype(static_cast<void>(T::rgba_8_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, rgba_8_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits

template<typename T>
struct HwVertexAttributeTraits<
	T,
	HwVertexAttribLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv_), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, uv_));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // HwVertexAttributeTraits


using HwVertexPosition = glm::vec3;
using HwVertexColor = bstone::Rgba8;
using HwVertexTextureCoordinates = glm::vec2;

struct HwVertexXyzUv
{
	HwVertexPosition xyz_;
	HwVertexTextureCoordinates uv_;
}; // HwVertexXyzUv

struct HwVertexXyzRgbaUv
{
	HwVertexPosition xyz_;
	HwVertexColor rgba_8_;
	HwVertexTextureCoordinates uv_;
}; // HwVertexXyzRgbaUv


struct HwVertex : HwVertexXyzRgbaUv {};
struct HwWallVertex : HwVertexXyzUv {};
struct HwPushwallVertex : HwVertexXyzRgbaUv {};
struct HwFlooringVertex : HwVertexXyzUv {};
struct HwCeilingVertex : HwVertexXyzUv {};
struct HwDoorVertex : HwVertexXyzUv {};
struct HwSpriteVertex : HwVertexXyzRgbaUv {};
struct HwPlayerWeaponVertex : HwVertexXyzUv {};
struct HwFadeVertex : HwVertexXyzUv {};


struct HwQuadFlags
{
	using Value = unsigned char;


	Value is_vertical_ : 1;
	Value is_back_face_ : 1;
}; // HwQuadFlags


struct HwWall;
using HwWallPtr = HwWall*;
using HwWallCPtr = const HwWall*;

struct HwWallSideFlags
{
	using Type = unsigned char;


	Type is_active_ : 1;
	Type is_vertical_ : 1;
	Type is_door_track_ : 1;
}; // HwWallSideFlags

struct HwWallSide
{
	HwWallCPtr wall_;

	HwWallSideFlags flags_;
	int vertex_index_;
	int texture_id_;
}; // HwWallSide

using HwWallSidePtr = HwWallSide*;
using HwWallSideCPtr = const HwWallSide*;


struct HwWall
{
	static constexpr auto max_sides = 4;

	using Sides = std::array<HwWallSide, max_sides>;


	int x_;
	int y_;

	Sides sides_;
}; // HwWall

using HwXyWallMap = std::unordered_map<int, HwWall>;
using HwWallsToRenderList = std::unordered_set<int>;

enum HwXyWallKind
{
	solid,
	push,
}; // HwXyWallKind

struct HwWallSideDrawItem
{
	int texture_id_;
	HwWallSideCPtr wall_side_;
}; // HwWallSideDrawItem


struct HwDoor;
using HwDoorPtr = HwDoor*;

struct HwDoorSide
{
	HwDoorPtr hw_door_;

	bool is_back_face_;
	int texture_id_;
}; // HwDoorSide

using HwDoorSidePtr = HwDoorSide*;
using HwDoorSideCPtr = const HwDoorSide*;


struct HwDoor
{
	using Sides = std::array<HwDoorSide, hw_sides_per_door>;

	int bs_door_index_;
	int vertex_index_;
	Sides sides_;
}; // HwDoor


struct HwDoorDrawItem
{
	int texture_id_;
	HwDoorSideCPtr hw_door_side_;
}; // HwDoorDrawItem

using HwXyDoorMap = std::unordered_map<int, HwDoor>;
using HwDoorsToRenderList = std::unordered_set<int>;
using HwDoorDrawItems = std::vector<HwDoorDrawItem>;

using HwWallSideDrawItems = std::vector<HwWallSideDrawItem>;

using HwWallSideIndexBuffer = std::vector<std::uint16_t>;
using HwDoorIndexBuffer = std::vector<std::uint16_t>;


enum class HwSpriteKind
{
	none,
	stat,
	actor,
}; // HwSpriteKind

struct HwSpriteFlags
{
	using Value = unsigned char;


	Value is_visible_ : 1;
}; // HwSpriteFlags

struct HwSprite
{
	union BsObject
	{
		const statobj_t* stat_;
		const objtype* actor_;
	}; // BsObject


	double x_;
	double y_;
	int tile_x_;
	int tile_y_;
	int bs_sprite_id_;
	double square_distance_;

	HwSpriteKind kind_;
	HwSpriteFlags flags_;
	int vertex_index_;
	BsObject bs_object_;
}; // HwSprite


using HwSpritePtr = HwSprite*;
using HwSpriteCPtr = const HwSprite*;

struct HwSpriteDrawItem
{
	int texture_id_;
	HwSpriteCPtr sprite_;
}; // HwSpriteDrawItem

using HwSprites = std::vector<HwSprite>;
using HwSpritesPtr = HwSprites*;

using HwStaticsToRenderList = std::unordered_set<int>;
using HwActorsToRenderList = std::unordered_set<int>;
using HwSpritesDrawList = std::vector<HwSpriteDrawItem>;

using HwSpritesIndexBuffer = std::vector<std::uint16_t>;


template<typename TVertex>
using HwVertexBufferImageT = std::vector<TVertex>;

using HwVbi = HwVertexBufferImageT<HwVertex>;
using HwWallsVbi = HwVertexBufferImageT<HwWallVertex>;
using HwPushwallsVbi = HwVertexBufferImageT<HwPushwallVertex>;
using HwDoorsVbi = HwVertexBufferImageT<HwDoorVertex>;
using HwSpritesVbi = HwVertexBufferImageT<HwSpriteVertex>;
using HwPlayerWeaponVbi = HwVertexBufferImageT<HwPlayerWeaponVertex>;
using HwFadeVbi = HwVertexBufferImageT<HwFadeVertex>;


using HwSamplerVarMod = bstone::ModValue<int>;
HwSamplerVarMod hw_sampler_var_;

using HwShadingModeMod = bstone::ModValue<int>;
HwShadingModeMod hw_shading_mode_;

using HwBsShadeMaxMod = bstone::ModValue<int>;
HwBsShadeMaxMod hw_bs_shade_max_;

using HwBsNormalShadeMod = bstone::ModValue<int>;
HwBsNormalShadeMod hw_bs_normal_shade_;

using HwBsHeightNumeratorMod = bstone::ModValue<double>;
HwBsHeightNumeratorMod hw_bs_height_numerator_;

using HwBsLightingMod = bstone::ModValue<int>;
HwBsLightingMod hw_bs_lighting_;

using HwBsViewDirectionMod = bstone::ModValue<glm::vec2>;
HwBsViewDirectionMod hw_bs_view_direction_;

using HwBsViewPositionMod = bstone::ModValue<glm::vec2>;
HwBsViewPositionMod hw_bs_view_position_;


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
constexpr auto hw_2d_fillers_quad_count = hw_2d_fillers_ui_quad_count + hw_2d_fillers_hud_quad_count;

constexpr auto hw_2d_fillers_index_count_ = hw_2d_fillers_quad_count * 6;
constexpr auto hw_2d_fillers_ui_index_offset_ = 0;
constexpr auto hw_2d_fillers_hud_index_offset_ = 6 * hw_2d_fillers_ui_quad_count;

constexpr auto hw_2d_fillers_vertex_count_ = hw_2d_fillers_quad_count * 4;



// Reference horizontal FOV of the camera (degrees).
// (Found out empirical.)
const double hw_ref_camera_hfov_deg = 49.0;

// Vertical FOV in radians of the camera.
double hw_camera_vfov_rad = 0.0;

// Vertical FOV in degrees of the camera.
double hw_camera_vfov_deg = 0.0;

// Distance to the near plane of the camera.
double hw_camera_near_distance = 0.0;

// Distance to the far plane of the camera.
double hw_camera_far_distance = 0.0;


bstone::Ren3dSamplerState hw_ui_sampler_state_;
bstone::Ren3dSamplerUPtr hw_ui_sampler_;

bstone::Ren3dSamplerState hw_wall_sampler_state_;
bstone::Ren3dSamplerUPtr hw_wall_sampler_;

bstone::Ren3dSamplerState hw_sprite_sampler_state_;
bstone::Ren3dSamplerUPtr hw_sprite_sampler_;

bstone::Ren3dSamplerState hw_player_weapon_sampler_state_;
bstone::Ren3dSamplerUPtr hw_player_weapon_sampler_;

bstone::Ren3dSamplerUPtr hw_fade_sampler_;


Hw2dVbi hw_2d_vertices_;

bstone::Ren3dDeviceFeatures hw_device_features_;
bstone::Ren3dMgrUPtr hw_renderer_manager_;
bstone::Ren3dPtr hw_renderer_ = nullptr;

bstone::HwTextureMgrUPtr hw_texture_manager_;

bstone::Rgba8Palette hw_palette_;
bstone::Rgba8Palette hw_default_palette_;

bstone::Ren3dCmdBufferUPtr hw_common_command_buffer_;
bstone::Ren3dCmdBufferUPtr hw_2d_command_buffer_;
bstone::Ren3dCmdBufferUPtr hw_3d_command_buffer_;

using HwRen3dCmdBuffers = std::array<bstone::Ren3dCmdBufferPtr, 3>;
HwRen3dCmdBuffers hw_command_buffers_;

bstone::Ren3dTexture2dPtr hw_ui_t2d_ = nullptr;
bstone::Ren3dBufferUPtr hw_ui_ib_;
bstone::Ren3dBufferUPtr hw_ui_vb_;
bstone::Ren3dVertexInputUPtr hw_ui_vi_;

bstone::Ren3dTexture2dPtr hw_2d_black_t2d_1x1_ = nullptr;
bstone::Ren3dTexture2dPtr hw_2d_white_t2d_1x1_ = nullptr;

bstone::Ren3dBufferUPtr hw_2d_fillers_ib_;
bstone::Ren3dBufferUPtr hw_2d_fillers_vb_;
bstone::Ren3dVertexInputUPtr hw_2d_fillers_vi_;

bool hw_2d_fade_is_enabled_ = false;
HwVertexColor hw_2d_fade_color_ = bstone::Rgba8{};
bstone::Ren3dTexture2dPtr hw_2d_fade_t2d_ = nullptr;


auto hw_3d_matrix_bs_to_r_ = glm::mat4{};
auto hw_3d_matrix_model_ = glm::mat4{};
auto hw_3d_matrix_view_ = glm::mat4{};
auto hw_3d_matrix_projection_ = glm::mat4{};


bstone::Ren3dBufferUPtr hw_flooring_ib_;
bstone::Ren3dBufferUPtr hw_flooring_vb_;
bstone::Ren3dVertexInputUPtr hw_flooring_vi_;
bstone::Ren3dTexture2dPtr hw_flooring_solid_t2d_ = nullptr;
bstone::Ren3dTexture2dPtr hw_flooring_textured_t2d_ = nullptr;

bstone::Ren3dBufferUPtr hw_ceiling_ib_;
bstone::Ren3dBufferUPtr hw_ceiling_vb_;
bstone::Ren3dVertexInputUPtr hw_ceiling_vi_;
bstone::Ren3dTexture2dPtr hw_ceiling_solid_t2d_ = nullptr;
bstone::Ren3dTexture2dPtr hw_ceiling_textured_t2d_ = nullptr;


auto hw_player_angle_rad_ = double{};
auto hw_view_direction_ = glm::dvec2{};
auto hw_player_position_ = glm::dvec2{};
auto hw_view_position_ = glm::dvec3{};


bool hw_has_active_pushwall_ = false;
int hw_active_pushwall_next_x_ = 0;
int hw_active_pushwall_next_y_ = 0;

int hw_wall_count_ = 0;
int hw_wall_side_count_ = 0;
int hw_wall_vertex_count_ = 0;
int hw_wall_last_xy_to_render_at_ = 0;
HwXyWallMap hw_xy_wall_map_;
HwWallsToRenderList hw_walls_to_render_;

int hw_wall_side_draw_item_count_ = 0;
HwWallSideDrawItems hw_wall_side_draw_items_;

bstone::Ren3dBufferUPtr hw_wall_sides_ib_;
bstone::Ren3dBufferUPtr hw_wall_sides_vb_;
bstone::Ren3dVertexInputUPtr hw_wall_sides_vi_;

HwWallSideIndexBuffer hw_wall_sides_ibi_;


int hw_pushwall_count_ = 0;
int hw_pushwall_side_count_ = 0;
int hw_pushwall_last_xy_to_render_at_ = 0;
HwXyWallMap hw_xy_pushwall_map_;
HwWallsVbi hw_pushwall_to_wall_vbi_;
HwWallsToRenderList hw_pushwalls_to_render_;

int hw_pushwall_side_draw_item_count_ = 0;
HwWallSideDrawItems hw_pushwall_side_draw_items_;

bstone::Ren3dBufferUPtr hw_pushwall_sides_ib_;
bstone::Ren3dBufferUPtr hw_pushwall_sides_vb_;
bstone::Ren3dVertexInputUPtr hw_pushwall_sides_vi_;

HwWallSideIndexBuffer hw_pushwall_sides_ibi_;
HwPushwallsVbi hw_pushwalls_vbi_;


int hw_door_count_ = 0;

HwXyDoorMap hw_xy_door_map_;

int hw_door_draw_item_count_ = 0;
int hw_door_last_xy_to_render_at_ = 0;
HwDoorsToRenderList hw_doors_to_render_;
HwDoorDrawItems hw_door_draw_items_;

bstone::Ren3dBufferUPtr hw_door_sides_ib_;
bstone::Ren3dBufferUPtr hw_door_sides_vb_;
bstone::Ren3dVertexInputUPtr hw_door_sides_vi_;

HwDoorIndexBuffer hw_door_sides_ibi_;
HwDoorsVbi hw_doors_vbi_;


HwSprites hw_statics_;
HwStaticsToRenderList hw_statics_to_render_;

using HwActorsToReposition = std::vector<HwSprite>;
using HwActorsToRenderList = std::unordered_set<int>;
HwSprites hw_actors_;

int hw_sprites_draw_count_ = 0;
HwActorsToRenderList hw_actors_to_render_;
HwSpritesDrawList hw_sprites_draw_list_;

bstone::Ren3dBufferUPtr hw_sprites_ib_;
bstone::Ren3dBufferUPtr hw_sprites_vb_;
bstone::Ren3dVertexInputUPtr hw_sprites_vi_;

HwSpritesIndexBuffer hw_sprites_ibi_;
HwVbi hw_sprites_vbi_;


bstone::Ren3dBufferUPtr hw_player_weapon_ib_;
bstone::Ren3dBufferUPtr hw_player_weapon_vb_;
bstone::Ren3dVertexInputUPtr hw_player_weapon_vi_;
glm::mat4 hw_player_weapon_model_matrix_;
glm::mat4 hw_player_weapon_view_matrix_;
glm::mat4 hw_player_weapon_projection_matrix_;


bool hw_3d_fade_is_enabled_ = false;
bstone::Ren3dBufferUPtr hw_3d_fade_ib_;
bstone::Ren3dBufferUPtr hw_3d_fade_vb_;
bstone::Ren3dVertexInputUPtr hw_3d_fade_vi_;
bstone::Ren3dTexture2dPtr hw_3d_fade_t2d_ = nullptr;

bool hw_fizzle_fx_is_enabled_ = false;
bool hw_fizzle_fx_is_fading_ = false;
int hw_fizzle_fx_color_index_ = 0;
float hw_fizzle_fx_ratio_ = 0.0F;

bstone::Ren3dShaderUPtr hw_fragment_shader_;
bstone::Ren3dShaderUPtr hw_vertex_shader_;
bstone::Ren3dShaderStageUPtr hw_shader_stage_;

bstone::Ren3dShaderMat4VarPtr hw_model_mat_uniform_;
bstone::Ren3dShaderMat4VarPtr hw_view_mat_uniform_;
bstone::Ren3dShaderMat4VarPtr hw_projection_mat_uniform_;

bstone::Ren3dShaderSampler2dVarPtr hw_sampler_uniform_;
bstone::Ren3dShaderInt32VarPtr hw_shading_mode_uniform_;
bstone::Ren3dShaderFloat32VarPtr hw_shade_max_uniform_;
bstone::Ren3dShaderFloat32VarPtr hw_normal_shade_uniform_;
bstone::Ren3dShaderFloat32VarPtr hw_height_numerator_uniform_;
bstone::Ren3dShaderFloat32VarPtr hw_extra_lighting_uniform_;
bstone::Ren3dShaderVec2VarPtr hw_view_direction_uniform_;
bstone::Ren3dShaderVec2VarPtr hw_view_position_uniform_;



void hw_apply_texture_upscale();

void hw_initialize_player_weapon();
void hw_update_player_weapon_model_matrix();


const bstone::Ren3dVec2& hw_cast_glm_vec2(
	const glm::vec2& glm_vec2)
{
	return reinterpret_cast<const bstone::Ren3dVec2&>(glm_vec2);
}

const bstone::Ren3dVec4& hw_cast_glm_vec4(
	const glm::vec4& glm_vec4)
{
	return reinterpret_cast<const bstone::Ren3dVec4&>(glm_vec4);
}

const bstone::Ren3dMat4& hw_cast_glm_mat4(
	const glm::mat4& glm_mat4)
{
	return reinterpret_cast<const bstone::Ren3dMat4&>(glm_mat4);
}

bstone::Ren3dMipmapMode hw_cfg_texture_mipmap_filter_to_renderer(
	const bstone::Ren3dFilterKind filter_kind)
{
	switch (filter_kind)
	{
		case bstone::Ren3dFilterKind::nearest:
			return bstone::Ren3dMipmapMode::nearest;

		case bstone::Ren3dFilterKind::linear:
			return bstone::Ren3dMipmapMode::linear;

		default:
			Quit("Invalid mipmap mode.");
	}
}

int hw_cfg_texture_anisotropy_to_renderer(
	const int value)
{
	if (value < bstone::Ren3dLimits::min_anisotropy_off)
	{
		return bstone::Ren3dLimits::min_anisotropy_off;
	}
	else if (value > bstone::Ren3dLimits::max_anisotropy)
	{
		return bstone::Ren3dLimits::max_anisotropy;
	}
	else
	{
		return value;
	}
}

int hw_get_static_index(
	const statobj_t& bs_static)
{
	return static_cast<int>(&bs_static - statobjlist.data());
}

int hw_get_actor_index(
	const objtype& bs_actor)
{
	return static_cast<int>(&bs_actor - objlist);
}

constexpr int hw_encode_xy(
	const int x,
	const int y)
{
	return (x << 8) | y;
}

HwVertexColor hw_vga_color_to_rgba_8(
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

void hw_destroy_sampler(
	bstone::Ren3dSamplerUPtr& sampler)
{
	sampler = nullptr;
}

void hw_destroy_index_buffer(
	bstone::Ren3dBufferUPtr& index_buffer)
{
	index_buffer = nullptr;
}

bstone::Ren3dBufferUPtr hw_create_index_buffer(
	const bstone::Ren3dBufferUsageKind usage_kind,
	const int byte_depth,
	const int index_count)
{
	const auto index_buffer_size = index_count * byte_depth;

	auto param = bstone::Ren3dCreateBufferParam{};
	param.kind_ = bstone::Ren3dBufferKind::index;
	param.usage_kind_ = usage_kind;
	param.size_ = index_buffer_size;

	return hw_renderer_->create_buffer(param);
}

template<typename TIndex>
void hw_update_index_buffer(
	const bstone::Ren3dBufferUPtr& index_buffer,
	const int index_offset,
	const int index_count,
	const TIndex* const indices)
{
	const auto byte_depth = static_cast<int>(sizeof(TIndex));
	const auto offset = index_offset * byte_depth;
	const auto size = index_count * byte_depth;

	auto param = bstone::Ren3dUpdateBufferParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = indices;

	index_buffer->update(param);
}

void hw_destroy_vertex_buffer(
	bstone::Ren3dBufferUPtr& vertex_buffer)
{
	vertex_buffer = nullptr;
}

template<typename TVertex>
bstone::Ren3dBufferUPtr hw_create_vertex_buffer(
	const bstone::Ren3dBufferUsageKind usage_kind,
	const int vertex_count)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto vertex_buffer_size = vertex_count * vertex_size;

	auto param = bstone::Ren3dCreateBufferParam{};
	param.kind_ = bstone::Ren3dBufferKind::vertex;
	param.usage_kind_ = usage_kind;
	param.size_ = vertex_buffer_size;

	return hw_renderer_->create_buffer(param);
}

template<typename TVertex>
void hw_update_vertex_buffer(
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	const int vertex_offset,
	const int vertex_count,
	const TVertex* const vertices)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto offset = vertex_offset * vertex_size;
	const auto size = vertex_count * vertex_size;

	auto param = bstone::Ren3dUpdateBufferParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = vertices;

	vertex_buffer->update(param);
}

void hw_destroy_vertex_input(
	bstone::Ren3dVertexInputUPtr& vertex_input)
{
	vertex_input = nullptr;
}

template<
	typename TVertex,
	bool TIsExist = false
>
struct HwVertexInputAddAttribDescr
{
	void operator()(
		const int location,
		const bstone::Ren3dVertexAttribFormat format,
		const int offset,
		const int stride,
		const glm::vec4& default_value,
		const bstone::Ren3dBufferUPtr& vertex_buffer,
		bstone::Ren3dVertexAttribDescrs& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = true;
		description.location_ = location;
		description.format_ = bstone::Ren3dVertexAttribFormat::none;
		description.vertex_buffer_ = nullptr;
		description.offset_ = -1;
		description.stride_ = -1;
		description.default_value_ = hw_cast_glm_vec4(default_value);
	}
}; // HwVertexInputAddAttribDescr

template<
	typename TVertex>
struct HwVertexInputAddAttribDescr<TVertex, true>
{
	void operator()(
		const int location,
		const bstone::Ren3dVertexAttribFormat format,
		const int offset,
		const int stride,
		const glm::vec4& default_value,
		const bstone::Ren3dBufferUPtr& vertex_buffer,
		bstone::Ren3dVertexAttribDescrs& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = false;
		description.location_ = location;
		description.format_ = format;
		description.vertex_buffer_ = vertex_buffer.get();
		description.offset_ = offset;
		description.stride_ = stride;
		description.default_value_ = hw_cast_glm_vec4(default_value);
	}
}; // HwVertexInputAddAttribDescr

template<
	typename TVertex,
	HwVertexAttribLocationId TLocationId>
void hw_add_vertex_input_attrib_descr(
	const bstone::Ren3dVertexAttribFormat format,
	const glm::vec4& default_value,
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	bstone::Ren3dVertexAttribDescrs& attribute_descriptions)
{
	const auto traits = HwVertexAttributeTraits<TVertex, TLocationId>{};
	const auto add_attribute = HwVertexInputAddAttribDescr<TVertex, traits.is_valid>{};

	auto location = 0;

	switch (TLocationId)
	{
		case HwVertexAttribLocationId::position:
			location = bstone::HwShaderRegistry::get_a_position_location();
			break;

		case HwVertexAttribLocationId::color:
			location = bstone::HwShaderRegistry::get_a_color_location();
			break;

		case HwVertexAttribLocationId::texture_coordinates:
			location = bstone::HwShaderRegistry::get_a_tx_coords_location();
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
void hw_create_vertex_input(
	const bstone::Ren3dBufferUPtr& index_buffer,
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	bstone::Ren3dVertexInputUPtr& vertex_input)
{
	auto param = bstone::Ren3dCreateVertexInputParam{};
	param.index_buffer_ = index_buffer.get();

	auto& descriptions = param.attrib_descrs_;
	descriptions.reserve(3);

	hw_add_vertex_input_attrib_descr<TVertex, HwVertexAttribLocationId::position>(
		bstone::Ren3dVertexAttribFormat::rgb_32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	hw_add_vertex_input_attrib_descr<TVertex, HwVertexAttribLocationId::color>(
		bstone::Ren3dVertexAttribFormat::rgba_8_unorm,
		glm::vec4{1.0F, 1.0F, 1.0F, 1.0F},
		vertex_buffer,
		descriptions
	);

	hw_add_vertex_input_attrib_descr<TVertex, HwVertexAttribLocationId::texture_coordinates>(
		bstone::Ren3dVertexAttribFormat::rg_32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	vertex_input = hw_renderer_->create_vertex_input(param);
}

void hw_update_player_direction()
{
	hw_player_angle_rad_ = bstone::math::deg_to_rad(player->angle);

	hw_view_direction_.x = std::cos(hw_player_angle_rad_);
	hw_view_direction_.y = -std::sin(hw_player_angle_rad_);

	hw_bs_view_direction_ = hw_view_direction_;
}

void hw_update_player_position()
{
	hw_player_position_.x = player->x;
	hw_player_position_.y = player->y;
}

void hw_update_player_view_position()
{
	const auto focal_delta = glm::dvec2
	{
		hw_view_direction_.x * focallength,
		hw_view_direction_.y * focallength,
	};

	hw_view_position_ = glm::dvec3{hw_player_position_ - focal_delta, 0.5};

	hw_bs_view_position_ = hw_view_position_;
}

void hw_update_player()
{
	hw_update_player_direction();
	hw_update_player_position();
	hw_update_player_view_position();
}

void hw_initialize_ui_buffer()
{
	sw_initialize_ui_buffer();
}

void hw_destroy_shader(
	bstone::Ren3dShaderUPtr& shader)
{
	shader = nullptr;
}

void hw_create_shader(
	const bstone::Ren3dShaderKind kind,
	bstone::Ren3dShaderUPtr& shader)
{
	auto param = bstone::Ren3dCreateShaderParam{};
	param.kind_ = kind;

	const auto renderer_kind = hw_renderer_->get_kind();

	switch (kind)
	{
		case bstone::Ren3dShaderKind::fragment:
			param.source_ = bstone::HwShaderRegistry::get_fragment(renderer_kind);
			break;

		case bstone::Ren3dShaderKind::vertex:
			param.source_ = bstone::HwShaderRegistry::get_vertex(renderer_kind);
			break;

		default:
			Quit("Unsupported shader kind.");
	}

	shader = hw_renderer_->create_shader(param);
}

void hw_destroy_fragment_shader()
{
	hw_destroy_shader(hw_fragment_shader_);
}

void hw_create_fragment_shader()
{
	hw_create_shader(bstone::Ren3dShaderKind::fragment, hw_fragment_shader_);
}

void hw_destroy_vertex_shader()
{
	hw_destroy_shader(hw_vertex_shader_);
}

void hw_create_vertex_shader()
{
	hw_create_shader(bstone::Ren3dShaderKind::vertex, hw_vertex_shader_);
}

void hw_destroy_shader_stage()
{
	hw_shader_stage_ = nullptr;
}

void hw_create_shader_stage()
{
	static const auto input_bindings = bstone::Ren3dShaderStageInputBindings
	{
		{0, bstone::HwShaderRegistry::get_a_position_name()},
		{1, bstone::HwShaderRegistry::get_a_color_name()},
		{2, bstone::HwShaderRegistry::get_a_tx_coords_name()},
	};

	auto param = bstone::Ren3dCreateShaderStageParam{};
	param.fragment_shader_ = hw_fragment_shader_.get();
	param.vertex_shader_ = hw_vertex_shader_.get();
	param.input_bindings_ = input_bindings;

	hw_shader_stage_ = hw_renderer_->create_shader_stage(param);
}

template<typename T>
void hw_uninitialize_uniform(
	T*& var)
{
	var = nullptr;
}

void hw_uninitialize_model_mat_uniform()
{
	hw_uninitialize_uniform(hw_model_mat_uniform_);
}

struct HwInitializeInt32UniformTag{};
struct HwInitializeFloat32UniformTag{};
struct HwInitializeVec2UniformTag{};
struct HwInitializeMat4UniformTag{};
struct HwInitializeSampler2dUniformTag{};

void hw_initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderInt32VarPtr& var,
	const HwInitializeInt32UniformTag)
{
	var = hw_shader_stage_->find_int32_var(name);
}

void hw_initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderFloat32VarPtr& var,
	const HwInitializeFloat32UniformTag)
{
	var = hw_shader_stage_->find_float32_var(name);
}

void hw_initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderVec2VarPtr& var,
	const HwInitializeVec2UniformTag)
{
	var = hw_shader_stage_->find_vec2_var(name);
}

void hw_initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderMat4VarPtr& var,
	const HwInitializeMat4UniformTag)
{
	var = hw_shader_stage_->find_mat4_var(name);
}

void hw_initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderSampler2dVarPtr& var,
	const HwInitializeSampler2dUniformTag)
{
	var = hw_shader_stage_->find_sampler_2d_var(name);
}

template<typename T>
void hw_initialize_uniform(
	const std::string& name,
	T*& var)
{
	using Tag = std::conditional_t<
		std::is_same<T, bstone::Ren3dShaderVarInt32>::value,
		HwInitializeInt32UniformTag,
		std::conditional_t<
			std::is_same<T, bstone::Ren3dShaderVarFloat32>::value,
			HwInitializeFloat32UniformTag,
			std::conditional_t<
				std::is_same<T, bstone::Ren3dShaderVarVec2>::value,
				HwInitializeVec2UniformTag,
				std::conditional_t<
					std::is_same<T, bstone::Ren3dShaderVarMat4>::value,
					HwInitializeMat4UniformTag,
					std::conditional_t<
						std::is_same<T, bstone::Ren3dShaderVarSampler2d>::value,
						HwInitializeSampler2dUniformTag,
						void
					>
				>
			>
		>
	>;

	static_assert(!std::is_same<Tag, void>::value, "Unsupported type.");

	hw_initialize_uniform(name, var, Tag{});

	if (!var)
	{
		vid_hw_log_error_and_throw("Shader variable \"" + name + "\" not found.");
	}
}

void hw_initialize_model_mat_uniform()
{
	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_model_mat_name(),
		hw_model_mat_uniform_
	);
}

void hw_uninitialize_view_mat_uniform()
{
	hw_uninitialize_uniform(hw_view_mat_uniform_);
}

void hw_initialize_view_mat_uniform()
{
	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_mat_name(),
		hw_view_mat_uniform_
	);
}

void hw_uninitialize_projection_mat_uniform()
{
	hw_uninitialize_uniform(hw_projection_mat_uniform_);
}

void hw_initialize_projection_mat_uniform()
{
	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_projection_mat_name(),
		hw_projection_mat_uniform_
	);
}

void hw_uninitialize_sampler_uniform()
{
	hw_uninitialize_uniform(hw_sampler_uniform_);
}

void hw_initialize_sampler_uniform()
{
	hw_sampler_var_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_sampler_name(),
		hw_sampler_uniform_
	);
}

void hw_uninitialize_shading_mode_uniform()
{
	hw_uninitialize_uniform(hw_shading_mode_uniform_);
}

void hw_initialize_shading_mode_uniform()
{
	hw_shading_mode_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_shading_mode_name(),
		hw_shading_mode_uniform_
	);
}

void hw_uninitialize_shade_max_uniform()
{
	hw_uninitialize_uniform(hw_shade_max_uniform_);
}

void hw_initialize_shade_max_uniform()
{
	hw_bs_shade_max_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_shade_max_name(),
		hw_shade_max_uniform_
	);
}

void hw_uninitialize_normal_shade_uniform()
{
	hw_uninitialize_uniform(hw_normal_shade_uniform_);
}

void hw_initialize_normal_shade_uniform()
{
	hw_bs_normal_shade_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_normal_shade_name(),
		hw_normal_shade_uniform_
	);
}

void hw_uninitialize_height_numerator_uniform()
{
	hw_uninitialize_uniform(hw_height_numerator_uniform_);
}

void hw_initialize_height_numerator_uniform()
{
	hw_bs_height_numerator_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_height_numerator_name(),
		hw_height_numerator_uniform_
	);
}

void hw_uninitialize_extra_lighting_uniform()
{
	hw_uninitialize_uniform(hw_extra_lighting_uniform_);
}

void hw_initialize_extra_lighting_uniform()
{
	hw_bs_lighting_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_extra_lighting_name(),
		hw_extra_lighting_uniform_
	);
}

void hw_uninitialize_view_direction_uniform()
{
	hw_uninitialize_uniform(hw_view_direction_uniform_);
}

void hw_initialize_view_direction_uniform()
{
	hw_bs_view_direction_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_direction_name(),
		hw_view_direction_uniform_
	);
}

void hw_uninitialize_view_position_uniform()
{
	hw_uninitialize_uniform(hw_view_position_uniform_);
}

void hw_initialize_view_position_uniform()
{
	hw_bs_view_position_.set_is_modified(true);

	hw_initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_position_name(),
		hw_view_position_uniform_
	);
}

void hw_uninitialize_shading_uniforms()
{
	hw_uninitialize_sampler_uniform();
	hw_uninitialize_shading_mode_uniform();
	hw_uninitialize_shade_max_uniform();
	hw_uninitialize_normal_shade_uniform();
	hw_uninitialize_height_numerator_uniform();
	hw_uninitialize_extra_lighting_uniform();
	hw_uninitialize_view_direction_uniform();
	hw_uninitialize_view_position_uniform();
}

void hw_initialize_shading_uniforms()
{
	hw_initialize_sampler_uniform();
	hw_initialize_shading_mode_uniform();
	hw_initialize_shade_max_uniform();
	hw_initialize_normal_shade_uniform();
	hw_initialize_height_numerator_uniform();
	hw_initialize_extra_lighting_uniform();
	hw_initialize_view_direction_uniform();
	hw_initialize_view_position_uniform();
}

void hw_uninitialize_uniforms()
{
	hw_uninitialize_model_mat_uniform();
	hw_uninitialize_view_mat_uniform();
	hw_uninitialize_projection_mat_uniform();
	hw_uninitialize_shading_uniforms();
}

void hw_initialize_uniforms()
{
	vid_log("Initializing shader variables.");

	hw_initialize_model_mat_uniform();
	hw_initialize_view_mat_uniform();
	hw_initialize_projection_mat_uniform();
	hw_initialize_shading_uniforms();
}

void hw_uninitialize_program()
{
	hw_uninitialize_uniforms();
	hw_destroy_shader_stage();
	hw_destroy_fragment_shader();
	hw_destroy_vertex_shader();
}

void hw_initialize_program()
{
	vid_log();
	vid_log("Initializing shader program.");

	hw_create_fragment_shader();
	hw_create_vertex_shader();
	hw_create_shader_stage();
	hw_initialize_uniforms();
}

bstone::Ren3dKind hw_get_renderer_kind(
	const bstone::RendererKind renderer_kind)
{
	switch (renderer_kind)
	{
		case bstone::RendererKind::gl_2_0:
			return bstone::Ren3dKind::gl_2_0;

		case bstone::RendererKind::gl_3_2_core:
			return bstone::Ren3dKind::gl_3_2_core;

		case bstone::RendererKind::gles_2_0:
			return bstone::Ren3dKind::gles_2_0;

		default:
			throw bstone::Exception{"Unsupported renderer kind."};
	}
}

void hw_log_device_features()
{
	const auto& device_features = hw_renderer_->get_device_features();

	vid_log();
	vid_log("Device features");
	vid_log("===============");
	vid_log("V-Sync: " + vid_to_string(device_features.is_vsync_available_));
	vid_log("V-Sync requires restart: " + vid_to_string(device_features.is_vsync_requires_restart_));
	vid_log("Texture max dimension: " + vid_to_string(device_features.max_texture_dimension_));
	vid_log("Viewport max width: " + vid_to_string(device_features.max_viewport_width_));
	vid_log("Viewport max height: " + vid_to_string(device_features.max_viewport_height_));
	vid_log("Anisotropy: " + vid_to_string(device_features.is_anisotropy_available_));
	vid_log("Anisotropy max degree: " + vid_to_string(device_features.max_anisotropy_degree_));
	vid_log("Non-power-of-two textures: " + vid_to_string(device_features.is_npot_available_));
	vid_log("Mipmap auto-generation: " + vid_to_string(device_features.is_mipmap_available_));
	vid_log("Samplers: " + vid_to_string(device_features.is_mipmap_available_));
	vid_log("MSAA: " + vid_to_string(device_features.is_msaa_available_));
	vid_log("MSAA (render-to-window): " + vid_to_string(device_features.is_msaa_render_to_window_));
	vid_log("MSAA requires restart: " + vid_to_string(device_features.is_msaa_requires_restart_));
	vid_log("MSAA max degree: " + vid_to_string(device_features.max_msaa_degree_));
	vid_log("Vertex input max locations: " + vid_to_string(device_features.max_vertex_input_locations_));
}

void hw_initialize_renderer()
{
	vid_log();
	vid_log("Initializing 3D renderer.");


	const auto title = vid_get_game_name_and_game_version_string();


	// Initialization parameter.
	//
	auto param = bstone::Ren3dCreateParam{};

	param.aa_kind_ = vid_cfg_.aa_kind_;
	param.aa_value_ = vid_cfg_.aa_degree_;

	param.is_vsync_ = vid_cfg_.is_vsync_;

#ifdef __vita__
	param.window_.is_visible_ = true;
#endif // __vita__

	param.window_.rect_2d_.extent_.width_ = vid_dimensions_.window_width_;
	param.window_.rect_2d_.extent_.height_ = vid_dimensions_.window_height_;

	if (!vid_cfg_.is_windowed_)
	{
		param.window_.is_borderless_ = true;
		param.window_.is_fake_fullscreen_ = true;
	}

	param.window_.is_positioned_ = vid_cfg_.is_positioned_;
	param.window_.rect_2d_.offset_.x_ = vid_cfg_.windowed_x_;
	param.window_.rect_2d_.offset_.y_ = vid_cfg_.windowed_y_;

	param.window_.title_ = title;

	using RendererKindList = std::vector<bstone::Ren3dKind>;

	auto renderer_kind_list = RendererKindList{};

	const auto is_auto_detect = (vid_cfg_.renderer_kind_ == bstone::RendererKind::auto_detect);

	if (is_auto_detect)
	{
#ifndef BSTONE_REN_3D_TEST_NO_GL
		renderer_kind_list =
		{
#ifndef BSTONE_REN_3D_TEST_NO_GL_3_2_C
			bstone::Ren3dKind::gl_3_2_core,
#endif // !BSTONE_REN_3D_TEST_NO_GL_3_2_C

#ifndef BSTONE_REN_3D_TEST_NO_GL_2_0
			bstone::Ren3dKind::gl_2_0,
#endif // !BSTONE_REN_3D_TEST_NO_GL_2_0

#ifndef BSTONE_REN_3D_TEST_NO_GLES_2_0
			bstone::Ren3dKind::gles_2_0,
#endif // !BSTONE_REN_3D_TEST_NO_GLES_2_0
		};
#endif // BSTONE_REN_3D_TEST_NO_GL
	}
	else
	{
		renderer_kind_list = {hw_get_renderer_kind(vid_cfg_.renderer_kind_)};
	}

	for (const auto renderer_kind : renderer_kind_list)
	{
		param.renderer_kind_ = renderer_kind;

		vid_log("Trying to initialize \"" + vid_to_string(renderer_kind) + "\".");

		try
		{
			hw_renderer_ = hw_renderer_manager_->renderer_initialize(param);

			return;
		}
		catch (const bstone::Exception& ex)
		{
			vid_log_error(ex.what());
		}
	}

	throw bstone::Exception{"Not found any 3D renderer."};
}

void hw_destroy_ui_ib()
{
	hw_destroy_index_buffer(hw_ui_ib_);
}

void hw_create_ui_ib()
{
	vid_log("Creating UI index buffer.");

	hw_ui_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		hw_2d_index_count_
	);


	using Indices = std::array<std::uint8_t, hw_2d_index_count_>;

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

	hw_update_index_buffer(
		hw_ui_ib_,
		0,
		hw_2d_index_count_,
		indices.data());
}

void hw_destroy_ui_vi()
{
	hw_destroy_vertex_input(hw_ui_vi_);
}

void hw_create_ui_vi()
{
	vid_log("Creating UI vertex input.");

	hw_create_vertex_input<HwVertex>(
		hw_ui_ib_,
		hw_ui_vb_,
		hw_ui_vi_);
}

void hw_fill_2d_vb(
	const float left_f,
	const float right_f,
	const float width_f,
	const int vertex_offset)
{
	static_cast<void>(width_f);

	auto vertex_index = vertex_offset;
	auto& vertices = hw_2d_vertices_;

	const auto height_f = static_cast<float>(vid_dimensions_.screen_height_);

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_f, 0.0F, 0.0F};
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_f, 0.0F, 0.0F};
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Upper right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_f, height_f, 0.0F};
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Upper left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_f, height_f, 0.0F};
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}
}

void hw_fill_2d_vb_stretched()
{
	const auto left_f = 0.0F;
	const auto right_f = static_cast<float>(vid_dimensions_.screen_width_);
	const auto width_f = static_cast<float>(vid_dimensions_.screen_width_);

	hw_fill_2d_vb(left_f, right_f, width_f, hw_2d_stretched_vertex_offset_);
}

void hw_fill_2d_vb_non_stretched()
{
	const auto left_f = static_cast<float>(vid_dimensions_.screen_left_filler_width_);
	const auto right_f = static_cast<float>(vid_dimensions_.screen_width_ - vid_dimensions_.screen_right_filler_width_);
	const auto width_f = static_cast<float>(vid_dimensions_.screen_width_4x3_);

	hw_fill_2d_vb(left_f, right_f, width_f, hw_2d_non_stretched_vertex_offset_);
}

void hw_destroy_ui_vb()
{
	hw_destroy_vertex_buffer(hw_ui_vb_);
}

void hw_create_ui_vb()
{
	vid_log("Creating UI vertex buffer.");

	hw_ui_vb_ = hw_create_vertex_buffer<HwVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		hw_2d_vertex_count_
	);

	hw_fill_2d_vb_stretched();
	hw_fill_2d_vb_non_stretched();

	hw_update_vertex_buffer(
		hw_ui_vb_,
		0,
		hw_2d_vertex_count_,
		hw_2d_vertices_.data()
	);
}

void hw_destroy_2d_fillers_ib()
{
	hw_destroy_index_buffer(hw_2d_fillers_ib_);
}

void hw_create_2d_fillers_ib()
{
	vid_log("Creating 2D filler index buffer.");

	hw_2d_fillers_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		hw_2d_fillers_index_count_
	);

	using Indices = std::array<std::uint8_t, hw_2d_fillers_index_count_>;

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

	hw_update_index_buffer(
		hw_2d_fillers_ib_,
		0,
		hw_2d_fillers_index_count_,
		indices.data());
}

void hw_destroy_2d_fillers_vb()
{
	hw_destroy_vertex_buffer(hw_2d_fillers_vb_);
}

void hw_create_2d_fillers_vb()
{
	vid_log("Creating 2D filler vertex buffer.");

	hw_2d_fillers_vb_ = hw_create_vertex_buffer<HwVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		hw_2d_fillers_vertex_count_
	);

	const auto& filler_color = hw_vga_color_to_rgba_8(
		vgapal[(vid_cfg_.filler_color_index * 3) + 0],
		vgapal[(vid_cfg_.filler_color_index * 3) + 1],
		vgapal[(vid_cfg_.filler_color_index * 3) + 2]
	);

	const auto left_left_f = static_cast<float>(0.0F);
	const auto left_right_f = static_cast<float>(vid_dimensions_.screen_left_filler_width_);

	const auto right_left_f = static_cast<float>(vid_dimensions_.screen_width_ - vid_dimensions_.screen_right_filler_width_);
	const auto right_right_f = static_cast<float>(vid_dimensions_.screen_width_);

	const auto top_top_f = static_cast<float>(vid_dimensions_.screen_height_);
	const auto top_bottom_f = static_cast<float>(vid_dimensions_.screen_height_ - vid_dimensions_.screen_top_filler_height_);

	const auto bottom_top_f = static_cast<float>(vid_dimensions_.screen_bottom_filler_height_);
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, bottom_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, bottom_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, bottom_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, bottom_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
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
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_bottom_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz_ = HwVertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba_8_ = filler_color;
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Top left.
	// ----------------------------------------------------------------------

	//
	// HUD fillers.
	// ======================================================================


	hw_update_vertex_buffer(
		hw_2d_fillers_vb_,
		0,
		hw_2d_fillers_vertex_count_,
		vertices.data()
	);
}

void hw_destroy_2d_fillers_vi()
{
	hw_destroy_vertex_input(hw_2d_fillers_vi_);
}

void hw_create_2d_fillers_vi()
{
	vid_log("Creating 2D filler vertex input.");

	hw_create_vertex_input<HwVertex>(
		hw_2d_fillers_ib_,
		hw_2d_fillers_vb_,
		hw_2d_fillers_vi_
	);
}

void hw_destroy_black_1x1_2d_texture()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
	}

	hw_2d_black_t2d_1x1_ = nullptr;
}

void hw_create_black_1x1_2d_texture()
{
	vid_log("Creating 2D 1x1 black texture.");

	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
	hw_2d_black_t2d_1x1_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
}

void hw_destroy_white_1x1_2d_texture()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
	}

	hw_2d_white_t2d_1x1_ = nullptr;
}

void hw_create_white_1x1_2d_texture()
{
	vid_log("Creating 2D 1x1 white texture.");

	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
	hw_2d_white_t2d_1x1_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
}

void hw_destroy_fade_1x1_2d_texture()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
	}

	hw_2d_fade_t2d_ = nullptr;
}

void hw_create_fade_1x1_2d_texture()
{
	vid_log("Creating 2D 1x1 fade texture.");

	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
	hw_2d_fade_t2d_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
}

void hw_destroy_ui_texture()
{
	if (hw_ui_t2d_ == nullptr)
	{
		return;
	}

	hw_texture_manager_->destroy_ui();
	hw_ui_t2d_ = nullptr;
}

void hw_create_ui_texture()
{
	vid_log("Creating UI texture.");

	hw_texture_manager_->create_ui(vid_ui_buffer_.data(), vid_mask_buffer_.data(), &hw_palette_);
	hw_ui_t2d_ = hw_texture_manager_->get_ui();
}

void hw_uninitialize_2d()
{
	hw_destroy_ui_texture();

	hw_destroy_ui_vi();
	hw_destroy_ui_ib();
	hw_destroy_ui_vb();

	hw_destroy_2d_fillers_vi();
	hw_destroy_2d_fillers_ib();
	hw_destroy_2d_fillers_vb();

	hw_destroy_black_1x1_2d_texture();
	hw_destroy_white_1x1_2d_texture();
	hw_destroy_fade_1x1_2d_texture();
}

void hw_initialize_2d()
{
	vid_log();
	vid_log("Initializing 2D resources.");

	hw_create_ui_ib();
	hw_create_ui_vb();
	hw_create_ui_vi();

	hw_create_2d_fillers_ib();
	hw_create_2d_fillers_vb();
	hw_create_2d_fillers_vi();

	hw_create_ui_texture();
	hw_create_black_1x1_2d_texture();
	hw_create_white_1x1_2d_texture();
	hw_create_fade_1x1_2d_texture();
}

void hw_destroy_flooring_ib()
{
	hw_destroy_index_buffer(hw_flooring_ib_);
}

void hw_create_flooring_ib()
{
	vid_log("Creating 3D flooring index buffer.");

	const auto index_count = 6;

	{
		hw_flooring_ib_ = hw_create_index_buffer(
			bstone::Ren3dBufferUsageKind::draw_static,
			1,
			index_count
		);
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 1, 2,
			0, 2, 3,
		};

		hw_update_index_buffer(
			hw_flooring_ib_,
			0,
			index_count,
			indices.data()
		);
	}
}

void hw_destroy_flooring_vb()
{
	hw_destroy_vertex_buffer(hw_flooring_vb_);
}

void hw_create_flooring_vb()
{
	vid_log("Creating 3D flooring vertex buffer.");

	const auto vertex_count = 4;

	{
		hw_flooring_vb_ = hw_create_vertex_buffer<HwFlooringVertex>(
			bstone::Ren3dBufferUsageKind::draw_static,
			vertex_count
		);
	}

	{
		const auto map_dimension_f = static_cast<float>(MAPSIZE);

		using Vertices = std::array<HwFlooringVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, 0.0F, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, map_dimension_f};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, map_dimension_f, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{map_dimension_f, map_dimension_f, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{map_dimension_f, 0.0F};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{map_dimension_f, 0.0F, 0.0F};
			vertex.uv_ = HwVertexTextureCoordinates{map_dimension_f, map_dimension_f};
		}

		hw_update_vertex_buffer(
			hw_flooring_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}
}

void hw_destroy_flooring_vi()
{
	hw_destroy_vertex_input(hw_flooring_vi_);
}

void hw_create_flooring_vi()
{
	vid_log("Creating 3D flooring vertex input.");

	hw_create_vertex_input<HwFlooringVertex>(
		hw_flooring_ib_,
		hw_flooring_vb_,
		hw_flooring_vi_);
}

void hw_destroy_solid_flooring_texture_2d()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
	}

	hw_flooring_solid_t2d_ = nullptr;
}

void hw_create_solid_flooring_texture_2d()
{
	vid_log("Creating 3D flooring 1x1 solid texture.");

	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
	hw_flooring_solid_t2d_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
}

void hw_uninitialize_flooring()
{
	hw_destroy_flooring_vi();
	hw_destroy_flooring_ib();
	hw_destroy_flooring_vb();

	hw_destroy_solid_flooring_texture_2d();

	// Managed by texture manager. No need to destroy.
	hw_flooring_textured_t2d_ = nullptr;
}

void hw_initialize_flooring()
{
	vid_log();
	vid_log("Initializing 3D flooring.");

	hw_create_flooring_ib();
	hw_create_flooring_vb();
	hw_create_flooring_vi();
	hw_create_solid_flooring_texture_2d();
}

void hw_destroy_ceiling_ib()
{
	hw_destroy_index_buffer(hw_ceiling_ib_);
}

void hw_create_ceiling_ib()
{
	vid_log("Creating 3D ceiling index buffer.");

	const auto index_count = 6;

	{
		hw_ceiling_ib_ = hw_create_index_buffer(
			bstone::Ren3dBufferUsageKind::draw_static,
			1,
			index_count
		);
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 2, 1,
			0, 3, 2,
		};

		hw_update_index_buffer(
			hw_ceiling_ib_,
			0,
			index_count,
			indices.data()
		);
	}
}

void hw_destroy_ceiling_vb()
{
	hw_destroy_vertex_buffer(hw_ceiling_vb_);
}

void hw_create_ceiling_vb()
{
	vid_log("Creating 3D ceiling vertex buffer.");

	const auto vertex_count = 4;

	{
		hw_ceiling_vb_ = hw_create_vertex_buffer<HwCeilingVertex>(
			bstone::Ren3dBufferUsageKind::draw_static,
			vertex_count
		);
	}

	{
		using Vertices = std::array<HwCeilingVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, 0.0F, hw_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, hw_map_dimension_f};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{0.0F, hw_map_dimension_f, hw_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{hw_map_dimension_f, hw_map_dimension_f, hw_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{hw_map_dimension_f, 0.0F};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz_ = HwVertexPosition{hw_map_dimension_f, 0.0F, hw_map_height_f};
			vertex.uv_ = HwVertexTextureCoordinates{hw_map_dimension_f, hw_map_dimension_f};
		}

		hw_update_vertex_buffer(
			hw_ceiling_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}
}

void hw_destroy_ceiling_vi()
{
	hw_destroy_vertex_input(hw_ceiling_vi_);
}

void hw_create_ceiling_vi()
{
	vid_log("Creating 3D ceiling vertex input.");

	hw_create_vertex_input<HwCeilingVertex>(
		hw_ceiling_ib_,
		hw_ceiling_vb_,
		hw_ceiling_vi_);
}

void hw_destroy_solid_ceiling_texture_2d()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
	}

	hw_ceiling_solid_t2d_ = nullptr;
}

void hw_create_solid_ceiling_texture_2d()
{
	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
	hw_ceiling_solid_t2d_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
}

void hw_initialize_ceiling()
{
	vid_log();
	vid_log("Initializing 3D ceiling.");

	hw_create_ceiling_ib();
	hw_create_ceiling_vb();
	hw_create_ceiling_vi();
	hw_create_solid_ceiling_texture_2d();
}

void hw_uninitialize_ceiling()
{
	hw_destroy_ceiling_vi();
	hw_destroy_ceiling_ib();
	hw_destroy_ceiling_vb();

	hw_destroy_solid_ceiling_texture_2d();

	// Managed by texture manager. No need to destroy.
	hw_ceiling_textured_t2d_ = nullptr;
}

void hw_create_walls_ib()
{
	vid_log("Creating wall index buffer.");

	const auto index_count = hw_wall_side_count_ * hw_indices_per_wall_side;

	hw_wall_sides_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);

	hw_wall_sides_ibi_.clear();
	hw_wall_sides_ibi_.resize(index_count);
}

void hw_destroy_walls_ib()
{
	hw_destroy_index_buffer(hw_wall_sides_ib_);
	hw_wall_sides_ibi_.clear();
}

void hw_create_walls_vb()
{
	vid_log("Creating wall vertex buffer.");

	const auto vertex_count = hw_wall_side_count_ * hw_vertices_per_wall_side;

	hw_wall_sides_vb_ = hw_create_vertex_buffer<HwWallVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		vertex_count
	);
}

void hw_destroy_walls_vi()
{
	hw_destroy_vertex_input(hw_wall_sides_vi_);
}

void hw_create_walls_vi()
{
	vid_log("Creating wall vertex input.");

	hw_create_vertex_input<HwWallVertex>(
		hw_wall_sides_ib_,
		hw_wall_sides_vb_,
		hw_wall_sides_vi_);
}

void hw_destroy_walls_vb()
{
	hw_destroy_vertex_buffer(hw_wall_sides_vb_);
}

void hw_initialize_walls()
{
	vid_log();
	vid_log("Initializing walls.");

	hw_xy_wall_map_.reserve(hw_wall_count_);

	hw_walls_to_render_.clear();

	hw_wall_side_draw_item_count_ = 0;
	hw_wall_side_draw_items_.clear();
	hw_wall_side_draw_items_.resize(hw_wall_side_count_);

	hw_create_walls_ib();
	hw_create_walls_vb();
	hw_create_walls_vi();
}

void hw_uninitialize_walls()
{
	hw_wall_count_ = 0;
	hw_wall_side_count_ = 0;
	hw_xy_wall_map_.clear();

	hw_wall_side_draw_item_count_ = 0;
	hw_wall_side_draw_items_.clear();

	hw_destroy_walls_ib();
	hw_destroy_walls_vb();
	hw_destroy_walls_vi();
}

void hw_create_pushwalls_ibi()
{
	const auto index_count = hw_pushwall_side_count_ * hw_indices_per_wall_side;

	hw_pushwall_sides_ibi_.clear();
	hw_pushwall_sides_ibi_.resize(index_count);
}

void hw_create_pushwalls_ib()
{
	vid_log("Creating pushwall index buffer.");

	const auto index_count = hw_pushwall_side_count_ * hw_indices_per_wall_side;

	hw_pushwall_sides_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);
}

void hw_destroy_pushwalls_ibi()
{
	hw_pushwall_sides_ibi_.clear();
}

void hw_destroy_pushwalls_ib()
{
	hw_destroy_index_buffer(hw_pushwall_sides_ib_);
}

void hw_create_pushwalls_vb()
{
	vid_log("Creating pushwall vertex buffer.");

	const auto vertex_count = hw_pushwall_side_count_ * hw_vertices_per_wall_side;

	hw_pushwall_sides_vb_ = hw_create_vertex_buffer<HwPushwallVertex>(
		bstone::Ren3dBufferUsageKind::draw_dynamic,
		vertex_count
	);
}

void hw_destroy_pushwalls_vi()
{
	hw_destroy_vertex_input(hw_pushwall_sides_vi_);
}

void hw_create_pushwalls_vi()
{
	vid_log("Creating pushwall vertex input.");

	hw_create_vertex_input<HwPushwallVertex>(
		hw_pushwall_sides_ib_,
		hw_pushwall_sides_vb_,
		hw_pushwall_sides_vi_);
}

void hw_destroy_pushwalls_vb()
{
	hw_destroy_vertex_buffer(hw_pushwall_sides_vb_);
}

void hw_initialize_pushwalls()
{
	vid_log();
	vid_log("Initializing pushwalls.");

	hw_xy_pushwall_map_.reserve(hw_pushwall_count_);

	hw_pushwall_to_wall_vbi_.clear();
	hw_pushwall_to_wall_vbi_.resize(hw_sides_per_wall * hw_vertices_per_wall_side);

	hw_pushwalls_to_render_.clear();

	hw_pushwall_side_draw_item_count_ = 0;
	hw_pushwall_side_draw_items_.clear();
	hw_pushwall_side_draw_items_.resize(hw_pushwall_side_count_);

	hw_create_pushwalls_ibi();
	hw_create_pushwalls_ib();
	hw_create_pushwalls_vb();
	hw_create_pushwalls_vi();
}

void hw_uninitialize_pushwalls()
{
	hw_pushwall_count_ = 0;
	hw_pushwall_side_count_ = 0;
	hw_xy_pushwall_map_.clear();

	hw_pushwall_side_draw_item_count_ = 0;
	hw_pushwall_side_draw_items_.clear();

	hw_destroy_pushwalls_vi();

	hw_destroy_pushwalls_ibi();
	hw_destroy_pushwalls_ib();

	hw_destroy_pushwalls_vb();
}

void hw_create_door_sides_ibi()
{
	const auto index_count = hw_door_count_ * hw_indices_per_door_side;

	hw_door_sides_ibi_.clear();
	hw_door_sides_ibi_.resize(index_count);
}

void hw_create_door_sides_ib()
{
	vid_log("Creating door index buffer.");

	const auto index_count = hw_door_count_ * hw_indices_per_door_side;

	hw_door_sides_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);
}

void hw_destroy_door_sides_ibi()
{
	hw_door_sides_ibi_.clear();
}

void hw_destroy_door_sides_ib()
{
	hw_destroy_index_buffer(hw_door_sides_ib_);

	hw_door_sides_ibi_.clear();
}

void hw_create_door_sides_vb()
{
	vid_log("Creating door vertex buffer.");

	const auto vertex_count = hw_door_count_ * hw_indices_per_door_side;

	hw_door_sides_vb_ = hw_create_vertex_buffer<HwDoorVertex>(
		bstone::Ren3dBufferUsageKind::draw_dynamic,
		vertex_count
	);
}

void hw_destroy_door_sides_vb()
{
	hw_destroy_vertex_buffer(hw_door_sides_vb_);
}

void hw_destroy_door_sides_vi()
{
	hw_destroy_vertex_input(hw_door_sides_vi_);
}

void hw_create_door_sides_vi()
{
	vid_log("Creating door vertex input.");

	hw_create_vertex_input<HwDoorVertex>(
		hw_door_sides_ib_,
		hw_door_sides_vb_,
		hw_door_sides_vi_);
}

void hw_initialize_door_sides()
{
	vid_log();
	vid_log("Initializing doors.");

	hw_xy_door_map_.reserve(hw_door_count_);

	const auto max_draw_item_count = hw_door_count_ * hw_door_halves_per_door;

	hw_door_draw_item_count_ = 0;
	hw_door_draw_items_.clear();
	hw_door_draw_items_.resize(max_draw_item_count);

	hw_create_door_sides_ibi();
	hw_create_door_sides_ib();
	hw_create_door_sides_vb();
	hw_create_door_sides_vi();
}

void hw_uninitialize_door_sides()
{
	hw_xy_door_map_.clear();

	hw_door_draw_item_count_ = 0;
	hw_door_draw_items_.clear();

	hw_destroy_door_sides_vi();

	hw_destroy_door_sides_ibi();
	hw_destroy_door_sides_ib();

	hw_destroy_door_sides_vb();
}

void hw_update_palette(
	const int first_index,
	const int color_count)
{
	for (int i = 0; i < color_count; ++i)
	{
		const auto color_index = first_index + i;
		const auto& vga_color = vid_vga_palette_[color_index];
		auto& hw_color = hw_palette_[color_index];

		hw_color = hw_vga_color_to_rgba_8(
			vga_color.r_,
			vga_color.g_,
			vga_color.b_
		);
	}
}

void hw_initialize_palette()
{
	hw_palette_ = {};

	hw_update_palette(0, bstone::RgbPalette::get_max_color_count());

	hw_default_palette_ = bstone::Rgba8Palette{};

	for (int i = 0; i < bstone::RgbPalette::get_max_color_count(); ++i)
	{
		const auto vga_color = vgapal + (i * 3);
		auto& hw_color = hw_default_palette_[i];

		hw_color = hw_vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);
	}
}

void hw_calculate_dimensions()
{
	auto src_param = vid_create_screen_size_param();

	vid_calculate_window_elements_dimensions(src_param, vid_dimensions_);

	vid_calculate_vga_dimensions();
}

void hw_build_2d_model_matrix()
{
	hw_2d_matrix_model_ = glm::identity<glm::mat4>();
}

void hw_build_2d_view_matrix()
{
	hw_2d_matrix_view_ = glm::identity<glm::mat4>();
}

void hw_build_2d_projection_matrix()
{
	hw_2d_matrix_projection_ = glm::orthoRH_NO(
		0.0, // left
		static_cast<double>(vid_dimensions_.screen_width_), // right
		0.0, // bottom
		static_cast<double>(vid_dimensions_.screen_height_), // top
		0.0, // zNear
		1.0 // zFar
	);
}

void hw_build_2d_matrices()
{
	hw_build_2d_model_matrix();
	hw_build_2d_view_matrix();
	hw_build_2d_projection_matrix();
}

void hw_calculate_camera_parameters()
{
	// Vertical FOV.
	//
	// V = 2 * arctan(tan(H / 2) * (h / w))
	//

	const auto ref_r_ratio = static_cast<double>(vga_ref_height_4x3) / static_cast<double>(vga_ref_width);

	const auto half_hfov_deg = hw_ref_camera_hfov_deg / 2.0;
	const auto half_hfov_rad = bstone::math::deg_to_rad(half_hfov_deg);
	const auto tan_half_hfov_rad = std::tan(half_hfov_rad);
	const auto half_vfov_rad = tan_half_hfov_rad * ref_r_ratio;

	// Radians.
	const auto vfov_rad = 2.0 * half_vfov_rad;
	hw_camera_vfov_rad = vfov_rad;

	// Degrees.
	const auto half_vfov_deg = bstone::math::rad_to_deg(half_vfov_rad);
	const auto vfov_deg = 2.0 * half_vfov_deg;
	hw_camera_vfov_deg = vfov_deg;


	// Distances to the planes.
	//
	hw_camera_near_distance = 0.05;
	hw_camera_far_distance = (std::sqrt(2.0) * hw_map_dimension_d) + 0.5;
}

void hw_build_bs_to_ren_matrix()
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
	const auto m_23 = static_cast<float>(height_compensation_factor);
	const auto m_24 = 0.0F;

	const auto m_31 = -1.0F;
	const auto m_32 = 0.0F;
	const auto m_33 = 0.0F;
	const auto m_34 = 0.0F;

	const auto m_41 = 0.0F;
	const auto m_42 = 0.0F;
	const auto m_43 = 0.0F;
	const auto m_44 = 1.0F;

	hw_3d_matrix_bs_to_r_ = glm::mat4
	{
		m_11, m_21, m_31, m_41,
		m_12, m_22, m_32, m_42,
		m_13, m_23, m_33, m_43,
		m_14, m_24, m_34, m_44,
	};
}

void hw_build_model_matrix()
{
	hw_3d_matrix_model_ = glm::identity<glm::mat4>();
}

void hw_build_view_matrix()
{
	if (!player)
	{
		hw_3d_matrix_view_ = glm::identity<glm::mat4>();

		return;
	}

	auto view_matrix = glm::identity<glm::dmat4>();

	view_matrix = glm::rotate(view_matrix, hw_player_angle_rad_, glm::dvec3{0.0, 0.0, 1.0});
	view_matrix = glm::translate(view_matrix, -hw_view_position_);

	hw_3d_matrix_view_ = view_matrix;
}

void hw_build_projection_matrix()
{
	const auto perspective = glm::perspectiveFovRH_NO(
		static_cast<float>(hw_camera_vfov_rad),
		static_cast<float>(vid_dimensions_.screen_viewport_width_),
		static_cast<float>(vid_dimensions_.screen_viewport_height_),
		static_cast<float>(hw_camera_near_distance),
		static_cast<float>(hw_camera_far_distance)
	);

	hw_3d_matrix_projection_ = perspective * hw_3d_matrix_bs_to_r_;
}

void hw_build_3d_matrices()
{
	hw_calculate_camera_parameters();

	hw_build_bs_to_ren_matrix();
	hw_build_model_matrix();
	hw_build_view_matrix();
	hw_build_projection_matrix();
}

void hw_build_matrices()
{
	hw_build_2d_matrices();
	hw_build_3d_matrices();
}

void hw_set_ui_sampler_default_state()
{
	hw_ui_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_ui_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_ui_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	hw_ui_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	hw_ui_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	hw_ui_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void hw_update_ui_sampler_state()
{
	hw_ui_sampler_state_.min_filter_ = vid_cfg_.d2_texture_filter_;
	hw_ui_sampler_state_.mag_filter_ = vid_cfg_.d2_texture_filter_;
}

void hw_update_ui_sampler()
{
	hw_update_ui_sampler_state();

	if (hw_ui_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = hw_ui_sampler_state_;
		hw_ui_sampler_->update(param);
	}
}

void hw_destroy_ui_sampler()
{
	hw_ui_sampler_ = nullptr;
}

void hw_create_ui_sampler()
{
	vid_log("Creating UI sampler.");

	hw_update_ui_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = hw_ui_sampler_state_;

	hw_ui_sampler_ = hw_renderer_->create_sampler(param);
}

void hw_set_sprite_sampler_default_state()
{
	hw_sprite_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_sprite_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_sprite_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::nearest;
	hw_sprite_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	hw_sprite_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	hw_sprite_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void hw_update_sprite_sampler_state()
{
	hw_sprite_sampler_state_.min_filter_ = vid_cfg_.d3_texture_image_filter_;
	hw_sprite_sampler_state_.mag_filter_ = vid_cfg_.d3_texture_image_filter_;

	hw_sprite_sampler_state_.mipmap_mode_ = hw_cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_.d3_texture_mipmap_filter_);

	hw_sprite_sampler_state_.anisotropy_ = hw_cfg_texture_anisotropy_to_renderer(
		vid_cfg_.d3_texture_anisotropy_
	);
}

void hw_update_sprite_sampler()
{
	hw_update_sprite_sampler_state();

	if (hw_sprite_sampler_)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = hw_sprite_sampler_state_;
		hw_sprite_sampler_->update(param);
	}
}

void hw_destroy_sprite_sampler()
{
	hw_destroy_sampler(hw_sprite_sampler_);
}

void hw_create_sprite_sampler()
{
	vid_log("Creating sprite sampler.");

	hw_update_sprite_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = hw_sprite_sampler_state_;

	hw_sprite_sampler_ = hw_renderer_->create_sampler(param);
}

void hw_set_wall_sampler_default_state()
{
	hw_wall_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_wall_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_wall_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::nearest;
	hw_wall_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::repeat;
	hw_wall_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::repeat;
	hw_wall_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void hw_update_wall_sampler_state()
{
	hw_wall_sampler_state_.min_filter_ = vid_cfg_.d3_texture_image_filter_;
	hw_wall_sampler_state_.mag_filter_ = vid_cfg_.d3_texture_image_filter_;

	hw_wall_sampler_state_.mipmap_mode_ = hw_cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_.d3_texture_mipmap_filter_);

	hw_wall_sampler_state_.anisotropy_ = hw_cfg_texture_anisotropy_to_renderer(
		vid_cfg_.d3_texture_anisotropy_
	);
}

void hw_update_wall_sampler()
{
	hw_update_wall_sampler_state();

	if (hw_wall_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = hw_wall_sampler_state_;
		hw_wall_sampler_->update(param);
	}
}

void hw_destroy_wall_sampler()
{
	hw_destroy_sampler(hw_wall_sampler_);
}

void hw_create_wall_sampler()
{
	vid_log("Creating wall side sampler.");

	hw_update_wall_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = hw_wall_sampler_state_;

	hw_wall_sampler_ = hw_renderer_->create_sampler(param);
}

void hw_update_player_weapon_vb()
{
	auto vertices = HwPlayerWeaponVbi{};
	vertices.resize(hw_vertices_per_sprite);

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
	hw_update_vertex_buffer(
		hw_player_weapon_vb_,
		0,
		hw_vertices_per_sprite,
		vertices.data()
	);
}

void hw_destroy_player_weapon_ib()
{
	hw_destroy_index_buffer(hw_player_weapon_ib_);
}

void hw_create_player_weapon_ib()
{
	vid_log("Creating player's weapon index buffer.");

	hw_player_weapon_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		hw_indices_per_sprite
	);
}

void hw_update_player_weapon_ib()
{
	using Indices = std::array<std::uint8_t, hw_indices_per_sprite>;

	auto indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	hw_update_index_buffer(
		hw_player_weapon_ib_,
		0,
		hw_indices_per_sprite,
		indices.data()
	);
}

void hw_destroy_player_weapon_vb()
{
	hw_destroy_vertex_buffer(hw_player_weapon_vb_);
}

void hw_create_player_weapon_vb()
{
	vid_log("Creating player's weapon vertex buffer.");

	hw_player_weapon_vb_ = hw_create_vertex_buffer<HwPlayerWeaponVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		hw_vertices_per_sprite
	);
}

void hw_destroy_player_weapon_vi()
{
	hw_destroy_vertex_input(hw_player_weapon_vi_);
}

void hw_create_player_weapon_vi()
{
	vid_log("Creating player's weapon vertex input.");

	hw_create_vertex_input<HwPlayerWeaponVertex>(
		hw_player_weapon_ib_,
		hw_player_weapon_vb_,
		hw_player_weapon_vi_);
}

void hw_update_player_weapon_model_matrix()
{
	const auto& assets_info = get_assets_info();

	const auto aog_scale = 128.0 / 64.0;
	const auto ps_scale = 88.0 / 64.0;

	const auto game_scalar = (assets_info.is_ps() ? ps_scale : aog_scale);
	const auto scalar = game_scalar * vga_height_scale;

	const auto translate_x = 0.5 * static_cast<double>(vid_dimensions_.screen_viewport_width_);

	const auto is_bobbing_enabled = (!g_no_weapon_bobbing && assets_info.is_ps());
	const auto bounce_offset = (is_bobbing_enabled ? -player_get_weapon_bounce_offset() : 0.0);
	const auto translate_y = vga_height_scale * bounce_offset;

	const auto translate_v = glm::vec3
	{
		static_cast<float>(translate_x),
		static_cast<float>(height_compensation_factor * translate_y),
		0.0F
	};

	const auto& identity = glm::identity<glm::mat4>();
	const auto& translate = glm::translate(identity, translate_v);

	const auto& scale = glm::scale(
		identity,
		glm::vec3{scalar, height_compensation_factor * scalar, 0.0F}
	);

	hw_player_weapon_model_matrix_ = translate * scale;
}

void hw_update_player_weapon_view_matrix()
{
	hw_player_weapon_view_matrix_ = glm::identity<glm::mat4>();
}

void hw_build_player_weapon_projection_matrix()
{
	const auto ortho = glm::orthoRH_NO(
		0.0F, // left
		static_cast<float>(vid_dimensions_.screen_viewport_width_), // right
		0.0F, // bottom
		static_cast<float>(vid_dimensions_.screen_viewport_height_), // top
		0.0F, // zNear
		1.0F // zFar
	);

	hw_player_weapon_projection_matrix_ = ortho;
}

void hw_set_player_weapon_sampler_default_state()
{
	hw_player_weapon_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_player_weapon_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	hw_player_weapon_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	hw_player_weapon_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	hw_player_weapon_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	hw_player_weapon_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void hw_update_player_weapon_sampler_state()
{
	hw_player_weapon_sampler_state_.min_filter_ = vid_cfg_.d3_texture_image_filter_;
	hw_player_weapon_sampler_state_.mag_filter_ = vid_cfg_.d3_texture_image_filter_;
}

void hw_update_player_weapon_sampler()
{
	hw_update_player_weapon_sampler_state();

	if (hw_player_weapon_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = hw_player_weapon_sampler_state_;
		hw_player_weapon_sampler_->update(param);
	}
}

void hw_destroy_player_weapon_sampler()
{
	hw_player_weapon_sampler_ = nullptr;
}

void hw_create_player_weapon_sampler()
{
	vid_log("Creating player's weapon sampler.");

	hw_update_player_weapon_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = hw_player_weapon_sampler_state_;

	hw_player_weapon_sampler_ = hw_renderer_->create_sampler(param);
}

void hw_uninitialize_player_weapon()
{
	hw_destroy_player_weapon_vi();
	hw_destroy_player_weapon_ib();
	hw_destroy_player_weapon_vb();
	hw_destroy_player_weapon_sampler();
}

void hw_initialize_player_weapon()
{
	vid_log();
	vid_log("Initializing player's weapon.");

	hw_create_player_weapon_ib();
	hw_create_player_weapon_vb();
	hw_create_player_weapon_vi();
	hw_create_player_weapon_sampler();

	hw_update_player_weapon_ib();
	hw_update_player_weapon_vb();

	hw_update_player_weapon_model_matrix();
	hw_update_player_weapon_view_matrix();
	hw_build_player_weapon_projection_matrix();
}

void hw_destroy_fade_sampler()
{
	hw_destroy_sampler(hw_fade_sampler_);
}

void hw_create_fade_sampler()
{
	vid_log("Creating fade sampler.");

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	param.state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	param.state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	param.state_.address_mode_u_ = bstone::Ren3dAddressMode::repeat;
	param.state_.address_mode_v_ = bstone::Ren3dAddressMode::repeat;
	param.state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;

	hw_fade_sampler_ = hw_renderer_->create_sampler(param);
}

void hw_set_samplers_default_states()
{
	hw_set_ui_sampler_default_state();
	hw_set_sprite_sampler_default_state();
	hw_set_wall_sampler_default_state();
}

void hw_uninitialize_samplers()
{
	hw_destroy_ui_sampler();
	hw_destroy_sprite_sampler();
	hw_destroy_wall_sampler();
	hw_destroy_fade_sampler();
}

void hw_initialize_samplers()
{
	vid_log();
	vid_log("Initializing samplers.");

	hw_create_ui_sampler();
	hw_create_sprite_sampler();
	hw_create_wall_sampler();
	hw_create_fade_sampler();
}

void hw_destroy_command_buffers()
{
	hw_command_buffers_.fill(nullptr);
}

void hw_create_command_buffers()
{
	vid_log("Initializing command buffers.");

	hw_command_buffers_[0] = hw_common_command_buffer_.get();
	hw_command_buffers_[1] = hw_3d_command_buffer_.get();
	hw_command_buffers_[2] = hw_2d_command_buffer_.get();
}

void hw_destroy_common_command_buffer()
{
	hw_common_command_buffer_ = nullptr;
}

void hw_create_common_command_buffer()
{
	vid_log("Creating common command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = hw_common_command_buffer_initial_size;
	param.resize_delta_size_ = hw_common_command_buffer_resize_delta_size;

	hw_common_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void hw_destroy_2d_command_buffer()
{
	hw_2d_command_buffer_ = nullptr;
}

void hw_create_2d_command_buffer()
{
	vid_log("Creating 2D command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = hw_2d_command_buffer_initial_size;
	param.resize_delta_size_ = hw_2d_command_buffer_resize_delta_size;

	hw_2d_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void hw_destroy_3d_command_buffer()
{
	hw_3d_command_buffer_ = nullptr;
}

void hw_create_3d_command_buffer()
{
	vid_log("Creating 3D command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = hw_3d_command_buffer_initial_size;
	param.resize_delta_size_ = hw_3d_command_buffer_resize_delta_size;

	hw_3d_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void hw_uninitialize_command_buffers()
{
	hw_destroy_3d_command_buffer();
	hw_destroy_2d_command_buffer();
	hw_destroy_common_command_buffer();
	hw_destroy_command_buffers();
}

void hw_initialize_command_buffers()
{
	vid_log();
	vid_log("Initializing command buffers.");

	hw_create_common_command_buffer();
	hw_create_3d_command_buffer();
	hw_create_2d_command_buffer();
	hw_create_command_buffers();
}

void hw_destroy_3d_fade_ib()
{
	hw_destroy_index_buffer(hw_3d_fade_ib_);
}

void hw_create_3d_fade_ib()
{
	vid_log("Creating 3D fade index buffer.");

	hw_3d_fade_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		6
	);
}

void hw_destroy_3d_fade_vb()
{
	hw_destroy_vertex_buffer(hw_3d_fade_vb_);
}

void hw_create_3d_fade_vb()
{
	vid_log("Creating 3D fade vertex buffer.");

	hw_3d_fade_vb_ = hw_create_vertex_buffer<HwFadeVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		4
	);
}

void hw_destroy_3d_fade_vi()
{
	hw_destroy_vertex_input(hw_3d_fade_vi_);
}

void hw_create_3d_fade_vi()
{
	vid_log("Creating 3D fade vertex input.");

	hw_create_vertex_input<HwFadeVertex>(
		hw_3d_fade_ib_,
		hw_3d_fade_vb_,
		hw_3d_fade_vi_);
}

void hw_update_3d_fade_ib()
{
	using Indices = std::array<std::uint8_t, 6>;

	const auto& indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	hw_update_index_buffer(
		hw_3d_fade_ib_,
		0,
		6,
		indices.data()
	);
}

void hw_update_3d_fade_vb()
{
	auto vertices = HwFadeVbi{};
	vertices.resize(4);

	auto vertex_index = 0;

	const auto width_f = vid_dimensions_.screen_viewport_width_;
	const auto height_f = vid_dimensions_.screen_viewport_height_;

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

	hw_update_vertex_buffer<HwFadeVertex>(
		hw_3d_fade_vb_,
		0,
		4,
		vertices.data()
	);
}

void hw_destroy_3d_fade_texture_2d()
{
	if (hw_texture_manager_ != nullptr)
	{
		hw_texture_manager_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
	}

	hw_3d_fade_t2d_ = nullptr;
}

void hw_create_3d_fade_texture_2d()
{
	vid_log("Create 3D fade texture.");

	hw_texture_manager_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
	hw_3d_fade_t2d_ = hw_texture_manager_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
}

void hw_uninitialize_3d_fade()
{
	hw_destroy_3d_fade_vi();
	hw_destroy_3d_fade_vb();
	hw_destroy_3d_fade_ib();
	hw_destroy_3d_fade_texture_2d();
}

void hw_destroy_texture_manager()
{
	hw_texture_manager_ = nullptr;
}

void hw_create_texture_manager()
{
	vid_log();
	vid_log("Creating texture manager.");

	hw_texture_manager_ = bstone::HwTextureMgrFactory::create(
		hw_renderer_,
		&vid_sprite_cache,
		mt_task_manager_
	);
}

void hw_initialize_3d_fade()
{
	vid_log();
	vid_log("Initializing 3D fade.");

	hw_create_3d_fade_ib();
	hw_create_3d_fade_vb();
	hw_create_3d_fade_vi();
	hw_create_3d_fade_texture_2d();

	hw_update_3d_fade_ib();
	hw_update_3d_fade_vb();
}

void hw_refresh_common()
{
	hw_sampler_var_ = 0;
	hw_shading_mode_ = 0;
	hw_bs_shade_max_ = shade_max;
	hw_bs_normal_shade_ = normalshade;
	hw_bs_height_numerator_ = heightnumerator;
	hw_bs_lighting_ = 0;


	auto& command_buffer = hw_common_command_buffer_;

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Build commands.
	//

	// Clear buffers.
	//
	{
		static const auto clear_color = bstone::Rgba8{};
		auto& clear = *command_buffer->write_clear();
		clear.clear_.color_ = clear_color;
	}

	// Set shader's 2D sampler.
	//
	if (hw_sampler_var_.is_modified())
	{
		hw_sampler_var_.set_is_modified(false);

		auto command = command_buffer->write_set_sampler_2d_uniform();
		command->var_ = hw_sampler_uniform_;
		command->value_ = hw_sampler_var_;
	}

	// Set shader stage.
	//
	{
		auto& shader_stage = command_buffer->write_set_shader_stage()->shader_stage_;
		shader_stage = hw_shader_stage_.get();
	}

	// Set shading mode.
	//
	if (hw_shading_mode_.is_modified())
	{
		hw_shading_mode_.set_is_modified(false);

		auto& command = *command_buffer->write_set_int32_uniform();
		command.var_ = hw_shading_mode_uniform_;
		command.value_ = hw_shading_mode_;
	}

	// Set shade_max.
	//
	if (hw_bs_shade_max_.is_modified())
	{
		hw_bs_shade_max_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = hw_shade_max_uniform_;
		command.value_ = static_cast<float>(hw_bs_shade_max_);
	}

	// Set normal_shade.
	//
	if (hw_bs_normal_shade_.is_modified())
	{
		hw_bs_normal_shade_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = hw_normal_shade_uniform_;
		command.value_ = static_cast<float>(hw_bs_normal_shade_);
	}

	// Set height_numerator.
	//
	if (hw_bs_height_numerator_.is_modified())
	{
		hw_bs_height_numerator_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = hw_height_numerator_uniform_;
		command.value_ = static_cast<float>(hw_bs_height_numerator_);
	}

	// Set extra_lighting.
	//
	if (hw_bs_lighting_.is_modified())
	{
		hw_bs_lighting_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = hw_extra_lighting_uniform_;
		command.value_ = static_cast<float>(hw_bs_lighting_);
	}

	// Set view_direction.
	//
	if (vid_hw_is_draw_3d_ && hw_bs_view_direction_.is_modified())
	{
		hw_bs_view_direction_.set_is_modified(false);

		auto& command = *command_buffer->write_set_vec2_uniform();
		command.var_ = hw_view_direction_uniform_;
		command.value_ = hw_cast_glm_vec2(hw_bs_view_direction_);
	}

	// Set view_position.
	//
	if (vid_hw_is_draw_3d_ && hw_bs_view_position_.is_modified())
	{
		hw_bs_view_position_.set_is_modified(false);

		auto& command = *command_buffer->write_set_vec2_uniform();
		command.var_ = hw_view_position_uniform_;
		command.value_ = hw_cast_glm_vec2(hw_bs_view_position_);
	}

	// Finalize.
	//
	command_buffer->end_write();
}

void hw_refresh_2d()
{
	// Update 2D texture.
	//
	{
		hw_texture_manager_->update_ui();
	}

	// Update fade color.
	//
	if (hw_2d_fade_is_enabled_)
	{
		hw_texture_manager_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d, hw_2d_fade_color_);
	}


	auto& command_buffer = hw_2d_command_buffer_;

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Build commands.
	//

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = false;
	}

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport()->viewport_;
		viewport.x_ = vid_dimensions_.window_viewport_left_width_;
		viewport.y_ = vid_dimensions_.window_viewport_bottom_height_;
		viewport.width_ = vid_dimensions_.screen_width_;
		viewport.height_ = vid_dimensions_.screen_height_;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 0.0F;
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = hw_ui_sampler_.get();
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_model_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_2d_matrix_model_);
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_view_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_2d_matrix_view_);
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_projection_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_2d_matrix_projection_);
	}

	// Fillers.
	//
	if (!vid_cfg_.is_ui_stretched_)
	{
		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = (vid_is_movie ? hw_2d_black_t2d_1x1_ : hw_2d_white_t2d_1x1_);
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = hw_2d_fillers_vi_.get();
		}

		{
			auto count = 0;
			auto index_offset = 0;

			if (vid_is_hud)
			{
				count = hw_2d_fillers_hud_quad_count;
				index_offset = hw_2d_fillers_hud_index_offset_;
			}
			else
			{
				count = hw_2d_fillers_ui_quad_count;
				index_offset = hw_2d_fillers_ui_index_offset_;
			}

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = count * hw_vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}
	}

	// Draw 2D (UI, menu, etc.).
	//
	{
		if (vid_is_hud)
		{
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}
		}

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = hw_ui_t2d_;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = hw_ui_vi_.get();
		}

		{
			const auto index_offset = (vid_cfg_.is_ui_stretched_
				?
				hw_2d_stretched_index_offset_
				:
				hw_2d_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = hw_vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}

		if (vid_is_hud)
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = false;
		}
	}

	// 2D fade in or out.
	//
	if (hw_2d_fade_is_enabled_)
	{
		// Enable blending.
		//
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = true;
		}

		// Set blending function.
		//
		{
			auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
			blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
			blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
		}

		// Set texture.
		//
		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = hw_2d_fade_t2d_;
		}

		// Set sampler.
		//
		{
			auto& command = *command_buffer->write_set_sampler();
			command.sampler_ = hw_fade_sampler_.get();
		}

		// Set vertex input.
		//
		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = hw_ui_vi_.get();
		}

		// Draw the quad.
		//
		{
			const auto index_offset = (vid_cfg_.is_ui_stretched_ || vid_hw_is_draw_3d_
				?
				hw_2d_stretched_index_offset_
				:
				hw_2d_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = hw_vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}

		// Disable blending.
		//
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = false;
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
}

void hw_render_walls()
{
	if (hw_wall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = hw_wall_side_draw_items_;

	if (hw_walls_to_render_.empty())
	{
		return;
	}

	const auto wall_map_end_it = hw_xy_wall_map_.cend();

	for (const auto wall_xy : hw_walls_to_render_)
	{
		const auto wall_map_it = hw_xy_wall_map_.find(wall_xy);

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

			draw_item.texture_id_ = side.texture_id_;
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
			return lhs.texture_id_ < rhs.texture_id_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = hw_wall_sides_ibi_;

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

		hw_update_index_buffer(
			hw_wall_sides_ib_,
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
	auto& command_buffer = hw_3d_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id_;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id_)
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
				const auto texture_2d = hw_texture_manager_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_wall_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * hw_vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += hw_indices_per_wall_side * draw_quad_count;
			}
		}
	}

	hw_wall_side_draw_item_count_ = draw_side_index;
}

void hw_render_pushwalls()
{
	if (hw_pushwall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = hw_pushwall_side_draw_items_;

	if (hw_pushwalls_to_render_.empty())
	{
		return;
	}

	const auto pushwall_map_end_it = hw_xy_pushwall_map_.cend();

	for (const auto pushwall_xy : hw_pushwalls_to_render_)
	{
		const auto pushwall_map_it = hw_xy_pushwall_map_.find(pushwall_xy);

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

			draw_item.texture_id_ = side.texture_id_;
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
			return lhs.texture_id_ < rhs.texture_id_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = hw_pushwall_sides_ibi_;

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

		hw_update_index_buffer(
			hw_pushwall_sides_ib_,
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
	auto& command_buffer = hw_3d_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id_;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id_)
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
				const auto texture_2d = hw_texture_manager_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_pushwall_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * hw_vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += hw_indices_per_wall_side * draw_quad_count;
			}
		}
	}

	hw_pushwall_side_draw_item_count_ = draw_side_index;
}

void hw_render_doors()
{
	if (hw_door_count_ == 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = hw_door_draw_items_;

	if (hw_doors_to_render_.empty())
	{
		return;
	}

	const auto door_map_end_it = hw_xy_door_map_.cend();

	for (const auto door_xy : hw_doors_to_render_)
	{
		const auto door_map_it = hw_xy_door_map_.find(door_xy);

		if (door_map_it == door_map_end_it)
		{
			continue;
		}

		for (const auto& side : door_map_it->second.sides_)
		{
			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_id_ = side.texture_id_;
			draw_item.hw_door_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_id_ < rhs.texture_id_;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = hw_door_sides_ibi_;

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

		hw_update_index_buffer(
			hw_door_sides_ib_,
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
	auto& command_buffer = hw_3d_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id_;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id_)
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
				const auto texture_2d = hw_texture_manager_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_door_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * hw_vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset;

				draw_index_offset += 6 * draw_quad_count;
			}
		}
	}

	hw_door_draw_item_count_ = draw_side_index;
}

void hw_update_cloaked_actor(
	const HwSprite& sprite)
{
	if (!sprite.flags_.is_visible_)
	{
		return;
	}

	if (sprite.kind_ != HwSpriteKind::actor)
	{
		return;
	}

	const auto& actor = *sprite.bs_object_.actor_;

	const auto is_cloaked = ((actor.flags2 & (FL2_CLOAKED | FL2_DAMAGE_CLOAK)) == FL2_CLOAKED);

	const auto vertex_color = (
		is_cloaked
		?
		HwVertexColor{0x00, 0x00, 0x00, hw_cloaked_actor_alpha_u8}
		:
		HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF}
	);


	auto vertex_index = sprite.vertex_index_;

	// Bottom-left.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = vertex_color;
	}

	// Bottom-right.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = vertex_color;
	}

	// Top-right.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = vertex_color;
	}

	// Top-left.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = vertex_color;
	}
}

int hw_calculate_actor_anim_rotation(
	const objtype& bs_actor)
{
	auto dir = bs_actor.dir;

	const auto view_dir_x = bs_actor.x - player->x;
	const auto view_dir_y = -bs_actor.y + player->y;

	const auto view_angle_rad = std::atan2(view_dir_y, view_dir_x);
	const auto view_angle = bstone::math::rad_to_deg(view_angle_rad);

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(bs_actor.trydir & 127);
	}

	auto target_angle = (view_angle - 180) - dirangle[dir];

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
		return static_cast<int>(4 * (target_angle / (ANGLES / 2))); // seperated by 3 (art layout...)

	}

	return static_cast<int>(target_angle / (ANGLES / 8));
}

int hw_get_bs_actor_sprite_id(
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
		result += hw_calculate_actor_anim_rotation(bs_actor);
	}

	return result;
}

void hw_map_actor(
	const objtype& bs_actor);

void hw_update_actor(
	const std::intptr_t bs_actor_index)
{
	auto& hw_actor = hw_actors_[bs_actor_index];
	const auto& bs_actor = objlist[bs_actor_index];

	if (hw_actor.kind_ == HwSpriteKind::none)
	{
		hw_map_actor(bs_actor);
	}

	const auto new_bs_sprite_id = hw_get_bs_actor_sprite_id(bs_actor);

	if (hw_actor.bs_sprite_id_ == 0 || hw_actor.bs_sprite_id_ != new_bs_sprite_id)
	{
		hw_actor.bs_sprite_id_ = new_bs_sprite_id;
	}

	if (hw_actor.x_ != bs_actor.x || hw_actor.y_ != bs_actor.y)
	{
		hw_actor.x_ = bs_actor.x;
		hw_actor.y_ = bs_actor.y;
	}
}

void hw_orient_sprite(
	HwSprite& sprite)
{
	sprite.flags_.is_visible_ = false;

	if (sprite.bs_sprite_id_ <= 0)
	{
		return;
	}

	if (sprite.kind_ == HwSpriteKind::actor)
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

	if (sprite.kind_ == HwSpriteKind::actor)
	{
		sprite_origin[0] = sprite.x_;
		sprite_origin[1] = sprite.y_;
	}
	else
	{
		sprite_origin[0] = static_cast<double>(sprite.tile_x_) + 0.5;
		sprite_origin[1] = static_cast<double>(sprite.tile_y_) + 0.5;
	};

	auto direction = hw_player_position_ - sprite_origin;

	sprite.flags_.is_visible_ = true;


	auto bottom_left_vertex = sprite_origin;
	auto bottom_right_vertex = sprite_origin;

	const auto square_distance = glm::dot(direction, direction);

	sprite.square_distance_ = square_distance;

	// Orient the sprite along the player's line of sight (inverted).
	//
	direction[0] = -hw_view_direction_[0];
	direction[1] = -hw_view_direction_[1];

	const auto perpendicular_dx = hw_tile_half_dimension_d * direction[1];
	const auto perpendicular_dy = hw_tile_half_dimension_d * direction[0];

	bottom_left_vertex[0] += -perpendicular_dx;
	bottom_left_vertex[1] += +perpendicular_dy;

	bottom_right_vertex[0] += +perpendicular_dx;
	bottom_right_vertex[1] += -perpendicular_dy;


	auto vertex_index = sprite.vertex_index_;

	// Bottom-left.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			0.0F
		};
	}

	// Bottom-right.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			0.0F
		};
	}

	// Top-right.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			hw_tile_dimension_f
		};
	}

	// Top-left.
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			hw_tile_dimension_f
		};
	}

	hw_update_cloaked_actor(sprite);
}

void hw_map_static(
	const statobj_t& bs_static);

void hw_update_static(
	const int bs_static_index)
{
	auto& sprite = hw_statics_[bs_static_index];
	const auto& bs_static = statobjlist[bs_static_index];
	auto& hw_static = hw_statics_[bs_static_index];

	if (sprite.kind_ == HwSpriteKind::none)
	{
		hw_map_static(bs_static);
	}
	else
	{
		hw_static.bs_sprite_id_ = bs_static.shapenum;
	}

	hw_orient_sprite(hw_static);
}

void hw_render_sprites()
{
	// Build draw list.
	//
	auto draw_sprite_index = 0;
	auto& draw_items = hw_sprites_draw_list_;

	auto min_vertex_index = hw_max_sprites_vertices;
	auto max_vertex_index = 0;

	for (const auto bs_static_index : hw_statics_to_render_)
	{
		hw_update_static(bs_static_index);

		auto& hw_static = hw_statics_[bs_static_index];

		auto& draw_item = draw_items[draw_sprite_index++];
		draw_item.texture_id_ = hw_static.bs_sprite_id_;
		draw_item.sprite_ = &hw_static;

		min_vertex_index = std::min(hw_static.vertex_index_, min_vertex_index);
		max_vertex_index = std::max(hw_static.vertex_index_, max_vertex_index);
	}

	for (const auto bs_actor_index : hw_actors_to_render_)
	{
		hw_update_actor(bs_actor_index);

		auto& hw_actor = hw_actors_[bs_actor_index];
		hw_orient_sprite(hw_actor);

		auto& draw_item = draw_items[draw_sprite_index++];
		draw_item.texture_id_ = hw_actor.bs_sprite_id_;
		draw_item.sprite_ = &hw_actor;

		min_vertex_index = std::min(hw_actor.vertex_index_, min_vertex_index);
		max_vertex_index = std::max(hw_actor.vertex_index_, max_vertex_index);
	}

	if (draw_sprite_index == 0)
	{
		return;
	}

	const auto vertex_count = max_vertex_index - min_vertex_index + hw_vertices_per_sprite;

	hw_update_vertex_buffer(
		hw_sprites_vb_,
		min_vertex_index,
		vertex_count,
		&hw_sprites_vbi_[min_vertex_index]
	);

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
		auto& ib_buffer = hw_sprites_ibi_;

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

		hw_update_index_buffer(
			hw_sprites_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto& command_buffer = hw_3d_command_buffer_;

	// Disable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = false;
	}

	// Enable blending.
	//
	{
		auto& command = *command_buffer->write_enable_blending();
		command.is_enable_ = true;
	}

	using CurrentTextureId = bstone::ModValue<int>;

	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto current_texture_id = CurrentTextureId{};

	while (draw_index < draw_sprite_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;
		auto last_lighting = 0;

		draw_quad_count = 0;

		while (draw_index < draw_sprite_index)
		{
			const auto& draw_item = draw_items[draw_index];

			const auto texture_id = draw_item.texture_id_;

			auto lighting = 0;

			if (!gp_no_shading_)
			{
				const auto& hw_sprite = *draw_item.sprite_;

				switch (hw_sprite.kind_)
				{
					case HwSpriteKind::actor:
						lighting = hw_sprite.bs_object_.actor_->lighting;
						break;

					case HwSpriteKind::stat:
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

				last_texture_id = texture_id;
				last_lighting = lighting;
			}
			else if (last_texture_id == texture_id && last_lighting == lighting)
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
			if (!gp_no_shading_)
			{
				hw_bs_lighting_ = last_lighting;

				if (hw_bs_lighting_.is_modified())
				{
					hw_bs_lighting_.set_is_modified(false);

					auto& command = *command_buffer->write_set_float32_uniform();
					command.var_ = hw_extra_lighting_uniform_;
					command.value_ = static_cast<float>(hw_bs_lighting_);
				}
			}

			current_texture_id = last_texture_id;

			if (current_texture_id.is_modified())
			{
				current_texture_id.set_is_modified(false);
				const auto texture_2d = hw_texture_manager_->get_sprite(current_texture_id);

				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_sprites_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * hw_vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += hw_indices_per_sprite * draw_quad_count;
			}
		}
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = true;
	}

	// Disable blending.
	//
	{
		auto& command = *command_buffer->write_enable_blending();
		command.is_enable_ = false;
	}

	hw_sprites_draw_count_ = draw_sprite_index;
}

void hw_apply_widescreen()
{
	hw_calculate_dimensions();
	SetViewSize();
	hw_build_projection_matrix();
	hw_build_player_weapon_projection_matrix();
	hw_update_3d_fade_vb();
}

void hw_update_3d_fade()
{
	hw_3d_fade_is_enabled_ = false;

	auto r_f = 1.0F;
	auto g_f = 1.0F;
	auto b_f = 1.0F;
	auto a_f = 1.0F;

	const auto& palette_shift_info = palette_shift_get_info();

	if (hw_fizzle_fx_is_enabled_)
	{
		hw_3d_fade_is_enabled_ = true;

		auto ratio = hw_fizzle_fx_ratio_;

		if (!hw_fizzle_fx_is_fading_)
		{
			ratio = 1.0F - ratio;
		}

		const auto vga_color = vgapal + (3 * hw_fizzle_fx_color_index_);
		const auto& color_32 = hw_vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);

		r_f = static_cast<float>(color_32.r_) / 255.0F;
		g_f = static_cast<float>(color_32.g_) / 255.0F;
		b_f = static_cast<float>(color_32.b_) / 255.0F;
		a_f = ratio;
	}
	else if (palette_shift_info.is_bonus_shifted_ || palette_shift_info.is_damage_shifted_)
	{
		hw_3d_fade_is_enabled_ = true;

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

	if (!hw_3d_fade_is_enabled_)
	{
		return;
	}

	const auto r = static_cast<std::uint8_t>(a_f * r_f * 255.0F);
	const auto g = static_cast<std::uint8_t>(a_f * g_f * 255.0F);
	const auto b = static_cast<std::uint8_t>(a_f * b_f * 255.0F);
	const auto a = static_cast<std::uint8_t>(a_f * 255.0F);

	const auto rgba_8 = bstone::Rgba8{r, g, b, a};

	hw_texture_manager_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d, rgba_8);
}

void hw_refresh_3d()
{
	auto& command_buffer = hw_3d_command_buffer_;

	command_buffer->enable(false);

	if (!vid_hw_is_draw_3d_ || screenfaded)
	{
		return;
	}

	const auto is_shading = (!gp_no_shading_);

	const auto& assets_info = get_assets_info();

	hw_update_3d_fade();

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport()->viewport_;
		viewport.x_ = vid_dimensions_.screen_viewport_left_width_;
		viewport.y_ = vid_dimensions_.window_viewport_bottom_height_ + vid_dimensions_.screen_viewport_bottom_height_;
		viewport.width_ = vid_dimensions_.screen_viewport_width_;
		viewport.height_ = vid_dimensions_.screen_viewport_height_;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 1.0F;
	}

	// Enable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = true;
	}

	// Enable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = true;
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = true;
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_model_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_3d_matrix_model_);
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_view_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_3d_matrix_view_);
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = hw_projection_mat_uniform_;
		command.value_ = hw_cast_glm_mat4(hw_3d_matrix_projection_);
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = hw_wall_sampler_.get();
	}

	// Set shading mode.
	//
	{
		hw_shading_mode_ = is_shading;

		if (hw_shading_mode_.is_modified())
		{
			hw_shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_set_int32_uniform();
			command.var_ = hw_shading_mode_uniform_;
			command.value_ = hw_shading_mode_;
		}
	}

	// Draw solid walls.
	//
	hw_render_walls();

	// Draw pushwalls.
	//
	hw_render_pushwalls();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = hw_sprite_sampler_.get();
	}

	// Draw doors.
	//
	hw_render_doors();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = hw_wall_sampler_.get();
	}

	// Draw flooring.
	//
	{
		auto texture_2d = (!gp_is_flooring_solid_
			?
			hw_flooring_textured_t2d_
			:
			hw_flooring_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = hw_flooring_vi_.get();
		}

		{
			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = hw_vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = 0;
		}
	}

	// Draw ceiling.
	//
	{
		auto texture_2d = (!gp_is_ceiling_solid_
			?
			hw_ceiling_textured_t2d_
			:
			hw_ceiling_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = hw_ceiling_vi_.get();
		}

		{
			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = hw_vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = 0;
		}
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = hw_sprite_sampler_.get();
	}

	// Draw statics and actors.
	//
	hw_render_sprites();

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = false;
	}

	// Disable shading mode.
	//
	if (is_shading)
	{
		hw_shading_mode_ = 0;

		if (hw_shading_mode_.is_modified())
		{
			hw_shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_set_int32_uniform();
			command.var_ = hw_shading_mode_uniform_;
			command.value_ = hw_shading_mode_;
		}
	}

	// Draw player's weapon.
	//
	if (vid_is_hud)
	{
		const auto player_weapon_sprite_id = player_get_weapon_sprite_id();

		if (player_weapon_sprite_id > 0 || hw_3d_fade_is_enabled_)
		{
			// Set projection matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = hw_projection_mat_uniform_;
				command.value_ = hw_cast_glm_mat4(hw_player_weapon_projection_matrix_);
			}
		}

		if (player_weapon_sprite_id > 0)
		{
			const auto player_weapon_texture = hw_texture_manager_->get_sprite(player_weapon_sprite_id);

			if (assets_info.is_ps())
			{
				hw_update_player_weapon_model_matrix();
			}

			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = hw_model_mat_uniform_;
				command.value_ = hw_cast_glm_mat4(hw_player_weapon_model_matrix_);
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = hw_view_mat_uniform_;
				command.value_ = hw_cast_glm_mat4(hw_player_weapon_view_matrix_);
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = player_weapon_texture;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_set_sampler();
				command.sampler_ = hw_player_weapon_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_player_weapon_vi_.get();
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}

			// Draw the weapon.
			//
			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = hw_vertices_per_quad;
				param.index_byte_depth_ = 1;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = 0;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = false;
			}
		}


		// 3D fade (bonus, damage, death, etc).
		//
		if (hw_3d_fade_is_enabled_)
		{
			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = hw_model_mat_uniform_;
				command.value_ = hw_cast_glm_mat4(glm::identity<glm::mat4>());
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = hw_view_mat_uniform_;
				command.value_ = hw_cast_glm_mat4(glm::identity<glm::mat4>());
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = hw_3d_fade_t2d_;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_set_sampler();
				command.sampler_ = hw_fade_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = hw_3d_fade_vi_.get();
			}

			// Draw the quad.
			//
			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = hw_vertices_per_quad;
				param.index_byte_depth_ = 1;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = 0;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = false;
			}
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
}

bool hw_is_activated_pushwall_tile(
	const int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_flags;
}

bool hw_is_door(
	const int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_flag;
}

bool hw_is_door_track(
	const int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_track_flag;
}

bool hw_is_solid_wall(
	const int tile)
{
	if (tile == 0)
	{
		return false;
	}

	if (hw_is_activated_pushwall_tile(tile))
	{
		return false;
	}

	if (hw_is_door(tile))
	{
		return false;
	}

	return true;
}

bool hw_is_pushwall(
	const int x,
	const int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return false;
	}

	const auto tile_wall = tilemap[x][y];

	if (tile_wall == 0)
	{
		return false;
	}

	if (hw_is_activated_pushwall_tile(tile_wall))
	{
		return true;
	}

	const auto tile_object = mapsegs[1][(MAPSIZE * y) + x];

	if (tile_object == PUSHABLETILE)
	{
		return true;
	}

	return false;
}

bool hw_is_solid_wall(
	const int x,
	const int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return true;
	}

	const auto tile_wall = tilemap[x][y];

	if (!hw_is_solid_wall(tile_wall))
	{
		return false;
	}

	if (hw_is_pushwall(x, y))
	{
		return false;
	}

	if (hw_has_active_pushwall_ &&
		hw_active_pushwall_next_x_ == x &&
		hw_active_pushwall_next_y_ == y)
	{
		return false;
	}

	return true;
}

int hw_get_solid_wall_side_count(
	const int x,
	const int y)
{
	assert(x >= 0 && x < MAPSIZE);
	assert(y >= 0 && y < MAPSIZE);
	assert(hw_is_solid_wall(x, y));

	auto side_count = 4;

	side_count -= hw_is_solid_wall(x + 0, y - 1); // north
	side_count -= hw_is_solid_wall(x + 1, y + 0); // east
	side_count -= hw_is_solid_wall(x + 0, y + 1); // south
	side_count -= hw_is_solid_wall(x - 1, y + 0); // west

	return side_count;
}

int hw_get_door_track_wall_id(
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
			vid_quit("Invalid direction.");
	}

	if (bs_door_x < 0 || bs_door_x >= MAPSIZE || bs_door_y < 0 || bs_door_y >= MAPSIZE)
	{
		return -1;
	}

	const auto bs_door_tile = tilemap[bs_door_x][bs_door_y];

	if (!hw_is_door(bs_door_tile))
	{
		return -1;
	}

	const auto door_index = bs_door_tile & tilemap_wall_mask;
	const auto& door = doorobjlist[door_index];

	return door_get_track_texture_id(door);
}

void hw_refresh()
{
	if (hw_renderer_ == nullptr)
	{
		return;
	}

	if (vid_is_hud && player != nullptr)
	{
		vid_hw_is_draw_3d_ = true;

		hw_update_player();
		hw_build_view_matrix();
	}

	hw_refresh_common();
	hw_refresh_3d();
	hw_refresh_2d();

	hw_renderer_->submit_commands(
		hw_command_buffers_.data(),
		static_cast<int>(hw_command_buffers_.size())
	);

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	hw_renderer_->present();

	vid_hw_is_draw_3d_ = false;
}

void hw_precache_flooring()
{
	hw_texture_manager_->cache_wall(FloorTile);
	hw_flooring_textured_t2d_ = hw_texture_manager_->get_wall(FloorTile);

	const auto vga_index = BottomColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = hw_vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	hw_texture_manager_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring, renderer_color);
}

void hw_precache_ceiling()
{
	hw_texture_manager_->cache_wall(CeilingTile);
	hw_ceiling_textured_t2d_ = hw_texture_manager_->get_wall(CeilingTile);

	const auto vga_index = TopColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = hw_vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	hw_texture_manager_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling, renderer_color);
}

void hw_precache_wall(
	const int wall_id)
{
	hw_texture_manager_->cache_wall(wall_id);
}

void hw_precache_horizontal_wall(
	const int tile_wall)
{
	const auto wall_id = horizwall[tile_wall];

	hw_precache_wall(wall_id);
}

void hw_precache_vertical_wall(
	const int tile_wall)
{
	const auto wall_id = vertwall[tile_wall];

	hw_precache_wall(wall_id);
}

void hw_precache_switches()
{
	hw_precache_horizontal_wall(OFF_SWITCH);
	hw_precache_vertical_wall(OFF_SWITCH);
	hw_precache_horizontal_wall(ON_SWITCH);
	hw_precache_vertical_wall(ON_SWITCH);
}

void hw_precache_door_track(
	const int x,
	const int y)
{
	const auto tile = tilemap[x][y];
	const auto tile_wall = tile & tilemap_wall_mask;

	const auto& bs_door = doorobjlist[tile_wall];

	if (bs_door.tilex != x || bs_door.tiley != y)
	{
		vid_quit("Expected a door at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
	}

	const auto wall_id = door_get_track_texture_id(bs_door);

	hw_precache_wall(wall_id);
}

void hw_precache_walls()
{
	auto has_switch = false;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			const auto tile = tilemap[x][y];
			const auto tile_wall = tile & tilemap_wall_mask;

			if (hw_is_door(tile))
			{
				hw_precache_door_track(x, y);

				continue;
			}

			if (!hw_is_solid_wall(x, y))
			{
				continue;
			}

			if (tile_wall == ON_SWITCH || tile_wall == OFF_SWITCH)
			{
				has_switch = true;
			}

			hw_precache_horizontal_wall(tile_wall);
			hw_precache_vertical_wall(tile_wall);
		}
	}

	if (has_switch)
	{
		hw_precache_switches();
	}
}

void hw_precache_pushwalls()
{
	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!hw_is_pushwall(x, y))
			{
				continue;
			}

			const auto tile = tilemap[x][y];
			const auto tile_wall = tile & tilemap_wall_mask;

			hw_precache_horizontal_wall(tile_wall);
			hw_precache_vertical_wall(tile_wall);
		}
	}
}

void hw_precache_door_side(
	const int page_number)
{
	hw_texture_manager_->cache_wall(page_number);
}

void hw_precache_door(
	const doorobj_t& door)
{
	auto horizontal_locked_page_number = 0;
	auto horizontal_unlocked_page_number = 0;
	auto vertical_locked_page_number = 0;
	auto vertical_unlocked_page_number = 0;

	door_get_page_numbers_for_caching(
		door,
		horizontal_locked_page_number,
		horizontal_unlocked_page_number,
		vertical_locked_page_number,
		vertical_unlocked_page_number
	);

	hw_precache_door_side(horizontal_locked_page_number);
	hw_precache_door_side(horizontal_unlocked_page_number);
	hw_precache_door_side(vertical_locked_page_number);
	hw_precache_door_side(vertical_unlocked_page_number);
}

void hw_precache_doors()
{
	hw_door_count_ = 0;

	for (auto bs_door = doorobjlist; lastdoorobj && bs_door != lastdoorobj; ++bs_door)
	{
		++hw_door_count_;

		hw_precache_door(*bs_door);
	}
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
	const auto traits = HwVertexAttributeTraits<TVertex, HwVertexAttribLocationId::position>{};

	HwUpdateVertexXyz<TVertex, traits.is_valid>{}(vertex, xyz);
}

template<typename TVertex, bool TIsExists = false>
struct HwUpdateVertexRgba
{
	void operator()(
		TVertex& vertex,
		const HwVertexColor& rgba_8) const
	{
		static_cast<void>(vertex);
		static_cast<void>(rgba_8);
	}
}; // HwUpdateVertexRgba

template<typename TVertex>
struct HwUpdateVertexRgba<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const HwVertexColor& rgba_8) const
	{
		vertex.rgba_8_ = rgba_8;
	}
}; // HwUpdateVertexRgba

template<typename TVertex>
void hw_update_vertex_rgba(
	TVertex& vertex,
	const HwVertexColor& rgba_8)
{
	const auto traits = HwVertexAttributeTraits<TVertex, HwVertexAttribLocationId::color>{};

	HwUpdateVertexRgba<TVertex, traits.is_valid>{}(vertex, rgba_8);
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
	const auto traits = HwVertexAttributeTraits<TVertex, HwVertexAttribLocationId::texture_coordinates>{};

	HwUpdateVertexUv<TVertex, traits.is_valid>{}(vertex, uv);
}

template<typename TVertex>
void hw_map_wall_side(
	const controldir_t side_direction,
	HwWall& wall,
	int& vertex_index,
	HwVertexBufferImageT<TVertex>& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{hw_tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{hw_tile_dimension_f, hw_tile_dimension_f, hw_tile_dimension_f, 0.0F,},
		{0.0F, hw_tile_dimension_f, hw_tile_dimension_f, hw_tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, hw_tile_dimension_f,},
	};


	const auto x = wall.x_;
	const auto y = wall.y_;

	const auto tile = tilemap[x][y];
	const auto wall_id = tile & tilemap_wall_mask;
	const auto has_door_tracks = hw_is_door_track(tile);

	auto is_vertical = false;
	auto wall_texture_id = 0;

	switch (side_direction)
	{
	case di_north:
	case di_south:
		wall_texture_id = horizwall[wall_id];
		break;

	case di_east:
	case di_west:
		is_vertical = true;
		wall_texture_id = vertwall[wall_id];
		break;

	default:
		vid_quit("Invalid direction.");
	}

	auto is_door_track = false;

	if (has_door_tracks)
	{
		const auto door_track_wall_id = hw_get_door_track_wall_id(x, y, side_direction);

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
	side.texture_id_ = wall_texture_id;
	side.wall_ = &wall;

	const auto& rgba_8 = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};

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

		hw_update_vertex_xyz(vertex, xyz);
		hw_update_vertex_rgba(vertex, rgba_8);
		hw_update_vertex_uv(vertex, uv);
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

		hw_update_vertex_xyz(vertex, xyz);
		hw_update_vertex_rgba(vertex, rgba_8);
		hw_update_vertex_uv(vertex, uv);
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			hw_map_height_f,
		};

		const auto& uv = HwVertexTextureCoordinates{1.0F, 1.0F};

		hw_update_vertex_xyz(vertex, xyz);
		hw_update_vertex_rgba(vertex, rgba_8);
		hw_update_vertex_uv(vertex, uv);
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = HwVertexPosition
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			hw_map_height_f,
		};

		const auto& uv = HwVertexTextureCoordinates{0.0F, 1.0F};

		hw_update_vertex_xyz(vertex, xyz);
		hw_update_vertex_rgba(vertex, rgba_8);
		hw_update_vertex_uv(vertex, uv);
	}
}

template<typename TVertex>
void hw_map_xy_to_xwall(
	const HwXyWallKind wall_kind,
	const int x,
	const int y,
	HwXyWallMap& map,
	int& vertex_index,
	HwVertexBufferImageT<TVertex>& vb_buffer)
{
	switch (wall_kind)
	{
	case HwXyWallKind::solid:
	case HwXyWallKind::push:
		break;

	default:
		vid_quit("Invalid wall kind.");
	}

	auto is_north_solid = false;
	auto is_east_solid = false;
	auto is_south_solid = false;
	auto is_west_solid = false;

	if (wall_kind == HwXyWallKind::solid)
	{
		is_north_solid = hw_is_solid_wall(x + 0, y - 1);
		is_east_solid = hw_is_solid_wall(x + 1, y + 0);
		is_south_solid = hw_is_solid_wall(x + 0, y + 1);
		is_west_solid = hw_is_solid_wall(x - 1, y + 0);
	}

	if (is_north_solid && is_east_solid && is_south_solid && is_west_solid)
	{
		// Nothing to draw.
		// This solid wall is surrounded by other solid ones.

		return;
	}

	const auto xy = hw_encode_xy(x, y);

	if (map.find(xy) != map.cend())
	{
		Quit("Wall mapping already exist.");
	}

	map[xy] = HwWall{};
	auto& wall = map[xy];

	wall.x_ = x;
	wall.y_ = y;

	// A north side.
	if (!is_north_solid)
	{
		hw_map_wall_side(
			di_north,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An east side.
	if (!is_east_solid)
	{
		hw_map_wall_side(
			di_east,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An south side.
	if (!is_south_solid)
	{
		hw_map_wall_side(
			di_south,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// A west side.
	if (!is_west_solid)
	{
		hw_map_wall_side(
			di_west,
			wall,
			vertex_index,
			vb_buffer
		);
	}
}

void hw_build_walls()
{
	hw_uninitialize_walls();

	// Check for moving pushwall.
	//
	hw_has_active_pushwall_ = (pwallstate != 0.0);

	hw_active_pushwall_next_x_ = 0;
	hw_active_pushwall_next_y_ = 0;

	if (hw_has_active_pushwall_)
	{
		hw_active_pushwall_next_x_ = pwallx;
		hw_active_pushwall_next_y_ = pwally;

		switch (pwalldir)
		{
		case di_north:
			--hw_active_pushwall_next_y_;
			break;

		case di_east:
			++hw_active_pushwall_next_x_;
			break;

		case di_south:
			++hw_active_pushwall_next_y_;
			break;

		case di_west:
			--hw_active_pushwall_next_x_;
			break;

		default:
			Quit("Invalid direction.");
		}
	}

	// Count walls and their sides.
	//
	hw_wall_count_ = 0;
	hw_wall_side_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!hw_is_solid_wall(x, y))
			{
				continue;
			}

			if (hw_has_active_pushwall_ &&
				x == hw_active_pushwall_next_x_ &&
				y == hw_active_pushwall_next_y_)
			{
				continue;
			}

			hw_wall_count_ += 1;
			hw_wall_side_count_ += hw_get_solid_wall_side_count(x, y);
		}
	}

	// Check for maximums.
	//
	hw_wall_count_ += hw_pushwall_count_;
	hw_wall_side_count_ += hw_pushwall_count_ * hw_sides_per_wall;

	const auto index_count = hw_wall_side_count_ * hw_indices_per_wall_side;

	if (index_count == 0)
	{
		return;
	}

	if (index_count > hw_max_wall_sides_indices)
	{
		vid_quit("Too many wall indices.");
	}

	// Create index an vertex buffers.
	//
	hw_initialize_walls();

	// Build the map (XY to wall).
	//
	const auto vertex_count = hw_wall_side_count_ * hw_vertices_per_wall_side;

	auto vb_buffer = HwWallsVbi{};
	vb_buffer.resize(vertex_count);

	hw_xy_wall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!hw_is_solid_wall(x, y))
			{
				continue;
			}

			if (hw_has_active_pushwall_ &&
				x == hw_active_pushwall_next_x_ &&
				y == hw_active_pushwall_next_y_)
			{
				continue;
			}

			hw_map_xy_to_xwall(
				HwXyWallKind::solid,
				x,
				y,
				hw_xy_wall_map_,
				vertex_index,
				vb_buffer
			);
		}
	}

	hw_wall_vertex_count_ = vertex_index;

	// Update vertex buffer.
	//
	hw_update_vertex_buffer(
		hw_wall_sides_vb_,
		0,
		vertex_count,
		vb_buffer.data()
	);

	hw_walls_to_render_.reserve(hw_wall_count_);
}

void hw_translate_pushwall_side(
	const float translate_x,
	const float translate_y,
	const controldir_t side_direction,
	const HwWall& wall,
	int& vertex_index,
	HwPushwallsVbi& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{hw_tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{hw_tile_dimension_f, hw_tile_dimension_f, hw_tile_dimension_f, 0.0F,},
		{0.0F, hw_tile_dimension_f, hw_tile_dimension_f, hw_tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, hw_tile_dimension_f,},
	};


	switch (side_direction)
	{
	case di_north:
	case di_south:
	case di_east:
	case di_west:
		break;

	default:
		vid_quit("Invalid direction.");
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
			hw_map_height_f,
		};
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz_ = HwVertexPosition
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			hw_map_height_f,
		};
	}
}

void hw_translate_pushwall(
	const HwWall& wall,
	int& vertex_index,
	HwPushwallsVbi& vb_buffer)
{
	auto translate_distance = static_cast<float>(pwallpos);

	auto translate_x = 0.0F;
	auto translate_y = 0.0F;

	switch (pwalldir)
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
		vid_quit("Invalid direction.");
	}

	hw_translate_pushwall_side(
		translate_x,
		translate_y,
		di_north,
		wall,
		vertex_index,
		vb_buffer
	);

	hw_translate_pushwall_side(
		translate_x,
		translate_y,
		di_east,
		wall,
		vertex_index,
		vb_buffer
	);

	hw_translate_pushwall_side(
		translate_x,
		translate_y,
		di_south,
		wall,
		vertex_index,
		vb_buffer
	);

	hw_translate_pushwall_side(
		translate_x,
		translate_y,
		di_west,
		wall,
		vertex_index,
		vb_buffer
	);
}

void hw_translate_pushwall()
{
	const auto xy = hw_encode_xy(pwallx, pwally);

	const auto wall_item_it = hw_xy_pushwall_map_.find(xy);

	if (wall_item_it == hw_xy_pushwall_map_.cend())
	{
		vid_quit("Pushwall mapping not found.");
	}

	const auto& wall = wall_item_it->second;

	const auto first_vertex_index = wall.sides_.front().vertex_index_;

	auto vertex_index = first_vertex_index;

	hw_translate_pushwall(wall, vertex_index, hw_pushwalls_vbi_);

	const auto vertex_count = vertex_index - first_vertex_index;

	hw_update_vertex_buffer(
		hw_pushwall_sides_vb_,
		first_vertex_index,
		vertex_count,
		&hw_pushwalls_vbi_[first_vertex_index]
	);
}

void hw_step_pushwall(
	const int old_x,
	const int old_y)
{
	const auto old_xy = hw_encode_xy(old_x, old_y);

	const auto old_wall_item_it = hw_xy_pushwall_map_.find(old_xy);

	if (old_wall_item_it == hw_xy_pushwall_map_.cend())
	{
		vid_quit("Pushwall mapping not found.");
	}

	const auto new_xy = hw_encode_xy(pwallx, pwally);

	auto wall = old_wall_item_it->second;
	wall.x_ = pwallx;
	wall.y_ = pwally;

	static_cast<void>(hw_xy_pushwall_map_.erase(old_xy));
	hw_xy_pushwall_map_[new_xy] = wall;

	hw_translate_pushwall();
}

void hw_build_pushwalls()
{
	hw_uninitialize_pushwalls();

	// Count pushwalls and their sides.
	//
	hw_pushwall_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!hw_is_pushwall(x, y))
			{
				continue;
			}

			hw_pushwall_count_ += 1;
		}
	}

	hw_pushwall_side_count_ = hw_pushwall_count_ * hw_sides_per_wall;

	// Check for maximums.
	//
	const auto index_count = hw_pushwall_side_count_ * hw_indices_per_wall_side;

	if (index_count > hw_max_wall_sides_indices)
	{
		vid_quit("Too many pushwall indices.");
	}

	if (hw_pushwall_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	hw_initialize_pushwalls();

	// Build the map (XY to pushwall).
	//
	const auto vertex_count = hw_pushwall_side_count_ * hw_vertices_per_wall_side;

	hw_pushwalls_vbi_.clear();
	hw_pushwalls_vbi_.resize(vertex_count);

	hw_xy_pushwall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!hw_is_pushwall(x, y))
			{
				continue;
			}

			hw_map_xy_to_xwall(
				HwXyWallKind::push,
				x,
				y,
				hw_xy_pushwall_map_,
				vertex_index,
				hw_pushwalls_vbi_
			);
		}
	}

	// Update vertex buffer.
	//
	hw_update_vertex_buffer(
		hw_pushwall_sides_vb_,
		0,
		vertex_count,
		hw_pushwalls_vbi_.data()
	);


	hw_pushwalls_to_render_.reserve(hw_pushwall_count_);
}

template<typename TVertex>
void hw_update_quad_vertices(
	const HwQuadFlags flags,
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

void hw_map_door_side(
	HwDoorSide& door_side,
	int& vertex_index,
	HwDoorsVbi& vb_buffer)
{
	const auto& hw_door = *door_side.hw_door_;
	const auto bs_door_index = hw_door.bs_door_index_;
	const auto& bs_door = doorobjlist[bs_door_index];
	const auto door_offset = static_cast<float>(0.5 * doorposition[bs_door_index]);

	auto flags = HwQuadFlags{};
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

		hw_update_quad_vertices(
			flags,
			origin,
			size,
			vertex_index,
			vb_buffer
		);
	}
}

void hw_map_xy_to_door(
	const doorobj_t& bs_door,
	int& vertex_index,
	HwDoorsVbi& vb_buffer)
{
	const auto xy = hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = hw_xy_door_map_.find(xy);

	if (map_it != hw_xy_door_map_.cend())
	{
		vid_quit("Door already mapped.");
	}

	hw_xy_door_map_[xy] = HwDoor{};
	auto& hw_door = hw_xy_door_map_[xy];

	const auto bs_door_index = static_cast<int>(&bs_door - doorobjlist);
	hw_door.bs_door_index_ = bs_door_index;
	hw_door.vertex_index_ = vertex_index;

	for (int i = 0; i < hw_door_halves_per_side; ++i)
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

	hw_map_door_side(hw_door.sides_.front(), vertex_index, vb_buffer);

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

	hw_door.sides_[0].texture_id_ = front_face_page_number;
	hw_door.sides_[1].texture_id_ = back_face_page_number;
}

void hw_build_doors()
{
	hw_uninitialize_door_sides();

	if (hw_door_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	hw_initialize_door_sides();

	// Build the map (XY to door).
	//
	const auto vertex_count = hw_vertices_per_door * hw_door_count_;

	hw_doors_vbi_.clear();
	hw_doors_vbi_.resize(vertex_count);

	hw_xy_door_map_.clear();

	auto vertex_index = 0;

	for (auto bs_door = doorobjlist; bs_door != lastdoorobj; ++bs_door)
	{
		hw_map_xy_to_door(
			*bs_door,
			vertex_index,
			hw_doors_vbi_
		);
	}

	// Update vertex buffer.
	//
	hw_update_vertex_buffer(
		hw_door_sides_vb_,
		0,
		vertex_count,
		hw_doors_vbi_.data()
	);

	hw_doors_to_render_.reserve(hw_door_count_);
}

void hw_initialize_sprites_ibi()
{
	vid_log("Creating sprite index buffer.");

	const auto index_count = hw_max_sprites_indices;

	hw_sprites_ib_ = hw_create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);

	hw_sprites_ibi_.clear();
	hw_sprites_ibi_.resize(index_count);
}

void hw_uninitialize_sprites_ib()
{
	hw_destroy_index_buffer(hw_sprites_ib_);
	hw_sprites_ibi_.clear();
}

void hw_initialize_sprites_vb()
{
	vid_log("Creating sprite vertex buffer.");

	const auto vertex_count = hw_max_sprites_vertices;

	hw_sprites_vb_ = hw_create_vertex_buffer<HwSpriteVertex>(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		vertex_count
	);

	hw_sprites_vbi_.resize(vertex_count);
}

void hw_uninitialize_sprites_vb()
{
	hw_destroy_vertex_buffer(hw_sprites_vb_);
	hw_sprites_vbi_.clear();
}

void hw_uninitialize_sprites_vi()
{
	hw_destroy_vertex_input(hw_sprites_vi_);
}

void hw_initialize_sprites_vi()
{
	vid_log("Creating sprite vertex input.");

	hw_create_vertex_input<HwSpriteVertex>(
		hw_sprites_ib_,
		hw_sprites_vb_,
		hw_sprites_vi_);
}

void hw_initialize_statics()
{
	hw_statics_.resize(MAXSTATS);

	hw_statics_to_render_.clear();
	hw_statics_to_render_.reserve(MAXSTATS);
}

void hw_initialize_actors()
{
	hw_actors_.resize(MAXACTORS);

	hw_actors_to_render_.clear();
	hw_actors_to_render_.reserve(MAXACTORS);
}

void hw_initialize_sprites()
{
	vid_log("Initializing sprites.");

	hw_sprites_draw_count_ = 0;
	hw_sprites_draw_list_.clear();
	hw_sprites_draw_list_.resize(hw_max_sprites);

	hw_initialize_sprites_ibi();
	hw_initialize_sprites_vb();
	hw_initialize_sprites_vi();
	hw_initialize_statics();
	hw_initialize_actors();
}

void hw_uninitialize_statics()
{
	hw_statics_.clear();
}

void hw_uninitialize_actors()
{
	hw_actors_.clear();
}

void hw_uninitialize_sprites()
{
	hw_uninitialize_statics();
	hw_uninitialize_actors();

	hw_sprites_draw_count_ = 0;
	hw_sprites_draw_list_.clear();

	hw_uninitialize_sprites_vi();
	hw_uninitialize_sprites_ib();
	hw_uninitialize_sprites_vb();
}

void hw_map_sprite(
	const HwSpriteKind sprite_kind,
	int vertex_index,
	HwSprite& sprite)
{
	sprite.kind_ = sprite_kind;
	sprite.vertex_index_ = vertex_index;

	// Bottom-left.
	//
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = hw_sprites_vbi_[vertex_index++];
		vertex.rgba_8_ = HwVertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = hw_sprites_vbi_[vertex_index];
		vertex.rgba_8_ = bstone::Rgba8{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv_ = HwVertexTextureCoordinates{0.0F, 1.0F};
	}
}

void hw_map_static(
	const statobj_t& bs_static)
{
	const auto bs_static_index = static_cast<int>(&bs_static - statobjlist.data());

	auto vertex_index = hw_statics_base_vertex_index;
	vertex_index += (bs_static_index * hw_vertices_per_sprite);

	auto& sprite = hw_statics_[bs_static_index];
	sprite.tile_x_ = bs_static.tilex;
	sprite.tile_y_ = bs_static.tiley;
	sprite.bs_object_.stat_ = &bs_static;
	sprite.bs_sprite_id_ = bs_static.shapenum;

	hw_map_sprite(HwSpriteKind::stat, vertex_index, sprite);
}

void hw_cache_sprite(
	const int bs_sprite_id)
{
	hw_texture_manager_->cache_sprite(bs_sprite_id);
}

void hw_precache_water_bowl()
{
	hw_cache_sprite(SPR_STAT_40); // Full.
	hw_cache_sprite(SPR_STAT_41); // Empty.
}

void hw_precache_chicken_leg()
{
	hw_cache_sprite(SPR_STAT_42); // Intact.
	hw_cache_sprite(SPR_STAT_43); // Gnawed.
}

void hw_precache_ham()
{
	hw_cache_sprite(SPR_STAT_44); // Intact.
	hw_cache_sprite(SPR_STAT_45); // Bone.
}

void hw_precache_candy_bar()
{
	hw_cache_sprite(SPR_CANDY_BAR); // Intact.
	hw_cache_sprite(SPR_CANDY_WRAPER); // Wrapper.
}

void hw_precache_sandwich()
{
	hw_cache_sprite(SPR_SANDWICH); // Intact.
	hw_cache_sprite(SPR_SANDWICH_WRAPER); // Wrapper.
}

void hw_precache_plasma_detonator_explosion()
{
	hw_cache_sprite(SPR_DETONATOR_EXP1);
	hw_cache_sprite(SPR_DETONATOR_EXP2);
	hw_cache_sprite(SPR_DETONATOR_EXP3);
	hw_cache_sprite(SPR_DETONATOR_EXP4);
	hw_cache_sprite(SPR_DETONATOR_EXP5);
	hw_cache_sprite(SPR_DETONATOR_EXP6);
	hw_cache_sprite(SPR_DETONATOR_EXP7);
	hw_cache_sprite(SPR_DETONATOR_EXP8);
}

void hw_precache_plasma_detonator()
{
	hw_cache_sprite(SPR_DOORBOMB);
	hw_cache_sprite(SPR_ALT_DOORBOMB);

	hw_precache_plasma_detonator_explosion();
}

void hw_precache_static(
	const statobj_t& bs_static)
{
	const auto sprite_number = bs_static.shapenum;

	if (false)
	{
	}
	else if (sprite_number == SPR_STAT_40 || sprite_number == SPR_STAT_41)
	{
		hw_precache_water_bowl();
	}
	else if (sprite_number == SPR_STAT_42 || sprite_number == SPR_STAT_43)
	{
		hw_precache_chicken_leg();
	}
	else if (sprite_number == SPR_STAT_44 || sprite_number == SPR_STAT_45)
	{
		hw_precache_ham();
	}
	else if (sprite_number == SPR_CANDY_BAR || sprite_number == SPR_CANDY_WRAPER)
	{
		hw_precache_candy_bar();
	}
	else if (sprite_number == SPR_SANDWICH || sprite_number == SPR_SANDWICH_WRAPER)
	{
		hw_precache_sandwich();
	}
	else if (sprite_number == SPR_DOORBOMB || sprite_number == SPR_ALT_DOORBOMB)
	{
		hw_precache_plasma_detonator();
	}
	else
	{
		hw_cache_sprite(sprite_number);
	}
}

void hw_precache_statics()
{
	for (auto bs_static = statobjlist.data(); laststatobj && bs_static != laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		hw_precache_static(*bs_static);
	}
}

void hw_map_actor(
	const objtype& bs_actor)
{
	const auto bs_actor_index = hw_get_actor_index(bs_actor);

	auto vertex_index = hw_actors_base_vertex_index;
	vertex_index += (bs_actor_index * hw_vertices_per_sprite);

	auto& sprite = hw_actors_[bs_actor_index];

	hw_map_sprite(HwSpriteKind::actor, vertex_index, sprite);

	sprite.x_ = bs_actor.x;
	sprite.y_ = bs_actor.y;
	sprite.tile_x_ = bs_actor.tilex;
	sprite.tile_y_ = bs_actor.tiley;
	sprite.bs_sprite_id_ = hw_get_bs_actor_sprite_id(bs_actor);

	sprite.bs_object_.actor_ = &bs_actor;
}

// Explosion.
void hw_precache_explosion()
{
	hw_cache_sprite(SPR_EXPLOSION_1);
	hw_cache_sprite(SPR_EXPLOSION_2);
	hw_cache_sprite(SPR_EXPLOSION_3);
	hw_cache_sprite(SPR_EXPLOSION_4);
	hw_cache_sprite(SPR_EXPLOSION_5);
}

// Clip Explosion.
void hw_precache_clip_explosion()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_CLIP_EXP1);
		hw_cache_sprite(SPR_CLIP_EXP2);
		hw_cache_sprite(SPR_CLIP_EXP3);
		hw_cache_sprite(SPR_CLIP_EXP4);
		hw_cache_sprite(SPR_CLIP_EXP5);
		hw_cache_sprite(SPR_CLIP_EXP6);
		hw_cache_sprite(SPR_CLIP_EXP7);
		hw_cache_sprite(SPR_CLIP_EXP8);
	}
}

// Grenade explosion.
void hw_precache_grenade_explosion()
{
	hw_cache_sprite(SPR_GRENADE_EXPLODE1);
	hw_cache_sprite(SPR_GRENADE_EXPLODE2);
	hw_cache_sprite(SPR_GRENADE_EXPLODE3);
	hw_cache_sprite(SPR_GRENADE_EXPLODE4);
	hw_cache_sprite(SPR_GRENADE_EXPLODE5);
}

// Flying grenade.
void hw_precache_flying_grenade()
{
	hw_cache_sprite(SPR_GRENADE_FLY1);
	hw_cache_sprite(SPR_GRENADE_FLY2);
	hw_cache_sprite(SPR_GRENADE_FLY3);
	hw_cache_sprite(SPR_GRENADE_FLY4);

	hw_precache_grenade_explosion();
}

void hw_precache_anti_plasma_cannon_explosion()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_BFG_EXP1);
		hw_cache_sprite(SPR_BFG_EXP2);
		hw_cache_sprite(SPR_BFG_EXP3);
		hw_cache_sprite(SPR_BFG_EXP4);
		hw_cache_sprite(SPR_BFG_EXP5);
		hw_cache_sprite(SPR_BFG_EXP6);
		hw_cache_sprite(SPR_BFG_EXP7);
		hw_cache_sprite(SPR_BFG_EXP8);
	}
}

void hw_precache_anti_plasma_cannon_shot()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_BFG_WEAPON_SHOT1);
		hw_cache_sprite(SPR_BFG_WEAPON_SHOT2);
		hw_cache_sprite(SPR_BFG_WEAPON_SHOT3);


		hw_precache_anti_plasma_cannon_explosion();
	}
}

// A rubble.
void hw_precache_rubble()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_RUBBLE);
	}
}

// Toxic waste (green #1).
void hw_precache_toxic_waste_green_1()
{
	hw_cache_sprite(SPR_GREEN_OOZE1);
	hw_cache_sprite(SPR_GREEN_OOZE2);
	hw_cache_sprite(SPR_GREEN_OOZE3);
}

// Toxic waste (green #2).
void hw_precache_toxic_waste_green_2()
{
	hw_cache_sprite(SPR_GREEN2_OOZE1);
	hw_cache_sprite(SPR_GREEN2_OOZE2);
	hw_cache_sprite(SPR_GREEN2_OOZE3);
}

// Toxic waste (black #1).
void hw_precache_toxic_waste_black_1()
{
	hw_cache_sprite(SPR_BLACK_OOZE1);
	hw_cache_sprite(SPR_BLACK_OOZE2);
	hw_cache_sprite(SPR_BLACK_OOZE3);
}

// Toxic waste (black #2).
void hw_precache_toxic_waste_black_2()
{
	hw_cache_sprite(SPR_BLACK2_OOZE1);
	hw_cache_sprite(SPR_BLACK2_OOZE2);
	hw_cache_sprite(SPR_BLACK2_OOZE3);
}

// Coin (1).
void hw_precache_coin_1()
{
	hw_cache_sprite(SPR_STAT_77);
}

// Red Access Card.
void hw_precache_red_access_card()
{
	hw_cache_sprite(SPR_STAT_32);
}

// Yellow Access Card.
void hw_precache_yellow_access_card()
{
	hw_cache_sprite(SPR_STAT_33);
}

// Green Access Card (AOG).
void hw_precache_green_access_card()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		hw_cache_sprite(SPR_STAT_34);
	}
}

// Blue Access Card.
void hw_precache_blue_access_card()
{
	hw_cache_sprite(SPR_STAT_35);
}

// Golden Access Card (AOG).
void hw_precache_golden_access_card()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		hw_cache_sprite(SPR_STAT_36);
	}
}

// Small yellow box (PS).
void hw_precache_small_yellow_box()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_STAT_36);
	}
}

// Partial Charge Pack.
void hw_precache_partial_charge_pack()
{
	hw_cache_sprite(SPR_STAT_26);
}

// Charge Pack.
void hw_precache_charge_pack()
{
	hw_cache_sprite(SPR_STAT_31);
}

// Slow Fire Protector.
void hw_precache_slow_fire_protector()
{
	hw_cache_sprite(SPR_STAT_24);
}

// Rapid Assault Weapon.
void hw_precache_rapid_assault_weapon()
{
	hw_cache_sprite(SPR_STAT_27);
}

// Dual Neutron Disruptor.
void hw_precache_dual_neutron_disruptor_weapon()
{
	hw_cache_sprite(SPR_STAT_28);
}

// Plasma Discharge Unit.
void hw_precache_plasma_discharge_unit_weapon()
{
	hw_cache_sprite(SPR_STAT_46);
}

// Anti-Plasma Cannon.
void hw_precache_anti_plasma_cannon_weapon()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_STAT_34);
	}
}

// Generic alien spit (#1).
void hw_precache_generic_alien_spit_1()
{
	hw_cache_sprite(SPR_SPIT1_1);
	hw_cache_sprite(SPR_SPIT1_2);
	hw_cache_sprite(SPR_SPIT1_3);

	hw_cache_sprite(SPR_SPIT_EXP1_1);
	hw_cache_sprite(SPR_SPIT_EXP1_2);
	hw_cache_sprite(SPR_SPIT_EXP1_3);
}

// Generic alien spit (#2).
void hw_precache_generic_alien_spit_2()
{
	hw_cache_sprite(SPR_SPIT2_1);
	hw_cache_sprite(SPR_SPIT2_2);
	hw_cache_sprite(SPR_SPIT2_3);

	hw_cache_sprite(SPR_SPIT_EXP2_1);
	hw_cache_sprite(SPR_SPIT_EXP2_2);
	hw_cache_sprite(SPR_SPIT_EXP2_3);
}

// Generic alien spit (#3).
void hw_precache_generic_alien_spit_3()
{
	hw_cache_sprite(SPR_SPIT3_1);
	hw_cache_sprite(SPR_SPIT3_2);
	hw_cache_sprite(SPR_SPIT3_3);

	hw_cache_sprite(SPR_SPIT_EXP3_1);
	hw_cache_sprite(SPR_SPIT_EXP3_2);
	hw_cache_sprite(SPR_SPIT_EXP3_3);
}

// Electrical Shot.
void hw_precache_electrical_shot()
{
	hw_cache_sprite(SPR_ELEC_SHOT1);
	hw_cache_sprite(SPR_ELEC_SHOT2);
	hw_cache_sprite(SPR_ELEC_SHOT_EXP1);
	hw_cache_sprite(SPR_ELEC_SHOT_EXP2);
}

// Sector Patrol (AOG) / Sector Guard (PS).
void hw_precache_sector_patrol_or_sector_guard()
{
	hw_cache_sprite(SPR_RENT_S_1);
	hw_cache_sprite(SPR_RENT_S_2);
	hw_cache_sprite(SPR_RENT_S_3);
	hw_cache_sprite(SPR_RENT_S_4);
	hw_cache_sprite(SPR_RENT_S_5);
	hw_cache_sprite(SPR_RENT_S_6);
	hw_cache_sprite(SPR_RENT_S_7);
	hw_cache_sprite(SPR_RENT_S_8);

	hw_cache_sprite(SPR_RENT_W1_1);
	hw_cache_sprite(SPR_RENT_W1_2);
	hw_cache_sprite(SPR_RENT_W1_3);
	hw_cache_sprite(SPR_RENT_W1_4);
	hw_cache_sprite(SPR_RENT_W1_5);
	hw_cache_sprite(SPR_RENT_W1_6);
	hw_cache_sprite(SPR_RENT_W1_7);
	hw_cache_sprite(SPR_RENT_W1_8);

	hw_cache_sprite(SPR_RENT_W2_1);
	hw_cache_sprite(SPR_RENT_W2_2);
	hw_cache_sprite(SPR_RENT_W2_3);
	hw_cache_sprite(SPR_RENT_W2_4);
	hw_cache_sprite(SPR_RENT_W2_5);
	hw_cache_sprite(SPR_RENT_W2_6);
	hw_cache_sprite(SPR_RENT_W2_7);
	hw_cache_sprite(SPR_RENT_W2_8);

	hw_cache_sprite(SPR_RENT_W3_1);
	hw_cache_sprite(SPR_RENT_W3_2);
	hw_cache_sprite(SPR_RENT_W3_3);
	hw_cache_sprite(SPR_RENT_W3_4);
	hw_cache_sprite(SPR_RENT_W3_5);
	hw_cache_sprite(SPR_RENT_W3_6);
	hw_cache_sprite(SPR_RENT_W3_7);
	hw_cache_sprite(SPR_RENT_W3_8);

	hw_cache_sprite(SPR_RENT_W4_1);
	hw_cache_sprite(SPR_RENT_W4_2);
	hw_cache_sprite(SPR_RENT_W4_3);
	hw_cache_sprite(SPR_RENT_W4_4);
	hw_cache_sprite(SPR_RENT_W4_5);
	hw_cache_sprite(SPR_RENT_W4_6);
	hw_cache_sprite(SPR_RENT_W4_7);
	hw_cache_sprite(SPR_RENT_W4_8);

	hw_cache_sprite(SPR_RENT_DIE_1);
	hw_cache_sprite(SPR_RENT_DIE_2);
	hw_cache_sprite(SPR_RENT_DIE_3);
	hw_cache_sprite(SPR_RENT_DIE_4);
	hw_cache_sprite(SPR_RENT_PAIN_1);
	hw_cache_sprite(SPR_RENT_DEAD);

	hw_cache_sprite(SPR_RENT_SHOOT1);
	hw_cache_sprite(SPR_RENT_SHOOT2);
	hw_cache_sprite(SPR_RENT_SHOOT3);


	// Goodies.
	//
	hw_precache_slow_fire_protector();
	hw_precache_partial_charge_pack();
	hw_precache_coin_1();
}

// Robot Turret.
void hw_precache_robot_turret()
{
	hw_cache_sprite(SPR_TERROT_1);
	hw_cache_sprite(SPR_TERROT_2);
	hw_cache_sprite(SPR_TERROT_3);
	hw_cache_sprite(SPR_TERROT_4);
	hw_cache_sprite(SPR_TERROT_5);
	hw_cache_sprite(SPR_TERROT_6);
	hw_cache_sprite(SPR_TERROT_7);
	hw_cache_sprite(SPR_TERROT_8);

	hw_cache_sprite(SPR_TERROT_FIRE_1);
	hw_cache_sprite(SPR_TERROT_FIRE_2);
	hw_cache_sprite(SPR_TERROT_DIE_1);
	hw_cache_sprite(SPR_TERROT_DIE_2);
	hw_cache_sprite(SPR_TERROT_DIE_3);
	hw_cache_sprite(SPR_TERROT_DIE_4);
	hw_cache_sprite(SPR_TERROT_DEAD);
}

// Bio-Technician.
void hw_precache_bio_technician()
{
	hw_cache_sprite(SPR_OFC_S_1);
	hw_cache_sprite(SPR_OFC_S_2);
	hw_cache_sprite(SPR_OFC_S_3);
	hw_cache_sprite(SPR_OFC_S_4);
	hw_cache_sprite(SPR_OFC_S_5);
	hw_cache_sprite(SPR_OFC_S_6);
	hw_cache_sprite(SPR_OFC_S_7);
	hw_cache_sprite(SPR_OFC_S_8);

	hw_cache_sprite(SPR_OFC_W1_1);
	hw_cache_sprite(SPR_OFC_W1_2);
	hw_cache_sprite(SPR_OFC_W1_3);
	hw_cache_sprite(SPR_OFC_W1_4);
	hw_cache_sprite(SPR_OFC_W1_5);
	hw_cache_sprite(SPR_OFC_W1_6);
	hw_cache_sprite(SPR_OFC_W1_7);
	hw_cache_sprite(SPR_OFC_W1_8);

	hw_cache_sprite(SPR_OFC_W2_1);
	hw_cache_sprite(SPR_OFC_W2_2);
	hw_cache_sprite(SPR_OFC_W2_3);
	hw_cache_sprite(SPR_OFC_W2_4);
	hw_cache_sprite(SPR_OFC_W2_5);
	hw_cache_sprite(SPR_OFC_W2_6);
	hw_cache_sprite(SPR_OFC_W2_7);
	hw_cache_sprite(SPR_OFC_W2_8);

	hw_cache_sprite(SPR_OFC_W3_1);
	hw_cache_sprite(SPR_OFC_W3_2);
	hw_cache_sprite(SPR_OFC_W3_3);
	hw_cache_sprite(SPR_OFC_W3_4);
	hw_cache_sprite(SPR_OFC_W3_5);
	hw_cache_sprite(SPR_OFC_W3_6);
	hw_cache_sprite(SPR_OFC_W3_7);
	hw_cache_sprite(SPR_OFC_W3_8);

	hw_cache_sprite(SPR_OFC_W4_1);
	hw_cache_sprite(SPR_OFC_W4_2);
	hw_cache_sprite(SPR_OFC_W4_3);
	hw_cache_sprite(SPR_OFC_W4_4);
	hw_cache_sprite(SPR_OFC_W4_5);
	hw_cache_sprite(SPR_OFC_W4_6);
	hw_cache_sprite(SPR_OFC_W4_7);
	hw_cache_sprite(SPR_OFC_W4_8);

	hw_cache_sprite(SPR_OFC_PAIN_1);
	hw_cache_sprite(SPR_OFC_DIE_1);
	hw_cache_sprite(SPR_OFC_DIE_2);
	hw_cache_sprite(SPR_OFC_DIE_3);
	hw_cache_sprite(SPR_OFC_PAIN_2);
	hw_cache_sprite(SPR_OFC_DIE_4);
	hw_cache_sprite(SPR_OFC_DEAD);

	hw_cache_sprite(SPR_OFC_SHOOT1);
	hw_cache_sprite(SPR_OFC_SHOOT2);
	hw_cache_sprite(SPR_OFC_SHOOT3);


	// Goodies.
	//
	hw_precache_partial_charge_pack();
	hw_precache_coin_1();
}

// Pod Alien.
void hw_precache_pod_alien()
{
	hw_cache_sprite(SPR_POD_WALK1);
	hw_cache_sprite(SPR_POD_WALK2);
	hw_cache_sprite(SPR_POD_WALK3);
	hw_cache_sprite(SPR_POD_WALK4);
	hw_cache_sprite(SPR_POD_ATTACK1);
	hw_cache_sprite(SPR_POD_ATTACK2);
	hw_cache_sprite(SPR_POD_ATTACK3);
	hw_cache_sprite(SPR_POD_OUCH);
	hw_cache_sprite(SPR_POD_DIE1);
	hw_cache_sprite(SPR_POD_DIE2);
	hw_cache_sprite(SPR_POD_DIE3);
	hw_cache_sprite(SPR_POD_SPIT1);
	hw_cache_sprite(SPR_POD_SPIT2);
	hw_cache_sprite(SPR_POD_SPIT3);


	hw_precache_generic_alien_spit_3();
}

// Pod Alien Egg.
void hw_precache_pod_alien_egg()
{
	hw_cache_sprite(SPR_POD_EGG);
	hw_cache_sprite(SPR_POD_HATCH1);
	hw_cache_sprite(SPR_POD_HATCH2);
	hw_cache_sprite(SPR_POD_HATCH3);


	hw_precache_pod_alien();
}

// High Energy Plasma Alien.
void hw_precache_high_energy_plasma_alien()
{
	hw_cache_sprite(SPR_ELEC_APPEAR1);
	hw_cache_sprite(SPR_ELEC_APPEAR2);
	hw_cache_sprite(SPR_ELEC_APPEAR3);
	hw_cache_sprite(SPR_ELEC_WALK1);
	hw_cache_sprite(SPR_ELEC_WALK2);
	hw_cache_sprite(SPR_ELEC_WALK3);
	hw_cache_sprite(SPR_ELEC_WALK4);
	hw_cache_sprite(SPR_ELEC_OUCH);
	hw_cache_sprite(SPR_ELEC_SHOOT1);
	hw_cache_sprite(SPR_ELEC_SHOOT2);
	hw_cache_sprite(SPR_ELEC_SHOOT3);
	hw_cache_sprite(SPR_ELEC_DIE1);
	hw_cache_sprite(SPR_ELEC_DIE2);
	hw_cache_sprite(SPR_ELEC_DIE3);


	hw_precache_electrical_shot();
}

// Plasma Sphere.
void hw_precache_plasma_sphere()
{
	hw_cache_sprite(SPR_ELECTRO_SPHERE_ROAM1);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_ROAM2);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_ROAM3);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_OUCH);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_DIE1);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_DIE2);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_DIE3);
	hw_cache_sprite(SPR_ELECTRO_SPHERE_DIE4);
}

// Star Sentinel (AOG) / Tech Warrior (PS).
void hw_precache_star_sentinel_or_tech_warrior()
{
	hw_cache_sprite(SPR_PRO_S_1);
	hw_cache_sprite(SPR_PRO_S_2);
	hw_cache_sprite(SPR_PRO_S_3);
	hw_cache_sprite(SPR_PRO_S_4);
	hw_cache_sprite(SPR_PRO_S_5);
	hw_cache_sprite(SPR_PRO_S_6);
	hw_cache_sprite(SPR_PRO_S_7);
	hw_cache_sprite(SPR_PRO_S_8);

	hw_cache_sprite(SPR_PRO_W1_1);
	hw_cache_sprite(SPR_PRO_W1_2);
	hw_cache_sprite(SPR_PRO_W1_3);
	hw_cache_sprite(SPR_PRO_W1_4);
	hw_cache_sprite(SPR_PRO_W1_5);
	hw_cache_sprite(SPR_PRO_W1_6);
	hw_cache_sprite(SPR_PRO_W1_7);
	hw_cache_sprite(SPR_PRO_W1_8);

	hw_cache_sprite(SPR_PRO_W2_1);
	hw_cache_sprite(SPR_PRO_W2_2);
	hw_cache_sprite(SPR_PRO_W2_3);
	hw_cache_sprite(SPR_PRO_W2_4);
	hw_cache_sprite(SPR_PRO_W2_5);
	hw_cache_sprite(SPR_PRO_W2_6);
	hw_cache_sprite(SPR_PRO_W2_7);
	hw_cache_sprite(SPR_PRO_W2_8);

	hw_cache_sprite(SPR_PRO_W3_1);
	hw_cache_sprite(SPR_PRO_W3_2);
	hw_cache_sprite(SPR_PRO_W3_3);
	hw_cache_sprite(SPR_PRO_W3_4);
	hw_cache_sprite(SPR_PRO_W3_5);
	hw_cache_sprite(SPR_PRO_W3_6);
	hw_cache_sprite(SPR_PRO_W3_7);
	hw_cache_sprite(SPR_PRO_W3_8);

	hw_cache_sprite(SPR_PRO_W4_1);
	hw_cache_sprite(SPR_PRO_W4_2);
	hw_cache_sprite(SPR_PRO_W4_3);
	hw_cache_sprite(SPR_PRO_W4_4);
	hw_cache_sprite(SPR_PRO_W4_5);
	hw_cache_sprite(SPR_PRO_W4_6);
	hw_cache_sprite(SPR_PRO_W4_7);
	hw_cache_sprite(SPR_PRO_W4_8);

	hw_cache_sprite(SPR_PRO_PAIN_1);
	hw_cache_sprite(SPR_PRO_DIE_1);
	hw_cache_sprite(SPR_PRO_DIE_2);
	hw_cache_sprite(SPR_PRO_DIE_3);
	hw_cache_sprite(SPR_PRO_PAIN_2);
	hw_cache_sprite(SPR_PRO_DIE_4);
	hw_cache_sprite(SPR_PRO_DEAD);

	hw_cache_sprite(SPR_PRO_SHOOT1);
	hw_cache_sprite(SPR_PRO_SHOOT2);
	hw_cache_sprite(SPR_PRO_SHOOT3);


	// Goodies.
	//
	hw_precache_rapid_assault_weapon();
	hw_precache_partial_charge_pack();
	hw_precache_coin_1();
}

// High-Security Genetic Guard.
void hw_precache_high_security_genetic_guard()
{
	hw_cache_sprite(SPR_GENETIC_W1);
	hw_cache_sprite(SPR_GENETIC_W2);
	hw_cache_sprite(SPR_GENETIC_W3);
	hw_cache_sprite(SPR_GENETIC_W4);
	hw_cache_sprite(SPR_GENETIC_SWING1);
	hw_cache_sprite(SPR_GENETIC_SWING2);
	hw_cache_sprite(SPR_GENETIC_SWING3);
	hw_cache_sprite(SPR_GENETIC_DEAD);
	hw_cache_sprite(SPR_GENETIC_DIE1);
	hw_cache_sprite(SPR_GENETIC_DIE2);
	hw_cache_sprite(SPR_GENETIC_DIE3);
	hw_cache_sprite(SPR_GENETIC_DIE4);
	hw_cache_sprite(SPR_GENETIC_OUCH);
	hw_cache_sprite(SPR_GENETIC_SHOOT1);
	hw_cache_sprite(SPR_GENETIC_SHOOT2);
	hw_cache_sprite(SPR_GENETIC_SHOOT3);


	// Goodies.
	//
	hw_precache_slow_fire_protector();
	hw_precache_partial_charge_pack();
}

// Experimental Mech-Sentinel.
void hw_precache_experimental_mech_sentinel()
{
	hw_cache_sprite(SPR_MUTHUM1_W1);
	hw_cache_sprite(SPR_MUTHUM1_W2);
	hw_cache_sprite(SPR_MUTHUM1_W3);
	hw_cache_sprite(SPR_MUTHUM1_W4);
	hw_cache_sprite(SPR_MUTHUM1_SWING1);
	hw_cache_sprite(SPR_MUTHUM1_SWING2);
	hw_cache_sprite(SPR_MUTHUM1_SWING3);
	hw_cache_sprite(SPR_MUTHUM1_DEAD);
	hw_cache_sprite(SPR_MUTHUM1_DIE1);
	hw_cache_sprite(SPR_MUTHUM1_DIE2);
	hw_cache_sprite(SPR_MUTHUM1_DIE3);
	hw_cache_sprite(SPR_MUTHUM1_DIE4);
	hw_cache_sprite(SPR_MUTHUM1_OUCH);
	hw_cache_sprite(SPR_MUTHUM1_SPIT1);
	hw_cache_sprite(SPR_MUTHUM1_SPIT2);
	hw_cache_sprite(SPR_MUTHUM1_SPIT3);

	hw_precache_electrical_shot();


	// Goodies.
	//
	hw_precache_partial_charge_pack();
}

// Experimental Mutant Human.
void hw_precache_experimental_mutant_human()
{
	hw_cache_sprite(SPR_MUTHUM2_W1);
	hw_cache_sprite(SPR_MUTHUM2_W2);
	hw_cache_sprite(SPR_MUTHUM2_W3);
	hw_cache_sprite(SPR_MUTHUM2_W4);
	hw_cache_sprite(SPR_MUTHUM2_SWING1);
	hw_cache_sprite(SPR_MUTHUM2_SWING2);
	hw_cache_sprite(SPR_MUTHUM2_SWING3);
	hw_cache_sprite(SPR_MUTHUM2_DEAD);
	hw_cache_sprite(SPR_MUTHUM2_DIE1);
	hw_cache_sprite(SPR_MUTHUM2_DIE2);
	hw_cache_sprite(SPR_MUTHUM2_DIE3);
	hw_cache_sprite(SPR_MUTHUM2_DIE4);
	hw_cache_sprite(SPR_MUTHUM2_OUCH);
	hw_cache_sprite(SPR_MUTHUM2_SPIT1);
	hw_cache_sprite(SPR_MUTHUM2_SPIT2);
	hw_cache_sprite(SPR_MUTHUM2_SPIT3);


	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_precache_electrical_shot();
	}
	else
	{
		hw_precache_generic_alien_spit_1();
	}
}

// Morphing Experimental Mutant Human.
void hw_precache_experimental_mutant_human_morphing()
{
	hw_cache_sprite(SPR_MUTHUM2_MORPH1);
	hw_cache_sprite(SPR_MUTHUM2_MORPH2);
	hw_cache_sprite(SPR_MUTHUM2_MORPH3);
	hw_cache_sprite(SPR_MUTHUM2_MORPH4);
	hw_cache_sprite(SPR_MUTHUM2_MORPH5);
	hw_cache_sprite(SPR_MUTHUM2_MORPH6);
	hw_cache_sprite(SPR_MUTHUM2_MORPH7);
	hw_cache_sprite(SPR_MUTHUM2_MORPH8);
	hw_cache_sprite(SPR_MUTHUM2_MORPH9);


	hw_precache_experimental_mutant_human();
}

// Large Experimental Genetic Alien.
void hw_precache_large_experimental_genetic_alien()
{
	hw_cache_sprite(SPR_LCAN_ALIEN_W1);
	hw_cache_sprite(SPR_LCAN_ALIEN_W2);
	hw_cache_sprite(SPR_LCAN_ALIEN_W3);
	hw_cache_sprite(SPR_LCAN_ALIEN_W4);
	hw_cache_sprite(SPR_LCAN_ALIEN_SWING1);
	hw_cache_sprite(SPR_LCAN_ALIEN_SWING2);
	hw_cache_sprite(SPR_LCAN_ALIEN_SWING3);
	hw_cache_sprite(SPR_LCAN_ALIEN_DEAD);
	hw_cache_sprite(SPR_LCAN_ALIEN_DIE1);
	hw_cache_sprite(SPR_LCAN_ALIEN_DIE2);
	hw_cache_sprite(SPR_LCAN_ALIEN_DIE3);
	hw_cache_sprite(SPR_LCAN_ALIEN_DIE4);
	hw_cache_sprite(SPR_LCAN_ALIEN_OUCH);
	hw_cache_sprite(SPR_LCAN_ALIEN_SPIT1);
	hw_cache_sprite(SPR_LCAN_ALIEN_SPIT2);
	hw_cache_sprite(SPR_LCAN_ALIEN_SPIT3);


	hw_precache_generic_alien_spit_3();
}

// A canister with large Experimental Genetic Alien.
void hw_precache_canister_with_large_experimental_genetic_alien()
{
	hw_cache_sprite(SPR_LCAN_ALIEN_READY);
	hw_cache_sprite(SPR_LCAN_ALIEN_B1);
	hw_cache_sprite(SPR_LCAN_ALIEN_B2);
	hw_cache_sprite(SPR_LCAN_ALIEN_B3);
	hw_cache_sprite(SPR_LCAN_ALIEN_EMPTY);

	hw_precache_large_experimental_genetic_alien();
}

// Small Experimental Genetic Alien.
void hw_precache_experimental_genetic_alien_small()
{
	hw_cache_sprite(SPR_SCAN_ALIEN_W1);
	hw_cache_sprite(SPR_SCAN_ALIEN_W2);
	hw_cache_sprite(SPR_SCAN_ALIEN_W3);
	hw_cache_sprite(SPR_SCAN_ALIEN_W4);
	hw_cache_sprite(SPR_SCAN_ALIEN_SWING1);
	hw_cache_sprite(SPR_SCAN_ALIEN_SWING2);
	hw_cache_sprite(SPR_SCAN_ALIEN_SWING3);
	hw_cache_sprite(SPR_SCAN_ALIEN_DEAD);
	hw_cache_sprite(SPR_SCAN_ALIEN_DIE1);
	hw_cache_sprite(SPR_SCAN_ALIEN_DIE2);
	hw_cache_sprite(SPR_SCAN_ALIEN_DIE3);
	hw_cache_sprite(SPR_SCAN_ALIEN_DIE4);
	hw_cache_sprite(SPR_SCAN_ALIEN_OUCH);
	hw_cache_sprite(SPR_SCAN_ALIEN_SPIT1);
	hw_cache_sprite(SPR_SCAN_ALIEN_SPIT2);
	hw_cache_sprite(SPR_SCAN_ALIEN_SPIT3);


	hw_precache_generic_alien_spit_1();
}

// A canister with small Experimental Genetic Alien.
void hw_precache_canister_with_small_experimental_genetic_alien()
{
	hw_cache_sprite(SPR_SCAN_ALIEN_READY);
	hw_cache_sprite(SPR_SCAN_ALIEN_B1);
	hw_cache_sprite(SPR_SCAN_ALIEN_B2);
	hw_cache_sprite(SPR_SCAN_ALIEN_B3);
	hw_cache_sprite(SPR_SCAN_ALIEN_EMPTY);


	hw_precache_experimental_genetic_alien_small();
}

// Mutated Guard.
void hw_precache_mutated_guard()
{
	hw_cache_sprite(SPR_GURNEY_MUT_W1);
	hw_cache_sprite(SPR_GURNEY_MUT_W2);
	hw_cache_sprite(SPR_GURNEY_MUT_W3);
	hw_cache_sprite(SPR_GURNEY_MUT_W4);
	hw_cache_sprite(SPR_GURNEY_MUT_SWING1);
	hw_cache_sprite(SPR_GURNEY_MUT_SWING2);
	hw_cache_sprite(SPR_GURNEY_MUT_SWING3);
	hw_cache_sprite(SPR_GURNEY_MUT_DEAD);
	hw_cache_sprite(SPR_GURNEY_MUT_DIE1);
	hw_cache_sprite(SPR_GURNEY_MUT_DIE2);
	hw_cache_sprite(SPR_GURNEY_MUT_DIE3);
	hw_cache_sprite(SPR_GURNEY_MUT_DIE4);
	hw_cache_sprite(SPR_GURNEY_MUT_OUCH);


	// Goodies.
	//
	hw_precache_slow_fire_protector();
	hw_precache_partial_charge_pack();
}

// Mutated Guard (waiting).
void hw_precache_mutated_guard_waiting()
{
	hw_cache_sprite(SPR_GURNEY_MUT_READY);
	hw_cache_sprite(SPR_GURNEY_MUT_B1);
	hw_cache_sprite(SPR_GURNEY_MUT_B2);
	hw_cache_sprite(SPR_GURNEY_MUT_B3);
	hw_cache_sprite(SPR_GURNEY_MUT_EMPTY);


	hw_precache_mutated_guard();
}

// Fluid Alien Shot.
void hw_precache_fluid_alien_shot()
{
	hw_cache_sprite(SPR_LIQUID_SHOT_FLY_1);
	hw_cache_sprite(SPR_LIQUID_SHOT_FLY_2);
	hw_cache_sprite(SPR_LIQUID_SHOT_FLY_3);
	hw_cache_sprite(SPR_LIQUID_SHOT_BURST_1);
	hw_cache_sprite(SPR_LIQUID_SHOT_BURST_2);
	hw_cache_sprite(SPR_LIQUID_SHOT_BURST_3);
}

// Fluid Alien.
void hw_precache_fluid_alien()
{
	hw_cache_sprite(SPR_LIQUID_M1);
	hw_cache_sprite(SPR_LIQUID_M2);
	hw_cache_sprite(SPR_LIQUID_M3);
	hw_cache_sprite(SPR_LIQUID_R1);
	hw_cache_sprite(SPR_LIQUID_R2);
	hw_cache_sprite(SPR_LIQUID_R3);
	hw_cache_sprite(SPR_LIQUID_R4);
	hw_cache_sprite(SPR_LIQUID_S1);
	hw_cache_sprite(SPR_LIQUID_S2);
	hw_cache_sprite(SPR_LIQUID_S3);
	hw_cache_sprite(SPR_LIQUID_OUCH);
	hw_cache_sprite(SPR_LIQUID_DIE_1);
	hw_cache_sprite(SPR_LIQUID_DIE_2);
	hw_cache_sprite(SPR_LIQUID_DIE_3);
	hw_cache_sprite(SPR_LIQUID_DIE_4);
	hw_cache_sprite(SPR_LIQUID_DEAD);


	hw_precache_fluid_alien_shot();
}

// Star Trooper (AOG) / Alien Protector (PS).
void hw_precache_star_trooper_or_alien_protector()
{
	hw_cache_sprite(SPR_SWAT_S_1);
	hw_cache_sprite(SPR_SWAT_S_2);
	hw_cache_sprite(SPR_SWAT_S_3);
	hw_cache_sprite(SPR_SWAT_S_4);
	hw_cache_sprite(SPR_SWAT_S_5);
	hw_cache_sprite(SPR_SWAT_S_6);
	hw_cache_sprite(SPR_SWAT_S_7);
	hw_cache_sprite(SPR_SWAT_S_8);

	hw_cache_sprite(SPR_SWAT_W1_1);
	hw_cache_sprite(SPR_SWAT_W1_2);
	hw_cache_sprite(SPR_SWAT_W1_3);
	hw_cache_sprite(SPR_SWAT_W1_4);
	hw_cache_sprite(SPR_SWAT_W1_5);
	hw_cache_sprite(SPR_SWAT_W1_6);
	hw_cache_sprite(SPR_SWAT_W1_7);
	hw_cache_sprite(SPR_SWAT_W1_8);

	hw_cache_sprite(SPR_SWAT_W2_1);
	hw_cache_sprite(SPR_SWAT_W2_2);
	hw_cache_sprite(SPR_SWAT_W2_3);
	hw_cache_sprite(SPR_SWAT_W2_4);
	hw_cache_sprite(SPR_SWAT_W2_5);
	hw_cache_sprite(SPR_SWAT_W2_6);
	hw_cache_sprite(SPR_SWAT_W2_7);
	hw_cache_sprite(SPR_SWAT_W2_8);

	hw_cache_sprite(SPR_SWAT_W3_1);
	hw_cache_sprite(SPR_SWAT_W3_2);
	hw_cache_sprite(SPR_SWAT_W3_3);
	hw_cache_sprite(SPR_SWAT_W3_4);
	hw_cache_sprite(SPR_SWAT_W3_5);
	hw_cache_sprite(SPR_SWAT_W3_6);
	hw_cache_sprite(SPR_SWAT_W3_7);
	hw_cache_sprite(SPR_SWAT_W3_8);

	hw_cache_sprite(SPR_SWAT_W4_1);
	hw_cache_sprite(SPR_SWAT_W4_2);
	hw_cache_sprite(SPR_SWAT_W4_3);
	hw_cache_sprite(SPR_SWAT_W4_4);
	hw_cache_sprite(SPR_SWAT_W4_5);
	hw_cache_sprite(SPR_SWAT_W4_6);
	hw_cache_sprite(SPR_SWAT_W4_7);
	hw_cache_sprite(SPR_SWAT_W4_8);

	hw_cache_sprite(SPR_SWAT_PAIN_1);
	hw_cache_sprite(SPR_SWAT_DIE_1);
	hw_cache_sprite(SPR_SWAT_DIE_2);
	hw_cache_sprite(SPR_SWAT_DIE_3);
	hw_cache_sprite(SPR_SWAT_PAIN_2);
	hw_cache_sprite(SPR_SWAT_DIE_4);
	hw_cache_sprite(SPR_SWAT_DEAD);

	hw_cache_sprite(SPR_SWAT_SHOOT1);
	hw_cache_sprite(SPR_SWAT_SHOOT2);
	hw_cache_sprite(SPR_SWAT_SHOOT3);

	hw_cache_sprite(SPR_SWAT_WOUNDED1);
	hw_cache_sprite(SPR_SWAT_WOUNDED2);
	hw_cache_sprite(SPR_SWAT_WOUNDED3);
	hw_cache_sprite(SPR_SWAT_WOUNDED4);


	// Goodies.
	//
	hw_precache_rapid_assault_weapon();
	hw_precache_partial_charge_pack();
	hw_precache_coin_1();
}

// Dr. Goldfire.
void hw_precache_dr_goldfire()
{
	hw_cache_sprite(SPR_GOLD_S_1);
	hw_cache_sprite(SPR_GOLD_S_2);
	hw_cache_sprite(SPR_GOLD_S_3);
	hw_cache_sprite(SPR_GOLD_S_4);
	hw_cache_sprite(SPR_GOLD_S_5);
	hw_cache_sprite(SPR_GOLD_S_6);
	hw_cache_sprite(SPR_GOLD_S_7);
	hw_cache_sprite(SPR_GOLD_S_8);

	hw_cache_sprite(SPR_GOLD_W1_1);
	hw_cache_sprite(SPR_GOLD_W1_2);
	hw_cache_sprite(SPR_GOLD_W1_3);
	hw_cache_sprite(SPR_GOLD_W1_4);
	hw_cache_sprite(SPR_GOLD_W1_5);
	hw_cache_sprite(SPR_GOLD_W1_6);
	hw_cache_sprite(SPR_GOLD_W1_7);
	hw_cache_sprite(SPR_GOLD_W1_8);

	hw_cache_sprite(SPR_GOLD_W2_1);
	hw_cache_sprite(SPR_GOLD_W2_2);
	hw_cache_sprite(SPR_GOLD_W2_3);
	hw_cache_sprite(SPR_GOLD_W2_4);
	hw_cache_sprite(SPR_GOLD_W2_5);
	hw_cache_sprite(SPR_GOLD_W2_6);
	hw_cache_sprite(SPR_GOLD_W2_7);
	hw_cache_sprite(SPR_GOLD_W2_8);

	hw_cache_sprite(SPR_GOLD_W3_1);
	hw_cache_sprite(SPR_GOLD_W3_2);
	hw_cache_sprite(SPR_GOLD_W3_3);
	hw_cache_sprite(SPR_GOLD_W3_4);
	hw_cache_sprite(SPR_GOLD_W3_5);
	hw_cache_sprite(SPR_GOLD_W3_6);
	hw_cache_sprite(SPR_GOLD_W3_7);
	hw_cache_sprite(SPR_GOLD_W3_8);

	hw_cache_sprite(SPR_GOLD_W4_1);
	hw_cache_sprite(SPR_GOLD_W4_2);
	hw_cache_sprite(SPR_GOLD_W4_3);
	hw_cache_sprite(SPR_GOLD_W4_4);
	hw_cache_sprite(SPR_GOLD_W4_5);
	hw_cache_sprite(SPR_GOLD_W4_6);
	hw_cache_sprite(SPR_GOLD_W4_7);
	hw_cache_sprite(SPR_GOLD_W4_8);

	hw_cache_sprite(SPR_GOLD_PAIN_1);

	hw_cache_sprite(SPR_GOLD_WRIST_1);
	hw_cache_sprite(SPR_GOLD_WRIST_2);

	hw_cache_sprite(SPR_GOLD_SHOOT1);
	hw_cache_sprite(SPR_GOLD_SHOOT2);
	hw_cache_sprite(SPR_GOLD_SHOOT3);

	hw_cache_sprite(SPR_GOLD_WARP1);
	hw_cache_sprite(SPR_GOLD_WARP2);
	hw_cache_sprite(SPR_GOLD_WARP3);
	hw_cache_sprite(SPR_GOLD_WARP4);
	hw_cache_sprite(SPR_GOLD_WARP5);


	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		hw_cache_sprite(SPR_GOLD_DEATH1);
		hw_cache_sprite(SPR_GOLD_DEATH2);
		hw_cache_sprite(SPR_GOLD_DEATH3);
		hw_cache_sprite(SPR_GOLD_DEATH4);
		hw_cache_sprite(SPR_GOLD_DEATH5);
	}


	// Goodies.
	//
	hw_precache_golden_access_card();
}

// Morphed Dr. Goldfire Shot.
void hw_precache_morphed_dr_goldfire_shot()
{
	hw_cache_sprite(SPR_MGOLD_SHOT1);
	hw_cache_sprite(SPR_MGOLD_SHOT2);
	hw_cache_sprite(SPR_MGOLD_SHOT3);
	hw_cache_sprite(SPR_MGOLD_SHOT_EXP1);
	hw_cache_sprite(SPR_MGOLD_SHOT_EXP2);
	hw_cache_sprite(SPR_MGOLD_SHOT_EXP3);
}

// Morphed Dr. Goldfire.
void hw_precache_morphed_dr_goldfire()
{
	hw_cache_sprite(SPR_MGOLD_OUCH);

	hw_cache_sprite(SPR_GOLD_MORPH1);
	hw_cache_sprite(SPR_GOLD_MORPH2);
	hw_cache_sprite(SPR_GOLD_MORPH3);
	hw_cache_sprite(SPR_GOLD_MORPH4);
	hw_cache_sprite(SPR_GOLD_MORPH5);
	hw_cache_sprite(SPR_GOLD_MORPH6);
	hw_cache_sprite(SPR_GOLD_MORPH7);
	hw_cache_sprite(SPR_GOLD_MORPH8);

	hw_cache_sprite(SPR_MGOLD_WALK1);
	hw_cache_sprite(SPR_MGOLD_WALK2);
	hw_cache_sprite(SPR_MGOLD_WALK3);
	hw_cache_sprite(SPR_MGOLD_WALK4);
	hw_cache_sprite(SPR_MGOLD_ATTACK1);
	hw_cache_sprite(SPR_MGOLD_ATTACK2);
	hw_cache_sprite(SPR_MGOLD_ATTACK3);
	hw_cache_sprite(SPR_MGOLD_ATTACK4);


	hw_precache_morphed_dr_goldfire_shot();


	// Goodies.
	//
	hw_precache_golden_access_card();
}

// Volatile Material Transport.
void hw_precache_volatile_material_transport()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw())
	{
		return;
	}

	hw_cache_sprite(SPR_GSCOUT_W1_1);
	hw_cache_sprite(SPR_GSCOUT_W1_2);
	hw_cache_sprite(SPR_GSCOUT_W1_3);
	hw_cache_sprite(SPR_GSCOUT_W1_4);
	hw_cache_sprite(SPR_GSCOUT_W1_5);
	hw_cache_sprite(SPR_GSCOUT_W1_6);
	hw_cache_sprite(SPR_GSCOUT_W1_7);
	hw_cache_sprite(SPR_GSCOUT_W1_8);

	hw_cache_sprite(SPR_GSCOUT_W2_1);
	hw_cache_sprite(SPR_GSCOUT_W2_2);
	hw_cache_sprite(SPR_GSCOUT_W2_3);
	hw_cache_sprite(SPR_GSCOUT_W2_4);
	hw_cache_sprite(SPR_GSCOUT_W2_5);
	hw_cache_sprite(SPR_GSCOUT_W2_6);
	hw_cache_sprite(SPR_GSCOUT_W2_7);
	hw_cache_sprite(SPR_GSCOUT_W2_8);

	hw_cache_sprite(SPR_GSCOUT_W3_1);
	hw_cache_sprite(SPR_GSCOUT_W3_2);
	hw_cache_sprite(SPR_GSCOUT_W3_3);
	hw_cache_sprite(SPR_GSCOUT_W3_4);
	hw_cache_sprite(SPR_GSCOUT_W3_5);
	hw_cache_sprite(SPR_GSCOUT_W3_6);
	hw_cache_sprite(SPR_GSCOUT_W3_7);
	hw_cache_sprite(SPR_GSCOUT_W3_8);

	hw_cache_sprite(SPR_GSCOUT_W4_1);
	hw_cache_sprite(SPR_GSCOUT_W4_2);
	hw_cache_sprite(SPR_GSCOUT_W4_3);
	hw_cache_sprite(SPR_GSCOUT_W4_4);
	hw_cache_sprite(SPR_GSCOUT_W4_5);
	hw_cache_sprite(SPR_GSCOUT_W4_6);
	hw_cache_sprite(SPR_GSCOUT_W4_7);
	hw_cache_sprite(SPR_GSCOUT_W4_8);

	hw_cache_sprite(SPR_GSCOUT_DIE1);
	hw_cache_sprite(SPR_GSCOUT_DIE2);
	hw_cache_sprite(SPR_GSCOUT_DIE3);
	hw_cache_sprite(SPR_GSCOUT_DIE4);
	hw_cache_sprite(SPR_GSCOUT_DIE5);
	hw_cache_sprite(SPR_GSCOUT_DIE6);
	hw_cache_sprite(SPR_GSCOUT_DIE7);
	hw_cache_sprite(SPR_GSCOUT_DIE8);

	hw_cache_sprite(SPR_GSCOUT_DEAD);


	//
	hw_precache_explosion();
	hw_precache_toxic_waste_green_1();
}

void hw_precache_perscan_drone()
{
	hw_cache_sprite(SPR_FSCOUT_W1_1);
	hw_cache_sprite(SPR_FSCOUT_W1_2);
	hw_cache_sprite(SPR_FSCOUT_W1_3);
	hw_cache_sprite(SPR_FSCOUT_W1_4);
	hw_cache_sprite(SPR_FSCOUT_W1_5);
	hw_cache_sprite(SPR_FSCOUT_W1_6);
	hw_cache_sprite(SPR_FSCOUT_W1_7);
	hw_cache_sprite(SPR_FSCOUT_W1_8);

	const auto& assets_info = get_assets_info();

	if (!assets_info.is_aog_sw())
	{
		hw_cache_sprite(SPR_FSCOUT_W2_1);
		hw_cache_sprite(SPR_FSCOUT_W2_2);
		hw_cache_sprite(SPR_FSCOUT_W2_3);
		hw_cache_sprite(SPR_FSCOUT_W2_4);
		hw_cache_sprite(SPR_FSCOUT_W2_5);
		hw_cache_sprite(SPR_FSCOUT_W2_6);
		hw_cache_sprite(SPR_FSCOUT_W2_7);
		hw_cache_sprite(SPR_FSCOUT_W2_8);

		hw_cache_sprite(SPR_FSCOUT_W3_1);
		hw_cache_sprite(SPR_FSCOUT_W3_2);
		hw_cache_sprite(SPR_FSCOUT_W3_3);
		hw_cache_sprite(SPR_FSCOUT_W3_4);
		hw_cache_sprite(SPR_FSCOUT_W3_5);
		hw_cache_sprite(SPR_FSCOUT_W3_6);
		hw_cache_sprite(SPR_FSCOUT_W3_7);
		hw_cache_sprite(SPR_FSCOUT_W3_8);

		hw_cache_sprite(SPR_FSCOUT_W4_1);
		hw_cache_sprite(SPR_FSCOUT_W4_2);
		hw_cache_sprite(SPR_FSCOUT_W4_3);
		hw_cache_sprite(SPR_FSCOUT_W4_4);
		hw_cache_sprite(SPR_FSCOUT_W4_5);
		hw_cache_sprite(SPR_FSCOUT_W4_6);
		hw_cache_sprite(SPR_FSCOUT_W4_7);
		hw_cache_sprite(SPR_FSCOUT_W4_8);
	}

	hw_cache_sprite(SPR_FSCOUT_DIE1);
	hw_cache_sprite(SPR_FSCOUT_DIE2);
	hw_cache_sprite(SPR_FSCOUT_DIE3);
	hw_cache_sprite(SPR_FSCOUT_DIE4);
	hw_cache_sprite(SPR_FSCOUT_DIE5);
	hw_cache_sprite(SPR_FSCOUT_DIE6);
	hw_cache_sprite(SPR_FSCOUT_DIE7);
	hw_cache_sprite(SPR_FSCOUT_DEAD);

	//
	hw_precache_explosion();
}

// Security Cube Explosion.
void hw_precache_security_cube_explosion()
{
	hw_cache_sprite(SPR_CUBE_EXP1);
	hw_cache_sprite(SPR_CUBE_EXP2);
	hw_cache_sprite(SPR_CUBE_EXP3);
	hw_cache_sprite(SPR_CUBE_EXP4);
	hw_cache_sprite(SPR_CUBE_EXP5);
	hw_cache_sprite(SPR_CUBE_EXP6);
	hw_cache_sprite(SPR_CUBE_EXP7);
	hw_cache_sprite(SPR_CUBE_EXP8);
}

// Security Cube.
void hw_precache_security_cube_or_projection_generator()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		hw_cache_sprite(SPR_VITAL_STAND);
		hw_cache_sprite(SPR_VITAL_DIE_1);
		hw_cache_sprite(SPR_VITAL_DIE_2);
		hw_cache_sprite(SPR_VITAL_DIE_3);
		hw_cache_sprite(SPR_VITAL_DIE_4);
		hw_cache_sprite(SPR_VITAL_DIE_5);
		hw_cache_sprite(SPR_VITAL_DIE_6);
		hw_cache_sprite(SPR_VITAL_DIE_7);
		hw_cache_sprite(SPR_VITAL_DIE_8);
		hw_cache_sprite(SPR_VITAL_DEAD_1);
		hw_cache_sprite(SPR_VITAL_DEAD_2);
		hw_cache_sprite(SPR_VITAL_DEAD_3);
		hw_cache_sprite(SPR_VITAL_OUCH);


		hw_precache_explosion();
	}
	else
	{
		hw_cache_sprite(SPR_CUBE1);
		hw_cache_sprite(SPR_CUBE2);
		hw_cache_sprite(SPR_CUBE3);
		hw_cache_sprite(SPR_CUBE4);
		hw_cache_sprite(SPR_CUBE5);
		hw_cache_sprite(SPR_CUBE6);
		hw_cache_sprite(SPR_CUBE7);
		hw_cache_sprite(SPR_CUBE8);
		hw_cache_sprite(SPR_CUBE9);
		hw_cache_sprite(SPR_CUBE10);
		hw_cache_sprite(SPR_DEAD_CUBE);


		hw_precache_security_cube_explosion();
	}
}

// Spider Mutant Shot.
void hw_precache_spider_mutant_shot()
{
	hw_cache_sprite(SPR_BOSS1_PROJ1);
	hw_cache_sprite(SPR_BOSS1_PROJ2);
	hw_cache_sprite(SPR_BOSS1_PROJ3);
	hw_cache_sprite(SPR_BOSS1_EXP1);
	hw_cache_sprite(SPR_BOSS1_EXP2);
	hw_cache_sprite(SPR_BOSS1_EXP3);
}

// Spider Mutant.
void hw_precache_spider_mutant()
{
	hw_cache_sprite(SPR_BOSS1_W1);
	hw_cache_sprite(SPR_BOSS1_W2);
	hw_cache_sprite(SPR_BOSS1_W3);
	hw_cache_sprite(SPR_BOSS1_W4);
	hw_cache_sprite(SPR_BOSS1_SWING1);
	hw_cache_sprite(SPR_BOSS1_SWING2);
	hw_cache_sprite(SPR_BOSS1_SWING3);
	hw_cache_sprite(SPR_BOSS1_DEAD);
	hw_cache_sprite(SPR_BOSS1_DIE1);
	hw_cache_sprite(SPR_BOSS1_DIE2);
	hw_cache_sprite(SPR_BOSS1_DIE3);
	hw_cache_sprite(SPR_BOSS1_DIE4);
	hw_cache_sprite(SPR_BOSS1_OUCH);
	hw_cache_sprite(SPR_BOSS1_PROJ1);
	hw_cache_sprite(SPR_BOSS1_PROJ2);
	hw_cache_sprite(SPR_BOSS1_PROJ3);
	hw_cache_sprite(SPR_BOSS1_EXP1);
	hw_cache_sprite(SPR_BOSS1_EXP2);
	hw_cache_sprite(SPR_BOSS1_EXP3);


	hw_precache_spider_mutant_shot();
}

// Morphing Spider Mutant.
void hw_precache_spider_mutant_morphing()
{
	hw_cache_sprite(SPR_BOSS1_MORPH1);
	hw_cache_sprite(SPR_BOSS1_MORPH2);
	hw_cache_sprite(SPR_BOSS1_MORPH3);
	hw_cache_sprite(SPR_BOSS1_MORPH4);
	hw_cache_sprite(SPR_BOSS1_MORPH5);
	hw_cache_sprite(SPR_BOSS1_MORPH6);
	hw_cache_sprite(SPR_BOSS1_MORPH7);
	hw_cache_sprite(SPR_BOSS1_MORPH8);
	hw_cache_sprite(SPR_BOSS1_MORPH9);


	hw_precache_spider_mutant();
}

// Breather Beast.
void hw_precache_breather_beast()
{
	hw_cache_sprite(SPR_BOSS2_W1);
	hw_cache_sprite(SPR_BOSS2_W2);
	hw_cache_sprite(SPR_BOSS2_W3);
	hw_cache_sprite(SPR_BOSS2_W4);
	hw_cache_sprite(SPR_BOSS2_SWING1);
	hw_cache_sprite(SPR_BOSS2_SWING2);
	hw_cache_sprite(SPR_BOSS2_SWING3);
	hw_cache_sprite(SPR_BOSS2_DEAD);
	hw_cache_sprite(SPR_BOSS2_DIE1);
	hw_cache_sprite(SPR_BOSS2_DIE2);
	hw_cache_sprite(SPR_BOSS2_DIE3);
	hw_cache_sprite(SPR_BOSS2_DIE4);
	hw_cache_sprite(SPR_BOSS2_OUCH);
}

// Cyborg Warrior.
void hw_precache_cyborg_warrior()
{
	hw_cache_sprite(SPR_BOSS3_W1);
	hw_cache_sprite(SPR_BOSS3_W2);
	hw_cache_sprite(SPR_BOSS3_W3);
	hw_cache_sprite(SPR_BOSS3_W4);
	hw_cache_sprite(SPR_BOSS3_SWING1);
	hw_cache_sprite(SPR_BOSS3_SWING2);
	hw_cache_sprite(SPR_BOSS3_SWING3);
	hw_cache_sprite(SPR_BOSS3_DEAD);
	hw_cache_sprite(SPR_BOSS3_DIE1);
	hw_cache_sprite(SPR_BOSS3_DIE2);
	hw_cache_sprite(SPR_BOSS3_DIE3);
	hw_cache_sprite(SPR_BOSS3_DIE4);
	hw_cache_sprite(SPR_BOSS3_OUCH);
}

// Reptilian Warrior.
void hw_precache_reptilian_warrior()
{
	hw_cache_sprite(SPR_BOSS4_W1);
	hw_cache_sprite(SPR_BOSS4_W2);
	hw_cache_sprite(SPR_BOSS4_W3);
	hw_cache_sprite(SPR_BOSS4_W4);
	hw_cache_sprite(SPR_BOSS4_SWING1);
	hw_cache_sprite(SPR_BOSS4_SWING2);
	hw_cache_sprite(SPR_BOSS4_SWING3);
	hw_cache_sprite(SPR_BOSS4_DEAD);
	hw_cache_sprite(SPR_BOSS4_DIE1);
	hw_cache_sprite(SPR_BOSS4_DIE2);
	hw_cache_sprite(SPR_BOSS4_DIE3);
	hw_cache_sprite(SPR_BOSS4_DIE4);
	hw_cache_sprite(SPR_BOSS4_OUCH);
}

// Reptilian Warrior (morphing).
void hw_precache_reptilian_warrior_morphing()
{
	hw_cache_sprite(SPR_BOSS4_MORPH1);
	hw_cache_sprite(SPR_BOSS4_MORPH2);
	hw_cache_sprite(SPR_BOSS4_MORPH3);
	hw_cache_sprite(SPR_BOSS4_MORPH4);
	hw_cache_sprite(SPR_BOSS4_MORPH5);
	hw_cache_sprite(SPR_BOSS4_MORPH6);
	hw_cache_sprite(SPR_BOSS4_MORPH7);
	hw_cache_sprite(SPR_BOSS4_MORPH8);
	hw_cache_sprite(SPR_BOSS4_MORPH9);


	hw_precache_reptilian_warrior();
}

// Acid Dragon Shot.
void hw_precache_acid_dragon_shot()
{
	hw_cache_sprite(SPR_BOSS5_PROJ1);
	hw_cache_sprite(SPR_BOSS5_PROJ2);
	hw_cache_sprite(SPR_BOSS5_PROJ3);
	hw_cache_sprite(SPR_BOSS5_EXP1);
	hw_cache_sprite(SPR_BOSS5_EXP2);
	hw_cache_sprite(SPR_BOSS5_EXP3);
}

// Acid Dragon.
void hw_precache_acid_dragon()
{
	hw_cache_sprite(SPR_BOSS5_W1);
	hw_cache_sprite(SPR_BOSS5_W2);
	hw_cache_sprite(SPR_BOSS5_W3);
	hw_cache_sprite(SPR_BOSS5_W4);
	hw_cache_sprite(SPR_BOSS5_SWING1);
	hw_cache_sprite(SPR_BOSS5_SWING2);
	hw_cache_sprite(SPR_BOSS5_SWING3);
	hw_cache_sprite(SPR_BOSS5_DEAD);
	hw_cache_sprite(SPR_BOSS5_DIE1);
	hw_cache_sprite(SPR_BOSS5_DIE2);
	hw_cache_sprite(SPR_BOSS5_DIE3);
	hw_cache_sprite(SPR_BOSS5_DIE4);
	hw_cache_sprite(SPR_BOSS5_OUCH);


	hw_precache_acid_dragon_shot();
}

// Bio-Mech Guardian.
void hw_precache_bio_mech_guardian()
{
	hw_cache_sprite(SPR_BOSS6_W1);
	hw_cache_sprite(SPR_BOSS6_W2);
	hw_cache_sprite(SPR_BOSS6_W3);
	hw_cache_sprite(SPR_BOSS6_W4);
	hw_cache_sprite(SPR_BOSS6_SWING1);
	hw_cache_sprite(SPR_BOSS6_SWING2);
	hw_cache_sprite(SPR_BOSS6_SWING3);
	hw_cache_sprite(SPR_BOSS6_DEAD);
	hw_cache_sprite(SPR_BOSS6_DIE1);
	hw_cache_sprite(SPR_BOSS6_DIE2);
	hw_cache_sprite(SPR_BOSS6_DIE3);
	hw_cache_sprite(SPR_BOSS6_DIE4);
	hw_cache_sprite(SPR_BOSS6_OUCH);
}

// The Giant Stalker.
void hw_precache_the_giant_stalker()
{
	hw_cache_sprite(SPR_BOSS7_W1);
	hw_cache_sprite(SPR_BOSS7_W2);
	hw_cache_sprite(SPR_BOSS7_W3);
	hw_cache_sprite(SPR_BOSS7_W4);
	hw_cache_sprite(SPR_BOSS7_SHOOT1);
	hw_cache_sprite(SPR_BOSS7_SHOOT2);
	hw_cache_sprite(SPR_BOSS7_SHOOT3);
	hw_cache_sprite(SPR_BOSS7_DEAD);
	hw_cache_sprite(SPR_BOSS7_DIE1);
	hw_cache_sprite(SPR_BOSS7_DIE2);
	hw_cache_sprite(SPR_BOSS7_DIE3);
	hw_cache_sprite(SPR_BOSS7_DIE4);
	hw_cache_sprite(SPR_BOSS7_OUCH);
}

// The Spector Demon.
void hw_precache_the_spector_demon()
{
	hw_cache_sprite(SPR_BOSS8_W1);
	hw_cache_sprite(SPR_BOSS8_W2);
	hw_cache_sprite(SPR_BOSS8_W3);
	hw_cache_sprite(SPR_BOSS8_W4);
	hw_cache_sprite(SPR_BOSS8_SHOOT1);
	hw_cache_sprite(SPR_BOSS8_SHOOT2);
	hw_cache_sprite(SPR_BOSS8_SHOOT3);
	hw_cache_sprite(SPR_BOSS8_DIE1);
	hw_cache_sprite(SPR_BOSS8_DIE2);
	hw_cache_sprite(SPR_BOSS8_DIE3);
	hw_cache_sprite(SPR_BOSS8_DIE4);
	hw_cache_sprite(SPR_BOSS8_DEAD);
	hw_cache_sprite(SPR_BOSS8_OUCH);


	//
	hw_precache_morphed_dr_goldfire_shot();
}

// The Armored Stalker.
void hw_precache_the_armored_stalker()
{
	hw_cache_sprite(SPR_BOSS9_W1);
	hw_cache_sprite(SPR_BOSS9_W2);
	hw_cache_sprite(SPR_BOSS9_W3);
	hw_cache_sprite(SPR_BOSS9_W4);
	hw_cache_sprite(SPR_BOSS9_SHOOT1);
	hw_cache_sprite(SPR_BOSS9_SHOOT2);
	hw_cache_sprite(SPR_BOSS9_SHOOT3);
	hw_cache_sprite(SPR_BOSS9_DIE1);
	hw_cache_sprite(SPR_BOSS9_DIE2);
	hw_cache_sprite(SPR_BOSS9_DIE3);
	hw_cache_sprite(SPR_BOSS9_DIE4);
	hw_cache_sprite(SPR_BOSS9_DEAD);
	hw_cache_sprite(SPR_BOSS9_OUCH);
}

// The Crawler Beast Shot.
void hw_precache_the_crawler_beast_shot()
{
	hw_cache_sprite(SPR_BOSS10_SPIT1);
	hw_cache_sprite(SPR_BOSS10_SPIT2);
	hw_cache_sprite(SPR_BOSS10_SPIT3);

	hw_cache_sprite(SPR_BOSS10_SPIT_EXP1);
	hw_cache_sprite(SPR_BOSS10_SPIT_EXP2);
	hw_cache_sprite(SPR_BOSS10_SPIT_EXP3);
}

// The Crawler Beast.
void hw_precache_the_crawler_beast()
{
	hw_cache_sprite(SPR_BOSS10_W1);
	hw_cache_sprite(SPR_BOSS10_W2);
	hw_cache_sprite(SPR_BOSS10_W3);
	hw_cache_sprite(SPR_BOSS10_W4);
	hw_cache_sprite(SPR_BOSS10_SHOOT1);
	hw_cache_sprite(SPR_BOSS10_SHOOT2);
	hw_cache_sprite(SPR_BOSS10_SHOOT3);
	hw_cache_sprite(SPR_BOSS10_DEAD);
	hw_cache_sprite(SPR_BOSS10_DIE1);
	hw_cache_sprite(SPR_BOSS10_DIE2);
	hw_cache_sprite(SPR_BOSS10_DIE3);
	hw_cache_sprite(SPR_BOSS10_DIE4);
	hw_cache_sprite(SPR_BOSS10_OUCH);


	hw_precache_the_crawler_beast_shot();
}

// Blake Stone.
void hw_precache_blake_stone()
{
	hw_cache_sprite(SPR_BLAKE_W1);
	hw_cache_sprite(SPR_BLAKE_W2);
	hw_cache_sprite(SPR_BLAKE_W3);
	hw_cache_sprite(SPR_BLAKE_W4);
}

void hw_precache_vent_and_dripping_blood()
{
	hw_cache_sprite(SPR_BLOOD_DRIP1);
	hw_cache_sprite(SPR_BLOOD_DRIP2);
	hw_cache_sprite(SPR_BLOOD_DRIP3);
	hw_cache_sprite(SPR_BLOOD_DRIP4);
}

void hw_precache_vent_and_dripping_water()
{
	hw_cache_sprite(SPR_WATER_DRIP1);
	hw_cache_sprite(SPR_WATER_DRIP2);
	hw_cache_sprite(SPR_WATER_DRIP3);
	hw_cache_sprite(SPR_WATER_DRIP4);
}

void hw_precache_flicker_light()
{
	hw_cache_sprite(SPR_DECO_ARC_1);
	hw_cache_sprite(SPR_DECO_ARC_2);
	hw_cache_sprite(SPR_DECO_ARC_3);
}

void hw_precache_crate_content()
{
	hw_precache_chicken_leg();
	hw_precache_ham();

	hw_precache_charge_pack();

	hw_precache_slow_fire_protector();
	hw_precache_rapid_assault_weapon();
	hw_precache_dual_neutron_disruptor_weapon();
	hw_precache_plasma_discharge_unit_weapon();
	hw_precache_anti_plasma_cannon_weapon();

	hw_precache_small_yellow_box();

	hw_cache_sprite(SPR_STAT_48); // money bag
	hw_cache_sprite(SPR_STAT_49); // loot
	hw_cache_sprite(SPR_STAT_50); // gold
	hw_cache_sprite(SPR_STAT_51); // bonus
	hw_cache_sprite(SPR_STAT_57); // Body Parts
}

void hw_precache_crate_1()
{
	hw_cache_sprite(SPR_CRATE_1);


	// Goodies.
	//
	hw_precache_grenade_explosion();
	hw_precache_crate_content();
}

void hw_precache_crate_2()
{
	hw_cache_sprite(SPR_CRATE_2);


	// Goodies.
	//
	hw_precache_grenade_explosion();
	hw_precache_crate_content();
}

void hw_precache_crate_3()
{
	hw_cache_sprite(SPR_CRATE_3);


	// Goodies.
	//
	hw_precache_grenade_explosion();
	hw_precache_crate_content();
}

void hw_precache_electrical_post_barrier()
{
	hw_cache_sprite(SPR_ELEC_POST1);
	hw_cache_sprite(SPR_ELEC_POST2);
	hw_cache_sprite(SPR_ELEC_POST3);
	hw_cache_sprite(SPR_ELEC_POST4);
}

void hw_precache_electrical_arc_barrier()
{
	hw_cache_sprite(SPR_ELEC_ARC1);
	hw_cache_sprite(SPR_ELEC_ARC2);
	hw_cache_sprite(SPR_ELEC_ARC3);
	hw_cache_sprite(SPR_ELEC_ARC4);
}

void hw_precache_vertical_post_barrier()
{
	hw_cache_sprite(SPR_VPOST1);
	hw_cache_sprite(SPR_VPOST2);
	hw_cache_sprite(SPR_VPOST3);
	hw_cache_sprite(SPR_VPOST4);
	hw_cache_sprite(SPR_VPOST5);
	hw_cache_sprite(SPR_VPOST6);
	hw_cache_sprite(SPR_VPOST7);
	hw_cache_sprite(SPR_VPOST8);
}

void hw_precache_vertical_spike_barrier()
{
	hw_cache_sprite(SPR_VSPIKE1);
	hw_cache_sprite(SPR_VSPIKE2);
	hw_cache_sprite(SPR_VSPIKE3);
	hw_cache_sprite(SPR_VSPIKE4);
	hw_cache_sprite(SPR_VSPIKE5);
	hw_cache_sprite(SPR_VSPIKE6);
	hw_cache_sprite(SPR_VSPIKE7);
	hw_cache_sprite(SPR_VSPIKE8);
}

void hw_precache_security_light()
{
	hw_cache_sprite(SPR_SECURITY_NORMAL);
	hw_cache_sprite(SPR_SECURITY_ALERT);
}

void hw_precache_grate_and_steam()
{
	hw_cache_sprite(SPR_GRATE);
	hw_cache_sprite(SPR_STEAM_1);
	hw_cache_sprite(SPR_STEAM_2);
	hw_cache_sprite(SPR_STEAM_3);
	hw_cache_sprite(SPR_STEAM_4);
}

void hw_precache_pipe_and_steam()
{
	hw_cache_sprite(SPR_STEAM_PIPE);
	hw_cache_sprite(SPR_PIPE_STEAM_1);
	hw_cache_sprite(SPR_PIPE_STEAM_2);
	hw_cache_sprite(SPR_PIPE_STEAM_3);
	hw_cache_sprite(SPR_PIPE_STEAM_4);
}

void hw_precache_special_stuff()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		if (gamestate.mapon == 9)
		{
			hw_precache_blake_stone();
			hw_precache_dr_goldfire();
		}
	}
	else if (assets_info.is_ps())
	{
		if (gamestate.mapon == 19)
		{
			hw_precache_morphed_dr_goldfire();
		}
	}

	if (GoldsternInfo.SpawnCnt > 0)
	{
		hw_precache_dr_goldfire();
	}

	if (NumEAWalls > 0)
	{
		hw_precache_high_energy_plasma_alien();
	}
}

void hw_precache_access_cards()
{
	hw_precache_red_access_card();
	hw_precache_yellow_access_card();
	hw_precache_green_access_card();
	hw_precache_blue_access_card();
	hw_precache_golden_access_card();
}

void hw_precache_player_weapon_auto_charge_pistol()
{
	hw_cache_sprite(SPR_KNIFEREADY);
	hw_cache_sprite(SPR_KNIFEATK1);
	hw_cache_sprite(SPR_KNIFEATK2);
	hw_cache_sprite(SPR_KNIFEATK3);
	hw_cache_sprite(SPR_KNIFEATK4);
}

void hw_precache_player_weapon_slow_fire_protector()
{
	hw_cache_sprite(SPR_PISTOLREADY);
	hw_cache_sprite(SPR_PISTOLATK1);
	hw_cache_sprite(SPR_PISTOLATK2);
	hw_cache_sprite(SPR_PISTOLATK3);
	hw_cache_sprite(SPR_PISTOLATK4);
}

void hw_precache_player_weapon_rapid_assault_weapon()
{
	hw_cache_sprite(SPR_MACHINEGUNREADY);
	hw_cache_sprite(SPR_MACHINEGUNATK1);
	hw_cache_sprite(SPR_MACHINEGUNATK2);
	hw_cache_sprite(SPR_MACHINEGUNATK3);
	hw_cache_sprite(SPR_MACHINEGUNATK4);
}

void hw_precache_player_weapon_dual_neutron_disruptor()
{
	hw_cache_sprite(SPR_CHAINREADY);
	hw_cache_sprite(SPR_CHAINATK1);
	hw_cache_sprite(SPR_CHAINATK2);
	hw_cache_sprite(SPR_CHAINATK3);
	hw_cache_sprite(SPR_CHAINATK4);
}

void hw_precache_player_weapon_plasma_discharge_unit()
{
	hw_cache_sprite(SPR_GRENADEREADY);
	hw_cache_sprite(SPR_GRENADEATK1);
	hw_cache_sprite(SPR_GRENADEATK2);
	hw_cache_sprite(SPR_GRENADEATK3);
	hw_cache_sprite(SPR_GRENADEATK4);

	hw_precache_flying_grenade();
	hw_precache_grenade_explosion();
	hw_precache_explosion();
}

void hw_precache_player_weapon_anti_plasma_cannon()
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		return;
	}

	hw_cache_sprite(SPR_BFG_WEAPON1);
	hw_cache_sprite(SPR_BFG_WEAPON1);
	hw_cache_sprite(SPR_BFG_WEAPON2);
	hw_cache_sprite(SPR_BFG_WEAPON3);
	hw_cache_sprite(SPR_BFG_WEAPON4);
	hw_cache_sprite(SPR_BFG_WEAPON5);

	hw_precache_anti_plasma_cannon_shot();
	hw_precache_anti_plasma_cannon_explosion();

	hw_precache_rubble();

	hw_precache_explosion();
	hw_precache_clip_explosion();
}

void hw_precache_player_weapons()
{
	hw_precache_player_weapon_auto_charge_pistol();
	hw_precache_player_weapon_slow_fire_protector();
	hw_precache_player_weapon_rapid_assault_weapon();
	hw_precache_player_weapon_dual_neutron_disruptor();
	hw_precache_player_weapon_plasma_discharge_unit();
	hw_precache_player_weapon_anti_plasma_cannon();
}

void hw_precache_dead(
	const objtype& bs_actor)
{
	const auto sprite_id = hw_get_bs_actor_sprite_id(bs_actor);

	hw_cache_sprite(sprite_id);
}

void hw_precache_actors()
{
	if (!player)
	{
		return;
	}

	for (auto bs_actor = player->next; bs_actor; bs_actor = bs_actor->next)
	{
		switch (bs_actor->obclass)
		{
		case nothing:
			break;

		case rentacopobj:
			hw_precache_sector_patrol_or_sector_guard();
			break;

		case hang_terrotobj:
			hw_precache_robot_turret();
			break;

		case gen_scientistobj:
			hw_precache_bio_technician();
			break;

		case podobj:
			hw_precache_pod_alien();
			break;

		case electroobj:
			hw_precache_high_energy_plasma_alien();
			break;

		case electrosphereobj:
			hw_precache_plasma_sphere();
			break;

		case proguardobj:
			hw_precache_star_sentinel_or_tech_warrior();
			break;

		case genetic_guardobj:
			hw_precache_high_security_genetic_guard();
			break;

		case mutant_human1obj:
			hw_precache_experimental_mech_sentinel();
			break;

		case mutant_human2obj:
			hw_precache_experimental_mutant_human();
			break;

		case lcan_wait_alienobj:
			hw_precache_canister_with_large_experimental_genetic_alien();
			break;

		case lcan_alienobj:
			hw_precache_large_experimental_genetic_alien();
			break;

		case scan_wait_alienobj:
			hw_precache_canister_with_small_experimental_genetic_alien();
			break;

		case scan_alienobj:
			hw_precache_experimental_genetic_alien_small();
			break;

		case gurney_waitobj:
			hw_precache_mutated_guard_waiting();
			break;

		case gurneyobj:
			hw_precache_mutated_guard();
			break;

		case liquidobj:
			hw_precache_fluid_alien();
			break;

		case swatobj:
			hw_precache_star_trooper_or_alien_protector();
			break;

		case goldsternobj:
			hw_precache_dr_goldfire();
			break;

		case gold_morphobj:
			hw_precache_morphed_dr_goldfire();
			break;

		case volatiletransportobj:
			hw_precache_volatile_material_transport();
			break;

		case floatingbombobj:
			hw_precache_perscan_drone();
			break;

		case rotating_cubeobj:
			hw_precache_security_cube_or_projection_generator();
			break;

		case spider_mutantobj:
			hw_precache_spider_mutant();
			break;

		case breather_beastobj:
			hw_precache_breather_beast();
			break;

		case cyborg_warriorobj:
			hw_precache_cyborg_warrior();
			break;

		case reptilian_warriorobj:
			hw_precache_reptilian_warrior();
			break;

		case acid_dragonobj:
			hw_precache_acid_dragon();
			break;

		case mech_guardianobj:
			hw_precache_bio_mech_guardian();
			break;

		case final_boss1obj:
			hw_precache_the_giant_stalker();
			break;

		case final_boss2obj:
			hw_precache_the_spector_demon();
			break;

		case final_boss3obj:
			hw_precache_the_armored_stalker();
			break;

		case final_boss4obj:
			hw_precache_the_crawler_beast();
			break;

		case blakeobj:
			hw_precache_blake_stone();
			break;

		case crate1obj:
			hw_precache_crate_1();
			break;

		case crate2obj:
			hw_precache_crate_2();
			break;

		case crate3obj:
			hw_precache_crate_3();
			break;

		case green_oozeobj:
			hw_precache_toxic_waste_green_1();
			break;

		case black_oozeobj:
			hw_precache_toxic_waste_black_1();
			break;

		case green2_oozeobj:
			hw_precache_toxic_waste_green_2();
			break;

		case black2_oozeobj:
			hw_precache_toxic_waste_black_2();
			break;

		case podeggobj:
			hw_precache_pod_alien_egg();
			break;

		case morphing_spider_mutantobj:
			hw_precache_spider_mutant_morphing();
			break;

		case morphing_reptilian_warriorobj:
			hw_precache_reptilian_warrior_morphing();
			break;

		case morphing_mutanthuman2obj:
			hw_precache_experimental_mutant_human_morphing();
			break;

		case electroshotobj:
			hw_precache_electrical_shot();
			break;

		case post_barrierobj:
			hw_precache_electrical_post_barrier();
			break;

		case arc_barrierobj:
			hw_precache_electrical_arc_barrier();
			break;

		case vpost_barrierobj:
			hw_precache_vertical_post_barrier();
			break;

		case vspike_barrierobj:
			hw_precache_vertical_spike_barrier();
			break;

		case goldmorphshotobj:
			hw_precache_morphed_dr_goldfire_shot();
			break;

		case security_lightobj:
			hw_precache_security_light();
			break;

		case explosionobj:
			hw_precache_explosion();
			hw_precache_clip_explosion();
			break;

		case steamgrateobj:
			hw_precache_grate_and_steam();
			break;

		case steampipeobj:
			hw_precache_pipe_and_steam();
			break;

		case liquidshotobj:
			hw_precache_fluid_alien_shot();
			break;

		case lcanshotobj:
			hw_precache_generic_alien_spit_3();
			break;

		case podshotobj:
			hw_precache_generic_alien_spit_3();
			break;

		case scanshotobj:
			hw_precache_generic_alien_spit_1();
			break;

		case dogshotobj:
			hw_precache_generic_alien_spit_1();
			break;

		case mut_hum1shotobj:
			hw_precache_electrical_shot();
			break;

		case ventdripobj:
			hw_precache_vent_and_dripping_blood();
			hw_precache_vent_and_dripping_water();
			break;

		case playerspshotobj:
			break;

		case flickerlightobj:
			hw_precache_flicker_light();
			break;

		case plasma_detonatorobj:
		case plasma_detonator_reserveobj:
			hw_precache_plasma_detonator();
			break;

		case grenadeobj:
			hw_precache_flying_grenade();
			break;

		case bfg_shotobj:
			hw_precache_anti_plasma_cannon_shot();
			break;

		case bfg_explosionobj:
			hw_precache_anti_plasma_cannon_explosion();
			break;

		case pd_explosionobj:
			hw_precache_plasma_detonator_explosion();
			break;

		case spider_mutantshotobj:
			hw_precache_spider_mutant_shot();
			break;

		case breather_beastshotobj:
			break;

		case cyborg_warriorshotobj:
			break;

		case reptilian_warriorshotobj:
			break;

		case acid_dragonshotobj:
			hw_precache_acid_dragon_shot();
			break;

		case mech_guardianshotobj:
			break;

		case final_boss2shotobj:
			hw_precache_morphed_dr_goldfire_shot();
			break;

		case final_boss4shotobj:
			hw_precache_the_crawler_beast_shot();
			break;

		case doorexplodeobj:
			hw_precache_explosion();
			hw_precache_rubble();
			break;

		case gr_explosionobj:
			hw_precache_explosion();
			hw_precache_grenade_explosion();
			break;

		case gold_morphingobj:
			hw_precache_morphed_dr_goldfire();
			break;

		case deadobj:
			hw_precache_dead(*bs_actor);
			break;

		default:
			break;
		}
	}

	hw_precache_special_stuff();
	hw_precache_access_cards();
	hw_precache_player_weapons();
}

void hw_precache_sprites()
{
	hw_precache_statics();
	hw_precache_actors();
}

void hw_build_statics()
{
	hw_uninitialize_statics();

	hw_initialize_statics();

	if (!laststatobj)
	{
		return;
	}

	for (auto bs_static = statobjlist.data(); bs_static != laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		hw_map_static(*bs_static);
	}
}

void hw_build_actors()
{
	hw_uninitialize_actors();
	hw_initialize_actors();

	if (!player)
	{
		return;
	}

	for (auto bs_actor = player->next; bs_actor; bs_actor = bs_actor->next)
	{
		hw_map_actor(*bs_actor);
	}
}

void hw_build_sprites()
{
	hw_uninitialize_sprites();
	hw_initialize_sprites();

	hw_build_statics();
	hw_build_actors();
}

void hw_precache_resources()
{
	vid_log();
	vid_log("Precaching resources.");

	hw_texture_manager_->begin_cache();

	hw_precache_flooring();
	hw_precache_ceiling();
	hw_precache_walls();
	hw_precache_pushwalls();
	hw_precache_doors();
	hw_precache_sprites();

	hw_texture_manager_->end_cache();

	hw_texture_manager_->purge_cache();
}

void hw_destroy_texture_upscale_resources()
{
	hw_ui_t2d_ = nullptr;
	hw_flooring_textured_t2d_ = nullptr;
	hw_ceiling_textured_t2d_ = nullptr;
}

void hw_create_texture_upscale_resources()
{
	hw_ui_t2d_ = hw_texture_manager_->get_ui();

	if (FloorTile > 0)
	{
		hw_flooring_textured_t2d_ = hw_texture_manager_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		hw_ceiling_textured_t2d_ = hw_texture_manager_->get_wall(CeilingTile);
	}
}

void hw_apply_texture_upscale()
{
	vid_log();
	vid_log("Applying texture upscale.");


	hw_destroy_texture_upscale_resources();

	hw_texture_manager_->set_upscale_filter(
		vid_cfg_.texture_upscale_kind_,
		vid_cfg_.texture_upscale_xbrz_degree_
	);

	hw_create_texture_upscale_resources();
}

void hw_update_samplers()
{
	hw_update_ui_sampler();
	hw_update_sprite_sampler();
	hw_update_wall_sampler();
	hw_update_player_weapon_sampler();
}

void hw_apply_filler_color_index()
{
	hw_destroy_2d_fillers_vi();
	hw_destroy_2d_fillers_ib();
	hw_destroy_2d_fillers_vb();

	hw_create_2d_fillers_ib();
	hw_create_2d_fillers_vb();
	hw_create_2d_fillers_vi();
}

void hw_destroy_external_textures_resources()
{
	hw_flooring_textured_t2d_ = nullptr;
	hw_ceiling_textured_t2d_ = nullptr;
}

void hw_create_external_textures_resources()
{
	if (FloorTile > 0)
	{
		hw_flooring_textured_t2d_ = hw_texture_manager_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		hw_ceiling_textured_t2d_ = hw_texture_manager_->get_wall(CeilingTile);
	}
}

void hw_apply_external_textures()
{
	vid_log();
	vid_log("Applying external textures.");


	hw_destroy_external_textures_resources();
	hw_texture_manager_->enable_external_textures(vid_cfg_.is_external_textures_enabled_);
	hw_create_external_textures_resources();
}

void hw_uninitialize_video()
{
	hw_uninitialize_command_buffers();

	hw_uninitialize_program();

	hw_uninitialize_walls();
	hw_uninitialize_pushwalls();
	hw_uninitialize_door_sides();
	hw_uninitialize_sprites();

	hw_uninitialize_flooring();
	hw_uninitialize_ceiling();

	hw_uninitialize_3d_fade();

	hw_uninitialize_player_weapon();
	hw_uninitialize_2d();

	hw_destroy_texture_manager();

	hw_uninitialize_samplers();

	hw_renderer_ = nullptr;
	hw_renderer_manager_ = nullptr;

	vid_is_hw_ = false;
}

void hw_initialize_video()
{
	vid_log();
	vid_log("Initializing hardware accelerated video system.");

	vid_log("Initializing task manager.");

	vid_is_hw_ = false;

	hw_set_samplers_default_states();
	hw_set_player_weapon_sampler_default_state();

	vid_log("Creating renderer manager.");

	hw_renderer_manager_ = bstone::Ren3dMgrFactory::create();

	vid_initialize_common();
	hw_calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	hw_initialize_renderer();
	hw_log_device_features();
	hw_initialize_program();
	hw_create_texture_manager();
	hw_initialize_ui_buffer();
	hw_initialize_palette();
	hw_initialize_2d();
	hw_initialize_flooring();
	hw_initialize_ceiling();
	hw_initialize_3d_fade();
	hw_initialize_player_weapon();
	hw_initialize_command_buffers();
	hw_build_matrices();
	hw_initialize_samplers();

	hw_device_features_ = hw_renderer_->get_device_features();

	vid_is_hw_ = true;

	hw_apply_external_textures();
	hw_apply_texture_upscale();

	const auto window_title = vid_get_window_title_for_renderer();
	hw_renderer_->set_window_title(window_title);
	hw_renderer_->show_window(true);

	in_grab_mouse(true);
}

void hw_take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
{
	auto is_flipped_vertically = false;

	hw_renderer_->read_pixels_rgb_888(src_pixels_rgb_888.get(), is_flipped_vertically);

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		is_flipped_vertically
	);
}

//
// Hardware accelerated renderer (HW).
// ==========================================================================


std::string vid_get_sw_renderer_name()
{
	auto sdl_renderer_info = SDL_RendererInfo{};

	const auto sdl_result = SDL_GetRendererInfo(sw_renderer_.get(), &sdl_renderer_info);

	if (sdl_result != 0)
	{
		vid_throw_sdl_error("Failed to get renderer's information.");
	}

	auto result = std::string{};
	result += "software (";
	result += sdl_renderer_info.name;
	result += ")";

	return result;
}

std::string vid_get_hw_renderer_name()
{
	return hw_renderer_->get_name();
}

std::string vid_get_window_title_for_renderer()
{
	const auto game_name_and_game_version_string = vid_get_game_name_and_game_version_string();
	const auto port_version_string = vid_get_port_version_string();

	auto renderer_name =  std::string{};

	if (sw_renderer_)
	{
		renderer_name = vid_get_sw_renderer_name();
	}
	else if (hw_renderer_)
	{
		renderer_name = vid_get_hw_renderer_name();
	}

	auto result = std::string{};
	result += game_name_and_game_version_string;
	result += " [";
	result += port_version_string;

	if (!renderer_name.empty())
	{
		result += " / ";
		result += renderer_name;
	}

	result += ']';

	return result;
}

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically)
{
	for (auto& task : vid_save_screenshot_mt_tasks)
	{
		if (task.is_completed())
		{
			task.reset(
				width,
				height,
				stride_rgb_888,
				std::move(src_pixels_rgb_888),
				is_flipped_vertically
			);

			bstone::MtTaskPtr tasks_ptr[] = {&task};

			mt_task_manager_->add_tasks(tasks_ptr, 1);

			return;
		}
	}

	vid_log_error("No more screenshot tasks available.");
}

void vid_take_screenshot()
try
{
	vid_is_take_screenshot_scheduled = false;

	const auto width = vid_cfg_.windowed_width_;
	const auto height = vid_cfg_.windowed_height_;
	const auto stride_rgb_888 = (((3 * width) + 3) / 4) * 4;
	auto src_rgb_888_pixels = std::make_unique<std::uint8_t[]>(stride_rgb_888 * height);

	if (vid_is_hw_)
	{
		hw_take_screenshot(
			width,
			height,
			stride_rgb_888,
			std::move(src_rgb_888_pixels)
		);
	}
	else
	{
		sw_take_screenshot(
			width,
			height,
			stride_rgb_888,
			std::move(src_rgb_888_pixels)
		);
	}
}
catch (const std::exception& ex)
{
	vid_log_error("Failed to take a screenshot.");
	vid_log_error(ex.what());
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

	sys_sleep_for(1000 * vbls / TickBase);
}

// ===========================================================================


// BBi Moved from jm_free.cpp
void VL_Startup()
{
	vid_cl_read();

	const auto is_sw = (vid_cfg_.renderer_kind_ == bstone::RendererKind::software);

	auto is_try_sw = false;

	if (is_sw)
	{
		is_try_sw = true;
	}
	else
	{
		try
		{
			hw_initialize_video();
			hw_refresh();
		}
		catch (const bstone::Exception& ex)
		{
			is_try_sw = true;

			hw_uninitialize_video();

			vid_log_error(ex.what());
			vid_log("Falling back to software accelerated video system.");
		}
	}

	if (is_try_sw)
	{
		try
		{
			sw_initialize_video();
			sw_refresh_screen();
		}
		catch (const bstone::Exception& ex)
		{
			sw_uninitialize_video();

			vid_log_error(ex.what());

			throw;
		}
	}

	VL_SetPalette(0, 255, vgapal);

	in_handle_events();

	vid_check_vsync();


	vid_get_window_size_list();
}
// BBi

void VL_Shutdown()
{
	if (vid_is_hw_)
	{
		hw_uninitialize_video();
	}
	else
	{
		sw_uninitialize_video();
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
	for (auto& vga_color : vid_vga_palette_)
	{
		vga_color.r_ = red;
		vga_color.g_ = green;
		vga_color.b_ = blue;
		vga_color.a_ = 255;
	}

	if (vid_is_hw_)
	{
		hw_update_palette(0, bstone::RgbPalette::get_max_color_count());
	}
	else
	{
		sw_update_palette(0, bstone::RgbPalette::get_max_color_count());
	}
}

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		auto& vga_color = vid_vga_palette_[first + i];

		vga_color.r_ = palette[(3 * i) + 0];
		vga_color.g_ = palette[(3 * i) + 1];
		vga_color.b_ = palette[(3 * i) + 2];
		vga_color.a_ = 255;
	}

	if (vid_is_hw_)
	{
		hw_update_palette(0, bstone::RgbPalette::get_max_color_count());
	}
	else
	{
		sw_update_palette(0, bstone::RgbPalette::get_max_color_count());
	}
}

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
{
	for (int i = 0; i < count; ++i)
	{
		const auto& vga_color = vid_vga_palette_[first + i];

		palette[(3 * i) + 0] = vga_color.r_;
		palette[(3 * i) + 1] = vga_color.g_;
		palette[(3 * i) + 2] = vga_color.b_;
	}
}

void vl_hw_fade_out(
	const int red,
	const int green,
	const int blue,
	const int step_count)
{
	hw_2d_fade_is_enabled_ = true;

	hw_2d_fade_color_ = hw_vga_color_to_rgba_8(red, green, blue);

	if (!g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = (i * alpha) / step_count;

			hw_2d_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

			VL_RefreshScreen();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	hw_2d_fade_color_.a_ = 0xFF;

	VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	VL_RefreshScreen();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	hw_2d_fade_is_enabled_ = false;

	screenfaded = true;
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

	if (vid_is_hw_)
	{
		vl_hw_fade_out(red, green, blue, steps);

		return;
	}

	if (!g_no_fade_in_or_out)
	{
		VL_GetPalette(0, 256, &palette1[0][0]);

		std::uninitialized_copy_n(&palette1[0][0], 768, &palette2[0][0]);

		//
		// fade through intermediate frames
		//
		for (int i = 0; i < steps; ++i)
		{
			auto origptr = &palette1[start][0];
			auto newptr = &palette2[start][0];

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

			VL_SetPalette(0, 256, &palette2[0][0]);

			VL_RefreshScreen();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	VL_FillPalette(
		static_cast<std::uint8_t>(red),
		static_cast<std::uint8_t>(green),
		static_cast<std::uint8_t>(blue));

	VL_RefreshScreen();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = true;
}

void vl_hw_fade_in(
	const std::uint8_t* const palette,
	const int step_count)
{
	hw_2d_fade_is_enabled_ = true;
	screenfaded = false;
	
	VL_SetPalette(0, 256, palette);

	hw_2d_fade_color_.a_ = 0xFF;

	if (!g_no_fade_in_or_out)
	{
		const auto alpha = 0xFF;

		for (int i = 0; i < step_count; ++i)
		{
			const auto new_alpha = ((step_count - 1 - i) * alpha) / step_count;

			hw_2d_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

			VL_RefreshScreen();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	hw_2d_fade_color_.a_ = 0x00;

	VL_RefreshScreen();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	hw_2d_fade_is_enabled_ = false;
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

	if (vid_is_hw_)
	{
		vl_hw_fade_in(palette, steps);

		return;
	}

	if (!g_no_fade_in_or_out)
	{
		VL_GetPalette(0, 256, &palette1[0][0]);

		std::uninitialized_copy_n(&palette1[0][0], 768, &palette2[0][0]);

		const auto start_index = start * 3;
		const auto end_index = (end * 3) + 2;

		//
		// fade through intermediate frames
		//
		for (int i = 0; i < steps; ++i)
		{
			for (int j = start_index; j <= end_index; ++j)
			{
				const int delta = palette[j] - palette1[0][j];

				palette2[0][j] =
					static_cast<std::uint8_t>(palette1[0][j] + ((delta * i) / steps));
			}

			VL_SetPalette(0, 256, &palette2[0][0]);

			VL_RefreshScreen();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	VL_SetPalette(0, 256, palette);

	VL_RefreshScreen();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = false;
}

void VL_SetPaletteIntensity(
	int start,
	int end,
	const std::uint8_t* palette,
	int intensity)
{
	auto cmap = &palette1[0][0] + (start * 3);

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

	VL_SetPalette(
		start,
		end - start + 1,
		&palette1[0][0]);
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
	const auto offset = (y * vga_ref_width) + x;

	vid_ui_buffer_[offset] = color;
	vid_mask_buffer_[offset] = !is_transparent;
}

void VL_Hlin(
	int x,
	int y,
	int width,
	std::uint8_t color)
{
	VL_Bar(x, y, width, 1, color);
}

void VL_Vlin(
	int x,
	int y,
	int height,
	std::uint8_t color)
{
	VL_Bar(x, y, 1, height, color);
}

void VL_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	const bool is_transparent)
{
	if (x == 0 && width == vga_ref_width)
	{
		const auto offset = y * vga_ref_width;
		const auto count = height * vga_ref_width;

		std::uninitialized_fill(
			vid_ui_buffer_.begin() + offset,
			vid_ui_buffer_.begin() + offset + count,
			color);

		std::uninitialized_fill(
			vid_mask_buffer_.begin() + offset,
			vid_mask_buffer_.begin() + offset + count,
			!is_transparent);
	}
	else
	{
		for (int i = 0; i < height; ++i)
		{
			const auto offset = ((y + i) * vga_ref_width) + x;

			std::uninitialized_fill(
				vid_ui_buffer_.begin() + offset,
				vid_ui_buffer_.begin() + offset + width,
				color);

			std::uninitialized_fill(
				vid_mask_buffer_.begin() + offset,
				vid_mask_buffer_.begin() + offset + width,
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

				latches_cache[offset] = pixel;
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
				VL_Plot(x + w, y + h, *source++);
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
					VL_Plot(x + w, y + h, color);
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
				*dest++ = vl_get_pixel(bufferofs, x + w, y + h);
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
		const auto dst_offset = (vga_ref_width * (y + h)) + x;

		std::uninitialized_copy(
			latches_cache.cbegin() + src_offset,
			latches_cache.cbegin() + src_offset + width,
			vid_ui_buffer_.begin() + dst_offset);

		std::uninitialized_fill(
			vid_mask_buffer_.begin() + dst_offset,
			vid_mask_buffer_.begin() + dst_offset + width,
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
		const auto src_offset = source + (h * vga_ref_width);
		const auto dst_offset = dest + (h * vga_ref_width);

		std::uninitialized_copy(
			vid_ui_buffer_.cbegin() + src_offset,
			vid_ui_buffer_.cbegin() + src_offset + width,
			vid_ui_buffer_.begin() + dst_offset);

		std::uninitialized_fill(
			vid_mask_buffer_.begin() + dst_offset,
			vid_mask_buffer_.begin() + dst_offset + width,
			true);
	}
}


void JM_VGALinearFill(
	int start,
	int length,
	std::uint8_t fill)
{
	std::uninitialized_fill(
		vid_ui_buffer_.begin() + start,
		vid_ui_buffer_.begin() + start + length,
		fill);

	std::uninitialized_fill(
		vid_mask_buffer_.begin() + start,
		vid_mask_buffer_.begin() + start + length,
		true);
}

void VL_RefreshScreen()
{
	if (vid_is_hw_)
	{
		hw_refresh();
	}
	else
	{
		sw_refresh_screen();
	}
}

int vl_get_offset(
	int base_offset,
	int x,
	int y)
{
	return base_offset + (y * vga_width) + x;
}

std::uint8_t vl_get_pixel(
	int base_offset,
	int x,
	int y)
{
	static_cast<void>(base_offset);

	return vid_ui_buffer_[(y * vga_ref_width) + x];
}

void vl_update_widescreen()
{
	if (vid_is_hw_)
	{
		hw_apply_widescreen();
	}
	else
	{
		sw_apply_widescreen();
	}
}

void vid_set_ui_mask(
	bool value)
{
	vid_mask_buffer_.fill(
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
		const auto offset = ((y + h) * vga_ref_width) + x;

		std::uninitialized_fill(
			vid_mask_buffer_.begin() + offset,
			vid_mask_buffer_.begin() + offset + width,
			value);
	}
}

void vid_set_ui_mask_3d(
	bool value)
{
	vid_set_ui_mask(
		0,
		ref_3d_view_top_y - ref_3d_margin,
		vga_ref_width,
		ref_3d_view_height + 2 * ref_3d_margin,
		value);
}

void vid_clear_3d()
{
	std::uninitialized_fill(
		sw_vga_buffer_.begin(),
		sw_vga_buffer_.end(),
		VgaBuffer::value_type{});
}

void vid_export_ui(
	VgaBuffer& dst_buffer)
{
	dst_buffer = vid_ui_buffer_;
}

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent)
{
	vid_ui_buffer_ = src_buffer;
	vid_set_ui_mask(!is_transparent);
}

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer)
{
	dst_buffer = vid_mask_buffer_;
}

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer)
{
	vid_mask_buffer_ = src_buffer;
}

const std::string& vid_filter_to_string(
	const bstone::Ren3dFilterKind filter)
{
	switch (filter)
	{
		case bstone::Ren3dFilterKind::nearest:
			return vid_get_nearest_value_string();

		case bstone::Ren3dFilterKind::linear:
			return vid_get_linear_value_string();

		default:
			Quit("Invalid filter.");
	}
}

void vid_cfg_read_renderer_kind(
	const std::string& value_string)
{
	if (false)
	{
	}
	else if (value_string == vid_get_none_value_string() ||
		value_string == vid_get_auto_detect_value_string())
	{
		vid_cfg_.renderer_kind_ = bstone::RendererKind::auto_detect;
	}
	else if (value_string == vid_get_software_value_string())
	{
		vid_cfg_.renderer_kind_ = bstone::RendererKind::software;
	}
	else if (value_string == vid_get_gl_2_0_value_string())
	{
		vid_cfg_.renderer_kind_ = bstone::RendererKind::gl_2_0;
	}
	else if (value_string == vid_get_gl_3_2_c_value_string())
	{
		vid_cfg_.renderer_kind_ = bstone::RendererKind::gl_3_2_core;
	}
	else if (value_string == vid_get_gles_2_0_value_string())
	{
		vid_cfg_.renderer_kind_ = bstone::RendererKind::gles_2_0;
	}
}

void vid_cfg_read_width(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.windowed_width_ = value;
	}
}

void vid_cfg_read_height(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.windowed_height_ = value;
	}
}

void vid_cfg_read_is_windowed(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_windowed_ = (value != 0);
	}
}

void vid_cfg_read_vsync(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_vsync_ = (value != 0);
	}
}

void vid_cfg_read_is_widescreen(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_widescreen_ = (value != 0);
	}
}

void vid_cfg_read_is_ui_stretched(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_ui_stretched_ = (value != 0);
	}
}

void vid_cfg_read_hw_2d_texture_filter(
	const std::string& value_string)
{
	if (value_string == vid_get_nearest_value_string())
	{
		vid_cfg_.d2_texture_filter_ = bstone::Ren3dFilterKind::nearest;
	}
	else if (value_string == vid_get_linear_value_string())
	{
		vid_cfg_.d2_texture_filter_ = bstone::Ren3dFilterKind::linear;
	}
}

void vid_cfg_read_hw_3d_texture_image_filter(
	const std::string& value_string)
{
	if (value_string == vid_get_nearest_value_string())
	{
		vid_cfg_.d3_texture_image_filter_ = bstone::Ren3dFilterKind::nearest;
	}
	else if (value_string == vid_get_linear_value_string())
	{
		vid_cfg_.d3_texture_image_filter_ = bstone::Ren3dFilterKind::linear;
	}
}

void vid_cfg_read_hw_3d_texture_mipmap_filter(
	const std::string& value_string)
{
	if (value_string == vid_get_nearest_value_string())
	{
		vid_cfg_.d3_texture_mipmap_filter_ = bstone::Ren3dFilterKind::nearest;
	}
	else if (value_string == vid_get_linear_value_string())
	{
		vid_cfg_.d3_texture_mipmap_filter_ = bstone::Ren3dFilterKind::linear;
	}
}

void vid_cfg_read_hw_3d_texture_anisotropy(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.d3_texture_anisotropy_ = value;
	}
}

void vid_cfg_read_hw_aa_kind(
	const std::string& value_string)
{
	if (value_string == vid_get_msaa_value_string())
	{
		vid_cfg_.aa_kind_ = bstone::Ren3dAaKind::ms;
	}
	else
	{
		vid_cfg_.aa_kind_ = bstone::Ren3dAaKind::none;
	}
}

void vid_cfg_read_hw_aa_value(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.aa_degree_ = value;
	}
}

void vid_cfg_read_hw_texture_upscale_filter(
	const std::string& value_string)
{
	if (value_string == vid_get_none_value_string())
	{
		vid_cfg_.texture_upscale_kind_ = bstone::HwTextureMgrUpscaleFilterKind::none;
	}
	else if (value_string == vid_get_xbrz_value_string())
	{
		vid_cfg_.texture_upscale_kind_ = bstone::HwTextureMgrUpscaleFilterKind::xbrz;
	}
}

void vid_cfg_read_hw_texture_upscale_xbrz_factor(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.texture_upscale_xbrz_degree_ = value;
	}
}

void vid_cfg_read_filler_color_index(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.filler_color_index = vid_clamp_filler_color_index(value);
	}
}

void vid_cfg_read_external_textures(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_external_textures_enabled_ = (value != 0);
	}
}

bool vid_cfg_parse_key_value(
	const std::string& key_string,
	const std::string& value_string)
{
	if (false)
	{
	}
	else if (key_string == vid_get_renderer_key_name())
	{
		vid_cfg_read_renderer_kind(value_string);
	}
	else if (key_string == vid_get_is_windowed_key_name())
	{
		vid_cfg_read_is_windowed(value_string);
	}
	else if (key_string == vid_get_is_vsync_key_name())
	{
		vid_cfg_read_vsync(value_string);
	}
	else if (key_string == vid_get_windowed_width_key_name())
	{
		vid_cfg_read_width(value_string);
	}
	else if (key_string == vid_get_windowed_height_key_name())
	{
		vid_cfg_read_height(value_string);
	}
	else if (key_string == vid_get_is_widescreen_key_name())
	{
		vid_cfg_read_is_widescreen(value_string);
	}
	else if (key_string == vid_get_is_ui_stretched_key_name())
	{
		vid_cfg_read_is_ui_stretched(value_string);
	}
	else if (key_string == vid_get_2d_texture_filter_key_name())
	{
		vid_cfg_read_hw_2d_texture_filter(value_string);
	}
	else if (key_string == vid_get_3d_texture_image_filter_key_name())
	{
		vid_cfg_read_hw_3d_texture_image_filter(value_string);
	}
	else if (key_string == vid_get_3d_texture_mipmap_filter_key_name())
	{
		vid_cfg_read_hw_3d_texture_mipmap_filter(value_string);
	}
	else if (key_string == vid_get_3d_texture_anisotropy_key_name())
	{
		vid_cfg_read_hw_3d_texture_anisotropy(value_string);
	}
	else if (key_string == vid_get_aa_kind_key_name())
	{
		vid_cfg_read_hw_aa_kind(value_string);
	}
	else if (key_string == vid_get_aa_degree_key_name())
	{
		vid_cfg_read_hw_aa_value(value_string);
	}
	else if (key_string == vid_get_texture_upscale_filter_key_name())
	{
		vid_cfg_read_hw_texture_upscale_filter(value_string);
	}
	else if (key_string == vid_get_texture_upscale_xbrz_degree_key_name())
	{
		vid_cfg_read_hw_texture_upscale_xbrz_factor(value_string);
	}
	else if (key_string == vid_get_filler_color_index_name())
	{
		vid_cfg_read_filler_color_index(value_string);
	}
	else if (key_string == vid_get_external_textures_name())
	{
		vid_cfg_read_external_textures(value_string);
	}
	else
	{
		return false;
	}

	return true;
}

void vid_write_hw_aa_kind_cfg(
	bstone::TextWriter& text_writer)
{
	switch (vid_cfg_.aa_kind_)
	{
		case bstone::Ren3dAaKind::ms:
			cfg_file_write_entry(
				text_writer,
				vid_get_aa_kind_key_name(),
				vid_get_msaa_value_string()
			);

			break;

		default:
			cfg_file_write_entry(
				text_writer,
				vid_get_aa_kind_key_name(),
				vid_get_none_value_string()
			);

			break;
	}
}

void vid_write_renderer_kind_cfg(
	bstone::TextWriter& text_writer)
{
	switch (vid_cfg_.renderer_kind_)
	{
		case bstone::RendererKind::software:
			cfg_file_write_entry(
				text_writer,
				vid_get_renderer_key_name(),
				vid_get_software_value_string()
			);

			break;

		case bstone::RendererKind::gl_2_0:
			cfg_file_write_entry(
				text_writer,
				vid_get_renderer_key_name(),
				vid_get_gl_2_0_value_string()
			);

			break;

		case bstone::RendererKind::gl_3_2_core:
			cfg_file_write_entry(
				text_writer,
				vid_get_renderer_key_name(),
				vid_get_gl_3_2_c_value_string()
			);

			break;

		case bstone::RendererKind::gles_2_0:
			cfg_file_write_entry(
				text_writer,
				vid_get_renderer_key_name(),
				vid_get_gles_2_0_value_string()
			);

			break;

		case bstone::RendererKind::auto_detect:
		default:
			cfg_file_write_entry(
				text_writer,
				vid_get_renderer_key_name(),
				vid_get_auto_detect_value_string()
			);

			break;
	}
}

void vid_write_hw_texture_upscale_filter_kind_cfg(
	bstone::TextWriter& text_writer)
{
	switch (vid_cfg_.texture_upscale_kind_)
	{
		case bstone::HwTextureMgrUpscaleFilterKind::xbrz:
			cfg_file_write_entry(
				text_writer,
				vid_get_texture_upscale_filter_key_name(),
				vid_get_xbrz_value_string()
			);

			break;

		default:
			cfg_file_write_entry(
				text_writer,
				vid_get_texture_upscale_filter_key_name(),
				vid_get_none_value_string()
			);

			break;
	}
}

void vid_cfg_write(
	bstone::TextWriter& text_writer)
{
	text_writer.write("\n// Video\n");

	vid_write_renderer_kind_cfg(text_writer);

	cfg_file_write_entry(
		text_writer,
		vid_get_is_windowed_key_name(),
		std::to_string(vid_cfg_.is_windowed_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_is_positioned_key_name(),
		std::to_string(vid_cfg_.is_positioned_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_windowed_x_key_name(),
		std::to_string(vid_cfg_.windowed_x_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_windowed_y_key_name(),
		std::to_string(vid_cfg_.windowed_y_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_windowed_width_key_name(),
		std::to_string(vid_cfg_.windowed_width_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_windowed_height_key_name(),
		std::to_string(vid_cfg_.windowed_height_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_is_vsync_key_name(),
		std::to_string(vid_cfg_.is_vsync_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_is_ui_stretched_key_name(),
		std::to_string(vid_cfg_.is_ui_stretched_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_is_widescreen_key_name(),
		std::to_string(vid_cfg_.is_widescreen_)
	);

	vid_write_hw_aa_kind_cfg(text_writer);

	cfg_file_write_entry(
		text_writer,
		vid_get_aa_degree_key_name(),
		std::to_string(vid_cfg_.aa_degree_)
	);

	vid_write_hw_texture_upscale_filter_kind_cfg(text_writer);

	cfg_file_write_entry(
		text_writer,
		vid_get_texture_upscale_xbrz_degree_key_name(),
		std::to_string(vid_cfg_.texture_upscale_xbrz_degree_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_2d_texture_filter_key_name(),
		vid_filter_to_string(vid_cfg_.d2_texture_filter_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_3d_texture_image_filter_key_name(),
		vid_filter_to_string(vid_cfg_.d3_texture_image_filter_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_3d_texture_mipmap_filter_key_name(),
		vid_filter_to_string(vid_cfg_.d3_texture_mipmap_filter_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_3d_texture_anisotropy_key_name(),
		std::to_string(vid_cfg_.d3_texture_anisotropy_)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_filler_color_index_name(),
		std::to_string(vid_cfg_.filler_color_index)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_external_textures_name(),
		std::to_string(vid_cfg_.is_external_textures_enabled_)
	);
}

void vid_cfg_set_defaults()
{
	vid_cfg_ = {};

	vid_cfg_.renderer_kind_ = bstone::RendererKind::auto_detect;

	vid_cfg_.is_windowed_ = true;
	vid_cfg_.is_positioned_ = false;
	vid_cfg_.windowed_x_ = 0;
	vid_cfg_.windowed_y_ = 0;
	vid_cfg_.windowed_width_ = default_window_width;
	vid_cfg_.windowed_height_ = default_window_height;
	vid_cfg_.is_vsync_ = true;

	vid_cfg_.is_ui_stretched_ = false;
	vid_cfg_.is_widescreen_ = true;

	vid_cfg_.d2_texture_filter_ = bstone::Ren3dFilterKind::nearest;

	vid_cfg_.d3_texture_image_filter_ = bstone::Ren3dFilterKind::nearest;
	vid_cfg_.d3_texture_mipmap_filter_ = bstone::Ren3dFilterKind::nearest;

	vid_cfg_.d3_texture_anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;

	vid_cfg_.aa_kind_ = bstone::Ren3dAaKind::none;
	vid_cfg_.aa_degree_ = bstone::Ren3dLimits::min_aa_off;

	vid_cfg_.texture_upscale_kind_ = bstone::HwTextureMgrUpscaleFilterKind::none;
	vid_cfg_.texture_upscale_xbrz_degree_ = 0;

	vid_cfg_.filler_color_index = 0;

	vid_cfg_.is_external_textures_enabled_ = false;
}

VidCfg& vid_cfg_get()
{
	return vid_cfg_;
}

const VidRendererKinds& vid_get_available_renderer_kinds()
{
	static const auto result = VidRendererKinds
	{
		bstone::RendererKind::auto_detect,
		bstone::RendererKind::software,

		bstone::RendererKind::gl_2_0,
		bstone::RendererKind::gl_3_2_core,
		bstone::RendererKind::gles_2_0,
	};

	return result;
}

const VidWindowSizes& vid_get_window_size_list()
{
	static auto result = VidWindowSizes{};

	const auto display_index = 0;
	const auto sdl_mode_count = SDL_GetNumDisplayModes(display_index);

	result.clear();
	result.reserve(sdl_mode_count);

	int sdl_result;
	auto sdl_mode = SDL_DisplayMode{};
	auto is_current_added = false;
	auto is_custom_added = false;

	for (int i = 0; i < sdl_mode_count; ++i)
	{
		sdl_result = SDL_GetDisplayMode(display_index, i, &sdl_mode);

		if (sdl_result == 0)
		{
			const auto is_added = std::any_of(
				result.cbegin(),
				result.cend(),
				[&](const auto& item)
				{
					return item.windowed_width_ == sdl_mode.w && item.windowed_height_ == sdl_mode.h;
				}
			);

			if (!is_added)
			{
				result.emplace_back();
				auto& window_size = result.back();
				window_size.windowed_width_ = sdl_mode.w;
				window_size.windowed_height_ = sdl_mode.h;

				//
				const auto is_current =
					sdl_mode.w == vid_dimensions_.windowed_width_ &&
					sdl_mode.h == vid_dimensions_.windowed_height_;

				window_size.is_current_ = is_current;

				if (is_current)
				{
					is_current_added = true;
				}

				//
				const auto is_custom =
					sdl_mode.w == vid_cfg_.windowed_width_ &&
					sdl_mode.h == vid_cfg_.windowed_height_;

				window_size.is_custom_ = is_custom;

				if (is_custom)
				{
					is_custom_added = true;
				}
			}
		}
	}

	std::sort(
		result.begin(),
		result.end(),
		[](const auto& lhs, const auto& rhs)
		{
			if (lhs.windowed_width_ != rhs.windowed_width_)
			{
				return lhs.windowed_width_ < rhs.windowed_width_;
			}

			return lhs.windowed_height_ < rhs.windowed_height_;
		}
	);

	if (!is_current_added && !is_custom_added)
	{
		result.emplace_back();
		auto& window_size = result.back();
		window_size.windowed_width_ = vid_cfg_.windowed_width_;
		window_size.windowed_height_ = vid_cfg_.windowed_height_;

		window_size.is_current_ = true;
		window_size.is_custom_ = true;
	}

	return result;
}

void vid_draw_ui_sprite(
	const int sprite_id,
	const int center_x,
	const int center_y,
	const int new_side)
{
	constexpr auto dimension = bstone::Sprite::dimension;

	const auto sprite_ptr = vid_sprite_cache.cache(
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

		if (x >= vga_ref_width)
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

			if (y >= vga_ref_height)
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

			VL_Plot(x, y, color_index);
		}
	}
}

void vid_hw_on_load_level()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_precache_resources();

	hw_build_pushwalls();
	hw_build_walls();
	hw_build_doors();
	hw_build_sprites();
}

void vid_hw_on_update_wall_switch(
	const int x,
	const int y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	assert(x >= 0 && x < MAPSIZE && y >= 0 && y < MAPSIZE);

	const auto xy = hw_encode_xy(x, y);

	auto wall_it = hw_xy_wall_map_.find(xy);

	if (wall_it == hw_xy_wall_map_.cend())
	{
		const auto& assets_info = get_assets_info();

		if (assets_info.is_aog())
		{
			Quit("Expected wall at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
		}
		else
		{
			// Operable non-directly.
			return;
		}
	}

	const auto tile_wall = tilemap[x][y] & tilemap_wall_mask;

	assert(tile_wall == OFF_SWITCH || tile_wall == ON_SWITCH);

	const auto horizontal_wall_id = horizwall[tile_wall];
	const auto vertical_wall_id = vertwall[tile_wall];

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
			side.texture_id_ = vertical_wall_id;
		}
		else
		{
			side.texture_id_ = horizontal_wall_id;
		}
	}
}

void vid_hw_on_move_pushwall()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_translate_pushwall();
}

void vid_hw_on_step_pushwall(
	const int old_x,
	const int old_y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_step_pushwall(old_x, old_y);
}

void vid_hw_on_pushwall_to_wall(
	const int old_x,
	const int old_y,
	const int new_x,
	const int new_y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	//assert(old_x != new_x || old_y != new_y);

	const auto old_xy = hw_encode_xy(old_x, old_y);
	const auto old_pushwall_it = hw_xy_pushwall_map_.find(old_xy);

	if (old_pushwall_it == hw_xy_pushwall_map_.cend())
	{
		Quit("Pushwall not found.");

		return;
	}

	hw_xy_pushwall_map_.erase(old_pushwall_it);

	auto vertex_index = 0;

	hw_map_xy_to_xwall(
		HwXyWallKind::solid,
		new_x,
		new_y,
		hw_xy_wall_map_,
		vertex_index,
		hw_pushwall_to_wall_vbi_
	);

	// Adjust vertex indices.
	//
	const auto new_xy = hw_encode_xy(new_x, new_y);
	auto& wall = hw_xy_wall_map_[new_xy];

	for (auto& wall_side : wall.sides_)
	{
		if (!wall_side.flags_.is_active_)
		{
			continue;
		}

		wall_side.vertex_index_ += hw_wall_vertex_count_;
	}

	hw_update_vertex_buffer(
		hw_wall_sides_vb_,
		hw_wall_vertex_count_,
		vertex_index,
		hw_pushwall_to_wall_vbi_.data()
	);

	hw_wall_vertex_count_ += vertex_index;
}

void vid_hw_on_move_door(
	const int door_index)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto& bs_door = doorobjlist[door_index];

	const auto xy = hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = hw_xy_door_map_.find(xy);

	if (map_it == hw_xy_door_map_.cend())
	{
		Quit("Door mapping not found.");
	}

	auto& hw_door = hw_xy_door_map_[xy];

	auto vertex_index = hw_door.vertex_index_;
	const auto old_vertex_index = vertex_index;

	hw_map_door_side(hw_door.sides_.front(), vertex_index, hw_doors_vbi_);

	hw_update_vertex_buffer(
		hw_door_sides_vb_,
		old_vertex_index,
		hw_vertices_per_door,
		&hw_doors_vbi_[old_vertex_index]
	);
}

void vid_hw_on_update_door_lock(
	const int bs_door_index)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto& bs_door = doorobjlist[bs_door_index];

	const auto xy = hw_encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = hw_xy_door_map_.find(xy);

	if (map_it == hw_xy_door_map_.cend())
	{
		Quit("Door mapping not found.");
	}

	auto& door = hw_xy_door_map_[xy];

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

	door.sides_[0].texture_id_ = front_face_page_number;
	door.sides_[1].texture_id_ = back_face_page_number;
}

void vid_hw_on_remove_static(
	const statobj_t& bs_static)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto bs_static_index = hw_get_static_index(bs_static);
	auto& hw_static = hw_statics_[bs_static_index];
	hw_static = {};
}

void vid_hw_on_remove_actor(
	const objtype& bs_actor)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto bs_actor_index = hw_get_actor_index(bs_actor);
	auto& hw_actor = hw_actors_[bs_actor_index];
	hw_actor = {};
}

void vid_hw_enable_fizzle_fx(
	const bool is_enabled)
{
	hw_fizzle_fx_is_enabled_ = is_enabled;
}

void vid_hw_enable_fizzle_fx_fading(
	const bool is_fading)
{
	hw_fizzle_fx_is_fading_ = is_fading;
}

void vid_hw_set_fizzle_fx_color_index(
	const int color_index)
{
	hw_fizzle_fx_color_index_ = color_index;
}

void vid_hw_set_fizzle_fx_ratio(
	const float ratio)
{
	hw_fizzle_fx_ratio_ = ratio;
}

void vid_hw_clear_wall_render_list()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_wall_last_xy_to_render_at_ = -1;
	hw_walls_to_render_.clear();
}

void vid_hw_add_wall_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto xy = hw_encode_xy(tile_x, tile_y);

	if (hw_wall_last_xy_to_render_at_ == xy)
	{
		return;
	}

	hw_wall_last_xy_to_render_at_ = xy;

	const auto is_pushwall =
		hw_is_pushwall(tile_x, tile_y) ||
		hw_xy_pushwall_map_.find(xy) != hw_xy_pushwall_map_.cend();

	if (is_pushwall)
	{
		hw_pushwalls_to_render_.insert(xy);
	}
	else
	{
		hw_walls_to_render_.insert(xy);
	}
}

void vid_hw_clear_pushwall_render_list()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_pushwall_last_xy_to_render_at_ = -1;
	hw_pushwalls_to_render_.clear();
}

void vid_hw_add_pushwall_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto xy = hw_encode_xy(tile_x, tile_y);

	if (hw_pushwall_last_xy_to_render_at_ == xy)
	{
		return;
	}

	hw_pushwall_last_xy_to_render_at_ = xy;

	hw_pushwalls_to_render_.insert(xy);
}

void vid_hw_clear_door_render_list()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_door_last_xy_to_render_at_ = -1;
	hw_doors_to_render_.clear();
}

void vid_hw_add_door_render_item(
	const int tile_x,
	const int tile_y)
{
	if (!vid_is_hw_)
	{
		return;
	}

	const auto xy = hw_encode_xy(tile_x, tile_y);

	if (hw_door_last_xy_to_render_at_ == xy)
	{
		return;
	}

	hw_door_last_xy_to_render_at_ = xy;

	hw_doors_to_render_.insert(xy);
}

void vid_hw_clear_static_render_list()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_statics_to_render_.clear();
}

void vid_hw_add_static_render_item(
	const int bs_static_index)
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_statics_to_render_.insert(bs_static_index);
}

void vid_hw_clear_actor_render_list()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_actors_to_render_.clear();
}

void vid_hw_add_actor_render_item(
	const int bs_actor_index)
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_actors_to_render_.emplace(bs_actor_index);
}

const bstone::Rgba8Palette& vid_hw_get_default_palette()
{
	return hw_default_palette_;
}

void vid_sw_apply_window_mode()
{
	sw_calculate_dimensions();
	vid_initialize_vanilla_raycaster();

	auto param = bstone::Ren3dSetWindowModeParam{};
	param.is_windowed_ = vid_cfg_.is_windowed_;
	param.rect_2d_.extent_.width_ = vid_cfg_.windowed_width_;
	param.rect_2d_.extent_.height_ = vid_cfg_.windowed_height_;
	bstone::detail::Ren3dUtils::set_window_mode(sw_window_.get(), param);

	vid_initialize_common();

	sw_initialize_textures();
	sw_initialize_vga_buffer();
}

void vid_hw_apply_window_mode()
{
	hw_calculate_dimensions();
	vid_initialize_vanilla_raycaster();

	auto param = bstone::Ren3dSetWindowModeParam{};
	param.is_windowed_ = vid_cfg_.is_windowed_;
	param.rect_2d_.extent_.width_ = vid_dimensions_.window_width_;
	param.rect_2d_.extent_.height_ = vid_dimensions_.window_height_;
	hw_renderer_->set_window_mode(param);

	vid_initialize_common();

	hw_uninitialize_2d();
	hw_initialize_2d();

	hw_uninitialize_3d_fade();
	hw_initialize_3d_fade();

	hw_uninitialize_player_weapon();
	hw_initialize_player_weapon();

	hw_build_matrices();
}

void vid_apply_window_mode()
{
	if (vid_is_hw_)
	{
		vid_hw_apply_window_mode();
	}
	else
	{
		vid_sw_apply_window_mode();
	}
}

void vid_hw_apply_vsync()
{
	hw_renderer_->enable_vsync(vid_cfg_.is_vsync_);
}

void vid_apply_vsync()
{
	if (vid_is_hw_)
	{
		vid_hw_apply_vsync();
	}
}

void vid_hw_apply_msaa()
{
	hw_renderer_->set_anti_aliasing(vid_cfg_.aa_kind_, vid_cfg_.aa_degree_);
}

void vid_apply_msaa()
{
	if (vid_is_hw_)
	{
		vid_hw_apply_msaa();
	}
}

void vid_apply_video_mode(
	const VideoModeCfg& video_mode_cfg)
{
	auto is_restart = false;

	if (!is_restart &&
		vid_cfg_.renderer_kind_ != video_mode_cfg.renderer_kind_)
	{
		is_restart = true;
	}

	if (!is_restart &&
		vid_cfg_.is_vsync_ != video_mode_cfg.is_vsync_ &&
		(!vid_is_hw_ ||
			(vid_is_hw_ &&
				hw_device_features_.is_vsync_available_ &&
				hw_device_features_.is_vsync_requires_restart_)
		))
	{
		is_restart = true;
	}

	if (!is_restart &&
		(vid_cfg_.aa_kind_ != video_mode_cfg.aa_kind_ ||
			vid_cfg_.aa_degree_ != video_mode_cfg.aa_degree_) &&
		vid_is_hw_ &&
		video_mode_cfg.aa_kind_ == bstone::Ren3dAaKind::ms &&
		hw_device_features_.is_msaa_available_ &&
		hw_device_features_.is_msaa_requires_restart_)
	{
		is_restart = true;
	}

	const auto is_window_modified = (
		vid_cfg_.is_windowed_ != video_mode_cfg.is_windowed_ ||
		(
			video_mode_cfg.is_windowed_ &&
			(
				vid_cfg_.windowed_width_ != video_mode_cfg.windowed_width_ ||
				vid_cfg_.windowed_height_ != video_mode_cfg.windowed_height_
			)
		)
	);

	const auto is_vsync_modified = (vid_cfg_.is_vsync_ != video_mode_cfg.is_vsync_);

	const auto is_aa_modified = (
		vid_cfg_.aa_kind_ != video_mode_cfg.aa_kind_ ||
		vid_cfg_.aa_degree_ != video_mode_cfg.aa_degree_);

	vid_cfg_.renderer_kind_ = video_mode_cfg.renderer_kind_;
	vid_cfg_.is_windowed_ = video_mode_cfg.is_windowed_;
	vid_cfg_.windowed_width_ = video_mode_cfg.windowed_width_;
	vid_cfg_.windowed_height_ = video_mode_cfg.windowed_height_;
	vid_cfg_.is_vsync_ = video_mode_cfg.is_vsync_;
	vid_cfg_.aa_kind_ = video_mode_cfg.aa_kind_;
	vid_cfg_.aa_degree_ = video_mode_cfg.aa_degree_;

	if (is_restart)
	{
		VL_Shutdown();
		VL_Startup();

		vid_hw_on_load_level();

		return;
	}

	if (is_window_modified)
	{
		vid_apply_window_mode();
	}

	if (is_vsync_modified)
	{
		vid_apply_vsync();
	}

	if (is_aa_modified)
	{
		vid_apply_msaa();
	}
}

void vid_apply_anisotropy()
{
	if (vid_is_hw_)
	{
		hw_update_samplers();
	}
}

void vid_apply_2d_image_filter()
{
	if (vid_is_hw_)
	{
		hw_update_samplers();
	}
}

void vid_apply_3d_image_filter()
{
	if (vid_is_hw_)
	{
		hw_update_samplers();
	}
}

void vid_apply_mipmap_filter()
{
	if (vid_is_hw_)
	{
		hw_update_samplers();
	}
}

void vid_apply_upscale()
{
	if (vid_is_hw_)
	{
		hw_apply_texture_upscale();
	}
}

int vid_clamp_filler_color_index(
	int filler_color_index) noexcept
{
	return bstone::math::clamp(filler_color_index, 0, 255);
}

void vid_apply_filler_color()
{
	if (vid_is_hw_)
	{
		hw_apply_filler_color_index();
	}
	else
	{
		sw_apply_filler_color_index();
	}
}

void vid_apply_external_textures()
{
	if (!vid_is_hw_)
	{
		return;
	}

	hw_apply_external_textures();
}

void vid_schedule_take_screenshot()
{
	vid_is_take_screenshot_scheduled = true;
}

bool operator==(
	const VideoModeCfg& lhs,
	const VideoModeCfg& rhs) noexcept
{
	return
		lhs.renderer_kind_ == rhs.renderer_kind_ &&
		lhs.is_windowed_ == rhs.is_windowed_ &&
		lhs.windowed_width_ == rhs.windowed_width_ &&
		lhs.windowed_height_ == rhs.windowed_height_ &&
		lhs.is_vsync_ == rhs.is_vsync_ &&
		lhs.aa_kind_ == rhs.aa_kind_ &&
		lhs.aa_degree_ == rhs.aa_degree_;
}

bool operator!=(
	const VideoModeCfg& lhs,
	const VideoModeCfg& rhs) noexcept
{
	return !(lhs == rhs);
}
// BBi
