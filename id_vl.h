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


// ID_VL.H


#ifndef ID_VL_H
#define ID_VL_H


#include <cstdint>


#if 0
// wolf compatability
void Quit(
    const char* error,
    ...);
#endif

// ===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (80) // default screen width in bytes

#define TILEWIDTH (4)

// ===========================================================================

extern int bufferofs; // all drawing is reletive to this
extern int displayofs; // last setscreen coordinates

extern int* ylookup;

extern boolean screenfaded;

// BBi
const int k_ref_width = 320;
const int k_ref_height = 200;

const int k_vga_ref_size = 256 * 1024;
const int k_vga_plane_count = 4;
const int k_vga_plane_width = k_ref_width / 4;
const int k_vga_plane_height = k_ref_height + 8;
const int k_vga_plane_size = k_vga_plane_width * k_vga_plane_height;
const int k_vga_page_size = k_vga_plane_count * k_vga_plane_size;
const int k_vga_page_count = 3;
const int k_vga_palette_size = 3 * 256;

extern int screen_x;
extern int screen_y;

extern int screen_width;
extern int screen_height;

extern int vga_scale;
extern int vga_width;
extern int vga_height;
extern int vga_area;

extern int window_width;
extern int window_height;

extern SDL_Window* sdl_window;

// ===========================================================================

void VL_Startup();
void VL_Shutdown();

void VL_SetVGAPlane();
void VL_SetTextMode();
void VL_DePlaneVGA();
void VL_SetVGAPlaneMode();
void VL_ClearVideo(
    uint8_t color);

void VL_SetLineWidth(
    int width);
void WaitVBL(
    int16_t vbls);

void VL_WaitVBL(
    uint32_t vbls);
void VL_CrtcStart(
    int16_t crtc);

void VL_FillPalette(
    uint8_t red,
    uint8_t green,
    uint8_t blue);
void VL_SetColor(
    int color,
    int red,
    int green,
    int blue);
void VL_GetColor(
    int color,
    int* red,
    int* green,
    int* blue);

void VL_SetPalette(
    int first,
    int count,
    const uint8_t* palette,
    bool refresh_screen = true);

void VL_GetPalette(
    int first,
    int count,
    uint8_t* palette);
void VL_SetPaletteIntensity(
    int start,
    int end,
    const uint8_t* palette,
    int intensity);
void VL_FadeOut(
    int start,
    int end,
    int red,
    int green,
    int blue,
    int steps);
void VL_FadeIn(
    int start,
    int end,
    const uint8_t* palette,
    int steps);
void VL_ColorBorder(
    int16_t color);

void VL_Plot(
    int x,
    int y,
    uint8_t color);
void VL_Hlin(
    int x,
    int y,
    int width,
    uint8_t color);
void VL_Vlin(
    int x,
    int y,
    int height,
    uint8_t color);
void VL_Bar(
    int x,
    int y,
    int width,
    int height,
    uint8_t color);

void VL_DrawPicBare(
    int16_t x,
    int16_t y,
    uint8_t* pic,
    int16_t width,
    int16_t height);
void VL_MemToLatch(
    const uint8_t* source,
    int width,
    int height,
    int dest);
void VL_ScreenToScreen(
    int source,
    int dest,
    int width,
    int height);
void VL_MemToScreen(
    const uint8_t* source,
    int width,
    int height,
    int x,
    int y);
void VL_MaskMemToScreen(
    const uint8_t* source,
    int width,
    int height,
    int x,
    int y,
    uint8_t mask);
void VL_ScreenToMem(
    uint8_t* dest,
    int width,
    int height,
    int x,
    int y);

void VL_DrawTile8String(
    char* str,
    char* tile8ptr,
    int16_t printx,
    int16_t printy);
void VL_DrawLatch8String(
    char* str,
    uint16_t tile8ptr,
    int16_t printx,
    int16_t printy);
void VL_SizeTile8String(
    char* str,
    int16_t* width,
    int16_t* height);
void VL_DrawPropString(
    char* str,
    uint16_t tile8ptr,
    int16_t printx,
    int16_t printy);
void VL_SizePropString(
    char* str,
    int16_t* width,
    int16_t* height,
    char* font);

// BBi
void VL_RefreshScreen();

int vl_get_offset(
    int base_offset);

int vl_get_offset(
    int base_offset,
    int x,
    int y);

uint8_t vl_get_pixel(
    int base_offset,
    int x,
    int y);


#endif // ID_VL_H
