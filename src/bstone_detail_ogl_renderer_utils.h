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
#include "glm/glm.hpp"
#include "bstone_detail_ogl_extension_manager.h"
#include "bstone_detail_renderer_utils.h"
#include "bstone_ogl.h"
#include "bstone_sdl_types.h"


namespace bstone
{
namespace detail
{


enum class OglRendererUtilsContextType
{
	invalid,
	none,
	core,
	compatibility,
	es,
}; // OglRendererUtilsContextType


struct OglRendererUtilsDeviceFeatures
{
	OglRendererUtilsContextType context_type_;


	// ======================================================================
	// Anisotropy.
	//

	bool anisotropy_is_available_;
	bool anisotropy_is_ext_; // EXT (true) or ARB (false).
	int anisotropy_max_value_;

	//
	// Anisotropy.
	// ======================================================================


	// ======================================================================
	// Non-power-of-two textures.
	//

	bool npot_is_available_;

	//
	// Non-power-of-two textures.
	// ======================================================================


	// ======================================================================
	// Mipmap generation.
	//

	bool mipmap_is_available_;

	//
	// Mipmap generation.
	// ======================================================================
}; // OglRendererUtilsDeviceFeatures


class OglRendererUtils
{
public:
	const std::string& get_error_message() const;


	bool load_library();

	static void unload_library();

	static void* resolve_symbol(
		const char* const symbol);


	SdlGlContextUPtr create_context(
		SdlWindowPtr sdl_window);


	bool make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContextPtr sdl_gl_context);


	bool create_window_and_context(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowUPtr& sdl_window,
		SdlGlContextUPtr& sdl_gl_context);

	bool create_probe_window_and_context(
		SdlWindowUPtr& sdl_window,
		SdlGlContextUPtr& sdl_gl_context);


	bool window_get_drawable_size(
		SdlWindowPtr sdl_window,
		int& width,
		int& height);


	static OglRendererUtilsContextType context_get_type();

	static bool context_get_version(
		int& major_version,
		int& minor_version);


	static int anisotropy_get_max_value(
		const OglRendererUtilsDeviceFeatures& ogl_device_features);

	static void anisotropy_set_value(
		const GLenum ogl_target,
		const OglRendererUtilsDeviceFeatures& ogl_device_features,
		const int anisotropy_value);

	static void anisotropy_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features,
		OglRendererUtilsDeviceFeatures& ogl_device_features);


	static void npot_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features,
		OglRendererUtilsDeviceFeatures& ogl_device_features);


	static void mipmap_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features,
		OglRendererUtilsDeviceFeatures& ogl_device_features);


	static void framebuffer_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features);

	static int framebuffer_get_min_value();

	static int framebuffer_get_max_value(
		OglExtensionManagerPtr extension_manager);


	static void clear_buffers();

	static void swap_window(
		SdlWindowPtr sdl_window);

	static bool was_errors();


	static void set_color_buffer_clear_color(
		const RendererColor32& color);

	static void scissor_enable(
		const bool is_enabled);

	static void scissor_set_box(
		const int x,
		const int y,
		const int width,
		const int height);

	static void viewport_set_rectangle(
		const int x,
		const int y,
		const int width,
		const int height);

	static void viewport_set_depth_range(
		const float min_depth,
		const float max_depth);

	static void texture_2d_set(
		const GLuint ogl_texture_name);

	static void blending_set_function(
		const RendererBlendingFactor src_factor,
		const RendererBlendingFactor dst_factor);

	static GLenum index_buffer_get_element_type_by_byte_depth(
		const int byte_depth);

	bool renderer_features_set(
		RendererDeviceFeatures& device_features);


private:
	std::string error_message_;


	static GLenum blending_get_factor(
		const RendererBlendingFactor factor);
}; // OglRendererUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED
