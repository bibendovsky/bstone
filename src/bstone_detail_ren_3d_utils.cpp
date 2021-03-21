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


#include "bstone_detail_ren_3d_utils.h"

#include <algorithm>
#include <unordered_set>

#include "bstone_exception.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_sdl2_exception.h"
#include "bstone_sprite.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dUtilsCreateWindowException
//

class Ren3dUtilsCreateWindowException :
	public Exception
{
public:
	explicit Ren3dUtilsCreateWindowException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_CREATE_WND] "} + message}
	{
	}
}; // Ren3dUtilsCreateWindowException

//
// Ren3dUtilsCreateWindowException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsSetWindowModeException
//

class Ren3dUtilsSetWindowModeException :
	public Exception
{
public:
	explicit Ren3dUtilsSetWindowModeException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_SET_WND_MODE] "} + message}
	{
	}
}; // Ren3dUtilsSetWindowModeException

//
// Ren3dUtilsSetWindowModeException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsShowWindowException
//

class Ren3dUtilsShowWindowException :
	public Exception
{
public:
	explicit Ren3dUtilsShowWindowException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_SHOW_WND] "} + message}
	{
	}
}; // Ren3dUtilsShowWindowException

//
// Ren3dUtilsShowWindowException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsSetWindowTitleException
//

class Ren3dUtilsSetWindowTitleException :
	public Exception
{
public:
	explicit Ren3dUtilsSetWindowTitleException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_SET_WND_TITLE] "} + message}
	{
	}
}; // Ren3dUtilsSetWindowTitleException

//
// Ren3dUtilsSetWindowTitleException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsRendererInitException
//

class Ren3dUtilsRendererInitException :
	public Exception
{
public:
	explicit Ren3dUtilsRendererInitException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_RENDER_INIT] "} + message}
	{
	}
}; // Ren3dUtilsRendererInitException

//
// Ren3dUtilsRendererInitException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsBufferInitException
//

class Ren3dUtilsBufferInitException :
	public Exception
{
public:
	explicit Ren3dUtilsBufferInitException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_BUFFER_INIT] "} + message}
	{
	}
}; // Ren3dUtilsBufferInitException

//
// Ren3dUtilsBufferInitException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsBufferUpdateException
//

class Ren3dUtilsBufferUpdateException :
	public Exception
{
public:
	explicit Ren3dUtilsBufferUpdateException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_BUFFER_UPD] "} + message}
	{
	}
}; // Ren3dUtilsBufferUpdateException

//
// Ren3dUtilsBufferUpdateException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsVertexInputCreateException
//

class Ren3dUtilsVertexInputCreateException :
	public Exception
{
public:
	explicit Ren3dUtilsVertexInputCreateException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_VTX_INPUT_INIT] "} + message}
	{
	}
}; // Ren3dUtilsVertexInputCreateException

//
// Ren3dUtilsVertexInputCreateException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsIndexedToRgba8Exception
//

class Ren3dUtilsIndexedToRgba8Exception :
	public Exception
{
public:
	explicit Ren3dUtilsIndexedToRgba8Exception(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_IDXED_TO_RGBA8] "} + message}
	{
	}
}; // Ren3dUtilsIndexedToRgba8Exception

//
// Ren3dUtilsIndexedToRgba8Exception
// ==========================================================================

// ==========================================================================
// Ren3dUtilsIndexedNpotToRgba8PotException
//

class Ren3dUtilsIndexedNpotToRgba8PotException :
	public Exception
{
public:
	explicit Ren3dUtilsIndexedNpotToRgba8PotException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_IDXED_NPOT_TO_RGBA8_POT] "} + message}
	{
	}
}; // Ren3dUtilsIndexedNpotToRgba8PotException

//
// Ren3dUtilsIndexedNpotToRgba8PotException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsIndexedSpriteToRgba8PotException
//

