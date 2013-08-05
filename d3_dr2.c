//
// Former D3_DR_A2.ASM
//
// To emulate D3_DR_A.ASM define W3D.
//


#include <dos.h>

#include "3d_def.h"


static const int DEG90 = 900;
static const int DEG180 = 1800;
static const int DEG270 = 2700;
static const int DEG360 = 3600;


extern long far finetangent[FINEANGLES / 4];
void HitHorizWall();
void HitVertWall();
void HitHorizDoor();
void HitVertDoor();
void HitHorizPWall();
void HitVertPWall();


extern int viewwidth;
extern byte tilemap[MAPSIZE][MAPSIZE];
extern byte spotvis[MAPSIZE][MAPSIZE];
extern int pixelangle[MAXVIEWWIDTH];
extern int midangle;
extern int focaltx;
extern int focalty;
extern int viewtx;
extern int viewty;
extern fixed viewx;
extern fixed viewy;
extern unsigned xpartialup;
extern unsigned xpartialdown;
extern unsigned ypartialup;
extern unsigned ypartialdown;
extern unsigned tilehit;
extern unsigned pixx;
extern unsigned wallheight[MAXVIEWWIDTH];
extern int xtile;
extern int ytile;
extern int xtilestep;
extern int ytilestep;
extern long xintercept;
extern long yintercept;
extern long xstep;
extern long ystep;
extern unsigned doorposition[MAXDOORS];
extern unsigned pwallpos;


static long partial_by_step(
    long step,
    unsigned partial)
{
    long fracs = (((unsigned long)step & 0xFFFF) * partial) >> 16;
    long ints = (step >> 16) * partial;
    return ints + fracs + (step < 0 ? 1 : 0);
}


