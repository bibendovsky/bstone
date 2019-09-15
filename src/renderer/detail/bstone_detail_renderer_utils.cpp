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
#include <algorithm>
#include <unordered_set>
#include "bstone_exception.h"
#include "bstone_sprite.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// RendererUtils
//

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
	//
	// mipmap_count = [log2(max(width, height))] + 1
	//

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

SdlWindowUPtr RendererUtils::window_create(
	const RendererUtilsCreateWindowParam& param)
{
	const auto error_message_prefix = "Failed to create a window. ";

	create_window_validate_param(param);
	create_window_set_ogl_attributes(param);

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
		auto error_message = std::string{error_message_prefix};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}

	return sdl_window;
}

void RendererUtils::window_show(
	SdlWindowPtr sdl_window,
	const bool is_visible)
{
	const auto error_message_prefix = "Failed to show a window. ";

	if (!sdl_window)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += "Null SDL window.";

		throw Exception{std::move(error_message)};
	}

	const auto sdl_function = (is_visible ? ::SDL_ShowWindow : ::SDL_HideWindow);

	sdl_function(sdl_window);
}

void RendererUtils::window_set_title(
	const SdlWindowPtr sdl_window,
	const std::string& title_utf8)
{
	if (!sdl_window)
	{
		throw Exception{"No window."};
	}

	::SDL_SetWindowTitle(sdl_window, title_utf8.c_str());
}

void RendererUtils::validate_initialize_param(
	const RendererCreateParam& param)
{
	switch (param.renderer_kind_)
	{
	case RendererKind::auto_detect:
	case RendererKind::ogl_2:
		break;

	default:
		throw Exception{"Unsupported renderer kind."};
	}

	if (param.window_.width_ <= 0)
	{
		throw Exception{"Invalid window width."};
	}

	if (param.window_.height_ <= 0)
	{
		throw Exception{"Invalid window height."};
	}

	switch (param.aa_kind_)
	{
		case RendererAaKind::none:
		case RendererAaKind::ms:
			break;

		default:
			throw Exception{"Invalid antialiasing kind."};
	}
}

void RendererUtils::validate_index_buffer_create_param(
	const RendererIndexBufferCreateParam& param)
{
	switch (param.usage_kind_)
	{
		case RendererBufferUsageKind::stream_draw:
		case RendererBufferUsageKind::static_draw:
		case RendererBufferUsageKind::dynamic_draw:
			break;

		default:
			throw Exception{"Invalid usage kind."};
	}

	switch (param.byte_depth_)
	{
	case 1:
	case 2:
	case 4:
		break;

	default:
		throw Exception{"Invalid byte depth."};
	}

	if (param.size_ <= 0)
	{
		throw Exception{"Invalid size."};
	}
}

void RendererUtils::validate_buffer_update_param(
	const RendererBufferUpdateParam& param)
{
	if (param.offset_ < 0)
	{
		throw Exception{"Invalid offset."};
	}

	if (param.size_ <= 0)
	{
		throw Exception{"Invalid count."};
	}

	if (!param.data_)
	{
		throw Exception{"Null indices."};
	}
}

void RendererUtils::validate_vertex_buffer_create_param(
	const RendererVertexBufferCreateParam& param)
{
	if (param.size_ <= 0)
	{
		throw Exception{"Invalid size."};
	}
}

void RendererUtils::validate_texture_2d_create_param(
	const RendererTexture2dCreateParam& param)
{
	switch (param.storage_pixel_format_)
	{
		case RendererPixelFormat::r8g8b8_unorm:
		case RendererPixelFormat::r8g8b8a8_unorm:
			break;

		default:
			throw Exception{"Invalid pixel format."};
	}

	if (param.width_ <= 0)
	{
		throw Exception{"Invalid width."};
	}

	if (param.height_ <= 0)
	{
		throw Exception{"Invalid height."};
	}

	if (param.mipmap_count_ <= 0)
	{
		throw Exception{"Invalid mipmap count."};
	}
}

void RendererUtils::validate_texture_2d_update_param(
	const RendererTexture2dUpdateParam& param)
{
	if (param.mipmap_level_ < 0 ||
		param.mipmap_level_ >= get_max_mipmap_count())
	{
		throw Exception{"Invalid mipmap level."};
	}

	if (param.rgba_pixels_ == nullptr)
	{
		throw Exception{"Null pixel source."};
	}
}

