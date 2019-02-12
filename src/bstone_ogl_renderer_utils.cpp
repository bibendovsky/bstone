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
#include "bstone_ogl_renderer_utils.h"
#include <cassert>
#include "SDL_video.h"
#include "bstone_ogl.h"


namespace bstone
{


// ==========================================================================
// OglRendererUtils::Detail
//

class OglRendererUtils::Detail
{
public:
	static const int v1_0_unique_symbol_count = 306;
	static const int v1_1_unique_symbol_count = 30;


	static bool load_library(
		std::string& error_message);

	static void unload_library();


	static bool create_context_validate_params(
		SdlWindowPtr sdl_window,
		std::string& error_message);

	static bool create_context(
		SdlWindowPtr sdl_window,
		SdlGlContext& sdl_ogl_context,
		std::string& error_message);


	static bool make_context_current_validate_params(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_ogl_context,
		std::string& error_message);

	static bool make_context_current(
		SdlWindowPtr sdl_window,
		SdlGlContext sdl_ogl_context,
		std::string& error_message);


	static bool create_window_and_context(
		const RendererUtilsCreateWindowParam& param,
		SdlWindowPtr& sdl_window,
		SdlGlContext& sdl_ogl_context,
		std::string& error_message);


	static void* resolve_symbol(
		const char* const symbol);

	template<typename T>
	static void resolve_symbol(
		const char* const name,
		T& symbol,
		Strings& missing_symbols)
	{
		symbol = reinterpret_cast<T>(resolve_symbol(name));

		if (!symbol)
		{
			missing_symbols.emplace_back(name);
		}
	}

	static void clear_unique_symbols_1_0();

	static bool resolve_unique_symbols_1_0(
		Strings& missing_symbols);


	static void clear_unique_symbols_1_1();

