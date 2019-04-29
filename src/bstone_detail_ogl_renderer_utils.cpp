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
#include <limits>
#include <sstream>
#include "SDL_video.h"
#include "glm/gtc/matrix_transform.hpp"
#include "bstone_ogl.h"


using namespace std::string_literals;


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
	auto is_succeed = true;
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

bool OglRendererUtils::resolve_symbols_1_1()
{
	if (!resolve_unique_symbols_1_0())
	{
		return false;
	}

	if (!resolve_unique_symbols_1_1())
	{
		return false;
	}

	return true;
}

RendererUtilsExtensions OglRendererUtils::extensions_get(
	const bool is_core_profile)
{
	if (is_core_profile)
	{
		return extensions_get_core();
	}
	else
	{
		return extensions_get_compatibility();
	}
}

const std::string& OglRendererUtils::extension_gl_arb_texture_filter_anisotropic_get_name()
{
	static const auto result = "GL_ARB_texture_filter_anisotropic"s;

	return result;
}

const std::string& OglRendererUtils::extension_gl_ext_texture_filter_anisotropic_get_name()
{
	static const auto result = "GL_EXT_texture_filter_anisotropic"s;

	return result;
}

int OglRendererUtils::anisotropy_get_max_value(
	const OglRendererUtilsDeviceFeatures& ogl_device_features)
{
	if (!ogl_device_features.anisotropy_is_available_)
	{
		return RendererSampler::anisotropy_min;
	}

	auto ogl_max_value_enum = GLenum{};
	
	if (ogl_device_features.anisotropy_is_ext_)
	{
		ogl_max_value_enum = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
	}
	else
	{
		ogl_max_value_enum = GL_MAX_TEXTURE_MAX_ANISOTROPY;
	}

	auto ogl_max_value = GLfloat{};

	::glGetFloatv(ogl_max_value_enum, &ogl_max_value);

	assert(!OglRendererUtils::was_errors());

	if (ogl_max_value == GLfloat{})
	{
		return RendererSampler::anisotropy_min;
	}

	return static_cast<int>(ogl_max_value);
}

void OglRendererUtils::anisotropy_set_value(
	const GLenum ogl_target,
	const OglRendererUtilsDeviceFeatures& ogl_device_features,
	const int anisotropy_value)
{
	if (!ogl_device_features.anisotropy_is_available_)
	{
		return;
	}

	auto clamped_value = anisotropy_value;

	if (clamped_value < RendererSampler::anisotropy_min)
	{
		clamped_value = RendererSampler::anisotropy_min;
	}
	else if (clamped_value > ogl_device_features.anisotropy_max_value_)
	{
		clamped_value = ogl_device_features.anisotropy_max_value_;
	}

	const auto ogl_value = static_cast<GLfloat>(clamped_value);

	if (ogl_device_features.anisotropy_is_ext_)
	{
		::glTexParameterfv(ogl_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, &ogl_value);
	}
	else
	{
		::glTexParameterfv(ogl_target, GL_TEXTURE_MAX_ANISOTROPY, &ogl_value);
	}

	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::anisotropy_probe(
	const RendererUtilsExtensions& extensions,
	RendererDeviceFeatures& device_features,
	OglRendererUtilsDeviceFeatures& ogl_device_features)
{
	auto dst_ogl_device_features = ogl_device_features;

	if (!dst_ogl_device_features.anisotropy_is_available_)
	{
		dst_ogl_device_features.anisotropy_is_available_ = RendererUtils::extension_has(
			extension_gl_arb_texture_filter_anisotropic_get_name(),
			extensions
		);

		if (dst_ogl_device_features.anisotropy_is_available_)
		{
			dst_ogl_device_features.anisotropy_is_ext_ = false;
		}
	}

	if (!dst_ogl_device_features.anisotropy_is_available_)
	{
		dst_ogl_device_features.anisotropy_is_available_ = RendererUtils::extension_has(
			extension_gl_ext_texture_filter_anisotropic_get_name(),
			extensions
		);

		if (dst_ogl_device_features.anisotropy_is_available_)
		{
			dst_ogl_device_features.anisotropy_is_ext_ = true;
		}
	}

	if (dst_ogl_device_features.anisotropy_is_available_)
	{
		dst_ogl_device_features.anisotropy_max_value_ = anisotropy_get_max_value(dst_ogl_device_features);

		if (dst_ogl_device_features.anisotropy_max_value_ <= RendererSampler::anisotropy_min)
		{
			dst_ogl_device_features.anisotropy_is_available_ = false;
		}
	}

	if (dst_ogl_device_features.anisotropy_is_available_)
	{
		device_features.anisotropy_is_available_ = true;
		device_features.anisotropy_min_value_ = RendererSampler::anisotropy_min;
		device_features.anisotropy_max_value_ = dst_ogl_device_features.anisotropy_max_value_;

		ogl_device_features = dst_ogl_device_features;
	}
	else
	{
		device_features.anisotropy_is_available_ = false;
		device_features.anisotropy_min_value_ = RendererSampler::anisotropy_min;
		device_features.anisotropy_max_value_ = RendererSampler::anisotropy_min;

		ogl_device_features.anisotropy_is_available_ = false;
		ogl_device_features.anisotropy_is_ext_ = false;
		ogl_device_features.anisotropy_max_value_ = RendererSampler::anisotropy_min;
	}
}

// GL_ARB_texture_non_power_of_two
const std::string& OglRendererUtils::extension_gl_arb_texture_non_power_of_two_get_name()
{
	static const auto result = "GL_ARB_texture_non_power_of_two"s;

	return result;
}

void OglRendererUtils::npot_probe(
	const RendererUtilsExtensions& extensions,
	RendererDeviceFeatures& device_features,
	OglRendererUtilsDeviceFeatures& ogl_device_features)
{
	auto dst_ogl_device_features = ogl_device_features;

	if (!dst_ogl_device_features.npot_is_available_)
	{
		dst_ogl_device_features.npot_is_available_ = RendererUtils::extension_has(
			extension_gl_arb_texture_non_power_of_two_get_name(),
			extensions
		);
	}

	if (dst_ogl_device_features.npot_is_available_)
	{
		device_features.npot_is_available_ = true;

		ogl_device_features = dst_ogl_device_features;
	}
	else
	{
		device_features.npot_is_available_ = false;

		ogl_device_features.npot_is_available_ = false;
	}
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

void OglRendererUtils::set_color_buffer_clear_color(
	const RendererColor32& color)
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

		return {};
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
			return {};
	}
}

