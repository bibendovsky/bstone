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
// Former MARKHACK.ASM
//


#include "3d_def.h"


extern Sint16 viewwidth;
extern Sint16 viewheight;
extern int bufferofs;
extern Uint16 ylookup[MAXSCANLINES];
extern Uint16 centery;
extern Uint16 bufx;
extern Uint16 postheight;
extern const Uint8* shadingtable;
extern int post_planes;


enum DrawMode {
    DRAW_DEFAULT,
    DRAW_LIGHTED
}; // enum DrawMode


static void generic_draw_post(DrawMode draw_mode)
{
    if (postheight == 0)
        return;

    Uint8 pixel;
    Uint8 pixel_index;

    int cur_step = (32L * 65536L) / postheight;

    int step = cur_step;
    cur_step /= 2;

    int fraction = vga_width;

    int screen_column = vl_get_offset(bufferofs, 0, centery - 1) + postx;

    int n = postheight;

    if (postheight > (centery * vga_scale))
        n = centery;

    for (int h = 0; h < n; ++h) {
        // top half

        pixel_index = postsource[31 - (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        vga_memory[screen_column] = pixel;


        // bottom half

        pixel_index = postsource[32 + (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        vga_memory[screen_column + fraction] = pixel;

        screen_column -= vga_width;
        fraction += 2 * vga_width;
        cur_step += step;
    }
}


//
// Draws an unmasked post centered in the viewport
//

void DrawPost()
{
    generic_draw_post(DRAW_DEFAULT);
}


//
// Draws an unmasked light sourced post centered in the viewport
//

void DrawLSPost()
{
    generic_draw_post(DRAW_LIGHTED);
}
