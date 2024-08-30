/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cmath>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vl.h"

#include "bstone_exception.h"
#include "bstone_math.h"


void OpenDoor(
	std::int16_t door);

void A_DeathScream(
	objtype* ob);

void PlaceItemType(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley);

void PlaceItemNearTile(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley);

void ChangeShootMode(
	objtype* ob);

void SeekPlayerOrStatic(
	objtype* ob,
	std::int16_t* deltax,
	std::int16_t* deltay);

bool PlayerIsBlocking(
	objtype* ob);


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/


dirtype opposite[9] =
{west, southwest, south, southeast, east, northeast, north, northwest, nodir};

dirtype diagonal[9][9] = {
	/* east */{nodir, nodir, northeast, nodir, nodir, nodir, southeast, nodir, nodir},
{nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir},
/* north */{northeast, nodir, nodir, nodir, northwest, nodir, nodir, nodir, nodir},
{nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir},
/* west */{nodir, nodir, northwest, nodir, nodir, nodir, southwest, nodir, nodir},
{nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir},
/* south */{southeast, nodir, nodir, nodir, southwest, nodir, nodir, nodir, nodir},
{nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir},
{nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir, nodir}
};


void SpawnNewObj(
	std::uint16_t tilex,
	std::uint16_t tiley,
	statetype* state);

void NewState(
	objtype* ob,
	statetype* state);

bool TryWalk(
	objtype* ob,
	bool moveit);

void MoveObj(
	objtype* ob,
	const double move);

void KillActor(
	objtype* ob);

bool CheckLine(
	objtype* from_obj,
	objtype* to_obj);

void FirstSighting(
	objtype* ob);

bool CheckSight(
	objtype* from_obj,
	objtype* to_obj);

bool ElevatorFloor(
	std::int8_t x,
	std::int8_t y);

/*
===================
=
= SpawnNewObj
=
= Spaws a new actor at the given TILE coordinates, with the given state, and
= the given size in GLOBAL units.
=
= new = a pointer to an initialized new actor
=
===================
*/
void SpawnNewObj(
	std::uint16_t tilex,
	std::uint16_t tiley,
	statetype* state)
{
	GetNewActor();
	new_actor->state = state;
	new_actor->ticcount = static_cast<std::int16_t>(Random(static_cast<std::uint16_t>(
		state->tictime)) + 1);

	new_actor->tilex = static_cast<std::uint8_t>(tilex);
	new_actor->tiley = static_cast<std::uint8_t>(tiley);
	new_actor->x = tilex + 0.5;
	new_actor->y = tiley + 0.5;
	new_actor->dir = new_actor->trydir = nodir;

	if (!nevermark)
	{
		if (!actorat[tilex][tiley])
		{
			actorat[tilex][tiley] = new_actor;
		}
	}

	new_actor->areanumber = GetAreaNumber(new_actor->tilex, new_actor->tiley);
}

/*
===================
=
= NewState
=
= Changes ob to a new state, setting ticcount to the max for that state
=
===================
*/
void NewState(
	objtype* ob,
	statetype* state)
{
	ob->state = state;
	ob->ticcount = static_cast<std::int16_t>(state->tictime);
}


/*
=============================================================================

								ENEMY TILE WORLD MOVEMENT CODE

=============================================================================
*/

static bool CHECKDIAG(
	int x,
	int y)
{
	auto actor = actorat[x][y];
	auto temp = reinterpret_cast<std::size_t>(actor);

	if (temp != 0)
	{
		if (temp < 256)
		{
			return false;
		}

		if ((actor->flags & FL_SOLID) != 0)
		{
			return false;
		}
	}

	if (ElevatorFloor(static_cast<std::int8_t>(x), static_cast<std::int8_t>(y)))
	{
		return false;
	}

	return true;
}

static bool CHECKSIDE(
	int x,
	int y,
	std::int16_t& door_index)
{
	auto actor = actorat[x][y];
	auto temp = reinterpret_cast<std::size_t>(actor);

	if (temp != 0)
	{
		if (temp < 128)
		{
			return false;
		}

		if (temp < 256)
		{
			door_index = temp & 63;

			if (doorobjlist[door_index].lock != kt_none)
			{
				return false;
			}
		}
		else if ((actor->flags & FL_SOLID) != 0)
		{
			return false;
		}
	}

	return true;
}

/*
==================================
=
= TryWalk
=
= Attempts to move ob in its current (ob->dir) direction.
=
= If blocked by either a wall or an actor returns FALSE
=
= If move is either clear or blocked only by a door, returns TRUE and sets
=
= ob->tilex = new destination
= ob->tiley
= ob->areanumber = the floor tile number (0-(NUMAREAS-1)) of destination
= ob->distance = TILEGLOBAl, or -doornumber if a door is blocking the way
=
= If a door is in the way, an OpenDoor call is made to start it opening.
= The actor code should wait until
=       doorobjlist[-ob->distance].action = dr_open, meaning the door has been
=       fully opened
=
==================================
*/
bool TryWalk(
	objtype* ob,
	bool moveit)
{
	std::uint8_t old_tilex = ob->tilex, old_tiley = ob->tiley;

	if (ElevatorFloor(ob->tilex, ob->tiley))
	{
		return false;
	}

	std::int16_t door_index = -1;

	switch (ob->dir)
	{
	case north:
		if (!CHECKSIDE(ob->tilex, ob->tiley - 1, door_index))
		{
			return false;
		}

		if (ElevatorFloor(ob->tilex, ob->tiley - 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tiley--;
		break;

	case northeast:
		if (!CHECKDIAG(ob->tilex + 1, ob->tiley - 1))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex + 1, ob->tiley))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex, ob->tiley - 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex++;
		ob->tiley--;
		break;

	case east:
		if (!CHECKSIDE(ob->tilex + 1, ob->tiley, door_index))
		{
			return false;
		}

		if (ElevatorFloor(ob->tilex + 1, ob->tiley))
		{
			if ((door_index != -1) && (ob->obclass != electrosphereobj))
			{
				OpenDoor(door_index);
			}

			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex++;
		break;

	case southeast:
		if (!CHECKDIAG(ob->tilex + 1, ob->tiley + 1))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex + 1, ob->tiley))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex, ob->tiley + 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex++;
		ob->tiley++;
		break;

	case south:
		if (!CHECKSIDE(ob->tilex, ob->tiley + 1, door_index))
		{
			return false;
		}

		if (ElevatorFloor(ob->tilex, ob->tiley + 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tiley++;
		break;

	case southwest:
		if (!CHECKDIAG(ob->tilex - 1, ob->tiley + 1))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex - 1, ob->tiley))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex, ob->tiley + 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex--;
		ob->tiley++;
		break;

	case west:
		if (!CHECKSIDE(ob->tilex - 1, ob->tiley, door_index))
		{
			return false;
		}

		if (ElevatorFloor(ob->tilex - 1, ob->tiley))
		{
			if ((door_index != -1) && (ob->obclass != electrosphereobj))
			{
				OpenDoor(door_index);
			}

			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex--;
		break;

	case northwest:
		if (!CHECKDIAG(ob->tilex - 1, ob->tiley - 1))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex - 1, ob->tiley))
		{
			return false;
		}
		if (!CHECKDIAG(ob->tilex, ob->tiley - 1))
		{
			return false;
		}

		if (!moveit)
		{
			return true;
		}

		ob->tilex--;
		ob->tiley--;
		break;

	case nodir:
		return false;

	default:
		return false; // jam/jdebug
	}

	// Should actor open this door?
	//
	if (door_index != -1)
	{
		switch (ob->obclass)
		{
			// Actors that don't open doors.
			//
		case liquidobj:
		case electrosphereobj:
			ob->tilex = old_tilex;
			ob->tiley = old_tiley;
			return false;

			// All other actors open doors.
			//
		default:
			OpenDoor(door_index);
			ob->distance = -door_index - 1;
			return true;
		}
	}

	ob->areanumber = GetAreaNumber(ob->tilex, ob->tiley);
	ob->distance = 1.0;
	return true;
}

