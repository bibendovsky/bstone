// WOLFHACK.C

#include "3d_def.h"

#define	MAXVIEWHEIGHT	200
#define  GAMESTATE_TEST 	(1)


void MapLSRow();


Uint16 CeilingTile=126, FloorTile=126;

void (*MapRowPtr)();

Sint16  spanstart[MAXVIEWHEIGHT/2];		// jtr - far

fixed	stepscale[MAXVIEWHEIGHT/2];
fixed	basedist[MAXVIEWHEIGHT/2];

extern Uint8 planepics[8192];	// 4k of ceiling, 4k of floor

Sint16		halfheight = 0;

// FIXME
#if 0
Uint8	*planeylookup[MAXVIEWHEIGHT/2];
#endif // 0
int planeylookup[MAXVIEWHEIGHT / 2];

Uint16	mirrorofs[MAXVIEWHEIGHT/2];

fixed	psin, pcos;

fixed FixedMul (fixed a, fixed b)
{
	return (a>>8)*(b>>8);
}


Sint16		mr_rowofs;
Sint16		mr_count;
Sint16		mr_xstep;
Sint16		mr_ystep;
Sint16		mr_xfrac;
Sint16		mr_yfrac;

// FIXME
#if 0
Sint16		mr_dest;
#endif // 0
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


void DrawSpans (Sint16 x1, Sint16 x2, Sint16 height)
{
	fixed		length;
	Sint16			prestep;
	fixed		startxfrac, startyfrac;

	Sint16			startx, plane, startplane;
// FIXME
#if 0
	Uint8		*toprow, *dest;
#endif
    int toprow;

	Sint32 i;

	toprow = planeylookup[height]+bufferofs;
	mr_rowofs = mirrorofs[height];

	mr_xstep = static_cast<Sint16>((psin<<1)/height);
	mr_ystep = static_cast<Sint16>((pcos<<1)/height);

	length = basedist[height];
	startxfrac = (viewx + FixedMul(length,pcos));
	startyfrac = (viewy - FixedMul(length,psin));

// draw two spans simultaniously

	if (gamestate.flags & GS_LIGHTING)
	{

	i=shade_max-(63l*(Uint32)height/(Uint32)normalshade);
	if (i<0)
		i=0;
   else
   	if (i>63)
      	i = 63;
	shadingtable=lightsource+(i<<8);
	plane = startplane = x1&3;
	prestep = viewwidth/2 - x1;
	do
	{
        // FIXME
#if 0
		outportb (SC_INDEX+1,1<<plane);
#endif // 0
        mr_plane = plane;

		mr_xfrac = static_cast<Sint16>(startxfrac - (mr_xstep>>2)*prestep);
		mr_yfrac = static_cast<Sint16>(startyfrac - (mr_ystep>>2)*prestep);

		startx = x1>>2;
		mr_dest = toprow + startx;
		mr_count = ((x2-plane)>>2) - startx + 1;
		x1++;
		prestep--;

#if GAMESTATE_TEST
		if (mr_count)
			MapRowPtr();
#else
		if (mr_count)
			MapLSRow ();
#endif

		plane = (plane+1)&3;
	} while (plane != startplane);
	}
	else
	{
	plane = startplane = x1&3;
	prestep = viewwidth/2 - x1;
	do
	{
        // FIXME
#if 0
		outportb (SC_INDEX+1,1<<plane);
#endif // 0
        mr_plane = plane;

		mr_xfrac = static_cast<Sint16>(startxfrac - (mr_xstep>>2)*prestep);
		mr_yfrac = static_cast<Sint16>(startyfrac - (mr_ystep>>2)*prestep);

		startx = x1>>2;
		mr_dest = toprow + startx;
		mr_count = ((x2-plane)>>2) - startx + 1;
		x1++;
		prestep--;

#if GAMESTATE_TEST
		if (mr_count)
			MapRowPtr();
#else
		if (mr_count)
			MapRow ();
#endif

		plane = (plane+1)&3;
	} while (plane != startplane);
	}
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
	Sint16		x,y;
	Uint8 	*dest, *src;

	halfheight = viewheight>>1;


	for (y=0 ; y<halfheight ; y++)
	{
        //FIXME
#if 0
		planeylookup[y] = (Uint8 *)0xa0000000l + (halfheight-1-y)*SCREENBWIDE;;
#endif // 0
        planeylookup[y] = (halfheight - 1 - y) * SCREENBWIDE;

		mirrorofs[y] = (y*2+1)*SCREENBWIDE;

		stepscale[y] = y*GLOBAL1/32;
		if (y>0)
			basedist[y] = GLOBAL1/2*scale/y;
	}

	src = static_cast<Uint8*>(PM_GetPage(CeilingTile));
	dest = planepics;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}
	src = static_cast<Uint8*>(PM_GetPage(FloorTile));
	dest = planepics+1;
	for (x=0 ; x<4096 ; x++)
	{
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

void DrawPlanes (void)
{
	Sint16		height, lastheight;
	Sint16		x;

#if IN_DEVELOPMENT
	if (!MapRowPtr)
   	DRAW2_ERROR(NULL_FUNC_PTR_PASSED);
#endif


	if (viewheight>>1 != halfheight)
		SetPlaneViewSize ();		// screen size has changed


	psin = viewsin;
	if (psin < 0)
		psin = -(psin&0xffff);
	pcos = viewcos;
	if (pcos < 0)
		pcos = -(pcos&0xffff);

//
// loop over all columns
//
	lastheight = halfheight;

	for (x=0 ; x<viewwidth ; x++)
	{
		height = wallheight[x]>>3;
		if (height < lastheight)
		{	// more starts
			do
			{
				spanstart[--lastheight] = x;
			} while (lastheight > height);
		}
		else if (height > lastheight)
		{	// draw spans
			if (height > halfheight)
				height = halfheight;
			for ( ; lastheight < height ; lastheight++)
         	if (lastheight>0)
					DrawSpans (spanstart[lastheight], x-1, lastheight);
		}
	}

	height = halfheight;
	for ( ; lastheight < height ; lastheight++)
     	if (lastheight>0)
			DrawSpans (spanstart[lastheight], x-1, lastheight);
}

