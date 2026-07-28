/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// 3D Renderer: Utils

#ifndef BSTONE_R3R_UTILS_INCLUDED
#define BSTONE_R3R_UTILS_INCLUDED

#include <string>

#include "bstone_r3r.h"
#include "bstone_rgb_palette.h"
#include "bstone_sprite.h"
#include "bstone_sys_display_mode.h"
#include "bstone_sys_gl_context_attributes.h"
#include "bstone_sys_window_mgr.h"

namespace bstone {

class R3rUtilsCreateWindowParam
{
public:
	R3rType renderer_type;

	R3rAaType aa_type;
	int aa_value;
};

struct R3rUtilsSetWindowModeParam
{
	bool is_positioned;
	sys::WindowPosition position;
	sys::WindowFullscreenType fullscreen_mode;
	sys::DisplayMode display_mode;
};

class R3rUtils
{
public:
	static constexpr int absolute_max_texture_dimension = 1 << 16;

	static constexpr int absolute_max_viewport_dimension = 1 << 16;

	using Rgba8Buffer = std::vector<Rgba8>;

	struct IndexedToRgba8Param
	{
		int width;
		int height;
		int actual_width;
		int actual_height;
		bool indexed_is_column_major;
		const std::uint8_t* indexed_pixels;
		const Rgba8Palette* indexed_palette;
		const bool* indexed_alphas;
		Rgba8Buffer* rgba_8_buffer;
	};

public:
	static int find_nearest_pot_value(int value);
	static bool is_pot_value(int value);
	static int calculate_mip_level_count(int width, int height);

	static sys::WindowFullscreenType get_fullscreen_mode_from_cvar();

	static sys::WindowUPtr create_window(
		const R3rUtilsCreateWindowParam& param,
		sys::WindowMgr& window_mgr);

	// Applies the mode and returns the window's resulting size in pixels.
	// SDL3 applies mode changes asynchronously (and defers them entirely for
	// hidden windows), so the returned size is derived from the requested or
	// chosen mode rather than a live query wherever possible.
	static sys::WindowSize set_window_mode(sys::Window& window, const R3rUtilsSetWindowModeParam& param);

	static void validate_initialize_param(const R3rInitParam& param);

	static void validate_buffer_update_param(const R3rUpdateBufferParam& param);
	static void validate_buffer_create_param(const R3rBufferInitParam& param);

	static void vertex_input_validate_format(R3rVertexAttribFormat attribute_format);

	static void vertex_input_validate_param(
		int max_locations,
		const R3rCreateVertexInputParam& param);

	// Converts indexed opaque (MxN) or indexed transparent (MxN) to RGBA (MxN).
	static void indexed_to_rgba_8(const IndexedToRgba8Param& param);

	// Converts indexed opaque or indexed transparent, non-power-of-two pixels to RGBA ones.
	static void indexed_npot_to_rgba_8_pot(const IndexedToRgba8Param& param);

	// Converts indexed pixels to RGBA ones.
	static void indexed_to_rgba_8_pot(const IndexedToRgba8Param& param);

	// Converts indexed sprite pixels to RGBA ones.
	static void indexed_sprite_to_rgba_8_pot(
		const Sprite& indexed_sprite,
		const Rgba8Palette& indexed_palette,
		Rgba8Buffer& texture_buffer);

	// Converts RGBA non-power-of-two pixels to RGBA power-of-two ones.
	static void rgba_8_npot_to_rgba_8_pot(
		int width,
		int height,
		int actual_width,
		int actual_height,
		const Rgba8* rgba_8_pixels,
		Rgba8Buffer& texture_buffer);

	// Builds mip with premultiplied alpha.
	static void build_mip(
		int previous_width,
		int previous_height,
		const Rgba8* src_colors,
		Rgba8* dst_colors);

private:
	static sys::GlContextAttributes create_window_make_gl_context_attributes(
		const R3rUtilsCreateWindowParam& param);

	// Indexed (row major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_rm_na(const IndexedToRgba8Param& param);

	// Indexed (row major, has alpha) -> RGBA
	static void indexed_to_rgba_8_rm_ha(const IndexedToRgba8Param& param);

	// Indexed (column major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_cm_na(const IndexedToRgba8Param& param);

	// Indexed (column major, has alpha) -> RGBA
	static void indexed_to_rgba_8_cm_ha(const IndexedToRgba8Param& param);

	// Indexed (row major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_na(const IndexedToRgba8Param& param);

	// Indexed (row major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_ha(const IndexedToRgba8Param& param);

	// Indexed (column major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_na(const IndexedToRgba8Param& param);

	// Indexed (column major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_ha(const IndexedToRgba8Param& param);

	static void build_mip_1(
		int previous_dimension,
		const Rgba8* src_colors,
		Rgba8* dst_colors);

	// Averages two colors and premultiplies alpha.
	static Rgba8 average_pa(Rgba8 color_0, Rgba8 color_1);

	// Averages four colors and premultiplies alpha.
	static Rgba8 average_pa(Rgba8 color_0, Rgba8 color_1, Rgba8 color_2, Rgba8 color_3);
};

} // namespace bstone

#endif // BSTONE_R3R_UTILS_INCLUDED
