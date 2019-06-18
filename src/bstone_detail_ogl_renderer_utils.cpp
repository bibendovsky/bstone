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
#include "bstone_detail_ogl_renderer_utils.h"
#include <cassert>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>
#include "SDL_video.h"
#include "glm/gtc/matrix_transform.hpp"
#include "bstone_ogl.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglRendererUtils
//

const std::string& OglRendererUtils::get_error_message() const
{
	return error_message_;
}

bool OglRendererUtils::load_library()
{
	const auto error_message_prefix = "Failed to load default OpenGL library. ";

	const auto sdl_result = ::SDL_GL_LoadLibrary(nullptr);

	if (sdl_result < 0)
	{
		error_message_ = error_message_prefix;
		error_message_ += ::SDL_GetError();

		return false;
	}

	return true;
}

void OglRendererUtils::unload_library()
{
	::SDL_GL_UnloadLibrary();
}

void* OglRendererUtils::resolve_symbol(
	const char* const symbol)
{
	return ::SDL_GL_GetProcAddress(symbol);
}

SdlGlContextUPtr OglRendererUtils::create_context(
	SdlWindowPtr sdl_window)
{
	const auto error_message_prefix = "Failed to create OpenGL context. ";

	if (!sdl_window)
	{
		error_message_ = error_message_prefix;
		error_message_ += "Null SDL window.";

		return nullptr;
	}

	auto sdl_gl_context = SdlGlContextUPtr{::SDL_GL_CreateContext(sdl_window)};

	if (!sdl_gl_context)
	{
		error_message_ = error_message_prefix;
		error_message_ += ::SDL_GetError();
	}

	return sdl_gl_context;
}

bool OglRendererUtils::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContextPtr sdl_gl_context)
{
	const auto error_message_prefix = "Failed to make a context current. ";

	if (!sdl_window)
	{
		error_message_ = error_message_prefix;
		error_message_ += "Null SDL window.";

		return false;
	}

	const auto sdl_result = ::SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

	if (sdl_result < 0)
	{
		error_message_ = error_message_prefix;
		error_message_ += ::SDL_GetError();

		return false;
	}

	return true;
}

bool OglRendererUtils::create_window_and_context(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowUPtr& sdl_window,
	SdlGlContextUPtr& sdl_gl_context)
{
	auto renderer_utils = RendererUtils{};

	auto sdl_window_result = renderer_utils.create_window(param);

	if (!sdl_window_result)
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	auto sdl_gl_context_result = create_context(sdl_window_result.get());

	if (!sdl_gl_context_result)
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	sdl_window = std::move(sdl_window_result);
	sdl_gl_context = std::move(sdl_gl_context_result);

	return true;
}

bool OglRendererUtils::create_probe_window_and_context(
	SdlWindowUPtr& sdl_window,
	SdlGlContextUPtr& sdl_gl_context)
{
	auto param = RendererUtilsCreateWindowParam{};
	param.is_opengl_ = true;
	param.window_.width_ = 1;
	param.window_.height_ = 1;
	param.aa_kind_ = RendererAaKind::none;
	param.aa_value_ = 0;

	return create_window_and_context(param, sdl_window, sdl_gl_context);
}

bool OglRendererUtils::window_get_drawable_size(
	SdlWindowPtr sdl_window,
	int& width,
	int& height)
{
	width = 0;
	height = 0;

	if (!sdl_window)
	{
		error_message_ = "Null window.";

		return false;
	}

	::SDL_GL_GetDrawableSize(sdl_window, &width, &height);

	if (width <= 0 || height <= 0)
	{
		error_message_ = ::SDL_GetError();

		return false;
	}

	return true;
}

OglRendererUtilsContextType OglRendererUtils::context_get_type()
{
	const auto sdl_ogl_context = ::SDL_GL_GetCurrentContext();

	if (sdl_ogl_context == nullptr)
	{
		return OglRendererUtilsContextType::invalid;
	}

	auto sdl_attribute = 0;

	const auto sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &sdl_attribute);

	if (sdl_result != 0)
	{
		return OglRendererUtilsContextType::invalid;
	}

	switch (sdl_attribute)
	{
		case 0:
			return OglRendererUtilsContextType::none;

		case SDL_GL_CONTEXT_PROFILE_CORE:
			return OglRendererUtilsContextType::core;

		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
			return OglRendererUtilsContextType::compatibility;

		case SDL_GL_CONTEXT_PROFILE_ES:
			return OglRendererUtilsContextType::es;

		default:
			return OglRendererUtilsContextType::invalid;
	}
}

bool OglRendererUtils::context_get_version(
	int& major_version,
	int& minor_version)
{
	major_version = 0;
	minor_version = 0;

	const auto sdl_ogl_context = ::SDL_GL_GetCurrentContext();

	if (sdl_ogl_context == nullptr)
	{
		return false;
	}

	auto sdl_result = 0;

	// Major.
	//
	auto sdl_major_version = 0;

	sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &sdl_major_version);

	if (sdl_result != 0)
	{
		return false;
	}

	// Minor.
	//
	auto sdl_minor_version = 0;

	sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &sdl_minor_version);

	if (sdl_result != 0)
	{
		return false;
	}

	// Result.
	//
	major_version = sdl_major_version;
	minor_version = sdl_minor_version;

	return true;
}