bool ElevatorFloor(
	std::int8_t x,
	std::int8_t y)
{
	std::uint8_t tile = static_cast<std::uint8_t>(mapsegs[0][farmapylookup[static_cast<int>(y)] + x]);

	if (tile >= HIDDENAREATILE)
	{
		tile -= HIDDENAREATILE;
	}
	else
	{
		tile -= AREATILE;
	}

	return tile == 0;
}

/*
==================================
=
= SelectDodgeDir
=
= Attempts to choose and initiate a movement for ob that sends it towards
= the player while dodging
=
= If there is no possible move (ob is totally surrounded)
=
= ob->dir = nodir
=
= Otherwise
=
= ob->dir = new direction to follow
= ob->distance = TILEGLOBAL or -doornumber
= ob->tilex = new destination
= ob->tiley
= ob->areanumber = the floor tile number (0-(NUMAREAS-1)) of destination
=
==================================
*/
void SelectDodgeDir(
	objtype* ob)
{
	std::int16_t deltax = 0, deltay = 0, i;
	std::uint16_t absdx, absdy;
	dirtype dirtry[5]{};
	dirtype turnaround, tdir;

	if (ob->flags & FL_FIRSTATTACK)
	{
		//
		// turning around is only ok the very first time after noticing the
		// player
		//
		turnaround = nodir;
		ob->flags &= ~FL_FIRSTATTACK;
	}
	else
	{
		turnaround = opposite[ob->dir];
	}

	SeekPlayerOrStatic(ob, &deltax, &deltay);

	//
	// arange 5 direction choices in order of preference
	// the four cardinal directions plus the diagonal straight towards
	// the player
	//

	if (deltax > 0)
	{
		dirtry[1] = east;
		dirtry[3] = west;
	}
	else if (deltax <= 0)
	{
		dirtry[1] = west;
		dirtry[3] = east;
	}

	if (deltay > 0)
	{
		dirtry[2] = south;
		dirtry[4] = north;
	}
	else if (deltay <= 0)
	{
		dirtry[2] = north;
		dirtry[4] = south;
	}

	//
	// randomize a bit for dodging
	//
	absdx = static_cast<std::uint16_t>(abs(deltax));
	absdy = static_cast<std::uint16_t>(abs(deltay));

	if (absdx > absdy)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	if (US_RndT() < 128)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	dirtry[0] = diagonal[dirtry[1]][dirtry[2]];

	//
	// try the directions util one works
	//
	for (i = 0; i < 5; i++)
	{
		if (dirtry[i] == nodir || dirtry[i] == turnaround)
		{
			continue;
		}

		ob->dir = dirtry[i];
		if (TryWalk(ob, true))
		{
			return;
		}
	}

	//
	// turn around only as a last resort
	//
	if (turnaround != nodir)
	{
		ob->dir = turnaround;

		if (TryWalk(ob, true))
		{
			return;
		}
	}

	ob->dir = nodir;

	if (ob->obclass == electrosphereobj)
	{
		ob->s_tilex = 0;
	}
}

/*
============================
=
= SelectChaseDir
=
= As SelectDodgeDir, but doesn't try to dodge
=
============================
*/
void SelectChaseDir(
	objtype* ob)
{
	std::int16_t deltax = 0, deltay = 0;
	dirtype d[3]{};
	dirtype tdir, olddir, turnaround;


	olddir = ob->dir;
	turnaround = opposite[olddir];

	SeekPlayerOrStatic(ob, &deltax, &deltay);

	d[1] = nodir;
	d[2] = nodir;

	if (deltax > 0)
	{
		d[1] = east;
	}
	else if (deltax < 0)
	{
		d[1] = west;
	}
	if (deltay > 0)
	{
		d[2] = south;
	}
	else if (deltay < 0)
	{
		d[2] = north;
	}

	if (abs(deltay) > abs(deltax))
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] == turnaround)
	{
		d[1] = nodir;
	}
	if (d[2] == turnaround)
	{
		d[2] = nodir;
	}


	if (d[1] != nodir)
	{
		ob->dir = d[1];
		if (TryWalk(ob, true))
		{
			return; /*either moved forward or attacked*/
		}
	}

	if (d[2] != nodir)
	{
		ob->dir = d[2];
		if (TryWalk(ob, true))
		{
			return;
		}
	}

	/* there is no direct path to the player, so pick another direction */

	if (olddir != nodir)
	{
		ob->dir = olddir;
		if (TryWalk(ob, true))
		{
			return;
		}
	}

	if (US_RndT() > 128)
	{ /*randomly determine direction of search*/
		for (tdir = north; tdir <= west; tdir++)
		{
			if (tdir != turnaround)
			{
				ob->dir = tdir;
				if (TryWalk(ob, true))
				{
					return;
				}
			}
		}
	}
	else
	{
		for (tdir = west; tdir >= north; tdir--)
		{
			if (tdir != turnaround)
			{
				ob->dir = tdir;
				if (TryWalk(ob, true))
				{
					return;
				}
			}
		}
	}

	if (turnaround != nodir)
	{
		ob->dir = turnaround;
		if (ob->dir != nodir)
		{
			if (TryWalk(ob, true))
			{
				return;
			}
		}
	}

	ob->dir = nodir; // can't move
	if (ob->obclass == electrosphereobj)
	{
		ob->s_tilex = 0;
	}
}

void GetCornerSeek(
	objtype* ob)
{
	std::uint8_t SeekPointX[] = {32, 63, 32, 1}; // s_tilex can't seek to 0!
	std::uint8_t SeekPointY[] = {1, 63, 32, 1};
	std::uint8_t seek_tile = US_RndT() & 3;

	ob->flags &= ~FL_RUNTOSTATIC;
	ob->s_tilex = SeekPointX[seek_tile];
	ob->s_tiley = SeekPointY[seek_tile];
}


extern std::int32_t last_objy;

//
// Moves ob be move global units in ob->dir direction
// Actors are not allowed to move inside the player
// Does NOT check to see if the move is tile map valid
//
// ob->x = adjusted for new position
// ob->y
//
void MoveObj(
	objtype* ob,
	const double move)
{
	auto sign_x = 0;
	auto sign_y = 0;

	switch (ob->dir)
	{
		case north:
			sign_y = -1;
			break;

		case northeast:
			sign_x = 1;
			sign_y = -1;
			break;

		case east:
			sign_x = 1;
			break;

		case southeast:
			sign_x = 1;
			sign_y = 1;
			break;

		case south:
			sign_y = 1;
			break;

		case southwest:
			sign_x = -1;
			sign_y = 1;
			break;

		case west:
			sign_x = -1;
			break;

		case northwest:
			sign_x = -1;
			sign_y = -1;
			break;

		case nodir:
			return;

		default:
			BSTONE_THROW_STATIC_SOURCE("Illegal direction passed.");
	}

	ob->x += sign_x * move;
	ob->y += sign_y * move;

	//
	// check to make sure it's not on top of player
	//
	if (ob->obclass != electrosphereobj &&
		ob->areanumber < NUMAREAS &&
		areabyplayer[ob->areanumber])
	{
		const auto dx = std::abs(ob->x - player->x);
		const auto dy = std::abs(ob->y - player->y);

		if (!(dx > MINACTORDIST || dy > MINACTORDIST))
		{
			//
			// back up
			//

			sign_x = -sign_x;
			sign_y = -sign_y;

			ob->x += sign_x * move;
			ob->y += sign_y * move;

			PlayerIsBlocking(ob);

			return;
		}
	}

	ob->distance -= move;
}


