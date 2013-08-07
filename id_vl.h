// ID_VL.H

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

extern	unsigned short	bufferofs;			// all drawing is reletive to this
extern	unsigned short	displayofs,pelpan;	// last setscreen coordinates

extern	unsigned short	screenseg;			// set to 0xa000 for asm convenience

extern	unsigned short	linewidth;
extern	unsigned short	ylookup[MAXSCANLINES];

extern	boolean		screenfaded;
extern	unsigned short	bordercolor;

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
void VL_ClearVideo (byte color);

void VL_SetLineWidth (unsigned short width);
void VL_SetSplitScreen (short linenum);
void WaitVBL (short vbls);

void VL_WaitVBL (short vbls);
void VL_CrtcStart (short crtc);
void VL_SetScreen (short crtc, short pelpan);

void VL_FillPalette (short red, short green, short blue);
void VL_SetColor	(short color, short red, short green, short blue);
void VL_GetColor	(short color, short *red, short *green, short *blue);
void VL_SetPalette (byte firstreg, unsigned short numregs, byte* palette);
void VL_GetPalette (byte firstreg, unsigned short numregs, byte* palette);
void VL_SetPaletteIntensity(short start, short end, byte* palette, char intensity);
void VL_FadeOut (short start, short end, short red, short green, short blue, short steps);
void VL_FadeIn (short start, short end, byte* palette, short steps);
void VL_ColorBorder (short color);

void VL_Plot (short x, short y, short color);
void VL_Hlin (unsigned short x, unsigned short y, unsigned short width, unsigned short color);
void VL_Vlin (short x, short y, short height, short color);
void VL_Bar (short x, short y, short width, short height, short color);

void VL_MungePic (byte* source, unsigned short width, unsigned short height);
void VL_DrawPicBare (short x, short y, byte* pic, short width, short height);
void VL_MemToLatch (byte* source, short width, short height, unsigned short dest);
void VL_ScreenToScreen (unsigned short source, unsigned short dest,short width, short height);
void VL_MemToScreen (byte* source, short width, short height, short x, short y);
void VL_MaskMemToScreen (byte* source, short width, short height, short x, short y, byte mask);
void VL_ScreenToMem(byte* dest, short width, short height, short x, short y);

void VL_DrawTile8String (char *str, char* tile8ptr, short printx, short printy);
void VL_DrawLatch8String (char *str, unsigned short tile8ptr, short printx, short printy);
void VL_SizeTile8String (char *str, short *width, short *height);
void VL_DrawPropString (char *str, unsigned short tile8ptr, short printx, short printy);
void VL_SizePropString (char *str, short *width, short *height, char* font);


