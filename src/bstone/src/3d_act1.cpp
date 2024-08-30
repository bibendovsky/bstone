/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include <algorithm>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vl.h"
#include "bstone_exception.h"


static constexpr auto door_step = 1.0 / 64.0;


// ===========================================================================
//
// PROTOTYPES
//
// ===========================================================================

void OpenDoor(
	std::int16_t door);

void CloseDoor(
	std::int16_t door);

void PlaceItemNearTile(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley);

void HealSelf(
	std::int16_t points);

std::int16_t TransformAreas(
	std::int8_t tilex,
	std::int8_t tiley,
	std::int8_t xform);

void FreeMsgCache(
	mCacheList* mList,
	std::uint16_t infoSize);

void FindNewGoldieSpawnSite();


// ===========================================================================
//
// LOCALS
//
// ===========================================================================


concession_t ConHintList = {};


/*
=============================================================================

 STATICS

=============================================================================
*/

StatObjList statobjlist;
statobj_t* laststatobj;
StatInfos statinfo;


void initialize_static_info_constants()
{
	const auto& assets_info = get_assets_info();

	statinfo =
	{
		{SPR_STAT_0, bo_water_puddle}, // Water Puddle SPR1V
		{SPR_STAT_1, block}, // Containment Canister
		{SPR_STAT_2, block}, // Lunch Table
		{SPR_STAT_3, block}, // Floor Lamp
		{SPR_STAT_4, block}, // Lab Table
		{SPR_STAT_5, block}, // Pillar
		{SPR_STAT_6, dressing}, // Blood Puddle
		{SPR_STAT_7, dressing}, // Piss Puddle

		{SPR_STAT_8, block}, // Ficus Tree SPR2V
		{SPR_STAT_9, dressing}, // Half-Eaten Corpse
		{SPR_STAT_10, block}, // Water Fountain
		{SPR_STAT_11, block}, // Plant 1
		{SPR_STAT_12, block}, // Vase
		{SPR_STAT_13, block}, // General Table
		{SPR_STAT_14, dressing}, // Ceiling Light
		{SPR_STAT_15, block}, // General Chair

		{SPR_STAT_16, block}, // Kitchen Trash SPR3V
		{SPR_STAT_17, dressing}, // Office Trash
		{SPR_STAT_18, block}, // Plant 2
		{SPR_STAT_19, block}, // Gurney No-Blood
		{SPR_STAT_20, dressing}, // Indirect Half-Sphere
		{SPR_STAT_21, dressing}, // Exit Sign
		{SPR_STAT_22, dressing}, // Transporter
		{SPR_STAT_23, block}, // Body Can

		{SPR_STAT_24, bo_pistol}, // PISTOL SPR4V
		{SPR_STAT_25, block}, // Statue

		{assets_info.is_ps() ? SPR_STAT_31 : SPR_STAT_26, bo_clip}, // Charge Unit

		{SPR_STAT_27, bo_burst_rifle}, // Auto-Burst Rifle
		{SPR_STAT_28, bo_ion_cannon}, // Particle Charged ION
		{SPR_STAT_29, bo_firstaid}, // First Aid
		{SPR_VSPIKE8, block}, // Static VSPIKE

		{assets_info.is_ps() ? SPR_STAT_26 : SPR_STAT_31, bo_clip2}, // Big Charge pack/clip

		{SPR_STAT_32, bo_red_key}, // Red Key SPR5V
		{SPR_STAT_33, bo_yellow_key}, // Yellow Key
		{SPR_STAT_34, assets_info.is_ps() ? bo_bfg_cannon : bo_green_key}, // BFG Cannon
		{SPR_STAT_35, bo_blue_key}, // Blue Key
		{SPR_STAT_36, assets_info.is_ps() ? dressing : bo_gold_key}, // OPEN
		{SPR_STAT_37, block}, // Office Desk
		{SPR_STAT_38, block}, // Office Chair
		{SPR_STAT_39, block}, // Security Desk

		{SPR_STAT_40, bo_water}, // Full Water Bowl SPR7V
		{SPR_STAT_41, dressing}, // Empty Water Bowl
		{SPR_STAT_42, bo_chicken}, // Chicken Leg
		{SPR_STAT_43, dressing}, // Chicken Bone
		{SPR_STAT_44, bo_ham}, // Ham
		{SPR_STAT_45, dressing}, // Ham Bone
		{SPR_STAT_46, bo_grenade}, // Grande Launcher
		{SPR_STAT_47, dressing}, // Video Game Machine

		{SPR_VPOST8, block}, // Static VPOST

		// -- VARIOUS --

		{SPR_GURNEY_MUT_READY, block}, // 49 Gurney Mutant
		{SPR_LCAN_ALIEN_READY, block}, // 50 Large Alien Canister
		{SPR_SCAN_ALIEN_READY, block}, // 51 Small Alien Canister

		{SPR_GURNEY_MUT_EMPTY, block}, // 52 Gurney Mutant
		{SPR_LCAN_ALIEN_EMPTY, block}, // 53 Large Alien Canister
		{SPR_SCAN_ALIEN_EMPTY, block}, // 54 Small Alien Canister

		{SPR_OFC_DEAD, dressing}, // 55 Dead Gen Sci.

		{SPR_DEMO, dressing}, // 56 Spacer

		{SPR_AIR_VENT, bo_plainvent}, // 57 Plain air vent
		{SPR_AIR_VENT, bo_bloodvent}, // 58 Blood air vent
		{SPR_AIR_VENT, bo_watervent}, // 59 Water air vent
		{SPR_GRATE, dressing}, // 60 Floor Grate
		{SPR_STEAM_PIPE, dressing}, // 61 Steam Pipe

		{SPR_STAT_48, bo_money_bag}, // 62 money bag
		{SPR_STAT_49, bo_loot}, // 63 loot
		{SPR_STAT_50, bo_gold}, // 64 gold
		{SPR_STAT_51, bo_bonus}, // 65 bonus

		{SPR_STAT_52, block}, // 66 Greek Post
		{SPR_STAT_53, block}, // 67 Red/Blue post
		{SPR_STAT_54, block}, // 68 Red HiTech Post
		{SPR_STAT_55, dressing}, // 69 Ceiling Lamp #2
		{SPR_STAT_56, dressing}, // 70 Ceiling Lamp #3
		{SPR_STAT_57, dressing}, // 71 Body Parts
		{SPR_STAT_58, dressing}, // 72 OR Lamp
		{SPR_STAT_59, block}, // 73 Office Sink
		{SPR_STAT_57, dressing}, // EMPTY - Copy of 71 - Body Parts...
		{SPR_CANDY_BAR, bo_candybar}, // 75 candy bar
		{SPR_SANDWICH, bo_sandwich}, // 76 sandwich
		{SPR_CRATE_1, block}, // 77 Crate #1
		{SPR_CRATE_2, block}, // 78 Crate #2
		{SPR_CRATE_3, block}, // 79 Crate #3
		{SPR_STAT_61, block}, // 80 Table
		{SPR_STAT_62, block}, // 81 Chair
		{SPR_STAT_63, block}, // 82 Stool
		{SPR_STAT_64, dressing}, // 83 Gore

		{SPR_STAT_65, bo_gold3}, // Gold 3
		{SPR_STAT_66, bo_gold2}, // Gold 2
		{SPR_STAT_67, bo_gold1}, // Gold 1

		{SPR_STAT_68, block}, //
		{SPR_STAT_69, block}, //
		{SPR_STAT_70, block}, //
		{SPR_STAT_71, block}, //
		{SPR_STAT_72, block}, //
		{SPR_STAT_73, dressing}, //
		{SPR_STAT_74, dressing}, //
		{SPR_STAT_75, dressing}, //
		{SPR_STAT_76, dressing}, //

		{SPR_RENT_DEAD, dressing}, //
		{SPR_PRO_DEAD, dressing}, //
		{SPR_SWAT_DEAD, dressing}, //
		{SPR_GSCOUT_DEAD, dressing}, //
		{SPR_FSCOUT_DEAD, dressing}, //
		{SPR_MUTHUM1_DEAD, dressing},
		{SPR_MUTHUM2_DEAD, dressing},
		{SPR_LCAN_ALIEN_DEAD, dressing},
		{SPR_SCAN_ALIEN_DEAD, dressing},
		{SPR_GURNEY_MUT_DEAD, dressing},
		{SPR_TERROT_DEAD, dressing},
		{SPR_POD_DIE3, dressing},
		{SPR_STAT_77, bo_coin}, // Concession Machine Money
		{SPR_STAT_78, bo_coin5}, // Concession Machine Money
		{SPR_STAT_79, dressing}, // Auto-Charge Pistol

		{SPR_DOORBOMB, bo_plasma_detonator}, // Plasma Detonator
		{SPR_RUBBLE, dressing}, // Door Rubble
		{SPR_AUTOMAPPER, bo_automapper1}, // Auto Mapper Bonus #1
		{SPR_BONZI_TREE, block}, // BonziTree
		{SPR_POT_PLANT, block}, // Yellow Potted Plant
		{SPR_TUBE_PLANT, block}, // Tube Plant
		{SPR_HITECH_CHAIR, block}, // Hi Tech table and chair
		{SPR_DEAD_RENT, dressing}, // Dead AOG: Rent A Cop
		{SPR_DEAD_PRO, dressing}, // Dead AOG: Pro Guard
		{SPR_DEAD_SWAT, dressing}, // Dead AOG: Swat Guad

		{-1, dressing}, // terminator
	};
}