	static bool resolve_unique_symbols_1_1(
		Strings& missing_symbols);
}; // OglRendererUtils::Detail


bool OglRendererUtils::Detail::load_library(
	std::string& error_message)
{
	const auto sdl_result = ::SDL_GL_LoadLibrary(nullptr);

	if (sdl_result < 0)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

void OglRendererUtils::Detail::unload_library()
{
	::SDL_GL_UnloadLibrary();
}

bool OglRendererUtils::Detail::create_context_validate_params(
	SdlWindowPtr sdl_window,
	std::string& error_message)
{
	if (!sdl_window)
	{
		error_message = "Null SDL window.";

		return false;
	}

	return true;
}

bool OglRendererUtils::Detail::create_context(
	SdlWindowPtr sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	sdl_ogl_context = ::SDL_GL_CreateContext(sdl_window);

	if (!sdl_ogl_context)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

bool OglRendererUtils::Detail::make_context_current_validate_params(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	if (!sdl_window)
	{
		error_message = "Null SDL window.";

		return false;
	}

	static_cast<void>(sdl_ogl_context);

	return true;
}

bool OglRendererUtils::Detail::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	const auto sdl_result = ::SDL_GL_MakeCurrent(sdl_window, sdl_ogl_context);

	if (sdl_result < 0)
	{
		error_message = ::SDL_GetError();

		return false;
	}

	return true;
}

void* OglRendererUtils::Detail::resolve_symbol(
	const char* const symbol)
{
	return ::SDL_GL_GetProcAddress(symbol);
}

void OglRendererUtils::Detail::clear_unique_symbols_1_0()
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

bool OglRendererUtils::Detail::resolve_unique_symbols_1_0(
	Strings& missing_symbols)
{
	resolve_symbol("glCullFace", ::glCullFace, missing_symbols);
	resolve_symbol("glFrontFace", ::glFrontFace, missing_symbols);
	resolve_symbol("glHint", ::glHint, missing_symbols);
	resolve_symbol("glLineWidth", ::glLineWidth, missing_symbols);
	resolve_symbol("glPointSize", ::glPointSize, missing_symbols);
	resolve_symbol("glPolygonMode", ::glPolygonMode, missing_symbols);
	resolve_symbol("glScissor", ::glScissor, missing_symbols);
	resolve_symbol("glTexParameterf", ::glTexParameterf, missing_symbols);
	resolve_symbol("glTexParameterfv", ::glTexParameterfv, missing_symbols);
	resolve_symbol("glTexParameteri", ::glTexParameteri, missing_symbols);
	resolve_symbol("glTexParameteriv", ::glTexParameteriv, missing_symbols);
	resolve_symbol("glTexImage1D", ::glTexImage1D, missing_symbols);
	resolve_symbol("glTexImage2D", ::glTexImage2D, missing_symbols);
	resolve_symbol("glDrawBuffer", ::glDrawBuffer, missing_symbols);
	resolve_symbol("glClear", ::glClear, missing_symbols);
	resolve_symbol("glClearColor", ::glClearColor, missing_symbols);
	resolve_symbol("glClearStencil", ::glClearStencil, missing_symbols);
	resolve_symbol("glClearDepth", ::glClearDepth, missing_symbols);
	resolve_symbol("glStencilMask", ::glStencilMask, missing_symbols);
	resolve_symbol("glColorMask", ::glColorMask, missing_symbols);
	resolve_symbol("glDepthMask", ::glDepthMask, missing_symbols);
	resolve_symbol("glDisable", ::glDisable, missing_symbols);
	resolve_symbol("glEnable", ::glEnable, missing_symbols);
	resolve_symbol("glFinish", ::glFinish, missing_symbols);
	resolve_symbol("glFlush", ::glFlush, missing_symbols);
	resolve_symbol("glBlendFunc", ::glBlendFunc, missing_symbols);
	resolve_symbol("glLogicOp", ::glLogicOp, missing_symbols);
	resolve_symbol("glStencilFunc", ::glStencilFunc, missing_symbols);
	resolve_symbol("glStencilOp", ::glStencilOp, missing_symbols);
	resolve_symbol("glDepthFunc", ::glDepthFunc, missing_symbols);
	resolve_symbol("glPixelStoref", ::glPixelStoref, missing_symbols);
	resolve_symbol("glPixelStorei", ::glPixelStorei, missing_symbols);
	resolve_symbol("glReadBuffer", ::glReadBuffer, missing_symbols);
	resolve_symbol("glReadPixels", ::glReadPixels, missing_symbols);
	resolve_symbol("glGetBooleanv", ::glGetBooleanv, missing_symbols);
	resolve_symbol("glGetDoublev", ::glGetDoublev, missing_symbols);
	resolve_symbol("glGetError", ::glGetError, missing_symbols);
	resolve_symbol("glGetFloatv", ::glGetFloatv, missing_symbols);
	resolve_symbol("glGetIntegerv", ::glGetIntegerv, missing_symbols);
	resolve_symbol("glGetString", ::glGetString, missing_symbols);
	resolve_symbol("glGetTexImage", ::glGetTexImage, missing_symbols);
	resolve_symbol("glGetTexParameterfv", ::glGetTexParameterfv, missing_symbols);
	resolve_symbol("glGetTexParameteriv", ::glGetTexParameteriv, missing_symbols);
	resolve_symbol("glGetTexLevelParameterfv", ::glGetTexLevelParameterfv, missing_symbols);
	resolve_symbol("glGetTexLevelParameteriv", ::glGetTexLevelParameteriv, missing_symbols);
	resolve_symbol("glIsEnabled", ::glIsEnabled, missing_symbols);
	resolve_symbol("glDepthRange", ::glDepthRange, missing_symbols);
	resolve_symbol("glViewport", ::glViewport, missing_symbols);
	resolve_symbol("glNewList", ::glNewList, missing_symbols);
	resolve_symbol("glEndList", ::glEndList, missing_symbols);
	resolve_symbol("glCallList", ::glCallList, missing_symbols);
	resolve_symbol("glCallLists", ::glCallLists, missing_symbols);
	resolve_symbol("glDeleteLists", ::glDeleteLists, missing_symbols);
	resolve_symbol("glGenLists", ::glGenLists, missing_symbols);
	resolve_symbol("glListBase", ::glListBase, missing_symbols);
	resolve_symbol("glBegin", ::glBegin, missing_symbols);
	resolve_symbol("glBitmap", ::glBitmap, missing_symbols);
	resolve_symbol("glColor3b", ::glColor3b, missing_symbols);
	resolve_symbol("glColor3bv", ::glColor3bv, missing_symbols);
	resolve_symbol("glColor3d", ::glColor3d, missing_symbols);
	resolve_symbol("glColor3dv", ::glColor3dv, missing_symbols);
	resolve_symbol("glColor3f", ::glColor3f, missing_symbols);
	resolve_symbol("glColor3fv", ::glColor3fv, missing_symbols);
	resolve_symbol("glColor3i", ::glColor3i, missing_symbols);
	resolve_symbol("glColor3iv", ::glColor3iv, missing_symbols);
	resolve_symbol("glColor3s", ::glColor3s, missing_symbols);
	resolve_symbol("glColor3sv", ::glColor3sv, missing_symbols);
	resolve_symbol("glColor3ub", ::glColor3ub, missing_symbols);
	resolve_symbol("glColor3ubv", ::glColor3ubv, missing_symbols);
	resolve_symbol("glColor3ui", ::glColor3ui, missing_symbols);
	resolve_symbol("glColor3uiv", ::glColor3uiv, missing_symbols);
	resolve_symbol("glColor3us", ::glColor3us, missing_symbols);
	resolve_symbol("glColor3usv", ::glColor3usv, missing_symbols);
	resolve_symbol("glColor4b", ::glColor4b, missing_symbols);
	resolve_symbol("glColor4bv", ::glColor4bv, missing_symbols);
	resolve_symbol("glColor4d", ::glColor4d, missing_symbols);
	resolve_symbol("glColor4dv", ::glColor4dv, missing_symbols);
	resolve_symbol("glColor4f", ::glColor4f, missing_symbols);
	resolve_symbol("glColor4fv", ::glColor4fv, missing_symbols);
	resolve_symbol("glColor4i", ::glColor4i, missing_symbols);
	resolve_symbol("glColor4iv", ::glColor4iv, missing_symbols);
	resolve_symbol("glColor4s", ::glColor4s, missing_symbols);
	resolve_symbol("glColor4sv", ::glColor4sv, missing_symbols);
	resolve_symbol("glColor4ub", ::glColor4ub, missing_symbols);
	resolve_symbol("glColor4ubv", ::glColor4ubv, missing_symbols);
	resolve_symbol("glColor4ui", ::glColor4ui, missing_symbols);
	resolve_symbol("glColor4uiv", ::glColor4uiv, missing_symbols);
	resolve_symbol("glColor4us", ::glColor4us, missing_symbols);
	resolve_symbol("glColor4usv", ::glColor4usv, missing_symbols);
	resolve_symbol("glEdgeFlag", ::glEdgeFlag, missing_symbols);
	resolve_symbol("glEdgeFlagv", ::glEdgeFlagv, missing_symbols);
	resolve_symbol("glEnd", ::glEnd, missing_symbols);
	resolve_symbol("glIndexd", ::glIndexd, missing_symbols);
	resolve_symbol("glIndexdv", ::glIndexdv, missing_symbols);
	resolve_symbol("glIndexf", ::glIndexf, missing_symbols);
	resolve_symbol("glIndexfv", ::glIndexfv, missing_symbols);
	resolve_symbol("glIndexi", ::glIndexi, missing_symbols);
	resolve_symbol("glIndexiv", ::glIndexiv, missing_symbols);
	resolve_symbol("glIndexs", ::glIndexs, missing_symbols);
	resolve_symbol("glIndexsv", ::glIndexsv, missing_symbols);
	resolve_symbol("glNormal3b", ::glNormal3b, missing_symbols);
	resolve_symbol("glNormal3bv", ::glNormal3bv, missing_symbols);
	resolve_symbol("glNormal3d", ::glNormal3d, missing_symbols);
	resolve_symbol("glNormal3dv", ::glNormal3dv, missing_symbols);
	resolve_symbol("glNormal3f", ::glNormal3f, missing_symbols);
	resolve_symbol("glNormal3fv", ::glNormal3fv, missing_symbols);
	resolve_symbol("glNormal3i", ::glNormal3i, missing_symbols);
	resolve_symbol("glNormal3iv", ::glNormal3iv, missing_symbols);
	resolve_symbol("glNormal3s", ::glNormal3s, missing_symbols);
	resolve_symbol("glNormal3sv", ::glNormal3sv, missing_symbols);
	resolve_symbol("glRasterPos2d", ::glRasterPos2d, missing_symbols);
	resolve_symbol("glRasterPos2dv", ::glRasterPos2dv, missing_symbols);
	resolve_symbol("glRasterPos2f", ::glRasterPos2f, missing_symbols);
	resolve_symbol("glRasterPos2fv", ::glRasterPos2fv, missing_symbols);
	resolve_symbol("glRasterPos2i", ::glRasterPos2i, missing_symbols);
	resolve_symbol("glRasterPos2iv", ::glRasterPos2iv, missing_symbols);
	resolve_symbol("glRasterPos2s", ::glRasterPos2s, missing_symbols);
	resolve_symbol("glRasterPos2sv", ::glRasterPos2sv, missing_symbols);
	resolve_symbol("glRasterPos3d", ::glRasterPos3d, missing_symbols);
	resolve_symbol("glRasterPos3dv", ::glRasterPos3dv, missing_symbols);
	resolve_symbol("glRasterPos3f", ::glRasterPos3f, missing_symbols);
	resolve_symbol("glRasterPos3fv", ::glRasterPos3fv, missing_symbols);
	resolve_symbol("glRasterPos3i", ::glRasterPos3i, missing_symbols);
	resolve_symbol("glRasterPos3iv", ::glRasterPos3iv, missing_symbols);
	resolve_symbol("glRasterPos3s", ::glRasterPos3s, missing_symbols);
	resolve_symbol("glRasterPos3sv", ::glRasterPos3sv, missing_symbols);
	resolve_symbol("glRasterPos4d", ::glRasterPos4d, missing_symbols);
	resolve_symbol("glRasterPos4dv", ::glRasterPos4dv, missing_symbols);
	resolve_symbol("glRasterPos4f", ::glRasterPos4f, missing_symbols);
	resolve_symbol("glRasterPos4fv", ::glRasterPos4fv, missing_symbols);
	resolve_symbol("glRasterPos4i", ::glRasterPos4i, missing_symbols);
	resolve_symbol("glRasterPos4iv", ::glRasterPos4iv, missing_symbols);
	resolve_symbol("glRasterPos4s", ::glRasterPos4s, missing_symbols);
	resolve_symbol("glRasterPos4sv", ::glRasterPos4sv, missing_symbols);
	resolve_symbol("glRectd", ::glRectd, missing_symbols);
	resolve_symbol("glRectdv", ::glRectdv, missing_symbols);
	resolve_symbol("glRectf", ::glRectf, missing_symbols);
	resolve_symbol("glRectfv", ::glRectfv, missing_symbols);
	resolve_symbol("glRecti", ::glRecti, missing_symbols);
	resolve_symbol("glRectiv", ::glRectiv, missing_symbols);
	resolve_symbol("glRects", ::glRects, missing_symbols);
	resolve_symbol("glRectsv", ::glRectsv, missing_symbols);
	resolve_symbol("glTexCoord1d", ::glTexCoord1d, missing_symbols);
	resolve_symbol("glTexCoord1dv", ::glTexCoord1dv, missing_symbols);
	resolve_symbol("glTexCoord1f", ::glTexCoord1f, missing_symbols);
	resolve_symbol("glTexCoord1fv", ::glTexCoord1fv, missing_symbols);
	resolve_symbol("glTexCoord1i", ::glTexCoord1i, missing_symbols);
	resolve_symbol("glTexCoord1iv", ::glTexCoord1iv, missing_symbols);
	resolve_symbol("glTexCoord1s", ::glTexCoord1s, missing_symbols);
	resolve_symbol("glTexCoord1sv", ::glTexCoord1sv, missing_symbols);
	resolve_symbol("glTexCoord2d", ::glTexCoord2d, missing_symbols);
	resolve_symbol("glTexCoord2dv", ::glTexCoord2dv, missing_symbols);
	resolve_symbol("glTexCoord2f", ::glTexCoord2f, missing_symbols);
	resolve_symbol("glTexCoord2fv", ::glTexCoord2fv, missing_symbols);
	resolve_symbol("glTexCoord2i", ::glTexCoord2i, missing_symbols);
	resolve_symbol("glTexCoord2iv", ::glTexCoord2iv, missing_symbols);
	resolve_symbol("glTexCoord2s", ::glTexCoord2s, missing_symbols);
	resolve_symbol("glTexCoord2sv", ::glTexCoord2sv, missing_symbols);
	resolve_symbol("glTexCoord3d", ::glTexCoord3d, missing_symbols);
	resolve_symbol("glTexCoord3dv", ::glTexCoord3dv, missing_symbols);
	resolve_symbol("glTexCoord3f", ::glTexCoord3f, missing_symbols);
	resolve_symbol("glTexCoord3fv", ::glTexCoord3fv, missing_symbols);
	resolve_symbol("glTexCoord3i", ::glTexCoord3i, missing_symbols);
	resolve_symbol("glTexCoord3iv", ::glTexCoord3iv, missing_symbols);
	resolve_symbol("glTexCoord3s", ::glTexCoord3s, missing_symbols);
	resolve_symbol("glTexCoord3sv", ::glTexCoord3sv, missing_symbols);
	resolve_symbol("glTexCoord4d", ::glTexCoord4d, missing_symbols);
	resolve_symbol("glTexCoord4dv", ::glTexCoord4dv, missing_symbols);
	resolve_symbol("glTexCoord4f", ::glTexCoord4f, missing_symbols);
	resolve_symbol("glTexCoord4fv", ::glTexCoord4fv, missing_symbols);
	resolve_symbol("glTexCoord4i", ::glTexCoord4i, missing_symbols);
	resolve_symbol("glTexCoord4iv", ::glTexCoord4iv, missing_symbols);
	resolve_symbol("glTexCoord4s", ::glTexCoord4s, missing_symbols);
	resolve_symbol("glTexCoord4sv", ::glTexCoord4sv, missing_symbols);
	resolve_symbol("glVertex2d", ::glVertex2d, missing_symbols);
	resolve_symbol("glVertex2dv", ::glVertex2dv, missing_symbols);
	resolve_symbol("glVertex2f", ::glVertex2f, missing_symbols);
	resolve_symbol("glVertex2fv", ::glVertex2fv, missing_symbols);
	resolve_symbol("glVertex2i", ::glVertex2i, missing_symbols);
	resolve_symbol("glVertex2iv", ::glVertex2iv, missing_symbols);
	resolve_symbol("glVertex2s", ::glVertex2s, missing_symbols);
	resolve_symbol("glVertex2sv", ::glVertex2sv, missing_symbols);
	resolve_symbol("glVertex3d", ::glVertex3d, missing_symbols);
	resolve_symbol("glVertex3dv", ::glVertex3dv, missing_symbols);
	resolve_symbol("glVertex3f", ::glVertex3f, missing_symbols);
	resolve_symbol("glVertex3fv", ::glVertex3fv, missing_symbols);
	resolve_symbol("glVertex3i", ::glVertex3i, missing_symbols);
	resolve_symbol("glVertex3iv", ::glVertex3iv, missing_symbols);
	resolve_symbol("glVertex3s", ::glVertex3s, missing_symbols);
	resolve_symbol("glVertex3sv", ::glVertex3sv, missing_symbols);
	resolve_symbol("glVertex4d", ::glVertex4d, missing_symbols);
	resolve_symbol("glVertex4dv", ::glVertex4dv, missing_symbols);
	resolve_symbol("glVertex4f", ::glVertex4f, missing_symbols);
	resolve_symbol("glVertex4fv", ::glVertex4fv, missing_symbols);
	resolve_symbol("glVertex4i", ::glVertex4i, missing_symbols);
	resolve_symbol("glVertex4iv", ::glVertex4iv, missing_symbols);
	resolve_symbol("glVertex4s", ::glVertex4s, missing_symbols);
	resolve_symbol("glVertex4sv", ::glVertex4sv, missing_symbols);
	resolve_symbol("glClipPlane", ::glClipPlane, missing_symbols);
	resolve_symbol("glColorMaterial", ::glColorMaterial, missing_symbols);
	resolve_symbol("glFogf", ::glFogf, missing_symbols);
	resolve_symbol("glFogfv", ::glFogfv, missing_symbols);
	resolve_symbol("glFogi", ::glFogi, missing_symbols);
	resolve_symbol("glFogiv", ::glFogiv, missing_symbols);
	resolve_symbol("glLightf", ::glLightf, missing_symbols);
	resolve_symbol("glLightfv", ::glLightfv, missing_symbols);
	resolve_symbol("glLighti", ::glLighti, missing_symbols);
	resolve_symbol("glLightiv", ::glLightiv, missing_symbols);
	resolve_symbol("glLightModelf", ::glLightModelf, missing_symbols);
	resolve_symbol("glLightModelfv", ::glLightModelfv, missing_symbols);
	resolve_symbol("glLightModeli", ::glLightModeli, missing_symbols);
	resolve_symbol("glLightModeliv", ::glLightModeliv, missing_symbols);
	resolve_symbol("glLineStipple", ::glLineStipple, missing_symbols);
	resolve_symbol("glMaterialf", ::glMaterialf, missing_symbols);
	resolve_symbol("glMaterialfv", ::glMaterialfv, missing_symbols);
	resolve_symbol("glMateriali", ::glMateriali, missing_symbols);
	resolve_symbol("glMaterialiv", ::glMaterialiv, missing_symbols);
	resolve_symbol("glPolygonStipple", ::glPolygonStipple, missing_symbols);
	resolve_symbol("glShadeModel", ::glShadeModel, missing_symbols);
	resolve_symbol("glTexEnvf", ::glTexEnvf, missing_symbols);
	resolve_symbol("glTexEnvfv", ::glTexEnvfv, missing_symbols);
	resolve_symbol("glTexEnvi", ::glTexEnvi, missing_symbols);
	resolve_symbol("glTexEnviv", ::glTexEnviv, missing_symbols);
	resolve_symbol("glTexGend", ::glTexGend, missing_symbols);
	resolve_symbol("glTexGendv", ::glTexGendv, missing_symbols);
	resolve_symbol("glTexGenf", ::glTexGenf, missing_symbols);
	resolve_symbol("glTexGenfv", ::glTexGenfv, missing_symbols);
	resolve_symbol("glTexGeni", ::glTexGeni, missing_symbols);
	resolve_symbol("glTexGeniv", ::glTexGeniv, missing_symbols);
	resolve_symbol("glFeedbackBuffer", ::glFeedbackBuffer, missing_symbols);
	resolve_symbol("glSelectBuffer", ::glSelectBuffer, missing_symbols);
	resolve_symbol("glRenderMode", ::glRenderMode, missing_symbols);
	resolve_symbol("glInitNames", ::glInitNames, missing_symbols);
	resolve_symbol("glLoadName", ::glLoadName, missing_symbols);
	resolve_symbol("glPassThrough", ::glPassThrough, missing_symbols);
	resolve_symbol("glPopName", ::glPopName, missing_symbols);
	resolve_symbol("glPushName", ::glPushName, missing_symbols);
	resolve_symbol("glClearAccum", ::glClearAccum, missing_symbols);
	resolve_symbol("glClearIndex", ::glClearIndex, missing_symbols);
	resolve_symbol("glIndexMask", ::glIndexMask, missing_symbols);
	resolve_symbol("glAccum", ::glAccum, missing_symbols);
	resolve_symbol("glPopAttrib", ::glPopAttrib, missing_symbols);
	resolve_symbol("glPushAttrib", ::glPushAttrib, missing_symbols);
	resolve_symbol("glMap1d", ::glMap1d, missing_symbols);
	resolve_symbol("glMap1f", ::glMap1f, missing_symbols);
	resolve_symbol("glMap2d", ::glMap2d, missing_symbols);
	resolve_symbol("glMap2f", ::glMap2f, missing_symbols);
	resolve_symbol("glMapGrid1d", ::glMapGrid1d, missing_symbols);
	resolve_symbol("glMapGrid1f", ::glMapGrid1f, missing_symbols);
	resolve_symbol("glMapGrid2d", ::glMapGrid2d, missing_symbols);
	resolve_symbol("glMapGrid2f", ::glMapGrid2f, missing_symbols);
	resolve_symbol("glEvalCoord1d", ::glEvalCoord1d, missing_symbols);
	resolve_symbol("glEvalCoord1dv", ::glEvalCoord1dv, missing_symbols);
	resolve_symbol("glEvalCoord1f", ::glEvalCoord1f, missing_symbols);
	resolve_symbol("glEvalCoord1fv", ::glEvalCoord1fv, missing_symbols);
	resolve_symbol("glEvalCoord2d", ::glEvalCoord2d, missing_symbols);
	resolve_symbol("glEvalCoord2dv", ::glEvalCoord2dv, missing_symbols);
	resolve_symbol("glEvalCoord2f", ::glEvalCoord2f, missing_symbols);
	resolve_symbol("glEvalCoord2fv", ::glEvalCoord2fv, missing_symbols);
	resolve_symbol("glEvalMesh1", ::glEvalMesh1, missing_symbols);
	resolve_symbol("glEvalPoint1", ::glEvalPoint1, missing_symbols);
	resolve_symbol("glEvalMesh2", ::glEvalMesh2, missing_symbols);
	resolve_symbol("glEvalPoint2", ::glEvalPoint2, missing_symbols);
	resolve_symbol("glAlphaFunc", ::glAlphaFunc, missing_symbols);
	resolve_symbol("glPixelZoom", ::glPixelZoom, missing_symbols);
	resolve_symbol("glPixelTransferf", ::glPixelTransferf, missing_symbols);
	resolve_symbol("glPixelTransferi", ::glPixelTransferi, missing_symbols);
	resolve_symbol("glPixelMapfv", ::glPixelMapfv, missing_symbols);
	resolve_symbol("glPixelMapuiv", ::glPixelMapuiv, missing_symbols);
	resolve_symbol("glPixelMapusv", ::glPixelMapusv, missing_symbols);
	resolve_symbol("glCopyPixels", ::glCopyPixels, missing_symbols);
	resolve_symbol("glDrawPixels", ::glDrawPixels, missing_symbols);
	resolve_symbol("glGetClipPlane", ::glGetClipPlane, missing_symbols);
	resolve_symbol("glGetLightfv", ::glGetLightfv, missing_symbols);
	resolve_symbol("glGetLightiv", ::glGetLightiv, missing_symbols);
	resolve_symbol("glGetMapdv", ::glGetMapdv, missing_symbols);
	resolve_symbol("glGetMapfv", ::glGetMapfv, missing_symbols);
	resolve_symbol("glGetMapiv", ::glGetMapiv, missing_symbols);
	resolve_symbol("glGetMaterialfv", ::glGetMaterialfv, missing_symbols);
	resolve_symbol("glGetMaterialiv", ::glGetMaterialiv, missing_symbols);
	resolve_symbol("glGetPixelMapfv", ::glGetPixelMapfv, missing_symbols);
	resolve_symbol("glGetPixelMapuiv", ::glGetPixelMapuiv, missing_symbols);
	resolve_symbol("glGetPixelMapusv", ::glGetPixelMapusv, missing_symbols);
	resolve_symbol("glGetPolygonStipple", ::glGetPolygonStipple, missing_symbols);
	resolve_symbol("glGetTexEnvfv", ::glGetTexEnvfv, missing_symbols);
	resolve_symbol("glGetTexEnviv", ::glGetTexEnviv, missing_symbols);
	resolve_symbol("glGetTexGendv", ::glGetTexGendv, missing_symbols);
	resolve_symbol("glGetTexGenfv", ::glGetTexGenfv, missing_symbols);
	resolve_symbol("glGetTexGeniv", ::glGetTexGeniv, missing_symbols);
	resolve_symbol("glIsList", ::glIsList, missing_symbols);
	resolve_symbol("glFrustum", ::glFrustum, missing_symbols);
	resolve_symbol("glLoadIdentity", ::glLoadIdentity, missing_symbols);
	resolve_symbol("glLoadMatrixf", ::glLoadMatrixf, missing_symbols);
	resolve_symbol("glLoadMatrixd", ::glLoadMatrixd, missing_symbols);
	resolve_symbol("glMatrixMode", ::glMatrixMode, missing_symbols);
	resolve_symbol("glMultMatrixf", ::glMultMatrixf, missing_symbols);
	resolve_symbol("glMultMatrixd", ::glMultMatrixd, missing_symbols);
	resolve_symbol("glOrtho", ::glOrtho, missing_symbols);
	resolve_symbol("glPopMatrix", ::glPopMatrix, missing_symbols);
	resolve_symbol("glPushMatrix", ::glPushMatrix, missing_symbols);
	resolve_symbol("glRotated", ::glRotated, missing_symbols);
	resolve_symbol("glRotatef", ::glRotatef, missing_symbols);
	resolve_symbol("glScaled", ::glScaled, missing_symbols);
	resolve_symbol("glScalef", ::glScalef, missing_symbols);
	resolve_symbol("glTranslated", ::glTranslated, missing_symbols);
	resolve_symbol("glTranslatef", ::glTranslatef, missing_symbols);

	if (!missing_symbols.empty())
	{
		clear_unique_symbols_1_0();

		return false;
	}

	return true;
}

void OglRendererUtils::Detail::clear_unique_symbols_1_1()
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

bool OglRendererUtils::Detail::resolve_unique_symbols_1_1(
	Strings& missing_symbols)
{
	resolve_symbol("glDrawArrays", ::glDrawArrays, missing_symbols);
	resolve_symbol("glDrawElements", ::glDrawElements, missing_symbols);
	resolve_symbol("glGetPointerv", ::glGetPointerv, missing_symbols);
	resolve_symbol("glPolygonOffset", ::glPolygonOffset, missing_symbols);
	resolve_symbol("glCopyTexImage1D", ::glCopyTexImage1D, missing_symbols);
	resolve_symbol("glCopyTexImage2D", ::glCopyTexImage2D, missing_symbols);
	resolve_symbol("glCopyTexSubImage1D", ::glCopyTexSubImage1D, missing_symbols);
	resolve_symbol("glCopyTexSubImage2D", ::glCopyTexSubImage2D, missing_symbols);
	resolve_symbol("glTexSubImage1D", ::glTexSubImage1D, missing_symbols);
	resolve_symbol("glTexSubImage2D", ::glTexSubImage2D, missing_symbols);
	resolve_symbol("glBindTexture", ::glBindTexture, missing_symbols);
	resolve_symbol("glDeleteTextures", ::glDeleteTextures, missing_symbols);
	resolve_symbol("glGenTextures", ::glGenTextures, missing_symbols);
	resolve_symbol("glIsTexture", ::glIsTexture, missing_symbols);
	resolve_symbol("glArrayElement", ::glArrayElement, missing_symbols);
	resolve_symbol("glColorPointer", ::glColorPointer, missing_symbols);
	resolve_symbol("glDisableClientState", ::glDisableClientState, missing_symbols);
	resolve_symbol("glEdgeFlagPointer", ::glEdgeFlagPointer, missing_symbols);
	resolve_symbol("glEnableClientState", ::glEnableClientState, missing_symbols);
	resolve_symbol("glIndexPointer", ::glIndexPointer, missing_symbols);
	resolve_symbol("glInterleavedArrays", ::glInterleavedArrays, missing_symbols);
	resolve_symbol("glNormalPointer", ::glNormalPointer, missing_symbols);
	resolve_symbol("glTexCoordPointer", ::glTexCoordPointer, missing_symbols);
	resolve_symbol("glVertexPointer", ::glVertexPointer, missing_symbols);
	resolve_symbol("glAreTexturesResident", ::glAreTexturesResident, missing_symbols);
	resolve_symbol("glPrioritizeTextures", ::glPrioritizeTextures, missing_symbols);
	resolve_symbol("glIndexub", ::glIndexub, missing_symbols);
	resolve_symbol("glIndexubv", ::glIndexubv, missing_symbols);
	resolve_symbol("glPopClientAttrib", ::glPopClientAttrib, missing_symbols);
	resolve_symbol("glPushClientAttrib", ::glPushClientAttrib, missing_symbols);

	if (!missing_symbols.empty())
	{
		clear_unique_symbols_1_1();

		return false;
	}

	return true;
}

//
// OglRendererUtils::Detail
// ==========================================================================


// ==========================================================================
// OglRendererUtils
//

bool OglRendererUtils::load_library(
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to load default OpenGL library. ";

	if (!Detail::load_library(error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

void OglRendererUtils::unload_library()
{
	Detail::unload_library();
}

bool OglRendererUtils::create_context(
	SdlWindowPtr sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to create OpenGL context. ";

	sdl_ogl_context = nullptr;

	if (!Detail::create_context_validate_params(sdl_window, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::create_context(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

bool OglRendererUtils::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContext sdl_ogl_context,
	std::string& error_message)
{
	const auto error_message_prefix = "Failed to make a context current. ";

	if (!Detail::make_context_current_validate_params(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	if (!Detail::make_context_current(sdl_window, sdl_ogl_context, error_message))
	{
		error_message = error_message_prefix + error_message;

		return false;
	}

	return true;
}

bool OglRendererUtils::create_window_and_context(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowPtr& sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	auto is_succeed = true;

	if (is_succeed)
	{
		if (!RendererUtils::create_window(param, sdl_window, error_message))
		{
			is_succeed = false;
		}
	}

	if (is_succeed)
	{
		if (!create_context(sdl_window, sdl_ogl_context, error_message))
		{
			is_succeed = false;
		}
	}

	if (!is_succeed)
	{
		destroy_window_and_context(sdl_window, sdl_ogl_context);

		return false;
	}

	return true;
}

bool OglRendererUtils::create_probe_window_and_context(
	SdlWindowPtr& sdl_window,
	SdlGlContext& sdl_ogl_context,
	std::string& error_message)
{
	auto param = RendererUtilsCreateWindowParam{};
	param.is_opengl_ = true;
	param.width_ = 1;
	param.height_ = 1;

	return create_window_and_context(param, sdl_window, sdl_ogl_context, error_message);
}

void OglRendererUtils::destroy_window_and_context(
	SdlWindowPtr& sdl_window,
	SdlGlContext& sdl_ogl_context)
{
	if (sdl_ogl_context)
	{
		::SDL_GL_DeleteContext(sdl_ogl_context);

		sdl_ogl_context = nullptr;
	}

	if (sdl_window)
	{
		::SDL_DestroyWindow(sdl_window);

		sdl_window = nullptr;
	}
}

bool OglRendererUtils::resolve_symbols_1_1(
	Strings& missing_symbols)
{
	const auto symbol_count =
		Detail::v1_0_unique_symbol_count +
		Detail::v1_1_unique_symbol_count +
		0;

	missing_symbols.clear();
	missing_symbols.reserve(symbol_count);

	Detail::resolve_unique_symbols_1_0(missing_symbols);
	Detail::resolve_unique_symbols_1_1(missing_symbols);

	return missing_symbols.empty();
}

void OglRendererUtils::swap_window(
	SdlWindowPtr sdl_window)
{
	assert(sdl_window != nullptr);

	::SDL_GL_SwapWindow(sdl_window);
}

//
// OglRendererUtils
// ==========================================================================


} // bstone
