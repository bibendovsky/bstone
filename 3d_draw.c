// 3D_DRAW.C

#include "3D_DEF.H"
#include <time.h>
#pragma hdrstop

//#define DEBUGWALLS
//#define DEBUGTICS

//#define WOLFDOORS

#define MASKABLE_DOORS		(false)
#define MASKABLE_POSTS		(false | MASKABLE_DOORS)

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

// the door is the last picture before the sprites

#define DOORWALL	(PMSpriteStart-(NUMDOORTYPES))

#define ACTORSIZE	0x4000

void DrawRadar(void);

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//
// player interface stuff
//
int		weaponchangetics,itemchangetics,bodychangetics;
int		plaqueon,plaquetime,getpic;

star_t *firststar,*laststar;		


#ifdef DEBUGWALLS
unsigned screenloc[3]= {PAGE1START,PAGE1START,PAGE1START};
#else
unsigned screenloc[3]= {PAGE1START,PAGE2START,PAGE3START};
#endif
unsigned freelatch = FREESTART;

long 	lasttimecount;
long 	frameon;
long framecount;

unsigned	wallheight[MAXVIEWWIDTH];

fixed	mindist		= MINDIST;


//
// math tables
//
int			pixelangle[MAXVIEWWIDTH];
long		far finetangent[FINEANGLES/4];
fixed 		far sintable[ANGLES+ANGLES/4],far *costable = sintable+(ANGLES/4);

//
// refresh variables
//
fixed	viewx,viewy;			// the focal point
int		viewangle;
fixed	viewsin,viewcos;

#ifndef WOLFDOORS
char far thetile[64];
byte far * mytile;
#endif


fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
int		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);



//
// wall optimization variables
//
int		lastside;		// true for vertical
long	lastintercept;
int		lasttilehit;


//
// ray tracing variables
//
int			focaltx,focalty,viewtx,viewty;

int			midangle,angle;
unsigned	xpartial,ypartial;
unsigned	xpartialup,xpartialdown,ypartialup,ypartialdown;
unsigned	xinttile,yinttile;

unsigned	tilehit;
unsigned	pixx;

int		xtile,ytile;
int		xtilestep,ytilestep;
long	xintercept,yintercept;
long	xstep,ystep;

int		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];



unsigned viewflags;
extern byte lightson;

// Global Cloaked Shape flag..

boolean cloaked_shape = false;



/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void AsmRefresh (void);					// in 3D_DR_A.ASM
void NoWallAsmRefresh (void);			// in 3D_DR_A.ASM

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

#pragma warn -rvl			// I stick the return value in with ASMs

fixed FixedByFrac (fixed a, fixed b)
{
//
// setup
//
asm	mov	si,[WORD PTR b+2]	// sign of result = sign of fraction

asm	mov	ax,[WORD PTR a]
asm	mov	cx,[WORD PTR a+2]

asm	or		cx,cx
asm	jns	aok:				// negative?
asm	neg	cx
asm	neg	ax
asm	sbb	cx,0
asm	xor	si,0x8000			// toggle sign of result
aok:

//
// multiply  cx:ax by bx
//
asm	mov	bx,[WORD PTR b]
asm	mul	bx					// fraction*fraction
asm	mov	di,dx				// di is low word of result
asm	mov	ax,cx				//
asm	mul	bx					// units*fraction
asm 	add	ax,di
asm	adc	dx,0

//
// put result dx:ax in 2's complement
//
asm	test	si,0x8000		// is the result negative?
asm	jz		ansok:
asm	neg	dx
asm	neg	ax
asm	sbb	dx,0

ansok:;

}

#pragma warn +rvl

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
	int ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	long	temp;

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

	ob->viewx = centerx + ny*scale/nx;	// DEBUG: use assembly divide

