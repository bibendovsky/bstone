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

extern const uint8_t* shadingtable;
extern uint8_t* vga_memory;


uint8_t planepics[8192]; // 4k of ceiling, 4k of floor


static void generic_map_row(
    DrawOptions draw_options,
    ShadingOptions shading_options)
{
    int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
    int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

    int screen_offset = mr_dest;

    for (int i = 0; i < mr_count; ++i) {
        int xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

        int pics_index = xy & 0xFFFF;

        if (draw_options == DO_CEILING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            uint8_t ceiling_index = planepics[pics_index + 0];

            uint8_t ceiling_pixel =
                (shading_options == SO_DEFAULT) ?
                shadingtable[ceiling_index] :
                ceiling_index;

            vga_memory[screen_offset] = ceiling_pixel;
        }

        if (draw_options == DO_FLOORING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            uint8_t flooring_index = planepics[pics_index + 1];

            uint8_t flooring_pixel =
                (shading_options == SO_DEFAULT) ?
                shadingtable[flooring_index] :
                flooring_index;

            vga_memory[screen_offset + mr_rowofs] = flooring_pixel;
        }

        ++screen_offset;
        xy_frac += xy_step;
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
