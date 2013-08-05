//
// Former D3_DASM2.ASM.
//


#include <dos.h>


typedef enum {
    DO_CEILING,
    DO_FLOORING,
    DO_CEILING_AND_FLOORING,
} DrawOptions;

typedef enum {
    SO_NONE,
    SO_DEFAULT,
} ShadingOptions;


extern int mr_rowofs;
extern int mr_count;
extern int mr_xstep;
extern int mr_ystep;
extern int mr_xfrac;
extern int mr_yfrac;
extern int mr_dest;

extern unsigned char* shadingtable;


unsigned char planepics[8192]; // 4k of ceiling, 4k of floor


static void generic_map_row(
    DrawOptions draw_options,
    ShadingOptions shading_options)
{
    int i;
    int dest;
    int count;
    int rowofs;
    unsigned long xy;
    unsigned long xy_step;
    unsigned long xy_frac;
    unsigned pics_index;
    unsigned char ceiling_index;
    unsigned char flooring_index;
    unsigned char* screen;

    rowofs = mr_rowofs;
    count = mr_count;

    xy_step = ((unsigned long)mr_ystep) << 16;
    xy_step |= ((unsigned long)mr_xstep) & 0xFFFF;

    xy_frac = ((unsigned long)mr_yfrac) << 16;
    xy_frac |= ((unsigned long)mr_xfrac) & 0xFFFF;

    dest = mr_dest;

    // FIXME
    //screen = MK_FP(0xA000, 0x0000);

    for (i = 0; i < count; ++i) {
        xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

        xy_frac += xy_step;

        pics_index = xy & 0xFFFF;

        if (draw_options == DO_CEILING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            ceiling_index = planepics[pics_index + 0];

            if (shading_options == SO_DEFAULT)
                screen[dest] = shadingtable[ceiling_index];
            else
                screen[dest] = ceiling_index;
        }

        if (draw_options == DO_FLOORING ||
            draw_options == DO_CEILING_AND_FLOORING)
        {
            flooring_index = planepics[pics_index + 1];

            if (shading_options == SO_DEFAULT)
                screen[dest + rowofs] = shadingtable[flooring_index];
            else
                screen[dest + rowofs] = flooring_index;
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
