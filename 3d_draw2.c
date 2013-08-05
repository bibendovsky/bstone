// WOLFHACK.C

#include "3D_DEF.H"

#define	MAXVIEWHEIGHT	200
#define  GAMESTATE_TEST 	(true)


unsigned CeilingTile=126, FloorTile=126;

void (*MapRowPtr)();

int  far	spanstart[MAXVIEWHEIGHT/2];		// jtr - far

fixed	stepscale[MAXVIEWHEIGHT/2];
fixed	basedist[MAXVIEWHEIGHT/2];

extern char	far	planepics[8192];	// 4k of ceiling, 4k of floor

int		halfheight = 0;

byte	far *planeylookup[MAXVIEWHEIGHT/2];
unsigned	far mirrorofs[MAXVIEWHEIGHT/2];

fixed	psin, pcos;

fixed FixedMul (fixed a, fixed b)
{
	return (a>>8)*(b>>8);
}


int		mr_rowofs;
int		mr_count;
int		mr_xstep;
int		mr_ystep;
int		mr_xfrac;
int		mr_yfrac;
int		mr_dest;


/*
==============
=
= DrawSpans
=
= Height ranges from 0 (infinity) to viewheight/2 (nearest)
==============
*/
extern byte far * lightsource;
extern byte far * shadingtable;


void DrawSpans (int x1, int x2, int height)
{
	fixed		length;
	int			ofs;
	int			prestep;
	fixed		startxfrac, startyfrac;

	int			x, startx, count, plane, startplane;
	byte		far	*toprow, far *dest;
	long i;

	toprow = planeylookup[height]+bufferofs;
	mr_rowofs = mirrorofs[height];

	mr_xstep = (psin<<1)/height;
	mr_ystep = (pcos<<1)/height;

	length = basedist[height];
	startxfrac = (viewx + FixedMul(length,pcos));
	startyfrac = (viewy - FixedMul(length,psin));

// draw two spans simultaniously

	if (gamestate.flags & GS_LIGHTING)
	{

	i=shade_max-(63l*(unsigned long)height/(unsigned long)normalshade);
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
		outportb (SC_INDEX+1,1<<plane);
		mr_xfrac = startxfrac - (mr_xstep>>2)*prestep;
		mr_yfrac = startyfrac - (mr_ystep>>2)*prestep;

		startx = x1>>2;
		mr_dest = (unsigned)toprow + startx;
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
		outportb (SC_INDEX+1,1<<plane);
		mr_xfrac = startxfrac - (mr_xstep>>2)*prestep;
		mr_yfrac = startyfrac - (mr_ystep>>2)*prestep;

		startx = x1>>2;
		mr_dest = (unsigned)toprow + startx;
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
	int		x,y;
	byte 	far *dest, far *src;

	halfheight = viewheight>>1;


	for (y=0 ; y<halfheight ; y++)
	{
		planeylookup[y] = (byte far *)0xa0000000l + (halfheight-1-y)*SCREENBWIDE;;
		mirrorofs[y] = (y*2+1)*SCREENBWIDE;

		stepscale[y] = y*GLOBAL1/32;
		if (y>0)
			basedist[y] = GLOBAL1/2*scale/y;
	}

	src = PM_GetPage(CeilingTile);
	dest = planepics;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}
	src = PM_GetPage(FloorTile);
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
	int		height, lastheight;
	int		x;

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

