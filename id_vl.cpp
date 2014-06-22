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


// ID_VL.C

#include <stdexcept>
#include <vector>

#include "id_heads.h"

#include "bstone_ogl_api.h"


#ifdef MSVC
#pragma hdrstop
#endif

#if defined(BSTONE_PANDORA) // Pandora VSync Support
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif
int fbdev = -1;
#endif // BSTONE_PANDORA


//
// SC_INDEX is expected to stay at SC_MAPMASK for proper operation
//

int	bufferofs;
int	displayofs,pelpan;

Uint16	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

Uint16	linewidth;
Uint16	ylookup[MAXSCANLINES];

boolean		screenfaded;
Uint16	bordercolor;

//boolean		fastpalette;				// if true, use outsb to set

Uint8		palette1[256][3], palette2[256][3];


// BBi
namespace {


//
// Common stuff
//

SDL_DisplayMode display_mode;

int window_width = 320;
int window_height = 200;


Uint8* vga_palette = NULL;

bool (*vid_pre_subsystem_creation)() = NULL;
bool (*vid_pre_window_creation)() = NULL;
uint32_t (*vid_get_window_flags)() = NULL;
bool (*vid_initialize_renderer)() = NULL;
void (*vid_refresh_screen)() = NULL;
void (*vid_update_screen)() = NULL;
void (*vid_uninitialize_video)() = NULL;

bool sdl_use_custom_window_position = false;
int sdl_window_x = 0;
int sdl_window_y = 0;

void initialize_video();
void uninitialize_video();


//
// OpenGL stuff
//

enum OglObjectType {
    OGL_OT_NONE,
    OGL_OT_SHADER,
    OGL_OT_PROGRAM
}; // enum OglObjectType


struct ScreenVertex {
    float x;
    float y;

    float s;
    float t;
}; // struct ScreenVertex


void ogl_draw_screen();
void ogl_refresh_screen();
void ogl_update_screen();

bool ogl_initialize_video();
void ogl_uninitialize_video();


static const GLchar* screen_fs_text =
#ifdef BSTONE_USE_GLES
    "#version 100\n"
    "precision mediump float;\n"
#else
    "#version 120\n"
#endif

    "uniform sampler2D screen_tu;\n"
    "uniform sampler2D palette_tu;\n"

    "varying vec2 tc;\n"

    "void main()\n"
    "{\n"
    "    vec2 palette_index = texture2D(screen_tu, tc).rg;\n"
    "    palette_index = clamp(palette_index, 0.0, 1.0);\n"
    "    vec4 color = vec4(texture2D(palette_tu, palette_index).rgb, 1.0);\n"
    "    gl_FragColor = (color * 255.0) / 63.0;\n"
    "}\n"
;

static const GLchar* screen_vs_text =
#ifdef BSTONE_USE_GLES
    "#version 100\n"
    "precision mediump float;\n"
#else
    "#version 120\n"
#endif

    "attribute vec4 pos_vec4;\n"
    "attribute vec2 tc0_vec2;\n"

    "uniform mat4 proj_mat4;\n"

    "varying vec2 tc;\n"

    "void main()\n"
    "{\n"
    "    tc = tc0_vec2;\n"
    "    gl_Position = proj_mat4 * pos_vec4;\n"
    "}\n"
;


GLuint screen_tex = GL_NONE;
GLuint palette_tex = GL_NONE;
GLuint screen_vbo = GL_NONE;
GLuint screen_fso = GL_NONE;
GLuint screen_vso = GL_NONE;
GLuint screen_po = GL_NONE;


ScreenVertex screen_vertices[4];

// vertex attribute: position
GLint a_pos_vec4 = -1;

// vertex attribute: texture coordinates
GLint a_tc0_vec2 = -1;

// uniform: projection matrix
GLint u_proj_mat4 = -1;

// uniform: screen texture unit
GLint u_screen_tu = -1;

// uniform: palette texture unit
GLint u_palette_tu = -1;


SDL_GLContext sdl_ogl_context = NULL;


//
// Software stuff
//

const uint32_t sdl_pixel_format = SDL_PIXELFORMAT_RGBA8888;


class SdlPalette {
public:
    SdlPalette() :
        palette_(),
        color_shifts_()
    {
    }

    ~SdlPalette()
    {
    }

    uint32_t operator[](
        int index) const
    {
        if (!is_initialized())
            throw std::runtime_error("Not initialized.");

        return palette_[index];
    }

    bool initialize(
        uint32_t pixel_format)
    {
        uninitialize();

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Initializing SDL palette...");

        typedef std::vector<uint32_t> Masks;

        int bpp = 0;
        Masks masks(4);

        SDL_bool sdl_result = SDL_FALSE;

        sdl_result = SDL_PixelFormatEnumToMasks(
            pixel_format, &bpp,
            &masks[0], &masks[1], &masks[2], &masks[3]);

        if (sdl_result == SDL_FALSE) {
            SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "SDL: %s", SDL_GetError());
            return false;
        }

        if (bpp != 32) {
            SDL_LogInfo(SDL_LOG_CATEGORY_ERROR,
                "SDL: %s", "Pixel format should have 32 bits per pixel");
            return false;
        }

        Palette palette(256);

        ColorShifts color_shifts(4);

        for (int i = 0; i < 4; ++i)
            color_shifts[i] = get_color_shift(masks[i]);

        palette_.swap(palette);
        color_shifts_.swap(color_shifts);

        return true;
    }

