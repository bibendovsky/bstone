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
// Former SCALE.ASM
//


#include "3d_def.h"


extern Uint32 dc_iscale;
extern Uint32 dc_frac;
extern Uint32 dc_source;
extern Uint8* dc_seg;
extern Uint32 dc_length;
extern Sint32 dc_x;
extern Sint32 dc_y;
extern Sint32 dc_dy;

extern const Uint8* shadingtable;


enum DrawMode {
    DRAW_DEFAULT,
    DRAW_LIGHTED
}; // enum DrawMode


static void generic_draw_column(DrawMode draw_mode)
{
    unsigned int fraction = dc_frac;

    Uint8* source = dc_seg + dc_source;
    int base_offset = vl_get_offset(bufferofs) + dc_x;

    for (unsigned int i = 0; i < dc_length; ++i) {
        Uint8 pixel;
        Uint8 pixel_index = source[fraction >> 16];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        int offset = base_offset + ((dc_y + dc_dy + i) * vga_width);
        vga_memory[offset] = pixel;

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

    for (unsigned int i = 0; i < dc_length; ++i) {
        int offset = base_offset + ((dc_y + dc_dy + i) * vga_width);
        Uint8 pixel_index = vga_memory[offset];
        Uint8 pixel = shadingtable[0x1000 | pixel_index];
        vga_memory[offset] = pixel;
    }
}

void R_DrawLSColumn()
{
    generic_draw_column(DRAW_LIGHTED);
}
