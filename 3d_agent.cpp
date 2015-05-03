/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


// 3D_AGENT.C


#include "3d_def.h"


void InitWeaponBounce();
void HandleWeaponBounce();
void VL_LatchToScreen(
    int source,
    int width,
    int height,
    int x,
    int y);
void StartDamageFlash(
    int damage);
void StartBonusFlash();
int16_t CalcAngle(
    objtype* from_obj,
    objtype* to_obj);
void PushWall(
    int16_t checkx,
    int16_t checky,
    int16_t dir);
void OperateDoor(
    int16_t door);

void TryDropPlasmaDetonator();

void ClearMemory();
void VH_UpdateScreen();
void InitAreas();
void FirstSighting(
    objtype* ob);
void OpenDoor(
    int16_t door);
void DrawTopInfo(
    sp_type type);
void DoActor(
    objtype* ob);

void RunBlakeRun();

#define VW_UpdateScreen() VH_UpdateScreen()

int16_t DrawShape(
    int16_t x,
    int16_t y,
    int16_t shapenum,
    pisType shapetype);


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/

// #define ACTIVATE_TERMINAL

#define MAXMOUSETURN 10

#define MOVESCALE 150l
#define BACKMOVESCALE 100l
#define ANGLESCALE 20
#define MAX_DA 100

#define MAX_TERM_COMMAND_LEN 31

// Max Score displayable

#define MAX_DISPLAY_SCORE (9999999L)
#define SCORE_ROLL_WAIT (60 * 10) // Tics

// IFDEF switches

// #define NO_STATUS_BAR

// ECG scroll rate (delay).

#define HEALTH_SCROLL_RATE 7
#define HEALTH_PULSE 70

// Text "InfoArea" defines
#define INFOAREA_X 3
#define INFOAREA_Y ((uint16_t)200 - STATUSLINES + 3)
#define INFOAREA_W 109
#define INFOAREA_H 37

#define INFOAREA_BCOLOR 0x01
#define INFOAREA_CCOLOR 0x1A
#define INFOAREA_TCOLOR 0xA6
#define INFOAREA_TSHAD_COLOR 0x04 // Text Shadow Color

#define GRENADE_ENERGY_USE 4
#define BFG_ENERGY_USE (GRENADE_ENERGY_USE << 1)


#define NUM_AMMO_SEGS 21


#define AMMO_SMALL_FONT_NUM_WIDTH 5

/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

extern boolean noShots;
extern int16_t bounceOk;

int16_t tryDetonatorDelay = 0;

//
// player state info
//
int32_t thrustspeed;

// unsigned plux,pluy; // player coordinates scaled to unsigned

int16_t anglefrac;

objtype* LastAttacker;

boolean PlayerInvisable = false;

char LocationText[MAX_LOCATION_DESC_LEN];

#ifdef ACTIVATE_TERMINAL
char term_com_name[13] = { "TERM_CMD." };
char term_msg_name[13] = { "TERM_MSG." };
#endif

uint16_t player_oldtilex;
uint16_t player_oldtiley;

// BBi
extern bstone::MemoryStream g_playtemp;

/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

void writeTokenStr(
    std::string& str);

void ShowOverheadChunk();

void LoadOverheadChunk(
    int tpNum);
void SaveOverheadChunk(
    int tpNum);

void DisplayTeleportName(
    char tpNum,
    boolean locked);

void ForceUpdateStatusBar();

void UpdateRadarGuage();
void DrawLedStrip(
    int16_t x,
    int16_t y,
    int16_t frac,
    int16_t max);
void DisplayPinballBonus();
void CheckPinballBonus(
    int32_t points);
uint8_t LevelCompleted();
void T_Player(
    objtype* ob);
void T_Attack(
    objtype* ob);

statetype s_player = { 0, 0, 0, &T_Player, nullptr, nullptr };
statetype s_attack = { 0, 0, 0, &T_Attack, nullptr, nullptr };

int32_t playerxmove, playerymove;

atkinf_t attackinfo[7][14] = {
    { { 6, 0, 1 }, { 6, 2, 2 }, { 6, 0, 3 }, { 6, -1, 4 } }, // Auto charge
    { { 6, 0, 1 }, { 6, 1, 2 }, { 6, 0, 3 }, { 6, -1, 4 } }, // Pistol
    { { 6, 0, 1 }, { 6, 1, 2 }, { 5, 3, 3 }, { 5, -1, 4 } }, // Pulse
    { { 6, 0, 1 }, { 6, 1, 2 }, { 3, 4, 3 }, { 3, -1, 4 } }, // ION
    { { 6, 0, 1 }, { 6, 5, 2 }, { 6, 6, 3 }, { 6, -1, 4 } },
    { { 6, 0, 1 }, { 6, 9, 2 }, { 6, 10, 3 }, { 6, -1, 4 } },
    { { 5, 7, 0 }, { 5, 8, 0 }, { 2, -2, 0 }, { 0, 0, 0 } },
};

// int  strafeangle[9] = {0,90,180,270,45,135,225,315,0};

#define GD0 0x55
#define YD0 0x35
#define RD0 0x15

#define GD1 0x53
#define YD1 0x33
#define RD1 0x13

char DimAmmo[2][22] = { { GD0, GD0, GD0, GD0, GD0, GD0, GD0, YD0, YD0, YD0, YD0, YD0, YD0, YD0, RD0, RD0, RD0, RD0, RD0, RD0, RD0, RD0 },
                        { GD1, GD1, GD1, GD1, GD1, GD1, GD1, YD1, YD1, YD1, YD1, YD1, YD1, YD1, RD1, RD1, RD1, RD1, RD1, RD1, RD1, RD1 } };

#define GL0 0x58
#define YL0 0x38
#define RL0 0x18

#define GL1 0x56
#define YL1 0x36
#define RL1 0x16

char LitAmmo[2][22] = { { GL0, GL0, GL0, GL0, GL0, GL0, GL0, YL0, YL0, YL0, YL0, YL0, YL0, YL0, RL0, RL0, RL0, RL0, RL0, RL0, RL0, RL0 },
                        { GL1, GL1, GL1, GL1, GL1, GL1, GL1, YL1, YL1, YL1, YL1, YL1, YL1, YL1, RL1, RL1, RL1, RL1, RL1, RL1, RL1, RL1 } };


#define IA_MAX_LINE 30
struct InfoArea_Struct {
    int16_t x, y;
    int16_t text_color;
    int16_t backgr_color;
    int16_t left_margin;
    char delay;
    char numanims;
    char framecount;
}; // InfoArea_Struct

uint16_t LastMsgPri = 0;
int16_t MsgTicsRemain = 0;
classtype LastInfoAttacker = nothing;

int16_t LastInfoAttacker_Cloaked = 0;

infomsg_type LastMsgType = MT_NOTHING;
InfoArea_Struct InfoAreaSetup;

char DrawRadarGuage_COUNT = 3;
char DrawAmmoNum_COUNT = 3;
char DrawAmmoPic_COUNT = 3;
// char DrawPDAmmoPic_COUNT = 3;
char DrawScoreNum_COUNT = 3;
char DrawWeaponPic_COUNT = 3;
char DrawKeyPics_COUNT = 3;
char DrawHealthNum_COUNT = 3;

char DrawInfoArea_COUNT = 3;
char InitInfoArea_COUNT = 3;
char ClearInfoArea_COUNT = 3;

void DrawWeapon();
void GiveWeapon(
    int16_t weapon);
void GiveAmmo(
    int16_t ammo);
void DrawGAmmoNum();
void DrawMAmmoNum();
void DrawPDAmmoMsg();
void ComputeAvailWeapons();
void SW_HandleActor(
    objtype* obj);
void SW_HandleStatic(
    statobj_t* stat,
    uint16_t tilex,
    uint16_t tiley);

// ===========================================================================

// ----------

uint8_t ShowRatio(
    int16_t bx,
    int16_t by,
    int16_t px,
    int16_t py,
    int32_t total,
    int32_t perc,
    ss_type type);
void Attack();
void Use();
void Search(
    objtype* ob);
void SelectWeapon();
void SelectItem();

// ----------

void SpawnPlayer(
    int16_t tilex,
    int16_t tiley,
    int16_t dir);
void Thrust(
    int16_t angle,
    int32_t speed);
boolean TryMove(
    objtype* ob);
void T_Player(
    objtype* ob);

boolean ClipMove(
    objtype* ob,
    int32_t xmove,
    int32_t ymove);

void SocketToggle(
    boolean TurnOn);
void CheckStatsBonus();

void T_Stand(
    objtype* ob);

/*
=============================================================================

 CONTROL STUFF

=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-6 change weapons
=
=
======================
*/


