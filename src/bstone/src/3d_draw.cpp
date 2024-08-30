/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cassert>
#include <cstring>

#include <algorithm>

#include "gfxv.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_algorithm.h"
#include "bstone_door.h"
#include "bstone_generic_fizzle_fx.h"
#include "bstone_globals.h"
#include "bstone_math.h"


#define MASKABLE_DOORS (0)
#define MASKABLE_POSTS (MASKABLE_DOORS)


namespace
{


//
// Door Objects
//

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

int door_get_page_base_index()
{
	return bstone::globals::page_mgr->get_wall_count() - NUMDOORTYPES;
}

constexpr auto ACTORSIZE = bstone::math::fixed_to_floating(0x4000);


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

std::int32_t lasttimecount;
std::int32_t frameon;
std::int32_t framecount;

WallHeight wallheight;


//
// math tables
//
std::vector<int> pixelangle;
FineTangent finetangent;
double sintable[ANGLES + (ANGLES / 4) + 1];
double* costable = &sintable[ANGLES / 4];

//
// refresh variables
//
double viewx;
double viewy; // the focal point
int viewangle;
double viewsin;
double viewcos;

char thetile[64];
std::uint8_t* mytile;


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
double xpartialup;
double xpartialdown;
double ypartialup;
double ypartialdown;
int xinttile;
int yinttile;

int tilehit;
int pixx;

int xtile;
int ytile;
int xtilestep;
int ytilestep;
double xintercept;
double yintercept;

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
	//
	// translate point to view centered coordinates
	//
	const auto gx = ob->x - viewx;
	const auto gy = ob->y - viewy;

	//
	// calculate newx
	//
	const auto gxt1 = gx * viewcos;
	const auto gyt1 = gy * viewsin;
	const auto nx = gxt1 - gyt1 - ACTORSIZE; // fudge the shape forward a bit, because
	// the midpoint could put parts of the shape
	// into an adjacent wall

	//
	// calculate newy
	//
	const auto gxt2 = gx * viewsin;
	const auto gyt2 = gy * viewcos;
	const auto ny = gyt2 + gxt2;

	//
	// calculate perspective ratio
	//
	if (nx < MINDIST)
	{
		// too close, don't overflow the divide
		ob->viewheight = 0;
		return;
	}

	ob->viewx = static_cast<std::int16_t>(centerx + (ny * scale_ / nx));
	ob->viewheight = static_cast<std::uint16_t>(256.0 * heightnumerator / nx);
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
	//
	// translate point to view centered coordinates
	//
	const auto gx = tx - viewx + 0.5;
	const auto gy = ty - viewy + 0.5;

	//
	// calculate newx
	//
	constexpr auto object_size = bstone::math::fixed_to_floating(0x2000);

	auto gxt = gx * viewcos;
	auto gyt = gy * viewsin;
	const auto nx = gxt - gyt - object_size;

	//
	// calculate newy
	//
	gxt = gx * viewsin;
	gyt = gy * viewcos;
	const auto ny = gyt + gxt;


	//
	// calculate perspective ratio
	//
	if (nx < MINDIST)
	{
		// too close, don't overflow the divide
		*dispheight = 0;

		return;
	}

