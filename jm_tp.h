#ifndef _JM_TP_H_
#define _JM_TP_H_

//-------------------------------------------------------------------------
// defines
//-------------------------------------------------------------------------

#define PI_MAX_NUM_DISP_STRS		1		// Num display str ptrs allocated for text presenter

#define TP_CASE_SENSITIVE			 		// ctrl codes are case sensitive

#define TP_640x200			0				// is presenter in 640 x 200 mode?

#define TP_RETURN_CHAR			'\r'
#define TP_CONTROL_CHAR			'^'

#define TP_CURSOR_SAVES		8				// MAX different points to save

#define TP_CNVT_CODE(c1,c2)	((c1)|(c2<<8))

#define TP_MAX_ANIMS 10
#define TP_MAX_PAGES	41

#define TP_MARGIN	1			// distance between xl/xh/yl/yh points and text

#define is_shadowed     ((flags & fl_shadowtext) == fl_shadowtext)
#define ch_width(ch)		(font->width[ch]) // +((flags & fl_shadowtext) == fl_shadowtext))
#define font_height 		(font->height)     //+((flags & fl_shadowtext) == fl_shadowtext))
#define TP_INIT_DISPLAY_STR(num,str_ptr)	  {if ((num)<PI_MAX_NUM_DISP_STRS)			\
																piStringTable[(num)]=(str_ptr);     \
															else                                   \
																TP_ERROR(TP_DISPLAY_STR_NUM_BAD);}


//
// global static flags
//

#define fl_center				0x0001
#define fl_uncachefont		0x0002
#define fl_boxshape			0x0004
#define fl_shadowtext		0x0008
#define fl_presenting		0x0010
#define fl_startofline		0x0020
#define fl_upreleased		0x0040
#define fl_dnreleased		0x0080
#define fl_pagemanager		0x0100
#define fl_hidecursor		0x0200
#define fl_shadowpic			0x0400
#define fl_clearscback		0x0800


//
// PresenterInfo structure flags
//

#define TPF_CACHED_SCRIPT	0x0001
#define TPF_CACHE_NO_GFX	0x0002
#define TPF_CONTINUE			0x0004
#define TPF_USE_CURRENT    0x0008
#define TPF_SHOW_CURSOR		0x0010
#define TPF_SCROLL_REGION	0x0020
#define TPF_SHOW_PAGES		0x0040
#define TPF_TERM_SOUND		0x0080
#define TPF_ABORTABLE		0x0100


//-------------------------------------------------------------------------
//  typedefs
//-------------------------------------------------------------------------
typedef struct {
	unsigned flags,gflags;
	char far *script[TP_MAX_PAGES];
	memptr scriptstart;
	char numpages,pagenum;
	unsigned xl,yl,xh,yh;
	char fontnumber;
	char bgcolor,ltcolor,dkcolor,shcolor;
	unsigned cur_x,cur_y;
	char print_delay;
	byte highlight_color,fontcolor;
	short id_cache;
	char far *infoline;
} PresenterInfo;

typedef enum tpCacheType {ct_scaled,
								  ct_chunk,
								  ct_marks,
								  ct_music,
} tpCacheType;

typedef enum pisType {pis_pic,
							 pis_sprite,
							 pis_scaled,
							 pis_scwall,
							 pis_latchpic,
#if TP_640x200
							 pis_pic2x,
							 pis_sprite2x,
#endif
} pisType;

typedef struct {
	unsigned shapenum;
	pisType shapetype;
} piShapeInfo;

typedef enum piaType {pia_grabscript,
							 pia_shapetable,
} piaType;

typedef enum pidType {pid_cycle,
							 pid_rebound,
} pidType;

typedef struct {
	short baseshape;
	char frame;
	char maxframes;
	short delay;
	short maxdelay;
	piaType animtype;
	pidType dirtype;
	short x,y;
	char diradd;
} piAnimInfo;

//-------------------------------------------------------------------------
// variable externs
//-------------------------------------------------------------------------
extern piShapeInfo far piShapeTable[];
extern piAnimInfo far piAnimTable[];
extern piAnimInfo far piAnimList[TP_MAX_ANIMS];
extern char far * far piStringTable[PI_MAX_NUM_DISP_STRS];
extern byte TPscan;

//-------------------------------------------------------------------------
// Function prototypes
//-------------------------------------------------------------------------
void TP_Presenter(PresenterInfo *pi);
void TP_WrapText(void);
void TP_HandleCodes(void);
short TP_DrawShape(short x, short y, short shapenum, pisType type);
unsigned TP_VALUE(char far *ptr,char num_nybbles);
long TP_LoadScript(char *filename,PresenterInfo *pi, unsigned id_cache);
void TP_FreeScript(PresenterInfo *pi,unsigned id_cache);
void TP_InitScript(PresenterInfo *pi);
void TP_AnimatePage(short numanims);
short TP_BoxAroundShape(short x1, short y1, unsigned shapenum, pisType shapetype);
void TP_JumpCursor(void);
void TP_Print(char far *str,boolean single_char);
boolean TP_SlowPrint(char far *str, char delay);
void TP_PurgeAllGfx(void);
void TP_CachePage(char far *script);
void TP_CacheIn(tpCacheType type, short chunk);
void TP_ResetPagePointers(void);
short TP_LineCommented(char far *s);
void TP_PrintPageNumber(void);

#endif
