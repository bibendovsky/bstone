//
// Former SCALE.ASM
//


#include "3d_def.h"
#include "id_vl.h"


extern longword dc_iscale;
extern longword dc_frac;
extern unsigned dc_source;
extern unsigned dc_seg;
extern unsigned dc_length;
extern unsigned dc_dest;

extern byte* shadingtable;


typedef enum {
    DRAW_DEFAULT,
    DRAW_LIGHTED,
} DrawMode;


static void generic_draw_column(DrawMode draw_mode)
{
    unsigned i;
    byte pixel;
    long fraction = dc_frac;
    byte* source = MK_FP(dc_seg, dc_source);
    unsigned screen_offset = dc_dest;
    byte* screen = MK_FP(0xA000, 0);

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
    unsigned i;
    unsigned screen_offset = dc_dest;
    byte* screen = MK_FP(0xA000, 0);

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
