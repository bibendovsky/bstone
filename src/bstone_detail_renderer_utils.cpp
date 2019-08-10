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


#include "bstone_precompiled.h"
#include "bstone_detail_renderer_utils.h"
#include <cassert>
#include <algorithm>
#include <unordered_set>
#include "SDL_video.h"
#include "bstone_sprite.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// RendererUtils
//

const std::string& RendererUtils::get_error_message() const
{
	return error_message_;
}

int RendererUtils::get_max_mipmap_count()
{
	return 31;
}

float RendererUtils::deg_to_rad(
	const float angle_deg)
{
	return angle_deg * pi_over_180;
}

bool RendererUtils::is_pot_value(
		const int value)
{
	const auto nearest_value = find_nearest_pot_value(value);

	return nearest_value == value;
}

int RendererUtils::find_nearest_pot_value(
	const int value)
{
	for (int i = 0; i < 32; ++i)
	{
		const auto new_value = 1 << i;

		if (new_value >= value)
		{
			return new_value;
		}
	}

	return 0;
}

int RendererUtils::calculate_mipmap_count(
	const int width,
	const int height)
{
	// mipmap_count = [log2(max(width, height))] + 1

	const auto max_size = std::max(width, height);

	int log_2;

	for (log_2 = 0; log_2 < 32; ++log_2)
	{
		if ((1 << log_2) >= max_size)
		{
			break;
		}
	}

	const auto result = log_2 + 1;

	return result;
}

SdlWindowUPtr RendererUtils::create_window(
	const RendererUtilsCreateWindowParam& param)
{
	const auto error_message_prefix = "Failed to create a window. ";

	if (!create_window_validate_param(param))
	{
		error_message_ = error_message_prefix + error_message_;

		return nullptr;
	}

	if (!create_window_set_ogl_attributes(param))
	{
		error_message_ = error_message_prefix + error_message_;

		return nullptr;
	}

	const auto sdl_flags = create_window_sdl_flags(param);

	const auto x = (
		param.window_.is_positioned_ ?
			param.window_.x_
			:
			SDL_WINDOWPOS_CENTERED
	);

	const auto y = (
		param.window_.is_positioned_ ?
			param.window_.y_
			:
			SDL_WINDOWPOS_CENTERED
	);

	auto sdl_window = SdlWindowUPtr{::SDL_CreateWindow(
		param.window_.title_utf8_.c_str(),
		x,
		y,
		param.window_.width_,
		param.window_.height_,
		sdl_flags
	)};

	if (!sdl_window)
	{
		error_message_ = error_message_prefix;
		error_message_ += ::SDL_GetError();
	}

	return sdl_window;
}

bool RendererUtils::show_window(
	SdlWindowPtr sdl_window,
	const bool is_visible)
{
	const auto error_message_prefix = "Failed to show a window. ";

	if (!sdl_window)
	{
		error_message_ = error_message_prefix;
		error_message_ += "Null SDL window.";

		return false;
	}

	const auto sdl_function = (is_visible ? ::SDL_ShowWindow : ::SDL_HideWindow);

	sdl_function(sdl_window);

	return true;
}

bool RendererUtils::validate_initialize_param(
	const RendererInitializeParam& param)
{
	switch (param.renderer_kind_)
	{
	case RendererKind::ogl_2_x:
		break;

	default:
		error_message_ = "Unsupported renderer kind.";

		return false;
	}

	if (param.window_.width_ <= 0)
	{
		error_message_ = "Invalid window width.";

		return false;
	}

	if (param.window_.height_ <= 0)
	{
		error_message_ = "Invalid window height.";

		return false;
	}

	switch (param.aa_kind_)
	{
		case RendererAaKind::none:
		case RendererAaKind::ms:
			break;

		default:
			error_message_ = "Invalid antialiasing kind.";

			return false;
	}

	return true;
}

