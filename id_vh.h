// ID_VH.H


#define WHITE			15			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			15
#define F_BLACK			0
#define F_FIRSTCOLOR	1
#define F_SECONDCOLOR	12

//===========================================================================

#define MAXSHIFTS	1

typedef struct
{
  short	width,
	height,
	orgx,orgy,
	xl,yl,xh,yh,
	shifts;
} spritetabletype;

typedef	struct
{
	unsigned short	sourceoffset[MAXSHIFTS];
	unsigned short	planesize[MAXSHIFTS];
	unsigned short	width[MAXSHIFTS];
	byte		data[];
} spritetype;		// the memptr for each sprite points to this

typedef struct
{
	short width,height;
} pictabletype;


typedef struct
{
	short height;
	short location[256];
	char width[256];
} fontstruct;


//===========================================================================


extern	pictabletype	*pictable;
extern	pictabletype	*picmtable;
extern	spritetabletype *spritetable;

extern	byte	fontcolor;
extern	short	fontnumber;
extern	short	px,py;
extern	boolean allcaps;




//
// Double buffer management routines
//

void VW_InitDoubleBuffer (void);
short	 VW_MarkUpdateBlock (short x1, short y1, short x2, short y2);
//void VW_UpdateScreen (void);			// Made a macro

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (short x, short y, short tile);
void VWB_DrawTile8M (short x, short y, short tile);
void VWB_DrawTile16 (short x, short y, short tile);
void VWB_DrawTile16M (short x, short y, short tile);
void VWB_DrawPic (short x, short y, short chunknum);
void VWB_DrawMPic(short x, short y, short chunknum);
void VWB_Bar (short x, short y, short width, short height, short color);

void VWB_DrawPropString	 (char *string);
void VW_DrawPropString (char *string);
void VWB_DrawMPropString (char *string);
void VWB_DrawSprite (short x, short y, short chunknum);
void VWB_Plot (short x, short y, short color);
void VWB_Hlin (short x1, short x2, short y, short color);
void VWB_Vlin (short y1, short y2, short x, short color);


//
// wolfenstein EGA compatability stuff
//
extern byte vgapal;

void VH_SetDefaultColors (void);

#define VW_Startup		VL_Startup
#define VW_Shutdown		VL_Shutdown
#define VW_SetCRTC		VL_SetCRTC
#define VW_SetScreen	VL_SetScreen
#define VW_Bar			VL_Bar
#define VW_Plot			VL_Plot
#define VW_Hlin(x,z,y,c)	VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)	VL_Vlin(x,y,(z)-(y)+1,c)
#define VW_DrawPic		VH_DrawPic
#define VW_SetSplitScreen	VL_SetSplitScreen
#define VW_SetLineWidth		VL_SetLineWidth
#define VW_ColorBorder	VL_ColorBorder
#define VW_WaitVBL		VL_WaitVBL
#define VW_FadeIn()		VL_FadeIn(0,255,&vgapal,30);
#define VW_FadeOut()	VL_FadeOut(0,255,0,0,0,30);
#define VW_ScreenToScreen	VL_ScreenToScreen
#define VW_SetDefaultColors	VH_SetDefaultColors
void	VW_MeasurePropString (char *string, word *width, word *height);
#define EGAMAPMASK(x)	VGAMAPMASK(x)
#define EGAWRITEMODE(x)	VGAWRITEMODE(x)

//#define VW_MemToScreen	VL_MemToLatch

#define VW_UpdateScreen() 	VH_UpdateScreen()


#define MS_Quit			Quit


#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)

void LatchDrawPic (unsigned short x, unsigned short y, unsigned short picnum);
void 	LoadLatchMem (void);
boolean 	FizzleFade (unsigned short source, unsigned short dest,
	unsigned short width,unsigned short height, unsigned short frames,boolean abortable);


#define NUMLATCHPICS	100
extern	unsigned short	latchpics[NUMLATCHPICS];
extern	unsigned short freelatch;

extern unsigned short LatchMemFree;
