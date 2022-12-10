/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
#include "bstone_string_view.h"
#include "bstone_sw_video.h"
#include "bstone_text_writer.h"
#include "bstone_time.h"
#include "bstone_version.h"


namespace {

constexpr int default_window_width = 640;
constexpr int default_window_height = 480;

// Common names.

constexpr auto vid_none_string = bstone::StringView{"none"};
constexpr auto vid_nearest_string = bstone::StringView{"nearest"};
constexpr auto vid_linear_string = bstone::StringView{"linear"};

constexpr bstone::StringView vid_filter_strings[] =
{
	vid_nearest_string,
	vid_linear_string,
};

// vid_renderer

constexpr auto vid_renderer_cvar_name = bstone::StringView{"vid_renderer"};
constexpr auto vid_renderer_cvar_auto_detect = bstone::StringView{"auto-detect"};
constexpr auto vid_renderer_cvar_software = bstone::StringView{"software"};
constexpr auto vid_renderer_cvar_gl_2_0 = bstone::StringView{"gl_2_0"};
constexpr auto vid_renderer_cvar_gl_3_2_c = bstone::StringView{"gl_3_2_c"};
constexpr auto vid_renderer_cvar_gles_2_0 = bstone::StringView{"gles_2_0"};

constexpr bstone::StringView vid_renderer_cvar_values[] =
{
	vid_renderer_cvar_auto_detect,
	vid_renderer_cvar_software,
	vid_renderer_cvar_gl_2_0,
	vid_renderer_cvar_gl_3_2_c,
	vid_renderer_cvar_gles_2_0,
};

auto vid_renderer_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_renderer_cvar_name,
	bstone::CVarFlags::archive,
	vid_renderer_cvar_auto_detect,
	bstone::make_span(vid_renderer_cvar_values)};

// vid_is_positioned

constexpr auto vid_is_positioned_cvar_name = bstone::StringView{"vid_is_positioned"};
constexpr auto vid_is_positioned_cvar_default = false;

auto vid_is_positioned_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_positioned_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_positioned_cvar_default};

// vid_x

constexpr auto vid_x_cvar_name = bstone::StringView{"vid_x"};
constexpr auto vid_x_cvar_default = 0;

auto vid_x_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_x_cvar_name,
	bstone::CVarFlags::archive,
	vid_x_cvar_default};

// vid_y

constexpr auto vid_y_cvar_name = bstone::StringView{"vid_y"};
constexpr auto vid_y_cvar_default = 0;

auto vid_y_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_y_cvar_name,
	bstone::CVarFlags::archive,
	vid_y_cvar_default};

// vid_width

constexpr auto vid_width_cvar_name = bstone::StringView{"vid_width"};
constexpr auto vid_width_cvar_default = default_window_width;

auto vid_width_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_width_cvar_name,
	bstone::CVarFlags::archive,
	vid_width_cvar_default};

// vid_height

constexpr auto vid_height_cvar_name = bstone::StringView{"vid_height"};
constexpr auto vid_height_cvar_default = default_window_height;

auto vid_height_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_height_cvar_name,
	bstone::CVarFlags::archive,
	vid_height_cvar_default};

// vid_is_vsync

constexpr auto vid_is_vsync_cvar_name = bstone::StringView{"vid_is_vsync"};
constexpr auto vid_is_vsync_cvar_default = true;

auto vid_is_vsync_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_vsync_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_vsync_cvar_default};

// vid_is_ui_stretched

constexpr auto vid_is_ui_stretched_cvar_name = bstone::StringView{"vid_is_ui_stretched"};
constexpr auto vid_is_ui_stretched_cvar_default = false;

auto vid_is_ui_stretched_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_ui_stretched_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_ui_stretched_cvar_default};

// vid_is_widescreen

constexpr auto vid_is_widescreen_cvar_name = bstone::StringView{"vid_is_widescreen"};
constexpr auto vid_is_widescreen_cvar_default = true;

auto vid_is_widescreen_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_is_widescreen_cvar_name,
	bstone::CVarFlags::archive,
	vid_is_widescreen_cvar_default};

// vid_aa_kind

constexpr auto vid_aa_kind_cvar_name = bstone::StringView{"vid_aa_kind"};
constexpr auto vid_aa_kind_cvar_msaa = bstone::StringView{"msaa"};

constexpr bstone::StringView vid_aa_kind_cvar_values[] =
{
	vid_none_string,
	vid_aa_kind_cvar_msaa,
};

auto vid_aa_kind_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_aa_kind_cvar_name,
	bstone::CVarFlags::archive,
	vid_none_string,
	bstone::make_span(vid_aa_kind_cvar_values)};

// vid_aa_degree