bool RendererUtils::validate_index_buffer_create_param(
	const RendererIndexBufferCreateParam& param)
{
	switch (param.usage_kind_)
	{
		case RendererBufferUsageKind::stream_draw:
		case RendererBufferUsageKind::static_draw:
		case RendererBufferUsageKind::dynamic_draw:
			break;

		default:
			error_message_ = "Invalid usage kind.";
		return false;
	}

	switch (param.byte_depth_)
	{
	case 1:
	case 2:
	case 4:
		break;

	default:
		error_message_ = "Invalid byte depth.";
		return false;
	}

	if (param.size_ <= 0)
	{
		error_message_ = "Invalid size.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_index_buffer_update_param(
	const RendererIndexBufferUpdateParam& param)
{
	if (param.offset_ < 0)
	{
		error_message_ = "Invalid offset.";

		return false;
	}

	if (param.size_ <= 0)
	{
		error_message_ = "Invalid count.";

		return false;
	}

	if (!param.data_)
	{
		error_message_ = "Null indices.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_vertex_buffer_create_param(
	const RendererVertexBufferCreateParam& param)
{
	if (param.size_ <= 0)
	{
		error_message_ = "Invalid size.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_vertex_buffer_update_param(
	const RendererVertexBufferUpdateParam& param)
{
	if (param.offset_ < 0)
	{
		error_message_ = "Invalid offset.";

		return false;
	}

	if (param.size_ <= 0)
	{
		error_message_ = "Invalid size.";

		return false;
	}

	if (!param.data_)
	{
		error_message_ = "Null data.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_texture_2d_create_param(
	const RendererTexture2dCreateParam& param)
{
	switch (param.storage_pixel_format_)
	{
		case RendererPixelFormat::r8g8b8_unorm:
		case RendererPixelFormat::r8g8b8a8_unorm:
			break;

		default:
			error_message_ = "Invalid pixel format.";

			return false;
	}

	if (param.width_ <= 0)
	{
		error_message_ = "Invalid width.";

		return false;
	}

	if (param.height_ <= 0)
	{
		error_message_ = "Invalid height.";

		return false;
	}

	if (param.mipmap_count_ <= 0)
	{
	}

	return true;
}

bool RendererUtils::validate_texture_2d_update_param(
	const RendererTexture2dUpdateParam& param)
{
	if (param.mipmap_level_ < 0 ||
		param.mipmap_level_ >= get_max_mipmap_count())
	{
		error_message_ = "Invalid mipmap level.";

		return false;
	}

	if (param.rgba_pixels_ == nullptr)
	{
		error_message_ = "Null pixel source.";

		return false;
	}

	return true;
}

bool RendererUtils::vertex_input_validate_format(
	const RendererVertexAttributeFormat attribute_format)
{
	switch (attribute_format)
	{
		case RendererVertexAttributeFormat::r8g8b8a8_unorm:
		case RendererVertexAttributeFormat::r32g32_sfloat:
		case RendererVertexAttributeFormat::r32g32b32_sfloat:
			return true;

		default:
			return false;
	}
}

bool RendererUtils::vertex_input_validate_param(
	const int max_locations,
	const RendererVertexInputCreateParam& param)
{
	assert(max_locations >= 0);

	if (!param.index_buffer_)
	{
		error_message_ = "Null index buffer.";

		return false;
	}

	const auto& attribute_descriptions = param.attribute_descriptions_;

	if (attribute_descriptions.empty())
	{
		error_message_ = "No descriptions.";

		return false;
	}

	using UsedLocations = std::unordered_set<int>;
	auto used_locations = UsedLocations{};
	used_locations.reserve(max_locations);

	for (const auto& attribute_description : attribute_descriptions)
	{
		if (attribute_description.is_default_)
		{
			continue;
		}

		// Location.
		//
		const auto location = attribute_description.location_;

		if (location < 0 || location >= max_locations)
		{
			error_message_ = "Location out of range.";

			return false;
		}

		const auto has_location = (used_locations.find(location) != used_locations.cend());

		if (has_location)
		{
			error_message_ = "Duplicate location.";

			return false;
		}

		used_locations.insert(location);


		// Format.
		//
		if (!vertex_input_validate_format(attribute_description.format_))
		{
			error_message_ = "Invalid format.";

			return false;
		}


		// Vertex buffer.
		//
		if (!attribute_description.vertex_buffer_)
		{
			error_message_ = "Null vertex buffer.";

			return false;
		}


		// Offset.
		//
		if (attribute_description.offset_ < 0)
		{
			error_message_ = "Invalid offset.";

			return false;
		}


		// Stride.
		//
		if (attribute_description.stride_ < 0)
		{
			error_message_ = "Invalid stride.";

			return false;
		}
	}

	return true;
}

bool RendererUtils::is_ogl_renderer_path(
	const RendererKind renderer_path)
{
	switch (renderer_path)
	{
	case RendererKind::ogl_2_x:
		return true;

	default:
		return false;
	}
}

int RendererUtils::aa_get_min_value()
{
	return 1;
}

void RendererUtils::indexed_pot_to_rgba_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const bool indexed_is_column_major,
	const std::uint8_t* const indexed_pixels,
	const R8g8b8a8Palette& indexed_palette,
	const bool* const indexed_alphas,
	TextureBuffer& texture_buffer)
{
	assert(width > 0);
	assert(height > 0);
	assert(actual_width > 0);
	assert(actual_height > 0);
	assert(indexed_pixels);
	assert(texture_buffer.size() >= (actual_width * actual_height));

	const auto has_alphas = (indexed_alphas != nullptr);

	auto dst_index = 0;

	for (int src_y = 0; src_y < actual_height; ++src_y)
	{
		for (int src_x = 0; src_x < actual_width; ++src_x)
		{
			auto src_index = 0;

			if (indexed_is_column_major)
			{
				src_index = (src_x * actual_height) + src_y;
			}
			else
			{
				src_index = dst_index;
			}

			auto& dst_pixel = texture_buffer[dst_index];

			dst_pixel = indexed_palette[indexed_pixels[src_index]];

			if (has_alphas)
			{
				const auto is_transparent = !indexed_alphas[src_index];

				if (is_transparent)
				{
					dst_pixel.a = 0x00;
				}
			}

			++dst_index;
		}
	}
}

void RendererUtils::indexed_npot_to_rgba_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const bool indexed_is_column_major,
	const std::uint8_t* const indexed_pixels,
	const R8g8b8a8Palette& indexed_palette,
	const bool* const indexed_alphas,
	TextureBuffer& texture_buffer)
{
	assert(width > 0);
	assert(height > 0);
	assert(actual_width > 0);
	assert(actual_height > 0);
	assert(indexed_pixels);
	assert(texture_buffer.size() >= (actual_width * actual_height));

	const auto has_alphas = (indexed_alphas != nullptr);

	const auto src_du_f = static_cast<float>(width) / static_cast<float>(actual_width);
	const auto src_dv_f = static_cast<float>(height) / static_cast<float>(actual_height);

	auto dst_index = 0;

	auto src_v_f = 0.5F * src_dv_f;

	for (int h = 0; h < actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);

		auto src_u_f = 0.5F * src_du_f;

		for (int w = 0; w < actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);

			auto src_index = 0;

			if (indexed_is_column_major)
			{
				src_index = (src_u * height) + src_v;
			}
			else
			{
				src_index = (src_v * width) + src_u;
			}

			auto& dst_pixel = texture_buffer[dst_index];

			dst_pixel = indexed_palette[indexed_pixels[src_index]];

			if (has_alphas)
			{
				const auto is_transparent = !indexed_alphas[src_index];

				if (is_transparent)
				{
					dst_pixel.a = 0x00;
				}
			}

			++dst_index;

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

void RendererUtils::indexed_to_rgba_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const bool indexed_is_column_major,
	const std::uint8_t* const indexed_pixels,
	const R8g8b8a8Palette& indexed_palette,
	const bool* const indexed_alphas,
	TextureBuffer& texture_buffer)
{
	const auto is_npot = (width != actual_width || height != actual_height);

	if (!is_npot)
	{
		indexed_pot_to_rgba_pot(
			width,
			height,
			actual_width,
			actual_height,
			indexed_is_column_major,
			indexed_pixels,
			indexed_palette,
			indexed_alphas,
			texture_buffer
		);
	}
	else if (is_npot)
	{
		indexed_npot_to_rgba_pot(
			width,
			height,
			actual_width,
			actual_height,
			indexed_is_column_major,
			indexed_pixels,
			indexed_palette,
			indexed_alphas,
			texture_buffer
		);
	}
}

void RendererUtils::indexed_sprite_to_rgba_pot(
	const Sprite& indexed_sprite,
	const R8g8b8a8Palette& indexed_palette,
	TextureBuffer& texture_buffer)
{
	assert(indexed_sprite.is_initialized());

	const auto left = indexed_sprite.get_left();
	const auto right = indexed_sprite.get_right();
	const auto top = indexed_sprite.get_top();
	const auto bottom = indexed_sprite.get_bottom();

	for (int w = 0; w < Sprite::dimension; ++w)
	{
		const std::int16_t* column = nullptr;

		if (w >= left && w <= right)
		{
			column = indexed_sprite.get_column(w - left);
		}

		for (int h = 0; h < Sprite::dimension; ++h)
		{
			const auto dst_index = (h * Sprite::dimension) + w;

			auto& dst_pixel = texture_buffer[dst_index];

			if (column && h >= top && h <= bottom)
			{
				const auto src_index = h - top;

				const auto src_pixel = column[src_index];

				if (src_pixel < 0)
				{
					dst_pixel = R8g8b8a8{};
				}
				else
				{
					dst_pixel = indexed_palette[src_pixel];
				}
			}
			else
			{
				dst_pixel = R8g8b8a8{};
			}
		}
	}
}

void RendererUtils::rgba_npot_to_rgba_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const R8g8b8a8* const rgba_pixels,
	TextureBuffer& texture_buffer)
{
	assert(width > 0);
	assert(height > 0);
	assert(actual_width > 0);
	assert(actual_height > 0);
	assert(rgba_pixels);

	const auto src_du_d =
		static_cast<double>(width) /
		static_cast<double>(actual_width);

	const auto src_dv_d =
		static_cast<double>(height) /
		static_cast<double>(actual_height);

	auto src_v_d = 0.5 * src_dv_d;

	auto dst_index = 0;

	for (int h = 0; h < actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_d);

		auto src_u_d = 0.5 * src_du_d;

		for (int w = 0; w < actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_d);

			const auto src_index = (src_v * width) + src_u;

			texture_buffer[dst_index] = rgba_pixels[src_index];

			++dst_index;

			src_u_d += src_du_d;
		}

		src_v_d += src_dv_d;
	}
}

