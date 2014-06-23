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


// 3D_SCALE.C

#include "3d_def.h"

#ifdef MSVC
#pragma hdrstop
#endif

void R_DrawSLSColumn();
void R_DrawLSColumn();
void R_DrawColumn();


#define OP_RETF	0xcb

#define CLOAKED_SHAPES			(1)

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

//t_compscale *scaledirectory[MAXSCALEHEIGHT+1];
//long			fullscalefarcall[MAXSCALEHEIGHT+1];

Sint16			maxscale,maxscaleshl2;
Uint16    centery;

Sint16 normalshade;
Sint16 normalshade_div = 1;
Sint16 shade_max = 1;

Sint16 nsd_table[] = { 1, 6, 3, 4, 1, 2};
Sint16 sm_table[] =  {36,51,62,63,18,52};


/*
=============================================================================

						  LOCALS

=============================================================================
*/

//t_compscale 	*work;
Uint16 BuildCompScale (Sint16 height, void**finalspot);

Sint16			stepbytwo;

//===========================================================================

#if 0
/*
==============
=
= BadScale
=
==============
*/

void BadScale (void)
{
	SCALE_ERROR(BADSCALE_ERROR);
}
#endif




/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (Sint16 maxscaleheight)
{
	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;
	normalshade=(3*(maxscale>>2))/normalshade_div;
	centery=viewheight>>1;
}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
=
= Calling the compiled scaler only destroys AL
=
========================
*/

#if 0
unsigned BuildCompScale (int height, memptr *finalspot)
{
	Uint8		*code;

	int			i;
	Sint32		fix,step;
	unsigned	src,totalscaled,totalsize;
	int			startpix,endpix,toppix;


	step = ((Sint32)height<<16) / 64;
	code = &work->code[0];
	toppix = (viewheight-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

		if (endpix>startpix)
			work->width[src] = endpix-startpix;
		else
			work->width[src] = 0;

//
// mark the start of the code
//
		work->codeofs[src] = FP_OFF(code);

//
// compile some code if the source pixel generates any screen pixels
//
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight || src == 64)
			continue;

	//
	// mov al,[si+src]
	//
		*code++ = 0x8a;
		*code++ = 0x44;
		*code++ = src;

		for (;startpix<endpix;startpix++)
		{
			if (startpix >= viewheight)
				break;						// off the bottom of the view area
			if (startpix < 0)
				continue;					// not into the view area

		//
		// mov [es:di+heightofs],al
		//
			*code++ = 0x26;
			*code++ = 0x88;
			*code++ = 0x85;
			*((unsigned *)code)++ = startpix*SCREENBWIDE;
		}

	}

//
// retf
//
	*code++ = 0xcb;

	totalsize = FP_OFF(code);
	MM_GetPtr (finalspot,totalsize);
	_fmemcpy ((Uint8 *)(*finalspot),(Uint8 *)work,totalsize);

	return totalsize;
}

#endif


// Draw Column vars

Uint32 dc_iscale;
Uint32 dc_frac;
Uint32 dc_source;
Uint8* dc_seg;
Uint32 dc_length;
Uint32 dc_dest;

// BBi
Sint32 dc_x;
Sint32 dc_y;
Sint32 dc_dy;

// An index of plane to draw in.
int dc_plane = 1;

#define SFRACUNIT 0x10000

extern Uint16 * linecmds;

extern boolean useBounceOffset;

fixed bounceOffset=0;

/*
=======================
=
= ScaleMaskedLSPost with Light sourcing
=
=======================
*/

void ScaleMaskedLSPost(int height, int buf)
{
    int bounce;

    if (useBounceOffset)
        bounce = bounceOffset;
    else
        bounce = 0;

    bounce *= vga_scale;

    const Uint16* srcpost = linecmds;
    dc_iscale = (64U * 65536U) / static_cast<unsigned int>(height);
    unsigned int screenstep = static_cast<unsigned int>(height) << 10;

    int sprtopoffset = ((viewheight * vga_scale) << 15) -
        (height << 15) + (bounce >> 1);

    int end = (bstone::Endian::le(*srcpost++)) / 2;

    while (end != 0) {
        dc_source = bstone::Endian::le(*srcpost++);

        int start = bstone::Endian::le(*srcpost++) / 2;

        dc_source += start;
        dc_source %= 65536;

        int length = end - start;
        int topscreen = sprtopoffset + static_cast<int>(screenstep * start);
        int bottomscreen = topscreen + static_cast<int>(screenstep * length);

        int dc_yl = (topscreen + SFRACUNIT - 1) >> 16;
        int dc_yh = (bottomscreen - 1) >> 16;

        if (dc_yh >= (viewheight * vga_scale))
            dc_yh = (viewheight * vga_scale) - 1;

        if (dc_yl < 0) {
            dc_frac = dc_iscale * (-dc_yl);
            dc_yl = 0;
        } else
            dc_frac = 0;

        if (dc_yl <= dc_yh) {
            dc_dy = dc_yl;
            dc_length = dc_yh - dc_yl + 1;
#if CLOAKED_SHAPES
            if (cloaked_shape)
                R_DrawSLSColumn();
            else
#endif
                R_DrawLSColumn();
        }

        end = bstone::Endian::le(*srcpost++) / 2;
    }
}