constexpr auto vid_aa_degree_cvar_name = bstone::StringView{"vid_aa_degree"};
constexpr auto vid_aa_degree_cvar_min = vid_min_aa;
constexpr auto vid_aa_degree_cvar_max = vid_max_aa;
constexpr auto vid_aa_degree_cvar_default = vid_aa_degree_cvar_min;

auto vid_aa_degree_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_aa_degree_cvar_name,
	bstone::CVarFlags::archive,
	vid_aa_degree_cvar_default,
	vid_aa_degree_cvar_min,
	vid_aa_degree_cvar_max};

// vid_texture_upscale_filter

constexpr auto vid_texture_upscale_filter_cvar_name = bstone::StringView{"vid_texture_upscale_filter"};
constexpr auto vid_texture_upscale_filter_cvar_xbrz = bstone::StringView{"xbrz"};

constexpr bstone::StringView vid_texture_upscale_filter_cvar_values[] =
{
	vid_none_string,
	vid_texture_upscale_filter_cvar_xbrz,
};

auto vid_texture_upscale_filter_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_texture_upscale_filter_cvar_name,
	bstone::CVarFlags::archive,
	vid_none_string,
	bstone::make_span(vid_texture_upscale_filter_cvar_values)};

// vid_texture_upscale_xbrz_degree

constexpr auto vid_texture_upscale_xbrz_degree_cvar_name = bstone::StringView{"vid_texture_upscale_xbrz_degree"};
constexpr auto vid_texture_upscale_xbrz_degree_cvar_default = 0;

auto vid_texture_upscale_xbrz_degree_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_texture_upscale_xbrz_degree_cvar_name,
	bstone::CVarFlags::archive,
	vid_texture_upscale_xbrz_degree_cvar_default};

// vid_2d_texture_filter

constexpr auto vid_2d_texture_filter_cvar_name = bstone::StringView{"vid_2d_texture_filter"};

auto vid_2d_texture_filter_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_2d_texture_filter_cvar_name,
	bstone::CVarFlags::archive,
	vid_nearest_string,
	bstone::make_span(vid_filter_strings)};

// vid_3d_texture_image_filter

constexpr auto vid_3d_texture_image_filter_cvar_name = bstone::StringView{"vid_3d_texture_image_filter"};

auto vid_3d_texture_image_filter_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_3d_texture_image_filter_cvar_name,
	bstone::CVarFlags::archive,
	vid_nearest_string,
	bstone::make_span(vid_filter_strings)};

// vid_3d_texture_mipmap_filter

constexpr auto vid_3d_texture_mipmap_filter_cvar_name = bstone::StringView{"vid_3d_texture_mipmap_filter"};

auto vid_3d_texture_mipmap_filter_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	vid_3d_texture_mipmap_filter_cvar_name,
	bstone::CVarFlags::archive,
	vid_nearest_string,
	bstone::make_span(vid_filter_strings)};

// vid_3d_texture_anisotropy

constexpr auto vid_3d_texture_anisotropy_cvar_name = bstone::StringView{"vid_3d_texture_anisotropy"};
constexpr auto vid_3d_texture_anisotropy_cvar_min = vid_min_anisotropic;
constexpr auto vid_3d_texture_anisotropy_cvar_max = vid_max_anisotropic;
constexpr auto vid_3d_texture_anisotropy_cvar_default = vid_min_anisotropic;

auto vid_3d_texture_anisotropy_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_3d_texture_anisotropy_cvar_name,
	bstone::CVarFlags::archive,
	vid_3d_texture_anisotropy_cvar_default,
	vid_3d_texture_anisotropy_cvar_min,
	vid_3d_texture_anisotropy_cvar_max};

// vid_filler_color_index

constexpr auto vid_filler_color_index_cvar_name = bstone::StringView{"vid_filler_color_index"};
constexpr auto vid_filler_color_index_cvar_min = 0;
constexpr auto vid_filler_color_index_cvar_max = 255;
constexpr auto vid_filler_color_index_cvar_default = vid_filler_color_index_cvar_min;

auto vid_filler_color_index_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	vid_filler_color_index_cvar_name,
	bstone::CVarFlags::archive,
	vid_filler_color_index_cvar_default,
	vid_filler_color_index_cvar_min,
	vid_filler_color_index_cvar_max};

// vid_external_textures

constexpr auto vid_external_textures_cvar_name = bstone::StringView{"vid_external_textures"};
constexpr auto vid_external_textures_cvar_default = false;

auto vid_external_textures_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	vid_external_textures_cvar_name,
	bstone::CVarFlags::archive,
	vid_external_textures_cvar_default};

} // namespace

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
catch (...)
{
	vid_log_error("Failed to save a screenshot.");

	const auto messages = bstone::extract_exception_messages();

	for (const auto& message : messages)
	{
		vid_log_error(message);
	}
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

} // namespace


