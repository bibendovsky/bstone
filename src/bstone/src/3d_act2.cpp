/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cmath>
#include <cstring>

#include <algorithm>
#include <array>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vl.h"

#include "bstone_exception.h"
#include "bstone_math.h"


// 3d_def.h stuff
//
namespace
{


const std::int16_t DOOR_RUBBLE_STATNUM = 112; // Door Rubble sprite


struct ofs_anim_t
{
	static const int animtype_offset = 0;
	static const int animtype_size = 2;

	static const int curframe_offset = animtype_offset + animtype_size;
	static const int curframe_size = 5;

	static const int maxframe_offset = curframe_offset + curframe_size;
	static const int maxframe_size = 5;

	static const int animdir_offset = maxframe_offset + maxframe_size;
	static const int animdir_size = 1;


	template<int TOffset, int TSize>
	static std::uint16_t get(
		const objtype* o)
	{
		return (o->temp3 >> TOffset) & ((1 << TSize) - 1);
	}

	template<int TOffset, int TSize>
	static void set(
		const std::uint16_t value,
		objtype* o)
	{
		o->temp3 &= ~(((1 << TSize) - 1) << TOffset);
		o->temp3 |= ((value & ((1 << TSize) - 1)) << TOffset);
	}

	static std::uint16_t get_animtype(
		const objtype* o)
	{
		return get<animtype_offset, animtype_size>(o);
	}

	static void set_animtype(
		const std::uint16_t value,
		objtype* o)
	{
		set<animtype_offset, animtype_size>(value, o);
	}


	static std::uint16_t get_curframe(
		const objtype* o)
	{
		return get<curframe_offset, curframe_size>(o);
	}

	static void set_curframe(
		const std::uint16_t value,
		objtype* o)
	{
		set<curframe_offset, curframe_size>(value, o);
	}


	static std::uint16_t get_maxframe(
		const objtype* o)
	{
		return get<maxframe_offset, maxframe_size>(o);
	}

	static void set_maxframe(
		const std::uint16_t value,
		objtype* o)
	{
		set<maxframe_offset, maxframe_size>(value, o);
	}


	static std::uint16_t get_animdir(
		const objtype* o)
	{
		return get<animdir_offset, animdir_size>(o);
	}

	static void set_animdir(
		const std::uint16_t value,
		objtype* o)
	{
		set<animdir_offset, animdir_size>(value, o);
	}
}; // ofs_anim_t


std::uint16_t MAPSPOT(
	const std::uint8_t x,
	const std::uint8_t y,
	const std::uint8_t plane)
{
	return mapsegs[plane][farmapylookup[y] + x];
}

objtype* SLIDE_TEMP(
	const objtype* const obj)
{
	return ui16_to_actor(obj->hitpoints);
}

void SpawnExplosion(
	const double x,
	const double y)
{
	SpawnCusExplosion(x, y, SPR_EXPLOSION_1, 4, 5, explosionobj);
}

void SpawnFlash(
	const double x,
	const double y)
{
	SpawnCusExplosion(x, y, SPR_EXPLOSION_1, 4, 5, deadobj);
}


} // namespace


void FirstSighting(
	objtype* ob);

bool SightPlayer(
	objtype* ob);

void TakeDamage(
	std::int16_t points,
	objtype* attacker);

void OpenDoor(
	std::int16_t door);

bool CheckView(
	objtype* from_obj,
	objtype* to_obj);

std::int16_t CalcAngle(
	objtype* from_obj,
	objtype* to_obj);

bool ClipMove(
	objtype* ob,
	const double xmove,
	const double ymove);

void DisplaySwitchOperateMsg(
	int coords);

void T_Hit(
	objtype* ob);

void CheckForSpecialTile(
	objtype* obj,
	std::uint16_t tilex,
	std::uint16_t tiley);

void ExplodeRadius(
	objtype* obj,
	std::int16_t damage,
	bool damageplayer);

void ToggleBarrier(
	objtype* obj);

bool AnimateOfsObj(
	objtype* obj);

void T_Seek(
	objtype* ob);

void T_ExplodeDamage(
	objtype* obj);

void AdvanceAnimFWD(
	objtype* obj);

void T_ChangeShape(
	objtype* obj);

void T_LiquidStand(
	objtype* obj);

void T_ChangeShape(
	objtype* obj);

void T_Security(
	objtype* obj);


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/


#define BFG_SHOT_STOPS (1)

constexpr auto SPDPATROL = bstone::math::fixed_to_floating(512);
constexpr auto SPDPROJ = bstone::math::fixed_to_floating(7168);

constexpr auto PROJCHECKSIZE = bstone::math::fixed_to_floating(0x8000); // scan for actor range
constexpr auto PROJWALLSIZE = bstone::math::fixed_to_floating(0x2000); // collision with wall range
constexpr auto PROJECTILESIZE = bstone::math::fixed_to_floating(0xC000);

#define SEEK_TURN_DELAY (30) // tics

#define CLOSE_RANGE (1) // Tiles

constexpr auto ALIENSPEED = SPDPATROL;
#define ALIENAMMOINIT (30 + (US_RndT() % 60))

#define GR_DAMAGE (40 + (US_RndT() & 0x7F)) // 20 & ... 0x3F
#define BFG_DAMAGE (GR_DAMAGE << 1) // Hit Point damage cause by BFG
#define PLASMA_DETONATOR_DAMAGE (500)
#define DETONATOR_FLASH_RATE (20)

#define EXPLODE_DAMAGE (20) // 5

#define VPOST_BARRIER_SPEED (7) // Tics per anim step
#define VPOST_WAIT_DELAY (90) // Tics delay in cycling

// Min number of statics avail
// before 50/50 chance of door rubble on exp. doors.
#define DR_MIN_STATICS (50)


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

std::int8_t detonators_spawned = 0;


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

const StartHitPoints aog_start_hit_points =
{
	//
	// BABY MODE
	//
	SubStartHitPoints
	{
		6, // Rent-A-Cop
		6, // Hang Terrot
		6, // general scientist
		28, // pod alien
		18, // Electro-Alien
		1, // Electro-Sphere
		43, // ProGuard
		20, // Genetic Guard
		27, // Mutant Human Type 1
		27, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		37, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		30, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		33, // Gurney Mutant - Spawned HP
		47, // Liquid Alien -
		88, // Swat Guards
		50, // Goldstern
		0, // Morphed Goldstern
		17, // Volatile Material Transport
		20, // Floating Bomb
		150, // Vital Defence Obj  - DON'T CHANGE
		800, // spider_mutant
		850, // breather_beast
		900, // cyborg_warror
		950, // reptilian_warrior
		1000, // acid_dragon
		1050, // mech_guardian
		0, // final_boss 1
		0, // final_boss 2
		0, // final_boss 3
		0, // final_boss 4
		0, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		0, // green ooze
		0, // black ooze
		0, // green ooze 2
		0, // black ooze 2
		1, // Pod egg
		0, // morphing_spider_mutant
		0, // morphing_reptilian_warrior
		0, // morphing_Mutant Human Type 2
	},

	//
	// DON'T HURT ME MODE
	//

	SubStartHitPoints
	{
		9, // Rent-A-Cop
		9, // Hang Terrot
		9, // general scientist
		60, // pod alien
		37, // Electro-Alien
		1, // Electro-Sphere
		63, // ProGuard
		60, // Genetic Guard
		50, // Mutant Human Type 1
		50, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		75, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		60, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		37, // Gurney Mutant - Spawned HP
		66, // Liquid Alien -
		112, // SWAT Guards
		63, // Goldstern
		0, // Morphed Goldstern
		25, // Volatile Material Transport
		40, // Floating Bomb
		175, // Vital Defence Obj  - DON'T CHANGE
		1000, // spider_mutant
		1250, // breather_beast
		1500, // cyborg_warror
		1750, // reptilian_warrior
		2000, // acid_dragon
		2500, // mech_guardian
		0, // final_boss 1
		0, // final_boss 2
		0, // final_boss 3
		0, // final_boss 4
		0, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		0, // green ooze
		0, // black ooze
		0, // green ooze 2
		0, // black ooze 2
		1, // Pod egg
		0, // morphing_spider_mutant
		0, // morphing_reptilian_warrior
		0, // morphing_Mutant Human Type 2
	},

	//
	// BRING 'EM ON MODE
	//

	SubStartHitPoints
	{
		25, // Rent-A-Cop
		23, // Hang Terrot
		23, // general scientist
		160, // pod alien
		112, // Electro-Alien
		1, // Electro-Sphere
		150, // ProGuard
		180, // Genetic Guard
		155, // Mutant Human Type 1
		155, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		225, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		180, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		150, // Gurney Mutant - Spawned HP
		163, // Liquid Alien -
		325, // SWAT Guards
		125, // Goldstern
		0, // Morphed Goldstern
		63, // Volatile Material Transport
		60, // Floating Bomb
		375, // Vital Defence Obj  - DON'T CHANGE
		2000, // spider_mutant
		2500, // breather_beast
		3000, // cyborg_warror
		3500, // reptilian_warrior
		4000, // acid_dragon
		5000, // mech_guardian
		0, // final_boss 1
		0, // final_boss 2
		0, // final_boss 3
		0, // final_boss 4
		0, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		0, // green ooze
		0, // black ooze
		0, // green ooze 2
		0, // black ooze 2
		1, // Pod egg
		0, // morphing_spider_mutant
		0, // morphing_reptilian_warrior
		0, // morphing_Mutant Human Type 2
	},

	//
	// DEATH INCARNATE MODE
	//

	SubStartHitPoints
	{
		38, // Rent-A-Cop
		28, // Hang Terrot
		28, // general scientist
		210, // pod alien
		150, // Electro-Alien
		1, // Electro-Sphere
		175, // ProGuard
		240, // Genetic Guard
		210, // Mutant Human Type 1
		210, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		300, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		240, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		200, // Gurney Mutant - Spawned HP
		210, // Liquid Alien -
		425, // SWAT Gaurds
		188, // Goldstern
		0, // Morphed Goldstern
		75, // Volatile Material Transport
		85, // Floating Bomb
		450, // Vital Defence Obj - DON'T CHANGE
		3000, // spider_mutant
		3500, // breather_beast
		4000, // cyborg_warror
		4500, // reptilian_warrior
		5000, // acid_dragon
		6000, // mech_guardian
		0, // final_boss 1
		0, // final_boss 2
		0, // final_boss 3
		0, // final_boss 4
		0, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		0, // green ooze
		0, // black ooze
		0, // green ooze 2
		0, // black ooze 2
		1, // Pod egg
		0, // morphing_spider_mutant
		0, // morphing_reptilian_warrior
		0, // morphing_Mutant Human Type 2
	},
}; // aog_start_hit_points

const StartHitPoints ps_start_hit_points =
{
	//
	// BABY MODE
	//
	SubStartHitPoints
	{
		4, // Rent-A-Cop
		4, // Hang Terrot
		4, // general scientist
		22, // pod alien
		13, // Electro-Alien
		1, // Electro-Sphere
		33, // ProGuard
		20, // Genetic Guard
		24, // Mutant Human Type 1
		24, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		34, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		25, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		27, // Gurney Mutant - Spawned HP
		30, // Liquid Alien -
		78, // Swat Guards
		90, // Goldstern
		4000, // Morphed Goldstern
		17, // Volatile Material Transport
		15, // Floating Bomb
		175, // Vital Defence Obj  - DON'T CHANGE
		60, // spider_mutant
		65, // breather_beast
		70, // cyborg_warror
		65, // reptilian_warrior
		60, // acid_dragon
		65, // mech_guardian
		1600, // final_boss 1
		1700, // final_boss 2
		1800, // final_boss 3
		1900, // final_boss 4
		1, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		1, // green ooze
		1, // black ooze
		1, // green ooze 2
		1, // black ooze 2
		1, // Pod egg
		1, // morphing_spider_mutant
		1, // morphing_reptilian_warrior
		1, // morphing_Mutant Human Type 2
	},

	//
	// DON'T HURT ME MODE
	//

	SubStartHitPoints
	{
		9, // Rent-A-Cop
		9, // Hang Terrot
		9, // general scientist
		60, // pod alien
		37, // Electro-Alien
		1, // Electro-Sphere
		63, // ProGuard
		60, // Genetic Guard
		50, // Mutant Human Type 1
		50, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		75, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		60, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		37, // Gurney Mutant - Spawned HP
		66, // Liquid Alien -
		112, // SWAT Guards
		100, // Goldstern
		4500, // Morphed Goldstern
		25, // Volatile Material Transport
		40, // Floating Bomb
		175, // Vital Defence Obj  - DON'T CHANGE
		100, // spider_mutant
		115, // breather_beast
		100, // cyborg_warror
		115, // reptilian_warrior
		100, // acid_dragon
		115, // mech_guardian
		1700, // final_boss 1
		1800, // final_boss 2
		1900, // final_boss 3
		2000, // final_boss 4
		1, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		1, // green ooze
		1, // black ooze
		1, // green ooze 2
		1, // black ooze 2
		1, // Pod egg
		1, // morphing_spider_mutant
		1, // morphing_reptilian_warrior
		1, // morphing_Mutant Human Type 2
	},

	//
	// BRING 'EM ON MODE
	//

	SubStartHitPoints
	{
		25, // Rent-A-Cop
		23, // Hang Terrot
		23, // general scientist
		160, // pod alien
		112, // Electro-Alien
		1, // Electro-Sphere
		150, // ProGuard
		180, // Genetic Guard
		155, // Mutant Human Type 1
		155, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		225, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		180, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		150, // Gurney Mutant - Spawned HP
		163, // Liquid Alien -
		325, // SWAT Guards
		150, // Goldstern
		4800, // Morphed Goldstern
		63, // Volatile Material Transport
		60, // Floating Bomb
		175, // Vital Defence Obj  - DON'T CHANGE
		150, // spider_mutant
		165, // breather_beast
		150, // cyborg_warror
		165, // reptilian_warrior
		150, // acid_dragon
		165, // mech_guardian
		1800, // final_boss 1
		1900, // final_boss 2
		2000, // final_boss 3
		2100, // final_boss 4
		1, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		1, // green ooze
		1, // black ooze
		1, // green ooze 2
		1, // black ooze 2
		1, // Pod egg
		1, // morphing_spider_mutant
		1, // morphing_reptilian_warrior
		1, // morphing_Mutant Human Type 2
	},

	//
	// DEATH INCARNATE MODE
	//

	SubStartHitPoints
	{
		38, // Rent-A-Cop
		28, // Hang Terrot
		28, // general scientist
		210, // pod alien
		150, // Electro-Alien
		1, // Electro-Sphere
		175, // ProGuard
		240, // Genetic Guard
		210, // Mutant Human Type 1
		210, // Mutant Human Type 2
		1, // Large Canister Alien - CONTAINER HP
		300, // Large Can-Alien - ALIEN HP
		1, // Small Canister Alien - CONTAINER HP
		240, // Small Can-Alien - ALIEN HP
		1, // Gurney Mutant - Waiting HP
		200, // Gurney Mutant - Spawned HP
		210, // Liquid Alien -
		425, // SWAT Gaurds
		250, // Goldstern
		5400, // Morphed Goldstern
		75, // Volatile Material Transport
		85, // Floating Bomb
		175, // Vital Defence Obj - DON'T CHANGE
		200, // spider_mutant
		225, // breather_beast
		200, // cyborg_warror
		225, // reptilian_warrior
		200, // acid_dragon
		225, // mech_guardian
		1900, // final_boss 1
		2000, // final_boss 2
		2100, // final_boss 3
		2200, // final_boss 4
		1, // Blake
		1, // Crate 1
		1, // Crate 2
		1, // Crate 3
		1, // green ooze
		1, // black ooze
		1, // green ooze 2
		1, // black ooze 2
		1, // Pod egg
		1, // morphing_spider_mutant
		1, // morphing_reptilian_warrior
		1, // morphing_Mutant Human Type 2
	},
}; // ps_start_hit_points


const StartHitPoints* starthitpoints = nullptr;


using BossIds = std::vector<std::int16_t>;

BossIds BossShotShapes;
BossIds BossShapes;
BossIds MorphShapes;
BossIds MorphEndShapes;
BossIds MorphSounds;

std::uint16_t MorphClass[] = {
	spider_mutantobj,
	reptilian_warriorobj,
	mutant_human2obj,
};


void initialize_hit_point_table()
{
	const auto& assets_info = get_assets_info();

	starthitpoints = (assets_info.is_aog() ? &aog_start_hit_points : &ps_start_hit_points);
}

std::uint16_t get_start_hit_point(
	const int index)
{
	if (index >= NUMHITENEMIES)
	{
		return 0;
	}

	return (*starthitpoints)[gamestate.difficulty][index];
}

void initialize_boss_constants()
{
	const auto& assets_info = get_assets_info();

	BossShotShapes = {
		SPR_BOSS1_PROJ1,
		0,
		0,
		0,
		SPR_BOSS5_PROJ1,
		0,
		0,
		assets_info.is_ps() ? SPR_BOSS10_SPIT1 : static_cast<std::int16_t>(0),
	};

	BossShapes = {
		SPR_BOSS1_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS2_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS3_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS4_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS5_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS6_W1,
		assets_info.is_ps() ? SPR_BOSS7_W1 : static_cast<std::int16_t>(0),
		assets_info.is_ps() ? SPR_BOSS8_W1 : static_cast<std::int16_t>(0),
		assets_info.is_ps() ? SPR_BOSS9_W1 : static_cast<std::int16_t>(0),
		assets_info.is_ps() ? SPR_BOSS10_W1 : static_cast<std::int16_t>(0),
	};

	MorphShapes = {
		SPR_BOSS1_MORPH1,
		SPR_BOSS4_MORPH1,
		SPR_MUTHUM2_MORPH1,
	};

	MorphEndShapes = {
		SPR_BOSS1_W1,
		assets_info.is_aog_sw() ? static_cast<std::int16_t>(0) : SPR_BOSS4_W1,
		SPR_MUTHUM2_W1,
	};

	MorphSounds = {
		SCANHALTSND,
		GGUARDHALTSND,
		DOGBOYHALTSND,
	};
}


std::uint16_t bars_connected = 0;

std::uint16_t SpecialSpawnFlags[] = {
	FL2_DROP_RKEY,
	FL2_DROP_YKEY,
	FL2_DROP_BKEY,
	FL2_DROP_BFG,
	FL2_DROP_ION,
	FL2_DROP_DETONATOR,
};

void T_Path(
	objtype* ob);

void T_Shoot(
	objtype* ob);

void T_Shade(
	objtype* obj);

void T_Chase(
	objtype* ob);

void T_Projectile(
	objtype* ob);

void T_Stand(
	objtype* ob);


void T_OfsThink(
	objtype* obj);

void A_DeathScream(
	objtype* ob);

std::int16_t RandomSphereDir(
	enemy_t dir);

void CheckForcedMove(
	objtype* ob);

void T_SwatWound(
	objtype* ob);

void T_BlowBack(
	objtype* obj);

void DoAttack(
	objtype* ob);

bool MoveTrappedDiag(
	objtype* ob);

bool CheckTrappedDiag(
	objtype* ob);

void ChangeShootMode(
	objtype* ob);


static int get_remaining_generators()
{
	int remaining_generators = 0;

	for (objtype* o = player->next; o; o = o->next)
	{
		if (o->obclass != rotating_cubeobj)
		{
			continue;
		}

		if ((o->flags & FL_SHOOTABLE) == 0)
		{
			continue;
		}

		remaining_generators += 1;
	}

	return remaining_generators;
}

static void display_remaining_generators()
{
	static const std::string message_part_1 =
		"^FC57 PROJECTION GENERATOR\r"
		"      DESTROYED!\r"
		"\r"
		"^FCA6   - "
		;

	static const std::string message_part_2 =
		" REMAINING -\r"
		" DESTROY THEM TO WIN!"
		;


	const auto remaining_generators = get_remaining_generators();

	static std::string message;

	message =
		message_part_1 +
		std::to_string(remaining_generators) +
		message_part_2;

	DISPLAY_TIMED_MSG(message.c_str(), MP_FLOOR_UNLOCKED, MT_GENERAL);
}


// ===========================================================================
//
// OFFSET OBJECT ROUTINES
//
// * NOTES: OffsetObjects require the use of TEMP1 of the object structure!
//
// * NOTES: THINK function AnimateOfsObj() requires the use of TEMP3 of the
//                              object structure!
//
// ===========================================================================

//
// Local Externs
//
extern statetype s_ofs_stand;

extern statetype s_ofs_chase1;
extern statetype s_ofs_chase1s;
extern statetype s_ofs_chase2;
extern statetype s_ofs_chase3;
extern statetype s_ofs_chase3s;
extern statetype s_ofs_chase4;

extern statetype s_ofs_pain;

extern statetype s_ofs_die1;
extern statetype s_ofs_die1s;
extern statetype s_ofs_die2;
extern statetype s_ofs_die3;
extern statetype s_ofs_die4;
extern statetype s_ofs_die5;

extern statetype s_ofs_attack1;
extern statetype s_ofs_attack2;
extern statetype s_ofs_attack3;


extern statetype s_ofs_spit1;
extern statetype s_ofs_spit2;
extern statetype s_ofs_spit3;

extern statetype s_ofs_shoot1;
extern statetype s_ofs_shoot2;
extern statetype s_ofs_shoot3;
extern statetype s_ofs_shoot4;
extern statetype s_ofs_shoot5;
extern statetype s_ofs_shoot6;
extern statetype s_ofs_shoot7;

extern statetype s_ofs_smart_anim;
extern statetype s_ofs_smart_anim2;

extern statetype s_ofs_projectile1;
extern statetype s_ofs_projectile2;

extern statetype s_ofs_proj_exp1;
extern statetype s_ofs_proj_exp2;

extern statetype s_ofs_pod_spit1;
extern statetype s_ofs_pod_spit2;
extern statetype s_ofs_pod_spit3;

extern statetype s_ofs_pod_attack1;
extern statetype s_ofs_pod_attack1a;
extern statetype s_ofs_pod_attack2;

extern statetype s_ofs_pod_death1;
extern statetype s_ofs_pod_death2;
extern statetype s_ofs_pod_death3;

extern statetype s_ofs_roam;

extern statetype s_ofs_esphere_death1;
extern statetype s_ofs_esphere_death2;
extern statetype s_ofs_esphere_death3;



//
// Local Prototypes
//

void T_SmartThink(objtype* ob);

void T_SmartThought(
	objtype* obj);

bool ProjectileTryMove(
	objtype* ob,
	const double deltax,
	const double deltay);

void T_Projectile(
	objtype* ob);

void SphereStartDir(
	objtype* ob);

void T_OfsBounce(
	objtype* obj);

//
// Local Offset Anim Structures
//
statetype s_ofs_stand = {0, SPR_DEMO, 0, T_OfsThink, nullptr, &s_ofs_stand};

