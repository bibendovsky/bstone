// ID_VL.C

#include <dos.h>
//#include <alloc.h>
//#include <mem.h>
#include <string.h>
#include "ID_HEAD.H"
#include "ID_VL.H"
#pragma hdrstop


#define MK_FP(s,o) NULL


//
// SC_INDEX is expected to stay at SC_MAPMASK for proper operation
//

Uint16	bufferofs;
Uint16	displayofs,pelpan;

Uint16	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

Uint16	linewidth;
Uint16	ylookup[MAXSCANLINES];

boolean		screenfaded;
Uint16	bordercolor;

//boolean		fastpalette;				// if true, use outsb to set

Uint8		palette1[256][3], palette2[256][3];


// BBi
Uint8* vga_memory;


//===========================================================================

// asm

Sint16	 VL_VideoID (void);
void VL_SetCRTC (Sint16 crtc);
void VL_SetScreen (Sint16 crtc, Sint16 pelpan);
void VL_WaitVBL (Sint16 vbls);

//===========================================================================


/*
=======================
=
= VL_Startup
=
=======================
*/

#if 0
void	VL_Startup (void)
{
	if ( !MS_CheckParm ("HIDDENCARD") && VL_VideoID () != 5)
		MS_Quit ("You need a VGA graphics card to run this!");

	asm	cld;				// all string instructions assume forward
}

#endif

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void	VL_Shutdown (void)
{
	VL_SetTextMode ();
}

#if RESTART_PICTURE_PAUSE

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void	VL_SetVGAPlaneMode (void)
{
asm	mov	ax,0x13
asm	int	0x10
	VL_DePlaneVGA ();
	VGAMAPMASK(15);
	VL_SetLineWidth (40);
}

#endif

/*
=======================
=
= VL_SetTextMode
=
=======================
*/

void	VL_SetTextMode (void)
{
// FIXME
#if 0
asm	mov	ax,3
asm	int	0x10
#endif // 0
}

//===========================================================================

#if RESTART_PICTURE_PAUSE

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo (Uint8 color)
{
asm	mov	dx,GC_INDEX
asm	mov	al,GC_MODE
asm	out	dx,al
asm	inc	dx
asm	in	al,dx
asm	and	al,0xfc				// write mode 0 to store directly to video
asm	out	dx,al

asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax				// write through all four planes

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,[color]
asm	mov	ah,al
asm	mov	cx,0x8000			// 0x8000 words, clearing 8 video bytes/word
asm	xor	di,di
asm	rep	stosw
}


/*
=============================================================================

			VGA REGISTER MANAGEMENT ROUTINES

=============================================================================
*/


/*
=================
=
= VL_DePlaneVGA
=
=================
*/

void VL_DePlaneVGA (void)
{

//
// change CPU addressing to non linear mode
//

//
// turn off chain 4 and odd/even
//
	outportb (SC_INDEX,SC_MEMMODE);
	outportb (SC_INDEX+1,(inportb(SC_INDEX+1)&~8)|4);

	outportb (SC_INDEX,SC_MAPMASK);		// leave this set throughought

//
// turn off odd/even and set write mode 0
//
	outportb (GC_INDEX,GC_MODE);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~0x13);

//
// turn off chain
//
	outportb (GC_INDEX,GC_MISCELLANEOUS);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~2);

//
// clear the entire buffer space, because int 10h only did 16 k / plane
//
	VL_ClearVideo (0);

//
// change CRTC scanning from doubleword to byte mode, allowing >64k scans
//
	outportb (CRTC_INDEX,CRTC_UNDERLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)&~0x40);

	outportb (CRTC_INDEX,CRTC_MODE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)|0x40);
}

//===========================================================================

/*
====================
=
= VL_SetLineWidth
=
= Line witdh is in WORDS, 40 words is normal width for vgaplanegr
=
====================
*/

void VL_SetLineWidth (unsigned width)
{
	int i,offset;

//
// set wide virtual screen
//
	outport (CRTC_INDEX,CRTC_OFFSET+width*256);

//
// set up lookup tables
//
	linewidth = width*2;

	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += linewidth;
	}
}

#endif

#if 0

/*
====================
=
= VL_SetSplitScreen
=
====================
*/

