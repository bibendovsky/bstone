/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include <cstring>
#include "gfxv.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_pm.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "bstone_fixed_point.h"
#include "bstone_generic_fizzle_fx.h"


#define MASKABLE_DOORS (0)
#define MASKABLE_POSTS (MASKABLE_DOORS)


namespace
{


//
// Door Objects
//
enum doortype
{
	// LOCKED DOORS

	L_METAL,
	L_METAL_SHADE,

	L_BIO,
	L_BIO_SHADE,

	L_ELEVATOR,
	L_ELEVATOR_SHADE,

	L_SPACE,
	L_SPACE_SHADE,

	L_PRISON,
	L_PRISON_SHADE,

	L_HIGH_SECURITY,
	L_HIGH_SECURITY_SHADE,

	L_ENTER_ONLY,
	L_ENTER_ONLY_SHADE,

	L_HIGH_TECH,
	L_HIGH_TECH_SHADE,


	// UNLOCKED DOORS

	UL_METAL,
	UL_METAL_SHADE,

	UL_BIO,
	UL_BIO_SHADE,

	UL_ELEVATOR,
	UL_ELEVATOR_SHADE,

	UL_SPACE,
	UL_SPACE_SHADE,

	UL_PRISON,
	UL_PRISON_SHADE,

	UL_HIGH_SECURITY,
	UL_HIGH_SECURITY_SHADE,

	UL_ENTER_ONLY,
	UL_ENTER_ONLY_SHADE,

	UL_HIGH_TECH,
	UL_HIGH_TECH_SHADE,


	// MISC DOORS

	NOEXIT,
	NOEXIT_SHADE,

	STEEL_JAM,
	STEEL_JAM_SHADE,

	SPACE_JAM,
	SPACE_JAM_SHADE,

	OFFICE_JAM,
	OFFICE_JAM_SHADE,

	BIO_JAM,
	BIO_JAM_SHADE,

	SPACE_JAM_2,
	SPACE_JAM_2_SHADE,


	// END OF DOOR LIST
	NUMDOORTYPES,
}; // doortype


struct visobj_t
{
	std::int16_t viewx;
	std::int16_t viewheight;
	std::int16_t shapenum;
	std::int8_t lighting;
	bool cloaked;
}; // visobj_t

struct star_t
{
	std::int32_t x;
	std::int32_t y;
	std::int32_t z;
	std::uint8_t color;
}; // star_t


} // namespace


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/

// the door is the last picture before the sprites
#define DOORWALL (PMSpriteStart - (NUMDOORTYPES))

#define ACTORSIZE (0x4000)


void DrawRadar();
void DrawLSPost();
void DrawPost();

void GetBonus(
	statobj_t* check);

void ScaleLSShape(
	int xcenter,
	int shapenum,
	int height,
	std::int8_t lighting);

void DrawAmmoPic();
void DrawScoreNum();
void DrawWeaponPic();
void DrawAmmoNum();
void DrawKeyPics();
void DrawHealthNum();
void UpdateStatusBar();
void UpdateRadarGuage();

void UpdateTravelTable();


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

//
// player interface stuff
//

star_t* firststar;
star_t* laststar;


std::int32_t lasttimecount;
std::int32_t frameon;
std::int32_t framecount;

WallHeight wallheight;

fixed mindist = MINDIST;


//
// math tables
//
std::vector<int> pixelangle;
int finetangent[FINEANGLES / 4];
int sintable[ANGLES + (ANGLES / 4) + 1];
int* costable = &sintable[ANGLES / 4];

//
// refresh variables
//
int viewx;
int viewy; // the focal point
int viewangle;
int viewsin;
int viewcos;

char thetile[64];
std::uint8_t* mytile;


fixed FixedByFrac(
	fixed a,
	fixed b);

void TransformActor(
	objtype* ob);

void BuildTables();

std::int16_t CalcRotate(
	const objtype* ob);

void DrawScaleds();
void CalcTics();
void ThreeDRefresh();


//
// wall optimization variables
//
int lastside; // true for vertical
int lastintercept;
int lasttilehit;

//
// ray tracing variables
//
int focaltx;
int focalty;
int viewtx;
int viewty;

int midangle;
int angle;
int xpartial;
int ypartial;
int xpartialup;
int xpartialdown;
int ypartialup;
int ypartialdown;
int xinttile;
int yinttile;

int tilehit;
int pixx;

int xtile;
int ytile;
int xtilestep;
int ytilestep;
int xintercept;
int yintercept;
int xstep;
int ystep;

std::int16_t horizwall[MAXWALLTILES];
std::int16_t vertwall[MAXWALLTILES];


std::uint16_t viewflags;
extern std::uint8_t lightson;
extern const std::uint8_t rndtable[256];

// Global Cloaked Shape flag.
bool cloaked_shape = false;


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

void AsmRefresh();