    void uninitialize()
    {
        Palette().swap(palette_);
        ColorShifts().swap(color_shifts_);
    }

    void update(
        const uint8_t* palette,
        int offset,
        int count)
    {
        if (!is_initialized())
            throw std::runtime_error("Not initialized.");

        if (offset < 0 || offset > 256)
            throw std::out_of_range("offset");

        if (count < 0 || count > 256)
            throw std::out_of_range("count");

        if ((offset + count) > 256)
            throw std::out_of_range("offset + count");

        int index_to = offset + count;

        for (int i = offset; i < index_to; ++i) {
            const uint8_t* palette_color = &palette[3 * i];

            uint32_t color = 0;

            for (int j = 0; j < 3; ++j) {
                uint32_t vga_color =
                    static_cast<uint32_t>(palette_color[j]);

                uint32_t pc_color = (255 * vga_color) / 63;

                color |= pc_color << color_shifts_[j];
            }

            color |= 0x000000FF << color_shifts_[3];

            palette_[i] = color;
        }
    }

    bool is_initialized() const
    {
        return !palette_.empty();
    }

private:
    typedef std::vector<uint32_t> Palette;
    typedef std::vector<int> ColorShifts;

    Palette palette_;
    ColorShifts color_shifts_;

    SdlPalette(
        const SdlPalette& that);

    SdlPalette& operator=(
        const SdlPalette& that);

    static int get_color_shift(
        uint32_t mask)
    {
        switch (mask) {
        case 0x000000FF:
            return 0;

        case 0x0000FF00:
            return 8;

        case 0x00FF0000:
            return 16;

        case 0xFF000000:
            return 24;

        default:
            return -1;
        }
    }
}; // class SdlPalette


SDL_Renderer* sdl_soft_renderer = NULL;
SDL_Texture* sdl_soft_screen_tex = NULL;
SdlPalette sdl_palette;


void soft_draw_screen();
void soft_refresh_screen();
void soft_update_screen();

bool soft_initialize_video();
void soft_uninitialize_video();


} // namespace


enum RendererType {
    RT_NONE,
    RT_AUTO_DETECT,
    RT_SOFTWARE,
    RT_OPEN_GL
}; // enum RendererType


extern const Uint8 vgapal[768];

Uint8* vga_memory = NULL;

int vga_scale = 0;
int vga_width = 0;
int vga_height = 0;
int vga_area = 0;

int screen_x = 0;
int screen_y = 0;

int screen_width = 0;
int screen_height = 0;

bool sdl_is_windowed = false;
SDL_Window* sdl_window = NULL;
RendererType g_renderer_type;
// BBi

//===========================================================================

// asm

Sint16	 VL_VideoID (void);
void VL_SetCRTC (Sint16 crtc);
void VL_SetScreen (Sint16 crtc, Sint16 pelpan);
void VL_WaitVBL (Uint32 vbls);

//===========================================================================


/*
=======================
=
= VL_Startup
=
=======================
*/

// BBi Moved from jm_free.cpp
void VL_Startup()
{
}
// BBi

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown()
{
    uninitialize_video();
}

#if !RESTART_PICTURE_PAUSE

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode()
{
    initialize_video();

    const int k_vga_size = vga_scale * vga_scale * k_vga_ref_size;

    delete [] vga_memory;
    vga_memory = new Uint8[k_vga_size];
    std::uninitialized_fill_n(vga_memory, k_vga_size, 0);

    delete [] vga_palette;
    vga_palette = new Uint8[k_vga_palette_size];
    std::uninitialized_fill_n(vga_palette, k_vga_palette_size, 0);
}

#endif


//===========================================================================

/*
====================
=
= VL_SetLineWidth
=
= Line witdh is in WORDS, 40 words is normal width for vgaplanegr
=
====================
*/

void VL_SetLineWidth(int width)
{
    int i;
    int offset;

    offset = 0;
    linewidth = static_cast<Uint16>(2 * width);

    for (i = 0; i < MAXSCANLINES; ++i) {
        ylookup[i] = static_cast<Uint16>(offset);
        offset += linewidth;
    }
}


/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette(int red, int green, int blue)
{
    int i;

    for (i = 0; i < 256; ++i) {
        vga_palette[(3 * i) + 0] = (Uint8)red;
        vga_palette[(3 * i) + 1] = (Uint8)green;
        vga_palette[(3 * i) + 2] = (Uint8)blue;
    }

    VL_SetPalette(0, 255, vga_palette);
}


//===========================================================================