void VL_SetSplitScreen (int linenum)
{
	VL_WaitVBL (1);
	linenum=linenum*2-1;
	outportb (CRTC_INDEX,CRTC_LINECOMPARE);
	outportb (CRTC_INDEX+1,linenum % 256);
	outportb (CRTC_INDEX,CRTC_OVERFLOW);
	outportb (CRTC_INDEX+1, 1+16*(linenum/256));
	outportb (CRTC_INDEX,CRTC_MAXSCANLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1) & (255-64));
}

#endif  


/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (Sint16 red, Sint16 green, Sint16 blue)
{
// FIXME
#if 0
	int	i;

	outportb (PEL_WRITE_ADR,0);
	for (i=0;i<256;i++)
	{
		outportb (PEL_DATA,red);
		outportb (PEL_DATA,green);
		outportb (PEL_DATA,blue);
	}
#endif // 0
}

//===========================================================================


#if 0		
/*
=================
=
= VL_SetColor
=
=================
*/

void VL_SetColor	(int color, int red, int green, int blue)
{
	outportb (PEL_WRITE_ADR,color);
	outportb (PEL_DATA,red);
	outportb (PEL_DATA,green);
	outportb (PEL_DATA,blue);
}
#endif 


//===========================================================================

#if 0	  

/*
=================
=
= VL_GetColor
=
=================
*/

void VL_GetColor	(int color, int *red, int *green, int *blue)
{
	outportb (PEL_READ_ADR,color);
	*red = inportb (PEL_DATA);
	*green = inportb (PEL_DATA);
	*blue = inportb (PEL_DATA);
}

#endif 	

//===========================================================================

/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= (some cards don't like outsb palette setting)
=
=================
*/

void VL_SetPalette (Uint8 firstreg, Uint16 numregs, Uint8* palette)
{
// FIXME
#if 0
	int	i; 	//,three=3;

	VL_WaitVBL(1);

	asm	mov	dx,PEL_WRITE_ADR
	asm	mov	al,[firstreg]
	asm	out	dx,al
	asm	mov	dx,PEL_DATA
	asm	lds	si,[palette]

#if 0
	asm	test	[ss:fastpalette],1
	asm	jz	slowset

// set palette fast for cards that can take it
//
	asm	mov	ax,[numregs]
	asm	mul	[three]
	asm	mov	cx,ax
	asm	rep 	outsb
	asm	jmp	done
#endif

// set palette slowly for some video cards
//
slowset:
	asm	mov	cx,[numregs]
setloop:
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	loop	setloop

done:
	asm	mov	ax,ss
	asm	mov	ds,ax
#endif // 0
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void VL_GetPalette (Uint8 firstreg, Uint16 numregs, Uint8* palette)
{
// FIXME
#if 0
	int	i;

	numregs *= 3;

	VL_WaitVBL(1);
	outportb (PEL_READ_ADR,firstreg);
	for (i=0;i<numregs;i++)
		*palette++ = inportb(PEL_DATA);
#endif // 0
}

//===========================================================================


/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (Sint16 start, Sint16 end, Sint16 red, Sint16 green, Sint16 blue, Sint16 steps)
{
	Sint16		i,j,orig,delta;
	Uint8	*origptr, *newptr;

	VL_GetPalette (0,256,&palette1[0][0]);
	memcpy (palette2,palette1,768);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start][0];
		newptr = &palette2[start][0];
		for (j=start;j<=end;j++)
		{
			orig = *origptr++;
			delta = red-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = green-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = blue-orig;
			*newptr++ = orig + delta * i / steps;
		}

		VL_SetPalette (0,256,&palette2[0][0]);
	}

//
// final color
//
	VL_FillPalette (red,green,blue);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (Sint16 start, Sint16 end, Uint8* palette, Sint16 steps)
{
	Sint16		i,j,delta;

	VL_GetPalette (0,256,&palette1[0][0]);
	memcpy (&palette2[0][0],&palette1[0][0],sizeof(palette1));

	start *= 3;
	end = end*3+2;

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j]-palette1[0][j];
			palette2[0][j] = palette1[0][j] + delta * i / steps;
		}

		VL_SetPalette (0,256,&palette2[0][0]);
	}

//
// final color
//
	VL_SetPalette (0,256,palette);
	screenfaded = false;
}