extern statetype s_terrot_die1;

char dki_msg[] =
"^FC39  YOU JUST SHOT AN\r"
"	    INFORMANT!\r"
"^FC79 ONLY SHOOT BIO-TECHS\r"
"  THAT SHOOT AT YOU!\r"
"^FC19	    DO NOT SHOOT\r"
"	    INFORMANTS!!\r";

const ActorPoints actor_points =
{
	1025, // rent-a-cop
	1050, // turret
	500, // general scientist
	5075, // pod alien
	5150, // electric alien
	2055, // electro-sphere
	5000, // pro guard
	10000, // genetic guard
	5055, // mutant human1
	6055, // mutant human2
	0, // large canister wait
	6050, // large canister alien
	0, // small canister wait
	3750, // small canister alien
	0, // gurney wait
	3750, // gurney
	12000, // liquid
	7025, // swat
	5000, // goldtern
	5000, // goldstern Morphed
	2025, // volatile transport
	2025, // floating bomb
	50000, // projection generator

	5000, // spider_mutant
	6000, // breather_beast
	7000, // cyborg_warror
	8000, // reptilian_warrior
	9000, // acid_dragon
	9000, // mech_guardian
	30000, // final boss #1
	40000, // final_boss #2
	50000, // final_boss #3
	60000, // final_boss #4

	0, // blake
	0, // crate 1
	0, // crate 2
	0, // crate 3
	0, // green ooze
	0, // black ooze
	0, // green ooze 2
	0, // black ooze 2
	0, // pod egg

	5000, // morphing_spider_mutant
	8000, // morphing_reptilian_warrior
	6055, // morphing_mutant human2
};

// ---------------------------------------------------------------------------
//  CheckAndReserve() - Checks for room in the obj_list and returns a ptr
//      to the new object or a nullptr.
//
// ---------------------------------------------------------------------------
objtype* CheckAndReserve()
{
	usedummy = nevermark = true;
	SpawnNewObj(0, 0, &s_hold);
	usedummy = nevermark = false;

	if (new_actor == &dummyobj)
	{
		return nullptr;
	}
	else
	{
		return new_actor;
	}
}