statetype s_ofs_chase1 = {0, SPR_DEMO, 10, T_Chase, T_SmartThink, &s_ofs_chase1s};
statetype s_ofs_chase1s = {0, SPR_DEMO, 5, nullptr, nullptr, &s_ofs_chase2};
statetype s_ofs_chase2 = {0, SPR_DEMO, 8, T_Chase, nullptr, &s_ofs_chase3};
statetype s_ofs_chase3 = {0, SPR_DEMO, 10, T_Chase, T_SmartThink, &s_ofs_chase3s};
statetype s_ofs_chase3s = {0, SPR_DEMO, 5, nullptr, nullptr, &s_ofs_chase4};
statetype s_ofs_chase4 = {0, SPR_DEMO, 8, T_Chase, nullptr, &s_ofs_chase1};

statetype s_ofs_pain = {0, SPR_DEMO, 15, nullptr, nullptr, &s_ofs_chase1};

statetype s_ofs_die1 = {0, SPR_DEMO, 18, T_BlowBack, A_DeathScream, &s_ofs_die1s};
statetype s_ofs_die1s = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_ofs_die2};
statetype s_ofs_die2 = {0, SPR_DEMO, 22, T_BlowBack, nullptr, &s_ofs_die3};
statetype s_ofs_die3 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_ofs_die4};
statetype s_ofs_die4 = {0, SPR_DEMO, 18, T_BlowBack, nullptr, &s_ofs_die5};
statetype s_ofs_die5 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_ofs_die5};

statetype s_ofs_attack1 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_ofs_attack2};
statetype s_ofs_attack2 = {0, SPR_DEMO, 20, nullptr, T_Hit, &s_ofs_attack3};
statetype s_ofs_attack3 = {0, SPR_DEMO, 30, nullptr, nullptr, &s_ofs_chase1};

statetype s_ofs_spit1 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_ofs_spit2};
statetype s_ofs_spit2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_ofs_spit3};
statetype s_ofs_spit3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_ofs_chase1};

statetype s_ofs_shoot1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_ofs_shoot2};
statetype s_ofs_shoot2 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_ofs_shoot3};
statetype s_ofs_shoot3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_ofs_chase1};

// General 'offset-object' states...
//

statetype s_ofs_pod_attack1 = {0, SPR_DEMO, 8, nullptr, nullptr, &s_ofs_pod_attack1a};
statetype s_ofs_pod_attack1a = {0, SPR_DEMO, 8, nullptr, nullptr, &s_ofs_attack2};
statetype s_ofs_pod_attack2 = {0, SPR_DEMO, 8, nullptr, T_Hit, &s_ofs_chase1};

statetype s_ofs_pod_spit1 = {0, SPR_DEMO, 8, nullptr, nullptr, &s_ofs_pod_spit2};
statetype s_ofs_pod_spit2 = {0, SPR_DEMO, 8, nullptr, nullptr, &s_ofs_pod_spit3};
statetype s_ofs_pod_spit3 = {0, SPR_DEMO, 8, nullptr, T_Shoot, &s_ofs_chase1};

statetype s_ofs_pod_death1 = {0, 0, 60, nullptr, nullptr, &s_ofs_pod_death2};
statetype s_ofs_pod_death2 = {0, 1, 30, nullptr, nullptr, &s_ofs_pod_death3};
statetype s_ofs_pod_death3 = {0, 2, 0, nullptr, nullptr, &s_ofs_pod_death3};

statetype s_ofs_pod_ouch = {0, SPR_DEMO, 11, nullptr, nullptr, &s_ofs_chase1};

statetype s_ofs_bounce = {0, 0, 10, T_OfsBounce, T_OfsThink, &s_ofs_bounce};

statetype s_ofs_ouch = {0, 0, 15, T_OfsBounce, nullptr, &s_ofs_bounce};

statetype s_ofs_esphere_death1 = {0, 0, 15, nullptr, A_DeathScream, &s_ofs_esphere_death2};
statetype s_ofs_esphere_death2 = {0, 1, 15, nullptr, nullptr, &s_ofs_esphere_death3};
statetype s_ofs_esphere_death3 = {0, 2, 15, nullptr, nullptr, nullptr};

statetype s_ofs_random = {0, 0, 1, T_OfsThink, nullptr, &s_ofs_random};

statetype s_ofs_static = {0, 0, 1, nullptr, nullptr, &s_ofs_static};

statetype s_hold = {0, 0, 1, nullptr, nullptr, &s_hold};

std::uint16_t scan_value;


void SpawnOffsetObj(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley)
{
	const auto& assets_info = get_assets_info();

	enemy_t dir_which = en_rentacop;

	switch (which)
	{
	case en_vertsphere:
	case en_horzsphere:
	case en_diagsphere:
		dir_which = which;
		which = en_electrosphere;
		break;

	case en_bloodvent:
	case en_watervent:
		dir_which = which;
		which = en_ventdrip;

	default:
		break;
	}

	SpawnNewObj(tilex, tiley, &s_ofs_stand);
	new_actor->flags |= FL_SHOOTABLE | FL_SOLID | FL_OFFSET_STATES;
	new_actor->obclass = static_cast<classtype>(rentacopobj + which);

	switch (which)
	{
	case en_final_boss2:
		new_actor->lighting = NO_SHADING;
	case en_final_boss1:
	case en_final_boss3:
	case en_final_boss4:
	case en_spider_mutant:
	case en_breather_beast:
	case en_cyborg_warrior:
	case en_reptilian_warrior:
	case en_acid_dragon:
	case en_mech_guardian:
		new_actor->temp1 = static_cast<std::int16_t>(BossShapes[which - en_spider_mutant]);
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFG_SHOOTABLE | FL2_BFGSHOT_SOLID : 0);
		break;

	case en_green_ooze:
		InitSmartSpeedAnim(
			new_actor,
			SPR_GREEN_OOZE1,
			US_RndT() % 3,
			2,
			at_CYCLE,
			ad_FWD,
			(assets_info.is_ps() ? 5 + (US_RndT() & 2) : 30));

		new_actor->flags &= ~(FL_SHOOTABLE | FL_SOLID);
		break;

	case en_black_ooze:
		InitSmartSpeedAnim(
			new_actor,
			SPR_BLACK_OOZE1,
			US_RndT() % 3,
			2,
			at_CYCLE,
			ad_FWD,
			(assets_info.is_ps() ? 5 + (US_RndT() & 2) : 30));

		new_actor->flags &= ~(FL_SHOOTABLE | FL_SOLID);
		break;

	case en_green2_ooze:
		InitSmartSpeedAnim(new_actor, SPR_GREEN2_OOZE1, US_RndT() % 3, 2, at_CYCLE, ad_FWD, 5 + (US_RndT() & 2));
		new_actor->flags &= ~(FL_SHOOTABLE | FL_SOLID);
		break;

	case en_black2_ooze:
		InitSmartSpeedAnim(new_actor, SPR_BLACK2_OOZE1, US_RndT() % 3, 2, at_CYCLE, ad_FWD, 5 + (US_RndT() & 2));
		new_actor->flags &= ~(FL_SHOOTABLE | FL_SOLID);
		break;

	case en_crate1:
	case en_crate2:
	case en_crate3:
		new_actor->temp1 = static_cast<std::int16_t>(SPR_CRATE_1 + which - en_crate1);
		new_actor->flags |= FL_NO_SLIDE | FL_FAKE_STATIC;

		// NOTE : TEMP2 is modified in ScanInfoPlane to determine if
		//                       this object is a "blastable" or not.
		break;

	case en_rotating_cube:
		if (!assets_info.is_ps())
		{
			InitSmartSpeedAnim(new_actor, SPR_VITAL_STAND, 0, 0, at_NONE, ad_FWD, 0);
		}
		else
		{
			InitSmartSpeedAnim(new_actor, SPR_CUBE1, 0, 9, at_CYCLE, ad_FWD, 5);
			new_actor->flags2 = FL2_BFGSHOT_SOLID;
		}
		new_actor->lighting = LAMP_ON_SHADING;
		break;

	case en_ventdrip:
		if (dir_which == en_bloodvent)
		{
			new_actor->temp1 = SPR_BLOOD_DRIP1;
		}
		else
		{
			new_actor->temp1 = SPR_WATER_DRIP1;
		}
		new_actor->temp2 = 5 + (US_RndT() % 10);
		new_actor->temp3 = 0;
		NewState(new_actor, &s_ofs_random);
		new_actor->flags &= ~(FL_SHOOTABLE | FL_SOLID);
		break;

	case en_plasma_detonator:
	case en_plasma_detonator_reserve:
		NewState(new_actor, &s_ofs_random);
		new_actor->temp1 = SPR_DOORBOMB;
		new_actor->temp3 = PLASMA_DETONATORS_DELAY;
		new_actor->flags &= ~(FL_SOLID | FL_SHOOTABLE);
		if (detonators_spawned++)
		{
			BSTONE_THROW_STATIC_SOURCE(
				"Too many Fission/Plasma Detonators are placed in this map! You can only have one!");
		}
		break;

	case en_flickerlight:
		new_actor->temp1 = SPR_DECO_ARC_1;
		new_actor->temp2 = (2 + (US_RndT() % 3)) * 60;
		NewState(new_actor, &s_ofs_random);
		new_actor->flags |= FL_NONMARK | FL_NEVERMARK;
		new_actor->flags &= ~(FL_SOLID | FL_SHOOTABLE);
		new_actor->lighting = LAMP_ON_SHADING;
		break;

	case en_security_light:
		new_actor->flags &= ~(FL_SOLID | FL_SHOOTABLE); // ick - this is stupid...
		NewState(new_actor, &s_security_light);
		break;

	case en_electrosphere:
		new_actor->trydir = static_cast<dirtype>(dir_which);
		new_actor->flags &= ~FL_SOLID;
		new_actor->temp1 = SPR_ELECTRO_SPHERE_ROAM1;
		new_actor->speed = bstone::math::fixed_to_floating(3096);
		new_actor->lighting = NO_SHADING; // NO shading
		NewState(new_actor, &s_ofs_bounce);
		SphereStartDir(new_actor);
		break;

	case en_podegg:
		new_actor->temp1 = SPR_POD_EGG;
		if (scan_value == 0xffff)
		{
			new_actor->temp2 = 60 * 5 + (60 * (US_RndT() % 20));
		}
		else
		{
			new_actor->temp2 = scan_value * 60;
		}
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		if (new_actor->temp2 == 0xff * 60)
		{
			new_actor->flags &= ~FL_SHOOTABLE;
		}
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE | FL_FAKE_STATIC;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_pod:
		new_actor->temp1 = SPR_POD_WALK1;
		new_actor->speed = SPDPATROL;
		new_actor->ammo = static_cast<std::uint8_t>(-1);
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	case en_genetic_guard:
		new_actor->temp1 = SPR_GENETIC_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	case en_mutant_human1:
		new_actor->temp1 = SPR_MUTHUM1_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_mutant_human2:
		new_actor->temp1 = SPR_MUTHUM2_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_scan_wait_alien:
		new_actor->temp1 = SPR_SCAN_ALIEN_READY;
		new_actor->flags |= FL_STATIONARY | FL_NO_SLIDE | FL_FAKE_STATIC;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_lcan_wait_alien:
		new_actor->temp1 = SPR_LCAN_ALIEN_READY;
		new_actor->flags |= FL_STATIONARY | FL_NO_SLIDE | FL_FAKE_STATIC;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_morphing_spider_mutant:
	case en_morphing_reptilian_warrior:
	case en_morphing_mutanthuman2:
		if (scan_value == 0xffff)
		{
			new_actor->temp2 = -1; // set to max! // 60*5+(60*(US_RndT()%20));
		}
		else
		{
			new_actor->temp2 = scan_value * 60;
		}

		if (new_actor->temp2 == 0xff * 60)
		{
			new_actor->flags &= ~FL_SHOOTABLE;
		}

		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->temp1 = MorphShapes[which - en_morphing_spider_mutant];
		new_actor->flags |= FL_FAKE_STATIC;
		new_actor->flags2 = FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE;
		NewState(new_actor, &s_ofs_random);
		break;

	case en_gurney_wait:
		if (scan_value == 0xffff)
		{
			new_actor->temp3 = 0;
		}
		else
		{
			new_actor->temp3 = (scan_value & 0xff) * 60;
		}
		new_actor->temp1 = SPR_GURNEY_MUT_READY;
		new_actor->flags |= FL_STATIONARY | FL_PROJ_TRANSPARENT | FL_NO_SLIDE | FL_FAKE_STATIC;

		new_actor->flags2 =
			(assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_lcan_alien: // Large Canister Alien - Out of can.
		new_actor->temp1 = SPR_LCAN_ALIEN_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	case en_scan_alien: // Small Canister Alien - Out of can.
		new_actor->temp1 = SPR_SCAN_ALIEN_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags |= FL_PROJ_TRANSPARENT;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	case en_gurney: // Gurney Mutant - Off of gurney.
		new_actor->temp1 = SPR_GURNEY_MUT_W1;
		new_actor->speed = ALIENSPEED;
		new_actor->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE;
		new_actor->ammo = ALIENAMMOINIT;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	default:
		break;
	}

	CheckForSpecialTile(new_actor, tilex, tiley);

	if (which < static_cast<enemy_t>(SPACER1_OBJ))
	{
		new_actor->hitpoints = get_start_hit_point(which);
	}
}


// ---------------------------------------------------------------------------
// T_OfsThink() - Think for general Offset Objects
//
// NOTE: This think is used for NON-SmartAnim objects
// ---------------------------------------------------------------------------

using GrenadeShapes = std::vector<std::int16_t>;

GrenadeShapes grenade_shapes;

void initialize_grenade_shape_constants()
{
	grenade_shapes = {
		SPR_GRENADE_FLY3,
		SPR_GRENADE_FLY3,
		SPR_GRENADE_FLY2,
		SPR_GRENADE_FLY1,
		SPR_GRENADE_FLY2,
		SPR_GRENADE_FLY2,
		SPR_GRENADE_FLY3,
		SPR_GRENADE_FLY4,
		0,
	};
}

void T_OfsThink(
	objtype* obj)
{
	const auto& assets_info = get_assets_info();

	std::int8_t dx, dy, dist;

	switch (obj->obclass)
	{
	case plasma_detonator_reserveobj:
		break;

	case plasma_detonatorobj:
		if (obj->temp3 < tics)
		{
			BlastNearDoors(obj->tilex, obj->tiley);

			obj->lighting = NO_SHADING; // no shading
			obj->flags &= ~FL_SHOOTABLE;
			obj->obclass = pd_explosionobj;
			A_DeathScream(obj);
			InitSmartSpeedAnim(obj, SPR_DETONATOR_EXP1, 0, 7, at_ONCE, ad_FWD, 3 + (US_RndT() & 3));
			return;
		}
		else
		{
			obj->temp3 -= tics;
		}

		if (obj->ammo >= tics)
		{
			obj->ammo -= static_cast<std::uint8_t>(tics);
		}
		else
		{
			obj->ammo = DETONATOR_FLASH_RATE;
			if (obj->temp1 == SPR_DOORBOMB)
			{
				obj->temp1 = SPR_ALT_DOORBOMB;
			}
			else
			{
				obj->temp1 = SPR_DOORBOMB;
			}
		}
		break;

	case grenadeobj:
	{
		T_Projectile(obj);

		// Check for range and update shape...

		if (obj->obclass == grenadeobj)
		{
			//
			// Has not exploded yet...
			//

			dx = obj->s_tilex - obj->tilex;
			dx = ABS(dx);
			dy = obj->s_tiley - obj->tiley;
			dy = ABS(dy);
			dist = dx > dy ? dx : dy;

			// Reached end of range?

			if ((obj->temp1 = grenade_shapes[static_cast<int>(dist)]) == 0)
			{
				obj->obclass = gr_explosionobj;
				InitSmartSpeedAnim(obj, SPR_GRENADE_EXPLODE1, 0, 4, at_ONCE, ad_FWD, 3 + (US_RndT() & 3));
			}
		}

		if (obj->obclass != grenadeobj)
		{
			ExplodeRadius(obj, 50 + (US_RndT() & 0x7f), false);
			A_DeathScream(obj);
		}
	}
	break;

	case bfg_shotobj:
	{
		T_Projectile(obj);

		if (obj->temp1 != SPR_BFG_WEAPON_SHOT3 && obj->obclass == bfg_shotobj)
		{
			//
			// Has not exploded yet...
			//

			dx = obj->s_tilex - obj->tilex;
			dx = ABS(dx);
			dy = obj->s_tiley - obj->tiley;
			dy = ABS(dy);
			dist = dx > dy ? dx : dy;

			obj->temp1 = SPR_BFG_WEAPON_SHOT2 + (dist >> 2);
		}

		if (obj->obclass != bfg_shotobj)
		{
			ExplodeRadius(obj, 50 + (US_RndT() & 0x7f), false);
			A_DeathScream(obj);
		}
	}
	break;

	case ventdripobj:
		// Decrement timer...
		//
		if (tics < obj->temp2)
		{
			obj->temp2 -= tics;
			break;
		}

		// Assign random delay and next shape.
		//
		obj->temp2 = 5 + (US_RndT() % 10);

		obj->temp1 -= obj->temp3;
		obj->temp3 = (obj->temp3 + 1) % 4;
		obj->temp1 += obj->temp3;
		break;

	case flickerlightobj:
		if (tics < obj->temp2)
		{
			obj->temp2 -= tics;
			break;
		}

		if ((obj->temp1 == SPR_DECO_ARC_1) || (US_RndT() & 15))
		{
			// Becomes darker....

			auto t = static_cast<std::int8_t>((US_RndT() & 1));

			obj->lighting = LAMP_ON_SHADING + ((t + 3) << 2);
			obj->temp1 = SPR_DECO_ARC_2 + t;
			obj->temp2 = 2 + (US_RndT() % 2);
		}
		else
		{
			obj->temp1 = SPR_DECO_ARC_1;
			obj->temp2 = (4 + US_RndT() % 10) * 60;
			obj->lighting = LAMP_ON_SHADING;
		}
		break;

	case electrosphereobj:
		{
			const auto max_frames = (assets_info.is_aog() ? 3 : 4);
			const auto oldofs = obj->temp1 - SPR_ELECTRO_SPHERE_ROAM1;
			auto ofs = US_RndT() % max_frames;

			if (ofs == oldofs)
			{
				ofs = (ofs + 1) % max_frames;
			}

			obj->temp1 = static_cast<std::int16_t>(SPR_ELECTRO_SPHERE_ROAM1 + ofs);
			break;
		}

	case podobj:
		if (obj->flags & FL_VISIBLE)
		{
			FirstSighting(obj);
		}
		break;

	case podeggobj:
		if (!(obj->flags & FL_VISIBLE))
		{
			break;
		}

		if (obj->temp2 == 0xff * 60)
		{
			break;
		}

		if (obj->temp2 > tics)
		{
			obj->temp2 -= tics;
			break;
		}

		sd_play_actor_voice_sound(PODHATCHSND, *obj);
		InitSmartSpeedAnim(obj, SPR_POD_HATCH1, 0, 2, at_ONCE, ad_FWD, assets_info.is_aog() ? 45 : 7);
		break;

	case morphing_spider_mutantobj:
	case morphing_reptilian_warriorobj:
	case morphing_mutanthuman2obj:
		if (!(obj->flags & FL_VISIBLE))
		{
			break;
		}

		if (obj->temp2 > tics)
		{
			obj->temp2 -= tics;
			break;
		}

		obj->flags &= ~FL_SHOOTABLE;
		InitSmartSpeedAnim(obj, obj->temp1, 0, 8, at_ONCE, ad_FWD, 2);
		break;

	case crate1obj:
	case crate2obj:
	case crate3obj:
	case lcan_wait_alienobj: // These objs wait till they are destroyed
	case scan_wait_alienobj: //   before doing anything.
		break;

	case gurney_waitobj:
		if (obj->flags & FL_VISIBLE)
		{
			SightPlayer(obj);
		}
		break;

	case spider_mutantshotobj:
	case final_boss4shotobj:
	case acid_dragonshotobj:
		obj->temp1 = BossShotShapes[obj->obclass - spider_mutantshotobj] + (US_RndT() % 3);
		T_Projectile(obj);
		break;

	default:
		T_Stand(obj);
		break;
	}
}

std::int16_t RandomSphereDir(
	enemy_t enemy)
{
	std::int16_t dir = 0;

	switch (enemy)
	{
	case en_vertsphere:
		dir = ((US_RndT() % 2) * 4) + 2;
		break;

	case en_horzsphere:
		dir = (US_RndT() % 2) * 4;
		break;

	case en_diagsphere:
		dir = ((US_RndT() % 4) * 2) + 1;
		break;

	default:
		break;
	}

	return dir;
}

