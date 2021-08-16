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

#include "SDL.h"

#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_atomic_flag.h"
#include "bstone_file_stream.h"
#include "bstone_file_system.h"
#include "bstone_hw_video.h"
#include "bstone_image_encoder.h"
#include "bstone_logger.h"
#include "bstone_mt_task_mgr.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_sdl_exception.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_sw_video.h"
#include "bstone_text_writer.h"
#include "bstone_time.h"
#include "bstone_version.h"


extern bool is_full_menu_active;


int bufferofs;

bool screenfaded;

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

bstone::SpriteCache vid_sprite_cache;


// BBi
bool vid_is_take_screenshot_scheduled = false;

double height_compensation_factor = 1.2;

namespace
{


void vid_log(
	const std::string& message);

void vid_log_error(
	const std::string& message);


} // namespace


void vid_take_screenshot();


namespace
{


class VideoException :
	public bstone::Exception
{
public:
	explicit VideoException(
		const char* message) noexcept
		:
		bstone::Exception{"VIDEO", message}
	{
	}
}; // VideoException


[[noreturn]]
void fail(
	const char* message)
{
	throw VideoException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(VideoException{message});
}


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
			fail(("Failed to open a file \"" + path + "\".").c_str());
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


constexpr int default_window_width = 640;
constexpr int default_window_height = 480;


} // namespace


VgaBuffer vid_ui_buffer_;
UiMaskBuffer vid_mask_buffer_;

VidLayout vid_layout_;


namespace
{


SDL_DisplayMode vid_display_mode_;


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
	int dimension) noexcept
{
	const auto alignment = 2;

	return ((dimension + (alignment - 1)) / alignment) * alignment;
}

void vid_cfg_fix_window_dimension(
	int& dimension,
	int min_value,
	int default_value)
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
		vid_cfg_.windowed_width,
		vga_ref_width,
		vid_display_mode_.w
	);
}

void vid_cfg_fix_window_height()
{
	vid_cfg_fix_window_dimension(
		vid_cfg_.windowed_height,
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


} // namespace


void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	VidLayout& dst_param) noexcept
{
	dst_param.windowed_width = src_param.windowed_width;
	dst_param.windowed_height = src_param.windowed_height;

	const auto window_width = src_param.window_width;
	const auto window_height = src_param.window_height;

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

	if (src_param.is_widescreen)
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
	dst_param.window_width = window_width;
	dst_param.window_height = window_height;

	dst_param.window_viewport_left_width = window_viewport_left_width;
	dst_param.window_viewport_right_width = window_viewport_right_width;
	dst_param.window_viewport_top_height = window_viewport_top_height;
	dst_param.window_viewport_bottom_height = window_viewport_bottom_height;

	dst_param.screen_width = screen_width;
	dst_param.screen_height = screen_height;

	dst_param.screen_width_4x3 = screen_width_4x3;

	dst_param.screen_left_filler_width = screen_left_filler_width;
	dst_param.screen_right_filler_width = screen_right_filler_width;

	dst_param.screen_top_filler_height = screen_top_filler_height;
	dst_param.screen_bottom_filler_height = screen_bottom_filler_height;

	dst_param.screen_viewport_left_width = screen_viewport_left_width;
	dst_param.screen_viewport_right_width = screen_viewport_right_width;
	dst_param.screen_viewport_width = screen_viewport_width;

	dst_param.screen_viewport_top_height = screen_viewport_top_height;
	dst_param.screen_viewport_bottom_height = screen_viewport_bottom_height;
	dst_param.screen_viewport_height = screen_viewport_height;
}


