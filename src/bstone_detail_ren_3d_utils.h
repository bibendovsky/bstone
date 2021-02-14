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


//
// 3D renderer utils.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_UTILS_INCLUDED
#define BSTONE_DETAIL_REN_3D_UTILS_INCLUDED


#include <string>

#include "bstone_ren_3d.h"
#include "bstone_rgb_palette.h"
#include "bstone_sdl2_types.h"
#include "bstone_sprite.h"


namespace bstone
{
namespace detail
{


class Ren3dUtilsCreateWindowParam
{
public:
	Ren3dKind renderer_kind_;

	Ren3dCreateWindowParam window_;

	Ren3dAaKind aa_kind_;
	int aa_value_;

	bool is_default_depth_buffer_disabled_;
}; // Ren3dUtilsCreateWindowParam


class Ren3dUtils
{
public:
	static constexpr float pi = static_cast<float>(3.14159265358979323846);
	static constexpr float pi_over_180 = static_cast<float>(0.0174532925199432957692);

	static constexpr int absolute_max_texture_dimension = 1 << 16;

	static constexpr int absolute_max_viewport_dimension = 1 << 16;


	using Rgba8Buffer = std::vector<Rgba8>;
	using Rgba8BufferPtr = Rgba8Buffer*;


	struct IndexedToRgba8Param
	{
		int width_;
		int height_;
		int actual_width_;
		int actual_height_;
		bool indexed_is_column_major_;
		const std::uint8_t* indexed_pixels_;
		Rgba8PaletteCPtr indexed_palette_;
		const bool* indexed_alphas_;
		Rgba8BufferPtr rgba_8_buffer_;
	}; // IndexedToRgba8Param


	static float deg_to_rad(
		const float angle_deg);

	static bool is_pot_value(
		const int value);

	static int find_nearest_pot_value(
		const int value);

	static int calculate_mipmap_count(
		const int width,
		const int height);

	static SdlWindowUPtr create_window(
		const Ren3dUtilsCreateWindowParam& param);


	static void set_window_mode(
		SdlWindowPtr sdl_window,
		const Ren3dSetWindowModeParam& param);

	static void show_window(
		SdlWindowPtr sdl_window,
		const bool is_visible);

	static void set_window_title(
		const SdlWindowPtr sdl_window,
		const std::string& title_utf8);


	static void validate_initialize_param(
		const Ren3dCreateParam& param);


	static void validate_buffer_update_param(
		const Ren3dUpdateBufferParam& param);


	static void validate_buffer_create_param(
		const Ren3dCreateBufferParam& param);


	static void vertex_input_validate_format(
		const Ren3dVertexAttribFormat attribute_format);

	static void vertex_input_validate_param(
		const int max_locations,
		const Ren3dCreateVertexInputParam& param);


	// Converts indexed opaque (MxN) or indexed transparent (MxN) to RGBA (MxN).
	static void indexed_to_rgba_8(
		const IndexedToRgba8Param& param);

	// Converts indexed opaque or indexed transparent, non-power-of-two pixels to RGBA ones.
	static void indexed_npot_to_rgba_8_pot(
		const IndexedToRgba8Param& param);

	// Converts indexed pixels to RGBA ones.
	static void indexed_to_rgba_8_pot(
		const IndexedToRgba8Param& param);


	// Converts indexed sprite pixels to RGBA ones.
	static void indexed_sprite_to_rgba_8_pot(
		const Sprite& indexed_sprite,
		const Rgba8Palette& indexed_palette,
		Rgba8Buffer& texture_buffer);


	// Converts RGBA non-power-of-two pixels to RGBA power-of-two ones.
	static void rgba_8_npot_to_rgba_8_pot(
		const int width,
		const int height,
		const int actual_width,
		const int actual_height,
		const Rgba8* const rgba_8_pixels,
		Rgba8Buffer& texture_buffer);


	// Builds mipmap with premultiplied alpha.
	static void build_mipmap(
		const int previous_width,
		const int previous_height,
		const Rgba8CPtr src_colors,
		const Rgba8Ptr dst_colors);


private:
	Ren3dUtils();


	static void create_window_validate_param(
		const Ren3dUtilsCreateWindowParam& param);

	static void create_window_set_gl_profile_and_version(
		const Ren3dKind renderer_kind);

	static void create_window_set_gl_attributes(
		const Ren3dUtilsCreateWindowParam& param);

	static std::uint32_t create_window_sdl_flags(
		const Ren3dUtilsCreateWindowParam& param);


	// Indexed (row major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_rm_na(
		const IndexedToRgba8Param& param);

	// Indexed (row major, has alpha) -> RGBA
	static void indexed_to_rgba_8_rm_ha(
		const IndexedToRgba8Param& param);

	// Indexed (column major, has no alpha) -> RGBA
	static void indexed_to_rgba_8_cm_na(
		const IndexedToRgba8Param& param);

	// Indexed (column major, has alpha) -> RGBA
	static void indexed_to_rgba_8_cm_ha(
		const IndexedToRgba8Param& param);


	// Indexed (row major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_na(
		const IndexedToRgba8Param& param);

	// Indexed (row major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_rm_ha(
		const IndexedToRgba8Param& param);

	// Indexed (column major, has no alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_na(
		const IndexedToRgba8Param& param);

	// Indexed (column major, has alpha) -> RGBA POT
	static void indexed_npot_to_rgba_8_cm_ha(
		const IndexedToRgba8Param& param);


	static void build_mipmap_1(
		const int previous_dimension,
		const Rgba8CPtr src_colors,
		const Rgba8Ptr dst_colors);


	// Averages two colors and premultiplies alpha.
	static Rgba8 average_pa(
		const Rgba8 color_0,
		const Rgba8 color_1) noexcept;

	// Averages four colors and premultiplies alpha.
	static Rgba8 average_pa(
		const Rgba8 color_0,
		const Rgba8 color_1,
		const Rgba8 color_2,
		const Rgba8 color_3) noexcept;
}; // Ren3dUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_UTILS_INCLUDED
