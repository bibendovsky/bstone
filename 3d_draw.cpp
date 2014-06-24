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


// 3D_DRAW.C

#include "3d_def.h"

#ifdef MSVC
#pragma hdrstop
#endif

//#define DEBUGWALLS
//#define DEBUGTICS

//#define WOLFDOORS

#define MASKABLE_DOORS		(0)
#define MASKABLE_POSTS		(MASKABLE_DOORS)

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

// the door is the last picture before the sprites

#define DOORWALL	(PMSpriteStart-(NUMDOORTYPES))

#define ACTORSIZE	0x4000

void DrawRadar(void);
void DrawLSPost();
void DrawPost();
void GetBonus (statobj_t *check);
void ScaleLSShape (Sint16 xcenter, Sint16 shapenum, Uint16 height, char lighting);
void DrawAmmoPic(void);
void DrawScoreNum(void);
void DrawWeaponPic(void);
void DrawAmmoNum(void);
void DrawKeyPics(void);
void DrawHealthNum(void);
void UpdateStatusBar(void);
Sint16 NextBuffer();
void UpdateRadarGuage(void);


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//
// player interface stuff
//
Sint16		weaponchangetics,itemchangetics,bodychangetics;
Sint16		plaqueon,plaquetime,getpic;

star_t *firststar,*laststar;		


#ifdef DEBUGWALLS
Uint16 screenloc[3]= {PAGE1START,PAGE1START,PAGE1START};
#else
Uint16 screenloc[3]= {PAGE1START,PAGE2START,PAGE3START};
#endif
Uint16 freelatch = FREESTART;

Sint32 	lasttimecount;
Sint32 	frameon;
Sint32 framecount;

Uint16* wallheight = NULL;

fixed	mindist		= MINDIST;


//
// math tables
//
int* pixelangle = NULL;
Sint32		finetangent[FINEANGLES/4];
fixed 		sintable[ANGLES+ANGLES/4],*costable = sintable+(ANGLES/4);

//
// refresh variables
//
fixed	viewx,viewy;			// the focal point
Sint16		viewangle;
fixed	viewsin,viewcos;

#ifndef WOLFDOORS
char thetile[64];
Uint8 * mytile;
#endif


fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
Sint16		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);



//
// wall optimization variables
//
Sint16		lastside;		// true for vertical
Sint32	lastintercept;
Sint16		lasttilehit;


//
// ray tracing variables
//
Sint16			focaltx,focalty,viewtx,viewty;

Sint16			midangle,angle;
Uint16	xpartial,ypartial;
Uint16	xpartialup,xpartialdown,ypartialup,ypartialdown;
Uint16	xinttile,yinttile;

Uint16	tilehit;
Uint16	pixx;

Sint16		xtile,ytile;
Sint16		xtilestep,ytilestep;
Sint32	xintercept,yintercept;
Sint32	xstep,ystep;

Sint16		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];



Uint16 viewflags;
extern Uint8 lightson;

// Global Cloaked Shape flag..

boolean cloaked_shape = false;



/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void AsmRefresh (void);					// in 3D_DR_A.ASM
void NoWallAsmRefresh (void);			// in 3D_DR_A.ASM


// BBi
static int last_texture_offset = -1;
static const Uint8* last_texture_data = NULL;


/*
============================================================================

			   3 - D  DEFINITIONS

============================================================================
*/


//==========================================================================


/*
========================
=
= FixedByFrac
=
= multiply a 16/16 bit, 2's complement fixed point number by a 16 bit
= fraction, passed as a signed magnitude 32 bit number
=
========================
*/

fixed FixedByFrac(fixed a, fixed b)
{
    int b_sign;
    Uint32 ub;
    Sint32 fracs;
    Sint32 ints;
    Sint32 result;

    b_sign = (b < 0) ? -1 : 1;

    if (b_sign < 0) {
        a = -a;
        b_sign = -b_sign;
    }

    ub = (Uint32)b & 0xFFFF;
    fracs = (((Uint32)a & 0xFFFF) * ub) >> 16;
    ints = (a >> 16) * ub;
    result = ints + fracs;
    result *= b_sign;

    return result;
}

//==========================================================================

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy				: globalx/globaly of point
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale				: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/


