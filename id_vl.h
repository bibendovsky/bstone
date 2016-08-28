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


#ifndef BSTONE_ID_VL_INCLUDED
#define BSTONE_ID_VL_INCLUDED


#include <cstdint>


// ===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (320) // default screen width in bytes

#define TILEWIDTH (4)

// ===========================================================================

extern int bufferofs; // all drawing is reletive to this

extern bool screenfaded;

// BBi
const int vga_ref_width = 320;
const int vga_ref_height = 200;

const int vga_ref_size = 256 * 1024;
const int vga_plane_count = 4;
const int vga_plane_width = vga_ref_width / 4;
const int vga_plane_height = vga_ref_height + 8;
const int vga_plane_size = vga_plane_width * vga_plane_height;
const int vga_page_size = vga_plane_count * vga_plane_size;
const int vga_page_count = 3;

extern int screen_x;
extern int screen_y;

extern int screen_width;
extern int screen_height;

extern int vga_scale;
extern int vga_width;
extern int vga_height;
extern int vga_area;

const bool default_vid_stretch = false;
extern bool vid_stretch;

// Is VSync enabled?
extern bool vid_has_vsync;

// Are we drawing HUD and a level?
extern bool vid_is_hud;

// Are we drawing a level?
extern bool vid_is_3d;
// BBi

// ===========================================================================

void VL_Startup();

void VL_Shutdown();

void VL_SetVGAPlane();

void VL_SetTextMode();

void VL_DePlaneVGA();

void VL_ClearVideo(
    uint8_t color);

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
    const uint8_t* palette);

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

void vl_update_vid_stretch();

void vid_set_ui_mask(
    bool value);

void vid_set_ui_mask(
    int x,
    int y,
    int width,
    int height,
    bool value);

void vid_set_ui_mask_3d(
    bool value);

void vid_clear_3d();


#endif // BSTONE_ID_VL_INCLUDED