// BBi
static int last_texture_offset = -1;
static const std::uint8_t* last_texture_data = nullptr;


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
fixed FixedByFrac(
	fixed a,
	fixed b)
{
	int b_sign;
	std::uint32_t ub;
	std::int32_t fracs;
	std::int32_t ints;
	std::int32_t result;

	b_sign = (b < 0) ? -1 : 1;

	if (b_sign < 0)
	{
		a = -a;
		b_sign = -b_sign;
	}

	ub = (std::uint32_t)b & 0xFFFF;
	fracs = (((std::uint32_t)a & 0xFFFF) * ub) >> 16;
	ints = (a >> 16) * ub;
	result = ints + fracs;
	result *= b_sign;

	return result;
}

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy : globalx/globaly of point
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/
void TransformActor(
	objtype* ob)
{
	fixed gx, gy, gxt, gyt, nx, ny;
	std::int32_t temp;
	std::int32_t q;
	std::int32_t r;

	//
	// translate point to view centered coordinates
	//
	gx = ob->x - viewx;
	gy = ob->y - viewy;

	//
	// calculate newx
	//
	gxt = FixedByFrac(gx, viewcos);
	gyt = FixedByFrac(gy, viewsin);
	nx = gxt - gyt - ACTORSIZE; // fudge the shape forward a bit, because
	// the midpoint could put parts of the shape
	// into an adjacent wall

//
// calculate newy
//
	gxt = FixedByFrac(gx, viewsin);
	gyt = FixedByFrac(gy, viewcos);
	ny = gyt + gxt;

	//
	// calculate perspective ratio
	//
	ob->transx = nx;
	ob->transy = ny;

	if (nx < mindist)
	{ // too close, don't overflow the divide
		ob->viewheight = 0;
		return;
	}

	ob->viewx = static_cast<std::int16_t>(centerx + ny * scale / nx); // DEBUG: use assembly divide

	q = (heightnumerator / (nx >> 8)) & 0xFFFF;
	r = (heightnumerator % (nx >> 8)) & 0xFFFF;
	temp = (r << 16) | q;

	ob->viewheight = static_cast<std::uint16_t>(temp);
}

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty               : tile the object is centered in
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/
void TransformTile(
	std::int16_t tx,
	std::int16_t ty,
	std::int16_t* dispx,
	std::int16_t* dispheight)
{
	fixed gx;
	fixed gy;
	fixed gxt;
	fixed gyt;
	fixed nx;
	fixed ny;

	std::int32_t temp;
	std::int32_t q;
	std::int32_t r;

	//
	// translate point to view centered coordinates
	//
	gx = ((std::int32_t)tx << TILESHIFT) + 0x8000 - viewx;
	gy = ((std::int32_t)ty << TILESHIFT) + 0x8000 - viewy;

	//
	// calculate newx
	//
	gxt = FixedByFrac(gx, ::viewcos);
	gyt = FixedByFrac(gy, ::viewsin);
	nx = gxt - gyt - 0x2000; // 0x2000 is size of object

	//
	// calculate newy
	//
	gxt = ::FixedByFrac(gx, ::viewsin);
	gyt = ::FixedByFrac(gy, ::viewcos);
	ny = gyt + gxt;


	//
	// calculate perspective ratio
	//
	if (nx < mindist)
	{
		// too close, don't overflow the divide
		*dispheight = 0;

		return;
	}

	*dispx = static_cast<std::int16_t>(centerx + ((ny * scale) / nx)); // DEBUG: use assembly divide

	q = (heightnumerator / (nx >> 8)) & 0xFFFF;
	r = (heightnumerator % (nx >> 8)) & 0xFFFF;
	temp = (r << 16) | q;

	*dispheight = static_cast<std::int16_t>(temp);
}

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/
int CalcHeight()
{
	int gx = xintercept - viewx;
	int gxt = FixedByFrac(gx, viewcos);

	int gy = yintercept - viewy;
	int gyt = FixedByFrac(gy, viewsin);

	int nx = gxt - gyt;

	//
	// calculate perspective ratio (heightnumerator/(nx>>8))
	//

	if (nx < mindist)
	{
		nx = mindist; // don't let divide overflow

	}
	int result = heightnumerator / (nx / 256);

	if (result < 8)
	{
		result = 8;
	}

	return result;
}


const std::uint8_t* postsource;
int postx;

// BBi
int posty;

int postheight;
const std::uint8_t* shadingtable;
extern const std::uint8_t* lightsource;

void ScalePost()
{
	const auto height = ::wallheight[postx] / 8;

	::postheight = height;

	if ((::gamestate.flags & GS_LIGHTING) != 0)
	{
		auto i = ::shade_max - ((63 * height) / ::normalshade);

		if (i < 0)
		{
			i = 0;
		}
		else if (i > 63)
		{
			i = 63;
		}

		::shadingtable = ::lightsource + (i * 256);

		::DrawLSPost();
	}
	else
	{
		::DrawPost();
	}
}

void FarScalePost() // just so other files can call
{
	ScalePost();
}


std::uint16_t DoorJamsShade[] = {
	BIO_JAM_SHADE, // dr_bio
	SPACE_JAM_2_SHADE, // dr_normal
	STEEL_JAM_SHADE, // dr_prison
	SPACE_JAM_2_SHADE, // dr_elevator
	STEEL_JAM_SHADE, // dr_high_sec
	OFFICE_JAM_SHADE, // dr_office
	STEEL_JAM_SHADE, // dr_oneway_left
	STEEL_JAM_SHADE, // dr_oneway_up
	STEEL_JAM_SHADE, // dr_oneway_right
	STEEL_JAM_SHADE, // dr_oneway_down
	SPACE_JAM_SHADE, // dr_space
};