void InitStaticList()
{
	laststatobj = &statobjlist[0];
}

// ---------------------------------------------------------------------------
// FindStatic()
//
// FUNCTION: Searches the stat obj list and returns ptr to a static obj
//           at a particular tile x & tile y coords.
//
// RETURNS: Ptr == Pointer to static obj.
//          NULL == No static found.
// ---------------------------------------------------------------------------
statobj_t* FindStatic(
	std::uint16_t tilex,
	std::uint16_t tiley)
{
	statobj_t* spot;

	for (spot = statobjlist.data(); spot != laststatobj; spot++)
	{
		if (spot->shapenum != -1 && spot->tilex == tilex && spot->tiley == tiley)
		{
			return spot;
		}
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// FindEmptyStatic()
//
// FUNCTION: Searches the stat obj list and returns ptr to an empty
//          static object.
//
// RETURNS: Ptr == Pointer to empty static obj.
//          NULL == static objlist full.
// ---------------------------------------------------------------------------
statobj_t* FindEmptyStatic()
{
	const auto start_spot = statobjlist.data();

	for (auto spot = start_spot; spot != laststatobj; ++spot)
	{
		if (spot->shapenum == -1)
		{
			return spot;
		}
	}

	if ((laststatobj - start_spot) < MAXSTATS)
	{
		return laststatobj++;
	}

	return nullptr;
}

statobj_t* SpawnStatic(
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t type)
{
	const auto& assets_info = get_assets_info();

	statobj_t* spot;

	spot = FindEmptyStatic();

	if (!spot)
	{
		return nullptr;
	}

	spot->shapenum = statinfo[type].picnum;
	spot->tilex = static_cast<std::uint8_t>(tilex);
	spot->tiley = static_cast<std::uint8_t>(tiley);
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = 0;

	if ((!assets_info.is_aog_sw() && spot->shapenum == SPR_STAT_3) || // // floor lamp
		spot->shapenum == SPR_STAT_14 || // ceiling light
		(!assets_info.is_aog_sw() && spot->shapenum == SPR_STAT_20) ||
		spot->shapenum == SPR_STAT_47 ||
		spot->shapenum == SPR_STAT_51 ||
		spot->shapenum == SPR_STAT_55 ||
		spot->shapenum == SPR_STAT_56)
	{
		spot->lighting = LAMP_ON_SHADING;
	}
	else
	{
		spot->lighting = 0;
	}


	switch (statinfo[type].type)
	{
	case block:
		// consider it a blocking tile
		actorat[tilex][tiley] = (objtype*)1;
		break;

	case bo_green_key:
	case bo_gold_key:
		if (assets_info.is_ps())
		{
			BSTONE_THROW_DYNAMIC_SOURCE(
				("Green/Gold key (AOG) at (" + std::to_string(tilex) + ", " + std::to_string(tiley) + ").").
					c_str());
		}
		travel_table_[tilex][tiley] |= TT_KEYS;
		spot->flags = FL_BONUS;
		spot->itemnumber = static_cast<std::uint8_t>(statinfo[type].type);
		break;

	case bo_plasma_detonator:
		if (!assets_info.is_ps())
		{
			BSTONE_THROW_DYNAMIC_SOURCE(
				("Plasma detonator (PS) at (" + std::to_string(tilex) + ", " + std::to_string(tiley) + ").").
					c_str());
		}
		travel_table_[tilex][tiley] |= TT_KEYS;
		spot->flags = FL_BONUS;
		spot->itemnumber = static_cast<std::uint8_t>(statinfo[type].type);
		break;

	case bo_red_key:
	case bo_yellow_key:
	case bo_blue_key:
		travel_table_[tilex][tiley] |= TT_KEYS;

	case bo_gold1:
	case bo_gold2:
	case bo_gold3:
	case bo_gold:
	case bo_bonus:
	case bo_money_bag:
	case bo_loot:

	case bo_fullheal:
	case bo_firstaid:
	case bo_clip:
	case bo_clip2:
	case bo_burst_rifle:
	case bo_ion_cannon:
	case bo_grenade:
	case bo_bfg_cannon:
	case bo_pistol:
	case bo_chicken:
	case bo_ham:
	case bo_water:
	case bo_water_puddle:
	case bo_sandwich:
	case bo_candybar:
	case bo_coin:
	case bo_coin5:
	case bo_automapper1:
		spot->flags = FL_BONUS;
		spot->itemnumber = static_cast<std::uint8_t>(statinfo[type].type);
		break;

	default:
		break;
	}

	spot->areanumber = GetAreaNumber(spot->tilex, spot->tiley);

	auto result = spot;

	++spot;

	if (spot == (statobjlist.data() + MAXSTATS))
	{
		BSTONE_THROW_STATIC_SOURCE("Too many static objects.");
	}

	return result;
}

// ---------------------------------------------------------------------------
// ReserveStatic()
//
// Reserves a static object at location 0,0 (unseen).  This function is
// used to guarantee that a static will be available.
// ---------------------------------------------------------------------------
statobj_t* ReserveStatic()
{
	auto spot = FindEmptyStatic();

	if (!spot)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many static objects.");
	}

	// Mark as Used.

	spot->shapenum = 1;
	spot->tilex = 0;
	spot->tiley = 0;
	spot->visspot = &spotvis[0][0];

	return spot;
}

// ---------------------------------------------------------------------------
// FindReservedStatic()
//
// Finds a Reserved static object at location 0,0 (unseen).  This function is
// used to guarantee that a static will be available.
// ---------------------------------------------------------------------------
statobj_t* FindReservedStatic()
{
	const auto last_static = statobjlist.data()  + MAXSTATS;

	for (auto spot = &statobjlist[0]; spot < last_static; ++spot)
	{
		if (spot->shapenum == 1 && (!spot->tilex) && (!spot->tiley))
		{ // -1 is a free spot
			return spot;
		}
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// UseReservedStatic()
//
// Finds a Reserved static object and moves it to a new location with new
// attributes.
//
// This function acts like PlaceItemType - But REQUIRES a reserved
// static.  Before using this function, make sure that you have already
// reserved a static to be used using ReserveStatic();
// ---------------------------------------------------------------------------
statobj_t* UseReservedStatic(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley)
{
	auto spot = FindReservedStatic();
	std::int16_t type;

	if (!spot)
	{
		BSTONE_THROW_STATIC_SOURCE("Count not find a reserved static at location (0, 0) with shape #1.");
	}

	//
	// find the item number
	//

	for (type = 0;; type++)
	{
		if (statinfo[type].picnum == -1)
		{ // End of Static List...
			BSTONE_THROW_STATIC_SOURCE("Couldn't find type.");
		}

		if (statinfo[type].type == itemtype)
		{ // Bingo, Found it!
			break;
		}
	}

	//
	// place it
	//

	const auto& assets_info = get_assets_info();

	switch (itemtype)
	{
	case bo_plasma_detonator:
		if (!assets_info.is_ps())
		{
			break;
		}

	case bo_green_key:
	case bo_gold_key:
	case bo_red_key:
	case bo_yellow_key:
	case bo_blue_key:
		travel_table_[tilex][tiley] |= TT_KEYS;
		break;
	}

	spot->shapenum = statinfo[type].picnum;
	spot->tilex = static_cast<std::uint8_t>(tilex);
	spot->tiley = static_cast<std::uint8_t>(tiley);
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = FL_BONUS;
	spot->itemnumber = static_cast<std::uint8_t>(statinfo[type].type);

	spot->areanumber = GetAreaNumber(spot->tilex, spot->tiley);

	return spot;
}

std::int8_t pint_xy[8][2] = {
	{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
{0, 1}, {1, -1}, {1, 0}, {1, 1},
};

void PlaceReservedItemNearTile(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley)
{
	std::int8_t loop;

	for (loop = 0; loop < 8; loop++)
	{
		std::int8_t x = static_cast<std::int8_t>(tilex + pint_xy[static_cast<int>(loop)][1]), y = static_cast<std::int8_t>(tiley + pint_xy[static_cast<int>(loop)][0]);

		if (!tilemap[static_cast<int>(x)][static_cast<int>(y)])
		{
			if (actorat[static_cast<int>(x)][static_cast<int>(y)] == reinterpret_cast<objtype*>(1))
			{ // Check for a SOLID static
				continue;
			}

			UseReservedStatic(itemtype, x, y);
			return;
		}
	}

	UseReservedStatic(itemtype, tilex, tiley);
}

/*
===============
=
= PlaceItemType
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
===============
*/
void PlaceItemType(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley)
{
	std::int16_t type;
	statobj_t* spot;

	//
	// find the item number
	//
	for (type = 0;; type++)
	{
		if (statinfo[type].picnum == -1)
		{ // end of list
			BSTONE_THROW_STATIC_SOURCE("Couldn't find type.");
		}
		if (statinfo[type].type == itemtype)
		{
			break;
		}
	}

	//
	// find a spot in statobjlist to put it in
	//
	spot = FindEmptyStatic();

	if (!spot)
	{
		return;
	}

	//
	// place it
	//
	spot->shapenum = statinfo[type].picnum;
	spot->tilex = static_cast<std::uint8_t>(tilex);
	spot->tiley = static_cast<std::uint8_t>(tiley);
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = FL_BONUS;
	spot->itemnumber = static_cast<std::uint8_t>(statinfo[type].type);

	spot->areanumber = GetAreaNumber(spot->tilex, spot->tiley);
}

void PlaceItemNearTile(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley)
{
	// [0] is the y offset
	// [1] is the x offset
	//
	std::int8_t loop;

	for (loop = 0; loop < 8; loop++)
	{
		std::int8_t x = static_cast<std::int8_t>(tilex + pint_xy[static_cast<int>(loop)][1]), y = static_cast<std::int8_t>(tiley + pint_xy[static_cast<int>(loop)][0]);

		if (!tilemap[static_cast<int>(x)][static_cast<int>(y)])
		{
			if (actorat[static_cast<int>(x)][static_cast<int>(y)] == reinterpret_cast<objtype*>(1))
			{ // Check for a SOLID static
				continue;
			}

			PlaceItemType(itemtype, x, y);
			return;
		}
	}

	PlaceItemType(itemtype, tilex, tiley);
}

// --------------------------------------------------------------------------
// ExplodeStatics()
//
//  NOTES: Explodes statics in a one tile radius from a given tile x and tile y
//
// --------------------------------------------------------------------------
void ExplodeStatics(
	std::int16_t tilex,
	std::int16_t tiley)
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		return;
	}

	statobj_t* spot;
	std::int16_t y_diff, x_diff;
	bool remove;

	for (spot = &statobjlist[0]; spot != laststatobj; spot++)
	{
		if (spot->shapenum != -1)
		{
			y_diff = spot->tiley - tiley;
			y_diff = ABS(y_diff);

			x_diff = spot->tilex - tilex;
			x_diff = ABS(x_diff);

			if (x_diff < 2 && y_diff < 2)
			{
				remove = false;

				//
				// Test for specific statics..
				//

				switch (spot->itemnumber)
				{
					//
					// Check for Clips
					//
				case bo_clip:
				case bo_clip2:
					remove = true;

					SpawnCusExplosion(
						spot->tilex + 0.5,
						spot->tiley + 0.5,
						SPR_CLIP_EXP1,
						7,
						3 + (US_RndT() & 0x3),
						explosionobj
					);

					break;
				}


				//
				// Check to see if we remove it.
				//
				if (remove)
				{
					// Remove static
					spot->shapenum = -1;
					spot->itemnumber = bo_nothing;

					vid_hw_on_remove_static(*spot);
				}
			}
		}
	}
}

/*
=============================================================================

 DOORS

doorobjlist[] holds most of the information for the doors

doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
		this is directly accessed by AsmRefresh during rendering

The number of doors is limited to 64 because a spot in tilemap holds the
		door number in the low 6 bits, with the high bit meaning a door center
		and bit 6 meaning a door side tile

Open doors conect two areas, so sounds will travel between them and sight
		will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
		An area is true if it connects with the player's current spor.

=============================================================================
*/

static const std::int16_t OPENTICS = 300;

doorobj_t doorobjlist[MAXDOORS];
doorobj_t* lastdoorobj;
std::int16_t doornum;

// leading edge of door 0=closed, 0xffff = fully open
 // !!! Used in saved game.
DoorPositions doorposition;

 // !!! Used in saved game.
AreaConnect areaconnect;

 // !!! Used in saved game.
AreaByPlayer areabyplayer;


/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/
void RecursiveConnect(
	std::int16_t areanumber)
{
	std::int16_t i;

	for (i = 0; i < NUMAREAS; i++)
	{
		if (!areabyplayer[i] && areaconnect[areanumber][i])
		{
			areabyplayer[i] = true;
			RecursiveConnect(i);
		}
	}
}

void ConnectAreas()
{
	areabyplayer.reset();
	areabyplayer[player->areanumber] = true;
	RecursiveConnect(player->areanumber);
}

void InitAreas()
{
	areabyplayer.reset();
	areabyplayer[player->areanumber] = true;
}

void InitDoorList()
{
	areabyplayer.reset();
	areaconnect = AreaConnect{};

	lastdoorobj = &doorobjlist[0];
	doornum = 0;
}

void SpawnDoor(
	std::int16_t tilex,
	std::int16_t tiley,
	bool vertical,
	keytype lock,
	door_t type)
{
	std::uint16_t* map[2] =
	{
		&mapsegs[0][farmapylookup[tiley] + tilex],
		&mapsegs[1][farmapylookup[tiley] + tilex],
	};

	if (doornum == 64)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many doors in level.");
	}

	doorposition[doornum] = 0.0; // doors start out fully closed
	lastdoorobj->tilex = static_cast<std::uint8_t>(tilex);
	lastdoorobj->tiley = static_cast<std::uint8_t>(tiley);
	lastdoorobj->vertical = vertical;
	lastdoorobj->lock = lock;
	lastdoorobj->type = type;
	lastdoorobj->action = dr_closed;
	lastdoorobj->flags = DR_BLASTABLE; // JIM - Do something with this! jdebug

	if (vertical)
	{
		lastdoorobj->areanumber[0] = GetAreaNumber(static_cast<std::int8_t>(tilex + 1), static_cast<std::int8_t>(tiley));
		lastdoorobj->areanumber[1] = GetAreaNumber(static_cast<std::int8_t>(tilex - 1), static_cast<std::int8_t>(tiley));
	}
	else
	{
		lastdoorobj->areanumber[0] = GetAreaNumber(static_cast<std::int8_t>(tilex), static_cast<std::int8_t>(tiley - 1));
		lastdoorobj->areanumber[1] = GetAreaNumber(static_cast<std::int8_t>(tilex), static_cast<std::int8_t>(tiley + 1));
	}

	// consider it a solid wall
	actorat[tilex][tiley] = reinterpret_cast<objtype*>(static_cast<std::size_t>(doornum | 0x80));

	//
	// make the door tile a special tile, and mark the adjacent tiles
	// for door sides
	//
	tilemap[tilex][tiley] = static_cast<std::uint8_t>(doornum | 0x80);

	if (vertical)
	{
		if (*(map[0] - mapwidth - 1) == TRANSPORTERTILE)
		{
			*map[0] = GetAreaNumber(static_cast<std::int8_t>(tilex + 1), static_cast<std::int8_t>(tiley));
		}
		else
		{
			*map[0] = GetAreaNumber(static_cast<std::int8_t>(tilex - 1), static_cast<std::int8_t>(tiley));
		}
		tilemap[tilex][tiley - 1] |= 0x40;
		tilemap[tilex][tiley + 1] |= 0x40;
	}
	else
	{
		*map[0] = GetAreaNumber(static_cast<std::int8_t>(tilex), static_cast<std::int8_t>(tiley - 1));
		tilemap[tilex - 1][tiley] |= 0x40;
		tilemap[tilex + 1][tiley] |= 0x40;
	}

	doornum++;
	lastdoorobj++;
}

void CheckLinkedDoors(
	std::int16_t door,
	std::int16_t door_dir)
{
	static std::int16_t LinkCheck = 0;
	static std::int16_t base_tilex;
	static std::int16_t base_tiley;

	std::int16_t tilex = doorobjlist[door].tilex;
	std::int16_t tiley = doorobjlist[door].tiley;
	std::int16_t next_tilex = 0;
	std::int16_t next_tiley = 0;

	// Find next door in link.
	//
	if (mapsegs[1][farmapylookup[tiley] + tilex])
	{
		std::uint16_t value = mapsegs[1][farmapylookup[tiley] + tilex];

		// Define the next door in the link.
		//
		next_tilex = (value & 0xff00) >> 8;
		next_tiley = value & 0xff;

		// Is this the head of the link?
		//
		if (!LinkCheck)
		{
			base_tilex = tilex;
			base_tiley = tiley;
		}
	}

	LinkCheck++;

	// Recursively open/close linked doors.
	//
	if ((next_tilex) &&
		(next_tiley) &&
		((next_tilex != base_tilex) || (next_tiley != base_tiley))
		)
	{
		std::int16_t door_index = tilemap[next_tilex][next_tiley] & ~0x80;

		switch (door_dir)
		{
		case dr_opening:
			doorobjlist[door_index].lock = kt_none;
			OpenDoor(door_index);

			vid_hw_on_update_door_lock(door_index);

			break;

		case dr_closing:
			doorobjlist[door_index].lock = kt_none;
			CloseDoor(door_index);

			vid_hw_on_update_door_lock(door_index);

			break;
		}
	}

	LinkCheck--;
}

void OpenDoor(
	std::int16_t door)
{

	if (doorobjlist[door].action == dr_jammed)
	{
		return;
	}

	if (doorobjlist[door].action == dr_open)
	{
		doorobjlist[door].ticcount = 0; // reset open time
	}
	else
	{
		doorobjlist[door].action = dr_opening; // start it opening

	}
	CheckLinkedDoors(door, dr_opening);
}

objtype* get_actor_near_door(
	int tile_x,
	int tile_y)
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		for (int i = 0; i < doornum; ++i)
		{
			const doorobj_t& door = doorobjlist[i];

			if (door.tilex == tile_x && door.tiley == tile_y)
			{
				// It's a closing door, not an actor.
				return nullptr;
			}
		}
	}

	return actorat[tile_x][tile_y];
}