/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= (some cards don't like outsb palette setting)
=
=================
*/

void VL_SetPalette(
    int first,
    int count,
    const Uint8* palette)
{
    int offset = 3 * first;
    int size = 3 * count;

    memmove(&vga_palette[offset], palette, size);

    switch (g_renderer_type) {
    case RT_OPEN_GL:
        glActiveTexture(GL_TEXTURE1);

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            256,
            1,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            vga_palette);

        ogl_refresh_screen();
        break;

    case RT_SOFTWARE:
        sdl_palette.update(palette, first, count);
        soft_refresh_screen();
        break;

    default:
        throw std::runtime_error("g_renderer_type");
    }
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void VL_GetPalette(
    int first,
    int count,
    Uint8* palette)
{
    int offset = 3 * first;
    int size = 3 * count;

    memmove(palette, &vga_palette[offset], size);
}

//===========================================================================


/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (
    int start,
    int end,
    int red,
    int green,
    int blue,
    int steps)
{
    int i;
    int j;
    int orig;
    int delta;
    Uint8* origptr;
    Uint8* newptr;

    VL_GetPalette(0, 256, &palette1[0][0]);
    memcpy(palette2, palette1, 768);

    //
    // fade through intermediate frames
    //
    for (i = 0; i < steps; ++i) {
        origptr = &palette1[start][0];
        newptr = &palette2[start][0];

        for (j = start; j <= end; ++j) {
            orig = *origptr++;
            delta = red-orig;
            *newptr++ = static_cast<Uint8>(orig + delta * i / steps);
            orig = *origptr++;
            delta = green-orig;
            *newptr++ = static_cast<Uint8>(orig + delta * i / steps);
            orig = *origptr++;
            delta = blue-orig;
            *newptr++ = static_cast<Uint8>(orig + delta * i / steps);
        }

        VL_SetPalette(0, 256, &palette2[0][0]);
    }

    //
    // final color
    //
    VL_FillPalette(red, green, blue);

    screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn(
    int start,
    int end,
    const Uint8* palette,
    int steps)
{
    int i;
    int j;
    int delta;

    VL_GetPalette(0, 256, &palette1[0][0]);
    memcpy(&palette2[0][0], &palette1[0][0], sizeof(palette1));

    start *= 3;
    end = (end * 3) + 2;

    //
    // fade through intermediate frames
    //
    for (i = 0; i < steps; ++i) {
        for (j = start; j <= end; ++j) {
            delta = palette[j] - palette1[0][j];
            palette2[0][j] = static_cast<Uint8>(palette1[0][j] + ((delta * i) / steps));
        }

        VL_SetPalette(0, 256, &palette2[0][0]);
    }

    //
    // final color
    //
    VL_SetPalette(0, 256, palette);

    screenfaded = false;
}

//------------------------------------------------------------------------
// VL_SetPaletteIntensity()
//------------------------------------------------------------------------
void VL_SetPaletteIntensity(
    int start,
    int end,
    const Uint8* palette,
    int intensity)
{
    int loop;
    char red;
    char green;
    char blue;
    Uint8* cmap = &palette1[0][0] + (start * 3);

    intensity = 63 - intensity;

    for (loop = start; loop <= end; ++loop) {
        red = static_cast<char>(*palette++ - intensity);

        if (red < 0)
            red = 0;

        *cmap++ = red;

        green = static_cast<char>(*palette++ - intensity);

        if (green < 0)
            green = 0;

        *cmap++ = green;

        blue = static_cast<char>(*palette++ - intensity);

        if (blue < 0)
            blue = 0;

        *cmap++ = blue;
    }

    VL_SetPalette(start, end - start + 1, &palette1[0][0]);
}


/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (Sint16 color)
{
	bordercolor = color;
}



/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

Uint8	pixmasks[4] = {1,2,4,8};
Uint8	leftmasks[4] = {15,14,12,8};
Uint8	rightmasks[4] = {1,3,7,15};


/*
=================
=
= VL_Plot
=
=================
*/
void VL_Plot(int x, int y, int color)
{
    int offset = (4 * bufferofs) + (y * vga_width) + x;
    vga_memory[offset] = (Uint8)color;
}


/*
=================
=
= VL_Hlin
=
=================
*/
void VL_Hlin(int x, int y, int width, int color)
{
    VL_Bar(x, y, width, 1, color);
}


/*
=================
=
= VL_Vlin
=
=================
*/
void VL_Vlin(int x, int y, int height, int color)
{
    VL_Bar(x, y, 1, height, color);
}


/*
=================
=
= VL_Bar
=
=================
*/
void VL_Bar(int x, int y, int width, int height, int color)
{
    int i;
    int offset = (4 * bufferofs) + (y * vga_width) + x;

    for (i = 0; i < height; ++i) {
        memset(&vga_memory[offset], color, width);
        offset += vga_width;
    }
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch(
    const Uint8* source,
    int width,
    int height,
    int dest)
{
    int i;
    int j;
    int count = ((width + 3) / 4) * height;

    for (i = 0; i < 4; ++i) {
        int offset = (4 * dest) + i;

        for (j = 0; j < count; ++j) {
            vga_memory[offset] = *source++;
            offset += 4;
        }
    }
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen(const Uint8* source, int width, int height, int x, int y)
{
    int i;
    int j;
    int k;
    int q_width = width / 4;
    int base_offset = (4 * bufferofs) + (y * vga_width) + x;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < height; ++j) {
            int offset = base_offset + i + (j * vga_width);

            for (k = 0; k < q_width; ++k) {
                vga_memory[offset] = *source++;
                offset += 4;
            }
        }
    }
}

//==========================================================================

//------------------------------------------------------------------------
// VL_MaskMemToScreen()
//------------------------------------------------------------------------

void VL_MaskMemToScreen(
    const Uint8* source,
    int width,
    int height,
    int x,
    int y,
    int mask)
{
    int plane;
    int row;
    int column;
    int q_width = width / 4;
    int offset;
    int base_offset;

    base_offset = (4 * bufferofs) + (vga_width * y) + x;

    for (plane = 0; plane < 4; ++plane) {
        for (row = 0; row < height; ++row) {
            offset = base_offset + (row * vga_width) + plane;

            for (column = 0; column < q_width; ++column) {
                Uint8 pixel = *source++;

                if (pixel != mask)
                    vga_memory[offset] = pixel;

                offset += 4;
            }
        }
    }
}



//------------------------------------------------------------------------
// VL_ScreenToMem()
//------------------------------------------------------------------------

void VL_ScreenToMem(
    Uint8* dest,
    int width,
    int height,
    int x,
    int y)
{
    int plane;
    int row;
    int column;
    int q_width = width / 4;
    int offset;
    int base_offset;

    base_offset = (4 * bufferofs) + (vga_width * y) + x;

    for (plane = 0; plane < 4; ++plane) {
        for (row = 0; row < height; ++row) {
            offset = base_offset + (row * vga_width) + plane;

            for (column = 0; column < q_width; ++column) {
                *dest = vga_memory[offset];

                offset += 4;
                ++dest;
            }
        }
    }
}


//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen(int source, int width, int height, int x, int y)
{
    int i;
    int count = 4 * width;
    int src_offset = (4 * source);
    int dst_offset = (4 * bufferofs) + (y * vga_width) + x;

    for (i = 0; i < height; ++i) {
        memmove(
            &vga_memory[dst_offset],
            &vga_memory[src_offset],
            count);

        src_offset += count;
        dst_offset += vga_width;
    }
}


//===========================================================================

void VL_ScreenToScreen(
    int source,
    int dest,
    int width,
    int height)
{
    source *= 4;
    dest *= 4;
    width *= 4;

    const Uint8* src = &vga_memory[source];
    Uint8* dst = &vga_memory[dest];

    for (int y = 0; y < height; ++y) {
        memmove(dst, src, width);
        src += vga_width;
        dst += vga_width;
    }
}


// BBi
namespace {


// Builds an orthographic projection matrix with upside-downed origin.
void ogl_ortho(
    int width,
    int height,
    float matrix[16])
{
    assert(width > 0);
    assert(height > 0);
    assert(matrix != NULL);

    matrix[0] = 2.0F / width;
    matrix[1] = 0.0F;
    matrix[2] = 0.0F;
    matrix[3] = 0.0F;

    matrix[4] = 0.0F;
    matrix[5] = -2.0F / height;
    matrix[6] = 0.0F;
    matrix[7] = 0.0F;

    matrix[8] = 0.0F;
    matrix[9] = 0.0F;
    matrix[10] = -1.0F;
    matrix[11] = 0.0F;

    matrix[12] = -1.0F;
    matrix[13] = 1.0F;
    matrix[14] = 0.0F;
    matrix[15] = 1.0F;
}

// Clears error flags and returns true if any error flag was set,
// otherwise it return false.
boolean ogl_check_for_and_clear_errors()
{
    boolean result = false;

    while (glGetError() != GL_NONE)
        result = true;

    return result;
}

// Just draws a screen texture.
void ogl_draw_screen()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#if defined(BSTONE_PANDORA) // Pandora VSync
    if (fbdev >= 0) {
        int arg = 0;
        ioctl( fbdev, FBIO_WAITFORVSYNC, &arg );
    }
#endif

    SDL_GL_SwapWindow(sdl_window);
}

// Updates screen texture with display data and
// draws it.
void ogl_refresh_screen()
{
    GLenum format =
        bstone::OglApi::has_ext_texture_rg() ?
            bstone::OglApi::get_gl_red() :
            GL_LUMINANCE;

    glActiveTexture(GL_TEXTURE0);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        vga_width,
        vga_height,
        format,
        GL_UNSIGNED_BYTE,
        &vga_memory[4 * displayofs]);

    ogl_draw_screen();
}

// Copies buffer page to a display one,
// updates screen texture with display page data
// and draws it.
void ogl_update_screen()
{
    if (displayofs != bufferofs) {
        int src_offset = vl_get_offset(bufferofs);
        int dst_offset = vl_get_offset(displayofs);

        std::uninitialized_copy(
            &vga_memory[src_offset],
            &vga_memory[src_offset + vga_area],
            &vga_memory[dst_offset]);
    }

    ogl_refresh_screen();
}

// Returns an information log of a shader or a program.
std::string ogl_get_info_log(
    GLuint object)
{
    if (object == GL_NONE)
        return std::string();

    OglObjectType object_type = OGL_OT_NONE;
    GLint info_log_size = 0; // with a null terminator

    if (glIsShader(object)) {
        object_type = OGL_OT_SHADER;

        glGetShaderiv(
            object,
            GL_INFO_LOG_LENGTH,
            &info_log_size);
    } else if (glIsProgram(object)) {
        object_type = OGL_OT_PROGRAM;

        glGetProgramiv(
            object,
            GL_INFO_LOG_LENGTH,
            &info_log_size);
    } else
        return std::string();

    if (info_log_size <= 1)
        return std::string();

    GLsizei info_log_length; // without a null terminator
    std::auto_ptr<GLchar> info_log(new GLchar[info_log_size]);

    switch (object_type) {
    case OGL_OT_SHADER:
        glGetShaderInfoLog(
            object,
            info_log_size,
            &info_log_length,
            info_log.get());
        break;

    case OGL_OT_PROGRAM:
        glGetProgramInfoLog(
            object,
            info_log_size,
            &info_log_length,
            info_log.get());
        break;

    default:
        return std::string();
    }

    if (info_log_length > 0)
        return info_log.get();

    return std::string();
}

bool ogl_load_shader(
    GLuint shader_object,
    const GLchar* shader_text)
{
    GLint compile_status = GL_FALSE;
    const GLchar* lines[1] = { shader_text };
    GLint lengths[1] = {
        static_cast<GLint>(std::string::traits_type::length(shader_text))
    };

    glShaderSource(shader_object, 1, lines, lengths);
    glCompileShader(shader_object);
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compile_status);

    std::string shader_log = ogl_get_info_log(shader_object);

    if (compile_status != GL_FALSE) {
        if (!shader_log.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s", shader_log.c_str());
        }

        return true;
    }

    if (shader_log.empty())
        shader_log = "Generic compile error.";

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s", shader_log.c_str());

    return false;
}

