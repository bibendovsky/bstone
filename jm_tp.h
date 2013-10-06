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
#define ch_width(ch)		(font->width[static_cast<int>(ch)]) // +((flags & fl_shadowtext) == fl_shadowtext))
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
	Uint16 flags,gflags;
	const char *script[TP_MAX_PAGES];
	void* scriptstart;
	char numpages,pagenum;
	Uint16 xl,yl,xh,yh;
	char fontnumber;
	char bgcolor,ltcolor,dkcolor,shcolor;
	Uint16 cur_x,cur_y;
	char print_delay;
	Uint8 highlight_color,fontcolor;
	Sint16 id_cache;
	char *infoline;
} PresenterInfo;

typedef enum tpCacheType {ct_scaled,
								  ct_chunk,
								  ct_marks,
								  ct_music
} tpCacheType;

typedef enum pisType {pis_pic,
							 pis_sprite,
							 pis_scaled,
							 pis_scwall,
							 pis_latchpic
#if TP_640x200
    ,
							 pis_pic2x,
							 pis_sprite2x
#endif
} pisType;

typedef struct {
	Uint16 shapenum;
	pisType shapetype;
} piShapeInfo;

typedef enum piaType {pia_grabscript,
							 pia_shapetable
} piaType;

typedef enum pidType {pid_cycle,
							 pid_rebound
} pidType;

typedef struct {
	Sint16 baseshape;
	char frame;
	char maxframes;
	Sint16 delay;
	Sint16 maxdelay;
	piaType animtype;
	pidType dirtype;
	Sint16 x,y;
	char diradd;
} piAnimInfo;

//-------------------------------------------------------------------------
// variable externs
//-------------------------------------------------------------------------
extern piShapeInfo piShapeTable[];
extern piAnimInfo piAnimTable[];
extern piAnimInfo piAnimList[TP_MAX_ANIMS];
extern char *piStringTable[PI_MAX_NUM_DISP_STRS];
extern Uint8 TPscan;

//-------------------------------------------------------------------------
// Function prototypes
//-------------------------------------------------------------------------
void TP_Presenter(PresenterInfo *pi);
void TP_WrapText(void);
void TP_HandleCodes(void);
Sint16 TP_DrawShape(Sint16 x, Sint16 y, Sint16 shapenum, pisType type);
Uint16 TP_VALUE(const char *ptr,char num_nybbles);
Sint32 TP_LoadScript(const char *filename,PresenterInfo *pi, Uint16 id_cache);
void TP_FreeScript(PresenterInfo *pi,Uint16 id_cache);
void TP_InitScript(PresenterInfo *pi);
void TP_AnimatePage(Sint16 numanims);
Sint16 TP_BoxAroundShape(Sint16 x1, Sint16 y1, Uint16 shapenum, pisType shapetype);
void TP_JumpCursor(void);
void TP_Print(const char *str,boolean single_char);
boolean TP_SlowPrint(const char *str, char delay);
void TP_PurgeAllGfx(void);
void TP_CachePage(const char *script);
void TP_CacheIn(tpCacheType type, Sint16 chunk);
void TP_ResetPagePointers(void);
Sint16 TP_LineCommented(const char *s);
void TP_PrintPageNumber(void);

#endif
