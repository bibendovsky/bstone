/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_ID_VL_INCLUDED
#define BSTONE_ID_VL_INCLUDED


#include <cstdint>

#include <array>
#include <vector>

#include "bstone_extent_2d.h"
#include "bstone_ref_values.h"
#include "bstone_renderer_type.h"
#include "bstone_r3r_types.h"
#include "bstone_rgb_palette.h"
#include "bstone_hw_texture_mgr.h"
#include "bstone_cvar_mgr.h"
#include "bstone_string_view.h"
#include "bstone_fizzle_fx.h"


namespace bstone
{


class TextWriter;


} // bstone


struct statobj_t;
struct objtype;


using VgaColor = std::array<std::uint8_t, 3>;
using VgaPalette = std::array<VgaColor, 256>;

using VgaBuffer = std::vector<std::uint8_t>;
using UiMaskBuffer = std::array<bool, vga_ref_width * vga_ref_height>;

struct VideoModeCfg
{
	bstone::RendererType renderer_type;
	int width;
	int height;
	bool is_vsync_;
	bstone::R3rAaType aa_type;
	int aa_degree_;
}; // VideoModeCfg

bool operator==(
	const VideoModeCfg& lhs,
	const VideoModeCfg& rhs) noexcept;

bool operator!=(
	const VideoModeCfg& lhs,
	const VideoModeCfg& rhs) noexcept;


extern std::uint8_t* vga_memory;

constexpr auto vid_min_aa = 1;
constexpr auto vid_max_aa = 32;

constexpr auto vid_min_anisotropic = 1;
constexpr auto vid_max_anisotropic = 16;

constexpr auto vid_upscale_min_degree = 2;
constexpr auto vid_upscale_max_degree = 6;


struct VidLayout
{
	int width;
	int height;

	int screen_width;
	int screen_height;

	int window_width;
	int window_height;

	int window_viewport_left_width;
	int window_viewport_right_width;
	int window_viewport_top_height;
	int window_viewport_bottom_height;

	int screen_left_filler_width;
	int screen_right_filler_width;
	int screen_top_filler_height;
	int screen_bottom_filler_height;
	int screen_width_4x3;

	int screen_viewport_left_width;
	int screen_viewport_right_width;
	int screen_viewport_width;

	int screen_viewport_top_height;
	int screen_viewport_bottom_height;
	int screen_viewport_height;
}; // VidLayout

extern VidLayout vid_layout_;

using ScreenshotBuffer = std::unique_ptr<std::uint8_t[]>;

using SdlPalette = std::array<std::uint32_t, bstone::RgbPalette::get_max_color_count()>;

extern VgaBuffer vid_ui_buffer_;
extern UiMaskBuffer vid_mask_buffer_;

extern bool vid_is_take_screenshot_scheduled;

struct CalculateScreenSizeInputParam
{
	bool is_widescreen;

	int width;
	int height;

	int window_width;
	int window_height;
}; // CalculateScreenSizeInputParam


// ===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (320) // default screen width in bytes

#define TILEWIDTH (4)

// ===========================================================================

extern int bufferofs; // all drawing is reletive to this

extern bool screenfaded;

// BBi
const int vga_ref_size = 256 * 1'024;
const int vga_plane_count = 4;
const int vga_plane_width = vga_ref_width / 4;
const int vga_plane_height = vga_ref_height + 8;
const int vga_plane_size = vga_plane_width * vga_plane_height;
const int vga_page_size = vga_plane_count * vga_plane_size;
const int vga_page_count = 3;

extern double vga_height_scale; // vga_width / 240
extern double vga_width_scale; // vga_width / 320
extern double vga_wide_scale; // vga_width / 320
extern int vga_width;
extern int vga_height;
extern int vga_area;
extern int vga_3d_view_top_y;
extern int vga_3d_view_bottom_y;

const bool default_vid_widescreen = true;

// Is VSync enabled?
extern bool vid_has_vsync;

// Are we drawing HUD and a level?
extern bool vid_is_hud;

// Are we drawing a level?
extern bool vid_is_3d;

// Is it intro/outro/you-win/etc?
extern bool vid_is_movie;

extern bstone::SpriteCache vid_sprite_cache;

extern double height_compensation_factor;
// BBi

