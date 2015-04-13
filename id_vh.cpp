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


#include "id_heads.h"


void VL_LatchToScreen(
    int source,
    int width,
    int height,
    int x,
    int y);
void IN_StartAck();
boolean IN_CheckAck();
void CalcTics();
void ForceUpdateStatusBar();

pictabletype* pictable;
pictabletype* picmtable;

int px;
int py;
Uint8 fontcolor;
Uint8 backcolor;
int fontnumber;

bool allcaps = false;


void VW_DrawPropString(
    const char* string)
{
    fontstruct* font =
        static_cast<fontstruct*>(grsegs[STARTFONT + fontnumber]);

    int height = font->height;

    int string_length = static_cast<int>(strlen(string));

    for (int c = 0; c < string_length; ++c) {
        Uint8 ch = string[c];
        int width = font->width[ch];

        const Uint8* source =
            (reinterpret_cast<const Uint8*>(font)) + font->location[ch];

        for (int w = 0; w < width; ++w) {
            for (int h = 0; h < height; ++h) {
                if (source[h * width] != 0) {
                    VL_Plot(px + w, py + h, fontcolor);
                }
            }

            ++source;
        }

        px += width;
    }
}

void VWL_MeasureString(
    const char* string,
    int* width,
    int* height,
    fontstruct* font)
{
    *height = font->height;

    for (*width = 0; string[0] != '\0'; ++string) {
        // proportional width
        *width += font->width[static_cast<Uint8>(*string)];
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
        static_cast<const Uint8*>(grsegs[chunknum]),
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
        static_cast<const Uint8*>(grsegs[chunknum]),
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
    Uint8 color)
{
    VW_Bar(x, y, width, height, color);
}

void VWB_Plot(
    int x,
    int y,
    Uint8 color)
{
    VW_Plot(x, y, color);
}

void VWB_Hlin(
    int x1,
    int x2,
    int y,
    Uint8 color)
{
    VW_Hlin(x1, x2, y, color);
}

void VWB_Vlin(
    int y1,
    int y2,
    int x,
    Uint8 color)
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

    VL_LatchToScreen(source, wide / 4, height, x * 8, y);
}

int destoff;

void LoadLatchMem()
{
    int picnum = 0;

    //
    // tile 8s
    //
    latchpics[picnum++] = freelatch;
    CA_CacheGrChunk(STARTTILE8);
    const Uint8* src = static_cast<const Uint8*>(grsegs[STARTTILE8]);
    destoff = freelatch;

    for (int i = 0; i < NUMTILE8; ++i) {
        VL_MemToLatch(src, 8, 8, destoff);
        src += 64;
        destoff += 16;
    }

    UNCACHEGRCHUNK(STARTTILE8);

    //
    // pics
    //
    ++picnum;

    for (int i = LATCHPICS_LUMP_START; i <= LATCHPICS_LUMP_END; ++i) {
        latchpics[picnum++] = destoff;
        CA_CacheGrChunk(i);
        int width = pictable[i - STARTPICS].width;
        int height = pictable[i - STARTPICS].height;

        VL_MemToLatch(static_cast<const Uint8*>(grsegs[i]),
                      width, height, destoff);

        destoff += (width / 4) * height;
        UNCACHEGRCHUNK(i);
    }
}

extern ControlInfo c;

// Returns true if aborted
bool FizzleFade(
    int source,
    int dest,
    int width,
    int height,
    int frames,
    bool abortable)
{
    int rndval = 1;
    int pixperframe = 64000 / frames;
    int remain_pixels = 64000 - (frames * pixperframe);
    int src_offset = vl_get_offset(source);
    int dst_offset = vl_get_offset(dest);

    IN_StartAck();

    TimeCount = 0;
    int frame = 0;
    LastScan = sc_none;

    bool finished = false;
    bool do_full_copy = false;

    while (!finished) {
        if (abortable && IN_CheckAck()) {
            return true;
        }

        if (!do_full_copy) {
            int pixel_count = pixperframe + remain_pixels;
            remain_pixels = 0;

            for (int p = 0; p < pixel_count; ++p) {
                int x = (rndval >> 8) & 0xFFFF;
                int y = ((rndval & 0xFF) - 1) & 0xFF;

                bool carry = ((rndval & 1) != 0);

                rndval >>= 1;

                if (carry) {
                    rndval ^= 0x00012000;
                }

                if (x > width || y > height) {
                    continue;
                }

                int pixel_offset = vga_scale * ((y * vga_width) + x);

                for (int dy = 0; dy < vga_scale; ++dy) {
                    for (int dx = 0; dx < vga_scale; ++dx) {
                        int offset = pixel_offset + dx;

                        vga_memory[dst_offset + offset] =
                            vga_memory[src_offset + offset];
                    }

                    pixel_offset += vga_width;
                }

                if (rndval == 1) {
                    do_full_copy = true;
                }
            }
        } else {
            finished = true;

            std::uninitialized_copy(
                &vga_memory[src_offset],
                &vga_memory[src_offset + (width * height)],
                &vga_memory[dst_offset]);
        }

        VL_RefreshScreen();

        ++frame;

        CalcTics();
    }

    return !finished;
}