//------------------------------------------------------------------------
// VL_SetPaletteIntensity()
//------------------------------------------------------------------------
void VL_SetPaletteIntensity(Sint16 start, Sint16 end, Uint8* palette, char intensity)
{
	Sint16 loop;
	char red,green,blue;
	Uint8* cmap = &palette1[0][0]+start*3;

	intensity = 63 - intensity;
	for (loop=start; loop<=end;loop++)
	{
		red = *palette++ - intensity;
		if (red < 0)
			red = 0;
		*cmap++ = red;

		green = *palette++ - intensity;
		if (green < 0)
			green = 0;
		*cmap++ = green;

		blue = *palette++ - intensity;
		if (blue < 0)
			blue = 0;
		*cmap++ = blue;
	}

	VL_SetPalette(start,end-start+1,&palette1[0][0]);
}

#if 0
//------------------------------------------------------------------------
// FadeOut()
//------------------------------------------------------------------------
void FadeOut(char *colormap, Sint16 numcolors, char delay)
{
	Sint16 loop;

	for (loop=63; loop>=0; loop--)
	{
		SetPaletteIntensity(colormap,numcolors,loop);
		if (delay)
			WaitVBL(delay);
	}
}

//------------------------------------------------------------------------
// FadeIn()
//------------------------------------------------------------------------
void FadeIn(char *colormap, Sint16 numcolors, char delay)
{
	Sint16 loop;

	for (loop=0; loop<64; loop++)
	{
		SetPaletteIntensity(colormap,numcolors,loop);
		if (delay)
			WaitVBL(delay);
	}
}
#endif




#if 0
/*
=================
=
= VL_TestPaletteSet
=
= Sets the palette with outsb, then reads it in and compares
= If it compares ok, fastpalette is set to true.
=
=================
*/

void VL_TestPaletteSet (void)
{
	int	i;

	for (i=0;i<768;i++)
		palette1[0][i] = i;

	fastpalette = true;
	VL_SetPalette (0,256,&palette1[0][0]);
	VL_GetPalette (0,256,&palette2[0][0]);
	if (_fmemcmp (&palette1[0][0],&palette2[0][0],768))
		fastpalette = false;
}
#endif

/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (Sint16 color)
{
// FIXME
#if 0
	_AH=0x10;
	_AL=1;
	_BH=color;
	geninterrupt (0x10);
#endif // 0

	bordercolor = color;
}



/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

Uint8	pixmasks[4] = {1,2,4,8};
Uint8	leftmasks[4] = {15,14,12,8};
Uint8	rightmasks[4] = {1,3,7,15};


/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (Sint16 x, Sint16 y, Sint16 color)
{
	Uint8 mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(Uint8* )MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
}


/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin (Uint16 x, Uint16 y, Uint16 width, Uint16 color)
{
	Uint16		xbyte;
	Uint8			*dest;
	Uint8			leftmask,rightmask;
	Sint16				midbytes;

	xbyte = x>>2;
	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	midbytes = ((x+width+3)>>2) - xbyte - 2;

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+xbyte);

	if (midbytes<0)
	{
	// all in one byte
		VGAMAPMASK(leftmask&rightmask);
		*dest = color;
		VGAMAPMASK(15);
		return;
	}

	VGAMAPMASK(leftmask);
	*dest++ = color;

	VGAMAPMASK(15);
	memset (dest,color,midbytes);
	dest+=midbytes;

	VGAMAPMASK(rightmask);
	*dest = color;

	VGAMAPMASK(15);
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (Sint16 x, Sint16 y, Sint16 height, Sint16 color)
{
	Uint8	*dest,mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	while (height--)
	{
		*dest = color;
		dest += linewidth;
	}

	VGAMAPMASK(15);
}