	*dispx = static_cast<std::int16_t>(centerx + (ny * scale_ / nx));
	*dispheight = static_cast<std::int16_t>(256.0 * heightnumerator / nx);
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
double CalcHeight()
{
	const auto gx = xintercept - viewx;
	const auto gxt = gx * viewcos;

	const auto gy = yintercept - viewy;
	const auto gyt = gy * viewsin;

	constexpr auto min_result = 8.0;
	const auto nx = std::max(gxt - gyt, min_fixed_floating);
	const auto result = std::max((256.0 * heightnumerator) / nx, min_result);

	return result;
}


const std::uint8_t* postsource;
int postx;

// BBi
int posty;

double postheight;
const std::uint8_t* shadingtable;
extern const std::uint8_t* lightsource;

void ScalePost()
{
	if (vid_is_hw())
	{
		return;
	}

	const auto height = wallheight[postx] / 8.0;

	postheight = height;

	if (!gp_no_shading())
	{
		auto i = shade_max - ((63 * height) / normalshade);

		if (i < 0.0)
		{
			i = 0.0;
		}
		else if (i > 63.0)
		{
			i = 63.0;
		}

		shadingtable = lightsource + (static_cast<std::ptrdiff_t>(i) * 256);

		DrawLSPost();
	}
	else
	{
		DrawPost();
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

// tilehit bit 7 is 0, because it's not a door tile
// if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
void HitVertWall()
{
	auto texture_d = get_fractional(yintercept);

	if (xtilestep < 0)
	{
		texture_d = 1.0 - texture_d;
		xintercept += 1.0;
	}

	const auto texture = static_cast<int>(texture_d * 4'096.0) & 0xFC0;

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

		if (lastside >= 0)
		{
			// if not the first scaled post
			ScalePost();
		}

		lastside = 1;
		lastintercept = xtile;
		lasttilehit = tilehit;
		postx = pixx;

		auto wallpic = 0;

		if ((tilehit & 0x40) != 0)
		{
			// check for adjacent doors
			//

			ytile = static_cast<int>(yintercept);

			const auto door_index = tilemap[xtile - xtilestep][ytile];

			if ((door_index & 0x80) != 0 && (door_index & 0xC0) != 0xC0)
			{
				const auto& door = doorobjlist[door_index & 0x3F];
				wallpic = DOORWALL + DoorJamsShade[door.type];
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

		last_texture_data = bstone::globals::page_mgr->get(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_wall_render_item(xtile, ytile);
}

// tilehit bit 7 is 0, because it's not a door tile
// if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
void HitHorizWall()
{
	auto texture_d = get_fractional(xintercept);

	if (ytilestep < 0)
	{
		yintercept += 1.0;
	}
	else
	{
		texture_d = 1.0 - texture_d;
	}

	const auto texture = static_cast<int>(texture_d * 4'096.0) & 0xFC0;

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
		if (lastside >= 0)
		{
			// if not the first scaled post
			ScalePost();
		}

		lastside = 0;
		lastintercept = ytile;
		lasttilehit = tilehit;
		postx = pixx;

		auto wallpic = 0;

		if ((tilehit & 0x40) != 0)
		{
			// check for adjacent doors

			xtile = static_cast<int>(xintercept);

			auto door_index = tilemap[xtile][ytile - ytilestep];

			if ((door_index & 0x80) != 0 && (door_index & 0xC0) != 0xC0)
			{
				const auto& door = doorobjlist[door_index & 0x3F];
				wallpic = DOORWALL + DoorJams[door.type];
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

		last_texture_data = bstone::globals::page_mgr->get(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_wall_render_item(xtile, ytile);
}

static int get_door_page_number(
	const int door_index,
	const bool is_vertical)
{
	auto doorpage = DOORWALL + is_vertical;
	auto lockable = true;

	const auto is_forward =
		(!is_vertical && player->tiley > doorobjlist[door_index].tiley) ||
		(is_vertical && player->tilex > doorobjlist[door_index].tilex);

	switch (doorobjlist[door_index].type)
	{
		case dr_normal:
			doorpage += L_METAL;
			break;

		case dr_elevator:
			doorpage += L_ELEVATOR;
			break;

		case dr_prison:
			doorpage += L_PRISON;
			break;

		case dr_space:
			doorpage += L_SPACE;
			break;

		case dr_bio:
			doorpage += L_BIO;
			break;

		case dr_high_security:
			doorpage += L_HIGH_SECURITY;
			break;

		case dr_oneway_up:
		case dr_oneway_left:
			if (is_forward)
			{
				doorpage += L_ENTER_ONLY;
			}
			else
			{
				doorpage += NOEXIT;
				lockable = false;
			}

			break;

		case dr_oneway_right:
		case dr_oneway_down:
			if (is_forward)
			{
				doorpage += NOEXIT;
				lockable = false;
			}
			else
			{
				doorpage += L_ENTER_ONLY;
			}

			break;

		case dr_office:
			doorpage += L_HIGH_TECH;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported door type.");
	}


	//
	// If door is unlocked, Inc shape ptr to unlocked door shapes
	//

	if (lockable && doorobjlist[door_index].lock == kt_none)
	{
		doorpage += UL_METAL;
	}

	return doorpage;
}

namespace {

bool bs_is_tile_solid_wall(int tile) noexcept
{
	return
		tile != 0 &&
		((tile & tilemap_door_flags) == 0 || (tile & tilemap_door_flags) == tilemap_door_track_flag);
}

} // namespace

void HitHorizDoor()
{
	const auto door_index = tilehit & 0x3F;
	const auto& bs_door = doorobjlist[door_index];

	// Ensure to render solid walls on both sides of the door (#485).

	if (bs_is_tile_solid_wall(tilemap[bs_door.tilex - 1][bs_door.tiley]))
	{
		vid_hw_add_wall_render_item(bs_door.tilex - 1, bs_door.tiley);
	}

	if (bs_is_tile_solid_wall(tilemap[bs_door.tilex + 1][bs_door.tiley]))
	{
		vid_hw_add_wall_render_item(bs_door.tilex + 1, bs_door.tiley);
	}

	if (doorobjlist[door_index].action == dr_jammed)
	{
		return;
	}

	const auto door_half_position = 0.5 * doorposition[door_index];

	auto texture_d = get_fractional(xintercept);

	if (texture_d > 0.5)
	{
		texture_d -= door_half_position;
	}
	else
	{
		texture_d += door_half_position;
	}

	texture_d *= 4'096.0;

	const auto texture = static_cast<int>(texture_d) & 0xFC0;

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
		if (lastside >= 0) // if not the first scaled post
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

		const auto doorpage = get_door_page_number(door_index, false);

		last_texture_data = bstone::globals::page_mgr->get(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_door_render_item(bs_door.tilex, bs_door.tiley);
}

void HitVertDoor()
{
	const auto door_index = tilehit & 0x3F;
	const auto& bs_door = doorobjlist[door_index];

	// Ensure to render solid walls on both sides of the door (#485).

	if (bs_is_tile_solid_wall(tilemap[bs_door.tilex][bs_door.tiley - 1]))
	{
		vid_hw_add_wall_render_item(bs_door.tilex, bs_door.tiley - 1);
	}

	if (bs_is_tile_solid_wall(tilemap[bs_door.tilex][bs_door.tiley + 1]))
	{
		vid_hw_add_wall_render_item(bs_door.tilex, bs_door.tiley + 1);
	}

	if (doorobjlist[door_index].action == dr_jammed)
	{
		return;
	}

	const auto door_half_position = 0.5 * doorposition[door_index];

	auto texture_d = get_fractional(yintercept);

	if (texture_d > 0.5)
	{
		texture_d -= door_half_position;
	}
	else
	{
		texture_d += door_half_position;
	}

	texture_d *= 4'096.0;

	const auto texture = static_cast<int>(texture_d) & 0xFC0;

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
		if (lastside >= 0) // if not the first scaled post
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

		const auto doorpage = get_door_page_number(door_index, true);

		last_texture_data = bstone::globals::page_mgr->get(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_door_render_item(bs_door.tilex, bs_door.tiley);
}

// A pushable wall in action has been hit
void HitHorizPWall()
{
	auto texture_d = get_fractional(xintercept);

	if (ytilestep < 0)
	{
		yintercept += 1.0 - pwallpos;
	}
	else
	{
		texture_d = 1.0 - texture_d;
		yintercept += pwallpos;
	}

	const auto texture = static_cast<int>(texture_d * 4'096.0) & 0xFC0;

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
		if (lastside >= 0)
		{
			// if not the first scaled post
			ScalePost();
		}

		lasttilehit = tilehit;
		postx = pixx;

		const auto wallpic = horizwall[tilehit & 63];
		last_texture_data = bstone::globals::page_mgr->get(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_pushwall_render_item(pwallx, pwally);
}

// A pushable wall in action has been hit
void HitVertPWall()
{
	auto texture_d = get_fractional(yintercept);

	if (xtilestep < 0)
	{
		xintercept += 1.0 - pwallpos;
		texture_d = 1.0 - texture_d;
	}
	else
	{
		xintercept += pwallpos;
	}

	const auto texture = static_cast<int>(texture_d * 4'096.0) & 0xFC0;

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
		if (lastside >= 0)
		{
			// if not the first scaled post
			ScalePost();
		}

		lasttilehit = tilehit;
		postx = pixx;

		const auto wallpic = vertwall[tilehit & 63];
		last_texture_data = bstone::globals::page_mgr->get(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

	vid_hw_add_pushwall_render_item(pwallx, pwally);
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
	int pixel_offset = vl_get_offset(bufferofs, 0, y_offset);

	if (viewwidth == vga_width)
	{
		std::fill_n(
			&vga_memory[pixel_offset],
			height * vga_width,
			static_cast<std::uint8_t>(color));
	}
	else
	{
		for (int y = 0; y < height; ++y)
		{
			std::fill_n(
				&vga_memory[pixel_offset],
				viewwidth,
				static_cast<std::uint8_t>(color));

			pixel_offset += vga_width;
		}
	}
}


} // namespace
// BBi


void VGAClearScreen()
{
	if (vid_is_hw())
	{
		return;
	}

	viewflags = gamestate.flags;

	int half_height = viewheight / 2;

	if (gp_is_ceiling_solid())
	{
		vga_clear_screen(0, half_height, TopColor);
	}

	if (gp_is_flooring_solid())
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

	int view_angle = player->angle + ((centerx - ob->viewx) / 8);

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(bstone::clamp(ob->trydir & 127, 0, 8));
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
		return static_cast<std::int16_t>(4 * (target_angle / (ANGLES / 2))); // separated by 3 (art layout...)

	}
	return static_cast<std::int16_t>(target_angle / (ANGLES / 8));
}



const int MAXVISIBLE = 50;
visobj_t vislist[MAXVISIBLE];
visobj_t* visptr;
visobj_t* visstep;
visobj_t* farthest;


namespace
{


void hw_draw_sprites()
{
	//
	// place static objects
	//

	vid_hw_clear_static_render_list();

	for (auto statptr = statobjlist.data(); statptr != laststatobj; ++statptr)
	{
		if (statptr->shapenum == -1)
		{
			continue; // object has been deleted
		}

		if (!(*statptr->visspot))
		{
			continue; // not visable
		}

		auto dispx = int16_t{};
		auto dispheight = int16_t{};

		TransformTile(statptr->tilex, statptr->tiley, &dispx, &dispheight);

		if (dispheight <= 0)
		{
			continue; // to close to the object
		}

		const auto bs_static_index = static_cast<int>(statptr - statobjlist.data());
		vid_hw_add_static_render_item(bs_static_index);
	}


	//
	// place active objects
	//

	vid_hw_clear_actor_render_list();

	const auto& assets_info = get_assets_info();

	for (auto obj = player->next; obj; obj = obj->next)
	{
		if ((obj->flags & FL_OFFSET_STATES) != 0)
		{
			const auto shapenum = obj->temp1 + obj->state->shapenum;

			if (shapenum == 0)
			{
				continue; // no shape
			}
		}
		else
		{
			const auto shapenum = obj->state->shapenum;

			if (shapenum == 0)
			{
				continue; // no shape
			}
		}

		const auto spotloc = (obj->tilex << 6) + obj->tiley; // optimize: keep in struct?

		// BBi Do not draw detonator if it's not visible.
		if (spotloc == 0)
		{
			continue;
		}

		const auto visspot = &spotvis[0][0] + spotloc;
		const auto tilespot = &tilemap[0][0] + spotloc;

		//
		// could be in any of the nine surrounding tiles
		//

		if (*visspot ||
			(visspot[-1] && !tilespot[-1]) ||
			(visspot[+1] && !tilespot[+1]) ||
			(visspot[-65] && !tilespot[-65]) ||
			(visspot[-64] && !tilespot[-64]) ||
			(visspot[-63] && !tilespot[-63]) ||
			(visspot[+65] && !tilespot[+65]) ||
			(visspot[+64] && !tilespot[+64]) ||
			(visspot[+63] && !tilespot[+63]))
		{
			obj->active = ac_yes;

			TransformActor(obj);

			if (obj->viewheight <= 0)
			{
				continue; // too close or far away
			}

			if (assets_info.is_ps() && (obj->flags & FL_DEADGUY) == 0)
			{
				obj->flags2 &= ~FL2_DAMAGE_CLOAK;
			}

			obj->flags |= FL_VISIBLE;
		}
		else
		{
			obj->flags &= ~FL_VISIBLE;
		}

		const auto bs_actor_index = static_cast<int>(obj - objlist);
		vid_hw_add_actor_render_item(bs_actor_index);
	}

	cloaked_shape = false;
}


} // namespace


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visible
=
=====================
*/
void DrawScaleds()
{
	if (vid_is_hw())
	{
		hw_draw_sprites();

		return;
	}

	std::int16_t i;
	std::int16_t least;
	std::int16_t numvisible;
	std::int16_t height;
	std::uint8_t* tilespot;
	std::uint8_t* visspot;
	std::uint16_t spotloc;

	statobj_t* statptr;
	objtype* obj;

	visptr = &vislist[0];

	//
	// place static objects
	//
	for (statptr = statobjlist.data(); statptr != laststatobj; ++statptr)
	{
		visptr->shapenum = statptr->shapenum;

		if (visptr->shapenum == -1)
		{
			continue; // object has been deleted
		}

		if (!*statptr->visspot)
		{
			continue; // not visible
		}

		TransformTile(statptr->tilex, statptr->tiley, &visptr->viewx, &visptr->viewheight);

		if (visptr->viewheight <= 0)
		{
			continue; // to close to the object
		}

		visptr->cloaked = false;
		visptr->lighting = statptr->lighting; // Could add additional flashing/lighting

		if (visptr < &vislist[MAXVISIBLE - 1])
		{
			// don't let it overflow
			++visptr;
		}
	}

	//
	// place active objects
	//
	for (obj = player->next; obj; obj = obj->next)
	{
		if ((obj->flags & FL_OFFSET_STATES) != 0)
		{
			visptr->shapenum = static_cast<std::int16_t>(obj->temp1 + obj->state->shapenum);

			if (visptr->shapenum == 0)
			{
				continue; // no shape
			}
		}
		else
		{
			visptr->shapenum = static_cast<std::int16_t>(obj->state->shapenum);

			if (visptr->shapenum == 0)
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

		visspot = &spotvis[0][0] + spotloc;
		tilespot = &tilemap[0][0] + spotloc;

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

			TransformActor(obj);

			if (obj->viewheight <= 0)
			{
				continue; // too close or far away
			}

			const auto& assets_info = get_assets_info();

			if (assets_info.is_ps() &&
				(obj->flags2 & (FL2_CLOAKED | FL2_DAMAGE_CLOAK)) == FL2_CLOAKED)
			{
				visptr->cloaked = true;
				visptr->lighting = 0;
			}
			else
			{
				visptr->cloaked = false;
				visptr->lighting = obj->lighting;
			}

			if (assets_info.is_ps() && (obj->flags & FL_DEADGUY) == 0)
			{
				obj->flags2 &= ~FL2_DAMAGE_CLOAK;
			}

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;

			if (visptr->shapenum == -1)
			{
				visptr->shapenum = obj->temp1; // special shape
			}

			if ((obj->state->flags & SF_ROTATE) != 0)
			{
				visptr->shapenum += CalcRotate(obj);
			}

			if (visptr < &vislist[MAXVISIBLE - 1])
			{
				// don't let it overflow
				++visptr;
			}

			obj->flags |= FL_VISIBLE;
		}
		else
		{
			obj->flags &= ~FL_VISIBLE;
		}
	}

	//
	// draw from back to front
	//
	numvisible = static_cast<std::int16_t>(visptr - &vislist[0]);

	if (!numvisible)
	{
		return; // no visible objects
	}

	for (i = 0; i < numvisible; i++)
	{
		least = 32000;

		for (visstep = &vislist[0]; visstep < visptr; ++visstep)
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
		if ((!gp_no_shading() && farthest->lighting != NO_SHADING) || cloaked_shape)
		{
			ScaleLSShape(farthest->viewx, farthest->shapenum, farthest->viewheight, farthest->lighting);
		}
		else
		{
			ScaleShape(farthest->viewx, farthest->shapenum, farthest->viewheight);
		}

		farthest->viewheight = 32000;
	}

	cloaked_shape = false;
}


using WeaponScale = std::vector<std::int16_t>;

WeaponScale weaponscale;

void initialize_weapon_constants()
{
	const auto& assets_info = get_assets_info();

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
	if (vid_is_hw())
	{
		return;
	}

	if (playstate == ex_victorious)
	{
		return;
	}

	if (gamestate.weapon != -1)
	{
		const auto shapenum =
			weaponscale[static_cast<int>(gamestate.weapon)] +
			gamestate.weaponframe;

		if (shapenum != 0)
		{
			const auto& assets_info = get_assets_info();

			const auto height = assets_info.is_aog() ? 128 : 88;

			useBounceOffset = !gp_no_weapon_bobbing() && assets_info.is_ps();
			scale_player_weapon(shapenum, height);
			useBounceOffset = false;
		}
	}
}

void CalcTics()
{
	std::int32_t newtime;

	//
	// calculate tics since last refresh for adaptive timing
	//
	if (lasttimecount > TimeCount)
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
	viewx = player->x - (focallength * viewcos);
	viewy = player->y + (focallength * viewsin);

	focaltx = static_cast<int>(viewx);
	focalty = static_cast<int>(viewy);

	viewtx = static_cast<int>(player->x);
	viewty = static_cast<int>(player->y);

	xpartialdown = get_fractional(viewx);
	xpartialup = 1.0 - xpartialdown;
	ypartialdown = get_fractional(viewy);
	ypartialup = 1.0 - ypartialdown;

	lastside = -1; // the first pixel is on a new wall

	AsmRefresh();
	ScalePost(); // no more optimization on last post
}


extern std::int16_t MsgTicsRemain;
extern std::uint16_t LastMsgPri;

void RedrawStatusAreas()
{
	DrawInfoArea_COUNT = 3;
	InitInfoArea_COUNT = 3;

	LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	ShadowPrintLocationText(sp_normal);

	LatchDrawPic(0, 200 - STATUSLINES, STATUSBARPIC);
	DrawAmmoPic();
	DrawScoreNum();
	DrawWeaponPic();
	DrawAmmoNum();
	DrawKeyPics();
	DrawHealthNum();
}

void F_MapLSRow();
void C_MapLSRow();
void MapLSRow();

void ThreeDRefresh()
{
	spotvis = SpotVis{};

	bufferofs = 0;

	UpdateInfoAreaClock();
	UpdateStatusBar();

	// BBi
	vid_is_3d = true;

	bufferofs += screenofs;

	//
	// follow the walls from there to the right, drawwing as we go
	//

	const auto is_ceiling_textured = (!gp_is_ceiling_solid());
	const auto is_floor_textured = (!gp_is_flooring_solid());

	if (!gp_no_shading())
	{
		if (is_ceiling_textured && is_floor_textured)
		{
			MapRowPtr = MapLSRow;
			WallRefresh();
			DrawPlanes();
		}
		else if (!is_ceiling_textured && is_floor_textured)
		{
			MapRowPtr = F_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
		}
		else if (is_ceiling_textured && !is_floor_textured)
		{
			MapRowPtr = C_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
		}
		else
		{
			VGAClearScreen();
			WallRefresh();
		}
	}
	else
	{
		if (is_ceiling_textured && is_floor_textured)
		{
			MapRowPtr = MapRow;
			WallRefresh();
			DrawPlanes();
		}
		else if (!is_ceiling_textured && is_floor_textured)
		{
			MapRowPtr = F_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
		}
		else if (is_ceiling_textured && !is_floor_textured)
		{
			MapRowPtr = C_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
		}
		else
		{
			VGAClearScreen();
			WallRefresh();
		}
	}

	UpdateTravelTable();

	//
	// draw all the scaled images
	//
	DrawScaleds(); // draw scaled stuf

	// BBi
	vid_is_3d = false;

	DrawPlayerWeapon(); // draw player's hands

//
// show screen and time last cycle
//
	if (fizzlein)
	{
		fizzlein = false;

		vid_set_ui_mask_3d(false);

		bstone::GenericFizzleFX fizzle(BLACK, false);

		fizzle.initialize(gp_vanilla_fizzle_fx());

		static_cast<void>(fizzle.present());

		lasttimecount = TimeCount; // don't make a big tic count
	}

	bufferofs = 0;

	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		DrawRadar();
	}

	// BBi
	VL_RefreshScreen();

	frameon++;
}

// !!! Used in saved game.
TravelTable travel_table_;

void UpdateTravelTable()
{
	for (int i = 0; i < MAPSIZE; ++i)
	{
		for (int j = 0; j < MAPSIZE; ++j)
		{
			travel_table_[i][j] |= spotvis[i][j];
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
	const int x,
	const int y,
	const bool is_show_all)
{
	if (tilemap[x][y] == 0)
	{
		return false;
	}

	if (is_show_all)
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

		const auto iconnum = mapsegs[1][farmapylookup[new_y] + new_x];

		if (iconnum == PUSHABLETILE)
		{
			continue;
		}

		if ((travel_table_[new_x][new_y] & TT_TRAVELED) != 0)
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
	const auto& assets_info = get_assets_info();

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
	auto player_x = player->x;
	auto player_y = player->y;

	if ((flags & OV_WHOLE_MAP) != 0)
	{
		player_angle = 90;
		player_x = 32.5;
		player_y = player_x;
	}

	// Get sin/cos values
	//
	const auto psin = sintable[player_angle];
	const auto pcos = costable[player_angle];

	// Convert radius to fixed integer and calc rotation.
	//
	const auto dx = static_cast<double>(radius);
	const auto dy = dx;

	auto baselmx = player_x + ((dx * pcos) - (dy * psin));
	auto baselmy = player_y - ((dx * psin) + (dy * pcos));

	// Get x/y increment values.
	//
	const auto xinc = -pcos;
	const auto yinc = psin;

	const auto diameter = static_cast<int>(radius * 2.0);

	const auto is_show_all = ((flags & OV_SHOWALL) != 0);

	// Draw rotated radar.
	//

	for (auto x = 0; x < diameter; ++x)
	{
		auto lmx = baselmx;
		auto lmy = baselmy;

		for (auto y = 0; y < diameter; ++y)
		{
			std::uint8_t color = 0x00;
			auto go_to_draw = false;

			if (snow)
			{
				color = 0x42 + (rndtable[rndindex] & 3);
				rndindex += 1;
				go_to_draw = true;
			}

			// Don't evaluate if point is outside of map.
			//
			auto mx = 0;
			auto my = 0;

			if (!go_to_draw)
			{
				color = UNMAPPED_COLOR;
				mx = static_cast<int>(lmx);
				my = static_cast<int>(lmy);

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
				if ((travel_table_[mx][my] & TT_TRAVELED) != 0 || is_show_all)
				{
					// What's at this map location?
					//
					const auto tile = tilemap[mx][my];
					const auto door = tile & 0x3F;

					auto check_for_hidden_area = false;

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
						static_cast<void>(GetAreaNumber(mx, my));

						if (GAN_HiddenArea)
						{
							color = HIDDEN_COLOR;
						}
					}

					// SHOW KEYS
					//
					if ((flags & OV_KEYS) != 0 &&
						(travel_table_[mx][my] & TT_KEYS) != 0)
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
						auto iconnum = mapsegs[1][farmapylookup[my] + mx];

						// SHOW PUSHWALLS
						//
						if (iconnum == PUSHABLETILE)
						{
							if (show_pwalls_on_automap(mx, my, is_show_all))
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
		result += DoorJams[door.type];
	}
	else
	{
		result += DoorJamsShade[door.type];
	}

	return result;
}

int actor_calculate_rotation(
	const objtype& actor)
{
	return CalcRotate(&actor);
}

int player_get_weapon_sprite_id()
{
	if (playstate == ex_victorious || gamestate.weapon == -1)
	{
		return 0;
	}

	return
		weaponscale[static_cast<std::intptr_t>(gamestate.weapon)] +
		gamestate.weaponframe;
}
