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
// OpenGL extension manager (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_extension_manager.h"
#include <cassert>
#include <bitset>
#include <sstream>
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_sdl_types.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglExtensionManagerImpl
//

class OglExtensionManagerImpl :
	public OglExtensionManager
{
public:
	OglExtensionManagerImpl();

	OglExtensionManagerImpl(
		const OglExtensionManagerImpl& rhs) = delete;

	OglExtensionManagerImpl(
		OglExtensionManagerImpl&& rhs);

	~OglExtensionManagerImpl() override;


	int get_extension_count() const override;

	const std::string& get_extension_name(
		const int extension_index) const override;


	void probe_extension(
		const OglExtensionId extension_id) override;


	bool has_extension(
		const OglExtensionId extension_id) const override;

	bool operator[](
		const OglExtensionId extension_id) const override;


	bool initialize();


private:
	static constexpr auto max_extensions = 16;


	using ExtensionNames = std::vector<std::string>;
	using ProbedExtensions = std::bitset<max_extensions>;
	using AvailableExtensions = std::bitset<max_extensions>;
	using ResolveSymbolsFunction = bool (OglExtensionManagerImpl::*)();


	ExtensionNames extension_names_;
	ProbedExtensions probed_extensions_;
	AvailableExtensions available_extensions_;


	static const std::string& get_empty_extension_name();

	static int get_extension_index(
		const OglExtensionId extension_id);

	static const char* get_extension_name(
		const OglExtensionId extension_id);


	template<typename T>
	static void resolve_symbol(
		const char* const name,
		T& symbol,
		bool& is_failed)
	{
		symbol = reinterpret_cast<T>(OglRendererUtils::resolve_symbol(name));

		if (symbol == nullptr)
		{
			is_failed = true;
		}
	}

	void get_core_extension_names();

	void get_compatibility_extension_names();

	void get_extension_names();


	void probe_generic(
		const OglExtensionId extension_id,
		ResolveSymbolsFunction resolve_symbols_function);


	void clear_v1_0();

	bool resolve_v1_0();


	void clear_v1_1();

	bool resolve_v1_1();


	void clear_arb_framebuffer_object();

	bool resolve_arb_framebuffer_object();


	void clear_ext_framebuffer_blit();

	bool resolve_ext_framebuffer_blit();


	void clear_ext_framebuffer_multisample();

	bool resolve_ext_framebuffer_multisample();


	void clear_ext_framebuffer_object();

	bool resolve_ext_framebuffer_object();
}; // OglExtensionManagerImpl


using OglExtensionManagerImplPtr = OglExtensionManagerImpl*;
using OglExtensionManagerImplUPtr = std::unique_ptr<OglExtensionManagerImpl>;


OglExtensionManagerImpl::OglExtensionManagerImpl()
	:
	extension_names_{},
	probed_extensions_{},
	available_extensions_{}
{
}

OglExtensionManagerImpl::OglExtensionManagerImpl(
	OglExtensionManagerImpl&& rhs)
	:
	extension_names_{std::move(rhs.extension_names_)},
	probed_extensions_{std::move(rhs.probed_extensions_)},
	available_extensions_{std::move(rhs.available_extensions_)}
{
}

OglExtensionManagerImpl::~OglExtensionManagerImpl()
{
}

bool OglExtensionManagerImpl::initialize()
{
	const auto sdl_gl_context = ::SDL_GL_GetCurrentContext();

	if (sdl_gl_context == nullptr)
	{
		return false;
	}

	get_extension_names();

	return true;
}

int OglExtensionManagerImpl::get_extension_count() const
{
	return static_cast<int>(extension_names_.size());
}

const std::string& OglExtensionManagerImpl::get_extension_name(
	const int extension_index) const
{
	if (extension_index < 0 || extension_index >= get_extension_count())
	{
		return get_empty_extension_name();
	}

	return extension_names_[extension_index];
}