VgaBuffer vid_ui_buffer_;
UiMaskBuffer vid_mask_buffer_;

VidLayout vid_layout_;


namespace
{


SDL_DisplayMode vid_display_mode_;

int vid_align_dimension(
	int dimension) noexcept
{
	const auto alignment = 2;

	return ((dimension + (alignment - 1)) / alignment) * alignment;
}

void vid_cfg_fix_window_dimension(
	int& dimension,
	int min_value,
	int default_value) noexcept
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

void vid_cfg_fix_window_width() noexcept
{
	auto width = vid_cfg_get_width();

	vid_cfg_fix_window_dimension(
		width,
		vga_ref_width,
		vid_display_mode_.w);

	vid_cfg_set_width(width);
}

void vid_cfg_fix_window_height() noexcept
{
	auto height = vid_cfg_get_height();

	vid_cfg_fix_window_dimension(
		height,
		vga_ref_height_4x3,
		vid_display_mode_.h);

	vid_cfg_set_height(height);
}

void vid_cfg_fix_window_size()
{
	vid_cfg_fix_window_width();
	vid_cfg_fix_window_height();
}

void vid_cfg_adjust_window_position()
{
	vid_cfg_set_width(std::max(vid_cfg_get_width(), 0));
	vid_cfg_set_height(std::max(vid_cfg_get_height(), 0));
}

} // namespace