void RendererUtils::build_mipmap(
	const int previous_width,
	const int previous_height,
	const R8g8b8a8CPtr src_colors,
	const R8g8b8a8Ptr dst_colors)
{
	assert(previous_width >= 1);
	assert(previous_height >= 1);
	assert(!(previous_width == 1 && previous_height == 1));
	assert(src_colors);
	assert(dst_colors);
	assert(src_colors != dst_colors);

	const auto is_width_one = (previous_width == 1);
	const auto is_height_one = (previous_height == 1);

	const auto width = (is_width_one ? 1 : previous_width / 2);
	const auto height = (is_height_one ? 1 : previous_height / 2);

	const auto src_du = (is_width_one ? 1 : 2);
	const auto src_half_du = src_du / 2;

	const auto src_dv = (is_height_one ? 1 : 2);
	const auto src_half_dv = src_dv / 2;

	auto dst_v = 0;

	auto dst_index = 0;

	for (int h = 0; h < height; ++h)
	{
		const auto src_v1 = dst_v + (0 * src_half_dv);
		const auto src_v2 = dst_v + (1 * src_half_dv);

		auto dst_u = 0;

		for (int w = 0; w < width; ++w)
		{
			const auto src_u1 = dst_u + (0 * src_half_du);
			const auto src_u2 = dst_u + (1 * src_half_du);

			const auto& src_color_1 = src_colors[(src_v1 * previous_width) + src_u1];
			const auto& src_color_2 = src_colors[(src_v1 * previous_width) + src_u2];
			const auto& src_color_3 = src_colors[(src_v2 * previous_width) + src_u1];
			const auto& src_color_4 = src_colors[(src_v2 * previous_width) + src_u2];

			const auto red = (src_color_1.r + src_color_2.r + src_color_3.r + src_color_4.r) / 4;
			const auto green = (src_color_1.g + src_color_2.g + src_color_3.g + src_color_4.g) / 4;
			const auto blue = (src_color_1.b + src_color_2.b + src_color_3.b + src_color_4.b) / 4;
			const auto alpha = (src_color_1.a + src_color_2.a + src_color_3.a + src_color_4.a) / 4;;

			auto& dst_color = dst_colors[dst_index];

			dst_color.r = static_cast<glm::u8>(red);
			dst_color.g = static_cast<glm::u8>(green);
			dst_color.b = static_cast<glm::u8>(blue);
			dst_color.a = static_cast<glm::u8>(alpha);

			dst_u += src_du;

			++dst_index;
		}

		dst_v += src_dv;
	}
}