void RendererUtils::vertex_input_validate_format(
	const RendererVertexAttributeFormat attribute_format)
{
	switch (attribute_format)
	{
		case RendererVertexAttributeFormat::r8g8b8a8_unorm:
		case RendererVertexAttributeFormat::r32g32_sfloat:
		case RendererVertexAttributeFormat::r32g32b32_sfloat:
			return;

		default:
			throw Exception{"Unsupported vertex input attribute format."};
	}
}

void RendererUtils::vertex_input_validate_param(
	const int max_locations,
	const RendererVertexInputCreateParam& param)
{
	if (max_locations < 0)
	{
		throw Exception{"Maximum location count out of range."};
	}

	if (!param.index_buffer_)
	{
		throw Exception{"Null index buffer."};
	}

	const auto& attribute_descriptions = param.attribute_descriptions_;

	if (attribute_descriptions.empty())
	{
		throw Exception{"No descriptions."};
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
			throw Exception{"Location out of range."};
		}

		const auto has_location = (used_locations.find(location) != used_locations.cend());

		if (has_location)
		{
			throw Exception{"Duplicate location."};
		}

		used_locations.insert(location);


		// Format.
		//
		vertex_input_validate_format(attribute_description.format_);


		// Vertex buffer.
		//
		if (!attribute_description.vertex_buffer_)
		{
			throw Exception{"Null vertex buffer."};
		}


		// Offset.
		//
		if (attribute_description.offset_ < 0)
		{
			throw Exception{"Invalid offset."};
		}


		// Stride.
		//
		if (attribute_description.stride_ < 0)
		{
			throw Exception{"Invalid stride."};
		}
	}
}

void RendererUtils::indexed_to_rgba(
	const IndexedToRgbaParam& param)
{
	if (param.width_ <= 0)
	{
		throw Exception{"Width out of range."};
	}

	if (param.height_ <= 0)
	{
		throw Exception{"Height out of range."};
	}

	if (!param.indexed_pixels_)
	{
		throw Exception{"Null indexed pixels."};
	}

	if (param.rgba_buffer_->size() < (param.width_ * param.height_))
	{
		throw Exception{"Bitmap buffer too small."};
	}

	const auto is_column_major = param.indexed_is_column_major_;
	const auto has_alphas = (param.indexed_alphas_ != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_to_rgba_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_to_rgba_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_to_rgba_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_to_rgba_cm_ha(param);
	}
	else
	{
		throw Exception{"Unsupported values combination."};
	}
}

void RendererUtils::indexed_npot_to_rgba_pot(
	const IndexedToRgbaParam& param)
{
	if (param.width_ <= 0)
	{
		throw Exception{"Width out of range."};
	}

	if (param.height_ <= 0)
	{
		throw Exception{"Height out of range."};
	}

	if (param.actual_width_ <= 0)
	{
		throw Exception{"Actual width out of range."};
	}

	if (param.actual_height_ <= 0)
	{
		throw Exception{"Actual height out of range."};
	}

	if (!param.indexed_pixels_)
	{
		throw Exception{"Null indexed pixels."};
	}

	if (param.rgba_buffer_->size() < (param.actual_width_ * param.actual_height_))
	{
		throw Exception{"Indexed bitmap buffer too small."};
	}

#if 0
	const auto has_alphas = (param.indexed_alphas_ != nullptr);

	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	auto dst_index = 0;

	auto src_v_f = 0.0;

	for (int h = 0; h < param.actual_height_; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);

		auto src_u_f = 0.0;

		for (int w = 0; w < param.actual_width_; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);

			auto src_index = 0;

			if (param.indexed_is_column_major_)
			{
				src_index = (src_u * param.height_) + src_v;
			}
			else
			{
				src_index = (src_v * param.width_) + src_u;
			}

			auto dst_color = (*param.indexed_palette_)[param.indexed_pixels_[src_index]];

			auto& dst_pixel = (*param.rgba_buffer_)[dst_index];

			dst_pixel = dst_color;

			if (has_alphas)
			{
				const auto is_transparent = !param.indexed_alphas_[src_index];

				if (is_transparent)
				{
					dst_pixel.reset();
				}
			}

			++dst_index;

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
#else
	const auto is_column_major = param.indexed_is_column_major_;
	const auto has_alphas = (param.indexed_alphas_ != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_cm_ha(param);
	}
	else
	{
		throw Exception{"Unsupported values combination."};
	}
#endif
}