class Ren3dUtilsIndexedSpriteToRgba8PotException :
	public Exception
{
public:
	explicit Ren3dUtilsIndexedSpriteToRgba8PotException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_IDXED_SPR_TO_RGBA8_POT] "} + message}
	{
	}
}; // Ren3dUtilsIndexedSpriteToRgba8PotException

//
// Ren3dUtilsIndexedSpriteToRgba8PotException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsRgba8NpotToRgba8PotException
//

class Ren3dUtilsRgba8NpotToRgba8PotException :
	public Exception
{
public:
	explicit Ren3dUtilsRgba8NpotToRgba8PotException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_RGBA8_NPOT_TO_RGBA8_POT] "} + message}
	{
	}
}; // Ren3dUtilsRgba8NpotToRgba8PotException

//
// Ren3dUtilsIndexedSpriteToRgba8PotException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsBuildMipmapException
//

class Ren3dUtilsBuildMipmapException :
	public Exception
{
public:
	explicit Ren3dUtilsBuildMipmapException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_BUILD_MIPMAP] "} + message}
	{
	}
}; // Ren3dUtilsBuildMipmapException

//
// Ren3dUtilsBuildMipmapException
// ==========================================================================

// ==========================================================================
// Ren3dUtilsBuildMipmapException
//

class Ren3dUtilsCalcMipmapCountException :
	public Exception
{
public:
	explicit Ren3dUtilsCalcMipmapCountException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_UTL_CALC_MIPMAP_CNT] "} + message}
	{
	}
}; // Ren3dUtilsCalcMipmapCountException

//
// Ren3dUtilsCalcMipmapCountException
// ==========================================================================


// ==========================================================================
// Ren3dUtils
//

float Ren3dUtils::deg_to_rad(
	const float angle_deg)
{
	return angle_deg * pi_over_180;
}

bool Ren3dUtils::is_pot_value(
		const int value)
{
	const auto nearest_value = find_nearest_pot_value(value);

	return nearest_value == value;
}

int Ren3dUtils::find_nearest_pot_value(
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

int Ren3dUtils::calculate_mipmap_count(
	const int width,
	const int height)
{
	//
	// mipmap_count = [log2(max(width, height))] + 1
	//

	if (width <= 0)
	{
		throw Ren3dUtilsCalcMipmapCountException("Width out of range.");
	}

	if (height <= 0)
	{
		throw Ren3dUtilsCalcMipmapCountException("Height out of range.");
	}

	auto log_2 = 0;
	auto max_size = std::max(width, height);

	while (max_size > 1)
	{
		log_2 += 1;
		max_size /= 2;
	}

	const auto result = log_2 + 1;

	return result;
}

SdlWindowUPtr Ren3dUtils::create_window(
	const Ren3dUtilsCreateWindowParam& param)
{
	create_window_validate_param(param);
	create_window_set_gl_attributes(param);

	const auto sdl_flags = create_window_sdl_flags(param);

	const auto x = (
		param.window_.is_positioned_ ?
			param.window_.rect_2d_.offset_.x_
			:
			SDL_WINDOWPOS_CENTERED
	);

	const auto y = (
		param.window_.is_positioned_ ?
			param.window_.rect_2d_.offset_.y_
			:
			SDL_WINDOWPOS_CENTERED
	);

	auto sdl_window = SdlWindowUPtr{SDL_CreateWindow(
		param.window_.title_.c_str(),
		x,
		y,
		param.window_.rect_2d_.extent_.width_,
		param.window_.rect_2d_.extent_.height_,
		sdl_flags
	)};

	try
	{
		Sdl2EnsureResult{sdl_window};
	}
	catch (const std::exception&)
	{
		std::throw_with_nested(Ren3dUtilsCreateWindowException{"Failed."});
	}

	return sdl_window;
}

void Ren3dUtils::set_window_mode(
	SdlWindowPtr sdl_window,
	const Ren3dSetWindowModeParam& param)
{
	if (!sdl_window)
	{
		throw Ren3dUtilsSetWindowModeException{"Null window."};
	}

	if (param.rect_2d_.extent_.height_ <= 0)
	{
		throw Ren3dUtilsSetWindowModeException{"Height out of range."};
	}

	if (param.rect_2d_.extent_.width_ <= 0)
	{
		throw Ren3dUtilsSetWindowModeException{"Width out of range."};
	}


	//
	const auto sdl_window_flags = SDL_GetWindowFlags(sdl_window);

	const auto is_current_windowed =
		((sdl_window_flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) == 0);

	const auto is_windowed_changed = (is_current_windowed != param.is_windowed_);


	//
	int current_width = 0;
	int current_height = 0;

	SDL_GetWindowSize(sdl_window, &current_width, &current_height);

	if (current_width <= 0 || current_height <= 0)
	{
		throw Ren3dUtilsSetWindowModeException{"Failed to get current window size."};
	}

	const auto is_size_changed = (
		current_width != param.rect_2d_.extent_.width_ ||
		current_height != param.rect_2d_.extent_.height_);


	//
	if (!is_windowed_changed && !is_size_changed)
	{
		return;
	}

	if (is_windowed_changed)
	{
		const Uint32 sdl_fullscreen_flags = (param.is_windowed_ ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);

		const auto sdl_result = SDL_SetWindowFullscreen(sdl_window, sdl_fullscreen_flags);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsSetWindowModeException{"Failed to change fullscreen mode."};
		}
	}

	if (is_size_changed && param.is_windowed_)
	{
		SDL_SetWindowSize(sdl_window, param.rect_2d_.extent_.width_, param.rect_2d_.extent_.height_);

		if (param.is_positioned_)
		{
			const auto x = std::max(param.rect_2d_.offset_.x_, 0);
			const auto y = std::max(param.rect_2d_.offset_.y_, 0);

			SDL_SetWindowPosition(sdl_window, x, y);
		}
		else
		{
			SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}
	}
}

