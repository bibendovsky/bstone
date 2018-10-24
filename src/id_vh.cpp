/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


#include "id_heads.h"


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

pictabletype* pictable;
pictabletype* picmtable;

std::int16_t px;
std::int16_t py;
std::uint8_t fontcolor;
std::uint8_t backcolor;
std::int16_t fontnumber;

bool allcaps = false;

// BBi
LatchesCache latches_cache;

void VW_DrawPropString(
	const char* string)
{
	fontstruct* font =
		static_cast<fontstruct*>(grsegs[STARTFONT + fontnumber]);

	int height = font->height;

	int string_length = static_cast<int>(strlen(string));

	for (int c = 0; c < string_length; ++c)
	{
		std::uint8_t ch = string[c];
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
		static_cast<fontstruct*>(grsegs[STARTFONT + fontnumber]));
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
		static_cast<const std::uint8_t*>(grsegs[chunknum]),
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
		static_cast<const std::uint8_t*>(grsegs[chunknum]),
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
		::STARTTILE8 * 8 * 8;

	int pics_total_size = 0;

	for (int i = ::LATCHPICS_LUMP_START; i <= ::LATCHPICS_LUMP_END; ++i)
	{
		const auto width = pictable[i - ::STARTPICS].width;
		const auto height = pictable[i - ::STARTPICS].height;

		pics_total_size += width * height;
	}

	const auto latches_cache_size = tile8_total_size + pics_total_size;

	::latches_cache.resize(
		latches_cache_size);


	int destoff = 0;
	int picnum = 0;

	//
	// tile 8s
	//
	::latchpics[picnum++] = destoff;
	::CA_CacheGrChunk(::STARTTILE8);
	auto src = static_cast<const std::uint8_t*>(::grsegs[::STARTTILE8]);

	for (int i = 0; i < ::NUMTILE8; ++i)
	{
		::VL_MemToLatch(src, 8, 8, destoff);
		src += 64;
		destoff += 64;
	}

	::UNCACHEGRCHUNK(::STARTTILE8);

	//
	// pics
	//
	++picnum;

	for (int i = ::LATCHPICS_LUMP_START; i <= ::LATCHPICS_LUMP_END; ++i)
	{
		const auto width = pictable[i - ::STARTPICS].width;
		const auto height = pictable[i - ::STARTPICS].height;

		::CA_CacheGrChunk(i);

		::VL_MemToLatch(
			static_cast<const std::uint8_t*>(::grsegs[i]),
			width,
			height,
			destoff);

		::UNCACHEGRCHUNK(i);

		::latchpics[picnum++] = destoff;
		destoff += width * height;
	}
}

extern ControlInfo c;
