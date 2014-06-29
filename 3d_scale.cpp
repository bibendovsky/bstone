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


enum ShapeDrawMode {
    e_sdm_simple,
    e_sdm_shaded
}; // enum ShapeDrawMode


extern const Uint8 * shadingtable;
extern const Uint8 * lightsource;

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

int maxscale;
int maxscaleshl2;
int centery;

int normalshade;
int normalshade_div = 1;
int shade_max = 1;

Sint16 nsd_table[] = { 1, 6, 3, 4, 1, 2};
Sint16 sm_table[] =  {36,51,62,63,18,52};
Uint16* linecmds;

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

void SetupScaling(
    int maxscaleheight)
{
    maxscaleheight /= 2; // one scaler every two pixels

    maxscale = maxscaleheight - 1;
    maxscaleshl2 = maxscale * 4;
    normalshade = (3 * maxscale) / (4 * normalshade_div);
    centery = viewheight / 2;
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

int dc_iscale;
int dc_frac;
int dc_source;
Uint8* dc_seg;
int dc_length;
int dc_dest;
int dc_x;
int dc_y;
int dc_dy;


#define SFRACUNIT 0x10000

extern Uint16* linecmds;

extern boolean useBounceOffset;

fixed bounceOffset=0;

void generic_scale_masked_post(
    int height,
    ShapeDrawMode draw_mode)
{
    int bounce;

    if (useBounceOffset)
        bounce = bounceOffset;
    else
        bounce = 0;

    bounce *= vga_scale;

    const Uint16* srcpost = linecmds;
    dc_iscale = (64 * 65536) / height;
    int screenstep = height << 10;

    int sprtopoffset = ((viewheight * vga_scale) << 15) -
        (height << 15) + (bounce >> 1);

    int end = (bstone::Endian::le(*srcpost++)) / 2;

    while (end != 0) {
        dc_source = bstone::Endian::le(*srcpost++);

        int start = bstone::Endian::le(*srcpost++) / 2;

        dc_source += start;
        dc_source %= 65536;

        int length = end - start;
        int topscreen = sprtopoffset + (screenstep * start);
        int bottomscreen = topscreen + (screenstep * length);

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
            if (draw_mode == e_sdm_shaded) {
#if CLOAKED_SHAPES
                if (cloaked_shape)
                    R_DrawSLSColumn();
                else
#endif
                    R_DrawLSColumn();
            } else
                R_DrawColumn();
        }

        end = bstone::Endian::le(*srcpost++) / 2;
    }
}

void generic_scale_shape(
    int xcenter,
    int shapenum,
    int height,
    char lighting,
    ShapeDrawMode draw_mode)
{
    if ((height / 2) > (maxscaleshl2 * vga_scale) || ((height / 2) == 0))
        return;

    xcenter += centerx * (vga_scale - 1);

    t_compshape* shape =
        static_cast<t_compshape*>(PM_GetSpritePage(shapenum));

    dc_seg = reinterpret_cast<Uint8*>(shape);

    Sint64 xscale = static_cast<Sint64>(height) << 12;

    Sint64 xcent = (static_cast<Sint64>(xcenter) << 20) -
        (static_cast<Sint64>(height) << 17) + 0x80000;

    //
    // calculate edges of the shape
    //
    int x1 = static_cast<int>((xcent + (shape->leftpix * xscale)) >> 20);

    if (x1 >= (viewwidth * vga_scale))
        return; // off the right side

    int x2 = static_cast<int>((xcent + (shape->rightpix * xscale)) >> 20);

    if (x2 < 0)
        return; // off the left side

    int screenscale = (256 << 20) / height;

    //
    // store information in a vissprite
    //
    int frac;

    if (x1 < 0) {
        frac = (-x1) * screenscale;
        x1 = 0;
    } else
        frac = screenscale / 2;

    if (x2 >= (viewwidth * vga_scale))
        x2 = (viewwidth * vga_scale) - 1;

    if (draw_mode == e_sdm_shaded) {
        int i = shade_max - (63 * height / (normalshade * 8 * vga_scale)) +
            lighting;

        if (i < 0)
            i = 0;
        else if (i > 63)
            i = 63;

        shadingtable = &lightsource[i * 256];
    }

    dc_y = 0;
    int swidth = shape->rightpix - shape->leftpix;

    for ( ; x1 <= x2; ++x1, frac += screenscale) {
        if (wallheight[x1] > height)
            continue;

        dc_x = x1;

        int texturecolumn = frac >> 20;

        if (texturecolumn > swidth)
            texturecolumn = swidth;

        linecmds = reinterpret_cast<Uint16*>(
            &dc_seg[shape->dataofs[texturecolumn]]);

        generic_scale_masked_post(height / 4, draw_mode);
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
void ScaleLSShape(
    int xcenter,
    int shapenum,
    int height,
    char lighting)
{
    generic_scale_shape(xcenter, shapenum, height, lighting, e_sdm_shaded);
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
void ScaleShape(
    int xcenter,
    int shapenum,
    int height)
{
    generic_scale_shape(xcenter, shapenum, height, 0, e_sdm_simple);
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
void SimpleScaleShape(
    int xcenter,
    int shapenum,
    int height)
{
    generic_scale_shape(xcenter, shapenum, height, 0, e_sdm_simple);
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
    int xscale = height << 14;
    int xcent = (xcenter << 20) - (height << 19) + 0x80000;

    //
    // calculate edges of the shape
    //
    int x1 = (xcent + (shape->leftpix * xscale)) >> 20;

    if (x1 >= (viewwidth * vga_scale))
        return; // off the right side

    int x2 = (xcent + (shape->rightpix * xscale)) >> 20;

    if (x2 < 0)
        return; // off the left side

    int screenscale = (64 << 20) / height;

    //
    // Choose shade table.
    //
    shadingtable = lightsource + (shade << 8);

    //
    // store information in a vissprite
    //
    int frac;

    if (x1 < 0) {
        frac = screenscale * (-x1);
        x1 = 0;
    } else
        frac = screenscale >> 1;

    if (x2 >= (viewwidth * vga_scale))
        x2 = (viewwidth * vga_scale) - 1;

    int swidth = shape->rightpix - shape->leftpix;

    for ( ; x1 <= x2; ++x1, frac += screenscale) {
        dc_x = x1;

        int texturecolumn = frac >> 20;

        if (texturecolumn > swidth)
            texturecolumn = swidth;

        linecmds = reinterpret_cast<Uint16*>(
            &dc_seg[shape->dataofs[texturecolumn]]);

        generic_scale_masked_post(height, e_sdm_shaded);
    }
}