int OglRendererUtils::anisotropy_get_max_value()
{
	auto ogl_max_value = GLfloat{};

	::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &ogl_max_value);

	assert(!OglRendererUtils::was_errors());

	if (ogl_max_value < static_cast<GLfloat>(RendererSampler::anisotropy_min))
	{
		return RendererSampler::anisotropy_min;
	}

	return static_cast<int>(ogl_max_value);
}

void OglRendererUtils::anisotropy_set_value(
	const GLenum ogl_target,
	const RendererDeviceFeatures& device_features,
	const int anisotropy_value)
{
	auto clamped_value = anisotropy_value;

	if (clamped_value < RendererSampler::anisotropy_min)
	{
		clamped_value = RendererSampler::anisotropy_min;
	}
	else if (clamped_value > device_features.anisotropy_max_value_)
	{
		clamped_value = device_features.anisotropy_max_value_;
	}

	const auto ogl_value = static_cast<GLfloat>(clamped_value);

	::glTexParameterfv(ogl_target, GL_TEXTURE_MAX_ANISOTROPY, &ogl_value);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::anisotropy_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features)
{
	device_features.anisotropy_is_available_ = false;

	if (!device_features.anisotropy_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_texture_filter_anisotropic);

		device_features.anisotropy_is_available_ =
			extension_manager->has_extension(OglExtensionId::arb_texture_filter_anisotropic);
	}

	if (!device_features.anisotropy_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::ext_texture_filter_anisotropic);

		device_features.anisotropy_is_available_ =
			extension_manager->has_extension(OglExtensionId::ext_texture_filter_anisotropic);
	}

	if (device_features.anisotropy_is_available_)
	{
		device_features.anisotropy_min_value_ = RendererSampler::anisotropy_min;
		device_features.anisotropy_max_value_ = anisotropy_get_max_value();
	}
}

void OglRendererUtils::npot_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features)
{
	device_features.npot_is_available_ = false;

	if (!device_features.npot_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_texture_non_power_of_two);

		device_features.npot_is_available_ =
			extension_manager->has_extension(OglExtensionId::arb_texture_non_power_of_two);
	}
}

void OglRendererUtils::mipmap_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features,
	OglRendererUtilsDeviceFeatures& ogl_device_features)
{
	device_features.mipmap_is_available_ = false;
	ogl_device_features.mipmap_function_ = nullptr;

	if (!device_features.mipmap_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_framebuffer_object);

		if (extension_manager->has_extension(OglExtensionId::arb_framebuffer_object))
		{
			device_features.mipmap_is_available_ = true;
			ogl_device_features.mipmap_function_ = ::glGenerateMipmap;
		}
	}

	if (!device_features.mipmap_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_object);

		if (extension_manager->has_extension(OglExtensionId::ext_framebuffer_object))
		{
			device_features.mipmap_is_available_ = true;
			ogl_device_features.mipmap_function_ = ::glGenerateMipmapEXT;
		}
	}
}

void OglRendererUtils::framebuffer_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features,
	OglRendererUtilsDeviceFeatures& ogl_device_features)
{
	auto is_arb = false;
	auto is_available = false;

	if (!is_available)
	{
		extension_manager->probe_extension(OglExtensionId::arb_framebuffer_object);

		is_available = extension_manager->has_extension(OglExtensionId::arb_framebuffer_object);

		if (is_available)
		{
			is_arb = true;
		}
	}

	if (!is_available)
	{
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_blit);
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_multisample);
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_object);
		extension_manager->probe_extension(OglExtensionId::ext_packed_depth_stencil);

		is_available =
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_blit) &&
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_multisample) &&
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_object) &&
			extension_manager->has_extension(OglExtensionId::ext_packed_depth_stencil)
		;
	}

	device_features.msaa_min_value_ = RendererUtils::aa_get_min_value();
	device_features.msaa_max_value_ = msaa_get_max_value(extension_manager);

	if (device_features.msaa_min_value_ == device_features.msaa_max_value_)
	{
		is_available = false;
	}

	device_features.framebuffer_is_available_ = is_available;

	if (is_available)
	{
		ogl_device_features.framebuffer_is_arb_ = is_arb;
	}
}

int OglRendererUtils::msaa_get_max_value(
	OglExtensionManagerPtr extension_manager)
{
	auto max_value = GLint{};

	if (extension_manager->has_extension(OglExtensionId::arb_framebuffer_object))
	{
		::glGetIntegerv(GL_MAX_SAMPLES, &max_value);
		assert(!OglRendererUtils::was_errors());
	}
	else if (
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_blit) &&
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_multisample) &&
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_object) &&
		extension_manager->has_extension(OglExtensionId::ext_packed_depth_stencil))
	{
		::glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_value);
		assert(!OglRendererUtils::was_errors());
	}

	return std::max(max_value, RendererUtils::aa_get_min_value());
}