bool ogl_initialize_textures()
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Initializing textures...");

    bool is_succeed = true;

    if (is_succeed) {
        screen_tex = GL_NONE;
        glGenTextures(1, &screen_tex);

        if (screen_tex == GL_NONE) {
            is_succeed = false;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", "Screen texture failed.");
        }
    }

    GLenum format = GL_NONE;
    GLenum internal_format = GL_NONE;

    if (is_succeed) {
        if (bstone::OglApi::has_ext_texture_rg()) {
            format = bstone::OglApi::get_gl_red();
            internal_format = bstone::OglApi::get_gl_r8();
        } else {
            format = GL_LUMINANCE;
            internal_format = GL_LUMINANCE;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internal_format,
            vga_width,
            vga_height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            NULL);
    }

    if (is_succeed) {
        palette_tex = GL_NONE;
        glGenTextures(1, &palette_tex);

        if (palette_tex == GL_NONE) {
            is_succeed = false;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", "Palette texture failed.");
        }
    }

    if (is_succeed) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, palette_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            256,
            1,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            NULL);
    }

    if (is_succeed)
        return true;

    if (screen_tex != GL_NONE) {
        glDeleteTextures(1, &screen_tex);
        screen_tex = GL_NONE;
    }

    if (palette_tex != GL_NONE) {
        glDeleteTextures(1, &palette_tex);
        palette_tex = GL_NONE;
    }

    return false;
}