void CheckWeaponChange()
{
    int16_t i;
    const int16_t n = ::is_ps() ? wp_bfg_cannon : wp_grenade;

    for (i = wp_autocharge; i <= n; i++) {
        if (buttonstate[bt_ready_autocharge + i - wp_autocharge]) {
            if (gamestate.useable_weapons & (1 << i)) {
                gamestate.weapon = static_cast<char>(i);
                gamestate.chosenweapon = static_cast<char>(i);

                DISPLAY_TIMED_MSG(WeaponAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
                DrawWeapon();
                return;
            } else {
                DISPLAY_TIMED_MSG(WeaponNotAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
            }
        }
    }
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement(
    objtype* ob)
{
    bool use_classic_strafe =
        (in_use_modern_bindings && in_is_binding_pressed(e_bi_strafe)) ||
        (!in_use_modern_bindings && buttonstate[bt_strafe]);

    bool use_modern_strafe = false;

    thrustspeed = 0;

    int oldx = player->x;
    int oldy = player->y;

    //
    // side to side move
    //

    if (use_classic_strafe) {
        if (in_use_modern_bindings) {
            use_modern_strafe = true;
        } else {
            if (controlx > 0) {
                int angle = ob->angle - ANGLES / 4;
                if (angle < 0) {
                    angle += ANGLES;
                }
                Thrust(angle, controlx * MOVESCALE); // move to left
            } else if (controlx < 0) {
                int angle = ob->angle + ANGLES / 4;
                if (angle >= ANGLES) {
                    angle -= ANGLES;
                }
                Thrust(angle, -controlx * MOVESCALE); // move to right
            }
        }
    } else if (!gamestate.turn_around) {
        use_modern_strafe = true;
    }

    if (use_modern_strafe && strafe_value != 0) {
        int sign = (strafe_value > 0) ? 1 : -1;
        int angle = ob->angle + (sign * (ANGLES / 4));

        if (angle < 0) {
            angle += ANGLES;
        } else if (angle >= ANGLES) {
            angle -= ANGLES;
        }

        Thrust(angle, -abs(strafe_value) * MOVESCALE);
    }

    if (!use_classic_strafe) {
        if (gamestate.turn_around) {
            controlx = 100 * tics;
            if (gamestate.turn_around < 0) {
                controlx = -controlx;
            }
        }

        //
        // not strafing
        //
        anglefrac += controlx;
        int angleunits = anglefrac / ANGLESCALE;
        anglefrac -= angleunits * ANGLESCALE;
        ob->angle -= angleunits;

        if (ob->angle >= ANGLES) {
            ob->angle -= ANGLES;
        }
        if (ob->angle < 0) {
            ob->angle += ANGLES;
        }

        if (gamestate.turn_around) {
            boolean done = false;

            if (gamestate.turn_around > 0) {
                gamestate.turn_around -= angleunits;
                if (gamestate.turn_around <= 0) {
                    done = true;
                }
            } else {
                gamestate.turn_around -= angleunits;
                if (gamestate.turn_around >= 0) {
                    done = true;
                }
            }

            if (done) {
                gamestate.turn_around = 0;
                ob->angle = gamestate.turn_angle;
            }
        }
    }


    //
    // forward/backwards move
    //
    if (controly < 0) {
        Thrust(ob->angle, -controly * MOVESCALE); // move forwards
    } else if (controly > 0) {
        int angle = ob->angle + ANGLES / 2;
        if (angle >= ANGLES) {
            angle -= ANGLES;
        }
        Thrust(angle, controly * BACKMOVESCALE); // move backwards
    } else if (bounceOk) {
        bounceOk--;
    }

    if (controly) {
        bounceOk = 8;
    } else if (bounceOk) {
        bounceOk--;
    }

    ob->dir = static_cast<dirtype>(((ob->angle + 22) % 360) / 45);

    //
    // calculate total move
    //
    playerxmove = player->x - oldx;
    playerymove = player->y - oldy;
}

/*
=============================================================================

 STATUS WINDOW STUFF

=============================================================================
*/

#define STATUSDRAWPIC(x, y, picnum) LatchDrawPic((x), (y + (200 - STATUSLINES)), (picnum))


/*
==================
=
= StatusDrawPic
=
==================
*/
void StatusAllDrawPic(
    uint16_t x,
    uint16_t y,
    uint16_t picnum)
{
    uint16_t temp;

#ifdef PAGEFLIP

    temp = static_cast<uint16_t>(bufferofs);
    bufferofs = PAGE1START + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = PAGE2START + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = PAGE3START + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = temp;

#else

    temp = bufferofs;
    bufferofs = screenloc[0] + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = screenloc[1] + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = screenloc[2] + (200 - STATUSLINES) * SCREENWIDTH;
    LatchDrawPic(x, y, picnum);
    bufferofs = temp;

#endif

}


/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/
void LatchNumber(
    int16_t x,
    int16_t y,
    int16_t width,
    int32_t number)
{
    auto wide = 0;
    auto str = std::to_string(number);
    auto length = static_cast<int>(str.length());

    while (length < width && wide < width) {
        STATUSDRAWPIC(x, y, N_BLANKPIC);
        ++x;
        ++wide;
        ++length;
    }

    auto c = 0;

    while (wide < width) {
        STATUSDRAWPIC(x, y, str[c] - '0' + N_0PIC);
        ++x;
        ++c;
        ++wide;
    }
}




// ===========================================================================
//
//
// SCORE DISPLAY ROUTINES
//
//
// ===========================================================================


namespace {

int ecg_scroll_tics = 0;
int ecg_next_scroll_tics = 0;
std::deque<int> ecg_legend(6);
std::deque<int> ecg_segments(6);

int heart_picture_index = ECG_GRID_PIECE;
int heart_sign_tics = 0;
int heart_sign_next_tics = 0;

} // namespace

// Draws electrocardiogram (ECG) and the heart sign
void DrawHealthMonitor()
{
    //
    // ECG
    //

    // ECG segment indices:
    // 0 - silence
    // 1..8 - shape #1 (health 66%-100%)
    // 9..17 - shape #2 (health 33%-65%)
    // 18..27 - shape #3 (health 0%-32%)

    // ECG segment legend:
    // 0 - silence
    // 1 - signal #1 (health 66%-100%)
    // 2 - signal #2 (health 33%-65%)
    // 3 - signal #3 (health 0%-32%)

    if (ecg_scroll_tics >= ecg_next_scroll_tics) {
        ecg_scroll_tics = 0;
        ecg_next_scroll_tics = HEALTH_SCROLL_RATE;

        bool carry = false;

        for (int i = 5; i >= 0; --i) {
            if (carry) {
                carry = false;
                ecg_legend[i] = ecg_legend[i + 1];
                ecg_segments[i] = ecg_segments[i + 1] - 4;
            } else if (ecg_segments[i] != 0) {
                ecg_segments[i] += 1;

                bool use_carry = false;

                if (ecg_legend[i] == 1 && ecg_segments[i] == 5) {
                    use_carry = true;
                } else if (ecg_legend[i] == 2 && ecg_segments[i] == 13) {
                    use_carry = true;
                }
                if (ecg_legend[i] == 3 &&
                    (ecg_segments[i] == 22 || ecg_segments[i] == 27))
                {
                    use_carry = true;
                }

                if (use_carry) {
                    carry = true;
                } else {
                    bool skip = false;

                    if (ecg_legend[i] == 1 && ecg_segments[i] > 8) {
                        skip = true;
                    } else if (ecg_legend[i] == 2 && ecg_segments[i] > 17) {
                        skip = true;
                    }
                    if (ecg_legend[i] == 3 && ecg_segments[i] > 27) {
                        skip = true;
                    }

                    if (skip) {
                        ecg_legend[i] = 0;
                        ecg_segments[i] = 0;
                    }
                }
            }
        }

        if (gamestate.health > 0 && ecg_legend[5] == 0) {
            if (gamestate.health < 33) {
                ecg_legend[5] = 3;
                ecg_segments[5] = 18;
            } else if (gamestate.health >= 66) {
                if (ecg_legend[4] == 0 || ecg_legend[4] != 1) {
                    ecg_legend[5] = 1;
                    ecg_segments[5] = 1;
                }
            } else {
                ecg_legend[5] = 2;
                ecg_segments[5] = 9;
            }
        }
    } else {
        ecg_scroll_tics += tics;
    }

    for (int i = 0; i < 6; ++i) {
        ::CA_CacheGrChunk(
            static_cast<int16_t>(ECG_HEARTBEAT_00 + ecg_segments[i]));

        ::VWB_DrawPic(
            120 + (i * 8),
            200 - STATUSLINES + 8,
            ECG_HEARTBEAT_00 + ecg_segments[i]);
    }


    //
    // Heart sign
    //

    bool reset_heart_tics = false;

    if (gamestate.health <= 0) {
        reset_heart_tics = true;
        heart_picture_index = ECG_GRID_PIECE;
    } else if (gamestate.health < 40) {
        reset_heart_tics = true;
        heart_picture_index = ECG_HEART_BAD;
    } else if (heart_sign_tics >= heart_sign_next_tics) {
        reset_heart_tics = true;

        if (heart_picture_index == ECG_GRID_PIECE) {
            heart_picture_index = ECG_HEART_GOOD;

            if (::g_heart_beat_sound) {
                ::sd_play_player_sound(H_BEATSND, bstone::AC_ITEM);
            }
        } else {
            heart_picture_index = ECG_GRID_PIECE;
        }
    }

    if (reset_heart_tics) {
        heart_sign_tics = 0;
        heart_sign_next_tics = HEALTH_PULSE / 2;
    } else {
        heart_sign_tics += 1;
    }

    ::CA_CacheGrChunk(
        static_cast<int16_t>(heart_picture_index));

    ::VWB_DrawPic(
        120,
        200 - STATUSLINES + 32,
        heart_picture_index);
}

// --------------------------------------------------------------------------
// DrawHealth()
//
// PURPOSE : Marks the Health_NUM to be refreshed durring the next
//                               StatusBarRefresh.
// --------------------------------------------------------------------------
void DrawHealth()
{
    if (::is_ps()) {
        auto str = std::to_string(gamestate.health);

        std::uninitialized_fill_n(
            gamestate.health_str,
            4,
            '\0');

        auto index = 0;

        for (auto& ch : str) {
            gamestate.health_str[index] = ch - '0';
            index += 1;
        }
    }

    DrawHealthNum_COUNT = 3;
}

// --------------------------------------------------------------------------
// DrawHealthNum()
// --------------------------------------------------------------------------
void DrawHealthNum()
{
    if (!::is_ps()) {
        ::CA_CacheGrChunk(ECG_GRID_PIECE);

        for (int i = 0; i < 3; ++i) {
            ::VWB_DrawPic(
                144 + (i * 8),
                200 - STATUSLINES + 32,
                ECG_GRID_PIECE);
        }

        std::string health_string(4, ' ');

        bstone::StringHelper::lexical_cast(
            gamestate.health,
            health_string);

        if (gamestate.health < 100) {
            health_string.insert(0, 1, ' ');

            if (gamestate.health < 10) {
                health_string.insert(0, 1, ' ');
            }
        }

        health_string += '%';

        fontnumber = 2;

        // FIXME Should be slightly blue
        fontcolor = 0x9D;

        PrintX = 149;
        PrintY = 200 - STATUSLINES + 34;

        px = PrintX;
        py = PrintY;

        VW_DrawPropString(health_string.c_str());

        DrawHealthNum_COUNT -= 1;
    } else {
        char loop, num;
        int16_t check = 100;

        DrawHealthNum_COUNT--;

        for (loop = num = 0; loop < 3; loop++, check /= 10) {
            if (gamestate.health < check) {
                LatchDrawPic(16 + loop, 162, NG_BLANKPIC);
            } else {
                LatchDrawPic(16 + loop, 162, gamestate.health_str[static_cast<int>(num++)] + NG_0PIC);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// TakeDamage()
// ---------------------------------------------------------------------------
void TakeDamage(
    int16_t points,
    objtype* attacker)
{
    LastAttacker = attacker;

    if (gamestate.flags & GS_ATTACK_INFOAREA) {
        if (attacker) {
            if ((LastMsgType == MT_ATTACK) && (LastInfoAttacker == attacker->obclass)) {
                MsgTicsRemain = DISPLAY_MSG_STD_TIME;
            } else {
                if (DISPLAY_TIMED_MSG(ActorInfoMsg[attacker->obclass - rentacopobj], MP_TAKE_DAMAGE, MT_ATTACK)) {
                    LastInfoAttacker = attacker->obclass;

                    if (::is_ps()) {
                        LastInfoAttacker_Cloaked = attacker->flags2 & FL2_CLOAKED;
                    }
                }
            }
        }
    }

    if (godmode) {
        return;
    }

    if (gamestate.difficulty == gd_baby) {
        points >>= 2;
    }

    gamestate.health -= points;

    if (gamestate.health <= 0) {
        gamestate.health = 0;
        playstate = ex_died;
        killerobj = attacker;
        if (killerobj) {
            killerobj->flags |= FL_FREEZE;
        }
    }

    StartDamageFlash(points);
    DrawHealth();
}

// ---------------------------------------------------------------------------
// HealSelf()
// ---------------------------------------------------------------------------
void HealSelf(
    int16_t points)
{
    gamestate.health += points;
    if (gamestate.health > 100) {
        gamestate.health = 100;
    }

    DrawHealth();
}




// ===========================================================================
//
//
// SCORE DISPLAY ROUTINES
//
//
// ===========================================================================

// --------------------------------------------------------------------------
// DrawScore()
//
// PURPOSE : Marks the Score to be refreshed durring the next
//      StatusBarRefresh.
// --------------------------------------------------------------------------
void DrawScore()
{
    DrawScoreNum_COUNT = 3;
}

extern uint8_t music_num;

// --------------------------------------------------------------------------
// DrawScoreNum()
//
// NOTE : Could do some sort of "scrolling" animation on LED screen with
//      chars and a simple table.....
// --------------------------------------------------------------------------
void DrawScoreNum()
{
#define Y 3
#define X 32

    if (gamestate.tic_score > MAX_DISPLAY_SCORE) {
        if (gamestate.score_roll_wait) {
            LatchDrawPic(X + 0, (200 - STATUSLINES) + Y, N_BLANKPIC);
            LatchDrawPic(X + 1, (200 - STATUSLINES) + Y, N_DASHPIC);
            LatchDrawPic(X + 2, (200 - STATUSLINES) + Y, N_RPIC);
            LatchDrawPic(X + 3, (200 - STATUSLINES) + Y, N_OPIC);
            LatchDrawPic(X + 4, (200 - STATUSLINES) + Y, N_LPIC);
            LatchDrawPic(X + 5, (200 - STATUSLINES) + Y, N_LPIC);
            LatchDrawPic(X + 6, (200 - STATUSLINES) + Y, N_DASHPIC);
        } else {
            LatchNumber(X, Y, 7, gamestate.tic_score % (MAX_DISPLAY_SCORE + 1));
        }
    } else {
        if (gamestate.flags & GS_TICS_FOR_SCORE) {
            LatchNumber(X, Y, 7, realtics);
        } else if (gamestate.flags & GS_MUSIC_TEST) {
            LatchNumber(X, Y, 7, music_num);
        } else {
            LatchNumber(X, Y, 7, gamestate.tic_score);
        }
    }
}

// --------------------------------------------------------------------------
// UpdateScore()
// --------------------------------------------------------------------------
void UpdateScore()
{
    int32_t score_diff, temp_tics;

    score_diff = gamestate.score - gamestate.tic_score;

    if (score_diff) {
        if (score_diff > 1500) {
            temp_tics = score_diff >> 2;
        } else {
            temp_tics = tics << 3;
        }

        if (score_diff > temp_tics) {
            gamestate.tic_score += temp_tics;
        } else {
            gamestate.tic_score = gamestate.score;
        }

        DrawScore();
    }


    if (gamestate.score_roll_wait) {
        if ((gamestate.score_roll_wait -= tics) <= 0) {
            gamestate.score_roll_wait = 0;
        }
        DrawScore();
    }
}

// --------------------------------------------------------------------------
// GivePoints()
//
// .score = Holds real score
// .tic_score  = Holds displayed score (tic'ing toward .score)
//
// --------------------------------------------------------------------------
void GivePoints(
    int32_t points,
    boolean add_to_stats)
{
// Add score to statistics.
//
    if (add_to_stats) {
        gamestuff.level[gamestate.mapon].stats.accum_points += points;
    }

// Check for bonuses!
//
    CheckPinballBonus(points);

// Add points to score
//
    gamestate.score += points;
}

// ===========================================================================
//
//
//                      SECURITY KEY DISPLAY ROUTINES
//
//
// ===========================================================================



// ---------------------------------------------------------------------------
// DrawKeys()
//
// PURPOSE : Marks the security key pics to be refreshed during the next
//      StatusBarRefresh.
// ---------------------------------------------------------------------------
void DrawKeys()
{
    DrawKeyPics_COUNT = 3;
}

// ---------------------------------------------------------------------------
// DrawKeyPics()
// ---------------------------------------------------------------------------
void DrawKeyPics()
{
    char loop;

    DrawKeyPics_COUNT--;

    if (!::is_ps()) {
        static const int indices[NUMKEYS] = {
            0, 1, 3, 2, 4
        }; // indices

        static const uint8_t off_colors[NUMKEYS] = {
            0x11, 0x31, 0x91, 0x51, 0x21
        }; // off_colors

        static const uint8_t on_colors[NUMKEYS] = {
            0xC9, 0xB9, 0x9C, 0x5B, 0x2B
        }; // on_colors

        for (int i = 0; i < NUMKEYS; ++i) {
            int index = indices[i];
            uint8_t color = 0;

            if (gamestate.numkeys[index] > 0) {
                color = on_colors[index];
            } else {
                color = off_colors[index];
            }

            ::VWB_Bar(
                257 + (i * 8),
                200 - STATUSLINES + 25,
                7,
                7,
                color);
        }
    } else {
        for (loop = 0; loop < NUMKEYS; loop++) {
            if (gamestate.numkeys[static_cast<int>(loop)]) {
                LatchDrawPic(15 + 2 * loop, 179, RED_KEYPIC + loop);
            } else {
                LatchDrawPic(15 + 2 * loop, 179, NO_KEYPIC);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GiveKey
// ---------------------------------------------------------------------------
void GiveKey(
    int16_t key)
{
    gamestate.numkeys[key]++;
    DrawKeys();
}

// ---------------------------------------------------------------------------
// TakeKey
// ---------------------------------------------------------------------------
void TakeKey(
    int16_t key)
{
    gamestate.numkeys[key]--;
    DrawKeys();
}


// ===========================================================================
//
//
//                              WEAPON DISPLAY ROUTINES
//
//
// ===========================================================================


// ---------------------------------------------------------------------------
// DrawWeapon()
//
// PURPOSE : Marks the Weapon pics to be refreshed durring the next
//      StatusBarRefresh.
// ---------------------------------------------------------------------------
void DrawWeapon()
{
    DrawWeaponPic_COUNT = 3;
    DrawAmmo(true);
}

// ---------------------------------------------------------------------------
// DrawWeaponPic()
// ---------------------------------------------------------------------------
void DrawWeaponPic()
{
    if (gamestate.weapon == -1) {
        return;
    }

    ::LatchDrawPic(
        ::is_ps() ? 31 : 22,
        ::is_ps() ? 176 : 152,
        WEAPON1PIC + gamestate.weapon);

    DrawWeaponPic_COUNT--;
}

// ---------------------------------------------------------------------------
// GiveWeapon()
// ---------------------------------------------------------------------------
void GiveWeapon(
    int16_t weapon)
{
    GiveAmmo(6);

    if (!(gamestate.weapons & (1 << weapon))) {
        gamestate.weapons |= (1 << weapon);

        if (gamestate.weapon < weapon) {
            gamestate.weapon = static_cast<char>(weapon);
            gamestate.chosenweapon = static_cast<char>(weapon);
            DrawWeapon();
        }
    }
}

// ===========================================================================
//
//
//                              AMMO DISPLAY ROUTINES
//
//
// ===========================================================================

// ---------------------------------------------------------------------------
// DrawAmmo()
//
// PURPOSE : Marks the AMMO NUM & AMMO PIC (if necessary) to be refreshed
//                               durring the next StatusBarRefresh.
//
// NOTE : This re-computes the number of LEDs to be lit.
// ---------------------------------------------------------------------------
void DrawAmmo(
    boolean ForceRefresh)
{
    int16_t temp;
    uint16_t ammo, max_ammo;

    ComputeAvailWeapons();

    //
    // Which weapon are we needing a refresh for?
    //

    switch (gamestate.weapon) {
//      case wp_plasma_detonators:
//                      DrawAmmoPic_COUNT = 3;
//                      DrawAmmoNum_COUNT = 0;
//              return;

    case wp_autocharge:
        DrawAmmoPic_COUNT = 3;
        DrawAmmoNum_COUNT = 0;
        return;

    default:
        ammo = gamestate.ammo;
        max_ammo = MAX_AMMO;
        break;
    }

    if (ammo) {
        temp = (ammo * NUM_AMMO_SEGS) / max_ammo;
        if (!temp) {
            temp = 1;
        }
    } else {
        temp = 0;
    }

    gamestate.ammo_leds = static_cast<char>(temp);

    if ((temp != gamestate.lastammo_leds) || ForceRefresh) {
        gamestate.lastammo_leds = static_cast<char>(temp);
        DrawAmmoPic_COUNT = 3;
    }

    DrawAmmoNum_COUNT = 3;
}



// ---------------------------------------------------------------------------
// DrawAmmoNum()
// ---------------------------------------------------------------------------
void DrawAmmoNum()
{
    if (gamestate.weapon == -1) {
        return;
    }

    fontnumber = 2;
    fontcolor = 0x9D;

    PrintX = (::is_ps() ? 252 : 211);
    PrintY = 200 - STATUSLINES + 38;

    if (::is_ps() || (!::is_ps() && gamestate.weapon != wp_autocharge)) {
        ::DrawGAmmoNum();
    }

    DrawAmmoNum_COUNT--;
}



// ---------------------------------------------------------------------------
// DrawGAmmoNum()
// ---------------------------------------------------------------------------
void DrawGAmmoNum()
{
    if (gamestate.ammo < 100) {
        PrintX += AMMO_SMALL_FONT_NUM_WIDTH;
        if (gamestate.ammo < 10) {
            PrintX += AMMO_SMALL_FONT_NUM_WIDTH;
        }
    }

    if (::is_ps()) {
        ::LatchDrawPic(31, 184, W1_CORNERPIC + gamestate.weapon);
    }

    px = PrintX;
    py = PrintY;

    auto ammo_string = std::to_string(gamestate.ammo);
    VW_DrawPropString(ammo_string.c_str());
    VW_DrawPropString("%");
}

// ---------------------------------------------------------------------------
// DrawAmmoPic()
// ---------------------------------------------------------------------------
void DrawAmmoPic()
{
    switch (gamestate.weapon) {
    case wp_autocharge:
        DrawAmmoMsg();
        break;

//              case wp_plasma_detonators:
//                      DrawPDAmmoMsg();
//     break;

    default:
        DrawAmmoGuage();
        break;
    }

    DrawAmmoPic_COUNT--;
}

// ---------------------------------------------------------------------------
// DrawAmmoMsg() -
// ---------------------------------------------------------------------------
void DrawAmmoMsg()
{
    int x = (::is_ps() ? 30 : 29);

    if (gamestate.weapon_wait) {
        LatchDrawPic(x, (200 - STATUSLINES), WAITPIC);
    } else {
        LatchDrawPic(x, (200 - STATUSLINES), READYPIC);
    }
}

// ---------------------------------------------------------------------------
// DrawPDAmmoMsg() -
// ---------------------------------------------------------------------------
void DrawPDAmmoMsg()
{
    if (gamestate.plasma_detonators) {
        LatchDrawPic(30, (200 - STATUSLINES), READYPIC);
    } else {
        LatchDrawPic(30, (200 - STATUSLINES), WAITPIC);
    }
}

// ---------------------------------------------------------------------------
// UpdateAmmoMsg() -
// ---------------------------------------------------------------------------
void UpdateAmmoMsg()
{
    if (gamestate.weapon_wait) {
        if ((gamestate.weapon_wait -= static_cast<char>(tics)) <= 0) {
            gamestate.weapon_wait = 0;
            DrawAmmoPic_COUNT = 3;
        }
    }
}

// ---------------------------------------------------------------------------
// DrawAmmoGuage()
// ---------------------------------------------------------------------------
void DrawAmmoGuage()
{
    DrawLedStrip(::is_ps() ? 243 : 234, 155, gamestate.ammo_leds, NUM_AMMO_SEGS);
}

// ---------------------------------------------------------------------------
// UpdateRadarGuage()
// ---------------------------------------------------------------------------
void UpdateRadarGuage()
{
    int16_t temp;

    if (gamestate.rpower) {
        temp = ((int32_t)gamestate.rpower * NUM_AMMO_SEGS) / MAX_RADAR_ENERGY;

        if (temp > NUM_AMMO_SEGS) {
            temp = NUM_AMMO_SEGS;
        }

        if (!temp) {
            temp = 1;
        }
    } else {
        temp = 0;
    }

    gamestate.radar_leds = static_cast<char>(temp);

    if (temp != gamestate.lastradar_leds) {
        gamestate.lastradar_leds = static_cast<char>(temp);
    }

    DrawRadarGuage_COUNT = 3;
}

// ---------------------------------------------------------------------------
// DrawRadarGuage()
// ---------------------------------------------------------------------------
void DrawRadarGuage()
{
    if (!::is_ps()) {
        return;
    }

    char zoom;

    DrawLedStrip(235, 155, gamestate.radar_leds, NUM_AMMO_SEGS);

    if (gamestate.rpower) {
        zoom = gamestate.rzoom;
    } else {
        zoom = 0;
    }

    LatchDrawPic(22, 152, ONEXZOOMPIC + zoom);
}

// ---------------------------------------------------------------------------
// DrawLedStrip()
// ---------------------------------------------------------------------------
void DrawLedStrip(
    int16_t x,
    int16_t y,
    int16_t frac,
    int16_t max)
{
    int16_t ypos;
    uint16_t amount;
    char leds;

    leds = static_cast<char>(frac);

    if (leds) {
        amount = max - leds;
    } else {
        amount = max;
    }

    int width = (::is_ps() ? 5 : 11);

// Draw dim LEDs.
//
    for (ypos = 0; ypos < amount; ypos++) {
        VW_Hlin(x, x + (width - 1), y++, DimAmmo[0][amount]);
        VW_Hlin(x, x + (width - 1), y++, DimAmmo[1][amount]);
    }

// Draw lit LEDs.
//
    for (; ypos < NUM_AMMO_SEGS; ypos++) {
        VW_Hlin(x, x + (width - 1), y++, LitAmmo[0][amount]);
        VW_Hlin(x, x + (width - 1), y++, LitAmmo[1][amount]);
    }
}


// ---------------------------------------------------------------------------
// GiveAmmo()
// ---------------------------------------------------------------------------
void GiveAmmo(
    int16_t ammo)
{

#if MP_NO_MORE_AMMO > MP_BONUS
    if (LastMsgType == MT_OUT_OF_AMMO) {
        MsgTicsRemain = 1;
        LastMsgType = MT_CLEAR;
    }
#endif

    gamestate.ammo += ammo;
    if (gamestate.ammo > MAX_AMMO) {
        gamestate.ammo = MAX_AMMO;
    }

    DrawAmmo(false);

    if (::is_ps()) {
        if (gamestate.weapon != gamestate.chosenweapon) {
            if (gamestate.useable_weapons & (1 << gamestate.chosenweapon)) {
                gamestate.weapon = gamestate.chosenweapon;
                DrawWeapon();
            }
        }
    } else {
        DrawWeapon();
    }

    ::sd_play_player_sound(GETAMMOSND, bstone::AC_ITEM);


#if 0
#if MP_NO_MORE_AMMO > MP_BONUS
    if (LastMsgType == MT_OUT_OF_AMMO) {
        MsgTicsRemain = 1;
        LastMsgType = MT_CLEAR;
    }
#endif

    gamestate.ammo += ammo;
    if (gamestate.ammo > MAX_AMMO) {
        gamestate.ammo = MAX_AMMO;
    }

    // JIM - This needs to be optomized.

    if (gamestate.weapon != gamestate.chosenweapon) {
        if (!((gamestate.chosenweapon == wp_grenade) && (gamestate.ammo < GRENADE_ENERGY_USE)) ||
            !((gamestate.chosenweapon == wp_bfg_cannon) && (gamestate.ammo < BFG_ENERGY_USE)))
        {
            gamestate.weapon = gamestate.chosenweapon;
            DrawWeapon();
        }
    }

    DrawAmmo(false);
    SD_PlaySound(GETAMMOSND);
#endif
}


// ---------------------------------------------------------------------------
// ComputeAvailWeapons()
//
// This function creates a Bit MASK for gamestate.weapons according to what
// weapon is available for useage due to ammo avail.
//
// ---------------------------------------------------------------------------
void ComputeAvailWeapons()
{

    //
    // Determine what ammo ammounts we have avail
    //

    if (gamestate.ammo) {
        if (::is_ps() && gamestate.ammo >= BFG_ENERGY_USE) {
            gamestate.useable_weapons = (1 << wp_bfg_cannon)
                                        | (1 << wp_grenade)
                                        | (1 << wp_ion_cannon)
                                        | (1 << wp_burst_rifle)
                                        | (1 << wp_pistol)
                                        | (1 << wp_autocharge);
        } else if (gamestate.ammo >= GRENADE_ENERGY_USE) {
            gamestate.useable_weapons = (1 << wp_grenade)
                                        | (1 << wp_ion_cannon)
                                        | (1 << wp_burst_rifle)
                                        | (1 << wp_pistol)
                                        | (1 << wp_autocharge);
        } else {
            gamestate.useable_weapons = (1 << wp_ion_cannon)
                                        | (1 << wp_burst_rifle)
                                        | (1 << wp_pistol)
                                        | (1 << wp_autocharge);
        }
    } else {
        gamestate.useable_weapons = (1 << wp_autocharge);
    }

    //
    // Do special weapons.
    //

//   if (gamestate.plasma_detonators)
//              gamestate.useable_weapons |= (1<<wp_plasma_detonators);

    //
    // mask off with the weapons being carried.
    //

    gamestate.useable_weapons &= gamestate.weapons;

}



// ---------------------------------------------------------------------------
// TakePlasmaDetonator()
// ---------------------------------------------------------------------------
void TakePlasmaDetonator(
    int16_t count)
{
    if (gamestate.plasma_detonators < count) {
        gamestate.plasma_detonators = 0;
    } else {
        gamestate.plasma_detonators -= count;
    }
}

// ---------------------------------------------------------------------------
// GivePlasmaDetonator()
// ---------------------------------------------------------------------------
void GivePlasmaDetonator(
    int16_t count)
{
    gamestate.plasma_detonators += count;

    if (gamestate.plasma_detonators > MAX_PLASMA_DETONATORS) {
        gamestate.plasma_detonators = MAX_PLASMA_DETONATORS;
    }

//   if (gamestate.chosenweapon == wp_plasma_detonators)
//      {
//              gamestate.weapon = gamestate.chosenweapon;
//              DrawWeapon ();
//      }

    ComputeAvailWeapons();
}

// ---------------------------------------------------------------------------
// GiveToken()
// ---------------------------------------------------------------------------
void GiveToken(
    int16_t tokens)
{
#if MP_NO_MORE_TOKENS > MP_BONUS
    if (LastMsgType == MT_NO_MO_FOOD_TOKENS) {
        MsgTicsRemain = 1;
        LastMsgType = MT_CLEAR;
    }
#endif

    gamestate.tokens += tokens;
    if (gamestate.tokens > MAX_TOKENS) {
        gamestate.tokens = MAX_TOKENS;
    }

    ::sd_play_player_sound(GOTTOKENSND, bstone::AC_ITEM);
}

// ===========================================================================
//
//
//                               INFO AREA ROUTINES
//
//
// ===========================================================================

// --------------------------------------------------------------------------
// DisplayInfoMsg() - Returns if Higher Pri message is holding.
//
// SEE MACROS:   DISPLAY_TIMED_MSG() & DISPLAY_MSG() -- Def.h
//
//      DISPLAY_TIMED_MSG(msg,pri,type) - For E-Z Timed Msgs (std. display time)
//     DISPLAY_MSG(msg,pri,type)                 - For E-Z NON-Timed Msgs.
// --------------------------------------------------------------------------
boolean DisplayInfoMsg(
    const std::string& Msg,
    msg_priorities Priority,
    int16_t DisplayTime,
    int16_t MsgType)
{
    if (Msg.empty()) {
        return false;
    }

    return ::DisplayInfoMsg(
        Msg.c_str(),
        Priority,
        DisplayTime,
        MsgType);
}

boolean DisplayInfoMsg(
    const char* Msg,
    msg_priorities Priority,
    int16_t DisplayTime,
    int16_t MsgType)
{
    if (Priority >= LastMsgPri) {
        if (Priority == MP_max_val) { // "System" msgs
            LastMsgPri = MP_min_val;
        } else {
            LastMsgPri = static_cast<uint16_t>(Priority);
        }

        if ((MsgTicsRemain = DisplayTime) != 0) {
            StatusAllDrawPic(0, 40, BRI_LIGHTPIC);
        }

        gamestate.msg = Msg;

        DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;

        LastMsgType = static_cast<infomsg_type>(MsgType);

        if (::is_ps() && LastMsgType != MT_ATTACK) {
            LastInfoAttacker_Cloaked = 0;
        }

        return true;
    } else {
        return false;
    }
}


// --------------------------------------------------------------------------
// ClearInfoArea()
// --------------------------------------------------------------------------
void ClearInfoArea()
{
#if IN_DEVELOPMENT
    if (gamestate.flags & GS_SHOW_OVERHEAD) {
        return;
    }
#endif

    if (ClearInfoArea_COUNT) {
        ClearInfoArea_COUNT--;
    }

    InfoAreaSetup.x = InfoAreaSetup.left_margin;
    InfoAreaSetup.y = INFOAREA_Y;
    InfoAreaSetup.framecount = InfoAreaSetup.numanims = 0;

    LatchDrawPic(0, 200 - STATUSLINES, INFOAREAPIC);
}


// --------------------------------------------------------------------------
// InitInfoArea()
// --------------------------------------------------------------------------
void InitInfoArea()
{
    InfoAreaSetup.left_margin = INFOAREA_X;
    InfoAreaSetup.text_color = INFOAREA_TCOLOR;
    InfoAreaSetup.backgr_color = INFOAREA_BCOLOR;
    InitInfoArea_COUNT--;

    ClearInfoArea();
}


// --------------------------------------------------------------------------
// UpdateInfoArea()
// --------------------------------------------------------------------------
void UpdateInfoArea()
{

    if (InfoAreaSetup.numanims) {
        AnimatePage();
    }

    if (InitInfoArea_COUNT) {
        InitInfoArea();
    } else if (ClearInfoArea_COUNT) {
        ClearInfoArea();
    }

    if (DrawInfoArea_COUNT) {
        DrawInfoArea();
    }
}

// ---------------------------------------------------------------------------
// UpdateInfoAreaClock() - This routine is called ONLY ONCE per refresh
//      to update the InfoArea Clock and to release
//      any messages that have expired.
// ---------------------------------------------------------------------------
void UpdateInfoAreaClock()
{

    if (playstate == ex_title || playstate == ex_victorious) {
        return;
    }

    //
    // Check for existing timed messages
    //

    if (LastMsgPri && MsgTicsRemain) {
        //
        // Tic' that 'Puppy' down - Yea!
        //

        if ((MsgTicsRemain -= tics) <= 0) {
            // Message has expired.
            DisplayNoMoMsgs();
        }
    }

}

// ---------------------------------------------------------------------------
// DisplayTokens()
// ---------------------------------------------------------------------------
char default_msg[] = { "\r    NO MESSAGES."
                       "^FCA8\r    FOOD TOKENS:      "
                       "                                 " };

char needDetonator_msg[] = "\r\r^FC39 FIND THE DETONATOR!";

char haveDetonator_msg[] = "\r\r^FC39DESTROY SECURITY CUBE!";

char destroyGoldfire_msg[] = "\r\r^FC39  DESTROY GOLDFIRE!";

void DisplayNoMoMsgs()
{
    LastMsgPri = MP_min_val;

    if (BONUS_QUEUE) {
        DisplayPinballBonus();
        return;
    }

    MsgTicsRemain = 0;
    StatusAllDrawPic(0, 40, DIM_LIGHTPIC);
    sprintf((char*)&default_msg[40], "%-d", gamestate.tokens);
    if (gamestuff.level[gamestate.mapon + 1].locked) {
        switch (gamestate.mapon) {
        case 19:
            if (::is_ps()) {
                strcat(default_msg, destroyGoldfire_msg);
            }
            break;

        case 20:
        case 21:
        case 22:
        case 23:
            break;

        default:
            if (::is_ps()) {
                if (gamestate.plasma_detonators) {
                    strcat(default_msg, haveDetonator_msg);
                } else {
                    strcat(default_msg, needDetonator_msg);
                }
            }
            break;
        }
    }

    DisplayInfoMsg(default_msg, MP_max_val, 0, MT_NOTHING);
}

// --------------------------------------------------------------------------
// DrawInfoArea()
//
//
// Active control codes:
//
//  ^ANnn                       - define animation
//  ^FCnn                       - set font color
//  ^LMnnn                      - set left margin (if 'nnn' == "fff" uses current x)
//  ^EP                         - end of page (waits for 'M' to read MORE)
//  ^PXnnn                      - move x to coordinate 'n'
//  ^PYnnn                      - move y to coordinate 'n'
//  ^SHnnn                      - display shape 'n' at current x,y
//  ^BGn                                - set background color
//  ^DM                         - Default Margins
//
// Other info:
//
// All 'n' values are hex numbers (0 - f), case insensitive.
// The number of N's listed is the number of digits REQUIRED by that control
// code. (IE: ^LMnnn MUST have 3 values! --> 003, 1a2, 01f, etc...)
//
// If a line consists only of control codes, the cursor is NOT advanced
// to the next line (the ending <CR><LF> is skipped). If just ONE non-control
// code is added, the number "8" for example, then the "8" is displayed
// and the cursor is advanced to the next line.
//
// The text presenter now handles sprites, but they are NOT masked! Also,
// sprite animations will be difficult to implement unless all frames are
// of the same dimensions.
//
// --------------------------------------------------------------------------

char* HandleControlCodes(
    char* first_ch);


void DrawInfoArea()
{
#define IA_FONT_HEIGHT 6

//      int16_t length,i;
    char* first_ch;
    char* scan_ch, temp;

#if IN_DEVELOPMENT
    if (gamestate.flags & GS_SHOW_OVERHEAD) {
        return;
    }
#endif

    DrawInfoArea_COUNT--;

    if (!gamestate.msg) {
        return;
    }

    if (!*gamestate.msg) {
        return;
    }

    std::vector<char> buffer(
        gamestate.msg,
        gamestate.msg + std::string::traits_type::length(gamestate.msg) + 1);

    first_ch = &buffer[0];

    fontnumber = 2;
    fontcolor = static_cast<uint8_t>(InfoAreaSetup.text_color);

    while (first_ch && *first_ch) {

        if (*first_ch != TP_CONTROL_CHAR) {
            scan_ch = first_ch;

            while ((*scan_ch) && (*scan_ch != '\n') && (*scan_ch != TP_RETURN_CHAR) && (*scan_ch != TP_CONTROL_CHAR)) {
                scan_ch++;
            }

            // print current line
            //

            temp = *scan_ch;
            *scan_ch = 0;

            if (*first_ch != TP_RETURN_CHAR) {
                char temp_color;

                temp_color = fontcolor;
                fontcolor = INFOAREA_TSHAD_COLOR;

                px = InfoAreaSetup.x + 1;
                py = InfoAreaSetup.y + 1;
                VW_DrawPropString(first_ch);
                fontcolor = temp_color;

                px = InfoAreaSetup.x;
                py = InfoAreaSetup.y;
                VW_DrawPropString(first_ch);
            }

            *scan_ch = temp;
            first_ch = scan_ch;

            // skip SPACES / RETURNS at end of line
            //

            if ((*first_ch == ' ') || (*first_ch == TP_RETURN_CHAR)) {
                first_ch++;
            }

            // TP_CONTROL_CHARs don't advance to next character line
            //

            if (*scan_ch != TP_CONTROL_CHAR) {
                InfoAreaSetup.x = InfoAreaSetup.left_margin;
                InfoAreaSetup.y += IA_FONT_HEIGHT;
            } else {
                InfoAreaSetup.x = px;
            }
        } else {
            first_ch = HandleControlCodes(first_ch);
        }
    }
}

// ---------------------------------------------------------------------------
// HandleControlCodes()
// ---------------------------------------------------------------------------
char* HandleControlCodes(
    char* first_ch)
{
//      piShapeInfo *shape_info;
    piShapeInfo* shape;
    piAnimInfo* anim;
    uint16_t shapenum;

    first_ch++;

#ifndef TP_CASE_SENSITIVE
    *first_ch = toupper(*first_ch);
    *(first_ch + 1) = toupper(*(first_ch + 1));
#endif

    uint16_t code = *reinterpret_cast<const uint16_t*>(first_ch);
    first_ch += 2;

    switch (code) {

    // INIT ANIMATION ---------------------------------------------------
    //
    case TP_CNVT_CODE('A', 'N'):
        shapenum = TP_VALUE(first_ch, 2);
        first_ch += 2;
        memcpy(&piAnimList[static_cast<int>(InfoAreaSetup.numanims)], &piAnimTable[shapenum], sizeof(piAnimInfo));
        anim = &piAnimList[static_cast<int>(InfoAreaSetup.numanims++)];
        shape = &piShapeTable[anim->baseshape + anim->frame]; // BUG!! (assumes "pia_shapetable")
// spr = &spritetable[shape->shapenum-STARTSPRITES];

        anim->y = InfoAreaSetup.y;
        anim->x = DrawShape(InfoAreaSetup.x, InfoAreaSetup.y, shape->shapenum, shape->shapetype);
        InfoAreaSetup.framecount = 3;
        InfoAreaSetup.left_margin = InfoAreaSetup.x;
        break;

    // DRAW SHAPE -------------------------------------------------------
    //
    case TP_CNVT_CODE('S', 'H'):

        // NOTE : This needs to handle the left margin....

        shapenum = TP_VALUE(first_ch, 3);
        first_ch += 3;
        shape = &piShapeTable[shapenum];
// spr = &spritetable[shape->shapenum-STARTSPRITES];

        DrawShape(InfoAreaSetup.x, InfoAreaSetup.y, shape->shapenum, shape->shapetype);
        InfoAreaSetup.left_margin = InfoAreaSetup.x;
        break;

    // FONT COLOR -------------------------------------------------------
    //
    case TP_CNVT_CODE('F', 'C'):
        InfoAreaSetup.text_color = TP_VALUE(first_ch, 2);
        fontcolor = static_cast<uint8_t>(TP_VALUE(first_ch, 2));
        first_ch += 2;
        break;

    // BACKGROUND COLOR -------------------------------------------------
    //
    case TP_CNVT_CODE('B', 'G'):
        InfoAreaSetup.backgr_color = TP_VALUE(first_ch, 2);
        first_ch += 2;
        break;

    // DEFAULT MARGINS -------------------------------------------------
    //
    case TP_CNVT_CODE('D', 'M'):
        InfoAreaSetup.left_margin = INFOAREA_X;
        break;

    // LEFT MARGIN ------------------------------------------------------
    //
    case TP_CNVT_CODE('L', 'M'):
        shapenum = TP_VALUE(first_ch, 3);
        first_ch += 3;
        if (shapenum == 0xfff) {
            InfoAreaSetup.left_margin = InfoAreaSetup.x;
        } else {
            InfoAreaSetup.left_margin = shapenum;
        }
        break;

#ifdef UNLOCK_FLOORS
    // UNLOCK FLOOR ----------------------------------------------------
    //
    case TP_CNVT_CODE('U', 'F'):
        shapenum = TP_VALUE(first_ch++, 1);
        gamestuff.level[shapenum].locked = false;
        break;
#endif
    }

    return first_ch;

}

// --------------------------------------------------------------------------
// DrawShape()
// --------------------------------------------------------------------------
int16_t DrawShape(
    int16_t x,
    int16_t y,
    int16_t shapenum,
    pisType shapetype)
{
    int16_t width = 0;
    uint16_t shade;

// width=TP_BoxAroundShape(x,y,shapenum,shapetype);

    //
    // If Image is Cloaked... Shade the image
    //
    if (LastInfoAttacker_Cloaked) {
        shade = 35; // 63 == BLACK | 0 == NO SHADING
    } else
        shade = 0;

    switch (shapetype) {
    case pis_scaled:
// old_ofs = bufferofs;
// for (i=0;i<3;i++)
// {
//      bufferofs = screenloc[i];
//      VWB_Bar(x,y,37,37,InfoAreaSetup.backgr_color);
        VW_Bar(x, y, 37, 37, InfoAreaSetup.backgr_color); // JTR changed
        MegaSimpleScaleShape(x + 19, y + 20, shapenum, 37, shade);
// }
// bufferofs = old_ofs;
        width = 37;
        break;

    case pis_latchpic:
        x = (x + 7) & 0xFFF8;
// old_ofs = bufferofs;
// for (i=0;i<3;i++)
// {
//      bufferofs = screenloc[i];
        LatchDrawPic(x >> 3, y, shapenum);
// }
// bufferofs = old_ofs;
        break;

    case pis_pic:
        x = (x + 7) & 0xFFF8;
        width = pictable[shapenum - STARTPICS].width;
        CA_MarkGrChunk(shapenum);
        CA_CacheMarks();
// old_ofs = bufferofs;
// for (i=0;i<3;i++)
// {
//      bufferofs = screenloc[i];
        VWB_DrawPic(x, y, shapenum);
// }
// bufferofs = old_ofs;
        UNCACHEGRCHUNK(shapenum);
        break;

#if NUMSPRITES && 0
    case pis_sprite:
// VW_geDrawSprite(x,y-(spr->orgy>>G_P_SHIFT),shapenum,shapetype == pis_sprite2x);
        break;
#endif

    default:
        break;
    }

    InfoAreaSetup.x += width;
    return x;
}

// --------------------------------------------------------------------------
// AnimatePage()
// --------------------------------------------------------------------------
void AnimatePage()
{
    piAnimInfo* anim = piAnimList;
    piShapeInfo* shape;

    // Dec Timers
    //

    anim->delay += tics;

    if (anim->delay >= anim->maxdelay) {
        InfoAreaSetup.framecount = 3;
        anim->delay = 0;
    }

    // Test framecount - Do we need to draw a shape?
    //

    if (InfoAreaSetup.framecount) {
        // Draw shapes

        switch (anim->animtype) {
        case pia_shapetable:
            shape = &piShapeTable[anim->baseshape + anim->frame];
            DrawShape(anim->x, anim->y, shape->shapenum, shape->shapetype);
            break;

        case pia_grabscript:
            shape = &piShapeTable[anim->baseshape];
            DrawShape(anim->x, anim->y, shape->shapenum + anim->frame, shape->shapetype);
            break;
        }

        // Dec frame count

        InfoAreaSetup.framecount--;
        if (!InfoAreaSetup.framecount) {
            // Have drawn all pages... Inc Frame count

            anim->frame++;
            if (anim->frame == anim->maxframes) {
                anim->frame = 0;
            }
        }
    }

}

#if 0

// --------------------------------------------------------------------------
// AnimatePage()
// --------------------------------------------------------------------------
void AnimatePage(
    int16_t numanims)
{
    piAnimInfo* anim = piAnimList;
    piShapeInfo* shape;

    anim->delay += tics;

    if (anim->delay >= anim->maxdelay) {
        anim->delay = 0;
        anim->frame++;

        if (anim->frame == anim->maxframes) {
            anim->frame = 0;
        }

        switch (anim->animtype) {
        case pia_shapetable:
            shape = &piShapeTable[anim->baseshape + anim->frame];
            DrawShape(anim->x, anim->y, shape->shapenum, shape->shapetype);
            break;

        case pia_grabscript:
            shape = &piShapeTable[anim->baseshape];
            DrawShape(anim->x, anim->y, shape->shapenum + anim->frame, shape->shapetype);
            break;
        }
    }
}

#endif

// ===========================================================================
//
//
//                       STATUS BAR REFRESH ROUTINES
//
//
// ===========================================================================


// ---------------------------------------------------------------------------
// UpdateStatusBar()
// ---------------------------------------------------------------------------
void UpdateStatusBar()
{
    if (playstate == ex_title || playstate == ex_victorious) {
        return;
    }

#ifdef NO_STATUS_BAR
    return;
#endif


    //
    // Call specific status bar managers
    //

    UpdateScore();
    UpdateInfoArea();

    //
    // Refresh Status Area
    //

    if (DrawAmmoPic_COUNT) {
        DrawAmmoPic();
    }

//      if (DrawScoreNum_COUNT)
    DrawScoreNum();

    if (DrawWeaponPic_COUNT) {
        DrawWeaponPic();
    }

    if (DrawRadarGuage_COUNT) {
        DrawRadarGuage();
    }

//      if (DrawAmmoNum_COUNT)
    DrawAmmoNum();

    if (DrawKeyPics_COUNT) {
        DrawKeyPics();
    }

    if (DrawHealthNum_COUNT) {
        DrawHealthNum();
    }

    if (gamestate.flags & (GS_TICS_FOR_SCORE)) {
        DrawScore();
    }

    if (!::is_ps()) {
        ::DrawHealthMonitor();
    }
}

// ---------------------------------------------------------------------------
// ForceUpdateStatusBar() - Force Draw status bar onto ALL display pages
// ---------------------------------------------------------------------------
void ForceUpdateStatusBar()
{
    uint16_t old_ofs, i;

    old_ofs = static_cast<uint16_t>(bufferofs);

    DrawScore();
    DrawWeapon();
    DrawKeys();
    DrawHealth();
    UpdateRadarGuage();

    for (i = 0; i < 3; i++) {
        bufferofs = screenloc[i];
        UpdateStatusBar();
    }

    bufferofs = old_ofs;
}


/*
=============================================================================

 MOVEMENT

=============================================================================
*/



/*
===================
=
= GetBonus
=
===================
*/

uint16_t static_points[] = { 100, // money bag
                           500, // loot
                           250, // gold1
                           500, // gold2
                           750, // gold3
                           1000, // major gold!
                           5000 // bonus
};

using StaticHealthTable = std::vector<std::array<uint16_t, 3>>;

StaticHealthTable static_health;

void initialize_static_health_table()
{
    static_health = {
        { 100, HEALTH2SND, static_cast<uint16_t>(-1), }, // Full Heal
        { 30, HEALTH1SND, static_cast<uint16_t>(-1), }, // First Aid
        { 20, HEALTH1SND, SPR_STAT_45, }, // Steak
        { 15, HEALTH1SND, SPR_STAT_43, }, // Chicken Leg
        { 10, HEALTH1SND, SPR_SANDWICH_WRAPER, }, // Sandwich
        { 8, HEALTH1SND, SPR_CANDY_WRAPER, }, // Candy Bar
        { 5, HEALTH1SND, SPR_STAT_41, }, // Water bowl
        { 5, HEALTH1SND, static_cast<uint8_t>(-1), }, // Water puddle
    };
}

extern std::string bonus_msg24;
extern std::string bonus_msg25;

void GetBonus(
    statobj_t* check)
{
    boolean givepoints = false;
    int16_t shapenum = -1;

    switch (check->itemnumber) {
    case bo_red_key:
    case bo_yellow_key:
    case bo_blue_key:
    case bo_green_key:
    case bo_gold_key:
        {
            uint16_t keynum = 0;

            if (::is_aog()) {
                switch (check->itemnumber) {
                case bo_red_key:
                    keynum = 0;
                    break;

                case bo_yellow_key:
                    keynum = 1;
                    break;

                case bo_blue_key:
                    keynum = 2;
                    break;

                case bo_green_key:
                    keynum = 3;
                    break;

                case bo_gold_key:
                    keynum = 4;
                    break;
                }
            } else {
                keynum = check->itemnumber - bo_red_key;
            }

            if (gamestate.numkeys[keynum] >= MAXKEYS) {
                return;
            }

            GiveKey(keynum);

            ::sd_play_player_sound(GETKEYSND, bstone::AC_ITEM);

            TravelTable[check->tilex][check->tiley] &= ~TT_KEYS;
            break;
        }

    case bo_money_bag:
        ::sd_play_player_sound(BONUS1SND, bstone::AC_ITEM);
        givepoints = true;
        break;

    case bo_loot:
        ::sd_play_player_sound(BONUS2SND, bstone::AC_ITEM);

        givepoints = true;
        break;


    case bo_gold1:
    case bo_gold2:
    case bo_gold3:
    case bo_gold:
        ::sd_play_player_sound(BONUS3SND, bstone::AC_ITEM);
        givepoints = true;
        break;


    case bo_bonus:
#if 0
        SD_PlaySound(BONUS4SND);
#endif // 0

        ::sd_play_player_sound(BONUS4SND, bstone::AC_ITEM);

        givepoints = true;
        break;

    case bo_water_puddle:
        if (gamestate.health > 15) {
            return;
        }
    case bo_fullheal:
    case bo_firstaid:
    case bo_ham: // STEAK
    case bo_chicken:
    case bo_sandwich:
    case bo_candybar:
    case bo_water:
        if (gamestate.health == 100) {
            return;
        }

        ::sd_play_player_sound(static_health[check->itemnumber - bo_fullheal][1],
                               bstone::AC_ITEM);

        HealSelf(static_health[check->itemnumber - bo_fullheal][0]);
        check->flags &= ~FL_BONUS;
        shapenum = static_health[check->itemnumber - bo_fullheal][2];
        break;

    case bo_clip:
        if (gamestate.ammo == MAX_AMMO) {
            return;
        }
        GiveAmmo(8);
        bonus_msg7[45] = '8';
        break;

    case bo_clip2: {
        uint8_t ammo;

        if (gamestate.ammo == MAX_AMMO) {
            return;
        }

        ammo = 1 + (US_RndT() & 0x7);
        bonus_msg7[45] = '0' + ammo;
        GiveAmmo(ammo);
    }
    break;

    case bo_plasma_detonator:
        TravelTable[check->tilex][check->tiley] &= ~TT_KEYS;
        GivePlasmaDetonator(1);
        ::sd_play_player_sound(GETDETONATORSND, bstone::AC_ITEM);
        break;

    case bo_pistol:
        ::sd_play_player_sound(GETPISTOLSND, bstone::AC_ITEM);
        GiveWeapon(wp_pistol);
        break;

    case bo_burst_rifle:
        ::sd_play_player_sound(GETBURSTRIFLESND, bstone::AC_ITEM);
        GiveWeapon(wp_burst_rifle);
        break;

    case bo_ion_cannon:
        ::sd_play_player_sound(GETIONCANNONSND, bstone::AC_ITEM);
        GiveWeapon(wp_ion_cannon);
        break;

    case bo_grenade:
        ::sd_play_player_sound(GETCANNONSND, bstone::AC_ITEM);
        GiveWeapon(wp_grenade);
        break;

    case bo_bfg_cannon:
        ::sd_play_player_sound(GETCANNONSND, bstone::AC_ITEM);
        GiveWeapon(wp_bfg_cannon);
        break;

    case bo_coin:
        if (gamestate.tokens == MAX_TOKENS) {
            return;
        }
        GiveToken(1);

        writeTokenStr(bonus_msg24);
        break;

    case bo_coin5:
        if (gamestate.tokens == MAX_TOKENS) {
            return;
        }
        GiveToken(5);

        writeTokenStr(bonus_msg25);
        break;

    case bo_automapper1:
        if (gamestate.rpower > MAX_RADAR_ENERGY - (RADAR_PAK_VALUE / 8)) {
            return;
        }
        gamestate.rpower += RADAR_PAK_VALUE;

        ::sd_play_player_sound(RADAR_POWERUPSND, bstone::AC_ITEM);

        UpdateRadarGuage();
        break;
    }

    if (givepoints) {
        GivePoints(static_points[check->itemnumber - bo_money_bag], true);
#if IN_DEVELOPMENT
#ifdef DEBUG_STATICS
        debug_bonus[1][db_count++] = static_points[check->itemnumber - bo_money_bag];
#endif
#endif
    }

    DISPLAY_TIMED_MSG(BonusMsg[check->itemnumber - 1], MP_BONUS, MT_BONUS);
    StartBonusFlash();
    check->shapenum = shapenum; // remove from list if shapenum == -1
    check->itemnumber = bo_nothing;
}

void writeTokenStr(
    std::string& str)
{
    auto token_string = std::to_string(gamestate.tokens);

    if (token_string.length() == 1) {
        token_string = '0' + token_string;
    }

    str.erase(str.length() - 2, 2);
    str += token_string;
}


/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/


boolean TryMove(
    objtype* ob)
{
    int16_t xl, yl, xh, yh, x, y, xx, yy;
    objtype* check;
    int32_t deltax, deltay;

    if (ob == player) {
        xl = (ob->x - PLAYERSIZE) >> TILESHIFT;
        yl = (ob->y - PLAYERSIZE) >> TILESHIFT;
        xh = (ob->x + PLAYERSIZE) >> TILESHIFT;
        yh = (ob->y + PLAYERSIZE) >> TILESHIFT;
    } else {
        if (ob->obclass == blakeobj) {
            xl = (ob->x - (0x1000l)) >> TILESHIFT;
            yl = (ob->y - (0x1000l)) >> TILESHIFT;
            xh = (ob->x + (0x1000l)) >> TILESHIFT;
            yh = (ob->y + (0x1000l)) >> TILESHIFT;
        } else {
            xl = (ob->x - (0x7FFFl)) >> TILESHIFT;
            yl = (ob->y - (0x7FFFl)) >> TILESHIFT;
            xh = (ob->x + (0x7FFFl)) >> TILESHIFT;
            yh = (ob->y + (0x7FFFl)) >> TILESHIFT;
        }
    }


//
// check for solid walls
//

    for (y = yl; y <= yh; y++) {
        for (x = xl; x <= xh; x++) {
            if (check = actorat[x][y]) {
                if ((check < objlist) || (check->flags & FL_FAKE_STATIC)) {
                    return false;
                }
            }
        }
    }

//
// check for actors....
//

    yl -= 2;
    yh += 2;
    xl -= 2;
    xh += 2;

    // NOTE: xl,yl may go NEGITIVE!
    //  ----  xh,yh may exceed 63 (MAPWIDTH-1)

    for (y = yl; y <= yh; y++) {
        for (x = xl; x <= xh; x++) {
            xx = x & 0x3f;

            yy = y & 0x3f;

            check = actorat[xx][yy];

            if ((check > objlist) && ((check->flags & (FL_SOLID | FL_FAKE_STATIC)) == FL_SOLID)) {
                deltax = ob->x - check->x;
                if ((deltax < -MINACTORDIST) || (deltax > MINACTORDIST)) {
                    continue;
                }

                deltay = ob->y - check->y;
                if ((deltay < -MINACTORDIST) || (deltay > MINACTORDIST)) {
                    continue;
                }

                return false;
            }
        }
    }

    return true;
}


/*
===================
=
= ClipMove
=
= returns true if object hit a wall
=
===================
*/

boolean ClipMove(
    objtype* ob,
    int32_t xmove,
    int32_t ymove)
{
    int32_t basex, basey;

    basex = ob->x;
    basey = ob->y;

    ob->x = (basex + xmove);
    ob->y = (basey + ymove);

    if (TryMove(ob)) {
        return false;
    }

#if (!BETA_TEST) && IN_DEVELOPMENT
    if ((!(gamestate.flags & GS_CLIP_WALLS)) && (ob == player)) {
        return true;
    }
#endif

    if (!g_no_wall_hit_sound) {
        if (!::sd_is_player_channel_playing(bstone::AC_HIT_WALL)) {
            ::sd_play_player_sound(HITWALLSND, bstone::AC_HIT_WALL);
        }
    }

    ob->x = (basex + xmove);
    ob->y = basey;

    if (TryMove(ob)) {
        return true;
    }

    ob->x = basex;
    ob->y = (basey + ymove);


    if (TryMove(ob)) {
        return true;
    }

    ob->x = basex;
    ob->y = basey;

    return true;
}

// ==========================================================================

/*
===================
=
= Thrust
=
===================
*/

void Thrust(
    int16_t angle,
    int32_t speed)
{
    extern uint8_t TravelTable[MAPSIZE][MAPSIZE];
    objtype dumb;
    int32_t xmove, ymove;
    uint16_t offset, * map[2];
    int16_t dx, dy;
    int16_t dangle;
    boolean ignore_map1;

    thrustspeed += speed;
//
// moving bounds speed
//
    if (speed >= MINDIST * 2) {
        speed = MINDIST * 2 - 1;
    }

    xmove = FixedByFrac(speed, costable[angle]);
    ymove = -FixedByFrac(speed, sintable[angle]);

    ClipMove(player, xmove, ymove);

    player_oldtilex = player->tilex;
    player_oldtiley = player->tiley;
    player->tilex = static_cast<uint8_t>(player->x >> TILESHIFT); // scale to tile values
    player->tiley = static_cast<uint8_t>(player->y >> TILESHIFT);

    player->areanumber = GetAreaNumber(player->tilex, player->tiley);
    areabyplayer[player->areanumber] = true;
    TravelTable[player->tilex][player->tiley] |= TT_TRAVELED;

    offset = farmapylookup[player->tiley] + player->tilex;
    map[0] = mapsegs[0] + offset;
    map[1] = mapsegs[1] + offset;

// Check for trigger tiles.
//
    switch (*map[0]) {
    case DOORTRIGGERTILE:
        dx = *map[1] >> 8; // x
        dy = *map[1] & 255; // y
        if (OperateSmartSwitch(dx, dy, ST_TOGGLE, false)) { // Operate & Check for removeal
            *map[0] = AREATILE + player->areanumber;    // Remove switch
        }
        ignore_map1 = true;
        break;

    case SMART_OFF_TRIGGER:
    case SMART_ON_TRIGGER:
        dx = *map[1] >> 8;
        dy = *map[1] & 255;
        OperateSmartSwitch(dx, dy, static_cast<char>((*map[0]) - SMART_OFF_TRIGGER), false);
        ignore_map1 = true;
        break;

    case WINTIGGERTILE:
        playstate = ex_victorious;
        dumb.x = ((int32_t)gamestate.wintilex << TILESHIFT) + TILEGLOBAL / 2;
        dumb.y = ((int32_t)gamestate.wintiley << TILESHIFT) + TILEGLOBAL / 2;
        dumb.flags = 0;
        dangle = CalcAngle(player, &dumb);
        RotateView(dangle, 2);
        if (!::is_ps()) {
            RunBlakeRun();
        }
        ignore_map1 = true;
        break;

    default:
        ignore_map1 = false;
        break;
    }

    if (!ignore_map1) {
        // Change sky and ground color on-the-fly.
        //

        offset = *(map[1] + 1); // 'offset' used as temp...
        switch (*map[1]) {
#ifdef CEILING_FLOOR_COLORS
        case 0xfe00:
            TopColor = offset & 0xff00;
            TopColor |= TopColor >> 8;
            BottomColor = offset & 0xff;
            BottomColor |= BottomColor << 8;
            break;
#else
#if IN_DEVELOPMENT
        case 0xfe00:
            // Give error
            break;
#endif
#endif

#if 0
        case 0xF600: // Lighting effects
            normalshade_div = (offset & 0xff00) >> 8;
            if (normalshade_div > 12) {
                AGENT_ERROR(NORMAL_SHADE_TOO_BIG);
            }
            shade_max = offset & 0xff;
            if (shade_max > 63 || shade_max < 5) {
                AGENT_ERROR(SHADEMAX_VALUE_BAD);
            }
            normalshade = (3 * (maxscale >> 2)) / normalshade_div;
            break;
#endif
        }
    }

}

extern int16_t an_offset[];

boolean GAN_HiddenArea;

// ------------------------------------------------------------------------
// GetAreaNumber()
// ------------------------------------------------------------------------
char GetAreaNumber(
    char tilex,
    char tiley)
{
    ::GAN_HiddenArea = false;

    // Are we on a wall?
    //
    if (::tilemap[static_cast<int>(tilex)][static_cast<int>(tiley)] != 0 &&
        (::tilemap[static_cast<int>(tilex)][static_cast<int>(tiley)] & 0xC0) == 0)
    {
        return 127;
    }

    // Get initial areanumber from map
    //
    auto offset = ::farmapylookup[static_cast<int>(tiley)] + tilex;

    const uint16_t* ptr[2] = {
        mapsegs[0] + offset,
        mapsegs[1] + offset,
    }; // ptr

    // Special tile areas must use a valid areanumber tile around it.
    //
    auto areanumber = ::ValidAreaTile(ptr[0]);

    if (areanumber == 0) {
        auto loop = 0;

        for (loop = 0; loop < 8; ++loop) {
            auto offset_delta = ::an_offset[loop];
            auto new_offset = offset + offset_delta;

            if (new_offset < 0 || new_offset >= (MAPSIZE * MAPSIZE)) {
                continue;
            }

            areanumber = ::ValidAreaTile(ptr[0] + offset_delta);

            if (areanumber != 0) {
                break;
            }
        }

        if (loop == 8) {
            areanumber = AREATILE;
        }
    }

// Merge hidden areanumbers into non-hidden areanumbers AND pull all
// values down to an indexable range.
//
    if (areanumber >= HIDDENAREATILE) {
        ::GAN_HiddenArea = true;
        areanumber -= HIDDENAREATILE;
    } else {
        areanumber -= AREATILE;
    }

    return areanumber;
}


// ------------------------------------------------------------------------
// ValidAreaTile()
// ------------------------------------------------------------------------
uint8_t ValidAreaTile(
    const uint16_t* ptr)
{
    switch (*ptr) {
    case AREATILE:
    case HIDDENAREATILE:
    case DOORTRIGGERTILE:
    case WINTIGGERTILE:
    case SMART_ON_TRIGGER:
    case SMART_OFF_TRIGGER:
    case AMBUSHTILE:
    case LINC_TILE:
    case CLOAK_AMBUSH_TILE:
        break;

    default:
        if (*ptr > AREATILE) {
            return static_cast<uint8_t>(*ptr);
        }
        break;
    }

    return 0;
}


/*
=============================================================================

 ACTIONS

=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire()
{
    if (noShots) {
        return;
    }

    if ((gamestate.weapon == wp_autocharge) && (gamestate.weapon_wait)) {
        return;
    }

    buttonheld[bt_attack] = true;

    gamestate.weaponframe = 0;

    player->state = &s_attack;

    gamestate.attackframe = 0;
    gamestate.attackcount = attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].tics;
    gamestate.weaponframe = attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].frame;
}

// ===========================================================================


void Cmd_Use()
{
    int16_t checkx, checky, doornum, dir;
    uint16_t iconnum;
    uint8_t static interrogate_delay = 0;

    boolean tryDetonator = false;

// Find which cardinal direction the player is facing
//
    if (player->angle < ANGLES / 8 || player->angle > 7 * ANGLES / 8) {
        checkx = player->tilex + 1;
        checky = player->tiley;
        dir = di_east;
    } else if (player->angle < 3 * ANGLES / 8) {
        checkx = player->tilex;
        checky = player->tiley - 1;
        dir = di_north;
    } else if (player->angle < 5 * ANGLES / 8) {
        checkx = player->tilex - 1;
        checky = player->tiley;
        dir = di_west;
    } else {
        checkx = player->tilex;
        checky = player->tiley + 1;
        dir = di_south;
    }

    doornum = tilemap[checkx][checky];
    iconnum = *(mapsegs[1] + farmapylookup[checky] + checkx);

// Test for a pushable wall
//
    if (iconnum == PUSHABLETILE) {
        PushWall(checkx, checky, dir);
    } else if (!buttonheld[bt_use]) {
        // Test for doors / elevator
        //
        if ((doornum & 0x80) && ((pwallx != checkx) || (pwally != checky))) {
            buttonheld[bt_use] = true;
            OperateDoor(doornum & ~0x80);
        } else {
            // Test for special tile types...
            //
            switch (doornum & 63) {
            // Test for 'display elevator buttons'
            //
            case TRANSPORTERTILE: {
                int16_t new_floor;

                if ((new_floor = InputFloor()) != -1 && new_floor != gamestate.mapon) {
                    int16_t angle = player->angle;

                    gamestuff.level[gamestate.mapon].ptilex = player->tilex;
                    gamestuff.level[gamestate.mapon].ptiley = player->tiley;

                    if (::is_ps()) {
                        angle = player->angle - 180;
                        if (angle < 0) {
                            angle += ANGLES;
                        }
                    }

                    gamestuff.level[gamestate.mapon].pangle = angle;
                    playstate = ::is_ps() ? ex_transported : ex_warped;

                    gamestate.lastmapon = gamestate.mapon;
                    gamestate.mapon = new_floor - 1;
                } else {
                    DrawPlayScreen(false);
                }
            }
            break;

            case DIRECTTRANSPORTTILE:
                switch (iconnum & 0xff00) {
                case 0xf400:
                    playstate = ex_transported;
                    gamestate.lastmapon = gamestate.mapon;
                    gamestate.mapon = (iconnum & 0xff) - 1;

                    if (!::is_ps()) {
                        gamestuff.level[gamestate.mapon + 1].ptilex = player->tilex;
                        gamestuff.level[gamestate.mapon + 1].ptiley = player->tiley;

                        {
                            int angle = player->angle - 180;

                            if (angle < 0) {
                                angle += ANGLES;
                            }

                            gamestuff.level[gamestate.mapon + 1].pangle = angle;
                        }
                    }
                    break;

                default:
                    // Stay in current level warp to new location

                    playstate = ex_transported;
                    Warped();
                    playstate = ex_stillplaying;

                    player->tilex = (iconnum >> 8);
                    player->tiley = iconnum & 0xff;
                    player->x = ((int32_t)player->tilex << TILESHIFT) + TILEGLOBAL / 2;
                    player->y = ((int32_t)player->tiley << TILESHIFT) + TILEGLOBAL / 2;

                    DrawWarpIn();
                    break;
                }
                break;

            //
            // Test for Wall Switch Activation
            //
            case OFF_SWITCH:
            case ON_SWITCH:
                ActivateWallSwitch(iconnum, checkx, checky);
                break;


            // Test for Concession Machines
            //

            case FOODTILE:
            case SODATILE:
                OperateConcession(static_cast<uint16_t>(reinterpret_cast<size_t>(actorat[checkx][checky])));
                break;

            default:
                if (::is_ps()) {
                    tryDetonator = true;
                }
                break;
            }
        }
    } else if (!interrogate_delay) {
#define INTERROGATEDIST (MINACTORDIST)
#define MDIST 2
#define INTG_ANGLE 45

        char x, y;
        objtype* intg_ob = nullptr, * ob;
        int32_t dx, dy, dist, intg_dist = INTERROGATEDIST + 1;

        for (y = -MDIST; y < MDIST + 1; y++) {
            for (x = -MDIST; x < MDIST + 1; x++) {
                // Don't check outside of the map plane:
                if (player->tilex + x > 63 || player->tiley + y > 63) {
                    continue;
                }

                if ((!tilemap[player->tilex + x][player->tiley + y]) &&
                    (actorat[player->tilex + x][player->tiley + y] >= objlist))
                {
                    ob = actorat[player->tilex + x][player->tiley + y];
                } else {
                    continue;
                }
                dx = player->x - ob->x;
                dx = LABS(dx);
                dy = player->y - ob->y;
                dy = LABS(dy);
                dist = dx < dy ? dx : dy;
                if ((ob->obclass == gen_scientistobj) &&
                    ((ob->flags & (FL_FRIENDLY | FL_VISABLE)) == (FL_FRIENDLY | FL_VISABLE)) &&
                    (dist < intg_dist))
                {
                    if ((ob->flags & FL_ATTACKMODE) != 0) {
                        ob->flags &= ~(FL_FRIENDLY | FL_INFORMANT);
                    } else {
                        int16_t angle = CalcAngle(player, ob);

                        angle = ABS(player->angle - angle);
                        if (angle > INTG_ANGLE / 2) {
                            continue;
                        }

                        intg_ob = ob;
                        intg_dist = dist;
                    }
                }
            }
        }

        if (intg_ob) {
            if (Interrogate(intg_ob)) {
                interrogate_delay = 20; // Informants have 1/3 sec delay
            } else {
                interrogate_delay = 120; // Non-informants have 2 sec delay
            }
        } else if (::is_ps()) {
            tryDetonator = true;
        }
    } else {
        if (tics < interrogate_delay) {
            interrogate_delay -= static_cast<uint8_t>(tics);
        } else {
            interrogate_delay = 0;
        }

        if (::is_ps()) {
            tryDetonator = true;
        }
    }

    if (::is_ps()) {
        if (tryDetonator) {
            if ((!tryDetonatorDelay) && gamestate.plasma_detonators) {
                TryDropPlasmaDetonator();
                tryDetonatorDelay = 60;
            }
        } else {
            tryDetonatorDelay = 60;
        }
    }

    if (!buttonheld[bt_use]) {
        interrogate_delay = 0;
    }
}

// ==========================================================================
//
//                           INTERROGATE CODE
//
// ==========================================================================

#define MSG_BUFFER_LEN 150

char msg[MSG_BUFFER_LEN + 1];

char* InfAreaMsgs[MAX_INF_AREA_MSGS];
uint8_t NumAreaMsgs, LastInfArea;
int16_t FirstGenInfMsg, TotalGenInfMsgs;

scientist_t InfHintList; // Informant messages
scientist_t NiceSciList; // Non-informant, non-pissed messages
scientist_t MeanSciList; // Non-informant, pissed messages

char int_interrogate[] = "INTERROGATE:",
     int_informant[] = " ^FC3aINFORMANT^FCa6",
     int_rr[] = "\r\r",
     int_xx[] = "^XX",
     int_haveammo[] = " HEY BLAKE,\r TAKE MY CHARGE PACK!",
     int_havetoken[] = " HEY BLAKE,\r TAKE MY FOOD TOKENS!";

// --------------------------------------------------------------------------
// Interrogate()
// --------------------------------------------------------------------------
boolean Interrogate(
    objtype* ob)
{
    boolean rt_value = true;
    char* msgptr = nullptr;

    strcpy(msg, int_interrogate);

    if (ob->flags & FL_INFORMANT) { // Informant
        strcat(msg, int_informant);

        if (ob->flags & FL_INTERROGATED) {
            if ((ob->flags & FL_HAS_AMMO) && (gamestate.ammo != MAX_AMMO)) {
                GiveAmmo((US_RndT() % 8) + 1);
                ob->flags &= ~FL_HAS_AMMO;
                msgptr = int_haveammo;
            } else if ((ob->flags & FL_HAS_TOKENS) && (gamestate.tokens != MAX_TOKENS)) {
                GiveToken(5);
                ob->flags &= ~FL_HAS_TOKENS;
                msgptr = int_havetoken;
            }
        }

        if (!msgptr) {
            // If new areanumber OR no 'area msgs' have been compiled, compile
            // a list of all special messages for this areanumber.
            //
            if ((LastInfArea == 0xff) || (LastInfArea != ob->areanumber)) {
                sci_mCacheInfo* ci = InfHintList.smInfo;

                NumAreaMsgs = 0;
                for (; ci->areanumber != 0xff; ci++) {
                    if (ci->areanumber == ob->areanumber) {
                        InfAreaMsgs[NumAreaMsgs++] = InfHintList.smInfo[ci->mInfo.local_val].mInfo.mSeg;
                    }
                }

                LastInfArea = ob->areanumber;
            }

            // Randomly select an informant hint, either: specific to areanumber
            // or general hint...
            //
            if (NumAreaMsgs) {
                if (ob->ammo != ob->areanumber) {
                    ob->s_tilex = 0xff;
                }
                ob->ammo = ob->areanumber;
                if (ob->s_tilex == 0xff) {
                    ob->s_tilex = static_cast<uint8_t>(Random(NumAreaMsgs));
                }
                msgptr = InfAreaMsgs[ob->s_tilex];
            } else {
                if (ob->s_tiley == 0xff) {
                    ob->s_tiley = static_cast<uint8_t>(FirstGenInfMsg + Random(TotalGenInfMsgs));
                }
                msgptr = InfHintList.smInfo[ob->s_tiley].mInfo.mSeg;
            }

            // Still no msgptr? This is a shared message! Use smInfo[local_val]
            // for this message.
            //
            if (!msgptr) {
                msgptr = InfHintList.smInfo[InfHintList.smInfo[ob->s_tiley].mInfo.local_val].mInfo.mSeg;
            }

            ob->flags |= FL_INTERROGATED; // Scientist has been interrogated
        }
    } else { // Non-Informant
        scientist_t* st;

        rt_value = false;
        if ((ob->flags & FL_MUST_ATTACK) || (US_RndT() & 1)) { // Mean
            ob->flags &= ~FL_FRIENDLY; // Make him attack!
            ob->flags |= FL_INTERROGATED; //  "    "     "
            st = &MeanSciList;
        } else { // Nice
            ob->flags |= FL_MUST_ATTACK; // Make him mean!
            st = &NiceSciList;
        }

        msgptr = st->smInfo[Random(st->NumMsgs)].mInfo.mSeg;
    }

    if (msgptr) {
        strcat(msg, int_rr);
        strcat(msg, msgptr);
        strcat(msg, int_xx);
        if (strlen(msg) > MSG_BUFFER_LEN) {
            AGENT_ERROR(INTERROGATE_LONG_MSG);
        }
        DisplayInfoMsg(msg, MP_INTERROGATE, DISPLAY_MSG_STD_TIME * 2, MT_GENERAL);

        // BBi
        // FIXME Create a new actor channel type for interrogation?
        ::sd_play_player_sound(INTERROGATESND, bstone::AC_ITEM);
    }

    return rt_value;
}

// ==========================================================================
//
//                            ELEVATOR CODE
//
// ==========================================================================


char if_help[] = "UP/DN MOVES SELECTOR - ENTER ACTIVATES";
char if_noImage[] = "   AREA\n"
                    "  UNMAPPED\n"
                    "\n"
                    "\n"
                    " PRESS ENTER\n"
                    " TO TELEPORT";

statsInfoType ov_stats;
static uint8_t* ov_buffer;
boolean ov_noImage = false;

#define TOV_X 16
#define TOV_Y 132

// --------------------------------------------------------------------------
// InputFloor
// --------------------------------------------------------------------------
int16_t InputFloor()
{
    if (::is_aog()) {
        const std::string messages[4] = {
            // "Current floor:\nSelect a floor."
            ::ca_load_script(ELEVMSG0_TEXT),
            // "RED access card used to unlock floor!"
            ::ca_load_script(ELEVMSG1_TEXT),
            // "Floor is locked. Try another floor."
            ::ca_load_script(ELEVMSG4_TEXT),
            // "You must first get the red access keycard!"
            ::ca_load_script(ELEVMSG5_TEXT)
        }; // messages

        const char* const floor_number_strings[10] = {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
        };

        ::CA_CacheGrChunk(STARTFONT + 3);
        ::CacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);

        ::VW_FadeOut();

        ::DrawTopInfo(sp_normal);

        auto border_width = 7;
        auto border_height = 5;
        auto outer_height = 200 - STATUSLINES - TOP_STRIP_HEIGHT;

        ::BevelBox(
            0,
            TOP_STRIP_HEIGHT,
            ::k_ref_width,
            outer_height,
            BORDER_HI_COLOR,
            BORDER_MED_COLOR,
            BORDER_LO_COLOR);

        ::BevelBox(
            border_width,
            TOP_STRIP_HEIGHT + border_height,
            ::k_ref_width - (2 * border_width),
            outer_height - (2 * border_height),
            BORDER_LO_COLOR,
            BORDER_MED_COLOR,
            BORDER_HI_COLOR);

        ::CacheDrawPic(8, TOP_STRIP_HEIGHT + 6, TELEPORTBACKPIC);

        ::fontnumber = 1;
        ::CA_CacheGrChunk(STARTFONT + 1);
        ::CacheBMAmsg(FLOORMSG_TEXT);
        ::UNCACHEGRCHUNK(STARTFONT + 1);

        ::ShowOverhead(
            14,
            TOP_STRIP_HEIGHT + 55,
            32,
            0,
            OV_KEYS | OV_WHOLE_MAP);

        ::IN_ClearKeysDown();

        auto result = -2;
        auto draw_stats = true;
        auto draw_message = true;
        auto draw_current_floor = true;
        auto draw_locked_floor = false;
        auto use_delay = false;
        auto draw_button = false;
        auto button_index = 0;
        auto is_button_pressed = false;
        auto message = &messages[0];

        PresenterInfo pi {};
        pi.xl = 24;
        pi.yl = TOP_STRIP_HEIGHT + 8;
        pi.xh = pi.xl + 210;
        pi.yh = pi.yl + 34;
        pi.fontnumber = 3;
        pi.custom_line_height = 17;

        ::fontcolor = 0x38;

        while (result == -2) {
            ::CalcTics();
            ::in_handle_events();

            if (::Keyboard[sc_escape]) {
                result = -1;
            }

            auto target_level = 0;

            for (int i = sc_1; i <= sc_0; ++i) {
                if (::Keyboard[i]) {
                    target_level = i - sc_1 + 1;
                    break;
                }
            }

            if (target_level > 0 && target_level != ::gamestate.mapon) {
                ::sd_play_player_sound(ELEV_BUTTONSND, bstone::AC_ITEM);

                draw_button = true;
                is_button_pressed = true;
                button_index = target_level - 1;

                if (!::gamestuff.level[target_level].locked) {
                    result = target_level;
                } else if (::gamestate.numkeys[kt_red] > 0 &&
                    target_level == ::gamestate.key_floor)
                {
                    result = target_level;

                    use_delay = true;
                    draw_message = true;
                    draw_current_floor = false;
                    message = &messages[1];

                    ::gamestate.numkeys[kt_red] = 0;
                    ::gamestate.key_floor += 1;
                } else {
                    use_delay = true;
                    draw_message = true;
                    draw_current_floor = false;

                    if (target_level == ::gamestate.key_floor) {
                        draw_locked_floor = false;
                        message = &messages[3];
                    } else {
                        draw_locked_floor = true;
                        message = &messages[2];
                    }
                }
            }

            if (draw_message) {
                draw_message = false;

                ::VWB_DrawPic(24, TOP_STRIP_HEIGHT + 10, TELEPORT_TEXT_BG);

                ::fontcolor = 0x97;
                pi.script[0] = message->c_str();
                pi.flags = TPF_CACHE_NO_GFX | TPF_USE_CURRENT;
                pi.cur_x = 0xFFFF;
                pi.cur_y = 0xFFFF;
                ::TP_InitScript(&pi);
                ::TP_Presenter(&pi);

                if (draw_current_floor) {
                    ::fontnumber = 3;
                    ::fontcolor = 0x38;

                    ::px = 167;
                    ::py = TOP_STRIP_HEIGHT + 10;

                    ::USL_DrawString(
                        floor_number_strings[::gamestate.mapon - 1]);
                }

                if (draw_locked_floor) {
                    ::fontnumber = 3;
                    ::fontcolor = 0x38;

                    ::px = 82;
                    ::py = TOP_STRIP_HEIGHT + 10;

                    ::USL_DrawString(
                        floor_number_strings[target_level - 1]);
                }

                if (draw_button) {
                    draw_button = false;

                    auto base_x = 264;
                    auto base_y = TOP_STRIP_HEIGHT + 98;
                    auto step_x = 24;
                    auto step_y = 20;

                    auto x = base_x + (step_x * (button_index % 2));
                    auto y = base_y - (step_y * (button_index / 2));

                    auto base_pic =
                        is_button_pressed ?
                        TELEPORT1ONPIC :
                        TELEPORT1OFFPIC;

                    ::VWB_DrawPic(
                        x,
                        y,
                        base_pic + button_index);
                }
            }

            ::CycleColors();
            ::VW_UpdateScreen();

            if (::screenfaded) {
                ::VW_FadeIn();
            }

            if (draw_stats) {
                draw_stats = false;

                static_cast<void>(::ShowStats(
                    167,
                    TOP_STRIP_HEIGHT + 76,
                    ss_normal,
                    &::gamestuff.level[::gamestate.mapon].stats));

                ::IN_ClearKeysDown();
            }

            if (use_delay) {
                use_delay = false;
                draw_message = true;
                draw_current_floor = true;
                draw_locked_floor = false;
                draw_button = true;
                is_button_pressed = false;
                message = &messages[0];

                ::IN_UserInput(210);
                ::IN_ClearKeysDown();
            }
        }

        ::IN_ClearKeysDown();

        return static_cast<int16_t>(result);
    } else {
        const auto RADAR_FLAGS = OV_KEYS;
        const auto MAX_TELEPORTS = 20;
        const char MAX_MOVE_DELAY = 10;

        int16_t buttonPic, buttonY;
        int16_t rt_code = -2, tpNum = gamestate.mapon, lastTpNum = tpNum;
        int16_t teleX[MAX_TELEPORTS] = { 16, 40, 86, 23, 44, 62, 83, 27, 118, 161, 161, 161, 213, 213, 184, 205, 226, 256, 276, 276 };
        int16_t teleY[MAX_TELEPORTS] = { 13, 26, 9, 50, 50, 50, 50, 62, 42, 17, 26, 35, 41, 50, 62, 62, 62, 10, 10, 30 };
        char moveActive = 0;
        objtype old_player;
        boolean locked = false, buttonsDrawn = false;

        ClearMemory();
        VW_FadeOut();

        CacheDrawPic(0, 0, TELEPORTBACKTOPPIC);
        CacheDrawPic(0, 12 * 8, TELEPORTBACKBOTPIC);
        DisplayTeleportName(static_cast<char>(tpNum), locked);
        CacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);
        VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1ONPIC + tpNum);

        memcpy(&old_player, player, sizeof(objtype));
        player->angle = 90;
        player->x = player->y = ((int32_t)32 << TILESHIFT) + (TILEGLOBAL / 2);

        ov_buffer = new uint8_t[4096];
        ShowStats(0, 0, ss_justcalc, &gamestuff.level[gamestate.mapon].stats);
        memcpy(&ov_stats, &gamestuff.level[gamestate.mapon].stats, sizeof(statsInfoType));
        ShowOverhead(TOV_X, TOV_Y, 32, 0, RADAR_FLAGS);
        SaveOverheadChunk(tpNum);

        px = 115;
        py = 188;
        fontcolor = 0xaf;
        fontnumber = 2;
        ShPrint(if_help, 0, false);

        controlx = controly = 0;
        IN_ClearKeysDown();
        while (rt_code == -2) {
            // Handle ABORT and ACCEPT
            //
    //              if (!screenfaded)
    //                      PollControls();

            CalcTics();

            // BBi
            ::in_handle_events();

            if (Keyboard[sc_left_arrow]) {
                controlx = -1;
            } else if (Keyboard[sc_right_arrow]) {
                controlx = 1;
            } else {
                controlx = 0;
            }

            if (Keyboard[sc_up_arrow]) {
                controly = -1;
            } else if (Keyboard[sc_down_arrow]) {
                controly = 1;
            } else {
                controly = 0;
            }

            if (Keyboard[sc_escape] || buttonstate[bt_strafe]) {
                rt_code = -1; // ABORT

                LoadLocationText(gamestate.mapon + MAPS_PER_EPISODE * gamestate.episode);
                break;
            } else if (Keyboard[sc_return] || buttonstate[bt_attack]) {
                if (locked) {
                    if (!::sd_is_player_channel_playing(bstone::AC_NO_WAY)) {
                        ::sd_play_player_sound(NOWAYSND, bstone::AC_NO_WAY);
                    }
                } else {
                    char loop;

                    rt_code = tpNum; // ACCEPT

                    // Flash selection
                    //
                    for (loop = 0; loop < 10; loop++) {
                        VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1OFFPIC + tpNum);
                        VW_UpdateScreen();
                        VW_WaitVBL(4);

                        VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1ONPIC + tpNum);
                        VW_UpdateScreen();
                        VW_WaitVBL(4);
                    }

                    break;
                }
            }

            CheckMusicToggle();

            // Handle delay
            //
            if (moveActive) {
                moveActive -= static_cast<char>(tics);
                if (moveActive < 0) {
                    moveActive = 0;
                }
            }

            // Move to NEXT / PREV teleport?
            //
            buttonY = 0;
            if (controlx > 0 || controly > 0) {
                if (!moveActive && tpNum < MAX_TELEPORTS - 1) {
                    tpNum++; // MOVE NEXT
                    moveActive = MAX_MOVE_DELAY;
                }

                buttonPic = TELEDNONPIC;
                buttonY = 104;
            } else if (controlx < 0 || controly < 0) {
                if (!moveActive && tpNum) {
                    tpNum--; // MOVE PREV
                    moveActive = MAX_MOVE_DELAY;
                }

                buttonPic = TELEUPONPIC;
                buttonY = 91;
            }

            // Light buttons?
            //
            if (buttonY) {
                VWB_DrawMPic(34, 91, TELEUPOFFPIC);
                VWB_DrawMPic(270, 91, TELEUPOFFPIC);
                VWB_DrawMPic(34, 104, TELEDNOFFPIC);
                VWB_DrawMPic(270, 104, TELEDNOFFPIC);

                VWB_DrawMPic(34, buttonY, buttonPic);
                VWB_DrawMPic(270, buttonY, buttonPic);
                buttonsDrawn = true;
            } else
            // Unlight buttons?
            //
            if (buttonsDrawn) {
                VWB_DrawMPic(34, 91, TELEUPOFFPIC);
                VWB_DrawMPic(270, 91, TELEUPOFFPIC);
                VWB_DrawMPic(34, 104, TELEDNOFFPIC);
                VWB_DrawMPic(270, 104, TELEDNOFFPIC);
                buttonsDrawn = false;
            }

            // Change visual information
            //
            if (tpNum != lastTpNum) {
                locked = gamestuff.level[tpNum].locked;
                DisplayTeleportName(static_cast<char>(tpNum), locked);

                VWB_DrawMPic(teleX[lastTpNum], teleY[lastTpNum], TELEPORT1OFFPIC + lastTpNum);
                VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1ONPIC + tpNum);

                LoadOverheadChunk(tpNum);
                ShowOverheadChunk();
                if (ov_noImage) {
                    fontcolor = 0x57;
                    WindowX = WindowW = TOV_X;
                    WindowY = WindowH = TOV_Y;
                    WindowW += 63;
                    WindowH += 63;
                    PrintX = TOV_X + 5;
                    PrintY = TOV_Y + 13;
                    US_Print(if_noImage);
                }
                lastTpNum = tpNum;
            }

            if (locked) {
                ShowOverhead(TOV_X, TOV_Y, 32, -1, RADAR_FLAGS);
            }

            CycleColors();
            VW_UpdateScreen();
            if (screenfaded) {
                VW_FadeIn();
                ShowStats(235, 138, ss_normal, &ov_stats);
                IN_ClearKeysDown();
                controlx = controly = 0;
            }
        }

#if 0
        for (buttonY = 63; buttonY >= 0; buttonY -= 2) {
            char shps[] = { TELEPORT1ONPIC, TELEPORT1OFFPIC };

            if (rt_code != -1) {
                VWB_DrawMPic(teleX[tpNum], teleY[tpNum], shps[(buttonY & 4) >> 2] + tpNum);
            }

            if (locked) {
                ShowOverhead(TOV_X, TOV_Y, 32, -locked, RADAR_FLAGS);
            }

            CycleColors();
            VL_SetPaletteIntensity(0, 255, &vgapal, buttonY);
            VW_UpdateScreen();
        }
#else
        VW_FadeOut();
#endif

        delete [] ov_buffer;
        ov_buffer = nullptr;

        memcpy(player, &old_player, sizeof(objtype));
        UnCacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);

        DrawPlayScreen(false);
        IN_ClearKeysDown();

        return rt_code;
    }
}

// --------------------------------------------------------------------------
// ShowOverheadChunk()
// --------------------------------------------------------------------------
void ShowOverheadChunk()
{
    VL_MemToScreen(static_cast<const uint8_t*>(ov_buffer), 64, 64, TOV_X, TOV_Y);
    ShowStats(235, 138, ss_quick, &ov_stats);
}

// --------------------------------------------------------------------------
// LoadOverheadChunk()
// --------------------------------------------------------------------------
void LoadOverheadChunk(
    int tpNum)
{
    // Find and load chunk
    //
    ::g_playtemp.set_position(0);

    std::string chunk_name = "OV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << tpNum).to_string();

    bool is_succeed = true;
    uint32_t checksum = 0;
    bstone::BinaryReader reader(&g_playtemp);

    if (::FindChunk(&g_playtemp, chunk_name) > 0) {
        try {
            ::deserialize_field(
                reinterpret_cast<uint8_t(&)[4096]>(ov_buffer[0]),
                reader, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }

        ov_stats.deserialize(reader, checksum);

        uint32_t saved_checksum = 0;
        is_succeed &= reader.read(saved_checksum);
        bstone::Endian::lei(saved_checksum);
        is_succeed &= (saved_checksum == checksum);
    } else {
        is_succeed = false;
    }

    if (!is_succeed) {
        std::uninitialized_fill_n(
            ov_buffer,
            4096,
            0x52);

        std::uninitialized_fill_n(
            reinterpret_cast<uint8_t*>(&ov_stats),
            sizeof(statsInfoType),
            0);
    }

    ov_noImage = !is_succeed;
}

// --------------------------------------------------------------------------
// SaveOverheadChunk()
// --------------------------------------------------------------------------
void SaveOverheadChunk(
    int tpNum)
{
    // Remove level chunk from file
    //
    std::string chunk_name = "OV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << tpNum).to_string();

    ::DeleteChunk(g_playtemp, chunk_name);

    // Prepare buffer
    //
    ::VL_ScreenToMem(ov_buffer, 64, 64, TOV_X, TOV_Y);

    uint32_t checksum = 0;
    bstone::BinaryWriter writer(&g_playtemp);

    // Write chunk ID, SIZE, and IMAGE
    //
    g_playtemp.seek(0, bstone::StreamSeekOrigin::end);
    g_playtemp.write(chunk_name.c_str(), 4);
    g_playtemp.skip(4);

    int64_t beg_offset = g_playtemp.get_position();

    ::serialize_field(
        reinterpret_cast<const uint8_t(&)[4096]>(ov_buffer[0]),
        writer, checksum);
    ov_stats.serialize(writer, checksum);
    writer.write(bstone::Endian::le(checksum));

    int64_t end_offset = g_playtemp.get_position();
    int32_t chunk_size = static_cast<int32_t>(end_offset - beg_offset);
    g_playtemp.seek(-(chunk_size + 4), bstone::StreamSeekOrigin::current);
    writer.write(bstone::Endian::le(chunk_size));
}

// --------------------------------------------------------------------------
// DisplayTeleportName()
// --------------------------------------------------------------------------
void DisplayTeleportName(
    char tpNum,
    boolean locked)
{
    const char* s;
    int w;
    int h;

    if (locked) {
        fontcolor = 0xf5;
        s = "-- TELEPORT DISABLED --";
    } else {
        fontcolor = 0x57;
        LoadLocationText(tpNum);
        s = LocationText;
    }
    VW_MeasurePropString(s, &w, &h);
    py = 103;
    px = 160 - w / 2;
    VW_Bar(54, 101, 212, 9, 0x52);
    ShPrint(s, 0, false);
}

// --------------------------------------------------------------------------
// CacheDrawPic()
// --------------------------------------------------------------------------
void CacheDrawPic(
    int x,
    int y,
    int pic)
{
    CA_CacheGrChunk(static_cast<int16_t>(pic));
    VWB_DrawPic(x, y, pic);
    UNCACHEGRCHUNK(static_cast<uint16_t>(pic));
}

// ===========================================================================
//
// MISSION STATISTICS CODE
//
// ===========================================================================

#define BAR_W 48
#define BAR_H 5

#define BAR1_COLOR 0xe0
#define BAR2_COLOR 0x30
#define BAR3_COLOR 0x10

#define PERC_W 13
#define PERC_H 5

boolean show_stats_quick;

// --------------------------------------------------------------------------
// ShowStats()
// --------------------------------------------------------------------------
int16_t ShowStats(
    int16_t bx,
    int16_t by,
    ss_type type,
    statsInfoType* stats)
{
    int16_t floor, total = 0, mission = 0, p1, p2, p3, loop, maxPerFloor;

// Define max points per floor...
//
    if (stats->total_points || stats->total_inf || stats->total_enemy) {
        maxPerFloor = 300;
    } else {
        maxPerFloor = 0;
    }

// Setup to test for bypassing stats.
//
    LastScan = sc_none;

    if (type == ss_quick) {
        show_stats_quick = true;
    } else {
        show_stats_quick = false;
    }

// Show ratio for each statistic:
//
//      TOTAL POINTS, INFORMANTS ALIVE, ENEMY DESTROYED,
//      OVERALL FLOOR, OVERALL MISSION
//

// Show TOTAL POINTS ratio.
//
    p1 = ShowRatio(bx, by, bx + 52, by, stats->total_points, stats->accum_points, type);

// Show INFORMANTS ALIVE ratio.
//
    by += 7;
    p2 = ShowRatio(bx, by, bx + 52, by, stats->total_inf, stats->accum_inf, type);

// Show ENEMY DESTROYED ratio.
//
    by += 7;
    p3 = ShowRatio(bx, by, bx + 52, by, stats->total_enemy, stats->accum_enemy, type);

// Show OVERALL FLOOR ratio.
//
    by += (::is_ps() ? 13 : 12);
    floor = p1 + p2 + p3;
    ShowRatio(bx, by, bx + 52, by, maxPerFloor, floor, type);

// Show OVERALL MISSION ratio.
//
    by += 7;
    stats->overall_floor = floor;
    for (loop = 0; loop < MAPS_WITH_STATS; loop++) {
        total += 300;
        mission += gamestuff.level[loop].stats.overall_floor;
    }
    mission = ShowRatio(bx, by, bx + 52, by, total, mission, type);

    if (show_stats_quick) {
        VW_UpdateScreen();
    }

    return mission;
}

// --------------------------------------------------------------------------
// ShowRatio()
// --------------------------------------------------------------------------
uint8_t ShowRatio(
    int16_t bx,
    int16_t by,
    int16_t nx,
    int16_t ny,
    int32_t total,
    int32_t perc,
    ss_type type)
{
    char numbars;
    char maxperc;
    char percentage = 1, loop;

//      if (perc > total)
//              perc = total;

// Catch those nasty divide-by-zeros!
//
    if (total) {
        maxperc = static_cast<char>(LRATIO(100, total, perc, 10));
        numbars = LRATIO(48, 100, maxperc, 10);
    } else {
        if (type != ss_justcalc) {
            fontcolor = 0x57;
            VW_Bar(bx, by, BAR_W, BAR_H, 0);
            VW_Bar(nx, ny, PERC_W + 6, PERC_H, 0);
            PrintX = nx;
            PrintY = ny;
            US_Print("N/A");
        }
        return 100;
    }

    if (type == ss_justcalc) {
        return maxperc;
    }

    PrintY = ny;
    fontcolor = 0xaf;
    fontnumber = 2;

    VW_Bar(bx, by, BAR_W, BAR_H, 0x07);
    PrintStatPercent(nx, ny, 0);
    for (loop = 0; loop < numbars; loop++) {
        if (LastScan) {
            show_stats_quick = true;
        }

        // Print one line of bar
        //
        VL_Vlin(bx++, by, BAR_H, 0xc8);

        // Keep up with current percentage
        //
        if (loop == numbars - 1) {
            percentage = maxperc;
        } else {
            percentage += 2;
        }

        PrintStatPercent(nx, ny, percentage);

        if (!show_stats_quick) {
            if (!(loop % 2)) {
                ::sd_play_player_sound(STATS1SND, bstone::AC_ITEM);
            }
            VW_WaitVBL(1);
            VW_UpdateScreen();
        }
    }

    if (!show_stats_quick && numbars) {
        ::sd_play_player_sound(STATS2SND, bstone::AC_ITEM);

        while (::SD_SoundPlaying() && !LastScan) {
            ::in_handle_events();
        }
    }

    return maxperc;
}

// --------------------------------------------------------------------------
// PrintStatPercent()
// --------------------------------------------------------------------------
void PrintStatPercent(
    int16_t nx,
    int16_t ny,
    char percentage)
{
    if (percentage < 10) {
        PrintX = nx + 9;
    } else if (percentage < 100) {
        PrintX = nx + 4;
    } else {
        PrintX = nx - 1;
    }

    VW_Bar(nx, ny, PERC_W + 5, PERC_H, 0);
    US_PrintUnsigned(percentage);
    US_Print("%");
}

// --------------------------------------------------------------------------
// PerfectStats()
// --------------------------------------------------------------------------
boolean PerfectStats()
{
    if ((gamestuff.level[gamestate.mapon].stats.total_points == gamestuff.level[gamestate.mapon].stats.accum_points) &&
        (gamestuff.level[gamestate.mapon].stats.total_inf == gamestuff.level[gamestate.mapon].stats.accum_inf) &&
        (gamestuff.level[gamestate.mapon].stats.total_enemy == gamestuff.level[gamestate.mapon].stats.accum_enemy))
    {
        return true;
    }

    return false;
}

// ===========================================================================
//
// PINBALL BONUS DISPLAY CODE
//
// ===========================================================================

// --------------------------------------------------------------------------
// B_GAliFunc()
// --------------------------------------------------------------------------
void B_GAliFunc()
{
    extern char B_GAlienDead2[];

    if (gamestate.episode == 5) {
        DisplayInfoMsg(B_GAlienDead2, MP_PINBALL_BONUS, 7 * 60, MT_BONUS);
    }
}

// --------------------------------------------------------------------------
// B_EManFunc()
// --------------------------------------------------------------------------
void B_EManFunc()
{
    uint16_t temp, i;

    ::sd_play_player_sound(EXTRA_MANSND, bstone::AC_ITEM);

    fontnumber = 2;

    temp = static_cast<uint16_t>(bufferofs);

    for (i = 0; i < 3; i++) {
        bufferofs = screenloc[i];
        LatchDrawPic(0, 0, TOP_STATUSBARPIC);
        ShadowPrintLocationText(sp_normal);
    }

    bufferofs = temp;
}

// --------------------------------------------------------------------------
// B_MillFunc()
// --------------------------------------------------------------------------
void B_MillFunc()
{
    GiveAmmo(99);
    HealSelf(99);
}

// --------------------------------------------------------------------------
// B_RollFunc()
// --------------------------------------------------------------------------
void B_RollFunc()
{
    B_MillFunc();
    gamestate.score_roll_wait = SCORE_ROLL_WAIT;
}

// --------------------------------------------------------------------------
// Pinball Bonus Text
// --------------------------------------------------------------------------

char B_GAlienDead2[] = "^FC57    GUARDIAN ALIEN\r"
                       "      DESTROYED!\r\r"
                       "^FCA6 FIND AND DESTROY ALL\r"
                       "PROJECTION GENERATORS!";

char B_GAlienDead[] = "^FC57    GUARDIAN ALIEN\r"
                      "      DESTROYED!\r\r"
                      "^FCA6   FIND THE EXIT TO\r"
                      "COMPLETE THIS MISSION";

char B_ScoreRolled[] = "^FC57\rROLLED SCORE DISPLAY!\r"
                       "^FCA6   FULL AMMO BONUS!\r"
                       "  FULL HEALTH BONUS!\r"
                       "1,000,000 POINT BONUS!";

char B_OneMillion[] = "^FC57\r     GREAT SCORE!\r"
                      "^FCA6   FULL AMMO BONUS!\r"
                      "  FULL HEALTH BONUS!\r"
                      "1,000,000 POINT BONUS!";

char B_ExtraMan[] = "^FC57\r\r     GREAT SCORE!\r"
                    "^FCA6  EXTRA LIFE BONUS!\r";

char B_EnemyDestroyed[] = "^FC57\r\r ALL ENEMY DESTROYED!\r"
                          "^FCA6  50,000 POINT BONUS!\r";

char B_TotalPoints[] = "^FC57\r\r ALL POINTS COLLECTED!\r"
                       "^FCA6  50,000 POINT BONUS!\r";

char B_InformantsAlive[] = "^FC57\r\r ALL INFORMANTS ALIVE!\r"
                           "^FCA6  50,000 POINT BONUS!\r";

// --------------------------------------------------------------------------
// Pinball Bonus Table
// --------------------------------------------------------------------------
PinballBonusInfo PinballBonus[] = {

//                                        Special
//  BonusText           Points   Recur? Function
// -----------------------------------------------------
    { B_GAlienDead, 0, false, B_GAliFunc },
    { B_ScoreRolled, 1000000l, true, B_RollFunc },
    { B_OneMillion, 1000000l, false, B_MillFunc },
    { B_ExtraMan, 0, true, B_EManFunc },
    { B_EnemyDestroyed, 50000l, false, nullptr },
    { B_TotalPoints, 50000l, false, nullptr },
    { B_InformantsAlive, 50000l, false, nullptr },

};

// --------------------------------------------------------------------------
// DisplayPinballBonus()
// --------------------------------------------------------------------------
void DisplayPinballBonus()
{
    char loop;

// Check queue for bonuses
//
    for (loop = 0; loop < static_cast<char>(sizeof(gamestuff.level[0].bonus_queue) * 8); loop++) {
        if ((BONUS_QUEUE & (1 << loop)) && (LastMsgPri < MP_PINBALL_BONUS)) {
            // Start this bonus!
            //
            ::sd_play_player_sound(ROLL_SCORESND, bstone::AC_ITEM);

            DisplayInfoMsg(PinballBonus[static_cast<int>(loop)].BonusText, MP_PINBALL_BONUS, 7 * 60, MT_BONUS);

            // Add to "shown" ... Remove from "queue"
            //
            if (!PinballBonus[static_cast<int>(loop)].Recurring) {
                BONUS_SHOWN |= (1 << loop);
            }
            BONUS_QUEUE &= ~(1 << loop);

            // Give points and execute special function.
            //
            GivePoints(PinballBonus[static_cast<int>(loop)].Points, false);
            if (PinballBonus[static_cast<int>(loop)].func) {
                PinballBonus[static_cast<int>(loop)].func();
            }
        }
    }
}

// --------------------------------------------------------------------------
// CheckPinballBonus()
// --------------------------------------------------------------------------
void CheckPinballBonus(
    int32_t points)
{
    int32_t score_before = gamestate.score,
           score_after = gamestate.score + points;

// Check SCORE ROLLED bonus
//
    if (score_before <= MAX_DISPLAY_SCORE && score_after > MAX_DISPLAY_SCORE) {
        ActivatePinballBonus(B_SCORE_ROLLED);
    }

// Check ONE MILLION bonus
//
    if (score_before < 500000l && score_after >= 500000l) {
        ActivatePinballBonus(B_ONE_MILLION);
    }

// Check EXTRA MAN bonus
//
    if (score_after >= gamestate.nextextra) {
        gamestate.nextextra += EXTRAPOINTS;
        if (gamestate.lives < MAX_EXTRA_LIVES) {
            gamestate.lives++;
            ActivatePinballBonus(B_EXTRA_MAN);
        }
    }

// Check TOTAL ENEMY bonus
//
    if (gamestuff.level[gamestate.mapon].stats.total_enemy == gamestuff.level[gamestate.mapon].stats.accum_enemy) {
        ActivatePinballBonus(B_ENEMY_DESTROYED);
    }

// Check TOTAL POINTS bonus
//
    if (gamestuff.level[gamestate.mapon].stats.total_points == gamestuff.level[gamestate.mapon].stats.accum_points) {
        ActivatePinballBonus(B_TOTAL_POINTS);
    }

// Check INFORMANTS ALIVE bonus
//
    if ((gamestuff.level[gamestate.mapon].stats.total_inf == gamestuff.level[gamestate.mapon].stats.accum_inf) && // All informants alive?
        (gamestuff.level[gamestate.mapon].stats.total_inf) && // Any informants in level?
        ((BONUS_SHOWN & (B_TOTAL_POINTS | B_ENEMY_DESTROYED)) == (B_TOTAL_POINTS | B_ENEMY_DESTROYED))) // Got ENEMY and POINTS bonuses?
    {
        ActivatePinballBonus(B_INFORMANTS_ALIVE);
    }

// Display bonuses?
//
    if (BONUS_QUEUE) {
        DisplayPinballBonus();
    }
}

// ===========================================================================
//
//
// COMPUTER TERMINAL ROUTINES
//
//
// ===========================================================================

#ifdef ACTIVATE_TERMINAL

#define TERM_BUFFERED_DISPLAY
#define TERM_VIEW_WIDTH 246
#define TERM_VIEW_HEIGHT 95
// #define TERM_BACK_COLOR 2 // Defined in 3d)menu.h
#define TERM_BACK_XOFS 8
#define TERM_BACK_YOFS 22
#define TERM_BACK_WIDTH 304
#define TERM_BACK_HEIGHT 124

#define TERM_BCOLOR 3 // Dark Grey
#define TERM_TCOLOR 88 // Green MONO text color 87=LOW intensity
#define TERM_TSHAD_COLOR 0 // "Shadow" color

#define TERM_SCREEN_XOFS (TERM_BACK_XOFS + 19)
#define TERM_SCREEN_YOFS (TERM_BACK_YOFS + 14)

static uint16_t tcursor_x = TERM_SCREEN_XOFS,
              tcursor_y = TERM_SCREEN_YOFS;


char TERM_sound_on = 1;


char* Commands[TC_LAST];

memptr TermMessages = nullptr;
memptr TermCommands = nullptr;

#define FreeTerminalCommands() MM_FreePtr(&TermCommands)
#define FreeTerminalMessages() MM_FreePtr(&TermMessages)
#define LoadTerminalText() IO_LoadFile(term_msg_name, &TermMessages)
// #define LoadTerminalText() IO_LoadFile("TERM_MSG.TXT",&TermMessages)

// ---------------------------------------------------------------------------
//
// LoadTerminalCommands()
//
// Caches in the commands in TERM_COMMANDS grsegs and sparces the file for
// commands - Assigning Commands[] to the beginning of each command and
// null terminating the command.  Each command is seperated with ^XX.  Leading
// returns&linefeeds are skipped.
//
// NOTE: This expects that TC_LAST in the enum list of commands is concurrent
//       with the grseg TERM_COMMANDS.
//
// ---------------------------------------------------------------------------
void LoadTerminalCommands()
{
    char* Message;
    uint8_t pos;

//      IO_LoadFile("TERM_CMD.TXT",&TermCommands);
    IO_LoadFile(term_com_name, &TermCommands);
    Message = TermCommands;

    for (pos = 0; pos < TC_LAST; pos++) {
        // Bump past any leading returns/linefeeds

        while (*Message == '\n' || *Message == '\r') {
            Message++;
        }

        // Assign ptrs

        Commands[pos] = Message;

        if (!(Message = _fstrstr(Message, int_xx))) {
            ACT1_ERROR(INVALID_CACHE_MSG_NUM);
        }

        *Message = 0;   // Null Terminate String
        Message += 3; // Bump to start of next Message
    }
}




boolean term_cursor_vis = true;
boolean shadow_text = true;

#if 1

PresenterInfo Terminal_PI;

// ---------------------------------------------------------------------------
// TerminalPrint()
// ---------------------------------------------------------------------------
void TerminalPrint(
    char* msg,
    boolean FastPrint)
{
    Terminal_PI.print_delay = !FastPrint;
    Terminal_PI.script[0] = msg;
    TP_Presenter(&Terminal_PI);
}

#else

// ---------------------------------------------------------------------------
// TerminalPrint()
//
// NOTE : Terminal Control Chars
//
//          @ - Square Box (IE. Cursor)
// ---------------------------------------------------------------------------

void TerminalPrint(
    char* msg,
    boolean FastPrint)
{
#define TERM_PRINT_DELAY 1
    fontstruct* font;
    char buf[2] = { 0, 0 };
    char old_color, old_color2;
    char fontheight;

    font = (fontstruct*)grsegs[STARTFONT + fontnumber];
    fontheight = font->height;

    while (msg && *msg) {
        buf[0] = *msg++;

        if (buf[0] == '^') {
            //
            //  Handle Control Codes
            //

            switch (*((uint16_t*)msg)++) {
            // FONT COLOR
            //
            case TP_CNVT_CODE('F', 'C'):
                fontcolor = TP_VALUE(msg, 2);
                msg += 2;
                break;

            // BELL
            //
            case TP_CNVT_CODE('B', 'E'):
                SD_PlaySound(TERM_BEEPSND);
                SD_WaitSoundDone();
                break;

            // HIDE CURSOR
            //
            case TP_CNVT_CODE('H', 'I'):
                px = tcursor_x;
                py = tcursor_y;
                old_color = fontcolor;
                fontcolor = TERM_BCOLOR;
                VW_DrawPropString("@");
                fontcolor = old_color;
                break;


            // PAUSE
            //
            case TP_CNVT_CODE('P', 'A'):
                VW_WaitVBL(30);
                break;


            // END OF MSG
            //

            case TP_CNVT_CODE('X', 'X'):
                msg = nullptr;
                break;

            }
        } else {
            //
            // Process Text Char (Like print it!)
            //

            bufferofs = displayofs;

            if (term_cursor_vis) {
                px = tcursor_x;
                py = tcursor_y;

                old_color = fontcolor; // Store Cursor Color
                fontcolor = TERM_BCOLOR;

                VW_DrawPropString("@");

                fontcolor = old_color;
            }

            if (buf[0] != '\n') {
                // Blast "Shadow" on screen

                if (shadow_text) {
                    px = tcursor_x + 1;
                    py = tcursor_y + 1;
                    old_color2 = fontcolor; // STORE Old Colr
                    fontcolor = TERM_TSHAD_COLOR;
                    VW_DrawPropString(buf);
                    fontcolor = old_color2; // RESTORE Old Colr
                }

                // Blast normal Text color to screen

                px = tcursor_x;
                py = tcursor_y;
                VW_DrawPropString(buf);

                if (sound_on) {
                    if (buf[0] != ' ') {
                        SD_PlaySound(TERM_TYPESND);
                    }
                }

                tcursor_x = px;

                if (term_cursor_vis) {
                    VW_DrawPropString("@");
                }
            } else {
                if (tcursor_y > 90 + TERM_SCREEN_XOFS) {
                    VL_ScreenToScreen(displayofs + ((TERM_SCREEN_YOFS + fontheight) * SCREENWIDTH) + (TERM_SCREEN_XOFS / 4),
                                      displayofs + TERM_SCREEN_YOFS * SCREENWIDTH + TERM_SCREEN_XOFS / 4,
                                      (248 / 4), 93);
                } else {
                    tcursor_y += fontheight;
                }

                tcursor_x = TERM_SCREEN_XOFS;
            }

            if (!FastPrint) {
                VL_WaitVBL(TERM_PRINT_DELAY);
            }

            VW_UpdateScreen();
        }
    }
}

#endif




// ---------------------------------------------------------------------------
// CacheTerminalPrint()
//
// This prints a message in the TERM_MESSAGES grsegs which MUST
// already be loaded into memory.
// ---------------------------------------------------------------------------
void CacheTerminalPrint(
    int16_t MsgNum,
    boolean FastPrint)
{
    char* Message;

    Message = TermMessages;

// Search for end of MsgNum-1 (Start of our message)
//
#pragma warn -pia
    while (MsgNum--) {
        if (!(Message = _fstrstr(Message, int_xx))) {
            AGENT_ERROR(BAD_TERMINAL_MSG_NUM);
        }
        Message += 3;           // Bump to start of next Message
    }
#pragma warn +pia

// Move past LFs and CRs that follow "^XX"
//
//      while ((*Message=='\n') || (*Message=='\r'))
//              Message++;

    Message += 2; // Move past LF and CR that follows "^XX"

    TerminalPrint(Message, FastPrint);
}



char TERM_MSG[] = "^ST1^CEEnter commands and press ENTER.\r^CEPress ESC to exit terminal.^XX";

// ---------------------------------------------------------------------------
// ActivateTerminal()
// ---------------------------------------------------------------------------
void ActivateTerminal(
    boolean skiplink)
{
#define MAX_INPUT 30
    char buffer[MAX_INPUT];
    bool temp_caps = allcaps, ExitMoFo;
    uint16_t oldwidth;
    US_CursorStruct TermCursor = { '@', 0, 0x58, 2 }; // Holds Font#, etc.
    int16_t msgnum;


// Setup for text presenter
//
    memset(&Terminal_PI, 0, sizeof(Terminal_PI));
    Terminal_PI.flags = TPF_USE_CURRENT | TPF_SHOW_CURSOR | TPF_SCROLL_REGION;
    Terminal_PI.xl = 21;
    Terminal_PI.yl = 32;
    Terminal_PI.xh = 277;
    Terminal_PI.yh = 134;
    Terminal_PI.ltcolor = 255;
    Terminal_PI.bgcolor = TERM_BCOLOR;
    Terminal_PI.dkcolor = 255;
    Terminal_PI.shcolor = TERM_TSHAD_COLOR;
    Terminal_PI.fontnumber = 2;
    Terminal_PI.cur_x = -1;
    Terminal_PI.print_delay = 1;


#ifndef TERM_BUFFERED_DISPLAY
    bufferofs = displayofs;
#endif

    ClearMemory();

    oldwidth = viewwidth / 16;
    if (oldwidth != FULL_VIEW_WIDTH) {
        NewViewSize(FULL_VIEW_WIDTH);
    }


    DrawPlayScreen(false);

    StopMusic();

    fontnumber = 1;
    CA_CacheGrChunk(STARTFONT + FONT2);                                                 // Medium font

    BMAmsg(TERM_MSG);

    CacheDrawPic(TERM_BACK_XOFS, TERM_BACK_YOFS, TERMINAL_SCREENPIC);

    LoadTerminalText();
    LoadTerminalCommands();

#ifdef TERM_BUFFERED_DISPLAY
    VW_UpdateScreen();
#endif


    fontnumber = 2;
    allcaps = true;
    fontcolor = TERM_TCOLOR;
    tcursor_x = TERM_SCREEN_XOFS;
    tcursor_y = TERM_SCREEN_YOFS;

    //
    // Set up custom cursor
    //

    use_custom_cursor = true;
    US_CustomCursor = TermCursor;

    //
    // Start term stuff..
    //

    VW_FadeIn();

    ExitMoFo = false;

    TerminalPrint("^ST1^XX", false);
    if (!skiplink) {
        CacheTerminalPrint(TM_LINK, false);

        if (Keyboard[sc_h] & Keyboard[sc_o] & Keyboard[sc_t]) {
            CacheTerminalPrint(TM_CHEATER, false);
        } else {
            VW_WaitVBL(1 * 60 + (US_RndT() % 60 * 2));

            if (gamestate.TimeCount & 0x1000) {
                CacheTerminalPrint(TM_LINK_BAD, false);
                IN_Ack();
                ExitMoFo = true;
            } else {
                CacheTerminalPrint(TM_LINK_OK, false);
            }
        }
    }


    IN_ClearKeysDown();

    while (!ExitMoFo) {
        backcolor = TERM_BCOLOR;
        CacheTerminalPrint(TM_READY, false);

        if (US_LineInput(px + 1, py, buffer, nil, true, MAX_INPUT, 240 + TERM_SCREEN_XOFS - px)) {
            CacheTerminalPrint(TM_RETURN, false);
            if (*buffer) {
                switch (msgnum = US_CheckParm(buffer, Commands)) {
                case TC_HINT:
//              case TC_GOLDSTERN:
                case TC_JAM:
                case TC_HELP:
                case TC_APOGEE:
                case TC_THANKS:
                case TC_GOOBERS:
//          case TC_BSTONE:
                case TC_JERRY:
                case TC_MIKE:
                case TC_JIM:
                    CacheTerminalPrint(msgnum, false);
                    break;


                case TC_EXIT:
                case TC_QUIT:
                case TC_OFF:
                case TC_BYE:
                    ExitMoFo = true;
                    break;

                case TC_STAR:
                    CacheTerminalPrint(TM_STAR, false);
                    break;

                case TC_JOSHUA:
                    CacheTerminalPrint(TM_JOSHUA, false);
//              PowerBall = 1;
                    break;

                case TC_BLUEPRINT:
                    FloorCheat(255);
                    break;

                case TC_SOUND:
                    TERM_sound_on ^= 1;
                    CacheTerminalPrint(TM_SOUND_OFF + TERM_sound_on, false);
                    break;

                case TC_ARRIVAL_GOLDSTERN: {
                    if (GoldsternInfo.GoldSpawned) {
                        CacheTerminalPrint(TM_GOLDSTERN_ARRIVED, false);
                    } else if (GoldsternInfo.flags == GS_COORDFOUND) {
                        CacheTerminalPrint(TM_GOLDSTERN_WILL_AR, false);
                        sprintf(buffer, " %d^XX", GoldsternInfo.WaitTime / 60);
                        TerminalPrint(buffer, false);
                        CacheTerminalPrint(TM_SECONDS, false);
                    } else {
                        if (GoldsternInfo.WaitTime) {
                            CacheTerminalPrint(TM_GOLDSTERN_NO_PICK, false);
                            sprintf(buffer, " %d^XX", GoldsternInfo.WaitTime / 60);
                            TerminalPrint(buffer, false);
                            CacheTerminalPrint(TM_SECONDS, false);
                        } else {
                            CacheTerminalPrint(TM_GOLDSTERN_NO_INFO, false);
                        }
                    }
                }
                break;

                case TC_DEACTIVATE_SECURITY: {
                    objtype* obj;

                    CacheTerminalPrint(TM_RESET_SECURITY, false);
                    for (obj = player; obj; obj = obj->next) {
                        if (obj->obclass == security_lightobj) {
                            obj->temp1 = 0;
                            obj->flags &= ~FL_ALERTED;
                        }
                    }
                }
                break;

                case TC_SATALITE_STATUS: {
                    CacheTerminalPrint(TM_VITALS1, false);
                    TerminalPrint(buffer, false);

                    CacheTerminalPrint(TM_VITALS2, false);
                    sprintf(buffer, " %d\r\n^XX", gamestate.VitalsRemain);
                    TerminalPrint(buffer, false);
                }
                break;

                case TC_PROFILE:
                    CacheTerminalPrint(TM_PROFILE_WHO, false);
                    if (US_LineInput(px + 1, py, buffer, nil, true, MAX_INPUT, 246 + TERM_SCREEN_XOFS - px)) {
                        CacheTerminalPrint(TM_RETURN, false);
                        if (*buffer) {
                            switch (US_CheckParm(buffer, Commands)) {
                            case TC_GOLDSTERN:
                                CacheTerminalPrint(TM_PROFILE_GOLDSTERN, false);
                                break;

                            case TC_BSTONE:
                                CacheTerminalPrint(TM_PROFILE_BLAKE, false);
                                break;

                            case TC_SSTONE:
                                CacheTerminalPrint(TM_PROFILE_SARA, false);
                                break;

                            default:
                                CacheTerminalPrint(TM_PROFILE_UNKNOWN, false);
                                break;
                            }
                        }
                    }
                    break;

                default:
                    CacheTerminalPrint(TM_UNRECOGNIZED_COMMAND, false);
                    break;
                }
            }
        } else {
            // User pressed escape....

            ExitMoFo = true;
        }

#ifdef TERM_BUFFERED_DISPLAY
        VW_UpdateScreen();
#endif
    }

    //
    // Free everything cached in...Exit terminal
    //

    FreeTerminalCommands();
    FreeTerminalMessages();

    UNCACHEGRCHUNK(STARTFONT + 1);
    NewViewSize(oldwidth);


    StartMusic(false);
    PM_CheckMainMem();

    DrawPlayScreen(false);

    IN_ClearKeysDown();
    allcaps = temp_caps;
    use_custom_cursor = false;

}


// ---------------------------------------------------------------------------
// FloorCheat()
// ---------------------------------------------------------------------------
void FloorCheat(
    uint16_t RadarFlags)
{
#define FC_EMBED_COLOR(ColorCodes) { _fstrncpy(&pbuffer[pos], ColorCodes, 5); pos += 5; }
#define FC_NORM_COLOR() FC_EMBED_COLOR("^FC57")

    uint16_t x, y, pos;
    objtype* actor;
    char* pbuffer;
    memptr buffer;

    MM_GetPtr(&buffer, 512);

    pbuffer = buffer;

    CacheTerminalPrint(TM_BLUEPRINTS, false);

    shadow_text = term_cursor_vis = false;
    Terminal_PI.flags &= ~TPF_SHOW_CURSOR;

    //
    // Cache in the "Radar Font"
    //

    CA_CacheGrChunk(STARTFONT + 5);
//      fontnumber = 5;
    Terminal_PI.fontnumber = 5;

    //
    // Display the radar/floor-plans

//      TerminalPrint("\r\n^XX",true);
    for (y = 0; y < 64; y++) {
        pos = 0;
        for (x = 0; x < 64; x++) {
            //
            // Get wall/actor && Check for force placement of player on radar..
            //

            if (DebugOk && x == player->tilex && y == player->tiley) {
                actor = player;
            } else {
                actor = actorat[x][y];
            }

            //
            //  Check for walls

            if (!TravelTable[x][y]) { // Map only shows where you've seen!
                pbuffer[pos++] = '!';
            } else if (((uint16_t)actor && (uint16_t)actor < 108) || // 108 == LAST WALL TILE

#if IN_DEVELOPMENT

                       (*(mapsegs[0] + farmapylookup[y] + x) >= HIDDENAREATILE && (!DebugOk)) ||

#endif

                       (((uint16_t)actor & 0x80) && actor < objlist && (!DebugOk))) // Treat doors as walls in NoDebug
            {
                // Mark Wall piece
                //
                pbuffer[pos++] = '!';
            } else {
                // Not a wall Piece
                //
                if (((RadarFlags & RS_PERSONNEL_TRACKER) && actor >= objlist) && (!(actor->flags & FL_DEADGUY))) {
                    switch (actor->obclass) {
                    case playerobj:
                        if (RadarFlags & RS_PERSONNEL_TRACKER) {
                            //
                            // Mark map piece as the "player"
                            //
                            FC_EMBED_COLOR("^FC0F"); //  WHITE
                            pbuffer[pos++] = '!';
                            FC_NORM_COLOR();
                        } else {
                            pbuffer[pos++] = ' ';
                        }
                        break;

                    case security_lightobj:
                        if (RadarFlags & RS_SECURITY_STATUS) {
                            //
                            // Mark map piece as "Alerted Security Lamp"
                            //
                            if (actor->temp1) {
                                FC_EMBED_COLOR("^FC1C"); // Red
                            } else {
                                FC_EMBED_COLOR("^FC5C"); // Green
                            }

                            pbuffer[pos++] = '!';
                            FC_NORM_COLOR();
                            break;
                        } else {
                            pbuffer[pos++] = ' ';
                        }
                        break;

                    case lcan_wait_alienobj:
                    case scan_wait_alienobj:
                    case hang_terrotobj:
                    case gurney_waitobj:
                        pbuffer[pos++] = ' ';
                        break;

                    case goldsternobj:
                        if (RadarFlags & RS_GOLDSTERN_TRACKER) {
                            //
                            // Mark map piece as "goldstern"
                            //
                            FC_EMBED_COLOR("^FC38"); //  Yellow ...or.. err, like gold!
                            pbuffer[pos++] = '!';
                            FC_NORM_COLOR();
                            break;
                        } else {
                            pbuffer[pos++] = ' ';
                        }
                        break;

                    default:
                        if (RadarFlags & RS_PERSONNEL_TRACKER) {
                            //
                            // Mark map piece as a "general object"
                            //
                            FC_EMBED_COLOR("^FC18"); // Red
                            pbuffer[pos++] = '!';
                            FC_NORM_COLOR();
                        } else {
                            pbuffer[pos++] = ' ';
                        }
                        break;
                    }
                } else {
                    pbuffer[pos++] = ' ';
                }

            }

        }

//              pbuffer[pos++] = '\n';
        _fstrcpy(pbuffer + pos, "\r\n^XX");
        pbuffer[pos + 5] = 0;

        TerminalPrint(pbuffer, true);
    }


    TerminalPrint("\r\n\r\n\r\n\r\n^XX", true);
    MM_FreePtr(&buffer);

    UNCACHEGRCHUNK(STARTFONT + 5);
    Terminal_PI.fontnumber = 2;
    TerminalPrint("\r\n^XX", true);
    Terminal_PI.flags |= TPF_SHOW_CURSOR;

}

#endif


/*
=============================================================================

 PLAYER CONTROL

=============================================================================
*/




void SpawnPlayer(
    int16_t tilex,
    int16_t tiley,
    int16_t dir)
{
    if (gamestuff.level[gamestate.mapon].ptilex &&
        gamestuff.level[gamestate.mapon].ptiley)
    {
        tilex = gamestuff.level[gamestate.mapon].ptilex;
        tiley = gamestuff.level[gamestate.mapon].ptiley;
        dir = 1 + (gamestuff.level[gamestate.mapon].pangle / 90);
    }

    player->obclass = playerobj;
    player->active = ac_yes;
    player->tilex = static_cast<uint8_t>(tilex);
    player->tiley = static_cast<uint8_t>(tiley);

    player->areanumber = GetAreaNumber(player->tilex, player->tiley);

    player->x = ((int32_t)tilex << TILESHIFT) + TILEGLOBAL / 2;
    player->y = ((int32_t)tiley << TILESHIFT) + TILEGLOBAL / 2;
    player->state = &s_player;
    player->angle = (1 - dir) * 90;
    if (player->angle < 0) {
        player->angle += ANGLES;
    }
    player->flags = FL_NEVERMARK;
    Thrust(0, 0); // set some variables

    InitAreas();

    InitWeaponBounce();
}


// ===========================================================================

// ------------------------------------------------------------------------
// GunAttack()
// ------------------------------------------------------------------------
void GunAttack(
    objtype* ob)
{
    objtype* check, * closest, * oldclosest;
    int16_t damage;
    int16_t dx, dy, dist;
    int32_t viewdist;
    boolean skip = false;

    if (gamestate.weapon != wp_autocharge) {
        MakeAlertNoise(ob);
    }

    switch (gamestate.weapon) {
    case wp_autocharge:
        ::sd_play_player_sound(ATKAUTOCHARGESND, bstone::AC_WEAPON);

        skip = true;
        break;

    case wp_pistol:
        ::sd_play_player_sound(ATKCHARGEDSND, bstone::AC_WEAPON);

        skip = true;
        break;

    case wp_burst_rifle:
        ::sd_play_player_sound(ATKBURSTRIFLESND, bstone::AC_WEAPON);
        break;

    case wp_ion_cannon:
        ::sd_play_player_sound(ATKIONCANNONSND, bstone::AC_WEAPON);
        break;

    }

    //
    // find potential targets
    //

    viewdist = 0x7fffffffl;
    closest = nullptr;

    while (true) {
        oldclosest = closest;

        for (check = ob->next; check; check = check->next) {
            if ((check->flags & FL_SHOOTABLE) &&
                (check->flags & FL_VISABLE) &&
                (abs(check->viewx - centerx) < shootdelta))
            {
                if (check->transx < viewdist) {
                    if ((skip && (check->obclass == hang_terrotobj))
                        || (check->flags2 & FL2_NOTGUNSHOOTABLE))
                    {
                        continue;
                    }

                    viewdist = check->transx;
                    closest = check;
                }
            }
        }

        if (closest == oldclosest) {
            return; // no more targets, all missed

        }
        //
        // trace a line from player to enemey
        //
        if (CheckLine(closest, player)) {
            break;
        }
    }

    //
    // hit something
    //

    dx = static_cast<int16_t>(abs(closest->tilex - player->tilex));
    dy = static_cast<int16_t>(abs(closest->tiley - player->tiley));
    dist = dx > dy ? dx : dy;

    if (dist < 2) {
        damage = US_RndT() / 2; // 4
    } else if (dist < 4) {
        damage = US_RndT() / 4; // 6
    } else {
        if ((US_RndT() / 12) < dist) { // missed
            return;
        }
        damage = US_RndT() / 4; // 6
    }

    DamageActor(closest, damage, player);
}


// ===========================================================================




// ===========================================================================


/*
===============
=
= T_Attack
=
===============
*/


void T_Attack(
    objtype* ob)
{
    atkinf_t* cur;
    int16_t x;

    if (noShots) {
        ob->state = &s_player;
        gamestate.attackframe = gamestate.weaponframe = 0;
        return;
    }

    if (gamestate.weapon == wp_autocharge) {
        UpdateAmmoMsg();
    }

    if (buttonstate[bt_use] && !buttonheld[bt_use]) {
        buttonstate[bt_use] = false;
    }

    if (buttonstate[bt_attack] && !buttonheld[bt_attack]) {
        buttonstate[bt_attack] = false;
    }

    ControlMovement(ob);

    player->tilex = static_cast<uint8_t>(player->x >> TILESHIFT); // scale to tile values
    player->tiley = static_cast<uint8_t>(player->y >> TILESHIFT);

//
// change frame and fire
//
    gamestate.attackcount -= tics;
    if (gamestate.attackcount <= 0) {
        cur = &attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe];
        switch (cur->attack) {
        case -1:
            ob->state = &s_player;

            if (!gamestate.ammo) {
                if (gamestate.weapon != wp_autocharge) {
                    gamestate.weapon = wp_autocharge;
                    DrawWeapon();
                    DisplayInfoMsg(EnergyPackDepleted, MP_NO_MORE_AMMO, DISPLAY_MSG_STD_TIME << 1, MT_OUT_OF_AMMO);
                }
            } else {
                if (!(gamestate.useable_weapons & (1 << gamestate.weapon))) {
                    gamestate.weapon = wp_autocharge;
                    DrawWeapon();
                    DisplayInfoMsg(NotEnoughEnergyForWeapon, MP_NO_MORE_AMMO, DISPLAY_MSG_STD_TIME << 1, MT_OUT_OF_AMMO);
                }
            }
            gamestate.attackframe = gamestate.weaponframe = 0;
            return;

        case -2:
            ob->state = &s_player;
            if (!gamestate.plasma_detonators) {
                // Check to see what weapons are possible.
                //
                const int16_t n_x = (::is_ps() ? wp_bfg_cannon : wp_grenade);

                for (x = n_x; x >= wp_autocharge; x--) {
                    if (gamestate.useable_weapons & (1 << x)) {
                        gamestate.weapon = static_cast<char>(x);
                        break;
                    }
                }

                DrawWeapon();
// DisplayInfoMsg(pd_switching,MP_NO_MORE_AMMO,DISPLAY_MSG_STD_TIME<<1,MT_OUT_OF_AMMO);
            }
            gamestate.attackframe = gamestate.weaponframe = 0;
            return;

        case 4:
            if (!gamestate.ammo) {
                break;
            }
            if (buttonstate[bt_attack]) {
                gamestate.attackframe -= 2;
            }

        case 0:
            if (gamestate.weapon == wp_grenade) {
                if (!objfreelist) {
                    DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
                    gamestate.attackframe++;
                }
            }
            break;

        case 1:
            if (!gamestate.ammo) { // can only happen with chain gun
                gamestate.attackframe++;
                break;
            }
            GunAttack(ob);
            if (!godmode) {
                gamestate.ammo--;
            }
            if (!::is_ps()) {
                DrawWeapon();
            } else {
                DrawAmmo(false);
            }
            break;

        case 2:
            if (gamestate.weapon_wait) {
                break;
            }
            GunAttack(ob);
            gamestate.weapon_wait = AUTOCHARGE_WAIT;
            if (!::is_ps()) {
                DrawWeapon();
            } else {
                DrawAmmo(false);
            }
            break;

        case 3:
            if (gamestate.ammo && buttonstate[bt_attack]) {
                gamestate.attackframe -= 2;
            }
            break;

        case 6:
            if (gamestate.ammo && buttonstate[bt_attack]) {
                if (objfreelist) {
                    gamestate.attackframe -= 2;
                }
            } else if (gamestate.ammo == 0) {
                DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
            }
            break;

        case 5:
            if (!objfreelist) {
                DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
                gamestate.attackframe++;
            } else {
                if (LastMsgType == MT_MALFUNCTION) {
                    MsgTicsRemain = 1; // Clear Malfuction Msg before anim

                }
                if (!godmode) {
                    if (gamestate.ammo >= GRENADE_ENERGY_USE) {
                        gamestate.ammo -= GRENADE_ENERGY_USE;
                        if (!::is_ps()) {
                            DrawWeapon();
                        } else {
                            DrawAmmo(false);
                        }
                    } else {
                        gamestate.attackframe++;
                    }
                }

                ::sd_play_player_sound(ATKGRENADESND, bstone::AC_WEAPON);

                SpawnProjectile(ob, grenadeobj);
                MakeAlertNoise(ob);
            }
            break;

        case 7:
            TryDropPlasmaDetonator();
            DrawAmmo(false);
            break;

        case 8:
            if (gamestate.plasma_detonators && buttonstate[bt_attack]) {
                gamestate.attackframe -= 2;
            }
            break;

        case 9:
            if (!objfreelist) {
                DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
                gamestate.attackframe++;
            } else {
                if (LastMsgType == MT_MALFUNCTION) {
                    MsgTicsRemain = 1; // Clear Malfuction Msg before anim

                }
                if (!godmode) {
                    if (gamestate.ammo >= BFG_ENERGY_USE) {
                        gamestate.ammo -= BFG_ENERGY_USE;
                        DrawAmmo(false);
                    } else {
                        gamestate.attackframe++;
                    }
                }

                ::sd_play_player_sound(ATKIONCANNONSND, bstone::AC_WEAPON);

                SpawnProjectile(ob, bfg_shotobj);
                MakeAlertNoise(ob);
            }
            break;

        case 10:
            if (gamestate.ammo && buttonstate[bt_attack]) {
                if (objfreelist) {
                    gamestate.attackframe -= 2;
                }
            } else if (gamestate.ammo == 0) {
                DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
            }
            break;
        }

        gamestate.attackcount += cur->tics;
        gamestate.attackframe++;
        gamestate.weaponframe =
            attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].frame;
    }
}


// ===========================================================================

/*
===============
=
= T_Player
=
===============
*/

void T_Player(
    objtype* ob)
{
    CheckWeaponChange();

    if (gamestate.weapon == wp_autocharge) {
        UpdateAmmoMsg();
    }

    if (::is_ps()) {
        if (tryDetonatorDelay > tics) {
            tryDetonatorDelay -= tics;
        } else {
            tryDetonatorDelay = 0;
        }
    }

    if (buttonstate[bt_use]) {
        Cmd_Use();
        ::sd_play_player_sound(HITWALLSND, bstone::AC_HIT_WALL);
    }

    if (buttonstate[bt_attack] && !buttonheld[bt_attack]) {
        Cmd_Fire();
    }

    ControlMovement(ob);
    HandleWeaponBounce();


//      plux = player->x >> UNSIGNEDSHIFT; // scale to fit in unsigned
//      pluy = player->y >> UNSIGNEDSHIFT;
    player->tilex = static_cast<uint8_t>(player->x >> TILESHIFT); // scale to tile values
    player->tiley = static_cast<uint8_t>(player->y >> TILESHIFT);
}

// -------------------------------------------------------------------------
// RunBlakeRun()
// -------------------------------------------------------------------------
void RunBlakeRun()
{
#define BLAKE_SPEED (MOVESCALE * 50)

    int32_t xmove, ymove;
    objtype* blake;
    int16_t startx, starty, dx, dy;

// Spawn Blake and set pointer.
//
    SpawnPatrol(en_blake, player->tilex, player->tiley, player->dir >> 1);
    blake = new_actor;

// Blake object starts one tile behind player object.
//
    switch (blake->dir) {
    case north:
        blake->tiley += 2;
        break;

    case south:
        blake->tiley -= 2;
        break;

    case east:
        blake->tilex -= 2;
        break;

    case west:
        blake->tilex += 2;
        break;
    }

// Align Blake on the middle of the tile.
//
    blake->x = ((int32_t)blake->tilex << TILESHIFT) + TILEGLOBAL / 2;
    blake->y = ((int32_t)blake->tiley << TILESHIFT) + TILEGLOBAL / 2;
    startx = blake->tilex = blake->x >> TILESHIFT;
    starty = blake->tiley = blake->y >> TILESHIFT;

// Run, Blake, Run!
//
    do {
        // Calc movement in X and Y directions.
        //
        xmove = FixedByFrac(BLAKE_SPEED, costable[player->angle]);
        ymove = -FixedByFrac(BLAKE_SPEED, sintable[player->angle]);

        // Move, animate, and redraw.
        //
        if (ClipMove(blake, xmove, ymove)) {
            break;
        }
        DoActor(blake);
        ThreeDRefresh();

        // Calc new tile X/Y.
        //
        blake->tilex = blake->x >> TILESHIFT;
        blake->tiley = blake->y >> TILESHIFT;

        // Evaluate distance from start.
        //
        dx = blake->tilex - startx;
        dx = ABS(dx);
        dy = blake->tiley - starty;
        dy = ABS(dy);

        // BBi
        ::in_handle_events();
    } while ((dx < 6) && (dy < 6));
}


// -------------------------------------------------------------------------
// SW_HandleActor() - Handle all actors connected to a smart switch.
// -------------------------------------------------------------------------
void SW_HandleActor(
    objtype* obj)
{
    if (!obj->active) {
        obj->active = ac_yes;
    }

    switch (obj->obclass) {
    case rentacopobj:
    case gen_scientistobj:
    case swatobj:
    case goldsternobj:
    case proguardobj:
        if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK))) {
            FirstSighting(obj);
        }
        break;

    case morphing_spider_mutantobj:
    case morphing_reptilian_warriorobj:
    case morphing_mutanthuman2obj:
    case crate1obj:
    case crate2obj:
    case crate3obj:
    case podeggobj:
        KillActor(obj);
        break;

    case gurney_waitobj:
    case scan_wait_alienobj:
    case lcan_wait_alienobj:
        break;

// case electrosphereobj:
//    break;

    case floatingbombobj:
    case volatiletransportobj:
        if (obj->flags & FL_STATIONARY) {
            KillActor(obj);
        } else if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK))) {
            FirstSighting(obj);
        }
        break;

    case spider_mutantobj:
    case breather_beastobj:
    case cyborg_warriorobj:
    case reptilian_warriorobj:
    case acid_dragonobj:
    case mech_guardianobj:
    case liquidobj:
    case genetic_guardobj:
    case mutant_human1obj:
    case mutant_human2obj:
    case lcan_alienobj:
    case scan_alienobj:
    case gurneyobj:
    case podobj:
    case final_boss1obj:
    case final_boss2obj:
    case final_boss3obj:
    case final_boss4obj:
        if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK))) {
            FirstSighting(obj);
        }
        break;

// case electroobj:
// case liquidobj:
//    break;

    case post_barrierobj:
    case arc_barrierobj:
        break;

    default:
        break;
    }
}


