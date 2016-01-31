/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// A dynamic loader for an OpenGL API.
//


#include "bstone_ogl_api.h"
#include <list>
#include "bstone_log.h"


namespace {


bool is_initialized_ = false;
std::string vendor_;
std::string renderer_;
bstone::OglVersion version_;


} // namespace


///////////////////////////////////////////////////////////////////////////
// API v1.1
//

#if !defined(BSTONE_OGL_DIRECT_LINK)


namespace {


void (APIENTRY* glBindTexture_)(
    GLenum target,
    GLuint texture) = nullptr;

void (APIENTRY* glClear_)(
    GLbitfield mask) = nullptr;

void (APIENTRY* glClearColor_)(
    GLclampf red,
    GLclampf green,
    GLclampf blue,
    GLclampf alpha) = nullptr;

void (APIENTRY* glDeleteTextures_)(
    GLsizei n,
    const GLuint* textures) = nullptr;

void (APIENTRY* glDisable_)(
    GLenum cap) = nullptr;

void (APIENTRY* glDrawArrays_)(
    GLenum mode,
    GLint first,
    GLsizei count) = nullptr;

void (APIENTRY* glEnable_)(
    GLenum cap) = nullptr;

void (APIENTRY* glGenTextures_)(
    GLsizei n,
    GLuint* textures) = nullptr;

GLenum (APIENTRY* glGetError_)() = nullptr;

const GLubyte* (APIENTRY* glGetString_)(
    GLenum name) = nullptr;

void (APIENTRY* glTexImage2D_)(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = nullptr;

void (APIENTRY* glTexParameteri_)(
    GLenum target,
    GLenum pname,
    GLint param) = nullptr;

void (APIENTRY* glTexSubImage2D_)(
    GLenum target,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = nullptr;

void (APIENTRY* glViewport_)(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height) = nullptr;


} // namespace


void APIENTRY glBindTexture(
    GLenum target,
    GLuint texture)
{
    ::glBindTexture_(
        target,
        texture);
}

void APIENTRY glClear(
    GLbitfield mask)
{
    ::glClear_(
        mask);
}

void APIENTRY glClearColor(
    GLclampf red,
    GLclampf green,
    GLclampf blue,
    GLclampf alpha)
{
    ::glClearColor_(
        red,
        green,
        blue,
        alpha);
}

void APIENTRY glDeleteTextures(
    GLsizei n,
    const GLuint* textures)
{
    ::glDeleteTextures_(
        n,
        textures);
}

void APIENTRY glDisable(
    GLenum cap)
{
    ::glDisable_(
        cap);
}

void APIENTRY glDrawArrays(
    GLenum mode,
    GLint first,
    GLsizei count)
{
    ::glDrawArrays_(
        mode,
        first,
        count);
}

void APIENTRY glEnable(
    GLenum cap)
{
    ::glEnable_(
        cap);
}

void APIENTRY glGenTextures(
    GLsizei n,
    GLuint* textures)
{
    ::glGenTextures_(
        n,
        textures);
}

GLenum APIENTRY glGetError()
{
    return ::glGetError_();
}

const GLubyte* APIENTRY glGetString(
    GLenum name)
{
    return ::glGetString_(
        name);
}

void APIENTRY glTexImage2D(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* pixels)
{
    ::glTexImage2D_(
        target,
        level,
        internalformat,
        width,
        height,
        border,
        format,
        type,
        pixels);
}

void APIENTRY glTexParameteri(
    GLenum target,
    GLenum pname,
    GLint param)
{
    ::glTexParameteri_(
        target,
        pname,
        param);
}

void APIENTRY glTexSubImage2D(
    GLenum target,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid* pixels)
{
    ::glTexSubImage2D_(
        target,
        level,
        xoffset,
        yoffset,
        width,
        height,
        format,
        type,
        pixels);
}

void APIENTRY glViewport(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height)
{
    ::glViewport_(
        x,
        y,
        width,
        height);
}


///////////////////////////////////////////////////////////////////////////
// API v1.2+
//

namespace {


PFNGLACTIVETEXTUREPROC glActiveTexture_ = nullptr;
PFNGLATTACHSHADERPROC glAttachShader_ = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer_ = nullptr;
PFNGLBUFFERDATAPROC glBufferData_ = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader_ = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram_ = nullptr;
PFNGLCREATESHADERPROC glCreateShader_ = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers_ = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram_ = nullptr;
PFNGLDELETESHADERPROC glDeleteShader_ = nullptr;
PFNGLDETACHSHADERPROC glDetachShader_ = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers_ = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_ = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv_ = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv_ = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ = nullptr;
PFNGLISPROGRAMPROC glIsProgram_ = nullptr;
PFNGLISSHADERPROC glIsShader_ = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram_ = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource_ = nullptr;
PFNGLUNIFORM1IPROC glUniform1i_ = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram_ = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ = nullptr;


} // namespace


void APIENTRY glActiveTexture(
    GLenum texture)
{
    ::glActiveTexture_(
        texture);
}

void APIENTRY glAttachShader(
    GLuint program,
    GLuint shader)
{
    ::glAttachShader_(
        program,
        shader);
}

void APIENTRY glBindBuffer(
    GLenum target,
    GLuint buffer)
{
    ::glBindBuffer_(
        target,
        buffer);
}

void APIENTRY glBufferData(
    GLenum target,
    GLsizeiptr size,
    const GLvoid* data,
    GLenum usage)
{
    ::glBufferData_(
        target,
        size,
        data,
        usage);
}

void APIENTRY glCompileShader(
    GLuint shader)
{
    ::glCompileShader_(
        shader);
}

GLuint APIENTRY glCreateProgram()
{
    return ::glCreateProgram_();
}

GLuint APIENTRY glCreateShader(
    GLenum type)
{
    return ::glCreateShader_(
        type);
}

void APIENTRY glDeleteBuffers(
    GLsizei n,
    const GLuint* buffers)
{
    ::glDeleteBuffers_(
        n,
        buffers);
}

void APIENTRY glDeleteProgram(
    GLuint program)
{
    ::glDeleteProgram_(
        program);
}

void APIENTRY glDeleteShader(
    GLuint shader)
{
    ::glDeleteShader_(
        shader);
}

void APIENTRY glDetachShader(
    GLuint program,
    GLuint shader)
{
    ::glDetachShader_(
        program,
        shader);
}

void APIENTRY glDisableVertexAttribArray(
    GLuint index)
{
    ::glDisableVertexAttribArray_(
        index);
}

void APIENTRY glEnableVertexAttribArray(
    GLuint index)
{
    ::glEnableVertexAttribArray_(
        index);
}

void APIENTRY glGenBuffers(
    GLsizei n,
    GLuint* buffers)
{
    ::glGenBuffers_(
        n,
        buffers);
}

GLint APIENTRY glGetAttribLocation(
    GLuint program,
    const GLchar* name)
{
    return ::glGetAttribLocation_(
        program,
        name);
}

void APIENTRY glGetProgramInfoLog(
    GLuint program,
    GLsizei bufSize,
    GLsizei* length,
    GLchar* infoLog)
{
    ::glGetProgramInfoLog_(
        program,
        bufSize,
        length,
        infoLog);
}

void APIENTRY glGetProgramiv(
    GLuint program,
    GLenum pname,
    GLint* params)
{
    ::glGetProgramiv_(
        program,
        pname,
        params);
}

void APIENTRY glGetShaderInfoLog(
    GLuint shader,
    GLsizei bufSize,
    GLsizei* length,
    GLchar* infoLog)
{
    ::glGetShaderInfoLog_(
        shader,
        bufSize,
        length,
        infoLog);
}

void APIENTRY glGetShaderiv(
    GLuint shader,
    GLenum pname,
    GLint* params)
{
    ::glGetShaderiv_(
        shader,
        pname,
        params);
}

GLint APIENTRY glGetUniformLocation(
    GLuint program,
    const GLchar* name)
{
    return ::glGetUniformLocation_(
        program,
        name);
}

GLboolean APIENTRY glIsProgram(
    GLuint program)
{
    return ::glIsProgram_(
        program);
}

GLboolean APIENTRY glIsShader(
    GLuint shader)
{
    return ::glIsShader_(
        shader);
}

void APIENTRY glLinkProgram(
    GLuint program)
{
    ::glLinkProgram_(
        program);
}

void APIENTRY glShaderSource(
    GLuint shader,
    GLsizei count,
    const GLchar* const* string,
    const GLint* length)
{
    ::glShaderSource_(
        shader,
        count,
        string,
        length);
}

void APIENTRY glUniform1i(
    GLint location,
    GLint v0)
{
    ::glUniform1i_(
        location,
        v0);
}

void APIENTRY glUniformMatrix4fv(
    GLint location,
    GLsizei count,
    GLboolean transpose,
    const GLfloat* value)
{
    ::glUniformMatrix4fv_(
        location,
        count,
        transpose,
        value);
}

void APIENTRY glUseProgram(
    GLuint program)
{
    ::glUseProgram_(
        program);
}

void APIENTRY glVertexAttribPointer(
    GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const GLvoid* pointer)
{
    ::glVertexAttribPointer_(
        index,
        size,
        type,
        normalized,
        stride,
        pointer);
}


#endif // BSTONE_OGL_DIRECT_LINK


///////////////////////////////////////////////////////////////////////////
// OglApi
//

namespace {


using Strings = std::list<std::string>;
using StringsCIt = Strings::const_iterator;

// Pointer-to-object to pointer-to-function cast
template<typename T>
union Cast {
public:
    explicit Cast(
        void* object) :
            object_(object)
    {
    }

    operator T()
    {
        return function_;
    }

private:
    T function_;
    void* object_;
}; // union Cast


template<typename T>
void ogl_api_get_base_symbol(
    const char* symbol_name,
    T& symbol,
    Strings& missing_symbols)
{
    symbol = Cast<T>(::SDL_GL_GetProcAddress(
        symbol_name));

    if (!symbol) {
        missing_symbols.push_back(
            symbol_name);
    }
}

template<typename T>
void ogl_api_get_ext_symbol(
    const char* symbol_name,
    T& symbol)
{
    symbol = Cast<T>(::SDL_GL_GetProcAddress(
        symbol_name));
}


} // namespace


namespace bstone {


bool OglApi::initialize()
{
    uninitialize();

#if !defined(BSTONE_OGL_DIRECT_LINK)
    if (!::SDL_GL_GetCurrentContext()) {
        bstone::Log::write_error(
            "OGLAPI: No current context.");

        return false;
    }


    //
    // Base API
    //

    Strings missing_symbols;

    ogl_api_get_base_symbol(
        "glBindTexture",
        glBindTexture_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glClear",
        glClear_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glClearColor",
        glClearColor_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDeleteTextures",
        glDeleteTextures_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDisable",
        glDisable_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDrawArrays",
        glDrawArrays_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glEnable",
        glEnable_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGenTextures",
        glGenTextures_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetError",
        glGetError_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetString",
        glGetString_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glTexImage2D",
        glTexImage2D_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glTexParameteri",
        glTexParameteri_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glTexSubImage2D",
        glTexSubImage2D_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glViewport",
        glViewport_,
        missing_symbols);


    //
    // Required extensions
    //

    ogl_api_get_base_symbol(
        "glActiveTexture",
        glActiveTexture_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glAttachShader",
        glAttachShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glBindBuffer",
        glBindBuffer_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glBufferData",
        glBufferData_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glCompileShader",
        glCompileShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glCreateProgram",
        glCreateProgram_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glCreateShader",
        glCreateShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDeleteBuffers",
        glDeleteBuffers_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDeleteProgram",
        glDeleteProgram_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDeleteShader",
        glDeleteShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDetachShader",
        glDetachShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glDisableVertexAttribArray",
        glDisableVertexAttribArray_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glEnableVertexAttribArray",
        glEnableVertexAttribArray_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGenBuffers",
        glGenBuffers_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetAttribLocation",
        glGetAttribLocation_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetProgramInfoLog",
        glGetProgramInfoLog_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetProgramiv",
        glGetProgramiv_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetShaderInfoLog",
        glGetShaderInfoLog_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetShaderiv",
        glGetShaderiv_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glGetUniformLocation",
        glGetUniformLocation_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glIsProgram",
        glIsProgram_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glIsShader",
        glIsShader_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glLinkProgram",
        glLinkProgram_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glShaderSource",
        glShaderSource_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glUniform1i",
        glUniform1i_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glUniformMatrix4fv",
        glUniformMatrix4fv_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glUseProgram",
        glUseProgram_,
        missing_symbols);

    ogl_api_get_base_symbol(
        "glVertexAttribPointer",
        glVertexAttribPointer_,
        missing_symbols);
#endif // BSTONE_OGL_DIRECT_LINK


#if !defined(BSTONE_OGL_DIRECT_LINK)
    if (!missing_symbols.empty()) {
        for(const auto& missing_symbol : missing_symbols) {
            bstone::Log::write_error(
                "OGLAPI: Missing symbol: {}.", missing_symbol);
        }

        uninitialize();

        return false;
    }
#endif // BSTONE_OGL_DIRECT_LINK


    // Vendor

    auto vendor = ::glGetString(
        GL_VENDOR);

    if (!vendor) {
        bstone::Log::write_error(
            "OGLAPI: Failed to get a vendor.");

        uninitialize();

        return false;
    }


    // Renderer

    auto renderer = ::glGetString(
        GL_RENDERER);

    if (!renderer) {
        bstone::Log::write_error(
            "OGLAPI: Failed to get a renderer.");

        uninitialize();

        return false;
    }


    // Version

    auto version_string = ::glGetString(
        GL_VERSION);

    if (!version_string) {
        bstone::Log::write_error(
            "OGLAPI: Failed to get a version.");

        uninitialize();

        return false;
    }

    version_.set(reinterpret_cast<const char*>(
        version_string));


    is_initialized_ = true;
    vendor_ = reinterpret_cast<const char*>(vendor);
    renderer_ = reinterpret_cast<const char*>(renderer);

    return true;
}

void OglApi::uninitialize()
{
#if !defined(BSTONE_OGL_DIRECT_LINK)
    glBindTexture_ = nullptr;
    glClear_ = nullptr;
    glClearColor_ = nullptr;
    glDeleteTextures_ = nullptr;
    glDisable_ = nullptr;
    glDrawArrays_ = nullptr;
    glEnable_ = nullptr;
    glGenTextures_ = nullptr;
    glGetError_ = nullptr;
    glGetString_ = nullptr;
    glTexImage2D_ = nullptr;
    glTexParameteri_ = nullptr;
    glTexSubImage2D_ = nullptr;
    glViewport_ = nullptr;

    glIsShader_ = nullptr;
    glActiveTexture_ = nullptr;
    glAttachShader_ = nullptr;
    glBindBuffer_ = nullptr;
    glBufferData_ = nullptr;
    glCompileShader_ = nullptr;
    glCreateProgram_ = nullptr;
    glCreateShader_ = nullptr;
    glDeleteBuffers_ = nullptr;
    glDeleteProgram_ = nullptr;
    glDeleteShader_ = nullptr;
    glDetachShader_ = nullptr;
    glDisableVertexAttribArray_ = nullptr;
    glEnableVertexAttribArray_ = nullptr;
    glGenBuffers_ = nullptr;
    glGetAttribLocation_ = nullptr;
    glGetProgramInfoLog_ = nullptr;
    glGetProgramiv_ = nullptr;
    glGetShaderInfoLog_ = nullptr;
    glGetShaderiv_ = nullptr;
    glGetUniformLocation_ = nullptr;
    glIsProgram_ = nullptr;
    glLinkProgram_ = nullptr;
    glShaderSource_ = nullptr;
    glUniform1i_ = nullptr;
    glUniformMatrix4fv_ = nullptr;
    glUseProgram_ = nullptr;
    glVertexAttribPointer_ = nullptr;
#endif // BSTONE_OGL_DIRECT_LINK

    is_initialized_ = false;
    vendor_.clear();
    renderer_.clear();
    version_.reset();
}

bool OglApi::is_initialized()
{
    return is_initialized_;
}

const std::string& OglApi::get_vendor()
{
    return vendor_;
}

const std::string& OglApi::get_renderer()
{
    return renderer_;
}

const OglVersion& OglApi::get_version()
{
    return version_;
}


} // bstone
