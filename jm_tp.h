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


#ifndef _JM_TP_H_
#define _JM_TP_H_

// -------------------------------------------------------------------------
// defines
// -------------------------------------------------------------------------

#define PI_MAX_NUM_DISP_STRS 1 // Num display str ptrs allocated for text presenter

#define TP_CASE_SENSITIVE // ctrl codes are case sensitive

#define TP_640x200 0 // is presenter in 640 x 200 mode?

#define TP_RETURN_CHAR '\r'
#define TP_CONTROL_CHAR '^'

#define TP_CURSOR_SAVES 8 // MAX different points to save

#define TP_CNVT_CODE(c1, c2) ((c1) | (c2 << 8))

#define TP_MAX_ANIMS 10
#define TP_MAX_PAGES 41

#define TP_MARGIN 1 // distance between xl/xh/yl/yh points and text

#define is_shadowed ((flags & fl_shadowtext) == fl_shadowtext)
#define ch_width(ch) (font->width[static_cast<int>(ch)]) // +((flags & fl_shadowtext) == fl_shadowtext))
#define font_height (font->height) // +((flags & fl_shadowtext) == fl_shadowtext))
#define TP_INIT_DISPLAY_STR(num, str_ptr) { if ((num) < PI_MAX_NUM_DISP_STRS) { \
                                                piStringTable[(num)] = (str_ptr); } \
                                            else { \
                                                TP_ERROR(TP_DISPLAY_STR_NUM_BAD); } }


//
// global static flags
//

#define fl_center 0x0001
#define fl_uncachefont 0x0002
#define fl_boxshape 0x0004
#define fl_shadowtext 0x0008
#define fl_presenting 0x0010
#define fl_startofline 0x0020
#define fl_upreleased 0x0040
#define fl_dnreleased 0x0080
#define fl_pagemanager 0x0100
#define fl_hidecursor 0x0200
#define fl_shadowpic 0x0400
#define fl_clearscback 0x0800


//
// PresenterInfo structure flags
//

#define TPF_CACHED_SCRIPT 0x0001
#define TPF_CACHE_NO_GFX 0x0002
#define TPF_CONTINUE 0x0004
#define TPF_USE_CURRENT 0x0008
#define TPF_SHOW_CURSOR 0x0010
#define TPF_SCROLL_REGION 0x0020
#define TPF_SHOW_PAGES 0x0040
#define TPF_TERM_SOUND 0x0080
#define TPF_ABORTABLE 0x0100


// -------------------------------------------------------------------------
//  typedefs
// -------------------------------------------------------------------------
struct PresenterInfo {
    Uint16 flags;
    Uint16 gflags;
    const char* script[TP_MAX_PAGES];
    void* scriptstart;
    char numpages;
    char pagenum;
    Uint16 xl;
    Uint16 yl;
    Uint16 xh;
    Uint16 yh;
    char fontnumber;
    char bgcolor;
    char ltcolor;
    char dkcolor;
    char shcolor;
    Uint16 cur_x;
    Uint16 cur_y;
    char print_delay;
    Uint8 highlight_color;
    Uint8 fontcolor;
    Sint16 id_cache;
    char* infoline;
    int custom_line_height;
}; // struct PresenterInfo;

enum tpCacheType {
    ct_scaled,
    ct_chunk,
    ct_marks,
    ct_music
}; // enum tpCacheType

enum pisType {
    pis_pic,
    pis_sprite,
    pis_scaled,
    pis_scwall,
    pis_latchpic

#if TP_640x200
    ,
    pis_pic2x,
    pis_sprite2x
#endif
}; // enum pisType;

struct piShapeInfo {
    Uint16 shapenum;
    pisType shapetype;
}; // struct piShapeInfo

enum piaType {
    pia_grabscript,
    pia_shapetable
}; // enum piaType;

enum pidType {
    pid_cycle,
    pid_rebound
}; // enum pidType

struct piAnimInfo {
    Sint16 baseshape;
    char frame;
    char maxframes;
    Sint16 delay;
    Sint16 maxdelay;
    piaType animtype;
    pidType dirtype;
    Sint16 x, y;
    char diradd;
}; // struct piAnimInfo

// -------------------------------------------------------------------------
// variable externs
// -------------------------------------------------------------------------
extern piShapeInfo piShapeTable[];
extern piAnimInfo piAnimTable[];
extern piAnimInfo piAnimList[TP_MAX_ANIMS];
extern char* piStringTable[PI_MAX_NUM_DISP_STRS];
extern Uint8 TPscan;

// -------------------------------------------------------------------------
// Function prototypes
// -------------------------------------------------------------------------
void TP_Presenter(
    PresenterInfo* pi);
void TP_WrapText();
void TP_HandleCodes();
Sint16 TP_DrawShape(
    Sint16 x,
    Sint16 y,
    Sint16 shapenum,
    pisType type);
Uint16 TP_VALUE(
    const char* ptr,
    char num_nybbles);
Sint32 TP_LoadScript(
    const char* filename,
    PresenterInfo* pi,
    Uint16 id_cache);
void TP_FreeScript(
    PresenterInfo* pi,
    Uint16 id_cache);
void TP_InitScript(
    PresenterInfo* pi);
void TP_AnimatePage(
    Sint16 numanims);
Sint16 TP_BoxAroundShape(
    Sint16 x1,
    Sint16 y1,
    Uint16 shapenum,
    pisType shapetype);
void TP_JumpCursor();
void TP_Print(
    const char* str,
    boolean single_char);
boolean TP_SlowPrint(
    const char* str,
    char delay);
void TP_PurgeAllGfx();
void TP_CachePage(
    const char* script);
void TP_CacheIn(
    tpCacheType type,
    Sint16 chunk);
void TP_ResetPagePointers();
Sint16 TP_LineCommented(
    const char* s);
void TP_PrintPageNumber();

#endif