CalculateScreenSizeInputParam vid_create_screen_size_param() noexcept
{
	auto window_width = (
		vid_cfg_.is_windowed ?
		vid_cfg_.windowed_width :
		vid_display_mode_.w
	);

	auto window_height = (
		vid_cfg_.is_windowed ?
		vid_cfg_.windowed_height :
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
	result.is_widescreen = vid_cfg_.is_widescreen;
	result.windowed_width = vid_cfg_.windowed_width;
	result.windowed_height = vid_cfg_.windowed_height;
	result.window_width = window_width;
	result.window_height = window_height;

	return result;
}

void vid_calculate_vga_dimensions() noexcept
{
	vga_width = vid_layout_.screen_viewport_width;
	vga_height = vid_align_dimension((10 * vid_layout_.screen_height) / 12);

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


namespace
{


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
	vid_cl_read_bool(vid_get_is_windowed_key_name(), vid_cfg_.is_windowed);
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
	vid_cl_read_int(vid_get_windowed_width_key_name(), vid_cfg_.windowed_width);
}

void vid_cl_read_windowed_height()
{
	vid_cl_read_int(vid_get_windowed_height_key_name(), vid_cfg_.windowed_height);
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
	vid_cl_read_bool(vid_get_is_widescreen_key_name(), vid_cfg_.is_widescreen);
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

void vid_cl_read_renderer_kind()
{
	auto value = bstone::RendererKind::software;

	const auto value_string = ::g_args.get_option_value(vid_get_renderer_key_name());

	if (value_string.empty())
	{
		return;
	}

	if (false)
	{
	}
	else if (value_string == vid_get_auto_detect_value_string())
	{
		value = bstone::RendererKind::auto_detect;
	}
	else if (value_string == vid_get_gl_2_0_value_string())
	{
		value = bstone::RendererKind::gl_2_0;
	}
	else if (value_string == vid_get_gl_3_2_c_value_string())
	{
		value = bstone::RendererKind::gl_3_2_core;
	}
	else if (value_string == vid_get_gles_2_0_value_string())
	{
		value = bstone::RendererKind::gles_2_0;
	}
	else
	{
		value = bstone::RendererKind::software;
	}

	vid_cfg_.renderer_kind_ = value;
}

const std::string& vid_get_vid_string()
{
	static const auto result = std::string{"[VID]"};

	return result;
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

void vid_log_error(
	const std::string& message)
{
	bstone::logger_->write(
		bstone::LoggerMessageKind::error,
		vid_get_vid_string() + ' ' + message
	);
}

void vid_get_current_display_mode()
{
	vid_log("Getting display mode.");

	bstone::SdlEnsureResult{SDL_GetDesktopDisplayMode(0, &vid_display_mode_)};
}

void vid_cl_read()
{
	static auto is_already_read = false;

	if (is_already_read)
	{
		return;
	}

	is_already_read = true;

	vid_cl_read_renderer_kind();
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


} // namespace


const std::string& vid_to_string(
	bool value)
{
	static const auto false_string = std::string{"false"};
	static const auto true_string = std::string{"true"};

	return value ? true_string : false_string;
}

std::string vid_to_string(
	int value)
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
			fail("Unsupported renderer filter kind.");
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
			fail("Unsupported anti-aliasing kind.");
	}
}

const std::string& vid_to_string(
	bstone::RendererKind renderer_kind)
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
			fail("Unsupported renderer kind.");
	}
}

const std::string& vid_to_string(
	bstone::Ren3dKind renderer_kind)
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
			fail("Unsupported renderer kind.");
	}
}

const std::string& vid_to_string(
	bstone::HwTextureMgrUpscaleFilterKind upscale_filter_kind)
{
	switch (upscale_filter_kind)
	{
		case bstone::HwTextureMgrUpscaleFilterKind::none:
			return vid_get_none_value_string();

		case bstone::HwTextureMgrUpscaleFilterKind::xbrz:
			return vid_get_xbrz_value_string();

		default:
			fail("Unsupported texture upscale filter kind.");
	}
}


void vid_initialize_vanilla_raycaster()
{
	SetupWalls();
	NewViewSize();
	SetPlaneViewSize();
}