void CloseDoor(
	std::int16_t door)
{
	std::int16_t tilex, tiley, area;
	objtype* check;

	if (doorobjlist[door].action == dr_jammed)
	{
		return;
	}

	//
	// don't close on anything solid
	//
	tilex = doorobjlist[door].tilex;
	tiley = doorobjlist[door].tiley;

	if (actorat[tilex][tiley])
	{
		return;
	}

	if (player->tilex == tilex && player->tiley == tiley)
	{
		return;
	}

	if (doorobjlist[door].vertical)
	{
		if (player->tiley == tiley)
		{
			if (static_cast<int>(player->x + MINDIST) == tilex)
			{
				return;
			}

			if (static_cast<int>(player->x - MINDIST) == tilex)
			{
				return;
			}
		}

		check = get_actor_near_door(tilex - 1, tiley);

		if (check != nullptr &&
			static_cast<int>(check->x + MINDIST) == tilex)
		{
			return;
		}

		check = get_actor_near_door(tilex + 1, tiley);

		if (check != nullptr &&
			static_cast<int>(check->x - MINDIST) == tilex)
		{
			return;
		}
	}
	else if (!doorobjlist[door].vertical)
	{
		if (player->tilex == tilex)
		{
			if (static_cast<int>(player->y + MINDIST) == tiley)
			{
				return;
			}

			if (static_cast<int>(player->y - MINDIST) == tiley)
			{
				return;
			}
		}

		check = get_actor_near_door(tilex, tiley - 1);

		if (check != nullptr &&
			static_cast<int>(check->y + MINDIST) == tiley)
		{
			return;
		}

		check = get_actor_near_door(tilex, tiley + 1);

		if (check != nullptr &&
			static_cast<int>(check->y - MINDIST) == tiley)
		{
			return;
		}
	}


	//
	// play door sound if in a connected area
	//
	area = GetAreaNumber(doorobjlist[door].tilex, doorobjlist[door].tiley);

	if (area >= NUMAREAS)
	{
		BSTONE_THROW_STATIC_SOURCE("[CLOSE_DOOR] Area number out of range.");
	}

	if (areabyplayer[area])
	{
		switch (doorobjlist[door].type)
		{
		case dr_bio:
		case dr_office:
		case dr_space:
		case dr_normal:
			sd_play_door_sound(HTECHDOORCLOSESND, doorobjlist[door]);
			break;

		default:
			sd_play_door_sound(CLOSEDOORSND, doorobjlist[door]);
			break;
		}
	}

	doorobjlist[door].action = dr_closing;
	//
	// make the door space solid
	//

	actorat[tilex][tiley] = reinterpret_cast<objtype*>(static_cast<std::size_t>(door | 0x80));

	CheckLinkedDoors(door, dr_closing);

}