void* OglRendererUtils::resolve_symbol(
	const char* const symbol)
{
	return ::SDL_GL_GetProcAddress(symbol);
}

void OglRendererUtils::clear_unique_symbols_1_0()
{
	::glCullFace = nullptr;
	::glFrontFace = nullptr;
	::glHint = nullptr;
	::glLineWidth = nullptr;
	::glPointSize = nullptr;
	::glPolygonMode = nullptr;
	::glScissor = nullptr;
	::glTexParameterf = nullptr;
	::glTexParameterfv = nullptr;
	::glTexParameteri = nullptr;
	::glTexParameteriv = nullptr;
	::glTexImage1D = nullptr;
	::glTexImage2D = nullptr;
	::glDrawBuffer = nullptr;
	::glClear = nullptr;
	::glClearColor = nullptr;
	::glClearStencil = nullptr;
	::glClearDepth = nullptr;
	::glStencilMask = nullptr;
	::glColorMask = nullptr;
	::glDepthMask = nullptr;
	::glDisable = nullptr;
	::glEnable = nullptr;
	::glFinish = nullptr;
	::glFlush = nullptr;
	::glBlendFunc = nullptr;
	::glLogicOp = nullptr;
	::glStencilFunc = nullptr;
	::glStencilOp = nullptr;
	::glDepthFunc = nullptr;
	::glPixelStoref = nullptr;
	::glPixelStorei = nullptr;
	::glReadBuffer = nullptr;
	::glReadPixels = nullptr;
	::glGetBooleanv = nullptr;
	::glGetDoublev = nullptr;
	::glGetError = nullptr;
	::glGetFloatv = nullptr;
	::glGetIntegerv = nullptr;
	::glGetString = nullptr;
	::glGetTexImage = nullptr;
	::glGetTexParameterfv = nullptr;
	::glGetTexParameteriv = nullptr;
	::glGetTexLevelParameterfv = nullptr;
	::glGetTexLevelParameteriv = nullptr;
	::glIsEnabled = nullptr;
	::glDepthRange = nullptr;
	::glViewport = nullptr;
	::glNewList = nullptr;
	::glEndList = nullptr;
	::glCallList = nullptr;
	::glCallLists = nullptr;
	::glDeleteLists = nullptr;
	::glGenLists = nullptr;
	::glListBase = nullptr;
	::glBegin = nullptr;
	::glBitmap = nullptr;
	::glColor3b = nullptr;
	::glColor3bv = nullptr;
	::glColor3d = nullptr;
	::glColor3dv = nullptr;
	::glColor3f = nullptr;
	::glColor3fv = nullptr;
	::glColor3i = nullptr;
	::glColor3iv = nullptr;
	::glColor3s = nullptr;
	::glColor3sv = nullptr;
	::glColor3ub = nullptr;
	::glColor3ubv = nullptr;
	::glColor3ui = nullptr;
	::glColor3uiv = nullptr;
	::glColor3us = nullptr;
	::glColor3usv = nullptr;
	::glColor4b = nullptr;
	::glColor4bv = nullptr;
	::glColor4d = nullptr;
	::glColor4dv = nullptr;
	::glColor4f = nullptr;
	::glColor4fv = nullptr;
	::glColor4i = nullptr;
	::glColor4iv = nullptr;
	::glColor4s = nullptr;
	::glColor4sv = nullptr;
	::glColor4ub = nullptr;
	::glColor4ubv = nullptr;
	::glColor4ui = nullptr;
	::glColor4uiv = nullptr;
	::glColor4us = nullptr;
	::glColor4usv = nullptr;
	::glEdgeFlag = nullptr;
	::glEdgeFlagv = nullptr;
	::glEnd = nullptr;
	::glIndexd = nullptr;
	::glIndexdv = nullptr;
	::glIndexf = nullptr;
	::glIndexfv = nullptr;
	::glIndexi = nullptr;
	::glIndexiv = nullptr;
	::glIndexs = nullptr;
	::glIndexsv = nullptr;
	::glNormal3b = nullptr;
	::glNormal3bv = nullptr;
	::glNormal3d = nullptr;
	::glNormal3dv = nullptr;
	::glNormal3f = nullptr;
	::glNormal3fv = nullptr;
	::glNormal3i = nullptr;
	::glNormal3iv = nullptr;
	::glNormal3s = nullptr;
	::glNormal3sv = nullptr;
	::glRasterPos2d = nullptr;
	::glRasterPos2dv = nullptr;
	::glRasterPos2f = nullptr;
	::glRasterPos2fv = nullptr;
	::glRasterPos2i = nullptr;
	::glRasterPos2iv = nullptr;
	::glRasterPos2s = nullptr;
	::glRasterPos2sv = nullptr;
	::glRasterPos3d = nullptr;
	::glRasterPos3dv = nullptr;
	::glRasterPos3f = nullptr;
	::glRasterPos3fv = nullptr;
	::glRasterPos3i = nullptr;
	::glRasterPos3iv = nullptr;
	::glRasterPos3s = nullptr;
	::glRasterPos3sv = nullptr;
	::glRasterPos4d = nullptr;
	::glRasterPos4dv = nullptr;
	::glRasterPos4f = nullptr;
	::glRasterPos4fv = nullptr;
	::glRasterPos4i = nullptr;
	::glRasterPos4iv = nullptr;
	::glRasterPos4s = nullptr;
	::glRasterPos4sv = nullptr;
	::glRectd = nullptr;
	::glRectdv = nullptr;
	::glRectf = nullptr;
	::glRectfv = nullptr;
	::glRecti = nullptr;
	::glRectiv = nullptr;
	::glRects = nullptr;
	::glRectsv = nullptr;
	::glTexCoord1d = nullptr;
	::glTexCoord1dv = nullptr;
	::glTexCoord1f = nullptr;
	::glTexCoord1fv = nullptr;
	::glTexCoord1i = nullptr;
	::glTexCoord1iv = nullptr;
	::glTexCoord1s = nullptr;
	::glTexCoord1sv = nullptr;
	::glTexCoord2d = nullptr;
	::glTexCoord2dv = nullptr;
	::glTexCoord2f = nullptr;
	::glTexCoord2fv = nullptr;
	::glTexCoord2i = nullptr;
	::glTexCoord2iv = nullptr;
	::glTexCoord2s = nullptr;
	::glTexCoord2sv = nullptr;
	::glTexCoord3d = nullptr;
	::glTexCoord3dv = nullptr;
	::glTexCoord3f = nullptr;
	::glTexCoord3fv = nullptr;
	::glTexCoord3i = nullptr;
	::glTexCoord3iv = nullptr;
	::glTexCoord3s = nullptr;
	::glTexCoord3sv = nullptr;
	::glTexCoord4d = nullptr;
	::glTexCoord4dv = nullptr;
	::glTexCoord4f = nullptr;
	::glTexCoord4fv = nullptr;
	::glTexCoord4i = nullptr;
	::glTexCoord4iv = nullptr;
	::glTexCoord4s = nullptr;
	::glTexCoord4sv = nullptr;
	::glVertex2d = nullptr;
	::glVertex2dv = nullptr;
	::glVertex2f = nullptr;
	::glVertex2fv = nullptr;
	::glVertex2i = nullptr;
	::glVertex2iv = nullptr;
	::glVertex2s = nullptr;
	::glVertex2sv = nullptr;
	::glVertex3d = nullptr;
	::glVertex3dv = nullptr;
	::glVertex3f = nullptr;
	::glVertex3fv = nullptr;
	::glVertex3i = nullptr;
	::glVertex3iv = nullptr;
	::glVertex3s = nullptr;
	::glVertex3sv = nullptr;
	::glVertex4d = nullptr;
	::glVertex4dv = nullptr;
	::glVertex4f = nullptr;
	::glVertex4fv = nullptr;
	::glVertex4i = nullptr;
	::glVertex4iv = nullptr;
	::glVertex4s = nullptr;
	::glVertex4sv = nullptr;
	::glClipPlane = nullptr;
	::glColorMaterial = nullptr;
	::glFogf = nullptr;
	::glFogfv = nullptr;
	::glFogi = nullptr;
	::glFogiv = nullptr;
	::glLightf = nullptr;
	::glLightfv = nullptr;
	::glLighti = nullptr;
	::glLightiv = nullptr;
	::glLightModelf = nullptr;
	::glLightModelfv = nullptr;
	::glLightModeli = nullptr;
	::glLightModeliv = nullptr;
	::glLineStipple = nullptr;
	::glMaterialf = nullptr;
	::glMaterialfv = nullptr;
	::glMateriali = nullptr;
	::glMaterialiv = nullptr;
	::glPolygonStipple = nullptr;
	::glShadeModel = nullptr;
	::glTexEnvf = nullptr;
	::glTexEnvfv = nullptr;
	::glTexEnvi = nullptr;
	::glTexEnviv = nullptr;
	::glTexGend = nullptr;
	::glTexGendv = nullptr;
	::glTexGenf = nullptr;
	::glTexGenfv = nullptr;
	::glTexGeni = nullptr;
	::glTexGeniv = nullptr;
	::glFeedbackBuffer = nullptr;
	::glSelectBuffer = nullptr;
	::glRenderMode = nullptr;
	::glInitNames = nullptr;
	::glLoadName = nullptr;
	::glPassThrough = nullptr;
	::glPopName = nullptr;
	::glPushName = nullptr;
	::glClearAccum = nullptr;
	::glClearIndex = nullptr;
	::glIndexMask = nullptr;
	::glAccum = nullptr;
	::glPopAttrib = nullptr;
	::glPushAttrib = nullptr;
	::glMap1d = nullptr;
	::glMap1f = nullptr;
	::glMap2d = nullptr;
	::glMap2f = nullptr;
	::glMapGrid1d = nullptr;
	::glMapGrid1f = nullptr;
	::glMapGrid2d = nullptr;
	::glMapGrid2f = nullptr;
	::glEvalCoord1d = nullptr;
	::glEvalCoord1dv = nullptr;
	::glEvalCoord1f = nullptr;
	::glEvalCoord1fv = nullptr;
	::glEvalCoord2d = nullptr;
	::glEvalCoord2dv = nullptr;
	::glEvalCoord2f = nullptr;
	::glEvalCoord2fv = nullptr;
	::glEvalMesh1 = nullptr;
	::glEvalPoint1 = nullptr;
	::glEvalMesh2 = nullptr;
	::glEvalPoint2 = nullptr;
	::glAlphaFunc = nullptr;
	::glPixelZoom = nullptr;
	::glPixelTransferf = nullptr;
	::glPixelTransferi = nullptr;
	::glPixelMapfv = nullptr;
	::glPixelMapuiv = nullptr;
	::glPixelMapusv = nullptr;
	::glCopyPixels = nullptr;
	::glDrawPixels = nullptr;
	::glGetClipPlane = nullptr;
	::glGetLightfv = nullptr;
	::glGetLightiv = nullptr;
	::glGetMapdv = nullptr;
	::glGetMapfv = nullptr;
	::glGetMapiv = nullptr;
	::glGetMaterialfv = nullptr;
	::glGetMaterialiv = nullptr;
	::glGetPixelMapfv = nullptr;
	::glGetPixelMapuiv = nullptr;
	::glGetPixelMapusv = nullptr;
	::glGetPolygonStipple = nullptr;
	::glGetTexEnvfv = nullptr;
	::glGetTexEnviv = nullptr;
	::glGetTexGendv = nullptr;
	::glGetTexGenfv = nullptr;
	::glGetTexGeniv = nullptr;
	::glIsList = nullptr;
	::glFrustum = nullptr;
	::glLoadIdentity = nullptr;
	::glLoadMatrixf = nullptr;
	::glLoadMatrixd = nullptr;
	::glMatrixMode = nullptr;
	::glMultMatrixf = nullptr;
	::glMultMatrixd = nullptr;
	::glOrtho = nullptr;
	::glPopMatrix = nullptr;
	::glPushMatrix = nullptr;
	::glRotated = nullptr;
	::glRotatef = nullptr;
	::glScaled = nullptr;
	::glScalef = nullptr;
	::glTranslated = nullptr;
	::glTranslatef = nullptr;
}