//
// calculate height (heightnumerator/(nx>>8))
//
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx

	ob->viewheight = temp;
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
boolean TransformTile (int tx, int ty, int *dispx, int *dispheight)
{
	int ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	long	temp;

//
// translate point to view centered coordinates
//
	gx = ((long)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((long)ty<<TILESHIFT)+0x8000-viewy;

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

	*dispx = centerx + ny*scale/nx;	// DEBUG: use assembly divide


//
// calculate height (heightnumerator/(nx>>8))
//
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx

	*dispheight = temp;

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

#pragma warn -rvl			// I stick the return value in with ASMs

int	CalcHeight (void)
{
	int	transheight;
	int 	ratio;
	fixed gxt,gyt,nx,ny;
	long	gx,gy;

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

	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
   asm	cmp	ax,8
   asm	jge  	exit_func
	asm	mov	ax,8

exit_func:

}


//==========================================================================



/*
===================
=
= ScalePost
=
===================
*/

long           postsource;
unsigned       postx;
unsigned       bufx;
unsigned       postwidth;
unsigned       postheight;
byte far *     shadingtable;
extern byte far * lightsource;

void   ScalePost (void)      // VGA version
{
	int height;
	long i;
	byte ofs;
	byte msk;

	height=(wallheight[postx])>>3;
	postheight=height;
	if (gamestate.flags & GS_LIGHTING)
		{

		i=shade_max-(63l*(unsigned long)height/(unsigned long)normalshade);

		if (i<0)
			i=0;
      else
      	if (i > 63)
         	i = 63;					// Debugging.. put break point here!

		shadingtable=lightsource+(i<<8);
		bufx=postx>>2;
		ofs=((postx&3)<<3)+postwidth-1;
		outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
		DrawLSPost();
		msk=(byte)*((byte *)mapmasks2+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawLSPost();
		msk=(byte)*((byte *)mapmasks3+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawLSPost();
		}
	else
		{
		bufx=postx>>2;
		ofs=((postx&3)<<3)+postwidth-1;
		outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
		DrawPost();
		msk=(byte)*((byte *)mapmasks2+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawPost();
		msk=(byte)*((byte *)mapmasks3+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawPost();
		}
}

void  FarScalePost ()				// just so other files can call
{
	ScalePost ();
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

unsigned far DoorJamsShade[] =
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

unsigned far DoorJams[] =
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
	int			wallpic;
	unsigned	texture;
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
		// in the same wall type as last time, so check for optimized draw
		if (texture == (unsigned)postsource && postwidth < 8)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
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
		postwidth = 1;

		if (tilehit & 0x40)
		{
			// check for adjacent doors
         //

			ytile = yintercept>>TILESHIFT;

			if ((doornum = tilemap[xtile-xtilestep][ytile])&0x80 )
				wallpic = DOORWALL+DoorJamsShade[doorobjlist[doornum & 0x7f].type];
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];

		*(((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
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
	int			wallpic;
	unsigned	texture;
	unsigned char doornum;

	texture = (xintercept>>4)&0xfc0;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0xfc0-texture;
	wallheight[pixx] = CalcHeight();

	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		if (texture == (unsigned)postsource && postwidth < 8)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
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
		postwidth = 1;



		if (tilehit & 0x40)
		{								// check for adjacent doors

			xtile = xintercept>>TILESHIFT;
			if ((doornum = tilemap[xtile][ytile-ytilestep]) & 0x80)
			{
				wallpic = DOORWALL+DoorJams[doorobjlist[doornum & 0x7f].type];
			}
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];


		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
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
	unsigned	texture,doorpage = -1,doornum,xint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

	if (doorobjlist[doornum].action == dr_jammed)
		return;

#ifdef WOLFDOORS
	texture = ((xintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
   xint=xintercept&0xffff;

	if (xint>0x7fff)
		texture = ( (xint-(unsigned)(doorposition[doornum]>>1)) >> 4) &0xfc0;
   else
		texture = ( (xint+(unsigned)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same door as last time, so check for optimized draw

		if (texture == (unsigned)postsource && postwidth < 8)
		{
			// wide scale

			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
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
		postwidth = 1;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = DOORWALL+L_METAL;
				break;

			case dr_elevator:
				doorpage = DOORWALL+L_ELEVATOR;
				break;

			case dr_prison:
				doorpage = DOORWALL+L_PRISON;
				break;

			case dr_space:
				doorpage = DOORWALL+L_SPACE;
				break;

			case dr_bio:
				doorpage = DOORWALL+L_BIO;
				break;

			case dr_high_security:
					doorpage = DOORWALL+L_HIGH_SECURITY;		       	// Reverse View
			break;

			case dr_oneway_up:
			case dr_oneway_left:
				if (player->tiley > doorobjlist[doornum].tiley)
					doorpage = DOORWALL+L_ENTER_ONLY;				// normal view
				else
				{
					doorpage = DOORWALL+NOEXIT;		 	      	// Reverse View
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tiley > doorobjlist[doornum].tiley)
				{
					doorpage = DOORWALL+NOEXIT;						// normal view
					lockable = false;
				}
				else
					doorpage = DOORWALL+L_ENTER_ONLY;			// Reverse View
				break;

			case dr_office:
				doorpage = DOORWALL+L_HIGH_TECH;
				break;
		}


		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;


		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(doorpage);
		(unsigned)postsource = texture;
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
	unsigned	texture,doorpage,doornum,yint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

	if (doorobjlist[doornum].action == dr_jammed)
		return;

#ifdef WOLFDOORS
	texture = ( (yintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
		  yint=yintercept&0xffff;
		  if (yint>0x7fff)
			  texture = ( (yint-(unsigned)(doorposition[doornum]>>1)) >> 4) &0xfc0;
		  else
			  texture = ( (yint+(unsigned)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
	// in the same door as last time, so check for optimized draw
		if (texture == (unsigned)postsource && postwidth < 8)
		{
			// wide scale

			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
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
		postwidth = 1;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = DOORWALL+L_METAL_SHADE;
				break;

			case dr_elevator:
				doorpage = DOORWALL+L_ELEVATOR_SHADE;
				break;

			case dr_prison:
				doorpage = DOORWALL+L_PRISON_SHADE;
				break;

			case dr_space:
				doorpage = DOORWALL+L_SPACE_SHADE;
				break;

			case dr_bio:
         	doorpage = DOORWALL+L_BIO;
				break;

			case dr_high_security:
					doorpage = DOORWALL+L_HIGH_SECURITY_SHADE;
			break;

			case dr_oneway_left:
			case dr_oneway_up:
				if (player->tilex > doorobjlist[doornum].tilex)
					doorpage = DOORWALL+L_ENTER_ONLY_SHADE;			// Reverse View
				else
				{
					doorpage = DOORWALL+NOEXIT_SHADE;       			// Normal view
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tilex > doorobjlist[doornum].tilex)
				{
					doorpage = DOORWALL+NOEXIT_SHADE;       		// Reverse View
					lockable = false;
				}
				else
					doorpage = DOORWALL+L_ENTER_ONLY_SHADE;		// Normal View
				break;


			case dr_office:
				doorpage = DOORWALL+L_HIGH_TECH_SHADE;
				break;

		}

		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;

		*(((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(doorpage);
		(unsigned)postsource = texture;
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
	int			wallpic;
	unsigned	texture,offset;

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
		// in the same wall type as last time, so check for optimized draw
		if (texture == (unsigned)postsource && postwidth < 8)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = horizwall[tilehit&63];

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
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
	int			wallpic;
	unsigned	texture,offset;

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
		// in the same wall type as last time, so check for optimized draw
		if (texture == (unsigned)postsource && postwidth < 8)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = vertwall[tilehit&63];

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
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
 unsigned floor=egaFloor[gamestate.episode*MAPS_PER_EPISODE+mapon],
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

void VGAClearScreen (void)
{
	viewflags = gamestate.flags;

//
// clear the screen
//

asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256	// write through all planes
asm	out	dx,ax

asm	mov	dx,80
asm	mov	ax,[viewwidth]
asm	shr	ax,2
asm	sub	dx,ax					// dx = 40-viewwidth/2

asm	mov	bx,[viewwidth]
asm	shr	bx,3					// bl = viewwidth/8
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1

asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

asm 	mov	ax,[viewflags]
asm	test	ax,GS_DRAW_CEILING
asm   jnz   skiptop

asm	mov	ax,[TopColor]

//
// Draw Top
//

toploop:

asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

//
//   Skip 'SkipTop' mods...
//


asm	jmp	bottominit

//
//  SkipTop mods - Compute the correct offset for the  floor
//

skiptop:
asm	mov	al,bh
asm	mov   cl,80
asm	mul	cl
asm	add	di,ax

//
// Test to see if bottom needs drawing
//

bottominit:
asm 	mov	ax,[viewflags]
asm	test	ax,GS_DRAW_FLOOR
asm   jnz   exit_mofo

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1
asm	mov	ax,[BottomColor]

//
// Draw Bottom
//

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop

exit_mofo:

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

int	CalcRotate (objtype *ob)
{
	int	angle,viewangle;
	dirtype dir=ob->dir;

	// this isn't exactly correct, as it should vary by a trig value,
	// but it is close enough with only eight rotations

	viewangle = player->angle + (centerx - ob->viewx)/8;

	if (dir == nodir)
		dir = ob->trydir&127;
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
	int 		i,j,least,numvisable,height;
	memptr	shape;
	byte		*tilespot,*visspot;
	int		shapenum;
	unsigned	spotloc;

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
			if (!(visptr->shapenum = obj->temp1+obj->state->shapenum))
				continue;					// no shape
		}
		else
		if (!(visptr->shapenum = obj->state->shapenum))
			continue;						// no shape

		spotloc = (obj->tilex<<6)+obj->tiley;	// optimize: keep in struct?
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
			obj->active = true;

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
	numvisable = visptr-&vislist[0];

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
		if (gamestate.flags & GS_LIGHTING && (farthest->lighting != NO_SHADING) || cloaked_shape)
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

int	weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY
		,SPR_MACHINEGUNREADY,SPR_CHAINREADY,SPR_GRENADEREADY,SPR_BFG_WEAPON1,0};

boolean useBounceOffset=false;

void DrawPlayerWeapon (void)
{
	int	shapenum;

	if (playstate==ex_victorious)
		return;

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
		if (shapenum)
		{
			static int vh=63;
			static int ce=100;

			char v_table[15]={87,81,77,63,61,60,56,53,50,47,43,41,39,35,31};
			char c_table[15]={88,85,81,80,75,70,64,59,55,50,44,39,34,28,24};

			int oldviewheight=viewheight;
			int centery;

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
	long	newtime,oldtimecount;

#ifdef MYPROFILE
	tics = 3;
	return;
#endif

//
// calculate tics since last refresh for adaptive timing
//
	if (lasttimecount > TimeCount)
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
			tics = newtime-lasttimecount;
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
	VW_ScreenToScreen (displayofs,bufferofs,viewwidth/8,viewheight);
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
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = TILEGLOBAL-ypartialdown;

	lastside = -1;			// the first pixel is on a new wall

	AsmRefresh();
	ScalePost ();			// no more optimization on last post
}



//==========================================================================

extern short MsgTicsRemain;
extern unsigned LastMsgPri;

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
		if (bufferofs > PAGE3START)
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
	int tracedir;

// this wouldn't need to be done except for my debugger/video wierdness
	outportb (SC_INDEX,SC_MAPMASK);

//
// clear out the traced array
//
asm	mov	ax,ds
asm	mov	es,ax
asm	mov	di,OFFSET spotvis
asm	xor	ax,ax
asm	mov	cx,2048							// 64*64 / 2
asm	rep stosw

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

	frameon++;
	PM_NextFrame();
}

//--------------------------------------------------------------------------
// NextBuffer()
//--------------------------------------------------------------------------
int NextBuffer()
{
	displayofs=bufferofs;

#ifdef PAGEFLIP
	asm	cli
	asm	mov	cx,[bufferofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
	asm	sti
#endif

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;
}

byte far TravelTable[MAPSIZE][MAPSIZE];

//--------------------------------------------------------------------------
// UpdateTravelTable()
//--------------------------------------------------------------------------
void UpdateTravelTable()
{
asm	mov	si,OFFSET [spotvis]
asm	mov	ax,SEG [TravelTable]
asm	mov	es,ax
asm	mov	di,OFFSET [TravelTable]
asm	mov	cx,00800h							// HARDCODED for 64x64 / 2!!

loop1:
asm	mov	ax,[si]
asm	inc	si
asm   inc   si
asm	or		[es:di],ax
asm	inc	di
asm	inc	di
asm	loop	loop1
}

extern short an_offset[];


//--------------------------------------------------------------------------
// DrawRadar()
//--------------------------------------------------------------------------
void DrawRadar()
{
	char zoom=gamestate.rzoom;
	byte flags = OV_KEYS|OV_PUSHWALLS|OV_ACTORS;

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

clock_t tc_start,tc_end;
unsigned tc_time;


//--------------------------------------------------------------------------
// ShowOverhead()
//--------------------------------------------------------------------------
void ShowOverhead(short bx, short by, short radius, short zoom, unsigned flags)
{
	#define PLAYER_COLOR 	0xf1
	#define UNMAPPED_COLOR	0x52
	#define MAPPED_COLOR		0x55

	extern byte pixmasks[];
	extern byte far rndtable[];

	byte color,quad;
	byte tile,door;
	objtype *ob;

	fixed dx,dy,psin,pcos,lmx,lmy,baselmx,baselmy,xinc,yinc;
	short rx,ry,mx,my;
	byte far *dstptr,far *basedst,mask,startmask;
	boolean drawplayerok=true;
	byte rndindex;
	boolean snow=false;

// -zoom == make it snow!
//
	if (zoom<0)
	{
		zoom = 0;
		snow = true;
		rndindex = US_RndT();
	}

	zoom = 1<<zoom;
	radius /= zoom;

// Get sin/cos values
//
	psin=sintable[player->angle];
	pcos=costable[player->angle];

// Convert radius to fixed integer and calc rotation.
//
	dx = dy = (long)radius<<TILESHIFT;
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
	basedst=MK_FP(SCREENSEG,bufferofs+ylookup[by]+(bx>>2));
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
	VGAMAPMASK(mask);

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
							if (doorobjlist[door].lock!=kt_none)
								color=0x18;										// locked!
							else
								if (doorobjlist[door].action==dr_closed)
									color=0x58;									// closed!
								else
									color = MAPPED_COLOR;					// floor!
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
						color = 0x10+ob->obclass;

					if ((zoom == 4) || (ExtraRadarFlags & OV_PUSHWALLS))
					{
						unsigned iconnum;

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
			*dstptr = color;
			dstptr += 80;

			if (zoom > 1)						// handle 2x zoom
			{
				*dstptr = color;
				dstptr += 80;

				if (zoom > 2)					// handle 4x zoom
				{
					*dstptr = color;
					dstptr += 80;

					*dstptr = color;
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
		VGAMAPMASK(mask);
	}

	VGAMAPMASK(15);
}
