// 3D_SCALE.C

#include "3d_def.h"
#pragma hdrstop


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
	Sint16		i,x,y;
	Uint8	*dest;

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
unsigned dc_source;
Uint8* dc_seg;
unsigned dc_length;
unsigned dc_dest;

// BBi
// A mask of planes to draw in.
int dc_planes = 1;

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

// FIXME
#if 0
void ScaleMaskedLSPost (Sint16 height, Uint16 buf)
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
		bounce=bounceOffset;
	else
		bounce=0;

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(Uint32)height;
	screenstep = ((Uint32)height)<<10;

	sprtopoffset=((Sint32)viewheight<<15)-((Sint32)height<<15)+(bounce>>1);
	dc_seg=*(((Uint16 *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
	{
		dc_source=*(srcpost++);

		start=(*(srcpost++))>>1;

		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (Sint32)(screenstep*(Sint32)start);
		bottomscreen = topscreen + (Sint32)(screenstep*(Sint32)length);

		dc_yl = (topscreen+SFRACUNIT-1)>>16;
		dc_yh = (bottomscreen-1)>>16;

		if (dc_yh >= viewheight)
			dc_yh = viewheight-1;

		if (dc_yl < 0)
		{
			dc_frac=dc_iscale*(-dc_yl);
			dc_yl = 0;
		}
		else
			dc_frac=0;

		if (dc_yl<=dc_yh)
		{
			dc_dest=buf+(Uint16)ylookup[(Uint16)dc_yl];
			dc_length=(Uint16)(dc_yh-dc_yl+1);
#if CLOAKED_SHAPES
         if (cloaked_shape)
				R_DrawSLSColumn();
         else
#endif
				R_DrawLSColumn();
		}
		end=(*(srcpost++))>>1;
	}
}
#endif // 0

void ScaleMaskedLSPost(int height, int buf)
{
    Sint16 length;
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
    dc_iscale= (64U * 65536U) / (Uint32)height;
    screenstep = ((Uint32)height) << 10;

    sprtopoffset=((Sint32)viewheight << 15) - (height << 15) + (bounce >> 1);

    end = (*(srcpost++)) >> 1;
    for ( ; end != 0; ) {
        dc_source = *srcpost++;

        start = (*srcpost++) >> 1;

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
            dc_dest = buf + ylookup[dc_yl];
            dc_length = dc_yh - dc_yl + 1;
#if CLOAKED_SHAPES
            if (cloaked_shape)
                R_DrawSLSColumn();
            else
#endif
                R_DrawLSColumn();
        }

        end = (*srcpost++) >> 1;
    }
}

/*
=======================
=
= ScaleMaskedWideLSPost with Light sourcing
=
=======================
*/

// FIXME
#if 0
void ScaleMaskedWideLSPost (Sint16 height, Uint16 buf, Uint16 xx, Uint16 pwidth)
{
	Uint8  ofs;
	Uint8  msk;
	Uint16 ii;

	buf+=(Uint16)xx>>2;
	ofs=((Uint8)(xx&3)<<3)+(Uint8)pwidth-1;
	outp(SC_INDEX+1,(Uint8)*((Uint8 *)mapmasks1+ofs));
	ScaleMaskedLSPost(height,buf);
	msk=(Uint8)*((Uint8 *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
	msk=(Uint8)*((Uint8 *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
}
#endif // 0

void ScaleMaskedWideLSPost(int height, int buf, Uint16 xx, Uint16 pwidth)
{
    Uint8  ofs;
    Uint8  msk;
    Uint16 ii;

    buf += xx >> 2;
    ofs = ((Uint8)(xx & 3) << 3) + (Uint8)pwidth - 1;
    dc_planes = ((Uint8*)mapmasks1)[ofs];
    ScaleMaskedLSPost(height, buf);
    msk = ((Uint8*)mapmasks2)[ofs];

    if (msk == 0)
        return;

    ++buf;
    dc_planes = msk;
    ScaleMaskedLSPost(height, buf);
    msk = ((Uint8*)mapmasks3)[ofs];

    if (msk == 0)
        return;

    ++buf;
    dc_planes = msk;
    ScaleMaskedLSPost(height, buf);
}

/*
=======================
=
= ScaleMaskedPost without Light sourcing
=
=======================
*/

// FIXME
#if 0
void ScaleMaskedPost (Sint16 height, Uint16 buf)
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
		bounce=bounceOffset;
	else
		bounce=0;

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(Uint32)height;
	screenstep = ((Uint32)height)<<10;

	sprtopoffset=((Sint32)viewheight<<15)-((Sint32)height<<15)+(bounce>>1);
	dc_seg=*(((Uint16 *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
		{
		dc_source=*(srcpost++);
		start=(*(srcpost++))>>1;
		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (Sint32)(screenstep*(Sint32)start);
		bottomscreen = topscreen + (Sint32)(screenstep*(Sint32)length);
		dc_yl = (topscreen+SFRACUNIT-1)>>16;
		dc_yh = (bottomscreen-1)>>16;
		if (dc_yh >= viewheight)
			dc_yh = viewheight-1;
		if (dc_yl < 0)
			{
			dc_frac=dc_iscale*(-dc_yl);
			dc_yl = 0;
			}
		else
			dc_frac=0;
		if (dc_yl<=dc_yh)
			{
			dc_dest=buf+(Uint16)ylookup[(Uint16)dc_yl];
			dc_length=(Uint16)(dc_yh-dc_yl+1);
			R_DrawColumn();
			}
		end=(*(srcpost++))>>1;
		}

}
#endif // 0

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

    end = (*srcpost++) >> 1;

    for ( ; end != 0; ) {
        dc_source = *srcpost++;
        start = (*srcpost++) >> 1;
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

        end=(*srcpost++) >> 1;
    }
}



/*
=======================
=
= ScaleMaskedWidePost without Light sourcing
=
=======================
*/

// FIXME
#if 0
void ScaleMaskedWidePost (Sint16 height, Uint16 buf, Uint16 xx, Uint16 pwidth)
{
	Uint8  ofs;
	Uint8  msk;
	Uint16 ii;

	buf+=(Uint16)xx>>2;
	ofs=((Uint8)(xx&3)<<3)+(Uint8)pwidth-1;
	outp(SC_INDEX+1,(Uint8)*((Uint8 *)mapmasks1+ofs));
	ScaleMaskedPost(height,buf);
	msk=(Uint8)*((Uint8 *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
	msk=(Uint8)*((Uint8 *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
}
#endif // 0

void ScaleMaskedWidePost(Sint16 height, Uint16 buf, Uint16 xx, Uint16 pwidth)
{
    Uint8 ofs;
    Uint8 msk;
    Uint16 ii;

    buf += xx >> 2;
    ofs = ((Uint8)(xx & 3) << 3) + (Uint8)pwidth - 1;
    dc_planes = ((Uint8*)mapmasks1)[ofs];
    ScaleMaskedPost(height, buf);
    msk = ((Uint8*)mapmasks2)[ofs];

    if (msk == 0)
        return;

    ++buf;
    dc_planes = msk;
    ScaleMaskedPost(height, buf);
    msk = ((Uint8*)mapmasks3)[ofs];

    if (msk == 0)
        return;

    ++buf;
    dc_planes = msk;
    ScaleMaskedPost(height, buf);
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
	Sint16      dest;
	Sint16      i;
	Uint32 frac;
	Uint16 width;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     lastcolumn;
	Sint16      startx;
	Uint16 swidth;
	Sint32     xcent;

	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = (t_compshape*)PM_GetSpritePage(shapenum);

// FIXME
#if 0
	*(((Uint16 *)&linecmds)+1)=(Uint16)shape;		// seg of shape
#endif // 0

    dc_seg = (Uint8*)shape;

    dc_planes = 1;

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
	if (height>256)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				{
				if (lastcolumn>=0)
					{
// FIXME
#if 0
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
					ScaleMaskedWideLSPost(height>>2,(Uint16)bufferofs,(Uint16)startx,width);
#endif // 0

                    linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];
                    ScaleMaskedWideLSPost(height >> 2, bufferofs, (Uint16)startx, width);

					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (Sint32)(frac>>20);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
// FIXME
#if 0
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
					ScaleMaskedWideLSPost(height>>2,(Uint16)bufferofs,(Uint16)startx,width);
#endif // 0

                    linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];
                    ScaleMaskedWideLSPost(height >> 2, bufferofs, (Uint16)startx, width);

					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					width=1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
			ScaleMaskedWideLSPost(height>>2,bufferofs,(Uint16)startx,width);
#endif // 0

                linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];
                ScaleMaskedWideLSPost(height >> 2, bufferofs, (Uint16)startx, width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;

// FIXME
#if 0
			outp(SC_INDEX+1,1<<(Uint8)(x1&3));
#endif // 0

            dc_planes = 1 << (x1 & 3);

			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;

// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)texturecolumn];
#endif // 0

            linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

			ScaleMaskedLSPost(height>>2,bufferofs+((Uint16)x1>>2));
			}
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
	Sint16      dest;
	Sint16      i;
	Uint32 frac;
	Uint16 width;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     lastcolumn;
	Sint16      startx;
	Sint32     xcent;
	Uint16 swidth;


	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = (t_compshape*)PM_GetSpritePage(shapenum);

// FIXME
#if 0
	*(((Uint16 *)&linecmds)+1)=(Uint16)shape;		// seg of shape
#endif // 0

    dc_seg = (Uint8*)shape;

    dc_planes = 1;

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
	if (height>256)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				{
				if (lastcolumn>=0)
					{
// FIXME
#if 0
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
					ScaleMaskedWidePost(height>>2,(Uint16)bufferofs,(Uint16)startx,width);
#endif // 0

                    linecmds = (Uint16*)(&dc_seg[shape->dataofs[(Uint16)lastcolumn]]);
                    ScaleMaskedWidePost(height >> 2, bufferofs, (Uint16)startx, width);

					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (Sint32)(frac>>20);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
// FIXME
#if 0
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
					ScaleMaskedWidePost(height>>2,(Uint16)bufferofs,(Uint16)startx,width);
#endif // 0

                    linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];
                    ScaleMaskedWidePost(height >> 2, bufferofs, (Uint16)startx, width);

					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					width=1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
			ScaleMaskedWidePost(height>>2,bufferofs,(Uint16)startx,width);
#endif // 0

            linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];
            ScaleMaskedWidePost(height >> 2, bufferofs, (Uint16)startx, width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;

// FIXME
#if 0
			outp(SC_INDEX+1,1<<(Uint8)(x1&3));
#endif // 0

            dc_planes = 1 << (Uint8)(x1 & 3);

			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;

// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)texturecolumn];
#endif // 0

            linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

			ScaleMaskedPost(height>>2,bufferofs+((Uint16)x1>>2));
			}
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
	Sint16      dest;
	Sint16      i;
	Uint32 frac;
	Sint16      width;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     lastcolumn;
	Sint16      startx;
	Sint32     xcent;
	Uint16 swidth;

	shape = (t_compshape*)PM_GetSpritePage (shapenum);

// FIXME
#if 0
	*(((Uint16 *)&linecmds)+1)=(Uint16)shape;		// seg of shape
#endif // 0

    dc_seg = (Uint8*)shape;

    dc_planes = 1;

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
	if (height>64)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			texturecolumn = (Sint32)(frac>>16);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
// FIXME
#if 0
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
#endif // 0

                    linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];

					ScaleMaskedWidePost(height,bufferofs,startx,width);
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
#endif // 0

            linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)lastcolumn]];

			ScaleMaskedWidePost(height,bufferofs,startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
// FIXME
#if 0
			outp(SC_INDEX+1,1<<(x1&3));
#endif // 0

            dc_planes = 1 << (x1 & 3);

			texturecolumn=frac>>16;
			if (texturecolumn>swidth)
				texturecolumn=swidth;

// FIXME
#if 0
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)texturecolumn];
#endif // 0

            linecmds = (Uint16*)&dc_seg[shape->dataofs[(Uint16)texturecolumn]];

			ScaleMaskedPost(height,bufferofs+(x1>>2));
			}
		}
}

//-------------------------------------------------------------------------
// MegaSimpleScaleShape()
//
// NOTE: Parameter SHADE determines which Shade palette to use on the shape.
//       0 == NO Shading
//       63 == Max Shade (BLACK or near)
//-------------------------------------------------------------------------

// FIXME
#if 0
void MegaSimpleScaleShape (Sint16 xcenter, Sint16 ycenter, Sint16 shapenum, Uint16 height, Uint16 shade)
{
	t_compshape	*shape;
	Sint16      dest;
	Sint16      i;
	Uint32 frac;
	Sint16      width;
	Sint16      x1,x2;
	Uint32 xscale;
	Uint32 screenscale;
	Sint32		texturecolumn;
	Sint32     lastcolumn;
	Sint16      startx;
	Sint32     xcent;
	Uint16 old_bufferofs;
	Sint16 		swidth;


	old_bufferofs = bufferofs;
	ycenter -=34;
	bufferofs -= ((viewheight-64)>>1)*SCREENBWIDE;
	bufferofs += SCREENBWIDE*ycenter;

	shape = PM_GetSpritePage (shapenum);
	*(((Uint16 *)&linecmds)+1)=(Uint16)shape;		// seg of shape
	xscale=(Uint32)height<<14;
	xcent=(Sint32)((Sint32)xcenter<<20)-((Sint32)height<<(19))+0x80000;
//
// calculate edges of the shape
//
	x1 = (Sint16)((Sint32)(xcent+((Sint32)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (Sint16)((Sint32)(xcent+((Sint32)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side

	screenscale=(64L<<20L)/(Uint32)height;

//
// Choose shade table.
//
	shadingtable=lightsource+(shade<<8);	

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
	if (height>64)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			texturecolumn = (Sint32)(frac>>20);
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
					ScaleMaskedWideLSPost(height,bufferofs,startx,width);
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)lastcolumn];
			ScaleMaskedWideLSPost(height,bufferofs,startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			outp(SC_INDEX+1,1<<(x1&3));
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			(Uint16)linecmds=(Uint16)shape->dataofs[(Uint16)texturecolumn];
			ScaleMaskedLSPost(height,bufferofs+(x1>>2));
			}
		}
	bufferofs = old_bufferofs;

}
#endif // 0

void MegaSimpleScaleShape(
    int xcenter,
    int ycenter,
    int shapenum,
    int height,
    int shade)
{
    t_compshape* shape;
    int dest;
    int i;
    unsigned frac;
    int width;
    int x1;
    int x2;
    unsigned xscale;
    unsigned screenscale;
    int texturecolumn;
    int lastcolumn;
    int startx;
    int xcent;
    int old_bufferofs;
    int swidth;


    dc_planes = 1;

    old_bufferofs = bufferofs;
    ycenter -=34;
    bufferofs -= ((viewheight - 64) >> 1) * SCREENBWIDE;
    bufferofs += SCREENBWIDE * ycenter;

    shape = (t_compshape*)PM_GetSpritePage(shapenum);
    dc_seg = (Uint8*)shape;
    xscale=(Uint32)height << 14;
    xcent = (xcenter << 20) - (height << 19) + 0x80000;

    //
    // calculate edges of the shape
    //
    x1 = (Sint16)((Sint32)(xcent + (shape->leftpix * xscale)) >> 20);

    if (x1 >= viewwidth)
        return; // off the right side

    x2 = (Sint16)((Sint32)(xcent + (shape->rightpix * xscale)) >> 20);

    if (x2 < 0)
        return; // off the left side

    screenscale = (64L << 20L) / (Uint32)height;

    //
    // Choose shade table.
    //
    shadingtable = lightsource + (shade << 8);

    //
    // store information in a vissprite
    //
    if (x1 < 0) {
        frac = screenscale * (-x1);
        x1 = 0;
    } else
        frac = screenscale >> 1;

    x2 = x2 >= viewwidth ? viewwidth - 1 : x2;
    swidth = shape->rightpix-shape->leftpix;

    if (height > 64) {
        width = 1;
        startx = 0;
        lastcolumn = -1;

        for ( ; x1 <= x2; ++x1, frac += screenscale) {
            texturecolumn = (Sint32)(frac >> 20);

            if (texturecolumn == lastcolumn) {
                ++width;
                continue;
            } else {
                if (lastcolumn >= 0) {
                    linecmds = (Uint16*)&dc_seg[shape->dataofs[lastcolumn]];
                    ScaleMaskedWideLSPost(height, bufferofs, startx, width);
                    width = 1;
                    startx = x1;
                    lastcolumn = texturecolumn;
                } else {
                    startx = x1;
                    lastcolumn = texturecolumn;
                }
            }
        }

        if (lastcolumn != -1) {
            linecmds = (Uint16*)&dc_seg[shape->dataofs[lastcolumn]];
            ScaleMaskedWideLSPost(height, bufferofs, startx, width);
        }
    } else {
        for ( ; x1 <= x2; ++x1, frac += screenscale) {
            dc_planes = 1 << (x1 & 3);

            texturecolumn = frac >> 20;

            if (texturecolumn > swidth)
                texturecolumn = swidth;

            linecmds = (Uint16*)&dc_seg[shape->dataofs[texturecolumn]];
            ScaleMaskedLSPost(height, bufferofs + (x1 >> 2));
        }
    }

    bufferofs = old_bufferofs;
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

