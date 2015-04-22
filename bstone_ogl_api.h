/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//
// A dynamic loader for an OpenGL API.
//


#ifndef BSTONE_OGL_API_H
#define BSTONE_OGL_API_H


#include "SDL.h"


// Avoid __declspec(dllimport).
#ifdef __WIN32__
#ifndef _GDI32_
#define _GDI32_
#endif // _GDI32_
#endif // __WIN32__

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif // GL_GLEXT_PROTOTYPES

#if defined(BSTONE_USE_GLES)
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif

#include "bstone_ogl_version.h"


namespace bstone {


// A dynamic loader for an OpenGL API.
class OglApi {
public:
    // Initializes the loader.
    static bool initialize();

    // Uninitializes the loader.
    static void uninitialize();

    // Returns true if the loader is initialized or
    // false otherwise.
    static bool is_initialized();

    // Returns the company responsible for this GL implementation.
    static const std::string& get_vendor();

    // Returns the name of the renderer.
    static const std::string& get_renderer();

    // Returns a version.
    static const OglVersion& get_version();

    // Returns true if R/RG texture formats are supported or
    // false otherwise.
    static bool has_ext_texture_rg();

    // Returns API independent constant GL_R8.
    static GLenum get_gl_r8();

    // Returns API independent constant GL_RED.
    static GLenum get_gl_red();

private:
    OglApi();

    OglApi(
        const OglApi& that);

    ~OglApi();

    OglApi& operator=(
        const OglApi& that);
}; // class OglApi


} // namespace bstone


#endif // BSTONE_OGL_API_H