void KillActor(
	objtype* ob)
{
	std::int16_t tilex, tiley;
	bool KeepSolid = false;
	bool givepoints = true;
	bool deadguy = true;
	classtype clas;

	tilex = static_cast<std::int16_t>(ob->x); // drop item on center
	tiley = static_cast<std::int16_t>(ob->y);

	ob->flags &= ~(FL_FRIENDLY | FL_SHOOTABLE);
	clas = ob->obclass;

	const auto& assets_info = get_assets_info();

	switch (clas)
	{
	case podeggobj:
		sd_play_actor_voice_sound(PODHATCHSND, *ob);
		InitSmartSpeedAnim(ob, SPR_POD_HATCH1, 0, 2, at_ONCE, ad_FWD, assets_info.is_aog() ? 45 : 7);
		KeepSolid = true;

		if (assets_info.is_aog())
		{
			givepoints = false;
		}
		else
		{
			deadguy = false;
			givepoints = false;
		}

		break;

	case morphing_spider_mutantobj:
	case morphing_reptilian_warriorobj:
	case morphing_mutanthuman2obj:
		if (assets_info.is_ps())
		{
			ob->flags &= ~FL_SHOOTABLE;
			InitSmartSpeedAnim(ob, ob->temp1, 0, 8, at_ONCE, ad_FWD, 2);
			KeepSolid = true;
			deadguy = givepoints = false;
		}

		break;

	case crate1obj:
	case crate2obj:
	case crate3obj:
		{
			auto bs_static = ui16_to_static_object(ob->temp3);

			bs_static->shapenum = -1;
			vid_hw_on_remove_static(*bs_static);

			static_cast<void>(SpawnStatic(tilex, tiley, ob->temp2));
		}

		ob->obclass = (assets_info.is_aog() ? explosionobj : deadobj);
		ob->lighting = NO_SHADING; // No Shading

		InitSmartSpeedAnim(
			ob,
			SPR_GRENADE_EXPLODE2,
			0,
			3,
			at_ONCE,
			ad_FWD,
			(assets_info.is_aog() ? 24 : 3) + (US_RndT() & 7)
		);

		A_DeathScream(ob);
		MakeAlertNoise(ob);
		break;

	case floatingbombobj:
		ob->lighting = EXPLOSION_SHADING;
		A_DeathScream(ob);
		InitSmartSpeedAnim(ob, SPR_FSCOUT_DIE1, 0, 7, at_ONCE, ad_FWD, assets_info.is_ps() ? 5 : 20);
		break;

	case volatiletransportobj:
		ob->lighting = EXPLOSION_SHADING;
		A_DeathScream(ob);
		InitSmartSpeedAnim(ob, SPR_GSCOUT_DIE1, 0, 8, at_ONCE, ad_FWD, assets_info.is_ps() ? 5 : 20);
		break;

	case goldsternobj:
		NewState(ob, &s_goldwarp_it);
		GoldsternInfo.flags = GS_NEEDCOORD;
		GoldsternInfo.GoldSpawned = false;

		// Init timer.  Search for a location out of all possible locations.

		GoldsternInfo.WaitTime = MIN_GOLDIE_WAIT + Random(MAX_GOLDIE_WAIT - MIN_GOLDIE_WAIT); // Reinit Delay Timer before spawning on new position
		clas = goldsternobj;

		if (assets_info.is_aog() &&
			gamestate.mapon == 9 &&
			!gamestate.boss_key_dropped)
		{
			gamestate.boss_key_dropped = true;

			static_cast<void>(ReserveStatic());
			PlaceReservedItemNearTile(bo_gold_key, ob->tilex, ob->tiley);
		}

		break;

	case gold_morphobj:
		GoldsternInfo.flags = GS_NO_MORE;
		sd_play_actor_voice_sound(PODDEATHSND, *ob);
		ob->flags |= FL_OFFSET_STATES;
		InitAnim(ob, SPR_GOLD_DEATH1, 0, 4, at_ONCE, ad_FWD, 25, 9);
		break;

	case gen_scientistobj:
		if (ob->flags & FL_INFORMANT)
		{
			givepoints = false;
			clas = nothing;
			gamestuff.level[gamestate.mapon].stats.accum_inf--;
			if (!(gamestate.flags & GS_KILL_INF_WARN) || (US_RndT() < 25))
			{
				DisplayInfoMsg(dki_msg, static_cast<msg_priorities>(MP_INTERROGATE - 1), DISPLAY_MSG_STD_TIME * 3, MT_GENERAL);
				gamestate.flags |= GS_KILL_INF_WARN;
			}
		}
		NewState(ob, &s_ofcdie1);
		if ((ob->ammo) && !(ob->flags & FL_INFORMANT))
		{
			if (US_RndT() < 65)
			{
				PlaceItemType(bo_coin, tilex, tiley);
			}
			else
			{
				PlaceItemType(bo_clip2, tilex, tiley);
			}
		}
		break;

	case rentacopobj:
		NewState(ob, &s_rent_die1);
		if (!(gamestate.weapons & (1 << wp_pistol)))
		{
			PlaceItemType(bo_pistol, tilex, tiley);
		}
		else if (US_RndT() < 65 || (!ob->ammo))
		{
			PlaceItemType(bo_coin, tilex, tiley);
		}
		else if (ob->ammo)
		{
			PlaceItemType(bo_clip2, tilex, tiley);
		}
		break;

	case swatobj:
		NewState(ob, &s_swatdie1);
		if (!(gamestate.weapons & (1 << wp_burst_rifle)))
		{
			PlaceItemType(bo_burst_rifle, tilex, tiley);
		}
		else if (US_RndT() < 65 || (!ob->ammo))
		{
			PlaceItemType(bo_coin, tilex, tiley);
		}
		else if (ob->ammo)
		{
			PlaceItemType(bo_clip2, tilex, tiley);
		}
		break;


	case proguardobj:
		NewState(ob, &s_prodie1);
		if (!(gamestate.weapons & (1 << wp_burst_rifle)))
		{
			PlaceItemType(bo_burst_rifle, tilex, tiley);
		}
		else if (US_RndT() < 65 || (!ob->ammo))
		{
			PlaceItemType(bo_coin, tilex, tiley);
		}
		else if (ob->ammo)
		{
			PlaceItemType(bo_clip2, tilex, tiley);
		}
		break;

	case electroobj:
		NewState(ob, &s_electro_die1);
		eaList[ob->temp2].aliens_out--;
		ob->obclass = nothing;
		actorat[ob->tilex][ob->tiley] = nullptr;
		break;

	case liquidobj:
		NewState(ob, &s_liquid_die1);
		ob->obclass = nothing;
		actorat[ob->tilex][ob->tiley] = nullptr;
		break;

	case podobj:
		ob->temp1 = SPR_POD_DIE1;
		NewState(ob, &s_ofs_pod_death1);
		A_DeathScream(ob);
		break;

	case electrosphereobj:
		ob->obclass = nothing;
		ob->temp1 = SPR_ELECTRO_SPHERE_DIE1;
		NewState(ob, &s_ofs_esphere_death1);
		actorat[ob->tilex][ob->tiley] = nullptr;
		break;

	case mutant_human1obj:
		PlaceItemNearTile(bo_clip2, tilex, tiley);
	case final_boss3obj:
	case final_boss4obj:
	case mutant_human2obj:
	case scan_alienobj:
	case lcan_alienobj:
		NewState(ob, &s_ofs_die1);
		break;

	case cyborg_warriorobj:
	case mech_guardianobj:
	case reptilian_warriorobj:
		if (assets_info.is_ps())
		{
			PlaceItemNearTile(bo_clip2, tilex, tiley);
		}
	case spider_mutantobj:
	case breather_beastobj:
	case acid_dragonobj:
		NewState(ob, &s_ofs_die1);

		if (!assets_info.is_ps())
		{
			static_cast<void>(ReserveStatic());
			PlaceReservedItemNearTile(bo_gold_key, ob->tilex, ob->tiley);
			ActivatePinballBonus(B_GALIEN_DESTROYED);
		}
		break;

	case final_boss2obj:
		sd_play_actor_voice_sound(PODDEATHSND, *ob);
		InitAnim(ob, SPR_BOSS8_DIE1, 0, 4, at_ONCE, ad_FWD, 25, 9);
		break;

	case genetic_guardobj:
	case final_boss1obj:
	case gurneyobj:
		if (!(gamestate.weapons & (1 << wp_pistol)))
		{
			PlaceItemNearTile(bo_pistol, tilex, tiley);
		}
		else
		{
			PlaceItemNearTile(bo_clip2, tilex, tiley);
		}
		NewState(ob, &s_ofs_die1);
		break;

	case gurney_waitobj: // mutant asleep on gurney
		InitSmartAnim(ob, SPR_GURNEY_MUT_B1, 0, 3, at_ONCE, ad_FWD);
		KeepSolid = true;
		givepoints = false;
		break;

	case scan_wait_alienobj: // Actual Canister - Destroyed
		InitSmartAnim(ob, SPR_SCAN_ALIEN_B1, 0, 3, at_ONCE, ad_FWD);
		KeepSolid = true;
		givepoints = false;
		break;

	case lcan_wait_alienobj: // Actual Canister - Destroyed
		InitSmartAnim(ob, SPR_LCAN_ALIEN_B1, 0, 3, at_ONCE, ad_FWD);
		KeepSolid = true;
		givepoints = false;
		break;

	case hang_terrotobj:
		NewState(ob, &s_terrot_die1);
		ob->lighting = EXPLOSION_SHADING;
		break;

	case rotating_cubeobj:
		if (assets_info.is_ps() || assets_info.is_aog_sw())
		{
			givepoints = false;
		}
		else
		{
			A_DeathScream(ob);
			ob->ammo = 0;
			ob->lighting = EXPLOSION_SHADING;
			InitSmartSpeedAnim(ob, SPR_VITAL_DIE_1, 0, 7, at_ONCE, ad_FWD, 7);
		}
		break;

	default:
		break;
	}

#if LOOK_FOR_DEAD_GUYS
	switch (clas)
	{
	case SMART_ACTORS:
		DeadGuys[NumDeadGuys++] = ob;
		break;
	}
#endif

	if (KeepSolid)
	{
		ob->flags &= ~(FL_SHOOTABLE);

		if (assets_info.is_ps())
		{
			ob->flags2 &= ~FL2_BFG_SHOOTABLE;
		}

		if (deadguy)
		{
			ob->flags |= FL_DEADGUY;
		}
	}
	else
	{
		if (deadguy)
		{
			ob->flags |= (FL_NONMARK | FL_DEADGUY);
		}

		if (clas >= rentacopobj && clas < crate1obj && clas != electroobj && clas != goldsternobj)
		{
			gamestuff.level[gamestate.mapon].stats.accum_enemy++;
		}

		if (givepoints)
		{
			if ((clas == electroobj) || (clas == goldsternobj))
			{
				GivePoints(actor_points[clas - rentacopobj], false);
			}
			else
			{
				GivePoints(actor_points[clas - rentacopobj], true);
			}
		}

		ob->flags &= ~(FL_SHOOTABLE | FL_SOLID | FL_FAKE_STATIC);

		if (assets_info.is_ps())
		{
			ob->flags2 &= ~FL2_BFGSHOT_SOLID;
		}

		if ((actorat[ob->tilex][ob->tiley]) == ob)
		{
			// Clear actor from WHERE IT WAS GOING in actorat[].
			//
			if (!(tilemap[ob->tilex][ob->tiley] & 0x80))
			{
				actorat[ob->tilex][ob->tiley] = nullptr;
			}

			// Set actor WHERE IT DIED in actorat[], IF there's a door!
			// Otherwise, just leave it removed!
			//
			if (tilemap[tilex][tiley] & 0x80)
			{
				actorat[tilex][tiley] = ob;
			}
			else
			{
				ob->flags |= FL_NEVERMARK;
			}
		}
	}

	DropCargo(ob);

	ob->tilex = static_cast<std::uint8_t>(tilex);
	ob->tiley = static_cast<std::uint8_t>(tiley);

	if ((LastMsgPri == MP_TAKE_DAMAGE) && (LastInfoAttacker == clas))
	{
		MsgTicsRemain = 1;
	}

	switch (clas)
	{
	case electroobj:
	case liquidobj:
	case electrosphereobj:
		ob->obclass = clas;
		ob->flags |= FL_NEVERMARK;
		break;

	default:
		break;
	}
}

void DoAttack(
	objtype* ob);

extern statetype s_proshoot2;
extern statetype s_goldmorphwait1;
extern bool barrier_damage;

