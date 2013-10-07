//
// A dynamic loader for an OpenGL API.
//


#include "bstone_ogl_api.h"

#include <list>


namespace {


bool is_initialized_ = false;
std::string vendor_;
std::string renderer_;
bstone::OglVersion version_;


} // namespace


///////////////////////////////////////////////////////////////////////////
// API v1.1
//

#if !defined(OGL_DIRECT_LINK)

namespace {


void (APIENTRY* glBindTexture_)(
    GLenum target,
    GLuint texture) = NULL;

void (APIENTRY* glClear_)(
    GLbitfield mask) = NULL;

void (APIENTRY* glClearColor_)(
    GLclampf red,
    GLclampf green,
    GLclampf blue,
    GLclampf alpha) = NULL;

void (APIENTRY* glDeleteTextures_)(
    GLsizei n,
    const GLuint* textures) = NULL;

void (APIENTRY* glDisable_)(
    GLenum cap) = NULL;

void (APIENTRY* glDrawArrays_)(
    GLenum mode,
    GLint first,
    GLsizei count) = NULL;

void (APIENTRY* glEnable_)(
    GLenum cap) = NULL;

void (APIENTRY* glGenTextures_)(
    GLsizei n,
    GLuint* textures) = NULL;

GLenum (APIENTRY* glGetError_)() = NULL;

const GLubyte* (APIENTRY* glGetString_)(
    GLenum name) = NULL;

void (APIENTRY* glTexImage1D_)(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = NULL;

void (APIENTRY* glTexImage2D_)(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = NULL;

void (APIENTRY* glTexParameteri_)(
    GLenum target,
    GLenum pname,
    GLint param) = NULL;

void (APIENTRY* glTexSubImage1D_)(
    GLenum target,
    GLint level,
    GLint xoffset,
    GLsizei width,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = NULL;

void (APIENTRY* glTexSubImage2D_)(
    GLenum target,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid* pixels) = NULL;

void (APIENTRY* glViewport_)(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height) = NULL;


} // namespace


void APIENTRY glBindTexture(
    GLenum target,
    GLuint texture)
{
    glBindTexture_(target, texture);
}

void APIENTRY glClear(
    GLbitfield mask)
{
    glClear_(mask);
}

void APIENTRY glClearColor(
    GLclampf red,
    GLclampf green,
    GLclampf blue,
    GLclampf alpha)
{
    glClearColor_(red, green, blue, alpha);
}

void APIENTRY glDeleteTextures(
    GLsizei n,
    const GLuint* textures)
{
    glDeleteTextures_(n, textures);
}

void APIENTRY glDisable(
    GLenum cap)
{
    glDisable_(cap);
}

void APIENTRY glDrawArrays(
    GLenum mode,
    GLint first,
    GLsizei count)
{
    glDrawArrays_(mode, first, count);
}

void APIENTRY glEnable(
    GLenum cap)
{
    glEnable_(cap);
}

void APIENTRY glGenTextures(
    GLsizei n,
    GLuint* textures)
{
    glGenTextures_(n, textures);
}

GLenum APIENTRY glGetError()
{
    return glGetError_();
}

const GLubyte* APIENTRY glGetString(
    GLenum name)
{
    return glGetString_(name);
}

void APIENTRY glTexImage1D(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLint border,
    GLenum format,
    GLenum type,
    const GLvoid* pixels)
{
    glTexImage1D_(target, level, internalformat,
        width, border, format, type, pixels);
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
    glTexImage2D_(target, level, internalformat,
        width, height, border, format, type, pixels);
}

void APIENTRY glTexParameteri(
    GLenum target,
    GLenum pname,
    GLint param)
{
    glTexParameteri_(target, pname, param);
}

void APIENTRY glTexSubImage1D(
    GLenum target,
    GLint level,
    GLint xoffset,
    GLsizei width,
    GLenum format,
    GLenum type,
    const GLvoid* pixels)
{
    glTexSubImage1D_(target, level, xoffset, width, format, type, pixels);
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
    glTexSubImage2D_(target, level, xoffset, yoffset,
        width, height, format, type, pixels);
}

void APIENTRY glViewport(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height)
{
    glViewport_(x, y, width, height);
}


///////////////////////////////////////////////////////////////////////////
// API v1.2+
//

namespace {


PFNGLACTIVETEXTUREPROC glActiveTexture_ = NULL;
PFNGLATTACHSHADERPROC glAttachShader_ = NULL;
PFNGLBINDBUFFERPROC glBindBuffer_ = NULL;
PFNGLBUFFERDATAPROC glBufferData_ = NULL;
PFNGLCOMPILESHADERPROC glCompileShader_ = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram_ = NULL;
PFNGLCREATESHADERPROC glCreateShader_ = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers_ = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram_ = NULL;
PFNGLDELETESHADERPROC glDeleteShader_ = NULL;
PFNGLDETACHSHADERPROC glDetachShader_ = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ = NULL;
PFNGLGENBUFFERSPROC glGenBuffers_ = NULL;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_ = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv_ = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv_ = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ = NULL;
PFNGLISPROGRAMPROC glIsProgram_ = NULL;
PFNGLISSHADERPROC glIsShader_ = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram_ = NULL;
PFNGLSHADERSOURCEPROC glShaderSource_ = NULL;
PFNGLUNIFORM1IPROC glUniform1i_ = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ = NULL;
PFNGLUSEPROGRAMPROC glUseProgram_ = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ = NULL;


} // namespace


GLAPI void APIENTRY glActiveTexture(
    GLenum texture)
{
    glActiveTexture_(texture);
}

GLAPI void APIENTRY glAttachShader(
    GLuint program,
    GLuint shader)
{
    glAttachShader_(program, shader);
}

GLAPI void APIENTRY glBindBuffer(
    GLenum target,
    GLuint buffer)
{
    glBindBuffer_(target, buffer);
}

GLAPI void APIENTRY glBufferData(
    GLenum target,
    GLsizeiptr size,
    const GLvoid* data,
    GLenum usage)
{
    glBufferData_(target, size, data, usage);
}

GLAPI void APIENTRY glCompileShader(
    GLuint shader)
{
    glCompileShader_(shader);
}

GLAPI GLuint APIENTRY glCreateProgram()
{
    return glCreateProgram_();
}

GLAPI GLuint APIENTRY glCreateShader(
    GLenum type)
{
    return glCreateShader_(type);
}

GLAPI void APIENTRY glDeleteBuffers(
    GLsizei n,
    const GLuint* buffers)
{
    glDeleteBuffers_(n, buffers);
}

GLAPI void APIENTRY glDeleteProgram(
    GLuint program)
{
    glDeleteProgram_(program);
}

GLAPI void APIENTRY glDeleteShader(
    GLuint shader)
{
    glDeleteShader_(shader);
}

GLAPI void APIENTRY glDetachShader(
    GLuint program,
    GLuint shader)
{
    glDetachShader_(program, shader);
}

GLAPI void APIENTRY glDisableVertexAttribArray(
    GLuint index)
{
    glDisableVertexAttribArray_(index);
}

GLAPI void APIENTRY glEnableVertexAttribArray(
    GLuint index)
{
    glEnableVertexAttribArray_(index);
}

GLAPI void APIENTRY glGenBuffers(
    GLsizei n,
    GLuint* buffers)
{
    glGenBuffers_(n, buffers);
}

GLAPI GLint APIENTRY glGetAttribLocation(
    GLuint program,
    const GLchar* name)
{
    return glGetAttribLocation_(program, name);
}

GLAPI void APIENTRY glGetProgramInfoLog(
    GLuint program,
    GLsizei bufSize,
    GLsizei* length,
    GLchar* infoLog)
{
    glGetProgramInfoLog_(program, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetProgramiv(
    GLuint program,
    GLenum pname,
    GLint* params)
{
    glGetProgramiv_(program, pname, params);
}

GLAPI void APIENTRY glGetShaderInfoLog(
    GLuint shader,
    GLsizei bufSize,
    GLsizei* length,
    GLchar* infoLog)
{
    glGetShaderInfoLog_(shader, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetShaderiv(
    GLuint shader,
    GLenum pname,
    GLint* params)
{
    glGetShaderiv_(shader, pname, params);
}

GLAPI GLint APIENTRY glGetUniformLocation(
    GLuint program,
    const GLchar* name)
{
    return glGetUniformLocation_(program, name);
}

GLAPI GLboolean APIENTRY glIsProgram(
    GLuint program)
{
    return glIsProgram_(program);
}

GLAPI GLboolean APIENTRY glIsShader(
    GLuint shader)
{
    return glIsShader_(shader);
}

GLAPI void APIENTRY glLinkProgram(
    GLuint program)
{
    glLinkProgram_(program);
}

GLAPI void APIENTRY glShaderSource(
    GLuint shader,
    GLsizei count,
    const GLchar** string,
    const GLint* length)
{
    glShaderSource_(shader, count, string, length);
}

GLAPI void APIENTRY glUniform1i(
    GLint location,
    GLint v0)
{
    glUniform1i_(location, v0);
}

GLAPI void APIENTRY glUniformMatrix4fv(
    GLint location,
    GLsizei count,
    GLboolean transpose,
    const GLfloat* value)
{
    glUniformMatrix4fv_(location, count, transpose, value);
}

GLAPI void APIENTRY glUseProgram(
    GLuint program)
{
    glUseProgram_(program);
}

GLAPI void APIENTRY glVertexAttribPointer(
    GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const GLvoid* pointer)
{
    glVertexAttribPointer_(index, size, type, normalized, stride, pointer);
}

#endif

///////////////////////////////////////////////////////////////////////////
// OglApi
//

namespace {


typedef std::list<std::string> Strings;
typedef Strings::const_iterator StringsCIt;


template<class T>
void ogl_api_get_base_symbol(
    const char* symbol_name,
    T& symbol,
    Strings& missing_symbols)
{
    symbol = reinterpret_cast<T>(::SDL_GL_GetProcAddress(symbol_name));

    if (symbol == NULL)
        missing_symbols.push_back(symbol_name);
}

template<class T>
void ogl_api_get_ext_symbol(
    const char* symbol_name,
    T& symbol)
{
    symbol = reinterpret_cast<T>(::SDL_GL_GetProcAddress(symbol_name));
}


} // namespace


namespace bstone {


// (static)
bool OglApi::initialize()
{
    uninitialize();

#if !defined(OGL_DIRECT_LINK)

    if (::SDL_GL_GetCurrentContext() == NULL) {
        ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s.", "No current context");

        return false;
    }


    //
    // Base API
    //

    Strings missing_symbols;

    ogl_api_get_base_symbol(
        "glBindTexture", glBindTexture_, missing_symbols);

    ogl_api_get_base_symbol(
        "glClear", glClear_, missing_symbols);

    ogl_api_get_base_symbol(
        "glClearColor", glClearColor_, missing_symbols);

    ogl_api_get_base_symbol(
        "glDeleteTextures", glDeleteTextures_, missing_symbols);

    ogl_api_get_base_symbol(
        "glDisable", glDisable_, missing_symbols);

    ogl_api_get_base_symbol(
        "glDrawArrays", glDrawArrays_, missing_symbols);

    ogl_api_get_base_symbol(
        "glEnable", glEnable_, missing_symbols);

    ogl_api_get_base_symbol(
        "glGenTextures", glGenTextures_, missing_symbols);

    ogl_api_get_base_symbol(
        "glGetError", glGetError_, missing_symbols);

    ogl_api_get_base_symbol(
        "glGetString", glGetString_, missing_symbols);

    ogl_api_get_base_symbol(
        "glTexImage1D", glTexImage1D_, missing_symbols);

    ogl_api_get_base_symbol(
        "glTexImage2D", glTexImage2D_, missing_symbols);

    ogl_api_get_base_symbol(
        "glTexParameteri", glTexParameteri_, missing_symbols);

    ogl_api_get_base_symbol(
        "glTexSubImage1D", glTexSubImage1D_, missing_symbols);

    ogl_api_get_base_symbol(
        "glTexSubImage2D", glTexSubImage2D_, missing_symbols);

    ogl_api_get_base_symbol(
        "glViewport", glViewport_, missing_symbols);

    if (!missing_symbols.empty()) {
        for(StringsCIt i = missing_symbols.begin();
            i != missing_symbols.end(); ++i)
        {
            ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "OGLAPI: Missing symbol: %s", i->c_str());
        }

        uninitialize();
        return false;
    }
#endif

    // Vendor

    const GLubyte* vendor = ::glGetString(GL_VENDOR);

    if (vendor == NULL) {
        ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s", "Failed to get a vendor.");

        uninitialize();
        return false;
    }

    // Renderer

    const GLubyte* renderer = ::glGetString(GL_RENDERER);

    if (renderer == NULL) {
        ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s", "Failed to get a renderer.");

        uninitialize();
        return false;
    }

    // Version

    const GLubyte* version_string = ::glGetString(GL_VERSION);

    if (version_string == NULL) {
        ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s", "Failed to get a version.");

        uninitialize();
        return false;
    }

    version_.set(reinterpret_cast<const char*>(version_string));


    //
    // Extensions
    //
#if !defined(OGL_DIRECT_LINK)

    ogl_api_get_ext_symbol("glActiveTexture", glActiveTexture_);
    ogl_api_get_ext_symbol("glAttachShader", glAttachShader_);
    ogl_api_get_ext_symbol("glBindBuffer", glBindBuffer_);
    ogl_api_get_ext_symbol("glBufferData", glBufferData_);
    ogl_api_get_ext_symbol("glCompileShader", glCompileShader_);
    ogl_api_get_ext_symbol("glCreateProgram", glCreateProgram_);
    ogl_api_get_ext_symbol("glCreateShader", glCreateShader_);
    ogl_api_get_ext_symbol("glDeleteBuffers", glDeleteBuffers_);
    ogl_api_get_ext_symbol("glDeleteProgram", glDeleteProgram_);
    ogl_api_get_ext_symbol("glDeleteShader", glDeleteShader_);
    ogl_api_get_ext_symbol("glDetachShader", glDetachShader_);

    ogl_api_get_ext_symbol("glDisableVertexAttribArray",
        glDisableVertexAttribArray_);

    ogl_api_get_ext_symbol("glEnableVertexAttribArray",
        glEnableVertexAttribArray_);

    ogl_api_get_ext_symbol("glGenBuffers", glGenBuffers_);
    ogl_api_get_ext_symbol("glGetAttribLocation", glGetAttribLocation_);
    ogl_api_get_ext_symbol("glGetProgramInfoLog", glGetProgramInfoLog_);
    ogl_api_get_ext_symbol("glGetProgramiv", glGetProgramiv_);
    ogl_api_get_ext_symbol("glGetShaderInfoLog", glGetShaderInfoLog_);
    ogl_api_get_ext_symbol("glGetShaderiv", glGetShaderiv_);
    ogl_api_get_ext_symbol("glGetUniformLocation", glGetUniformLocation_);
    ogl_api_get_ext_symbol("glIsProgram", glIsProgram_);
    ogl_api_get_ext_symbol("glIsShader", glIsShader_);
    ogl_api_get_ext_symbol("glLinkProgram", glLinkProgram_);
    ogl_api_get_ext_symbol("glShaderSource", glShaderSource_);
    ogl_api_get_ext_symbol("glUniform1i", glUniform1i_);
    ogl_api_get_ext_symbol("glUniformMatrix4fv", glUniformMatrix4fv_);
    ogl_api_get_ext_symbol("glUseProgram", glUseProgram_);

    ogl_api_get_ext_symbol("glVertexAttribPointer",
        glVertexAttribPointer_);

#endif

    is_initialized_ = true;
    vendor_ = reinterpret_cast<const char*>(vendor);
    renderer_ = reinterpret_cast<const char*>(renderer);

    return true;
}

// (static)
void OglApi::uninitialize()
{
#if !defined(OGL_DIRECT_LINK)
    glBindTexture_ = NULL;
    glClear_ = NULL;
    glClearColor_ = NULL;
    glDeleteTextures_ = NULL;
    glDisable_ = NULL;
    glDrawArrays_ = NULL;
    glEnable_ = NULL;
    glGenTextures_ = NULL;
    glGetError_ = NULL;
    glGetString_ = NULL;
    glTexImage1D_ = NULL;
    glTexImage2D_ = NULL;
    glTexParameteri_ = NULL;
    glTexSubImage1D_ = NULL;
    glTexSubImage2D_ = NULL;
    glViewport_ = NULL;

    glIsShader_ = NULL;
    glActiveTexture_ = NULL;
    glAttachShader_ = NULL;
    glBindBuffer_ = NULL;
    glBufferData_ = NULL;
    glCompileShader_ = NULL;
    glCreateProgram_ = NULL;
    glCreateShader_ = NULL;
    glDeleteBuffers_ = NULL;
    glDeleteProgram_ = NULL;
    glDeleteShader_ = NULL;
    glDetachShader_ = NULL;
    glDisableVertexAttribArray_ = NULL;
    glEnableVertexAttribArray_ = NULL;
    glGenBuffers_ = NULL;
    glGetAttribLocation_ = NULL;
    glGetProgramInfoLog_ = NULL;
    glGetProgramiv_ = NULL;
    glGetShaderInfoLog_ = NULL;
    glGetShaderiv_ = NULL;
    glGetUniformLocation_ = NULL;
    glIsProgram_ = NULL;
    glLinkProgram_ = NULL;
    glShaderSource_ = NULL;
    glUniform1i_ = NULL;
    glUniformMatrix4fv_ = NULL;
    glUseProgram_ = NULL;
    glVertexAttribPointer_ = NULL;
#endif

    is_initialized_ = false;
    vendor_.clear();
    renderer_.clear();
    version_.reset();
}

// (static)
bool OglApi::is_initialized()
{
    return is_initialized_;
}

// (static)
const std::string& OglApi::get_vendor()
{
    return vendor_;
}

// (static)
const std::string& OglApi::get_renderer()
{
    return renderer_;
}

// (static)
const OglVersion& OglApi::get_version()
{
    return version_;
}


} // namespace bstone
