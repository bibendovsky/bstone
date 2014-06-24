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
// Former D3_DR_A2.ASM
//
// To emulate D3_DR_A.ASM define W3D.
//


#include "3d_def.h"


static const Sint16 DEG90 = 900;
static const Sint16 DEG180 = 1800;
static const Sint16 DEG270 = 2700;
static const Sint16 DEG360 = 3600;


extern Sint32 finetangent[FINEANGLES / 4];
void HitHorizWall();
void HitVertWall();
void HitHorizDoor();
void HitVertDoor();
void HitHorizPWall();
void HitVertPWall();


extern Sint16 viewwidth;
extern Uint8 tilemap[MAPSIZE][MAPSIZE];
extern Uint8 spotvis[MAPSIZE][MAPSIZE];
extern int* pixelangle;
extern Sint16 midangle;
extern Sint16 focaltx;
extern Sint16 focalty;
extern Sint16 viewtx;
extern Sint16 viewty;
extern fixed viewx;
extern fixed viewy;
extern Uint16 xpartialup;
extern Uint16 xpartialdown;
extern Uint16 ypartialup;
extern Uint16 ypartialdown;
extern Uint16 tilehit;
extern Uint16 pixx;
extern Uint16* wallheight;
extern Sint16 xtile;
extern Sint16 ytile;
extern Sint16 xtilestep;
extern Sint16 ytilestep;
extern Sint32 xintercept;
extern Sint32 yintercept;
extern Sint32 xstep;
extern Sint32 ystep;
extern Uint16 doorposition[MAXDOORS];
extern Uint16 pwallpos;


static Sint32 partial_by_step(
    Sint32 step,
    Uint16 partial)
{
    Sint32 fracs = (((Uint32)step & 0xFFFF) * partial) >> 16;
    Sint32 ints = (step >> 16) * partial;
    return ints + fracs + (step < 0 ? 1 : 0);
}


