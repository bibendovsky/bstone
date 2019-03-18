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


#ifndef BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED
#define BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED


#include <vector>
#include "bstone_matn.h"
#include "bstone_detail_renderer_utils.h"


using SdlGlContext = void*;


namespace bstone
{
namespace detail
{


class OglRendererUtils
{
public:
	const std::string& get_error_message() const;


	bool load_library();

	static void unload_library();


	bool create_context(
		SdlWindowPtr sdl_window,
		SdlGlContext& sdl_gl_context);

	bool make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_gl_context);


	bool create_window_and_context(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context);

	bool create_probe_window_and_context(
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context);

	void destroy_window_and_context(
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_gl_context);

	bool window_get_drawable_size(
		SdlWindowPtr& sdl_window,
		int& width,
		int& height);


	static bool resolve_symbols_1_1();


	static void clear_buffers();

	static void swap_window(
		SdlWindowPtr sdl_window);

	static bool was_errors();


	static void set_color_buffer_clear_color(
		const RendererColor32& color);

	static void viewport_set(
		const int x,
		const int y,
		const int width,
		const int height);

	static Mat4F build_2d_projection_matrix(
		const int width,
		const int height);

	static Mat4F build_3d_translation_matrix(
		const float x,
		const float y,
		const float z);

	static Mat4F build_3d_rotataion_about_z_matrix(
		const int angle_deg);

	static Mat4F build_3d_model_matrix();

	static Mat4F build_3d_view_matrix(
		const int angle_deg,
		const Vec3F& position);

	static Mat4F build_3d_frustum(
		const float l,
		const float r,
		const float b,
		const float t,
		const float n,
		const float f);

	static Mat4F build_3d_projection_matrix(
		const int viewport_width,
		const int viewport_height,
		const int vfov_deg,
		const float near_distance,
		const float far_distance);


private:
	std::string error_message_;


	static void* resolve_symbol(
		const char* const symbol);

	template<typename T>
	static void resolve_symbol(
		const char* const name,
		T& symbol,
		bool& is_failed)
	{
		symbol = reinterpret_cast<T>(resolve_symbol(name));

		if (!symbol)
		{
			is_failed = true;
		}
	}

	static void clear_unique_symbols_1_0();

	static bool resolve_unique_symbols_1_0();


	static void clear_unique_symbols_1_1();

	static bool resolve_unique_symbols_1_1();
}; // OglRendererUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED
