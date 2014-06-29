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


#include "SDL.h"


// wolf compatability

#define MS_Quit	Quit

void Quit (const char *error,...);

//===========================================================================

#define NOFADE_CODE

#define SCREENWIDTH (80) // default screen width in bytes

#define TILEWIDTH (4)

//===========================================================================

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

//===========================================================================

void VL_Startup (void);
void VL_Shutdown (void);

void VL_SetVGAPlane (void);
void VL_SetTextMode (void);
void VL_DePlaneVGA (void);
void VL_SetVGAPlaneMode (void);
void VL_ClearVideo (Uint8 color);

void VL_SetLineWidth(int width);
void VL_SetSplitScreen (Sint16 linenum);
void WaitVBL (Sint16 vbls);

void VL_WaitVBL(Uint32 vbls);
void VL_CrtcStart (Sint16 crtc);

void VL_FillPalette(Uint8 red, Uint8 green, Uint8 blue);
void VL_SetColor(int color, int red, int green, int blue);
void VL_GetColor(int color, int* red, int* green, int* blue);
void VL_SetPalette(int first, int count, const Uint8* palette);
void VL_GetPalette(int first, int count, Uint8* palette);
void VL_SetPaletteIntensity(int start, int end, const Uint8* palette, int intensity);
void VL_FadeOut(int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn(int start, int end, const Uint8* palette, int steps);
void VL_ColorBorder(Sint16 color);

void VL_Plot(int x, int y, Uint8 color);
void VL_Hlin(int x, int y, int width, Uint8 color);
void VL_Vlin (int x, int y, int height, int color);
void VL_Bar(int x, int y, int width, int height, int color);

void VL_MungePic (Uint8* source, Uint16 width, Uint16 height);
void VL_DrawPicBare (Sint16 x, Sint16 y, Uint8* pic, Sint16 width, Sint16 height);
void VL_MemToLatch(const Uint8* source, int width, int height, int dest);
void VL_ScreenToScreen(int source, int dest, int width, int height);
void VL_MemToScreen(const Uint8* source, int width, int height, int x, int y);
void VL_MaskMemToScreen (const Uint8* source, int width, int height, int x, int y, int mask);
void VL_ScreenToMem(Uint8* dest, int width, int height, int x, int y);

void VL_DrawTile8String (char *str, char* tile8ptr, Sint16 printx, Sint16 printy);
void VL_DrawLatch8String (char *str, Uint16 tile8ptr, Sint16 printx, Sint16 printy);
void VL_SizeTile8String (char *str, Sint16 *width, Sint16 *height);
void VL_DrawPropString (char *str, Uint16 tile8ptr, Sint16 printx, Sint16 printy);
void VL_SizePropString (char *str, Sint16 *width, Sint16 *height, char* font);

// BBi
void VL_RefreshScreen();

int vl_get_offset(
    int base_offset);

int vl_get_offset(
    int base_offset,
    int x,
    int y);

Uint8 vl_get_pixel(
    int base_offset,
    int x,
    int y);


#endif // ID_VL_H