/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

const char* const od_oneway = "\r\r   DOOR LOCKED FROM\r      THIS SIDE.\r^XX";
const char* const od_locked = "\r\r   DOOR PERMANENTLY\r        LOCKED.\r^XX";
const char* const od_reddenied = "\r\r      RED LEVEL\r    ACCESS DENIED!\r^XX";
const char* const od_yellowdenied = "\r\r     YELLOW LEVEL\r    ACCESS DENIED!\r^XX";
const char* const od_bluedenied = "\r\r      BLUE LEVEL\r    ACCESS DENIED!\r^XX";

const char* const od_green_denied =
"\r"
"\r"
"     GREEN LEVEL\r"
"    ACCESS DENIED!\r"
"^XX"
;

const char* const od_gold_denied =
"\r"
"\r"
"      GOLD LEVEL\r"
"    ACCESS DENIED!\r"
"^XX"
;

const char* const od_granted = "\r\r    ACCESS GRANTED\r    DOOR UNLOCKED.\r^XX";
const char* const od_operating = "\r\r    OPERATING DOOR.\r^XX";

void OperateDoor(
	std::int16_t door)
{
	std::int16_t lock;
	bool oneway = false;


	//
	// Check for wrong way on a ONEWAY door.
	//

	switch (doorobjlist[door].type)
	{
	case dr_oneway_left:
		if (player->tilex < doorobjlist[door].tilex)
		{
			oneway = true;
		}
		break;

	case dr_oneway_right:
		if (player->tilex > doorobjlist[door].tilex)
		{
			oneway = true;
		}
		break;

	case dr_oneway_up:
		if (player->tiley < doorobjlist[door].tiley)
		{
			oneway = true;
		}
		break;

	case dr_oneway_down:
		if (player->tiley > doorobjlist[door].tiley)
		{
			oneway = true;
		}
		break;

	default:
		break;
	}

	if (oneway)
	{
		if (doorobjlist[door].action == dr_closed)
		{
			DISPLAY_TIMED_MSG(od_oneway, MP_DOOR_OPERATE, MT_GENERAL);
			sd_play_player_no_way_sound(NOWAYSND);
		}

		return;
	}

	//
	// Check for possibly being locked
	//

	lock = static_cast<std::int16_t>(doorobjlist[door].lock);
	if (lock != kt_none)
	{
		if (!(gamestate.numkeys[lock - kt_red]))
		{
			sd_play_player_no_way_sound(NOWAYSND);

			switch (lock)
			{
			case kt_red:
				DISPLAY_TIMED_MSG(od_reddenied, MP_DOOR_OPERATE, MT_GENERAL);
				break;

			case kt_yellow:
				DISPLAY_TIMED_MSG(od_yellowdenied, MP_DOOR_OPERATE, MT_GENERAL);
				break;

			case kt_blue:
				DISPLAY_TIMED_MSG(od_bluedenied, MP_DOOR_OPERATE, MT_GENERAL);
				break;

			case kt_green:
				DISPLAY_TIMED_MSG(od_green_denied, MP_DOOR_OPERATE, MT_GENERAL);
				break;

			case kt_gold:
				DISPLAY_TIMED_MSG(od_gold_denied, MP_DOOR_OPERATE, MT_GENERAL);
				break;

			default:
				DISPLAY_TIMED_MSG(od_locked, MP_DOOR_OPERATE, MT_GENERAL);
				break;
			}

			return;
		}
		else
		{
			TakeKey(lock - kt_red);
			DISPLAY_TIMED_MSG(od_granted, MP_DOOR_OPERATE, MT_GENERAL);
			doorobjlist[door].lock = kt_none;                           // UnLock door

			vid_hw_on_update_door_lock(door);
		}
	}
	else
	{
		DISPLAY_TIMED_MSG(od_operating, MP_DOOR_OPERATE, MT_GENERAL);
	}

	switch (doorobjlist[door].action)
	{
	case dr_closed:
	case dr_closing:
		OpenDoor(door);
		break;

	case dr_open:
	case dr_opening:
		CloseDoor(door);
		break;
	default:
		break;
	}
}

void BlockDoorOpen(
	std::int16_t door)
{
	doorobjlist[door].action = dr_jammed;
	doorobjlist[door].ticcount = 0;
	doorposition[door] = 1.0;
	doorobjlist[door].lock = kt_none;
	doorobjlist[door].flags &= ~DR_BLASTABLE;

	actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;

	TransformAreas(doorobjlist[door].tilex, doorobjlist[door].tiley, 1);

	vid_hw_on_move_door(door);
	vid_hw_on_update_door_lock(door);
}

void TryBlastDoor(
	std::int8_t door)
{
	switch (doorobjlist[static_cast<int>(door)].type)
	{
	case dr_oneway_left:
	case dr_oneway_up:
	case dr_oneway_right:
	case dr_oneway_down:
		break;

	default:
		if (doorposition[static_cast<int>(door)] < 0.5 &&
			doorobjlist[static_cast<int>(door)].action != dr_jammed &&
			doorobjlist[static_cast<int>(door)].lock == kt_none)
		{
			BlockDoorOpen(door);

			SpawnCusExplosion(
				doorobjlist[static_cast<int>(door)].tilex + 0.5,
				doorobjlist[static_cast<int>(door)].tiley + 0.5,
				SPR_EXPLOSION_1,
				4,
				3,
				doorexplodeobj
			);
		}
		break;
	}
}

void BlastNearDoors(
	std::int16_t tilex,
	std::int16_t tiley)
{
	std::uint8_t door;
	char* doorptr;
	std::int16_t x, y;

	doorptr = (char*)&tilemap[tilex][tiley];

	for (x = -1; x < 2; x++)
	{
		for (y = -64; y < 128; y += 64)
		{
			if ((door = *(doorptr + x + y)) & 0x80)
			{
				door &= ~0x80;
				TryBlastDoor(door);
			}
		}
	}
}

void DropPlasmaDetonator()
{
	auto obj = MoveHiddenOfs(
		plasma_detonator_reserveobj,
		plasma_detonatorobj,
		player->x,
		player->y
	);

	if (obj)
	{
		obj->flags |= FL_SHOOTABLE;

		DISPLAY_TIMED_MSG(pd_dropped, MP_DOOR_OPERATE, MT_GENERAL);
		sd_play_actor_voice_sound(ROBOT_SERVOSND, *obj);
		TakePlasmaDetonator(1);
		return;
	}

	BSTONE_THROW_STATIC_SOURCE("Could not find Fision/Plasma Detonator reserve object.");
}

// --------------------------------------------------------------------------
// TryDropPlasmaDetonator()  - Will check to see if player is close enough to
//                             drop a detonator.
// --------------------------------------------------------------------------
void TryDropPlasmaDetonator()
{
	const std::int16_t MAX_RANGE_DIST = 2;

	objtype* obj;
	std::int16_t distx, disty, distance;


	if (!gamestuff.level[gamestate.mapon + 1].locked)
	{
		DISPLAY_TIMED_MSG(pd_floornotlocked, MP_DETONATOR, MT_GENERAL);
		return;
	}

	if (gamestate.mapon > 19)
	{
		DISPLAY_TIMED_MSG(pd_no_computer, MP_DETONATOR, MT_GENERAL);
		return;
	}

	if (!gamestate.plasma_detonators)
	{
		DISPLAY_TIMED_MSG(pd_donthaveany, MP_DETONATOR, MT_GENERAL);
		return;
	}

	obj = FindObj(rotating_cubeobj, -1, -1);

	if (!obj)
	{
		BSTONE_THROW_STATIC_SOURCE("Cound not find security cube - Need to have one pal!");
	}

	if (obj->areanumber != player->areanumber)
	{
		DISPLAY_TIMED_MSG(pd_notnear, MP_DETONATOR, MT_GENERAL);
		return;
	}

	distx = player->tilex - obj->tilex;
	distx = ABS(distx);
	disty = player->tiley - obj->tiley;
	disty = ABS(disty);
	distance = distx > disty ? distx : disty;

	if (distance > MAX_RANGE_DIST)
	{
		DISPLAY_TIMED_MSG(pd_getcloser, MP_DETONATOR, MT_GENERAL);
		return;
	}
	else
	{
		DropPlasmaDetonator();
	}
}

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/
void DoorOpen(
	std::int16_t door)
{
	if ((doorobjlist[door].ticcount += tics) >= OPENTICS)
	{
		CloseDoor(door);
	}
}

