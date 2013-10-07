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

#if defined(USE_GLES)
#include "GLES2/gl2.h"
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