std::uint16_t DoorJams[] = {
	BIO_JAM, // dr_bio
	SPACE_JAM_2, // dr_normal
	STEEL_JAM, // dr_prison
	SPACE_JAM_2, // dr_elevator
	STEEL_JAM, // dr_high_sec
	OFFICE_JAM, // dr_office
	STEEL_JAM, // dr_oneway_left
	STEEL_JAM, // dr_oneway_up
	STEEL_JAM, // dr_oneway_right
	STEEL_JAM, // dr_oneway_down
	SPACE_JAM, // dr_space
};

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
void HitVertWall()
{
	std::int16_t wallpic;
	std::uint16_t texture;

	texture = (yintercept >> 4) & 0xfc0;
	if (xtilestep == -1)
	{
		texture = 0xfc0 - texture;
		xintercept += TILEGLOBAL;
	}

	wallheight[pixx] = CalcHeight();

	if (lastside == 1 && lastintercept == xtile && lasttilehit == tilehit)
	{
		ScalePost();

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		// new wall

		if (lastside != -1)
		{ // if not the first scaled post
			ScalePost();
		}

		lastside = true;
		lastintercept = xtile;

		lasttilehit = tilehit;
		postx = pixx;

		if (tilehit & 0x40)
		{
			// check for adjacent doors
			//

			ytile = yintercept >> TILESHIFT;

			auto door_index = ::tilemap[::xtile - ::xtilestep][::ytile];

			if ((door_index & 0x80) != 0 && (door_index & 0xC0) != 0xC0)
			{
				auto door = ::doorobjlist[door_index & 0x3F];

				wallpic = static_cast<std::int16_t>(DOORWALL + ::DoorJamsShade[door.type]);
			}
			else
			{
				wallpic = vertwall[tilehit & ~0x40];
			}
		}
		else
		{
			wallpic = vertwall[tilehit];
		}

		last_texture_data = (const std::uint8_t*)PM_GetPage(wallpic);
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
void HitHorizWall()
{
	std::int16_t wallpic;
	std::uint16_t texture;

	texture = (xintercept >> 4) & 0xfc0;
	if (ytilestep == -1)
	{
		yintercept += TILEGLOBAL;
	}
	else
	{
		texture = 0xfc0 - texture;
	}
	wallheight[pixx] = CalcHeight();

	if (lastside == 0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		ScalePost();

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		// new wall
		if (lastside != -1)
		{ // if not the first scaled post
			ScalePost();
		}

		lastside = 0;
		lastintercept = ytile;

		lasttilehit = tilehit;
		postx = pixx;

		if (tilehit & 0x40)
		{ // check for adjacent doors

			xtile = xintercept >> TILESHIFT;

			auto door_index = ::tilemap[::xtile][::ytile - ::ytilestep];

			if ((door_index & 0x80) != 0 && (door_index & 0xC0) != 0xC0)
			{
				auto door = ::doorobjlist[door_index & 0x3F];

				wallpic = static_cast<std::int16_t>(DOORWALL + ::DoorJams[door.type]);
			}
			else
			{
				wallpic = horizwall[tilehit & ~0x40];
			}
		}
		else
		{
			wallpic = horizwall[tilehit];
		}

		last_texture_data = (const std::uint8_t*)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

}

void HitHorizDoor()
{
	std::uint16_t texture;
	std::uint16_t doorpage = static_cast<std::uint16_t>(-1);
	std::uint16_t xint;
	bool lockable = true;

	int door_index = tilehit & 0x7F;

	if (doorobjlist[door_index].action == dr_jammed)
	{
		return;
	}

	xint = xintercept & 0xFFFF;

	if (xint > 0x7FFF)
	{
		texture = ((xint - (std::uint16_t)(doorposition[door_index] >> 1)) >> 4) & 0xFC0;
	}
	else
	{
		texture = ((xint + (std::uint16_t)(doorposition[door_index] >> 1)) >> 4) & 0xFC0;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
#if MASKABLE_DOORS
		ScaleMPost();
#else
		ScalePost();
#endif

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		if (lastside != -1) // if not the first scaled post
#if MASKABLE_DOORS
		{
			ScaleMPost();
		}
#else
		{
			ScalePost();
		}
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;

		switch (doorobjlist[door_index].type)
		{
		case dr_normal:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_METAL);
			break;

		case dr_elevator:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_ELEVATOR);
			break;

		case dr_prison:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_PRISON);
			break;

		case dr_space:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_SPACE);
			break;

		case dr_bio:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_BIO);
			break;

		case dr_high_security:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_HIGH_SECURITY); // Reverse View
			break;

		case dr_oneway_up:
		case dr_oneway_left:
			if (player->tiley > doorobjlist[door_index].tiley)
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + L_ENTER_ONLY); // normal view
			}
			else
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + NOEXIT); // Reverse View
				lockable = false;
			}
			break;

		case dr_oneway_right:
		case dr_oneway_down:
			if (player->tiley > doorobjlist[door_index].tiley)
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + NOEXIT); // normal view
				lockable = false;
			}
			else
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + L_ENTER_ONLY); // Reverse View
			}
			break;

		case dr_office:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_HIGH_TECH);
			break;
		}


		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[door_index].lock == kt_none)
		{
			doorpage += UL_METAL;
		}

		last_texture_data = (const std::uint8_t*)PM_GetPage(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}
}

void HitVertDoor()
{
	std::uint16_t texture;
	std::uint16_t doorpage = static_cast<std::uint16_t>(DOORWALL);
	std::uint16_t yint;
	bool lockable = true;

	int door_index = tilehit & 0x7F;

	if (doorobjlist[door_index].action == dr_jammed)
	{
		return;
	}

	yint = yintercept & 0xFFFF;
	if (yint > 0x7FFF)
	{
		texture = ((yint - (std::uint16_t)(doorposition[door_index] >> 1)) >> 4) & 0xFC0;
	}
	else
	{
		texture = ((yint + (std::uint16_t)(doorposition[door_index] >> 1)) >> 4) & 0xFC0;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
#if MASKABLE_DOORS
		ScaleMPost();
#else
		ScalePost();
#endif

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		if (lastside != -1) // if not the first scaled post
#if MASKABLE_DOORS
		{
			ScaleMPost();
		}
#else
		{
			ScalePost();
		}
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;

		switch (doorobjlist[door_index].type)
		{
		case dr_normal:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_METAL_SHADE);
			break;

		case dr_elevator:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_ELEVATOR_SHADE);
			break;

		case dr_prison:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_PRISON_SHADE);
			break;

		case dr_space:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_SPACE_SHADE);
			break;

		case dr_bio:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_BIO);
			break;

		case dr_high_security:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_HIGH_SECURITY_SHADE);
			break;

		case dr_oneway_left:
		case dr_oneway_up:
			if (player->tilex > doorobjlist[door_index].tilex)
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + L_ENTER_ONLY_SHADE); // Reverse View
			}
			else
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + NOEXIT_SHADE); // Normal view
				lockable = false;
			}
			break;

		case dr_oneway_right:
		case dr_oneway_down:
			if (player->tilex > doorobjlist[door_index].tilex)
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + NOEXIT_SHADE); // Reverse View
				lockable = false;
			}
			else
			{
				doorpage = static_cast<std::int16_t>(DOORWALL + L_ENTER_ONLY_SHADE); // Normal View
			}
			break;


		case dr_office:
			doorpage = static_cast<std::int16_t>(DOORWALL + L_HIGH_TECH_SHADE);
			break;

		}

		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[door_index].lock == kt_none)
		{
			doorpage += UL_METAL;
		}

		last_texture_data = (const std::uint8_t*)PM_GetPage(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}
}