void OglRendererUtils::clear_buffers()
{
	assert(::glClear != nullptr);

	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::swap_window(
	SdlWindowPtr sdl_window)
{
	assert(sdl_window != nullptr);

	::SDL_GL_SwapWindow(sdl_window);
}

bool OglRendererUtils::was_errors()
{
	assert(::glGetError != nullptr);

	const auto max_error_count = 32;

	auto was_error = false;

	for (int i = 0; i < max_error_count; ++i)
	{
		const auto last_error = ::glGetError();

		if (last_error == GL_NO_ERROR)
		{
			break;
		}

		was_error = true;
	}

	return was_error;
}

void OglRendererUtils::clear_errors()
{
	static_cast<void>(was_errors());
}

void OglRendererUtils::set_color_buffer_clear_color(
	const R8g8b8a8& color)
{
	assert(::glClearColor != nullptr);

	const auto reciprocal_255 = 1.0F / 255.0F;

	::glClearColor(
		static_cast<float>(color.r) * reciprocal_255,
		static_cast<float>(color.g) * reciprocal_255,
		static_cast<float>(color.b) * reciprocal_255,
		static_cast<float>(color.a) * reciprocal_255
	);

	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::scissor_enable(
	const bool is_enabled)
{
	if (is_enabled)
	{
		::glEnable(GL_SCISSOR_TEST);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_SCISSOR_TEST);
		assert(!OglRendererUtils::was_errors());
	}
}

void OglRendererUtils::scissor_set_box(
	const int x,
	const int y,
	const int width,
	const int height)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(width > 0);
	assert(height > 0);

	::glScissor(x, y, width, height);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::viewport_set_rectangle(
	const int x,
	const int y,
	const int width,
	const int height)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(width > 0);
	assert(height > 0);

	::glViewport(x, y, width, height);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::viewport_set_depth_range(
	const float min_depth,
	const float max_depth)
{
	assert(min_depth >= 0.0F && min_depth <= 1.0F);
	assert(max_depth >= 0.0F && max_depth <= 1.0F);

	::glDepthRange(min_depth, max_depth);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::texture_2d_set(
	const GLuint ogl_texture_name)
{
	::glBindTexture(GL_TEXTURE_2D, ogl_texture_name);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::blending_set_function(
	const RendererBlendingFactor src_factor,
	const RendererBlendingFactor dst_factor)
{
	auto ogl_src_factor = blending_get_factor(src_factor);
	auto ogl_dst_factor = blending_get_factor(dst_factor);

	::glBlendFunc(ogl_src_factor, ogl_dst_factor);
	assert(!OglRendererUtils::was_errors());
}

GLenum OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
	const int byte_depth)
{
	switch (byte_depth)
	{
	case 1:
		return GL_UNSIGNED_BYTE;

	case 2:
		return GL_UNSIGNED_SHORT;

	case 4:
		return GL_UNSIGNED_INT;

	default:
		assert(!"Invalid byte depth.");

		return GL_NONE;
	}
}

bool OglRendererUtils::renderer_features_set(
	RendererDeviceFeatures& device_features)
{
	// Max texture dimension.
	//
	auto ogl_texture_dimension = GLint{};

	::glGetIntegerv(GL_MAX_TEXTURE_SIZE, &ogl_texture_dimension);

	if (OglRendererUtils::was_errors() || ogl_texture_dimension == 0)
	{
		error_message_ = "Failed to get maximum texture dimension.";

		return false;
	}


	// Max viewport dimensions.
	//
	using OglViewportDimensions = std::array<GLint, 2>;
	auto ogl_viewport_dimensions = OglViewportDimensions{};

	::glGetIntegerv(GL_MAX_VIEWPORT_DIMS, ogl_viewport_dimensions.data());

	if (OglRendererUtils::was_errors() ||
		ogl_viewport_dimensions[0] == 0 || ogl_viewport_dimensions[1] == 0)
	{
		error_message_ = "Failed to get viewport dimensions.";

		return false;
	}


	// Set the values.
	//
	device_features.max_texture_dimension_ = ogl_texture_dimension;

	device_features.max_viewport_width_ = ogl_viewport_dimensions[0];
	device_features.max_viewport_height_ = ogl_viewport_dimensions[1];

	return true;
}

GLenum OglRendererUtils::blending_get_factor(
	const RendererBlendingFactor factor)
{
	switch (factor)
	{
		case RendererBlendingFactor::zero:
			return GL_ZERO;

		case RendererBlendingFactor::one:
			return GL_ONE;

		case RendererBlendingFactor::src_color:
			return GL_SRC_COLOR;

		case RendererBlendingFactor::src_alpha:
			return GL_SRC_ALPHA;

		case RendererBlendingFactor::one_minus_src_alpha:
			return GL_ONE_MINUS_SRC_ALPHA;

		default:
			return GL_NONE;
	}
}

//
// OglRendererUtils
// ==========================================================================


} // detail
} // bstone