/*
===================
=
= DamageActor
=
= Called when the player successfully hits an enemy.
=
= Does damage points to enemy ob, either putting it into a stun frame or
= killing it.
=
===================
*/
void DamageActor(
	objtype* ob,
	std::uint16_t damage,
	objtype* attacker)
{
	std::int16_t old_hp = ob->hitpoints, wound_mod, mod_before = 0, mod_after = 1;

	if (!(ob->flags & FL_SHOOTABLE))
	{
		return;
	}

	if (gamestate.weapon != wp_autocharge)
	{
		MakeAlertNoise(player);
	}

	if (ob->flags & FL_FREEZE)
	{
		return;
	}

	const auto& assets_info = get_assets_info();

	switch (ob->obclass)
	{
	case hang_terrotobj:
		if (gamestate.weapon < wp_burst_rifle)
		{
			return;
		}
		break;

	case gurney_waitobj:
		if (ob->temp3)
		{
			return;
		}
		break;

	case arc_barrierobj:
		if (attacker->obclass == bfg_shotobj)
		{
			if (BARRIER_STATE(ob) != bt_DISABLING)
			{
				BARRIER_STATE(ob) = bt_DISABLING;
				ob->hitpoints = 15;
				ob->temp3 = 0;
				ob->temp2 = US_RndT() & 0xf;
				NewState(ob, &s_barrier_shutdown);
			}
		}
		return;

	case rotating_cubeobj:
		if (assets_info.is_ps())
		{
			return;
		}
		break;

	case post_barrierobj:
		return;

	case plasma_detonatorobj:
		//
		// Detonate 'Em!
		//
		if (attacker == player)
		{
			ob->temp3 = 1;
		}
		else
		{
			ob->temp3 = damage;
		}
		return;

	default:
		break;
	}

	//
	// do double damage if shooting a non attack mode actor
	//
	if (!(ob->flags & FL_ATTACKMODE))
	{
		damage <<= 1;
	}

	ob->hitpoints -= damage;
	ob->flags2 |= (assets_info.is_ps() ? FL2_DAMAGE_CLOAK : 0);

	if (ob->hitpoints <= 0)
	{
		switch (ob->obclass)
		{
// FIXME
// Remove this or convert the reserved actor into real one.
#if 0
		case scan_wait_alienobj: // These actors do not have an ouch!
		case lcan_wait_alienobj: // So... RETURN!
		case gurney_waitobj:
			ob->temp2 = actor_to_ui16(CheckAndReserve());

			if (ob->temp2 == 0)
			{
				ob->hitpoints += damage;
				return;
			}
			break;
#endif

		case goldsternobj:
			if (gamestate.mapon == GOLD_MORPH_LEVEL)
			{
				extern std::int16_t morphWaitTime;
				extern bool noShots;

				morphWaitTime = 60;
				noShots = true;
				NewState(ob, &s_goldmorphwait1);
				ob->obclass = gold_morphingobj;
				ob->flags &= ~FL_SHOOTABLE;
				return;
			}
			break;

		default:
			break;

		}

		ob->hitpoints = actor_to_ui16(attacker);

		KillActor(ob);
		return;
	}
	else
	{
		switch (ob->obclass)
		{
		case swatobj:
			// Don't get wounded if it's an arc!
			//
			if ((attacker->obclass == arc_barrierobj) ||
				(attacker->obclass == post_barrierobj))
			{
				break;
			}

			// Calculate 'wound boundary' (based on NUM_WOUND_STAGES).
			//
			wound_mod = get_start_hit_point(en_swat) / (ob->temp1 + 1) + 1;
			mod_before = old_hp / wound_mod;
			mod_after = ob->hitpoints / wound_mod;

			// If modulo 'before' and 'after' are different, we've crossed
			// a 'wound boundary'!
			//
			if (mod_before != mod_after)
			{
				if (!assets_info.is_aog_sw())
				{
					sd_play_actor_voice_sound(SWATDEATH2SND, *ob);
				}

				NewState(ob, &s_swatwounded1);
				ob->flags &= ~(FL_SHOOTABLE | FL_SOLID);
				ob->temp2 = (5 * 60) + ((US_RndT() % 20) * 60);
				return;
			}
			break;

		default:
			break;
		}

		if (ob->flags & FL_LOCKED_STATE)
		{
			return;
		}

		if (!(ob->flags & FL_ATTACKMODE))
		{
			if ((ob->obclass == gen_scientistobj) && (ob->flags & FL_INFORMANT))
			{
				return;
			}
			FirstSighting(ob); // put into combat mode
		}

		switch (ob->obclass)
		{
		case volatiletransportobj:
		case floatingbombobj:
			T_PainThink(ob);
			break;

		case goldsternobj:
			NewState(ob, &s_goldpain);
			break;

		case gold_morphobj:
			NewState(ob, &s_mgold_pain);
			break;

		case liquidobj:
			NewState(ob, &s_liquid_ouch);
			break;

		case rentacopobj:
			NewState(ob, &s_rent_pain);
			break;

		case podobj:
			NewState(ob, &s_ofs_pod_ouch);
			break;

		case spider_mutantobj:
		case breather_beastobj:
		case cyborg_warriorobj:
		case reptilian_warriorobj:
		case acid_dragonobj:
		case mech_guardianobj:
		case final_boss1obj:
		case final_boss2obj:
		case final_boss3obj:
		case final_boss4obj:

		case genetic_guardobj:
		case mutant_human1obj:
		case mutant_human2obj:
		case scan_alienobj:
		case lcan_alienobj:
		case gurneyobj:
			NewState(ob, &s_ofs_pain);
			break;

		case electrosphereobj:
			NewState(ob, &s_ofs_ouch);
			ob->temp1 = SPR_ELECTRO_SPHERE_OUCH;
			break;

		case electroobj:
			NewState(ob, &s_electro_ouch);
			break;

		case gen_scientistobj:
			NewState(ob, &s_ofcpain);
			break;

		case swatobj:
			NewState(ob, &s_swatpain);
			break;

		case proguardobj:
			NewState(ob, &s_propain);
			break;

		case rotating_cubeobj:
			if (assets_info.is_ps())
			{
				break;
			}

			// Show 'pain' animation only once
			if ((ob->hitpoints + damage) == get_start_hit_point(en_rotating_cube))
			{
				InitSmartSpeedAnim(ob, SPR_VITAL_OUCH, 0, 0, at_ONCE, ad_FWD, 23);
			}
			break;

		default:
			break;
		}
	}

	// Make sure actors aren't sitting ducks!
	//

	if ((US_RndT() < 192) &&
		(!(ob->flags & (FL_LOCKED_STATE | FL_BARRIER_DAMAGE))))
	{
		ChangeShootMode(ob);
		DoAttack(ob);
	}

	ob->flags |= FL_LOCKED_STATE;
}


/*
=============================================================================

 CHECKSIGHT

=============================================================================
*/