/*
====================
=
= HitHorizPWall
=
= A pushable wall in action has been hit
=
====================
*/
void HitHorizPWall()
{
	std::int16_t wallpic;
	std::uint16_t texture, offset;

	texture = (xintercept >> 4) & 0xfc0;
	offset = pwallpos << 10;
	if (ytilestep == -1)
	{
		yintercept += TILEGLOBAL - offset;
	}
	else
	{
		texture = 0xfc0 - texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		ScalePost();

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		// new wall
		if (lastside != -1)
		{ // if not the first scaled post
			ScalePost();
		}

		lasttilehit = tilehit;
		postx = pixx;

		wallpic = horizwall[tilehit & 63];

		last_texture_data = (const std::uint8_t*)PM_GetPage(wallpic);
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
void HitVertPWall()
{
	std::int16_t wallpic;
	std::uint16_t texture, offset;

	texture = (yintercept >> 4) & 0xfc0;
	offset = pwallpos << 10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL - offset;
		texture = 0xfc0 - texture;
	}
	else
	{
		xintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		ScalePost();

		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];

		postx = pixx;
	}
	else
	{
		// new wall
		if (lastside != -1)
		{ // if not the first scaled post
			ScalePost();
		}

		lasttilehit = tilehit;
		postx = pixx;

		wallpic = vertwall[tilehit & 63];

		last_texture_data = (const std::uint8_t*)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

}

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
namespace
{


void vga_clear_screen(
	int y_offset,
	int height,
	int color)
{
	int pixel_offset = ::vl_get_offset(::bufferofs, 0, y_offset);

	if (::viewwidth == ::vga_width)
	{
		std::uninitialized_fill_n(
			&::vga_memory[pixel_offset],
			height * ::vga_width,
			static_cast<std::uint8_t>(color));
	}
	else
	{
		for (int y = 0; y < height; ++y)
		{
			std::uninitialized_fill_n(
				&::vga_memory[pixel_offset],
				::viewwidth,
				static_cast<std::uint8_t>(color));

			pixel_offset += ::vga_width;
		}
	}
}


} // namespace
// BBi


void VGAClearScreen()
{
	viewflags = gamestate.flags;

	int half_height = viewheight / 2;

	if ((viewflags & GS_DRAW_CEILING) == 0)
	{
		vga_clear_screen(0, half_height, TopColor);
	}

	if ((viewflags & GS_DRAW_FLOOR) == 0)
	{
		vga_clear_screen(
			viewheight - half_height, half_height, BottomColor);
	}
}

std::int16_t CalcRotate(
	const objtype* ob)
{
	dirtype dir = ob->dir;

	// this isn't exactly correct, as it should vary by a trig value,
	// but it is close enough with only eight rotations

	int view_angle = ::player->angle + ((::centerx - ob->viewx) / 8);

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(ob->trydir & 127);
	}

	int target_angle = (view_angle - 180) - dirangle[dir];

	target_angle += ANGLES / 16;

	while (target_angle >= ANGLES)
	{
		target_angle -= ANGLES;
	}

	while (target_angle < 0)
	{
		target_angle += ANGLES;
	}

	if ((ob->state->flags & SF_PAINFRAME) != 0)
	{ // 2 rotation pain frame
		return static_cast<std::int16_t>(4 * (target_angle / (ANGLES / 2))); // seperated by 3 (art layout...)

	}
	return static_cast<std::int16_t>(target_angle / (ANGLES / 8));
}



