/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// OpenGL 3D renderer utils.
//
// !!! Internal usage only. !!!
//

#include <cassert>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>

#include "bstone_exception.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"

namespace bstone {
namespace detail {

void Ren3dGlUtils::create_window_and_context(
	const Ren3dUtilsCreateWindowParam& param,
	sys::WindowMgr& window_mgr,
	sys::WindowUPtr& window,
	sys::GlContextUPtr& gl_context)
try
{
	window = Ren3dUtils::create_window(param, window_mgr);
	gl_context = window->make_gl_context();
}
BSTONE_STATIC_THROW_NESTED_FUNC

Ren3dGlContextKind Ren3dGlUtils::get_context_kind(const sys::GlContextAttributes& gl_attributes) noexcept
{
	switch (gl_attributes.profile)
	{
		case sys::GlContextProfile::none: return Ren3dGlContextKind::none;
		case sys::GlContextProfile::core: return Ren3dGlContextKind::core;
		case sys::GlContextProfile::compatibility: return Ren3dGlContextKind::compatibility;
		case sys::GlContextProfile::es: return Ren3dGlContextKind::es;
		default: return Ren3dGlContextKind::invalid;
	}
}

int Ren3dGlUtils::clamp_anisotropy_degree(int anisotropy_value, const Ren3dDeviceFeatures& device_features)
try
{
	auto clamped_value = anisotropy_value;

	if (clamped_value < Ren3dLimits::min_anisotropy_off)
	{
		clamped_value = Ren3dLimits::min_anisotropy_off;
	}
	else if (clamped_value < Ren3dLimits::min_anisotropy_on)
	{
		clamped_value = Ren3dLimits::min_anisotropy_on;
	}
	else if (clamped_value > device_features.max_anisotropy_degree_)
	{
		clamped_value = device_features.max_anisotropy_degree_;
	}

	return clamped_value;
}
BSTONE_STATIC_THROW_NESTED_FUNC

int Ren3dGlUtils::get_window_max_msaa(const Ren3dKind renderer_kind, sys::WindowMgr& window_mgr)
try
{
	auto window_param = Ren3dUtilsCreateWindowParam{};
	window_param.renderer_kind_ = renderer_kind;
	window_param.window_.is_native_ = false;
	window_param.window_.is_borderless_ = true;
	window_param.window_.rect_2d_.extent_.width_ = 1;
	window_param.window_.rect_2d_.extent_.height_ = 1;
	window_param.aa_kind_ = Ren3dAaKind::ms;

	auto max_msaa = Ren3dLimits::min_aa_off;

	for (int i = Ren3dLimits::min_aa_on; i <= Ren3dLimits::max_aa; i *= 2)
	{
		window_param.aa_value_ = i;

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
catch (const Exception&)
{
	return Ren3dLimits::min_aa_off;
}
BSTONE_STATIC_THROW_NESTED_FUNC

int Ren3dGlUtils::get_fbo_max_msaa(
	const Ren3dKind renderer_kind,
	sys::GlMgr& gl_mgr,
	sys::WindowMgr& window_mgr,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	auto window_param = Ren3dUtilsCreateWindowParam{};
	window_param.renderer_kind_ = renderer_kind;
	window_param.window_.is_native_ = false;
	window_param.window_.is_borderless_ = true;
	window_param.window_.rect_2d_.extent_.width_ = 1;
	window_param.window_.rect_2d_.extent_.height_ = 1;

	auto window = sys::WindowUPtr{};
	auto gl_context = sys::GlContextUPtr{};
	create_window_and_context(window_param, window_mgr, window, gl_context);

	auto extension_manager = Ren3dGlExtensionMgrFactory::create(gl_mgr);
	extension_manager->probe(Ren3dGlExtensionId::essentials);

	if (!extension_manager->has(Ren3dGlExtensionId::essentials))
	{
		BSTONE_STATIC_THROW("Essential functions not available.");
	}

	Ren3dGlUtils::probe_framebuffer(extension_manager.get(), gl_device_features );

	if (!gl_device_features.is_framebuffer_available_)
	{
		return Ren3dLimits::min_aa_off;
	}

	const auto gl_enum = (gl_device_features.is_framebuffer_ext_ ? GL_MAX_SAMPLES_EXT : GL_MAX_SAMPLES);

	auto gl_value = GLint{};

	glGetIntegerv(gl_enum, &gl_value);
	Ren3dGlError::ensure_debug();

	return gl_value;
}
catch (const Exception&)
{
	return Ren3dLimits::min_aa_off;
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_msaa(
	const Ren3dKind renderer_kind,
	sys::GlMgr& gl_mgr,
	sys::WindowMgr& window_mgr,
	Ren3dDeviceFeatures& device_features,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	device_features.is_msaa_available_ = false;
	device_features.is_msaa_render_to_window_ = false;
	device_features.is_msaa_requires_restart_ = false;
	device_features.max_msaa_degree_ = Ren3dLimits::min_aa_off;

	const auto msaa_window_max = Ren3dGlUtils::get_window_max_msaa(renderer_kind, window_mgr);

	if (msaa_window_max >= Ren3dLimits::min_aa_on)
	{
		device_features.is_msaa_available_ = true;

		if (msaa_window_max > device_features.max_msaa_degree_)
		{
			device_features.max_msaa_degree_ = msaa_window_max;
		}
	}

	const auto msaa_fbo_max = Ren3dGlUtils::get_fbo_max_msaa(
		renderer_kind,
		gl_mgr,
		window_mgr,
		gl_device_features);

	if (msaa_fbo_max >= Ren3dLimits::min_aa_on)
	{
		device_features.is_msaa_available_ = true;

		if (msaa_fbo_max > device_features.max_msaa_degree_)
		{
			device_features.max_msaa_degree_ = msaa_fbo_max;
		}
	}

	if (msaa_window_max >= Ren3dLimits::min_aa_on && msaa_fbo_max < Ren3dLimits::min_aa_on)
	{
		device_features.is_msaa_render_to_window_ = true;
		device_features.is_msaa_requires_restart_ = true;
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

int Ren3dGlUtils::get_window_msaa_value(const sys::GlContextAttributes& gl_attributes) noexcept
{
	if (gl_attributes.multisample_buffer_count <= 0)
	{
		return 0;
	}

	return gl_attributes.multisample_sample_count;
}

int Ren3dGlUtils::get_max_anisotropy_degree()
try
{
	auto gl_max_value = GLfloat{};

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &gl_max_value);
	Ren3dGlError::ensure_debug();

	if (gl_max_value <= static_cast<GLfloat>(Ren3dLimits::min_anisotropy_off))
	{
		return Ren3dLimits::min_anisotropy_off;
	}

	return static_cast<int>(gl_max_value);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_anisotropy_degree(
	const GLenum gl_target,
	const Ren3dDeviceFeatures& device_features,
	const int anisotropy_value)
try
{
	if (!device_features.is_anisotropy_available_)
	{
		return;
	}

	const auto clamped_value = clamp_anisotropy_degree(anisotropy_value, device_features);

	const auto gl_value = static_cast<GLfloat>(clamped_value);

	glTexParameterf(gl_target, GL_TEXTURE_MAX_ANISOTROPY, gl_value);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_anisotropy(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dDeviceFeatures& device_features)
try
{
	device_features.is_anisotropy_available_ = false;
	device_features.max_anisotropy_degree_ = Ren3dLimits::min_anisotropy_off;

#if !defined(BSTONE_REN_3D_TEST_NO_ANISOTROPY)
	if (!device_features.is_anisotropy_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::arb_texture_filter_anisotropic);

		device_features.is_anisotropy_available_ =
			extension_manager->has(Ren3dGlExtensionId::arb_texture_filter_anisotropic);
	}

	if (!device_features.is_anisotropy_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::ext_texture_filter_anisotropic);

		device_features.is_anisotropy_available_ =
			extension_manager->has(Ren3dGlExtensionId::ext_texture_filter_anisotropic);
	}

	if (device_features.is_anisotropy_available_)
	{
		device_features.max_anisotropy_degree_ = get_max_anisotropy_degree();
	}
#endif // BSTONE_REN_3D_TEST_NO_ANISOTROPY
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_npot(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dDeviceFeatures& device_features)
try
{
	device_features.is_npot_available_ = false;

#if !defined(BSTONE_REN_3D_TEST_POT_ONLY)
	if (!device_features.is_npot_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::arb_texture_non_power_of_two);

		if (extension_manager->has(Ren3dGlExtensionId::arb_texture_non_power_of_two))
		{
			device_features.is_npot_available_ = true;
		}
	}

	if (!device_features.is_npot_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::oes_texture_npot);

		if (extension_manager->has(Ren3dGlExtensionId::oes_texture_npot))
		{
			device_features.is_npot_available_ = true;
		}
	}
#endif // BSTONE_REN_3D_TEST_POT_ONLY
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_mipmap(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dDeviceFeatures& device_features,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	device_features.is_mipmap_available_ = false;
	gl_device_features.is_mipmap_ext_ = false;

#if !defined(BSTONE_REN_3D_TEST_SW_MIPMAP)
	if (gl_device_features.context_kind_ == Ren3dGlContextKind::es)
	{
		device_features.is_mipmap_available_ = true;
	}

	if (!device_features.is_mipmap_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::arb_framebuffer_object);

		if (extension_manager->has(Ren3dGlExtensionId::arb_framebuffer_object))
		{
			device_features.is_mipmap_available_ = true;
		}
	}

	if (!device_features.is_mipmap_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::ext_framebuffer_object);

		if (extension_manager->has(Ren3dGlExtensionId::ext_framebuffer_object))
		{
			device_features.is_mipmap_available_ = true;
			gl_device_features.is_mipmap_ext_ = true;
		}
	}
#endif // BSTONE_REN_3D_TEST_SW_MIPMAP
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::generate_mipmap(
	const GLenum gl_target,
	const Ren3dDeviceFeatures& device_features,
	const Ren3dGlDeviceFeatures& gl_device_features)
try
{
	if (!device_features.is_mipmap_available_)
	{
		BSTONE_STATIC_THROW("Not available.");
	}

	switch (gl_target)
	{
		case GL_TEXTURE_2D:
			break;

		default:
			BSTONE_STATIC_THROW("Unsupported texture target.");
	}

	const auto gl_function = (gl_device_features.is_mipmap_ext_ ? glGenerateMipmapEXT : glGenerateMipmap);

	gl_function(gl_target);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_framebuffer(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	gl_device_features.is_framebuffer_available_ = false;
	gl_device_features.is_framebuffer_ext_ = false;

#if !defined(BSTONE_RENDERER_TEST_3D_DEFAULT_FRAMEBUFFER)
	if (!gl_device_features.is_framebuffer_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::arb_framebuffer_object);

		if (extension_manager->has(Ren3dGlExtensionId::arb_framebuffer_object))
		{
			gl_device_features.is_framebuffer_available_ = true;
		}
	}

	if (!gl_device_features.is_framebuffer_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::ext_framebuffer_blit);
		extension_manager->probe(Ren3dGlExtensionId::ext_framebuffer_multisample);
		extension_manager->probe(Ren3dGlExtensionId::ext_framebuffer_object);
		extension_manager->probe(Ren3dGlExtensionId::ext_packed_depth_stencil);

		if (extension_manager->has(Ren3dGlExtensionId::ext_framebuffer_blit) &&
			extension_manager->has(Ren3dGlExtensionId::ext_framebuffer_multisample) &&
			extension_manager->has(Ren3dGlExtensionId::ext_framebuffer_object) &&
			extension_manager->has(Ren3dGlExtensionId::ext_packed_depth_stencil))
		{
			gl_device_features.is_framebuffer_available_ = true;
			gl_device_features.is_framebuffer_ext_ = true;
		}
	}
#endif // BSTONE_RENDERER_TEST_3D_DEFAULT_FRAMEBUFFER
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_sampler(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dDeviceFeatures& device_features)
try
{
	device_features.is_sampler_available_ = false;

#if !defined(BSTONE_REN_3D_TEST_SW_SAMPLER)
	extension_manager->probe(Ren3dGlExtensionId::arb_sampler_objects);

	if (extension_manager->has(Ren3dGlExtensionId::arb_sampler_objects))
	{
		device_features.is_sampler_available_ = true;
	}
#endif // BSTONE_REN_3D_TEST_SW_SAMPLER
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_sampler_anisotropy(
	const GLenum gl_sampler,
	const Ren3dDeviceFeatures& device_features,
	const int anisotropy_value)
try
{
	if (!device_features.is_sampler_available_ ||
		!device_features.is_anisotropy_available_)
	{
		return;
	}

	const auto clamped_value = clamp_anisotropy_degree(anisotropy_value, device_features);

	const auto gl_value = static_cast<GLfloat>(clamped_value);

	glSamplerParameterf(gl_sampler, GL_TEXTURE_MAX_ANISOTROPY, gl_value);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_vao(
	Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	gl_device_features.is_vao_available_ = false;

	if (!gl_device_features.is_vao_available_)
	{
		const auto& gl_version = extension_manager->get_gl_version();

		if (gl_device_features.context_kind_ == Ren3dGlContextKind::core ||
			(gl_version.is_es_ && gl_version.major_ >= 3))
		{
			gl_device_features.is_vao_available_ = true;
		}
	}

#if !defined(BSTONE_REN_3D_TEST_GL_NO_VAO)
	if (!gl_device_features.is_vao_available_)
	{
		extension_manager->probe(Ren3dGlExtensionId::arb_vertex_array_object);

		if (extension_manager->has(Ren3dGlExtensionId::arb_vertex_array_object))
		{
			gl_device_features.is_vao_available_ = true;
		}
	}
#endif // BSTONE_REN_3D_TEST_GL_NO_VAO
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_max_vertex_arrays(Ren3dDeviceFeatures& device_features)
try
{
	auto gl_count = GLint{};

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_count);
	Ren3dGlError::ensure_debug();

	device_features.max_vertex_input_locations_ = 0;

	if (gl_count > 0)
	{
		device_features.max_vertex_input_locations_ = gl_count;
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_vsync(
	sys::GlMgr& gl_mgr,
	Ren3dDeviceFeatures& device_features)
try
{
	device_features.is_vsync_available_ = false;
	device_features.is_vsync_requires_restart_ = false;

#if !defined(BSTONE_REN_3D_TEST_NO_SWAP_INTERVAL)
	try
	{
		gl_mgr.set_swap_interval(0);
		gl_mgr.set_swap_interval(1);
		device_features.is_vsync_available_ = true;
	}
	catch (const Exception&)
	{
	}
#endif // BSTONE_REN_3D_TEST_NO_SWAP_INTERVAL
}
BSTONE_STATIC_THROW_NESTED_FUNC

bool Ren3dGlUtils::get_vsync(sys::GlMgr& gl_mgr)
try
{
	switch (gl_mgr.get_swap_interval())
	{
		case 0:
			return false;

		case -1:
		case 1:
			return true;

		default:
			BSTONE_STATIC_THROW("Unsupported swap interval value.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_buffer_storage(
	const Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	gl_device_features.is_buffer_storage_available_ = false;

#if !defined(BSTONE_REN_3D_TEST_GL_NO_BUFFER_STORAGE)
	extension_manager->probe(Ren3dGlExtensionId::arb_buffer_storage);

	if (extension_manager->has(Ren3dGlExtensionId::arb_buffer_storage))
	{
		gl_device_features.is_buffer_storage_available_ = true;
	}
#endif // BSTONE_REN_3D_TEST_GL_NO_BUFFER_STORAGE
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_dsa(
	const Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	gl_device_features.is_dsa_available_ = false;

#if !defined(BSTONE_REN_3D_TEST_GL_NO_DSA)
	extension_manager->probe(Ren3dGlExtensionId::arb_direct_state_access);

	if (extension_manager->has(Ren3dGlExtensionId::arb_direct_state_access))
	{
		gl_device_features.is_dsa_available_ = true;
	}
#endif // !BSTONE_REN_3D_TEST_GL_NO_DSA
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::probe_sso(
	const Ren3dGlExtensionMgrPtr extension_manager,
	Ren3dGlDeviceFeatures& gl_device_features)
try
{
	gl_device_features.is_sso_available_ = false;

#if !defined( BSTONE_REN_3D_TEST_GL_NO_SSO)
	extension_manager->probe(Ren3dGlExtensionId::arb_separate_shader_objects);

	if (extension_manager->has(Ren3dGlExtensionId::arb_separate_shader_objects))
	{
		gl_device_features.is_sso_available_ = true;
	}
#endif // BSTONE_REN_3D_TEST_GL_NO_SSO
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::enable_scissor(bool is_enabled)
try
{
	(is_enabled ? glEnable : glDisable)(GL_SCISSOR_TEST);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_scissor_box(const Ren3dScissorBox& scissor_box)
try
{
	if (scissor_box.x < 0)
	{
		BSTONE_STATIC_THROW("Negative offset by X.");
	}

	if (scissor_box.y < 0)
	{
		BSTONE_STATIC_THROW("Negative offset by Y.");
	}

	if (scissor_box.width_ < 0)
	{
		BSTONE_STATIC_THROW("Negative width.");
	}

	if (scissor_box.height_ < 0)
	{
		BSTONE_STATIC_THROW("Negative height.");
	}

	glScissor(
		scissor_box.x,
		scissor_box.y,
		scissor_box.width_,
		scissor_box.height_);

	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_viewport_rect(const Ren3dViewport& viewport)
try
{
	if (viewport.x < 0)
	{
		BSTONE_STATIC_THROW("Negative offset by X.");
	}

	if (viewport.y < 0)
	{
		BSTONE_STATIC_THROW("Negative offset by Y.");
	}

	if (viewport.width_ < 0)
	{
		BSTONE_STATIC_THROW("Negative width.");
	}

	if (viewport.height_ < 0)
	{
		BSTONE_STATIC_THROW("Negative height.");
	}

	glViewport(
		viewport.x,
		viewport.y,
		viewport.width_,
		viewport.height_);

	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_viewport_depth_range(
	const Ren3dViewport& viewport,
	const Ren3dGlDeviceFeatures& gl_device_features)
try
{
	if (viewport.min_depth_ < 0.0F || viewport.min_depth_ > 1.0F)
	{
		BSTONE_STATIC_THROW("Minimum depth out of range.");
	}

	if (viewport.max_depth_ < 0.0F || viewport.max_depth_ > 1.0F)
	{
		BSTONE_STATIC_THROW("Maximum depth out of range.");
	}

	const auto is_es = (gl_device_features.context_kind_ == Ren3dGlContextKind::es);

	if (is_es)
	{
		glDepthRangef(viewport.min_depth_, viewport.max_depth_);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glDepthRange(viewport.min_depth_, viewport.max_depth_);
		Ren3dGlError::ensure_debug();
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::enable_culling(bool is_enable)
try
{
	(is_enable ? glEnable : glDisable)(GL_CULL_FACE);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_culling_face(Ren3dCullingFace culling_face)
try
{
	auto gl_culling_face = GLenum{};

	switch (culling_face)
	{
		case Ren3dCullingFace::clockwise:
			gl_culling_face = GL_CW;
			break;

		case Ren3dCullingFace::counter_clockwise:
			gl_culling_face = GL_CCW;
			break;

		default:
			BSTONE_STATIC_THROW("Unsupported front face.");
	}

	glFrontFace(gl_culling_face);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_culling_mode(Ren3dCullingMode culling_mode)
try
{
	auto gl_culling_mode = GLenum{};

	switch (culling_mode)
	{
		case Ren3dCullingMode::back:
			gl_culling_mode = GL_BACK;
			break;

		case Ren3dCullingMode::front:
			gl_culling_mode = GL_FRONT;
			break;

		case Ren3dCullingMode::both:
			gl_culling_mode = GL_FRONT_AND_BACK;
			break;

		default:
			BSTONE_STATIC_THROW("Unsupported culling mode.");
	}

	glCullFace(gl_culling_mode);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::enable_depth_test(bool is_enable)
try
{
	(is_enable ? glEnable : glDisable)(GL_DEPTH_TEST);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::enable_depth_write(bool is_enable)
try
{
	glDepthMask(is_enable);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::enable_blending(bool is_enable)
try
{
	(is_enable ? glEnable : glDisable)(GL_BLEND);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_blending_func(const Ren3dBlendingFunc& blending_func)
try
{
	auto gl_src_factor = get_blending_factor(blending_func.src_factor_);
	auto gl_dst_factor = get_blending_factor(blending_func.dst_factor_);
	glBlendFunc(gl_src_factor, gl_dst_factor);
	Ren3dGlError::ensure_debug();
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::index_buffer_get_element_type_by_byte_depth(int byte_depth)
try
{
	switch (byte_depth)
	{
		case 1: return GL_UNSIGNED_BYTE;
		case 2: return GL_UNSIGNED_SHORT;
		case 4: return GL_UNSIGNED_INT;
		default: BSTONE_STATIC_THROW("Invalid byte depth.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Ren3dGlUtils::set_renderer_features(Ren3dDeviceFeatures& device_features)
try
{
	// Max texture dimension.
	//
	auto gl_texture_dimension = GLint{};

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_texture_dimension);

	if (gl_texture_dimension == 0)
	{
		BSTONE_STATIC_THROW("Failed to get maximum texture dimension.");
	}

	// Max viewport dimensions.
	//
	using GlViewportDimensions = std::array<GLint, 2>;
	auto gl_viewport_dimensions = GlViewportDimensions{};

	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, gl_viewport_dimensions.data());

	if (gl_viewport_dimensions[0] == 0 || gl_viewport_dimensions[1] == 0)
	{
		BSTONE_STATIC_THROW("Failed to get viewport dimensions.");
	}

	// Set the values.
	//
	device_features.max_texture_dimension_ = gl_texture_dimension;

	device_features.max_viewport_width_ = gl_viewport_dimensions[0];
	device_features.max_viewport_height_ = gl_viewport_dimensions[1];
}
BSTONE_STATIC_THROW_NESTED_FUNC

Ren3dDeviceInfo Ren3dGlUtils::get_device_info()
try
{
	auto result = Ren3dDeviceInfo{};

	// Name.
	//
	const auto gl_name = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

	Ren3dGlError::ensure_debug();

	if (gl_name != nullptr)
	{
		result.name_ = gl_name;
	}

	// Vendor.
	//
	const auto gl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

	Ren3dGlError::ensure_debug();

	if (gl_vendor != nullptr)
	{
		result.vendor_ = gl_vendor;
	}

	// Version.
	//
	const auto gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

	Ren3dGlError::ensure_debug();

	if (gl_version != nullptr)
	{
		result.version_ = gl_version;
	}

	// Result.
	//
	return result;
}
BSTONE_STATIC_THROW_NESTED_FUNC

std::string Ren3dGlUtils::get_log(bool is_shader, GLuint gl_name)
try
{
	const auto gl_info_function = (is_shader ? glGetShaderInfoLog : glGetProgramInfoLog);
	const auto gl_size_function = (is_shader ? glGetShaderiv : glGetProgramiv);

	auto size = GLint{};
	gl_size_function(gl_name, GL_INFO_LOG_LENGTH, &size);
	Ren3dGlError::ensure_debug();

	auto result = std::string{};

	if (size > 0)
	{
		result.resize(size);
		auto info_size = GLsizei{};
		gl_info_function(gl_name, size, &info_size, &result[0]);
		Ren3dGlError::ensure_debug();

		if (info_size <= 0)
		{
			result.clear();
		}
	}

	return result;
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::get_mag_filter(Ren3dFilterKind mag_filter)
try
{
	switch (mag_filter)
	{
		case Ren3dFilterKind::nearest: return GL_NEAREST;
		case Ren3dFilterKind::linear: return GL_LINEAR;
		default: BSTONE_STATIC_THROW("Unsupported magnification filter.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::get_min_filter(Ren3dFilterKind min_filter, Ren3dMipmapMode mipmap_mode)
try
{
	switch (mipmap_mode)
	{
		case Ren3dMipmapMode::none:
			switch (min_filter)
			{
				case Ren3dFilterKind::nearest: return GL_NEAREST;
				case Ren3dFilterKind::linear: return GL_LINEAR;
				default: BSTONE_STATIC_THROW("Unsupported minification filter.");
			}

			break;

		case Ren3dMipmapMode::nearest:
			switch (min_filter)
			{
				case Ren3dFilterKind::nearest: return GL_NEAREST_MIPMAP_NEAREST;
				case Ren3dFilterKind::linear: return GL_LINEAR_MIPMAP_NEAREST;
				default: BSTONE_STATIC_THROW("Unsupported minification mipmap filter.");
			}

			break;

		case Ren3dMipmapMode::linear:
			switch (min_filter)
			{
				case Ren3dFilterKind::nearest: return GL_NEAREST_MIPMAP_LINEAR;
				case Ren3dFilterKind::linear: return GL_LINEAR_MIPMAP_LINEAR;
				default: BSTONE_STATIC_THROW("Unsupported minification mipmap filter.");
			}

			break;

		default:
			BSTONE_STATIC_THROW("Unsupported mipmap mode.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::get_address_mode(Ren3dAddressMode address_mode)
try
{
	switch (address_mode)
	{
		case Ren3dAddressMode::clamp: return GL_CLAMP_TO_EDGE;
		case Ren3dAddressMode::repeat: return GL_REPEAT;
		default: BSTONE_STATIC_THROW("Unsupported address mode.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::get_texture_wrap_axis(Ren3dTextureAxis texture_axis)
try
{
	switch (texture_axis)
	{
		case Ren3dTextureAxis::u: return GL_TEXTURE_WRAP_S;
		case Ren3dTextureAxis::v: return GL_TEXTURE_WRAP_T;
		default: BSTONE_STATIC_THROW("Unsupported texture axis.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

GLenum Ren3dGlUtils::get_blending_factor(Ren3dBlendingFactor factor)
try
{
	switch (factor)
	{
		case Ren3dBlendingFactor::zero: return GL_ZERO;
		case Ren3dBlendingFactor::one: return GL_ONE;
		case Ren3dBlendingFactor::src_color: return GL_SRC_COLOR;
		case Ren3dBlendingFactor::src_alpha: return GL_SRC_ALPHA;
		case Ren3dBlendingFactor::one_minus_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
		default: BSTONE_STATIC_THROW("Invalid blending factor.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace detail
} // namespace bstone