namespace
{


void vid_log_common_configuration()
{
	vid_log();
	vid_log("--------------------");
	vid_log("Common configuration");
	vid_log("--------------------");

	vid_log("Renderer: " + vid_to_string(vid_cfg_.renderer_kind_));

	vid_log("Is windowed: " + vid_to_string(vid_cfg_.is_windowed));
	vid_log("Window positioned: " + vid_to_string(vid_cfg_.is_positioned_));
	vid_log("Windowed x: " + vid_to_string(vid_cfg_.windowed_x_));
	vid_log("Windowed y: " + vid_to_string(vid_cfg_.windowed_y_));
	vid_log("Windowed width: " + vid_to_string(vid_cfg_.windowed_width));
	vid_log("Windowed height: " + vid_to_string(vid_cfg_.windowed_height));

	vid_log("UI stretched: " + vid_to_string(vid_cfg_.is_ui_stretched_));
	vid_log("Widescreen: " + vid_to_string(vid_cfg_.is_widescreen));

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


} // namespace


void vid_initialize_common()
{
	vid_get_current_display_mode();
	vid_cfg_adjust_window_position();
	vid_cfg_fix_window_size();

	vid_log_common_configuration();
}

void vid_initialize_ui_buffer()
{
	const auto area = vga_ref_width * vga_ref_height;

	vid_ui_buffer_.resize(area);
}


namespace
{


auto g_video = bstone::VideoUPtr{};


} // namespace


std::string vid_get_window_title_for_renderer(
	const std::string& renderer_name)
{
	const auto game_name_and_game_version_string = vid_get_game_name_and_game_version_string();
	const auto port_version_string = vid_get_port_version_string();

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

	const auto width = vid_cfg_.windowed_width;
	const auto height = vid_cfg_.windowed_height;
	const auto stride_rgb_888 = (((3 * width) + 3) / 4) * 4;
	auto src_rgb_888_pixels = std::make_unique<std::uint8_t[]>(stride_rgb_888 * height);

	::g_video->take_screenshot(
		width,
		height,
		stride_rgb_888,
		std::move(src_rgb_888_pixels)
	);
}
catch (const std::exception& ex)
{
	vid_log_error("Failed to take a screenshot.");
	vid_log_error(ex.what());
}


namespace
{


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
		::g_video->present();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	vid_has_vsync = (duration_ms >= min_expected_duration_ms);
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
	::g_video = nullptr;

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
			::g_video = bstone::make_hw_video();
		}
		catch (const bstone::Exception& ex)
		{
			is_try_sw = true;

			vid_log_error(ex.what());
			vid_log("Falling back to software accelerated video system.");
		}
	}

	if (is_try_sw)
	{
		try
		{
			::g_video = bstone::make_sw_video();
		}
		catch (const std::exception& ex)
		{
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
	::g_video = nullptr;
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
	::g_video->fill_palette(red, green, blue);
}

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
{
	::g_video->set_palette(first, count, palette);
}

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
{
	::g_video->get_palette(first, count, palette);
}

// Fades the current palette to the given color in the given number of steps.
void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(red >= 0 && red <= 0xFF);
	assert(green >= 0 && green <= 0xFF);
	assert(blue >= 0 && blue <= 0xFF);
	assert(steps > 0);
	assert(start <= end);

	::g_video->fade_out(start, end, red, green, blue, steps);
}

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps)
{
	assert(start >= 0);
	assert(end >= 0);
	assert(palette != nullptr);
	assert(steps > 0);
	assert(start <= end);

	::g_video->fade_in(start, end, palette, steps);
}

void VL_SetPaletteIntensity(
	int start,
	int end,
	const std::uint8_t* palette,
	int intensity)
{
	auto palette1 = ::VgaPalette{};

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
	bool is_transparent)
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
	bool is_transparent)
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
	::g_video->present();
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
	::g_video->apply_widescreen();
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
	::g_video->clear_vga_buffer();
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
	bstone::Ren3dFilterKind filter)
{
	switch (filter)
	{
		case bstone::Ren3dFilterKind::nearest:
			return vid_get_nearest_value_string();

		case bstone::Ren3dFilterKind::linear:
			return vid_get_linear_value_string();

		default:
			::fail("Invalid filter.");
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
		vid_cfg_.windowed_width = value;
	}
}

void vid_cfg_read_height(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.windowed_height = value;
	}
}