const int MAXVISABLE = 50;
visobj_t vislist[MAXVISABLE];
visobj_t* visptr;
visobj_t* visstep;
visobj_t* farthest;


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/
void DrawScaleds()
{
	std::int16_t i;
	std::int16_t least;
	std::int16_t numvisable;
	std::int16_t height;
	std::uint8_t* tilespot;
	std::uint8_t* visspot;
	std::uint16_t spotloc;

	statobj_t* statptr;
	objtype* obj;

	::visptr = &::vislist[0];

	//
	// place static objects
	//
	for (statptr = ::statobjlist; statptr != laststatobj; ++statptr)
	{
		::visptr->shapenum = statptr->shapenum;

		if (::visptr->shapenum == -1)
		{
			continue; // object has been deleted
		}

		if (!*statptr->visspot)
		{
			continue; // not visable
		}

		::TransformTile(statptr->tilex, statptr->tiley, &::visptr->viewx, &::visptr->viewheight);

		if (!::visptr->viewheight)
		{
			continue; // to close to the object
		}

		::visptr->cloaked = false;
		::visptr->lighting = statptr->lighting; // Could add additional flashing/lighting

		if (::visptr < &::vislist[MAXVISABLE - 1])
		{
			// don't let it overflow
			++visptr;
		}
	}

	//
	// place active objects
	//
	for (obj = ::player->next; obj; obj = obj->next)
	{
		if ((obj->flags & FL_OFFSET_STATES) != 0)
		{
			::visptr->shapenum = static_cast<std::int16_t>(obj->temp1 + obj->state->shapenum);

			if (visptr->shapenum == 0)
			{
				continue; // no shape
			}
		}
		else
		{
			visptr->shapenum = static_cast<std::int16_t>(obj->state->shapenum);

			if (::visptr->shapenum == 0)
			{
				continue; // no shape
			}
		}

		spotloc = (obj->tilex << 6) + obj->tiley; // optimize: keep in struct?

		// BBi Do not draw detonator if it's not visible.
		if (spotloc == 0)
		{
			continue;
		}

		visspot = &::spotvis[0][0] + spotloc;
		tilespot = &::tilemap[0][0] + spotloc;

		//
		// could be in any of the nine surrounding tiles
		//

		if (*visspot
			|| (*(visspot - 1) && !*(tilespot - 1))
			|| (*(visspot + 1) && !*(tilespot + 1))
			|| (*(visspot - 65) && !*(tilespot - 65))
			|| (*(visspot - 64) && !*(tilespot - 64))
			|| (*(visspot - 63) && !*(tilespot - 63))
			|| (*(visspot + 65) && !*(tilespot + 65))
			|| (*(visspot + 64) && !*(tilespot + 64))
			|| (*(visspot + 63) && !*(tilespot + 63)))
		{
			obj->active = ac_yes;

			::TransformActor(obj);

			if (!obj->viewheight)
			{
				continue; // too close or far away
			}

			const auto& assets_info = AssetsInfo{};

			if (assets_info.is_ps() &&
				(obj->flags2 & (FL2_CLOAKED | FL2_DAMAGE_CLOAK)) == FL2_CLOAKED)
			{
				::visptr->cloaked = true;
				::visptr->lighting = 0;
			}
			else
			{
				::visptr->cloaked = false;
				::visptr->lighting = obj->lighting;
			}

			if (assets_info.is_ps() && (obj->flags & FL_DEADGUY) == 0)
			{
				obj->flags2 &= ~FL2_DAMAGE_CLOAK;
			}

			::visptr->viewx = obj->viewx;
			::visptr->viewheight = obj->viewheight;

			if (::visptr->shapenum == -1)
			{
				::visptr->shapenum = obj->temp1; // special shape
			}

			if ((obj->state->flags & SF_ROTATE) != 0)
			{
				::visptr->shapenum += ::CalcRotate(obj);
			}

			if (::visptr < &::vislist[MAXVISABLE - 1])
			{
				// don't let it overflow
				++::visptr;
			}

			obj->flags |= FL_VISABLE;
		}
		else
		{
			obj->flags &= ~FL_VISABLE;
		}
	}

	//
	// draw from back to front
	//
	numvisable = static_cast<std::int16_t>(::visptr - &::vislist[0]);

	if (!numvisable)
	{
		return; // no visable objects
	}

	for (i = 0; i < numvisable; i++)
	{
		least = 32000;

		for (::visstep = &::vislist[0]; ::visstep < ::visptr; ++::visstep)
		{
			height = ::visstep->viewheight;

			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}

		//
		// Init our global flag...
		//
		::cloaked_shape = ::farthest->cloaked;

		//
		// draw farthest
		//
		if (((::gamestate.flags & GS_LIGHTING) != 0 && ::farthest->lighting != NO_SHADING) || cloaked_shape)
		{
			::ScaleLSShape(::farthest->viewx, ::farthest->shapenum, ::farthest->viewheight, ::farthest->lighting);
		}
		else
		{
			::ScaleShape(::farthest->viewx, ::farthest->shapenum, ::farthest->viewheight);
		}

		::farthest->viewheight = 32000;
	}

	::cloaked_shape = false;
}


using WeaponScale = std::vector<std::int16_t>;

WeaponScale weaponscale;

void initialize_weapon_constants()
{
	const auto& assets_info = AssetsInfo{};

	NUMWEAPONS = assets_info.is_ps() ? 7 : 6;

	weaponscale = {
		SPR_KNIFEREADY,
		SPR_PISTOLREADY,
		SPR_MACHINEGUNREADY,
		SPR_CHAINREADY,
		SPR_GRENADEREADY,
		SPR_BFG_WEAPON1,
		0,
	};
}


bool useBounceOffset = false;

void DrawPlayerWeapon()
{
	if (::playstate == ex_victorious)
	{
		return;
	}

	if (::gamestate.weapon != -1)
	{
		const auto shapenum =
			::weaponscale[static_cast<int>(::gamestate.weapon)] +
			::gamestate.weaponframe;

		if (shapenum != 0)
		{
			const auto& assets_info = AssetsInfo{};

			const auto height = assets_info.is_aog() ? 128 : 88;

			::useBounceOffset = assets_info.is_ps();
			::scale_player_weapon(shapenum, height);
			::useBounceOffset = false;
		}
	}
}

void CalcTics()
{
	std::int32_t newtime;

	//
	// calculate tics since last refresh for adaptive timing
	//
	if (static_cast<std::uint32_t>(lasttimecount) > TimeCount)
	{
		TimeCount = lasttimecount; // if the game was paused a LONG time


	}

	{
		//
		// non demo, so report actual time
		//
		do
		{
			newtime = TimeCount;
			auto diff = newtime - lasttimecount;
			if (diff <= 0)
			{
				tics = 0;
			}
			else
			{
				tics = static_cast<std::uint16_t>(diff);
			}
		} while (tics == 0); // make sure at least one tic passes

		lasttimecount = newtime;
		framecount++;

#ifdef FILEPROFILE
		strcpy(scratch, "\tTics:");
		itoa(tics, str, 10);
		strcat(scratch, str);
		strcat(scratch, "\n");
		write(profilehandle, scratch, strlen(scratch));
#endif

#ifdef DEBUGTICS
		VW_SetAtrReg(ATR_OVERSCAN, tics);
#endif

		realtics = tics;
		if (tics > MAXTICS)
		{
			TimeCount -= (tics - MAXTICS);
			tics = MAXTICS;
		}
	}
}