bool ogl_initialize_vertex_buffers()
{
    ScreenVertex* vertex;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Setting up a screen buffer object...");

    screen_vbo = GL_NONE;
    glGenBuffers(1, &screen_vbo);

    if (screen_vbo == GL_NONE) {
        SDL_LogInfo(SDL_LOG_CATEGORY_ERROR,
            "%s", "Failed to create an object.");
        return false;
    }

    vertex = &screen_vertices[0];
    vertex->x = 0.0F;
    vertex->y = 0.0F;
    vertex->s = 0.0F;
    vertex->t = 0.0F;

    vertex = &screen_vertices[1];
    vertex->x = 0.0F;
    vertex->y = static_cast<float>(vga_height);
    vertex->s = 0.0F;
    vertex->t = 1.0F;

    vertex = &screen_vertices[2];
    vertex->x = static_cast<float>(vga_width);
    vertex->y = 0.0F;
    vertex->s = 1.0F;
    vertex->t = 0.0F;

    vertex = &screen_vertices[3];
    vertex->x = static_cast<float>(vga_width);
    vertex->y = static_cast<float>(vga_height);
    vertex->s = 1.0F;
    vertex->t = 1.0F;

    glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(screen_vertices),
        screen_vertices,
        GL_STATIC_DRAW);

    return true;
}

bool ogl_initialize_shaders()
{
    bool is_succeed = true;

    if (is_succeed) {
        screen_fso = glCreateShader(GL_FRAGMENT_SHADER);

        if (screen_fso == GL_NONE) {
            is_succeed = false;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", "Failed to create an object.");
        }
    }

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "OGL: %s", "Loading a screen fragment shader...");

        is_succeed = ogl_load_shader(screen_fso, screen_fs_text);
    }

    if (is_succeed) {
        screen_vso = glCreateShader(GL_VERTEX_SHADER);

        if (screen_vso == GL_NONE) {
            is_succeed = false;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", "Failed to create an object.");
        }
    }

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "OGL: %s", "Loading a screen vertex shader...");

        is_succeed = ogl_load_shader(screen_vso, screen_vs_text);
    }

    return is_succeed;
}

bool ogl_initialize_programs()
{
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Setting up a screen program object...");

    bool is_succeed = true;

    if (is_succeed) {
        screen_po = glCreateProgram();

        if (screen_po == GL_NONE) {
            is_succeed = false;

            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", "Failed to create an object.");
        }
    }

    if (is_succeed) {
        GLint link_status = GL_FALSE;

        glAttachShader(screen_po, screen_fso);
        glAttachShader(screen_po, screen_vso);
        glLinkProgram(screen_po);
        glGetProgramiv(screen_po, GL_LINK_STATUS, &link_status);

        std::string program_log = ogl_get_info_log(screen_po);

        if (link_status != GL_FALSE) {
            if (!program_log.empty()) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "%s", program_log.c_str());
            }
        } else {
            is_succeed = false;

            if (program_log.empty())
                program_log = "Generic link error.";

            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", program_log.c_str());
        }
    }

    if (is_succeed) {
        glUseProgram(screen_po);

        a_pos_vec4 = glGetAttribLocation(screen_po, "pos_vec4");
        a_tc0_vec2 = glGetAttribLocation(screen_po, "tc0_vec2");

        float proj_mat4[16];
        u_proj_mat4 = glGetUniformLocation(screen_po, "proj_mat4");
        ogl_ortho(vga_width, vga_height, proj_mat4);
        glUniformMatrix4fv(u_proj_mat4, 1, GL_FALSE, proj_mat4);

        u_screen_tu = glGetUniformLocation(screen_po, "screen_tu");
        glUniform1i(u_screen_tu, 0);

        u_palette_tu = glGetUniformLocation(screen_po, "palette_tu");
        glUniform1i(u_palette_tu, 1);

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "OGL: %s", "Screen program object complete...");
    }

    return is_succeed;
}

