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


#include "bstone_precompiled.h"
#include "bstone_ogl_renderer_utils.h"
#include "bstone_renderer_utils.h"
#include "SDL_video.h"


namespace bstone
{


// ==========================================================================
// RendererUtils::Detail
//

class RendererUtils::Detail
{
public:
	static bool create_window_validate_param(
		const RendererUtilsCreateWindowParam& param,
		std::string& error_message);

	static bool create_window_set_ogl_attributes(
		std::string& error_message);

	static Uint32 create_window_sdl_flags(
		const RendererUtilsCreateWindowParam& param);

	static bool create_window(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowPtr& sdl_window,
		std::string& error_message);
}; // RendererUtils::Detail


bool RendererUtils::Detail::create_window_validate_param(
	const RendererUtilsCreateWindowParam& param,
	std::string& error_message)
{
	if (param.width_ <= 0 || param.height_ <= 0)
	{
		error_message = "Invalid dimensions.";

		return false;
	}

	return true;
}

bool RendererUtils::Detail::create_window_set_ogl_attributes(
	std::string& error_message)
{
	::SDL_GL_ResetAttributes();


	// Result.
	//
	return true;
}

Uint32 RendererUtils::Detail::create_window_sdl_flags(
	const RendererUtilsCreateWindowParam& param)
{
	auto flags = Uint32{};

	if (param.is_visible_)
	{
		flags |= SDL_WINDOW_SHOWN;
	}
	else
	{
		flags |= SDL_WINDOW_HIDDEN;
	}

	if (param.is_fullscreen_desktop_)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (param.is_opengl_)
	{
		flags |= SDL_WINDOW_OPENGL;
	}

	return flags;
}

bool RendererUtils::Detail::create_window(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowPtr& sdl_window,
	std::string& error_message)
{
	const auto sdl_flags = create_window_sdl_flags(param);

	sdl_window = ::SDL_CreateWindow(
		param.title_utf8_.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		param.width_,
		param.height_,
		sdl_flags
	);

	if (!sdl_window)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

//
// RendererUtils::Detail
// ==========================================================================


// ==========================================================================
// RendererUtils
//

bool RendererUtils::load_libraries(
	std::string& error_message)
{
	if (!OglRendererUtils::load_library(error_message))
	{
		return false;
	}

	return true;
}

void RendererUtils::unload_libraries()
{
	OglRendererUtils::unload_library();
}

bool RendererUtils::create_window(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowPtr& sdl_window,
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to create a window. ";

	sdl_window = nullptr;

	if (!Detail::create_window_validate_param(param, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::create_window_set_ogl_attributes(error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::create_window(param, sdl_window, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

//
// RendererUtils
// ==========================================================================


} // bstone