void vid_cfg_read_is_windowed(
	const std::string& value_string)
{
	int value = 0;

	if (bstone::StringHelper::string_to_int(value_string, value))
	{
		vid_cfg_.is_windowed = (value != 0);
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
		vid_cfg_.is_widescreen = (value != 0);
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
	auto value_string = std::string{};

	switch (vid_cfg_.renderer_kind_)
	{
		case bstone::RendererKind::software:
			value_string = vid_get_software_value_string();
			break;

		case bstone::RendererKind::gl_2_0:
			value_string = vid_get_gl_2_0_value_string();
			break;

		case bstone::RendererKind::gl_3_2_core:
			value_string = vid_get_gl_3_2_c_value_string();
			break;

		case bstone::RendererKind::gles_2_0:
			value_string = vid_get_gles_2_0_value_string();
			break;

		case bstone::RendererKind::auto_detect:
		default:
			value_string = vid_get_auto_detect_value_string();
			break;
	}

	cfg_file_write_entry(
		text_writer,
		vid_get_renderer_key_name(),
		value_string
	);
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
		std::to_string(vid_cfg_.is_windowed)
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
		std::to_string(vid_cfg_.windowed_width)
	);

	cfg_file_write_entry(
		text_writer,
		vid_get_windowed_height_key_name(),
		std::to_string(vid_cfg_.windowed_height)
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
		std::to_string(vid_cfg_.is_widescreen)
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

	vid_cfg_.is_windowed = true;
	vid_cfg_.is_positioned_ = false;
	vid_cfg_.windowed_x_ = 0;
	vid_cfg_.windowed_y_ = 0;
	vid_cfg_.windowed_width = default_window_width;
	vid_cfg_.windowed_height = default_window_height;
	vid_cfg_.is_vsync_ = true;

	vid_cfg_.is_ui_stretched_ = false;
	vid_cfg_.is_widescreen = true;

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

VidCfg& vid_cfg_get() noexcept
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
					return item.windowed_width == sdl_mode.w && item.windowed_height == sdl_mode.h;
				}
			);

			if (!is_added)
			{
				result.emplace_back();
				auto& window_size = result.back();
				window_size.windowed_width = sdl_mode.w;
				window_size.windowed_height = sdl_mode.h;

				//
				const auto is_current =
					sdl_mode.w == vid_layout_.windowed_width &&
					sdl_mode.h == vid_layout_.windowed_height;

				window_size.is_current_ = is_current;

				if (is_current)
				{
					is_current_added = true;
				}

				//
				const auto is_custom =
					sdl_mode.w == vid_cfg_.windowed_width &&
					sdl_mode.h == vid_cfg_.windowed_height;

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
			if (lhs.windowed_width != rhs.windowed_width)
			{
				return lhs.windowed_width < rhs.windowed_width;
			}

			return lhs.windowed_height < rhs.windowed_height;
		}
	);

	if (!is_current_added && !is_custom_added)
	{
		result.emplace_back();
		auto& window_size = result.back();
		window_size.windowed_width = vid_cfg_.windowed_width;
		window_size.windowed_height = vid_cfg_.windowed_height;

		window_size.is_current_ = true;
		window_size.is_custom_ = true;
	}

	return result;
}

void vid_draw_ui_sprite(
	int sprite_id,
	int center_x,
	int center_y,
	int new_side)
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
	::g_video->on_load_level();
}

void vid_hw_on_update_wall_switch(
	int x,
	int y)
{
	::g_video->on_update_wall_switch(x, y);
}

void vid_hw_on_move_pushwall()
{
	::g_video->on_move_pushwall();
}

void vid_hw_on_step_pushwall(
	int old_x,
	int old_y)
{
	::g_video->on_step_pushwall(old_x, old_y);
}

void vid_hw_on_pushwall_to_wall(
	int old_x,
	int old_y,
	int new_x,
	int new_y)
{
	::g_video->on_pushwall_to_wall(old_x, old_y, new_x, new_y);
}

void vid_hw_on_move_door(
	int door_index)
{
	::g_video->on_move_door(door_index);
}

void vid_hw_on_update_door_lock(
	int bs_door_index)
{
	::g_video->on_update_door_lock(bs_door_index);
}

void vid_hw_on_remove_static(
	const statobj_t& bs_static)
{
	::g_video->on_remove_static(bs_static);
}

void vid_hw_on_remove_actor(
	const objtype& bs_actor)
{
	::g_video->on_remove_actor(bs_actor);
}

void vid_hw_enable_fizzle_fx(
	bool is_enabled)
{
	::g_video->enable_fizzle_fx(is_enabled);
}

void vid_hw_enable_fizzle_fx_fading(
	bool is_fading)
{
	::g_video->enable_fizzle_fx_fading(is_fading);
}

void vid_hw_set_fizzle_fx_color_index(
	int color_index)
{
	::g_video->set_fizzle_fx_color_index(color_index);
}

void vid_hw_set_fizzle_fx_ratio(
	float ratio)
{
	::g_video->set_fizzle_fx_ratio(ratio);
}