void SphereStartDir(
	objtype* ob)
{
	std::int8_t loop;

	actorat[ob->tilex][ob->tiley] = nullptr;
	for (loop = 0; loop < 3; loop++)
	{
		ob->dir = static_cast<dirtype>(RandomSphereDir(
			static_cast<enemy_t>(en_vertsphere + ((ob->trydir - en_vertsphere + loop) % 3))));

		if (!TryWalk(ob, true))
		{
			ob->dir = opposite[ob->dir];
			if (!TryWalk(ob, true))
			{
				ob->dir = nodir;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	actorat[ob->tilex][ob->tiley] = ob;
}

void CheckForcedMove(
	objtype* ob)
{
	std::int16_t olddir;

	olddir = static_cast<std::int16_t>(ob->dir);
	ob->dir = static_cast<dirtype>(RandomSphereDir(static_cast<enemy_t>(ob->trydir)));
	if (!TryWalk(ob, false))
	{
		ob->dir = opposite[ob->dir];
		if (!TryWalk(ob, false))
		{
			ob->dir = static_cast<dirtype>(olddir);
		}
	}
}

void T_OfsBounce(
	objtype* ob)
{
	// Should Electro-Sphere decrease player's health?
	//

	const auto dx = std::abs(player->x - ob->x);
	const auto dy = std::abs(player->y - ob->y);
	const auto dist = dx > dy ? dx : dy;

	if (dist < 1.0)
	{
		sd_play_actor_weapon_sound(ELECARCDAMAGESND, *ob);
		TakeDamage(4, ob);
	}


	// Safety net - restart! Try to get a 'nodir' actor moving again.
	//
	if (ob->dir == nodir)
	{
		SphereStartDir(ob);

		if (ob->dir == nodir)
		{
			return;
		}
	}

	// Make actor bounce around the map!
	//
	auto move = ob->speed * tics;

	while (move != 0.0)
	{
		// Can actor move without reaching destination tile?
		//
		if (move < ob->distance)
		{
			MoveObj(ob, move); // YES!
			break;
		}

		// Align actor on destination tile.
		//
		ob->x = ob->tilex + 0.5;
		ob->y = ob->tiley + 0.5;

		// Decrement move distance and reset distance to next tile.
		//
		move -= ob->distance;
		ob->distance = 1.0;

		// Can actor continue moving in current direction?
		//
		auto oldtx = ob->tilex;
		auto oldty = ob->tiley;

		if (!TryWalk(ob, true))
		{
			auto check_opposite = false;

			// Restore tilex/tiley
			//
			ob->tilex = oldtx;
			ob->tiley = oldty;

			// Direction change is based on current direction.
			//
			switch (ob->dir)
			{
				case northeast:
				case northwest:
				case southeast:
				case southwest:
					if (ob->trydir != static_cast<dirtype>(en_diagsphere))
					{
						if (!MoveTrappedDiag(ob))
						{
							SphereStartDir(ob);
						}
						continue;
					}

					ob->dir = static_cast<dirtype>((ob->dir + 2) % 8); // Try 90 degrees to the left

					if (TryWalk(ob, false))
					{
						break;
					}

					ob->dir = opposite[ob->dir]; // Try 90 degrees to the right

					if (TryWalk(ob, false))
					{
						break;
					}

					ob->dir = static_cast<dirtype>((ob->dir + 2) % 8); // Back to original direction..
					// Must be in a corner...

					check_opposite = true;
					break;

				case north:
				case south:
					if (ob->trydir != static_cast<dirtype>(en_vertsphere))
					{
						if (!MoveTrappedDiag(ob))
						{
							SphereStartDir(ob);
						}
						continue;
					}

					check_opposite = true;
					break;

				case east:
				case west:
					if (ob->trydir != static_cast<dirtype>(en_horzsphere))
					{
						if (!MoveTrappedDiag(ob))
						{
							SphereStartDir(ob);
						}
						continue;
					}

					check_opposite = true;
					break;

				default:
					break;
			}

			// Check opposite direction?
			//
			if (check_opposite)
			{
				ob->dir = opposite[ob->dir];
				if (!TryWalk(ob, false))
				{
					ob->dir = nodir;
				}
			}

			// TryWalk  "true"  to set new destination tile.
			//
			if (!TryWalk(ob, true))
			{
				ob->dir = nodir;
			}
		}
		else
		{
			const auto orgx = ob->tilex;
			const auto orgy = ob->tiley;

			ob->tilex = oldtx;
			ob->tiley = oldty;

			if (!MoveTrappedDiag(ob))
			{
				ob->tilex = orgx;
				ob->tiley = orgy;
			}
		}
	}
}

bool MoveTrappedDiag(
	objtype* ob)
{
	// Don't mess with HORZs, VERTs, or normal DIAGs.
	//
	if ((ob->trydir != static_cast<dirtype>(en_diagsphere)) ||
		((ob->dir >> 1 << 1) != ob->dir))
	{
		return false;
	}

	// When a DIAG gets to an open area, change it to diagonal movement!
	//
	if (!CheckTrappedDiag(ob))
	{
		return true;
	}

	// DIAGs have a 50/50 chance of changing direction.
	//
	if (US_RndT() & 1)
	{
		return false;
	}

	// Try changing directions.
	//
	if (ob->dir == north || ob->dir == south)
	{
		ob->trydir = static_cast<dirtype>(en_horzsphere);
	}
	else
	{
		ob->trydir = static_cast<dirtype>(en_vertsphere);
	}

	CheckForcedMove(ob);

	ob->trydir = static_cast<dirtype>(en_diagsphere);

	return TryWalk(ob, true);
}

bool CheckTrappedDiag(
	objtype* ob)
{
	dirtype orgdir = ob->dir;

	for (ob->dir = northeast; ob->dir <= southeast; ob->dir += 2)
	{
		if (TryWalk(ob, false))
		{
			break;
		}
	}

	if (ob->dir > southeast)
	{
		ob->dir = orgdir;
		return true;
	}
	else
	{
		TryWalk(ob, true);
		return false;
	}
}

// ---------------------------------------------------------------------------
// FindObj() - This function will search the objlist for an object
//              of a given type at a given tilex & tiley coords.
//
// PARAMETERS:
//  which - objtype of actor to look for.
//  tilex - tile x coord of actor looking for (-1 == Dont care)
//  tiley - tile y coord of actor looking for (-1 == Dont care)
//
// ---------------------------------------------------------------------------
objtype* FindObj(
	classtype which,
	std::int16_t tilex,
	std::int16_t tiley)
{
	objtype* obj;

	for (obj = player->next; obj; obj = obj->next)
	{
		if (obj->obclass == which)
		{
			if (tilex != -1)
			{
				if (tilex != obj->tilex)
				{
					continue;
				}
			}

			if (tiley != -1)
			{
				if (tiley != obj->tiley)
				{
					continue;
				}
			}

			return obj;
		}
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// SpawnHiddenOfs() - This function will spawn a given offset actor at a passed
//                  x & y coords and then move the actor to (0,0) for hidding
//
// PURPOSE: This function was created mainly for spawning reserve objects that
//          would later be simply moved from their (0,0) coords to a new
//          location.
//
// NOTE: When this actor is moved from it's (0,0) coords, you will need to
//       update the area number that this actor is standing in.
//
// ---------------------------------------------------------------------------
void SpawnHiddenOfs(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley)
{
	nevermark = true;
	SpawnOffsetObj(which, tilex, tiley); // Spawn a reserve
	nevermark = false;
	new_actor->tilex = 0;
	new_actor->tiley = 0;
	new_actor->x = 0.5;
	new_actor->y = 0.5;
}

// ---------------------------------------------------------------------------
// FindHiddenOfs() - This function will find a hidden ofs actor which was
//                  hidden using SpawnHiddenOfs() and will return a pointer
//                  to the actor in the objlist.
//
// NOTE: When this actor is moved from it's (0,0) coords, you will need to
//       update the area number that this actor is standing in or use
//       MoveHiddenOfs().
// ---------------------------------------------------------------------------
objtype* FindHiddenOfs(
	classtype which)
{
	auto obj = FindObj(which, -1, -1);

	if (!obj)
	{
		BSTONE_THROW_STATIC_SOURCE("Unable to find a \"Hidden Actor\" at location (0, 0).");
	}

	return obj;
}

// ---------------------------------------------------------------------------
// MoveHiddenOfs() - This function will find a hidden ofs actor which was
//                  hidden using SpawnHiddenOfs() and will move the actor
//                  to a new passed coords and change the actors class.
//
// RETURNS: Success = Ptr to the object that was moved.
//          Failure = nullptr.
//
// NOTE: This function updates the actors 'areanumber' to the newly changed
//       coords.
// ---------------------------------------------------------------------------
objtype* MoveHiddenOfs(
	classtype which_class,
	classtype new_class,
	const double x,
	const double y)
{
	auto obj = FindHiddenOfs(which_class);

	if (obj)
	{
		obj->obclass = new_class;
		obj->x = x;
		obj->y = y;
		obj->tilex = static_cast<std::uint8_t>(x);
		obj->tiley = static_cast<std::uint8_t>(y);
		obj->areanumber = GetAreaNumber(obj->tilex, obj->tiley);

		return obj;
	}

	return nullptr;
}


// ===========================================================================
//
//  SMART_ANIM ANIMATION ROUTINES
//
//
// * NOTES: OffsetObjects require the use of TEMP1 of the object structure!
//
// * NOTES: THINK function AnimateOfsObj() requires the use of TEMP3 of the
//                              object structure!
//
// ===========================================================================

// ---------------------------------------------------------------------------
// InitSmartAnim() - Sets up an object for a SmartAnimation
//
// PARAMETERS:  Obj  - Ptr to object structure
//              ShapeNum - First shape number in anim
//              StartOfs - Starting offset in the animation to start with.
//              MaxOfs  - Ending offset in the animation.
//              AnimType - Type of animation (at_CYCLE,at_ONCE,or at_REBOUND)
//              AnimDir - Direction of animation to start (ad_FWD, or ad_REV)
//
// ** This function should ALWAY be used to init/start SmartAnimations! **
//
// NOTE : It is the programmers responsibility to watch bit field ranges on
//                       the passed parameters.
//
// NOTES: THINK function AnimateOfsObj() requires the use of TEMP3 of the
//                  object structure!
//
// ---------------------------------------------------------------------------

void InitSmartAnimStruct(
	objtype* obj,
	std::uint16_t ShapeNum,
	std::uint8_t StartOfs,
	std::uint8_t MaxOfs,
	animtype_t AnimType,
	animdir_t AnimDir)
{
	obj->temp1 = ShapeNum + StartOfs;

	obj->temp3 = 0;
	ofs_anim_t::set_curframe(StartOfs, obj);
	ofs_anim_t::set_maxframe(MaxOfs, obj);
	ofs_anim_t::set_animtype(static_cast<std::uint16_t>(AnimType), obj);
	ofs_anim_t::set_animdir(static_cast<std::uint16_t>(AnimDir), obj);

}

void InitAnim(
	objtype* obj,
	std::uint16_t ShapeNum,
	std::uint8_t StartOfs,
	std::uint8_t MaxOfs,
	animtype_t AnimType,
	animdir_t AnimDir,
	std::uint16_t Delay,
	std::uint16_t WaitDelay)
{
	InitSmartAnimStruct(obj, ShapeNum, StartOfs, MaxOfs, AnimType, AnimDir);
	obj->s_tilex = static_cast<std::uint8_t>(WaitDelay);
	obj->s_tiley = static_cast<std::uint8_t>(Delay);
	NewState(obj, &s_ofs_smart_anim);
}

//
// "s_ofs_smart_anim" - The following structure is designed for "Smart
//  Animations."
//
// OVERVIEW:    This structure will allow the flexiblity to perform ANY type
//  of animation, custom, etc. using one single structure.
//  All of the animations are using "Offset" techniques, where
//  the current shapenumber of the object is in TEMP1 of the
//  object structure,  Code is then written to handle changing
//  the shapenumber of each object individually.
//
// FUNCTIONS:  T_SmartThink() - Is a think that is call continusly and is
//  special coded for an object/actor.
//
// T_SmartThought() - Is called only once allowing for other
//  special codeing, (Ex. the spawning of another animation at
//  anypoint in the current animation, death screams, sounds,etc)
//
// * NOTES:     The T_SmartThink could be modified to handle generic animations
//          like cycle, once, rebound, etc. using flags in the object struct.
//
//

statetype s_ofs_smart_anim = {0, 0, 1, T_SmartThought, nullptr, &s_ofs_smart_anim};
statetype s_ofs_smart_anim2 = {0, 0, 1, T_SmartThought, nullptr, &s_ofs_smart_anim2};

//
// Functions
//

void T_SmartThink(objtype* ob)
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_aog_full())
	{
		return;
	}

	switch (ob->obclass)
	{
		case cyborg_warriorobj:
		case mech_guardianobj:
			sd_play_actor_walking_sound(ROBOT_SERVOSND, *ob);
			break;
	}
}

// ---------------------------------------------
// T_SmartThought() - A think for ofset objects
//  that is called ONLY ONCE
//  per state change.
// ---------------------------------------------
void T_SmartThought(
	objtype* obj)
{
	const auto& assets_info = get_assets_info();

	switch (obj->obclass)
	{
		case green_oozeobj:
		case black_oozeobj:
		case green2_oozeobj:
		case black2_oozeobj:
		{
			if (((US_RndT() & 7) == 7) &&
				ofs_anim_t::get_curframe(obj) == 2 &&
				obj->tilex == player->tilex &&
				obj->tiley == player->tiley)
			{
				TakeDamage(4, obj);
			}
		}
		break;

		case arc_barrierobj:
			if (BARRIER_STATE(obj) == bt_DISABLED)
			{
				return;
			}

			if (US_RndT() < 0x10)
			{
				constexpr auto min_distance = bstone::math::fixed_to_floating(0x16000);

				const auto dx = std::abs(player->x - obj->x);
				const auto dy = std::abs(player->y - obj->y);

				if (dy <= min_distance && dx <= min_distance)
				{
					sd_play_actor_weapon_sound(ELECARCDAMAGESND, *obj);
					TakeDamage(4, obj);
				}
			}

		case post_barrierobj:
			//
			// Check for Turn offs
			//
			if ((std::uint16_t)obj->temp2 != 0xffff)
			{
				const auto barrier_index = gamestate.get_barrier_index(obj->temp2);

				if (!gamestate.barrier_table[barrier_index].on)
				{
					ToggleBarrier(obj);
				}
			}
			break;

		case volatiletransportobj:
		case floatingbombobj:
			if (obj->lighting)
			{
				// Slowly inc back to

				obj->lighting += static_cast<std::int8_t>(ofs_anim_t::get_curframe(obj));
				if (obj->lighting > 0)
				{
					obj->lighting = 0;
				}
			}
			break;

		default:
			break;
	}

	if (ofs_anim_t::get_animtype(obj) != 0)
	{
		int old_frame = ofs_anim_t::get_curframe(obj);
		bool is_animated = AnimateOfsObj(obj);

		if (is_animated)
		{
			switch (obj->obclass)
			{
				case morphing_spider_mutantobj:
				case morphing_reptilian_warriorobj:
				case morphing_mutanthuman2obj:
					{
						auto dx = obj->obclass - morphing_spider_mutantobj;
						obj->temp1 = MorphEndShapes[dx];
						sd_play_actor_voice_sound(MorphSounds[dx], *obj);
						obj->obclass = static_cast<classtype>(MorphClass[dx]);
						obj->hitpoints = get_start_hit_point(MorphClass[dx] - rentacopobj);
						obj->flags &= ~FL_FAKE_STATIC;
						obj->flags |= FL_PROJ_TRANSPARENT | FL_SHOOTABLE;
						NewState(obj, &s_ofs_chase1);
					}
					break;

				case podeggobj:
					obj->flags |= FL_SHOOTABLE;
					obj->obclass = podobj;
					obj->temp1 = SPR_POD_WALK1;
					NewState(obj, &s_ofs_chase1);
					obj->hitpoints = get_start_hit_point(en_pod);
					break;

				case rotating_cubeobj:
					if (assets_info.is_aog_full())
					{
						if (obj->temp1 == SPR_VITAL_OUCH)
						{
							InitSmartSpeedAnim(obj, SPR_VITAL_STAND, 0, 0, at_NONE, ad_FWD, 0);
						}
						else if (obj->temp1 == SPR_VITAL_DIE_8)
						{
							InitSmartSpeedAnim(obj, SPR_VITAL_DEAD_1, 0, 2, at_CYCLE, ad_FWD, 16);

							if (get_remaining_generators() == 0)
							{
								obj->ammo = 1;
							}
						}
					}
					else if (assets_info.is_ps())
					{
						DISPLAY_TIMED_MSG(pd_floorunlocked, MP_FLOOR_UNLOCKED, MT_GENERAL);
						sd_play_player_item_sound(ROLL_SCORESND);
						obj->lighting = 0;
					}
					break;

				case inertobj:
				case fixup_inertobj:
				case scan_wait_alienobj:
				case lcan_wait_alienobj:
				case gurney_waitobj:
					break;

				case gold_morphobj:
					//
					// Game completed!
					//
					playstate = ex_victorious;
					obj->state = nullptr;  // Mark to be removed.
					break;


				case volatiletransportobj:
				case floatingbombobj:
					NewState(obj, &s_scout_dead);
					obj->lighting = 0;
					return;

				default:
					obj->state = nullptr;  // Mark to be removed.
					break;
			}
		}

		int new_frame = ofs_anim_t::get_curframe(obj);
		bool is_frame_changed = (old_frame != new_frame);

		if (assets_info.is_aog_full() &&
			!is_animated &&
			is_frame_changed &&
			obj->obclass == rotating_cubeobj)
		{
			if (obj->temp1 == SPR_VITAL_DIE_2)
			{
				display_remaining_generators();
			}
			else if (obj->temp1 == SPR_VITAL_DIE_4)
			{
				display_remaining_generators();
				ExplodeRadius(obj, 35 + (US_RndT() & 15), true);
			}
			else if (obj->temp1 == SPR_VITAL_DEAD_1)
			{
				if (obj->ammo > 0)
				{
					obj->ammo -= 1;

					if (obj->ammo == 0)
					{
						playstate = ex_victorious;
					}
				}
			}
		}

		if (ofs_anim_t::get_curframe(obj) == 3)
		{
			switch (obj->obclass)
			{
				case doorexplodeobj:
					if (!obj->temp2)
					{
						std::int16_t avail, total, i;

						// Make sure that there are at least DR_MIN_STATICS

						avail = MAXSTATS;
						total = static_cast<std::int16_t>(laststatobj - &statobjlist[0]);
						for (i = 0; i < total; i++)
						{
							if (statobjlist[i].shapenum != -1)
							{
								avail--;
							}
						}

						if ((avail > DR_MIN_STATICS) && (US_RndT() & 1))
						{
							static_cast<void>(SpawnStatic(obj->tilex, obj->tiley, DOOR_RUBBLE_STATNUM));
						}
					}
					break;

				case explosionobj:
					if (!obj->temp2)
					{
						ExplodeRadius(obj, 20, true);
						MakeAlertNoise(obj);
						obj->temp2 = 1;
					}
					break;

				case pd_explosionobj:
					if (!obj->temp2)
					{
						ExplodeRadius(obj, PLASMA_DETONATOR_DAMAGE, true);
						MakeAlertNoise(obj);
						obj->temp2 = 1;
					}

				case bfg_explosionobj:
					if (!obj->temp2)
					{
						ExplodeRadius(obj, BFG_DAMAGE, true);
						MakeAlertNoise(obj);
						obj->temp2 = 1;
					}
					break;

				case gurney_waitobj:
// FIXME
// Remove this or convert the reserved actor into real one.
#if 0
					if (obj->temp2)
					{
						RemoveObj(ui16_to_actor(obj->temp2));
					}
#endif

					SpawnOffsetObj(en_gurney, obj->tilex, obj->tiley);
					NewState(obj, &s_ofs_static);
					// obj->obclass = fixup_inertobj;
					break;

				case scan_wait_alienobj:
// FIXME
// Remove this or convert the reserved actor into real one.
#if 0
					if (obj->temp2)
					{
						RemoveObj(ui16_to_actor(obj->temp2));
					}
#endif

					SpawnOffsetObj(en_scan_alien, obj->tilex, obj->tiley);
					NewState(obj, &s_ofs_static);
					// obj->obclass = fixup_inertobj;
					break;

				case lcan_wait_alienobj:
// FIXME
// Remove this or convert the reserved actor into real one.
#if 0
					if (obj->temp2)
					{
						RemoveObj(ui16_to_actor(obj->temp2));
					}
#endif

					SpawnOffsetObj(en_lcan_alien, obj->tilex, obj->tiley);
					NewState(obj, &s_ofs_static);
					// obj->obclass = fixup_inertobj;
					break;

				default:
					break;
			}
		}

		if (ofs_anim_t::get_curframe(obj) == 2)
		{
			switch (obj->obclass)
			{
				case volatiletransportobj:
					if (!(obj->flags & FL_INTERROGATED))
					{
						if (US_RndT() < 0x7f)
						{
							usedummy = true;
							SpawnOffsetObj(en_green_ooze, obj->tilex, obj->tiley);
							new_actor->x = obj->x + (0.5 * US_RndT() / 255.0);
							new_actor->y = obj->y + (0.5 * US_RndT() / 255.0);
							new_actor->tilex = static_cast<std::uint8_t>(new_actor->x);
							new_actor->tiley = static_cast<std::uint8_t>(new_actor->y);
							usedummy = false;
						}
					}

				case floatingbombobj:
					if (!(obj->flags & FL_INTERROGATED))
					{
						T_ExplodeDamage(obj);
						obj->flags |= FL_INTERROGATED;
					}
					break;

				default:
					break;
			}
		}
	}
}

// ------------------------------------------------------------------
// AnimateOfsObj() - Animation routine for OffsetObjects.
//
// IMPORTANT NOTE: This does NOT work on the same principle as the old
// """"""""""""""  AnimateObj() function!
//
//
// obj->TEMP1   - Holds the current shape number.
//
// obj->TEMP3.maxframe  - Frames in anim. (0 == 1 Frame, 1 == 2 Frames, etc))
//
// obj->TEMP3.curframe  - Holds the the shape offset which TEMP1 is off from
//      the 1st shape in the anim. ALWAYS POSITIVE NUMBER!
//      REVerse animations have the "curframe" on the last
//      offset and TEMP1-CURFRAME should equ the first frame
//      and MAXFRAME should equ the offset for the first
//      frame.
//
// obj->TEMP3.animdir   - Determines the direction of the animation.
//
//
// obj->S_TILEX - Current number of tics remaining before advancing
//      to next frame.
//
// obj->S_TILEY - Delay Tic value per frame - This value is copied
//      into  S_TILEX upon reaching the end of a frame.
//
//      IF you want to do a reverse animation then you would need to init
// TEMP1 to the shape number of the LAST Shape, "curframe" to the       offset
//      value from the FIRST FRAME in anim and set "animdir" to at_REV.
//
// * NOTES: This is an OffsetObject which requires the use of TEMP1 of the
//      object structure!
//
// * NOTES: The use of a SmartAnim requires the use of TEMP3 of the object
//      structure!!! Therefore, Any other THINKS (like LookForGoodies)
//      CAN NOT be used while this routine is being called also.
//
// * NOTES: ALL SmartAnimations have the SAME animation delay rates!    Sorry!
//
// * NOTES: The SmartAnimations use S_TILEX & S_TILEY for animation delay
//      values - All SMART actors can not be used if they are using
//      a "smart" think!
//
// ------------------------------------------------------------------
bool AnimateOfsObj(
	objtype* obj)
{
	bool Done = false;

	if (obj->s_tilex)
	{ // Check animation delay.
		if (obj->s_tilex < tics)
		{
			obj->s_tilex = 0;
		}
		else
		{
			obj->s_tilex -= static_cast<std::uint8_t>(tics);
		}
		return false;
	}

	switch (ofs_anim_t::get_animtype(obj))
	{ // Animate this puppy!
	case at_ONCE:
	case at_CYCLE:
		switch (ofs_anim_t::get_animdir(obj))
		{
		case ad_FWD:
			if (ofs_anim_t::get_curframe(obj) < ofs_anim_t::get_maxframe(obj))
			{
				AdvanceAnimFWD(obj);
			}
			else if (ofs_anim_t::get_animtype(obj) == at_CYCLE)
			{
				// Pull shape number back to start...

				obj->temp1 -= ofs_anim_t::get_curframe(obj);

				// Reset Cycle Animation

				ofs_anim_t::set_curframe(0, obj);

				obj->s_tilex = obj->s_tiley;
			}
			else
			{
				// Terminate ONCE Anim type

				ofs_anim_t::set_animtype(at_NONE, obj);
				Done = true;
			}
			break;
		}
		break;
	}

	return Done;
}

void AdvanceAnimFWD(
	objtype* obj)
{
	auto curframe = ofs_anim_t::get_curframe(obj);
	ofs_anim_t::set_curframe(curframe + 1, obj);

	obj->temp1++;
	obj->s_tilex = obj->s_tiley;
}


// ==========================================================================
//
// WALL SWITCH ACTIVATION
//
// ==========================================================================


// --------------------------------------------------------------------------
// ActivateWallSwitch() - Updates the Map, Actors, and Tables for wall switchs
// --------------------------------------------------------------------------
void ActivateWallSwitch(
	std::uint16_t iconnum,
	std::int16_t x,
	std::int16_t y)
{
	const std::uint16_t states[] = {OFF_SWITCH, ON_SWITCH};

	if ((iconnum & 0xFF00) == 0xF800)
	{
		//
		// Update tile map & Display switch'd message
		//
		const auto num = iconnum & 0xFF;

		auto level = 0;
		auto index = 0;
		gamestate.decode_barrier_index(num, level, index);
		const auto group_offset = gamestate.get_barrier_group_offset(level);

		const auto barrier_index = group_offset + index;
		auto& barrier = gamestate.barrier_table[barrier_index];

		barrier.on ^= 1;
		auto newwall = states[barrier.on];
		tilemap[x][y] = static_cast<std::uint8_t>(states[barrier.on]);

		// BBi
		vid_hw_on_update_wall_switch(x, y);
		// BBi

		DisplaySwitchOperateMsg(num);
		sd_play_player_item_sound(SWITCHSND);
		auto tile = &tilemap[0][0];
		auto actor = reinterpret_cast<std::size_t*>(&actorat[0][0]);

		for (int mapx = 0; mapx < MAPSIZE; ++mapx)
		{
			for (int mapy = 0; mapy < MAPSIZE; ++mapy)
			{
				if (*tile == OFF_SWITCH || *tile == ON_SWITCH)
				{
					auto icon = mapsegs[1][farmapylookup[mapy] + mapx];

					if (icon == iconnum)
					{
						*tile = static_cast<std::uint8_t>(newwall);
						*actor = newwall;

						vid_hw_on_update_wall_switch(mapx, mapy);
					}
				}

				++tile;
				++actor;
			}
		}
	}
	else
	{
		DISPLAY_TIMED_MSG(SwitchNotActivateMsg, MP_WALLSWITCH_OPERATE, MT_GENERAL);
		sd_play_player_no_way_sound(NOWAYSND);
	}
}

// --------------------------------------------------------------------------
// DisplaySwitchOperateMsg() - Displays the Operating Barrier Switch message
//      for a particular level across the InfoArea.
// --------------------------------------------------------------------------
void DisplaySwitchOperateMsg(
	int coords)
{
	auto level = 0;
	auto index = 0;
	gamestate.decode_barrier_index(coords, level, index);
	const auto group_offset = gamestate.get_barrier_group_offset(level);

	const auto barrier_index = group_offset + index;
	auto barrier = &gamestate.barrier_table[barrier_index];

	static std::string message;

	if (barrier->on != 0)
	{
		message = "\r\r  ACTIVATING BARRIER";
	}
	else
	{
		message = "\r\r DEACTIVATING BARRIER";
	}

	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		if (assets_info.is_secret_level(level))
		{
			const auto secret_level = assets_info.secret_floor_get_index(level) + 1;

			message += "\r  ON SECRET FLOOR " + std::to_string(secret_level);
		}
		else
		{
			message += "\r      ON FLOOR " + std::to_string(level);
		}
	}

	DISPLAY_TIMED_MSG(message.c_str(), MP_WALLSWITCH_OPERATE, MT_GENERAL);
}

// --------------------------------------------------------------------------
// UpdateBarrierTable(x,y,level) - Finds/Inserts arc entry in arc list
//
// RETURNS: Offset into barrier_table[] for a particular arc
//
// --------------------------------------------------------------------------

namespace
{


std::uint16_t UpdateBarrierTable(
	const int level,
	const int x,
	const int y,
	const bool on_off)
{
	const auto& assets_info = get_assets_info();

	if (level >= assets_info.get_levels_per_episode())
	{
		BSTONE_THROW_STATIC_SOURCE("[BARR_UPD_TBL] Level index out of range.");
	}

	if (x <= 0 || x >= (MAPSIZE - 1) ||
		y <= 0 || y >= (MAPSIZE - 1))
	{
		BSTONE_THROW_STATIC_SOURCE("[BARR_UPD_TBL] Coordinates out of range.");
	}


	//
	// Scan Table...
	//
	const auto group_offset = gamestate.get_barrier_group_offset(level);
	const auto max_switches = assets_info.get_barrier_switches_per_level();

	for (int i = 0; i < max_switches; ++i)
	{
		auto& barrier = gamestate.barrier_table[group_offset + i];

		if (barrier.coord.tilex == x && barrier.coord.tiley == y)
		{
			return static_cast<std::uint16_t>(gamestate.encode_barrier_index(level, i));
		}
		else
		{
			if (barrier.on == 0xFF)
			{
				// Empty?
				// We have hit end of list - Add

				barrier.coord.tilex = static_cast<std::uint8_t>(x);
				barrier.coord.tiley = static_cast<std::uint8_t>(y);
				barrier.on = static_cast<std::uint8_t>(on_off);

				return static_cast<std::uint16_t>(gamestate.encode_barrier_index(level, i));
			}
		}
	}

	BSTONE_THROW_STATIC_SOURCE("[BARR_UPD_TBL] Too many barrier switches.");
}


} // namespace


std::uint16_t UpdateBarrierTable(
	const int level,
	const int x,
	const int y)
{
	const auto new_level = (level == 0xFF ? gamestate.mapon : level);

	return UpdateBarrierTable(new_level, x, y, true);
}

std::uint16_t UpdateBarrierTable(
	const int x,
	const int y,
	const bool on_off)
{
	return UpdateBarrierTable(gamestate.mapon, x, y, on_off);
}

// --------------------------------------------------------------------------
// ScanBarrierTable(x,y) - Scans a switch table for a arc in this level
//
// RETURNS :
//      0xFFFF - Not found in table
//      barrier - barrier_table of the barrier for [num]
//
// --------------------------------------------------------------------------
std::uint16_t ScanBarrierTable(
	std::uint8_t x,
	std::uint8_t y)
{
	const auto& assets_info = get_assets_info();
	const auto max_switches = assets_info.get_barrier_switches_per_level();

	const auto level = gamestate.mapon;
	const auto barrier_group_index = gamestate.get_barrier_group_offset(level);

	for (int i = 0; i < max_switches; ++i)
	{
		const auto& barrier = gamestate.barrier_table[barrier_group_index + i];

		if (barrier.coord.tilex == x && barrier.coord.tiley == y)
		{
			const auto code = gamestate.encode_barrier_index(level, i);

			// Found switch...
			return static_cast<std::uint16_t>(code);
		}
	}

	return static_cast<std::uint16_t>(0xFFFF); // Mark as EMPTY
}

// --------------------------------------------------------------------------
// Checks to see if the Barrier obj is free
// --------------------------------------------------------------------------
bool CheckActor(
	objtype* actor,
	std::uint16_t code)
{
	if (static_cast<std::uint16_t>(actor->temp2) == 0xFFFF)
	{ // Is this actor free?
//
// Connect actor to barrier switch (code is index into barrier table)
//

		actor->temp2 = static_cast<std::int16_t>(code); // This actor is NO longer a cycle actor.
		return true;
	}

	return false;
}

std::int16_t CheckAndConnect(
	std::int8_t x,
	std::int8_t y,
	std::uint16_t code)
{
	const std::int8_t offsets[] = {-1, 0, 1, 0};

	auto ob = objlist;

	do
	{
		switch (ob->obclass)
		{
		case arc_barrierobj:
		case post_barrierobj:
		case vpost_barrierobj:
		case vspike_barrierobj:
		{
			for (int loop = 0; loop < 4; loop++)
			{
				if ((ob->tilex == x + offsets[loop]) &&
					(ob->tiley == y + offsets[3 - loop]))
				{
					++bars_connected;

					if (CheckActor(ob, code))
					{
						CheckAndConnect(
							x + offsets[loop],
							y + offsets[3 - loop],
							code);
					}
				}
			}

			break;
		}

		default:
			break;
		}

		ob = ob->next;
	} while (ob);

	return bars_connected;
}

// --------------------------------------------------------------------------
// ConnectBarriers() - Scans the object list and finds the single barrier
//      that is connected by switch and checks to see if
//      there are any surrounding barriers that need to be
//      connected also.
// --------------------------------------------------------------------------
void ConnectBarriers()
{
	const auto& assets_info = get_assets_info();

	const auto max_switcher = assets_info.get_barrier_switches_per_level();
	const auto level = gamestate.mapon;
	const auto barrier_group_index = gamestate.get_barrier_group_offset(level);

	for (int i = 0; i < max_switcher; ++i)
	{
		const auto& barrier = gamestate.barrier_table[barrier_group_index + i];

		if (barrier.on != 0xFF)
		{
			bars_connected = 0;

			const auto code = gamestate.encode_barrier_index(level, i);

			if (CheckAndConnect(
				barrier.coord.tilex,
				barrier.coord.tiley,
				static_cast<std::uint16_t>(code)) == 0)
			{
				if (assets_info.is_ps())
				{
					BSTONE_THROW_STATIC_SOURCE("A barrier switch was not connect to any barriers.");
				}
			}
		}
	}
}


/*
=============================================================================

	BARRIERS

=============================================================================
*/

extern statetype s_barrier_transition;

void T_BarrierTransition(
	objtype* obj);

void T_BarrierShutdown(
	objtype* obj);

statetype s_barrier_transition = {0, 0, 15, T_BarrierTransition, nullptr, &s_barrier_transition};
statetype s_vpost_barrier = {0, SPR_DEMO, 15, T_BarrierTransition, nullptr, &s_vpost_barrier};
statetype s_spike_barrier = {0, SPR_DEMO, 15, T_BarrierTransition, nullptr, &s_spike_barrier};
statetype s_barrier_shutdown = {0, 0, 15, T_BarrierShutdown, nullptr, &s_barrier_shutdown};


void SpawnBarrier(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	bool OnOff)
{
	const auto& assets_info = get_assets_info();

	nevermark = !OnOff;
	SpawnNewObj(tilex, tiley, &s_ofs_stand);
	nevermark = false;

	if (OnOff)
	{
		new_actor->flags = FL_OFFSET_STATES | FL_BARRIER | FL_FAKE_STATIC | FL_SOLID;
	}
	else
	{
		new_actor->flags = FL_OFFSET_STATES | FL_BARRIER;
	}

	new_actor->obclass = static_cast<classtype>(rentacopobj + which);
	new_actor->ammo = static_cast<std::uint8_t>(OnOff);
	new_actor->temp2 = ScanBarrierTable(static_cast<std::uint8_t>(tilex), static_cast<std::uint8_t>(tiley));
	new_actor->flags2 = (assets_info.is_ps() ? FL2_BFGSHOT_SOLID : 0);

	switch (which)
	{
	case en_arc_barrier:
		new_actor->flags2 |= (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);

		if (OnOff)
		{
			InitSmartSpeedAnim(
				new_actor,
				SPR_ELEC_ARC1,
				US_RndT() % 3,
				2,
				at_CYCLE,
				ad_FWD,
				(assets_info.is_ps() ? 3 : 20) + (US_RndT() & (assets_info.is_ps() ? 3 : 7)));

			new_actor->lighting = LAMP_ON_SHADING;
		}
		else
		{
			NewState(new_actor, &s_barrier_transition);
			new_actor->temp3 = 0;
			new_actor->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
			new_actor->flags |= (FL_NEVERMARK | FL_NONMARK);
			new_actor->lighting = 0;
			BARRIER_STATE(new_actor) = bt_OFF;
			new_actor->temp1 = SPR_ELEC_ARC4;
		}
		break;

	case en_post_barrier:
		if (OnOff)
		{
			InitSmartSpeedAnim(
				new_actor,
				SPR_ELEC_POST1,
				US_RndT() % 3,
				2,
				at_CYCLE,
				ad_FWD,
				(assets_info.is_ps() ? 3 : 20) + (US_RndT() & (assets_info.is_ps() ? 3 : 7)));

			new_actor->lighting = LAMP_ON_SHADING;
		}
		else
		{
			NewState(new_actor, &s_barrier_transition);
			new_actor->temp3 = 0;
			new_actor->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
			new_actor->flags |= (FL_NEVERMARK | FL_NONMARK);
			new_actor->lighting = 0;
			BARRIER_STATE(new_actor) = bt_OFF;
			new_actor->temp1 = SPR_ELEC_POST4;
		}
		break;

	case en_vpost_barrier:
		NewState(new_actor, &s_vpost_barrier);
		if (OnOff)
		{
			new_actor->temp1 = SPR_VPOST8 - SPR_VPOST1;
		}
		break;


	case en_vspike_barrier:
		NewState(new_actor, &s_spike_barrier);
		if (OnOff)
		{
			new_actor->temp1 = SPR_VSPIKE8 - SPR_VSPIKE1;
		}
		break;

	default:
		break;
	}

}


// ---------------------------------------------------------------------------
// ToggleBarrier()
//
// OBJECT STATES:
//
// bt_ON -> bt_TURNING_OFF and think is changed to T_BarrierTrans
//
// ---------------------------------------------------------------------------

void TurnPostOff(
	objtype* obj)
{
	NewState(obj, &s_barrier_transition);
	obj->temp3 = 0;
	obj->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
	obj->flags |= (FL_NEVERMARK | FL_NONMARK);
	obj->lighting = 0;
	BARRIER_STATE(obj) = bt_OFF;

}

void TurnPostOn(
	objtype* obj)
{
	obj->flags |= (FL_SOLID | FL_FAKE_STATIC);
	obj->flags &= ~(FL_NEVERMARK | FL_NONMARK);
	obj->lighting = LAMP_ON_SHADING;
	BARRIER_STATE(obj) = bt_ON;
}

void ToggleBarrier(
	objtype* obj)
{
	const auto& assets_info = get_assets_info();

	switch (BARRIER_STATE(obj))
	{
	case bt_ON: // Same as closed
	case bt_CLOSING:
		//
		// Turn OFF/Open
		//

		switch (obj->obclass)
		{
		case post_barrierobj:
			obj->temp1 = SPR_ELEC_POST4;
			TurnPostOff(obj);
			break;

		case arc_barrierobj:
			obj->temp1 = SPR_ELEC_ARC4;
			TurnPostOff(obj);
			break;

		case vpost_barrierobj:
		case vspike_barrierobj:
			BARRIER_STATE(obj) = bt_OPENING;
			break;

		default:
			break;
		}

		break;

	case bt_OFF: // Same as open
	case bt_OPENING:
		//
		// Turn ON/Closed
		//

		switch (obj->obclass)
		{
		case post_barrierobj:
			InitSmartSpeedAnim(
				obj,
				SPR_ELEC_POST1,
				US_RndT() % 3,
				2,
				at_CYCLE,
				ad_FWD,
				(assets_info.is_ps() ? 3 : 20) + (US_RndT() & (assets_info.is_ps() ? 3 : 7)));

			TurnPostOn(obj);
			break;

		case arc_barrierobj:
			InitSmartSpeedAnim(
				obj,
				SPR_ELEC_ARC1,
				US_RndT() % 3,
				2,
				at_CYCLE,
				ad_FWD,
				(assets_info.is_ps() ? 3 : 20) + (US_RndT() & (assets_info.is_ps() ? 3 : 7)));

			TurnPostOn(obj);
			break;

		case vpost_barrierobj:
		case vspike_barrierobj:
			BARRIER_STATE(obj) = bt_CLOSING;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// T_BarrierShutdown() - This is used ONLY for electric arc barriers
//      which "flicker" out when disabled/destroyed.
//
// ---------------------------------------------------------------------------
void T_BarrierShutdown(
	objtype* obj)
{
	switch (BARRIER_STATE(obj))
	{
	case bt_DISABLING:
		if (obj->hitpoints)
		{
			if (obj->temp2 < tics)
			{
				if (obj->temp1 == SPR_ELEC_ARC4)
				{
					obj->temp1 = SPR_ELEC_ARC1 + obj->temp3;
					obj->temp3 = (obj->temp3 + 1) % 4;
					sd_play_actor_weapon_sound(ELECARCDAMAGESND, *obj);
					obj->flags |= (FL_SOLID | FL_FAKE_STATIC);
					obj->flags &= ~(FL_NEVERMARK | FL_NONMARK);
					obj->lighting = LAMP_ON_SHADING;
					obj->temp2 = US_RndT() & 0x7;
				}
				else
				{
					obj->temp1 = SPR_ELEC_ARC4;
					obj->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
					obj->flags |= (FL_NEVERMARK | FL_NONMARK);
					obj->lighting = 0;
					obj->temp2 = 5 + (US_RndT() & 0xf);
				}

				obj->hitpoints--;
			}
			else
			{
				obj->temp2 -= tics;
			}
		}
		else
		{
			BARRIER_STATE(obj) = bt_DISABLED;
		}
		break;

	case bt_DISABLED:
		obj->flags |= (FL_NEVERMARK | FL_NONMARK);
		obj->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
		obj->lighting = 0;
		if (obj->obclass == post_barrierobj)
		{
			obj->temp1 = SPR_ELEC_POST4;
		}
		else
		{
			obj->temp1 = SPR_ELEC_ARC4;
		}
		obj->temp3 = 0;
		NewState(obj, &s_ofs_smart_anim);
		break;

	}
}

void T_BarrierTransition(
	objtype* obj)
{
	switch (BARRIER_STATE(obj))
	{
			//
			// ON/CLOSED POSITION
			//
		case bt_ON:
			//
			// Check for cycle barrier.
			//
			if ((std::uint16_t)obj->temp2 == 0xFFFF)
			{
				if (obj->temp3 < tics)
				{
					ToggleBarrier(obj);
				}
				else
				{
					obj->temp3 -= tics;
				}
			}
			else
			{
				const auto barrier_index = gamestate.get_barrier_index(obj->temp2);

				if (!gamestate.barrier_table[barrier_index].on)
				{
					ToggleBarrier(obj);
				}
			}
			break;


			//
			// OFF/OPEN POSITION
			//
		case bt_OFF:
			//
			// Check for cycle barrier.
			//
			if ((std::uint16_t)obj->temp2 == 0xFFFF)
			{
				if (obj->temp3 < tics)
				{
					ToggleBarrier(obj);
				}
				else
				{
					obj->temp3 -= tics;
				}
			}
			else
			{
				const auto barrier_index = gamestate.get_barrier_index(obj->temp2);

				if (gamestate.barrier_table[barrier_index].on)
				{
					ToggleBarrier(obj);
				}
			}
			break;

			//
			// CLOSING/TURNING ON
			//
		case bt_CLOSING:
			// Check for damaging the player

			{
				const auto dx = std::abs(player->x - obj->x);
				const auto dy = std::abs(player->y - obj->y);

				constexpr auto check_distance = bstone::math::fixed_to_floating(0x18000);
				constexpr auto min_distance = bstone::math::fixed_to_floating(0x8000);

				if (dy <= check_distance && dx <= check_distance)
				{
					if (dy <= min_distance && dx <= min_distance)
					{
						TakeDamage(2, obj);
					}
					break;
				}
			}

			if (obj->temp3 < tics)
			{
				obj->temp3 = VPOST_BARRIER_SPEED;
				obj->temp1++;

				if (obj->temp1 > 7)
				{
					obj->temp1 = 7; // Errors...

				}
				switch (obj->temp1)
				{
				case 3:
					// Closed enough to be solid
					//

					obj->flags |= (FL_SOLID | FL_FAKE_STATIC);
					obj->flags &= ~(FL_NEVERMARK | FL_NONMARK);
					break;

				case 7:
					// Fully closed dude!
					//

					BARRIER_STATE(obj) = bt_ON;
					//
					// Check for cycle barrier.
					//
					if ((std::uint16_t)obj->temp2 == 0xFFFF)
					{
						obj->temp3 = VPOST_WAIT_DELAY;
					}
					break;
				}
			}
			else
			{
				obj->temp3 -= tics;
			}

			// Check to see if to was toggled

			if ((std::uint16_t)obj->temp2 != 0xFFFF)
			{
				const auto barrier_index = gamestate.get_barrier_index(obj->temp2);

				if (!gamestate.barrier_table[barrier_index].on)
				{
					ToggleBarrier(obj);
				}
			}
			break;

			//
			// OPENING/TURNNING OFF
			//
		case bt_OPENING:
			if (obj->temp3 < tics)
			{
				obj->temp3 = VPOST_BARRIER_SPEED;
				obj->temp1--;

				if (obj->temp1 < 0)
				{
					obj->temp1 = 0; // Errors...


				}
				switch (obj->temp1)
				{
					case 2:
						//
						// Open enough not to be solid.

						obj->flags &= ~(FL_SOLID | FL_FAKE_STATIC);
						obj->flags |= (FL_NEVERMARK | FL_NONMARK);
						break;

					case 0:
						// Fully Open dude!
						//

						BARRIER_STATE(obj) = bt_OFF;

						//
						// Check for cycle barrier.
						//
						if ((std::uint16_t)obj->temp2 == 0xFFFF)
						{
							obj->temp3 = VPOST_WAIT_DELAY;
						}
						break;
				}
			}
			else
			{
				obj->temp3 -= tics;
			}

			// Check to see if to was toggled

			if ((std::uint16_t)obj->temp2 != 0xFFFF)
			{
				const auto barrier_index = gamestate.get_barrier_index(obj->temp2);

				if (gamestate.barrier_table[barrier_index].on)
				{
					ToggleBarrier(obj);
				}
			}
			break;
	}
}


/*
=============================================================================

 GUARD

=============================================================================
*/

//
// Rent-A-Cop
//

extern statetype s_rent_stand;

extern statetype s_rent_path1;
extern statetype s_rent_path1s;
extern statetype s_rent_path2;
extern statetype s_rent_path3;
extern statetype s_rent_path3s;
extern statetype s_rent_path4;

extern statetype s_rent_pain;

extern statetype s_rent_giveup;

extern statetype s_rent_shoot1;
extern statetype s_rent_shoot2;
extern statetype s_rent_shoot3;
extern statetype s_rent_shoot4;

extern statetype s_rent_chase1;
extern statetype s_rent_chase1s;
extern statetype s_rent_chase2;
extern statetype s_rent_chase3;
extern statetype s_rent_chase3s;
extern statetype s_rent_chase4;

extern statetype s_rent_die1;
extern statetype s_rent_die1d;
extern statetype s_rent_die2;
extern statetype s_rent_die3;
extern statetype s_rent_die3s;
extern statetype s_rent_die4;

statetype s_rent_stand = {1, SPR_DEMO, 0, T_Stand, nullptr, &s_rent_stand};

statetype s_rent_path1 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_rent_path1s};
statetype s_rent_path1s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_rent_path2};
statetype s_rent_path2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_rent_path3};
statetype s_rent_path3 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_rent_path3s};
statetype s_rent_path3s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_rent_path4};
statetype s_rent_path4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_rent_path1};