// ===========================================================================

void VL_Startup();

void VL_Shutdown();

void VL_WaitVBL(
	std::uint32_t vbls);

void VL_FillPalette(
	std::uint8_t red,
	std::uint8_t green,
	std::uint8_t blue);

void VL_SetPalette(
	int first,
	int count,
	const std::uint8_t* palette);

void VL_GetPalette(
	int first,
	int count,
	std::uint8_t* palette);

void VL_SetPaletteIntensity(const std::uint8_t* palette, int intensity);

void VL_FadeOut(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps);

void VL_FadeIn(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps);

void VL_Plot(
	int x,
	int y,
	std::uint8_t color,
	bool is_transparent = false);

void VL_Hlin(
	int x,
	int y,
	int width,
	std::uint8_t color);

void VL_Vlin(
	int x,
	int y,
	int height,
	std::uint8_t color);

void VL_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	bool is_transparent = false);

void VL_MemToLatch(
	const std::uint8_t* source,
	int width,
	int height,
	int dest);

void VL_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height);

void VL_MemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y);

void VL_MaskMemToScreen(
	const std::uint8_t* source,
	int width,
	int height,
	int x,
	int y,
	std::uint8_t mask);

void VL_ScreenToMem(
	std::uint8_t* dest,
	int width,
	int height,
	int x,
	int y);

void vid_initialize_cvars(bstone::CVarMgr& cvar_mgr);


VideoModeCfg vid_cfg_get_video_mode() noexcept;

bool vid_cfg_parse_key_value(
	const std::string& key_string,
	const std::string& value_string);

void vid_cfg_write(
	bstone::TextWriter& text_writer);


using VidRendererTypes = std::vector<bstone::RendererType>;

const VidRendererTypes& vid_get_available_renderer_types();


struct VidWindowSize
{
	bool is_current_;
	bool is_custom_;

	int width;
	int height;
}; // VidWindowSize

using VidWindowSizes = std::vector<VidWindowSize>;

const VidWindowSizes& vid_get_window_size_list();


void VL_RefreshScreen();

int vl_get_offset(
	int base_offset,
	int x,
	int y);

std::uint8_t vl_get_pixel(
	int base_offset,
	int x,
	int y);

void vl_update_widescreen();


bstone::RendererType vid_cfg_get_renderer_type() noexcept;
void vid_cfg_set_renderer_type(bstone::RendererType renderer_type);

bool vid_cfg_is_positioned() noexcept;

bool vid_cfg_is_vsync() noexcept;
void vid_cfg_set_is_vsync(bool is_enabled);

bool vid_cfg_is_ui_stretched() noexcept;
void vid_cfg_set_is_ui_stretched(bool is_enabled);

bool vid_cfg_is_widescreen() noexcept;
void vid_cfg_set_is_widescreen(bool is_enabled);

int vid_cfg_get_x() noexcept;
int vid_cfg_get_y() noexcept;

int vid_cfg_get_width() noexcept;
void vid_cfg_set_width(int width);

int vid_cfg_get_height() noexcept;
void vid_cfg_set_height(int height);

bstone::R3rFilterType vid_cfg_get_2d_texture_filter() noexcept;
void vid_cfg_set_2d_texture_filter(bstone::R3rFilterType filter);

bstone::R3rFilterType vid_cfg_get_3d_texture_image_filter() noexcept;
void vid_cfg_set_3d_texture_image_filter(bstone::R3rFilterType filter);

bstone::R3rFilterType vid_cfg_get_3d_texture_mipmap_filter() noexcept;
void vid_cfg_set_3d_texture_mipmap_filter(bstone::R3rFilterType filter);

int vid_cfg_get_3d_texture_anisotropy() noexcept;
void vid_cfg_set_3d_texture_anisotropy(int anisotropy);

bstone::R3rAaType vid_cfg_get_aa_type() noexcept;
void vid_cfg_set_aa_type(bstone::R3rAaType aa_type);

int vid_cfg_get_aa_degree() noexcept;
void vid_cfg_set_aa_degree(int degree);

bstone::HwTextureMgrUpscaleFilterType vid_cfg_get_texture_upscale_type() noexcept;
void vid_cfg_set_texture_upscale_type(bstone::HwTextureMgrUpscaleFilterType filter);

