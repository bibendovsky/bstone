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
// OpenGL API.
//


#include "bstone_detail_ren_3d_gl_api.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OpenGL 1.0
//

#ifdef GL_VERSION_1_0

PFNGLCULLFACEPROC glCullFace = nullptr;
PFNGLFRONTFACEPROC glFrontFace = nullptr;
PFNGLHINTPROC glHint = nullptr;
PFNGLLINEWIDTHPROC glLineWidth = nullptr;
PFNGLPOINTSIZEPROC glPointSize = nullptr;
PFNGLPOLYGONMODEPROC glPolygonMode = nullptr;
PFNGLSCISSORPROC glScissor = nullptr;
PFNGLTEXPARAMETERFPROC glTexParameterf = nullptr;
PFNGLTEXPARAMETERFVPROC glTexParameterfv = nullptr;
PFNGLTEXPARAMETERIPROC glTexParameteri = nullptr;
PFNGLTEXPARAMETERIVPROC glTexParameteriv = nullptr;
PFNGLTEXIMAGE1DPROC glTexImage1D = nullptr;
PFNGLTEXIMAGE2DPROC glTexImage2D = nullptr;
PFNGLDRAWBUFFERPROC glDrawBuffer = nullptr;
PFNGLCLEARPROC glClear = nullptr;
PFNGLCLEARCOLORPROC glClearColor = nullptr;
PFNGLCLEARSTENCILPROC glClearStencil = nullptr;
PFNGLCLEARDEPTHPROC glClearDepth = nullptr;
PFNGLSTENCILMASKPROC glStencilMask = nullptr;
PFNGLCOLORMASKPROC glColorMask = nullptr;
PFNGLDEPTHMASKPROC glDepthMask = nullptr;
PFNGLDISABLEPROC glDisable = nullptr;
PFNGLENABLEPROC glEnable = nullptr;
PFNGLFINISHPROC glFinish = nullptr;
PFNGLFLUSHPROC glFlush = nullptr;
PFNGLBLENDFUNCPROC glBlendFunc = nullptr;
PFNGLLOGICOPPROC glLogicOp = nullptr;
PFNGLSTENCILFUNCPROC glStencilFunc = nullptr;
PFNGLSTENCILOPPROC glStencilOp = nullptr;
PFNGLDEPTHFUNCPROC glDepthFunc = nullptr;
PFNGLPIXELSTOREFPROC glPixelStoref = nullptr;
PFNGLPIXELSTOREIPROC glPixelStorei = nullptr;
PFNGLREADBUFFERPROC glReadBuffer = nullptr;
PFNGLREADPIXELSPROC glReadPixels = nullptr;
PFNGLGETBOOLEANVPROC glGetBooleanv = nullptr;
PFNGLGETDOUBLEVPROC glGetDoublev = nullptr;
PFNGLGETERRORPROC glGetError = nullptr;
PFNGLGETFLOATVPROC glGetFloatv = nullptr;
PFNGLGETINTEGERVPROC glGetIntegerv = nullptr;
PFNGLGETSTRINGPROC glGetString = nullptr;
PFNGLGETTEXIMAGEPROC glGetTexImage = nullptr;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv = nullptr;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv = nullptr;
PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv = nullptr;
PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv = nullptr;
PFNGLISENABLEDPROC glIsEnabled = nullptr;
PFNGLDEPTHRANGEPROC glDepthRange = nullptr;
PFNGLVIEWPORTPROC glViewport = nullptr;
PFNGLNEWLISTPROC glNewList = nullptr;
PFNGLENDLISTPROC glEndList = nullptr;
PFNGLCALLLISTPROC glCallList = nullptr;
PFNGLCALLLISTSPROC glCallLists = nullptr;
PFNGLDELETELISTSPROC glDeleteLists = nullptr;
PFNGLGENLISTSPROC glGenLists = nullptr;
PFNGLLISTBASEPROC glListBase = nullptr;
PFNGLBEGINPROC glBegin = nullptr;
PFNGLBITMAPPROC glBitmap = nullptr;
PFNGLCOLOR3BPROC glColor3b = nullptr;
PFNGLCOLOR3BVPROC glColor3bv = nullptr;
PFNGLCOLOR3DPROC glColor3d = nullptr;
PFNGLCOLOR3DVPROC glColor3dv = nullptr;
PFNGLCOLOR3FPROC glColor3f = nullptr;
PFNGLCOLOR3FVPROC glColor3fv = nullptr;
PFNGLCOLOR3IPROC glColor3i = nullptr;
PFNGLCOLOR3IVPROC glColor3iv = nullptr;
PFNGLCOLOR3SPROC glColor3s = nullptr;
PFNGLCOLOR3SVPROC glColor3sv = nullptr;
PFNGLCOLOR3UBPROC glColor3ub = nullptr;
PFNGLCOLOR3UBVPROC glColor3ubv = nullptr;
PFNGLCOLOR3UIPROC glColor3ui = nullptr;
PFNGLCOLOR3UIVPROC glColor3uiv = nullptr;
PFNGLCOLOR3USPROC glColor3us = nullptr;
PFNGLCOLOR3USVPROC glColor3usv = nullptr;
PFNGLCOLOR4BPROC glColor4b = nullptr;
PFNGLCOLOR4BVPROC glColor4bv = nullptr;
PFNGLCOLOR4DPROC glColor4d = nullptr;
PFNGLCOLOR4DVPROC glColor4dv = nullptr;
PFNGLCOLOR4FPROC glColor4f = nullptr;
PFNGLCOLOR4FVPROC glColor4fv = nullptr;
PFNGLCOLOR4IPROC glColor4i = nullptr;
PFNGLCOLOR4IVPROC glColor4iv = nullptr;
PFNGLCOLOR4SPROC glColor4s = nullptr;
PFNGLCOLOR4SVPROC glColor4sv = nullptr;
PFNGLCOLOR4UBPROC glColor4ub = nullptr;
PFNGLCOLOR4UBVPROC glColor4ubv = nullptr;
PFNGLCOLOR4UIPROC glColor4ui = nullptr;
PFNGLCOLOR4UIVPROC glColor4uiv = nullptr;
PFNGLCOLOR4USPROC glColor4us = nullptr;
PFNGLCOLOR4USVPROC glColor4usv = nullptr;
PFNGLEDGEFLAGPROC glEdgeFlag = nullptr;
PFNGLEDGEFLAGVPROC glEdgeFlagv = nullptr;
PFNGLENDPROC glEnd = nullptr;
PFNGLINDEXDPROC glIndexd = nullptr;
PFNGLINDEXDVPROC glIndexdv = nullptr;
PFNGLINDEXFPROC glIndexf = nullptr;
PFNGLINDEXFVPROC glIndexfv = nullptr;
PFNGLINDEXIPROC glIndexi = nullptr;
PFNGLINDEXIVPROC glIndexiv = nullptr;
PFNGLINDEXSPROC glIndexs = nullptr;
PFNGLINDEXSVPROC glIndexsv = nullptr;
PFNGLNORMAL3BPROC glNormal3b = nullptr;
PFNGLNORMAL3BVPROC glNormal3bv = nullptr;
PFNGLNORMAL3DPROC glNormal3d = nullptr;
PFNGLNORMAL3DVPROC glNormal3dv = nullptr;
PFNGLNORMAL3FPROC glNormal3f = nullptr;
PFNGLNORMAL3FVPROC glNormal3fv = nullptr;
PFNGLNORMAL3IPROC glNormal3i = nullptr;
PFNGLNORMAL3IVPROC glNormal3iv = nullptr;
PFNGLNORMAL3SPROC glNormal3s = nullptr;
PFNGLNORMAL3SVPROC glNormal3sv = nullptr;
PFNGLRASTERPOS2DPROC glRasterPos2d = nullptr;
PFNGLRASTERPOS2DVPROC glRasterPos2dv = nullptr;
PFNGLRASTERPOS2FPROC glRasterPos2f = nullptr;
PFNGLRASTERPOS2FVPROC glRasterPos2fv = nullptr;
PFNGLRASTERPOS2IPROC glRasterPos2i = nullptr;
PFNGLRASTERPOS2IVPROC glRasterPos2iv = nullptr;
PFNGLRASTERPOS2SPROC glRasterPos2s = nullptr;
PFNGLRASTERPOS2SVPROC glRasterPos2sv = nullptr;
PFNGLRASTERPOS3DPROC glRasterPos3d = nullptr;
PFNGLRASTERPOS3DVPROC glRasterPos3dv = nullptr;
PFNGLRASTERPOS3FPROC glRasterPos3f = nullptr;
PFNGLRASTERPOS3FVPROC glRasterPos3fv = nullptr;
PFNGLRASTERPOS3IPROC glRasterPos3i = nullptr;
PFNGLRASTERPOS3IVPROC glRasterPos3iv = nullptr;
PFNGLRASTERPOS3SPROC glRasterPos3s = nullptr;
PFNGLRASTERPOS3SVPROC glRasterPos3sv = nullptr;
PFNGLRASTERPOS4DPROC glRasterPos4d = nullptr;
PFNGLRASTERPOS4DVPROC glRasterPos4dv = nullptr;
PFNGLRASTERPOS4FPROC glRasterPos4f = nullptr;
PFNGLRASTERPOS4FVPROC glRasterPos4fv = nullptr;
PFNGLRASTERPOS4IPROC glRasterPos4i = nullptr;
PFNGLRASTERPOS4IVPROC glRasterPos4iv = nullptr;
PFNGLRASTERPOS4SPROC glRasterPos4s = nullptr;
PFNGLRASTERPOS4SVPROC glRasterPos4sv = nullptr;
PFNGLRECTDPROC glRectd = nullptr;
PFNGLRECTDVPROC glRectdv = nullptr;
PFNGLRECTFPROC glRectf = nullptr;
PFNGLRECTFVPROC glRectfv = nullptr;
PFNGLRECTIPROC glRecti = nullptr;
PFNGLRECTIVPROC glRectiv = nullptr;
PFNGLRECTSPROC glRects = nullptr;
PFNGLRECTSVPROC glRectsv = nullptr;
PFNGLTEXCOORD1DPROC glTexCoord1d = nullptr;
PFNGLTEXCOORD1DVPROC glTexCoord1dv = nullptr;
PFNGLTEXCOORD1FPROC glTexCoord1f = nullptr;
PFNGLTEXCOORD1FVPROC glTexCoord1fv = nullptr;
PFNGLTEXCOORD1IPROC glTexCoord1i = nullptr;
PFNGLTEXCOORD1IVPROC glTexCoord1iv = nullptr;
PFNGLTEXCOORD1SPROC glTexCoord1s = nullptr;
PFNGLTEXCOORD1SVPROC glTexCoord1sv = nullptr;
PFNGLTEXCOORD2DPROC glTexCoord2d = nullptr;
PFNGLTEXCOORD2DVPROC glTexCoord2dv = nullptr;
PFNGLTEXCOORD2FPROC glTexCoord2f = nullptr;
PFNGLTEXCOORD2FVPROC glTexCoord2fv = nullptr;
PFNGLTEXCOORD2IPROC glTexCoord2i = nullptr;
PFNGLTEXCOORD2IVPROC glTexCoord2iv = nullptr;
PFNGLTEXCOORD2SPROC glTexCoord2s = nullptr;
PFNGLTEXCOORD2SVPROC glTexCoord2sv = nullptr;
PFNGLTEXCOORD3DPROC glTexCoord3d = nullptr;
PFNGLTEXCOORD3DVPROC glTexCoord3dv = nullptr;
PFNGLTEXCOORD3FPROC glTexCoord3f = nullptr;
PFNGLTEXCOORD3FVPROC glTexCoord3fv = nullptr;
PFNGLTEXCOORD3IPROC glTexCoord3i = nullptr;
PFNGLTEXCOORD3IVPROC glTexCoord3iv = nullptr;
PFNGLTEXCOORD3SPROC glTexCoord3s = nullptr;
PFNGLTEXCOORD3SVPROC glTexCoord3sv = nullptr;
PFNGLTEXCOORD4DPROC glTexCoord4d = nullptr;
PFNGLTEXCOORD4DVPROC glTexCoord4dv = nullptr;
PFNGLTEXCOORD4FPROC glTexCoord4f = nullptr;
PFNGLTEXCOORD4FVPROC glTexCoord4fv = nullptr;
PFNGLTEXCOORD4IPROC glTexCoord4i = nullptr;
PFNGLTEXCOORD4IVPROC glTexCoord4iv = nullptr;
PFNGLTEXCOORD4SPROC glTexCoord4s = nullptr;
PFNGLTEXCOORD4SVPROC glTexCoord4sv = nullptr;
PFNGLVERTEX2DPROC glVertex2d = nullptr;
PFNGLVERTEX2DVPROC glVertex2dv = nullptr;
PFNGLVERTEX2FPROC glVertex2f = nullptr;
PFNGLVERTEX2FVPROC glVertex2fv = nullptr;
PFNGLVERTEX2IPROC glVertex2i = nullptr;
PFNGLVERTEX2IVPROC glVertex2iv = nullptr;
PFNGLVERTEX2SPROC glVertex2s = nullptr;
PFNGLVERTEX2SVPROC glVertex2sv = nullptr;
PFNGLVERTEX3DPROC glVertex3d = nullptr;
PFNGLVERTEX3DVPROC glVertex3dv = nullptr;
PFNGLVERTEX3FPROC glVertex3f = nullptr;
PFNGLVERTEX3FVPROC glVertex3fv = nullptr;
PFNGLVERTEX3IPROC glVertex3i = nullptr;
PFNGLVERTEX3IVPROC glVertex3iv = nullptr;
PFNGLVERTEX3SPROC glVertex3s = nullptr;
PFNGLVERTEX3SVPROC glVertex3sv = nullptr;
PFNGLVERTEX4DPROC glVertex4d = nullptr;
PFNGLVERTEX4DVPROC glVertex4dv = nullptr;
PFNGLVERTEX4FPROC glVertex4f = nullptr;
PFNGLVERTEX4FVPROC glVertex4fv = nullptr;
PFNGLVERTEX4IPROC glVertex4i = nullptr;
PFNGLVERTEX4IVPROC glVertex4iv = nullptr;
PFNGLVERTEX4SPROC glVertex4s = nullptr;
PFNGLVERTEX4SVPROC glVertex4sv = nullptr;
PFNGLCLIPPLANEPROC glClipPlane = nullptr;
PFNGLCOLORMATERIALPROC glColorMaterial = nullptr;
PFNGLFOGFPROC glFogf = nullptr;
PFNGLFOGFVPROC glFogfv = nullptr;
PFNGLFOGIPROC glFogi = nullptr;
PFNGLFOGIVPROC glFogiv = nullptr;
PFNGLLIGHTFPROC glLightf = nullptr;
PFNGLLIGHTFVPROC glLightfv = nullptr;
PFNGLLIGHTIPROC glLighti = nullptr;
PFNGLLIGHTIVPROC glLightiv = nullptr;
PFNGLLIGHTMODELFPROC glLightModelf = nullptr;
PFNGLLIGHTMODELFVPROC glLightModelfv = nullptr;
PFNGLLIGHTMODELIPROC glLightModeli = nullptr;
PFNGLLIGHTMODELIVPROC glLightModeliv = nullptr;
PFNGLLINESTIPPLEPROC glLineStipple = nullptr;
PFNGLMATERIALFPROC glMaterialf = nullptr;
PFNGLMATERIALFVPROC glMaterialfv = nullptr;
PFNGLMATERIALIPROC glMateriali = nullptr;
PFNGLMATERIALIVPROC glMaterialiv = nullptr;
PFNGLPOLYGONSTIPPLEPROC glPolygonStipple = nullptr;
PFNGLSHADEMODELPROC glShadeModel = nullptr;
PFNGLTEXENVFPROC glTexEnvf = nullptr;
PFNGLTEXENVFVPROC glTexEnvfv = nullptr;
PFNGLTEXENVIPROC glTexEnvi = nullptr;
PFNGLTEXENVIVPROC glTexEnviv = nullptr;
PFNGLTEXGENDPROC glTexGend = nullptr;
PFNGLTEXGENDVPROC glTexGendv = nullptr;
PFNGLTEXGENFPROC glTexGenf = nullptr;
PFNGLTEXGENFVPROC glTexGenfv = nullptr;
PFNGLTEXGENIPROC glTexGeni = nullptr;
PFNGLTEXGENIVPROC glTexGeniv = nullptr;
PFNGLFEEDBACKBUFFERPROC glFeedbackBuffer = nullptr;
PFNGLSELECTBUFFERPROC glSelectBuffer = nullptr;
PFNGLRENDERMODEPROC glRenderMode = nullptr;
PFNGLINITNAMESPROC glInitNames = nullptr;
PFNGLLOADNAMEPROC glLoadName = nullptr;
PFNGLPASSTHROUGHPROC glPassThrough = nullptr;
PFNGLPOPNAMEPROC glPopName = nullptr;
PFNGLPUSHNAMEPROC glPushName = nullptr;
PFNGLCLEARACCUMPROC glClearAccum = nullptr;
PFNGLCLEARINDEXPROC glClearIndex = nullptr;
PFNGLINDEXMASKPROC glIndexMask = nullptr;
PFNGLACCUMPROC glAccum = nullptr;
PFNGLPOPATTRIBPROC glPopAttrib = nullptr;
PFNGLPUSHATTRIBPROC glPushAttrib = nullptr;
PFNGLMAP1DPROC glMap1d = nullptr;
PFNGLMAP1FPROC glMap1f = nullptr;
PFNGLMAP2DPROC glMap2d = nullptr;
PFNGLMAP2FPROC glMap2f = nullptr;
PFNGLMAPGRID1DPROC glMapGrid1d = nullptr;
PFNGLMAPGRID1FPROC glMapGrid1f = nullptr;
PFNGLMAPGRID2DPROC glMapGrid2d = nullptr;
PFNGLMAPGRID2FPROC glMapGrid2f = nullptr;
PFNGLEVALCOORD1DPROC glEvalCoord1d = nullptr;
PFNGLEVALCOORD1DVPROC glEvalCoord1dv = nullptr;
PFNGLEVALCOORD1FPROC glEvalCoord1f = nullptr;
PFNGLEVALCOORD1FVPROC glEvalCoord1fv = nullptr;
PFNGLEVALCOORD2DPROC glEvalCoord2d = nullptr;
PFNGLEVALCOORD2DVPROC glEvalCoord2dv = nullptr;
PFNGLEVALCOORD2FPROC glEvalCoord2f = nullptr;
PFNGLEVALCOORD2FVPROC glEvalCoord2fv = nullptr;
PFNGLEVALMESH1PROC glEvalMesh1 = nullptr;
PFNGLEVALPOINT1PROC glEvalPoint1 = nullptr;
PFNGLEVALMESH2PROC glEvalMesh2 = nullptr;
PFNGLEVALPOINT2PROC glEvalPoint2 = nullptr;
PFNGLALPHAFUNCPROC glAlphaFunc = nullptr;
PFNGLPIXELZOOMPROC glPixelZoom = nullptr;
PFNGLPIXELTRANSFERFPROC glPixelTransferf = nullptr;
PFNGLPIXELTRANSFERIPROC glPixelTransferi = nullptr;
PFNGLPIXELMAPFVPROC glPixelMapfv = nullptr;
PFNGLPIXELMAPUIVPROC glPixelMapuiv = nullptr;
PFNGLPIXELMAPUSVPROC glPixelMapusv = nullptr;
PFNGLCOPYPIXELSPROC glCopyPixels = nullptr;
PFNGLDRAWPIXELSPROC glDrawPixels = nullptr;
PFNGLGETCLIPPLANEPROC glGetClipPlane = nullptr;
PFNGLGETLIGHTFVPROC glGetLightfv = nullptr;
PFNGLGETLIGHTIVPROC glGetLightiv = nullptr;
PFNGLGETMAPDVPROC glGetMapdv = nullptr;
PFNGLGETMAPFVPROC glGetMapfv = nullptr;
PFNGLGETMAPIVPROC glGetMapiv = nullptr;
PFNGLGETMATERIALFVPROC glGetMaterialfv = nullptr;
PFNGLGETMATERIALIVPROC glGetMaterialiv = nullptr;
PFNGLGETPIXELMAPFVPROC glGetPixelMapfv = nullptr;
PFNGLGETPIXELMAPUIVPROC glGetPixelMapuiv = nullptr;
PFNGLGETPIXELMAPUSVPROC glGetPixelMapusv = nullptr;
PFNGLGETPOLYGONSTIPPLEPROC glGetPolygonStipple = nullptr;
PFNGLGETTEXENVFVPROC glGetTexEnvfv = nullptr;
PFNGLGETTEXENVIVPROC glGetTexEnviv = nullptr;
PFNGLGETTEXGENDVPROC glGetTexGendv = nullptr;
PFNGLGETTEXGENFVPROC glGetTexGenfv = nullptr;
PFNGLGETTEXGENIVPROC glGetTexGeniv = nullptr;
PFNGLISLISTPROC glIsList = nullptr;
PFNGLFRUSTUMPROC glFrustum = nullptr;
PFNGLLOADIDENTITYPROC glLoadIdentity = nullptr;
PFNGLLOADMATRIXFPROC glLoadMatrixf = nullptr;
PFNGLLOADMATRIXDPROC glLoadMatrixd = nullptr;
PFNGLMATRIXMODEPROC glMatrixMode = nullptr;
PFNGLMULTMATRIXFPROC glMultMatrixf = nullptr;
PFNGLMULTMATRIXDPROC glMultMatrixd = nullptr;
PFNGLORTHOPROC glOrtho = nullptr;
PFNGLPOPMATRIXPROC glPopMatrix = nullptr;
PFNGLPUSHMATRIXPROC glPushMatrix = nullptr;
PFNGLROTATEDPROC glRotated = nullptr;
PFNGLROTATEFPROC glRotatef = nullptr;
PFNGLSCALEDPROC glScaled = nullptr;
PFNGLSCALEFPROC glScalef = nullptr;
PFNGLTRANSLATEDPROC glTranslated = nullptr;
PFNGLTRANSLATEFPROC glTranslatef = nullptr;

