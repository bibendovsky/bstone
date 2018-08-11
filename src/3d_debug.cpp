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
    int16_t points,
    objtype* attacker);

void SetPlaneViewSize();

void HealSelf(
    int16_t points);

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

int16_t DebugKeys();


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

bool PP_step = false;

int16_t maporgx;
int16_t maporgy;

void ViewMap();


void DebugMemory()
{
    ::CenterWindow(22, 15);

    ::US_Print("k\nTics      :");
    ::US_PrintUnsigned(tics);
    ::US_Print("\nReal Tics :");
    ::US_PrintUnsigned(realtics);

    if ((gamestate.flags & GS_DRAW_CEILING) != 0) {
        ::US_Print("\n\nCeiling TEX: ");
        ::US_PrintUnsigned(CeilingTile - START_TEXTURES);

        ::US_Print(" Floor TEX: ");
        ::US_PrintUnsigned(FloorTile - START_TEXTURES);
    } else {
        ::US_Print("\n\nTop COL: ");
        ::US_PrintUnsigned(TopColor & 0xFF);

        ::US_Print(" Bottom COL: ");
        ::US_PrintUnsigned(BottomColor & 0xFF);
    }

    if ((gamestate.flags & GS_LIGHTING) != 0) {
        ::US_Print("\nShade div :");
        ::US_PrintUnsigned(normalshade_div);

        ::US_Print("\nShade max :");
        ::US_PrintUnsigned(shade_max);
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
    int16_t i, total, count, active, inactive, doors;
    objtype* obj;

    CenterWindow(16, 7);
    active = inactive = count = doors = 0;

    US_Print("Total statics :");
    total = static_cast<int16_t>(laststatobj - &statobjlist[0]);
    US_PrintUnsigned(total);

    US_Print("\nIn use statics:");
    for (i = 0; i < total; i++) {
        if (statobjlist[i].shapenum != -1) {
            count++;
        } else {
            doors++; // debug
        }
    }
    US_PrintUnsigned(count);

    US_Print("\nDoors         :");
    US_PrintUnsigned(doornum);

    for (obj = player->next; obj; obj = obj->next) {
        if (obj->active) {
            active++;
        } else {
            inactive++;
        }
    }

    US_Print("\nTotal actors  :");
    US_PrintUnsigned(active + inactive);

    US_Print("\nActive actors :");
    US_PrintUnsigned(active);

    VW_UpdateScreen();
    IN_Ack();
}

void CountTotals()
{
    CenterWindow(20, 11);

    US_Print("  CURRENT MAP TOTALS\n");

    US_Print("\nTotal Enemy:\n");
    US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_enemy);

    US_Print("\nTotal Points:\n");
    US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_points);

    US_Print("\nTotal Informants:\n");
    US_PrintUnsigned(gamestuff.level[gamestate.mapon].stats.total_inf);

    VW_UpdateScreen();
    IN_Ack();
}

