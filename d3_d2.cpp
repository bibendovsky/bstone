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


//
// Former D3_DASM2.ASM.
//


#include "SDL.h"


enum DrawOptions {
    DO_CEILING,
    DO_FLOORING,
    DO_CEILING_AND_FLOORING
}; // enum DrawOptions

enum ShadingOptions {
    SO_NONE,
    SO_DEFAULT
}; // enum ShadingOptions


extern int mr_rowofs;
extern int mr_count;
extern int mr_xstep;
extern int mr_ystep;
extern int mr_xfrac;
extern int mr_yfrac;
extern int mr_dest;
extern int mr_plane;

extern const Uint8* shadingtable;
extern Uint8* vga_memory;


Uint8 planepics[8192]; // 4k of ceiling, 4k of floor


static void generic_map_row(
    DrawOptions draw_options,
    ShadingOptions shading_options)
{
    int rowofs = mr_rowofs;
    int count = mr_count;

    int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
    int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

    int dest = mr_dest;

    for (int i = 0; i < count; ++i) {
        int xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

        xy_frac += xy_step;

        int pics_index = xy & 0xFFFF;

        if (draw_options == DO_CEILING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            Uint8 ceiling_index = planepics[pics_index + 0];
            int screen_offset = (4 * dest) + mr_plane;

            if (shading_options == SO_DEFAULT)
                vga_memory[screen_offset] = shadingtable[ceiling_index];
            else
                vga_memory[screen_offset] = ceiling_index;
        }

        if (draw_options == DO_FLOORING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            Uint8 flooring_index = planepics[pics_index + 1];
            int screen_offset = (4 * (dest + rowofs)) + mr_plane;

            if (shading_options == SO_DEFAULT)
                vga_memory[screen_offset] = shadingtable[flooring_index];
            else
                vga_memory[screen_offset] = flooring_index;
        }

        ++dest;
    }
}

void MapLSRow()
{
    generic_map_row(DO_CEILING_AND_FLOORING, SO_DEFAULT);
}

void F_MapLSRow()
{
    generic_map_row(DO_FLOORING, SO_DEFAULT);
}

void C_MapLSRow()
{
    generic_map_row(DO_CEILING, SO_DEFAULT);
}

void MapRow()
{
    generic_map_row(DO_CEILING_AND_FLOORING, SO_NONE);
}

void F_MapRow()
{
    generic_map_row(DO_FLOORING, SO_NONE);
}

void C_MapRow()
{
    generic_map_row(DO_CEILING, SO_NONE);
}
