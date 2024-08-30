/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_ID_VH_INCLUDED
#define BSTONE_ID_VH_INCLUDED


#include <cstdint>

#include <vector>


#define WHITE (15) // graphics mode independent colors
#define BLACK (0)


struct spritetabletype
{
	std::int16_t width;
	std::int16_t height;
	std::int16_t orgx;
	std::int16_t orgy;
	std::int16_t xl;
	std::int16_t yl;
	std::int16_t xh;
	std::int16_t yh;
	std::int16_t shifts;
}; // spritetabletype

struct pictabletype
{
	std::int16_t width;
	std::int16_t height;
}; // pictabletype

struct fontstruct
{
	std::int16_t height;
	std::int16_t location[256];
	char width[256];
}; // fontstruct


using PicTable = std::vector<pictabletype>;
extern PicTable pictable;

extern pictabletype* picmtable;
extern spritetabletype* spritetable;

extern std::uint8_t fontcolor;
extern std::int16_t fontnumber;
extern std::int16_t px;
extern std::int16_t py;

//
// wolfenstein EGA compatibility stuff
//
extern const std::uint8_t vgapal[768];

extern int latchpics[];

extern int LatchMemFree;

using LatchesCache = std::vector<std::uint8_t>;
extern LatchesCache latches_cache;


//
// mode independent routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8(
	int x,
	int y,
	int tile);

void VWB_DrawPic(
	int x,
	int y,
	int chunknum);

void VWB_DrawMPic(
	int x,
	int y,
	int chunknum);

void VWB_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color);

void VWB_DrawPropString(
	const char* string);

void VW_DrawPropString(
	const char* string);

void VWB_Plot(
	int x,
	int y,
	std::uint8_t color);

void VWB_Hlin(
	int x1,
	int x2,
	int y,
	std::uint8_t color);

void VWB_Vlin(
	int y1,
	int y2,
	int x,
	std::uint8_t color);

void VW_MeasurePropString(
	const char* string,
	int* width,
	int* height);

void LatchDrawPic(
	int x,
	int y,
	int picnum);

void LoadLatchMem();

void vwb_rect(
	const int x,
	const int y,
	const int width,
	const int height,
	const int color);


void VW_Startup();

void VW_Shutdown();

void VW_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	const bool is_transparent = false);

void VW_Plot(
	int x,
	int y,
	std::uint8_t color,
	const bool is_transparent = false);

void VW_Hlin(
	int x,
	int z,
	int y,
	std::uint8_t c);

void VW_Vlin(
	int y,
	int z,
	int x,
	std::uint8_t c);

void VW_WaitVBL(
	std::uint32_t vbls);

void VW_FadeIn();

void VW_FadeOut();

void VW_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height);

void VW_UpdateScreen();

void LatchDrawChar(
	int x,
	int y,
	int p);

void LatchDrawTile(
	int x,
	int y,
	int p);


#endif // BSTONE_ID_VH_INCLUDED