#endif // GL_VERSION_1_0

//
// OpenGL 1.0
// ==========================================================================


// ==========================================================================
// OpenGL 1.1
//

#ifdef GL_VERSION_1_1

PFNGLDRAWARRAYSPROC glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC glDrawElements = nullptr;
PFNGLGETPOINTERVPROC glGetPointerv = nullptr;
PFNGLPOLYGONOFFSETPROC glPolygonOffset = nullptr;
PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D = nullptr;
PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D = nullptr;
PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D = nullptr;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D = nullptr;
PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D = nullptr;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = nullptr;
PFNGLBINDTEXTUREPROC glBindTexture = nullptr;
PFNGLDELETETEXTURESPROC glDeleteTextures = nullptr;
PFNGLGENTEXTURESPROC glGenTextures = nullptr;
PFNGLISTEXTUREPROC glIsTexture = nullptr;
PFNGLARRAYELEMENTPROC glArrayElement = nullptr;
PFNGLCOLORPOINTERPROC glColorPointer = nullptr;
PFNGLDISABLECLIENTSTATEPROC glDisableClientState = nullptr;
PFNGLEDGEFLAGPOINTERPROC glEdgeFlagPointer = nullptr;
PFNGLENABLECLIENTSTATEPROC glEnableClientState = nullptr;
PFNGLINDEXPOINTERPROC glIndexPointer = nullptr;
PFNGLINTERLEAVEDARRAYSPROC glInterleavedArrays = nullptr;
PFNGLNORMALPOINTERPROC glNormalPointer = nullptr;
PFNGLTEXCOORDPOINTERPROC glTexCoordPointer = nullptr;
PFNGLVERTEXPOINTERPROC glVertexPointer = nullptr;
PFNGLARETEXTURESRESIDENTPROC glAreTexturesResident = nullptr;
PFNGLPRIORITIZETEXTURESPROC glPrioritizeTextures = nullptr;
PFNGLINDEXUBPROC glIndexub = nullptr;
PFNGLINDEXUBVPROC glIndexubv = nullptr;
PFNGLPOPCLIENTATTRIBPROC glPopClientAttrib = nullptr;
PFNGLPUSHCLIENTATTRIBPROC glPushClientAttrib = nullptr;

