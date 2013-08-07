// 3D_SCALE.C

#include "3D_DEF.H"
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

short			maxscale,maxscaleshl2;
unsigned short    centery;

short normalshade;
short normalshade_div = 1;
short shade_max = 1;

short nsd_table[] = { 1, 6, 3, 4, 1, 2};
short sm_table[] =  {36,51,62,63,18,52};


/*
=============================================================================

						  LOCALS

=============================================================================
*/

//t_compscale 	*work;
unsigned short BuildCompScale (short height, void**finalspot);

short			stepbytwo;

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

void SetupScaling (short maxscaleheight)
{
	short		i,x,y;
	byte	*dest;

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
	byte		*code;

	int			i;
	long		fix,step;
	unsigned	src,totalscaled,totalsize;
	int			startpix,endpix,toppix;


	step = ((long)height<<16) / 64;
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
	_fmemcpy ((byte *)(*finalspot),(byte *)work,totalsize);

	return totalsize;
}

#endif


// Draw Column vars

longword dc_iscale;
longword dc_frac;
unsigned dc_source;
unsigned dc_seg;
unsigned dc_length;
unsigned dc_dest;

#define SFRACUNIT 0x10000

extern unsigned short * linecmds;

extern boolean useBounceOffset;

fixed bounceOffset=0;

/*
=======================
=
= ScaleMaskedLSPost with Light sourcing
=
=======================
*/