int vid_cfg_get_texture_upscale_xbrz_degree() noexcept;
void vid_cfg_set_texture_upscale_xbrz_degree(int degree);

int vid_cfg_get_filler_color_index() noexcept;
void vid_cfg_set_filler_color_index(int index);

bool vid_cfg_is_external_textures_enabled() noexcept;
void vid_cfg_set_is_external_textures_enabled(bool is_enabled);

bool vid_check_r3_api_call_for_errors() noexcept;
void vid_check_r3_api_call_for_errors(bool is_enabled);

void vid_set_ui_mask(
	bool value);

void vid_set_ui_mask(
	int x,
	int y,
	int width,
	int height,
	bool value);

void vid_set_ui_mask_3d(
	bool value);

void vid_clear_3d();

void vid_export_ui(
	VgaBuffer& dst_buffer);

void vid_import_ui(
	const VgaBuffer& src_buffer,
	bool is_transparent = false);

void vid_export_ui_mask(
	UiMaskBuffer& dst_buffer);

void vid_import_ui_mask(
	const UiMaskBuffer& src_buffer);

void vid_draw_ui_sprite(
	int sprite_id,
	int center_x,
	int center_y,
	int new_side);

void vid_hw_on_load_level();

void vid_hw_on_update_wall_switch(
	int x,
	int y);

void vid_hw_on_move_pushwall();

void vid_hw_on_step_pushwall(
	int old_x,
	int old_y);

void vid_hw_on_pushwall_to_wall(
	int old_x,
	int old_y,
	int new_x,
	int new_y);

void vid_hw_on_move_door(
	int door_index);

void vid_hw_on_update_door_lock(
	int door_index);

void vid_hw_on_remove_static(
	const statobj_t& bs_static);

void vid_hw_on_remove_actor(
	const objtype& bs_actor);


void vid_hw_enable_fizzle_fx(
	bool is_enabled);

void vid_hw_enable_fizzle_fx_fading(
	bool is_fading);

void vid_hw_set_fizzle_fx_color_index(
	int color_index);

void vid_hw_set_fizzle_fx_ratio(
	float ratio);


void vid_hw_clear_wall_render_list();

void vid_hw_add_wall_render_item(
	int tile_x,
	int tile_y);


void vid_hw_clear_pushwall_render_list();

void vid_hw_add_pushwall_render_item(
	int tile_x,
	int tile_y);


void vid_hw_clear_door_render_list();

void vid_hw_add_door_render_item(
	int tile_x,
	int tile_y);


void vid_hw_clear_static_render_list();

void vid_hw_add_static_render_item(
	int bs_static_index);


void vid_hw_clear_actor_render_list();

void vid_hw_add_actor_render_item(
	int bs_actor_index);

const bstone::Rgba8Palette& vid_hw_get_default_palette();

void vid_apply_video_mode(
	const VideoModeCfg& video_mode_cfg);

void vid_apply_anisotropy();

void vid_apply_2d_image_filter();

void vid_apply_3d_image_filter();

void vid_apply_mipmap_filter();

void vid_apply_upscale();

int vid_clamp_filler_color_index(
	int filler_color_index) noexcept;

void vid_apply_filler_color();

void vid_apply_external_textures();

void vid_schedule_take_screenshot();

void vid_schedule_save_screenshot_task(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888,
	bool is_flipped_vertically);

void vid_take_screenshot();

void vid_initialize_vanilla_raycaster();

void vid_initialize_common();

void vid_initialize_ui_buffer();

std::string vid_get_window_title_for_renderer(bstone::StringView renderer_name);

std::string vid_get_game_name_and_game_version_string();

CalculateScreenSizeInputParam vid_create_screen_size_param() noexcept;

void vid_calculate_window_elements_dimensions(
	const CalculateScreenSizeInputParam& src_param,
	VidLayout& dst_param) noexcept;

void vid_calculate_vga_dimensions() noexcept;

std::string vid_to_string(bool value);
std::string vid_to_string(int value);
std::string vid_to_string(const bstone::R3rFilterType filter_type);
std::string vid_to_string(bstone::R3rType renderer_type);

bool vid_is_hw();

bool vid_is_native_mode() noexcept;


#endif // BSTONE_ID_VL_INCLUDED