void WallRefresh()
{
	//
	// set up variables for this view
	//

	viewangle = player->angle;
	midangle = viewangle * (FINEANGLES / ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength, viewcos);
	viewy = player->y + FixedByFrac(focallength, viewsin);

	focaltx = viewx >> TILESHIFT;
	focalty = viewy >> TILESHIFT;

	viewtx = player->x >> TILESHIFT;
	viewty = player->y >> TILESHIFT;

	xpartialdown = viewx & (TILEGLOBAL - 1);
	xpartialup = static_cast<std::uint16_t>(TILEGLOBAL - xpartialdown);
	ypartialdown = viewy & (TILEGLOBAL - 1);
	ypartialup = static_cast<std::uint16_t>(TILEGLOBAL - ypartialdown);

	lastside = -1; // the first pixel is on a new wall

	AsmRefresh();
	ScalePost(); // no more optimization on last post
}


extern std::int16_t MsgTicsRemain;
extern std::uint16_t LastMsgPri;

void RedrawStatusAreas()
{
	::DrawInfoArea_COUNT = 3;
	::InitInfoArea_COUNT = 3;

	::LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	::ShadowPrintLocationText(sp_normal);

	::LatchDrawPic(0, 200 - STATUSLINES, STATUSBARPIC);
	::DrawAmmoPic();
	::DrawScoreNum();
	::DrawWeaponPic();
	::DrawAmmoNum();
	::DrawKeyPics();
	::DrawHealthNum();
}

void F_MapLSRow();
void C_MapLSRow();
void MapLSRow();

void ThreeDRefresh()
{
	::memset(::spotvis, 0, sizeof(::spotvis));

	::bufferofs = 0;

	::UpdateInfoAreaClock();
	::UpdateStatusBar();

	// BBi
	::vid_is_3d = true;

	::bufferofs += ::screenofs;

	//
	// follow the walls from there to the right, drawwing as we go
	//

	if ((::gamestate.flags & GS_LIGHTING) != 0)
	{
		switch (::gamestate.flags & (GS_DRAW_FLOOR | GS_DRAW_CEILING))
		{
		case GS_DRAW_FLOOR | GS_DRAW_CEILING:
			::MapRowPtr = ::MapLSRow;
			::WallRefresh();
			::DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			::MapRowPtr = ::F_MapLSRow;
			::VGAClearScreen();
			::WallRefresh();
			::DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			::MapRowPtr = ::C_MapLSRow;
			::VGAClearScreen();
			::WallRefresh();
			::DrawPlanes();
			break;

		default:
			::VGAClearScreen();
			::WallRefresh();
			break;
		}
	}
	else
	{
		switch (::gamestate.flags & (GS_DRAW_FLOOR | GS_DRAW_CEILING))
		{
		case GS_DRAW_FLOOR | GS_DRAW_CEILING:
			::MapRowPtr = ::MapRow;
			::WallRefresh();
			::DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			::MapRowPtr = ::F_MapRow;
			::VGAClearScreen();
			::WallRefresh();
			::DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			::MapRowPtr = ::C_MapRow;
			::VGAClearScreen();
			::WallRefresh();
			::DrawPlanes();
			break;

		default:
			::VGAClearScreen();
			::WallRefresh();
			break;
		}
	}

	::UpdateTravelTable();

	//
	// draw all the scaled images
	//
	::DrawScaleds(); // draw scaled stuf

	// BBi
	::vid_is_3d = false;

	::DrawPlayerWeapon(); // draw player's hands

//
// show screen and time last cycle
//
	if (::fizzlein)
	{
		::fizzlein = false;

		::vid_set_ui_mask_3d(false);

		bstone::GenericFizzleFX fizzle(BLACK, false);

		fizzle.initialize();

		static_cast<void>(fizzle.present(true));

		::lasttimecount = ::TimeCount; // don't make a big tic count
	}

	::bufferofs = 0;

	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		::DrawRadar();
	}

	// BBi
	::VL_RefreshScreen();

	::frameon++;
}

// !!! Used in saved game.
std::uint8_t TravelTable[MAPSIZE][MAPSIZE];

void UpdateTravelTable()
{
	for (int i = 0; i < MAPSIZE; ++i)
	{
		for (int j = 0; j < MAPSIZE; ++j)
		{
			TravelTable[i][j] |= spotvis[i][j];
		}
	}
}

void DrawRadar()
{
	std::int8_t zoom = gamestate.rzoom;
	std::uint8_t flags = OV_KEYS | OV_PUSHWALLS | OV_ACTORS;

	if (gamestate.rpower)
	{
		if ((frameon & 1) && (!godmode))
		{
			if (zoom)
			{
				gamestate.rpower -= tics << zoom;
			}
		}

		if (gamestate.rpower < 0)
		{
			gamestate.rpower = 0;
			DISPLAY_TIMED_MSG(RadarEnergyGoneMsg, MP_WEAPON_AVAIL, MT_GENERAL);
		}
		UpdateRadarGuage();
	}
	else
	{
		zoom = 0;
	}

	ShowOverhead(192, 156, 16, zoom, flags);
}

std::uint16_t tc_time;