/*
=====================
=
= CheckLine
=
= Returns true if a straight line between the player and ob is unobstructed
=
=====================
*/
bool CheckLine(
	objtype* from_obj,
	objtype* to_obj)
{
	const auto x1 = bstone::math::floating_to_fixed(from_obj->x) >> UNSIGNEDSHIFT; // 1/256 tile precision
	const auto y1 = bstone::math::floating_to_fixed(from_obj->y) >> UNSIGNEDSHIFT;
	const auto xt1 = x1 >> 8;
	const auto yt1 = y1 >> 8;

	//      x2 = plux;
	//      y2 = pluy;

	const auto x2 = bstone::math::floating_to_fixed(to_obj->x) >> UNSIGNEDSHIFT;
	const auto y2 = bstone::math::floating_to_fixed(to_obj->y) >> UNSIGNEDSHIFT;
	auto xt2 = to_obj->tilex;
	auto yt2 = to_obj->tiley;

	if (abs(xt2 - xt1) > 0)
	{
		auto xstep = 0;
		auto ystep = 0;
		auto partial = 0;

		if (xt2 > xt1)
		{
			partial = 256 - (x1 & 0xFF);
			xstep = 1;
		}
		else
		{
			partial = x1 & 0xFF;
			xstep = -1;
		}

		auto deltafrac = abs(x2 - x1);

		if (deltafrac == 0)
		{
			deltafrac = 1;
		}

		const auto delta = y2 - y1;
		const auto ltemp = (delta << 8) / deltafrac;

		if (ltemp > 0x7FFF)
		{
			ystep = 0x7FFF;
		}
		else if (ltemp < -0x7FFF)
		{
			ystep = -0x7FFF;
		}
		else
		{
			ystep = ltemp;
		}

		auto yfrac = y1 + ((ystep * partial) >> 8);

		auto x = xt1 + xstep;
		xt2 += static_cast<std::uint8_t>(xstep);

		do
		{
			auto y = yfrac >> 8;
			yfrac += ystep;

			auto value = tilemap[x][y];

			x += xstep;

			if (value == 0)
			{
				continue;
			}

			if (value < 128 || (value & 0xC0) == 0xC0)
			{
				return false;
			}

			//
			// see if the door is open enough
			//
			value &= ~0x80;
			std::uint16_t intercept = static_cast<std::uint16_t>(yfrac - ystep / 2);

			if (intercept > bstone::math::floating_to_fixed(doorposition[value]))
			{
				return false;
			}
		} while (x != xt2);
	}

	if (abs(yt2 - yt1) > 0)
	{
		auto xstep = 0;
		auto ystep = 0;
		auto partial = 0;

		if (yt2 > yt1)
		{
			partial = 256 - (y1 & 0xFF);
			ystep = 1;
		}
		else
		{
			partial = y1 & 0xFF;
			ystep = -1;
		}

		auto deltafrac = abs(y2 - y1);

		if (deltafrac == 0)
		{
			deltafrac = 1;
		}

		const auto delta = x2 - x1;
		const auto ltemp = (delta << 8) / deltafrac;

		if (ltemp > 0x7FFF)
		{
			xstep = 0x7FFF;
		}
		else if (ltemp < -0x7FFF)
		{
			xstep = -0x7FFF;
		}
		else
		{
			xstep = ltemp;
		}

		auto xfrac = x1 + ((xstep * partial) >> 8);

		auto y = yt1 + ystep;
		yt2 += static_cast<std::uint8_t>(ystep);

		do
		{
			auto x = xfrac >> 8;
			xfrac += xstep;

			auto value = tilemap[x][y];
			y += ystep;

			if (value == 0)
			{
				continue;
			}

			if (value < 128 || (value & 0xC0) == 0xC0)
			{
				return false;
			}

			//
			// see if the door is open enough
			//
			value &= ~0x80;
			std::uint16_t intercept = static_cast<std::uint16_t>(xfrac - xstep / 2);

			if (intercept > bstone::math::floating_to_fixed(doorposition[value]))
			{
				return false;
			}
		} while (y != yt2);
	}

	return true;
}

//
// Checks a straight line between player and current object
//
// If the sight is ok, check alertness and angle to see if they notice
//
// returns true if the player has been spoted
//
bool CheckSight(
	objtype* from_obj,
	objtype* to_obj)
{
	//
	// don't bother tracing a line if the area isn't connected to the player's
	//
	if (from_obj->areanumber < NUMAREAS && !areabyplayer[from_obj->areanumber])
	{
		return false;
	}

	//
	// if the player is real close, sight is automatic
	//
	const auto deltax = to_obj->x - from_obj->x;
	const auto deltay = to_obj->y - from_obj->y;

	constexpr auto MINSIGHT = bstone::math::fixed_to_floating(0x18000);

	if (std::abs(deltax) <= MINSIGHT && std::abs(deltay) <= MINSIGHT)
	{
		return true;
	}

	//
	// see if they are looking in the right direction
	//
	switch (from_obj->dir)
	{
		case north:
			if (deltay > 0.0)
			{
				return false;
			}
			break;

		case east:
			if (deltax < 0.0)
			{
				return false;
			}
			break;

		case south:
			if (deltay < 0.0)
			{
				return false;
			}
			break;

		case west:
			if (deltax > 0.0)
			{
				return false;
			}
			break;

		default:
			break;
	}

	//
	// trace a line to check for blocking tiles (corners)
	//
	return CheckLine(from_obj, to_obj);
}