void vid_hw_clear_wall_render_list()
{
	::g_video->clear_wall_render_list();
}

void vid_hw_add_wall_render_item(
	int tile_x,
	int tile_y)
{
	::g_video->add_wall_render_item(tile_x, tile_y);
}

void vid_hw_clear_pushwall_render_list()
{
	::g_video->clear_pushwall_render_list();
}

void vid_hw_add_pushwall_render_item(
	int tile_x,
	int tile_y)
{
	::g_video->add_pushwall_render_item(tile_x, tile_y);
}

void vid_hw_clear_door_render_list()
{
	::g_video->clear_door_render_list();
}

void vid_hw_add_door_render_item(
	int tile_x,
	int tile_y)
{
	::g_video->add_door_render_item(tile_x, tile_y);
}

void vid_hw_clear_static_render_list()
{
	::g_video->clear_static_render_list();
}

void vid_hw_add_static_render_item(
	int bs_static_index)
{
	::g_video->add_static_render_item(bs_static_index);
}

void vid_hw_clear_actor_render_list()
{
	::g_video->clear_actor_render_list();
}

void vid_hw_add_actor_render_item(
	int bs_actor_index)
{
	::g_video->add_actor_render_item(bs_actor_index);
}

const bstone::Rgba8Palette& vid_hw_get_default_palette()
{
	return ::g_video->get_default_palette();
}

void vid_apply_window_mode()
{
	::g_video->apply_window_mode();
}

void vid_apply_vsync()
{
	::g_video->apply_vsync();
}

void vid_apply_msaa()
{
	::g_video->apply_msaa();
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
		::g_video->get_device_features().is_vsync_available_ &&
			::g_video->get_device_features().is_vsync_requires_restart_
	)
	{
		is_restart = true;
	}

	if (!is_restart &&
		(vid_cfg_.aa_kind_ != video_mode_cfg.aa_kind_ ||
			vid_cfg_.aa_degree_ != video_mode_cfg.aa_degree_) &&
		video_mode_cfg.aa_kind_ == bstone::Ren3dAaKind::ms &&
		::g_video->get_device_features().is_msaa_available_ &&
		::g_video->get_device_features().is_msaa_requires_restart_
	)
	{
		is_restart = true;
	}

	const auto is_window_modified = (
		vid_cfg_.is_windowed != video_mode_cfg.is_windowed ||
		(
			video_mode_cfg.is_windowed &&
			(
				vid_cfg_.windowed_width != video_mode_cfg.windowed_width ||
				vid_cfg_.windowed_height != video_mode_cfg.windowed_height
			)
		)
	);

	const auto is_vsync_modified = (vid_cfg_.is_vsync_ != video_mode_cfg.is_vsync_);

	const auto is_aa_modified = (
		vid_cfg_.aa_kind_ != video_mode_cfg.aa_kind_ ||
		vid_cfg_.aa_degree_ != video_mode_cfg.aa_degree_);

	vid_cfg_.renderer_kind_ = video_mode_cfg.renderer_kind_;
	vid_cfg_.is_windowed = video_mode_cfg.is_windowed;
	vid_cfg_.windowed_width = video_mode_cfg.windowed_width;
	vid_cfg_.windowed_height = video_mode_cfg.windowed_height;
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
	::g_video->update_samplers();
}

void vid_apply_2d_image_filter()
{
	::g_video->update_samplers();
}

void vid_apply_3d_image_filter()
{
	::g_video->update_samplers();
}

void vid_apply_mipmap_filter()
{
	::g_video->update_samplers();
}

void vid_apply_upscale()
{
	::g_video->apply_texture_upscale();
}

int vid_clamp_filler_color_index(
	int filler_color_index) noexcept
{
	return bstone::math::clamp(filler_color_index, 0, 255);
}

void vid_apply_filler_color()
{
	::g_video->apply_filler_color_index();
}

void vid_apply_external_textures()
{
	::g_video->apply_external_textures();
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
		lhs.is_windowed == rhs.is_windowed &&
		lhs.windowed_width == rhs.windowed_width &&
		lhs.windowed_height == rhs.windowed_height &&
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

bool vid_is_hw()
{
	return ::g_video && ::g_video->is_hardware();
}
// BBi
