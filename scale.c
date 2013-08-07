//
// Former SCALE.ASM
//


#include "3d_def.h"
#include "id_vl.h"


extern longword dc_iscale;
extern longword dc_frac;
extern unsigned short dc_source;
extern unsigned short dc_seg;
extern unsigned short dc_length;
extern unsigned short dc_dest;

extern byte* shadingtable;


typedef enum {
    DRAW_DEFAULT,
    DRAW_LIGHTED,
} DrawMode;


static void generic_draw_column(DrawMode draw_mode)
{
    unsigned short i;
    byte pixel;
    long fraction = dc_frac;

    // FIXME
    byte* source = (byte*)dc_seg + dc_source;

    unsigned short screen_offset = dc_dest;

    // FIXME
    byte* screen = (byte*)0xA0000;

    for (i = 0; i < dc_length; ++i) {
        byte pixel_index = source[fraction >> 16];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        screen[screen_offset] = pixel;

        screen_offset += SCREENWIDTH;
        fraction += dc_iscale;
    }
}

void R_DrawColumn()
{
    generic_draw_column(DRAW_DEFAULT);
}

void R_DrawSLSColumn()
{
    unsigned short i;
    unsigned short screen_offset = dc_dest;
    byte* screen = (byte*)0xA0000;

    for (i = 0; i < dc_length; ++i) {
        byte pixel_index = screen[screen_offset];
        byte pixel = shadingtable[0x1000 | pixel_index];
        screen[screen_offset] = pixel;
        screen_offset += SCREENWIDTH;
    }
}

void R_DrawLSColumn()
{
    generic_draw_column(DRAW_LIGHTED);
}