void ScaleMaskedLSPost (short height, unsigned short buf)
{
	short  length;
	unsigned short end;
	unsigned short start;
	long sprtopoffset;
	long topscreen;
	long bottomscreen;
	longword screenstep;
	long dc_yl,dc_yh;
	unsigned short * srcpost;


	fixed bounce;

	if (useBounceOffset)
		bounce=bounceOffset;
	else
		bounce=0;

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(longword)height;
	screenstep = ((longword)height)<<10;

	sprtopoffset=((long)viewheight<<15)-((long)height<<15)+(bounce>>1);
	dc_seg=*(((unsigned short *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
	{
		dc_source=*(srcpost++);

		start=(*(srcpost++))>>1;

		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (long)(screenstep*(long)start);
		bottomscreen = topscreen + (long)(screenstep*(long)length);

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
			dc_dest=buf+(unsigned short)ylookup[(unsigned short)dc_yl];
			dc_length=(unsigned short)(dc_yh-dc_yl+1);
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

/*
=======================
=
= ScaleMaskedWideLSPost with Light sourcing
=
=======================
*/
void ScaleMaskedWideLSPost (short height, unsigned short buf, unsigned short xx, unsigned short pwidth)
{
	byte  ofs;
	byte  msk;
	unsigned short ii;

	buf+=(unsigned short)xx>>2;
	ofs=((byte)(xx&3)<<3)+(byte)pwidth-1;
	outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
	ScaleMaskedLSPost(height,buf);
	msk=(byte)*((byte *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
	msk=(byte)*((byte *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
}

/*
=======================
=
= ScaleMaskedPost without Light sourcing
=
=======================
*/
void ScaleMaskedPost (short height, unsigned short buf)
{
	short  length;
	unsigned short end;
	unsigned short start;
	long sprtopoffset;
	long topscreen;
	long bottomscreen;
	longword screenstep;
	long dc_yl,dc_yh;
	unsigned short * srcpost;

	fixed bounce;

	if (useBounceOffset)
		bounce=bounceOffset;
	else
		bounce=0;

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(longword)height;
	screenstep = ((longword)height)<<10;

	sprtopoffset=((long)viewheight<<15)-((long)height<<15)+(bounce>>1);
	dc_seg=*(((unsigned short *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
		{
		dc_source=*(srcpost++);
		start=(*(srcpost++))>>1;
		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (long)(screenstep*(long)start);
		bottomscreen = topscreen + (long)(screenstep*(long)length);
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
			dc_dest=buf+(unsigned short)ylookup[(unsigned short)dc_yl];
			dc_length=(unsigned short)(dc_yh-dc_yl+1);
			R_DrawColumn();
			}
		end=(*(srcpost++))>>1;
		}

}


/*
=======================
=
= ScaleMaskedWidePost without Light sourcing
=
=======================
*/
void ScaleMaskedWidePost (short height, unsigned short buf, unsigned short xx, unsigned short pwidth)
{
	byte  ofs;
	byte  msk;
	unsigned short ii;

	buf+=(unsigned short)xx>>2;
	ofs=((byte)(xx&3)<<3)+(byte)pwidth-1;
	outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
	ScaleMaskedPost(height,buf);
	msk=(byte)*((byte *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
	msk=(byte)*((byte *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
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
extern byte * shadingtable;
extern byte * lightsource;

void ScaleLSShape (short xcenter, short shapenum, unsigned short height, char lighting)
{
	t_compshape	*shape;
	short      dest;
	short      i;
	longword frac;
	unsigned short width;
	short      x1,x2;
	longword xscale;
	longword screenscale;
	long		texturecolumn;
	long     lastcolumn;
	short      startx;
	unsigned short swidth;
	long     xcent;

	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = PM_GetSpritePage (shapenum);
	*(((unsigned short *)&linecmds)+1)=(unsigned short)shape;		// seg of shape
	xscale=(longword)height<<12;
	xcent=(long)((long)xcenter<<20)-((long)height<<17)+0x80000;
//
// calculate edges of the shape
//
	x1 = (short)((long)(xcent+((long)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (short)((long)(xcent+((long)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((long)-x1)*(long)screenscale;
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;

	i=shade_max-(63l*(unsigned long)(height>>3)/(unsigned long)normalshade)+lighting;

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
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
					ScaleMaskedWideLSPost(height>>2,(unsigned short)bufferofs,(unsigned short)startx,width);
					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (long)(frac>>20);
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
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
					ScaleMaskedWideLSPost(height>>2,(unsigned short)bufferofs,(unsigned short)startx,width);
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
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
			ScaleMaskedWideLSPost(height>>2,bufferofs,(unsigned short)startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;
			outp(SC_INDEX+1,1<<(byte)(x1&3));
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)texturecolumn];
			ScaleMaskedLSPost(height>>2,bufferofs+((unsigned short)x1>>2));
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
void ScaleShape (short xcenter, short shapenum, unsigned short height)
{
	t_compshape	*shape;
	short      dest;
	short      i;
	longword frac;
	unsigned short width;
	short      x1,x2;
	longword xscale;
	longword screenscale;
	long		texturecolumn;
	long     lastcolumn;
	short      startx;
	long     xcent;
	unsigned short swidth;


	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = PM_GetSpritePage (shapenum);
	*(((unsigned short *)&linecmds)+1)=(unsigned short)shape;		// seg of shape
	xscale=(longword)height<<12;
	xcent=(long)((long)xcenter<<20)-((long)height<<(17))+0x80000;
//
// calculate edges of the shape
//
	x1 = (short)((long)(xcent+((long)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (short)((long)(xcent+((long)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((long)-x1)*(long)screenscale;
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
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
					ScaleMaskedWidePost(height>>2,(unsigned short)bufferofs,(unsigned short)startx,width);
					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (long)(frac>>20);
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
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
					ScaleMaskedWidePost(height>>2,(unsigned short)bufferofs,(unsigned short)startx,width);
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
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
			ScaleMaskedWidePost(height>>2,bufferofs,(unsigned short)startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;
			outp(SC_INDEX+1,1<<(byte)(x1&3));
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)texturecolumn];
			ScaleMaskedPost(height>>2,bufferofs+((unsigned short)x1>>2));
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

void SimpleScaleShape (short xcenter, short shapenum, unsigned short height)
{
	t_compshape	*shape;
	short      dest;
	short      i;
	longword frac;
	short      width;
	short      x1,x2;
	longword xscale;
	longword screenscale;
	long		texturecolumn;
	long     lastcolumn;
	short      startx;
	long     xcent;
	unsigned short swidth;

	shape = PM_GetSpritePage (shapenum);
	*(((unsigned short *)&linecmds)+1)=(unsigned short)shape;		// seg of shape
	xscale=(longword)height<<10;
	xcent=(long)((long)xcenter<<16)-((long)height<<(15))+0x8000;
//
// calculate edges of the shape
//
	x1 = (short)((long)(xcent+((long)shape->leftpix*xscale))>>16);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (short)((long)(xcent+((long)shape->rightpix*xscale))>>16);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(64*65536)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=screenscale*((long)-x1);
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
			texturecolumn = (long)(frac>>16);
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
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
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
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
			ScaleMaskedWidePost(height,bufferofs,startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			outp(SC_INDEX+1,1<<(x1&3));
			texturecolumn=frac>>16;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)texturecolumn];
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
void MegaSimpleScaleShape (short xcenter, short ycenter, short shapenum, unsigned short height, unsigned short shade)
{
	t_compshape	*shape;
	short      dest;
	short      i;
	longword frac;
	short      width;
	short      x1,x2;
	longword xscale;
	longword screenscale;
	long		texturecolumn;
	long     lastcolumn;
	short      startx;
	long     xcent;
	unsigned short old_bufferofs;
	short 		swidth;


	old_bufferofs = bufferofs;
	ycenter -=34;
	bufferofs -= ((viewheight-64)>>1)*SCREENBWIDE;
	bufferofs += SCREENBWIDE*ycenter;

	shape = PM_GetSpritePage (shapenum);
	*(((unsigned short *)&linecmds)+1)=(unsigned short)shape;		// seg of shape
	xscale=(longword)height<<14;
	xcent=(long)((long)xcenter<<20)-((long)height<<(19))+0x80000;
//
// calculate edges of the shape
//
	x1 = (short)((long)(xcent+((long)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (short)((long)(xcent+((long)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side

	screenscale=(64L<<20L)/(longword)height;

//
// Choose shade table.
//
	shadingtable=lightsource+(shade<<8);	

//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=screenscale*((long)-x1);
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
			texturecolumn = (long)(frac>>20);
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
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
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)lastcolumn];
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
			(unsigned short)linecmds=(unsigned short)shape->dataofs[(unsigned short)texturecolumn];
			ScaleMaskedLSPost(height,bufferofs+(x1>>2));
			}
		}
	bufferofs = old_bufferofs;

}


//
// bit mask tables for drawing scaled strips up to eight pixels wide
//
// down here so the STUPID inline assembler doesn't get confused!
//


byte	mapmasks1[4][8] = {
{1 ,3 ,7 ,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

byte	mapmasks2[4][8] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15} };

byte	mapmasks3[4][8] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7} };


#if 0

unsigned short	wordmasks[8][8] = {
{0x0080,0x00c0,0x00e0,0x00f0,0x00f8,0x00fc,0x00fe,0x00ff},
{0x0040,0x0060,0x0070,0x0078,0x007c,0x007e,0x007f,0x807f},
{0x0020,0x0030,0x0038,0x003c,0x003e,0x003f,0x803f,0xc03f},
{0x0010,0x0018,0x001c,0x001e,0x001f,0x801f,0xc01f,0xe01f},
{0x0008,0x000c,0x000e,0x000f,0x800f,0xc00f,0xe00f,0xf00f},
{0x0004,0x0006,0x0007,0x8007,0xc007,0xe007,0xf007,0xf807},
{0x0002,0x0003,0x8003,0xc003,0xe003,0xf003,0xf803,0xfc03},
{0x0001,0x8001,0xc001,0xe001,0xf001,0xf801,0xfc01,0xfe01} };

#endif

short			slinex,slinewidth;
unsigned short	*linecmds;
long		linescale;
unsigned short	maskword;