bool OglRendererUtils::resolve_unique_symbols_1_0()
{
	auto is_failed = false;

	resolve_symbol("glCullFace", ::glCullFace, is_failed);
	resolve_symbol("glFrontFace", ::glFrontFace, is_failed);
	resolve_symbol("glHint", ::glHint, is_failed);
	resolve_symbol("glLineWidth", ::glLineWidth, is_failed);
	resolve_symbol("glPointSize", ::glPointSize, is_failed);
	resolve_symbol("glPolygonMode", ::glPolygonMode, is_failed);
	resolve_symbol("glScissor", ::glScissor, is_failed);
	resolve_symbol("glTexParameterf", ::glTexParameterf, is_failed);
	resolve_symbol("glTexParameterfv", ::glTexParameterfv, is_failed);
	resolve_symbol("glTexParameteri", ::glTexParameteri, is_failed);
	resolve_symbol("glTexParameteriv", ::glTexParameteriv, is_failed);
	resolve_symbol("glTexImage1D", ::glTexImage1D, is_failed);
	resolve_symbol("glTexImage2D", ::glTexImage2D, is_failed);
	resolve_symbol("glDrawBuffer", ::glDrawBuffer, is_failed);
	resolve_symbol("glClear", ::glClear, is_failed);
	resolve_symbol("glClearColor", ::glClearColor, is_failed);
	resolve_symbol("glClearStencil", ::glClearStencil, is_failed);
	resolve_symbol("glClearDepth", ::glClearDepth, is_failed);
	resolve_symbol("glStencilMask", ::glStencilMask, is_failed);
	resolve_symbol("glColorMask", ::glColorMask, is_failed);
	resolve_symbol("glDepthMask", ::glDepthMask, is_failed);
	resolve_symbol("glDisable", ::glDisable, is_failed);
	resolve_symbol("glEnable", ::glEnable, is_failed);
	resolve_symbol("glFinish", ::glFinish, is_failed);
	resolve_symbol("glFlush", ::glFlush, is_failed);
	resolve_symbol("glBlendFunc", ::glBlendFunc, is_failed);
	resolve_symbol("glLogicOp", ::glLogicOp, is_failed);
	resolve_symbol("glStencilFunc", ::glStencilFunc, is_failed);
	resolve_symbol("glStencilOp", ::glStencilOp, is_failed);
	resolve_symbol("glDepthFunc", ::glDepthFunc, is_failed);
	resolve_symbol("glPixelStoref", ::glPixelStoref, is_failed);
	resolve_symbol("glPixelStorei", ::glPixelStorei, is_failed);
	resolve_symbol("glReadBuffer", ::glReadBuffer, is_failed);
	resolve_symbol("glReadPixels", ::glReadPixels, is_failed);
	resolve_symbol("glGetBooleanv", ::glGetBooleanv, is_failed);
	resolve_symbol("glGetDoublev", ::glGetDoublev, is_failed);
	resolve_symbol("glGetError", ::glGetError, is_failed);
	resolve_symbol("glGetFloatv", ::glGetFloatv, is_failed);
	resolve_symbol("glGetIntegerv", ::glGetIntegerv, is_failed);
	resolve_symbol("glGetString", ::glGetString, is_failed);
	resolve_symbol("glGetTexImage", ::glGetTexImage, is_failed);
	resolve_symbol("glGetTexParameterfv", ::glGetTexParameterfv, is_failed);
	resolve_symbol("glGetTexParameteriv", ::glGetTexParameteriv, is_failed);
	resolve_symbol("glGetTexLevelParameterfv", ::glGetTexLevelParameterfv, is_failed);
	resolve_symbol("glGetTexLevelParameteriv", ::glGetTexLevelParameteriv, is_failed);
	resolve_symbol("glIsEnabled", ::glIsEnabled, is_failed);
	resolve_symbol("glDepthRange", ::glDepthRange, is_failed);
	resolve_symbol("glViewport", ::glViewport, is_failed);
	resolve_symbol("glNewList", ::glNewList, is_failed);
	resolve_symbol("glEndList", ::glEndList, is_failed);
	resolve_symbol("glCallList", ::glCallList, is_failed);
	resolve_symbol("glCallLists", ::glCallLists, is_failed);
	resolve_symbol("glDeleteLists", ::glDeleteLists, is_failed);
	resolve_symbol("glGenLists", ::glGenLists, is_failed);
	resolve_symbol("glListBase", ::glListBase, is_failed);
	resolve_symbol("glBegin", ::glBegin, is_failed);
	resolve_symbol("glBitmap", ::glBitmap, is_failed);
	resolve_symbol("glColor3b", ::glColor3b, is_failed);
	resolve_symbol("glColor3bv", ::glColor3bv, is_failed);
	resolve_symbol("glColor3d", ::glColor3d, is_failed);
	resolve_symbol("glColor3dv", ::glColor3dv, is_failed);
	resolve_symbol("glColor3f", ::glColor3f, is_failed);
	resolve_symbol("glColor3fv", ::glColor3fv, is_failed);
	resolve_symbol("glColor3i", ::glColor3i, is_failed);
	resolve_symbol("glColor3iv", ::glColor3iv, is_failed);
	resolve_symbol("glColor3s", ::glColor3s, is_failed);
	resolve_symbol("glColor3sv", ::glColor3sv, is_failed);
	resolve_symbol("glColor3ub", ::glColor3ub, is_failed);
	resolve_symbol("glColor3ubv", ::glColor3ubv, is_failed);
	resolve_symbol("glColor3ui", ::glColor3ui, is_failed);
	resolve_symbol("glColor3uiv", ::glColor3uiv, is_failed);
	resolve_symbol("glColor3us", ::glColor3us, is_failed);
	resolve_symbol("glColor3usv", ::glColor3usv, is_failed);
	resolve_symbol("glColor4b", ::glColor4b, is_failed);
	resolve_symbol("glColor4bv", ::glColor4bv, is_failed);
	resolve_symbol("glColor4d", ::glColor4d, is_failed);
	resolve_symbol("glColor4dv", ::glColor4dv, is_failed);
	resolve_symbol("glColor4f", ::glColor4f, is_failed);
	resolve_symbol("glColor4fv", ::glColor4fv, is_failed);
	resolve_symbol("glColor4i", ::glColor4i, is_failed);
	resolve_symbol("glColor4iv", ::glColor4iv, is_failed);
	resolve_symbol("glColor4s", ::glColor4s, is_failed);
	resolve_symbol("glColor4sv", ::glColor4sv, is_failed);
	resolve_symbol("glColor4ub", ::glColor4ub, is_failed);
	resolve_symbol("glColor4ubv", ::glColor4ubv, is_failed);
	resolve_symbol("glColor4ui", ::glColor4ui, is_failed);
	resolve_symbol("glColor4uiv", ::glColor4uiv, is_failed);
	resolve_symbol("glColor4us", ::glColor4us, is_failed);
	resolve_symbol("glColor4usv", ::glColor4usv, is_failed);
	resolve_symbol("glEdgeFlag", ::glEdgeFlag, is_failed);
	resolve_symbol("glEdgeFlagv", ::glEdgeFlagv, is_failed);
	resolve_symbol("glEnd", ::glEnd, is_failed);
	resolve_symbol("glIndexd", ::glIndexd, is_failed);
	resolve_symbol("glIndexdv", ::glIndexdv, is_failed);
	resolve_symbol("glIndexf", ::glIndexf, is_failed);
	resolve_symbol("glIndexfv", ::glIndexfv, is_failed);
	resolve_symbol("glIndexi", ::glIndexi, is_failed);
	resolve_symbol("glIndexiv", ::glIndexiv, is_failed);
	resolve_symbol("glIndexs", ::glIndexs, is_failed);
	resolve_symbol("glIndexsv", ::glIndexsv, is_failed);
	resolve_symbol("glNormal3b", ::glNormal3b, is_failed);
	resolve_symbol("glNormal3bv", ::glNormal3bv, is_failed);
	resolve_symbol("glNormal3d", ::glNormal3d, is_failed);
	resolve_symbol("glNormal3dv", ::glNormal3dv, is_failed);
	resolve_symbol("glNormal3f", ::glNormal3f, is_failed);
	resolve_symbol("glNormal3fv", ::glNormal3fv, is_failed);
	resolve_symbol("glNormal3i", ::glNormal3i, is_failed);
	resolve_symbol("glNormal3iv", ::glNormal3iv, is_failed);
	resolve_symbol("glNormal3s", ::glNormal3s, is_failed);
	resolve_symbol("glNormal3sv", ::glNormal3sv, is_failed);
	resolve_symbol("glRasterPos2d", ::glRasterPos2d, is_failed);
	resolve_symbol("glRasterPos2dv", ::glRasterPos2dv, is_failed);
	resolve_symbol("glRasterPos2f", ::glRasterPos2f, is_failed);
	resolve_symbol("glRasterPos2fv", ::glRasterPos2fv, is_failed);
	resolve_symbol("glRasterPos2i", ::glRasterPos2i, is_failed);
	resolve_symbol("glRasterPos2iv", ::glRasterPos2iv, is_failed);
	resolve_symbol("glRasterPos2s", ::glRasterPos2s, is_failed);
	resolve_symbol("glRasterPos2sv", ::glRasterPos2sv, is_failed);
	resolve_symbol("glRasterPos3d", ::glRasterPos3d, is_failed);
	resolve_symbol("glRasterPos3dv", ::glRasterPos3dv, is_failed);
	resolve_symbol("glRasterPos3f", ::glRasterPos3f, is_failed);
	resolve_symbol("glRasterPos3fv", ::glRasterPos3fv, is_failed);
	resolve_symbol("glRasterPos3i", ::glRasterPos3i, is_failed);
	resolve_symbol("glRasterPos3iv", ::glRasterPos3iv, is_failed);
	resolve_symbol("glRasterPos3s", ::glRasterPos3s, is_failed);
	resolve_symbol("glRasterPos3sv", ::glRasterPos3sv, is_failed);
	resolve_symbol("glRasterPos4d", ::glRasterPos4d, is_failed);
	resolve_symbol("glRasterPos4dv", ::glRasterPos4dv, is_failed);
	resolve_symbol("glRasterPos4f", ::glRasterPos4f, is_failed);
	resolve_symbol("glRasterPos4fv", ::glRasterPos4fv, is_failed);
	resolve_symbol("glRasterPos4i", ::glRasterPos4i, is_failed);
	resolve_symbol("glRasterPos4iv", ::glRasterPos4iv, is_failed);
	resolve_symbol("glRasterPos4s", ::glRasterPos4s, is_failed);
	resolve_symbol("glRasterPos4sv", ::glRasterPos4sv, is_failed);
	resolve_symbol("glRectd", ::glRectd, is_failed);
	resolve_symbol("glRectdv", ::glRectdv, is_failed);
	resolve_symbol("glRectf", ::glRectf, is_failed);
	resolve_symbol("glRectfv", ::glRectfv, is_failed);
	resolve_symbol("glRecti", ::glRecti, is_failed);
	resolve_symbol("glRectiv", ::glRectiv, is_failed);
	resolve_symbol("glRects", ::glRects, is_failed);
	resolve_symbol("glRectsv", ::glRectsv, is_failed);
	resolve_symbol("glTexCoord1d", ::glTexCoord1d, is_failed);
	resolve_symbol("glTexCoord1dv", ::glTexCoord1dv, is_failed);
	resolve_symbol("glTexCoord1f", ::glTexCoord1f, is_failed);
	resolve_symbol("glTexCoord1fv", ::glTexCoord1fv, is_failed);
	resolve_symbol("glTexCoord1i", ::glTexCoord1i, is_failed);
	resolve_symbol("glTexCoord1iv", ::glTexCoord1iv, is_failed);
	resolve_symbol("glTexCoord1s", ::glTexCoord1s, is_failed);
	resolve_symbol("glTexCoord1sv", ::glTexCoord1sv, is_failed);
	resolve_symbol("glTexCoord2d", ::glTexCoord2d, is_failed);
	resolve_symbol("glTexCoord2dv", ::glTexCoord2dv, is_failed);
	resolve_symbol("glTexCoord2f", ::glTexCoord2f, is_failed);
	resolve_symbol("glTexCoord2fv", ::glTexCoord2fv, is_failed);
	resolve_symbol("glTexCoord2i", ::glTexCoord2i, is_failed);
	resolve_symbol("glTexCoord2iv", ::glTexCoord2iv, is_failed);
	resolve_symbol("glTexCoord2s", ::glTexCoord2s, is_failed);
	resolve_symbol("glTexCoord2sv", ::glTexCoord2sv, is_failed);
	resolve_symbol("glTexCoord3d", ::glTexCoord3d, is_failed);
	resolve_symbol("glTexCoord3dv", ::glTexCoord3dv, is_failed);
	resolve_symbol("glTexCoord3f", ::glTexCoord3f, is_failed);
	resolve_symbol("glTexCoord3fv", ::glTexCoord3fv, is_failed);
	resolve_symbol("glTexCoord3i", ::glTexCoord3i, is_failed);
	resolve_symbol("glTexCoord3iv", ::glTexCoord3iv, is_failed);
	resolve_symbol("glTexCoord3s", ::glTexCoord3s, is_failed);
	resolve_symbol("glTexCoord3sv", ::glTexCoord3sv, is_failed);
	resolve_symbol("glTexCoord4d", ::glTexCoord4d, is_failed);
	resolve_symbol("glTexCoord4dv", ::glTexCoord4dv, is_failed);
	resolve_symbol("glTexCoord4f", ::glTexCoord4f, is_failed);
	resolve_symbol("glTexCoord4fv", ::glTexCoord4fv, is_failed);
	resolve_symbol("glTexCoord4i", ::glTexCoord4i, is_failed);
	resolve_symbol("glTexCoord4iv", ::glTexCoord4iv, is_failed);
	resolve_symbol("glTexCoord4s", ::glTexCoord4s, is_failed);
	resolve_symbol("glTexCoord4sv", ::glTexCoord4sv, is_failed);
	resolve_symbol("glVertex2d", ::glVertex2d, is_failed);
	resolve_symbol("glVertex2dv", ::glVertex2dv, is_failed);
	resolve_symbol("glVertex2f", ::glVertex2f, is_failed);
	resolve_symbol("glVertex2fv", ::glVertex2fv, is_failed);
	resolve_symbol("glVertex2i", ::glVertex2i, is_failed);
	resolve_symbol("glVertex2iv", ::glVertex2iv, is_failed);
	resolve_symbol("glVertex2s", ::glVertex2s, is_failed);
	resolve_symbol("glVertex2sv", ::glVertex2sv, is_failed);
	resolve_symbol("glVertex3d", ::glVertex3d, is_failed);
	resolve_symbol("glVertex3dv", ::glVertex3dv, is_failed);
	resolve_symbol("glVertex3f", ::glVertex3f, is_failed);
	resolve_symbol("glVertex3fv", ::glVertex3fv, is_failed);
	resolve_symbol("glVertex3i", ::glVertex3i, is_failed);
	resolve_symbol("glVertex3iv", ::glVertex3iv, is_failed);
	resolve_symbol("glVertex3s", ::glVertex3s, is_failed);
	resolve_symbol("glVertex3sv", ::glVertex3sv, is_failed);
	resolve_symbol("glVertex4d", ::glVertex4d, is_failed);
	resolve_symbol("glVertex4dv", ::glVertex4dv, is_failed);
	resolve_symbol("glVertex4f", ::glVertex4f, is_failed);
	resolve_symbol("glVertex4fv", ::glVertex4fv, is_failed);
	resolve_symbol("glVertex4i", ::glVertex4i, is_failed);
	resolve_symbol("glVertex4iv", ::glVertex4iv, is_failed);
	resolve_symbol("glVertex4s", ::glVertex4s, is_failed);
	resolve_symbol("glVertex4sv", ::glVertex4sv, is_failed);
	resolve_symbol("glClipPlane", ::glClipPlane, is_failed);
	resolve_symbol("glColorMaterial", ::glColorMaterial, is_failed);
	resolve_symbol("glFogf", ::glFogf, is_failed);
	resolve_symbol("glFogfv", ::glFogfv, is_failed);
	resolve_symbol("glFogi", ::glFogi, is_failed);
	resolve_symbol("glFogiv", ::glFogiv, is_failed);
	resolve_symbol("glLightf", ::glLightf, is_failed);
	resolve_symbol("glLightfv", ::glLightfv, is_failed);
	resolve_symbol("glLighti", ::glLighti, is_failed);
	resolve_symbol("glLightiv", ::glLightiv, is_failed);
	resolve_symbol("glLightModelf", ::glLightModelf, is_failed);
	resolve_symbol("glLightModelfv", ::glLightModelfv, is_failed);
	resolve_symbol("glLightModeli", ::glLightModeli, is_failed);
	resolve_symbol("glLightModeliv", ::glLightModeliv, is_failed);
	resolve_symbol("glLineStipple", ::glLineStipple, is_failed);
	resolve_symbol("glMaterialf", ::glMaterialf, is_failed);
	resolve_symbol("glMaterialfv", ::glMaterialfv, is_failed);
	resolve_symbol("glMateriali", ::glMateriali, is_failed);
	resolve_symbol("glMaterialiv", ::glMaterialiv, is_failed);
	resolve_symbol("glPolygonStipple", ::glPolygonStipple, is_failed);
	resolve_symbol("glShadeModel", ::glShadeModel, is_failed);
	resolve_symbol("glTexEnvf", ::glTexEnvf, is_failed);
	resolve_symbol("glTexEnvfv", ::glTexEnvfv, is_failed);
	resolve_symbol("glTexEnvi", ::glTexEnvi, is_failed);
	resolve_symbol("glTexEnviv", ::glTexEnviv, is_failed);
	resolve_symbol("glTexGend", ::glTexGend, is_failed);
	resolve_symbol("glTexGendv", ::glTexGendv, is_failed);
	resolve_symbol("glTexGenf", ::glTexGenf, is_failed);
	resolve_symbol("glTexGenfv", ::glTexGenfv, is_failed);
	resolve_symbol("glTexGeni", ::glTexGeni, is_failed);
	resolve_symbol("glTexGeniv", ::glTexGeniv, is_failed);
	resolve_symbol("glFeedbackBuffer", ::glFeedbackBuffer, is_failed);
	resolve_symbol("glSelectBuffer", ::glSelectBuffer, is_failed);
	resolve_symbol("glRenderMode", ::glRenderMode, is_failed);
	resolve_symbol("glInitNames", ::glInitNames, is_failed);
	resolve_symbol("glLoadName", ::glLoadName, is_failed);
	resolve_symbol("glPassThrough", ::glPassThrough, is_failed);
	resolve_symbol("glPopName", ::glPopName, is_failed);
	resolve_symbol("glPushName", ::glPushName, is_failed);
	resolve_symbol("glClearAccum", ::glClearAccum, is_failed);
	resolve_symbol("glClearIndex", ::glClearIndex, is_failed);
	resolve_symbol("glIndexMask", ::glIndexMask, is_failed);
	resolve_symbol("glAccum", ::glAccum, is_failed);
	resolve_symbol("glPopAttrib", ::glPopAttrib, is_failed);
	resolve_symbol("glPushAttrib", ::glPushAttrib, is_failed);
	resolve_symbol("glMap1d", ::glMap1d, is_failed);
	resolve_symbol("glMap1f", ::glMap1f, is_failed);
	resolve_symbol("glMap2d", ::glMap2d, is_failed);
	resolve_symbol("glMap2f", ::glMap2f, is_failed);
	resolve_symbol("glMapGrid1d", ::glMapGrid1d, is_failed);
	resolve_symbol("glMapGrid1f", ::glMapGrid1f, is_failed);
	resolve_symbol("glMapGrid2d", ::glMapGrid2d, is_failed);
	resolve_symbol("glMapGrid2f", ::glMapGrid2f, is_failed);
	resolve_symbol("glEvalCoord1d", ::glEvalCoord1d, is_failed);
	resolve_symbol("glEvalCoord1dv", ::glEvalCoord1dv, is_failed);
	resolve_symbol("glEvalCoord1f", ::glEvalCoord1f, is_failed);
	resolve_symbol("glEvalCoord1fv", ::glEvalCoord1fv, is_failed);
	resolve_symbol("glEvalCoord2d", ::glEvalCoord2d, is_failed);
	resolve_symbol("glEvalCoord2dv", ::glEvalCoord2dv, is_failed);
	resolve_symbol("glEvalCoord2f", ::glEvalCoord2f, is_failed);
	resolve_symbol("glEvalCoord2fv", ::glEvalCoord2fv, is_failed);
	resolve_symbol("glEvalMesh1", ::glEvalMesh1, is_failed);
	resolve_symbol("glEvalPoint1", ::glEvalPoint1, is_failed);
	resolve_symbol("glEvalMesh2", ::glEvalMesh2, is_failed);
	resolve_symbol("glEvalPoint2", ::glEvalPoint2, is_failed);
	resolve_symbol("glAlphaFunc", ::glAlphaFunc, is_failed);
	resolve_symbol("glPixelZoom", ::glPixelZoom, is_failed);
	resolve_symbol("glPixelTransferf", ::glPixelTransferf, is_failed);
	resolve_symbol("glPixelTransferi", ::glPixelTransferi, is_failed);
	resolve_symbol("glPixelMapfv", ::glPixelMapfv, is_failed);
	resolve_symbol("glPixelMapuiv", ::glPixelMapuiv, is_failed);
	resolve_symbol("glPixelMapusv", ::glPixelMapusv, is_failed);
	resolve_symbol("glCopyPixels", ::glCopyPixels, is_failed);
	resolve_symbol("glDrawPixels", ::glDrawPixels, is_failed);
	resolve_symbol("glGetClipPlane", ::glGetClipPlane, is_failed);
	resolve_symbol("glGetLightfv", ::glGetLightfv, is_failed);
	resolve_symbol("glGetLightiv", ::glGetLightiv, is_failed);
	resolve_symbol("glGetMapdv", ::glGetMapdv, is_failed);
	resolve_symbol("glGetMapfv", ::glGetMapfv, is_failed);
	resolve_symbol("glGetMapiv", ::glGetMapiv, is_failed);
	resolve_symbol("glGetMaterialfv", ::glGetMaterialfv, is_failed);
	resolve_symbol("glGetMaterialiv", ::glGetMaterialiv, is_failed);
	resolve_symbol("glGetPixelMapfv", ::glGetPixelMapfv, is_failed);
	resolve_symbol("glGetPixelMapuiv", ::glGetPixelMapuiv, is_failed);
	resolve_symbol("glGetPixelMapusv", ::glGetPixelMapusv, is_failed);
	resolve_symbol("glGetPolygonStipple", ::glGetPolygonStipple, is_failed);
	resolve_symbol("glGetTexEnvfv", ::glGetTexEnvfv, is_failed);
	resolve_symbol("glGetTexEnviv", ::glGetTexEnviv, is_failed);
	resolve_symbol("glGetTexGendv", ::glGetTexGendv, is_failed);
	resolve_symbol("glGetTexGenfv", ::glGetTexGenfv, is_failed);
	resolve_symbol("glGetTexGeniv", ::glGetTexGeniv, is_failed);
	resolve_symbol("glIsList", ::glIsList, is_failed);
	resolve_symbol("glFrustum", ::glFrustum, is_failed);
	resolve_symbol("glLoadIdentity", ::glLoadIdentity, is_failed);
	resolve_symbol("glLoadMatrixf", ::glLoadMatrixf, is_failed);
	resolve_symbol("glLoadMatrixd", ::glLoadMatrixd, is_failed);
	resolve_symbol("glMatrixMode", ::glMatrixMode, is_failed);
	resolve_symbol("glMultMatrixf", ::glMultMatrixf, is_failed);
	resolve_symbol("glMultMatrixd", ::glMultMatrixd, is_failed);
	resolve_symbol("glOrtho", ::glOrtho, is_failed);
	resolve_symbol("glPopMatrix", ::glPopMatrix, is_failed);
	resolve_symbol("glPushMatrix", ::glPushMatrix, is_failed);
	resolve_symbol("glRotated", ::glRotated, is_failed);
	resolve_symbol("glRotatef", ::glRotatef, is_failed);
	resolve_symbol("glScaled", ::glScaled, is_failed);
	resolve_symbol("glScalef", ::glScalef, is_failed);
	resolve_symbol("glTranslated", ::glTranslated, is_failed);
	resolve_symbol("glTranslatef", ::glTranslatef, is_failed);

	if (is_failed)
	{
		clear_unique_symbols_1_0();

		return false;
	}

	return true;
}

