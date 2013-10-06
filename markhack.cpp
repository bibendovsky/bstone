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


typedef enum {
    DRAW_DEFAULT,
    DRAW_LIGHTED
} DrawMode;


static void generic_draw_post(DrawMode draw_mode)
{
    Sint32 step;
    Sint32 cur_step;

    Uint16 i;
    int j;
    Uint16 n;
    Uint16 fraction;
    Uint16 screen_column;
    Uint8 pixel;
    Uint8 pixel_index;
    int column_offset;

    Uint8* screen;

    if (postheight == 0)
        return;

    cur_step = (32L * 65536L) / postheight;

    step = cur_step;
    cur_step >>= 1;

    screen = vga_memory;

    fraction = SCREENBWIDE;

    screen_column = static_cast<Uint16>(bufferofs + bufx + ylookup[centery] - SCREENBWIDE);

    n = postheight;

    if (postheight > centery)
        n = centery;

    for (i = 0; i < n; ++i) {
        // top half

        pixel_index = postsource[31 - (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        column_offset = 4 * screen_column;

        for (j = 0; j < 4; ++j) {
            if ((post_planes & (1 << j)) != 0)
                screen[column_offset + j] = pixel;
        }


        // bottom half

        pixel_index = postsource[32 + (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        column_offset = 4 * (screen_column + fraction);

        for (j = 0; j < 4; ++j) {
            if ((post_planes & (1 << j)) != 0)
                screen[column_offset + j] = pixel;
        }

        screen_column -= SCREENBWIDE;
        fraction += 2 * SCREENBWIDE;
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
