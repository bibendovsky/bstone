/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Utils

#ifndef BSTONE_GL_R3R_UTILS_INCLUDED
#define BSTONE_GL_R3R_UTILS_INCLUDED

#include "bstone_sys_gl_context.h"
#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_gl_symbol_resolver.h"
#include "bstone_sys_window_mgr.h"

#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_device_features.h"
#include "bstone_gl_r3r_extension_mgr.h"
#include "bstone_r3r_utils.h"

namespace bstone {

class GlR3rUtils
{
public:
	static void create_window_and_context(
		const R3rUtilsCreateWindowParam& param,
		sys::WindowMgr& window_mgr,
		sys::WindowUPtr& window,
		sys::GlContextUPtr& gl_context);

	static void probe_msaa(
		const R3rType renderer_type,
		const sys::GlSymbolResolver& symbol_resolver,
		sys::WindowMgr& window_mgr,
		R3rDeviceFeatures& device_features,
		GlR3rDeviceFeatures& gl_device_features);

	static int get_window_msaa_value(const sys::GlContextAttributes& gl_attributes) noexcept;

	static int get_max_anisotropy_degree();

	static int clamp_anisotropy_degree(int anisotropy_value, const R3rDeviceFeatures& device_features);

	static void set_anisotropy_degree(
		const GLenum gl_target,
		const R3rDeviceFeatures& device_features,
		const int anisotropy_value);

	static void probe_anisotropy(
		GlR3rExtensionMgr* extension_manager,
		R3rDeviceFeatures& device_features);

	static void probe_npot(
		GlR3rExtensionMgr* extension_manager,
		R3rDeviceFeatures& device_features);

	static void probe_mipmap(
		GlR3rExtensionMgr* extension_manager,
		R3rDeviceFeatures& device_features,
		GlR3rDeviceFeatures& gl_device_features);

	static void generate_mipmap(
		const GLenum gl_target,
		const R3rDeviceFeatures& device_features,
		const GlR3rDeviceFeatures& gl_device_features);

	static void probe_framebuffer(
		GlR3rExtensionMgr* extension_manager,
		GlR3rDeviceFeatures& gl_device_features);

	static void probe_sampler(
		GlR3rExtensionMgr* extension_manager,
		R3rDeviceFeatures& device_features);

	static void set_sampler_anisotropy(
		const GLenum gl_sampler,
		const R3rDeviceFeatures& device_features,
		const int anisotropy_value);

	static void probe_vao(
		GlR3rExtensionMgr* extension_manager,
		GlR3rDeviceFeatures& gl_device_features);

	static void probe_max_vertex_arrays(R3rDeviceFeatures& device_features);

	static void probe_vsync(sys::GlCurrentContext& gl_current_context, R3rDeviceFeatures& device_features);
	static bool get_vsync(sys::GlCurrentContext& gl_current_context);

	static void probe_buffer_storage(
		GlR3rExtensionMgr* extension_manager,
		GlR3rDeviceFeatures& gl_device_features);

	static void probe_dsa(
		GlR3rExtensionMgr* extension_manager,
		GlR3rDeviceFeatures& gl_device_features);

	static void probe_sso(
		GlR3rExtensionMgr* extension_manager,
		GlR3rDeviceFeatures& gl_device_features);

	static void set_viewport_rect(
		const R3rViewport& viewport);

	static void set_viewport_depth_range(
		const R3rViewport& viewport,
		const GlR3rDeviceFeatures& gl_device_features);

	static void enable_scissor(bool is_enabled);
	static void set_scissor_box(const R3rScissorBox& scissor_box);

	static void enable_culling(bool is_enable);
	static void set_culling_face(R3rCullingFace culling_face);
	static void set_culling_mode(R3rCullingMode culling_mode);

	static void enable_depth_test(bool is_enable);
	static void enable_depth_write(bool is_enable);

	static void enable_blending(bool is_enable);
	static void set_blending_func(const R3rBlendingFunc& blending_func);

	static GLenum index_buffer_get_element_type_by_byte_depth(int byte_depth);

	static void set_renderer_features(R3rDeviceFeatures& device_features);

	static R3rDeviceInfo get_device_info();

	static std::string get_log(bool is_shader, GLuint gl_name);

	static GLenum get_mag_filter(R3rFilterType mag_filter);
	static GLenum get_min_filter(R3rFilterType min_filter, R3rMipmapMode mipmap_mode);

	static GLenum get_address_mode(R3rAddressMode address_mode);
	static GLenum get_texture_wrap_axis(R3rTextureAxis texture_axis);


private:
	static GLenum get_blending_factor(R3rBlendingFactor factor);

	static int get_window_max_msaa(R3rType renderer_type, sys::WindowMgr& window_mgr);

	static int get_fbo_max_msaa(
		R3rType renderer_type,
		const sys::GlSymbolResolver& symbol_resolver,
		sys::WindowMgr& window_mgr,
		GlR3rDeviceFeatures& gl_device_features);
};

} // namespace bstone

#endif // BSTONE_GL_R3R_UTILS_INCLUDED