// -------------------------------------------------------------------------
// SW_HandleStatic() - Handle all statics connected to a smart switch.
// -------------------------------------------------------------------------
void SW_HandleStatic(
    statobj_t* stat,
    uint16_t tilex,
    uint16_t tiley)
{
    switch (stat->itemnumber) {
    case bo_clip:
    case bo_clip2:
        if (::is_ps()) {
            SpawnCusExplosion((((fixed)tilex) << TILESHIFT) + 0x7FFF,
                              (((fixed)tiley) << TILESHIFT) + 0x7FFF,
                              SPR_CLIP_EXP1, 7, 30 + (US_RndT() & 0x27), explosionobj);
        }
        stat->shapenum = -1;
        stat->itemnumber = bo_nothing;
        break;
    }
}


// -------------------------------------------------------------------------
// OperateSmartSwitch() - Operates a Smart Switch
//
// PARAMETERS:
//      tilex - Tile X coord that the Smart switch points to.
//      tiley - Tile Y coord that the Smart switch points to.
//      force - Force switch operation.  Will not check the players current
//              and last tilex & tiley coords.  This is usefull for other
//              actors toggling barrier switches.
//
// RETURNS: Boolean: TRUE  - Remove switch from map
//      FALSE - Keep switch in map
//
// -------------------------------------------------------------------------
boolean OperateSmartSwitch(
    uint16_t tilex,
    uint16_t tiley,
    char Operation,
    boolean Force)
{
    enum what_is_it {
        wit_NOTHING,
        wit_DOOR,
        wit_WALL,
        wit_STATIC,
        wit_ACTOR
    }; // what_is_it

    what_is_it WhatItIs;
    objtype* obj;
    statobj_t* stat = nullptr;
    uint8_t tile, DoorNum = 0;
    uint16_t iconnum;

    //
    // Get some information about what
    // this switch is pointing to.
    //

    tile = tilemap[tilex][tiley];
    obj = actorat[tilex][tiley];
    iconnum = *(mapsegs[1] + farmapylookup[tiley] + tilex);
    WhatItIs = wit_NOTHING;

    //
    // Deterimine if the switch points to an
    // actor, door, wall, static or is Special.
    //

    if (obj < objlist) {
        if (obj == (objtype*)1 && tile == 0) {
            // We have a SOLID static!

            WhatItIs = wit_STATIC;
        } else {
            if (tile) {
                //
                // We have a wall of some type (maybe a door).
                //

                if (tile & 0x80) {
                    // We have a door

                    WhatItIs = wit_DOOR;
                    DoorNum = tile & 0x7f;
                } else {
                    // We have a wall

                    WhatItIs = wit_WALL;
                }
            } else {
                if (stat = FindStatic(tilex, tiley)) {
                    WhatItIs = wit_STATIC;
                }
            }
        }
    } else {
        if (obj < &objlist[MAXACTORS]) {
            // We have an actor.

            WhatItIs = wit_ACTOR;
        } else {
            WhatItIs = wit_NOTHING;
        }
    }

    //
    // Ok... Now do that voodoo that you do so well...
    //

    switch (WhatItIs) {
    //
    // Handle Doors
    //
    case wit_DOOR:
        if (doorobjlist[DoorNum].action == dr_jammed) {
            return false;
        }

        doorobjlist[DoorNum].lock = kt_none;
        OpenDoor(DoorNum);
        return false;


    //
    // Handle Actors
    //
    case wit_ACTOR:
        if (!(obj->flags & FL_DEADGUY)) {
            SW_HandleActor(obj);
        }
        return true;


    //
    // Handle Walls
    //
    case wit_WALL: {
        if (Force || player_oldtilex != player->tilex || player_oldtiley != player->tiley) {
            switch (tile) {
            case OFF_SWITCH:
                if (Operation == ST_TURN_OFF) {
                    return false;
                }

                ActivateWallSwitch(iconnum, tilex, tiley);
                break;

            case ON_SWITCH:
                if (Operation == ST_TURN_ON) {
                    return false;
                }
                ActivateWallSwitch(iconnum, tilex, tiley);
                break;
            }
        }
    }
        return false;


    //
    // Handle Statics
    //
    case wit_STATIC:
        stat = ::FindStatic(tilex, tiley);

        if (!stat) {
            return false;
        }

        SW_HandleStatic(stat, tilex, tiley);
        return true;


    //
    // Handle NON connected smart switches...
    //
    case wit_NOTHING:
        // Actor (or something) that was to be triggered has
        // moved... SSSOOOoo, Remove the switch.
        return true;
    }

    return false;
}