//
// transform actor
//
void TransformActor (objtype *ob)
{
	fixed gx,gy,gxt,gyt,nx,ny;
	Sint32	temp;
    Sint32 q;
    Sint32 r;

//
// translate point to view centered coordinates
//
	gx = ob->x-viewx;
	gy = ob->y-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-ACTORSIZE;		// fudge the shape forward a bit, because
										// the midpoint could put parts of the shape
										// into an adjacent wall

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

//
// calculate perspective ratio
//
	ob->transx = nx;
	ob->transy = ny;

	if (nx<mindist)			// too close, don't overflow the divide
	{
	  ob->viewheight = 0;
	  return;
	}

	ob->viewx = static_cast<Sint16>(centerx + ny*scale/nx);	// DEBUG: use assembly divide

    q = (heightnumerator / (nx >> 8)) & 0xFFFF;
    r = (heightnumerator % (nx >> 8)) & 0xFFFF;
    temp = (r << 16) | q;

	ob->viewheight = static_cast<Uint16>(temp);
}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty		: tile the object is centered in
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/
boolean TransformTile (Sint16 tx, Sint16 ty, Sint16 *dispx, Sint16 *dispheight)
{
	fixed gx,gy,gxt,gyt,nx,ny;
	Sint32	temp;
    Sint32 q;
    Sint32 r;

//
// translate point to view centered coordinates
//
	gx = ((Sint32)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((Sint32)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-0x2000;		// 0x2000 is size of object

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;


//
// calculate perspective ratio
//
	if (nx<mindist)			// too close, don't overflow the divide
	{
		*dispheight = 0;
		return false;
	}

	*dispx = static_cast<Sint16>(centerx + ny*scale/nx);	// DEBUG: use assembly divide

    q = (heightnumerator / (nx >> 8)) & 0xFFFF;
    r = (heightnumerator % (nx >> 8)) & 0xFFFF;
    temp = (r << 16) | q;

	*dispheight = static_cast<Sint16>(temp);

//
// see if it should be grabbed
//
	if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
		return true;
	else
		return false;
}




//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

Sint16 CalcHeight()
{
	fixed gxt,gyt,nx;
	Sint32	gx,gy;
    Sint16 result;

	gx = xintercept-viewx;
	gxt = FixedByFrac(gx,viewcos);

	gy = yintercept-viewy;
	gyt = FixedByFrac(gy,viewsin);

	nx = gxt-gyt;

  	//
  	// calculate perspective ratio (heightnumerator/(nx>>8))
	//

	if (nx<mindist)
		nx=mindist;			// don't let divide overflow

    result = (Sint16)(heightnumerator / (nx >> 8));

    if (result < 8)
        result = 8;

   return result;
}


//==========================================================================



/*
===================
=
= ScalePost
=
===================
*/

const Uint8* postsource;
Uint16       postx;
Uint16       postheight;
const Uint8 *     shadingtable;
extern const Uint8 * lightsource;

void ScalePost()
{
    Sint16 height;
    Sint32 i;

    height = wallheight[postx] >> 3;
    postheight = height;

    if ((gamestate.flags & GS_LIGHTING) != 0) {
        i = shade_max - (63L * (Uint32)height / (Uint32)normalshade);

        if (i < 0)
            i = 0;

        if (i > 63)
            i = 63;

        shadingtable = lightsource + (i << 8);
    }

    DrawLSPost();
}

void FarScalePost() // just so other files can call
{
    ScalePost();
}


/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

Uint16 DoorJamsShade[] =
{
	BIO_JAM_SHADE,					// dr_bio
	SPACE_JAM_2_SHADE,			// dr_normal
	STEEL_JAM_SHADE,				// dr_prison
	SPACE_JAM_2_SHADE,			// dr_elevator
	STEEL_JAM_SHADE,				// dr_high_sec
	OFFICE_JAM_SHADE,				// dr_office
	STEEL_JAM_SHADE,				// dr_oneway_left
	STEEL_JAM_SHADE,				// dr_oneway_up
	STEEL_JAM_SHADE,				// dr_oneway_right
	STEEL_JAM_SHADE,				// dr_oneway_down
	SPACE_JAM_SHADE,				// dr_space
};

Uint16 DoorJams[] =
{
	BIO_JAM,					// dr_bio
	SPACE_JAM_2,			// dr_normal
	STEEL_JAM,				// dr_prison
	SPACE_JAM_2,			// dr_elevator
	STEEL_JAM,				// dr_high_sec
	OFFICE_JAM,				// dr_office
	STEEL_JAM,				// dr_oneway_left
	STEEL_JAM,				// dr_oneway_up
	STEEL_JAM,				// dr_oneway_right
	STEEL_JAM,				// dr_oneway_down
	SPACE_JAM,				// dr_space
};



void HitVertWall (void)
{
	Sint16			wallpic;
	Uint16	texture;
	unsigned  char doornum;

	texture = (yintercept>>4)&0xfc0;
	if (xtilestep == -1)
	{
		texture = 0xfc0-texture;
		xintercept += TILEGLOBAL;
	}

	wallheight[pixx] = CalcHeight();

	if (lastside==1 && lastintercept == xtile && lasttilehit == tilehit)
	{
		ScalePost ();

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
	// new wall

		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = true;
		lastintercept = xtile;

		lasttilehit = tilehit;
		postx = pixx;

		if (tilehit & 0x40)
		{
			// check for adjacent doors
         //

			ytile = yintercept>>TILESHIFT;

			if ((doornum = tilemap[xtile-xtilestep][ytile])&0x80 )
				wallpic = static_cast<Sint16>(DOORWALL+DoorJamsShade[doorobjlist[doornum & 0x7f].type]);
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];

        last_texture_data = (const Uint8*)PM_GetPage(wallpic);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}
}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/
void HitHorizWall (void)
{
	Sint16			wallpic;
	Uint16	texture;
	Uint8 doornum;

	texture = (xintercept>>4)&0xfc0;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0xfc0-texture;
	wallheight[pixx] = CalcHeight();

	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		ScalePost ();

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = 0;
		lastintercept = ytile;

		lasttilehit = tilehit;
		postx = pixx;



		if (tilehit & 0x40)
		{								// check for adjacent doors

			xtile = xintercept>>TILESHIFT;
			if ((doornum = tilemap[xtile][ytile-ytilestep]) & 0x80)
			{
				wallpic = static_cast<Sint16>(DOORWALL+DoorJams[doorobjlist[doornum & 0x7f].type]);
			}
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];

        last_texture_data = (const Uint8*)PM_GetPage(wallpic);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}

}


//==========================================================================

/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor (void)
{
	Uint16	texture,doorpage = static_cast<Uint16>(-1),doornum,xint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

	if (doorobjlist[doornum].action == dr_jammed)
		return;

#ifdef WOLFDOORS
	texture = ((xintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
   xint=xintercept&0xffff;

	if (xint>0x7fff)
		texture = ( (xint-(Uint16)(doorposition[doornum]>>1)) >> 4) &0xfc0;
   else
		texture = ( (xint+(Uint16)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
#if MASKABLE_DOORS
		ScaleMPost();
#else
		ScalePost ();
#endif

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = static_cast<Sint16>(DOORWALL+L_METAL);
				break;

			case dr_elevator:
				doorpage = static_cast<Sint16>(DOORWALL+L_ELEVATOR);
				break;

			case dr_prison:
				doorpage = static_cast<Sint16>(DOORWALL+L_PRISON);
				break;

			case dr_space:
				doorpage = static_cast<Sint16>(DOORWALL+L_SPACE);
				break;

			case dr_bio:
				doorpage = static_cast<Sint16>(DOORWALL+L_BIO);
				break;

			case dr_high_security:
					doorpage = static_cast<Sint16>(DOORWALL+L_HIGH_SECURITY);		       	// Reverse View
			break;

			case dr_oneway_up:
			case dr_oneway_left:
				if (player->tiley > doorobjlist[doornum].tiley)
					doorpage = static_cast<Sint16>(DOORWALL+L_ENTER_ONLY);				// normal view
				else
				{
					doorpage = static_cast<Sint16>(DOORWALL+NOEXIT);		 	      	// Reverse View
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tiley > doorobjlist[doornum].tiley)
				{
					doorpage = static_cast<Sint16>(DOORWALL+NOEXIT);						// normal view
					lockable = false;
				}
				else
					doorpage = static_cast<Sint16>(DOORWALL+L_ENTER_ONLY);			// Reverse View
				break;

			case dr_office:
				doorpage = static_cast<Sint16>(DOORWALL+L_HIGH_TECH);
				break;
		}


		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;

        last_texture_data = (const Uint8*)PM_GetPage(doorpage);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}
}

//==========================================================================



/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor (void)
{
	Uint16	texture,doorpage = static_cast<Uint16>(DOORWALL),doornum,yint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

	if (doorobjlist[doornum].action == dr_jammed)
		return;

#ifdef WOLFDOORS
	texture = ( (yintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
		  yint=yintercept&0xffff;
		  if (yint>0x7fff)
			  texture = ( (yint-(Uint16)(doorposition[doornum]>>1)) >> 4) &0xfc0;
		  else
			  texture = ( (yint+(Uint16)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
#if MASKABLE_DOORS
		ScaleMPost();
#else
		ScalePost ();
#endif

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = static_cast<Sint16>(DOORWALL+L_METAL_SHADE);
				break;

			case dr_elevator:
				doorpage = static_cast<Sint16>(DOORWALL+L_ELEVATOR_SHADE);
				break;

			case dr_prison:
				doorpage = static_cast<Sint16>(DOORWALL+L_PRISON_SHADE);
				break;

			case dr_space:
				doorpage = static_cast<Sint16>(DOORWALL+L_SPACE_SHADE);
				break;

			case dr_bio:
         	doorpage = static_cast<Sint16>(DOORWALL+L_BIO);
				break;

			case dr_high_security:
					doorpage = static_cast<Sint16>(DOORWALL+L_HIGH_SECURITY_SHADE);
			break;

			case dr_oneway_left:
			case dr_oneway_up:
				if (player->tilex > doorobjlist[doornum].tilex)
					doorpage = static_cast<Sint16>(DOORWALL+L_ENTER_ONLY_SHADE);			// Reverse View
				else
				{
					doorpage = static_cast<Sint16>(DOORWALL+NOEXIT_SHADE);       			// Normal view
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tilex > doorobjlist[doornum].tilex)
				{
					doorpage = static_cast<Sint16>(DOORWALL+NOEXIT_SHADE);       		// Reverse View
					lockable = false;
				}
				else
					doorpage = static_cast<Sint16>(DOORWALL+L_ENTER_ONLY_SHADE);		// Normal View
				break;


			case dr_office:
				doorpage = static_cast<Sint16>(DOORWALL+L_HIGH_TECH_SHADE);
				break;

		}

		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;

        last_texture_data = (const Uint8*)PM_GetPage(doorpage);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}
}

//==========================================================================

/*
====================
=
= HitHorizPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitHorizPWall (void)
{
	Sint16			wallpic;
	Uint16	texture,offset;

	texture = (xintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL-offset;
	else
	{
		texture = 0xfc0-texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		ScalePost ();

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;

		wallpic = horizwall[tilehit&63];

        last_texture_data = (const Uint8*)PM_GetPage(wallpic);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}
}


/*
====================
=
= HitVertPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitVertPWall (void)
{
	Sint16			wallpic;
	Uint16	texture,offset;

	texture = (yintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	}
	else
		xintercept += offset;

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		ScalePost ();

        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;

		wallpic = vertwall[tilehit&63];

        last_texture_data = (const Uint8*)PM_GetPage(wallpic);
        last_texture_offset = texture;
        postsource = &last_texture_data[last_texture_offset];
	}

}

//==========================================================================

//==========================================================================

#if 0
/*
=====================
=
= ClearScreen
=
=====================
*/

void ClearScreen (void)
{
 Uint16 floor=egaFloor[gamestate.episode*MAPS_PER_EPISODE+mapon],
	  ceiling=egaCeiling[gamestate.episode*MAPS_PER_EPISODE+mapon];

  //
  // clear the screen
  //
asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*2		// read mode 0, write mode 2
asm	out	dx,ax
asm	mov	ax,GC_BITMASK + 255*256
asm	out	dx,ax

asm	mov	dx,40
asm	mov	ax,[viewwidth]
asm	shr	ax,3
asm	sub	dx,ax					// dx = 40-viewwidth/8

asm	mov	bx,[viewwidth]
asm	shr	bx,4					// bl = viewwidth/16
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height

asm	mov	ax,[ceiling]
asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height
asm	mov	ax,[floor]

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop


asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*10		// read mode 1, write mode 2
asm	out	dx,ax
asm	mov	al,GC_BITMASK
asm	out	dx,al

}


#endif
//==========================================================================


#ifdef CEILING_FLOOR_COLORS
/*
=====================
=
= VGAClearScreen
=
= NOTE: Before calling this function - Check to see if there even needs
= ====  to be a solid floor or solid ceiling color drawn.
=
=====================
*/

// BBi
namespace {

void vga_clear_screen(
    int y_offset,
    int height,
    int color)
{
    height *= vga_scale;

    int pixel_offset = vl_get_offset(bufferofs, 0, y_offset);

    if (viewwidth == k_ref_width) {
        std::uninitialized_fill_n(
            &vga_memory[pixel_offset],
            height * vga_width,
            static_cast<Uint8>(color));
    } else {
        for (int y = 0; y < height; ++y) {
            std::uninitialized_fill_n(
                &vga_memory[pixel_offset],
                vga_scale * viewwidth,
                static_cast<Uint8>(color));

            pixel_offset += vga_width;
        }
    }
}

} // namespace
// BBi

void VGAClearScreen (void)
{
	viewflags = gamestate.flags;

    int half_height = viewheight / 2;

    if ((viewflags & GS_DRAW_CEILING) == 0)
        vga_clear_screen(0, half_height, TopColor);

    if ((viewflags & GS_DRAW_FLOOR) == 0) {
        vga_clear_screen(
            viewheight - half_height, half_height, BottomColor);
    }
}
#endif

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

Sint16	CalcRotate (objtype *ob)
{
	Sint16	angle,viewangle;
	dirtype dir=ob->dir;

	// this isn't exactly correct, as it should vary by a trig value,
	// but it is close enough with only eight rotations

	viewangle = player->angle + (centerx - ob->viewx)/8;

	if (dir == nodir)
		dir = static_cast<dirtype>(ob->trydir&127);
	angle =  (viewangle-180)- dirangle[dir];

	angle+=ANGLES/16;
	while (angle>=ANGLES)
		angle-=ANGLES;
	while (angle<0)
		angle+=ANGLES;

	if ((ob->state->flags & SF_PAINFRAME)) // 2 rotation pain frame
		return 4*(angle/(ANGLES/2));        // seperated by 3 (art layout...)

	return angle/(ANGLES/8);
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

#define MAXVISABLE	50


#if 0
typedef struct
{
	int	viewx,
			viewheight,
			shapenum;
} visobj_t;
#endif


visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;


void DrawScaleds (void)
{
	Sint16 		i,least,numvisable,height;
	Uint8		*tilespot,*visspot;
	Uint16	spotloc;

	statobj_t	*statptr;
	objtype		*obj;

	visptr = &vislist[0];

//
// place static objects
//
	for (statptr = &statobjlist[0] ; statptr != laststatobj ; statptr++)
	{
		if ((visptr->shapenum = statptr->shapenum) == -1)
			continue;						// object has been deleted

		if ((Keyboard[sc_6] && (Keyboard[sc_7] || Keyboard[sc_8]) && DebugOk) && (statptr->flags & FL_BONUS))
		{
			GetBonus(statptr);
			continue;
		}

		if (!*statptr->visspot)
			continue;						// not visable


		if (TransformTile(statptr->tilex,statptr->tiley,&visptr->viewx,&visptr->viewheight) &&
			 (statptr->flags & FL_BONUS))
		{
			GetBonus (statptr);
			continue;
		}

		if (!visptr->viewheight)
			continue;						// to close to the object

		visptr->cloaked = false;
		visptr->lighting = statptr->lighting;			// Could add additional
        															// flashing/lighting
		if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
			visptr++;
	}

//
// place active objects
//
	for (obj = player->next;obj;obj=obj->next)
	{


		if (obj->flags & FL_OFFSET_STATES)
		{
			if ((visptr->shapenum = obj->temp1+obj->state->shapenum) == 0)
				continue;					// no shape
		}
		else
		if ((visptr->shapenum = obj->state->shapenum) == 0)
			continue;						// no shape

		spotloc = (obj->tilex<<6)+obj->tiley;	// optimize: keep in struct?

        // BBi Do not draw detonator if it's not visible.
        if (spotloc == 0)
            continue;

		visspot = &spotvis[0][0]+spotloc;
		tilespot = &tilemap[0][0]+spotloc;

		//
		// could be in any of the nine surrounding tiles
		//

		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ))
		{
			obj->active = ac_yes;

			TransformActor (obj);

			if (!obj->viewheight)
				continue;						// too close or far away

         if ((obj->flags2 & (FL2_CLOAKED|FL2_DAMAGE_CLOAK)) == (FL2_CLOAKED))
         {
				visptr->cloaked = 1;
				visptr->lighting = 0;
         }
         else
         {
         	visptr->cloaked = 0;
				visptr->lighting = obj->lighting;
         }

			if (!(obj->flags & FL_DEADGUY))
				obj->flags2 &= ~FL2_DAMAGE_CLOAK;

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;

			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;	// special shape

			if (obj->state->flags & SF_ROTATE)
				visptr->shapenum += CalcRotate (obj);

			if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
				visptr++;
			obj->flags |= FL_VISABLE;
		}
		else
			obj->flags &= ~FL_VISABLE;
	}

//
// draw from back to front
//
	numvisable = static_cast<Sint16>(visptr-&vislist[0]);

	if (!numvisable)
		return;									// no visable objects

	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
		{
			height = visstep->viewheight;
			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}

      //
      // Init our global flag...
      //

      cloaked_shape = farthest->cloaked;

		//
		// draw farthest
		//
		if (((gamestate.flags & GS_LIGHTING) != 0 && farthest->lighting != NO_SHADING) || cloaked_shape)
			ScaleLSShape(farthest->viewx,farthest->shapenum,farthest->viewheight,farthest->lighting);
		else
			ScaleShape(farthest->viewx,farthest->shapenum,farthest->viewheight);

		farthest->viewheight = 32000;
	}

	cloaked_shape = false;

}


//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

Sint16	weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY
		,SPR_MACHINEGUNREADY,SPR_CHAINREADY,SPR_GRENADEREADY,SPR_BFG_WEAPON1,0};

boolean useBounceOffset=false;

void DrawPlayerWeapon (void)
{
	Sint16	shapenum;

	if (playstate==ex_victorious)
		return;

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[static_cast<int>(gamestate.weapon)]+gamestate.weaponframe;
		if (shapenum)
		{
			char v_table[15]={87,81,77,63,61,60,56,53,50,47,43,41,39,35,31};
			char c_table[15]={88,85,81,80,75,70,64,59,55,50,44,39,34,28,24};

			Sint16 oldviewheight=viewheight;
			Sint16 centery;

			useBounceOffset=true;
#if 1
#if 0
			if (Keyboard[sc_PgUp])
			{
				vh++;
				Keyboard[sc_PgUp] = 0;
			}

			if (Keyboard[sc_PgDn])
			{
				if (vh)
					vh--;
				Keyboard[sc_PgDn] = 0;
			}

			if (Keyboard[sc_End])
			{
				ce++;
				Keyboard[sc_End] = 0;
			}

			if (Keyboard[sc_Home])
			{
				if (ce)
					ce--;
				Keyboard[sc_Home] = 0;
			}

			viewheight = vh;
			centery = ce;
#endif

			viewheight = v_table[20-viewsize];
			centery = c_table[20-viewsize];
			MegaSimpleScaleShape(centerx,centery,shapenum,viewheight+1,0);

#if 0
			mclear();
			mprintf("viewheight: %d   \n",viewheight);
			mprintf("   centery: %d   \n",centery);
#endif
#else
			SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
#endif
			useBounceOffset=false;

			viewheight=oldviewheight;
		}
	}
}

//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
	Sint32	newtime;

#ifdef MYPROFILE
	tics = 3;
	return;
#endif

//
// calculate tics since last refresh for adaptive timing
//
	if (static_cast<Uint32>(lasttimecount) > TimeCount)
		TimeCount = lasttimecount;		// if the game was paused a LONG time


#if 0

	if (DemoMode)					// demo recording and playback needs
	{								// to be constant
//
// take DEMOTICS or more tics, and modify Timecount to reflect time taken
//
		oldtimecount = lasttimecount;
		while (TimeCount<oldtimecount+DEMOTICS*2)
		;
		lasttimecount = oldtimecount + DEMOTICS;
		TimeCount = lasttimecount + DEMOTICS;
		tics = DEMOTICS;
	}
	else
#endif
	{
//
// non demo, so report actual time
//
		do
		{
			newtime = TimeCount;
			tics = static_cast<Uint16>(newtime-lasttimecount);
		} while (!tics);			// make sure at least one tic passes

		lasttimecount = newtime;
      framecount++;

#ifdef FILEPROFILE
			strcpy (scratch,"\tTics:");
			itoa (tics,str,10);
			strcat (scratch,str);
			strcat (scratch,"\n");
			write (profilehandle,scratch,strlen(scratch));
#endif

#ifdef DEBUGTICS
		VW_SetAtrReg (ATR_OVERSCAN,tics);
#endif

		realtics=tics;
		if (tics>MAXTICS)
		{
			TimeCount -= (tics-MAXTICS);
			tics = MAXTICS;
		}
	}
}