void ogl_uninitialize_video()
{
    if (sdl_ogl_context != NULL) {
        if (screen_po != GL_NONE) {
            glDisableVertexAttribArray(a_pos_vec4);
            glDisableVertexAttribArray(a_tc0_vec2);

            glUseProgram(GL_NONE);
            glDetachShader(screen_po, screen_fso);
            glDetachShader(screen_po, screen_vso);
            glDeleteProgram(screen_po);
            screen_po = GL_NONE;
        }

        if (screen_fso != GL_NONE) {
            glDeleteShader(screen_fso);
            screen_fso = GL_NONE;
        }

        if (screen_vso != GL_NONE) {
            glDeleteShader(screen_vso);
            screen_vso = GL_NONE;
        }

        if (screen_vbo != GL_NONE) {
            glDeleteBuffers(1, &screen_vbo);
            screen_vbo = GL_NONE;
        }

        if (screen_tex != GL_NONE) {
            glDeleteTextures(1, &screen_tex);
            screen_tex = GL_NONE;
        }

        if (palette_tex != GL_NONE) {
            glDeleteTextures(1, &palette_tex);
            palette_tex = GL_NONE;
        }

        SDL_GL_MakeCurrent(sdl_window, NULL);
        SDL_GL_DeleteContext(sdl_ogl_context);
        sdl_ogl_context = NULL;

        bstone::OglApi::uninitialize();
    }

    a_pos_vec4 = -1;
    a_tc0_vec2 = -1;
    u_screen_tu = -1;
    u_palette_tu = -1;

#if defined(BSTONE_PANDORA) // Pandora VSync
    close(fbdev);
    fbdev = -1;
#endif
}

bool ogl_pre_subsystem_creation()
{
#if defined(BSTONE_PANDORA) // Pandora VSync
    fbdev = open("/dev/fb0", O_RDONLY /* O_RDWR */);
    if (fbdev < 0) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Couldn't open /dev/fb0 for Pandora Vsync...");
    }
#endif

    return true;
}

bool ogl_pre_window_creation()
{
    int errors = 0;

#if defined(BSTONE_USE_GLES)
    errors += SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    errors += SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    errors += SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    errors += SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    errors += SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16);
    errors += SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    errors += SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    errors += SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    errors += SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_ES);
    errors &= SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#else
    errors += SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    errors += SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    errors += SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    errors += SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    errors += SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    errors += SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    errors += SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    errors += SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif

    return errors == 0;
}

uint32_t ogl_get_window_flags()
{
    return SDL_WINDOW_OPENGL;
}

bool ogl_initialize_renderer()
{
    bool is_succeed = true;

    if (is_succeed) {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Creating an OpenGL context...");

        sdl_ogl_context = SDL_GL_CreateContext(sdl_window);

        if (sdl_ogl_context == NULL) {
            is_succeed = false;
            SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
        }
    }

    if (is_succeed)
        is_succeed = bstone::OglApi::initialize();

    if (is_succeed) {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s: %s",
            "Vendor",
            bstone::OglApi::get_vendor().c_str());

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s: %s",
            "Renderer",
            bstone::OglApi::get_renderer().c_str());

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s: %s",
            "Original version",
            bstone::OglApi::get_version().get_original().c_str());

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "OGLAPI: %s: %s",
            "Parsed version",
            bstone::OglApi::get_version().to_string().c_str());
    }

    if (is_succeed)
        is_succeed = ogl_initialize_textures();

    if (is_succeed)
        is_succeed = ogl_initialize_vertex_buffers();

    if (is_succeed)
        is_succeed = ogl_initialize_shaders();

    if (is_succeed)
        is_succeed = ogl_initialize_programs();

    if (is_succeed) {
        glViewport(screen_x, screen_y, screen_width, screen_height);

        glEnable(GL_TEXTURE_2D);

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        if (a_pos_vec4 != -1) {
            glVertexAttribPointer(
                a_pos_vec4,
                2,
                GL_FLOAT,
                GL_FALSE,
                sizeof(ScreenVertex),
                reinterpret_cast<const GLvoid*>(offsetof(ScreenVertex,x)));

            glEnableVertexAttribArray(a_pos_vec4);
        }

        if (a_tc0_vec2 != -1) {
            glVertexAttribPointer(
                a_tc0_vec2,
                2,
                GL_FLOAT,
                GL_FALSE,
                sizeof(ScreenVertex),
                reinterpret_cast<const GLvoid*>(offsetof(ScreenVertex,s)));

            glEnableVertexAttribArray(a_tc0_vec2);
        }

        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    }

    return is_succeed;
}

// Just draws a screen texture.
void soft_draw_screen()
{
    SDL_Rect src_rect;
    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.w = vga_width;
    src_rect.h = vga_height;

    SDL_Rect dst_rect;
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = screen_width;
    dst_rect.h = screen_height;

    int sdl_result = 0;

    sdl_result = SDL_RenderClear(sdl_soft_renderer);

    sdl_result = SDL_RenderCopy(
        sdl_soft_renderer, sdl_soft_screen_tex, &src_rect, &dst_rect);

    SDL_RenderPresent(sdl_soft_renderer);
}

