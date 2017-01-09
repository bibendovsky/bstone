/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


#include "3d_def.h"


enum ShapeDrawMode {
    e_sdm_simple,
    e_sdm_shaded
}; // ShapeDrawMode


extern const uint8_t* shadingtable;
extern const uint8_t* lightsource;

void R_DrawSLSColumn();
void R_DrawLSColumn();
void R_DrawColumn();


#define CLOAKED_SHAPES (1)

/*
=============================================================================

 GLOBALS

=============================================================================
*/

int maxscale;
int maxscaleshl2;
int centery;

int normalshade;
int normalshade_div = 1;
int shade_max = 1;

int16_t nsd_table[] = { 1, 6, 3, 4, 1, 2 };
int16_t sm_table[] = { 36, 51, 62, 63, 18, 52 };
uint16_t* linecmds;


void SetupScaling(
    int maxscaleheight)
{
    maxscaleheight /= 2; // one scaler every two pixels

    maxscale = maxscaleheight - 1;
    maxscaleshl2 = maxscale * 4;
    normalshade = (3 * maxscale) / (4 * normalshade_div);
    centery = viewheight / 2;
}


// Draw Column vars

int dc_iscale;
int dc_frac;
int dc_source;
uint8_t* dc_seg;
int dc_length;
int dc_dest;
int dc_x;
int dc_y;
int dc_dy;


#define SFRACUNIT (0x10000)

extern bool useBounceOffset;

int bounceOffset = 0;

void generic_scale_masked_post(
    int height,
    ShapeDrawMode draw_mode)
{
    int bounce;

    if (useBounceOffset)
    {
        bounce = bounceOffset;
    }
    else
    {
        bounce = 0;
    }

    const uint16_t* srcpost = linecmds;
    dc_iscale = (64 * 65536) / height;
    int screenstep = height << 10;

    int sprtopoffset = (viewheight << 15) -
        (height << 15) + (bounce >> 1);

    int end = (bstone::Endian::le(*srcpost++)) / 2;

    while (end != 0)
    {
        dc_source = bstone::Endian::le(*srcpost++);

        int start = bstone::Endian::le(*srcpost++) / 2;

        dc_source += start;
        dc_source %= 65536;

        int length = end - start;
        int topscreen = sprtopoffset + (screenstep * start);
        int bottomscreen = topscreen + (screenstep * length);

        int dc_yl = (topscreen + SFRACUNIT - 1) >> 16;
        int dc_yh = (bottomscreen - 1) >> 16;

        if (dc_yh >= viewheight)
        {
            dc_yh = viewheight - 1;
        }

        if (dc_yl < 0)
        {
            dc_frac = dc_iscale * (-dc_yl);
            dc_yl = 0;
        }
        else
        {
            dc_frac = 0;
        }

        if (dc_yl <= dc_yh)
        {
            dc_dy = dc_yl;
            dc_length = dc_yh - dc_yl + 1;
            if (draw_mode == e_sdm_shaded)
            {
#if CLOAKED_SHAPES
                if (cloaked_shape)
                {
                    R_DrawSLSColumn();
                }
                else
#endif
                    R_DrawLSColumn();
            }
            else
            {
                R_DrawColumn();
            }
        }

        end = bstone::Endian::le(*srcpost++) / 2;
    }
}

void generic_scale_shape(
    int xcenter,
    int shapenum,
    int height,
    int8_t lighting,
    ShapeDrawMode draw_mode)
{
    if ((height / 2) > ::maxscaleshl2 || (height / 2) == 0)
    {
        return;
    }

    //xcenter += centerx * (vga_scale - 1);

    auto shape = static_cast<t_compshape*>(::PM_GetSpritePage(shapenum));

    ::dc_seg = reinterpret_cast<uint8_t*>(shape);

    auto xscale = static_cast<int64_t>(height) << 12;

    auto xcent = (static_cast<int64_t>(xcenter) << 20) -
        (static_cast<int64_t>(height) << 17) + 0x80000;

    //
    // calculate edges of the shape
    //
    int x1 = static_cast<int>((xcent + (shape->leftpix * xscale)) >> 20);

    if (x1 >= ::viewwidth)
    {
        return; // off the right side
    }

    int x2 = static_cast<int>((xcent + (shape->rightpix * xscale)) >> 20);

    if (x2 < 0)
    {
        return; // off the left side
    }

    const int screenscale = (256 << 20) / height;

    //
    // store information in a vissprite
    //
    int frac;

    if (x1 < 0)
    {
        frac = (-x1) * screenscale;
        x1 = 0;
    }
    else
    {
        frac = screenscale / 2;
    }

    if (x2 >= ::viewwidth)
    {
        x2 = ::viewwidth - 1;
    }

    if (draw_mode == e_sdm_shaded) {
        int i = shade_max - (63 * height / (normalshade * 8)) + lighting;

        if (i < 0)
        {
            i = 0;
        }
        else if (i > 63)
        {
            i = 63;
        }

        // BBi Don't shade cloaked shape
        if (::cloaked_shape)
        {
            i = 0;
        }

        ::shadingtable = &::lightsource[i * 256];
    }

    ::dc_y = 0;

    const int swidth = shape->rightpix - shape->leftpix;

    for ( ; x1 <= x2; ++x1, frac += screenscale)
    {
        if (::wallheight[x1] > height)
        {
            continue;
        }

        dc_x = x1;

        int texturecolumn = frac >> 20;

        if (texturecolumn > swidth)
        {
            texturecolumn = swidth;
        }

        ::linecmds = reinterpret_cast<uint16_t*>(
            &::dc_seg[shape->dataofs[texturecolumn]]);

        ::generic_scale_masked_post(height / 4, draw_mode);
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
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void ScaleLSShape(
    int xcenter,
    int shapenum,
    int height,
    int8_t lighting)
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
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
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
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
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