void Ren3dUtils::show_window(
	SdlWindowPtr sdl_window,
	const bool is_visible)
{
	if (!sdl_window)
	{
		throw Ren3dUtilsShowWindowException{"Null window."};
	}

	const auto sdl_function = (is_visible ? SDL_ShowWindow : SDL_HideWindow);

	sdl_function(sdl_window);
}

void Ren3dUtils::set_window_title(
	const SdlWindowPtr sdl_window,
	const std::string& title_utf8)
{
	if (!sdl_window)
	{
		throw Ren3dUtilsSetWindowTitleException{"Null window."};
	}

	SDL_SetWindowTitle(sdl_window, title_utf8.c_str());
}

void Ren3dUtils::validate_initialize_param(
	const Ren3dCreateParam& param)
{
	switch (param.renderer_kind_)
	{
	case Ren3dKind::gl_2_0:
	case Ren3dKind::gl_3_2_core:
	case Ren3dKind::gles_2_0:
		break;

	default:
		throw Ren3dUtilsRendererInitException{"Unsupported renderer kind."};
	}

	if (param.window_.rect_2d_.extent_.width_ <= 0)
	{
		throw Ren3dUtilsRendererInitException{"Invalid window width."};
	}

	if (param.window_.rect_2d_.extent_.height_ <= 0)
	{
		throw Ren3dUtilsRendererInitException{"Invalid window height."};
	}

	switch (param.aa_kind_)
	{
		case Ren3dAaKind::none:
		case Ren3dAaKind::ms:
			break;

		default:
			throw Ren3dUtilsRendererInitException{"Invalid antialiasing kind."};
	}
}

void Ren3dUtils::validate_buffer_create_param(
	const Ren3dCreateBufferParam& param)
{
	switch (param.usage_kind_)
	{
		case Ren3dBufferUsageKind::draw_streaming:
		case Ren3dBufferUsageKind::draw_static:
		case Ren3dBufferUsageKind::draw_dynamic:
			break;

		default:
			throw Ren3dUtilsBufferInitException{"Invalid usage kind."};
	}

	if (param.size_ <= 0)
	{
		throw Ren3dUtilsBufferInitException{"Invalid size."};
	}
}