void OglRendererUtils::clear_unique_symbols_1_1()
{
	::glDrawArrays = nullptr;
	::glDrawElements = nullptr;
	::glGetPointerv = nullptr;
	::glPolygonOffset = nullptr;
	::glCopyTexImage1D = nullptr;
	::glCopyTexImage2D = nullptr;
	::glCopyTexSubImage1D = nullptr;
	::glCopyTexSubImage2D = nullptr;
	::glTexSubImage1D = nullptr;
	::glTexSubImage2D = nullptr;
	::glBindTexture = nullptr;
	::glDeleteTextures = nullptr;
	::glGenTextures = nullptr;
	::glIsTexture = nullptr;
	::glArrayElement = nullptr;
	::glColorPointer = nullptr;
	::glDisableClientState = nullptr;
	::glEdgeFlagPointer = nullptr;
	::glEnableClientState = nullptr;
	::glIndexPointer = nullptr;
	::glInterleavedArrays = nullptr;
	::glNormalPointer = nullptr;
	::glTexCoordPointer = nullptr;
	::glVertexPointer = nullptr;
	::glAreTexturesResident = nullptr;
	::glPrioritizeTextures = nullptr;
	::glIndexub = nullptr;
	::glIndexubv = nullptr;
	::glPopClientAttrib = nullptr;
	::glPushClientAttrib = nullptr;
}