//==========================================================================


/*
========================
=
= FixOfs
=
========================
*/

void	FixOfs (void)
{
	VW_ScreenToScreen (static_cast<Uint16>(displayofs),static_cast<Uint16>(bufferofs),viewwidth/8,viewheight);
}


//==========================================================================



/*
====================
=
= WallRefresh
=
====================
*/

void WallRefresh (void)
{
//
// set up variables for this view
//

	viewangle = player->angle;
	midangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength,viewcos);
	viewy = player->y + FixedByFrac(focallength,viewsin);

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

	viewtx = player->x >> TILESHIFT;
	viewty = player->y >> TILESHIFT;

	xpartialdown = viewx&(TILEGLOBAL-1);
	xpartialup = static_cast<Uint16>(TILEGLOBAL-xpartialdown);
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = static_cast<Uint16>(TILEGLOBAL-ypartialdown);

	lastside = -1;			// the first pixel is on a new wall

	AsmRefresh();
	ScalePost ();			// no more optimization on last post
}



//==========================================================================

extern Sint16 MsgTicsRemain;
extern Uint16 LastMsgPri;

//-------------------------------------------------------------------------
// RedrawStatusAreas()
//-------------------------------------------------------------------------
void RedrawStatusAreas()
{
	char loop;

   DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;


	for (loop=0; loop<3; loop++)
	{
		LatchDrawPic(0,0,TOP_STATUSBARPIC);
		ShadowPrintLocationText(sp_normal);

		JLatchDrawPic(0,200-STATUSLINES,STATUSBARPIC);
		DrawAmmoPic();
		DrawScoreNum();
		DrawWeaponPic();
		DrawAmmoNum();
		DrawKeyPics();
		DrawHealthNum();

		bufferofs += SCREENSIZE;
		if (bufferofs > static_cast<int>(PAGE3START))
			bufferofs = PAGE1START;
	}
}

