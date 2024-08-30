/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include "gfxv.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "id_sd.h"

#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_scope_exit.h"


static int get_wall_page_count()
{
	return bstone::globals::page_mgr->get_wall_count();
}

#define NUM_TILES get_wall_page_count()


void TakeDamage(
	std::int16_t points,
	objtype* attacker);

void SetPlaneViewSize();

void HealSelf(
	std::int16_t points);

void GiveWeapon(
	int weapon);

void DrawScore();
void SetPlaneViewSize();


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWTILEX (viewwidth / 16)
#define VIEWTILEY (viewheight / 16)


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

bool ForceLoadDefault = false;

bool DebugKeys();


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

bool PP_step = false;

std::int16_t maporgx;
std::int16_t maporgy;

void ViewMap();


void DebugMemory()
{
	US_CenterWindow(22, 15);

	US_Print("\nTics: ");
	US_PrintUnsigned(tics);

	US_Print("\nReal Tics: ");
	US_PrintUnsigned(realtics);

	if (!gp_is_ceiling_solid())
	{
		US_Print("\n\nCeiling TEX: ");
		US_PrintUnsigned(CeilingTile - START_TEXTURES);

		US_Print("\nFloor TEX: ");
		US_PrintUnsigned(FloorTile - START_TEXTURES);
	}
	else
	{
		US_Print("\n\nTop COL: ");
		US_PrintUnsigned(TopColor & 0xFF);

		US_Print("\nBottom COL: ");
		US_PrintUnsigned(BottomColor & 0xFF);
	}

	if (!gp_no_shading())
	{
		US_Print("\nShade div: ");
		US_PrintUnsigned(normalshade_div);

		US_Print("\nShade max: ");
		US_PrintUnsigned(shade_max);
	}

	VW_UpdateScreen();
	IN_Ack();

	WindowW = 253;
	WindowH = 8;
	fontnumber = 2;

	LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	ShadowPrintLocationText(sp_normal);
}

void CountObjects()
{
	US_CenterWindow(16, 7);

	const auto total_statics = static_cast<int>(laststatobj - &statobjlist[0]);
	US_Print("Total statics: ");
	US_PrintUnsigned(total_statics);

	auto in_use_static_count = 0;

	for (auto i = 0; i < total_statics; ++i)
	{
		if (statobjlist[i].shapenum != -1)
		{
			in_use_static_count += 1;
		}
	}

	US_Print("\nIn use statics: ");
	US_PrintUnsigned(in_use_static_count);

	US_Print("\nDoors: ");
	US_PrintUnsigned(doornum);

	auto active_static_count = 0;
	auto inactive_static_count = 0;

	for (auto obj = player->next; obj; obj = obj->next)
	{
		if (obj->active)
		{
			active_static_count += 1;
		}
		else
		{
			inactive_static_count += 1;
		}
	}

	US_Print("\nTotal actors: ");
	US_PrintUnsigned(active_static_count + inactive_static_count);

	US_Print("\nActive actors: ");
	US_PrintUnsigned(active_static_count);

	VW_UpdateScreen();
	IN_Ack();
}

void CountTotals()
{
	US_CenterWindow(20, 11);

	US_Print("CURRENT MAP TOTALS\n");

	US_Print("\nTotal Enemy: ");
	US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_enemy);

	US_Print("\nTotal Points: ");
	US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_points);

	US_Print("\nTotal Informants: ");
	US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_inf);

	VW_UpdateScreen();
	IN_Ack();
}

void ShowMap()
{
	const auto old_player = *player;
	player->angle = 90;
	player->x = player->y = MAPSIZE + 0.5;

	US_CenterWindow(20, 11);

	US_CPrint("CURRENT MAP\n\n ");

	const auto old_flags = ExtraRadarFlags;

	const auto old_vid_is_3d = vid_is_3d;
	const auto old_vid_is_hud = vid_is_hud;
	vid_is_3d = false;
	vid_is_hud = false;

	ExtraRadarFlags |= OV_ACTORS | OV_PUSHWALLS;
	ShowOverhead(160 - 32, py, 32, 0, OV_ACTORS | OV_SHOWALL | OV_KEYS | OV_PUSHWALLS | OV_WHOLE_MAP);
	VW_UpdateScreen();
	ExtraRadarFlags = old_flags;

	vid_is_3d = old_vid_is_3d;
	vid_is_hud = old_vid_is_hud;

	*player = old_player;
	IN_Ack();
}


