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


//
// Former SCALE.ASM
//


#include "3d_def.h"


extern int dc_iscale;
extern int dc_frac;
extern int dc_source;
extern uint8_t* dc_seg;
extern int dc_length;
extern int dc_x;
extern int dc_y;
extern int dc_dy;

extern const uint8_t* shadingtable;


enum DrawMode {
    DRAW_DEFAULT,
    DRAW_LIGHTED
}; // DrawMode


static void generic_draw_column(
    DrawMode draw_mode)
{
    int fraction = dc_frac;

    uint8_t* source = dc_seg + dc_source;
    const int base_offset = vl_get_offset(bufferofs) + dc_x;

    for (int i = 0; i < dc_length; ++i)
    {
        uint8_t pixel = 0;
        uint8_t pixel_index = source[fraction >> 16];

        if (draw_mode == DRAW_LIGHTED)
        {
            pixel = shadingtable[pixel_index];
        }
        else
        {
            pixel = pixel_index;
        }

        if (::vid_is_3d)
        {
            const int offset = base_offset + ((dc_y + dc_dy + i) * vga_width);
            vga_memory[offset] = pixel;
        }
        else
        {
            ::VL_Plot(
                ::dc_x,
                ::dc_y + ::dc_dy + i,
                pixel);
        }

        fraction += dc_iscale;
    }
}

void R_DrawColumn()
{
    generic_draw_column(DRAW_DEFAULT);
}

void R_DrawSLSColumn()
{
    int base_offset = vl_get_offset(bufferofs) + dc_x;

    for (int i = 0; i < dc_length; ++i) {
        int offset = base_offset + ((dc_y + dc_dy + i) * vga_width);
        uint8_t pixel_index = vga_memory[offset];
        uint8_t pixel = shadingtable[0x1000 | pixel_index];
        vga_memory[offset] = pixel;
    }
}

void R_DrawLSColumn()
{
    generic_draw_column(DRAW_LIGHTED);
}