// Updates screen texture with display data and
// draws it.
void soft_refresh_screen()
{
    SDL_Rect screen_rect;
    screen_rect.x = 0;
    screen_rect.y = 0;
    screen_rect.w = vga_width;
    screen_rect.h = vga_height;

    int sdl_result = 0;
    int pitch = 0;
    void* data = NULL;

    sdl_result = SDL_LockTexture(
        sdl_soft_screen_tex, &screen_rect, &data, &pitch);

    uint8_t* octets = static_cast<uint8_t*>(data);

    for (int y = 0; y < vga_height; ++y) {
        int vga_offset = vl_get_offset(displayofs, 0, y);

        uint32_t* row = reinterpret_cast<uint32_t*>(octets);

        for (int x = 0; x < vga_width; ++x)
            row[x] = sdl_palette[vga_memory[vga_offset + x]];

        octets += pitch;
    }

    SDL_UnlockTexture(sdl_soft_screen_tex);

    soft_draw_screen();
}

// Copies buffer page to a display one,
// updates screen texture with display page data
// and draws it.
void soft_update_screen()
{
    if (displayofs != bufferofs) {
        int src_offset = vl_get_offset(bufferofs);
        int dst_offset = vl_get_offset(displayofs);

        std::uninitialized_copy(
            &vga_memory[src_offset],
            &vga_memory[src_offset + vga_area],
            &vga_memory[dst_offset]);
    }

    soft_refresh_screen();
}

bool soft_initialize_textures()
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Creating a screen texture...");

    sdl_soft_screen_tex = SDL_CreateTexture(
        sdl_soft_renderer,
        sdl_pixel_format,
        SDL_TEXTUREACCESS_STREAMING,
        vga_width,
        vga_height);

    if (sdl_soft_screen_tex != NULL)
        return true;

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s", SDL_GetError());

    return false;
}

void soft_uninitialize_textures()
{
    if (sdl_soft_screen_tex != NULL) {
        SDL_DestroyTexture(sdl_soft_screen_tex);
        sdl_soft_screen_tex = NULL;
    }
}

void soft_uninitialize_video()
{
    soft_uninitialize_textures();

    if (sdl_soft_renderer != NULL) {
        SDL_DestroyRenderer(sdl_soft_renderer);
        sdl_soft_renderer = NULL;
    }
}

bool soft_pre_subsystem_creation()
{
    return true;
}

bool soft_pre_window_creation()
{
    return sdl_palette.initialize(sdl_pixel_format);
}

uint32_t soft_get_window_flags()
{
    return 0;
}

bool soft_initialize_renderer()
{
    bool is_succeed = true;

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Creating a software renderer...");

        sdl_soft_renderer = SDL_CreateRenderer(
            sdl_window, -1, SDL_RENDERER_SOFTWARE);

        if (sdl_soft_renderer == NULL) {
            is_succeed = false;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", SDL_GetError());
        }
    }

    int sdl_result = 0;

    if (is_succeed)
        is_succeed = soft_initialize_textures();

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Initializing a view port...");

        SDL_Rect view_port;
        view_port.x = screen_x;
        view_port.y = screen_y;
        view_port.w = screen_width;
        view_port.h = screen_height;

        sdl_result = SDL_RenderSetViewport(
            sdl_soft_renderer, &view_port);

        if (sdl_result != 0) {
            is_succeed = false;

            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", SDL_GetError());
        }
    }

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Initializing default draw color...");

        sdl_result = SDL_SetRenderDrawColor(
            sdl_soft_renderer, 0, 0, 0, 255);

        if (sdl_result != 0) {
            is_succeed = false;

            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s", SDL_GetError());
        }
    }

    return is_succeed;
}

bool soft_initialize_video()
{
    return false;
}

bool x_initialize_video()
{
    bool is_succeed = true;

    if (is_succeed)
        is_succeed = vid_pre_window_creation();

    if (is_succeed) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Creating a window...");

        if (!sdl_use_custom_window_position) {
            sdl_window_x = (display_mode.w - window_width) / 2;
            sdl_window_y = (display_mode.h - window_height) / 2;
        }

        if (sdl_window_x < 0)
            sdl_window_x = 0;

        if (sdl_window_y < 0)
            sdl_window_y = 0;

        uint32_t flags = 0;

        flags |= SDL_WINDOW_HIDDEN;
        flags |= vid_get_window_flags();

        if (!sdl_is_windowed) {
            if (window_width == display_mode.w &&
                window_height == display_mode.h)
            {
                flags |= SDL_WINDOW_BORDERLESS;
            } else
                flags |= SDL_WINDOW_FULLSCREEN;
        }

        sdl_window = SDL_CreateWindow(
            "Blake Stone: Planet Strike",
            sdl_window_x,
            sdl_window_y,
            window_width,
            window_height,
            flags);

        if (sdl_window == NULL) {
            is_succeed = false;
            SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
        }
    }

    if (is_succeed)
        is_succeed = vid_initialize_renderer();

    if (is_succeed)
        return true;

    uninitialize_video();

    return false;
}