void OglExtensionManagerImpl::probe_extension(
	const OglExtensionId extension_id)
{
	switch (extension_id)
	{
		// ==================================================================
		// Versions.
		//

		case OglExtensionId::v1_0:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_v1_0);
			break;

		case OglExtensionId::v1_1:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_v1_1);
			break;

		//
		// Versions.
		// ==================================================================


		// ==================================================================
		// ARB
		//

		case OglExtensionId::arb_framebuffer_object:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_arb_framebuffer_object);
			break;

		case OglExtensionId::arb_texture_filter_anisotropic:
		case OglExtensionId::arb_texture_non_power_of_two:
			probe_generic(extension_id, nullptr);
			break;

		//
		// ARB
		// ==================================================================


		// ==================================================================
		// EXT
		//

		case OglExtensionId::ext_framebuffer_blit:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_ext_framebuffer_blit);
			break;

		case OglExtensionId::ext_framebuffer_multisample:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_ext_framebuffer_multisample);
			break;

		case OglExtensionId::ext_framebuffer_object:
			probe_generic(extension_id, &OglExtensionManagerImpl::resolve_ext_framebuffer_object);
			break;

		case OglExtensionId::ext_packed_depth_stencil:
			probe_generic(extension_id, nullptr);
			break;

		case OglExtensionId::ext_texture_filter_anisotropic:
			probe_generic(extension_id, nullptr);
			break;

		//
		// EXT
		// ==================================================================


		// ==================================================================
		// SGIS
		//

		case OglExtensionId::sgis_generate_mipmap:
			probe_generic(extension_id, nullptr);
			break;

		//
		// SGIS
		// ==================================================================


		default:
			assert(!"Invalid extension id.");
			break;
	}
}

bool OglExtensionManagerImpl::has_extension(
	const OglExtensionId extension_id) const
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	return available_extensions_[extension_index];
}

bool OglExtensionManagerImpl::operator[](
	const OglExtensionId extension_id) const
{
	return has_extension(extension_id);
}

const std::string& OglExtensionManagerImpl::get_empty_extension_name()
{
	static const auto result = std::string{};

	return result;
}

int OglExtensionManagerImpl::get_extension_index(
	const OglExtensionId extension_id)
{
	switch (extension_id)
	{
		case OglExtensionId::v1_0:
		case OglExtensionId::v1_1:
		case OglExtensionId::arb_framebuffer_object:
		case OglExtensionId::arb_texture_filter_anisotropic:
		case OglExtensionId::arb_texture_non_power_of_two:
		case OglExtensionId::ext_framebuffer_blit:
		case OglExtensionId::ext_framebuffer_multisample:
		case OglExtensionId::ext_framebuffer_object:
		case OglExtensionId::ext_packed_depth_stencil:
		case OglExtensionId::ext_texture_filter_anisotropic:
		case OglExtensionId::sgis_generate_mipmap:
			return static_cast<int>(extension_id);


		default:
			return -1;
	}
}

const char* OglExtensionManagerImpl::get_extension_name(
	const OglExtensionId extension_id)
{
	switch (extension_id)
	{
		case OglExtensionId::v1_0:
		case OglExtensionId::v1_1:
			return "";

		case OglExtensionId::arb_framebuffer_object:
			return "GL_ARB_framebuffer_object";

		case OglExtensionId::arb_texture_filter_anisotropic:
			return "GL_ARB_texture_filter_anisotropic";

		case OglExtensionId::arb_texture_non_power_of_two:
			return "GL_ARB_texture_non_power_of_two";

		case OglExtensionId::ext_framebuffer_blit:
			return "GL_EXT_framebuffer_blit";

		case OglExtensionId::ext_framebuffer_multisample:
			return "GL_EXT_framebuffer_multisample";

		case OglExtensionId::ext_framebuffer_object:
			return "GL_EXT_framebuffer_object";

		case OglExtensionId::ext_packed_depth_stencil:
			return "GL_EXT_packed_depth_stencil";

		case OglExtensionId::ext_texture_filter_anisotropic:
			return "GL_EXT_texture_filter_anisotropic";

		case OglExtensionId::sgis_generate_mipmap:
			return "GL_SGIS_generate_mipmap";


		default:
			return nullptr;
	}
}