statetype s_rent_pain = {2, SPR_DEMO, 15, nullptr, nullptr, &s_rent_chase1};

statetype s_rent_shoot1 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_rent_shoot2};
statetype s_rent_shoot2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_rent_shoot3};
statetype s_rent_shoot3 = {0, SPR_DEMO, 20, nullptr, T_Shade, &s_rent_chase1};

statetype s_rent_chase1 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_rent_chase1s};
statetype s_rent_chase1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_rent_chase2};
statetype s_rent_chase2 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_rent_chase3};
statetype s_rent_chase3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_rent_chase3s};
statetype s_rent_chase3s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_rent_chase4};
statetype s_rent_chase4 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_rent_chase1};

statetype s_rent_die1 = {0, SPR_DEMO, 17, T_BlowBack, A_DeathScream, &s_rent_die2};
statetype s_rent_die2 = {0, SPR_DEMO, 21, T_BlowBack, nullptr, &s_rent_die3};
statetype s_rent_die3 = {0, SPR_DEMO, 16, T_BlowBack, nullptr, &s_rent_die3s};
statetype s_rent_die3s = {0, SPR_DEMO, 13, T_BlowBack, nullptr, &s_rent_die4};
statetype s_rent_die4 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_rent_die4};


//
// officers
//

extern statetype s_ofcstand;