void F_MapLSRow();
void C_MapLSRow();
void MapLSRow();


/*
========================
=
= ThreeDRefresh
=
========================
*/

void	ThreeDRefresh (void)
{
    memset(spotvis, 0, sizeof(spotvis));

#ifndef PAGEFLIP
	bufferofs = displayofs = screenloc[0];
#endif

	UpdateInfoAreaClock();
	UpdateStatusBar();

	bufferofs += screenofs;

//
// follow the walls from there to the right, drawwing as we go
//

#ifdef CEILING_FLOOR_COLORS
	if (gamestate.flags & GS_LIGHTING)
	switch (gamestate.flags & (GS_DRAW_FLOOR|GS_DRAW_CEILING))
	{
		case GS_DRAW_FLOOR|GS_DRAW_CEILING:
			MapRowPtr = MapLSRow;
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			MapRowPtr = F_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			MapRowPtr = C_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		default:
			VGAClearScreen();
			WallRefresh();
			break;
	}
	else
	switch (gamestate.flags & (GS_DRAW_FLOOR|GS_DRAW_CEILING))
	{
		case GS_DRAW_FLOOR|GS_DRAW_CEILING:
			MapRowPtr = MapRow;
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			MapRowPtr = F_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			MapRowPtr = C_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		default:
			VGAClearScreen();
			WallRefresh();
			break;
	}
#else

	if (gamestate.flags & GS_LIGHTING)
		MapRowPtr = MapLSRow;
	else
		MapRowPtr = MapRow;

	WallRefresh();
	DrawPlanes();

#endif

	UpdateTravelTable();

//
// draw all the scaled images
//

	DrawScaleds();			// draw scaled stuf

	DrawPlayerWeapon ();	// draw player's hands


//
// show screen and time last cycle
//
	if (fizzlein)
	{
		FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);
		fizzlein = false;

		lasttimecount = TimeCount;		// don't make a big tic count
	}

	bufferofs -= screenofs;

	DrawRadar();