std::int16_t TransformAreas(
	std::int8_t tilex,
	std::int8_t tiley,
	std::int8_t xform)
{
	std::int16_t xofs = 0, yofs = 0;
	std::uint8_t area1, area2;

	// Is this walkway:  Horizontal?   Vertical?   Error?
	//
	if ((tilemap[static_cast<int>(tilex)][static_cast<int>(tiley + 1)]) && (tilemap[static_cast<int>(tilex)][static_cast<int>(tiley - 1)]))
	{
		xofs = 1;
		yofs = 0;
	}
	else if ((tilemap[static_cast<int>(tilex + 1)][static_cast<int>(tiley)]) && (tilemap[static_cast<int>(tilex - 1)][static_cast<int>(tiley)]))
	{
		xofs = 0;
		yofs = 1;
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid linkable area.");
	}

	// Define the two areas...
	//
	area1 = GetAreaNumber(static_cast<std::int8_t>(tilex + xofs), static_cast<std::int8_t>(tiley + yofs));
	if (area1 >= NUMAREAS)
	{
		BSTONE_THROW_STATIC_SOURCE("Area1 out of table range.");
	}

	area2 = GetAreaNumber(static_cast<std::int8_t>(tilex - xofs), static_cast<std::int8_t>(tiley - yofs));
	if (area2 >= NUMAREAS)
	{
		BSTONE_THROW_STATIC_SOURCE("Area2 out of table range.");
	}

	// Connect these two areas.
	//
	areaconnect[area1][area2] += xform;
	areaconnect[area2][area1] += xform;
	ConnectAreas();

	return area1;
}

void DoorOpening(
	std::int16_t door)
{
	auto position = doorposition[door];

	if (position <= 0.0)
	{
		const auto area1 = TransformAreas(doorobjlist[door].tilex, doorobjlist[door].tiley, 1);

		if (areabyplayer[area1])
		{
			switch (doorobjlist[door].type)
			{
			case dr_bio:
			case dr_office:
			case dr_space:
			case dr_normal:
				sd_play_door_sound(HTECHDOOROPENSND, doorobjlist[door]);
				break;

			default:
				sd_play_door_sound(OPENDOORSND, doorobjlist[door]);
				break;
			}
		}
	}

	//
	// slide the door by an adaptive amount
	//
	position += tics * door_step;

	if (position >= 1.0)
	{
		//
		// door is all the way open
		//
		position = 1.0;
		doorobjlist[door].ticcount = 0;
		doorobjlist[door].action = dr_open;
		actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
	}

	doorposition[door] = position;

	vid_hw_on_move_door(door);
}

void DoorClosing(
	std::int16_t door)
{
	const auto tilex = doorobjlist[door].tilex;
	const auto tiley = doorobjlist[door].tiley;

	if ((actorat[tilex][tiley] != reinterpret_cast<objtype*>(static_cast<std::size_t>(door | 0x80))) ||
		(player->tilex == tilex && player->tiley == tiley))
	{
		// something got inside the door
		OpenDoor(door);
		return;
	}

	auto position = doorposition[door];

	//
	// slide the door by an adaptive amount
	//
	position -= tics * door_step;

	if (position <= 0.0)
	{
		position = 0.0;
		doorobjlist[door].action = dr_closed;
		TransformAreas(doorobjlist[door].tilex, doorobjlist[door].tiley, -1);
	}

	doorposition[door] = position;

	vid_hw_on_move_door(door);
}

/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/
void MoveDoors()
{
	std::int16_t door;

	for (door = 0; door < doornum; door++)
	{
		switch (doorobjlist[door].action)
		{
		case dr_open:
			DoorOpen(door);
			break;

		case dr_opening:
			DoorOpening(door);
			break;

		case dr_closing:
			DoorClosing(door);
			break;

		default:
			break;
		}
	}
}


/*
=============================================================================

 PUSHABLE WALLS

=============================================================================
*/

static constexpr auto pwall_state_first_step = 1.0 / 64.0;
static constexpr auto pwall_state_step = 1.0 / 16.0;

static constexpr auto pwall_pos_end = 63.0 / 64.0;


// !!! Used in saved game.
double pwallstate;

// amount a pushable wall has been moved (0-63)
// !!! Used in saved game.
double pwallpos;

// !!! Used in saved game.
std::uint16_t pwallx = 0;

// !!! Used in saved game.
std::uint16_t pwally = 0;

// !!! Used in saved game.
std::int16_t pwalldir;

// !!! Used in saved game.
std::int16_t pwalldist;


void PushWall(
	std::int16_t checkx,
	std::int16_t checky,
	std::int16_t dir)
{
	if (pwallstate != 0.0)
	{
		return;
	}

	TransformAreas(static_cast<std::int8_t>(checkx), static_cast<std::int8_t>(checky), 1);

	const auto oldtile = tilemap[checkx][checky];

	if (oldtile == 0)
	{
		return;
	}

	switch (dir)
	{
		case di_north:
			if (actorat[checkx][checky - 1] != nullptr)
			{
				return;
			}

			tilemap[checkx][checky - 1] = oldtile;
			actorat[checkx][checky - 1] = reinterpret_cast<objtype*>(oldtile);
			break;

		case di_east:
			if (actorat[checkx + 1][checky] != nullptr)
			{
				return;
			}

			tilemap[checkx + 1][checky] = oldtile;
			actorat[checkx + 1][checky] = reinterpret_cast<objtype*>(oldtile);
			break;

		case di_south:
			if (actorat[checkx][checky + 1] != nullptr)
			{
				return;
			}

			tilemap[checkx][checky + 1] = oldtile;
			actorat[checkx][checky + 1] = reinterpret_cast<objtype*>(oldtile);
			break;

		case di_west:
			if (actorat[checkx - 1][checky] != nullptr)
			{
				return;
			}

			tilemap[checkx - 1][checky] = oldtile;
			actorat[checkx - 1][checky] = reinterpret_cast<objtype*>(oldtile);
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid pushwall direction.");
	}

	pwalldist = 2;
	pwallx = checkx;
	pwally = checky;
	pwalldir = dir;
	pwallstate = pwall_state_first_step;
	pwallpos = 0.0;
	tilemap[pwallx][pwally] |= 0xC0;
	mapsegs[1][farmapylookup[pwally] + pwallx] = 0; // remove P tile info

	sd_play_pwall_sound(PUSHWALLSND);
}

void MovePWalls()
{
	if (pwallstate == 0.0)
	{
		return;
	}

	const auto oldblock = static_cast<int>(0.5 * pwallstate);
	pwallstate += tics * pwall_state_step;
	const auto newblock = static_cast<int>(0.5 * pwallstate);

	if (newblock != oldblock)
	{
		--pwalldist;

		// block crossed into a new block
		const auto oldtile = static_cast<std::uint8_t>(tilemap[pwallx][pwally] & 63);

		//
		// the tile can now be walked into
		//
		tilemap[pwallx][pwally] = 0;
		actorat[pwallx][pwally] = nullptr;

		std::uint16_t areanumber = GetAreaNumber(player->tilex, player->tiley);

		if (GAN_HiddenArea)
		{
			areanumber += HIDDENAREATILE;
		}
		else
		{
			areanumber += AREATILE;
		}

		mapsegs[0][farmapylookup[pwally] + pwallx] = areanumber;

		//
		// see if it should be pushed farther
		//
		const auto old_x = pwallx;
		const auto old_y = pwally;

		auto next_dx = 0;
		auto next_dy = 0;

		switch (pwalldir)
		{
			case di_north:
				next_dy = -1;
				break;

			case di_east:
				next_dx = 1;
				break;

			case di_south:
				next_dy = 1;
				break;

			case di_west:
				next_dx = -1;
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Invalid pushwall direction.");
		}

		if (pwalldist == 0)
		{
			//
			// the block has been pushed two tiles
			//
			pwallstate = 0.0;
			pwallpos = 0.0;

			pwallx += static_cast<std::uint16_t>(next_dx);
			pwally += static_cast<std::uint16_t>(next_dy);

			vid_hw_on_step_pushwall(old_x, old_y);

			pwallpos = pwall_pos_end;

			return;
		}
		else
		{
			pwallx += static_cast<std::uint16_t>(next_dx);
			pwally += static_cast<std::uint16_t>(next_dy);

			vid_hw_on_step_pushwall(old_x, old_y);

			const auto next_x = pwallx + next_dx;
			const auto next_y = pwally + next_dy;

			auto& next_actorat = actorat[next_x][next_y];

			if (next_actorat != nullptr)
			{
				pwallstate = 0.0;
				pwallpos = pwall_pos_end;

				vid_hw_on_step_pushwall(pwallx, pwally);

				vid_hw_on_pushwall_to_wall(
					pwallx,
					pwally,
					pwallx,
					pwally
				);

				return;
			}

			tilemap[next_x][next_y] = oldtile;
			next_actorat = reinterpret_cast<objtype*>(static_cast<std::uintptr_t>(oldtile));

			tilemap[pwallx][pwally] = static_cast<std::uint8_t>(oldtile | 0xC0);
		}
	}

	pwallpos = get_fractional(0.5 * pwallstate);

	vid_hw_on_move_pushwall();
}