extern statetype s_ofcpath1;
extern statetype s_ofcpath1s;
extern statetype s_ofcpath2;
extern statetype s_ofcpath3;
extern statetype s_ofcpath3s;
extern statetype s_ofcpath4;

extern statetype s_ofcpain;

extern statetype s_ofcgiveup;

extern statetype s_ofcshoot1;
extern statetype s_ofcshoot2;
extern statetype s_ofcshoot3;
extern statetype s_ofcshoot4;

extern statetype s_ofcchase1;
extern statetype s_ofcchase1s;
extern statetype s_ofcchase2;
extern statetype s_ofcchase3;
extern statetype s_ofcchase3s;
extern statetype s_ofcchase4;

extern statetype s_ofcdie1;
extern statetype s_ofcdie2;
extern statetype s_ofcdie3;
extern statetype s_ofcdie4;
extern statetype s_ofcdie5;

statetype s_ofcstand = {1, SPR_DEMO, 0, T_Stand, nullptr, &s_ofcstand};

statetype s_ofcpath1 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_ofcpath1s};
statetype s_ofcpath1s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_ofcpath2};
statetype s_ofcpath2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_ofcpath3};
statetype s_ofcpath3 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_ofcpath3s};
statetype s_ofcpath3s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_ofcpath4};
statetype s_ofcpath4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_ofcpath1};

statetype s_ofcpain = {2, SPR_DEMO, 15, nullptr, nullptr, &s_ofcchase1};

statetype s_ofcshoot1 = {0, SPR_DEMO, 6, nullptr, nullptr, &s_ofcshoot2};
statetype s_ofcshoot2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_ofcshoot3};
statetype s_ofcshoot3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_ofcchase1};

statetype s_ofcchase1 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_ofcchase1s};
statetype s_ofcchase1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_ofcchase2};
statetype s_ofcchase2 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_ofcchase3};
statetype s_ofcchase3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_ofcchase3s};
statetype s_ofcchase3s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_ofcchase4};
statetype s_ofcchase4 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_ofcchase1};

statetype s_ofcdie1 = {0, SPR_DEMO, 15, T_BlowBack, A_DeathScream, &s_ofcdie2};
statetype s_ofcdie2 = {0, SPR_DEMO, 15, T_BlowBack, nullptr, &s_ofcdie3};
statetype s_ofcdie3 = {0, SPR_DEMO, 15, T_BlowBack, nullptr, &s_ofcdie4};
statetype s_ofcdie4 = {0, SPR_DEMO, 15, T_BlowBack, nullptr, &s_ofcdie5};
statetype s_ofcdie5 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_ofcdie5};


//
// SWAT
//

extern statetype s_swatstand;

extern statetype s_swatpath1;
extern statetype s_swatpath1s;
extern statetype s_swatpath2;
extern statetype s_swatpath3;
extern statetype s_swatpath3s;
extern statetype s_swatpath4;

extern statetype s_swatpain;

extern statetype s_swatgiveup;

extern statetype s_swatshoot1;
extern statetype s_swatshoot2;
extern statetype s_swatshoot3;
extern statetype s_swatshoot4;
extern statetype s_swatshoot5;
extern statetype s_swatshoot6;
extern statetype s_swatshoot7;

extern statetype s_swatchase1;
extern statetype s_swatchase1s;
extern statetype s_swatchase2;
extern statetype s_swatchase3;
extern statetype s_swatchase3s;
extern statetype s_swatchase4;

extern statetype s_swatwounded1;
extern statetype s_swatwounded2;
extern statetype s_swatwounded3;
extern statetype s_swatwounded4;

extern statetype s_swatunwounded1;
extern statetype s_swatunwounded2;
extern statetype s_swatunwounded3;
extern statetype s_swatunwounded4;

extern statetype s_swatdie1;
extern statetype s_swatdie2;
extern statetype s_swatdie3;
extern statetype s_swatdie4;
extern statetype s_swatdie5;


statetype s_swatstand = {1, SPR_DEMO, 0, T_Stand, nullptr, &s_swatstand};

statetype s_swatpath1 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_swatpath1s};
statetype s_swatpath1s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_swatpath2};
statetype s_swatpath2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_swatpath3};
statetype s_swatpath3 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_swatpath3s};
statetype s_swatpath3s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_swatpath4};
statetype s_swatpath4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_swatpath1};

statetype s_swatpain = {2, SPR_DEMO, 15, nullptr, nullptr, &s_swatshoot1};

statetype s_swatshoot1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatshoot2};
statetype s_swatshoot2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_swatshoot3};
statetype s_swatshoot3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_swatshoot4};
statetype s_swatshoot4 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_swatshoot5};
statetype s_swatshoot5 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_swatshoot6};
statetype s_swatshoot6 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_swatshoot7};
statetype s_swatshoot7 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_swatchase1};

statetype s_swatchase1 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_swatchase1s};
statetype s_swatchase1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_swatchase2};
statetype s_swatchase2 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_swatchase3};
statetype s_swatchase3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_swatchase3s};
statetype s_swatchase3s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_swatchase4};
statetype s_swatchase4 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_swatchase1};

statetype s_swatwounded1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatwounded2};
statetype s_swatwounded2 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatwounded3};
statetype s_swatwounded3 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatwounded4};
statetype s_swatwounded4 = {0, SPR_DEMO, 10, T_SwatWound, nullptr, &s_swatwounded4};

statetype s_swatunwounded1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatunwounded2};
statetype s_swatunwounded2 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatunwounded3};
statetype s_swatunwounded3 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_swatunwounded4};
statetype s_swatunwounded4 = {0, SPR_DEMO, 10, nullptr, T_SwatWound, &s_swatchase1};

statetype s_swatdie1 = {0, SPR_DEMO, 20, T_BlowBack, A_DeathScream, &s_swatdie2};
statetype s_swatdie2 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_swatdie3};
statetype s_swatdie3 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_swatdie4};
statetype s_swatdie4 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_swatdie5};
statetype s_swatdie5 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_swatdie5};


//
// PRO Guard
//

extern statetype s_prostand;

extern statetype s_propath1;
extern statetype s_propath1s;
extern statetype s_propath2;
extern statetype s_propath3;
extern statetype s_propath3s;
extern statetype s_propath4;

extern statetype s_propain;

extern statetype s_proshoot1;
extern statetype s_proshoot2;
extern statetype s_proshoot3;
extern statetype s_proshoot4;
extern statetype s_proshoot5;
extern statetype s_proshoot6;
extern statetype s_proshoot6a;

extern statetype s_prochase1;
extern statetype s_prochase1s;
extern statetype s_prochase2;
extern statetype s_prochase3;
extern statetype s_prochase3s;
extern statetype s_prochase4;

extern statetype s_prodie1;
extern statetype s_prodie2;
extern statetype s_prodie3;
extern statetype s_prodie3a;
extern statetype s_prodie4;

statetype s_prostand = {1, SPR_DEMO, 0, T_Stand, nullptr, &s_prostand};

statetype s_propath1 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_propath1s};
statetype s_propath1s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_propath2};
statetype s_propath2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_propath3};
statetype s_propath3 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_propath3s};
statetype s_propath3s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_propath4};
statetype s_propath4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_propath1};

statetype s_propain = {2, SPR_DEMO, 15, nullptr, nullptr, &s_prochase1};

statetype s_proshoot1 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_proshoot2};
statetype s_proshoot2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_proshoot3};
statetype s_proshoot3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_proshoot4};
statetype s_proshoot4 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_proshoot5};
statetype s_proshoot5 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_proshoot6};
statetype s_proshoot6 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_proshoot6a};
statetype s_proshoot6a = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_prochase1};

statetype s_prochase1 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_prochase1s};
statetype s_prochase1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_prochase2};
statetype s_prochase2 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_prochase3};
statetype s_prochase3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_prochase3s};
statetype s_prochase3s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_prochase4};
statetype s_prochase4 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_prochase1};

statetype s_prodie1 = {0, SPR_DEMO, 20, T_BlowBack, A_DeathScream, &s_prodie2};
statetype s_prodie2 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_prodie3};
statetype s_prodie3 = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_prodie3a};
statetype s_prodie3a = {0, SPR_DEMO, 20, T_BlowBack, nullptr, &s_prodie4};
statetype s_prodie4 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_prodie4};

extern statetype s_electro_appear1;
extern statetype s_electro_appear2;
extern statetype s_electro_appear3;

extern statetype s_electro_chase1;
extern statetype s_electro_chase2;
extern statetype s_electro_chase3;
extern statetype s_electro_chase4;

extern statetype s_electro_ouch;

extern statetype s_electro_shoot1;
extern statetype s_electro_shoot2;
extern statetype s_electro_shoot3;

extern statetype s_electro_shot1;
extern statetype s_electro_shot2;

extern statetype s_electro_shot_exp1;
extern statetype s_electro_shot_exp2;

extern statetype s_ofs_shot1;
extern statetype s_ofs_shot2;

extern statetype s_ofs_shot_exp1;
extern statetype s_ofs_shot_exp2;

extern statetype s_electro_die1;
extern statetype s_electro_die2;
extern statetype s_electro_die3;

statetype s_electro_appear1 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_appear2};
statetype s_electro_appear2 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_appear3};
statetype s_electro_appear3 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_chase1};

statetype s_electro_chase1 = {0, SPR_DEMO, 14, T_Chase, nullptr, &s_electro_chase2};
statetype s_electro_chase2 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_chase3};
statetype s_electro_chase3 = {0, SPR_DEMO, 14, T_Chase, nullptr, &s_electro_chase4};
statetype s_electro_chase4 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_chase1};

statetype s_electro_ouch = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_chase1};

statetype s_electro_shoot1 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_shoot2};
statetype s_electro_shoot2 = {0, SPR_DEMO, 14, T_Shoot, nullptr, &s_electro_shoot3};
statetype s_electro_shoot3 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_chase1};

statetype s_electro_shot1 = {0, SPR_DEMO, 1, T_Projectile, nullptr, &s_electro_shot2};
statetype s_electro_shot2 = {0, SPR_DEMO, 1, T_Projectile, nullptr, &s_electro_shot1};

statetype s_ofs_shot1 = {0, 0, 1, T_Projectile, nullptr, &s_ofs_shot2};
statetype s_ofs_shot2 = {0, 1, 1, T_Projectile, nullptr, &s_ofs_shot1};


statetype s_electro_die1 = {0, SPR_DEMO, 14, nullptr, A_DeathScream, &s_electro_die2};
statetype s_electro_die2 = {0, SPR_DEMO, 14, nullptr, nullptr, &s_electro_die3};
statetype s_electro_die3 = {0, SPR_DEMO, 14, nullptr, nullptr, nullptr};


extern statetype s_liquid_wait;
extern statetype s_liquid_move;

extern statetype s_liquid_rise1;
extern statetype s_liquid_rise2;
extern statetype s_liquid_rise3;
extern statetype s_liquid_rise4;

extern statetype s_liquid_stand;

extern statetype s_liquid_fall1;
extern statetype s_liquid_fall2;
extern statetype s_liquid_fall3;
extern statetype s_liquid_fall4;

extern statetype s_liquid_shoot1;
extern statetype s_liquid_shoot2;
extern statetype s_liquid_shoot3;

extern statetype s_liquid_ouch;

extern statetype s_liquid_die1;
extern statetype s_liquid_die2;
extern statetype s_liquid_die3;
extern statetype s_liquid_die4;
extern statetype s_liquid_dead;

extern statetype s_blake1;
extern statetype s_blake2;
extern statetype s_blake3;
extern statetype s_blake4;

extern statetype s_liquid_shot;

void T_LiquidMove(
	objtype* obj);

void T_Solid(
	objtype* obj);


statetype s_liquid_wait = {0, SPR_DEMO, 14, T_Stand, nullptr, &s_liquid_wait};

statetype s_liquid_move = {0, SPR_DEMO, 14, T_LiquidMove, T_ChangeShape, &s_liquid_move};

statetype s_liquid_rise1 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_liquid_rise2};
statetype s_liquid_rise2 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_liquid_rise3};
statetype s_liquid_rise3 = {0, SPR_DEMO, 12, nullptr, T_Solid, &s_liquid_shoot1};

statetype s_liquid_stand = {0, SPR_DEMO, 40, T_LiquidStand, nullptr, &s_liquid_stand};

statetype s_liquid_fall1 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_liquid_fall2};
statetype s_liquid_fall2 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_liquid_fall3};
statetype s_liquid_fall3 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_liquid_move};

statetype s_liquid_shoot1 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_liquid_shoot2};
statetype s_liquid_shoot2 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_liquid_shoot3};
statetype s_liquid_shoot3 = {0, SPR_DEMO, 12, nullptr, T_Shoot, &s_liquid_stand};

statetype s_liquid_ouch = {0, SPR_DEMO, 7, nullptr, nullptr, &s_liquid_shoot1};

statetype s_liquid_die1 = {0, SPR_DEMO, 20, nullptr, A_DeathScream, &s_liquid_die2};
statetype s_liquid_die2 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_liquid_die3};
statetype s_liquid_die3 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_liquid_die4};
statetype s_liquid_die4 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_liquid_dead};
statetype s_liquid_dead = {0, SPR_DEMO, 20, nullptr, nullptr, nullptr};

statetype s_liquid_shot = {0, 0, 10, T_Projectile, T_ChangeShape, &s_liquid_shot};

statetype s_blake1 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_blake2};
statetype s_blake2 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_blake3};
statetype s_blake3 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_blake4};
statetype s_blake4 = {0, SPR_DEMO, 12, nullptr, nullptr, &s_blake1};


void T_ChangeShape(
	objtype* obj)
{
	obj->temp1 = obj->temp2 + (US_RndT() % 3);
}

void T_MakeOffset(
	objtype* obj)
{
	obj->flags |= FL_OFFSET_STATES;
	obj->flags &= ~(FL_SOLID | FL_SHOOTABLE);
}

void T_Solid(
	objtype* obj)
{
	obj->flags |= (FL_SOLID | FL_SHOOTABLE);
}

void T_LiquidMove(
	objtype* obj)
{
	std::int16_t dx, dy, dist;


	//
	// Check to see if the Liquid Obj is VERY Close - Then FORCE up and
	// start firing...
	//

	dx = static_cast<std::int16_t>(abs(obj->tilex - player->tilex));
	dy = static_cast<std::int16_t>(abs(obj->tiley - player->tiley));
	dist = dx > dy ? dx : dy;

	if (dist < 6 && dx > 1 && dy > 1)
	{
		obj->flags &= ~(FL_OFFSET_STATES);
		obj->flags |= FL_SOLID;
		NewState(obj, &s_liquid_rise1);
	}
	else
	{
		T_Chase(obj);
	}
}

void T_LiquidStand(
	objtype* obj)
{
	std::int16_t dx, dy;

	obj->flags |= FL_SHOOTABLE | FL_SOLID;

	if (US_RndT() < 80 && obj->temp2 < 5)
	{
		obj->temp2++;
		NewState(obj, &s_liquid_shoot1);
	}
	else
	{
		dx = static_cast<std::int16_t>(abs(obj->tilex - player->tilex));
		dy = static_cast<std::int16_t>(abs(obj->tiley - player->tiley));

		if (dx > 1 || dy > 1)
		{
			if (!(obj->flags & FL_VISIBLE) || (US_RndT() < 40) || (obj->temp2 == 5))
			{
				NewState(obj, &s_liquid_fall1);
				obj->flags |= FL_OFFSET_STATES;
				obj->flags &= ~(FL_SOLID | FL_SHOOTABLE);
				obj->temp2 = 0;
			}
		}
		else
		{
			obj->temp2 = 0;
		}
	}
}

void T_SwatWound(
	objtype* ob)
{
	if (ob->state == &s_swatunwounded4)
	{
		ob->flags |= FL_SOLID | FL_SHOOTABLE;
	}
	else
	{
		if (ob->temp2)
		{
			if (tics < ob->temp2)
			{
				ob->temp2 -= tics;
				return;
			}
		}

		ob->temp2 = 0;

		const auto dx = std::abs(player->x - ob->x);
		const auto dy = std::abs(player->y - ob->y);

		if (dy > 1.0 || dx > 1.0)
		{
			ob->flags |= FL_SOLID | FL_SHOOTABLE;
			NewState(ob, &s_swatunwounded1);
		}
	}
}

void SpawnStand(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir)
{
	const auto& assets_info = get_assets_info();

	std::uint16_t ammo = 8;

	switch (which)
	{
	case en_goldstern:
		SpawnNewObj(tilex, tiley, &s_goldwarp_in1);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL;
		if (gamestate.mapon == 9)
		{
			new_actor->hitpoints = get_start_hit_point(which) * 15;
		}
		ammo = 25;
		break;

	case en_electro_alien:
		SpawnNewObj(tilex, tiley, &s_electro_appear1);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID | FL_PROJ_TRANSPARENT;
		new_actor->speed = SPDPATROL;
		new_actor->lighting = NO_SHADING; // no shading
		break;

	case en_liquid:
		SpawnNewObj(tilex, tiley, &s_liquid_wait);
		new_actor->flags = FL_OFFSET_STATES | FL_PROJ_TRANSPARENT;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL * 3;
		break;

	case en_rentacop:
		SpawnNewObj(tilex, tiley, &s_rent_stand);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL;
		break;

	case en_gen_scientist:
		SpawnNewObj(tilex, tiley, &s_ofcstand);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID | FL_FRIENDLY | FL_RANDOM_TURN;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);

		if (US_RndT() & 1)
		{
			new_actor->flags |= FL_INFORMANT;
		}
		new_actor->speed = SPDPATROL;
		break;

	case en_swat:
		SpawnNewObj(tilex, tiley, &s_swatstand);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL;
		ammo = 30;
		if (scan_value == 0xffff)
		{
			new_actor->temp1 = US_RndT() & 1;
		}
		else
		{
			new_actor->temp1 = scan_value;
		}
		break;

	case en_proguard:
		SpawnNewObj(tilex, tiley, &s_prostand);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL;
		ammo = 25;
		break;

	case en_hang_terrot:
		SpawnNewObj(tilex, tiley, &s_terrot_wait);
		new_actor->flags = FL_SHOOTABLE | FL_NONMARK | FL_NEVERMARK;
		new_actor->speed = SPDPATROL;
		break;

	case en_floatingbomb:
		SpawnNewObj(tilex, tiley, &s_scout_stand);
		new_actor->speed = SPDPATROL;
		new_actor->temp1 = SPR_FSCOUT_W1_1;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		new_actor->flags = FL_SHOOTABLE | FL_SOLID | FL_OFFSET_STATES | FL_FAKE_STATIC;
		break;

	case en_volatiletransport:
		SpawnNewObj(tilex, tiley, &s_scout_stand);
		new_actor->speed = SPDPATROL;
		new_actor->temp1 = SPR_GSCOUT_W1_1;
		new_actor->flags = FL_SHOOTABLE | FL_SOLID | FL_OFFSET_STATES | FL_STATIONARY | FL_FAKE_STATIC;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_steamgrate:
		SpawnNewObj(tilex, tiley, &s_steamgrate);
		new_actor->flags = FL_OFFSET_STATES;
		new_actor->temp1 = SPR_GRATE;
		new_actor->temp2 = 60 * 4;
		break;

	case en_steampipe:
		nevermark = true;
		SpawnNewObj(tilex, tiley, &s_steamgrate);
		nevermark = false;
		new_actor->flags = FL_OFFSET_STATES | FL_NONMARK | FL_NEVERMARK;
		new_actor->temp1 = SPR_STEAM_PIPE;
		new_actor->temp2 = 60 * 4;
		break;

	default:
		break;
	}

	CheckForSpecialTile(new_actor, tilex, tiley);

	new_actor->ammo = static_cast<std::uint8_t>(ammo);
	new_actor->obclass = static_cast<classtype>(rentacopobj + which);
	new_actor->hitpoints += get_start_hit_point(which);
	new_actor->dir = static_cast<dirtype>(dir << 1);

	if (new_actor->flags & FL_INFORMANT)
	{
		new_actor->hitpoints = 1;
		new_actor->ammo = 0;
		new_actor->flags |= FL_HAS_AMMO | FL_HAS_TOKENS;
		new_actor->s_tilex = new_actor->s_tiley = 0xff;
	}
}

// ---------------------------------------------------------------------------
// CheckForSpecialTile() - Adds special attributes to actor if standing on
//      special tiles.
// ---------------------------------------------------------------------------
void CheckForSpecialTile(
	objtype* obj,
	std::uint16_t tilex,
	std::uint16_t tiley)
{
	std::uint16_t* map, *map1;
	objtype* old_new;
	bool getarea = false;

	//
	// Only shootables can use special tiles...
	//
	// (This also tests to make sure that the plasma_detonatorobj &
	//  plasma_detonator_reservedobj will not enter this function.)
	//

	if (!(obj->flags & FL_SHOOTABLE))
	{
		return;
	}

	//
	// Check and handle special tiles... Only one per actor... now!
	//

	const auto& assets_info = get_assets_info();

	map = &mapsegs[0][farmapylookup[tiley] + tilex];

	switch (*map)
	{
	case CLOAK_AMBUSH_TILE:
		if (!assets_info.is_ps())
		{
			break;
		}

		obj->flags2 |= FL2_CLOAKED;

	case AMBUSHTILE:
		obj->flags |= FL_AMBUSH | FL_SHOOTABLE | FL_SOLID;
		getarea = true;
		break;

	case DETONATOR_TILE:
		if (!assets_info.is_ps())
		{
			break;
		}

		old_new = new_actor;
		SpawnHiddenOfs(en_plasma_detonator_reserve, tilex, tiley);
		new_actor = old_new;
		obj->flags &= ~FL_INFORMANT;
	case RKEY_TILE:
	case YKEY_TILE:
	case BKEY_TILE:
	case BFG_TILE:
	case ION_TILE:
		if (!assets_info.is_ps())
		{
			break;
		}

		ReserveStatic();
		obj->flags2 |= SpecialSpawnFlags[(*map) - RKEY_TILE];
		getarea = true;
		break;

	case CLOAK_TILE:
		if (!assets_info.is_ps())
		{
			break;
		}

		obj->flags2 |= FL2_CLOAKED;
		getarea = true;
		break;

	case LINC_TILE:
		if (!assets_info.is_ps())
		{
			break;
		}

		obj->flags2 |= FL2_LINC;
		obj->flags &= ~FL_INFORMANT; // Make sure informants dont have links
		getarea = true;
		map1 = &mapsegs[1][farmapylookup[tiley] + tilex + 1];
		obj->linc = *map1;
		*map1 = 0;
		break;
	}

	//
	// Init areanumbers and tilemaps...
	//

	if (getarea)
	{
		tilemap[tilex][tiley] = 0;
		*map = obj->areanumber = GetAreaNumber(static_cast<std::int8_t>(tilex), static_cast<std::int8_t>(tiley));
	}
}

