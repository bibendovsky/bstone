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


#include <cstdint>


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
    uint16_t flags;
    uint16_t gflags;
    const char* script[TP_MAX_PAGES];
    void* scriptstart;
    char numpages;
    char pagenum;
    uint16_t xl;
    uint16_t yl;
    uint16_t xh;
    uint16_t yh;
    char fontnumber;
    unsigned char bgcolor;
    unsigned char ltcolor;
    unsigned char dkcolor;
    unsigned char shcolor;
    uint16_t cur_x;
    uint16_t cur_y;
    char print_delay;
    unsigned char highlight_color;
    unsigned char fontcolor;
    int16_t id_cache;
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
    uint16_t shapenum;
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
    int16_t baseshape;
    char frame;
    char maxframes;
    int16_t delay;
    int16_t maxdelay;
    piaType animtype;
    pidType dirtype;
    int16_t x, y;
    char diradd;
}; // struct piAnimInfo

// -------------------------------------------------------------------------
// variable externs
// -------------------------------------------------------------------------
using PiShapeInfos = std::vector<piShapeInfo>;

extern PiShapeInfos piShapeTable;
extern piAnimInfo piAnimTable[];
extern piAnimInfo piAnimList[TP_MAX_ANIMS];
extern char* piStringTable[PI_MAX_NUM_DISP_STRS];
extern uint8_t TPscan;

// -------------------------------------------------------------------------
// Function prototypes
// -------------------------------------------------------------------------
void TP_Presenter(
    PresenterInfo* pi);
void TP_WrapText();
void TP_HandleCodes();
int16_t TP_DrawShape(
    int16_t x,
    int16_t y,
    int16_t shapenum,
    pisType type);
uint16_t TP_VALUE(
    const char* ptr,
    char num_nybbles);
int32_t TP_LoadScript(
    const char* filename,
    PresenterInfo* pi,
    uint16_t id_cache);
void TP_FreeScript(
    PresenterInfo* pi,
    uint16_t id_cache);
void TP_InitScript(
    PresenterInfo* pi);
void TP_AnimatePage(
    int16_t numanims);
int16_t TP_BoxAroundShape(
    int16_t x1,
    int16_t y1,
    uint16_t shapenum,
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
    int16_t chunk);
void TP_ResetPagePointers();
int16_t TP_LineCommented(
    const char* s);
void TP_PrintPageNumber();

#endif