bool RendererUtils::create_window_validate_param(
	const RendererUtilsCreateWindowParam& param)
{
	if (param.window_.is_positioned_ && (param.window_.x_ < 0 || param.window_.y_ < 0))
	{
		error_message_ = "Invalid position.";

		return false;
	}

	if (param.window_.width_ <= 0 || param.window_.height_ <= 0)
	{
		error_message_ = "Invalid dimensions.";

		return false;
	}

	return true;
}

bool RendererUtils::create_window_set_ogl_attributes(
	const RendererUtilsCreateWindowParam& param)
{
	::SDL_GL_ResetAttributes();

	auto sdl_result = 0;

	switch (param.aa_kind_)
	{
		case RendererAaKind::ms:
			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);

			if (sdl_result != 0)
			{
				error_message_ = "Failed to set multisample buffer count.";

				return false;
			}

			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, param.aa_value_);

			if (sdl_result != 0)
			{
				error_message_ = "Failed to set multisample sample count.";

				return false;
			}

			break;


		case RendererAaKind::none:
		default:
			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

			if (sdl_result != 0)
			{
				error_message_ = "Failed to set multisample buffer count.";

				return false;
			}

			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

			if (sdl_result != 0)
			{
				error_message_ = "Failed to set multisample sample count.";

				return false;
			}

			break;
	}

	if (param.is_default_depth_buffer_disabled_)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

		if (sdl_result != 0)
		{
			error_message_ = "Failed to set depth buffer bit depth.";

			return false;
		}
	}

	return true;
}

std::uint32_t RendererUtils::create_window_sdl_flags(
	const RendererUtilsCreateWindowParam& param)
{
	auto flags = Uint32{};

	if (param.window_.is_visible_)
	{
		flags |= SDL_WINDOW_SHOWN;
	}
	else
	{
		flags |= SDL_WINDOW_HIDDEN;
	}

	if (param.window_.is_fullscreen_desktop_)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (param.window_.is_borderless_)
	{
		flags |= SDL_WINDOW_BORDERLESS;
	}

	if (param.is_opengl_)
	{
		flags |= SDL_WINDOW_OPENGL;
	}

	return flags;
}

//
// RendererUtils
// ==========================================================================


} // detail
} // bstone