void AsmRefresh()
{
    Sint16 angle;

    Sint16 h_op;
    Sint16 v_op;
    Sint16 xpartial;
    Sint16 ypartial;
    Uint16 doorpos;
    Sint32 intercept;
    Sint16 intercept_h; // high part of intercept
    Uint16 intercept_l; // low part of intercept
    Sint16 xs; // temporary xspot
    Sint16 xt; // temporary xtile
    Sint32 xint; // temporary xintercept
    Sint16 xint_h; // high part of temporary xintercept
    Sint16 ys; // temporary yspot
    Sint16 yt; // temporary ytile
    Sint32 yint; // temporary yintercept
    Sint16 yint_h; // high part of temporary yintercept

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
    yint_h = (Sint16)(yintercept >> 16);

    xtile = focaltx + xtilestep;
    xs = (xtile << 6) + yint_h;

    xint = viewx + partial_by_step(xstep, ypartial);
    xint_h = (Sint16)(xint >> 16);
    xintercept &= 0xFFFF0000;
    xintercept |= xint & 0xFFFF;

    yt = focalty + ytilestep;
    ys = (xint_h << 6) + yt;

    xt = xtile;
    yint_h = (Sint16)(yintercept >> 16);


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

    if (((Uint8*)tilemap)[xs] != 0) {
        tilehit &= 0xFF00;
        tilehit |= ((Uint8*)tilemap)[xs];

        if ((tilehit & 0x80) != 0) {
            //
            // hit a special vertical wall, so find which coordinate a
            // door would be intersected at, and check to see if the
            // door is open past that point
            //

            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (Sint32)yint_h << 16;

            if ((tilehit & 0x40) != 0) {
                //
                // hit a sliding vertical wall
                //

                intercept = yintercept + ((ystep * (Sint32)pwallpos) >> 6);
                intercept_l = (Uint16)(yintercept & 0xFFFF);
                intercept_h = (Sint16)(yintercept >> 16);

                if ((Sint16)(yintercept >> 16) == intercept_h) {
                    //
                    // draw the pushable wall at the new height
                    //

                    yintercept = intercept;
                    xt = xtile;
                    xintercept = (Sint32)xt << 16;

                    HitVertPWall();

                    goto nextpix;
                }
            } else {
                doorpos = tilehit & 0x7F;

                intercept = yintercept + (ystep >> 1);
                intercept_l = (Uint16)(intercept & 0xFFFF);
                intercept_h = (Sint16)(intercept >> 16);

                if ((Sint16)(yintercept >> 16) == intercept_h) {
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

                        xintercept = ((Sint32)xtile << 16) | 0x8000;

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
            yint_h = (Sint16)(yintercept >> 16);
        } else {
            xintercept = (Sint32)xt << 16;
            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (Sint32)yint_h << 16;
            ytile = yint_h;

            HitVertWall();

            goto nextpix;
        }
    }

    ((Uint8*)spotvis)[xs] = 1;

    xt += xtilestep;

    yint = (yintercept & 0xFFFF) + (ystep & 0xFFFF);
    yintercept &= 0xFFFF0000;
    yintercept |= yint & 0xFFFF;

    yint_h += (Sint16)((ystep >> 16) + (yint >> 16));

    xs = (xt << 6) + yint_h;

    goto vertcheck;



    //
    // check intersections with horizontal walls
    //

horizcheck:

    if ((h_op > 0 && xint_h >= xt) || (h_op < 0 && xint_h <= xt))
        goto vertentry;

horizentry:

    if (((Uint8*)tilemap)[ys] != 0) {
        tilehit &= 0xFF00;
        tilehit |= ((Uint8*)tilemap)[ys];

        if ((tilehit & 0x80) != 0) {
            //
            // hit a special horizontal wall, so find which coordinate a
            // door would be intersected at, and check to see if the
            // door is open past that point
            //

            xtile = xt;

            yintercept &= 0xFFFF;
            yintercept |= (Sint32)yint_h << 16;

            if ((tilehit & 0x40) != 0) {
                //
                // hit a sliding horizontal wall
                //

                intercept = (xstep * (Sint32)pwallpos) >> 6;
                xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
                intercept_l = (Uint16)(xint & 0xFFFF);
                intercept_h = (Sint16)((intercept >> 16) + xint_h +
                    (xint >> 16));

                if (xint_h == intercept_h) {
                    //
                    // draw the pushable wall at the new height
                    //

                    xintercept = ((Sint32)intercept_h << 16) | intercept_l;
                    yintercept = (Sint32)yt << 16;

                    HitHorizPWall();

                    goto nextpix;
                }
            } else {
                doorpos = tilehit & 0x7F;

                intercept = xstep >> 1;
                xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
                intercept_l = (Uint16)(xint & 0xFFFF);
                intercept_h = (Sint16)((intercept >> 16) + xint_h +
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

                        xintercept = ((Sint32)xint_h << 16) | intercept_l;
                        yintercept = ((Sint32)yt << 16) | 0x8000;

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
            yint_h = (Sint16)(yintercept >> 16);
        } else {
            xintercept &= 0xFFFF;
            xintercept |= (Sint32)xint_h << 16;
            xtile = xint_h;

            yintercept = (Sint32)yt << 16;
            ytile = yt;

            HitHorizWall();

            goto nextpix;
        }
    }

    ((Uint8*)spotvis)[ys] = 1;

    yt += ytilestep;

    xint = (xintercept & 0xFFFF) + (xstep & 0xFFFF);
    xintercept &= 0xFFFF0000;
    xintercept |= xint & 0xFFFF;
    xint_h += (Sint16)((xstep >> 16) + (xint >> 16));

    ys = (xint_h << 6) + yt;

    goto horizcheck;


    //
    // next pixel over
    //

nextpix:
    ++pixx;

    if (pixx < (viewwidth * vga_scale))
        goto pixxloop;
}
