/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include "id_ca.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"
#include "gfxv.h"


extern ControlInfo c;


void VL_LatchToScreen(
	int source,
	int width,
	int height,
	int x,
	int y);

void IN_StartAck();

bool IN_CheckAck();

void CalcTics();

void ForceUpdateStatusBar();


PicTable pictable;
pictabletype* picmtable;

std::int16_t px;
std::int16_t py;
std::uint8_t fontcolor;
std::uint8_t backcolor;
std::int16_t fontnumber;


// BBi
LatchesCache latches_cache;


void VW_DrawPropString(
	const char* string)
{
	const auto font = reinterpret_cast<fontstruct*>(grsegs[STARTFONT + fontnumber].data());

	int height = font->height;

	int string_length = static_cast<int>(strlen(string));

	for (int i_ch = 0; i_ch < string_length; ++i_ch)
	{
		std::uint8_t ch = string[i_ch];
		int width = font->width[ch];

		const std::uint8_t* source =
			(reinterpret_cast<const std::uint8_t*>(font)) + font->location[ch];

		for (int w = 0; w < width; ++w)
		{
			for (int h = 0; h < height; ++h)
			{
				if (source[h * width] != 0)
				{
					VL_Plot(px + w, py + h, fontcolor);
				}
			}

			++source;
		}

		px = static_cast<std::int16_t>(px + width);
	}
}

void VWL_MeasureString(
	const char* string,
	int* width,
	int* height,
	fontstruct* font)
{
	*height = font->height;

	for (*width = 0; string[0] != '\0'; ++string)
	{
		// proportional width
		*width += font->width[static_cast<std::uint8_t>(*string)];
	}
}

void VW_MeasurePropString(
	const char* string,
	int* width,
	int* height)
{
	VWL_MeasureString(
		string,
		width,
		height,
		reinterpret_cast<fontstruct*>(grsegs[STARTFONT + fontnumber].data()));
}


/*
=============================================================================

 Double buffer management routines

=============================================================================
*/

void VWB_DrawTile8(
	int x,
	int y,
	int tile)
{
	LatchDrawChar(x, y, tile);
}

void VWB_DrawPic(
	int x,
	int y,
	int chunknum)
{
	int picnum = chunknum - STARTPICS;
	int width = pictable[picnum].width;
	int height = pictable[picnum].height;

	VL_MemToScreen(
		grsegs[chunknum].data(),
		width,
		height,
		x & (~7),
		y);
}

void VWB_DrawMPic(
	int x,
	int y,
	int chunknum)
{
	int picnum = chunknum - STARTPICS;
	int width = pictable[picnum].width;
	int height = pictable[picnum].height;

	VL_MaskMemToScreen(
		grsegs[chunknum].data(),
		width,
		height,
		x,
		y,
		255);
}

void VWB_DrawPropString(
	const char* string)
{
	VW_DrawPropString(string);
}

void VWB_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color)
{
	VW_Bar(x, y, width, height, color);
}

void VWB_Plot(
	int x,
	int y,
	std::uint8_t color)
{
	VW_Plot(x, y, color);
}

void VWB_Hlin(
	int x1,
	int x2,
	int y,
	std::uint8_t color)
{
	VW_Hlin(x1, x2, y, color);
}

void VWB_Vlin(
	int y1,
	int y2,
	int x,
	std::uint8_t color)
{
	VW_Vlin(y1, y2, x, color);
}

void vwb_rect(
	const int x,
	const int y,
	const int width,
	const int height,
	const int color)
{
	VWB_Bar(x, y, width, 1, static_cast<std::uint8_t>(color));
	VWB_Bar(x, y + height - 1, width, 1, static_cast<std::uint8_t>(color));

	if (height > 2)
	{
		VWB_Bar(x, y + 1, 1, height - 2, static_cast<std::uint8_t>(color));
		VWB_Bar(x + width - 1, y + 1, 1, height - 2, static_cast<std::uint8_t>(color));
	}
}


/*
=============================================================================

 WOLFENSTEIN STUFF

=============================================================================
*/

void LatchDrawPic(
	int x,
	int y,
	int picnum)
{
	int wide = pictable[picnum - STARTPICS].width;
	int height = pictable[picnum - STARTPICS].height;
	int source = latchpics[2 + picnum - LATCHPICS_LUMP_START];

	VL_LatchToScreen(source, wide, height, x * 8, y);
}

void LoadLatchMem()
{
	// Calculate total size of latches cache.
	//
	const auto tile8_total_size =
		STARTTILE8 * 8 * 8;

	int pics_total_size = 0;

	for (int i = LATCHPICS_LUMP_START; i <= LATCHPICS_LUMP_END; ++i)
	{
		const auto width = pictable[i - STARTPICS].width;
		const auto height = pictable[i - STARTPICS].height;

		pics_total_size += width * height;
	}

	const auto latches_cache_size = tile8_total_size + pics_total_size;

	latches_cache.resize(
		latches_cache_size);


	int destoff = 0;
	int picnum = 0;

	//
	// tile 8s
	//
	latchpics[picnum++] = destoff;
	CA_CacheGrChunk(STARTTILE8);
	auto src = grsegs[STARTTILE8].data();

	for (int i = 0; i < NUMTILE8; ++i)
	{
		VL_MemToLatch(src, 8, 8, destoff);
		src += 64;
		destoff += 64;
	}

	UNCACHEGRCHUNK(STARTTILE8);

	//
	// pics
	//
	++picnum;

	for (int i = LATCHPICS_LUMP_START; i <= LATCHPICS_LUMP_END; ++i)
	{
		const auto width = pictable[i - STARTPICS].width;
		const auto height = pictable[i - STARTPICS].height;

		CA_CacheGrChunk(static_cast<std::int16_t>(i));

		VL_MemToLatch(
			grsegs[i].data(),
			width,
			height,
			destoff);

		UNCACHEGRCHUNK(i);

		latchpics[picnum++] = destoff;
		destoff += width * height;
	}
}


void VW_Startup()
{
	VL_Startup();
}

void VW_Shutdown()
{
	VL_Shutdown();
}

void VW_Bar(
	int x,
	int y,
	int width,
	int height,
	std::uint8_t color,
	const bool is_transparent)
{
	VL_Bar(x, y, width, height, color, is_transparent);
}

void VW_Plot(
	int x,
	int y,
	std::uint8_t color,
	const bool is_transparent)
{
	VL_Plot(x, y, color, is_transparent);
}

void VW_Hlin(
	int x,
	int z,
	int y,
	std::uint8_t color)
{
	VL_Hlin(x, y, z - x + 1, color);
}

void VW_Vlin(
	int y,
	int z,
	int x,
	std::uint8_t color)
{
	VL_Vlin(x, y, z - y + 1, color);
}

void VW_WaitVBL(
	std::uint32_t vbls)
{
	VL_WaitVBL(vbls);
}

void VW_FadeIn()
{
	VL_FadeIn(0, 255, vgapal, 30);
}

void VW_FadeOut()
{
	VL_FadeOut(0, 255, 0, 0, 0, 30);
}

void VW_ScreenToScreen(
	int source,
	int dest,
	int width,
	int height)
{
	VL_ScreenToScreen(source, dest, width, height);
}

void VW_UpdateScreen()
{
	VL_RefreshScreen();
}

void LatchDrawChar(
	int x,
	int y,
	int p)
{
	VL_LatchToScreen(latchpics[0] + (p) * 64, 8, 8, x, y);
}

void LatchDrawTile(
	int x,
	int y,
	int p)
{
	VL_LatchToScreen(latchpics[1] + (p) * 64, 16, 16, x, y);
}
