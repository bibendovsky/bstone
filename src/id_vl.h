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


#ifndef BSTONE_ID_VL_INCLUDED
#define BSTONE_ID_VL_INCLUDED


#include <cstdint>
#include <array>
#include <vector>
#include "bstone_ref_values.h"


struct statobj_t;
struct objtype;


using VgaBuffer = std::vector<std::uint8_t>;
using UiMaskBuffer = std::array<bool, ::vga_ref_width * ::vga_ref_height>;


extern bool vid_is_hw_;

extern std::uint8_t* vga_memory;


// ===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (320) // default screen width in bytes

#define TILEWIDTH (4)

// ===========================================================================

extern int bufferofs; // all drawing is reletive to this

extern bool screenfaded;

// BBi
const int vga_ref_size = 256 * 1024;
const int vga_plane_count = 4;
const int vga_plane_width = vga_ref_width / 4;
const int vga_plane_height = vga_ref_height + 8;
const int vga_plane_size = vga_plane_width * vga_plane_height;
const int vga_page_size = vga_plane_count * vga_plane_size;
const int vga_page_count = 3;

extern int screen_x;
extern int screen_y;

extern int screen_width;
extern int screen_height;

extern int vga_scale;
extern double vga_height_scale; // vga_width / 240
extern double vga_width_scale; // vga_width / 320
extern double vga_wide_scale; // vga_width / 320
extern int vga_width;
extern int vga_height;
extern int vga_area;
extern int vga_3d_view_top_y;
extern int vga_3d_view_bottom_y;

const bool default_vid_widescreen = true;
extern bool vid_widescreen;
extern bool vid_is_ui_stretched;

// Is VSync enabled?
extern bool vid_has_vsync;

// Are we drawing HUD and a level?
extern bool vid_is_hud;

// Are we drawing a level?
extern bool vid_is_3d;

// Is fizzle fade in progress?
extern bool vid_is_fizzle_fade;

// Is it intro/outro/you-win/etc?
extern bool vid_is_movie;
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

void VL_SetPaletteIntensity(
	int start,
	int end,
	const std::uint8_t* palette,
	int intensity);

void VL_FadeOut(
	const int start,
	const int end,
	const int red,
	const int green,
	const int blue,
	const int steps);

void VL_FadeIn(
	const int start,
	const int end,
	const std::uint8_t* const palette,
	const int steps);

void VL_Plot(
	int x,
	int y,
	std::uint8_t color,
	const bool is_transparent = false);

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
	const bool is_transparent = false);

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

// BBi
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
	const int sprite_id,
	const int center_x,
	const int center_y,
	const int new_side);

void vid_hw_on_level_load();

void vid_hw_on_wall_switch_update(
	const int x,
	const int y);

void vid_hw_on_pushwall_move();

void vid_hw_on_pushwall_step(
	const int old_x,
	const int old_y);

void vid_hw_on_pushwall_to_wall(
	const int old_x,
	const int old_y,
	const int new_x,
	const int new_y);

void vid_hw_on_door_move(
	const int door_index);

void vid_hw_on_door_lock_update(
	const int door_index);

void vid_hw_on_static_add(
	const statobj_t& bs_static);

void vid_hw_on_static_change_texture(
	const statobj_t& bs_static);

void vid_hw_on_actor_add(
	const objtype& bs_actor);


void vid_hw_fizzle_fx_set_is_enabled(
	const bool is_enabled);

void vid_hw_fizzle_fx_set_is_fading(
	const bool is_fading);

void vid_hw_fizzle_fx_set_color_index(
	const int color_index);

void vid_hw_fizzle_fx_set_ratio(
	const float ratio);


void vid_hw_walls_clear_render_list();

void vid_hw_walls_add_render_item(
	const int tile_x,
	const int tile_y);


void vid_hw_pushwalls_clear_render_list();

void vid_hw_pushwalls_add_render_item(
	const int tile_x,
	const int tile_y);


void vid_hw_doors_clear_render_list();

void vid_hw_doors_add_render_item(
	const int tile_x,
	const int tile_y);


void vid_hw_statics_clear_render_list();

void vid_hw_statics_add_render_item(
	const int bs_static_index);


void vid_hw_actors_clear_render_list();

void vid_hw_actors_add_render_item(
	const int bs_actor_index);


#endif // BSTONE_ID_VL_INCLUDED