/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar (Sint16 x, Sint16 y, Sint16 width, Sint16 height, Sint16 color)
{
	Uint8	*dest;
	Uint8	leftmask,rightmask;
	Sint16		midbytes,linedelta;

	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	midbytes = ((x+width+3)>>2) - (x>>2) - 2;
	linedelta = linewidth-(midbytes+1);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	if (midbytes<0)
	{
	// all in one byte
		VGAMAPMASK(leftmask&rightmask);
		while (height--)
		{
			*dest = color;
			dest += linewidth;
		}
		VGAMAPMASK(15);
		return;
	}

	while (height--)
	{
		VGAMAPMASK(leftmask);
		*dest++ = color;

		VGAMAPMASK(15);
		memset (dest,color,midbytes);
		dest+=midbytes;

		VGAMAPMASK(rightmask);
		*dest = color;

		dest+=linedelta;
	}

	VGAMAPMASK(15);
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch (Uint8* source, Sint16 width, Sint16 height, Uint16 dest)
{
// FIXME
#if 0
	unsigned	count;
	Uint8	plane,mask;

	count = ((width+3)/4)*height;
	mask = 1;
	for (plane = 0; plane<4 ; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;

asm	mov	cx,count
asm mov ax,SCREENSEG
asm mov es,ax
asm	mov	di,[dest]
asm	lds	si,[source]
asm	rep movsb
asm mov	ax,ss
asm	mov	ds,ax

		source+= count;
	}
#endif // 0
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen (Uint8* source, Sint16 width, Sint16 height, Sint16 x, Sint16 y)
{
	Uint8   * screen,*dest,mask;
	Sint16		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	mask = 1 << (x&3);

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			memcpy (screen,source,width);
	}
}

//==========================================================================

//------------------------------------------------------------------------
// VL_MaskMemToScreen()
//------------------------------------------------------------------------
void VL_MaskMemToScreen (Uint8* source, Sint16 width, Sint16 height, Sint16 x, Sint16 y, Uint8 mask)
{
	Uint8    *screen,*dest,bmask;
	Sint16		plane,w,h,mod;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));
	bmask = 1 << (x&3);
	mod = linewidth - width;

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(bmask);

		screen = dest;
		h = height;
		while (h--)
		{
			w = width;
			while (w--)
			{
				if (*source != mask)
					*screen = *source;
				source++;
				screen++;
			}
			screen += mod;
		}

		bmask <<= 1;
		if (bmask == 16)
		{
			bmask = 1;
			dest++;
		}
	}
}


//------------------------------------------------------------------------
// VL_ScreenToMem()
//------------------------------------------------------------------------
void VL_ScreenToMem(Uint8* dest, Sint16 width, Sint16 height, Sint16 x, Sint16 y)
{
	Uint8    *screen,*source,mask;
	Sint16		plane;

	width>>=2;
	source = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	mask = 0;

	for (plane = 0; plane<4; plane++)
	{
		VGAREADMAP(mask);

		screen = source;
		for (y=0;y<height;y++,screen+=linewidth,dest+=width)
			memcpy (dest,screen,width);

		mask++;
		if (mask == 4)
		{
			mask = 0;
			source++;
		}
	}
}


//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen (Uint16 source, Sint16 width, Sint16 height, Sint16 x, Sint16 y)
{
// FIXEM
#if 0
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
asm	shl	di,1
asm	mov	di,[WORD PTR ylookup+di]
asm	add	di,[bufferofs]
asm	mov	ax,[x]
asm	shr	ax,2
asm	add	di,ax

asm	mov	si,[source]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
#endif // 0
}


//===========================================================================

#if 0

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (unsigned source, unsigned dest,int width, int height)
{
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	si,[source]
asm	mov	di,[dest]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	si,bx
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
}


#endif

/*
=============================================================================

						STRING OUTPUT ROUTINES

=============================================================================
*/



#if 0	
/*
===================
=
= VL_DrawTile8String
=
===================
*/

void VL_DrawTile8String (char *str, char *tile8ptr, int printx, int printy)
{
	int		i;
	unsigned	*dest,*screen,*src;

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[printy]+(printx>>2));

	while (*str)
	{
		src = (unsigned *)(tile8ptr + (*str<<6));
		// each character is 64 bytes

		VGAMAPMASK(1);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(2);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(4);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(8);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;

		str++;
		printx += 8;
		dest+=2;
	}
}
#endif

#if 0
/*
===================
=
= VL_DrawLatch8String
=
===================
*/

void VL_DrawLatch8String (char *str, unsigned tile8ptr, int printx, int printy)
{
	int		i;
	unsigned	src,dest;

	dest = bufferofs+ylookup[printy]+(printx>>2);

	VGAWRITEMODE(1);
	VGAMAPMASK(15);

	while (*str)
	{
		src = tile8ptr + (*str<<4);		// each character is 16 latch bytes

asm	mov	si,[src]
asm	mov	di,[dest]
asm	mov	dx,[linewidth]

asm	mov	ax,SCREENSEG
asm	mov	ds,ax

asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx
asm	lodsw
asm	mov	[di],ax
asm	add	di,dx

asm	mov	ax,ss
asm	mov	ds,ax

		str++;
		printx += 8;
		dest+=2;
	}

	VGAWRITEMODE(0);
}

#endif


#if 0

/*
===================
=
= VL_SizeTile8String
=
===================
*/

void VL_SizeTile8String (char *str, int *width, int *height)
{
	*height = 8;
	*width = 8*strlen(str);
}

#endif







