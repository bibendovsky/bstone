/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "3d_def.h"


void VH_UpdateScreen();

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
	::US_CenterWindow(22, 15);

	::US_Print("\nTics: ");
	::US_PrintUnsigned(::tics);

	::US_Print("\nReal Tics: ");
	::US_PrintUnsigned(::realtics);

	if ((::gamestate.flags & GS_DRAW_CEILING) != 0)
	{
		::US_Print("\n\nCeiling TEX: ");
		::US_PrintUnsigned(::CeilingTile - START_TEXTURES);

		::US_Print("\nFloor TEX: ");
		::US_PrintUnsigned(::FloorTile - START_TEXTURES);
	}
	else
	{
		::US_Print("\n\nTop COL: ");
		::US_PrintUnsigned(::TopColor & 0xFF);

		::US_Print("\nBottom COL: ");
		::US_PrintUnsigned(::BottomColor & 0xFF);
	}

	if ((::gamestate.flags & GS_LIGHTING) != 0)
	{
		::US_Print("\nShade div: ");
		::US_PrintUnsigned(::normalshade_div);

		::US_Print("\nShade max: ");
		::US_PrintUnsigned(::shade_max);
	}

	VW_UpdateScreen();
	::IN_Ack();

	::WindowW = 253;
	::WindowH = 8;
	::fontnumber = 2;

	::LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	::ShadowPrintLocationText(sp_normal);
}

void CountObjects()
{
	::US_CenterWindow(16, 7);

	const auto total_statics = static_cast<int>(::laststatobj - &::statobjlist[0]);
	::US_Print("Total statics: ");
	::US_PrintUnsigned(total_statics);

	auto in_use_static_count = 0;

	for (auto i = 0; i < total_statics; ++i)
	{
		if (::statobjlist[i].shapenum != -1)
		{
			in_use_static_count += 1;
		}
	}

	::US_Print("\nIn use statics: ");
	::US_PrintUnsigned(in_use_static_count);

	::US_Print("\nDoors: ");
	::US_PrintUnsigned(::doornum);

	auto active_static_count = 0;
	auto inactive_static_count = 0;

	for (auto obj = ::player->next; obj; obj = obj->next)
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

	::US_Print("\nTotal actors: ");
	::US_PrintUnsigned(active_static_count + inactive_static_count);

	::US_Print("\nActive actors: ");
	::US_PrintUnsigned(active_static_count);

	VW_UpdateScreen();
	::IN_Ack();
}

void CountTotals()
{
	::US_CenterWindow(20, 11);

	::US_Print("CURRENT MAP TOTALS\n");

	::US_Print("\nTotal Enemy: ");
	::US_PrintUnsigned(::gamestuff.level[gamestate.mapon].stats.total_enemy);

	::US_Print("\nTotal Points: ");
	::US_PrintUnsigned(::gamestuff.level[gamestate.mapon].stats.total_points);

	::US_Print("\nTotal Informants: ");
	::US_PrintUnsigned(::gamestuff.level[gamestate.mapon].stats.total_inf);

	VW_UpdateScreen();
	::IN_Ack();
}