void initialize_video()
{
    //
    // Option "windowed"
    //

    sdl_is_windowed = (g_args.find_option("windowed") >= 0);

    sdl_use_custom_window_position = false;

    //
    // Option "winx"
    //

    const std::string& winx_str = g_args.get_option_value("winx");

    if (bstone::StringHelper::lexical_cast(winx_str, sdl_window_x))
        sdl_use_custom_window_position = true;

    //
    // Option "winy"
    //

    const std::string& winy_str = g_args.get_option_value("winy");

    if (bstone::StringHelper::lexical_cast(winy_str, sdl_window_y))
        sdl_use_custom_window_position = true;

    //
    // Option "res"
    //

    std::string width_str;
    std::string height_str;

    g_args.get_option_values("res", width_str, height_str);

    bstone::StringHelper::lexical_cast(width_str, window_width);
    bstone::StringHelper::lexical_cast(height_str, window_height);

    if (window_width < k_ref_width)
        window_width = k_ref_width;

    if (window_height < k_ref_height)
        window_height = k_ref_height;

    vga_scale = 0;
    vga_width = 0;
    vga_height = 0;

    while (vga_width < window_width || vga_height < window_height) {
        ++vga_scale;
        vga_width += k_ref_width;
        vga_height += k_ref_height;
    }

    vga_area = vga_width * vga_height;


    //
    // Renderer initialization
    //

    g_renderer_type = RT_NONE;

    std::string ren_string = g_args.get_option_value("ren");

    if (!ren_string.empty()) {
        if (ren_string == "soft")
            g_renderer_type = RT_SOFTWARE;
        else if (ren_string == "ogl")
            g_renderer_type = RT_OPEN_GL;
        else {
            SDL_LogInfo(
                SDL_LOG_CATEGORY_APPLICATION,
                "CL: %s: %s", "Unknown renderer type", ren_string.c_str());
        }
    }

    if (g_renderer_type == RT_NONE)
        g_renderer_type = RT_AUTO_DETECT;


    bool initialize_result = false;

    switch (g_renderer_type) {
    case RT_AUTO_DETECT:
        g_renderer_type = RT_OPEN_GL;

    case RT_OPEN_GL:
        vid_pre_subsystem_creation = ogl_pre_subsystem_creation;
        vid_pre_window_creation = ogl_pre_window_creation;
        vid_get_window_flags = ogl_get_window_flags;
        vid_initialize_renderer = ogl_initialize_renderer;
        vid_refresh_screen = ogl_refresh_screen;
        vid_update_screen = ogl_update_screen;
        vid_uninitialize_video = ogl_uninitialize_video;
        break;

    case RT_SOFTWARE:
        vid_pre_subsystem_creation = soft_pre_subsystem_creation;
        vid_pre_window_creation = soft_pre_window_creation;
        vid_get_window_flags = soft_get_window_flags;
        vid_initialize_renderer = soft_initialize_renderer;
        vid_refresh_screen = soft_refresh_screen;
        vid_update_screen = soft_update_screen;
        vid_uninitialize_video = soft_uninitialize_video;
        break;

    default:
        throw std::runtime_error("g_renderer_type");
    }

    int sdl_result = 0;

    if (!vid_pre_subsystem_creation())
        Quit("%s", "Failed to pre-initialize video subsystem.");

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Setting up a video subsystem...");

    sdl_result = SDL_InitSubSystem(SDL_INIT_VIDEO);

    if (sdl_result != 0)
        Quit("%s", SDL_GetError());

    sdl_result = SDL_GetDesktopDisplayMode(0, &display_mode);

    if (sdl_result != 0)
        Quit("SDL: %s", "Failed to get a display mode.");

    if (!sdl_is_windowed) {
        window_width = display_mode.w;
        window_height = display_mode.h;
    }

    double h_scale = static_cast<double>(window_width) /
        vga_width;

    double v_scale = static_cast<double>(window_height) /
        vga_height;

    double scale;

    if (h_scale <= v_scale)
        scale = h_scale;
    else
        scale = v_scale;

    screen_width = static_cast<int>(
        (vga_width * scale) + 0.5);

    screen_height = static_cast<int>(
        (vga_height * scale) + 0.5);

    screen_x = (window_width - screen_width) / 2;
    screen_y = (window_height - screen_height) / 2;

    VL_SetLineWidth(40);

    initialize_result = x_initialize_video();

    if (!initialize_result && g_renderer_type == RT_AUTO_DETECT) {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL: %s", "Falling back to software renderer...");

        g_renderer_type = RT_SOFTWARE;
        vid_pre_subsystem_creation = soft_pre_subsystem_creation;
        vid_pre_window_creation = soft_pre_window_creation;
        vid_get_window_flags = soft_get_window_flags;
        vid_initialize_renderer = soft_initialize_renderer;
        vid_refresh_screen = soft_refresh_screen;
        vid_update_screen = soft_update_screen;
        vid_uninitialize_video = soft_uninitialize_video;

        initialize_result = x_initialize_video();
    }

    if (!initialize_result)
        Quit("SDL: %s", "Failed to initialize a renderer.");

    SDL_ShowWindow(sdl_window);
}

void uninitialize_video()
{
    if (vid_uninitialize_video != NULL)
        vid_uninitialize_video();

    if (sdl_window != NULL) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


} // namespace


void JM_VGALinearFill(int start, int length, char fill)
{
    memset(&vga_memory[4 * start], fill, 4 * length);
}

void VL_RefreshScreen()
{
    vid_refresh_screen();
}

void VH_UpdateScreen()
{
    vid_update_screen();
}

int vl_get_offset(
    int base_offset)
{
    return vga_scale * vga_scale * 4 * base_offset;
}

int vl_get_offset(
    int base_offset,
    int x,
    int y)
{
    return vga_scale * (vga_scale * (4 * base_offset) + (y * vga_width) + x);
}
// BBi
