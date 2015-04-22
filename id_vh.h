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


#define WHITE (15) // graphics mode independant colors
#define BLACK (0)


struct spritetabletype {
    int16_t width;
    int16_t height;
    int16_t orgx;
    int16_t orgy;
    int16_t xl;
    int16_t yl;
    int16_t xh;
    int16_t yh;
    int16_t shifts;
}; // struct spritetabletype

struct pictabletype {
    int16_t width;
    int16_t height;
}; // struct pictabletype

struct fontstruct {
    int16_t height;
    int16_t location[256];
    char width[256];
}; // struct fontstruct


extern pictabletype* pictable;
extern pictabletype* picmtable;
extern spritetabletype* spritetable;

extern uint8_t fontcolor;
extern int fontnumber;
extern int px;
extern int py;
extern bool allcaps;


//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8(
    int x,
    int y,
    int tile);
void VWB_DrawPic(
    int x,
    int y,
    int chunknum);
void VWB_DrawMPic(
    int x,
    int y,
    int chunknum);
void VWB_Bar(
    int x,
    int y,
    int width,
    int height,
    uint8_t color);

void VWB_DrawPropString(
    const char* string);
void VW_DrawPropString(
    const char* string);
void VWB_Plot(
    int x,
    int y,
    uint8_t color);
void VWB_Hlin(
    int x1,
    int x2,
    int y,
    uint8_t color);
void VWB_Vlin(
    int y1,
    int y2,
    int x,
    uint8_t color);


//
// wolfenstein EGA compatability stuff
//
extern const uint8_t vgapal[768];


#define VW_Startup VL_Startup
#define VW_Shutdown VL_Shutdown
#define VW_Bar VL_Bar
#define VW_Plot VL_Plot
#define VW_Hlin(x, z, y, c) VL_Hlin((x), (y), (z) - (x) + 1, (c))
#define VW_Vlin(y, z, x, c) VL_Vlin((x), (y), (z) - (y) + 1, (c))
#define VW_ColorBorder
#define VW_WaitVBL VL_WaitVBL
#define VW_FadeIn() VL_FadeIn(0, 255, vgapal, 30);
#define VW_FadeOut() VL_FadeOut(0, 255, 0, 0, 0, 30);
#define VW_ScreenToScreen VL_ScreenToScreen
void VW_MeasurePropString(
    const char* string,
    int* width,
    int* height);
#define VW_UpdateScreen() VH_UpdateScreen()

#define LatchDrawChar(x, y, p) VL_LatchToScreen(latchpics[0] + (p) * 16, 2, 8, x, y)
#define LatchDrawTile(x, y, p) VL_LatchToScreen(latchpics[1] + (p) * 64, 4, 16, x, y)

void LatchDrawPic(
    int x,
    int y,
    int picnum);
void LoadLatchMem();

bool FizzleFade(
    int source,
    int dest,
    int width,
    int height,
    int frames,
    bool abortable);

extern int latchpics[];
extern int freelatch;

extern int LatchMemFree;
