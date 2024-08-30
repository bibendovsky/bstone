/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Utils

#include <cassert>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>

#include "bstone_exception.h"
#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_utils.h"

namespace bstone {

void GlR3rUtils::create_window_and_context(
	const R3rUtilsCreateWindowParam& param,
	sys::WindowMgr& window_mgr,
	sys::WindowUPtr& window,
	sys::GlContextUPtr& gl_context)
try {
	window = R3rUtils::create_window(param, window_mgr);
	gl_context = window->make_gl_context();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rUtils::clamp_anisotropy_degree(int anisotropy_value, const R3rDeviceFeatures& device_features)
try {
	auto clamped_value = anisotropy_value;

	if (clamped_value < R3rLimits::min_anisotropy_off())
	{
		clamped_value = R3rLimits::min_anisotropy_off();
	}
	else if (clamped_value < R3rLimits::min_anisotropy_on())
	{
		clamped_value = R3rLimits::min_anisotropy_on();
	}
	else if (clamped_value > device_features.max_anisotropy_degree)
	{
		clamped_value = device_features.max_anisotropy_degree;
	}

	return clamped_value;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rUtils::get_window_max_msaa(const R3rType renderer_type, sys::WindowMgr& window_mgr)
try
{
	auto window_param = R3rUtilsCreateWindowParam{};
	window_param.renderer_type = renderer_type;
	window_param.aa_type = R3rAaType::ms;

	auto max_msaa = R3rLimits::min_aa_off();

	for (int i = R3rLimits::min_aa_on(); i <= R3rLimits::max_aa(); i *= 2)
	{
		window_param.aa_value = i;

		auto window = sys::WindowUPtr{};
		auto gl_context = sys::GlContextUPtr{};
		create_window_and_context(window_param, window_mgr, window, gl_context);

		if (gl_context->get_attributes().multisample_sample_count > 0)
		{
			max_msaa = std::max(i, max_msaa);
		}
		else
		{
			break;
		}
	}

	return max_msaa;
}
catch (const std::exception&)
{
	return R3rLimits::min_aa_off();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rUtils::get_fbo_max_msaa(
	const R3rType renderer_type,
	const sys::GlSymbolResolver& symbol_resolver,
	sys::WindowMgr& window_mgr,
	GlR3rDeviceFeatures& gl_device_features)
try
{
	auto window_param = R3rUtilsCreateWindowParam{};
	window_param.renderer_type = renderer_type;

	auto window = sys::WindowUPtr{};
	auto gl_context = sys::GlContextUPtr{};
	create_window_and_context(window_param, window_mgr, window, gl_context);

	auto extension_manager = make_gl_r3r_extension_mgr(symbol_resolver);
	extension_manager->probe(GlR3rExtensionId::essentials);

	if (!extension_manager->has(GlR3rExtensionId::essentials))
	{
		BSTONE_THROW_STATIC_SOURCE("Essential functions not available.");
	}

	GlR3rUtils::probe_framebuffer(extension_manager.get(), gl_device_features );

	if (!gl_device_features.is_framebuffer_available)
	{
		return R3rLimits::min_aa_off();
	}

	const auto gl_enum = (gl_device_features.is_framebuffer_ext ? GL_MAX_SAMPLES_EXT : GL_MAX_SAMPLES);

	auto gl_value = GLint{};

	glGetIntegerv(gl_enum, &gl_value);
	GlR3rError::check_optionally();

	return gl_value;
}
catch (const std::exception&)
{
	return R3rLimits::min_aa_off();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_msaa(
	const R3rType renderer_type,
	const sys::GlSymbolResolver& symbol_resolver,
	sys::WindowMgr& window_mgr,
	R3rDeviceFeatures& device_features,
	GlR3rDeviceFeatures& gl_device_features)
try {
	device_features.is_msaa_available = false;
	device_features.is_msaa_render_to_window = false;
	device_features.is_msaa_requires_restart = false;
	device_features.max_msaa_degree = R3rLimits::min_aa_off();

	const auto msaa_window_max = GlR3rUtils::get_window_max_msaa(renderer_type, window_mgr);

	if (msaa_window_max >= R3rLimits::min_aa_on())
	{
		device_features.is_msaa_available = true;

		if (msaa_window_max > device_features.max_msaa_degree)
		{
			device_features.max_msaa_degree = msaa_window_max;
		}
	}

	const auto msaa_fbo_max = GlR3rUtils::get_fbo_max_msaa(
		renderer_type,
		symbol_resolver,
		window_mgr,
		gl_device_features);

	if (msaa_fbo_max >= R3rLimits::min_aa_on())
	{
		device_features.is_msaa_available = true;

		if (msaa_fbo_max > device_features.max_msaa_degree)
		{
			device_features.max_msaa_degree = msaa_fbo_max;
		}
	}

	if (msaa_window_max >= R3rLimits::min_aa_on() && msaa_fbo_max < R3rLimits::min_aa_on())
	{
		device_features.is_msaa_render_to_window = true;
		device_features.is_msaa_requires_restart = true;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rUtils::get_window_msaa_value(const sys::GlContextAttributes& gl_attributes) noexcept
{
	if (gl_attributes.multisample_buffer_count <= 0)
	{
		return 0;
	}

	return gl_attributes.multisample_sample_count;
}

int GlR3rUtils::get_max_anisotropy_degree()
try {
	auto gl_max_value = GLfloat{};

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gl_max_value);
	GlR3rError::check_optionally();

	if (gl_max_value <= static_cast<GLfloat>(R3rLimits::min_anisotropy_off()))
	{
		return R3rLimits::min_anisotropy_off();
	}

	return static_cast<int>(gl_max_value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_anisotropy_degree(
	const GLenum gl_target,
	const R3rDeviceFeatures& device_features,
	const int anisotropy_value)
try {
	if (!device_features.is_anisotropy_available)
	{
		return;
	}

	const auto clamped_value = clamp_anisotropy_degree(anisotropy_value, device_features);

	const auto gl_value = static_cast<GLfloat>(clamped_value);

	glTexParameterf(gl_target, GL_TEXTURE_MAX_ANISOTROPY, gl_value);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_anisotropy(
	GlR3rExtensionMgr* extension_manager,
	R3rDeviceFeatures& device_features)
try {
	device_features.is_anisotropy_available = false;
	device_features.max_anisotropy_degree = R3rLimits::min_anisotropy_off();

#ifndef BSTONE_R3R_TEST_NO_ANISOTROPY
	if (!device_features.is_anisotropy_available)
	{
		extension_manager->probe(GlR3rExtensionId::arb_texture_filter_anisotropic);

		device_features.is_anisotropy_available =
			extension_manager->has(GlR3rExtensionId::arb_texture_filter_anisotropic);
	}

	if (!device_features.is_anisotropy_available)
	{
		extension_manager->probe(GlR3rExtensionId::ext_texture_filter_anisotropic);

		device_features.is_anisotropy_available =
			extension_manager->has(GlR3rExtensionId::ext_texture_filter_anisotropic);
	}

	if (device_features.is_anisotropy_available)
	{
		device_features.max_anisotropy_degree = get_max_anisotropy_degree();
	}
#endif // BSTONE_R3R_TEST_NO_ANISOTROPY
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_npot(
	GlR3rExtensionMgr* extension_manager,
	R3rDeviceFeatures& device_features)
try {
	device_features.is_npot_available = false;

#ifndef BSTONE_R3R_TEST_POT_ONLY
	if (!device_features.is_npot_available)
	{
		extension_manager->probe(GlR3rExtensionId::arb_texture_non_power_of_two);

		if (extension_manager->has(GlR3rExtensionId::arb_texture_non_power_of_two))
		{
			device_features.is_npot_available = true;
		}
	}

	if (!device_features.is_npot_available)
	{
		extension_manager->probe(GlR3rExtensionId::oes_texture_npot);

		if (extension_manager->has(GlR3rExtensionId::oes_texture_npot))
		{
			device_features.is_npot_available = true;
		}
	}
#endif // BSTONE_R3R_TEST_POT_ONLY
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_mipmap(
	GlR3rExtensionMgr* extension_manager,
	R3rDeviceFeatures& device_features,
	GlR3rDeviceFeatures& gl_device_features)
try {
	device_features.is_mipmap_available = false;
	gl_device_features.is_mipmap_ext = false;

#ifndef BSTONE_R3R_TEST_SW_MIPMAP
	if (gl_device_features.context_profile == sys::GlContextProfile::es)
	{
		device_features.is_mipmap_available = true;
	}

	if (!device_features.is_mipmap_available)
	{
		extension_manager->probe(GlR3rExtensionId::arb_framebuffer_object);

		if (extension_manager->has(GlR3rExtensionId::arb_framebuffer_object))
		{
			device_features.is_mipmap_available = true;
		}
	}

	if (!device_features.is_mipmap_available)
	{
		extension_manager->probe(GlR3rExtensionId::ext_framebuffer_object);

		if (extension_manager->has(GlR3rExtensionId::ext_framebuffer_object))
		{
			device_features.is_mipmap_available = true;
			gl_device_features.is_mipmap_ext = true;
		}
	}
#endif // BSTONE_R3R_TEST_SW_MIPMAP
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::generate_mipmap(
	const GLenum gl_target,
	const R3rDeviceFeatures& device_features,
	const GlR3rDeviceFeatures& gl_device_features)
try {
	if (!device_features.is_mipmap_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Not available.");
	}

	switch (gl_target)
	{
		case GL_TEXTURE_2D: break;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported texture target.");
	}

	const auto gl_function = (gl_device_features.is_mipmap_ext ? glGenerateMipmapEXT : glGenerateMipmap);

	gl_function(gl_target);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_framebuffer(
	GlR3rExtensionMgr* extension_manager,
	GlR3rDeviceFeatures& gl_device_features)
try {
	gl_device_features.is_framebuffer_available = false;
	gl_device_features.is_framebuffer_ext = false;

#ifndef BSTONE_R3R_TEST_DEFAULT_FRAMEBUFFER
	if (!gl_device_features.is_framebuffer_available)
	{
		extension_manager->probe(GlR3rExtensionId::arb_framebuffer_object);

		if (extension_manager->has(GlR3rExtensionId::arb_framebuffer_object))
		{
			gl_device_features.is_framebuffer_available = true;
		}
	}

	if (!gl_device_features.is_framebuffer_available)
	{
		extension_manager->probe(GlR3rExtensionId::ext_framebuffer_blit);
		extension_manager->probe(GlR3rExtensionId::ext_framebuffer_multisample);
		extension_manager->probe(GlR3rExtensionId::ext_framebuffer_object);
		extension_manager->probe(GlR3rExtensionId::ext_packed_depth_stencil);

		if (extension_manager->has(GlR3rExtensionId::ext_framebuffer_blit) &&
			extension_manager->has(GlR3rExtensionId::ext_framebuffer_multisample) &&
			extension_manager->has(GlR3rExtensionId::ext_framebuffer_object) &&
			extension_manager->has(GlR3rExtensionId::ext_packed_depth_stencil))
		{
			gl_device_features.is_framebuffer_available = true;
			gl_device_features.is_framebuffer_ext = true;
		}
	}
#endif // BSTONE_R3R_TEST_DEFAULT_FRAMEBUFFER
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_sampler(
	GlR3rExtensionMgr* extension_manager,
	R3rDeviceFeatures& device_features)
try {
	device_features.is_sampler_available = false;

#ifndef BSTONE_R3R_TEST_SW_SAMPLER
	extension_manager->probe(GlR3rExtensionId::arb_sampler_objects);

	if (extension_manager->has(GlR3rExtensionId::arb_sampler_objects))
	{
		device_features.is_sampler_available = true;
	}
#endif // BSTONE_R3R_TEST_SW_SAMPLER
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_sampler_anisotropy(
	const GLenum gl_sampler,
	const R3rDeviceFeatures& device_features,
	const int anisotropy_value)
try {
	if (!device_features.is_sampler_available ||
		!device_features.is_anisotropy_available)
	{
		return;
	}

	const auto clamped_value = clamp_anisotropy_degree(anisotropy_value, device_features);

	const auto gl_value = static_cast<GLfloat>(clamped_value);

	glSamplerParameterf(gl_sampler, GL_TEXTURE_MAX_ANISOTROPY, gl_value);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_vao(
	GlR3rExtensionMgr* extension_manager,
	GlR3rDeviceFeatures& gl_device_features)
try {
	gl_device_features.is_vao_available = false;

	if (!gl_device_features.is_vao_available)
	{
		const auto& gl_version = extension_manager->get_gl_version();

		if (gl_device_features.context_profile == sys::GlContextProfile::core ||
			(gl_version.is_es && gl_version.major >= 3))
		{
			gl_device_features.is_vao_available = true;
		}
	}

#ifndef BSTONE_R3R_TEST_GL_NO_VAO
	if (!gl_device_features.is_vao_available)
	{
		extension_manager->probe(GlR3rExtensionId::arb_vertex_array_object);

		if (extension_manager->has(GlR3rExtensionId::arb_vertex_array_object))
		{
			gl_device_features.is_vao_available = true;
		}
	}
#endif // BSTONE_R3R_TEST_GL_NO_VAO
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_max_vertex_arrays(R3rDeviceFeatures& device_features)
try {
	auto gl_count = GLint{};

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_count);
	GlR3rError::check_optionally();

	device_features.max_vertex_input_locations = 0;

	if (gl_count > 0)
	{
		device_features.max_vertex_input_locations = gl_count;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_vsync(
	sys::GlCurrentContext& gl_current_context,
	R3rDeviceFeatures& device_features)
try {
	device_features.is_vsync_available = false;
	device_features.is_vsync_requires_restart = false;

#ifndef BSTONE_R3R_TEST_NO_SWAP_INTERVAL
	try
	{
		gl_current_context.set_swap_interval(sys::SwapIntervalType::none);
		gl_current_context.set_swap_interval(sys::SwapIntervalType::standard);
		device_features.is_vsync_available = true;
	}
	catch (const std::exception&)
	{
	}
#endif // BSTONE_R3R_TEST_NO_SWAP_INTERVAL
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool GlR3rUtils::get_vsync(sys::GlCurrentContext& gl_current_context)
try {
	return gl_current_context.get_swap_interval() != sys::SwapIntervalType::none;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_buffer_storage(
	GlR3rExtensionMgr* extension_manager,
	GlR3rDeviceFeatures& gl_device_features)
try {
	gl_device_features.is_buffer_storage_available = false;

#ifndef BSTONE_R3R_TEST_GL_NO_BUFFER_STORAGE
	extension_manager->probe(GlR3rExtensionId::arb_buffer_storage);

	if (extension_manager->has(GlR3rExtensionId::arb_buffer_storage))
	{
		gl_device_features.is_buffer_storage_available = true;
	}
#endif // BSTONE_R3R_TEST_GL_NO_BUFFER_STORAGE
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_dsa(
	GlR3rExtensionMgr* extension_manager,
	GlR3rDeviceFeatures& gl_device_features)
try {
	gl_device_features.is_dsa_available = false;

#ifndef BSTONE_R3R_TEST_GL_NO_DSA
	extension_manager->probe(GlR3rExtensionId::arb_direct_state_access);

	if (extension_manager->has(GlR3rExtensionId::arb_direct_state_access))
	{
		gl_device_features.is_dsa_available = true;
	}
#endif // !BSTONE_R3R_TEST_GL_NO_DSA
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::probe_sso(
	GlR3rExtensionMgr* extension_manager,
	GlR3rDeviceFeatures& gl_device_features)
try {
	gl_device_features.is_sso_available = false;

#if !defined( BSTONE_R3R_TEST_GL_NO_SSO)
	extension_manager->probe(GlR3rExtensionId::arb_separate_shader_objects);

	if (extension_manager->has(GlR3rExtensionId::arb_separate_shader_objects))
	{
		gl_device_features.is_sso_available = true;
	}
#endif // BSTONE_R3R_TEST_GL_NO_SSO
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::enable_scissor(bool is_enabled)
try {
	(is_enabled ? glEnable : glDisable)(GL_SCISSOR_TEST);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_scissor_box(const R3rScissorBox& scissor_box)
try {
	if (scissor_box.x < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative offset by X.");
	}

	if (scissor_box.y < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative offset by Y.");
	}

	if (scissor_box.width < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative width.");
	}

	if (scissor_box.height < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative height.");
	}

	glScissor(
		scissor_box.x,
		scissor_box.y,
		scissor_box.width,
		scissor_box.height);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_viewport_rect(const R3rViewport& viewport)
try {
	if (viewport.x < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative offset by X.");
	}

	if (viewport.y < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative offset by Y.");
	}

	if (viewport.width < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative width.");
	}

	if (viewport.height < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative height.");
	}

	glViewport(
		viewport.x,
		viewport.y,
		viewport.width,
		viewport.height);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_viewport_depth_range(
	const R3rViewport& viewport,
	const GlR3rDeviceFeatures& gl_device_features)
try {
	if (viewport.min_depth < 0.0F || viewport.min_depth > 1.0F)
	{
		BSTONE_THROW_STATIC_SOURCE("Minimum depth out of range.");
	}

	if (viewport.max_depth < 0.0F || viewport.max_depth > 1.0F)
	{
		BSTONE_THROW_STATIC_SOURCE("Maximum depth out of range.");
	}

	const auto is_es = (gl_device_features.context_profile == sys::GlContextProfile::es);

	if (is_es)
	{
		glDepthRangef(viewport.min_depth, viewport.max_depth);
		GlR3rError::check_optionally();
	}
	else
	{
		glDepthRange(viewport.min_depth, viewport.max_depth);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::enable_culling(bool is_enable)
try {
	(is_enable ? glEnable : glDisable)(GL_CULL_FACE);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_culling_face(R3rCullingFace culling_face)
try {
	auto gl_culling_face = GLenum{};

	switch (culling_face)
	{
		case R3rCullingFace::clockwise:
			gl_culling_face = GL_CW;
			break;

		case R3rCullingFace::counter_clockwise:
			gl_culling_face = GL_CCW;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported front face.");
	}

	glFrontFace(gl_culling_face);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_culling_mode(R3rCullingMode culling_mode)
try {
	auto gl_culling_mode = GLenum{};

	switch (culling_mode)
	{
		case R3rCullingMode::back:
			gl_culling_mode = GL_BACK;
			break;

		case R3rCullingMode::front:
			gl_culling_mode = GL_FRONT;
			break;

		case R3rCullingMode::both:
			gl_culling_mode = GL_FRONT_AND_BACK;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported culling mode.");
	}

	glCullFace(gl_culling_mode);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::enable_depth_test(bool is_enable)
try {
	(is_enable ? glEnable : glDisable)(GL_DEPTH_TEST);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::enable_depth_write(bool is_enable)
try {
	glDepthMask(is_enable);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::enable_blending(bool is_enable)
try {
	(is_enable ? glEnable : glDisable)(GL_BLEND);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_blending_func(const R3rBlendingFunc& blending_func)
try {
	auto gl_src_factor = get_blending_factor(blending_func.src_factor);
	auto gl_dst_factor = get_blending_factor(blending_func.dst_factor);
	glBlendFunc(gl_src_factor, gl_dst_factor);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::index_buffer_get_element_type_by_byte_depth(int byte_depth)
try {
	switch (byte_depth)
	{
		case 1: return GL_UNSIGNED_BYTE;
		case 2: return GL_UNSIGNED_SHORT;
		case 4: return GL_UNSIGNED_INT;
		default: BSTONE_THROW_STATIC_SOURCE("Invalid byte depth.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rUtils::set_renderer_features(R3rDeviceFeatures& device_features)
try {
	// Max texture dimension.
	//
	auto gl_texture_dimension = GLint{};

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_texture_dimension);

	if (gl_texture_dimension == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get maximum texture dimension.");
	}

	// Max viewport dimensions.
	//
	using GlViewportDimensions = std::array<GLint, 2>;
	auto gl_viewport_dimensions = GlViewportDimensions{};

	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, gl_viewport_dimensions.data());

	if (gl_viewport_dimensions[0] == 0 || gl_viewport_dimensions[1] == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get viewport dimensions.");
	}

	// Set the values.
	//
	device_features.max_texture_dimension = gl_texture_dimension;

	device_features.max_viewport_width = gl_viewport_dimensions[0];
	device_features.max_viewport_height = gl_viewport_dimensions[1];
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rDeviceInfo GlR3rUtils::get_device_info()
try {
	auto result = R3rDeviceInfo{};

	// Name.
	//
	const auto gl_name = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

	GlR3rError::check_optionally();

	if (gl_name != nullptr)
	{
		result.name = gl_name;
	}

	// Vendor.
	//
	const auto gl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

	GlR3rError::check_optionally();

	if (gl_vendor != nullptr)
	{
		result.vendor = gl_vendor;
	}

	// Version.
	//
	const auto gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

	GlR3rError::check_optionally();

	if (gl_version != nullptr)
	{
		result.version = gl_version;
	}

	// Result.
	//
	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string GlR3rUtils::get_log(bool is_shader, GLuint gl_name)
try {
	const auto gl_info_function = (is_shader ? glGetShaderInfoLog : glGetProgramInfoLog);
	const auto gl_size_function = (is_shader ? glGetShaderiv : glGetProgramiv);

	auto size = GLint{};
	gl_size_function(gl_name, GL_INFO_LOG_LENGTH, &size);
	GlR3rError::check_optionally();

	auto result = std::string{};

	if (size > 0)
	{
		result.resize(size);
		auto info_size = GLsizei{};
		gl_info_function(gl_name, size, &info_size, &result[0]);
		GlR3rError::check_optionally();

		if (info_size <= 0)
		{
			result.clear();
		}
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::get_mag_filter(R3rFilterType mag_filter)
try {
	switch (mag_filter)
	{
		case R3rFilterType::nearest: return GL_NEAREST;
		case R3rFilterType::linear: return GL_LINEAR;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported magnification filter.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::get_min_filter(R3rFilterType min_filter, R3rMipmapMode mipmap_mode)
try {
	switch (mipmap_mode)
	{
		case R3rMipmapMode::none:
			switch (min_filter)
			{
				case R3rFilterType::nearest: return GL_NEAREST;
				case R3rFilterType::linear: return GL_LINEAR;
				default: BSTONE_THROW_STATIC_SOURCE("Unsupported minification filter.");
			}

			break;

		case R3rMipmapMode::nearest:
			switch (min_filter)
			{
				case R3rFilterType::nearest: return GL_NEAREST_MIPMAP_NEAREST;
				case R3rFilterType::linear: return GL_LINEAR_MIPMAP_NEAREST;
				default: BSTONE_THROW_STATIC_SOURCE("Unsupported minification mipmap filter.");
			}

			break;

		case R3rMipmapMode::linear:
			switch (min_filter)
			{
				case R3rFilterType::nearest: return GL_NEAREST_MIPMAP_LINEAR;
				case R3rFilterType::linear: return GL_LINEAR_MIPMAP_LINEAR;
				default: BSTONE_THROW_STATIC_SOURCE("Unsupported minification mipmap filter.");
			}

			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported mipmap mode.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::get_address_mode(R3rAddressMode address_mode)
try {
	switch (address_mode)
	{
		case R3rAddressMode::clamp: return GL_CLAMP_TO_EDGE;
		case R3rAddressMode::repeat: return GL_REPEAT;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported address mode.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::get_texture_wrap_axis(R3rTextureAxis texture_axis)
try {
	switch (texture_axis)
	{
		case R3rTextureAxis::u: return GL_TEXTURE_WRAP_S;
		case R3rTextureAxis::v: return GL_TEXTURE_WRAP_T;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported texture axis.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rUtils::get_blending_factor(R3rBlendingFactor factor)
try {
	switch (factor)
	{
		case R3rBlendingFactor::zero: return GL_ZERO;
		case R3rBlendingFactor::one: return GL_ONE;
		case R3rBlendingFactor::src_color: return GL_SRC_COLOR;
		case R3rBlendingFactor::src_alpha: return GL_SRC_ALPHA;
		case R3rBlendingFactor::one_minus_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
		default: BSTONE_THROW_STATIC_SOURCE("Invalid blending factor.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