// ==========================================================================
//
//                    'SPECIAL MESSAGE' CACHING SYSTEM
//
// When creating special 'types' of message caching structures, make sure
// all 'special data' is placed at the end of the BASIC message structures.
// In memory, BASIC INFO should appear first. ex:
//
// mCacheList
// ---> NumMsgs
// ---> mCacheInfo
//      ---> local_val
//      ---> global_val
//      ---> mSeg
//
// ... all special data follows ...
//
// ==========================================================================

void InitMsgCache(
	mCacheList* mList,
	std::uint16_t listSize,
	std::uint16_t infoSize)
{
	FreeMsgCache(mList, infoSize);

	std::fill_n(
		reinterpret_cast<std::uint8_t*>(mList),
		listSize,
		std::uint8_t{}
	);
}

void FreeMsgCache(
	mCacheList* mList,
	std::uint16_t infoSize)
{
	mCacheInfo* ci = mList->mInfo;
	char* ch_ptr;

	while (mList->NumMsgs--)
	{
		ci->mSeg.clear();

		ch_ptr = (char*)ci;
		ch_ptr += infoSize;
		ci = (mCacheInfo*)ch_ptr;
	}
}

// ---------------------------------------------------------------------------
// CacheMsg()
//
// Caches the specific message in FROM a given 'grsegs' block TO the
// next available message segment pointer.
// ---------------------------------------------------------------------------
void CacheMsg(
	mCacheInfo* ci,
	std::uint16_t SegNum,
	std::uint16_t MsgNum)
{
	ci->mSeg.clear();
	ci->mSeg.resize(MAX_CACHE_MSG_LEN);
	LoadMsg(ci->mSeg.data(), SegNum, MsgNum, MAX_CACHE_MSG_LEN);
}

// ---------------------------------------------------------------------------
// LoadMsg()
//
// Loads the specific message in FROM a given 'grsegs' block TO the
// the memory address provided.  Memory allocation and handleing prior and
// after this function usage is responsibility of the calling function(s).
//
// PARAMS:  hint_buffer - Destination address to store message
//          SegNum - GrSeg for messages in VGAGRAPH.BS?
//          MsgNum - Message number to load
//          MaxMsgLen - Max len of cache msg (Len of hint_buffer)
//
// RETURNS : Returns the length of the loaded message
// ---------------------------------------------------------------------------
std::int16_t LoadMsg(
	char* hint_buffer,
	std::uint16_t SegNum,
	std::uint16_t MsgNum,
	std::uint16_t MaxMsgLen)
{
	const auto msg_xx = "^XX";

	char* Message, *EndOfMsg;
	std::int16_t pos = 0;

	CA_CacheGrChunk(SegNum);
	Message = reinterpret_cast<char*>(grsegs[SegNum].data());

	// Search for end of MsgNum-1 (Start of our message)
	//
	while (--MsgNum)
	{
		Message = strstr(Message, msg_xx);

		if (!Message)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid 'Cached Message' number");
		}

		Message += 3;           // Bump to start of next Message
	}

	// Move past LFs and CRs that follow "^XX"
	//
	while ((*Message == '\n') || (*Message == '\r'))
	{
		Message++;
	}

	// Find the end of the message
	//
	if ((EndOfMsg = strstr(Message, msg_xx)) == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid 'Cached Message' number");
	}
	EndOfMsg += 3;

	// Copy to a temp buffer
	//
	while (Message != EndOfMsg)
	{
		if (*Message != '\n')
		{
			hint_buffer[pos++] = *Message;
		}

		if (pos >= MaxMsgLen)
		{
			BSTONE_THROW_STATIC_SOURCE("Cached Hint Message is to Long for allocated space.");
		}

		Message++;
	}

	hint_buffer[pos] = 0; // Null Terminate
	UNCACHEGRCHUNK(SegNum);

	return pos;
}


/*
=============================================================================

 CONCESSION MACHINES

=============================================================================
*/

// --------------------------------------------------------------------------
// SpawnConcession()
//
// actorat[][] - Holds concession machine number (1 - MAXCONCESSIONS+1)
// --------------------------------------------------------------------------
void SpawnConcession(
	std::int16_t tilex,
	std::int16_t tiley,
	std::uint16_t credits,
	std::uint16_t machinetype)
{
	con_mCacheInfo* ci = &ConHintList.cmInfo[ConHintList.NumMsgs];

	if (ConHintList.NumMsgs >= MAXCONCESSIONS)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many concession machines in level.");
	}

	if (credits != PUSHABLETILE)
	{
		switch (credits & 0xff00)
		{
		case 0:
		case 0xFC00: // Food Id
			ci->mInfo.local_val = credits & 0xff;
			ci->operate_cnt = 0;
			ci->type = static_cast<std::uint8_t>(machinetype);
			break;
		}
	}

	// Consider it a solid wall (val != 0)
	//
	if (++ConHintList.NumMsgs > MAX_CACHE_MSGS)
	{
		BSTONE_THROW_STATIC_SOURCE("[CONCESSIONS] Too many 'cached msgs' loaded.");
	}

	actorat[static_cast<int>(tilex)][static_cast<int>(tiley)] = reinterpret_cast<objtype*>(ConHintList.NumMsgs);
}

bool ReuseMsg(
	mCacheInfo* ci,
	std::int16_t count,
	std::int16_t struct_size)
{
	char* scan_ch = (char*)ci;
	mCacheInfo* scan = (mCacheInfo*)(scan_ch - struct_size);

	// Scan through all loaded messages -- see if we're loading one already
	// cached-in.
	//
	while (count--)
	{
		// Is this message already cached in?
		//
		if (scan->global_val == ci->global_val)
		{
			ci->local_val = scan->local_val;
			return true;
		}

		// Funky structure decrement... (structures can be any size...)
		//
		scan_ch = (char*)scan;
		scan_ch -= struct_size;
		scan = (mCacheInfo*)scan_ch;
	}

	return false;
}


extern std::string food_msg1;
extern std::string bevs_msg1;

extern void writeTokenStr(
	std::string& string);

const char* const OutOrder = "\r\r   FOOD UNIT MACHINE\r    IS OUT OF ORDER.^XX";

void OperateConcession(
	std::uint16_t concession)
{
	const auto& assets_info = get_assets_info();

	con_mCacheInfo* ci;
	bool ok = false;

	ci = &ConHintList.cmInfo[concession - 1];

	switch (ci->type)
	{
	case CT_FOOD:
	case CT_BEVS:
		if (ci->mInfo.local_val)
		{
			if (gamestate.health == 100)
			{
				DISPLAY_TIMED_MSG(noeat_msg1, MP_CONCESSION_OPERATE, MT_GENERAL);
				sd_play_player_item_sound(NOWAYSND);
				return;
			}
			else
			{
				ok = true;
			}
		}
		break;
	}

	if (ok)
	{
		// Whada' ya' want?

		switch (ci->type)
		{
		case CT_FOOD:
		case CT_BEVS:
			// One token please... Thank you.

			if (!gamestate.tokens)
			{
				DISPLAY_TIMED_MSG(NoFoodTokens, MP_NO_MORE_TOKENS, MT_NO_MO_FOOD_TOKENS);
				sd_play_player_item_sound(NOWAYSND);
				return;
			}
			else
			{
				gamestate.tokens--;
			}

			ci->mInfo.local_val--;
			sd_play_player_item_sound(CONCESSIONSSND);

			switch (ci->type)
			{
			case CT_FOOD:
				if (assets_info.is_ps())
				{
					writeTokenStr(food_msg1);
				}

				DISPLAY_TIMED_MSG(food_msg1, MP_CONCESSION_OPERATE, MT_GENERAL);
				HealSelf(10);
				break;

			case CT_BEVS:
				if (assets_info.is_ps())
				{
					writeTokenStr(bevs_msg1);
				}

				DISPLAY_TIMED_MSG(bevs_msg1, MP_CONCESSION_OPERATE, MT_GENERAL);
				HealSelf(7);
				break;
			}
			break;
		}
	}
	else
	{
		DISPLAY_TIMED_MSG(OutOrder, MP_CONCESSION_OUT_ORDER, MT_GENERAL);
		sd_play_player_item_sound(NOWAYSND);
	}
}