void RendererUtils::indexed_to_rgba_pot(
	const IndexedToRgbaParam& param)
{
	const auto is_npot = (
		param.width_ != param.actual_width_ ||
		param.height_ != param.actual_height_
	);

	if (!is_npot)
	{
		indexed_to_rgba(param);
	}
	else if (is_npot)
	{
		indexed_npot_to_rgba_pot(param);
	}
}

void RendererUtils::indexed_sprite_to_rgba_pot(
	const Sprite& indexed_sprite,
	const R8g8b8a8Palette& indexed_palette,
	RgbaBuffer& texture_buffer)
{
	if (!indexed_sprite.is_initialized())
	{
		throw Exception{"Indexed sprite not initialized."};
	}

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

		auto dst_pixels = &texture_buffer[w];

		for (int h = 0; h < Sprite::dimension; ++h)
		{
			auto& dst_pixel = *dst_pixels;

			if (column && h >= top && h <= bottom)
			{
				const auto src_index = h - top;

				const auto src_pixel = column[src_index];

				if (src_pixel < 0)
				{
					dst_pixel.reset();
				}
				else
				{
					dst_pixel = indexed_palette[src_pixel];
				}
			}
			else
			{
				dst_pixel.reset();
			}

			dst_pixels += Sprite::dimension;
		}
	}
}

void RendererUtils::rgba_npot_to_rgba_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const R8g8b8a8* const rgba_pixels,
	RgbaBuffer& texture_buffer)
{
	if (width <= 0)
	{
		throw Exception{"Width out of range."};
	}

	if (height <= 0)
	{
		throw Exception{"Height out of range."};
	}

	if (actual_width <= 0)
	{
		throw Exception{"Actual width out of range."};
	}

	if (actual_height <= 0)
	{
		throw Exception{"Actual height out of range."};
	}

	if (!rgba_pixels)
	{
		throw Exception{"Null RGBA pixels."};
	}

	const auto src_du_f = static_cast<double>(width) / static_cast<double>(actual_width);
	const auto src_dv_f = static_cast<double>(height) / static_cast<double>(actual_height);

	auto src_v_d = 0.0;

	auto dst_index = 0;

	for (int h = 0; h < actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_d);

		auto src_u_d = 0.0;

		for (int w = 0; w < actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_d);

			const auto src_index = (src_v * width) + src_u;

			texture_buffer[dst_index] = rgba_pixels[src_index];

			++dst_index;

			src_u_d += src_du_f;
		}

		src_v_d += src_dv_f;
	}
}

void RendererUtils::build_mipmap(
	const int previous_width,
	const int previous_height,
	const R8g8b8a8CPtr src_colors,
	const R8g8b8a8Ptr dst_colors)
{
	if (previous_width <= 0)
	{
		throw Exception{"Previous width out of range."};
	}

	if (previous_height <= 0)
	{
		throw Exception{"Previous height out of range."};
	}

	if (previous_width == 1 && previous_height == 1)
	{
		throw Exception{"No more mipmap to build."};
	}

	if (!src_colors)
	{
		throw Exception{"Null source colors."};
	}

	if (!dst_colors)
	{
		throw Exception{"Null target colors."};
	}

	if (previous_width == 1)
	{
		build_mipmap_1(previous_height, src_colors, dst_colors);

		return;
	}

	if (previous_height == 1)
	{
		build_mipmap_1(previous_width, src_colors, dst_colors);

		return;
	}

	const auto width = previous_width / 2;
	const auto height = previous_height / 2;

	auto dst_colors_0 = dst_colors;

	auto src_colors_0 = src_colors;
	auto src_colors_1 = src_colors_0 + previous_width;

	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			*dst_colors_0++ = R8g8b8a8::average_pa(
				src_colors_0[0],
				src_colors_0[1],
				src_colors_1[0],
				src_colors_1[1]
			);

			src_colors_0 += 2;
			src_colors_1 += 2;
		}

		src_colors_0 += previous_width;
		src_colors_1 += previous_width;
	}
}

