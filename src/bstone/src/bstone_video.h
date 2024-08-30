/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Accelerated video (interface).
//


#ifndef BSTONE_VIDEO_INCLUDED
#define BSTONE_VIDEO_INCLUDED


#include <memory>

#include "id_vl.h"


namespace bstone
{


class Video
{
public:
	Video() noexcept;

	virtual ~Video();


	virtual bool is_hardware() const noexcept = 0;

	virtual StringView get_renderer_name() = 0;

	virtual void clear_vga_buffer() = 0;

	virtual void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) = 0;

	virtual void vsync_present() = 0;
	virtual void present() = 0;


	virtual void get_palette(
		int offset,
		int count,
		std::uint8_t* vga_palette) const = 0;

	virtual void fill_palette(
		int r,
		int g,
		int b) noexcept = 0;

	virtual void set_palette(
		int offset,
		int count,
		const std::uint8_t* vga_palette) = 0;

	virtual void fade_out(
		int start,
		int end,
		int red,
		int green,
		int blue,
		int steps) = 0;

	virtual void fade_in(
		int start,
		int end,
		const std::uint8_t* palette,
		int steps) = 0;

	virtual void apply_widescreen() = 0;

	virtual void apply_window_mode() = 0;

	virtual void apply_filler_color_index() = 0;


	// HW
	//

	virtual const bstone::Rgba8Palette& get_default_palette() const noexcept = 0;

	virtual void enable_fizzle_fx(
		bool is_enabled) = 0;

	virtual void enable_fizzle_fx_fading(
		bool is_fading) = 0;

	virtual void set_fizzle_fx_color_index(
		int color_index) = 0;

	virtual void set_fizzle_fx_ratio(
		float ratio) = 0;

	virtual void clear_wall_render_list() noexcept = 0;

	virtual void add_wall_render_item(
		int tile_x,
		int tile_y) = 0;

	virtual void clear_pushwall_render_list() noexcept = 0;

	virtual void add_pushwall_render_item(
		int tile_x,
		int tile_y) = 0;

	virtual void clear_door_render_list() noexcept = 0;

	virtual void add_door_render_item(
		int tile_x,
		int tile_y) = 0;

	virtual void clear_static_render_list() noexcept = 0;

	virtual void add_static_render_item(
		int bs_static_index) = 0;

	virtual void clear_actor_render_list() noexcept = 0;

	virtual void add_actor_render_item(
		int bs_actor_index) = 0;

	virtual void on_load_level() = 0;

	virtual void on_update_wall_switch(
		int x,
		int y) = 0;

	virtual void on_move_pushwall() = 0;

	virtual void on_step_pushwall(
		int old_x,
		int old_y) = 0;

	virtual void on_pushwall_to_wall(
		int old_x,
		int old_y,
		int new_x,
		int new_y) = 0;

	virtual void on_move_door(
		int door_index) = 0;

	virtual void on_update_door_lock(
		int bs_door_index) = 0;

	virtual void on_remove_static(
		const statobj_t& bs_static) = 0;

	virtual void on_remove_actor(
		const objtype& bs_actor) = 0;

	virtual void apply_vsync() = 0;

	virtual void apply_msaa() = 0;

	virtual void apply_texture_upscale() = 0;

	virtual void apply_external_textures() = 0;

	virtual void update_samplers() = 0;

	virtual const bstone::R3rDeviceFeatures& get_device_features() const noexcept = 0;

	//
	// HW
}; // Video

using VideoUPtr = std::unique_ptr<Video>;


} // bstone


#endif // !BSTONE_VIDEO_INCLUDED
