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


// WOLFHACK.C

#include "3d_def.h"

#define	MAXVIEWHEIGHT	200
#define  GAMESTATE_TEST 	(1)


void MapLSRow();


Uint16 CeilingTile=126, FloorTile=126;

void (*MapRowPtr)();

int* spanstart;
int* stepscale;
int* basedist;
int* planeylookup;
int* mirrorofs;

extern Uint8 planepics[8192];	// 4k of ceiling, 4k of floor

int halfheight = 0;

fixed psin;
fixed pcos;

fixed FixedMul (fixed a, fixed b)
{
	return (a>>8)*(b>>8);
}


int mr_rowofs;
int mr_count;
int mr_xstep;
int mr_ystep;
int mr_xfrac;
int mr_yfrac;

int mr_dest = 0;

// BBi
int mr_plane = 0;


/*
==============
=
= DrawSpans
=
= Height ranges from 0 (infinity) to viewheight/2 (nearest)
==============
*/
extern const Uint8 * lightsource;
extern const Uint8 * shadingtable;


void DrawSpans(
    int x1,
    int x2,
    int height)
{
    fixed length;
    int prestep;
    fixed startxfrac;
    fixed startyfrac;
    int startx;
    int plane;
    int startplane;
    int toprow;
    int i;

    toprow = planeylookup[height] + bufferofs;
    mr_rowofs = mirrorofs[height];

    mr_xstep = (psin * 2) / height;
    mr_ystep = (pcos * 2) / height;

    length = basedist[height];
    startxfrac = (viewx + FixedMul(length, pcos));
    startyfrac = (viewy - FixedMul(length, psin));

    // draw two spans simultaniously

    if ((gamestate.flags & GS_LIGHTING) != 0) {
        i = shade_max - ((63 * height) / normalshade);

        if (i < 0)
            i = 0;
        else if (i > 63)
            i = 63;

        shadingtable = lightsource + (i * 256);
    }

    plane = startplane = x1 & 3;
    prestep = (viewwidth / 2) - x1;

    do {
        mr_plane = plane;

        mr_xfrac = startxfrac - (mr_xstep / 4) * prestep;
        mr_yfrac = startyfrac - (mr_ystep / 4) * prestep;

        startx = x1 / 4;
        mr_dest = toprow + startx;
        mr_count = ((x2 - plane) / 4) - startx + 1;
        x1++;
        prestep--;

#if GAMESTATE_TEST
        if (mr_count > 0)
            MapRowPtr();
#else
        if (mr_count > 0) {
            if ((gamestate.flags & GS_LIGHTING) != 0)
                MapLSRow();
            else
                MapRow();
        }
#endif

        plane = (plane + 1) & 3;
    } while (plane != startplane);
}




/*
===================
=
= SetPlaneViewSize
=
===================
*/

void SetPlaneViewSize (void)
{
    const Uint8* src;
    Uint8* dest;

    halfheight = (viewheight * vga_scale) / 2;

    for (int y = 0; y < halfheight; y++) {
        planeylookup[y] = (halfheight - 1 - y) * SCREENBWIDE;
        mirrorofs[y] = (y * 2 + 1) * SCREENBWIDE;
        stepscale[y] = y * GLOBAL1 / 32;

        if (y > 0)
            basedist[y] = GLOBAL1 / 2 * scale / y;
    }

    src = static_cast<const Uint8*>(PM_GetPage(CeilingTile));
    dest = planepics;

    for (int x = 0; x < 4096; x++) {
        *dest = *src++;
        dest += 2;
    }

    src = static_cast<const Uint8*>(PM_GetPage(FloorTile));
    dest = planepics + 1;

    for (int x = 0; x < 4096; x++) {
        *dest = *src++;
        dest += 2;
    }
}


/*
===================
=
= DrawPlanes
=
===================
*/

void DrawPlanes()
{
    int height;
    int lastheight;
    int x;

#if IN_DEVELOPMENT
    if (!MapRowPtr)
        DRAW2_ERROR(NULL_FUNC_PTR_PASSED);
#endif

    if ((viewheight / 2) != halfheight)
        SetPlaneViewSize(); // screen size has changed

    psin = viewsin;

    if (psin < 0)
        psin = -(psin & 0xFFFF);

    pcos = viewcos;

    if (pcos < 0)
        pcos = -(pcos & 0xFFFF);

    //
    // loop over all columns
    //
    lastheight = halfheight;

    for (x = 0; x < viewwidth; ++x) {
        height = wallheight[x] / 8;

        if (height < lastheight) { // more starts
            do {
                spanstart[--lastheight] = x;
            } while (lastheight > height);
        } else if (height > lastheight) { // draw spans
            if (height > halfheight)
                height = halfheight;

            for ( ; lastheight < height; ++lastheight)
                if (lastheight > 0)
                    DrawSpans(spanstart[lastheight], x - 1, lastheight);
        }
    }

    height = halfheight;

    for ( ; lastheight < height; ++lastheight) {
        if (lastheight > 0)
            DrawSpans(spanstart[lastheight], x - 1, lastheight);
    }
}

