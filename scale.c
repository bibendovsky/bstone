//
// Former SCALE.ASM
//


#include "3d_def.h"
#include "id_vl.h"


extern Uint32 dc_iscale;
extern Uint32 dc_frac;
extern Uint16 dc_source;
extern Uint8* dc_seg;
extern Uint16 dc_length;
extern Uint16 dc_dest;
extern int dc_plane;

extern Uint8* shadingtable;


typedef enum {
    DRAW_DEFAULT,
    DRAW_LIGHTED,
} DrawMode;


static void generic_draw_column(DrawMode draw_mode)
{
    Uint16 i;
    Uint8 pixel;
    Sint32 fraction = dc_frac;

    Uint8* source = dc_seg + dc_source;
    int screen_offset = dc_dest;
    Uint8* screen = vga_memory;

    for (i = 0; i < dc_length; ++i) {
        Uint8 pixel_index = source[fraction >> 16];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        screen[(4 * screen_offset) + dc_plane] = pixel;

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
    Uint16 i;
    Uint16 screen_offset = dc_dest;
    Uint8* screen = vga_memory;

    for (i = 0; i < dc_length; ++i) {
        int offset = (4 * screen_offset) + dc_plane;
        Uint8 pixel_index = screen[offset];
        Uint8 pixel = shadingtable[0x1000 | pixel_index];
        screen[offset] = pixel;
        screen_offset += SCREENWIDTH;
    }
}

void R_DrawLSColumn()
{
    generic_draw_column(DRAW_LIGHTED);
}