static bool show_pwalls_on_automap(
	int x,
	int y)
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_ps())
	{
		return true;
	}

	static const int x_spots[8] = {
		-1, 0, 1, 1, 1, 0, -1, -1,
	}; // x_spots

	static const int y_spots[8] = {
		1, 1, 1, 0, -1, -1, -1, 0,
	}; // y_spots

	for (auto i = 0; i < 8; ++i)
	{
		auto new_x = x + x_spots[i];
		auto new_y = y + y_spots[i];

		if (new_x < 0 || new_x >= 64 || new_y < 0 || new_y >= 64)
		{
			continue;
		}

		auto iconnum = *(mapsegs[1] + farmapylookup[new_y] + new_x);

		if (iconnum == PUSHABLETILE)
		{
			continue;
		}

		if ((TravelTable[new_x][new_y] & TT_TRAVELED) != 0)
		{
			return true;
		}
	}

	return false;
}

void ShowOverhead(
	int bx,
	int by,
	int radius,
	int zoom,
	int flags)
{
	const auto& assets_info = AssetsInfo{};

	const std::uint8_t PWALL_COLOR = 0xF6;
	const std::uint8_t PLAYER_COLOR = 0xF0;
	const std::uint8_t UNMAPPED_COLOR = (assets_info.is_ps() ? 0x52 : 0x06);
	const std::uint8_t MAPPED_COLOR = 0x55;
	const std::uint8_t HIDDEN_COLOR = 0x52;

	bool snow = false;
	std::uint8_t rndindex = 0;
	bool drawplayerok = true;

	// -zoom == make it snow!
	//
	if (zoom < 0)
	{
		zoom = 0;
		snow = true;
		rndindex = static_cast<std::uint8_t>(US_RndT());
	}

	zoom = 1 << zoom;
	radius /= zoom;

	int player_angle = player->angle;
	int player_x = player->x;
	int player_y = player->y;

	if ((flags & OV_WHOLE_MAP) != 0)
	{
		player_angle = 90;
		player_x = ((std::int32_t)32 << TILESHIFT) + (TILEGLOBAL / 2);
		player_y = player_x;
	}

	// Get sin/cos values
	//
	int psin = sintable[player_angle];
	int pcos = costable[player_angle];

	// Convert radius to fixed integer and calc rotation.
	//
	int dx = radius << TILESHIFT;
	int dy = dx;

	int baselmx = player_x + (FixedByFrac(dx, pcos) - FixedByFrac(dy, psin));
	int baselmy = player_y - (FixedByFrac(dx, psin) + FixedByFrac(dy, pcos));

	// Carmack's sin/cos tables use one's complement for negative numbers --
	// convert it to two's complement!
	//
	if ((pcos & 0x80000000) != 0)
	{
		pcos = -(pcos & 0xFFFF);
	}

	if ((psin & 0x80000000) != 0)
	{
		psin = -(psin & 0xFFFF);
	}

	// Get x/y increment values.
	//
	int xinc = -pcos;
	int yinc = psin;

	int diameter = radius * 2;

	// Draw rotated radar.
	//

	for (int x = 0; x < diameter; ++x)
	{
		int lmx = baselmx;
		int lmy = baselmy;

		for (int y = 0; y < diameter; ++y)
		{
			std::uint8_t color = 0x00;
			bool go_to_draw = false;

			if (snow)
			{
				color = 0x42 + (rndtable[rndindex] & 3);
				rndindex++;
				go_to_draw = true;
			}

			// Don't evaluate if point is outside of map.
			//
			int mx = 0;
			int my = 0;

			if (!go_to_draw)
			{
				color = UNMAPPED_COLOR;
				mx = lmx >> 16;
				my = lmy >> 16;

				if (mx < 0 || mx > 63 || my < 0 || my > 63)
				{
					go_to_draw = true;
				}
			}

			// SHOW PLAYER
			//
			if (!go_to_draw &&
				drawplayerok &&
				player->tilex == mx &&
				player->tiley == my)
			{
				color = PLAYER_COLOR;
				drawplayerok = false;
			}
			else if (!go_to_draw)
			{
				// SHOW TRAVELED
				//
				if ((TravelTable[mx][my] & TT_TRAVELED) != 0 ||
					(flags & OV_SHOWALL) != 0)
				{
					// What's at this map location?
					//
					std::uint8_t tile = tilemap[mx][my];
					std::uint8_t door = tile & 0x3F;

					bool check_for_hidden_area = false;

					// Evaluate wall or floor?
					//
					if (tile != 0)
					{
						// SHOW DOORS
						//
						if ((tile & 0x80) != 0)
						{
							if (assets_info.is_aog() && doorobjlist[door].type == dr_elevator)
							{
								color = 0xFD;
							}
							else if (doorobjlist[door].lock != kt_none)
							{
								color = 0x18; // locked!
							}
							else
							{
								if (doorobjlist[door].action == dr_closed)
								{
									color = 0x58; // closed!
								}
								else
								{
									color = MAPPED_COLOR; // floor!
									check_for_hidden_area = assets_info.is_aog();
								}
							}
						}
					}
					else
					{
						color = MAPPED_COLOR; // floor!
						check_for_hidden_area = assets_info.is_aog();
					}

					if (check_for_hidden_area)
					{
						static_cast<void>(::GetAreaNumber(mx, my));

						if (::GAN_HiddenArea)
						{
							color = HIDDEN_COLOR;
						}
					}

					// SHOW KEYS
					//
					if ((flags & OV_KEYS) != 0 &&
						(TravelTable[mx][my] & TT_KEYS) != 0)
					{
						color = 0xF3;
					}

					if ((ExtraRadarFlags & OV_ACTORS) != 0 ||
						(assets_info.is_ps() && zoom > 1 && (flags & OV_ACTORS) != 0))
					{
						const auto ob = actorat[mx][my];

						// SHOW ACTORS
						//
						if (ob >= objlist &&
							(ob->flags & FL_DEADGUY) == 0 &&
							ob->obclass > deadobj &&
							ob->obclass < SPACER1_OBJ)
						{
							color = static_cast<std::uint8_t>(0x10 + ob->obclass);
						}
					}

					if ((ExtraRadarFlags & OV_PUSHWALLS) != 0 ||
						(assets_info.is_ps() && zoom == 4 && (flags & OV_PUSHWALLS) != 0))
					{
						auto iconnum = *(mapsegs[1] + farmapylookup[my] + mx);

						// SHOW PUSHWALLS
						//
						if (iconnum == PUSHABLETILE)
						{
							if (::show_pwalls_on_automap(mx, my))
							{
								color = (assets_info.is_aog() ? PWALL_COLOR : 0x79);
							}
						}
					}
				}
				else
				{
					color = UNMAPPED_COLOR;
				}
			}

			VL_Bar(bx + (x * zoom), by + (y * zoom), zoom, zoom, color);

			lmx += xinc;
			lmy += yinc;
		}

		baselmx += yinc;
		baselmy -= xinc;
	}
}

