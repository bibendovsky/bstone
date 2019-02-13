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


#ifndef BSTONE_OGL_RENDERER_UTILS_INCLUDED
#define BSTONE_OGL_RENDERER_UTILS_INCLUDED


#include <vector>
#include "bstone_renderer_utils.h"


using SdlGlContext = void*;


namespace bstone
{


class OglRendererUtils
{
public:
	using Strings = std::vector<std::string>;


	static bool load_library(
		std::string& error_message);

	static void unload_library();


	static bool create_context(
		SdlWindowPtr sdl_window,
		SdlGlContext& sdl_gl_context,
		std::string& error_message);

	static bool make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_gl_context,
		std::string& error_message);


	static bool create_window_and_context(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context,
		std::string& error_message);

	static bool create_probe_window_and_context(
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context,
		std::string& error_message);

	static void destroy_window_and_context(
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context);


	static bool resolve_symbols_1_1(
		Strings& missing_symbols);


	static void swap_window(
		SdlWindowPtr sdl_window);


private:
	class Detail;
}; // OglRendererUtils


} // bstone


#endif // !BSTONE_OGL_RENDERER_UTILS_INCLUDED
