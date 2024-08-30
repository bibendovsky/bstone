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
#include "bstone_sys_gl_context_attributes.h"
#include "bstone_sys_window_mgr.h"

namespace bstone {

class R3rUtilsCreateWindowParam
{
public:
	R3rType renderer_type;

	R3rAaType aa_type;
	int aa_value;

	bool is_default_depth_buffer_disabled;
};

struct R3rUtilsSetWindowModeParam
{
	bool is_native;
	bool is_positioned;

	sys::WindowPosition position;
	sys::WindowSize size;
};

class R3rUtils
{
public:
	static constexpr int absolute_max_texture_dimension = 1 << 16;

	static constexpr int absolute_max_viewport_dimension = 1 << 16;

	using Rgba8Buffer = std::vector<Rgba8>;
	using Rgba8BufferPtr = Rgba8Buffer*;

	struct IndexedToRgba8Param
	{
		int width;
		int height;
		int actual_width;
		int actual_height;
		bool indexed_is_column_major;
		const std::uint8_t* indexed_pixels;
		Rgba8PaletteCPtr indexed_palette;
		const bool* indexed_alphas;
		Rgba8BufferPtr rgba_8_buffer;
	};

public:
	static int find_nearest_pot_value(int value) noexcept;
	static bool is_pot_value(int value) noexcept;
	static int calculate_mipmap_count(int width, int height);

	static sys::WindowUPtr create_window(
		const R3rUtilsCreateWindowParam& param,
		sys::WindowMgr& window_mgr);

	static void set_window_mode(sys::Window& window, const R3rUtilsSetWindowModeParam& param);

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

	// Builds mipmap with premultiplied alpha.
	static void build_mipmap(
		int previous_width,
		int previous_height,
		Rgba8CPtr src_colors,
		Rgba8Ptr dst_colors);

private:
	static sys::GlContextAttributes create_window_make_gl_context_attributes(
		const R3rUtilsCreateWindowParam& param);

	// Indexed (row major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_rm_na(const IndexedToRgba8Param& param) noexcept;

	// Indexed (row major, has alpha) -> RGBA
	static void indexed_to_rgba_8_rm_ha(const IndexedToRgba8Param& param) noexcept;

	// Indexed (column major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_cm_na(const IndexedToRgba8Param& param) noexcept;

	// Indexed (column major, has alpha) -> RGBA
	static void indexed_to_rgba_8_cm_ha(const IndexedToRgba8Param& param) noexcept;

	// Indexed (row major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_na(const IndexedToRgba8Param& param) noexcept;

	// Indexed (row major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_ha(const IndexedToRgba8Param& param) noexcept;

	// Indexed (column major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_na(const IndexedToRgba8Param& param) noexcept;

	// Indexed (column major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_ha(const IndexedToRgba8Param& param) noexcept;

	static void build_mipmap_1(
		int previous_dimension,
		Rgba8CPtr src_colors,
		Rgba8Ptr dst_colors) noexcept;

	// Averages two colors and premultiplies alpha.
	static Rgba8 average_pa(Rgba8 color_0, Rgba8 color_1) noexcept;

	// Averages four colors and premultiplies alpha.
	static Rgba8 average_pa(Rgba8 color_0, Rgba8 color_1, Rgba8 color_2, Rgba8 color_3) noexcept;
};

} // namespace bstone

#endif // BSTONE_R3R_UTILS_INCLUDED