void SpawnPatrol(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir)
{
	const auto& assets_info = get_assets_info();

	std::int16_t ammo = 8;

	switch (which)
	{
	case en_blake:
		SpawnNewObj(tilex, tiley, &s_blake1);
		new_actor->speed = SPDPATROL * 2;
		break;

	case en_rentacop:
		SpawnNewObj(tilex, tiley, &s_rent_path1);
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		new_actor->speed = SPDPATROL;
		break;

	case en_gen_scientist:
		SpawnNewObj(tilex, tiley, &s_ofcpath1);
		new_actor->flags = FL_FRIENDLY | FL_RANDOM_TURN;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);

		if (US_RndT() & 1)
		{
			new_actor->flags |= FL_INFORMANT;
		}
		new_actor->speed = SPDPATROL;
		break;

	case en_proguard:
		SpawnNewObj(tilex, tiley, &s_propath1);
		new_actor->speed = SPDPATROL;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		ammo = 25;
		break;

	case en_swat:
		SpawnNewObj(tilex, tiley, &s_swatpath1);
		new_actor->speed = SPDPATROL;
		ammo = 30;
		if (scan_value == 0xffff)
		{
			new_actor->temp1 = US_RndT() & 1;
		}
		else
		{
			new_actor->temp1 = scan_value;
		}
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFG_SHOOTABLE : 0);
		break;

	case en_floatingbomb:
		SpawnNewObj(tilex, tiley, &s_scout_path1);
		new_actor->speed = SPDPATROL;
		new_actor->temp1 = SPR_FSCOUT_W1_1;
		new_actor->flags = FL_OFFSET_STATES;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	case en_volatiletransport:
		SpawnNewObj(tilex, tiley, &s_scout_path1);
		new_actor->speed = SPDPATROL;
		new_actor->temp1 = SPR_GSCOUT_W1_1;
		new_actor->flags = FL_OFFSET_STATES;
		new_actor->flags2 = (assets_info.is_ps() ? FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE : 0);
		break;

	default:
		break;
	}

	new_actor->ammo = static_cast<std::uint8_t>(ammo);
	new_actor->obclass = static_cast<classtype>(rentacopobj + which);
	new_actor->dir = static_cast<dirtype>(dir << 1);
	new_actor->hitpoints = get_start_hit_point(which);
	new_actor->distance = 0.0;
	if (new_actor->obclass != blakeobj)
	{
		new_actor->flags |= FL_SHOOTABLE | FL_SOLID;
	}
	new_actor->active = ac_yes;
	if (new_actor->flags & FL_INFORMANT)
	{
		new_actor->hitpoints = 1;
		new_actor->ammo = 0;
		new_actor->flags |= FL_HAS_AMMO | FL_HAS_TOKENS;
		new_actor->s_tilex = new_actor->s_tiley = 0xff;
	}

	CheckForSpecialTile(new_actor, tilex, tiley);

	actorat[new_actor->tilex][new_actor->tiley] = nullptr; // don't use original spot

	TryWalk(new_actor, true);

	actorat[new_actor->tilex][new_actor->tiley] = new_actor;
}

void A_DeathScream(
	objtype* ob)
{
	const auto& assets_info = get_assets_info();

	switch (ob->obclass)
	{
	case swatobj:
	{
		const int sounds[] = {
			SWATDIESND,
			assets_info.is_ps() ? SWATDEATH2SND : SWATDEATH3SND,
		}; // sounds

		auto sound_index = (assets_info.is_aog_sw() ? 0 : (US_RndT() % 2));
		sd_play_actor_voice_sound(sounds[sound_index], *ob);
		break;
	}

	case rentacopobj:
	{
		const int sounds[] = {
			RENTDEATH1SND,
			RENTDEATH2SND,
		}; // sounds

		auto sound_index = (assets_info.is_aog_sw() ? 0 : (US_RndT() % 2));
		sd_play_actor_voice_sound(sounds[sound_index], *ob);
		break;
	}

	case mutant_human1obj:
	case hang_terrotobj:
	case floatingbombobj:
	case volatiletransportobj:
	case explosionobj:
	case gr_explosionobj:
	case bfg_explosionobj:
	case pd_explosionobj:
	case doorexplodeobj:
	{
		const int sounds[] = {
			EXPLODE1SND,
			EXPLODE2SND,
		}; // sounds

		auto sound_index = (assets_info.is_aog_sw() ? 0 : (US_RndT() % 2));
		sd_play_actor_voice_sound(sounds[sound_index], *ob);
		break;
	}

	case rotating_cubeobj:
		sd_play_actor_voice_sound(EXPLODE1SND, *ob);
		sd_play_player_item_sound(VITAL_GONESND);
		break;

	case gen_scientistobj:
	{
		auto is_informant = ((ob->flags & FL_INFORMANT) != 0);

		const int sounds[] = {
			is_informant ? INFORMANTDEATHSND : SCIENTISTDEATHSND,
			is_informant ? INFORMDEATH2SND : SCIDEATH2SND,
			is_informant ? INFORMDEATH3SND : SCIDEATH3SND,
		}; // sounds

		auto sound_index = assets_info.is_aog_sw() ? 0 : (US_RndT() % 3);
		sd_play_actor_voice_sound(sounds[sound_index], *ob);
		break;
	}

	case genetic_guardobj:
		if (assets_info.is_aog())
		{
			sd_play_actor_voice_sound(GGUARDDEATHSND, *ob);
			break;
		}

	case breather_beastobj:
	case cyborg_warriorobj:
	case acid_dragonobj:
	case podobj:
		sd_play_actor_voice_sound(PODDEATHSND, *ob);
		break;

	case liquidobj:
		sd_play_actor_voice_sound(LIQUIDDIESND, *ob);
		break;

	case proguardobj:
	{
		const int sounds[3] = {
			PROGUARDDEATHSND,
			PRODEATH2SND,
		}; // sounds

		auto sound_index = (assets_info.is_aog_sw() ? 0 : (US_RndT() % 2));
		sd_play_actor_voice_sound(sounds[sound_index], *ob);
		break;
	}

	case final_boss1obj:
	case spider_mutantobj:
		sd_play_actor_voice_sound(BLUEBOYDEATHSND, *ob);
		break;

	case mech_guardianobj:
	case final_boss3obj:
	case mutant_human2obj:
		sd_play_actor_voice_sound(DOGBOYDEATHSND, *ob);
		break;

	case reptilian_warriorobj:
	case scan_alienobj:
		sd_play_actor_voice_sound(SCANDEATHSND, *ob);
		break;

	case lcan_alienobj:
	case final_boss4obj:
		sd_play_actor_voice_sound(LCANDEATHSND, *ob);
		break;

	case gurneyobj:
		sd_play_actor_voice_sound(GURNEYDEATHSND, *ob);
		break;

	case lcan_wait_alienobj:
		sd_play_actor_voice_sound(LCANBREAKSND, *ob);
		break;

	case scan_wait_alienobj:
		sd_play_actor_voice_sound(SCANBREAKSND, *ob);
		break;

	default:
		break;

	}
}


// ============================================================================
//
// DROP
//
// ============================================================================

void DropCargo(
	objtype* obj)
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		return;
	}

	//
	// Keep separate... May later have MULTI "cargo's"
	//

	if (obj->flags2 & FL2_DROP_RKEY)
	{
		PlaceReservedItemNearTile(bo_red_key, obj->tilex, obj->tiley);
	}

	if (obj->flags2 & FL2_DROP_YKEY)
	{
		PlaceReservedItemNearTile(bo_yellow_key, obj->tilex, obj->tiley);
	}

	if (obj->flags2 & FL2_DROP_BKEY)
	{
		PlaceReservedItemNearTile(bo_blue_key, obj->tilex, obj->tiley);
	}

	if (obj->flags2 & FL2_DROP_BFG)
	{
		PlaceReservedItemNearTile(bo_bfg_cannon, obj->tilex, obj->tiley);
	}

	if (obj->flags2 & FL2_DROP_ION)
	{
		PlaceReservedItemNearTile(bo_ion_cannon, obj->tilex, obj->tiley);
	}

	if (obj->flags2 & FL2_DROP_DETONATOR)
	{
		PlaceReservedItemNearTile(bo_plasma_detonator, obj->tilex, obj->tiley);
	}

	if ((obj->flags2 & FL2_LINC) && obj->linc)
	{
		OperateSmartSwitch(obj->linc >> 8, obj->linc & 255, ST_TURN_OFF, true);
	}
}


/*
============================================================================

 STAND

============================================================================
*/

void T_Stand(
	objtype* ob)
{
	SightPlayer(ob);
}


/*
============================================================================

 CHASE

============================================================================
*/

void T_Chase(
	objtype* ob)
{
	ob->flags &= ~FL_LOCKED_STATE;

	if ((ob->flags & (FL_STATIONARY | FL_BARRIER_DAMAGE)) != 0)
	{
		return;
	}

	if (ob->ammo != 0)
	{
		if (CheckLine(ob, player) && (!PlayerInvisable))
		{
			auto nearattack = false;

			// got a shot at player?
			const auto dx = abs(ob->tilex - player->tilex);
			const auto dy = abs(ob->tiley - player->tiley);

			auto dist = dx > dy ? dx : dy;

			if (dist == 0)
			{
				dist = 1;
			}

			constexpr auto near_attack_min_distance = bstone::math::fixed_to_floating(0x4000);

			if (dist == 1 && ob->distance < near_attack_min_distance)
			{
				nearattack = true;
			}

			// Time to toggle SHOOTMODE?
			//
			switch (ob->obclass)
			{
				case mutant_human1obj:
				case genetic_guardobj:
				case gurneyobj:
				case podobj:
				case mutant_human2obj:
				case scan_alienobj:
				case lcan_alienobj:
				case spider_mutantobj:
				case breather_beastobj:
				case cyborg_warriorobj:
				case reptilian_warriorobj:
				case acid_dragonobj:
				case mech_guardianobj:
				case gold_morphobj:
				case final_boss1obj:
				case final_boss2obj:
				case final_boss3obj:
				case final_boss4obj:
					// Check for mode change
					//
					if (ob->ammo > tics)
					{
						ob->ammo -= static_cast<std::uint8_t>(tics);
					}
					else
					{
						ChangeShootMode(ob);

						// Move half as far when doing near attacks...
						//
						if ((ob->flags & FL_SHOOTMODE) == 0)
						{
							ob->ammo >>= 1;
						}
					}
					break;

				default:
					break;
			}

			auto chance = 0;

			if (nearattack)
			{
				// Always shoot when in SHOOTMODE -- Never shoot when not!
				//
				if ((ob->flags & FL_SHOOTMODE) != 0)
				{
					chance = 300;
				}
				else
				{
					chance = 0;
				}
			}
			else
			{
				switch (ob->obclass)
				{
					case mutant_human1obj:
					case genetic_guardobj:
					case gurneyobj:
					case podobj:
					case mutant_human2obj:
					case scan_alienobj:
					case lcan_alienobj:
					case spider_mutantobj:
					case breather_beastobj:
					case cyborg_warriorobj:
					case reptilian_warriorobj:
					case acid_dragonobj:
					case mech_guardianobj:
					case gold_morphobj:
					case final_boss1obj:
					case final_boss2obj:
					case final_boss3obj:
					case final_boss4obj:
						// Always shoot when in SHOOTMODE -- Never shoot when not!
						//
						if ((ob->flags & FL_SHOOTMODE) != 0)
						{
							chance = 300;
						}
						else
						{
							chance = 0;
						}
						break;

					default:
						chance = (tics << 4) / dist;
						break;
				}
			}

			if (US_RndT() < chance && ob->ammo != 0 && (ob->flags & FL_INTERROGATED) == 0)
			{
				DoAttack(ob);
				return;
			}
		}
		else
		{
			ChangeShootMode(ob);
		}
	}

	if (ob->dir == nodir)
	{
		switch (ob->obclass)
		{
			case floatingbombobj:
				SelectChaseDir(ob);
				break;

			default:
				SelectDodgeDir(ob);
				break;
		}

		if (ob->dir == nodir)
		{
			return; // object is blocked in
		}
	}

	auto move = ob->speed * tics;

	while (move != 0.0)
	{
		if (ob->distance < 0.0)
		{
			//
			// waiting for a door to open
			//
			const auto door_index = static_cast<std::int16_t>(-static_cast<int>(ob->distance) - 1);

			OpenDoor(door_index);

			if (doorobjlist[door_index].action != dr_open)
			{
				return;
			}

			ob->distance = 1.0; // go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj(ob, move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ob->tilex + 0.5;
		ob->y = ob->tiley + 0.5;

		move -= ob->distance;

		switch (ob->obclass)
		{
			case floatingbombobj:
				SelectChaseDir(ob);
				break;

			default:
				SelectDodgeDir(ob);
				break;
		}

		if (ob->dir == nodir)
		{
			return; // object is blocked in
		}
	}
}

void ChangeShootMode(
	objtype* ob)
{
	if (ob->flags & FL_SHOOTMODE)
	{
		ob->flags &= ~FL_SHOOTMODE;
		ob->ammo = 60 + (US_RndT() % 60);
	}
	else
	{
		ob->flags |= FL_SHOOTMODE;
		ob->ammo = 1 + (US_RndT() % 2);

		const auto& assets_info = get_assets_info();

		if (assets_info.is_ps() && ob->obclass == gold_morphobj)
		{
			ob->ammo += 3 + (US_RndT() % 5);
		}
	}
}

void DoAttack(
	objtype* ob)
{
	std::int16_t dx, dy, dist;

	dx = static_cast<std::int16_t>(abs(ob->tilex - player->tilex));
	dy = static_cast<std::int16_t>(abs(ob->tiley - player->tiley));
	dist = dx > dy ? dx : dy;
	if (!dist)
	{
		dist = 1;
	}

	switch (ob->obclass)
	{
	case floatingbombobj:
		if (dist <= 1)
		{
			ob->flags &= ~(FL_SHOOTABLE | FL_SOLID);
			ob->flags |= FL_NONMARK | FL_DEADGUY;
			KillActor(ob);
			return;
		}
		break;

	case goldsternobj:
		NewState(ob, &s_goldshoot1);
		break;

	case gold_morphobj:
		NewState(ob, &s_mgold_shoot1);
		break;

	case rentacopobj:
		NewState(ob, &s_rent_shoot1);
		break;

	case gen_scientistobj:
		NewState(ob, &s_ofcshoot1);
		break;

	case swatobj:
		NewState(ob, &s_swatshoot1);
		break;

	case liquidobj:
		if ((dx > 2 || dy > 2) && US_RndT() < 30)
		{
			ob->flags &= ~(FL_OFFSET_STATES);
			ob->flags |= FL_SOLID;
			NewState(ob, &s_liquid_rise1);
		}
		break;

	case proguardobj:
		NewState(ob, &s_proshoot1);
		break;

	case electroobj:
		NewState(ob, &s_electro_shoot1);
		break;

	case podobj:
		if (dist > CLOSE_RANGE)
		{
			NewState(ob, &s_ofs_pod_spit1);
		}
		else
		{
			NewState(ob, &s_ofs_pod_attack1);
		}
		break;

	case spider_mutantobj:
	case acid_dragonobj:
	case mech_guardianobj:
	case breather_beastobj:
	case cyborg_warriorobj:
	case reptilian_warriorobj:
	case gurneyobj:
	case mutant_human1obj:
	case final_boss1obj:
	case final_boss2obj:
	case final_boss3obj:
	case final_boss4obj:
		NewState(ob, &s_ofs_shoot1);
		break;


	case genetic_guardobj:
	case mutant_human2obj:
	case lcan_alienobj:
	case scan_alienobj:
		if (dist > CLOSE_RANGE)
		{
			NewState(ob, &s_ofs_spit1);
		}
		else
		{
			NewState(ob, &s_ofs_attack1);
		}
		break;

	default:
		break;
	}
}


/*
============================================================================

 PATH

============================================================================
*/

dirtype SelectPathDir(
	objtype* ob)
{
	bool CantWalk = false;
	bool RandomTurn = false;
	std::uint16_t spot;

	// Look for directional arrows!
	//
	spot = MAPSPOT(ob->tilex, ob->tiley, 1) - ICONARROWS;
	if (spot < 8)
	{
		ob->dir = static_cast<dirtype>(spot);
	}

	// Reset move distance and try to walk/turn.
	//
	ob->distance = 1.0;
	if (ob->flags & FL_RANDOM_TURN)
	{
		RandomTurn = US_RndT() > 180;
	}
	else
	{
		RandomTurn = false;
	}
	CantWalk = !TryWalk(ob, false);

	// Handle random turns and hitting walls
	//
	if (RandomTurn || CantWalk)
	{
		// Directional arrows have priority!
		//
		if ((!CantWalk) && (spot < 8))
		{
			goto exit_func;
		}

		// Either: path is blocked   OR   actor is randomly turning.
		//
		if (ob->trydir == nodir)
		{
			ob->trydir |= US_RndT() & 128;
		}
		else
		{
			ob->dir = static_cast<dirtype>(ob->trydir & 127);
		}

		// Turn this actor
		//
		if (ob->trydir & 128)
		{
			ob->dir--; // turn clockwise
			if (ob->dir < east)
			{
				ob->dir = static_cast<dirtype>(nodir - 1);
			}
		}
		else
		{
			ob->dir++; // turn counter-clockwise
			if (ob->dir >= nodir)
			{
				ob->dir = east;
			}
		}
		ob->trydir = static_cast<dirtype>((ob->trydir & 128) | ob->dir);

		// Walk into new direction?
		//
		if (!TryWalk(ob, false))
		{
			ob->dir = nodir;
		}
	}

exit_func:;
	if (ob->dir != nodir)
	{
		TryWalk(ob, true);
		ob->trydir = nodir;
	}

	return ob->dir;
}

void T_Path(
	objtype* ob)
{
	if ((ob->flags & FL_STATIONARY) != 0)
	{
		return;
	}

	switch (ob->obclass)
	{
		case volatiletransportobj:
			break;

		default:
			if ((ob->flags & FL_FRIENDLY) == 0 || madenoise)
			{
				if (SightPlayer(ob))
				{
					return;
				}
			}

#if LOOK_FOR_DEAD_GUYS
			if (LookForDeadGuys(ob))
			{
				return;
			}
#endif
			break;
	}

	if (ob->dir == nodir)
	{
		if (SelectPathDir(ob) == nodir)
		{
			return;
		}
	}

	auto move = ob->speed * tics;

	while (move != 0.0)
	{
		if (ob->distance < 0.0)
		{
			// Actor waiting for door to open
			//
			const auto door_index = static_cast<std::int16_t>(-static_cast<int>(ob->distance) - 1);

			OpenDoor(door_index);

			if (doorobjlist[door_index].action != dr_open)
			{
				return;
			}

			ob->distance = 1.0; // go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj(ob, move);
			break;
		}

		if (ob->tilex > MAPSIZE || ob->tiley > MAPSIZE)
		{
			BSTONE_THROW_STATIC_SOURCE("Actor walked out of map.");
		}

		ob->x = ob->tilex + 0.5;
		ob->y = ob->tiley + 0.5;
		move -= ob->distance;

		if (SelectPathDir(ob) == nodir)
		{
			return;
		}
	}
}


/*
=============================================================================

 FIGHT

=============================================================================
*/

std::int16_t morph_angle_adj = 0;

void T_Shoot(
	objtype* ob)
{
	std::int16_t dx, dy, dist;
	std::int16_t hitchance, damage;
	std::int16_t chance;

	switch (ob->obclass)
	{
	case SMART_ACTORS:
		if (!ob->ammo)
		{
			return;
		}

	default:
		break;
	}

	const auto& assets_info = get_assets_info();

	switch (ob->obclass)
	{
	case electroobj:
		SpawnProjectile(ob, electroshotobj);
		break;

	case mutant_human2obj:
		SpawnProjectile(ob, assets_info.is_ps() ? electroshotobj : scanshotobj);
		break;

	case liquidobj:
		SpawnProjectile(ob, liquidshotobj);
		break;

	case lcan_alienobj:
		SpawnProjectile(ob, lcanshotobj);
		break;

	case podobj:
		SpawnProjectile(ob, podshotobj);
		break;

	case scan_alienobj:
		SpawnProjectile(ob, scanshotobj);
		break;

	case gold_morphobj:
		SpawnProjectile(ob, goldmorphshotobj);

		if (ob->hitpoints < 500)
		{
			chance = 255 / 2;
		}
		else
		{
			chance = 255 / 4;
		}

		if (US_RndT() < chance)
		{
			morph_angle_adj = 24;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = -24;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = 16;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = -16;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = 8;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = -8;
			SpawnProjectile(ob, goldmorphshotobj);
			morph_angle_adj = 0;
		}
		break;

	case spider_mutantobj:
	case acid_dragonobj:
		SpawnProjectile(ob, static_cast<classtype>(spider_mutantshotobj + (ob->obclass - spider_mutantobj)));
		break;

	case final_boss2obj:
		SpawnProjectile(ob, final_boss2shotobj);
		break;

	case final_boss4obj:
		SpawnProjectile(ob, final_boss4shotobj);
		break;

	default:
		hitchance = 128;

		ob->lighting = -10;

		if (ob->areanumber < NUMAREAS && !areabyplayer[ob->areanumber])
		{
			return;
		}

		if (!CheckLine(ob, player))
		{ // player is behind a wall
			return;
		}

		dx = static_cast<std::int16_t>(abs(ob->tilex - player->tilex));
		dy = static_cast<std::int16_t>(abs(ob->tiley - player->tiley));
		dist = dx > dy ? dx : dy;

		if (ob->obclass == swatobj)
		{
			if (dist)
			{
				dist = dist * 2 / 3; // ss are better shots

			}
		}

		if (thrustspeed >= RUNSPEED)
		{
			if ((ob->flags & FL_VISIBLE) != 0)
			{
				hitchance = 160 - dist * 16; // player can see to dodge
			}
			else
			{
				hitchance = 160 - dist * 8;
			}
		}
		else
		{
			if ((ob->flags & FL_VISIBLE) != 0)
			{
				hitchance = 256 - dist * 16; // player can see to dodge
			}
			else
			{
				hitchance = 256 - dist * 8;
			}
		}

		// See if the shot was a hit.
		//
		if (US_RndT() < hitchance)
		{
			if (dist < 2)
			{
				damage = static_cast<std::int16_t>(US_RndT() >> 2);
			}
			else if (dist < 4)
			{
				damage = static_cast<std::int16_t>(US_RndT() >> 3);
			}
			else
			{
				damage = static_cast<std::int16_t>(US_RndT() >> 4);
			}

			TakeDamage(damage, ob);
		}

		switch (ob->obclass)
		{

		case proguardobj:
		case swatobj:
			sd_play_actor_weapon_sound(ATKBURSTRIFLESND, *ob);
			break;

		default:
			sd_play_actor_weapon_sound(ATKCHARGEDSND, *ob);
			break;
		}

#ifdef LIMITED_AMMO
		switch (ob->obclass)
		{
		case SMART_ACTORS:
			ob->ammo--;
			CheckRunChase(ob);
			break;

		default:
			break;
		}
#endif

		MakeAlertNoise(ob);
		break;
	}

	switch (ob->obclass)
	{
	case proguardobj:
	case swatobj:
		break;

	default:
		ob->flags &= ~FL_LOCKED_STATE;
		break;
	}
}

void T_Shade(
	objtype* obj)
{

	switch (obj->obclass)
	{
	case final_boss2obj:
		break;

	default:
		obj->lighting = 0;
		break;
	}
}

void T_Hit(
	objtype* ob)
{
	std::int16_t hitchance;
	std::int16_t damage;

	switch (ob->obclass)
	{
		case scan_alienobj:
		case lcan_alienobj:
		case podobj:
			hitchance = 220; // Higher - Better Chance (255 max!)
			damage = static_cast<std::int16_t>((US_RndT() >> 3) | 1);
			sd_play_actor_weapon_sound(CLAWATTACKSND, *ob);
			break;

		case genetic_guardobj:
		case mutant_human2obj:
			hitchance = 220; // Higher - Better Chance (255 max!)
			damage = static_cast<std::int16_t>((US_RndT() >> 3) | 1);
			sd_play_actor_weapon_sound(PUNCHATTACKSND, *ob);
			break;

		default:
			hitchance = 200; // Higher - Better Chance (255 max!)
			damage = static_cast<std::int16_t>(US_RndT() >> 4);
			break;
	}

	MakeAlertNoise(ob);

	const auto dx = std::abs(player->x - ob->x) - 1.0;

	if (dx <= MINACTORDIST)
	{
		const auto dy = std::abs(player->y - ob->y) - 1.0;

		if (dy <= MINACTORDIST)
		{
			if (US_RndT() < hitchance)
			{
				TakeDamage(damage, ob);
				return;
			}
		}
	}
}


/*
============================================================================

 DR. GOLDSTERN STATES & ROUTINES

============================================================================
*/

void A_Beep(
	objtype* obj);

void A_Laugh(
	objtype* obj);

void A_WarpIn(
	objtype* obj);

void A_WarpOut(
	objtype* obj);

void T_GoldMorph(
	objtype* obj);


extern statetype s_goldstand;

extern statetype s_goldpath1;
extern statetype s_goldpath1s;
extern statetype s_goldpath2;
extern statetype s_goldpath3;
extern statetype s_goldpath3s;
extern statetype s_goldpath4;

extern statetype s_goldpain;

extern statetype s_goldshoot1;
extern statetype s_goldshoot2;
extern statetype s_goldshoot3;
extern statetype s_goldshoot4;
extern statetype s_goldshoot5;
extern statetype s_goldshoot6;
extern statetype s_goldshoot7; // Add Line

extern statetype s_goldchase1;
extern statetype s_goldchase1s;
extern statetype s_goldchase2;
extern statetype s_goldchase3;
extern statetype s_goldchase3s;
extern statetype s_goldchase4;

extern statetype s_goldwarp_it;
extern statetype s_goldwarp_it1;
extern statetype s_goldwarp_it2;
extern statetype s_goldwarp_it3;
extern statetype s_goldwarp_it4;
extern statetype s_goldwarp_it5;

extern statetype s_goldwarp_out1;
extern statetype s_goldwarp_out2;
extern statetype s_goldwarp_out3;
extern statetype s_goldwarp_out4;
extern statetype s_goldwarp_out5;

extern statetype s_goldwarp_in1;
extern statetype s_goldwarp_in2;
extern statetype s_goldwarp_in3;
extern statetype s_goldwarp_in4;
extern statetype s_goldwarp_in5;

extern statetype s_goldmorphwait1;
extern statetype s_goldmorphwait2;


extern void T_GoldMorphWait(
	objtype* obj);


statetype s_goldstand = {1, SPR_DEMO, 20, T_Stand, nullptr, &s_goldpath1};

statetype s_goldpath1 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_goldpath1s};
statetype s_goldpath1s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_goldpath2};
statetype s_goldpath2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_goldpath3};
statetype s_goldpath3 = {1, SPR_DEMO, 20, T_Path, nullptr, &s_goldpath3s};
statetype s_goldpath3s = {1, SPR_DEMO, 5, nullptr, nullptr, &s_goldpath4};
statetype s_goldpath4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_goldpath1};

