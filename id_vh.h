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
    Sint16 width;
    Sint16 height;
    Sint16 orgx;
    Sint16 orgy;
    Sint16 xl;
    Sint16 yl;
    Sint16 xh;
    Sint16 yh;
    Sint16 shifts;
}; // struct spritetabletype

struct pictabletype {
    Sint16 width;
    Sint16 height;
}; // struct pictabletype

struct fontstruct {
    Sint16 height;
    Sint16 location[256];
    char width[256];
}; // struct fontstruct


extern pictabletype* pictable;
extern pictabletype* picmtable;
extern spritetabletype* spritetable;

extern Uint8 fontcolor;
extern int fontnumber;
extern int px;
extern int py;
extern bool allcaps;


//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8(int x, int y, int tile);
void VWB_DrawPic(int x, int y, int chunknum);
void VWB_DrawMPic(int x, int y, int chunknum);
void VWB_Bar(int x, int y, int width, int height, Uint8 color);

void VWB_DrawPropString(const char* string);
void VW_DrawPropString(const char* string);
void VWB_DrawMPropString (const char* string);
void VWB_DrawSprite (int x, int y, int chunknum);
void VWB_Plot (int x, int y, Uint8 color);
void VWB_Hlin (int x1, int x2, int y, Uint8 color);
void VWB_Vlin (int y1, int y2, int x, Uint8 color);


//
// wolfenstein EGA compatability stuff
//
extern const Uint8 vgapal[768];

void VH_SetDefaultColors();

#define VW_Startup		VL_Startup
#define VW_Shutdown		VL_Shutdown
#define VW_Bar			VL_Bar
#define VW_Plot			VL_Plot
#define VW_Hlin(x,z,y,c)	VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)	VL_Vlin(x,y,(z)-(y)+1,c)
#define VW_DrawPic		VH_DrawPic
#define VW_SetSplitScreen	VL_SetSplitScreen
#define VW_SetLineWidth		VL_SetLineWidth
#define VW_ColorBorder
#define VW_WaitVBL		VL_WaitVBL
#define VW_FadeIn()		VL_FadeIn(0,255,vgapal,30);
#define VW_FadeOut()	VL_FadeOut(0,255,0,0,0,30);
#define VW_ScreenToScreen	VL_ScreenToScreen
#define VW_SetDefaultColors	VH_SetDefaultColors
void VW_MeasurePropString(const char* string, int* width, int* height);
#define EGAMAPMASK(x)	VGAMAPMASK(x)
#define EGAWRITEMODE(x)	VGAWRITEMODE(x)

//#define VW_MemToScreen	VL_MemToLatch

#define VW_UpdateScreen() 	VH_UpdateScreen()


#define MS_Quit			Quit


#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)

void LatchDrawPic (int x, int y, int picnum);
void LoadLatchMem();

boolean FizzleFade(
    int source,
    int dest,
    int width,
    int height,
    int frames,
    boolean abortable);


#define NUMLATCHPICS	100
extern int latchpics[NUMLATCHPICS];
extern int freelatch;

extern int LatchMemFree;