// ==========================================================================
//
//                         WEAPON BOUNCE CODE
//
// ==========================================================================

#define wb_MaxPoint ((int32_t)10 << TILESHIFT)
#define wb_MidPoint ((int32_t)6 << TILESHIFT)
#define wb_MinPoint ((int32_t)2 << TILESHIFT)
#define wb_MaxGoalDist (wb_MaxPoint - wb_MidPoint)

#define wb_MaxOffset (wb_MaxPoint + ((int32_t)2 << TILESHIFT))
#define wb_MinOffset (wb_MinPoint - ((int32_t)2 << TILESHIFT))

extern fixed bounceOffset;

fixed bounceVel, bounceDest;
int16_t bounceOk;

// --------------------------------------------------------------------------
// InitWeaponBounce()
// --------------------------------------------------------------------------
void InitWeaponBounce()
{
    bounceOffset = wb_MidPoint;
    bounceDest = wb_MaxPoint;
    bounceVel = bounceOk = 0;
}

// --------------------------------------------------------------------------
// HandleWeaponBounce()
// --------------------------------------------------------------------------
void HandleWeaponBounce()
{
    int16_t bounceSpeed;

    bounceSpeed = 90 - ((20 - viewsize) * 6);

    if (bounceOk) {
        if (bounceOffset < bounceDest) {
            bounceVel += (sintable[bounceSpeed] + 1) >> 1;
            bounceOffset += bounceVel;
            if (bounceOffset > bounceDest) {
                bounceDest = wb_MinPoint;
                bounceVel >>= 2;
            }
        } else if (bounceOffset > bounceDest) {
            bounceVel -= sintable[bounceSpeed] >> 2;
            bounceOffset += bounceVel;

            if (bounceOffset < bounceDest) {
                bounceDest = wb_MaxPoint;
                bounceVel >>= 2;
            }
        }
    } else {
        if (bounceOffset > wb_MidPoint) {
            bounceOffset -= ((int32_t)2 << TILESHIFT);
            if (bounceOffset < wb_MidPoint) {
                bounceOffset = wb_MidPoint;
            }
        } else if (bounceOffset < wb_MidPoint) {
            bounceOffset += ((int32_t)2 << TILESHIFT);
            if (bounceOffset > wb_MidPoint) {
                bounceOffset = wb_MidPoint;
            }
        }

        bounceDest = wb_MaxPoint;
        bounceVel = 0;
    }

    if (bounceOffset > wb_MaxOffset) {
        bounceOffset = wb_MaxOffset;
    } else if (bounceOffset < wb_MinOffset) {
        bounceOffset = wb_MinOffset;
    }
}