void ShowMap()
{
    objtype old_player;

    memcpy(&old_player, player, sizeof(objtype));
    player->angle = 90;
    player->x = player->y = ((std::int32_t)32 << TILESHIFT) + (TILEGLOBAL / 2);

    ::US_CenterWindow(20, 11);

    US_CPrint("CURRENT MAP\n\n ");

    auto old_flags = ::ExtraRadarFlags;
    ::ExtraRadarFlags |= OV_ACTORS | OV_PUSHWALLS;

    ShowOverhead(160 - 32, py, 32, 0, OV_ACTORS | OV_SHOWALL | OV_KEYS | OV_PUSHWALLS);
    VW_UpdateScreen();

    ::ExtraRadarFlags = old_flags;

    memcpy(player, &old_player, sizeof(objtype));
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
    if (Value == MaxValue) {
        Value = 0;
    } else {
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
    if (Value == 0) {
        Value = MaxValue;
    } else {
        Value--;
    }

    return Value;
}


bool DebugKeys()
{
	if (::Keyboard[ScanCode::sc_a])
	{
		// A = Show Actors on AutoMap
		//

		::ExtraRadarFlags ^= OV_ACTORS;
		::US_CenterWindow(24, 3);

		if ((::ExtraRadarFlags & OV_ACTORS) != 0)
		{
			::US_PrintCentered("AUTOMAP: Show Actors ON");
		}
		else
		{
			::US_PrintCentered("AUTOMAP: Show Actors OFF");
		}

		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}

	if (::Keyboard[ScanCode::sc_k])
	{
		// K = Map Content totals
		//

		::CountTotals();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_c])
	{
		// C = count objects
		//

		::CountObjects();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_r])
	{
		// R = show full map
		//

		::ShowMap();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_d])
	{
		// D = Dumb/Blind Objects (Player Invisable)
		//

		::US_CenterWindow(19, 3);
		::PlayerInvisable = !::PlayerInvisable;

		if (::PlayerInvisable)
		{
			::US_PrintCentered("Player Invisible!");
		}
		else
		{
			::US_PrintCentered("Player visible");
		}

		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_e])
	{
		// E = Win Mission
		//

		::US_CenterWindow(19, 3);
		::US_PrintCentered("Instant Winner!");
		::InstantWin = 1;
		::playstate = ex_victorious;
		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_f])
	{
		// F = facing spot
		//

		::US_CenterWindow(18, 5);

		::US_Print("X: ");
		::US_PrintUnsigned(::player->x >> TILESHIFT);
		::US_Print(" (");
		::US_PrintUnsigned(::player->x);
		::US_Print(")");

		::US_Print("\nY: ");
		::US_PrintUnsigned(::player->y >> TILESHIFT);
		::US_Print(" (");
		::US_PrintUnsigned(::player->y);
		::US_Print(")");

		::US_Print("\nA: ");
		::US_PrintUnsigned(::player->angle);

		::US_Print("\nD: ");
		::US_PrintUnsigned(::player->dir);

		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}

	if (::Keyboard[ScanCode::sc_g])
	{
		// G = god mode
		//

		::US_CenterWindow(12, 2);

		if (::godmode)
		{
			::US_PrintCentered("God mode OFF");
		}
		else
		{
			::US_PrintCentered("God mode ON");
		}

		VW_UpdateScreen();
		::IN_Ack();
		::godmode = !::godmode;

		return true;
	}


	if (::Keyboard[ScanCode::sc_h])
	{
		// H = hurt self
		//

		::IN_ClearKeysDown();
		::TakeDamage(1, nullptr);
	}
	else if (::Keyboard[ScanCode::sc_i])
	{
		// I = item cheat
		//

		::US_CenterWindow(12, 3);
		::US_PrintCentered("Free items!");
		VW_UpdateScreen();
		::HealSelf(99);
		::GiveToken(5);

		const auto& assets_info = AssetsInfo{};

		const auto n = static_cast<int>(assets_info.is_ps() ? wp_bfg_cannon : wp_grenade);

		for (auto i = static_cast<int>(wp_autocharge); i <= n; ++i)
		{
			if ((::gamestate.weapons & (1 << i)) == 0)
			{
				::GiveWeapon(i);
				break;
			}
		}

		::gamestate.ammo += 50;

		if (::gamestate.ammo > MAX_AMMO)
		{
			::gamestate.ammo = MAX_AMMO;
		}

		::DrawAmmo(true);
		::DrawScore();
		::IN_Ack();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_m])
	{
		// M = memory info
		//

		::DebugMemory();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_q])
	{
		// Q = fast quit
		//

		::Quit();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_o])
	{
		// O = Show Push Walls
		//

		::ExtraRadarFlags ^= OV_PUSHWALLS;
		::US_CenterWindow(24, 3);

		if ((::ExtraRadarFlags & OV_PUSHWALLS) != 0)
		{
			::US_PrintCentered("AUTOMAP: Show PWalls ON");
		}
		else
		{
			::US_PrintCentered("AUTOMAP: Show PWalls OFF");
		}

		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_u])
	{
		// Unlock All Floors
		//

		::US_CenterWindow(24, 3);
		::US_PrintCentered("Unlock All Floors!");
		VW_UpdateScreen();
		::IN_Ack();

		for (auto i = 0; i < ::MAPS_WITH_STATS; ++i)
		{
			::gamestuff.level[i].locked = false;
		}

		return true;
	}
	else if (::Keyboard[ScanCode::sc_s])
	{
		// S = slow motion
		//

		::singlestep = !::singlestep;
		::US_CenterWindow(18, 3);

		if (::singlestep)
		{
			::US_PrintCentered("Slow motion ON");
		}
		else
		{
			::US_PrintCentered("Slow motion OFF");
		}

		VW_UpdateScreen();
		::IN_Ack();

		return true;
	}
	else if (::Keyboard[ScanCode::sc_w])
	{
		// W = warp to level
		//

		::ForceLoadDefault =
			Keyboard[ScanCode::sc_left_shift] |
			Keyboard[ScanCode::sc_right_shift] |
			Keyboard[ScanCode::sc_caps_lock];

		::US_CenterWindow(26, 5);

		::PrintY += 6;

		if (::ForceLoadDefault)
		{
			::US_Print("         --- LOAD DEFAULT ---\n");
		}

		::US_Print("  Current map: ");
		::US_PrintUnsigned(gamestate.mapon);
		::US_Print("\n  Enter map number: ");
		VW_UpdateScreen();

		char string[3];

		const auto esc = !::US_LineInput(px, py, string, nullptr, true, 2, 0);

		if (!esc && string[0] != '\0')
		{
			const auto& assets_info = AssetsInfo{};

			const auto max_warp_level = (assets_info.is_aog() ? 10 : 23);
			const auto level = ::atoi(string);

			if (level > -1 && level <= max_warp_level)
			{
				::gamestate.lastmapon = ::gamestate.mapon;
				::playstate = ex_warped;

				if (::ForceLoadDefault)
				{
					BONUS_QUEUE = 0;
					BONUS_SHOWN = 0;
				}

				::gamestate.mapon = static_cast<std::int16_t>(level - 1);
			}
		}

		return true;
	}
	else if (::Keyboard[ScanCode::sc_home])
	{
		// Dec top color
		//

		if ((::gamestate.flags & GS_DRAW_CEILING) != 0)
		{
			::CeilingTile = ::DecRange(::CeilingTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			::SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			::TopColor = ::DecRange((::TopColor & 0xFF), 0xFF);
			::TopColor |= (::TopColor << 8);
		}
	}
	else if (::Keyboard[ScanCode::sc_page_up])
	{
		// Inc top color
		//

		if ((::gamestate.flags & GS_DRAW_CEILING) != 0)
		{
			::CeilingTile = ::IncRange(::CeilingTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			::SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			::TopColor = ::IncRange((::TopColor & 0xFF), 0xFF);
			::TopColor |= (::TopColor << 8);
		}
	}
	else if (::Keyboard[ScanCode::sc_end])
	{
		// Dec bottom color
		//

		if ((::gamestate.flags & GS_DRAW_FLOOR) != 0)
		{
			::FloorTile = ::DecRange(::FloorTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			::SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			::BottomColor = ::DecRange((::BottomColor & 0xFF), 0xFF);
			::BottomColor |= (::BottomColor << 8);
		}
	}
	else if (::Keyboard[ScanCode::sc_page_down])
	{
		// Inc bottom color
		//

		if ((::gamestate.flags & GS_DRAW_FLOOR) != 0)
		{
			::FloorTile = ::IncRange(::FloorTile, static_cast<std::uint16_t>(NUM_TILES - 1));
			::SetPlaneViewSize(); // Init new textures

			return true;
		}
		else
		{
			::BottomColor = ::IncRange((::BottomColor & 0xFF), 0xFF);
			::BottomColor |= (::BottomColor << 8);
		}
	}

	if ((::gamestate.flags & GS_LIGHTING) != 0)
	{
		// Shading adjustments
		//

		if (::Keyboard[ScanCode::sc_equals] && ::normalshade_div < 12)
		{
			::normalshade_div += 1;
		}
		else if (::Keyboard[ScanCode::sc_minus] && ::normalshade_div > 1)
		{
			::normalshade_div -= 1;
		}

		if (::Keyboard[ScanCode::sc_right_bracket] && ::shade_max < 63)
		{
			::shade_max += 1;
		}
		else if (::Keyboard[ScanCode::sc_left_bracket] && ::shade_max > 5)
		{
			::shade_max -= 1;
		}

		::update_normalshade();
	}

	return false;
}