//	VW_WaitVBL(1);		// mike check this out

#ifdef PAGEFLIP
	NextBuffer();
#endif

    // BBi
    VL_RefreshScreen();

	frameon++;
}

//--------------------------------------------------------------------------
// NextBuffer()
//--------------------------------------------------------------------------
Sint16 NextBuffer()
{
	displayofs=bufferofs;
	bufferofs += SCREENSIZE;
	if (bufferofs > static_cast<int>(PAGE3START))
		bufferofs = PAGE1START;

    return 0;
}

Uint8 TravelTable[MAPSIZE][MAPSIZE];

//--------------------------------------------------------------------------
// UpdateTravelTable()
//--------------------------------------------------------------------------
void UpdateTravelTable()
{
    for (int i = 0; i < MAPSIZE; ++i) {
        for (int j = 0; j < MAPSIZE; ++j)
            TravelTable[i][j] |= spotvis[i][j];
    }
}

extern Sint16 an_offset[];


//--------------------------------------------------------------------------
// DrawRadar()
//--------------------------------------------------------------------------
void DrawRadar()
{
	char zoom=gamestate.rzoom;
	Uint8 flags = OV_KEYS|OV_PUSHWALLS|OV_ACTORS;

	if (gamestate.rpower)
	{
	   if ((frameon & 1) && (!godmode))
			if (zoom)
				gamestate.rpower -= tics<<zoom;

		if (gamestate.rpower < 0)
		{
			gamestate.rpower=0;
			DISPLAY_TIMED_MSG(RadarEnergyGoneMsg,MP_WEAPON_AVAIL,MT_GENERAL);
		}
		UpdateRadarGuage();
	}
	else
		zoom = 0;

	ShowOverhead(192,156,16,zoom,flags);
}