std::int8_t xy_offset[8][2] = {
	{0, -1}, {0, +1}, {-1, 0}, {+1, 0}, // vert / horz
{-1, -1}, {+1, +1}, {-1, +1}, {+1, -1}, // diagnals
};

void CheckSpawnEA()
{
	objtype temp, *ob;
	std::int8_t loop, ofs, x_diff, y_diff;

	if (objcount > MAXACTORS - 8)
	{
		return;
	}

	for (loop = 0; loop < NumEAWalls; loop++)
	{
		const auto map1 = &mapsegs[1][farmapylookup[static_cast<int>(eaList[static_cast<int>(loop)].tiley)] + eaList[static_cast<int>(loop)].tilex];

		// Limit the number of aliens spawned by each outlet.
		//
		if (eaList[static_cast<int>(loop)].aliens_out > gamestate.difficulty)
		{
			continue;
		}

		// Decrement 'spawn delay' for current outlet.
		//
		if (eaList[static_cast<int>(loop)].delay > tics)
		{
			eaList[static_cast<int>(loop)].delay -= tics;
			continue;
		}

		// Reset to 1 because it's possible that an alien won't be spawned...
		// If NOT, we'll try again on the next refresh.
		// If SO, the delay is set to a true value below.
		//
		eaList[static_cast<int>(loop)].delay = 1;

		// Does this wall touch the 'area' that the player is in?
		//
		for (ofs = 0; ofs < 4; ofs++)
		{
			const auto nx = eaList[static_cast<int>(loop)].tilex + xy_offset[static_cast<int>(ofs)][0];
			const auto ny = eaList[static_cast<int>(loop)].tiley + xy_offset[static_cast<int>(ofs)][1];

			if (nx < 0 || nx > 63 || ny < 0 || ny > 63)
			{
				continue;
			}

			const auto areanumber = GetAreaNumber(nx, ny);

			if (areanumber >= NUMAREAS)
			{
				continue;
			}

			if (areabyplayer[static_cast<int>(areanumber)])
			{
				break;
			}
		}

		// Wall doesn't touch player 'area'.
		//
		if (ofs == 4)
		{
			continue;
		}

		// Setup tile x,y in temp obj.
		//
		temp.tilex = eaList[static_cast<int>(loop)].tilex + xy_offset[static_cast<int>(ofs)][0];
		temp.tiley = eaList[static_cast<int>(loop)].tiley + xy_offset[static_cast<int>(ofs)][1];

		// Is another actor already on this tile?
		// If so, "continue" if it's alive...
		//
		ob = actorat[temp.tilex][temp.tiley];
		if (ob >= objlist)
		{
			if (!(ob->flags & FL_DEADGUY))
			{
				continue;
			}
		}

		// Is player already on this tile?
		//
		x_diff = player->tilex - temp.tilex;
		y_diff = player->tiley - temp.tiley;
		if (ABS(x_diff) < 2 && ABS(y_diff) < 2)
		{
			continue;
		}

		// Setup x,y in temp obj and see if obj is in player's view.
		// Actor is released if it's in player's view   OR
		// a random chance to release whether it can be seen or not.
		//
		temp.x = temp.tilex + 0.5;
		temp.y = temp.tiley + 0.5;

		if ((!CheckSight(player, &temp)) && (US_RndT() < 200))
		{
			continue;
		}

		// Spawn Electro-Alien!
		//
		usedummy = true;
		SpawnStand(en_electro_alien, temp.tilex, temp.tiley, 0);
		sd_play_actor_voice_sound(ELECAPPEARSND, *new_actor);
		usedummy = false;
		if (new_actor != &dummyobj)
		{
			eaList[static_cast<int>(loop)].aliens_out++;
			new_actor->temp2 = loop;
			sd_play_actor_voice_sound(ELECAPPEARSND, *new_actor);
		}

		// Reset spawn delay.
		//
		if ((*map1 & 0xff00) == 0xfa00)
		{
			eaList[static_cast<int>(loop)].delay = 60 * ((*map1) & 0xff);
		}
		else
		{
			eaList[static_cast<int>(loop)].delay = 60 * 8 + Random(60 * 22);
		}

		break;
	}
}

void CheckSpawnGoldstern()
{
	if (GoldsternInfo.WaitTime > tics)
	{
		//
		// Count down general timer before doing any Goldie Stuff..
		//

		GoldsternInfo.WaitTime -= tics;
	}
	else
	{
		//
		// What Kind of Goldie Stuff needs to be done?
		//

		if (GoldsternInfo.flags == GS_COORDFOUND)
		{
			std::uint16_t tilex, tiley;

			// See if we can spawn Dr. Goldstern...

			tilex = GoldieList[GoldsternInfo.LastIndex].tilex;
			tiley = GoldieList[GoldsternInfo.LastIndex].tiley;

			if ((!actorat[tilex][tiley]) && ABS(player->tilex - tilex) > 1 && ABS(player->tiley - tiley) > 1)
			{
				SpawnStand(en_goldstern, tilex, tiley, 0);
				GoldsternInfo.GoldSpawned = true;
			}
		}
		else
		{
			// Find a new coord to spawn Goldie (GS_NEEDCOORD or GS_FIRSTTIME)

			FindNewGoldieSpawnSite();
		}
	}
}

void FindNewGoldieSpawnSite()
{
	objtype temp;
	std::int8_t loop;

	GoldsternInfo.WaitTime = 0;

	for (loop = 0; loop < GoldsternInfo.SpawnCnt; loop++)
	{
		// Test for repeats - And avoid them!
		//

		if ((GoldsternInfo.SpawnCnt > 1) && (loop == GoldsternInfo.LastIndex))
		{
			continue;
		}

		// Setup tile x,y in temp obj.
		//

		temp.tilex = GoldieList[static_cast<int>(loop)].tilex;
		temp.tiley = GoldieList[static_cast<int>(loop)].tiley;

		// Setup x,y in temp obj and see if obj is in player's view.
		//

		temp.x = temp.tilex + 0.5;
		temp.y = temp.tiley + 0.5;

		if (!CheckSight(player, &temp))
		{
			continue;
		}

		// Mark to spawn Dr Goldstern
		//

		GoldsternInfo.LastIndex = loop;
		if (gamestate.mapon == 9)
		{
			GoldsternInfo.WaitTime = 60;
		}
		else if (GoldsternInfo.flags == GS_FIRSTTIME)
		{
			GoldsternInfo.WaitTime = MIN_GOLDIE_FIRST_WAIT + Random(MAX_GOLDIE_FIRST_WAIT - MIN_GOLDIE_FIRST_WAIT); // Reinit Delay Timer before spawning on new position
		}
		else
		{
			GoldsternInfo.WaitTime = MIN_GOLDIE_WAIT + Random(MAX_GOLDIE_WAIT - MIN_GOLDIE_WAIT); // Reinit Delay Timer before spawning on new position

		}
		GoldsternInfo.flags = GS_COORDFOUND;
		break;
	}
}
