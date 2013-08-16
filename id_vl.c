// ID_VL.C

#include <dos.h>
#include <string.h>

#include "GL/glew.h"

#include "id_head.h"
#include "id_vl.h"
#pragma hdrstop


#define MK_FP(s,o) NULL


//
// SC_INDEX is expected to stay at SC_MAPMASK for proper operation
//

int	bufferofs;
int	displayofs,pelpan;

Uint16	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

Uint16	linewidth;
Uint16	ylookup[MAXSCANLINES];

boolean		screenfaded;
Uint16	bordercolor;

//boolean		fastpalette;				// if true, use outsb to set

Uint8		palette1[256][3], palette2[256][3];


// BBi
typedef struct {
    float x;
    float y;

    float s;
    float t;
} ScreenVertex;

typedef enum {
    OGL_OT_NONE,
    OGL_OT_SHADER,
    OGL_OT_PROGRAM,
} OglObjectType;

typedef enum {
    OGL_SB_NONE,
    OGL_SB_HORIZONTAL,
    OGL_SB_VERTICAL,
    OGL_SB_FULL,
} OglScreenBoxing;


void ogl_draw_screen();
void ogl_refresh_screen();
void ogl_update_screen();

extern const Uint8 vgapal[768];


Uint8* vga_memory = NULL;

static Uint8* vga_palette = NULL;


static void ogl_initialize_video();
static void ogl_uninitialize_video();


static const GLchar* screen_fs_text =
    "#version 120\n"

    "uniform sampler2D screen_tu;\n"
    "uniform sampler1D palette_tu;\n"

    "varying vec2 tc;\n"

    "void main()\n"
    "{\n"
    "    float palette_index = texture2D(screen_tu, tc).r;\n"
    "    palette_index = clamp(palette_index, 0.0, 1.0);\n"
    "    vec4 color = texture1D(palette_tu, palette_index);\n"
    "    gl_FragColor = (color * 255.0) / 63.0;\n"
    "}\n"
;

static const GLchar* screen_vs_text =
    "#version 120\n"

    "attribute vec4 pos_vec4;\n"
    "attribute vec2 tc0_vec2;\n"

    "uniform mat4 proj_mat4;\n"

    "varying vec2 tc;\n"

    "void main()\n"
    "{\n"
    "    tc = tc0_vec2;\n"
    "    gl_Position = proj_mat4 * pos_vec4;\n"
    "}\n"
;


static GLuint screen_tex = GL_NONE;
static GLuint palette_tex = GL_NONE;
static GLuint screen_vbo = GL_NONE;
static GLuint screen_fso = GL_NONE;
static GLuint screen_vso = GL_NONE;
static GLuint screen_po = GL_NONE;


static ScreenVertex screen_vertices[4];

// vertex attribute: position
static GLint a_pos_vec4 = -1;

// vertex attribute: texture coordinates
static GLint a_tc0_vec2 = -1;

// uniform: projection matrix
static GLint u_proj_mat4 = -1;

// uniform: screen texture unit
static GLint u_screen_tu = -1;

// uniform: palette texture unit
static GLint u_palette_tu = -1;

static GLchar* ogl_info_log = NULL;
static GLchar* ogl_empty_info_log = "";

int window_width = 640;
int window_height = 480;

int vanilla_screen_width = 0;
int vanilla_screen_height = 0;
int vanilla_screen_area = 0;

int screen_x = 0;
int screen_y = 0;

int screen_width = 0;
int screen_height = 0;

SDL_Window* sdl_window = NULL;
SDL_GLContext sdl_gl_context = NULL;


//===========================================================================

// asm

Sint16	 VL_VideoID (void);
void VL_SetCRTC (Sint16 crtc);
void VL_SetScreen (Sint16 crtc, Sint16 pelpan);
void VL_WaitVBL (Uint32 vbls);

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

#if !RESTART_PICTURE_PAUSE

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode()
{
    const size_t VGA_MEM_SIZE = 4 * 64 * 1024;
    const size_t VGA_PAL_SIZE = 3 * 256;

    free(vga_memory);
    vga_memory = (Uint8*)malloc(VGA_MEM_SIZE);
    memset(vga_memory, 0, VGA_MEM_SIZE);

    free(vga_palette);
    vga_palette = (Uint8*)malloc(VGA_PAL_SIZE);
    memset(vga_palette, 0, VGA_PAL_SIZE);


    ogl_initialize_video();
}

#endif

/*
=======================
=
= VL_SetTextMode
=
=======================
*/