/*
===============
=
= FirstSighting
=
= Puts an actor into attack mode and possibly reverses the direction
= if the player is behind it
=
===============
*/
void FirstSighting(
	objtype* ob)
{
	if (PlayerInvisable)
	{
		return;
	}

	//
	// react to the player
	//
	switch (ob->obclass)
	{
	case floatingbombobj:
		if (ob->flags & FL_STATIONARY)
		{
			return;
		}

		sd_play_actor_voice_sound(SCOUT_ALERTSND, *ob);
		NewState(ob, &s_scout_run);
		ob->speed *= 3; // Haul Ass
		break;


	case goldsternobj:
		sd_play_actor_voice_sound(GOLDSTERNHALTSND, *ob);
		NewState(ob, &s_goldchase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case rentacopobj:
		sd_play_actor_voice_sound(HALTSND, *ob);
		NewState(ob, &s_rent_chase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case gen_scientistobj:
		sd_play_actor_voice_sound(SCIENTISTHALTSND, *ob);
		NewState(ob, &s_ofcchase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case swatobj:
		sd_play_actor_voice_sound(SWATHALTSND, *ob);
		NewState(ob, &s_swatchase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case breather_beastobj:
	case reptilian_warriorobj:
	case genetic_guardobj:
	case final_boss4obj:
	case final_boss2obj:
		sd_play_actor_voice_sound(GGUARDHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 3; // go faster when chasing player
		break;


	case cyborg_warriorobj:
	case mech_guardianobj:
	case mutant_human1obj:
	case final_boss3obj:
	case final_boss1obj:
		sd_play_actor_voice_sound(BLUEBOYHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 2; // go faster when chasing player
		break;

	case mutant_human2obj:
		sd_play_actor_voice_sound(DOGBOYHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 2; // go faster when chasing player
		break;

	case liquidobj:
		NewState(ob, &s_liquid_move);
		break;

	case spider_mutantobj:
	case scan_alienobj:
		sd_play_actor_voice_sound(SCANHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case lcan_alienobj:
		sd_play_actor_voice_sound(LCANHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case gurneyobj:
		sd_play_actor_voice_sound(GURNEYSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 3; // go faster when chasing player
		break;

	case acid_dragonobj:
	case podobj:
		sd_play_actor_voice_sound(PODHALTSND, *ob);
		NewState(ob, &s_ofs_chase1);
		ob->speed *= 2;
		break;

	case gurney_waitobj:
		if (ob->temp3)
		{
			ob->temp2 = actor_to_ui16(CheckAndReserve());

			if (ob->temp2 != 0)
			{
				ob->flags &= ~(FL_SHOOTABLE);
				InitSmartAnim(ob, SPR_GURNEY_MUT_B1, 0, 3, at_ONCE, ad_FWD);
			}
			else
			{
				return;
			}
		}
		break;

	case proguardobj:
		sd_play_actor_voice_sound(PROHALTSND, *ob);
		NewState(ob, &s_prochase1);
		ob->speed *= 4; // go faster when chasing player
		break;

	case hang_terrotobj:
		sd_play_actor_voice_sound(TURRETSND, *ob);
		NewState(ob, &s_terrot_seek1);
		break;

	default:
		break;

	}

	ob->flags |= FL_ATTACKMODE | FL_FIRSTATTACK;
}

/*
===============
=
= SightPlayer
=
= Called by actors that ARE NOT chasing the player.  If the player
= is detected (by sight, noise, or proximity), the actor is put into
= it's combat frame and true is returned.
=
= Incorporates a random reaction delay
=
===============
*/
bool SightPlayer(
	objtype* ob)
{
	if (ob->obclass == gen_scientistobj)
	{
		if (ob->flags & FL_INFORMANT)
		{
			return false;
		}
	}

	if (PlayerInvisable)
	{
		return false;
	}

	if (ob->flags & FL_ATTACKMODE)
	{
		return true;
	}

	if (ob->temp2)
	{
		//
		// count down reaction time
		//
		ob->temp2 -= tics;
		if (ob->temp2 > 0)
		{
			return false;
		}
		ob->temp2 = 0; // time to react
	}
	else
	{
		if (ob->areanumber < NUMAREAS && !areabyplayer[ob->areanumber])
		{
			return false;
		}

		if (ob->flags & FL_AMBUSH)
		{
			if (!CheckSight(ob, player))
			{
				return false;
			}
			ob->flags &= ~FL_AMBUSH;
		}
		else
		{
			bool sighted = false;

			if (madenoise || CheckSight(ob, player))
			{
				sighted = true;
			}

			switch (ob->obclass)
			{
				// Actors that look fine while JUST STANDING AROUND should go here.
				//
			case rentacopobj:
			case proguardobj:
			case swatobj:
			case goldsternobj:
			case gen_scientistobj:
			case floatingbombobj:
			case volatiletransportobj:
				break;

				// Actors that look funny when just standing around go here...
				//
			default:
				if (ob->flags & FL_VISIBLE)
				{
					sighted = true;
				}
				break;
			}

			if (!sighted)
			{
				return false;
			}
		}

		switch (ob->obclass)
		{
		case goldsternobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 4);
			break;


		case rentacopobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 4);
			break;

		case gen_scientistobj:
			ob->temp2 = 2;
			break;

		case swatobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 6);
			break;

		case proguardobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 6);
			break;

		case hang_terrotobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 4);
			break;

		case gurney_waitobj:
			ob->temp2 = ob->temp3;
			break;

		case liquidobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 6);
			break;

		case floatingbombobj:
			ob->temp2 = static_cast<std::int16_t>(1 + US_RndT() / 4);
			break;

		case genetic_guardobj:
		case mutant_human1obj:
		case mutant_human2obj:
		case scan_alienobj:
		case lcan_alienobj:
		case gurneyobj:
		case spider_mutantobj:
		case breather_beastobj:
		case cyborg_warriorobj:
		case reptilian_warriorobj:
		case acid_dragonobj:
		case mech_guardianobj:
		case final_boss1obj:
		case final_boss2obj:
		case final_boss3obj:
		case final_boss4obj:
			ob->temp2 = 1;
			break;

		default:
			break;
		}
		ob->flags &= ~FL_FRIENDLY;
		return false;
	}

	FirstSighting(ob);

	return true;
}


std::int16_t AdjAngleTable[2][8] = {
	{225, 270, 315, 360, 45, 90, 135, 180}, // Upper Bound
{180, 225, 270, 315, 0, 45, 90, 135}, // Lower Bound
};

// --------------------------------------------------------------------------
// CheckView()  Checks a straight line between player and current object
//      If the sight is ok, check angle to see if they notice
//      returns true if the player has been spoted
// --------------------------------------------------------------------------
bool CheckView(
	objtype* from_obj,
	objtype* to_obj)
{
	//
	// don't bother tracing a line if the area isn't connected to the player's
	//

	if (from_obj->areanumber < NUMAREAS && !areabyplayer[from_obj->areanumber])
	{
		return false;
	}

	const auto deltax = from_obj->x - to_obj->x;
	const auto deltay = to_obj->y - from_obj->y;

	auto fangle = std::atan2(deltay, deltax); // returns -pi to pi

	if (fangle < 0.0)
	{
		fangle += 2.0 * bstone::math::pi();
	}

	auto angle = static_cast<int>(fangle / (2.0 * bstone::math::pi()) * ANGLES + 23);

	if (angle > 360)
	{
		angle = 360;
	}

	if (angle <= AdjAngleTable[1][from_obj->dir] || angle >= AdjAngleTable[0][from_obj->dir])
	{
		return false;
	}

	//
	// trace a line to check for blocking tiles (corners)
	//

	return CheckLine(from_obj, to_obj);
}

#if LOOK_FOR_DEAD_GUYS
// --------------------------------------------------------------------------
// LookForDeadGuys()
// --------------------------------------------------------------------------
bool LookForDeadGuys(
	objtype* obj)
{
	std::uint8_t loop;
	bool DeadGuyFound = false;

	if ((obj->obclass == gen_scientistobj) && (obj->flags & FL_INFORMANT))
	{
		return false;
	}

	for (loop = 0; loop < NumDeadGuys; loop++)
	{
		if (CheckSight(obj, DeadGuys[loop]))
		{
			DeadGuyFound = true;
			FirstSighting(obj);
			break;
		}
	}

	return DeadGuyFound;
}
#endif

bool LookForGoodies(
	objtype* ob,
	std::uint16_t RunReason)
{
	statobj_t* statptr;
	bool just_find_door = false;

	// Don't look for goodies if this actor is simply a non-informant that
	// was interrogated. (These actors back away, then attack!)
	//
	if (RunReason == RR_INTERROGATED)
	{
		just_find_door = true;
		if (US_RndT() < 128)
		{
			ob->flags &= ~FL_INTERROGATED; // No longer runs, now attacks!
		}
	}

	// We'll let the computer-controlled actors cheat in some circumstances...
	//
	if ((player->areanumber != ob->areanumber) && (!(ob->flags & FL_VISIBLE)))
	{
		if (!ob->ammo)
		{
			ob->ammo += 8;
		}
		if (ob->hitpoints <= (get_start_hit_point(ob->obclass - rentacopobj) >> 1))
		{
			ob->hitpoints += 10;
		}
		return true;
	}

	// Let's REALLY look for some goodies!
	//
	if (!just_find_door)
	{
		for (statptr = &statobjlist[0]; statptr != laststatobj; statptr++)
		{
			if ((ob->areanumber == statptr->areanumber) && (statptr->shapenum != -1))
			{
				switch (statptr->itemnumber)
				{
				case bo_chicken:
				case bo_ham:
				case bo_water:
				case bo_clip:
				case bo_clip2:
				case bo_candybar:
				case bo_sandwich:

					// If actor is 'on top' of this object, get it!
					//
					if ((statptr->tilex == ob->tilex) && (statptr->tiley == ob->tiley))
					{
						std::int16_t shapenum = -1;

						switch (statptr->itemnumber)
						{
						case bo_clip:
						case bo_clip2:
							if (ob->ammo)
							{ // this actor has plenty
								continue; // of ammo!
							}
							ob->ammo += 8;
							break;

						case bo_candybar:
						case bo_sandwich:
						case bo_chicken:
							if (ob->hitpoints > (get_start_hit_point(ob->obclass - rentacopobj) >> 1))
							{
								continue; // actor has plenty of health!
							}
							ob->hitpoints += 8;
							shapenum = statptr->shapenum + 1;
							break;

						case bo_ham:
							if (ob->hitpoints > (get_start_hit_point(ob->obclass - rentacopobj) >> 1))
							{
								continue; // actor has plenty of health!
							}
							ob->hitpoints += 12;
							shapenum = statptr->shapenum + 1;
							break;

						case bo_water:
							if (ob->hitpoints > (get_start_hit_point(ob->obclass - rentacopobj) >> 1))
							{
								continue; // actor has plenty of health!
							}
							ob->hitpoints += 2;
							shapenum = statptr->shapenum + 1;
							break;
						}

						ob->s_tilex = 0; // reset for next search!
						statptr->shapenum = shapenum; // remove from list if necessary
						statptr->itemnumber = bo_nothing;
						statptr->flags &= ~FL_BONUS;

						if (statptr->shapenum == -1)
						{
							vid_hw_on_remove_static(*statptr);
						}

						return true;
					}

					// Give actor a chance to run towards this object.
					//
					if ((!(ob->flags & FL_RUNTOSTATIC)))
					{ // &&(US_RndT()<25))
						if (
							((RunReason & RR_AMMO) &&
							((statptr->itemnumber == bo_clip) ||
								(statptr->itemnumber == bo_clip2)))
							||
							((RunReason & RR_HEALTH) &&
							((statptr->itemnumber == bo_firstaid) ||
								(statptr->itemnumber == bo_water) ||
								(statptr->itemnumber == bo_chicken) ||
								(statptr->itemnumber == bo_candybar) ||
								(statptr->itemnumber == bo_sandwich) ||
								(statptr->itemnumber == bo_ham)))
							)
						{
							ob->flags |= FL_RUNTOSTATIC;
							ob->s_tilex = statptr->tilex;
							ob->s_tiley = statptr->tiley;
							return false;
						}
					}
				}
			}
		}
	}

	// Should actor run for a door? (quick escape!)
	//
	if (ob->areanumber < NUMAREAS && areabyplayer[ob->areanumber])
	{
		const int DOOR_CHOICES = 8;

		doorobj_t* door;
		doorobj_t* doorlist[DOOR_CHOICES]{};
		std::int8_t doorsfound = 0;

		// If actor is running for a 'goody' or a door -- leave it alone!
		//
		if (ob->flags & FL_RUNTOSTATIC)
		{
			return false;
		}

		// Search for all doors in actor's current area.
		//
		for (door = &doorobjlist[0]; door != lastdoorobj; door++)
		{
			// Is this an elevator door   OR   a locked door?
			//
			if ((!(mapsegs[0][farmapylookup[door->tiley] + (door->tilex - 1)] - AREATILE)) ||
				(!(mapsegs[0][farmapylookup[door->tiley] + (door->tilex + 1)] - AREATILE)) ||
				(door->lock != kt_none))
			{
				continue;
			}

			// Does this door connect the area the actor is in with another area?
			//
			if ((door->areanumber[0] == ob->areanumber) ||
				(door->areanumber[1] == ob->areanumber))
			{
				doorlist[static_cast<int>(doorsfound)] = door; // add to list
				if (++doorsfound == DOOR_CHOICES)
				{ // check for max
					break;
				}
			}
		}

		// Randomly choose a door if any were found.
		//
		if (doorsfound)
		{
			// Randomly choose a door from the list.
			// (Only choose the last door used if it's the only door in this area!)
			//
			int door_index = Random(doorsfound);
			door = doorlist[door_index];

			if (door == ui16_to_door_object(ob->temp3) && doorsfound > 1)
			{
				door_index++;
				if (door_index >= doorsfound)
				{
					door_index = 0;
				}
				door = doorlist[door_index];
			}

			ob->temp3 = door_object_to_ui16(door);

			ob->s_tilex = door->tilex;
			ob->s_tiley = door->tiley;

			ob->flags |= FL_RUNTOSTATIC;
		}
	}
	else
	{
		// Either: actor is running to corner (leave it alone)     OR
		//         actor is chasing an object already removed by another actor
		//         (make this actor run to corner)
		//
		if (ob->flags & FL_RUNTOSTATIC)
		{
			ob->s_tilex = 0;
		}
	}

	return false;
}

std::uint16_t CheckRunChase(
	objtype* ob)
{
	constexpr auto RUNAWAY_SPEED = bstone::math::fixed_to_floating(1000);

	std::uint16_t RunReason = 0;

	// Mark the reason for running.
	//
	if (!ob->ammo)
	{ // Out of ammo!
		RunReason |= RR_AMMO;
	}

	if (ob->hitpoints <= (get_start_hit_point(ob->obclass - rentacopobj) >> 1))
	{
		RunReason |= RR_HEALTH; // Feeling sickly!

	}
	if ((ob->flags & (FL_FRIENDLY | FL_INTERROGATED)) == FL_INTERROGATED)
	{
		RunReason |= RR_INTERROGATED; // Non-informant was interrogated!

	}

	// Time to RUN or CHASE?
	//
	if (RunReason)
	{ // run, Run, RUN!
		if (!(ob->flags & FL_RUNAWAY))
		{
			ob->temp3 = 0;
			ob->flags |= FL_RUNAWAY;
			ob->speed += RUNAWAY_SPEED;
		}
	}
	else
	{ // chase, Chase, CHASE!
		if (ob->flags & FL_RUNAWAY)
		{
			ob->flags &= ~FL_RUNAWAY;
			ob->speed -= RUNAWAY_SPEED;
		}
	}

	return RunReason;
}

void SeekPlayerOrStatic(
	objtype* ob,
	std::int16_t* deltax,
	std::int16_t* deltay)
{
	std::uint16_t whyrun = 0;
	bool smart = false;

	// Is this a "smart" actor?
	//
	switch (ob->obclass)
	{
	case SMART_ACTORS:
		smart = true;
		break;

	case electrosphereobj:
		if (!ob->s_tilex)
		{
			GetCornerSeek(ob);
		}
		*deltax = ob->s_tilex - ob->tilex;
		*deltay = ob->s_tiley - ob->tiley;
		return;
		break;

	default:
		break;
	}

	// Should actor run away (chase static) or chase player?
	//
	if ((smart) && ((whyrun = CheckRunChase(ob))) != 0)
	{
		// Initilize seek tile?
		//
		if (!ob->s_tilex)
		{
			GetCornerSeek(ob);
		}

		// Are there any goodies available?
		//
		if (!LookForGoodies(ob, whyrun))
		{
			// Change seek tile when actor reaches it.
			//
			if ((ob->tilex == ob->s_tilex) && (ob->tiley == ob->s_tiley))
			{ // don't forget me!
				GetCornerSeek(ob);
				ob->flags &= ~FL_INTERROGATED;
			} // add me, too

			// Calculate horizontal / vertical distance to seek point.
			//
			*deltax = ob->s_tilex - ob->tilex;
			*deltay = ob->s_tiley - ob->tiley;
		}
		else
		{
			whyrun = CheckRunChase(ob);
		}
	}

	// Make actor chase player if it's not running.
	//
	if (!whyrun)
	{
		*deltax = player->tilex - ob->tilex;
		*deltay = player->tiley - ob->tiley;
	}
}

bool PlayerIsBlocking(
	objtype* ob)
{
	std::int8_t opp_off[9][2] = {
		{-1, 0}, {-1, 1}, {0, 1}, {1, 1},
	{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {0, 0}
	};

	ob->tilex += opp_off[ob->dir][0];
	ob->tiley += opp_off[ob->dir][1];
	ob->dir = opposite[ob->dir];
	ob->distance = 1.0 - ob->distance;
	return true;
}

void MakeAlertNoise(
	objtype* obj)
{
	madenoise = true;
	alerted = 2;
	alerted_areanum = obj->areanumber;
}