// ---------------------------------------------------------------------------
// IncRange - Incs a value to a MAX value (including max value)
//
// NOTE: Assumes that 0 is the lowest value
// ---------------------------------------------------------------------------
std::uint16_t IncRange(
	std::uint16_t Value,
	std::uint16_t MaxValue)
{
	if (Value == MaxValue)
	{
		Value = 0;
	}
	else
	{
		Value++;
	}

	return Value;
}

// ---------------------------------------------------------------------------
// DecRange - Decs a value to 0 and resets to MAX_VALUE
//
// NOTE: Assumes that 0 is the lowest value
// ---------------------------------------------------------------------------
std::uint16_t DecRange(
	std::uint16_t Value,
	std::uint16_t MaxValue)
{
	if (Value == 0)
	{
		Value = MaxValue;
	}
	else
	{
		Value--;
	}

	return Value;
}

std::string get_bonus_item_name(const statobj_t& bs_static, bool& is_plural)
{
	is_plural = false;
	auto static_name = std::string{};

	if (false) {}
	else if (bs_static.itemnumber == bo_money_bag)
	{
		static_name = "Money Bag";
	}
	else if (bs_static.itemnumber == bo_loot)
	{
		static_name = "Loot";
	}
	else if (bs_static.itemnumber == bo_gold1)
	{
		static_name = "Gold Bar";
	}
	else if (bs_static.itemnumber == bo_gold2)
	{
		static_name = "Two Gold Bars";
		is_plural = true;
	}
	else if (bs_static.itemnumber == bo_gold3)
	{
		static_name = "Three Gold Bars";
		is_plural = true;
	}
	else if (bs_static.itemnumber == bo_gold)
	{
		static_name = "Five Gold Bars";
		is_plural = true;
	}
	else if (bs_static.itemnumber == bo_bonus)
	{
		static_name = "Xylan Orb";
	}

	return static_name;
}

std::string get_bonus_item_name(const statobj_t& bs_static)
{
	auto is_plural = false;
	return get_bonus_item_name(bs_static, is_plural);
}

void log_bonus_stuff()
{
	const auto& stats = gamestuff.level[gamestate.mapon].stats;

	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("<<<<<<<<");
	bstone::globals::logger->log_information("Current bonus items.");

	bstone::globals::logger->log_information((
		std::string{"stats:"} +
			std::to_string(stats.accum_points) + "/" +
			std::to_string(stats.total_points)).c_str());

	auto number = 0;
	auto static_name = std::string{};

	for (const auto& bs_static : statobjlist)
	{
		if (&bs_static == laststatobj)
		{
			break;
		}

		if (bs_static.shapenum < 0)
		{
			continue;
		}

		static_name = get_bonus_item_name(bs_static);

		if (!static_name.empty())
		{
			number += 1;

			bstone::globals::logger->log_information((
				std::to_string(number) +
					". (" +
					std::to_string(bs_static.tilex) +
					", " +
					std::to_string(bs_static.tiley) +
					") " +
					static_name).c_str());
		}
	}

	bstone::globals::logger->log_information(">>>>>>>>");
}