/*
=======================
=
= ScaleMaskedPost without Light sourcing
=
=======================
*/

void ScaleMaskedPost(Sint16 height, Uint16 buf)
{
    Sint16  length;
    Uint16 end;
    Uint16 start;
    Sint32 sprtopoffset;
    Sint32 topscreen;
    Sint32 bottomscreen;
    Uint32 screenstep;
    Sint32 dc_yl,dc_yh;
    Uint16 * srcpost;

    fixed bounce;

    if (useBounceOffset)
        bounce = bounceOffset;
    else
        bounce = 0;

    srcpost = linecmds;
    dc_iscale = (64U * 65536U) / (Uint32)height;
    screenstep = ((Uint32)height) << 10;

    sprtopoffset = ((Sint32)viewheight << 15) - ((Sint32)height << 15) + (bounce >> 1);

    end = bstone::Endian::le(*srcpost++) >> 1;

    for ( ; end != 0; ) {
        dc_source = bstone::Endian::le(*srcpost++);
        start = bstone::Endian::le(*srcpost++) >> 1;
        dc_source += start;
        dc_source %= 65536;
        length = end - start;
        topscreen = sprtopoffset + (Sint32)(screenstep * (Sint32)start);
        bottomscreen = topscreen + (Sint32)(screenstep * (Sint32)length);
        dc_yl = (topscreen + SFRACUNIT - 1) >> 16;
        dc_yh = (bottomscreen - 1) >> 16;

        if (dc_yh >= viewheight)
            dc_yh = viewheight - 1;

        if (dc_yl < 0) {
            dc_frac = dc_iscale * (-dc_yl);
            dc_yl = 0;
        } else
            dc_frac = 0;

        if (dc_yl <= dc_yh) {
            dc_dest = buf + ylookup[(Uint16)dc_yl];
            dc_length = (Uint16)(dc_yh - dc_yl + 1);
            R_DrawColumn();
        }

        end=bstone::Endian::le(*srcpost++) >> 1;
    }
}