#endif // GL_VERSION_1_1

//
// OpenGL 1.1
// ==========================================================================


// ==========================================================================
// OpenGL v1.2
//

#ifdef GL_VERSION_1_2

PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = nullptr;
PFNGLTEXIMAGE3DPROC glTexImage3D = nullptr;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = nullptr;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = nullptr;

#endif // GL_VERSION_1_2

//
// OpenGL v1.2
// ==========================================================================


// ==========================================================================
// OpenGL v1.3
//

#ifdef GL_VERSION_1_3

PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = nullptr;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = nullptr;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage = nullptr;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = nullptr;
PFNGLMULTITEXCOORD1DPROC glMultiTexCoord1d = nullptr;
PFNGLMULTITEXCOORD1DVPROC glMultiTexCoord1dv = nullptr;
PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f = nullptr;
PFNGLMULTITEXCOORD1FVPROC glMultiTexCoord1fv = nullptr;
PFNGLMULTITEXCOORD1IPROC glMultiTexCoord1i = nullptr;
PFNGLMULTITEXCOORD1IVPROC glMultiTexCoord1iv = nullptr;
PFNGLMULTITEXCOORD1SPROC glMultiTexCoord1s = nullptr;
PFNGLMULTITEXCOORD1SVPROC glMultiTexCoord1sv = nullptr;
PFNGLMULTITEXCOORD2DPROC glMultiTexCoord2d = nullptr;
PFNGLMULTITEXCOORD2DVPROC glMultiTexCoord2dv = nullptr;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f = nullptr;
PFNGLMULTITEXCOORD2FVPROC glMultiTexCoord2fv = nullptr;
PFNGLMULTITEXCOORD2IPROC glMultiTexCoord2i = nullptr;
PFNGLMULTITEXCOORD2IVPROC glMultiTexCoord2iv = nullptr;
PFNGLMULTITEXCOORD2SPROC glMultiTexCoord2s = nullptr;
PFNGLMULTITEXCOORD2SVPROC glMultiTexCoord2sv = nullptr;
PFNGLMULTITEXCOORD3DPROC glMultiTexCoord3d = nullptr;
PFNGLMULTITEXCOORD3DVPROC glMultiTexCoord3dv = nullptr;
PFNGLMULTITEXCOORD3FPROC glMultiTexCoord3f = nullptr;
PFNGLMULTITEXCOORD3FVPROC glMultiTexCoord3fv = nullptr;
PFNGLMULTITEXCOORD3IPROC glMultiTexCoord3i = nullptr;
PFNGLMULTITEXCOORD3IVPROC glMultiTexCoord3iv = nullptr;
PFNGLMULTITEXCOORD3SPROC glMultiTexCoord3s = nullptr;
PFNGLMULTITEXCOORD3SVPROC glMultiTexCoord3sv = nullptr;
PFNGLMULTITEXCOORD4DPROC glMultiTexCoord4d = nullptr;
PFNGLMULTITEXCOORD4DVPROC glMultiTexCoord4dv = nullptr;
PFNGLMULTITEXCOORD4FPROC glMultiTexCoord4f = nullptr;
PFNGLMULTITEXCOORD4FVPROC glMultiTexCoord4fv = nullptr;
PFNGLMULTITEXCOORD4IPROC glMultiTexCoord4i = nullptr;
PFNGLMULTITEXCOORD4IVPROC glMultiTexCoord4iv = nullptr;
PFNGLMULTITEXCOORD4SPROC glMultiTexCoord4s = nullptr;
PFNGLMULTITEXCOORD4SVPROC glMultiTexCoord4sv = nullptr;
PFNGLLOADTRANSPOSEMATRIXFPROC glLoadTransposeMatrixf = nullptr;
PFNGLLOADTRANSPOSEMATRIXDPROC glLoadTransposeMatrixd = nullptr;
PFNGLMULTTRANSPOSEMATRIXFPROC glMultTransposeMatrixf = nullptr;
PFNGLMULTTRANSPOSEMATRIXDPROC glMultTransposeMatrixd = nullptr;