void OglExtensionManagerImpl::get_core_extension_names()
{
	auto is_failed = false;
	auto gl_get_integer_v = PFNGLGETINTEGERVPROC{};
	auto gl_get_string_i = PFNGLGETSTRINGIPROC{};

	resolve_symbol("glGetIntegerv", gl_get_integer_v, is_failed);
	resolve_symbol("glGetStringi", gl_get_string_i, is_failed);

	if (is_failed)
	{
		return;
	}

	auto ogl_extension_count = GLint{};

	gl_get_integer_v(GL_NUM_EXTENSIONS, &ogl_extension_count);

	if (ogl_extension_count == 0)
	{
		return;
	}

	auto extension_names = ExtensionNames{};
	extension_names.resize(ogl_extension_count);

	for (auto i = GLint{}; i < ogl_extension_count; ++i)
	{
		const auto extension_name = gl_get_string_i(GL_EXTENSIONS, static_cast<GLuint>(i));

		if (extension_name == nullptr)
		{
			return;
		}

		extension_names[i] = reinterpret_cast<const char*>(extension_name);
	}

	std::swap(extension_names_, extension_names);
}

void OglExtensionManagerImpl::get_compatibility_extension_names()
{
	auto is_failed = false;
	auto gl_get_string = PFNGLGETSTRINGPROC{};

	resolve_symbol("glGetString", gl_get_string, is_failed);

	if (is_failed)
	{
		return;
	}

	const auto ogl_extensions_c_string = gl_get_string(GL_EXTENSIONS);

	if (ogl_extensions_c_string == nullptr)
	{
		return;
	}

	const auto& ogl_extensions_std_string = std::string
	{
		reinterpret_cast<const char*>(ogl_extensions_c_string)
	};

	const auto extension_count = 1 + std::count(
		ogl_extensions_std_string.cbegin(),
		ogl_extensions_std_string.cend(),
		' '
	);

	auto iss = std::istringstream{ogl_extensions_std_string};

	extension_names_.reserve(extension_count);

	extension_names_.assign(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}
	);
}

void OglExtensionManagerImpl::get_extension_names()
{
	const auto context_type = OglRendererUtils::context_get_type();

	if (context_type == OglRendererUtilsContextType::invalid)
	{
		return;
	}

	auto is_core = false;

	if (context_type == OglRendererUtilsContextType::es)
	{
		int major_version = 0;
		int minor_version = 0;

		if (!OglRendererUtils::context_get_version(major_version, minor_version))
		{
			return;
		}

		if (major_version >= 3)
		{
			is_core = true;
		}
	}
	else if (context_type == OglRendererUtilsContextType::core)
	{
		is_core = true;
	}

	if (is_core)
	{
		get_core_extension_names();
	}
	else
	{
		get_compatibility_extension_names();
	}

	std::sort(extension_names_.begin(), extension_names_.end());
}

void OglExtensionManagerImpl::probe_generic(
	const OglExtensionId extension_id,
	ResolveSymbolsFunction resolve_symbols_function)
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		assert("Invalid extension id.");

		return;
	}

	if (probed_extensions_.test(extension_index))
	{
		return;
	}

	probed_extensions_.set(extension_index);

	const auto extension_name = get_extension_name(extension_id);

	if (extension_name == nullptr)
	{
		assert("Null extension name.");

		return;
	}

	const auto is_version = (*extension_name == '\0');

	if (is_version && resolve_symbols_function == nullptr)
	{
		assert("Expected symbols loader for specific version.");

		return;
	}

	if (!is_version)
	{
		const auto has_extension_name = std::any_of(
			extension_names_.cbegin(),
			extension_names_.cend(),
			[=](const auto& item)
			{
				return item == extension_name;
			}
		);

		if (!has_extension_name)
		{
			return;
		}
	}

	if (resolve_symbols_function != nullptr)
	{
		if (!(this->*resolve_symbols_function)())
		{
			return;
		}
	}

	available_extensions_.set(extension_index);
}

void OglExtensionManagerImpl::clear_v1_0()
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

bool OglExtensionManagerImpl::resolve_v1_0()
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
		clear_v1_0();

		return false;
	}

	return true;
}

void OglExtensionManagerImpl::clear_v1_1()
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