void AsmRefresh()
{
    int angle;

    int h_op;
    int v_op;
    int xpartial;
    int ypartial;
    unsigned doorpos;
    long intercept;
    int intercept_h;
    unsigned intercept_l;
    int xs; // temporary xspot
    int xt; // temporary xtile
    long xint; // temporary xintercept
    int xint_h; // high word of temporary xintercept
    int ys; // temporary yspot
    int yt; // temporary ytile
    long yint; // temporary yintercept
    int yint_h; // high word of temporary yintercept

    pixx = 0;

pixxloop:

    angle = midangle + pixelangle[pixx];

    while (angle < 0)
        angle += FINEANGLES;

    while (angle >= DEG360)
        angle -= FINEANGLES;

    if (angle >= DEG270) {
        xtilestep = 1;
        ytilestep = 1;

        h_op = 1;
        v_op = 1;

        xstep = finetangent[angle - DEG270];
        ystep = finetangent[DEG360 - 1 - angle];

        xpartial = xpartialup;
        ypartial = ypartialup;
    } else if (angle >= DEG180) {
        xtilestep = -1;
        ytilestep = 1;

        h_op = -1;
        v_op = 1;

        xstep = -finetangent[DEG270 - 1 - angle];
        ystep = finetangent[angle - DEG180];

        xpartial = xpartialdown;
        ypartial = ypartialup;
    } else if (angle >= DEG90) {
        xtilestep = -1;
        ytilestep = -1;

        h_op = -1;
        v_op = -1;

        xstep = -finetangent[angle - DEG90];
        ystep = -finetangent[DEG180 - 1 - angle];

        xpartial = xpartialdown;
        ypartial = ypartialdown;
    } else {
        xtilestep = 1;
        ytilestep = -1;

        h_op = 1;
        v_op = -1;

        xstep = finetangent[DEG90 - 1 - angle];
        ystep = -finetangent[angle];

        xpartial = xpartialup;
        ypartial = ypartialdown;
    }


    //
    // initialise variables for intersection testing
    //

    yintercept = viewy + partial_by_step(ystep, xpartial);
    yint_h = (int)(yintercept >> 16);

    xtile = focaltx + xtilestep;
    xs = (xtile << 6) + yint_h;

    xint = viewx + partial_by_step(xstep, ypartial);
    xint_h = (int)(xint >> 16);
    xintercept &= 0xFFFF0000;
    xintercept |= xint & 0xFFFF;

    yt = focalty + ytilestep;
    ys = (xint_h << 6) + yt;

    xt = xtile;
    yint_h = (int)(yintercept >> 16);


    //
    // trace along this angle until we hit a wall
    //
    // CORE LOOP!
    //

    //
    // check intersections with vertical walls
    //

vertcheck:

    if ((v_op < 0 && yint_h <= yt) || (v_op > 0 && yint_h >= yt))
        goto horizentry;

vertentry:

    if (((byte*)tilemap)[xs] != 0) {
        tilehit &= 0xFF00;
        tilehit |= ((byte*)tilemap)[xs];

        if ((tilehit & 0x80) != 0) {
            //
            // hit a special vertical wall, so find which coordinate a
            // door would be intersected at, and check to see if the
            // door is open past that point
            //

            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (long)yint_h << 16;

            if ((tilehit & 0x40) != 0) {
                //
                // hit a sliding vertical wall
                //

                intercept = yintercept + ((ystep * (long)pwallpos) >> 6);
                intercept_l = (unsigned)(yintercept & 0xFFFF);
                intercept_h = (int)(yintercept >> 16);

                if ((int)(yintercept >> 16) == intercept_h) {
                    //
                    // draw the pushable wall at the new height
                    //

                    yintercept = intercept;
                    xt = xtile;
                    xintercept = (long)xt << 16;

                    HitVertPWall();

                    goto nextpix;
                }
            } else {
                doorpos = tilehit & 0x7F;

                intercept = yintercept + (ystep >> 1);
                intercept_l = (unsigned)(intercept & 0xFFFF);
                intercept_h = (int)(intercept >> 16);

                if ((int)(yintercept >> 16) == intercept_h) {
                    //
                    // the trace hit the door plane at pixel position,
                    // see if the door is closed that much
                    //
                    // draw the door
                    //

                    boolean skip_draw = false;

#ifndef W3D
                    if (intercept_l < 0x8000) {
                        if (intercept_l >
                            (0x7FFF - (doorposition[doorpos] >> 1)))
                        {
                            skip_draw = true;
                        }
                    } else {
                        if (intercept_l <
                            ((doorposition[doorpos] >> 1) + 0x8000))
                        {
                            skip_draw = true;
                        }
                    }
#else
                    if (intercept_l < doorposition[doorpos])
                        skip_draw = true;
#endif // W3D

                    if (!skip_draw) {
                        //
                        // draw the trek door
                        //

                        yintercept &= 0xFFFF0000;
                        yintercept |= intercept_l;

                        xintercept = ((long)xtile << 16) | 0x8000;

                        HitVertDoor();

                        goto nextpix;
                    }
                }
            }

            //
            // midpoint is outside tile,
            // so it hit the side of the wall before a door
            //

            xt = xtile;
            yint_h = (int)(yintercept >> 16);
        } else {
            xintercept = (long)xt << 16;    
            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (long)yint_h << 16;
            ytile = yint_h;

            HitVertWall();

            goto nextpix;
        }
    }

    ((byte*)spotvis)[xs] = 1;

    xt += xtilestep;

    yint = (yintercept & 0xFFFF) + (ystep & 0xFFFF);
    yintercept &= 0xFFFF0000;
    yintercept |= yint & 0xFFFF;

    yint_h += (int)((ystep >> 16) + (yint >> 16));

    xs = (xt << 6) + yint_h;

    goto vertcheck;



    //
    // check intersections with horizontal walls
    //

horizcheck:

    if ((h_op > 0 && xint_h >= xt) || (h_op < 0 && xint_h <= xt))
        goto vertentry;

horizentry:

    if (((byte*)tilemap)[ys] != 0) {
        tilehit &= 0xFF00;
        tilehit |= ((byte*)tilemap)[ys];

        if ((tilehit & 0x80) != 0) {
            //
            // hit a special horizontal wall, so find which coordinate a
            // door would be intersected at, and check to see if the
            // door is open past that point
            //

            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (long)yint_h << 16;

            if ((tilehit & 0x40) != 0) {
                //
                // hit a sliding horizontal wall
                //

                intercept = (xstep * (long)pwallpos) >> 6;
                xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
                intercept_l = (unsigned)(xint & 0xFFFF);
                intercept_h = (int)((intercept >> 16) + xint_h +
                    (xint >> 16));

                if (xint_h == intercept_h) {
                    //
                    // draw the pushable wall at the new height
                    //

                    xintercept = ((long)intercept_h << 16) | intercept_l;
                    yintercept = (long)yt << 16;

                    HitHorizPWall();

                    goto nextpix;
                }
            } else {
                doorpos = tilehit & 0x7F;

                intercept = xstep >> 1;
                xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
                intercept_l = (unsigned)(xint & 0xFFFF);
                intercept_h = (int)((intercept >> 16) + xint_h +
                    (xint >> 16));

                if (xint_h == intercept_h) {
                    //
                    // the trace hit the door plane at pixel position,
                    // see if the door is closed that much
                    //
                    // draw the door
                    //

                    boolean skip_draw = false;

#ifndef W3D
                    if (intercept_l < 0x8000) {
                        if (intercept_l >
                            (0x7FFF - (doorposition[doorpos] >> 1)))
                        {
                            skip_draw = true;
                        }
                    } else {
                        if (intercept_l <
                            ((doorposition[doorpos] >> 1) + 0x8000))
                        {
                            skip_draw = true;
                        }
                    }
#else
                    if (intercept_l < doorposition[doorpos])
                        skip_draw = true;
#endif // W3D

                    if (!skip_draw) {
                        //
                        // draw the trek door
                        //

                        xintercept = ((long)xint_h << 16) | intercept_l;
                        yintercept = ((long)yt << 16) | 0x8000;

                        HitHorizDoor();

                        goto nextpix;
                    }
                }
            }


            //
            // midpoint is outside tile,
            // so it hit the side of the wall before a door
            //

            xt = xtile;
            yint_h = (int)(yintercept >> 16);
        } else {
            xintercept &= 0xFFFF;
            xintercept |= (long)xint_h << 16;
            xtile = xint_h;

            yintercept = (long)yt << 16;
            ytile = yt;

            HitHorizWall();

            goto nextpix;
        }
    }

    ((byte*)spotvis)[ys] = 1;

    yt += ytilestep;

    xint = (xintercept & 0xFFFF) + (xstep & 0xFFFF);
    xintercept &= 0xFFFF0000;
    xintercept |= xint & 0xFFFF;
    xint_h += (int)((xstep >> 16) + (xint >> 16));

    ys = (xint_h << 6) + yt;

    goto horizcheck;


    //
    // next pixel over
    //

nextpix:
    ++pixx;

    if (pixx < viewwidth)
        goto pixxloop;
}