void ShowMap()
{
    objtype old_player;

    memcpy(&old_player, player, sizeof(objtype));
    player->angle = 90;
    player->x = player->y = ((int32_t)32 << TILESHIFT) + (TILEGLOBAL / 2);

    CenterWindow(20, 11);

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
uint16_t IncRange(
    uint16_t Value,
    uint16_t MaxValue)
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
uint16_t DecRange(
    uint16_t Value,
    uint16_t MaxValue)
{
    if (Value == 0) {
        Value = MaxValue;
    } else {
        Value--;
    }

    return Value;
}


int16_t DebugKeys()
{
    char string[3];
    bool esc;
    int16_t level;

    if (Keyboard[ScanCode::sc_a]) {       // A = Show Actors on AutoMap
        ExtraRadarFlags ^= OV_ACTORS;
        CenterWindow(24, 3);
        if (ExtraRadarFlags & OV_ACTORS) {
            US_PrintCentered("AUTOMAP: Show Actors ON");
        } else {
            US_PrintCentered("AUTOMAP: Show Actors OFF");
        }
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    }

    if (Keyboard[ScanCode::sc_k]) { // K = Map Content totals
        CountTotals();
        return 1;
    } else if (Keyboard[ScanCode::sc_c]) { // C = count objects
        CountObjects();
        return 1;
    } else if (Keyboard[ScanCode::sc_r]) { // R = show full map
        ShowMap();
        return 1;
    } else if (Keyboard[ScanCode::sc_d]) { // D = Dumb/Blind Objects (Player Invisable)
        CenterWindow(19, 3);
        ::PlayerInvisable = !::PlayerInvisable;
        if (PlayerInvisable) {
            US_PrintCentered("Player Invisible!");
        } else {
            US_PrintCentered("Player visible");
        }

        VW_UpdateScreen();
        IN_Ack();
        return 1;
    } else if (Keyboard[ScanCode::sc_e]) { // E = Win Mission
        CenterWindow(19, 3);
        US_PrintCentered("Instant Wiener!");
        InstantWin = 1;
        playstate = ex_victorious;
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    } else if (Keyboard[ScanCode::sc_f]) { // F = facing spot
        CenterWindow(18, 5);
        US_Print("X:");
        US_PrintUnsigned(player->x);
        US_Print("  ");
        US_PrintUnsigned(player->x >> TILESHIFT);
        US_Print("\nY:");
        US_PrintUnsigned(player->y);
        US_Print("  ");
        US_PrintUnsigned(player->y >> TILESHIFT);
        US_Print("\nA:");
        US_PrintUnsigned(player->angle);
        US_Print("\nD:");
        US_PrintUnsigned(player->dir);
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    }

    if (Keyboard[ScanCode::sc_g]) { // G = god mode
        CenterWindow(12, 2);
        if (godmode) {
            US_PrintCentered("God mode OFF");
        } else {
            US_PrintCentered("God mode ON");
        }
        VW_UpdateScreen();
        IN_Ack();
        ::godmode = !::godmode;
        return 1;
    }


    if (Keyboard[ScanCode::sc_h]) { // H = hurt self
        IN_ClearKeysDown();
        TakeDamage(1, nullptr);
    } else if (Keyboard[ScanCode::sc_i]) { // I = item cheat
        CenterWindow(12, 3);
        US_PrintCentered("Free items!");
        VW_UpdateScreen();
        HealSelf(99);
        GiveToken(5);

        const auto n = static_cast<int16_t>(::is_ps() ? wp_bfg_cannon : wp_grenade);

        for (int i = wp_autocharge; i <= n; i++) {
            if (!(gamestate.weapons & (1 << i))) {
                GiveWeapon(i);
                break;
            }
        }

        gamestate.ammo += 50;
        if (gamestate.ammo > MAX_AMMO) {
            gamestate.ammo = MAX_AMMO;
        }
        DrawAmmo(true);
        DrawScore();
        IN_Ack();
        return 1;
    } else if (Keyboard[ScanCode::sc_m]) { // M = memory info
        DebugMemory();
        return 1;
    }
    else if (Keyboard[ScanCode::sc_q]) { // Q = fast quit
        Quit();
    }
    else if (Keyboard[ScanCode::sc_o]) { // O = Show Push Walls
        ExtraRadarFlags ^= OV_PUSHWALLS;
        CenterWindow(24, 3);
        if (ExtraRadarFlags & OV_PUSHWALLS) {
            US_PrintCentered("AUTOMAP: Show PWalls ON");
        } else {
            US_PrintCentered("AUTOMAP: Show PWalls OFF");
        }
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    } else if (Keyboard[ScanCode::sc_u]) { // Unlock All Floors
        CenterWindow(24, 3);
        US_PrintCentered("Unlock All Floors!");
        VW_UpdateScreen();
        IN_Ack();

        for (int i = 0; i < ::MAPS_WITH_STATS; ++i) {
            gamestuff.level[i].locked = false;
        }

        return 1;
    } else if (Keyboard[ScanCode::sc_s]) { // S = slow motion
        ::singlestep = !::singlestep;
        CenterWindow(18, 3);
        if (singlestep) {
            US_PrintCentered("Slow motion ON");
        } else {
            US_PrintCentered("Slow motion OFF");
        }
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    } else if (Keyboard[ScanCode::sc_w]) { // W = warp to level
        ForceLoadDefault = Keyboard[ScanCode::sc_left_shift] | Keyboard[ScanCode::sc_right_shift] | Keyboard[ScanCode::sc_caps_lock];

        CenterWindow(26, 5);
        PrintY += 6;
        if (ForceLoadDefault) {
            US_Print("         --- LOAD DEFAULT ---\n");
        }
        US_Print("  Current map: ");
        US_PrintUnsigned(gamestate.mapon);
        US_Print("\n  Enter map number: ");
        VW_UpdateScreen();
        esc = !US_LineInput(px, py, string, nullptr, true, 2, 0);
        if (!esc && string[0] != '\0') {
            const int MAX_WARP_LEVEL = (::is_aog() ? 10 : 23);
            level = static_cast<int16_t>(atoi(string));
            if (level > -1 && level <= MAX_WARP_LEVEL) {
                gamestate.lastmapon = gamestate.mapon;
                playstate = ex_warped;
                if (ForceLoadDefault) {
                    BONUS_QUEUE = 0;
                    BONUS_SHOWN = 0;
                }
                gamestate.mapon = level - 1;
            }
        }
        return 1;
    } else if (Keyboard[ScanCode::sc_home]) { // Dec top color
        if (gamestate.flags & GS_DRAW_CEILING) {
            CeilingTile = DecRange(CeilingTile, static_cast<uint16_t>(NUM_TILES - 1));
            SetPlaneViewSize(); // Init new textures
            return 1;
        } else {
            TopColor = DecRange((TopColor & 0xff), 0xff);
            TopColor |= (TopColor << 8);
        }
    } else if (Keyboard[ScanCode::sc_page_up]) { // Inc top color
        if (gamestate.flags & GS_DRAW_CEILING) {
            CeilingTile = IncRange(CeilingTile, static_cast<uint16_t>(NUM_TILES - 1));
            SetPlaneViewSize(); // Init new textures
            return 1;
        } else {
            TopColor = IncRange((TopColor & 0xff), 0xff);
            TopColor |= (TopColor << 8);
        }
    } else if (Keyboard[ScanCode::sc_end]) { // Dec bottom color
        if (gamestate.flags & GS_DRAW_FLOOR) {
            FloorTile = DecRange(FloorTile, static_cast<uint16_t>(NUM_TILES - 1));
            SetPlaneViewSize(); // Init new textures
            return 1;
        } else {
            BottomColor = DecRange((BottomColor & 0xff), 0xff);
            BottomColor |= (BottomColor << 8);
        }
    } else if (Keyboard[ScanCode::sc_page_down]) { // Inc bottom color
        if (gamestate.flags & GS_DRAW_FLOOR) {
            FloorTile = IncRange(FloorTile, static_cast<uint16_t>(NUM_TILES - 1));
            SetPlaneViewSize(); // Init new textures
            return 1;
        } else {
            BottomColor = IncRange((BottomColor & 0xff), 0xff);
            BottomColor |= (BottomColor << 8);
        }
    }

    if (gamestate.flags & GS_LIGHTING) { // Shading adjustments
        if (Keyboard[ScanCode::sc_equals] && normalshade_div < 12) {
            normalshade_div++;
        } else if (Keyboard[ScanCode::sc_minus] && normalshade_div > 1) {
            normalshade_div--;
        }

        if (Keyboard[ScanCode::sc_right_bracket] && shade_max < 63) {
            shade_max++;
        } else if (Keyboard[ScanCode::sc_left_bracket] && shade_max > 5) {
            shade_max--;
        }

        ::update_normalshade();
    }

    return 0;
}
