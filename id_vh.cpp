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


void VL_LatchToScreen(int source, int width, int height, int x, int y);
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


void VW_DrawPropString(const char* string)
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
                if (source[h * width] != 0)
                    VL_Plot(px + w, py + h, fontcolor);
            }

            ++source;
        }

        px += width;
    }
}

//==========================================================================

#if 0

/*
=================
=
= VL_MungePic
=
=================
*/

void VL_MungePic (Uint8 *source, unsigned width, unsigned height)
{
	unsigned	x,y,plane,size,pwidth;
	Uint8		*temp, *dest, *srcline;

	size = width*height;

	if (width&3)
		VH_ERROR(VL_MUNGEPIC_NO_DIV_FOUR);

//
// copy the pic to a temp buffer
//
	MM_GetPtr (&(memptr)temp,size);
	_fmemcpy (temp,source,size);

//
// munge it back into the original buffer
//
	dest = source;
	pwidth = width/4;

	for (plane=0;plane<4;plane++)
	{
		srcline = temp;
		for (y=0;y<height;y++)
		{
			for (x=0;x<pwidth;x++)
				*dest++ = *(srcline+x*4+plane);
			srcline+=width;
		}
	}

	MM_FreePtr (&(memptr)temp);
}

#endif

void VWL_MeasureString(
    const char* string,
    int* width,
    int* height,
    fontstruct* font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((const Uint8*)string)];	// proportional width
}

void	VW_MeasurePropString (const char* string, int* width, int* height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

#if 0

void	VW_MeasureMPropString  (char *string, Uint16 *width, Uint16 *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONTM+fontnumber]);
}
#endif


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/


void VWB_DrawTile8 (int x, int y, int tile)
{
	LatchDrawChar(x,y,tile);
}

void VWB_DrawPic(int x, int y, int chunknum)
{
	Sint16	picnum = static_cast<Sint16>(chunknum - STARTPICS);
	Uint16 width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen (static_cast<const Uint8*>(grsegs[chunknum]),width,height,x,y);
}


//--------------------------------------------------------------------------
// VWB_DrawMPic()
//--------------------------------------------------------------------------
void VWB_DrawMPic (int x, int y, int chunknum)
{
	Sint16	picnum = chunknum - STARTPICS;
	Uint16 width,height;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MaskMemToScreen (static_cast<const Uint8*>(grsegs[chunknum]),width,height,x,y,255);
}


void VWB_DrawPropString(const char* string)
{
	VW_DrawPropString (string);
}


void VWB_Bar (int x, int y, int width, int height, Uint8 color)
{
	VW_Bar (x,y,width,height,color);
}

void VWB_Plot (int x, int y, Uint8 color)
{
	VW_Plot(x,y,color);
}

void VWB_Hlin (int x1, int x2, int y, Uint8 color)
{
	VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (int y1, int y2, int x, Uint8 color)
{
	VW_Vlin(y1,y2,x,color);
}


#if 0		/// MADE A MACRO
void VW_UpdateScreen (void)
{
	VH_UpdateScreen ();
}
#endif


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/


void LatchDrawPic (int x, int y, int picnum)
{
	Uint16 wide, height, source;

	x <<= 3;
	wide = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;
	source = latchpics[2+picnum-LATCHPICS_LUMP_START];

	VL_LatchToScreen (source,wide/4,height,x,y);
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

//unsigned LatchMemFree = 0xffff;		
Uint16	destoff;

void LoadLatchMem (void)
{
	Sint16	i,width,height;
	Uint8	*src;
	Uint16	picnum=0;


//
// tile 8s
//
	latchpics[picnum++] = freelatch;
	CA_CacheGrChunk (STARTTILE8);
	src = (Uint8 *)grsegs[STARTTILE8];
	destoff = freelatch;

	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src,8,8,destoff);
		src += 64;
		destoff +=16;
	}
	UNCACHEGRCHUNK (STARTTILE8);

#if 0	// ran out of latch space!
//
// tile 16s
//
	src = (Uint8 *)grsegs[STARTTILE16];
	latchpics[picnum++] = destoff;

	for (i=0;i<NUMTILE16;i++)
	{
		CA_CacheGrChunk (STARTTILE16+i);
		src = (Uint8 *)grsegs[STARTTILE16+i];
		VL_MemToLatch (src,16,16,destoff);
		destoff+=64;
		if (src)
			UNCACHEGRCHUNK (STARTTILE16+i);
	}
#endif

//
// pics
//
	picnum++;
	for (i=LATCHPICS_LUMP_START;i<=LATCHPICS_LUMP_END;i++)	  
	{
		latchpics[picnum++] = destoff;
		CA_CacheGrChunk (i);
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		VL_MemToLatch (static_cast<const Uint8*>(grsegs[i]),width,height,destoff);
		destoff += width/4 *height;
		UNCACHEGRCHUNK(i);
	}
}


//==========================================================================


/*
===================
=
= FizzleFade
=
= returns true if aborted
=
===================
*/

extern	ControlInfo	c;

boolean FizzleFade(
    int source,
    int dest,
    int width,
    int height,
    int frames,
    boolean abortable)
{
    int pixperframe;
    int remain_pixels;
    Uint16 x;
    Uint16 y;
    int p;
    int frame;
    Sint32 rndval;
    boolean carry;
    int src_offset;
    int dst_offset;
    int pixel_offset;
    int pixel_count;

    y = 0;
    rndval = 1;
    pixperframe = 64000 / frames;
    remain_pixels = 64000 - (frames * pixperframe);
    src_offset = vl_get_offset(source);
    dst_offset = vl_get_offset(dest);

    IN_StartAck();

    TimeCount = 0;
    frame = 0;
    LastScan = 0;

    bool finished = false;
    bool do_full_copy = false;

    while (!finished) {
        if (abortable && IN_CheckAck())
            return true;

        if (!do_full_copy) {
            pixel_count = pixperframe + remain_pixels;
            remain_pixels = 0;

            for (p = 0; p < pixel_count; ++p) {
                x = (rndval >> 8) & 0xFFFF;
                y = ((rndval & 0xFF) - 1) & 0xFF;

                carry = ((rndval & 1) != 0);

                rndval >>= 1;

                if (carry)
                    rndval ^= 0x00012000;

                if (x > width || y > height)
                    continue;

                pixel_offset = vga_scale * ((y * vga_width) + x);

                for (int dy = 0; dy < vga_scale; ++dy) {
                    for (int dx = 0; dx < vga_scale; ++dx) {
                        int offset = pixel_offset + dx;

                        vga_memory[dst_offset + offset] =
                            vga_memory[src_offset + offset];
                    }

                    pixel_offset += vga_width;
                }

                if (rndval == 1)
                    do_full_copy = true;
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

        while (TimeCount < static_cast<Uint32>(frame)) // don't go too fast
            ;

        CalcTics();
    };

    return !finished;
}
