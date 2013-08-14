// ID_VL.H


#ifndef ID_VL_H
#define ID_VL_H


#include "SDL.h"


// wolf compatability

#define MS_Quit	Quit

void Quit (char *error,...);

//===========================================================================


#define SC_INDEX			0x3C4
#define SC_RESET			0
#define SC_CLOCK			1
#define SC_MAPMASK			2
#define SC_CHARMAP			3
#define SC_MEMMODE			4

#define CRTC_INDEX			0x3D4
#define CRTC_H_TOTAL		0
#define CRTC_H_DISPEND		1
#define CRTC_H_BLANK		2
#define CRTC_H_ENDBLANK		3
#define CRTC_H_RETRACE		4
#define CRTC_H_ENDRETRACE 	5
#define CRTC_V_TOTAL		6
#define CRTC_OVERFLOW		7
#define CRTC_ROWSCAN		8
#define CRTC_MAXSCANLINE 	9
#define CRTC_CURSORSTART 	10
#define CRTC_CURSOREND		11
#define CRTC_STARTHIGH		12
#define CRTC_STARTLOW		13
#define CRTC_CURSORHIGH		14
#define CRTC_CURSORLOW		15
#define CRTC_V_RETRACE		16
#define CRTC_V_ENDRETRACE 	17
#define CRTC_V_DISPEND		18
#define CRTC_OFFSET			19
#define CRTC_UNDERLINE		20
#define CRTC_V_BLANK		21
#define CRTC_V_ENDBLANK		22
#define CRTC_MODE			23
#define CRTC_LINECOMPARE 	24


#define GC_INDEX			0x3CE
#define GC_SETRESET			0
#define GC_ENABLESETRESET 	1
#define GC_COLORCOMPARE		2
#define GC_DATAROTATE		3
#define GC_READMAP			4
#define GC_MODE				5
#define GC_MISCELLANEOUS 	6
#define GC_COLORDONTCARE 	7
#define GC_BITMASK			8

#define ATR_INDEX			0x3c0
#define ATR_MODE			16
#define ATR_OVERSCAN		17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN			19
#define ATR_COLORSELECT		20

#define	STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9


//===========================================================================

#define SCREENSEG		0xa000

#define NOFADE_CODE

#define SCREENWIDTH		80			// default screen width in bytes
#define MAXSCANLINES	200			// size of ylookup table

#define CHARWIDTH		2
#define TILEWIDTH		4

//===========================================================================

extern	int	bufferofs;			// all drawing is reletive to this
extern	int	displayofs,pelpan;	// last setscreen coordinates

extern	Uint16	screenseg;			// set to 0xa000 for asm convenience

extern	Uint16	linewidth;
extern	Uint16	ylookup[MAXSCANLINES];

extern	boolean		screenfaded;
extern	Uint16	bordercolor;


// BBi
extern int vanilla_screen_width;
extern int vanilla_screen_height;

//===========================================================================

//
// VGA hardware routines
//

// FIXME
#if 0
#define VGAWRITEMODE(x) asm{\
cli;\
mov dx,GC_INDEX;\
mov al,GC_MODE;\
out dx,al;\
inc dx;\
in al,dx;\
and al,252;\
or al,x;\
out dx,al;\
sti;}
#endif // 0

#define VGAWRITEMODE(x)

// FIXME
#if 0
#define VGAMAPMASK(x) asm{cli;mov dx,SC_INDEX;mov al,SC_MAPMASK;mov ah,x;out dx,ax;sti;}
#define VGAREADMAP(x) asm{cli;mov dx,GC_INDEX;mov al,GC_READMAP;mov ah,x;out dx,ax;sti;}
#endif // 0

#define VGAMAPMASK(x)
#define VGAREADMAP(x)


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
void VL_SetScreen (Sint16 crtc, Sint16 pelpan);

void VL_FillPalette(int red, int green, int blue);
void VL_SetColor(int color, int red, int green, int blue);
void VL_GetColor(int color, int* red, int* green, int* blue);
void VL_SetPalette(int first, int count, const Uint8* palette);
void VL_GetPalette(int first, int count, Uint8* palette);
void VL_SetPaletteIntensity(int start, int end, const Uint8* palette, int intensity);
void VL_FadeOut(int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn(int start, int end, Uint8* palette, int steps);
void VL_ColorBorder(Sint16 color);

void VL_Plot (int x, int y, int color);
void VL_Hlin (int x, int y, int width, int color);
void VL_Vlin (int x, int y, int height, int color);
void VL_Bar(int x, int y, int width, int height, int color);

void VL_MungePic (Uint8* source, Uint16 width, Uint16 height);
void VL_DrawPicBare (Sint16 x, Sint16 y, Uint8* pic, Sint16 width, Sint16 height);
void VL_MemToLatch(const Uint8* source, int width, int height, int dest);
void VL_ScreenToScreen (Uint16 source, Uint16 dest,Sint16 width, Sint16 height);
void VL_MemToScreen(const Uint8* source, int width, int height, int x, int y);
void VL_MaskMemToScreen (Uint8* source, Sint16 width, Sint16 height, Sint16 x, Sint16 y, Uint8 mask);
void VL_ScreenToMem(Uint8* dest, Sint16 width, Sint16 height, Sint16 x, Sint16 y);

void VL_DrawTile8String (char *str, char* tile8ptr, Sint16 printx, Sint16 printy);
void VL_DrawLatch8String (char *str, Uint16 tile8ptr, Sint16 printx, Sint16 printy);
void VL_SizeTile8String (char *str, Sint16 *width, Sint16 *height);
void VL_DrawPropString (char *str, Uint16 tile8ptr, Sint16 printx, Sint16 printy);
void VL_SizePropString (char *str, Sint16 *width, Sint16 *height, char* font);

// BBi
void VL_RefreshScreen();


#endif // ID_VL_H
