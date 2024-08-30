/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_JM_TP_INCLUDED
#define BSTONE_JM_TP_INCLUDED


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
struct PresenterInfo
{
	std::uint16_t flags;
	std::uint16_t gflags;
	const char* script[TP_MAX_PAGES];
	void* scriptstart;
	std::int8_t numpages;
	std::int8_t pagenum;
	std::uint16_t xl;
	std::uint16_t yl;
	std::uint16_t xh;
	std::uint16_t yh;
	std::int8_t fontnumber;
	std::uint8_t bgcolor;
	std::uint8_t ltcolor;
	std::uint8_t dkcolor;
	std::uint8_t shcolor;
	std::uint16_t cur_x;
	std::uint16_t cur_y;
	std::int8_t print_delay;
	std::uint8_t highlight_color;
	std::uint8_t fontcolor;
	std::int16_t id_cache;
	char* infoline;
	int custom_line_height;
}; // PresenterInfo;

enum tpCacheType
{
	ct_scaled,
	ct_chunk,
	ct_marks,
	ct_music
}; // tpCacheType

enum pisType
{
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
}; // pisType;

struct piShapeInfo
{
	std::int16_t shapenum;
	pisType shapetype;
}; // piShapeInfo

enum piaType
{
	pia_grabscript,
	pia_shapetable
}; // piaType;

enum pidType
{
	pid_cycle,
	pid_rebound
}; // pidType

struct piAnimInfo
{
	std::int16_t baseshape{};
	std::int8_t frame{};
	std::int8_t maxframes{};
	std::int16_t delay{};
	std::int16_t maxdelay{};
	piaType animtype{pia_grabscript};
	pidType dirtype{pid_cycle};
	std::int16_t x{}, y{};
	std::int8_t diradd{};
}; // piAnimInfo

// -------------------------------------------------------------------------
// variable externs
// -------------------------------------------------------------------------
using PiShapeInfos = std::vector<piShapeInfo>;
using PiAnimationInfos = std::vector<piAnimInfo>;

extern PiShapeInfos piShapeTable;
extern PiAnimationInfos piAnimTable;
extern piAnimInfo piAnimList[TP_MAX_ANIMS];
extern const char* piStringTable[PI_MAX_NUM_DISP_STRS];
extern ScanCode TPscan;

// -------------------------------------------------------------------------
// Function prototypes
// -------------------------------------------------------------------------
void TP_Presenter(
	PresenterInfo* pi);
void TP_WrapText();
void TP_HandleCodes();
std::int16_t TP_DrawShape(
	std::int16_t x,
	std::int16_t y,
	std::int16_t shapenum,
	pisType type);
std::uint16_t TP_VALUE(
	const char* ptr,
	std::int8_t num_nybbles);
std::int32_t TP_LoadScript(
	const char* filename,
	PresenterInfo* pi,
	std::uint16_t id_cache);
void TP_FreeScript(
	PresenterInfo* pi,
	std::uint16_t id_cache);
void TP_InitScript(
	PresenterInfo* pi);
void TP_AnimatePage(
	std::int16_t numanims);
std::int16_t TP_BoxAroundShape(
	std::int16_t x1,
	std::int16_t y1,
	std::uint16_t shapenum,
	pisType shapetype);
void TP_JumpCursor();
void TP_Print(
	const char* str,
	bool single_char);
bool TP_SlowPrint(
	const char* str,
	std::int8_t delay);
void TP_PurgeAllGfx();
void TP_CachePage(
	const char* script);
void TP_CacheIn(
	tpCacheType type,
	std::int16_t chunk);
std::int16_t TP_LineCommented(
	const char* s);
void TP_PrintPageNumber();


#endif // BSTONE_JM_TP_INCLUDED