void Ren3dUtils::validate_buffer_update_param(
	const Ren3dUpdateBufferParam& param)
{
	if (param.offset_ < 0)
	{
		throw Ren3dUtilsBufferUpdateException{"Invalid offset."};
	}

	if (param.size_ <= 0)
	{
		throw Ren3dUtilsBufferUpdateException{"Invalid count."};
	}

	if (!param.data_)
	{
		throw Ren3dUtilsBufferUpdateException{"Null indices."};
	}
}

void Ren3dUtils::vertex_input_validate_format(
	const Ren3dVertexAttribFormat attribute_format)
{
	switch (attribute_format)
	{
		case Ren3dVertexAttribFormat::rgba_8_unorm:
		case Ren3dVertexAttribFormat::rg_32_sfloat:
		case Ren3dVertexAttribFormat::rgb_32_sfloat:
			return;

		default:
			throw Ren3dUtilsVertexInputCreateException{"Unsupported vertex input attribute format."};
	}
}

void Ren3dUtils::vertex_input_validate_param(
	const int max_locations,
	const Ren3dCreateVertexInputParam& param)
{
	if (max_locations < 0)
	{
		throw Ren3dUtilsVertexInputCreateException{"Maximum location count out of range."};
	}

	if (param.index_buffer_)
	{
		if (param.index_buffer_->get_kind() != Ren3dBufferKind::index)
		{
			throw Ren3dUtilsVertexInputCreateException{"Non-index buffer."};
		}
	}

	const auto& attribute_descriptions = param.attrib_descrs_;

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
			throw Ren3dUtilsVertexInputCreateException{"Location out of range."};
		}

		const auto has_location = (used_locations.find(location) != used_locations.cend());

		if (has_location)
		{
			throw Ren3dUtilsVertexInputCreateException{"Duplicate location."};
		}

		used_locations.insert(location);


		// Format.
		//
		vertex_input_validate_format(attribute_description.format_);


		// Vertex buffer.
		//
		if (!attribute_description.vertex_buffer_)
		{
			throw Ren3dUtilsVertexInputCreateException{"Null vertex buffer."};
		}

		if (attribute_description.vertex_buffer_->get_kind() != Ren3dBufferKind::vertex)
		{
			throw Ren3dUtilsVertexInputCreateException{"Non-vertex buffer."};
		}


		// Offset.
		//
		if (attribute_description.offset_ < 0)
		{
			throw Ren3dUtilsVertexInputCreateException{"Invalid offset."};
		}


		// Stride.
		//
		if (attribute_description.stride_ < 0)
		{
			throw Ren3dUtilsVertexInputCreateException{"Invalid stride."};
		}
	}
}

void Ren3dUtils::indexed_to_rgba_8(
	const IndexedToRgba8Param& param)
{
	if (param.width_ <= 0)
	{
		throw Ren3dUtilsIndexedToRgba8Exception{"Width out of range."};
	}

	if (param.height_ <= 0)
	{
		throw Ren3dUtilsIndexedToRgba8Exception{"Height out of range."};
	}

	if (!param.indexed_pixels_)
	{
		throw Ren3dUtilsIndexedToRgba8Exception{"Null indexed pixels."};
	}

	if (param.rgba_8_buffer_->size() < static_cast<std::size_t>((param.width_ * param.height_)))
	{
		throw Ren3dUtilsIndexedToRgba8Exception{"Bitmap buffer too small."};
	}

	const auto is_column_major = param.indexed_is_column_major_;
	const auto has_alphas = (param.indexed_alphas_ != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_to_rgba_8_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_to_rgba_8_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_to_rgba_8_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_to_rgba_8_cm_ha(param);
	}
	else
	{
		throw Ren3dUtilsIndexedToRgba8Exception{"Unsupported values combination."};
	}
}

