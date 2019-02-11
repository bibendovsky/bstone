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
// OpenGL Renderer utils.
//
// !!! Internal usage only. !!!
//


#include "bstone_precompiled.h"
#include "bstone_ogl_renderer_utils.h"
#include <cassert>
#include "SDL_video.h"
#include "SDL_opengl.h"


namespace bstone
{


// ==========================================================================
// OglRendererUtils::Detail
//

class OglRendererUtils::Detail
{
public:
	static bool load_library(
		std::string& error_message);

	static void unload_library();


	static bool create_context_validate_params(
		SdlWindowPtr sdl_window,
		std::string& error_message);

	static bool create_context(
		SdlWindowPtr sdl_window,
		SdlGlContext& sdl_ogl_context,
		std::string& error_message);


	static bool make_context_current_validate_params(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_ogl_context,
		std::string& error_message);

	static bool make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_ogl_context,
		std::string& error_message);
}; // OglRendererUtils::Detail


bool OglRendererUtils::Detail::load_library(
	std::string& error_message)
{
	const auto sdl_result = ::SDL_GL_LoadLibrary(nullptr);

	if (sdl_result < 0)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

void OglRendererUtils::Detail::unload_library()
{
	::SDL_GL_UnloadLibrary();
}

bool OglRendererUtils::Detail::create_context_validate_params(
	SdlWindowPtr sdl_window,
	std::string& error_message)
{
	if (!sdl_window)
	{
		error_message = "Null SDL window.";

		return false;
	}

	return true;
}

bool OglRendererUtils::Detail::create_context(
	SdlWindowPtr sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	sdl_ogl_context = ::SDL_GL_CreateContext(sdl_window);

	if (!sdl_ogl_context)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

bool OglRendererUtils::Detail::make_context_current_validate_params(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	if (!sdl_window)
	{
		error_message = "Null SDL window.";

		return false;
	}

	static_cast<void>(sdl_ogl_context);

	return true;
}

bool OglRendererUtils::Detail::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	const auto sdl_result = ::SDL_GL_MakeCurrent(sdl_window, sdl_ogl_context);

	if (sdl_result < 0)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

//
// OglRendererUtils::Detail
// ==========================================================================


// ==========================================================================
// OglRendererUtils
//

bool OglRendererUtils::load_library(
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to load default OpenGL library. ";

	if (!Detail::load_library(error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

void OglRendererUtils::unload_library()
{
	Detail::unload_library();
}

bool OglRendererUtils::create_context(
	SdlWindowPtr sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to create OpenGL context. ";

	sdl_ogl_context = nullptr;

	if (!Detail::create_context_validate_params(sdl_window, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::create_context(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

bool OglRendererUtils::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to make a context current. ";

	if (!Detail::make_context_current_validate_params(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::make_context_current(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

void OglRendererUtils::swap_window(
	SdlWindowPtr sdl_window)
{
	assert(sdl_window != nullptr);

	::SDL_GL_SwapWindow(sdl_window);
}

//
// OglRendererUtils
// ==========================================================================


} // bstone