void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	VidLayout& dst_param) noexcept
{
	dst_param.width = src_param.width;
	dst_param.height = src_param.height;

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
	vid_cfg_set_width(std::max(vid_cfg_get_width(), vga_ref_width));
	vid_cfg_set_height(std::max(vid_cfg_get_height(), vga_ref_height_4x3));

	auto result = CalculateScreenSizeInputParam{};
	result.is_widescreen = vid_cfg_is_widescreen();
	result.width = vid_cfg_get_width();
	result.height = vid_cfg_get_height();
	result.window_width = vid_cfg_get_width();
	result.window_height = vid_cfg_get_height();
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
try
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
catch (...)
{
	fail_nested(__func__);
}

std::string vid_get_port_version_string()
try
{
	return "BStone v" + bstone::Version::get_string();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_initialize_cvars(bstone::CVarMgr& cvar_mgr)
{
	cvar_mgr.add(vid_renderer_cvar);
	cvar_mgr.add(vid_is_positioned_cvar);
	cvar_mgr.add(vid_x_cvar);
	cvar_mgr.add(vid_y_cvar);
	cvar_mgr.add(vid_width_cvar);
	cvar_mgr.add(vid_height_cvar);
	cvar_mgr.add(vid_is_vsync_cvar);
	cvar_mgr.add(vid_is_ui_stretched_cvar);
	cvar_mgr.add(vid_is_widescreen_cvar);
	cvar_mgr.add(vid_aa_kind_cvar);
	cvar_mgr.add(vid_aa_degree_cvar);
	cvar_mgr.add(vid_texture_upscale_filter_cvar);
	cvar_mgr.add(vid_texture_upscale_xbrz_degree_cvar);
	cvar_mgr.add(vid_2d_texture_filter_cvar);
	cvar_mgr.add(vid_3d_texture_image_filter_cvar);
	cvar_mgr.add(vid_3d_texture_mipmap_filter_cvar);
	cvar_mgr.add(vid_3d_texture_anisotropy_cvar);
	cvar_mgr.add(vid_filler_color_index_cvar);
	cvar_mgr.add(vid_external_textures_cvar);
}

namespace
{

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
try
{
	bstone::sdl_ensure_result(::SDL_GetCurrentDisplayMode(0, &vid_display_mode_));
}
catch (...)
{
	fail_nested(__func__);
}

} // namespace


std::string vid_to_string(bool value)
{
	return value ? "true" : "false";
}

std::string vid_to_string(int value)
try
{
	return std::to_string(value);
}
catch (...)
{
	fail_nested(__func__);
}

std::string vid_to_string(bstone::Ren3dFilterKind filter_type)
try
{
	auto filter_type_sv = bstone::StringView{};

	switch (filter_type)
	{
		case bstone::Ren3dFilterKind::nearest:
			filter_type_sv = vid_nearest_string;
			break;

		case bstone::Ren3dFilterKind::linear:
			filter_type_sv = vid_linear_string;
			break;

		default:
			fail("Unsupported renderer filter type.");
	}

	return std::string{filter_type_sv.get_data(), static_cast<std::size_t>(filter_type_sv.get_size())};
}
catch (...)
{
	fail_nested(__func__);
}

std::string vid_to_string(bstone::Ren3dAaKind aa_type)
try
{
	auto aa_type_sv = bstone::StringView{};

	switch (aa_type)
	{
		case bstone::Ren3dAaKind::ms:
			aa_type_sv = vid_aa_kind_cvar_msaa;
			break;

		case bstone::Ren3dAaKind::none:
			aa_type_sv = vid_none_string;
			break;

		default:
			fail("Unsupported anti-aliasing type.");
	}

	return std::string{aa_type_sv.get_data(), static_cast<std::size_t>(aa_type_sv.get_size())};
}
catch (...)
{
	fail_nested(__func__);
}

std::string vid_to_string(bstone::RendererKind renderer_type)
try
{
	auto renderer_type_sv = bstone::StringView{};

	switch (renderer_type)
	{
		case bstone::RendererKind::auto_detect:
			renderer_type_sv = "Auto-detect";
			break;

		case bstone::RendererKind::software:
			renderer_type_sv = "Software";
			break;

		case bstone::RendererKind::gl_2_0:
			renderer_type_sv = "OpenGL 2.0";
			break;

		case bstone::RendererKind::gl_3_2_core:
			renderer_type_sv = "OpenGL 3.2 core";
			break;

		case bstone::RendererKind::gles_2_0:
			renderer_type_sv = "OpenGL ES 2.0";
			break;

		default:
			fail("Unsupported renderer kind.");
	}

	return std::string{renderer_type_sv.get_data(), static_cast<std::size_t>(renderer_type_sv.get_size())};
}
catch (...)
{
	fail_nested(__func__);
}

std::string vid_to_string(bstone::Ren3dKind renderer_type)
try
{
	switch (renderer_type)
	{
		case bstone::Ren3dKind::gl_2_0: return "OpenGL 2.0";
		case bstone::Ren3dKind::gl_3_2_core: return "OpenGL 3.2 core";
		case bstone::Ren3dKind::gles_2_0: return "OpenGL ES 2.0";
		default: fail("Unsupported renderer kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

std::string vid_to_string(bstone::HwTextureMgrUpscaleFilterKind upscale_filter_type)
try
{
	auto upscale_filter_type_sv = bstone::StringView{};

	switch (upscale_filter_type)
	{
		case bstone::HwTextureMgrUpscaleFilterKind::none:
			upscale_filter_type_sv = vid_none_string;
			break;

		case bstone::HwTextureMgrUpscaleFilterKind::xbrz:
			upscale_filter_type_sv = vid_texture_upscale_filter_cvar_xbrz;
			break;

		default:
			fail("Unsupported texture upscale filter kind.");
	}

	return std::string{
		upscale_filter_type_sv.get_data(),
		static_cast<std::size_t>(upscale_filter_type_sv.get_size())};
}
catch (...)
{
	fail_nested(__func__);
}

void vid_initialize_vanilla_raycaster()
try
{
	SetupWalls();
	NewViewSize();
	SetPlaneViewSize();
}
catch (...)
{
	fail_nested(__func__);
}


namespace
{


void vid_log_common_configuration()
{
	vid_log();
	vid_log("--------------------");
	vid_log("Common configuration");
	vid_log("--------------------");

	vid_log("Renderer: " + vid_to_string(vid_cfg_get_renderer_kind()));

	vid_log("Window positioned: " + vid_to_string(vid_cfg_is_positioned()));
	vid_log("Windowed x: " + vid_to_string(vid_cfg_get_x()));
	vid_log("Windowed y: " + vid_to_string(vid_cfg_get_y()));
	vid_log("Windowed width: " + vid_to_string(vid_cfg_get_width()));
	vid_log("Windowed height: " + vid_to_string(vid_cfg_get_height()));

	vid_log("UI stretched: " + vid_to_string(vid_cfg_is_ui_stretched()));
	vid_log("Widescreen: " + vid_to_string(vid_cfg_is_widescreen()));

	vid_log("2D texture filter: " + vid_to_string(vid_cfg_get_2d_texture_filter()));

	vid_log("3D texture image filter: " + vid_to_string(vid_cfg_get_3d_texture_image_filter()));
	vid_log("3D texture mipmap filter: " + vid_to_string(vid_cfg_get_3d_texture_mipmap_filter()));

	vid_log("Texture anisotropy: " + vid_to_string(vid_cfg_get_3d_texture_anisotropy()));

	vid_log("Texture upscale filter: " + vid_to_string(vid_cfg_get_texture_upscale_kind()));
	vid_log("Texture upscale xBRZ factor: " + vid_to_string(vid_cfg_get_texture_upscale_xbrz_degree()));

	vid_log("Anti-aliasing kind: " + vid_to_string(vid_cfg_get_aa_kind()));
	vid_log("Anti-aliasing value: " + vid_to_string(vid_cfg_get_aa_degree()));

	vid_log("--------------------");
}


} // namespace


void vid_initialize_common()
try
{
	vid_get_current_display_mode();
	vid_cfg_adjust_window_position();
	vid_cfg_fix_window_size();

	vid_log_common_configuration();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_initialize_ui_buffer()
try
{
	const auto area = vga_ref_width * vga_ref_height;

	vid_ui_buffer_.resize(area);
}
catch (...)
{
	fail_nested(__func__);
}


namespace
{


auto g_video = bstone::VideoUPtr{};


} // namespace


std::string vid_get_window_title_for_renderer(
	const std::string& renderer_name)
try
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
catch (...)
{
	fail_nested(__func__);
}

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically)
try
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
catch (...)
{
	fail_nested(__func__);
}

void vid_take_screenshot()
try
{
	vid_is_take_screenshot_scheduled = false;

	const auto width = vid_cfg_get_width();
	const auto height = vid_cfg_get_height();
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
try
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
		::g_video->vsync_present();
	}

	const auto after_timestamp = Clock::now();

	const auto duration = after_timestamp - before_timestamp;

	const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		duration).count();

	vid_has_vsync = (duration_ms >= min_expected_duration_ms);
}
catch (...)
{
	fail_nested(__func__);
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
try
{
	::g_video = nullptr;

	const auto is_sw = (vid_cfg_get_renderer_kind() == bstone::RendererKind::software);

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
catch (...)
{
	fail_nested(__func__);
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
try
{
	::g_video->fill_palette(red, green, blue);
}
catch (...)
{
	fail_nested(__func__);
}

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette)
try
{
	::g_video->set_palette(first, count, palette);
}
catch (...)
{
	fail_nested(__func__);
}

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette)
try
{
	::g_video->get_palette(first, count, palette);
}
catch (...)
{
	fail_nested(__func__);
}

// Fades the current palette to the given color in the given number of steps.
void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps)
try
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
catch (...)
{
	fail_nested(__func__);
}

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps)
try
{
	assert(start >= 0);
	assert(end >= 0);
	assert(palette != nullptr);
	assert(steps > 0);
	assert(start <= end);

	::g_video->fade_in(start, end, palette, steps);
}
catch (...)
{
	fail_nested(__func__);
}

void VL_SetPaletteIntensity(
	int start,
	int end,
	const std::uint8_t* palette,
	int intensity)
try
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
catch (...)
{
	fail_nested(__func__);
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
try
{
	::g_video->present();
}
catch (...)
{
	fail_nested(__func__);
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
try
{
	::g_video->apply_widescreen();
}
catch (...)
{
	fail_nested(__func__);
}

bstone::RendererKind vid_cfg_get_renderer_kind() noexcept
{
	const auto renderer_sv = vid_renderer_cvar.get_string();

	if (renderer_sv == vid_renderer_cvar_software)
	{
		return bstone::RendererKind::software;
	}

	if (renderer_sv == vid_renderer_cvar_gl_2_0)
	{
		return bstone::RendererKind::gl_2_0;
	}

	if (renderer_sv == vid_renderer_cvar_gl_3_2_c)
	{
		return bstone::RendererKind::gl_3_2_core;
	}

	if (renderer_sv == vid_renderer_cvar_gles_2_0)
	{
		return bstone::RendererKind::gles_2_0;
	}

	return bstone::RendererKind::auto_detect;
}

void vid_cfg_set_renderer_kind(bstone::RendererKind renderer_type)
{
	auto renderer_sv = bstone::StringView{};

	switch (renderer_type)
	{
		case bstone::RendererKind::software:
			renderer_sv = vid_renderer_cvar_software;
			break;

		case bstone::RendererKind::gl_2_0:
			renderer_sv = vid_renderer_cvar_gl_2_0;
			break;

		case bstone::RendererKind::gl_3_2_core:
			renderer_sv = vid_renderer_cvar_gl_3_2_c;
			break;

		case bstone::RendererKind::gles_2_0:
			renderer_sv = vid_renderer_cvar_gles_2_0;
			break;

		default:
			renderer_sv = vid_renderer_cvar_auto_detect;
			break;
	}

	vid_renderer_cvar.set_string(renderer_sv);
}

bool vid_cfg_is_positioned() noexcept
{
	return vid_is_positioned_cvar.get_bool();
}

bool vid_cfg_is_vsync() noexcept
{
	return vid_is_vsync_cvar.get_bool();
}

void vid_cfg_set_is_vsync(bool is_enabled)
{
	vid_is_vsync_cvar.set_bool(is_enabled);
}

bool vid_cfg_is_ui_stretched() noexcept
{
	return vid_is_ui_stretched_cvar.get_bool();
}

void vid_cfg_set_is_ui_stretched(bool is_enabled)
{
	vid_is_ui_stretched_cvar.set_bool(is_enabled);
}

bool vid_cfg_is_widescreen() noexcept
{
	return vid_is_widescreen_cvar.get_bool();
}

void vid_cfg_set_is_widescreen(bool is_enabled)
{
	vid_is_widescreen_cvar.set_bool(is_enabled);
}

int vid_cfg_get_x() noexcept
{
	return vid_x_cvar.get_int32();
}

int vid_cfg_get_y() noexcept
{
	return vid_y_cvar.get_int32();
}

int vid_cfg_get_width() noexcept
{
	return vid_width_cvar.get_int32();
}

void vid_cfg_set_width(int width)
{
	vid_width_cvar.set_int32(width);
}

int vid_cfg_get_height() noexcept
{
	return vid_height_cvar.get_int32();
}

void vid_cfg_set_height(int height)
{
	vid_height_cvar.set_int32(height);
}

namespace {

bstone::Ren3dFilterKind vid_get_filter_type_from_sv(bstone::StringView filter_type_sv) noexcept
{
	if (filter_type_sv == vid_linear_string)
	{
		return bstone::Ren3dFilterKind::linear;
	}

	return bstone::Ren3dFilterKind::nearest;
}

bstone::StringView vid_get_sv_from_filter_type(bstone::Ren3dFilterKind filter_type) noexcept
{
	if (filter_type == bstone::Ren3dFilterKind::linear)
	{
		return vid_linear_string;
	}

	return vid_nearest_string;
}

} // namespace

bstone::Ren3dFilterKind vid_cfg_get_2d_texture_filter() noexcept
{
	return vid_get_filter_type_from_sv(vid_2d_texture_filter_cvar.get_string());
}

void vid_cfg_set_2d_texture_filter(bstone::Ren3dFilterKind filter)
{
	vid_2d_texture_filter_cvar.set_string(vid_get_sv_from_filter_type(filter));
}

bstone::Ren3dFilterKind vid_cfg_get_3d_texture_image_filter() noexcept
{
	return vid_get_filter_type_from_sv(vid_3d_texture_image_filter_cvar.get_string());
}

void vid_cfg_set_3d_texture_image_filter(bstone::Ren3dFilterKind filter)
{
	vid_3d_texture_image_filter_cvar.set_string(vid_get_sv_from_filter_type(filter));
}

bstone::Ren3dFilterKind vid_cfg_get_3d_texture_mipmap_filter() noexcept
{
	return vid_get_filter_type_from_sv(vid_3d_texture_mipmap_filter_cvar.get_string());
}

void vid_cfg_set_3d_texture_mipmap_filter(bstone::Ren3dFilterKind filter)
{
	vid_3d_texture_mipmap_filter_cvar.set_string(vid_get_sv_from_filter_type(filter));
}

int vid_cfg_get_3d_texture_anisotropy() noexcept
{
	return vid_3d_texture_anisotropy_cvar.get_int32();
}

void vid_cfg_set_3d_texture_anisotropy(int anisotropy)
{
	vid_3d_texture_anisotropy_cvar.set_int32(anisotropy);
}

bstone::Ren3dAaKind vid_cfg_get_aa_kind() noexcept
{
	if (vid_aa_kind_cvar.get_string() == vid_aa_kind_cvar_msaa)
	{
		return bstone::Ren3dAaKind::ms;
	}

	return bstone::Ren3dAaKind::none;
}

void vid_cfg_set_aa_kind(bstone::Ren3dAaKind aa_type)
{
	auto aa_type_sv = bstone::StringView{};

	switch (aa_type)
	{
		case bstone::Ren3dAaKind::ms:
			aa_type_sv = vid_aa_kind_cvar_msaa;
			break;

		default:
			aa_type_sv = vid_none_string;
			break;
	}

	vid_aa_kind_cvar.set_string(aa_type_sv);
}

int vid_cfg_get_aa_degree() noexcept
{
	return vid_aa_degree_cvar.get_int32();
}

void vid_cfg_set_aa_degree(int degree)
{
	vid_aa_degree_cvar.set_int32(degree);
}

bstone::HwTextureMgrUpscaleFilterKind vid_cfg_get_texture_upscale_kind() noexcept
{
	if (vid_texture_upscale_filter_cvar.get_string() == vid_texture_upscale_filter_cvar_xbrz)
	{
		return bstone::HwTextureMgrUpscaleFilterKind::xbrz;
	}

	return bstone::HwTextureMgrUpscaleFilterKind::none;
}

void vid_cfg_set_texture_upscale_kind(bstone::HwTextureMgrUpscaleFilterKind filter)
{
	auto filter_sv = bstone::StringView{};

	switch (filter)
	{
		case bstone::HwTextureMgrUpscaleFilterKind::xbrz:
			filter_sv = vid_texture_upscale_filter_cvar_xbrz;
			break;

		default:
			filter_sv = vid_none_string;
			break;
	}

	vid_texture_upscale_filter_cvar.set_string(filter_sv);
}

int vid_cfg_get_texture_upscale_xbrz_degree() noexcept
{
	return vid_texture_upscale_xbrz_degree_cvar.get_int32();
}

void vid_cfg_set_texture_upscale_xbrz_degree(int degree)
{
	vid_texture_upscale_xbrz_degree_cvar.set_int32(degree);
}

int vid_cfg_get_filler_color_index() noexcept
{
	return vid_filler_color_index_cvar.get_int32();
}

void vid_cfg_set_filler_color_index(int index)
{
	vid_filler_color_index_cvar.set_bool(index);
}

bool vid_cfg_is_external_textures_enabled() noexcept
{
	return vid_external_textures_cvar.get_bool();
}

void vid_cfg_set_is_external_textures_enabled(bool is_enabled)
{
	vid_external_textures_cvar.set_bool(is_enabled);
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
try
{
	::g_video->clear_vga_buffer();
}
catch (...)
{
	fail_nested(__func__);
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

VideoModeCfg vid_cfg_get_video_mode() noexcept
{
	auto cfg = VideoModeCfg{};
	cfg.renderer_kind_ = vid_cfg_get_renderer_kind();
	cfg.width = vid_cfg_get_width();
	cfg.height = vid_cfg_get_height();
	cfg.is_vsync_ = vid_cfg_is_vsync();
	cfg.aa_kind_ = vid_cfg_get_aa_kind();
	cfg.aa_degree_ = vid_cfg_get_aa_degree();
	return cfg;
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
try
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
					return item.width == sdl_mode.w && item.height == sdl_mode.h;
				}
			);

			if (!is_added)
			{
				result.emplace_back();
				auto& window_size = result.back();
				window_size.width = sdl_mode.w;
				window_size.height = sdl_mode.h;

				//
				const auto is_current =
					sdl_mode.w == vid_layout_.width &&
					sdl_mode.h == vid_layout_.height;

				window_size.is_current_ = is_current;

				if (is_current)
				{
					is_current_added = true;
				}

				//
				const auto is_custom =
					sdl_mode.w == vid_cfg_get_width() &&
					sdl_mode.h == vid_cfg_get_height();

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
			if (lhs.width != rhs.width)
			{
				return lhs.width < rhs.width;
			}

			return lhs.height < rhs.height;
		}
	);

	if (!is_current_added && !is_custom_added)
	{
		result.emplace_back();
		auto& window_size = result.back();
		window_size.width = vid_cfg_get_width();
		window_size.height = vid_cfg_get_height();

		window_size.is_current_ = true;
		window_size.is_custom_ = true;
	}

	return result;
}
catch (...)
{
	fail_nested(__func__);
}

void vid_draw_ui_sprite(
	int sprite_id,
	int center_x,
	int center_y,
	int new_side)
try
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
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_load_level()
try
{
	::g_video->on_load_level();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_update_wall_switch(
	int x,
	int y)
try
{
	::g_video->on_update_wall_switch(x, y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_move_pushwall()
try
{
	::g_video->on_move_pushwall();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_step_pushwall(
	int old_x,
	int old_y)
try
{
	::g_video->on_step_pushwall(old_x, old_y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_pushwall_to_wall(
	int old_x,
	int old_y,
	int new_x,
	int new_y)
try
{
	::g_video->on_pushwall_to_wall(old_x, old_y, new_x, new_y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_move_door(
	int door_index)
try
{
	::g_video->on_move_door(door_index);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_update_door_lock(
	int bs_door_index)
try
{
	::g_video->on_update_door_lock(bs_door_index);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_remove_static(
	const statobj_t& bs_static)
try
{
	::g_video->on_remove_static(bs_static);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_on_remove_actor(
	const objtype& bs_actor)
try
{
	::g_video->on_remove_actor(bs_actor);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_enable_fizzle_fx(
	bool is_enabled)
try
{
	::g_video->enable_fizzle_fx(is_enabled);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_enable_fizzle_fx_fading(
	bool is_fading)
try
{
	::g_video->enable_fizzle_fx_fading(is_fading);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_set_fizzle_fx_color_index(
	int color_index)
try
{
	::g_video->set_fizzle_fx_color_index(color_index);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_set_fizzle_fx_ratio(
	float ratio)
try
{
	::g_video->set_fizzle_fx_ratio(ratio);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_clear_wall_render_list()
try
{
	::g_video->clear_wall_render_list();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_add_wall_render_item(
	int tile_x,
	int tile_y)
try
{
	::g_video->add_wall_render_item(tile_x, tile_y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_clear_pushwall_render_list()
try
{
	::g_video->clear_pushwall_render_list();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_add_pushwall_render_item(
	int tile_x,
	int tile_y)
try
{
	::g_video->add_pushwall_render_item(tile_x, tile_y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_clear_door_render_list()
try
{
	::g_video->clear_door_render_list();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_add_door_render_item(
	int tile_x,
	int tile_y)
try
{
	::g_video->add_door_render_item(tile_x, tile_y);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_clear_static_render_list()
try
{
	::g_video->clear_static_render_list();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_add_static_render_item(
	int bs_static_index)
try
{
	::g_video->add_static_render_item(bs_static_index);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_clear_actor_render_list()
try
{
	::g_video->clear_actor_render_list();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_hw_add_actor_render_item(
	int bs_actor_index)
try
{
	::g_video->add_actor_render_item(bs_actor_index);
}
catch (...)
{
	fail_nested(__func__);
}

const bstone::Rgba8Palette& vid_hw_get_default_palette()
try
{
	return ::g_video->get_default_palette();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_window_mode()
try
{
	::g_video->apply_window_mode();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_vsync()
try
{
	::g_video->apply_vsync();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_msaa()
try
{
	::g_video->apply_msaa();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_video_mode(
	const VideoModeCfg& video_mode_cfg)
try
{
	auto is_restart = false;

	if (!is_restart &&
		vid_cfg_get_renderer_kind() != video_mode_cfg.renderer_kind_)
	{
		is_restart = true;
	}

	if (!is_restart &&
		vid_cfg_is_vsync() != video_mode_cfg.is_vsync_ &&
		::g_video->get_device_features().is_vsync_available_ &&
			::g_video->get_device_features().is_vsync_requires_restart_)
	{
		is_restart = true;
	}

	if (!is_restart &&
		(vid_cfg_get_aa_kind() != video_mode_cfg.aa_kind_ ||
			vid_cfg_get_aa_degree() != video_mode_cfg.aa_degree_) &&
		video_mode_cfg.aa_kind_ == bstone::Ren3dAaKind::ms &&
		::g_video->get_device_features().is_msaa_available_ &&
		::g_video->get_device_features().is_msaa_requires_restart_)
	{
		is_restart = true;
	}

	const auto is_window_modified =
		vid_cfg_get_width() != video_mode_cfg.width ||
		vid_cfg_get_height() != video_mode_cfg.height;

	const auto is_vsync_modified = (vid_cfg_is_vsync() != video_mode_cfg.is_vsync_);

	const auto is_aa_modified = (
		vid_cfg_get_aa_kind() != video_mode_cfg.aa_kind_ ||
		vid_cfg_get_aa_degree() != video_mode_cfg.aa_degree_);

	vid_cfg_set_renderer_kind(video_mode_cfg.renderer_kind_);
	vid_cfg_set_width(video_mode_cfg.width);
	vid_cfg_set_height(video_mode_cfg.height);
	vid_cfg_set_is_vsync(video_mode_cfg.is_vsync_);
	vid_cfg_set_aa_kind(video_mode_cfg.aa_kind_);
	vid_cfg_set_aa_degree(video_mode_cfg.aa_degree_);

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
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_anisotropy()
try
{
	::g_video->update_samplers();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_2d_image_filter()
try
{
	::g_video->update_samplers();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_3d_image_filter()
try
{
	::g_video->update_samplers();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_mipmap_filter()
try
{
	::g_video->update_samplers();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_upscale()
try
{
	::g_video->apply_texture_upscale();
}
catch (...)
{
	fail_nested(__func__);
}

int vid_clamp_filler_color_index(
	int filler_color_index) noexcept
try
{
	return bstone::math::clamp(filler_color_index, 0, 255);
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_filler_color()
try
{
	::g_video->apply_filler_color_index();
}
catch (...)
{
	fail_nested(__func__);
}

void vid_apply_external_textures()
try
{
	::g_video->apply_external_textures();
}
catch (...)
{
	fail_nested(__func__);
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
		lhs.width == rhs.width &&
		lhs.height == rhs.height &&
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

bool vid_is_native_mode() noexcept
{
#ifdef NDEBUG
	return
		vid_display_mode_.w == vid_width_cvar.get_int32() &&
		vid_display_mode_.h == vid_height_cvar.get_int32();
#else
	return false;
#endif // NDEBUG
}
// BBi