bool OglRendererUtils::resolve_unique_symbols_1_1()
{
	auto is_failed = false;

	resolve_symbol("glDrawArrays", ::glDrawArrays, is_failed);
	resolve_symbol("glDrawElements", ::glDrawElements, is_failed);
	resolve_symbol("glGetPointerv", ::glGetPointerv, is_failed);
	resolve_symbol("glPolygonOffset", ::glPolygonOffset, is_failed);
	resolve_symbol("glCopyTexImage1D", ::glCopyTexImage1D, is_failed);
	resolve_symbol("glCopyTexImage2D", ::glCopyTexImage2D, is_failed);
	resolve_symbol("glCopyTexSubImage1D", ::glCopyTexSubImage1D, is_failed);
	resolve_symbol("glCopyTexSubImage2D", ::glCopyTexSubImage2D, is_failed);
	resolve_symbol("glTexSubImage1D", ::glTexSubImage1D, is_failed);
	resolve_symbol("glTexSubImage2D", ::glTexSubImage2D, is_failed);
	resolve_symbol("glBindTexture", ::glBindTexture, is_failed);
	resolve_symbol("glDeleteTextures", ::glDeleteTextures, is_failed);
	resolve_symbol("glGenTextures", ::glGenTextures, is_failed);
	resolve_symbol("glIsTexture", ::glIsTexture, is_failed);
	resolve_symbol("glArrayElement", ::glArrayElement, is_failed);
	resolve_symbol("glColorPointer", ::glColorPointer, is_failed);
	resolve_symbol("glDisableClientState", ::glDisableClientState, is_failed);
	resolve_symbol("glEdgeFlagPointer", ::glEdgeFlagPointer, is_failed);
	resolve_symbol("glEnableClientState", ::glEnableClientState, is_failed);
	resolve_symbol("glIndexPointer", ::glIndexPointer, is_failed);
	resolve_symbol("glInterleavedArrays", ::glInterleavedArrays, is_failed);
	resolve_symbol("glNormalPointer", ::glNormalPointer, is_failed);
	resolve_symbol("glTexCoordPointer", ::glTexCoordPointer, is_failed);
	resolve_symbol("glVertexPointer", ::glVertexPointer, is_failed);
	resolve_symbol("glAreTexturesResident", ::glAreTexturesResident, is_failed);
	resolve_symbol("glPrioritizeTextures", ::glPrioritizeTextures, is_failed);
	resolve_symbol("glIndexub", ::glIndexub, is_failed);
	resolve_symbol("glIndexubv", ::glIndexubv, is_failed);
	resolve_symbol("glPopClientAttrib", ::glPopClientAttrib, is_failed);
	resolve_symbol("glPushClientAttrib", ::glPushClientAttrib, is_failed);

	if (is_failed)
	{
		clear_unique_symbols_1_1();

		return false;
	}

	return true;
}

RendererUtilsExtensions OglRendererUtils::extensions_get_core()
{
	throw "Not implemented.";
}

RendererUtilsExtensions OglRendererUtils::extensions_get_compatibility()
{
	const auto ogl_extensions_string = ::glGetString(GL_EXTENSIONS);

	if (was_errors())
	{
		assert(!"No context or core profile.");

		return {};
	}

	if (!ogl_extensions_string)
	{
		return {};
	}

	const auto& ogl_extensions_std_string = std::string
	{
		reinterpret_cast<const char*>(ogl_extensions_string)
	};

	const auto extension_count = 1 + static_cast<int>(std::count(
		ogl_extensions_std_string.cbegin(),
		ogl_extensions_std_string.cend(),
		' '
	));

	auto iss = std::istringstream{ogl_extensions_std_string};

	auto result = RendererUtilsExtensions{};
	result.reserve(extension_count);

	result.assign(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}
	);

	return result;
}

//
// OglRendererUtils
// ==========================================================================


} // detail
} // bstone