/*
=======================
=
= ScaleLSShape with Light sourcing
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/
extern const Uint8 * shadingtable;
extern const Uint8 * lightsource;

void ScaleLSShape (Sint16 xcenter, Sint16 shapenum, Uint16 height, char lighting)
{
	t_compshape	*shape;
	Sint16      i;
	Uint32 frac;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Uint16 swidth;
	Sint32     xcent;

	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = (t_compshape*)PM_GetSpritePage(shapenum);

    dc_seg = (Uint8*)shape;

	xscale=(Uint32)height<<12;
	xcent=(Sint32)((Sint32)xcenter<<20)-((Sint32)height<<17)+0x80000;
//
// calculate edges of the shape
//
	x1 = (Sint16)((Sint32)(xcent+((Sint32)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (Sint16)((Sint32)(xcent+((Sint32)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(Uint32)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((Sint32)-x1)*(Sint32)screenscale;
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;

	i=shade_max-(63l*(Uint32)(height>>3)/(Uint32)normalshade)+lighting;

	if (i<0)
		i=0;
   else
  	if (i > 63)
   	i = 63;

	shadingtable=lightsource+(i<<8);
	swidth=shape->rightpix-shape->leftpix;

	for (; x1<=x2 ; x1++, frac += screenscale)
		{
		if (wallheight[x1]>height)
			continue;

        dc_plane = x1 & 3;

		texturecolumn=frac>>20;
		if (texturecolumn>swidth)
			texturecolumn=swidth;

        linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

		ScaleMaskedLSPost(height>>2,bufferofs+((Uint16)x1>>2));
		}
}




/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/
void ScaleShape (Sint16 xcenter, Sint16 shapenum, Uint16 height)
{
	t_compshape	*shape;
	Uint32 frac;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     xcent;
	Uint16 swidth;


	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = (t_compshape*)PM_GetSpritePage(shapenum);

    dc_seg = (Uint8*)shape;

	xscale=(Uint32)height<<12;
	xcent=(Sint32)((Sint32)xcenter<<20)-((Sint32)height<<(17))+0x80000;
//
// calculate edges of the shape
//
	x1 = (Sint16)((Sint32)(xcent+((Sint32)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (Sint16)((Sint32)(xcent+((Sint32)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(Uint32)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((Sint32)-x1)*(Sint32)screenscale;
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	swidth=shape->rightpix-shape->leftpix;

	for (; x1<=x2 ; x1++, frac += screenscale)
		{
		if (wallheight[x1]>height)
			continue;

        dc_plane = x1 & 3;

		texturecolumn=frac>>20;
		if (texturecolumn>swidth)
			texturecolumn=swidth;

        linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

		ScaleMaskedPost(height>>2,static_cast<Uint16>(bufferofs+((Uint16)x1>>2)));
		}
}



/*
=======================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void SimpleScaleShape (Sint16 xcenter, Sint16 shapenum, Uint16 height)
{
	t_compshape	*shape;
	Uint32 frac;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     xcent;
	Uint16 swidth;

	shape = (t_compshape*)PM_GetSpritePage (shapenum);

    dc_seg = (Uint8*)shape;

	xscale=(Uint32)height<<10;
	xcent=(Sint32)((Sint32)xcenter<<16)-((Sint32)height<<(15))+0x8000;
//
// calculate edges of the shape
//
	x1 = (Sint16)((Sint32)(xcent+((Sint32)shape->leftpix*xscale))>>16);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (Sint16)((Sint32)(xcent+((Sint32)shape->rightpix*xscale))>>16);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(64*65536)/(Uint32)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=screenscale*((Sint32)-x1);
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	swidth=shape->rightpix-shape->leftpix;

	for (; x1<=x2 ; x1++, frac += screenscale)
		{
        dc_plane = x1 & 3;

		texturecolumn=frac>>16;
		if (texturecolumn>swidth)
			texturecolumn=swidth;

        linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

		ScaleMaskedPost(height,static_cast<Uint16>(bufferofs+(x1>>2)));
		}
}

//-------------------------------------------------------------------------
// MegaSimpleScaleShape()
//
// NOTE: Parameter SHADE determines which Shade palette to use on the shape.
//       0 == NO Shading
//       63 == Max Shade (BLACK or near)
//-------------------------------------------------------------------------

void MegaSimpleScaleShape(
    int xcenter,
    int ycenter,
    int shapenum,
    int height,
    int shade)
{
    dc_y = 0;
    dc_y -= (viewheight - 64) / 2;
    dc_y += ycenter - 34;
    dc_y *= vga_scale;

    xcenter *= vga_scale;
    height *= vga_scale;

    t_compshape* shape =
        static_cast<t_compshape*>(PM_GetSpritePage(shapenum));

    dc_seg = reinterpret_cast<Uint8*>(shape);
    unsigned int xscale = static_cast<Uint32>(height) << 14;
    int xcent = (xcenter << 20) - (height << 19) + 0x80000;

    //
    // calculate edges of the shape
    //
    int x1 = static_cast<Sint32>(xcent + (shape->leftpix * xscale)) >> 20;

    if (x1 >= (viewwidth * vga_scale))
        return; // off the right side

    int x2 = static_cast<Sint32>(xcent + (shape->rightpix * xscale)) >> 20;

    if (x2 < 0)
        return; // off the left side

    unsigned int screenscale = (64UL << 20UL) / static_cast<Uint32>(height);

    //
    // Choose shade table.
    //
    shadingtable = lightsource + (shade << 8);

    //
    // store information in a vissprite
    //
    unsigned int frac;

    if (x1 < 0) {
        frac = screenscale * (-x1);
        x1 = 0;
    } else
        frac = screenscale >> 1;

    x2 = x2 >= (viewwidth * vga_scale) ? (viewwidth * vga_scale) - 1 : x2;
    int swidth = shape->rightpix - shape->leftpix;

    for (; x1 <= x2; ++x1, frac += screenscale) {
        dc_x = x1;

        int texturecolumn = static_cast<int>(frac >> 20);

        if (texturecolumn > swidth)
            texturecolumn = swidth;

        linecmds = reinterpret_cast<Uint16*>(
            &dc_seg[shape->dataofs[texturecolumn]]);

        ScaleMaskedLSPost(height, 0);
    }
}


//
// bit mask tables for drawing scaled strips up to eight pixels wide
//
// down here so the STUPID inline assembler doesn't get confused!
//


Uint8	mapmasks1[4][8] = {
{1 ,3 ,7 ,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

Uint8	mapmasks2[4][8] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15} };

Uint8	mapmasks3[4][8] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7} };


#if 0

Uint16	wordmasks[8][8] = {
{0x0080,0x00c0,0x00e0,0x00f0,0x00f8,0x00fc,0x00fe,0x00ff},
{0x0040,0x0060,0x0070,0x0078,0x007c,0x007e,0x007f,0x807f},
{0x0020,0x0030,0x0038,0x003c,0x003e,0x003f,0x803f,0xc03f},
{0x0010,0x0018,0x001c,0x001e,0x001f,0x801f,0xc01f,0xe01f},
{0x0008,0x000c,0x000e,0x000f,0x800f,0xc00f,0xe00f,0xf00f},
{0x0004,0x0006,0x0007,0x8007,0xc007,0xe007,0xf007,0xf807},
{0x0002,0x0003,0x8003,0xc003,0xe003,0xf003,0xf803,0xfc03},
{0x0001,0x8001,0xc001,0xe001,0xf001,0xf801,0xfc01,0xfe01} };

#endif

Sint16			slinex,slinewidth;
Uint16	*linecmds;
Sint32		linescale;
Uint16	maskword;