void Ren3dUtils::indexed_npot_to_rgba_8_pot(
	const IndexedToRgba8Param& param)
{
	if (param.width_ <= 0)
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Width out of range."};
	}

	if (param.height_ <= 0)
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Height out of range."};
	}

	if (param.actual_width_ <= 0)
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Actual width out of range."};
	}

	if (param.actual_height_ <= 0)
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Actual height out of range."};
	}

	if (!param.indexed_pixels_)
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Null indexed pixels."};
	}

	if (param.rgba_8_buffer_->size() < static_cast<std::size_t>((param.actual_width_ * param.actual_height_)))
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Indexed bitmap buffer too small."};
	}

	const auto is_column_major = param.indexed_is_column_major_;
	const auto has_alphas = (param.indexed_alphas_ != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_8_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_8_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_8_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_8_cm_ha(param);
	}
	else
	{
		throw Ren3dUtilsIndexedNpotToRgba8PotException{"Unsupported values combination."};
	}
}

void Ren3dUtils::indexed_to_rgba_8_pot(
	const IndexedToRgba8Param& param)
{
	const auto is_npot = (
		param.width_ != param.actual_width_ ||
		param.height_ != param.actual_height_
	);

	if (!is_npot)
	{
		indexed_to_rgba_8(param);
	}
	else if (is_npot)
	{
		indexed_npot_to_rgba_8_pot(param);
	}
}

void Ren3dUtils::indexed_sprite_to_rgba_8_pot(
	const Sprite& indexed_sprite,
	const Rgba8Palette& indexed_palette,
	Rgba8Buffer& texture_buffer)
{
	if (!indexed_sprite.is_initialized())
	{
		throw Ren3dUtilsIndexedSpriteToRgba8PotException{"Indexed sprite not initialized."};
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

void Ren3dUtils::rgba_8_npot_to_rgba_8_pot(
	const int width,
	const int height,
	const int actual_width,
	const int actual_height,
	const Rgba8* const rgba_8_pixels,
	Rgba8Buffer& texture_buffer)
{
	if (width <= 0)
	{
		throw Ren3dUtilsRgba8NpotToRgba8PotException{"Width out of range."};
	}

	if (height <= 0)
	{
		throw Ren3dUtilsRgba8NpotToRgba8PotException{"Height out of range."};
	}

	if (actual_width <= 0)
	{
		throw Ren3dUtilsRgba8NpotToRgba8PotException{"Actual width out of range."};
	}

	if (actual_height <= 0)
	{
		throw Ren3dUtilsRgba8NpotToRgba8PotException{"Actual height out of range."};
	}

	if (!rgba_8_pixels)
	{
		throw Ren3dUtilsRgba8NpotToRgba8PotException{"Null RGBA pixels."};
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

			texture_buffer[dst_index] = rgba_8_pixels[src_index];

			++dst_index;

			src_u_d += src_du_f;
		}

		src_v_d += src_dv_f;
	}
}

void Ren3dUtils::build_mipmap(
	const int previous_width,
	const int previous_height,
	const Rgba8CPtr src_colors,
	const Rgba8Ptr dst_colors)
{
	if (previous_width <= 0)
	{
		throw Ren3dUtilsBuildMipmapException{"Previous width out of range."};
	}

	if (previous_height <= 0)
	{
		throw Ren3dUtilsBuildMipmapException{"Previous height out of range."};
	}

	if (previous_width == 1 && previous_height == 1)
	{
		throw Ren3dUtilsBuildMipmapException{"No more mipmap to build."};
	}

	if (!src_colors)
	{
		throw Ren3dUtilsBuildMipmapException{"Null source colors."};
	}

	if (!dst_colors)
	{
		throw Ren3dUtilsBuildMipmapException{"Null target colors."};
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
			*dst_colors_0++ = average_pa(
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

void Ren3dUtils::create_window_validate_param(
	const Ren3dUtilsCreateWindowParam& param)
{
	if (param.window_.is_positioned_ && (
		param.window_.rect_2d_.offset_.x_ < 0 ||
			param.window_.rect_2d_.offset_.y_ < 0))
	{
		throw Ren3dUtilsCreateWindowException{"Invalid position."};
	}

	if (param.window_.rect_2d_.extent_.width_ <= 0 ||
		param.window_.rect_2d_.extent_.height_ <= 0)
	{
		throw Ren3dUtilsCreateWindowException{"Invalid dimensions."};
	}
}

void Ren3dUtils::create_window_set_gl_profile_and_version(
	const Ren3dKind renderer_kind)
{
	auto sdl_profile_mask = 0;
	auto sdl_version_major = 0;
	auto sdl_version_minor = 0;

	switch (renderer_kind)
	{
		case Ren3dKind::gl_2_0:
			sdl_profile_mask = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
			sdl_version_major = 2;
			sdl_version_minor = 0;
			break;

		case Ren3dKind::gl_3_2_core:
			sdl_profile_mask = SDL_GL_CONTEXT_PROFILE_CORE;
			sdl_version_major = 3;
			sdl_version_minor = 2;
			break;

		case Ren3dKind::gles_2_0:
			sdl_profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
			sdl_version_major = 2;
			sdl_version_minor = 0;
			break;

		default:
			throw Ren3dUtilsCreateWindowException{"Unsupported 3D-renderer kind."};
	}

	{
		const auto sdl_result = SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_PROFILE_MASK,
			sdl_profile_mask
		);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set OpenGL context core profile attribute."};
		}
	}

	{
		const auto sdl_result = SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MAJOR_VERSION,
			sdl_version_major
		);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set OpenGL context major version attribute."};
		}
	}

	{
		const auto sdl_result = SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MINOR_VERSION,
			sdl_version_minor
		);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set OpenGL context minor version attribute."};
		}
	}
}