#endif // GL_VERSION_1_3

//
// OpenGL v1.3
// ==========================================================================


// ==========================================================================
// OpenGL v1.4
//

#ifdef GL_VERSION_1_4

PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = nullptr;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays = nullptr;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements = nullptr;
PFNGLPOINTPARAMETERFPROC glPointParameterf = nullptr;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv = nullptr;
PFNGLPOINTPARAMETERIPROC glPointParameteri = nullptr;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv = nullptr;
PFNGLFOGCOORDFPROC glFogCoordf = nullptr;
PFNGLFOGCOORDFVPROC glFogCoordfv = nullptr;
PFNGLFOGCOORDDPROC glFogCoordd = nullptr;
PFNGLFOGCOORDDVPROC glFogCoorddv = nullptr;
PFNGLFOGCOORDPOINTERPROC glFogCoordPointer = nullptr;
PFNGLSECONDARYCOLOR3BPROC glSecondaryColor3b = nullptr;
PFNGLSECONDARYCOLOR3BVPROC glSecondaryColor3bv = nullptr;
PFNGLSECONDARYCOLOR3DPROC glSecondaryColor3d = nullptr;
PFNGLSECONDARYCOLOR3DVPROC glSecondaryColor3dv = nullptr;
PFNGLSECONDARYCOLOR3FPROC glSecondaryColor3f = nullptr;
PFNGLSECONDARYCOLOR3FVPROC glSecondaryColor3fv = nullptr;
PFNGLSECONDARYCOLOR3IPROC glSecondaryColor3i = nullptr;
PFNGLSECONDARYCOLOR3IVPROC glSecondaryColor3iv = nullptr;
PFNGLSECONDARYCOLOR3SPROC glSecondaryColor3s = nullptr;
PFNGLSECONDARYCOLOR3SVPROC glSecondaryColor3sv = nullptr;
PFNGLSECONDARYCOLOR3UBPROC glSecondaryColor3ub = nullptr;
PFNGLSECONDARYCOLOR3UBVPROC glSecondaryColor3ubv = nullptr;
PFNGLSECONDARYCOLOR3UIPROC glSecondaryColor3ui = nullptr;
PFNGLSECONDARYCOLOR3UIVPROC glSecondaryColor3uiv = nullptr;
PFNGLSECONDARYCOLOR3USPROC glSecondaryColor3us = nullptr;
PFNGLSECONDARYCOLOR3USVPROC glSecondaryColor3usv = nullptr;
PFNGLSECONDARYCOLORPOINTERPROC glSecondaryColorPointer = nullptr;
PFNGLWINDOWPOS2DPROC glWindowPos2d = nullptr;
PFNGLWINDOWPOS2DVPROC glWindowPos2dv = nullptr;
PFNGLWINDOWPOS2FPROC glWindowPos2f = nullptr;
PFNGLWINDOWPOS2FVPROC glWindowPos2fv = nullptr;
PFNGLWINDOWPOS2IPROC glWindowPos2i = nullptr;
PFNGLWINDOWPOS2IVPROC glWindowPos2iv = nullptr;
PFNGLWINDOWPOS2SPROC glWindowPos2s = nullptr;
PFNGLWINDOWPOS2SVPROC glWindowPos2sv = nullptr;
PFNGLWINDOWPOS3DPROC glWindowPos3d = nullptr;
PFNGLWINDOWPOS3DVPROC glWindowPos3dv = nullptr;
PFNGLWINDOWPOS3FPROC glWindowPos3f = nullptr;
PFNGLWINDOWPOS3FVPROC glWindowPos3fv = nullptr;
PFNGLWINDOWPOS3IPROC glWindowPos3i = nullptr;
PFNGLWINDOWPOS3IVPROC glWindowPos3iv = nullptr;
PFNGLWINDOWPOS3SPROC glWindowPos3s = nullptr;
PFNGLWINDOWPOS3SVPROC glWindowPos3sv = nullptr;
PFNGLBLENDCOLORPROC glBlendColor = nullptr;
PFNGLBLENDEQUATIONPROC glBlendEquation = nullptr;

