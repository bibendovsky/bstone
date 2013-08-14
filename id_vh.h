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
  Sint16	width,
	height,
	orgx,orgy,
	xl,yl,xh,yh,
	shifts;
} spritetabletype;

typedef	struct
{
	Uint16	sourceoffset[MAXSHIFTS];
	Uint16	planesize[MAXSHIFTS];
	Uint16	width[MAXSHIFTS];
	Uint8		data[];
} spritetype;		// the memptr for each sprite points to this

typedef struct
{
	Sint16 width,height;
} pictabletype;


typedef struct
{
	Sint16 height;
	Sint16 location[256];
	char width[256];
} fontstruct;


//===========================================================================


extern	pictabletype	*pictable;
extern	pictabletype	*picmtable;
extern	spritetabletype *spritetable;

extern	Uint8	fontcolor;
extern	Sint16	fontnumber;
extern	Sint16	px,py;
extern	boolean allcaps;




//
// Double buffer management routines
//

void VW_InitDoubleBuffer (void);
Sint16	 VW_MarkUpdateBlock (Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2);
//void VW_UpdateScreen (void);			// Made a macro

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (Sint16 x, Sint16 y, Sint16 tile);
void VWB_DrawTile8M (Sint16 x, Sint16 y, Sint16 tile);
void VWB_DrawTile16 (Sint16 x, Sint16 y, Sint16 tile);
void VWB_DrawTile16M (Sint16 x, Sint16 y, Sint16 tile);
void VWB_DrawPic(int x, int y, int chunknum);
void VWB_DrawMPic(Sint16 x, Sint16 y, Sint16 chunknum);
void VWB_Bar (Sint16 x, Sint16 y, Sint16 width, Sint16 height, Sint16 color);

void VWB_DrawPropString	 (char *string);
void VW_DrawPropString (char *string);
void VWB_DrawMPropString (char *string);
void VWB_DrawSprite (Sint16 x, Sint16 y, Sint16 chunknum);
void VWB_Plot (Sint16 x, Sint16 y, Sint16 color);
void VWB_Hlin (Sint16 x1, Sint16 x2, Sint16 y, Sint16 color);
void VWB_Vlin (Sint16 y1, Sint16 y2, Sint16 x, Sint16 color);


//
// wolfenstein EGA compatability stuff
//
extern Uint8 vgapal;

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
void	VW_MeasurePropString (char *string, Uint16 *width, Uint16 *height);
#define EGAMAPMASK(x)	VGAMAPMASK(x)
#define EGAWRITEMODE(x)	VGAWRITEMODE(x)

//#define VW_MemToScreen	VL_MemToLatch

#define VW_UpdateScreen() 	VH_UpdateScreen()


#define MS_Quit			Quit


#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)

void LatchDrawPic (Uint16 x, Uint16 y, Uint16 picnum);
void 	LoadLatchMem (void);
boolean 	FizzleFade (Uint16 source, Uint16 dest,
	Uint16 width,Uint16 height, Uint16 frames,boolean abortable);


#define NUMLATCHPICS	100
extern	Uint16	latchpics[NUMLATCHPICS];
extern	Uint16 freelatch;

extern Uint16 LatchMemFree;