void RendererUtils::create_window_validate_param(
	const RendererUtilsCreateWindowParam& param)
{
	if (param.window_.is_positioned_ && (param.window_.x_ < 0 || param.window_.y_ < 0))
	{
		throw Exception{"Invalid position."};
	}

	if (param.window_.width_ <= 0 || param.window_.height_ <= 0)
	{
		throw Exception{"Invalid dimensions."};
	}
}

void RendererUtils::create_window_set_ogl_attributes_compatibility()
{
	const auto sdl_result = ::SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
	);

	if (sdl_result != 0)
	{
		throw Exception{"Failed to set OpenGL context compatibility profile attribute."};
	}
}

void RendererUtils::create_window_set_ogl_attributes_core(
	const int major,
	const int minor)
{
	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context core profile attribute."};
		}
	}

	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MAJOR_VERSION,
			major
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context major version attribute."};
		}
	}

	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MINOR_VERSION,
			minor
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context minor version attribute."};
		}
	}
}

void RendererUtils::create_window_set_ogl_attributes_es(
	const int major,
	const int minor)
{
	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_ES
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context ES profile attribute."};
		}
	}

	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MAJOR_VERSION,
			major
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context major version attribute."};
		}
	}

	{
		const auto sdl_result = ::SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MINOR_VERSION,
			minor
		);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set OpenGL context minor version attribute."};
		}
	}
}

void RendererUtils::create_window_set_ogl_attributes(
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
				throw Exception{"Failed to set multisample buffer count."};
			}

			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, param.aa_value_);

			if (sdl_result != 0)
			{
				throw Exception{"Failed to set multisample sample count."};
			}

			break;


		case RendererAaKind::none:
		default:
			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

			if (sdl_result != 0)
			{
				throw Exception{"Failed to set multisample buffer count."};
			}

			sdl_result = ::SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

			if (sdl_result != 0)
			{
				throw Exception{"Failed to set multisample sample count."};
			}

			break;
	}

	if (param.is_default_depth_buffer_disabled_)
	{
		sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

		if (sdl_result != 0)
		{
			throw Exception{"Failed to set depth buffer bit depth."};
		}
	}

	switch (param.renderer_kind_)
	{
		case RendererKind::ogl_2:
			create_window_set_ogl_attributes_compatibility();
			break;

		case RendererKind::ogl_3_2_core:
			create_window_set_ogl_attributes_core(3, 2);
			break;

		case RendererKind::ogl_es_2_0:
			create_window_set_ogl_attributes_es(2, 0);
			break;

		default:
			break;
	}
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

	switch (param.renderer_kind_)
	{
		case RendererKind::ogl_2:
		case RendererKind::ogl_3_2_core:
		case RendererKind::ogl_es_2_0:
			flags |= SDL_WINDOW_OPENGL;
			break;

		default:
			break;
	}

	return flags;
}

// Indexed (row major, has no alpha) -> RGBA
void RendererUtils::indexed_to_rgba_rm_na(
	const IndexedToRgbaParam& param)
{
	auto src_pixels = param.indexed_pixels_;
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_buffer_->data();

	const auto area = param.width_ * param.height_;

	for (int i = 0; i < area; ++i)
	{
		*dst_pixels++ = src_palette[*src_pixels++];
	}
}

// Indexed (row major, has alpha) -> RGBA
void RendererUtils::indexed_to_rgba_rm_ha(
	const IndexedToRgbaParam& param)
{
	auto src_pixels = param.indexed_pixels_;
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_buffer_->data();

	const auto area = param.width_ * param.height_;

	for (int i = 0; i < area; ++i)
	{
		const auto src_pixel = *src_pixels++;
		auto& dst_pixel = *dst_pixels++;

		if (param.indexed_alphas_[i])
		{
			dst_pixel = src_palette[src_pixel];
		}
		else
		{
			dst_pixel.reset();
		}
	}
}

// Indexed (column major, has no alpha) -> RGBA
void RendererUtils::indexed_to_rgba_cm_na(
	const IndexedToRgbaParam& param)
{
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_buffer_->data();

	for (int src_y = 0; src_y < param.height_; ++src_y)
	{
		auto src_index = src_y;

		for (int src_x = 0; src_x < param.width_; ++src_x)
		{
			*dst_pixels++ = src_palette[param.indexed_pixels_[src_index]];

			src_index += param.height_;
		}
	}
}