bool OglExtensionManagerImpl::resolve_v1_1()
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
		clear_v1_1();

		return false;
	}

	return true;
}

void OglExtensionManagerImpl::clear_arb_framebuffer_object()
{
	::glIsRenderbuffer = nullptr;
	::glBindRenderbuffer = nullptr;
	::glDeleteRenderbuffers = nullptr;
	::glGenRenderbuffers = nullptr;
	::glRenderbufferStorage = nullptr;
	::glGetRenderbufferParameteriv = nullptr;
	::glIsFramebuffer = nullptr;
	::glBindFramebuffer = nullptr;
	::glDeleteFramebuffers = nullptr;
	::glGenFramebuffers = nullptr;
	::glCheckFramebufferStatus = nullptr;
	::glFramebufferTexture1D = nullptr;
	::glFramebufferTexture2D = nullptr;
	::glFramebufferTexture3D = nullptr;
	::glFramebufferRenderbuffer = nullptr;
	::glGetFramebufferAttachmentParameteriv = nullptr;
	::glGenerateMipmap = nullptr;
	::glBlitFramebuffer = nullptr;
	::glRenderbufferStorageMultisample = nullptr;
	::glFramebufferTextureLayer = nullptr;
}

bool OglExtensionManagerImpl::resolve_arb_framebuffer_object()
{
	auto is_failed = false;

	resolve_symbol("glIsRenderbuffer", ::glIsRenderbuffer, is_failed);
	resolve_symbol("glBindRenderbuffer", ::glBindRenderbuffer, is_failed);
	resolve_symbol("glDeleteRenderbuffers", ::glDeleteRenderbuffers, is_failed);
	resolve_symbol("glGenRenderbuffers", ::glGenRenderbuffers, is_failed);
	resolve_symbol("glRenderbufferStorage", ::glRenderbufferStorage, is_failed);
	resolve_symbol("glGetRenderbufferParameteriv", ::glGetRenderbufferParameteriv, is_failed);
	resolve_symbol("glIsFramebuffer", ::glIsFramebuffer, is_failed);
	resolve_symbol("glBindFramebuffer", ::glBindFramebuffer, is_failed);
	resolve_symbol("glDeleteFramebuffers", ::glDeleteFramebuffers, is_failed);
	resolve_symbol("glGenFramebuffers", ::glGenFramebuffers, is_failed);
	resolve_symbol("glCheckFramebufferStatus", ::glCheckFramebufferStatus, is_failed);
	resolve_symbol("glFramebufferTexture1D", ::glFramebufferTexture1D, is_failed);
	resolve_symbol("glFramebufferTexture2D", ::glFramebufferTexture2D, is_failed);
	resolve_symbol("glFramebufferTexture3D", ::glFramebufferTexture3D, is_failed);
	resolve_symbol("glFramebufferRenderbuffer", ::glFramebufferRenderbuffer, is_failed);
	resolve_symbol("glGetFramebufferAttachmentParameteriv", ::glGetFramebufferAttachmentParameteriv, is_failed);
	resolve_symbol("glGenerateMipmap", ::glGenerateMipmap, is_failed);
	resolve_symbol("glBlitFramebuffer", ::glBlitFramebuffer, is_failed);
	resolve_symbol("glRenderbufferStorageMultisample", ::glRenderbufferStorageMultisample, is_failed);
	resolve_symbol("glFramebufferTextureLayer", ::glFramebufferTextureLayer, is_failed);

	if (is_failed)
	{
		clear_arb_framebuffer_object();

		return false;
	}

	return true;
}