statetype s_goldpain = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldchase1};

statetype s_goldshoot1 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_goldshoot2};
statetype s_goldshoot2 = {0, SPR_DEMO, 20, nullptr, T_Shoot, &s_goldshoot3};
statetype s_goldshoot3 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_goldshoot4};
statetype s_goldshoot4 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_goldshoot5};
statetype s_goldshoot5 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_goldshoot6};
statetype s_goldshoot6 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_goldshoot7};
statetype s_goldshoot7 = {0, SPR_DEMO, 10, nullptr, T_Shade, &s_goldchase1};

statetype s_goldchase1 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_goldchase1s};
statetype s_goldchase1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_goldchase2};
statetype s_goldchase2 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_goldchase3};
statetype s_goldchase3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_goldchase3s};
statetype s_goldchase3s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_goldchase4};
statetype s_goldchase4 = {1, SPR_DEMO, 8, T_Chase, nullptr, &s_goldchase1};

statetype s_goldwarp_it = {0, SPR_DEMO, 45, nullptr, A_Laugh, &s_goldwarp_it1};
statetype s_goldwarp_it1 = {0, SPR_DEMO, 25, nullptr, nullptr, &s_goldwarp_it2};
statetype s_goldwarp_it2 = {0, SPR_DEMO, 25, nullptr, A_Beep, &s_goldwarp_it3};
statetype s_goldwarp_it3 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldwarp_it4};
statetype s_goldwarp_it4 = {0, SPR_DEMO, 25, nullptr, A_Beep, &s_goldwarp_it5};
statetype s_goldwarp_it5 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldwarp_out1};

statetype s_goldwarp_out1 = {0, SPR_DEMO, 30, nullptr, A_WarpOut, &s_goldwarp_out2};
statetype s_goldwarp_out2 = {0, SPR_DEMO, 30, nullptr, nullptr, &s_goldwarp_out3};
statetype s_goldwarp_out3 = {0, SPR_DEMO, 30, nullptr, nullptr, &s_goldwarp_out4};
statetype s_goldwarp_out4 = {0, SPR_DEMO, 30, nullptr, nullptr, &s_goldwarp_out5};
statetype s_goldwarp_out5 = {0, SPR_DEMO, 40, nullptr, nullptr, nullptr};

statetype s_goldwarp_in1 = {0, SPR_DEMO, 15, nullptr, A_WarpIn, &s_goldwarp_in2};
statetype s_goldwarp_in2 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldwarp_in3};
statetype s_goldwarp_in3 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldwarp_in4};
statetype s_goldwarp_in4 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldwarp_in5};
statetype s_goldwarp_in5 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_goldpath1};

statetype s_goldmorphwait1 = {0, SPR_DEMO, 10, nullptr, T_GoldMorphWait, &s_goldmorphwait1};

statetype s_goldmorph1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph2};
statetype s_goldmorph2 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph3};
statetype s_goldmorph3 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph4};
statetype s_goldmorph4 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph5};
statetype s_goldmorph5 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph6};
statetype s_goldmorph6 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph7};
statetype s_goldmorph7 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_goldmorph8};
statetype s_goldmorph8 = {0, SPR_DEMO, 10, nullptr, T_GoldMorph, &s_mgold_chase1};

statetype s_mgold_chase1 = {0, SPR_DEMO, 10, T_Chase, nullptr, &s_mgold_chase2};
statetype s_mgold_chase2 = {0, SPR_DEMO, 8, nullptr, nullptr, &s_mgold_chase3};
statetype s_mgold_chase3 = {0, SPR_DEMO, 10, T_Chase, nullptr, &s_mgold_chase4};
statetype s_mgold_chase4 = {0, SPR_DEMO, 8, nullptr, nullptr, &s_mgold_chase1};

statetype s_mgold_shoot1 = {0, SPR_DEMO, 10, nullptr, nullptr, &s_mgold_shoot2};
statetype s_mgold_shoot2 = {0, SPR_DEMO, 20, nullptr, nullptr, &s_mgold_shoot3};
statetype s_mgold_shoot3 = {0, SPR_DEMO, 14, T_Shoot, nullptr, &s_mgold_shoot4};
statetype s_mgold_shoot4 = {0, SPR_DEMO, 12, T_Shade, nullptr, &s_mgold_chase1};

statetype s_mgold_pain = {0, SPR_DEMO, 25, nullptr, nullptr, &s_mgold_chase1};


bool noShots = false;

std::int16_t morphWaitTime;


void T_GoldMorphWait(
	objtype* obj)
{
	morphWaitTime -= tics;
	if (morphWaitTime < 0)
	{
		NewState(obj, &s_goldmorph1);
	}
}

void T_GoldMorph(
	objtype* obj)
{
	obj->speed = ALIENSPEED * 4;
	obj->ammo = ALIENAMMOINIT;
	obj->flags |= FL_PROJ_TRANSPARENT | FL_NO_SLIDE | FL_SHOOTABLE | FL_SOLID;
	obj->flags2 = FL2_BFGSHOT_SOLID | FL2_BFG_SHOOTABLE;
	obj->hitpoints = get_start_hit_point(en_gold_morph);
	obj->obclass = gold_morphobj;

	noShots = false;
}

void A_Laugh(
	objtype* obj)
{
	sd_play_actor_voice_sound(GOLDSTERNLAUGHSND, *obj);
}

void A_WarpIn(
	objtype*)
{
	sd_play_player_item_sound(WARPINSND);
}

void A_WarpOut(
	objtype*)
{
	sd_play_player_item_sound(WARPOUTSND);
}

void A_Beep(
	objtype*)
{
	sd_play_player_item_sound(ELEV_BUTTONSND);
}

void InitGoldsternInfo()
{
	GoldsternInfo = GoldsternInfo_t{};
	GoldsternInfo.LastIndex = GOLDIE_MAX_SPAWNS;
}


// ===========================================================================
//
//
// RED FLASHING SECURITY LAMPS
//
//
// ===========================================================================

void T_FlipShape(
	objtype* obj)
{
	if (obj->flags & FL_ALERTED)
	{
		if (obj->temp1 ^= 1)
		{
			obj->lighting = LAMP_ON_SHADING;
		}
		else
		{
			obj->lighting = 0;
		}
	}
}


statetype s_security_light = {0, SPR_DEMO, 20, T_Security, T_FlipShape, &s_security_light};

void T_Security(
	objtype* obj)
{

	if (!(obj->flags & FL_ALERTED))
	{
		if (alerted && obj->areanumber < NUMAREAS && areabyplayer[obj->areanumber])
		{
			obj->flags |= FL_ALERTED;
		}
	}
}


// ==========================================================================
//
//
//                              GROUND & FLOATING SCOUT ROUTINES
//
//
// ==========================================================================

void A_Scout_Alert(
	objtype* obj);

void T_PainThink(
	objtype* obj);

extern statetype s_scout_path1;
extern statetype s_scout_path1s;
extern statetype s_scout_path2;
extern statetype s_scout_path3;
extern statetype s_scout_path3s;
extern statetype s_scout_path4;

extern statetype s_scout_pain;
extern statetype s_scout_pain2;

extern statetype s_scout_run;
extern statetype s_scout_run1s;
extern statetype s_scout_run2;
extern statetype s_scout_run3;
extern statetype s_scout_run3s;
extern statetype s_scout_run4;

extern statetype s_scout_dead;

statetype s_scout_stand = {1, SPR_DEMO, 20, T_Stand, nullptr, &s_scout_stand};

statetype s_scout_path1 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_scout_path1};
statetype s_scout_path2 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_scout_path2};
statetype s_scout_path3 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_scout_path3};
statetype s_scout_path4 = {1, SPR_DEMO, 15, T_Path, nullptr, &s_scout_path4};

statetype s_scout_run = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_scout_run};
statetype s_scout_run2 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_scout_run2};
statetype s_scout_run3 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_scout_run3};
statetype s_scout_run4 = {1, SPR_DEMO, 10, T_Chase, nullptr, &s_scout_run4};

statetype s_scout_dead = {0, SPR_DEMO, 20, nullptr, nullptr, &s_scout_dead};


void A_Scout_Alert(
	objtype* obj)
{
	sd_play_actor_voice_sound(SCOUT_ALERTSND, *obj);
	MakeAlertNoise(obj);
}


void T_ExplodeScout(
	objtype* obj)
{
	constexpr auto base_offset = bstone::math::fixed_to_floating(0x4000);

	constexpr auto rnd_offset_count = 8;
	using RndOffsets = std::array<double, rnd_offset_count>;

	auto rnd_offsets = RndOffsets{};

	std::generate(
		rnd_offsets.begin(),
		rnd_offsets.end(),
		[]()
		{
			constexpr auto rnd_max_offset = bstone::math::fixed_to_floating(0x2000);

			return rnd_max_offset * US_RndT() / 255.0;
		}
	);

	SpawnExplosion(obj->x + base_offset + rnd_offsets[0], obj->y + base_offset + rnd_offsets[1]);
	SpawnExplosion(obj->x - base_offset - rnd_offsets[2], obj->y + base_offset + rnd_offsets[3]);
	SpawnExplosion(obj->x - base_offset - rnd_offsets[4], obj->y - base_offset - rnd_offsets[5]);
	SpawnExplosion(obj->x + base_offset + rnd_offsets[6], obj->y - base_offset - rnd_offsets[7]);
}

void T_ExplodeDamage(
	objtype* obj)
{
	ExplodeRadius(obj, EXPLODE_DAMAGE, true);
}

void T_PainThink(
	objtype* obj)
{
	const auto full_hp = get_start_hit_point(obj->obclass - rentacopobj);

	if (obj->hitpoints > ((3 * full_hp) / 4))
	{
		//       Original HitPoints
		//

		switch (obj->obclass)
		{
		case floatingbombobj:
			NewState(obj, &s_scout_run);
			break;

		case volatiletransportobj:
			NewState(obj, &s_scout_path1);
			break;

		default:
			break;
		}
	}
	else if (obj->hitpoints > (full_hp / 2))
	{
		//      3/4 Original HitPoints
		//

		switch (obj->obclass)
		{
		case floatingbombobj:
			NewState(obj, &s_scout_run2);
			break;

		case volatiletransportobj:
			NewState(obj, &s_scout_path2);
			break;

		default:
			break;
		}
	}
	else if (obj->hitpoints > (full_hp / 4))
	{
		//      1/2 Original HitPoints
		//

		switch (obj->obclass)
		{
		case floatingbombobj:
			NewState(obj, &s_scout_run3);
			break;

		case volatiletransportobj:
			NewState(obj, &s_scout_path3);
			break;

		default:
			break;
		}
	}
	else
	{
		//      1/4 Original HitPoints
		//

		switch (obj->obclass)
		{
		case floatingbombobj:
			NewState(obj, &s_scout_run4);
			break;

		case volatiletransportobj:
			NewState(obj, &s_scout_path4);
			break;

		default:
			break;
		}
	}
}


// ==========================================================================
//
// EXPLOSION STUFF
//
// ==========================================================================

// Spawns an explosion at a given x & y.
void SpawnCusExplosion(
	const double x,
	const double y,
	std::uint16_t StartFrame,
	std::uint16_t NumFrames,
	std::uint16_t Delay,
	std::uint16_t Class)
{
	const auto tilex = static_cast<std::int16_t>(x);
	const auto tiley = static_cast<std::int16_t>(y);

	usedummy = true;
	nevermark = true;
	SpawnNewObj(tilex, tiley, &s_ofs_smart_anim);

	usedummy = false;
	nevermark = false;
	new_actor->x = x;
	new_actor->y = y;
	new_actor->flags = FL_OFFSET_STATES | FL_NONMARK | FL_NEVERMARK;
	new_actor->obclass = static_cast<classtype>(Class);
	new_actor->lighting = NO_SHADING;

	InitAnim(
		new_actor,
		StartFrame,
		0, static_cast<std::uint8_t>(NumFrames),
		at_ONCE,
		ad_FWD,
		(US_RndT() & 0x7),
		Delay
	);

	A_DeathScream(new_actor);
	MakeAlertNoise(new_actor);
}

void T_SpawnExplosion(
	objtype* obj)
{
	SpawnCusExplosion(obj->x, obj->y, SPR_EXPLOSION_1, 4, 4, explosionobj);
}


// ==========================================================================
//
// STEAM OBJECT STUFF
//
// ==========================================================================

void T_SteamObj(
	objtype* obj);

extern statetype s_steamrelease1;
extern statetype s_steamrelease2;
extern statetype s_steamrelease3;
extern statetype s_steamrelease4;
extern statetype s_steamrelease5;
extern statetype s_steamrelease6;

statetype s_steamgrate = {0, 0, 1, T_SteamObj, nullptr, &s_steamgrate};

statetype s_steamrelease1 = {0, 1, 1, nullptr, A_DeathScream, &s_steamrelease2};
statetype s_steamrelease2 = {0, 2, 14, nullptr, nullptr, &s_steamrelease3};
statetype s_steamrelease3 = {0, 3, 14, nullptr, nullptr, &s_steamrelease4};
statetype s_steamrelease4 = {0, 2, 14, nullptr, nullptr, &s_steamrelease5};
statetype s_steamrelease5 = {0, 3, 14, nullptr, nullptr, &s_steamrelease6};
statetype s_steamrelease6 = {0, 4, 16, nullptr, nullptr, &s_steamgrate};


void T_SteamObj(
	objtype* obj)
{
	if (obj->flags & FL_VISIBLE)
	{
		if ((obj->temp2 -= tics) <= 0)
		{
			NewState(obj, &s_steamrelease1);
			obj->temp2 = static_cast<std::int16_t>(US_RndT() << 3); // Up to 34 Seconds
		}
		else
		{
			obj->temp2 -= tics;
		}
	}
}

bool CheckPosition(
	objtype* ob)
{
	const auto xl = static_cast<int>(ob->x - PLAYERSIZE);
	const auto yl = static_cast<int>(ob->y - PLAYERSIZE);

	const auto xh = static_cast<int>(ob->x + PLAYERSIZE);
	const auto yh = static_cast<int>(ob->y + PLAYERSIZE);

	//
	// check for solid walls
	//
	for (auto y = yl; y <= yh; y++)
	{
		for (auto x = xl; x <= xh; x++)
		{
			const auto check = actorat[x][y];

			if (check != nullptr && check < objlist)
			{
				return false;
			}
		}
	}

	return true;
}


// ===========================================================================
//
//
// HANGING TERROT GUN
//
//
// ===========================================================================

extern statetype s_terrot_wait;

extern statetype s_terrot_shoot1;
extern statetype s_terrot_shoot2;
extern statetype s_terrot_shoot3;
extern statetype s_terrot_shoot4;

extern statetype s_terrot_seek1;
extern statetype s_terrot_seek1s;

extern statetype s_terrot_die1;
extern statetype s_terrot_die2;
extern statetype s_terrot_die3;
extern statetype s_terrot_die4;
extern statetype s_terrot_die5;



statetype s_terrot_wait = {1, SPR_DEMO, 1, T_Seek, nullptr, &s_terrot_wait};
statetype s_terrot_found = {0, SPR_DEMO, 0, T_Seek, nullptr, &s_terrot_found};

statetype s_terrot_shoot1 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_terrot_shoot2};
statetype s_terrot_shoot2 = {0, SPR_DEMO, 20, nullptr, T_Shade, &s_terrot_shoot3};
statetype s_terrot_shoot3 = {0, SPR_DEMO, 10, nullptr, T_Shoot, &s_terrot_shoot4};
statetype s_terrot_shoot4 = {0, SPR_DEMO, 20, nullptr, T_Shade, &s_terrot_seek1};

statetype s_terrot_seek1 = {1, SPR_DEMO, 10, T_Seek, nullptr, &s_terrot_seek1s};
statetype s_terrot_seek1s = {1, SPR_DEMO, 3, nullptr, nullptr, &s_terrot_seek1};

statetype s_terrot_die1 = {0, SPR_DEMO, 15, nullptr, A_DeathScream, &s_terrot_die2};
statetype s_terrot_die2 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_terrot_die3};
statetype s_terrot_die3 = {0, SPR_DEMO, 15, nullptr, nullptr, &s_terrot_die4};
statetype s_terrot_die4 = {0, SPR_DEMO, 15, nullptr, T_Shade, &s_terrot_die5};


statetype s_terrot_die5 = {0, SPR_DEMO, 0, nullptr, nullptr, &s_terrot_die5};


// ============================================================================
//
// SEEK
//
// ============================================================================

// ---------------------------------------------------------------------------
// T_Seek() - Will rotate an object (not moving) until seeing it is able
//      to see the player in the facing direction.
//
// NOTE : This may change so that it will seek for any object.
// ---------------------------------------------------------------------------
void T_Seek(
	objtype* ob)
{
	const std::int16_t MAX_VIS_DIST = 15;

	std::int16_t dx, dy, dist, chance;
	bool target_found;

	target_found = false;


	if (((player->tilex != ob->tilex) || (player->tiley != ob->tiley)) && // Can you see
		CheckView(ob, player) && (!PlayerInvisable))
	{
		dx = ob->tilex - player->tilex;
		dx = LABS(dx);
		dy = ob->tiley - player->tiley;
		dy = LABS(dy);

		if (dy < MAX_VIS_DIST && dx < MAX_VIS_DIST)
		{
			dist = dx > dy ? dx : dy;

			constexpr auto min_distance = bstone::math::fixed_to_floating(0x4000);

			if (!dist || (dist == 1 && ob->distance < min_distance))
			{
				chance = 300;
			}
			else
			{
				chance = static_cast<std::int16_t>(US_RndT() / dist);
			}

			if (US_RndT() < chance)
			{
				NewState(ob, &s_terrot_shoot1);
				return;
			}
			target_found = true;
		}
	}

	if (!(ob->flags & FL_STATIONARY))
	{
		if (target_found)
		{
			NewState(ob, &s_terrot_found);
		}
		else if ((ob->temp2 -= tics) <= 0)
		{
			NewState(ob, &s_terrot_seek1);

			ob->dir++;
			ob->temp2 = SEEK_TURN_DELAY;
			if (ob->dir == nodir)
			{
				ob->dir = east;
			}
		}
	}
}