Uint16 tc_time;


//--------------------------------------------------------------------------
// ShowOverhead()
//--------------------------------------------------------------------------
void ShowOverhead(Sint16 bx, Sint16 by, Sint16 radius, Sint16 zoom, Uint16 flags)
{
	#define PLAYER_COLOR 	0xf1
	#define UNMAPPED_COLOR	0x52
	#define MAPPED_COLOR		0x55

	extern Uint8 pixmasks[];
	extern const Uint8 rndtable[256];

	Uint8 color;
	Uint8 tile,door;
	objtype *ob;

	fixed dx,dy,psin,pcos,lmx,lmy,baselmx,baselmy,xinc,yinc;
	Sint16 rx,ry,mx,my;

    int dstptr;
    int basedst;
    Uint8 mask = 0;
    Uint8 startmask = 0;
    int i;

	boolean drawplayerok=true;
	Uint8 rndindex = 0;
	boolean snow=false;

// -zoom == make it snow!
//
	if (zoom<0)
	{
		zoom = 0;
		snow = true;
		rndindex = static_cast<Uint8>(US_RndT());
	}

	zoom = 1<<zoom;
	radius /= zoom;

// Get sin/cos values
//
	psin=sintable[player->angle];
	pcos=costable[player->angle];

// Convert radius to fixed integer and calc rotation.
//
	dx = dy = (Sint32)radius<<TILESHIFT;
	baselmx = player->x+(FixedByFrac(dx,pcos)-FixedByFrac(dy,psin));
	baselmy = player->y-(FixedByFrac(dx,psin)+FixedByFrac(dy,pcos));

// Carmack's sin/cos tables use one's complement for negative numbers --
// convert it to two's complement!
//
	if (pcos & 0x80000000)
		pcos = -(pcos & 0xffff);

	if (psin & 0x80000000)
		psin = -(psin & 0xffff);

// Get x/y increment values.
//
	xinc = -pcos;
	yinc = psin;

// Calculate starting destination address.
//
    basedst = bufferofs + ylookup[by] + (bx >> 2);

	switch (zoom)
	{
		case 1:
			startmask = 1;
			mask = pixmasks[bx&3];
		break;

		case 2:							// bx MUST be byte aligned for 2x zoom
			mask = startmask = 3;
		break;

		case 4:							// bx MUST be byte aligned for 4x zoom
			mask = startmask = 15;
		break;
	}

// Draw rotated radar.
//
	rx = radius*2;
	while (rx--)
	{
		lmx = baselmx;
		lmy = baselmy;

		dstptr = basedst;

		ry = radius*2;
		while (ry--)
		{
			if (snow)
			{
				color = 0x42+(rndtable[rndindex]&3);
				rndindex++; 		// += ((rndindex<<1) + 1);
				goto nextx;
			}

		// Don't evaluate if point is outside of map.
		//
			color = UNMAPPED_COLOR;
			mx = lmx>>16;
			my = lmy>>16;
			if (mx<0 || mx>63 || my<0 || my>63)
				goto nextx;

		// SHOW PLAYER
		//
			if (drawplayerok && player->tilex==mx && player->tiley==my)
			{
				color = PLAYER_COLOR;
				drawplayerok=false;
			}
			else
		// SHOW TRAVELED
		//
			if ((TravelTable[mx][my] & TT_TRAVELED) || (flags & OV_SHOWALL))
			{
			// What's at this map location?
			//
				tile=tilemap[mx][my];
				door=tile&0x3f;

			// Evaluate wall or floor?
			//
				if (tile)
				{
				// SHOW DOORS
				//
					if (tile & 0x80)
                    {
							if (doorobjlist[door].lock!=kt_none)
                            {
								color=0x18;										// locked!
                            }
							else
                            {
								if (doorobjlist[door].action==dr_closed)
									color=0x58;									// closed!
								else
									color = MAPPED_COLOR;					// floor!
                            }
                    }
				}
				else
					color = MAPPED_COLOR;									// floor!

			// SHOW KEYS
			//
				if ((flags & OV_KEYS) && (TravelTable[mx][my] & TT_KEYS))
					color = 0xf3;

				if ((zoom > 1) || (ExtraRadarFlags & OV_ACTORS))
				{
					ob=(objtype *)actorat[mx][my];

				// SHOW ACTORS
				//
					if ((flags & OV_ACTORS) && (ob >= objlist) && (!(ob->flags & FL_DEADGUY)) &&
						 (ob->obclass > deadobj) && (ob->obclass < SPACER1_OBJ))
						color = static_cast<Uint8>(0x10+ob->obclass);

					if ((zoom == 4) || (ExtraRadarFlags & OV_PUSHWALLS))
					{
						Uint16 iconnum;

						iconnum = *(mapsegs[1]+farmapylookup[my]+mx);

					// SHOW PUSHWALLS
					//
						if ((flags & OV_PUSHWALLS) && (iconnum == PUSHABLETILE))
							color = 0x79;
					}
				}
			}
			else
				color = UNMAPPED_COLOR;

nextx:;
		// Display pixel for this quadrant and add x/y increments
		//

            for (i = 0; i < 4; ++i) {
                if ((mask & (1 << i)) != 0)
                    vga_memory[(4 * dstptr) + i] = color;
            }

            dstptr += 80;

            // handle 2x zoom
            if (zoom > 1) {
                for (i = 0; i < 4; ++i) {
                    if ((mask & (1 << i)) != 0)
                        vga_memory[(4 * dstptr) + i] = color;
                }

                dstptr += 80;

                // handle 4x zoom
                if (zoom > 2) {
                    for (i = 0; i < 4; ++i) {
                        if ((mask & (1 << i)) != 0)
                            vga_memory[(4 * dstptr) + i] = color;
                    }

                    dstptr += 80;

                    for (i = 0; i < 4; ++i) {
                        if ((mask & (1 << i)) != 0)
                            vga_memory[(4 * dstptr) + i] = color;
                    }

                    dstptr += 80;
                }
            }

			lmx += xinc;
			lmy += yinc;
		}

		baselmx += yinc;
		baselmy -= xinc;

		mask <<= zoom;
		if (mask>15)
		{
			mask=startmask;
			basedst++;
		}
	}
}
