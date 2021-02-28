/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_logger.h"


#define NUM_TILES (PMSpriteStart)


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

	if (!gp_is_ceiling_solid_)
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

	if (!gp_no_shading_)
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
	ShowOverhead(160 - 32, py, 32, 0, OV_ACTORS | OV_SHOWALL | OV_KEYS | OV_PUSHWALLS);
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

void log_bonus_stuff()
{
	bstone::logger_->write();
	bstone::logger_->write("<<<<<<<<");
	bstone::logger_->write("Current bonus items.");

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

		if (false)
		{
		}
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
			static_name = "One Gold Bar";
		}
		else if (bs_static.itemnumber == bo_gold2)
		{
			static_name = "Two Gold Bars";
		}
		else if (bs_static.itemnumber == bo_gold3)
		{
			static_name = "Three Gold Bars";
		}
		else if (bs_static.itemnumber == bo_gold)
		{
			static_name = "Five Gold Bars";
		}
		else if (bs_static.itemnumber == bo_bonus)
		{
			static_name = "Xylan Orb";
		}
		else
		{
			static_name.clear();
		}

		if (!static_name.empty())
		{
			number += 1;

			bstone::logger_->write(
				std::to_string(number) +
					". (" +
					std::to_string(bs_static.tilex) +
					", " +
					std::to_string(bs_static.tiley) +
					") " +
					static_name
			);
		}
	}

	bstone::logger_->write(">>>>>>>>");
}

void log_enemy_stuff()
{
	const auto& assets_info = get_assets_info();

	bstone::logger_->write();
	bstone::logger_->write("<<<<<<<<");
	bstone::logger_->write("Current enemies.");

	auto number = 0;
	auto actor_name = std::string{};

	for (auto bs_actor = objlist; bs_actor != nullptr; bs_actor = bs_actor->next)
	{
		if (bs_actor->hitpoints <= 0)
		{
			continue;
		}

		if ((bs_actor->flags & FL_DEADGUY) != 0)
		{
			continue;
		}

		const auto is_informant = ((bs_actor->flags & FL_INFORMANT) != 0);

		actor_name.clear();

		switch (bs_actor->obclass)
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
				actor_name = "High Enemy Plasma Alien (non-countable)";
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
				actor_name = "High Security Genetic Guard";
				break;

			case mutant_human1obj:
				actor_name = "Experimental Mech Sentinel";
				break;

			case mutant_human2obj:
				actor_name = "Experimental Mutant Human";
				break;

			case lcan_wait_alienobj:
				break;

			case lcan_alienobj:
				actor_name = "Large Experimental Genetic Alien";
				break;

			case scan_wait_alienobj:
				break;

			case scan_alienobj:
				actor_name = "Small Experimental Genetic Alien";
				break;

			case gurney_waitobj:
				break;

			case gurneyobj:
				actor_name = "Mutated Guard";
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
				actor_name = "Dr. Goldfire (non-countable)";
				break;

			case gold_morphobj:
				actor_name = "Morphed Dr. Goldfire";
				break;

			case volatiletransportobj:
				actor_name = "Volatile Material Transport";
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
				actor_name = "Spider Mutant";
				break;

			case breather_beastobj:
				actor_name = "Breather Beast";
				break;

			case cyborg_warriorobj:
				actor_name = "Cyborg Warrior";
				break;

			case reptilian_warriorobj:
				actor_name = "Reptilian Warrior";
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

			default:
				break;
		}

		if (!actor_name.empty())
		{
			number += 1;

			bstone::logger_->write(
				std::to_string(number) +
					". (" +
					std::to_string(bs_actor->tilex) +
					", " +
					std::to_string(bs_actor->tiley) +
					") " +
					actor_name
			);
		}
	}

	bstone::logger_->write(">>>>>>>>");
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

bool DebugKeys()
{
	if (Keyboard[ScanCode::sc_l])
	{
		log_stuff();
		return true;
	}

	if (Keyboard[ScanCode::sc_a])
	{
		// A = Show Actors on AutoMap
		//

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

		CountTotals();

		return true;
	}
	else if (Keyboard[ScanCode::sc_c])
	{
		// C = count objects
		//

		CountObjects();

		return true;
	}
	else if (Keyboard[ScanCode::sc_r])
	{
		// R = show full map
		//

		ShowMap();

		return true;
	}
	else if (Keyboard[ScanCode::sc_d])
	{
		// D = Dumb/Blind Objects (Player Invisable)
		//

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

		ForceLoadDefault =
			Keyboard[ScanCode::sc_left_shift] |
			Keyboard[ScanCode::sc_right_shift] |
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

		if (!gp_is_ceiling_solid_)
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

		if (!gp_is_ceiling_solid_)
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

		if (!gp_is_flooring_solid_)
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

		if (!gp_is_flooring_solid_)
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

	if (!gp_no_shading_)
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
