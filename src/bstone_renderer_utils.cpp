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
#include "bstone_renderer_utils.h"
#include "SDL_video.h"


namespace bstone
{


// ==========================================================================
// RendererUtils
//

const std::string& RendererUtils::get_error_message() const
{
	return error_message_;
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

bool RendererUtils::create_window(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowPtr& sdl_window)
{
	const auto error_message_prefix = "Failed to create a window. ";

	sdl_window = nullptr;

	if (!create_window_validate_param(param))
	{
		error_message_ = error_message_prefix + error_message_;

		return false;
	}

	if (!create_window_set_ogl_attributes())
	{
		error_message_ = error_message_prefix + error_message_;

		return false;
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

	sdl_window = ::SDL_CreateWindow(
		param.window_.title_utf8_.c_str(),
		x,
		y,
		param.window_.width_,
		param.window_.height_,
		sdl_flags
	);

	if (!sdl_window)
	{
		error_message_ = error_message_prefix;
		error_message_ += ::SDL_GetError();

		return false;
	}

	return true;
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
	switch (param.renderer_path_)
	{
	case RendererPath::ogl_1_x:
		return true;

	default:
		error_message_ = "Unsupported renderer path.";

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

	return true;
}

bool RendererUtils::validate_index_buffer_create_param(
	const RendererIndexBufferCreateParam& param)
{
	if (param.index_count_ <= 0)
	{
		error_message_ = "Invalid vertex count.";

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

	if (param.count_ <= 0)
	{
		error_message_ = "Invalid count.";

		return false;
	}

	if (!param.indices_)
	{
		error_message_ = "Null indices.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_vertex_buffer_create_param(
	const RendererVertexBufferCreateParam& param)
{
	if (param.vertex_count_ <= 0)
	{
		error_message_ = "Invalid vertex count.";

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

	if (param.count_ <= 0)
	{
		error_message_ = "Invalid count.";

		return false;
	}

	if (!param.vertices_)
	{
		error_message_ = "Null vertices.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_texture_2d_create_param(
	const RendererTexture2dCreateParam& param)
{
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

	if (!param.indexed_pixels_ && !param.rgba_pixels_)
	{
		error_message_ = "Null pixel source.";

		return false;
	}

	if (param.indexed_pixels_ && param.rgba_pixels_)
	{
		error_message_ = "Multiple pixel sources.";

		return false;
	}

	return true;
}

bool RendererUtils::validate_texture_2d_update_param(
	const RendererTexture2dUpdateParam& param)
{
	static_cast<void>(param);
	static_cast<void>(error_message_);

	return true;
}

bool RendererUtils::is_ogl_renderer_path(
	const RendererPath renderer_path)
{
	switch (renderer_path)
	{
	case RendererPath::ogl_1_x:
		return true;

	default:
		return false;
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

bool RendererUtils::create_window_set_ogl_attributes()
{
	::SDL_GL_ResetAttributes();

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


} // bstone
