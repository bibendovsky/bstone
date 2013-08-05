//
// Former MARKHACK.ASM
//


#include "3d_def.h"
#include "id_vl.h"


extern int viewwidth;
extern int viewheight;
extern unsigned bufferofs;
extern unsigned ylookup[MAXSCANLINES];
extern unsigned centery;
extern unsigned bufx;
extern unsigned postheight;
byte far* shadingtable;


typedef enum {
    DRAW_DEFAULT,
    DRAW_LIGHTED,
} DrawMode;


static void generic_draw_post(DrawMode draw_mode)
{
    long step;
    long cur_step;

    unsigned i;
    unsigned n;
    unsigned fraction;
    unsigned screen_column;
    unsigned char pixel;
    unsigned char pixel_index;

    byte far* screen;

    if (postheight == 0)
        return;

    cur_step = (32L * 65536L) / postheight;

    step = cur_step;
    cur_step >>= 1;

    screen = MK_FP(0xA000, 0);

    fraction = SCREENBWIDE;
    
    screen_column = bufferofs + bufx + ylookup[centery] - SCREENBWIDE;

    n = postheight;

    if (postheight > centery)
        n = centery;

    for (i = 0; i < n; ++i) {
        // top half

        pixel_index = ((byte far*)postsource)[31 - (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        screen[screen_column] = pixel;


        // bottom half

        pixel_index = ((byte far*)postsource)[32 + (cur_step >> 16)];

        if (draw_mode == DRAW_LIGHTED)
            pixel = shadingtable[pixel_index];
        else
            pixel = pixel_index;

        screen[screen_column + fraction] = pixel;

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
