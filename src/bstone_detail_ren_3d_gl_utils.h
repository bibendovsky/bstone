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
// OpenGL 3D renderer utils (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_UTILS_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_UTILS_INCLUDED


#include "bstone_sdl2_types.h"

#include "bstone_detail_ren_3d_gl_api.h"
#include "bstone_detail_ren_3d_gl_device_features.h"
#include "bstone_detail_ren_3d_gl_extension_mgr.h"
#include "bstone_detail_ren_3d_utils.h"


namespace bstone
{
namespace detail
{


class Ren3dGlUtils
{
public:
	static void load_library();

	static void unload_library() noexcept;


	static void* resolve_symbol(
		const char* const symbol);


	static SdlGlContextUPtr create_context(
		SdlWindowPtr sdl_window);


	static void create_window_and_context(
		const Ren3dUtilsCreateWindowParam& param,
		SdlWindowUPtr& sdl_window,
		SdlGlContextUPtr& sdl_gl_context);


	static void get_window_drawable_size(
		SdlWindowPtr sdl_window,
		int& width,
		int& height);


	static Ren3dGlContextKind get_context_kind();


	static void probe_msaa(
		const Ren3dKind renderer_kind,
		Ren3dDeviceFeatures& device_features,
		Ren3dGlDeviceFeatures& gl_device_features);

	static int get_window_msaa_value();


	static int get_max_anisotropy_degree();

	static int clamp_anisotropy_degree(
		const int anisotropy_value,
		const Ren3dDeviceFeatures& device_features);

	static void set_anisotropy_degree(
		const GLenum gl_target,
		const Ren3dDeviceFeatures& device_features,
		const int anisotropy_value);

	static void probe_anisotropy(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dDeviceFeatures& device_features);


	static void probe_npot(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dDeviceFeatures& device_features);


	static void probe_mipmap(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dDeviceFeatures& device_features,
		Ren3dGlDeviceFeatures& gl_device_features);

	static void generate_mipmap(
		const GLenum gl_target,
		const Ren3dDeviceFeatures& device_features,
		const Ren3dGlDeviceFeatures& gl_device_features);


	static void probe_framebuffer(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dGlDeviceFeatures& gl_device_features);


	static void probe_sampler(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dDeviceFeatures& device_features);

	static void set_sampler_anisotropy(
		const GLenum gl_sampler,
		const Ren3dDeviceFeatures& device_features,
		const int anisotropy_value);


	static void probe_vao(
		Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dGlDeviceFeatures& gl_device_features);

	static void probe_max_vertex_arrays(
		Ren3dDeviceFeatures& device_features);


	static void probe_vsync(
		Ren3dDeviceFeatures& device_features);

	static bool get_vsync();

	static bool enable_vsync(
		const bool is_enabled);


	static void probe_buffer_storage(
		const Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dGlDeviceFeatures& gl_device_features);


	static void probe_dsa(
		const Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dGlDeviceFeatures& gl_device_features);


	static void probe_sso(
		const Ren3dGlExtensionMgrPtr extension_manager,
		Ren3dGlDeviceFeatures& gl_device_features);


	static void swap_window(
		SdlWindowPtr sdl_window);


	static void set_viewport_rect(
		const Ren3dViewport& viewport);

	static void set_viewport_depth_range(
		const Ren3dViewport& viewport,
		const Ren3dGlDeviceFeatures& gl_device_features);


	static void enable_scissor(
		const bool is_enabled);

	static void set_scissor_box(
		const Ren3dScissorBox& scissor_box);


	static void enable_culling(
		const bool is_enable);

	static void set_culling_face(
		const Ren3dCullingFace culling_face);

	static void set_culling_mode(
		const Ren3dCullingMode culling_mode);


	static void enable_depth_test(
		const bool is_enable);

	static void enable_depth_write(
		const bool is_enable);


	static void enable_blending(
		const bool is_enable);

	static void set_blending_func(
		const Ren3dBlendingFunc& blending_func);


	static GLenum index_buffer_get_element_type_by_byte_depth(
		const int byte_depth);

	static void set_renderer_features(
		Ren3dDeviceFeatures& device_features);

	static Ren3dDeviceInfo get_device_info();

	static std::string get_log(
		const bool is_shader,
		const GLuint gl_name);


	static GLenum get_mag_filter(
		const Ren3dFilterKind mag_filter);

	static GLenum get_min_filter(
		const Ren3dFilterKind min_filter,
		const Ren3dMipmapMode mipmap_mode);

	static GLenum get_address_mode(
		const Ren3dAddressMode address_mode);

	static GLenum get_texture_wrap_axis(
		const Ren3dTextureAxis texture_axis);


private:
	static GLenum get_blending_factor(
		const Ren3dBlendingFactor factor);

	static int get_window_max_msaa(
		const Ren3dKind renderer_kind);

	static int get_fbo_max_msaa(
		const Ren3dKind renderer_kind,
		Ren3dDeviceFeatures& device_features,
		Ren3dGlDeviceFeatures& gl_device_features);
}; // Ren3dGlUtils


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_UTILS_INCLUDED