std::string get_enemy_actor_name(const objtype& bs_actor)
{
	constexpr auto large_alien_name = "Large Experimental\r Genetic Alien";
	constexpr auto small_alien_name = "Small Experimental\r Genetic Alien";
	constexpr auto mutated_guard_name = "Mutated Guard";

	constexpr auto spider_mutant_name = "Spider Mutant";
	constexpr auto reptilian_warrior_name = "Reptilian Warrior";
	constexpr auto experimental_mutant_human_name = "Experimental\r Mutant Human";

	constexpr auto asleep_string = "asleep";
	constexpr auto awake_string = "awake";

	const auto& assets_info = get_assets_info();
	const auto is_dead = bs_actor.hitpoints <= 0 || (bs_actor.flags & FL_DEADGUY) != 0;

	auto actor_name = std::string{};

	const auto set_actor_name_with_state_name = [&actor_name](
		const char* base_actor_name,
		const char* state_name)
	{
		actor_name = base_actor_name;
		actor_name += " (";
		actor_name += state_name;
		actor_name += ')';
	};

	const auto set_actor_name_with_state = [&actor_name, &awake_string, &asleep_string](
		const char* base_actor_name,
		bool is_awake)
	{
		actor_name = base_actor_name;
		actor_name += " (";
		actor_name += is_awake ? awake_string : asleep_string;
		actor_name += ')';
	};

	const auto is_informant = (bs_actor.flags & FL_INFORMANT) != 0;
	const auto is_static = bs_actor.state == &s_ofs_static;

	actor_name.clear();

	switch (bs_actor.obclass)
	{
	case rentacopobj:
		if (assets_info.is_aog())
		{
			actor_name = "Sector Patrol";
		}
		else
		{
			actor_name = "Sector Guard";
		}

		break;

	case hang_terrotobj:
		actor_name = "Robot Turret";
		break;

	case gen_scientistobj:
		if (!is_informant)
		{
			actor_name = "Mean Bio-Technician";
		}
		break;

	case podobj:
		actor_name = "Pod Alien";
		break;

	case electroobj:
		actor_name = "High Enemy\r Plasma Alien";
		break;

	case electrosphereobj:
		actor_name = "Plasma Sphere";
		break;

	case proguardobj:
		if (assets_info.is_aog())
		{
			actor_name = "Star Sentinel";
		}
		else
		{
			actor_name = "Tech Warrior";
		}

		break;

	case genetic_guardobj:
		actor_name = "High Security\r Genetic Guard";
		break;

	case mutant_human1obj:
		actor_name = "Experimental\r Mech Sentinel";
		break;

	case mutant_human2obj:
		actor_name = experimental_mutant_human_name;
		break;

	case lcan_wait_alienobj:
		if (!is_static)
		{
			set_actor_name_with_state(large_alien_name, is_dead);
		}

		break;

	case lcan_alienobj:
		actor_name = large_alien_name;
		break;

	case scan_wait_alienobj:
		if (!is_static)
		{
			set_actor_name_with_state(small_alien_name, is_dead);
		}

		break;

	case scan_alienobj:
		actor_name = small_alien_name;
		break;

	case gurney_waitobj:
		if (!is_static)
		{
			set_actor_name_with_state(mutated_guard_name, is_dead);
		}

		break;

	case gurneyobj:
		actor_name = mutated_guard_name;
		break;

	case liquidobj:
		actor_name = "Fluid Alien";
		break;

	case swatobj:
		if (assets_info.is_aog())
		{
			actor_name = "Star Trooper";
		}
		else
		{
			actor_name = "Alien Protector";
		}

		break;

	case goldsternobj:
		actor_name = "Dr. Goldfire";
		break;

	case gold_morphobj:
		actor_name = "Morphed Dr. Goldfire";
		break;

	case volatiletransportobj:
		actor_name = "Volatile\r Material Transport";
		break;

	case floatingbombobj:
		actor_name = "Perscan Drone";
		break;

	case rotating_cubeobj:
		if (assets_info.is_aog())
		{
			actor_name = "Projection Generator";
		}
		else
		{
			actor_name = "Security Cube";
		}

		break;

	case spider_mutantobj:
		actor_name = spider_mutant_name;
		break;

	case breather_beastobj:
		actor_name = "Breather Beast";
		break;

	case cyborg_warriorobj:
		actor_name = "Cyborg Warrior";
		break;

	case reptilian_warriorobj:
		actor_name = reptilian_warrior_name;
		break;

	case acid_dragonobj:
		actor_name = "Acid Dragon";
		break;

	case mech_guardianobj:
		actor_name = "Bio-Mech Guardian";
		break;

	case final_boss1obj:
		actor_name = "The Giant Stalker";
		break;

	case final_boss2obj:
		actor_name = "The Spector Demon";
		break;

	case final_boss3obj:
		actor_name = "The Armored Stalker";
		break;

	case final_boss4obj:
		actor_name = "The Crawler Beast";
		break;

	case blakeobj:
		actor_name = "Blake Stone";
		break;

	case morphing_spider_mutantobj:
		set_actor_name_with_state_name(spider_mutant_name, asleep_string);
		break;

	case morphing_reptilian_warriorobj:
		set_actor_name_with_state_name(reptilian_warrior_name, asleep_string);
		break;

	case morphing_mutanthuman2obj:
		set_actor_name_with_state_name(experimental_mutant_human_name, asleep_string);
		break;

	default:
		break;
	}

	return actor_name;
}