// Indexed (column major, has alpha) -> RGBA
void RendererUtils::indexed_to_rgba_cm_ha(
	const IndexedToRgbaParam& param)
{
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_buffer_->data();

	for (int src_y = 0; src_y < param.height_; ++src_y)
	{
		auto src_index = src_y;

		for (int src_x = 0; src_x < param.width_; ++src_x)
		{
			auto& dst_pixel = *dst_pixels++;

			if (param.indexed_alphas_[src_index])
			{
				dst_pixel = src_palette[param.indexed_pixels_[src_index]];
			}
			else
			{
				dst_pixel.reset();
			}

			src_index += param.height_;
		}
	}
}

// Indexed (row major, has no alpha) -> RGBA POT
void RendererUtils::indexed_npot_to_rgba_rm_na(
	const IndexedToRgbaParam& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = R8g8b8a8{};

	for (int h = 0; h < param.actual_height_; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		const auto src_base_index = src_v * param.width_;

		auto src_u_f = 0.0;

		for (int w = 0; w < param.actual_width_; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = src_base_index + src_u;

			if (src_index != cached_src_index)
			{
				cached_src_index = src_index;
				cached_color = src_palette[param.indexed_pixels_[src_index]];
			}

			*dst_colors++ = cached_color;

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (row major, has alpha) -> RGBA POT
void RendererUtils::indexed_npot_to_rgba_rm_ha(
	const IndexedToRgbaParam& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = R8g8b8a8{};

	for (int h = 0; h < param.actual_height_; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		const auto src_base_index = src_v * param.width_;

		auto src_u_f = 0.0;

		for (int w = 0; w < param.actual_width_; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = src_base_index + src_u;

			auto& dst_pixel = *dst_colors++;

			if (param.indexed_alphas_[src_index])
			{
				if (src_index != cached_src_index)
				{
					cached_src_index = src_index;
					cached_color = src_palette[param.indexed_pixels_[src_index]];
				}

				dst_pixel = cached_color;
			}
			else
			{
				dst_pixel.reset();
			}

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (column major, has no alpha) -> RGBA POT
void RendererUtils::indexed_npot_to_rgba_cm_na(
	const IndexedToRgbaParam& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = R8g8b8a8{};

	for (int h = 0; h < param.actual_height_; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);

		auto src_u_f = 0.0;

		for (int w = 0; w < param.actual_width_; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);

			const auto src_index = (src_u * param.height_) + src_v;

			if (src_index != cached_src_index)
			{
				cached_src_index = src_index;
				cached_color = src_palette[param.indexed_pixels_[src_index]];
			}

			*dst_colors++ = cached_color;

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (column major, has alpha) -> RGBA POT
void RendererUtils::indexed_npot_to_rgba_cm_ha(
	const IndexedToRgbaParam& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = R8g8b8a8{};

	for (int h = 0; h < param.actual_height_; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);

		auto src_u_f = 0.0;

		for (int w = 0; w < param.actual_width_; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);

			const auto src_index = (src_u * param.height_) + src_v;

			auto& dst_pixel = *dst_colors++;

			if (param.indexed_alphas_[src_index])
			{
				if (src_index != cached_src_index)
				{
					cached_src_index = src_index;
					cached_color = src_palette[param.indexed_pixels_[src_index]];
				}

				dst_pixel = cached_color;
			}
			else
			{
				dst_pixel.reset();
			}

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

void RendererUtils::build_mipmap_1(
	const int previous_dimension,
	const R8g8b8a8CPtr src_colors,
	const R8g8b8a8Ptr dst_colors)
{
	const auto dimension = previous_dimension / 2;

	auto src_colors_0 = src_colors;
	auto dst_colors_0 = dst_colors;

	for (int i = 0; i < dimension; ++i)
	{
		*dst_colors_0++ = R8g8b8a8::average_pa(
			src_colors_0[0],
			src_colors_0[1]
		);

		src_colors_0 += 2;
	}
}

//
// RendererUtils
// ==========================================================================


} // detail
} // bstone
