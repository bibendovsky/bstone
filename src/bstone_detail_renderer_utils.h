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


//
// Renderer utils.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_RENDERER_UTILS_INCLUDED
#define BSTONE_DETAIL_RENDERER_UTILS_INCLUDED


#include <string>
#include "bstone_renderer.h"
#include "bstone_rgb_palette.h"
#include "bstone_sdl_types.h"
#include "bstone_sprite.h"


namespace bstone
{
namespace detail
{


class RendererUtilsCreateWindowParam
{
public:
	bool is_opengl_;

	RendererCreateWindowParam window_;

	RendererAaKind aa_kind_;
	int aa_value_;

	bool is_default_depth_buffer_disabled_;
}; // RendererUtilsCreateWindowParam


class RendererUtils
{
public:
	static constexpr float pi = static_cast<float>(3.14159265358979323846);
	static constexpr float pi_over_180 = static_cast<float>(0.0174532925199432957692);

	static constexpr int absolute_max_texture_dimension = 1 << 16;

	static constexpr int absolute_max_viewport_dimension = 1 << 16;


	using TextureBuffer = std::vector<R8g8b8a8>;


	static int get_max_mipmap_count();

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
		const RendererUtilsCreateWindowParam& param);

	static void show_window(
		SdlWindowPtr sdl_window,
		const bool is_visible);

	static void validate_initialize_param(
		const RendererCreateParam& param);


	static void validate_buffer_update_param(
		const RendererBufferUpdateParam& param);


	static void validate_index_buffer_create_param(
		const RendererIndexBufferCreateParam& param);

	static void validate_vertex_buffer_create_param(
		const RendererVertexBufferCreateParam& param);


	static void validate_texture_2d_create_param(
		const RendererTexture2dCreateParam& param);

	static void validate_texture_2d_update_param(
		const RendererTexture2dUpdateParam& param);


	static void vertex_input_validate_format(
		const RendererVertexAttributeFormat attribute_format);

	static void vertex_input_validate_param(
		const int max_locations,
		const RendererVertexInputCreateParam& param);


	// Converts indexed, opaque or transparent, power-of-two pixels to RGBA ones.
	static void indexed_pot_to_rgba_pot(
		const int width,
		const int height,
		const int actual_width,
		const int actual_height,
		const bool indexed_is_column_major,
		const std::uint8_t* const indexed_pixels,
		const R8g8b8a8Palette& indexed_palette,
		const bool* const indexed_alphas,
		TextureBuffer& texture_buffer);

	// Converts indexed, opaque or transparent, non-power-of-two pixels to RGBA ones.
	static void indexed_npot_to_rgba_pot(
		const int width,
		const int height,
		const int actual_width,
		const int actual_height,
		const bool indexed_is_column_major,
		const std::uint8_t* const indexed_pixels,
		const R8g8b8a8Palette& indexed_palette,
		const bool* const indexed_alphas,
		TextureBuffer& texture_buffer);

	// Converts indexed pixels to RGBA ones.
	static void indexed_to_rgba_pot(
		const int width,
		const int height,
		const int actual_width,
		const int actual_height,
		const bool indexed_is_column_major,
		const std::uint8_t* const indexed_pixels,
		const R8g8b8a8Palette& indexed_palette,
		const bool* const indexed_alphas,
		TextureBuffer& texture_buffer);


	// Converts indexed sprite pixels to RGBA ones.
	static void indexed_sprite_to_rgba_pot(
		const Sprite& indexed_sprite,
		const R8g8b8a8Palette& indexed_palette,
		TextureBuffer& texture_buffer);


	// Converts RGBA non-power-of-two pixels to RGBA power-of-two ones.
	static void rgba_npot_to_rgba_pot(
		const int width,
		const int height,
		const int actual_width,
		const int actual_height,
		const R8g8b8a8* const rgba_pixels,
		TextureBuffer& texture_buffer);


	static void build_mipmap(
		const int previous_width,
		const int previous_height,
		const R8g8b8a8CPtr src_colors,
		const R8g8b8a8Ptr dst_colors);


private:
	RendererUtils();


	static void create_window_validate_param(
		const RendererUtilsCreateWindowParam& param);

	static void create_window_set_ogl_attributes(
		const RendererUtilsCreateWindowParam& param);

	static std::uint32_t create_window_sdl_flags(
		const RendererUtilsCreateWindowParam& param);
}; // RendererUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_RENDERER_UTILS_INCLUDED