void log_enemy_stuff()
{
	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("<<<<<<<<");
	bstone::globals::logger->log_information("Current enemies.");

	bstone::globals::logger->log_information((
		std::string("stats:")
		+std::to_string(gamestuff.level[gamestate.mapon].stats.accum_enemy)
		+"/"
		+std::to_string(gamestuff.level[gamestate.mapon].stats.total_enemy)
		).c_str());

	auto number = 0;

	for (auto bs_actor = objlist; bs_actor != nullptr; bs_actor = bs_actor->next)
	{
		const auto is_dead = bs_actor->hitpoints <= 0 || (bs_actor->flags & FL_DEADGUY) != 0;

		const auto is_asleep =
			bs_actor->obclass == lcan_wait_alienobj ||
			bs_actor->obclass == scan_wait_alienobj ||
			bs_actor->obclass == gurney_waitobj;

		if (is_dead)
		{
			if (!is_asleep)
			{
				continue;
			}
		}

		auto actor_name = get_enemy_actor_name(*bs_actor);

		if (bs_actor->obclass == goldsternobj || bs_actor->obclass == electroobj)
		{
			actor_name += " (non-countable)";
		}

		if (!actor_name.empty())
		{
			number += 1;

			bstone::globals::logger->log_information(
				(std::to_string(number) +
					". (" +
					std::to_string(bs_actor->tilex) +
					", " +
					std::to_string(bs_actor->tiley) +
					") " +
					actor_name).c_str()
			);
		}
	}

	bstone::globals::logger->log_information(">>>>>>>>");
}

void log_stuff()
{
	log_bonus_stuff();
	log_enemy_stuff();

	US_CenterWindow(19, 3);
	US_PrintCentered("See log for stuff.");
	VW_UpdateScreen();
	IN_Ack();
}

const statobj_t* find_bonus_item()
{
	for (const auto& bs_static : statobjlist)
	{
		if (&bs_static == laststatobj)
		{
			break;
		}

		if (bs_static.shapenum < 0)
		{
			continue;
		}

		const auto static_name = get_bonus_item_name(bs_static);

		if (static_name.empty())
		{
			continue;
		}

		return &bs_static;
	}

	return nullptr;
}

const objtype* find_countable_enemy()
{
	for (auto bs_actor = objlist; bs_actor != nullptr; bs_actor = bs_actor->next)
	{
		if (bs_actor->obclass == goldsternobj ||
			bs_actor->obclass == electroobj ||
			bs_actor->obclass == rotating_cubeobj)
		{
			continue;
		}

		const auto is_dead = bs_actor->hitpoints <= 0 || (bs_actor->flags & FL_DEADGUY) != 0;

		const auto is_asleep =
			bs_actor->obclass == lcan_wait_alienobj ||
			bs_actor->obclass == scan_wait_alienobj ||
			bs_actor->obclass == gurney_waitobj;

		if (is_dead)
		{
			if (!is_asleep)
			{
				continue;
			}
		}

		const auto actor_name = get_enemy_actor_name(*bs_actor);

		if (actor_name.empty())
		{
			continue;
		}

		return bs_actor;
	}

	return nullptr;
}