#endif // GL_VERSION_1_4

//
// OpenGL v1.4
// ==========================================================================


#ifdef GL_VERSION_1_5

PFNGLGENQUERIESPROC glGenQueries = nullptr;
PFNGLDELETEQUERIESPROC glDeleteQueries = nullptr;
PFNGLISQUERYPROC glIsQuery = nullptr;
PFNGLBEGINQUERYPROC glBeginQuery = nullptr;
PFNGLENDQUERYPROC glEndQuery = nullptr;
PFNGLGETQUERYIVPROC glGetQueryiv = nullptr;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv = nullptr;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLISBUFFERPROC glIsBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = nullptr;
PFNGLMAPBUFFERPROC glMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = nullptr;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = nullptr;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv = nullptr;

#endif // GL_VERSION_1_5

//
// OpenGL v1.5
// ==========================================================================


// ==========================================================================
// OpenGL v2.0
//

#ifdef GL_VERSION_2_0

PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = nullptr;
PFNGLDRAWBUFFERSPROC glDrawBuffers = nullptr;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = nullptr;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDETACHSHADERPROC glDetachShader = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = nullptr;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = nullptr;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLGETSHADERSOURCEPROC glGetShaderSource = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLGETUNIFORMFVPROC glGetUniformfv = nullptr;
PFNGLGETUNIFORMIVPROC glGetUniformiv = nullptr;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv = nullptr;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = nullptr;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = nullptr;
PFNGLISPROGRAMPROC glIsProgram = nullptr;
PFNGLISSHADERPROC glIsShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM2IPROC glUniform2i = nullptr;
PFNGLUNIFORM3IPROC glUniform3i = nullptr;
PFNGLUNIFORM4IPROC glUniform4i = nullptr;
PFNGLUNIFORM1FVPROC glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLUNIFORM1IVPROC glUniform1iv = nullptr;
PFNGLUNIFORM2IVPROC glUniform2iv = nullptr;
PFNGLUNIFORM3IVPROC glUniform3iv = nullptr;
PFNGLUNIFORM4IVPROC glUniform4iv = nullptr;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = nullptr;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = nullptr;
PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d = nullptr;
PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv = nullptr;
PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = nullptr;
PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv = nullptr;
PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s = nullptr;
PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv = nullptr;
PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d = nullptr;
PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv = nullptr;
PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = nullptr;
PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv = nullptr;
PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s = nullptr;
PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv = nullptr;
PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d = nullptr;
PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv = nullptr;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = nullptr;
PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv = nullptr;
PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s = nullptr;
PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv = nullptr;
PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv = nullptr;
PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv = nullptr;
PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv = nullptr;
PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub = nullptr;
PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv = nullptr;
PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv = nullptr;
PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv = nullptr;
PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv = nullptr;
PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d = nullptr;
PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv = nullptr;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = nullptr;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv = nullptr;
PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv = nullptr;
PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s = nullptr;
PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv = nullptr;
PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv = nullptr;
PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv = nullptr;
PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;

#endif // GL_VERSION_2_0

//
// OpenGL v2.0
// ==========================================================================


// ==========================================================================
// OpenGL v2.1
//

#ifdef GL_VERSION_2_1

PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv = nullptr;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv = nullptr;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv = nullptr;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv = nullptr;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv = nullptr;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv = nullptr;

#endif // GL_VERSION_2_1

//
// OpenGL v2.1
// ==========================================================================


// ==========================================================================
// OpenGL 3.0
//

#ifdef GL_VERSION_3_0

PFNGLCOLORMASKIPROC glColorMaski = nullptr;
PFNGLGETBOOLEANI_VPROC glGetBooleani_v = nullptr;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v = nullptr;
PFNGLENABLEIPROC glEnablei = nullptr;
PFNGLDISABLEIPROC glDisablei = nullptr;
PFNGLISENABLEDIPROC glIsEnabledi = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback = nullptr;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback = nullptr;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying = nullptr;
PFNGLCLAMPCOLORPROC glClampColor = nullptr;
PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender = nullptr;
PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = nullptr;
PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv = nullptr;
PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv = nullptr;
PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = nullptr;
PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = nullptr;
PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = nullptr;
PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = nullptr;
PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = nullptr;
PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = nullptr;
PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = nullptr;
PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = nullptr;
PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv = nullptr;
PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv = nullptr;
PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv = nullptr;
PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv = nullptr;
PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv = nullptr;
PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv = nullptr;
PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv = nullptr;
PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv = nullptr;
PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv = nullptr;
PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv = nullptr;
PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv = nullptr;
PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv = nullptr;
PFNGLGETUNIFORMUIVPROC glGetUniformuiv = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation = nullptr;
PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation = nullptr;
PFNGLUNIFORM1UIPROC glUniform1ui = nullptr;
PFNGLUNIFORM2UIPROC glUniform2ui = nullptr;
PFNGLUNIFORM3UIPROC glUniform3ui = nullptr;
PFNGLUNIFORM4UIPROC glUniform4ui = nullptr;
PFNGLUNIFORM1UIVPROC glUniform1uiv = nullptr;
PFNGLUNIFORM2UIVPROC glUniform2uiv = nullptr;
PFNGLUNIFORM3UIVPROC glUniform3uiv = nullptr;
PFNGLUNIFORM4UIVPROC glUniform4uiv = nullptr;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv = nullptr;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv = nullptr;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv = nullptr;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv = nullptr;
PFNGLCLEARBUFFERIVPROC glClearBufferiv = nullptr;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv = nullptr;
PFNGLCLEARBUFFERFVPROC glClearBufferfv = nullptr;
PFNGLCLEARBUFFERFIPROC glClearBufferfi = nullptr;
PFNGLGETSTRINGIPROC glGetStringi = nullptr;