void Ren3dUtils::create_window_set_gl_attributes(
	const Ren3dUtilsCreateWindowParam& param)
{
	SDL_GL_ResetAttributes();

	auto sdl_result = 0;

	switch (param.aa_kind_)
	{
		case Ren3dAaKind::ms:
			if (param.aa_value_ >= Ren3dLimits::min_aa_on)
			{
				sdl_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);

				if (sdl_result != 0)
				{
					throw Ren3dUtilsCreateWindowException{"Failed to set multisample buffer count."};
				}

				sdl_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, param.aa_value_);

				if (sdl_result != 0)
				{
					throw Ren3dUtilsCreateWindowException{"Failed to set multisample sample count."};
				}
			}

			break;


		case Ren3dAaKind::none:
		default:
			sdl_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

			if (sdl_result != 0)
			{
				throw Ren3dUtilsCreateWindowException{"Failed to set multisample buffer count."};
			}

			sdl_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

			if (sdl_result != 0)
			{
				throw Ren3dUtilsCreateWindowException{"Failed to set multisample sample count."};
			}

			break;
	}

	{
		sdl_result = SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set a red bit depth."};
		}
	}

	{
		sdl_result = SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set a green bit depth."};
		}
	}

	{
		sdl_result = SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set a blue bit depth."};
		}
	}

	{
		sdl_result = SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set an alpha bit depth."};
		}
	}

	{
		sdl_result = SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set an accelerated visual."};
		}
	}

	{
		const auto gl_depth_size = (
			param.is_default_depth_buffer_disabled_ ?
			0 :
			16
		);

		sdl_result = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gl_depth_size);

		if (sdl_result != 0)
		{
			throw Ren3dUtilsCreateWindowException{"Failed to set depth buffer bit depth."};
		}
	}

	create_window_set_gl_profile_and_version(param.renderer_kind_);
}

std::uint32_t Ren3dUtils::create_window_sdl_flags(
	const Ren3dUtilsCreateWindowParam& param)
{
	auto flags = Uint32{SDL_WINDOW_ALLOW_HIGHDPI};

	if (param.window_.is_visible_)
	{
		flags |= SDL_WINDOW_SHOWN;
	}
	else
	{
		flags |= SDL_WINDOW_HIDDEN;
	}

	if (param.window_.is_fake_fullscreen_)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (param.window_.is_borderless_)
	{
		flags |= SDL_WINDOW_BORDERLESS;
	}

	switch (param.renderer_kind_)
	{
		case Ren3dKind::gl_2_0:
		case Ren3dKind::gl_3_2_core:
		case Ren3dKind::gles_2_0:
			flags |= SDL_WINDOW_OPENGL;
			break;

		default:
			break;
	}

	return flags;
}

