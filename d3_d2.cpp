//
// Former D3_DASM2.ASM.
//


#include "SDL.h"


typedef enum {
    DO_CEILING,
    DO_FLOORING,
    DO_CEILING_AND_FLOORING,
} DrawOptions;

typedef enum {
    SO_NONE,
    SO_DEFAULT,
} ShadingOptions;


extern Sint16 mr_rowofs;
extern Sint16 mr_count;
extern Sint16 mr_xstep;
extern Sint16 mr_ystep;
extern Sint16 mr_xfrac;
extern Sint16 mr_yfrac;
extern int mr_dest;
extern int mr_plane;

extern const Uint8* shadingtable;
extern Uint8* vga_memory;


Uint8 planepics[8192]; // 4k of ceiling, 4k of floor


static void generic_map_row(
    DrawOptions draw_options,
    ShadingOptions shading_options)
{
    Sint16 i;
    int dest;
    Sint16 count;
    Sint16 rowofs;
    Uint32 xy;
    Uint32 xy_step;
    Uint32 xy_frac;
    Uint16 pics_index;
    Uint8 ceiling_index;
    Uint8 flooring_index;
    Uint8* screen;
    int screen_offset;

    rowofs = mr_rowofs;
    count = mr_count;

    xy_step = ((Uint32)mr_ystep) << 16;
    xy_step |= ((Uint32)mr_xstep) & 0xFFFF;

    xy_frac = ((Uint32)mr_yfrac) << 16;
    xy_frac |= ((Uint32)mr_xfrac) & 0xFFFF;

    dest = mr_dest;

    screen = vga_memory;

    for (i = 0; i < count; ++i) {
        xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

        xy_frac += xy_step;

        pics_index = xy & 0xFFFF;

        if (draw_options == DO_CEILING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            ceiling_index = planepics[pics_index + 0];
            screen_offset = (4 * dest) + mr_plane;

            if (shading_options == SO_DEFAULT)
                screen[screen_offset] = shadingtable[ceiling_index];
            else
                screen[screen_offset] = ceiling_index;
        }

        if (draw_options == DO_FLOORING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            flooring_index = planepics[pics_index + 1];
            screen_offset = (4 * (dest + rowofs)) + mr_plane;

            if (shading_options == SO_DEFAULT)
                screen[screen_offset] = shadingtable[flooring_index];
            else
                screen[screen_offset] = flooring_index;
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