bool DebugKeys()
{
	auto unmute_scene_sfx = false;

	if (Keyboard[ScanCode::sc_l])
	{
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		const auto unmute_scene_sfx_scope_guard = bstone::make_scope_exit(
			[&unmute_scene_sfx]()
			{
				if (unmute_scene_sfx)
				{
					sd_pause_scene_sfx(false);
				}
			});

		log_stuff();
		return true;
	}

	if (Keyboard[ScanCode::sc_a])
	{
		// A = Show Actors on AutoMap
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		ExtraRadarFlags ^= OV_ACTORS;
		US_CenterWindow(24, 3);

		if ((ExtraRadarFlags & OV_ACTORS) != 0)
		{
			US_PrintCentered("AUTOMAP: Show Actors ON");
		}
		else
		{
			US_PrintCentered("AUTOMAP: Show Actors OFF");
		}

		VW_UpdateScreen();
		IN_Ack();

		return true;
	}

	if (Keyboard[ScanCode::sc_k])
	{
		// K = Map Content totals
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		CountTotals();

		return true;
	}
	else if (Keyboard[ScanCode::sc_c])
	{
		// C = count objects
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		CountObjects();

		return true;
	}
	else if (Keyboard[ScanCode::sc_r])
	{
		// R = show full map
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		ShowMap();

		return true;
	}
	else if (Keyboard[ScanCode::sc_d])
	{
		// D = Dumb/Blind Objects (Player Invisable)
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(19, 3);
		PlayerInvisable = !PlayerInvisable;

		if (PlayerInvisable)
		{
			US_PrintCentered("Player Invisible!");
		}
		else
		{
			US_PrintCentered("Player visible");
		}

		VW_UpdateScreen();
		IN_Ack();

		return true;
	}
	else if (Keyboard[ScanCode::sc_e])
	{
		// E = Win Mission
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(19, 3);
		US_PrintCentered("Instant Winner!");
		InstantWin = 1;
		playstate = ex_victorious;
		VW_UpdateScreen();
		IN_Ack();

		return true;
	}
	else if (Keyboard[ScanCode::sc_f])
	{
		// F = facing spot
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(18, 5);

		US_Print("X: ");
		US_PrintUnsigned(static_cast<unsigned int>(player->x));
		US_Print(" (");
		US_PrintF64(player->x);
		US_Print(")");

		US_Print("\nY: ");
		US_PrintUnsigned(static_cast<unsigned int>(player->y));
		US_Print(" (");
		US_PrintF64(player->y);
		US_Print(")");

		US_Print("\nA: ");
		US_PrintUnsigned(player->angle);

		US_Print("\nD: ");
		US_PrintUnsigned(player->dir);

		VW_UpdateScreen();
		IN_Ack();

		return true;
	}

	if (Keyboard[ScanCode::sc_g])
	{
		// G = god mode
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(12, 2);

		if (godmode)
		{
			US_PrintCentered("God mode OFF");
		}
		else
		{
			US_PrintCentered("God mode ON");
		}

		VW_UpdateScreen();
		IN_Ack();
		godmode = !godmode;

		return true;
	}


	if (Keyboard[ScanCode::sc_h])
	{
		// H = hurt self
		//

		IN_ClearKeysDown();
		TakeDamage(1, nullptr);
	}
	else if (Keyboard[ScanCode::sc_i])
	{
		// I = item cheat
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(12, 3);
		US_PrintCentered("Free items!");
		VW_UpdateScreen();
		HealSelf(99);
		GiveToken(5);

		const auto& assets_info = get_assets_info();

		const auto n = static_cast<int>(assets_info.is_ps() ? wp_bfg_cannon : wp_grenade);

		for (auto i = static_cast<int>(wp_autocharge); i <= n; ++i)
		{
			if ((gamestate.weapons & (1 << i)) == 0)
			{
				GiveWeapon(i);
				break;
			}
		}

		gamestate.ammo += 50;

		if (gamestate.ammo > MAX_AMMO)
		{
			gamestate.ammo = MAX_AMMO;
		}

		DrawAmmo(true);
		DrawScore();
		IN_Ack();

		return true;
	}
	else if (Keyboard[ScanCode::sc_m])
	{
		// M = memory info
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		DebugMemory();

		return true;
	}
	else if (Keyboard[ScanCode::sc_q])
	{
		// Q = fast quit
		//

		Quit();
	}
	else if (Keyboard[ScanCode::sc_o])
	{
		// O = Show Push Walls
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		ExtraRadarFlags ^= OV_PUSHWALLS;
		US_CenterWindow(24, 3);

		if ((ExtraRadarFlags & OV_PUSHWALLS) != 0)
		{
			US_PrintCentered("AUTOMAP: Show PWalls ON");
		}
		else
		{
			US_PrintCentered("AUTOMAP: Show PWalls OFF");
		}

		VW_UpdateScreen();
		IN_Ack();

		return true;
	}
	else if (Keyboard[ScanCode::sc_u])
	{
		// Unlock All Floors
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		US_CenterWindow(24, 3);
		US_PrintCentered("Unlock All Floors!");
		VW_UpdateScreen();
		IN_Ack();

		const auto& assets_info = get_assets_info();
		const auto stats_levels_per_episode = assets_info.get_stats_levels_per_episode();

		for (auto i = 0; i < stats_levels_per_episode; ++i)
		{
			gamestuff.level[i].locked = false;
		}

		return true;
	}
	else if (Keyboard[ScanCode::sc_s])
	{
		// S = slow motion
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		singlestep = !singlestep;
		US_CenterWindow(18, 3);

		if (singlestep)
		{
			US_PrintCentered("Slow motion ON");
		}
		else
		{
			US_PrintCentered("Slow motion OFF");
		}

		VW_UpdateScreen();
		IN_Ack();

		return true;
	}
	else if (Keyboard[ScanCode::sc_w])
	{
		// W = warp to level
		//
		sd_pause_scene_sfx(true);
		unmute_scene_sfx = true;

		ForceLoadDefault =
			Keyboard[ScanCode::sc_left_shift] ||
			Keyboard[ScanCode::sc_right_shift] ||
			Keyboard[ScanCode::sc_caps_lock];

		US_CenterWindow(26, 5);

		PrintY += 6;

		if (ForceLoadDefault)
		{
			US_Print("         --- LOAD DEFAULT ---\n");
		}

		US_Print("  Current map: ");
		US_PrintUnsigned(gamestate.mapon);
		US_Print("\n  Enter map number: ");
		VW_UpdateScreen();

		char string[3];

		const auto esc = !US_LineInput(px, py, string, nullptr, true, 2, 0);

		if (!esc && string[0] != '\0')
		{
			const auto& assets_info = get_assets_info();

			const auto max_warp_level = assets_info.get_levels_per_episode() - 1;
			const auto level = atoi(string);

// FIXME
#if 0
			if (level > -1 && level <= max_warp_level)
#else
			if (level >= 0 && level <= max_warp_level)
#endif
			{
				if (mapheaderseg[level].name[0] != '\0')
				{
					gamestate.lastmapon = gamestate.mapon;
					playstate = ex_warped;

					if (ForceLoadDefault)
					{
						BONUS_QUEUE = 0;
						BONUS_SHOWN = 0;
					}

// FIXME
#if 0
					gamestate.mapon = static_cast<std::int16_t>(level - 1);
#else
					gamestate.mapon = static_cast<std::int16_t>(level);
#endif
				}
			}
		}

		return true;
	}
	else if (Keyboard[ScanCode::sc_home])
	{
		// Dec top color
		//

		if (!gp_is_ceiling_solid())
		{
			CeilingTile = DecRange(CeilingTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			TopColor = DecRange((TopColor & 0xFF), 0xFF);
			TopColor |= (TopColor << 8);
		}
	}
	else if (Keyboard[ScanCode::sc_page_up])
	{
		// Inc top color
		//

		if (!gp_is_ceiling_solid())
		{
			CeilingTile = IncRange(CeilingTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			TopColor = IncRange((TopColor & 0xFF), 0xFF);
			TopColor |= (TopColor << 8);
		}
	}
	else if (Keyboard[ScanCode::sc_end])
	{
		// Dec bottom color
		//

		if (!gp_is_flooring_solid())
		{
			FloorTile = DecRange(FloorTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			BottomColor = DecRange((BottomColor & 0xFF), 0xFF);
			BottomColor |= (BottomColor << 8);
		}
	}
	else if (Keyboard[ScanCode::sc_page_down])
	{
		// Inc bottom color
		//

		if (!gp_is_flooring_solid())
		{
			FloorTile = IncRange(FloorTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			BottomColor = IncRange((BottomColor & 0xFF), 0xFF);
			BottomColor |= (BottomColor << 8);
		}
	}

	if (!gp_no_shading())
	{
		// Shading adjustments
		//

		if (Keyboard[ScanCode::sc_equals] && normalshade_div < 12)
		{
			normalshade_div += 1;
		}
		else if (Keyboard[ScanCode::sc_minus] && normalshade_div > 1)
		{
			normalshade_div -= 1;
		}

		if (Keyboard[ScanCode::sc_right_bracket] && shade_max < 63)
		{
			shade_max += 1;
		}
		else if (Keyboard[ScanCode::sc_left_bracket] && shade_max > 5)
		{
			shade_max -= 1;
		}

		update_normalshade();
	}

	return false;
}