void VL_SetTextMode()
{
    ogl_uninitialize_video();
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

#endif // RESTART_PICTURE_PAUSE

//===========================================================================

// FIXME
#if 0
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
#endif // 0

void VL_SetLineWidth(int width)
{
    int i;
    int offset;

    offset = 0;
    linewidth = 2 * width;

    for (i = 0; i < MAXSCANLINES; ++i) {
        ylookup[i] = offset;
        offset += linewidth;
    }
}



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

void VL_FillPalette(int red, int green, int blue)
{
    int i;

    for (i = 0; i < 256; ++i) {
        vga_palette[(3 * i) + 0] = (Uint8)red;
        vga_palette[(3 * i) + 1] = (Uint8)green;
        vga_palette[(3 * i) + 2] = (Uint8)blue;
    }

    VL_SetPalette(0, 255, vga_palette);
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

void VL_SetPalette(
    int first,
    int count,
    const Uint8* palette)
{
    int offset = 3 * first;
    int size = 3 * count;

    memmove(&vga_palette[offset], palette, size);

    glActiveTexture(GL_TEXTURE1);

    glTexSubImage1D(
        GL_TEXTURE_1D,
        0,
        0,
        256,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        vga_palette);

    ogl_refresh_screen();
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

void VL_GetPalette(
    int first,
    int count,
    Uint8* palette)
{
    int offset = 3 * first;
    int size = 3 * count;

    memmove(palette, &vga_palette[offset], size);
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

void VL_FadeOut (
    int start,
    int end,
    int red,
    int green,
    int blue,
    int steps)
{
    int i;
    int j;
    int orig;
    int delta;
    Uint8* origptr;
    Uint8* newptr;

    VL_GetPalette(0, 256, &palette1[0][0]);
    memcpy(palette2, palette1, 768);

    //
    // fade through intermediate frames
    //
    for (i = 0; i < steps; ++i) {
        origptr = &palette1[start][0];
        newptr = &palette2[start][0];

        for (j = start; j <= end; ++j) {
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

        VL_SetPalette(0, 256, &palette2[0][0]);
    }

    //
    // final color
    //
    VL_FillPalette(red, green, blue);

    screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn(
    int start,
    int end,
    Uint8* palette,
    int steps)
{
    int i;
    int j;
    int delta;

    VL_GetPalette(0, 256, &palette1[0][0]);
    memcpy(&palette2[0][0], &palette1[0][0], sizeof(palette1));

    start *= 3;
    end = (end * 3) + 2;

    //
    // fade through intermediate frames
    //
    for (i = 0; i < steps; ++i) {
        for (j = start; j <= end; ++j) {
            delta = palette[j] - palette1[0][j];
            palette2[0][j] = palette1[0][j] + ((delta * i) / steps);
        }

        VL_SetPalette(0, 256, &palette2[0][0]);
    }

    //
    // final color
    //
    VL_SetPalette(0, 256, palette);

    screenfaded = false;
}

//------------------------------------------------------------------------
// VL_SetPaletteIntensity()
//------------------------------------------------------------------------
void VL_SetPaletteIntensity(
    int start,
    int end,
    const Uint8* palette,
    int intensity)
{
    int loop;
    char red;
    char green;
    char blue;
    Uint8* cmap = &palette1[0][0] + (start * 3);

    intensity = 63 - intensity;

    for (loop = start; loop <= end; ++loop) {
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

    VL_SetPalette(start, end - start + 1, &palette1[0][0]);
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

// FIXME
#if 0
void VL_Plot (Sint16 x, Sint16 y, Sint16 color)
{
	Uint8 mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(Uint8* )MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
}
#endif // 0

void VL_Plot(int x, int y, int color)
{
    int offset = (4 * bufferofs) + (y * vanilla_screen_width) + x;
    vga_memory[offset] = (Uint8)color;
}


/*
=================
=
= VL_Hlin
=
=================
*/

// FIXME
#if 0
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
#endif // 0

void VL_Hlin(int x, int y, int width, int color)
{
    VL_Bar(x, y, width, 1, color);
}


/*
=================
=
= VL_Vlin
=
=================
*/

// FIXME
#if 0
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
#endif // 0

void VL_Vlin(int x, int y, int height, int color)
{
    VL_Bar(x, y, 1, height, color);
}


/*
=================
=
= VL_Bar
=
=================
*/

// FIXME
#if 0
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
#endif // 0

void VL_Bar(int x, int y, int width, int height, int color)
{
    int i;
    int offset = (4 * bufferofs) + (y * vanilla_screen_width) + x;

    for (i = 0; i < height; ++i) {
        memset(&vga_memory[offset], color, width);
        offset += vanilla_screen_width;
    }
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

void VL_MemToLatch(
    const Uint8* source,
    int width,
    int height,
    int dest)
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

    int i;
    int j;
    int count = ((width + 3) / 4) * height;

    for (i = 0; i < 4; ++i) {
        int offset = (4 * dest) + i;

        for (j = 0; j < count; ++j) {
            vga_memory[offset] = *source++;
            offset += 4;
        }
    }
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

// FIXME
#if 0
void VL_MemToScreen(const Uint8* source, int width, int height, int x, int y)
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
#endif // 0

void VL_MemToScreen(const Uint8* source, int width, int height, int x, int y)
{
    int i;
    int j;
    int k;
    int q_width = width / 4;
    int base_offset = (4 * bufferofs) + (y * vanilla_screen_width) + x;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < height; ++j) {
            int offset = base_offset + i + (j * vanilla_screen_width);

            for (k = 0; k < q_width; ++k) {
                vga_memory[offset] = *source++;
                offset += 4;
            }
        }
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

void VL_LatchToScreen(int source, int width, int height, int x, int y)
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

    int i;
    int count = 4 * width;
    int src_offset = (4 * source);
    int dst_offset = (4 * bufferofs) + (y * vanilla_screen_width) + x;

    for (i = 0; i < height; ++i) {
        memmove(
            &vga_memory[dst_offset],
            &vga_memory[src_offset],
            count);

        src_offset += count;
        dst_offset += vanilla_screen_width;
    }
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


// BBi

// Builds an orthographic projection matrix with upside-downed origin.
void ogl_ortho(
    int width,
    int height,
    float matrix[16])
{
    assert(width > 0);
    assert(height > 0);
    assert(matrix != NULL);

    matrix[0] = 2.0F / width;
    matrix[1] = 0.0F;
    matrix[2] = 0.0F;
    matrix[3] = 0.0F;

    matrix[4] = 0.0F;
    matrix[5] = -2.0F / height;
    matrix[6] = 0.0F;
    matrix[7] = 0.0F;

    matrix[8] = 0.0F;
    matrix[9] = 0.0F;
    matrix[10] = -1.0F;
    matrix[11] = 0.0F;

    matrix[12] = -1.0F;
    matrix[13] = 1.0F;
    matrix[14] = 0.0F;
    matrix[15] = 1.0F;
}

// Clears error flags and returns true if any error flag was set,
// otherwise it return false.
boolean ogl_check_for_and_clear_errors()
{
    boolean result = false;

    while (glGetError())
        result = true;

    return result;
}

// Just draws a screen texture.
void ogl_draw_screen()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    SDL_GL_SwapWindow(sdl_window);
}

// Updates screen texture with display data and
// draws it.
void ogl_refresh_screen()
{
    int i;
    GLenum format;

    glActiveTexture(GL_TEXTURE0);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        vanilla_screen_width,
        vanilla_screen_height,
        GL_RED,
        GL_UNSIGNED_BYTE,
        &vga_memory[4 * displayofs]);

    ogl_draw_screen();
}

// Copies buffer page to a display one,
// updates screen texture with display page data
// and draws it.
void ogl_update_screen()
{
    if (displayofs != bufferofs) {
        memmove(
            &vga_memory[4 * displayofs],
            &vga_memory[4 * bufferofs],
            vanilla_screen_area);
    }

    ogl_refresh_screen();
}

// Returns an information log of a shader or a program.
static const GLchar* ogl_get_info_log(GLuint object)
{
    GLchar* info_log;
    OglObjectType object_type;
    GLint info_log_size = 0; // with a null terminator
    GLsizei info_log_length; // without a null terminator

    if (ogl_info_log != ogl_empty_info_log)
        free(ogl_info_log);

    ogl_info_log = ogl_empty_info_log;

    if (object == GL_NONE)
        return ogl_info_log;

    if (glIsShader(object)) {
        object_type = OGL_OT_SHADER;

        glGetShaderiv(
            object,
            GL_INFO_LOG_LENGTH,
            &info_log_size);
    } else if (glIsProgram(object)) {
        object_type = OGL_OT_PROGRAM;

        glGetProgramiv(
            object,
            GL_INFO_LOG_LENGTH,
            &info_log_size);
    } else
        Quit("OGL: %s", "Invalid object.");

    if (info_log_size <= 1)
        return ogl_info_log;

    info_log = (GLchar*)malloc(info_log_size);

    switch (object_type) {
    case OGL_OT_SHADER:
        glGetShaderInfoLog(
            object,
            info_log_size,
            &info_log_length,
            info_log);
        break;

    case OGL_OT_PROGRAM:
        glGetProgramInfoLog(
            object,
            info_log_size,
            &info_log_length,
            info_log);
        break;
    }

    if (info_log_length > 0)
        ogl_info_log = info_log;
    else
        free(info_log);

    return ogl_info_log;
}

static void ogl_load_shader(
    GLuint shader_object,
    const GLchar* shader_text)
{
    GLint compile_status = GL_FALSE;
    const GLchar* lines[1] = { shader_text, };
    GLint lengths[1] = { (GLint)SDL_strlen(shader_text), };
    const GLchar* shader_log;

    glShaderSource(shader_object, 1, lines, lengths);
    glCompileShader(shader_object);
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compile_status);

    shader_log = ogl_get_info_log(shader_object);

    if (compile_status != GL_FALSE) {
        if (shader_log != NULL && shader_log[0] != '\0')
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", shader_log);
    } else {
        if (shader_log == NULL || shader_log[0] == '\0')
            shader_log = "Generic compile error.";

        Quit("OGL: %s", shader_log);
    }
}

static void ogl_setup_textures()
{
    GLenum internal_format;


    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Setting up textures...");

    screen_tex = GL_NONE;
    glGenTextures(1, &screen_tex);

    if (screen_tex == GL_NONE)
        Quit("Screen texture failed.");

    if (GLEW_VERSION_3_0)
        internal_format = GL_R8;
    else
        internal_format = GL_LUMINANCE8;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internal_format,
        vanilla_screen_width,
        vanilla_screen_height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        NULL);


    palette_tex = GL_NONE;
    glGenTextures(1, &palette_tex);

    if (palette_tex == GL_NONE)
        Quit("Palette texture failed.");

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, palette_tex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage1D(
        GL_TEXTURE_1D,
        0,
        GL_RGB8,
        256,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        NULL);
}

static void ogl_setup_vertex_buffers()
{
    ScreenVertex* vertex;

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Setting up a screen buffer object...");

    screen_vbo = GL_NONE;
    glGenBuffers(1, &screen_vbo);

    if (screen_vbo == GL_NONE)
        Quit("Failed.");


    vertex = &screen_vertices[0];
    vertex->x = 0.0F;
    vertex->y = 0.0F;
    vertex->s = 0.0F;
    vertex->t = 0.0F;

    vertex = &screen_vertices[1];
    vertex->x = 0.0F;
    vertex->y = (float)vanilla_screen_height;
    vertex->s = 0.0F;
    vertex->t = 1.0F;

    vertex = &screen_vertices[2];
    vertex->x = (float)vanilla_screen_width;
    vertex->y = 0.0F;
    vertex->s = 1.0F;
    vertex->t = 0.0F;

    vertex = &screen_vertices[3];
    vertex->x = (float)vanilla_screen_width;
    vertex->y = (float)vanilla_screen_height;
    vertex->s = 1.0F;
    vertex->t = 1.0F;

    glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(screen_vertices),
        screen_vertices,
        GL_STATIC_DRAW);
}

static void ogl_setup_shaders()
{
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Loading a screen fragment shader...");

    screen_fso = glCreateShader(GL_FRAGMENT_SHADER);

    if (screen_fso == GL_NONE)
        Quit("Failed.");


    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Loading a screen vertex shader...");

    ogl_load_shader(screen_fso, screen_fs_text);

    screen_vso = glCreateShader(GL_VERTEX_SHADER);

    if (screen_vso == GL_NONE)
        Quit("Failed.");

    ogl_load_shader(screen_vso, screen_vs_text);
}

static void ogl_setup_programs()
{
    float proj_mat4[16];
    GLint link_status = GL_FALSE;
    const GLchar* program_log;

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "OGL: %s", "Setting up a screen program object...");

    screen_po = glCreateProgram();

    if (screen_po == GL_NONE)
        Quit("Failed.");

    glAttachShader(screen_po, screen_fso);
    glAttachShader(screen_po, screen_vso);
    glLinkProgram(screen_po);
    glGetProgramiv(screen_po, GL_LINK_STATUS, &link_status);

    program_log = ogl_get_info_log(screen_po);

    if (link_status != GL_FALSE) {
        if (program_log != NULL && program_log[0] != '\0')
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", program_log);
    } else {
        if (program_log == NULL || program_log[0] == '\0')
            program_log = "Generic link error.";

        Quit("%s", program_log);
    }


    glUseProgram(screen_po);

    a_pos_vec4 = glGetAttribLocation(screen_po, "pos_vec4");
    a_tc0_vec2 = glGetAttribLocation(screen_po, "tc0_vec2");

    u_proj_mat4 = glGetUniformLocation(screen_po, "proj_mat4");
    ogl_ortho(vanilla_screen_width, vanilla_screen_height, proj_mat4);
    glUniformMatrix4fv(u_proj_mat4, 1, GL_FALSE, proj_mat4);

    u_screen_tu = glGetUniformLocation(screen_po, "screen_tu");
    glUniform1i(u_screen_tu, 0);

    u_palette_tu = glGetUniformLocation(screen_po, "palette_tu");
    glUniform1i(u_palette_tu, 1);
}

static void ogl_uninitialize_video()
{
    if (sdl_gl_context != NULL) {
        if (screen_po != GL_NONE) {
            glDisableVertexAttribArray(a_pos_vec4);
            glDisableVertexAttribArray(a_tc0_vec2);

            glUseProgram(GL_NONE);
            glDetachShader(screen_po, screen_fso);
            glDetachShader(screen_po, screen_vso);
            glDeleteProgram(screen_po);
            screen_po = GL_NONE;
        }

        if (screen_fso != GL_NONE) {
            glDeleteShader(screen_fso);
            screen_fso = GL_NONE;
        }

        if (screen_vso != GL_NONE) {
            glDeleteShader(screen_vso);
            screen_vso = GL_NONE;
        }

        if (screen_vbo != GL_NONE) {
            glDeleteBuffers(1, &screen_vbo);
            screen_vbo = GL_NONE;
        }

        if (screen_tex != GL_NONE) {
            glDeleteTextures(1, &screen_tex);
            screen_tex = GL_NONE;
        }

        if (palette_tex != GL_NONE) {
            glDeleteTextures(1, &palette_tex);
            palette_tex = GL_NONE;
        }

        SDL_GL_MakeCurrent(sdl_window, NULL);
        SDL_GL_DeleteContext(sdl_gl_context);
        sdl_gl_context = NULL;
    }

    if (sdl_window != NULL) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    a_pos_vec4 = -1;
    a_tc0_vec2 = -1;
    u_screen_tu = -1;
    u_palette_tu = -1;
}

static void ogl_initialize_video()
{
    int sdl_result = 0;
    GLenum glew_result = GLEW_OK;

    double scale;
    double h_scale;
    double v_scale;

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Setting up a video subsystem...");

    sdl_result = SDL_InitSubSystem(SDL_INIT_VIDEO);

    if (sdl_result != 0)
        Quit("%s", SDL_GetError());


    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Creating a window...");

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);

    sdl_window = SDL_CreateWindow(
        "BSPS",
        100,
        100,
        window_width,
        window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

    if (sdl_window == NULL)
        Quit("%s", SDL_GetError());


    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Creating an OpenGL context...");

    sdl_gl_context = SDL_GL_CreateContext(sdl_window);

    if (sdl_gl_context == NULL)
        Quit("%s", SDL_GetError());


    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "GLEW: %s", "Initializing...");

    glew_result = glewInit();

    if (glew_result != GLEW_OK)
        Quit("%s", glewGetErrorString(glew_result));


    vanilla_screen_width = 320;
    vanilla_screen_height = 200;
    vanilla_screen_area = vanilla_screen_width * vanilla_screen_height;

    h_scale = (double)window_width / (double)vanilla_screen_width;
    v_scale = (double)window_height / (double)vanilla_screen_height;

    if (h_scale <= v_scale)
        scale = h_scale;
    else
        scale = v_scale;

    screen_width = (int)((vanilla_screen_width * scale) + 0.5);
    screen_height = (int)((vanilla_screen_height * scale) + 0.5);

    screen_x = (window_width - screen_width) / 2;
    screen_y = (window_height - screen_height) / 2;

    VL_SetLineWidth(40);

    ogl_setup_textures();
    ogl_setup_vertex_buffers();
    ogl_setup_shaders();
    ogl_setup_programs();

    SDL_ShowWindow(sdl_window);

    glViewport(screen_x, screen_y, screen_width, screen_height);

    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    if (a_pos_vec4 != -1) {
        glVertexAttribPointer(
            a_pos_vec4,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(ScreenVertex),
            (const GLvoid*)offsetof(ScreenVertex,x));

        glEnableVertexAttribArray(a_pos_vec4);
    }

    if (a_tc0_vec2 != -1) {
        glVertexAttribPointer(
            a_tc0_vec2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(ScreenVertex),
            (const GLvoid*)offsetof(ScreenVertex,s));

        glEnableVertexAttribArray(a_tc0_vec2);
    }

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
}

void JM_VGALinearFill(int start, int length, char fill)
{
    memset(&vga_memory[4 * start], fill, 4 * length);
}

void VL_RefreshScreen()
{
    ogl_refresh_screen();
}
// BBi