int door_get_track_texture_id(
	const doorobj_t& door)
{
	auto result = DOORWALL;

	if (door.vertical)
	{
		result += ::DoorJamsShade[door.type];
	}
	else
	{
		result += ::DoorJams[door.type];
	}

	return result;
}

void door_get_page_numbers_for_caching(
	const doorobj_t& door,
	int& horizontal_locked_page_number,
	int& horizontal_unlocked_page_number,
	int& vertical_locked_page_number,
	int& vertical_unlocked_page_number)
{
	switch (door.type)
	{
	case dr_normal:
		horizontal_locked_page_number = DOORWALL + L_METAL;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_METAL_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_elevator:
		horizontal_locked_page_number = DOORWALL + L_ELEVATOR;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_ELEVATOR_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_prison:
		horizontal_locked_page_number = DOORWALL + L_PRISON;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_PRISON_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_space:
		horizontal_locked_page_number = DOORWALL +  L_SPACE;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_SPACE_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_bio:
		horizontal_locked_page_number = DOORWALL + L_BIO;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_BIO_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_high_security:
		horizontal_locked_page_number = DOORWALL + L_HIGH_SECURITY;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_HIGH_SECURITY_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_office:
		horizontal_locked_page_number = DOORWALL + L_HIGH_TECH;
		horizontal_unlocked_page_number = horizontal_locked_page_number + UL_METAL;

		vertical_locked_page_number = DOORWALL + L_HIGH_TECH_SHADE;
		vertical_unlocked_page_number = vertical_locked_page_number + UL_METAL;

		break;

	case dr_oneway_up:
	case dr_oneway_left:
	case dr_oneway_right:
	case dr_oneway_down:
		horizontal_locked_page_number = DOORWALL + NOEXIT;
		horizontal_unlocked_page_number = DOORWALL + L_ENTER_ONLY;

		vertical_locked_page_number = DOORWALL + NOEXIT_SHADE;
		vertical_unlocked_page_number = DOORWALL + L_ENTER_ONLY_SHADE;

		break;

	default:
		horizontal_locked_page_number = 0;
		horizontal_unlocked_page_number = 0;

		vertical_locked_page_number = 0;
		vertical_unlocked_page_number = 0;

		::Quit("Invalid door type.");

		break;
	}
}

void door_get_page_numbers(
	const doorobj_t& door,
	int& front_face_page_number,
	int& back_face_page_number)
{
	const auto is_unlocked = (door.lock == kt_none);

	auto is_lockable = true;

	front_face_page_number = DOORWALL;
	back_face_page_number = DOORWALL;

	switch (door.type)
	{
	case dr_normal:
		front_face_page_number += (door.vertical ? L_METAL_SHADE : L_METAL);

		break;

	case dr_elevator:
		front_face_page_number += (door.vertical ? L_ELEVATOR_SHADE : L_ELEVATOR);

		break;

	case dr_prison:
		front_face_page_number += (door.vertical ? L_PRISON_SHADE : L_PRISON);

		break;

	case dr_space:
		front_face_page_number += (door.vertical ? L_SPACE_SHADE : L_SPACE);

		break;

	case dr_bio:
		front_face_page_number += (door.vertical ? L_BIO_SHADE : L_BIO);

		break;

	case dr_high_security:
		front_face_page_number += (door.vertical ? L_HIGH_SECURITY_SHADE : L_HIGH_SECURITY);

		break;

	case dr_office:
		front_face_page_number += (door.vertical ? L_HIGH_TECH_SHADE : L_HIGH_TECH);

		break;

	case dr_oneway_up:
		is_lockable = false;

		front_face_page_number += L_ENTER_ONLY;
		back_face_page_number += NOEXIT;

		break;

	case dr_oneway_left:
		is_lockable = false;

		front_face_page_number += NOEXIT_SHADE;
		back_face_page_number += L_ENTER_ONLY_SHADE;

		break;

	case dr_oneway_right:
		is_lockable = false;

		front_face_page_number += L_ENTER_ONLY_SHADE;
		back_face_page_number += NOEXIT_SHADE;

		break;

	case dr_oneway_down:
		is_lockable = false;

		front_face_page_number += NOEXIT;
		back_face_page_number += L_ENTER_ONLY;

		break;

	default:
		front_face_page_number = 0;
		back_face_page_number = 0;

		::Quit("Invalid door type.");

		break;
	}

	if (is_lockable)
	{
		if (is_unlocked)
		{
			front_face_page_number += UL_METAL;
		}

		back_face_page_number = front_face_page_number;
	}
}

int actor_calculate_rotation(
	const objtype& actor)
{
	return ::CalcRotate(&actor);
}

int player_get_weapon_sprite_id()
{
	if (::playstate == ex_victorious || ::gamestate.weapon == -1)
	{
		return 0;
	}

	return
		::weaponscale[static_cast<std::intptr_t>(::gamestate.weapon)] +
		::gamestate.weaponframe;
}