#endif // GL_VERSION_3_0

//
// OpenGL 3.0
// ==========================================================================


// ==========================================================================
// OpenGL v3.1
//

#ifdef GL_VERSION_3_1

PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced = nullptr;
PFNGLTEXBUFFERPROC glTexBuffer = nullptr;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex = nullptr;

#endif // GL_VERSION_3_1

//
// OpenGL v3.1
// ==========================================================================


// ==========================================================================
// OpenGL v3.2
//

#ifdef GL_VERSION_3_2

PFNGLGETINTEGER64I_VPROC glGetInteger64i_v = nullptr;
PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = nullptr;

#endif // GL_VERSION_3_2

//
// OpenGL v3.2
// ==========================================================================


// ==========================================================================
// OpenGL ES v2.0
//

#ifdef GL_ES_VERSION_2_0
#define GL_ES_VERSION_2_0 (1)

#if 0
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
PFNGLBINDTEXTUREPROC glBindTexture = nullptr;
PFNGLBLENDCOLORPROC glBlendColor = nullptr;
PFNGLBLENDEQUATIONPROC glBlendEquation = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = nullptr;
PFNGLBLENDFUNCPROC glBlendFunc = nullptr;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLCLEARPROC glClear = nullptr;
PFNGLCLEARCOLORPROC glClearColor = nullptr;
#endif // 0

PFNGLCLEARDEPTHFPROC glClearDepthf = nullptr;

#if 0
PFNGLCLEARSTENCILPROC glClearStencil = nullptr;
PFNGLCOLORMASKPROC glColorMask = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = nullptr;
PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D = nullptr;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLCULLFACEPROC glCullFace = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDELETETEXTURESPROC glDeleteTextures = nullptr;
PFNGLDEPTHFUNCPROC glDepthFunc = nullptr;
PFNGLDEPTHMASKPROC glDepthMask = nullptr;
#endif // 0

PFNGLDEPTHRANGEFPROC glDepthRangef = nullptr;

#if 0
PFNGLDETACHSHADERPROC glDetachShader = nullptr;
PFNGLDISABLEPROC glDisable = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLDRAWARRAYSPROC glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC glDrawElements = nullptr;
PFNGLENABLEPROC glEnable = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLFINISHPROC glFinish = nullptr;
PFNGLFLUSHPROC glFlush = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLFRONTFACEPROC glFrontFace = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
PFNGLGENTEXTURESPROC glGenTextures = nullptr;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = nullptr;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = nullptr;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;
PFNGLGETBOOLEANVPROC glGetBooleanv = nullptr;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = nullptr;
PFNGLGETERRORPROC glGetError = nullptr;
PFNGLGETFLOATVPROC glGetFloatv = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = nullptr;
PFNGLGETINTEGERVPROC glGetIntegerv = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
#endif // 0

PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat = nullptr;

#if 0
PFNGLGETSHADERSOURCEPROC glGetShaderSource = nullptr;
PFNGLGETSTRINGPROC glGetString = nullptr;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv = nullptr;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv = nullptr;
PFNGLGETUNIFORMFVPROC glGetUniformfv = nullptr;
PFNGLGETUNIFORMIVPROC glGetUniformiv = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = nullptr;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = nullptr;
PFNGLHINTPROC glHint = nullptr;
PFNGLISBUFFERPROC glIsBuffer = nullptr;
PFNGLISENABLEDPROC glIsEnabled = nullptr;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer = nullptr;
PFNGLISPROGRAMPROC glIsProgram = nullptr;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer = nullptr;
PFNGLISSHADERPROC glIsShader = nullptr;
PFNGLISTEXTUREPROC glIsTexture = nullptr;
PFNGLLINEWIDTHPROC glLineWidth = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLPIXELSTOREIPROC glPixelStorei = nullptr;
PFNGLPOLYGONOFFSETPROC glPolygonOffset = nullptr;
PFNGLREADPIXELSPROC glReadPixels = nullptr;
#endif // 0

PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler = nullptr;

#if 0
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = nullptr;
PFNGLSCISSORPROC glScissor = nullptr;
#endif // 0

PFNGLSHADERBINARYPROC glShaderBinary = nullptr;

#if 0
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLSTENCILFUNCPROC glStencilFunc = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = nullptr;
PFNGLSTENCILMASKPROC glStencilMask = nullptr;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = nullptr;
PFNGLSTENCILOPPROC glStencilOp = nullptr;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = nullptr;
PFNGLTEXIMAGE2DPROC glTexImage2D = nullptr;
PFNGLTEXPARAMETERFPROC glTexParameterf = nullptr;
PFNGLTEXPARAMETERFVPROC glTexParameterfv = nullptr;
PFNGLTEXPARAMETERIPROC glTexParameteri = nullptr;
PFNGLTEXPARAMETERIVPROC glTexParameteriv = nullptr;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM1FVPROC glUniform1fv = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1IVPROC glUniform1iv = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM2IPROC glUniform2i = nullptr;
PFNGLUNIFORM2IVPROC glUniform2iv = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORM3IPROC glUniform3i = nullptr;
PFNGLUNIFORM3IVPROC glUniform3iv = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLUNIFORM4IPROC glUniform4i = nullptr;
PFNGLUNIFORM4IVPROC glUniform4iv = nullptr;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = nullptr;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = nullptr;
PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = nullptr;
PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv = nullptr;
PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = nullptr;
PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv = nullptr;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = nullptr;
PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv = nullptr;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = nullptr;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLVIEWPORTPROC glViewport = nullptr;
#endif // 0

#endif // GL_ES_VERSION_2_0

//
// OpenGL ES v2.0
// ==========================================================================


// ==========================================================================
// GL_ARB_depth_buffer_float
//

#ifdef GL_ARB_depth_buffer_float
#endif // GL_ARB_depth_buffer_float


#ifdef GL_ARB_framebuffer_object

PFNGLISRENDERBUFFERPROC glIsRenderbuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = nullptr;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = nullptr;

#endif // GL_ARB_framebuffer_object

//
// GL_ARB_depth_buffer_float
// ==========================================================================


// ==========================================================================
// GL_ARB_texture_float
//

#ifdef GL_ARB_texture_float
#endif // GL_ARB_texture_float

//
// GL_ARB_texture_float
// ==========================================================================


// ==========================================================================
// GL_ARB_framebuffer_sRGB
//

#ifdef GL_ARB_framebuffer_sRGB
#endif // GL_ARB_framebuffer_sRGB

//
// GL_ARB_framebuffer_sRGB
// ==========================================================================


// ==========================================================================
// GL_ARB_half_float_vertex
//

#ifdef GL_ARB_half_float_vertex
#endif // GL_ARB_half_float_vertex

//
// GL_ARB_half_float_vertex
// ==========================================================================


// ==========================================================================
// GL_ARB_map_buffer_range
//

#ifdef GL_ARB_map_buffer_range

PFNGLMAPBUFFERRANGEPROC glMapBufferRange = nullptr;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange = nullptr;

#endif // GL_ARB_map_buffer_range