void OglExtensionManagerImpl::clear_ext_framebuffer_blit()
{
	::glBlitFramebufferEXT = nullptr;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_blit()
{
	auto is_failed = false;

	resolve_symbol("glBlitFramebufferEXT", ::glBlitFramebufferEXT, is_failed);

	if (is_failed)
	{
		clear_ext_framebuffer_blit();

		return false;
	}

	return true;
}

void OglExtensionManagerImpl::clear_ext_framebuffer_multisample()
{
	::glRenderbufferStorageMultisampleEXT = nullptr;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_multisample()
{
	auto is_failed = false;

	resolve_symbol("glRenderbufferStorageMultisampleEXT", ::glRenderbufferStorageMultisampleEXT, is_failed);

	if (is_failed)
	{
		clear_ext_framebuffer_multisample();

		return false;
	}

	return true;
}

void OglExtensionManagerImpl::clear_ext_framebuffer_object()
{
	::glIsRenderbufferEXT = nullptr;
	::glBindRenderbufferEXT = nullptr;
	::glDeleteRenderbuffersEXT = nullptr;
	::glGenRenderbuffersEXT = nullptr;
	::glRenderbufferStorageEXT = nullptr;
	::glGetRenderbufferParameterivEXT = nullptr;
	::glIsFramebufferEXT = nullptr;
	::glBindFramebufferEXT = nullptr;
	::glDeleteFramebuffersEXT = nullptr;
	::glGenFramebuffersEXT = nullptr;
	::glCheckFramebufferStatusEXT = nullptr;
	::glFramebufferTexture1DEXT = nullptr;
	::glFramebufferTexture2DEXT = nullptr;
	::glFramebufferTexture3DEXT = nullptr;
	::glFramebufferRenderbufferEXT = nullptr;
	::glGetFramebufferAttachmentParameterivEXT = nullptr;
	::glGenerateMipmapEXT = nullptr;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_object()
{
	auto is_failed = false;

	resolve_symbol("glIsRenderbufferEXT", ::glIsRenderbufferEXT, is_failed);
	resolve_symbol("glBindRenderbufferEXT", ::glBindRenderbufferEXT, is_failed);
	resolve_symbol("glDeleteRenderbuffersEXT", ::glDeleteRenderbuffersEXT, is_failed);
	resolve_symbol("glGenRenderbuffersEXT", ::glGenRenderbuffersEXT, is_failed);
	resolve_symbol("glRenderbufferStorageEXT", ::glRenderbufferStorageEXT, is_failed);
	resolve_symbol("glGetRenderbufferParameterivEXT", ::glGetRenderbufferParameterivEXT, is_failed);
	resolve_symbol("glIsFramebufferEXT", ::glIsFramebufferEXT, is_failed);
	resolve_symbol("glBindFramebufferEXT", ::glBindFramebufferEXT, is_failed);
	resolve_symbol("glDeleteFramebuffersEXT", ::glDeleteFramebuffersEXT, is_failed);
	resolve_symbol("glGenFramebuffersEXT", ::glGenFramebuffersEXT, is_failed);
	resolve_symbol("glCheckFramebufferStatusEXT", ::glCheckFramebufferStatusEXT, is_failed);
	resolve_symbol("glFramebufferTexture1DEXT", ::glFramebufferTexture1DEXT, is_failed);
	resolve_symbol("glFramebufferTexture2DEXT", ::glFramebufferTexture2DEXT, is_failed);
	resolve_symbol("glFramebufferTexture3DEXT", ::glFramebufferTexture3DEXT, is_failed);
	resolve_symbol("glFramebufferRenderbufferEXT", ::glFramebufferRenderbufferEXT, is_failed);
	resolve_symbol("glGetFramebufferAttachmentParameterivEXT", ::glGetFramebufferAttachmentParameterivEXT, is_failed);
	resolve_symbol("glGenerateMipmapEXT", ::glGenerateMipmapEXT, is_failed);

	if (is_failed)
	{
		clear_ext_framebuffer_object();

		return false;
	}

	return true;
}

//
// OglExtensionManagerImpl
// ==========================================================================


// ==========================================================================
// OglExtensionManager
//

OglExtensionManager::OglExtensionManager()
{
}

OglExtensionManager::~OglExtensionManager()
{
}

//
// OglExtensionManager
// ==========================================================================


// ==========================================================================
// OglExtensionManagerFactory
//

OglExtensionManagerUPtr OglExtensionManagerFactory::create()
{
	auto result = OglExtensionManagerImplUPtr{new OglExtensionManagerImpl{}};

	if (!result->initialize())
	{
		return nullptr;
	}

	return result;
}

//
// OglExtensionManagerFactory
// ==========================================================================


} // detail
} // bstone
