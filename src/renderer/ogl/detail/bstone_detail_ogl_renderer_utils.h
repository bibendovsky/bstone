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
// OpenGL Renderer utils (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED
#define BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED


#include <vector>
#include "glm/glm.hpp"
#include "bstone_sdl_types.h"
#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_extension_manager.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{
namespace detail
{


class OglRendererUtils
{
public:
	static void load_library();

	static void unload_library() noexcept;

	static void* resolve_symbol(
		const char* const symbol);


	static SdlGlContextUPtr create_context(
		SdlWindowPtr sdl_window);


	static void make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContextPtr sdl_gl_context);


	static void create_window_and_context(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowUPtr& sdl_window,
		SdlGlContextUPtr& sdl_gl_context);


	static void window_get_drawable_size(
		SdlWindowPtr sdl_window,
		int& width,
		int& height);


	static OglContextKind context_get_kind();

	static void context_get_version(
		int& major_version,
		int& minor_version);


	static void msaa_probe(
		const RendererKind renderer_kind,
		RendererDeviceFeatures& device_features,
		OglDeviceFeatures& ogl_device_features);

	static int msaa_window_get_value();


	static int anisotropy_get_max_value();

	static int anisotropy_clamp_value(
		const int anisotropy_value,
		const RendererDeviceFeatures& device_features);

	static void anisotropy_set_value(
		const GLenum ogl_target,
		const RendererDeviceFeatures& device_features,
		const int anisotropy_value);

	static void anisotropy_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features);


	static void npot_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features);


	static void mipmap_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features,
		OglDeviceFeatures& ogl_device_features);

	static void mipmap_generate(
		const GLenum ogl_target,
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);


	static void framebuffer_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features,
		OglDeviceFeatures& ogl_device_features);


	static void sampler_probe(
		OglExtensionManagerPtr extension_manager,
		RendererDeviceFeatures& device_features);

	static void sampler_set_anisotropy(
		const GLenum ogl_sampler,
		const RendererDeviceFeatures& device_features,
		const int anisotropy_value);


	static void vertex_input_vao_probe(
		OglExtensionManagerPtr extension_manager,
		OglDeviceFeatures& ogl_device_features);

	static void vertex_input_probe_max_locations(
		RendererDeviceFeatures& device_features);


	static void vsync_probe(
		RendererDeviceFeatures& device_features);

	static bool vsync_get();

	static bool vsync_set(
		const bool is_enabled);


	static void buffer_storage_probe(
		const OglExtensionManagerPtr extension_manager,
		OglDeviceFeatures& ogl_device_features);


	static void dsa_probe(
		const OglExtensionManagerPtr extension_manager,
		OglDeviceFeatures& ogl_device_features);


	static void sso_probe(
		const OglExtensionManagerPtr extension_manager,
		OglDeviceFeatures& ogl_device_features);


	static void clear_buffers();

	static void swap_window(
		SdlWindowPtr sdl_window);

	static bool was_errors();

	static void clear_errors();


	static void set_color_buffer_clear_color(
		const R8g8b8a8& color);


	static void viewport_set_rect(
		const RendererViewport& viewport);

	static void viewport_set_depth_range(
		const RendererViewport& viewport,
		const OglDeviceFeatures& ogl_device_features);


	static void scissor_enable(
		const bool is_enabled);

	static void scissor_set_box(
		const RendererScissorBox& scissor_box);


	static void culling_enable(
		const bool is_enable);

	static void culling_set_face(
		const RendererCullingFace culling_face);

	static void culling_set_mode(
		const RendererCullingMode culling_mode);


	static void depth_test_enable(
		const bool is_enable);

	static void depth_write_enable(
		const bool is_enable);


	static void blending_enable(
		const bool is_enable);

	static void blending_set_func(
		const RendererBlendingFunc& blending_func);


	static void texture_2d_bind(
		const GLuint ogl_texture_name);

	static void texture_2d_unbind();

	static GLenum index_buffer_get_element_type_by_byte_depth(
		const int byte_depth);

	static void renderer_features_set(
		RendererDeviceFeatures& device_features);

	static RendererDeviceInfo device_info_get();

	static std::string get_log(
		const bool is_shader,
		const GLuint ogl_name);


	static const glm::mat4& csc_get_texture();

	static const glm::mat4& csc_get_projection();


	static GLenum filter_get_mag(
		const RendererFilterKind mag_filter);

	static GLenum filter_get_min(
		const RendererFilterKind min_filter,
		const RendererMipmapMode mipmap_mode);

	static GLenum address_mode_get(
		const RendererAddressMode address_mode);

	static GLenum texture_wrap_get_axis(
		const RendererTextureAxis texture_axis);


private:
	static constexpr int dummy_window_min_dimension = 256;


	OglRendererUtils();


	static GLenum blending_get_factor(
		const RendererBlendingFactor factor);

	static int msaa_window_get_max(
		const RendererKind renderer_kind);

	static int msaa_fbo_get_max(
		const RendererKind renderer_kind,
		RendererDeviceFeatures& device_features,
		OglDeviceFeatures& ogl_device_features);
}; // OglRendererUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_RENDERER_UTILS_INCLUDED