void SpawnProjectile(
	objtype* shooter,
	classtype class_type)
{
	std::int16_t angle_adj = 0;
	std::uint16_t temp = 0;

	const auto tile_x = static_cast<std::uint8_t>(shooter->x);
	const auto tile_y = static_cast<std::uint8_t>(shooter->y);

	usedummy = true;
	nevermark = true;

	switch (class_type)
	{
		case spider_mutantshotobj:
		case acid_dragonshotobj:
		case final_boss4shotobj:
			SpawnNewObj(tile_x, tile_y, &s_ofs_random);
			sd_play_actor_voice_sound(SPITATTACKSND, *new_actor);
			new_actor->speed = SPDPROJ;
			angle_adj = 1 - (US_RndT() & 3);
			new_actor->temp1 = BossShotShapes[class_type - spider_mutantshotobj];
			new_actor->flags = FL_OFFSET_STATES | FL_PROJ_CHECK_TRANSPARENT | FL_STORED_OBJPTR;
			new_actor->temp3 = actor_to_ui16(shooter);
			break;

		case mut_hum1shotobj:
		case goldmorphshotobj:
		case electroshotobj:
		case final_boss2shotobj:
			SpawnNewObj(tile_x, tile_y, &s_ofs_shot1);
			sd_play_actor_voice_sound(ELECTSHOTSND, *new_actor);
			new_actor->speed = SPDPROJ;
			angle_adj = 1 - (US_RndT() & 3);
			new_actor->temp1 = SPR_ELEC_SHOT1;
			new_actor->flags = FL_OFFSET_STATES | FL_PROJ_CHECK_TRANSPARENT | FL_STORED_OBJPTR;
			new_actor->temp3 = actor_to_ui16(shooter);

			switch (class_type)
			{
				case final_boss2shotobj:
				case goldmorphshotobj:
					new_actor->temp1 = SPR_MGOLD_SHOT1;

				case electroshotobj:
					new_actor->lighting = NO_SHADING;

				default:
					break;
			}

			break;

		case lcanshotobj:
		case podshotobj:
			temp = SPR_SPIT3_1 - SPR_SPIT1_1;

		case scanshotobj:
		case dogshotobj:
			SpawnNewObj(tile_x, tile_y, &s_liquid_shot);
			sd_play_actor_voice_sound(SPITATTACKSND, *new_actor);
			new_actor->temp2 = SPR_SPIT1_1 + temp;
			new_actor->flags = FL_OFFSET_STATES | FL_PROJ_CHECK_TRANSPARENT | FL_STORED_OBJPTR;
			new_actor->speed = SPDPROJ + (US_RndT() / 65'536.0);
			angle_adj = 2 - (US_RndT() % 5);
			new_actor->temp3 = actor_to_ui16(shooter);
			break;

		case liquidshotobj:
			SpawnNewObj(tile_x, tile_y, &s_liquid_shot);
			new_actor->temp2 = SPR_LIQUID_SHOT_FLY_1;
			new_actor->flags = FL_OFFSET_STATES | FL_PROJ_CHECK_TRANSPARENT | FL_STORED_OBJPTR;
			new_actor->speed = SPDPROJ + (US_RndT() / 65'536.0);
			angle_adj = 2 - (US_RndT() % 5);
			new_actor->s_tilex = 0;
			new_actor->s_tiley = 0;
			new_actor->temp3 = actor_to_ui16(shooter);
			break;

		case grenadeobj:
			SpawnNewObj(tile_x, tile_y, &s_ofs_random);
			new_actor->speed = SPDPROJ * (1.0 + (0.5 * US_RndT() / 255.0));
			new_actor->angle = player->angle + 1 - (US_RndT() & 3);
			new_actor->temp1 = grenade_shapes[0];
			new_actor->flags = FL_OFFSET_STATES; // |FL_PROJ_CHECK_TRANSPARENT;
			new_actor->lighting = NO_SHADING; // no shading

			// Store off start tile x & y

			new_actor->s_tilex = tile_x;
			new_actor->s_tiley = tile_y;
			break;

		case bfg_shotobj:
			SpawnNewObj(tile_x, tile_y, &s_ofs_random);
			new_actor->speed = SPDPROJ * (1.0 + (US_RndT() / 255.0));
			new_actor->angle = player->angle + 1 - (US_RndT() & 3);
			new_actor->temp1 = SPR_BFG_WEAPON_SHOT2;
			new_actor->flags = FL_OFFSET_STATES;
			new_actor->lighting = NO_SHADING;

			// Store off start tile x & y

			new_actor->s_tilex = tile_x;
			new_actor->s_tiley = tile_y;
			break;

		default:
			break;
	}

	usedummy = false;
	nevermark = false;

	if (new_actor == &dummyobj)
	{
		return;
	}

	new_actor->x = shooter->x;
	new_actor->y = shooter->y;
	new_actor->active = ac_yes;
	new_actor->obclass = class_type;

	if (class_type != grenadeobj && class_type != bfg_shotobj)
	{
		new_actor->angle = CalcAngle(new_actor, player) + angle_adj;
	}

	if (shooter->obclass == gold_morphobj)
	{
		new_actor->angle += morph_angle_adj;
	}

	if (new_actor->angle <= 0)
	{
		new_actor->angle += 359;
	}
	else if (new_actor->angle > 359)
	{
		new_actor->angle -= 359;
	}

	new_actor->flags |= (FL_NONMARK | FL_NEVERMARK);

	// Move grenade slightly in front of player so you can see instant
	// explosions (ie: when you're face up against a wall).
	//
	if (class_type == grenadeobj || class_type == bfg_shotobj)
	{
		constexpr auto distance = 9.0 * MINDIST / 8.0;
		const auto deltax = distance * costable[new_actor->angle];
		const auto deltay = -distance * sintable[new_actor->angle];

		new_actor->x += deltax;
		new_actor->y += deltay;
	}
}

objtype* proj_check;
std::uint8_t proj_wall;

//
//  deltax - 'X' distance to travel
//  deltay - 'Y' distance to travel
//  distance - vectoral distance of travel
//
bool ProjectileTryMove(
	objtype* ob,
	const double deltax,
	const double deltay)
{
	constexpr auto PROJECTILE_MAX_STEP = PROJWALLSIZE;

	proj_wall = 0;

	//
	// Move that lil' projectile
	//

	for (auto steps = tics; steps != 0; --steps)
	{
		ob->x += deltax;
		ob->y += deltay;

		const auto xl = static_cast<int>(ob->x - PROJECTILE_MAX_STEP);
		const auto yl = static_cast<int>(ob->y - PROJECTILE_MAX_STEP);

		const auto xh = static_cast<int>(ob->x + PROJECTILE_MAX_STEP);
		const auto yh = static_cast<int>(ob->y + PROJECTILE_MAX_STEP);


		// Check for solid walls and actors.
		//

		for (auto y = yl; y <= yh; ++y)
		{
			for (auto x = xl; x <= xh; ++x)
			{
				proj_check = actorat[x][y];

				if (proj_check != nullptr)
				{
					if (proj_check < objlist)
					{
						if (proj_check == reinterpret_cast<objtype*>(1) && tilemap[x][y] == 0)
						{
							// We have a static!
							//
							// Test for collision radius using CENTER of static
							// NOT tile size boundries

							const auto ydist = std::abs(y + 0.5 - ob->y);
							const auto xdist = std::abs(x + 0.5 - ob->x);

							if (xdist < PROJCHECKSIZE && ydist < PROJCHECKSIZE)
							{
								proj_wall = 0;
								proj_check = nullptr;
								ob->tilex = static_cast<std::uint8_t>(ob->x);
								ob->tiley = static_cast<std::uint8_t>(ob->y);

								return false;
							}
						}
						else
						{
							// We have a wall!

							proj_wall = static_cast<std::uint8_t>(reinterpret_cast<std::size_t>(proj_check));
							proj_check = nullptr;
							ob->tilex = static_cast<std::uint8_t>(ob->x);
							ob->tiley = static_cast<std::uint8_t>(ob->y);

							return false;
						}
					}
					else if (proj_check < &objlist[MAXACTORS])
					{
						if ((ob->flags & FL_PROJ_CHECK_TRANSPARENT) != 0 &&
							(proj_check->flags & FL_PROJ_TRANSPARENT) != 0)
						{
							break;
						}
						else if ((proj_check->flags & FL_SOLID) != 0)
						{
							ob->tilex = static_cast<std::uint8_t>(ob->x);
							ob->tiley = static_cast<std::uint8_t>(ob->y);

							return false;
						}
					}
				}
			}
		}
	}

	return true;
}

void T_Projectile(
	objtype* ob)
{
	// Move this object.
	//
	const auto speed = ob->speed;

	auto deltax = speed * costable[ob->angle];
	auto deltay = -speed * sintable[ob->angle];


	// Did movement hit anything solid.
	//

	proj_check = nullptr;

	if (!ProjectileTryMove(ob, deltax, deltay))
	{
		switch (ob->obclass)
		{
			case spider_mutantshotobj:
				InitSmartSpeedAnim(ob, SPR_BOSS1_EXP1, 0, 2, at_ONCE, ad_FWD, 5);
				return;

			case acid_dragonshotobj:
				InitSmartSpeedAnim(ob, SPR_BOSS5_EXP1, 0, 2, at_ONCE, ad_FWD, 5);
				return;

			case mut_hum1shotobj:
			case electroshotobj: // Explode on walls
				InitSmartSpeedAnim(ob, SPR_ELEC_SHOT_EXP1, 0, 1, at_ONCE, ad_FWD, 5 + (US_RndT() & 3));
				return;

			case final_boss2shotobj:
			case goldmorphshotobj:
				InitSmartSpeedAnim(ob, SPR_MGOLD_SHOT_EXP1, 0, 1, at_ONCE, ad_FWD, 5 + (US_RndT() & 3));
				return;

			case final_boss4shotobj:
				InitSmartSpeedAnim(ob, SPR_BOSS10_SPIT_EXP1, 0, 1, at_ONCE, ad_FWD, 5 + (US_RndT() & 3));
				return;

			case lcanshotobj: // Explode on walls
			case podshotobj:
				InitSmartSpeedAnim(ob, SPR_SPIT_EXP3_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 3));
				return;

			case scanshotobj: // Explode on walls
			case dogshotobj:
				InitSmartSpeedAnim(ob, SPR_SPIT_EXP1_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
				return;

			case liquidshotobj: // Explode on walls
				InitSmartSpeedAnim(ob, SPR_LIQUID_SHOT_BURST_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
				return;

			case grenadeobj:
				// Hit actor - Hurt 'Em!
				if (proj_check != nullptr)
				{
					if ((proj_check->flags & FL_SHOOTABLE) != 0)
					{
						DamageActor(proj_check, GR_DAMAGE, ob);
					}
				}

				//
				// Start Anim, Sound, and mark as exploded...
				//

				ob->obclass = gr_explosionobj;
				InitSmartSpeedAnim(ob, SPR_EXPLOSION_1, 0, 4, at_ONCE, ad_FWD, 3 + (US_RndT() & 7));
				A_DeathScream(ob);
				return;

			case bfg_shotobj:

#if BFG_SHOT_STOPS
				//
				// Check to see if a collison has already occurred at this
				// tilex and tiley
				//
				if (ob->s_tilex == ob->tilex && ob->s_tiley == ob->tiley)
				{
					return;
				}

				ob->s_tilex = ob->tilex;
				ob->s_tilex = ob->tilex;
#endif

				if ((proj_wall & 0x80) != 0)
				{
					TryBlastDoor(proj_wall & (~0x80));
				}

				if (proj_check != nullptr)
				{
					// Hit actor - Hurt 'Em!

					if ((proj_check->flags2 & FL2_BFG_SHOOTABLE) != 0)
					{
						// Damage that actor
						DamageActor(proj_check, BFG_DAMAGE >> 1, ob); // bfg_damage>>3

						// Stop on actors that you don't kill.
					}

#if BFG_SHOT_STOPS
					if ((proj_check->flags2 & FL2_BFGSHOT_SOLID) != 0)
					{
						goto BlowIt;
					}

					if ((proj_check->flags & FL_DEADGUY) != 0)
					{
						return;
					}
#endif
				}


			BlowIt:
				//
				// Start Anim, Sound, and mark as exploded...
				//
				ob->obclass = bfg_explosionobj;
				InitAnim(ob, SPR_BFG_EXP1, 0, 7, at_ONCE, ad_FWD, (US_RndT() & 7), 5);
				A_DeathScream(ob);
				return;

			default:
				break;
		}
	}

	// Determine if object hit player.
	//
	if (ob->obclass != grenadeobj && ob->obclass != bfg_shotobj)
	{
		// Determine distance from player.
		//
		deltax = std::abs(ob->x - player->x);
		deltay = std::abs(ob->y - player->y);

		if (deltax < PROJECTILESIZE && deltay < PROJECTILESIZE)
		{
			deltax = PROJECTILESIZE * costable[ob->angle];
			deltay = -PROJECTILESIZE * sintable[ob->angle];

			ob->x -= deltax;
			ob->y -= deltay;

			auto damage = std::int16_t{};
			objtype* attacker = nullptr;

			if ((ob->flags & FL_STORED_OBJPTR) != 0)
			{
				attacker = ui16_to_actor(ob->temp3);
			}
			else
			{
				attacker = ob;
			}

			switch (ob->obclass)
			{
				case mut_hum1shotobj:
				case electroshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 5);
					InitSmartSpeedAnim(ob, SPR_ELEC_SHOT_EXP1, 0, 1, at_ONCE, ad_FWD, 3 + (US_RndT() & 7));
					break;

				case final_boss4shotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_BOSS10_SPIT_EXP1, 0, 1, at_ONCE, ad_FWD, 3 + (US_RndT() & 3));
					break;

				case goldmorphshotobj:
				case final_boss2shotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_MGOLD_SHOT_EXP1, 0, 1, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				case lcanshotobj:
				case podshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_SPIT_EXP3_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				case scanshotobj:
				case dogshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_SPIT_EXP1_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				case liquidshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_LIQUID_SHOT_BURST_1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				case spider_mutantshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_BOSS1_EXP1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				case acid_dragonshotobj:
					damage = static_cast<std::int16_t>(US_RndT() >> 4);
					InitSmartSpeedAnim(ob, SPR_BOSS5_EXP1, 0, 2, at_ONCE, ad_FWD, 5 + (US_RndT() & 7));
					break;

				default:
					break;
			}

			TakeDamage(damage, attacker);
			return;
		}
	}

	// Align tile coordinates on boundaries.
	//
	ob->tilex = static_cast<std::uint8_t>(ob->x);
	ob->tiley = static_cast<std::uint8_t>(ob->y);
}



#define EX_RADIUS 2 // Tiles out from center

using SubFfBuffer = std::array<std::int8_t, (EX_RADIUS * 2) + 1>;
using FfBuffer = std::array<SubFfBuffer, (EX_RADIUS * 2) + 1>;

FfBuffer ff_buffer;
std::int16_t ff_damageplayer, ff_damage;
objtype* ff_obj;

void ExplodeFill(
	std::int8_t tx,
	std::int8_t ty);

void ExplodeRadius(
	objtype* obj,
	std::int16_t damage,
	bool damageplayer)
{
	//
	// Did this object start out in a wall?
	//
	if (tilemap[obj->tilex][obj->tiley])
	{
		return;
	}

	// Setup globals to minimize parameter passing while recursing!
	//
	ff_obj = obj;
	ff_damage = damage;

	// Check to see if play is a Baby and should not be hurt by explosions,
	// except from actors that use explosions for attacks (IE. PerScan Drones).
	//
	if (gamestate.difficulty > gd_baby || obj->obclass == floatingbombobj)
	{
		ff_damageplayer = damageplayer;
	}
	else
	{
		ff_damageplayer = false;
	}

	ff_obj = obj;
	ff_buffer = FfBuffer{};
	ExplodeFill(obj->tilex, obj->tiley);
	ExplodeStatics(obj->tilex, obj->tiley);
}

void ExplodeFill(
	std::int8_t tx,
	std::int8_t ty)
{
	const auto& assets_info = get_assets_info();

	std::int8_t bx = tx - ff_obj->tilex + EX_RADIUS;
	std::int8_t by = ty - ff_obj->tiley + EX_RADIUS;
	std::int8_t door;
	std::int8_t no_wall;

	// Damage actors on this spot!
	//
	if (ff_damageplayer && tx == player->tilex && ty == player->tiley)
	{
		TakeDamage(EXPLODE_DAMAGE, ff_obj);
	}
	else
	{
		proj_check = actorat[tx & 63][ty & 63];

		if ((proj_check >= objlist) && (proj_check < &objlist[MAXACTORS]))
		{
			if ((proj_check->flags & FL_SHOOTABLE))
			{
				switch (proj_check->obclass)
				{
					// Detinate all floating bombs & VMTs
					//
				case floatingbombobj:
				case volatiletransportobj:
					DamageActor(proj_check, 500, ff_obj);
					break;

					// Hanging turrets are not effected by
					// concussion weapons.
					//
				case hang_terrotobj:
				case arc_barrierobj:
				case post_barrierobj:
				case vpost_barrierobj:
				case vspike_barrierobj:
					break;

					//
					// Test for Level completion object
					//
				case rotating_cubeobj:
					if (!assets_info.is_ps())
					{
						break;
					}

					if (ff_obj->obclass == pd_explosionobj)
					{
						proj_check->lighting = EXPLOSION_SHADING;
						proj_check->flags &= ~(FL_SOLID | FL_SHOOTABLE);
						InitSmartSpeedAnim(proj_check, SPR_CUBE_EXP1, 0, 8, at_ONCE, ad_FWD, 5);
						sd_play_actor_voice_sound(EXPLODE1SND, *proj_check);
						// Unlock Next floor
						gamestuff.level[gamestate.mapon + 1].locked = false;
					}
					break;

					//
					// Plasma/Fision Detonators (already armed)
					//
				case plasma_detonatorobj:
					if (ff_obj == player || // Player shot it with gun
						(ff_obj->tilex == tx && ff_obj->tiley == ty)) // Direct Hit with grenade
					{
						DamageActor(proj_check, 1, ff_obj);
					}
					else
					{
						DamageActor(proj_check, 20, ff_obj); // An explosion has started a chain reaction
					}
					break;


					// Everyone else gets the shit kicked
					// out of them...
					//
				default:
					if (!assets_info.is_ps() ||
						(assets_info.is_ps() && (proj_check->flags2 & FL2_CLOAKED) == 0))
					{
						SpawnFlash(proj_check->x, proj_check->y);
					}

					DamageActor(proj_check, ff_damage, ff_obj);
					break;
				}
			}
		}
	}

	// Mark spot as exploded!
	//
	ff_buffer[static_cast<int>(bx)][static_cast<int>(by)] = 1;

	// Explode to the EAST!
	//
	bx += 1;
	tx += 1;

	door = tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	if (door & 0x80)
	{
		no_wall = doorobjlist[door & 0x7f].action != dr_closed;
	}
	else
	{
		no_wall = !tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	}

	if ((bx <= (EX_RADIUS * 2)) && no_wall && (!ff_buffer[static_cast<int>(bx)][static_cast<int>(by)]))
	{
		ExplodeFill(tx, ty);
	}

	// Explode to the WEST!
	//
	bx -= 2;
	tx -= 2;

	door = tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	if (door & 0x80)
	{
		no_wall = doorobjlist[door & 0x7f].action != dr_closed;
	}
	else
	{
		no_wall = !tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	}

	if (bx >= 0 && no_wall && (!ff_buffer[static_cast<int>(bx)][static_cast<int>(by)]))
	{
		ExplodeFill(tx, ty);
	}

	// Explode to the SOUTH!
	//
	bx++;
	tx++;
	by += 1;
	ty += 1;

	door = tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	if (door & 0x80)
	{
		no_wall = doorobjlist[door & 0x7f].action != dr_closed;
	}
	else
	{
		no_wall = !tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	}

	if ((by <= (EX_RADIUS * 2)) &&
		no_wall &&
		(!ff_buffer[static_cast<int>(bx)][static_cast<int>(by)]))
	{
		ExplodeFill(tx, ty);
	}

	// Explode to the NORTH!
	//
	by -= 2;
	ty -= 2;

	door = tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	if (door & 0x80)
	{
		no_wall = doorobjlist[door & 0x7f].action != dr_closed;
	}
	else
	{
		no_wall = !tilemap[static_cast<int>(tx)][static_cast<int>(ty)];
	}

	if ((by >= 0) &&
		no_wall &&
		(!ff_buffer[static_cast<int>(bx)][static_cast<int>(by)]))
	{
		ExplodeFill(tx, ty);
	}
}

// ---------------------------------------------------------------------------
// CalcAngle() - Calculates angle from 1st object to 2nd object.
// ---------------------------------------------------------------------------
std::int16_t CalcAngle(
	objtype* from_obj,
	objtype* to_obj)
{
	// Calculate deltas from "from_obj" to "to_obj".
	//

	const auto deltax = to_obj->x - from_obj->x;
	const auto deltay = from_obj->y - to_obj->y;

	if (deltax == 0.0 && deltay == 0.0)
	{
		return 1;
	}

	// Calc Arc Tan from Obj1 to Obj2 - Returns radians

	auto angle = std::atan2(deltay, deltax);

	if (angle < 0.0)
	{
		angle += 2.0 * bstone::math::pi();
	}

	// Convert rads to degs

	const auto iangle = static_cast<std::int16_t>(angle / (2.0 * bstone::math::pi()) * ANGLES);

	return iangle;
}

void T_BlowBack(
	objtype* obj)
{
	static const std::uint16_t dist_table[] =
	{
		0x1000, // wp_autocharge,
		0x2000, // wp_pistol,
		0x3000, // wp_burst_rifle,
		0x4000, // wp_ion_cannon,
		0x5000, // wp_grenade,
	};

	if ((obj->flags & FL_NO_SLIDE) != 0)
	{
		return;
	}

	if ((obj->flags & FL_SLIDE_INIT) == 0)
	{
		// Check for NULL ptr

		auto killer = SLIDE_TEMP(obj);

		if (killer == nullptr)
		{
			obj->flags |= FL_NO_SLIDE;
			return;
		}

		obj->angle = CalcAngle(killer, obj);

		killer = SLIDE_TEMP(obj);

		if (killer == player)
		{
			reinterpret_cast<std::uint16_t&>(obj->hitpoints) =
				dist_table[static_cast<int>(gamestate.weapon)];
		}
		else
		{
			reinterpret_cast<std::uint16_t&>(obj->hitpoints) =
				dist_table[wp_grenade];
		}

		obj->flags |= FL_SLIDE_INIT;
	}


	constexpr auto SLIDE_SPEED = 0x2000;

	std::uint16_t dist;

	if (static_cast<std::uint16_t>(obj->hitpoints) > SLIDE_SPEED)
	{
		dist = SLIDE_SPEED;
		reinterpret_cast<std::uint16_t&>(obj->hitpoints) -= SLIDE_SPEED;
	}
	else
	{
		dist = static_cast<std::uint16_t>(obj->hitpoints);
		obj->flags |= FL_NO_SLIDE; // Stop any more sliding
	}

	const auto dist_f = bstone::math::fixed_to_floating(dist);
	const auto deltax = dist_f * costable[obj->angle]; // Optomize - Store in actor
	const auto deltay = -dist_f * sintable[obj->angle]; //

	if (ClipMove(obj, deltax, deltay))
	{
		obj->flags |= FL_NO_SLIDE;
	}

	obj->tilex = static_cast<std::uint8_t>(obj->x);
	obj->tiley = static_cast<std::uint8_t>(obj->y);

	if ((obj->flags & FL_NO_SLIDE) != 0)
	{
		// Set actor WHERE IT SLID in actorat[], IF there's a door!
		//
		if ((tilemap[obj->tilex][obj->tiley] & 0x80) != 0)
		{
			actorat[obj->tilex][obj->tiley] = obj;
			obj->flags &= ~FL_NEVERMARK;
		}
	}
}