//
// GL_ARB_map_buffer_range
// ==========================================================================


// ==========================================================================
// GL_ARB_texture_compression_rgtc
//

#ifdef GL_ARB_texture_compression_rgtc
#endif // GL_ARB_texture_compression_rgtc

//
// GL_ARB_texture_compression_rgtc
// ==========================================================================


// ==========================================================================
// GL_ARB_texture_rg
//

#ifdef GL_ARB_texture_rg
#endif // GL_ARB_texture_rg

//
// GL_ARB_texture_rg
// ==========================================================================


// ==========================================================================
// GL_ARB_vertex_array_object
//

#ifdef GL_ARB_vertex_array_object

PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLISVERTEXARRAYPROC glIsVertexArray = nullptr;

#endif // GL_ARB_vertex_array_object

//
// GL_ARB_vertex_array_object
// ==========================================================================


// ==========================================================================
// GL_ARB_copy_buffer
//

#ifdef GL_ARB_copy_buffer

PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData = nullptr;

#endif // GL_ARB_copy_buffer

//
// GL_ARB_copy_buffer
// ==========================================================================


// ==========================================================================
// GL_ARB_uniform_buffer_object
//

#ifdef GL_ARB_uniform_buffer_object

PFNGLGETUNIFORMINDICESPROC glGetUniformIndices = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = nullptr;

// Already defined in 3.0.
#if 0
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v = nullptr;
#endif // 0

#endif // GL_ARB_uniform_buffer_object

//
// GL_ARB_uniform_buffer_object
// ==========================================================================


// ==========================================================================
// GL_EXT_framebuffer_blit
//

#ifdef GL_EXT_framebuffer_blit

PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = nullptr;

#endif // GL_EXT_framebuffer_blit

//
// GL_EXT_framebuffer_blit
// ==========================================================================


// ==========================================================================
// GL_EXT_framebuffer_multisample
//

#ifdef GL_EXT_framebuffer_multisample

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = nullptr;

#endif // GL_EXT_framebuffer_multisample

//
// GL_EXT_framebuffer_multisample
// ==========================================================================


// ==========================================================================
// GL_EXT_framebuffer_object
//

#ifdef GL_EXT_framebuffer_object

PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = nullptr;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = nullptr;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = nullptr;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = nullptr;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = nullptr;
PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = nullptr;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = nullptr;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = nullptr;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = nullptr;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = nullptr;

#endif // GL_EXT_framebuffer_object

//
// GL_EXT_framebuffer_object
// ==========================================================================


// ==========================================================================
// GL_EXT_packed_depth_stencil
//

#ifdef GL_EXT_packed_depth_stencil
#endif // GL_EXT_packed_depth_stencil

//
// GL_EXT_packed_depth_stencil
// ==========================================================================


// ==========================================================================
// GL_ARB_color_buffer_float
//

#ifdef GL_ARB_color_buffer_float

PFNGLCLAMPCOLORARBPROC glClampColorARB = nullptr;

#endif // GL_ARB_color_buffer_float

//
// GL_ARB_color_buffer_float
// ==========================================================================


// ==========================================================================
// GL_ARB_vertex_buffer_object
//

#ifdef GL_ARB_vertex_buffer_object

PFNGLBINDBUFFERARBPROC glBindBufferARB = nullptr;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = nullptr;
PFNGLGENBUFFERSARBPROC glGenBuffersARB = nullptr;
PFNGLISBUFFERARBPROC glIsBufferARB = nullptr;
PFNGLBUFFERDATAARBPROC glBufferDataARB = nullptr;
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = nullptr;
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB = nullptr;
PFNGLMAPBUFFERARBPROC glMapBufferARB = nullptr;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = nullptr;
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = nullptr;
PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB = nullptr;

#endif // GL_ARB_vertex_buffer_object

//
// GL_ARB_vertex_buffer_object
// ==========================================================================


// ==========================================================================
// GL_ARB_sampler_objects
//

#ifdef GL_ARB_sampler_objects

PFNGLGENSAMPLERSPROC glGenSamplers = nullptr;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = nullptr;
PFNGLISSAMPLERPROC glIsSampler = nullptr;
PFNGLBINDSAMPLERPROC glBindSampler = nullptr;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv = nullptr;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv = nullptr;
PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv = nullptr;
PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv = nullptr;
PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv = nullptr;
PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv = nullptr;

#endif // GL_ARB_sampler_objects

//
// GL_ARB_sampler_objects
// ==========================================================================


// ==========================================================================
// GL_ARB_direct_state_access
//

#ifdef GL_ARB_direct_state_access