// Indexed (row major, has no alpha) -> RGBA
void Ren3dUtils::indexed_to_rgba_8_rm_na(
	const IndexedToRgba8Param& param)
{
	auto src_pixels = param.indexed_pixels_;
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_8_buffer_->data();

	const auto area = param.width_ * param.height_;

	for (int i = 0; i < area; ++i)
	{
		*dst_pixels++ = src_palette[*src_pixels++];
	}
}

// Indexed (row major, has alpha) -> RGBA
void Ren3dUtils::indexed_to_rgba_8_rm_ha(
	const IndexedToRgba8Param& param)
{
	auto src_pixels = param.indexed_pixels_;
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_8_buffer_->data();

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
void Ren3dUtils::indexed_to_rgba_8_cm_na(
	const IndexedToRgba8Param& param)
{
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_8_buffer_->data();

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
void Ren3dUtils::indexed_to_rgba_8_cm_ha(
	const IndexedToRgba8Param& param)
{
	const auto& src_palette = *param.indexed_palette_;

	auto dst_pixels = param.rgba_8_buffer_->data();

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
void Ren3dUtils::indexed_npot_to_rgba_8_rm_na(
	const IndexedToRgba8Param& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_8_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

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
void Ren3dUtils::indexed_npot_to_rgba_8_rm_ha(
	const IndexedToRgba8Param& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_8_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

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
void Ren3dUtils::indexed_npot_to_rgba_8_cm_na(
	const IndexedToRgba8Param& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_8_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

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
void Ren3dUtils::indexed_npot_to_rgba_8_cm_ha(
	const IndexedToRgba8Param& param)
{
	const auto src_du_f = static_cast<double>(param.width_) / static_cast<double>(param.actual_width_);
	const auto src_dv_f = static_cast<double>(param.height_) / static_cast<double>(param.actual_height_);

	const auto& src_palette = *param.indexed_palette_;
	auto dst_colors = param.rgba_8_buffer_->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

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

void Ren3dUtils::build_mipmap_1(
	const int previous_dimension,
	const Rgba8CPtr src_colors,
	const Rgba8Ptr dst_colors)
{
	const auto dimension = previous_dimension / 2;

	auto src_colors_0 = src_colors;
	auto dst_colors_0 = dst_colors;

	for (int i = 0; i < dimension; ++i)
	{
		*dst_colors_0++ = average_pa(
			src_colors_0[0],
			src_colors_0[1]
		);

		src_colors_0 += 2;
	}
}

Rgba8 Ren3dUtils::average_pa(
	const Rgba8 color_0,
	const Rgba8 color_1) noexcept
{
	constexpr auto color_count = 2;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_;
	const auto r_sum = color_0.r_ + color_1.r_;
	const auto g_sum = color_0.g_ + color_1.g_;
	const auto b_sum = color_0.b_ + color_1.b_;

	return Rgba8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

Rgba8 Ren3dUtils::average_pa(
	const Rgba8 color_0,
	const Rgba8 color_1,
	const Rgba8 color_2,
	const Rgba8 color_3) noexcept
{
	constexpr auto color_count = 4;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_ + color_2.a_ + color_3.a_;
	const auto r_sum = color_0.r_ + color_1.r_ + color_2.r_ + color_3.r_;
	const auto g_sum = color_0.g_ + color_1.g_ + color_2.g_ + color_3.g_;
	const auto b_sum = color_0.b_ + color_1.b_ + color_2.b_ + color_3.b_;

	return Rgba8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

//
// Ren3dUtils
// ==========================================================================


} // detail
} // bstone