PFNGLCREATETRANSFORMFEEDBACKSPROC glCreateTransformFeedbacks = nullptr;
PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC glTransformFeedbackBufferBase = nullptr;
PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC glTransformFeedbackBufferRange = nullptr;
PFNGLGETTRANSFORMFEEDBACKIVPROC glGetTransformFeedbackiv = nullptr;
PFNGLGETTRANSFORMFEEDBACKI_VPROC glGetTransformFeedbacki_v = nullptr;
PFNGLGETTRANSFORMFEEDBACKI64_VPROC glGetTransformFeedbacki64_v = nullptr;
PFNGLCREATEBUFFERSPROC glCreateBuffers = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage = nullptr;
PFNGLNAMEDBUFFERDATAPROC glNamedBufferData = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData = nullptr;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC glCopyNamedBufferSubData = nullptr;
PFNGLCLEARNAMEDBUFFERDATAPROC glClearNamedBufferData = nullptr;
PFNGLCLEARNAMEDBUFFERSUBDATAPROC glClearNamedBufferSubData = nullptr;
PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer = nullptr;
PFNGLMAPNAMEDBUFFERRANGEPROC glMapNamedBufferRange = nullptr;
PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer = nullptr;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC glFlushMappedNamedBufferRange = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC glGetNamedBufferParameteriv = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC glGetNamedBufferParameteri64v = nullptr;
PFNGLGETNAMEDBUFFERPOINTERVPROC glGetNamedBufferPointerv = nullptr;
PFNGLGETNAMEDBUFFERSUBDATAPROC glGetNamedBufferSubData = nullptr;
PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers = nullptr;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC glNamedFramebufferRenderbuffer = nullptr;
PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC glNamedFramebufferParameteri = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC glNamedFramebufferTexture = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC glNamedFramebufferTextureLayer = nullptr;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC glNamedFramebufferDrawBuffer = nullptr;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC glNamedFramebufferDrawBuffers = nullptr;
PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC glNamedFramebufferReadBuffer = nullptr;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC glInvalidateNamedFramebufferData = nullptr;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC glInvalidateNamedFramebufferSubData = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERIVPROC glClearNamedFramebufferiv = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC glClearNamedFramebufferuiv = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi = nullptr;
PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer = nullptr;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus = nullptr;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC glGetNamedFramebufferParameteriv = nullptr;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetNamedFramebufferAttachmentParameteriv = nullptr;
PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC glNamedRenderbufferStorage = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC glNamedRenderbufferStorageMultisample = nullptr;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC glGetNamedRenderbufferParameteriv = nullptr;
PFNGLCREATETEXTURESPROC glCreateTextures = nullptr;
PFNGLTEXTUREBUFFERPROC glTextureBuffer = nullptr;
PFNGLTEXTUREBUFFERRANGEPROC glTextureBufferRange = nullptr;
PFNGLTEXTURESTORAGE1DPROC glTextureStorage1D = nullptr;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D = nullptr;
PFNGLTEXTURESTORAGE3DPROC glTextureStorage3D = nullptr;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC glTextureStorage2DMultisample = nullptr;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC glTextureStorage3DMultisample = nullptr;
PFNGLTEXTURESUBIMAGE1DPROC glTextureSubImage1D = nullptr;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D = nullptr;
PFNGLTEXTURESUBIMAGE3DPROC glTextureSubImage3D = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC glCompressedTextureSubImage1D = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC glCompressedTextureSubImage2D = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC glCompressedTextureSubImage3D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE1DPROC glCopyTextureSubImage1D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE2DPROC glCopyTextureSubImage2D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE3DPROC glCopyTextureSubImage3D = nullptr;
PFNGLTEXTUREPARAMETERFPROC glTextureParameterf = nullptr;
PFNGLTEXTUREPARAMETERFVPROC glTextureParameterfv = nullptr;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri = nullptr;
PFNGLTEXTUREPARAMETERIIVPROC glTextureParameterIiv = nullptr;
PFNGLTEXTUREPARAMETERIUIVPROC glTextureParameterIuiv = nullptr;
PFNGLTEXTUREPARAMETERIVPROC glTextureParameteriv = nullptr;
PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap = nullptr;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit = nullptr;
PFNGLGETTEXTUREIMAGEPROC glGetTextureImage = nullptr;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC glGetCompressedTextureImage = nullptr;
PFNGLGETTEXTURELEVELPARAMETERFVPROC glGetTextureLevelParameterfv = nullptr;
PFNGLGETTEXTURELEVELPARAMETERIVPROC glGetTextureLevelParameteriv = nullptr;
PFNGLGETTEXTUREPARAMETERFVPROC glGetTextureParameterfv = nullptr;
PFNGLGETTEXTUREPARAMETERIIVPROC glGetTextureParameterIiv = nullptr;
PFNGLGETTEXTUREPARAMETERIUIVPROC glGetTextureParameterIuiv = nullptr;
PFNGLGETTEXTUREPARAMETERIVPROC glGetTextureParameteriv = nullptr;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays = nullptr;
PFNGLDISABLEVERTEXARRAYATTRIBPROC glDisableVertexArrayAttrib = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERSPROC glVertexArrayVertexBuffers = nullptr;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat = nullptr;
PFNGLVERTEXARRAYATTRIBIFORMATPROC glVertexArrayAttribIFormat = nullptr;
PFNGLVERTEXARRAYATTRIBLFORMATPROC glVertexArrayAttribLFormat = nullptr;
PFNGLVERTEXARRAYBINDINGDIVISORPROC glVertexArrayBindingDivisor = nullptr;
PFNGLGETVERTEXARRAYIVPROC glGetVertexArrayiv = nullptr;
PFNGLGETVERTEXARRAYINDEXEDIVPROC glGetVertexArrayIndexediv = nullptr;
PFNGLGETVERTEXARRAYINDEXED64IVPROC glGetVertexArrayIndexed64iv = nullptr;
PFNGLCREATESAMPLERSPROC glCreateSamplers = nullptr;
PFNGLCREATEPROGRAMPIPELINESPROC glCreateProgramPipelines = nullptr;
PFNGLCREATEQUERIESPROC glCreateQueries = nullptr;
PFNGLGETQUERYBUFFEROBJECTI64VPROC glGetQueryBufferObjecti64v = nullptr;
PFNGLGETQUERYBUFFEROBJECTIVPROC glGetQueryBufferObjectiv = nullptr;
PFNGLGETQUERYBUFFEROBJECTUI64VPROC glGetQueryBufferObjectui64v = nullptr;
PFNGLGETQUERYBUFFEROBJECTUIVPROC glGetQueryBufferObjectuiv = nullptr;

#endif // GL_ARB_direct_state_access

//
// GL_ARB_direct_state_access
// ==========================================================================


// ==========================================================================
// GL_ARB_buffer_storage
//

#ifdef GL_ARB_buffer_storage

PFNGLBUFFERSTORAGEPROC glBufferStorage = nullptr;

#endif // GL_ARB_buffer_storage

//
// GL_ARB_buffer_storage
// ==========================================================================


// ==========================================================================
// GL_ARB_separate_shader_objects
//

#ifdef GL_ARB_separate_shader_objects

PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = nullptr;
PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram = nullptr;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = nullptr;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = nullptr;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = nullptr;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline = nullptr;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv = nullptr;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = nullptr;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i = nullptr;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv = nullptr;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f = nullptr;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv = nullptr;
PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d = nullptr;
PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv = nullptr;
PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui = nullptr;
PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv = nullptr;
PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i = nullptr;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv = nullptr;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f = nullptr;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv = nullptr;
PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d = nullptr;
PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv = nullptr;
PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui = nullptr;
PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv = nullptr;
PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i = nullptr;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv = nullptr;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = nullptr;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv = nullptr;
PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d = nullptr;
PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv = nullptr;
PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui = nullptr;
PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv = nullptr;
PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i = nullptr;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv = nullptr;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f = nullptr;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv = nullptr;
PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d = nullptr;
PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv = nullptr;
PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui = nullptr;
PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv = nullptr;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline = nullptr;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog = nullptr;

#endif // GL_ARB_separate_shader_objects

//
// GL_ARB_separate_shader_objects
// ==========================================================================


// ==========================================================================
// GL_ARB_depth_clamp
//

#ifdef GL_ARB_depth_clamp
#endif // GL_ARB_depth_clamp

//
// GL_ARB_depth_clamp
// ==========================================================================


// ==========================================================================
// GL_ARB_draw_elements_base_vertex
//

#ifdef GL_ARB_draw_elements_base_vertex

PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = nullptr;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex = nullptr;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex = nullptr;

#endif // GL_ARB_draw_elements_base_vertex

//
// GL_ARB_draw_elements_base_vertex
// ==========================================================================


// ==========================================================================
// GL_ARB_fragment_coord_conventions
//

#ifdef GL_ARB_fragment_coord_conventions
#endif // GL_ARB_fragment_coord_conventions

//
// GL_ARB_fragment_coord_conventions
// ==========================================================================


// ==========================================================================
// GL_ARB_provoking_vertex
//

#ifdef GL_ARB_provoking_vertex

PFNGLPROVOKINGVERTEXPROC glProvokingVertex = nullptr;

#endif // GL_ARB_provoking_vertex

//
// GL_ARB_provoking_vertex
// ==========================================================================


// ==========================================================================
// GL_ARB_provoking_vertex
//

#ifdef GL_ARB_seamless_cube_map
#endif // GL_ARB_seamless_cube_map

//
// GL_ARB_provoking_vertex
// ==========================================================================


// ==========================================================================
// GL_ARB_sync
//

#ifdef GL_ARB_sync

PFNGLFENCESYNCPROC glFenceSync = nullptr;
PFNGLISSYNCPROC glIsSync = nullptr;
PFNGLDELETESYNCPROC glDeleteSync = nullptr;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync = nullptr;
PFNGLWAITSYNCPROC glWaitSync = nullptr;
PFNGLGETINTEGER64VPROC glGetInteger64v = nullptr;
PFNGLGETSYNCIVPROC glGetSynciv = nullptr;

#endif // GL_ARB_sync

//
// GL_ARB_sync
// ==========================================================================


// ==========================================================================
// GL_ARB_texture_multisample
//

#ifdef GL_ARB_texture_multisample

PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample = nullptr;
PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv = nullptr;
PFNGLSAMPLEMASKIPROC glSampleMaski = nullptr;

#endif // GL_ARB_texture_multisample

//
// GL_ARB_texture_multisample
// ==========================================================================


} // detail
} // bstone
