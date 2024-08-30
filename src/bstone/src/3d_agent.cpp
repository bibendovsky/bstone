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
#include <deque>
#include <limits>

#include "3d_def.h"
#include "audio.h"
#include "jm_tp.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"

#include "bstone_archiver.h"
#include "bstone_ascii.h"
#include "bstone_math.h"
#include "bstone_memory_stream.h"
#include "bstone_saved_game.h"
#include "bstone_string_helper.h"

// From 3d_debug.cpp
const objtype* find_countable_enemy();
const statobj_t* find_bonus_item();
std::string get_enemy_actor_name(const objtype& bs_actor);
std::string get_bonus_item_name(const statobj_t& bs_actor, bool& is_plural);
std::string get_bonus_item_name(const statobj_t& bs_actor);

namespace
{


void try_to_grab_bonus_items();


struct PinballBonusInfo
{
	const char* BonusText; // REBA text pointer
	std::int32_t Points; // Score for this bonus
	bool Recurring; // Appear multiple times in a single level?
	void (*func)(); // Code to execute when you get this bonus.
}; // PinballBonusInfo

using PinballBonusInfos = std::array<PinballBonusInfo, 7>;

struct atkinf_t
{
	std::int8_t tics;
	std::int8_t attack;
	std::int8_t frame; // attack is 1 for gun, 2 for knife
}; // atkinf_t


//
// M_BASE1 - represents 100 percent in 1st base
// M_BASE2 - represents 100 percent in 2nd base
// F_BASE2 - fractional portion of 2nd base
// SCALE - arbitrary scaling value (bigger number means more accurate)
//
// RETURNS: F_BASE1 - represents fractional portion of 1st base
//
// ex: RATIO(320,16,8,7)    returns  160
//
// Make sure values used won't overflow a WORD! In general, if largest number
// to be used (320 in ex: above) * (1<<SCALE) is greater than 65535, use
// LRATIO or a lower SCALE. Using a SCALE of 8 in the example above would
// overflow a WORD in some circumstances!
//
// LRATIO is to be used for larger SCALEs, thus, giving you massive accuracy!
//
int LRATIO(
	const std::int32_t M_BASE1,
	const std::int32_t M_BASE2,
	const std::int32_t F_BASE2,
	const std::int32_t SCALE)
{
	return (M_BASE1 * ((F_BASE2 << SCALE) / M_BASE2)) >> SCALE;
}


} // namespace

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

std::int16_t CalcAngle(
	objtype* from_obj,
	objtype* to_obj);

void PushWall(
	std::int16_t checkx,
	std::int16_t checky,
	std::int16_t dir);

void OperateDoor(
	std::int16_t door);

void TryDropPlasmaDetonator();

void ClearMemory();

void InitAreas();

void FirstSighting(
	objtype* ob);

void OpenDoor(
	std::int16_t door);

void DrawTopInfo(
	sp_type type);

void DoActor(
	objtype* ob);

void RunBlakeRun();

std::int16_t DrawShape(
	std::int16_t x,
	std::int16_t y,
	std::int16_t shapenum,
	pisType shapetype);

void DrawAmmoMsg();

void AnimatePage();

void DrawInfoArea();

std::uint8_t ValidAreaTile(
	const std::uint16_t* ptr);

int InputFloor();

bool Interrogate(
	objtype* ob);

void PrintStatPercent(
	std::int16_t nx,
	std::int16_t ny,
	std::int8_t percentage);

void DrawAmmoGuage();

void GetBonus(
	statobj_t* check);

void ClearPaletteShifts();


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/


#define MAXMOUSETURN (10)

constexpr auto MOVESCALE = bstone::math::fixed_to_floating(150);
constexpr auto BACKMOVESCALE = bstone::math::fixed_to_floating(100);

#define ANGLESCALE (20)
#define MAX_DA (100)

#define MAX_TERM_COMMAND_LEN (31)

// Max Score displayable

#define MAX_DISPLAY_SCORE (9999999L)
#define SCORE_ROLL_WAIT (60 * 10) // Tics

// ECG scroll rate (delay).
#define HEALTH_SCROLL_RATE (7)
#define HEALTH_PULSE (70)

// Text "InfoArea" defines
#define INFOAREA_X (3)
#define INFOAREA_Y (static_cast<std::uint16_t>(200) - STATUSLINES + 3)
#define INFOAREA_W (109)
#define INFOAREA_H (37)

#define INFOAREA_BCOLOR (0x01)
#define INFOAREA_CCOLOR (0x1A)
#define INFOAREA_TCOLOR (0xA6)
#define INFOAREA_TSHAD_COLOR (0x04) // Text Shadow Color

#define GRENADE_ENERGY_USE (4)
#define BFG_ENERGY_USE (GRENADE_ENERGY_USE << 1)

#define NUM_AMMO_SEGS (21)

#define AMMO_SMALL_FONT_NUM_WIDTH (5)


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

extern bool noShots;
extern std::int16_t bounceOk;

std::int16_t tryDetonatorDelay = 0;

//
// player state info
//
double thrustspeed;

// unsigned plux,pluy; // player coordinates scaled to unsigned

int anglefrac;

objtype* LastAttacker;

bool PlayerInvisable = false;

char LocationText[MAX_LOCATION_DESC_LEN];

std::uint16_t player_oldtilex;
std::uint16_t player_oldtiley;

// BBi
extern bstone::MemoryStream g_playtemp;
bool is_select_floor = false;


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

void writeTokenStr(
	std::string& string);

void ShowOverheadChunk();

void LoadOverheadChunk(
	int tpNum);

void SaveOverheadChunk(
	int tpNum);

void DisplayTeleportName(
	std::int8_t tpNum,
	bool locked);

void ForceUpdateStatusBar();
void UpdateRadarGuage();

void DrawLedStrip(
	std::int16_t x,
	std::int16_t y,
	std::int16_t frac,
	std::int16_t max);

void DisplayPinballBonus();

void CheckPinballBonus(
	std::int32_t points);

void T_Player(
	objtype* ob);

void T_Attack(
	objtype* ob);

statetype s_player = {0, 0, 0, &T_Player, nullptr, nullptr};
statetype s_attack = {0, 0, 0, &T_Attack, nullptr, nullptr};

atkinf_t attackinfo[7][14] = {
	{{6, 0, 1}, {6, 2, 2}, {6, 0, 3}, {6, -1, 4}}, // Auto charge
	{{6, 0, 1}, {6, 1, 2}, {6, 0, 3}, {6, -1, 4}}, // Pistol
	{{6, 0, 1}, {6, 1, 2}, {5, 3, 3}, {5, -1, 4}}, // Pulse
	{{6, 0, 1}, {6, 1, 2}, {3, 4, 3}, {3, -1, 4}}, // ION
	{{6, 0, 1}, {6, 5, 2}, {6, 6, 3}, {6, -1, 4}},
	{{6, 0, 1}, {6, 9, 2}, {6, 10, 3}, {6, -1, 4}},
	{{5, 7, 0}, {5, 8, 0}, {2, -2, 0}, {0, 0, 0}},
};


#define GD0 (0x55)
#define YD0 (0x35)
#define RD0 (0x15)

#define GD1 (0x53)
#define YD1 (0x33)
#define RD1 (0x13)

std::int8_t DimAmmo[2][22] = {
	{GD0, GD0, GD0, GD0, GD0, GD0, GD0, YD0, YD0, YD0, YD0, YD0, YD0, YD0, RD0, RD0, RD0, RD0, RD0, RD0, RD0, RD0},
	{GD1, GD1, GD1, GD1, GD1, GD1, GD1, YD1, YD1, YD1, YD1, YD1, YD1, YD1, RD1, RD1, RD1, RD1, RD1, RD1, RD1, RD1},
};

#define GL0 (0x58)
#define YL0 (0x38)
#define RL0 (0x18)

#define GL1 (0x56)
#define YL1 (0x36)
#define RL1 (0x16)

std::int8_t LitAmmo[2][22] = {
	{GL0, GL0, GL0, GL0, GL0, GL0, GL0, YL0, YL0, YL0, YL0, YL0, YL0, YL0, RL0, RL0, RL0, RL0, RL0, RL0, RL0, RL0},
	{GL1, GL1, GL1, GL1, GL1, GL1, GL1, YL1, YL1, YL1, YL1, YL1, YL1, YL1, RL1, RL1, RL1, RL1, RL1, RL1, RL1, RL1},
};

#define IA_MAX_LINE (30)

struct InfoArea_Struct
{
	std::int16_t x, y;
	std::int16_t text_color;
	std::int16_t backgr_color;
	std::int16_t left_margin;
	std::int8_t delay;
	std::int8_t numanims;
	std::int8_t framecount;
}; // InfoArea_Struct

std::uint16_t LastMsgPri = 0;
std::int16_t MsgTicsRemain = 0;
classtype LastInfoAttacker = nothing;

std::int16_t LastInfoAttacker_Cloaked = 0;

infomsg_type LastMsgType = MT_NOTHING;
InfoArea_Struct InfoAreaSetup;

std::int8_t DrawRadarGuage_COUNT = 3;
std::int8_t DrawAmmoNum_COUNT = 3;
std::int8_t DrawAmmoPic_COUNT = 3;
std::int8_t DrawScoreNum_COUNT = 3;
std::int8_t DrawWeaponPic_COUNT = 3;
std::int8_t DrawKeyPics_COUNT = 3;
std::int8_t DrawHealthNum_COUNT = 3;

std::int8_t DrawInfoArea_COUNT = 3;
std::int8_t InitInfoArea_COUNT = 3;
std::int8_t ClearInfoArea_COUNT = 3;

void DrawWeapon();

void GiveWeapon(
	int weapon);

void GiveAmmo(
	std::int16_t ammo);

void DrawGAmmoNum();
void DrawPDAmmoMsg();
void ComputeAvailWeapons();

void SW_HandleActor(
	objtype* obj);

void SW_HandleStatic(
	statobj_t* stat,
	std::uint16_t tilex,
	std::uint16_t tiley);

// ===========================================================================

std::uint8_t ShowRatio(
	std::int16_t bx,
	std::int16_t by,
	std::int16_t px,
	std::int16_t py,
	std::int32_t total,
	std::int32_t perc,
	ss_type type);

void SpawnPlayer(
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir);

void Thrust(
	std::int16_t angle,
	double speed);

bool TryMove(
	objtype* ob);

void T_Player(
	objtype* ob);

bool ClipMove(
	objtype* ob,
	const double xmove,
	const double ymove);

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
	const auto& assets_info = get_assets_info();

	const int n = assets_info.is_ps() ? wp_bfg_cannon : wp_grenade;

	for (int i = wp_autocharge; i <= n; i++)
	{
		if (buttonstate[bt_ready_autocharge + i - wp_autocharge])
		{
			if (gamestate.useable_weapons & (1 << i))
			{
				gamestate.weapon = static_cast<std::int8_t>(i);
				gamestate.chosenweapon = static_cast<std::int8_t>(i);

				DISPLAY_TIMED_MSG(WeaponAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
				DrawWeapon();
				return;
			}
			else
			{
				DISPLAY_TIMED_MSG(WeaponNotAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
			}
		}
	}
}

int clamp_angle(
	int angle)
{
	while (angle >= ANGLES)
	{
		angle -= ANGLES;
	}

	while (angle < 0)
	{
		angle += ANGLES;
	}

	return angle;
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
	const auto is_original_strafe =
		gamestate.turn_around == 0 &&
		in_is_binding_pressed(e_bi_strafe)
	;

	const auto is_modern_strafe =
		gamestate.turn_around == 0 &&
		!in_is_binding_pressed(e_bi_strafe) &&
		strafe_value != 0
	;

	thrustspeed = 0.0;

	//
	// side to side move
	//
#ifdef __vita__
	if (control2x != 0)
	{
		if (control2x > 0)
		{
			int angle2 = ob->angle - ANGLES / 4;
			if (angle2 < 0)
			{
				angle2 += ANGLES;
			}
			Thrust(static_cast<std::int16_t>(angle2), control2x * MOVESCALE); // move to left
		}
		else if (control2x < 0)
		{
			int angle2 = ob->angle + ANGLES / 4;
			if (angle2 >= ANGLES)
			{
				angle2 -= ANGLES;
			}
			Thrust(static_cast<std::int16_t>(angle2), -control2x * MOVESCALE); // move to right
		}
	}
#else
	if (is_original_strafe || (is_modern_strafe && controly == 0))
	{
		const auto sign = (strafe_value > 0 ? 1 : -1);
		const auto angle = clamp_angle(ob->angle + (sign * (ANGLES / 4)));

		Thrust(static_cast<std::int16_t>(angle), -abs(strafe_value) * MOVESCALE);
	}
#endif

	if (!is_original_strafe)
	{
		if (gamestate.turn_around != 0)
		{
			controlx = 100 * tics;

			if (gamestate.turn_around < 0)
			{
				controlx = -controlx;
			}
		}

		//
		// not strafing
		//
		anglefrac += controlx;
		int angleunits = anglefrac / ANGLESCALE;
		anglefrac -= angleunits * ANGLESCALE;
		ob->angle -= static_cast<std::int16_t>(angleunits);

		ob->angle = static_cast<std::int16_t>(clamp_angle(ob->angle));

		if (gamestate.turn_around != 0)
		{
			bool done = false;

			if (gamestate.turn_around > 0)
			{
				gamestate.turn_around = static_cast<std::int16_t>(gamestate.turn_around - angleunits);

				if (gamestate.turn_around <= 0)
				{
					done = true;
				}
			}
			else
			{
				gamestate.turn_around = static_cast<std::int16_t>(gamestate.turn_around - angleunits);

				if (gamestate.turn_around >= 0)
				{
					done = true;
				}
			}

			if (done)
			{
				gamestate.turn_around = 0;
				ob->angle = gamestate.turn_angle;
			}
		}
	}

	if (is_modern_strafe && controly != 0)
	{
		const auto x = -strafe_value * MOVESCALE;
		const auto y = -controly * (controly < 0 ? MOVESCALE : BACKMOVESCALE);

		const auto angle_delta = 90 - static_cast<int>(bstone::math::rad_to_deg(std::atan2(y, x)));
		const auto angle = clamp_angle(ob->angle + angle_delta);
		const auto value = std::abs(y);

		Thrust(static_cast<std::int16_t>(angle), value);
	}
	else
	{
		//
		// forward/backwards move
		//
		if (controly < 0)
		{
			Thrust(ob->angle, -controly * MOVESCALE); // move forwards
		}
		else if (controly > 0)
		{
			const auto angle = clamp_angle(ob->angle + ANGLES / 2);

			Thrust(static_cast<std::int16_t>(angle), controly * BACKMOVESCALE); // move backwards
		}
		else if (bounceOk != 0)
		{
			bounceOk -= 1;
		}
	}

	if (controly != 0)
	{
		bounceOk = 8;
	}
	else if (bounceOk != 0)
	{
		bounceOk -= 1;
	}

	ob->dir = static_cast<dirtype>(((ob->angle + 22) % 360) / 45);
}


/*
=============================================================================

 STATUS WINDOW STUFF

=============================================================================
*/

static void STATUSDRAWPIC(
	int x,
	int y,
	int picnum)
{
	LatchDrawPic(x, y + (200 - STATUSLINES), picnum);
}

void StatusAllDrawPic(
	std::uint16_t x,
	std::uint16_t y,
	std::uint16_t picnum)
{
	STATUSDRAWPIC(x, y, picnum);
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
	std::int16_t x,
	std::int16_t y,
	std::int16_t width,
	std::int32_t number)
{
	auto wide = 0;
	auto number_string = std::to_string(number);
	auto length = static_cast<int>(number_string.length());

	while (length < width && wide < width)
	{
		STATUSDRAWPIC(x, y, N_BLANKPIC);
		++x;
		++wide;
		++length;
	}

	auto c = 0;

	while (wide < width)
	{
		STATUSDRAWPIC(x, y, number_string[c] - '0' + N_0PIC);
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

namespace
{


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

	if (ecg_scroll_tics >= ecg_next_scroll_tics)
	{
		ecg_scroll_tics = 0;
		ecg_next_scroll_tics = HEALTH_SCROLL_RATE;

		bool carry = false;

		for (int i = 5; i >= 0; --i)
		{
			if (carry)
			{
				carry = false;
				ecg_legend[i] = ecg_legend[i + 1];
				ecg_segments[i] = ecg_segments[i + 1] - 4;
			}
			else if (ecg_segments[i] != 0)
			{
				ecg_segments[i] += 1;

				bool use_carry = false;

				if (ecg_legend[i] == 1 && ecg_segments[i] == 5)
				{
					use_carry = true;
				}
				else if (ecg_legend[i] == 2 && ecg_segments[i] == 13)
				{
					use_carry = true;
				}
				if (ecg_legend[i] == 3 &&
					(ecg_segments[i] == 22 || ecg_segments[i] == 27))
				{
					use_carry = true;
				}

				if (use_carry)
				{
					carry = true;
				}
				else
				{
					bool skip = false;

					if (ecg_legend[i] == 1 && ecg_segments[i] > 8)
					{
						skip = true;
					}
					else if (ecg_legend[i] == 2 && ecg_segments[i] > 17)
					{
						skip = true;
					}
					if (ecg_legend[i] == 3 && ecg_segments[i] > 27)
					{
						skip = true;
					}

					if (skip)
					{
						ecg_legend[i] = 0;
						ecg_segments[i] = 0;
					}
				}
			}
		}

		if (gamestate.health > 0 && ecg_legend[5] == 0)
		{
			if (gamestate.health < 33)
			{
				ecg_legend[5] = 3;
				ecg_segments[5] = 18;
			}
			else if (gamestate.health >= 66)
			{
				if (ecg_legend[4] == 0 || ecg_legend[4] != 1)
				{
					ecg_legend[5] = 1;
					ecg_segments[5] = 1;
				}
			}
			else
			{
				ecg_legend[5] = 2;
				ecg_segments[5] = 9;
			}
		}
	}
	else
	{
		ecg_scroll_tics += tics;
	}

	for (int i = 0; i < 6; ++i)
	{
		CA_CacheGrChunk(
			static_cast<std::int16_t>(ECG_HEARTBEAT_00 + ecg_segments[i]));

		VWB_DrawPic(
			120 + (i * 8),
			200 - STATUSLINES + 8,
			ECG_HEARTBEAT_00 + ecg_segments[i]);
	}


	//
	// Heart sign
	//

	bool reset_heart_tics = false;

	if (gamestate.health <= 0)
	{
		reset_heart_tics = true;
		heart_picture_index = ECG_GRID_PIECE;
	}
	else if (gamestate.health < 40)
	{
		reset_heart_tics = true;
		heart_picture_index = ECG_HEART_BAD;
	}
	else if (heart_sign_tics >= heart_sign_next_tics)
	{
		reset_heart_tics = true;

		if (heart_picture_index == ECG_GRID_PIECE)
		{
			heart_picture_index = ECG_HEART_GOOD;

			if (gp_use_heart_beat_sfx())
			{
				sd_play_player_item_sound(H_BEATSND);
			}
		}
		else
		{
			heart_picture_index = ECG_GRID_PIECE;
		}
	}

	if (reset_heart_tics)
	{
		heart_sign_tics = 0;
		heart_sign_next_tics = HEALTH_PULSE / 2;
	}
	else
	{
		heart_sign_tics += 1;
	}

	CA_CacheGrChunk(
		static_cast<std::int16_t>(heart_picture_index));

	VWB_DrawPic(
		120,
		200 - STATUSLINES + 32,
		heart_picture_index);
}

// --------------------------------------------------------------------------
// DrawHealth()
//
// PURPOSE : Marks the Health_NUM to be refreshed during the next
//           StatusBarRefresh.
// --------------------------------------------------------------------------
void DrawHealth()
{
	DrawHealthNum_COUNT = 3;
}

void DrawHealthNum()
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		CA_CacheGrChunk(ECG_GRID_PIECE);

		for (int i = 0; i < 3; ++i)
		{
			VWB_DrawPic(
				144 + (i * 8),
				200 - STATUSLINES + 32,
				ECG_GRID_PIECE);
		}

		auto&& health_string = std::to_string(gamestate.health);
		health_string.reserve(4);

		if (gamestate.health < 100)
		{
			health_string.insert(0, 1, ' ');

			if (gamestate.health < 10)
			{
				health_string.insert(0, 1, ' ');
			}
		}

		health_string += '%';

		fontnumber = 2;
		fontcolor = 0x9D;

		PrintX = 149;
		PrintY = 200 - STATUSLINES + 34;

		px = static_cast<std::int16_t>(PrintX);
		py = static_cast<std::int16_t>(PrintY);

		VW_DrawPropString(health_string.c_str());
	}
	else
	{
		const auto base_x = 16;
		const auto is_zero = (gamestate.health <= 0);

		auto check = 100;

		for (int i = 0; i < 3; ++i)
		{
			auto pic_num = 0;

			if (gamestate.health < check &&
				(!is_zero || (is_zero && i < 2)))
			{
				pic_num = NG_BLANKPIC;
			}
			else
			{
				const auto digit = (gamestate.health / check) % 10;
				pic_num = NG_0PIC + digit;
			}

			LatchDrawPic(base_x + i, 162, pic_num);

			check /= 10;
		}
	}

	DrawHealthNum_COUNT -= 1;
}

void TakeDamage(
	std::int16_t points,
	objtype* attacker)
{
	LastAttacker = attacker;

	if (!gp_hide_attacker_info())
	{
		if (attacker)
		{
			if ((LastMsgType == MT_ATTACK) && (LastInfoAttacker == attacker->obclass))
			{
				MsgTicsRemain = DISPLAY_MSG_STD_TIME;
			}
			else
			{
				if (DISPLAY_TIMED_MSG(ActorInfoMsg[attacker->obclass - rentacopobj], MP_TAKE_DAMAGE, MT_ATTACK))
				{
					LastInfoAttacker = attacker->obclass;

					const auto& assets_info = get_assets_info();

					if (assets_info.is_ps())
					{
						LastInfoAttacker_Cloaked = attacker->flags2 & FL2_CLOAKED;
					}
				}
			}
		}
	}

	if (godmode)
	{
		return;
	}

	if (gamestate.difficulty == gd_baby)
	{
		points >>= 2;
	}

	gamestate.health -= points;

	if (gamestate.health <= 0)
	{
		gamestate.health = 0;
		playstate = ex_died;
		killerobj = attacker;
		if (killerobj)
		{
			killerobj->flags |= FL_FREEZE;
		}
	}

	StartDamageFlash(points);
	DrawHealth();
}

void HealSelf(
	std::int16_t points)
{
	gamestate.health += points;
	if (gamestate.health > 100)
	{
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
// PURPOSE : Marks the Score to be refreshed during the next
//      StatusBarRefresh.
// --------------------------------------------------------------------------
void DrawScore()
{
	DrawScoreNum_COUNT = 3;
}

extern std::uint8_t music_num;


// --------------------------------------------------------------------------
// DrawScoreNum()
//
// NOTE : Could do some sort of "scrolling" animation on LED screen with
//      chars and a simple table.....
// --------------------------------------------------------------------------
void DrawScoreNum()
{
	const int Y = 3;
	const int X = 32;

	if (gamestate.tic_score > MAX_DISPLAY_SCORE)
	{
		if (gamestate.score_roll_wait)
		{
			LatchDrawPic(X + 0, (200 - STATUSLINES) + Y, N_BLANKPIC);
			LatchDrawPic(X + 1, (200 - STATUSLINES) + Y, N_DASHPIC);
			LatchDrawPic(X + 2, (200 - STATUSLINES) + Y, N_RPIC);
			LatchDrawPic(X + 3, (200 - STATUSLINES) + Y, N_OPIC);
			LatchDrawPic(X + 4, (200 - STATUSLINES) + Y, N_LPIC);
			LatchDrawPic(X + 5, (200 - STATUSLINES) + Y, N_LPIC);
			LatchDrawPic(X + 6, (200 - STATUSLINES) + Y, N_DASHPIC);
		}
		else
		{
			LatchNumber(X, Y, 7, gamestate.tic_score % (MAX_DISPLAY_SCORE + 1));
		}
	}
	else
	{
		LatchNumber(X, Y, 7, gamestate.tic_score);
	}
}

void UpdateScore()
{
	std::int32_t score_diff, temp_tics;

	score_diff = gamestate.score - gamestate.tic_score;

	if (score_diff)
	{
		if (score_diff > 1500)
		{
			temp_tics = score_diff >> 2;
		}
		else
		{
			temp_tics = tics << 3;
		}

		if (score_diff > temp_tics)
		{
			gamestate.tic_score += temp_tics;
		}
		else
		{
			gamestate.tic_score = gamestate.score;
		}

		DrawScore();
	}


	if (gamestate.score_roll_wait)
	{
		if ((gamestate.score_roll_wait -= tics) <= 0)
		{
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
	std::int32_t points,
	bool add_to_stats)
{
	// Add score to statistics.
	//
	if (add_to_stats)
	{
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

void DrawKeyPics()
{
	DrawKeyPics_COUNT -= 1;

	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		static const int indices[NUMKEYS] = {
			0, 1, 3, 2, 4,
		}; // indices

		static const std::uint8_t off_colors[NUMKEYS] = {
			0x11, 0x31, 0x91, 0x51, 0x21,
		}; // off_colors

		static const std::uint8_t on_colors[NUMKEYS] = {
			0xC9, 0xB9, 0x9C, 0x5B, 0x2B,
		}; // on_colors

		for (auto i = 0; i < NUMKEYS; ++i)
		{
			int index = indices[i];
			std::uint8_t color = 0;

			if (gamestate.numkeys[index] > 0)
			{
				color = on_colors[index];
			}
			else
			{
				color = off_colors[index];
			}

			VWB_Bar(
				257 + (i * 8),
				200 - STATUSLINES + 25,
				7,
				7,
				color);
		}
	}
	else
	{
		for (auto loop = 0; loop < 3; loop++)
		{
			if (gamestate.numkeys[loop])
			{
				LatchDrawPic(15 + 2 * loop, 179, RED_KEYPIC + loop);
			}
			else
			{
				LatchDrawPic(15 + 2 * loop, 179, NO_KEYPIC);
			}
		}
	}
}

void GiveKey(
	std::int16_t key)
{
	gamestate.numkeys[key]++;
	DrawKeys();
}

void TakeKey(
	std::int16_t key)
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
// PURPOSE : Marks the Weapon pics to be refreshed during the next
//      StatusBarRefresh.
// ---------------------------------------------------------------------------
void DrawWeapon()
{
	DrawWeaponPic_COUNT = 3;
	DrawAmmo(true);
}

void DrawWeaponPic()
{
	if (gamestate.weapon == -1)
	{
		return;
	}

	const auto& assets_info = get_assets_info();

	LatchDrawPic(
		assets_info.is_ps() ? 31 : 22,
		assets_info.is_ps() ? 176 : 152,
		WEAPON1PIC + gamestate.weapon);

	--DrawWeaponPic_COUNT;
}

void GiveWeapon(
	int weapon)
{
	GiveAmmo(6);

	if ((gamestate.weapons & (1 << weapon)) == 0)
	{
		gamestate.weapons |= 1 << weapon;

		if (gamestate.weapon < weapon)
		{
			gamestate.weapon = static_cast<std::int8_t>(weapon);
			gamestate.chosenweapon = static_cast<std::int8_t>(weapon);
			DrawWeapon();
		}

		ComputeAvailWeapons();
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
//                               during the next StatusBarRefresh.
//
// NOTE : This re-computes the number of LEDs to be lit.
// ---------------------------------------------------------------------------
void DrawAmmo(
	bool ForceRefresh)
{
	std::int16_t temp;
	std::uint16_t ammo, max_ammo;

	ComputeAvailWeapons();

	//
	// Which weapon are we needing a refresh for?
	//

	switch (gamestate.weapon)
	{
	case wp_autocharge:
		DrawAmmoPic_COUNT = 3;
		DrawAmmoNum_COUNT = 0;
		return;

	default:
		ammo = gamestate.ammo;
		max_ammo = MAX_AMMO;
		break;
	}

	if (ammo)
	{
		temp = (ammo * NUM_AMMO_SEGS) / max_ammo;
		if (!temp)
		{
			temp = 1;
		}
	}
	else
	{
		temp = 0;
	}

	gamestate.ammo_leds = static_cast<std::int8_t>(temp);

	if ((temp != gamestate.lastammo_leds) || ForceRefresh)
	{
		gamestate.lastammo_leds = static_cast<std::int8_t>(temp);
		DrawAmmoPic_COUNT = 3;
	}

	DrawAmmoNum_COUNT = 3;
}

void DrawAmmoNum()
{
	if (gamestate.weapon == -1)
	{
		return;
	}

	const auto& assets_info = get_assets_info();

	fontnumber = 2;
	fontcolor = 0x9D;

	PrintX = (assets_info.is_ps() ? 252 : 211);
	PrintY = 200 - STATUSLINES + 38;

	if (assets_info.is_ps() || (!assets_info.is_ps() && gamestate.weapon != wp_autocharge))
	{
		DrawGAmmoNum();
	}

	DrawAmmoNum_COUNT--;
}

void DrawGAmmoNum()
{
	const auto& assets_info = get_assets_info();

	if (gamestate.ammo < 100)
	{
		PrintX += AMMO_SMALL_FONT_NUM_WIDTH;
		if (gamestate.ammo < 10)
		{
			PrintX += AMMO_SMALL_FONT_NUM_WIDTH;
		}
	}

	if (assets_info.is_ps())
	{
		LatchDrawPic(31, 184, W1_CORNERPIC + gamestate.weapon);
	}

	px = static_cast<std::int16_t>(PrintX);
	py = static_cast<std::int16_t>(PrintY);

	auto ammo_string = std::to_string(gamestate.ammo);
	VW_DrawPropString(ammo_string.c_str());
	VW_DrawPropString("%");
}

void DrawAmmoPic()
{
	switch (gamestate.weapon)
	{
	case wp_autocharge:
		DrawAmmoMsg();
		break;

	default:
		DrawAmmoGuage();
		break;
	}

	DrawAmmoPic_COUNT--;
}

void DrawAmmoMsg()
{
	const auto& assets_info = get_assets_info();

	int x = (assets_info.is_ps() ? 30 : 29);

	if (gamestate.weapon_wait)
	{
		LatchDrawPic(x, (200 - STATUSLINES), WAITPIC);
	}
	else
	{
		LatchDrawPic(x, (200 - STATUSLINES), READYPIC);
	}
}

void DrawPDAmmoMsg()
{
	if (gamestate.plasma_detonators)
	{
		LatchDrawPic(30, (200 - STATUSLINES), READYPIC);
	}
	else
	{
		LatchDrawPic(30, (200 - STATUSLINES), WAITPIC);
	}
}

void UpdateAmmoMsg()
{
	if (gamestate.weapon_wait)
	{
		if ((gamestate.weapon_wait -= static_cast<std::int8_t>(tics)) <= 0)
		{
			gamestate.weapon_wait = 0;
			DrawAmmoPic_COUNT = 3;
		}
	}
}

void DrawAmmoGuage()
{
	const auto& assets_info = get_assets_info();

	DrawLedStrip(assets_info.is_ps() ? 243 : 234, 155, gamestate.ammo_leds, NUM_AMMO_SEGS);
}

void UpdateRadarGuage()
{
	std::int16_t temp;

	if (gamestate.rpower)
	{
		temp = ((std::int32_t)gamestate.rpower * NUM_AMMO_SEGS) / MAX_RADAR_ENERGY;

		if (temp > NUM_AMMO_SEGS)
		{
			temp = NUM_AMMO_SEGS;
		}

		if (!temp)
		{
			temp = 1;
		}
	}
	else
	{
		temp = 0;
	}

	gamestate.radar_leds = static_cast<std::int8_t>(temp);

	if (temp != gamestate.lastradar_leds)
	{
		gamestate.lastradar_leds = static_cast<std::int8_t>(temp);
	}

	DrawRadarGuage_COUNT = 3;
}

void DrawRadarGuage()
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		return;
	}

	std::int8_t zoom;

	DrawLedStrip(235, 155, gamestate.radar_leds, NUM_AMMO_SEGS);

	if (gamestate.rpower)
	{
		zoom = gamestate.rzoom;
	}
	else
	{
		zoom = 0;
	}

	LatchDrawPic(22, 152, ONEXZOOMPIC + zoom);
}

void DrawLedStrip(
	std::int16_t x,
	std::int16_t y,
	std::int16_t frac,
	std::int16_t max)
{
	std::int16_t ypos;
	std::uint16_t amount;
	std::int8_t leds;

	leds = static_cast<std::int8_t>(frac);

	if (leds)
	{
		amount = max - leds;
	}
	else
	{
		amount = max;
	}

	const auto& assets_info = get_assets_info();

	int width = (assets_info.is_ps() ? 5 : 11);

	// Draw dim LEDs.
	//
	for (ypos = 0; ypos < amount; ypos++)
	{
		VW_Hlin(x, x + (width - 1), y++, DimAmmo[0][amount]);
		VW_Hlin(x, x + (width - 1), y++, DimAmmo[1][amount]);
	}

	// Draw lit LEDs.
	//
	for (; ypos < NUM_AMMO_SEGS; ypos++)
	{
		VW_Hlin(x, x + (width - 1), y++, LitAmmo[0][amount]);
		VW_Hlin(x, x + (width - 1), y++, LitAmmo[1][amount]);
	}
}

void GiveAmmo(
	std::int16_t ammo)
{

#if MP_NO_MORE_AMMO > MP_BONUS
	if (LastMsgType == MT_OUT_OF_AMMO)
	{
		MsgTicsRemain = 1;
		LastMsgType = MT_CLEAR;
	}
#endif

	gamestate.ammo += ammo;
	if (gamestate.ammo > MAX_AMMO)
	{
		gamestate.ammo = MAX_AMMO;
	}

	DrawAmmo(false);

	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		if (gamestate.weapon != gamestate.chosenweapon)
		{
			if (gamestate.useable_weapons & (1 << gamestate.chosenweapon))
			{
				gamestate.weapon = gamestate.chosenweapon;
				DrawWeapon();
			}
		}
	}
	else
	{
		DrawWeapon();
	}

	sd_play_player_item_sound(GETAMMOSND);
}


// ---------------------------------------------------------------------------
// ComputeAvailWeapons()
//
// This function creates a Bit MASK for gamestate.weapons according to what
// weapon is available for usage due to ammo avail.
//
// ---------------------------------------------------------------------------
void ComputeAvailWeapons()
{
	const auto& assets_info = get_assets_info();

	//
	// Determine what ammo ammounts we have avail
	//

	if (gamestate.ammo > 0)
	{
		if (assets_info.is_ps() && gamestate.ammo >= BFG_ENERGY_USE)
		{
			gamestate.useable_weapons =
				(1 << wp_bfg_cannon) |
				(1 << wp_grenade) |
				(1 << wp_ion_cannon) |
				(1 << wp_burst_rifle) |
				(1 << wp_pistol) |
				(1 << wp_autocharge);
		}
		else if (gamestate.ammo >= GRENADE_ENERGY_USE)
		{
			gamestate.useable_weapons =
				(1 << wp_grenade) |
				(1 << wp_ion_cannon) |
				(1 << wp_burst_rifle) |
				(1 << wp_pistol) |
				(1 << wp_autocharge);
		}
		else
		{
			gamestate.useable_weapons =
				(1 << wp_ion_cannon) |
				(1 << wp_burst_rifle) |
				(1 << wp_pistol) |
				(1 << wp_autocharge);
		}
	}
	else
	{
		gamestate.useable_weapons = 1 << wp_autocharge;
	}

	//
	// mask off with the weapons being carried.
	//

	gamestate.useable_weapons &= gamestate.weapons;
}

void TakePlasmaDetonator(
	std::int16_t count)
{
	if (gamestate.plasma_detonators < count)
	{
		gamestate.plasma_detonators = 0;
	}
	else
	{
		gamestate.plasma_detonators -= count;
	}
}

void GivePlasmaDetonator(
	std::int16_t count)
{
	gamestate.plasma_detonators += count;

	if (gamestate.plasma_detonators > MAX_PLASMA_DETONATORS)
	{
		gamestate.plasma_detonators = MAX_PLASMA_DETONATORS;
	}

	ComputeAvailWeapons();
}

void GiveToken(
	std::int16_t tokens)
{
#if MP_NO_MORE_TOKENS > MP_BONUS
	if (LastMsgType == MT_NO_MO_FOOD_TOKENS)
	{
		MsgTicsRemain = 1;
		LastMsgType = MT_CLEAR;
	}
#endif

	gamestate.tokens += tokens;
	if (gamestate.tokens > MAX_TOKENS)
	{
		gamestate.tokens = MAX_TOKENS;
	}

	sd_play_player_item_sound(GOTTOKENSND);
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
bool DisplayInfoMsg(
	const std::string& Msg,
	msg_priorities Priority,
	std::int16_t DisplayTime,
	std::int16_t MsgType)
{
	if (Msg.empty())
	{
		return false;
	}

	return DisplayInfoMsg(
		Msg.c_str(),
		Priority,
		DisplayTime,
		MsgType);
}

bool DisplayInfoMsg(
	const char* Msg,
	msg_priorities Priority,
	std::int16_t DisplayTime,
	std::int16_t MsgType)
{
	if (Priority >= LastMsgPri)
	{
		const auto& assets_info = get_assets_info();

		if (Priority == MP_max_val)
		{ // "System" msgs
			LastMsgPri = MP_min_val;
		}
		else
		{
			LastMsgPri = static_cast<std::uint16_t>(Priority);
		}

		if ((MsgTicsRemain = DisplayTime) != 0)
		{
			StatusAllDrawPic(0, 40, BRI_LIGHTPIC);
		}

		gamestate.msg = Msg;

		DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;

		LastMsgType = static_cast<infomsg_type>(MsgType);

		if (assets_info.is_ps() && LastMsgType != MT_ATTACK)
		{
			LastInfoAttacker_Cloaked = 0;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void ClearInfoArea()
{
	if (ClearInfoArea_COUNT)
	{
		ClearInfoArea_COUNT--;
	}

	InfoAreaSetup.x = InfoAreaSetup.left_margin;
	InfoAreaSetup.y = INFOAREA_Y;
	InfoAreaSetup.framecount = InfoAreaSetup.numanims = 0;

	LatchDrawPic(0, 200 - STATUSLINES, INFOAREAPIC);
}

void InitInfoArea()
{
	InfoAreaSetup.left_margin = INFOAREA_X;
	InfoAreaSetup.text_color = INFOAREA_TCOLOR;
	InfoAreaSetup.backgr_color = INFOAREA_BCOLOR;
	InitInfoArea_COUNT--;

	ClearInfoArea();
}

void UpdateInfoArea()
{
	if (InfoAreaSetup.numanims)
	{
		AnimatePage();
	}

	if (InitInfoArea_COUNT)
	{
		InitInfoArea();
	}
	else if (ClearInfoArea_COUNT)
	{
		ClearInfoArea();
	}

	if (DrawInfoArea_COUNT)
	{
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

	if (playstate == ex_title || playstate == ex_victorious)
	{
		return;
	}

	//
	// Check for existing timed messages
	//

	if (LastMsgPri && MsgTicsRemain)
	{
		//
		// Tic' that 'Puppy' down - Yea!
		//

		if ((MsgTicsRemain -= tics) <= 0)
		{
			// Message has expired.
			DisplayNoMoMsgs();
		}
	}

}

auto status_message = std::string{};

void DisplayNoMoMsgs()
{
	LastMsgPri = MP_min_val;

	if (BONUS_QUEUE)
	{
		DisplayPinballBonus();

		return;
	}

	const auto need_detonator_msg = "\r\r^FC39 FIND THE DETONATOR!";
	const auto have_detonator_msg = "\r\r^FC39DESTROY SECURITY CUBE!";
	const auto destroy_goldfire_msg = "\r\r^FC39  DESTROY GOLDFIRE!";

	MsgTicsRemain = 0;
	StatusAllDrawPic(0, 40, DIM_LIGHTPIC);

	const auto default_msg_length = 78;

	status_message =
		"\r    NO MESSAGES."
		"^FCA8\r    FOOD TOKENS: ";

	status_message += std::to_string(gamestate.tokens);
	status_message.resize(default_msg_length, ' ');

	const auto& assets_info = get_assets_info();

	auto is_level_locked = true;

	if (gamestate.mapon < (assets_info.get_levels_per_episode() - 1))
	{
		is_level_locked = gamestuff.level[gamestate.mapon + 1].locked;
	}

	if (is_level_locked)
	{
		switch (gamestate.mapon)
		{
		case 19:
			if (assets_info.is_ps())
			{
				status_message += destroy_goldfire_msg;
			}
			break;

		case 20:
		case 21:
		case 22:
		case 23:
			break;

		default:
			if (assets_info.is_ps())
			{
				if (gamestate.plasma_detonators)
				{
					status_message += have_detonator_msg;
				}
				else
				{
					status_message += need_detonator_msg;
				}
			}
			break;
		}
	}

	DisplayInfoMsg(status_message.c_str(), MP_max_val, 0, MT_NOTHING);
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
	const std::int16_t IA_FONT_HEIGHT = 6;

	char* first_ch;
	char* scan_ch, temp;

	DrawInfoArea_COUNT--;

	if (!gamestate.msg)
	{
		return;
	}

	if (!*gamestate.msg)
	{
		return;
	}

	std::vector<char> buffer(
		gamestate.msg,
		gamestate.msg + std::string::traits_type::length(gamestate.msg) + 1);

	first_ch = &buffer[0];

	fontnumber = 2;
	fontcolor = static_cast<std::uint8_t>(InfoAreaSetup.text_color);

	while (first_ch && *first_ch)
	{

		if (*first_ch != TP_CONTROL_CHAR)
		{
			scan_ch = first_ch;

			while ((*scan_ch) && (*scan_ch != '\n') && (*scan_ch != TP_RETURN_CHAR) && (*scan_ch != TP_CONTROL_CHAR))
			{
				scan_ch++;
			}

			// print current line
			//

			temp = *scan_ch;
			*scan_ch = 0;

			if (*first_ch != TP_RETURN_CHAR)
			{
				std::int8_t temp_color;

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

			if ((*first_ch == ' ') || (*first_ch == TP_RETURN_CHAR))
			{
				first_ch++;
			}

			// TP_CONTROL_CHARs don't advance to next character line
			//

			if (*scan_ch != TP_CONTROL_CHAR)
			{
				InfoAreaSetup.x = InfoAreaSetup.left_margin;
				InfoAreaSetup.y += IA_FONT_HEIGHT;
			}
			else
			{
				InfoAreaSetup.x = px;
			}
		}
		else
		{
			first_ch = HandleControlCodes(first_ch);
		}
	}
}

char* HandleControlCodes(
	char* first_ch)
{
	piShapeInfo* shape;
	piAnimInfo* anim;
	std::uint16_t shapenum;

	first_ch++;

#ifndef TP_CASE_SENSITIVE
	*first_ch = toupper(*first_ch);
	*(first_ch + 1) = toupper(*(first_ch + 1));
#endif

	std::uint16_t code = *reinterpret_cast<const std::uint16_t*>(first_ch);
	first_ch += 2;

	switch (code)
	{

		// INIT ANIMATION ---------------------------------------------------
		//
	case TP_CNVT_CODE('A', 'N'):
		shapenum = TP_VALUE(first_ch, 2);
		first_ch += 2;
		piAnimList[static_cast<int>(InfoAreaSetup.numanims)] = piAnimTable[shapenum];
		anim = &piAnimList[static_cast<int>(InfoAreaSetup.numanims++)];
		shape = &piShapeTable[anim->baseshape + anim->frame]; // BUG!! (assumes "pia_shapetable")

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

		DrawShape(InfoAreaSetup.x, InfoAreaSetup.y, shape->shapenum, shape->shapetype);
		InfoAreaSetup.left_margin = InfoAreaSetup.x;
		break;

		// FONT COLOR -------------------------------------------------------
		//
	case TP_CNVT_CODE('F', 'C'):
		InfoAreaSetup.text_color = TP_VALUE(first_ch, 2);
		fontcolor = static_cast<std::uint8_t>(TP_VALUE(first_ch, 2));
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
		if (shapenum == 0xfff)
		{
			InfoAreaSetup.left_margin = InfoAreaSetup.x;
		}
		else
		{
			InfoAreaSetup.left_margin = shapenum;
		}
		break;
	}

	return first_ch;

}

std::int16_t DrawShape(
	std::int16_t x,
	std::int16_t y,
	std::int16_t shapenum,
	pisType shapetype)
{
	std::int16_t width = 0;

	switch (shapetype)
	{
	case pis_scaled:
		VW_Bar(x, y, 37, 37, static_cast<std::uint8_t>(InfoAreaSetup.backgr_color)); // JTR changed
		vid_draw_ui_sprite(shapenum, x + 19, y + 20, 37);
		width = 37;
		break;

	case pis_latchpic:
		x = (x + 7) & 0xFFF8;
		LatchDrawPic(x >> 3, y, shapenum);
		break;

	case pis_pic:
		x = (x + 7) & 0xFFF8;
		width = pictable[shapenum - STARTPICS].width;
		CA_MarkGrChunk(shapenum);
		CA_CacheMarks();
		VWB_DrawPic(x, y, shapenum);
		UNCACHEGRCHUNK(shapenum);
		break;

	default:
		break;
	}

	InfoAreaSetup.x += width;
	return x;
}

void AnimatePage()
{
	piAnimInfo* anim = piAnimList;
	piShapeInfo* shape;

	// Dec Timers
	//

	anim->delay += tics;

	if (anim->delay >= anim->maxdelay)
	{
		InfoAreaSetup.framecount = 3;
		anim->delay = 0;
	}

	// Test framecount - Do we need to draw a shape?
	//

	if (InfoAreaSetup.framecount)
	{
		// Draw shapes

		switch (anim->animtype)
		{
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
		if (!InfoAreaSetup.framecount)
		{
			// Have drawn all pages... Inc Frame count

			anim->frame++;
			if (anim->frame == anim->maxframes)
			{
				anim->frame = 0;
			}
		}
	}
}


// ===========================================================================
//
//
//                       STATUS BAR REFRESH ROUTINES
//
//
// ===========================================================================

void UpdateStatusBar()
{
	if (playstate == ex_title || playstate == ex_victorious)
	{
		return;
	}


	//
	// Call specific status bar managers
	//

	UpdateScore();
	UpdateInfoArea();

	//
	// Refresh Status Area
	//

	if (DrawAmmoPic_COUNT)
	{
		DrawAmmoPic();
	}

	DrawScoreNum();

	if (DrawWeaponPic_COUNT)
	{
		DrawWeaponPic();
	}

	if (DrawRadarGuage_COUNT)
	{
		DrawRadarGuage();
	}

	DrawAmmoNum();

	if (DrawKeyPics_COUNT)
	{
		DrawKeyPics();
	}

	if (DrawHealthNum_COUNT)
	{
		DrawHealthNum();
	}

	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		DrawHealthMonitor();
	}
}

// ---------------------------------------------------------------------------
// ForceUpdateStatusBar() - Force Draw status bar onto ALL display pages
// ---------------------------------------------------------------------------
void ForceUpdateStatusBar()
{
	DrawScore();
	DrawWeapon();
	DrawKeys();
	DrawHealth();
	UpdateRadarGuage();
	UpdateStatusBar();
}


/*
===================
=
= GetBonus
=
===================
*/

std::uint16_t static_points[] = {100, // money bag
500, // loot
250, // gold1
500, // gold2
750, // gold3
1000, // major gold!
5000 // bonus
};

using StaticHealthTable = std::vector<std::array<std::int16_t, 3>>;

StaticHealthTable static_health;

void initialize_static_health_table()
{
	static_health = {
		{100, HEALTH2SND, -1, }, // Full Heal
	{30, HEALTH1SND, -1, }, // First Aid
	{20, HEALTH1SND, SPR_STAT_45, }, // Steak
	{15, HEALTH1SND, SPR_STAT_43, }, // Chicken Leg
	{10, HEALTH1SND, SPR_SANDWICH_WRAPER, }, // Sandwich
	{8, HEALTH1SND, SPR_CANDY_WRAPER, }, // Candy Bar
	{5, HEALTH1SND, SPR_STAT_41, }, // Water bowl
	{5, HEALTH1SND, -1, }, // Water puddle
	};
}

extern std::string bonus_msg24;
extern std::string bonus_msg25;

void GetBonus(
	statobj_t* check)
{
	bool givepoints = false;
	std::int16_t shapenum = -1;

	switch (check->itemnumber)
	{
	case bo_red_key:
	case bo_yellow_key:
	case bo_blue_key:
	case bo_green_key:
	case bo_gold_key:
	{
		auto keynum = -1;
		const auto& assets_info = get_assets_info();

		if (assets_info.is_aog())
		{
			switch (check->itemnumber)
			{
				case bo_red_key: keynum = 0; break;
				case bo_yellow_key: keynum = 1; break;
				case bo_blue_key: keynum = 2; break;
				case bo_green_key: keynum = 3; break;
				case bo_gold_key: keynum = 4; break;
			}
		}
		else
		{
			switch (check->itemnumber)
			{
				case bo_red_key: keynum = 0; break;
				case bo_yellow_key: keynum = 1; break;
				case bo_blue_key: keynum = 2; break;
			}
		}

		if (keynum < 0)
		{
			auto error_message = std::string{};
			error_message.reserve(64);
			error_message += "Unknown key at (";
			error_message += std::to_string(check->tilex);
			error_message += ", ";
			error_message += std::to_string(check->tiley);
			error_message += ").";
			BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
		}

		if (gamestate.numkeys[keynum] >= MAXKEYS)
		{
			return;
		}

		GiveKey(static_cast<std::int16_t>(keynum));
		sd_play_player_item_sound(GETKEYSND);
		travel_table_[check->tilex][check->tiley] &= ~TT_KEYS;
		break;
	}

	case bo_money_bag:
		sd_play_player_item_sound(BONUS1SND);
		givepoints = true;
		break;

	case bo_loot:
		sd_play_player_item_sound(BONUS2SND);
		givepoints = true;
		break;


	case bo_gold1:
	case bo_gold2:
	case bo_gold3:
	case bo_gold:
		sd_play_player_item_sound(BONUS3SND);
		givepoints = true;
		break;


	case bo_bonus:
		sd_play_player_item_sound(BONUS4SND);
		givepoints = true;
		break;

	case bo_water_puddle:
		if (gamestate.health > 15)
		{
			return;
		}
	case bo_fullheal:
	case bo_firstaid:
	case bo_ham: // STEAK
	case bo_chicken:
	case bo_sandwich:
	case bo_candybar:
	case bo_water:
		if (gamestate.health == 100)
		{
			return;
		}

		sd_play_player_item_sound(static_health[check->itemnumber - bo_fullheal][1]);
		HealSelf(static_health[check->itemnumber - bo_fullheal][0]);
		check->flags &= ~FL_BONUS;
		shapenum = static_health[check->itemnumber - bo_fullheal][2];
		break;

	case bo_clip:
		if (gamestate.ammo == MAX_AMMO)
		{
			return;
		}
		GiveAmmo(8);
		bonus_msg7[45] = '8';
		break;

	case bo_clip2:
	{
		std::uint8_t ammo;

		if (gamestate.ammo == MAX_AMMO)
		{
			return;
		}

		ammo = 1 + (US_RndT() & 0x7);
		bonus_msg7[45] = '0' + ammo;
		GiveAmmo(ammo);
	}
	break;

	case bo_plasma_detonator:
		travel_table_[check->tilex][check->tiley] &= ~TT_KEYS;
		GivePlasmaDetonator(1);
		sd_play_player_item_sound(GETDETONATORSND);
		break;

	case bo_pistol:
		GiveWeapon(wp_pistol);
		sd_play_player_item_sound(GETPISTOLSND);
		break;

	case bo_burst_rifle:
		GiveWeapon(wp_burst_rifle);
		sd_play_player_item_sound(GETBURSTRIFLESND);
		break;

	case bo_ion_cannon:
		GiveWeapon(wp_ion_cannon);
		sd_play_player_item_sound(GETIONCANNONSND);
		break;

	case bo_grenade:
		GiveWeapon(wp_grenade);
		sd_play_player_item_sound(GETCANNONSND);
		break;

	case bo_bfg_cannon:
		GiveWeapon(wp_bfg_cannon);
		sd_play_player_item_sound(GETCANNONSND);
		break;

	case bo_coin:
		if (gamestate.tokens == MAX_TOKENS)
		{
			return;
		}
		GiveToken(1);

		writeTokenStr(bonus_msg24);
		break;

	case bo_coin5:
		if (gamestate.tokens == MAX_TOKENS)
		{
			return;
		}
		GiveToken(5);

		writeTokenStr(bonus_msg25);
		break;

	case bo_automapper1:
		if (gamestate.rpower > MAX_RADAR_ENERGY - (RADAR_PAK_VALUE / 8))
		{
			return;
		}
		gamestate.rpower += RADAR_PAK_VALUE;
		sd_play_player_item_sound(RADAR_POWERUPSND);
		UpdateRadarGuage();
		break;
	}

	if (givepoints)
	{
		GivePoints(static_points[check->itemnumber - bo_money_bag], true);
	}

	DISPLAY_TIMED_MSG(BonusMsg[check->itemnumber - 1], MP_BONUS, MT_BONUS);
	StartBonusFlash();
	check->shapenum = shapenum; // remove from list if shapenum == -1
	check->itemnumber = bo_nothing;

	if (check->shapenum == -1)
	{
		vid_hw_on_remove_static(*check);
	}
}

void writeTokenStr(
	std::string& string)
{
	auto token_string = std::to_string(gamestate.tokens);

	if (token_string.length() == 1)
	{
		token_string = '0' + token_string;
	}

	string.erase(string.length() - 2, 2);
	string += token_string;
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
bool TryMove(
	objtype* ob)
{
	int xl;
	int yl;
	int xh;
	int yh;

	if (ob == player)
	{
		xl = static_cast<int>(ob->x - PLAYERSIZE);
		yl = static_cast<int>(ob->y - PLAYERSIZE);
		xh = static_cast<int>(ob->x + PLAYERSIZE);
		yh = static_cast<int>(ob->y + PLAYERSIZE);
	}
	else
	{
		if (ob->obclass == blakeobj)
		{
			constexpr auto radius = bstone::math::fixed_to_floating(0x1000);

			xl = static_cast<int>(ob->x - radius);
			yl = static_cast<int>(ob->y - radius);
			xh = static_cast<int>(ob->x + radius);
			yh = static_cast<int>(ob->y + radius);
		}
		else
		{
			constexpr auto radius = bstone::math::fixed_to_floating(0x8000);

			xl = static_cast<int>(ob->x - radius);
			yl = static_cast<int>(ob->y - radius);
			xh = static_cast<int>(ob->x + radius);
			yh = static_cast<int>(ob->y + radius);
		}
	}

	//
	// check for solid walls
	//
	for (auto y = yl; y <= yh; ++y)
	{
		for (auto x = xl; x <= xh; ++x)
		{
			const auto check = actorat[x][y];

			if (check != nullptr)
			{
				if (check < objlist || (check->flags & FL_FAKE_STATIC) != 0)
				{
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

	for (auto y = yl; y <= yh; ++y)
	{
		if (y < 0 || y >= MAPSIZE)
		{
			continue;
		}

		for (auto x = xl; x <= xh; ++x)
		{
			if (x < 0 || x >= MAPSIZE)
			{
				continue;
			}

			const auto check = actorat[x][y];

			if (check > objlist && (check->flags & (FL_SOLID | FL_FAKE_STATIC)) == FL_SOLID)
			{
				auto deltax = ob->x - check->x;

				if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
				{
					continue;
				}

				auto deltay = ob->y - check->y;

				if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
				{
					continue;
				}

				return false;
			}
		}
	}

	return true;
}

//
// returns true if object hit a wall
//
bool ClipMove(
	objtype* ob,
	const double xmove,
	const double ymove)
{
	auto basex = ob->x;
	auto basey = ob->y;

	ob->x = basex + xmove;
	ob->y = basey + ymove;

	if (TryMove(ob))
	{
		return false;
	}

	if (!gp_no_wall_hit_sfx())
	{
		if (!sd_is_player_hit_wall_sound_playing())
		{
			sd_play_player_hit_wall_sound(HITWALLSND);
		}
	}

	ob->x = basex + xmove;
	ob->y = basey;

	if (TryMove(ob))
	{
		return true;
	}

	ob->x = basex;
	ob->y = basey + ymove;


	if (TryMove(ob))
	{
		return true;
	}

	ob->x = basex;
	ob->y = basey;

	return true;
}

void Thrust(
	std::int16_t angle,
	double speed)
{
	constexpr auto max_speed = 2.0 * MINDIST;

	extern TravelTable travel_table_;

	std::uint16_t offset;
	std::uint16_t* map[2]{};
	std::int16_t dx;
	std::int16_t dy;
	std::int16_t dangle;
	bool ignore_map1;

	thrustspeed += speed;

	//
	// moving bounds speed
	//
	if (speed > max_speed)
	{
		speed = max_speed;
	}

	const auto xmove = speed * costable[angle];
	const auto ymove = -speed * sintable[angle];

	ClipMove(player, xmove, ymove);

	player_oldtilex = player->tilex;
	player_oldtiley = player->tiley;
	player->tilex = static_cast<std::uint8_t>(player->x); // scale to tile values
	player->tiley = static_cast<std::uint8_t>(player->y);

	player->areanumber = GetAreaNumber(player->tilex, player->tiley);
	areabyplayer[player->areanumber] = true;
	travel_table_[player->tilex][player->tiley] |= TT_TRAVELED;

	offset = farmapylookup[player->tiley] + player->tilex;
	map[0] = &mapsegs[0][offset];
	map[1] = &mapsegs[1][offset];

	// Check for trigger tiles.
	//
	const auto& assets_info = get_assets_info();

	switch (*map[0])
	{
		case DOORTRIGGERTILE:
			dx = *map[1] >> 8; // x
			dy = *map[1] & 255; // y

			if (OperateSmartSwitch(dx, dy, ST_TOGGLE, false))
			{
				// Operate & Check for removeal
				*map[0] = AREATILE + player->areanumber;    // Remove switch
			}

			ignore_map1 = true;
			break;

		case SMART_OFF_TRIGGER:
		case SMART_ON_TRIGGER:
			dx = *map[1] >> 8;
			dy = *map[1] & 255;
			OperateSmartSwitch(dx, dy, static_cast<std::int8_t>((*map[0]) - SMART_OFF_TRIGGER), false);
			ignore_map1 = true;
			break;

		case WINTIGGERTILE:
			playstate = ex_victorious;

			{
				objtype dumb;
				dumb.x = gamestate.wintilex + 0.5;
				dumb.y = gamestate.wintiley + 0.5;
				dumb.flags = 0;
				dangle = CalcAngle(player, &dumb);
			}

			RotateView(dangle, 2);

			if (!assets_info.is_ps())
			{
				RunBlakeRun();
			}

			ignore_map1 = true;
			break;

		default:
			ignore_map1 = false;
			break;
	}

	if (!ignore_map1)
	{
		// Change sky and ground color on-the-fly.
		//

		offset = *(map[1] + 1); // 'offset' used as temp...

		switch (*map[1])
		{
			case 0xFE00:
				TopColor = offset & 0xFF00;
				TopColor |= TopColor >> 8;
				BottomColor = offset & 0xFF;
				BottomColor |= BottomColor << 8;
				break;
		}
	}
}

bool GAN_HiddenArea;

std::int8_t GetAreaNumber(
	int tilex,
	int tiley)
{
	GAN_HiddenArea = false;

	// Are we on a wall?
	//
	if (tilemap[tilex][tiley] != 0 &&
		(tilemap[tilex][tiley] & 0xC0) == 0)
	{
		return 127;
	}

	// Get initial areanumber from map
	//
	auto offset = farmapylookup[tiley] + tilex;
	auto areanumber = ValidAreaTile(&mapsegs[0][offset]);

	// Special tile areas must use a valid areanumber tile around it.
	//
	if (areanumber == 0)
	{
		auto found = false;

		for (auto i = 0; i < 8; ++i)
		{
			auto new_x = tilex + xy_offset[i][0];

			if (new_x < 0 || new_x >= MAPSIZE)
			{
				continue;
			}


			auto new_y = tiley + xy_offset[i][1];

			if (new_y < 0 || new_y >= MAPSIZE)
			{
				continue;
			}

			offset = farmapylookup[new_y] + new_x;
			areanumber = ValidAreaTile(&mapsegs[0][offset]);

			if (areanumber != 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			areanumber = AREATILE;
		}
	}

	// Merge hidden areanumbers into non-hidden areanumbers AND pull all
	// values down to an indexable range.
	//
	if (areanumber >= HIDDENAREATILE)
	{
		GAN_HiddenArea = true;
		areanumber -= HIDDENAREATILE;
	}
	else
	{
		areanumber -= AREATILE;
	}

	return areanumber;
}

std::uint8_t ValidAreaTile(
	const std::uint16_t* ptr)
{
	switch (*ptr)
	{
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
		if (*ptr > AREATILE)
		{
			return static_cast<std::uint8_t>(*ptr);
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

void select_floor()
{
	is_select_floor = false;

	const auto new_floor = InputFloor();

	if (new_floor != -1 && new_floor != gamestate.mapon)
	{
		const auto& assets_info = get_assets_info();
		const auto is_ps = assets_info.is_ps();

		if (is_ps)
		{
			gamestuff.level[gamestate.mapon].ptilex = player->tilex;
			gamestuff.level[gamestate.mapon].ptiley = player->tiley;

			auto angle = player->angle - 180;

			if (angle < 0)
			{
				angle += ANGLES;
			}

			gamestuff.level[gamestate.mapon].pangle = static_cast<std::int16_t>(angle);
		}

		playstate = is_ps ? ex_transported : ex_completed;

		gamestate.lastmapon = gamestate.mapon;
// FIXME
#if 0
		gamestate.mapon = static_cast<std::int16_t>(new_floor - 1);
#else
		gamestate.mapon = static_cast<std::int16_t>(new_floor);
#endif
		ClearPaletteShifts();
	}
	else
	{
		DrawPlayScreen(false);
	}
}

void Cmd_Fire()
{
	if (noShots)
	{
		return;
	}

	if ((gamestate.weapon == wp_autocharge) && (gamestate.weapon_wait))
	{
		return;
	}

	buttonheld[bt_attack] = true;

	gamestate.weaponframe = 0;

	player->state = &s_attack;

	gamestate.attackframe = 0;
	gamestate.attackcount = attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].tics;
	gamestate.weaponframe = attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].frame;
}

void Cmd_Use(
	bool& play_hit_wall_sound)
{
	play_hit_wall_sound = false;


	std::int16_t checkx;
	std::int16_t checky;
	std::int16_t door_index;
	std::int16_t dir;
	std::uint16_t iconnum;
	std::uint8_t static interrogate_delay = 0;

	bool tryDetonator = false;
	auto is_horizontal = false;

	// Find which cardinal direction the player is facing
	//
	if (player->angle < ANGLES / 8 || player->angle > 7 * ANGLES / 8)
	{
		checkx = player->tilex + 1;
		checky = player->tiley;
		dir = di_east;
		is_horizontal = true;
	}
	else if (player->angle < 3 * ANGLES / 8)
	{
		checkx = player->tilex;
		checky = player->tiley - 1;
		dir = di_north;
	}
	else if (player->angle < 5 * ANGLES / 8)
	{
		checkx = player->tilex - 1;
		checky = player->tiley;
		dir = di_west;
		is_horizontal = true;
	}
	else
	{
		checkx = player->tilex;
		checky = player->tiley + 1;
		dir = di_south;
	}

	door_index = tilemap[checkx][checky];
	iconnum = mapsegs[1][farmapylookup[checky] + checkx];

	// BBi Play sound only for walls
	if (door_index != 0 && (door_index & 0x80) == 0)
	{
		play_hit_wall_sound = true;
	}

	// Test for a pushable wall
	//
	const auto& assets_info = get_assets_info();

	if (iconnum == PUSHABLETILE)
	{
		PushWall(checkx, checky, dir);
	}
	else if (!buttonheld[bt_use])
	{
		// Test for doors / elevator
		//
		if ((door_index & 0xC0) == 0x80)
		{
			buttonheld[bt_use] = true;
			OperateDoor(door_index & ~0x80);
		}
		else
		{
			// Test for special tile types...
			//
			switch (door_index & 63)
			{
				// Test for 'display elevator buttons'
				//
				case TRANSPORTERTILE:
					is_select_floor = assets_info.is_aog() ? is_horizontal : true;
					break;

				case DIRECTTRANSPORTTILE:
					switch (iconnum & 0xff00)
					{
					case 0xf400:
						playstate = ex_transported;
						gamestate.lastmapon = gamestate.mapon;
// FIXME
#if 0
						gamestate.mapon = (iconnum & 0xff) - 1;
#else
						gamestate.mapon = iconnum & 0xff;
#endif
						break;

					default:
						// Stay in current level warp to new location

						playstate = ex_transported;
						Warped();
						playstate = ex_stillplaying;

						player->tilex = (iconnum >> 8);
						player->tiley = iconnum & 0xff;
						player->x = player->tilex + 0.5;
						player->y = player->tiley + 0.5;

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
					OperateConcession(static_cast<std::uint16_t>(reinterpret_cast<std::size_t>(actorat[checkx][checky])));
					break;

				default:
					if (assets_info.is_ps())
					{
						tryDetonator = true;
					}
					break;
			}
		}
	}
	else if (interrogate_delay == 0)
	{
		constexpr auto INTERROGATEDIST = MINACTORDIST;
		const std::int8_t MDIST = 2;
		const std::int16_t INTG_ANGLE = 45;

		objtype* intg_ob = nullptr;
		auto intg_dist = INTERROGATEDIST;

		for (auto y = -MDIST; y < MDIST + 1; ++y)
		{
			for (auto x = -MDIST; x < MDIST + 1; ++x)
			{
				const auto dst_x = player->tilex + x;
				const auto dst_y = player->tiley + y;

				// Don't check outside of the map plane:
				if (dst_x < 0 || dst_y < 0 || dst_x > 63 || dst_y > 63)
				{
					continue;
				}

				if (!(tilemap[dst_x][dst_y] == 0 && actorat[dst_x][dst_y] >= objlist))
				{
					continue;
				}

				const auto ob = actorat[dst_x][dst_y];
				const auto dx = std::abs(player->x - ob->x);
				const auto dy = std::abs(player->y - ob->y);
				const auto dist = dx < dy ? dx : dy;

				if (ob->obclass == gen_scientistobj &&
					(ob->flags & (FL_FRIENDLY | FL_VISIBLE)) == (FL_FRIENDLY | FL_VISIBLE) &&
					dist < intg_dist)
				{
					if ((ob->flags & FL_ATTACKMODE) != 0)
					{
						ob->flags &= ~(FL_FRIENDLY | FL_INFORMANT);
					}
					else
					{
						auto angle = CalcAngle(player, ob);

						angle = static_cast<std::int16_t>(std::abs(player->angle - angle));

						if (angle > (INTG_ANGLE / 2))
						{
							continue;
						}

						intg_ob = ob;
						intg_dist = dist;
					}
				}
			}
		}

		if (intg_ob != nullptr)
		{
			if (Interrogate(intg_ob))
			{
				interrogate_delay = 20; // Informants have 1/3 sec delay
			}
			else
			{
				interrogate_delay = 120; // Non-informants have 2 sec delay
			}
		}
		else if (assets_info.is_ps())
		{
			tryDetonator = true;
		}
	}
	else
	{
		if (tics < interrogate_delay)
		{
			interrogate_delay -= static_cast<std::uint8_t>(tics);
		}
		else
		{
			interrogate_delay = 0;
		}

		if (assets_info.is_ps())
		{
			tryDetonator = true;
		}
	}

	if (assets_info.is_ps())
	{
		if (tryDetonator)
		{
			if (!tryDetonatorDelay && gamestate.plasma_detonators != 0)
			{
				TryDropPlasmaDetonator();
				tryDetonatorDelay = 60;
			}
		}
		else
		{
			tryDetonatorDelay = 60;
		}
	}

	if (!buttonheld[bt_use])
	{
		interrogate_delay = 0;
	}
}


// ==========================================================================
//
//                           INTERROGATE CODE
//
// ==========================================================================

const int MSG_BUFFER_LEN = 150;

//char msg[MSG_BUFFER_LEN + 1];
auto msg = std::string{};

char* InfAreaMsgs[MAX_INF_AREA_MSGS];
std::uint8_t NumAreaMsgs, LastInfArea;
std::int16_t FirstGenInfMsg, TotalGenInfMsgs;

scientist_t InfHintList; // Informant messages
scientist_t NiceSciList; // Non-informant, non-pissed messages
scientist_t MeanSciList; // Non-informant, pissed messages

bool Interrogate(
	objtype* ob)
{
	const auto msg_interrogate = "INTERROGATE:";
	const auto msg_informant = " ^FC3aINFORMANT^FCa6";
	const auto msg_rr = "\r\r";
	const auto msg_xx = "^XX";
	const auto msg_have_ammo = " HEY BLAKE,\r TAKE MY CHARGE PACK!";
	const auto msg_have_token = " HEY BLAKE,\r TAKE MY FOOD TOKENS!";


	bool rt_value = true;
	const char* msgptr = nullptr;

	msg = msg_interrogate;
	auto local_message = std::string{};

	if ((ob->flags & FL_INFORMANT) != 0)
	{
		// Informant
		msg += msg_informant;

		if ((ob->flags & FL_INTERROGATED) != 0)
		{
			if ((ob->flags & FL_HAS_AMMO) != 0 && (gamestate.ammo != MAX_AMMO))
			{
				GiveAmmo((US_RndT() % 8) + 1);
				ob->flags &= ~FL_HAS_AMMO;
				msgptr = msg_have_ammo;
			}

			else if ((ob->flags & FL_HAS_TOKENS) && (gamestate.tokens != MAX_TOKENS))
			{
				GiveToken(5);
				ob->flags &= ~FL_HAS_TOKENS;
				msgptr = msg_have_token;
			}
		}

		if (!msgptr)
		{
			// If new areanumber OR no 'area msgs' have been compiled, compile
			// a list of all special messages for this areanumber.
			//
			if ((LastInfArea == 0xFF) || (LastInfArea != ob->areanumber))
			{
				NumAreaMsgs = 0;

				for (auto i = 0; i < InfHintList.NumMsgs; ++i)
				{
					const auto& ci = InfHintList.smInfo[i];

					if (ci.areanumber == 0xFF)
					{
						break;
					}

					if (ci.areanumber == ob->areanumber)
					{
						InfAreaMsgs[NumAreaMsgs++] = InfHintList.smInfo[ci.mInfo.local_val].mInfo.mSeg.data();
					}
				}

				LastInfArea = ob->areanumber;
			}

			// Randomly select an informant hint, either: specific to areanumber
			// or general hint...

			// unless points percentage is just bellow 100%
			// in which case report enemy or treasure location
			auto area_points_percent = 100;
			const auto& stats = gamestuff.level[gamestate.mapon].stats;

			if (stats.total_points != 0)
			{
				area_points_percent = stats.accum_points * 100 / stats.total_points;
			}

			if (area_points_percent > 96 && area_points_percent < 100)
			{
				local_message.reserve(256);
				const auto enemy_obj_to_kill = find_countable_enemy();

				if (enemy_obj_to_kill != nullptr)
				{
					local_message = " There is\r";
					local_message += get_enemy_actor_name(*enemy_obj_to_kill);

					local_message += "\r at " +
						std::to_string(enemy_obj_to_kill->tilex) +
						',' + std::to_string(enemy_obj_to_kill->tiley);

					local_message += "  (" +
						std::to_string(static_cast<int>(player->x)) + ',' +
						std::to_string(static_cast<int>(player->y)) + ')';
				}
				else
				{
					const auto bonus_item_obj = find_bonus_item();

					if (bonus_item_obj != nullptr)
					{
						auto is_plural = false;
						const auto name = get_bonus_item_name(*bonus_item_obj, is_plural);
						local_message = " There ";
						local_message += is_plural ? "are " : "is a\r ";

						local_message += name + "\r at "
							+ std::to_string(bonus_item_obj->tilex)
							+ ',' + std::to_string(bonus_item_obj->tiley);

						local_message += "  (" +
							std::to_string(static_cast<int>(player->x)) + ',' +
							std::to_string(static_cast<int>(player->y)) + ')';
					}
					else
					{
						// Wrong percentage?
						local_message = " You have collected\r all treasures";
					}
				}

				bstone::ascii::to_upper(local_message.begin(), local_message.end());
				msgptr = local_message.c_str();
			}
			else if (NumAreaMsgs)
			{
				if (ob->ammo != ob->areanumber)
				{
					ob->s_tilex = 0xFF;
				}

				ob->ammo = ob->areanumber;

				if (ob->s_tilex == 0xFF)
				{
					ob->s_tilex = static_cast<std::uint8_t>(Random(NumAreaMsgs));
				}

				msgptr = InfAreaMsgs[ob->s_tilex];
			}
			else
			{
				if (ob->s_tiley == 0xff)
				{
					ob->s_tiley = static_cast<std::uint8_t>(FirstGenInfMsg + Random(TotalGenInfMsgs));
				}

				msgptr = InfHintList.smInfo[ob->s_tiley].mInfo.mSeg.data();
			}

			// Still no msgptr? This is a shared message! Use smInfo[local_val]
			// for this message.
			//
			if (!msgptr)
			{
				msgptr = InfHintList.smInfo[InfHintList.smInfo[ob->s_tiley].mInfo.local_val].mInfo.mSeg.data();
			}

			ob->flags |= FL_INTERROGATED; // Scientist has been interrogated
		}
	}
	else
	{
		// Non-Informant
		//
		scientist_t* st;

		rt_value = false;

		if ((ob->flags & FL_MUST_ATTACK) != 0 || (US_RndT() & 1) != 0)
		{
			// Mean
			//
			ob->flags &= ~FL_FRIENDLY; // Make him attack!
			ob->flags |= FL_INTERROGATED; //  "    "     "
			st = &MeanSciList;
		}
		else
		{
			// Nice
			//
			ob->flags |= FL_MUST_ATTACK; // Make him mean!
			st = &NiceSciList;
		}

		msgptr = st->smInfo[Random(st->NumMsgs)].mInfo.mSeg.data();
	}

	if (msgptr)
	{
		msg += msg_rr;
		msg += msgptr;
		msg += msg_xx;

		if (msg.size() > static_cast<std::string::size_type>(MSG_BUFFER_LEN))
		{
			BSTONE_THROW_STATIC_SOURCE("Interrogation message too long.");
		}

		DisplayInfoMsg(msg.c_str(), MP_INTERROGATE, DISPLAY_MSG_STD_TIME * 2, MT_GENERAL);
		sd_play_player_interrogation_sound(INTERROGATESND);
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

// !!! Used in saved game.
static VgaBuffer ov_buffer;

bool ov_noImage = false;

const int TOV_X = 16;
const int TOV_Y = 132;


int aog_input_floor()
{
	const auto& assets_info = get_assets_info();

	static const std::string messages[4] =
	{
		// "Current floor:\nSelect a floor."
		ca_load_script(ELEVMSG0_TEXT),
		// "RED access card used to unlock floor!"
		ca_load_script(ELEVMSG1_TEXT),
		// "Floor is locked. Try another floor."
		ca_load_script(ELEVMSG4_TEXT),
		// "You must first get the red access keycard!"
		ca_load_script(ELEVMSG5_TEXT)
	}; // messages

	static const char* const floor_number_strings[10] =
	{
		"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
	};

	CA_CacheGrChunk(STARTFONT + 3);
	CacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);

	const auto old_vid_is_hud = vid_is_hud;
	vid_is_hud = true;
	vid_set_ui_mask_3d(false);
	VW_FadeOut();
	vid_is_hud = old_vid_is_hud;

	DrawTopInfo(sp_normal);

	auto border_width = 7;
	auto border_height = 5;
	auto outer_height = ref_view_height;

	BevelBox(
		0,
		ref_top_bar_height,
		static_cast<std::int16_t>(vga_ref_width),
		static_cast<std::int16_t>(outer_height),
		BORDER_HI_COLOR,
		BORDER_MED_COLOR,
		BORDER_LO_COLOR);

	BevelBox(
		static_cast<std::int16_t>(border_width),
		static_cast<std::int16_t>(ref_top_bar_height + border_height),
		static_cast<std::int16_t>(vga_ref_width - (2 * border_width)),
		static_cast<std::int16_t>(outer_height - (2 * border_height)),
		BORDER_LO_COLOR,
		BORDER_MED_COLOR,
		BORDER_HI_COLOR);

	CacheDrawPic(8, ref_top_bar_height + 6, TELEPORTBACKPIC);

	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT + 1);
	CacheBMAmsg(FLOORMSG_TEXT);
	UNCACHEGRCHUNK(STARTFONT + 1);

	ShowOverhead(
		14,
		ref_top_bar_height + 55,
		32,
		0,
		OV_KEYS | OV_WHOLE_MAP);

	IN_ClearKeysDown();

	auto result = -2;
	auto draw_stats = true;
	auto draw_message = true;
	auto draw_current_floor = true;
	auto draw_locked_floor = false;
	auto draw_cursor = true;
	auto use_delay = false;
	auto draw_button = false;
	auto button_index = 0;
	auto is_button_pressed = false;
	auto message = &messages[0];

	PresenterInfo pi{};
	pi.xl = 24;
	pi.yl = ref_top_bar_height + 8;
	pi.xh = pi.xl + 210;
	pi.yh = pi.yl + 34;
	pi.fontnumber = 3;
	pi.custom_line_height = 17;

	fontcolor = 0x38;

	auto last_unlocked_map = 0;

	const auto stats_levels_per_episode = assets_info.get_stats_levels_per_episode();

	for (int i = 1; i < stats_levels_per_episode; ++i)
	{
		if (!gamestuff.level[i].locked)
		{
			last_unlocked_map = i;
		}
	}

	const auto cursor_current_floor = static_cast<int>(gamestate.mapon);
	auto cursor_target_floor = cursor_current_floor;

	while (result == -2)
	{
		CalcTics();
		IN_ClearKeysDown();

		in_handle_events();

		if (Keyboard[ScanCode::sc_escape] ||
			Keyboard[ScanCode::sc_mouse_right])
		{
			result = -1;
		}

		auto target_level = 0;

		if (false)
		{
		}
		else if (Keyboard[ScanCode::sc_up_arrow] ||
			Keyboard[ScanCode::sc_right_arrow] ||
			in_is_binding_pressed(BindingId::e_bi_forward) ||
			in_is_binding_pressed(BindingId::e_bi_cycle_next_weapon) ||
			in_is_binding_pressed(BindingId::e_bi_right) ||
			in_is_binding_pressed(BindingId::e_bi_strafe_right))
		{
			cursor_target_floor += 1;

			if (cursor_target_floor > 10)
			{
				cursor_target_floor = 1;
			}

			draw_cursor = true;
		}
		else if (Keyboard[ScanCode::sc_down_arrow] ||
			Keyboard[ScanCode::sc_left_arrow] ||
			in_is_binding_pressed(BindingId::e_bi_backward) ||
			in_is_binding_pressed(BindingId::e_bi_cycle_previous_weapon) ||
			in_is_binding_pressed(BindingId::e_bi_left) ||
			in_is_binding_pressed(BindingId::e_bi_strafe_left))
		{
			cursor_target_floor -= 1;

			if (cursor_target_floor < 1)
			{
				cursor_target_floor = 10;
			}

			draw_cursor = true;
		}
		else if (
			Keyboard[ScanCode::sc_space] ||
			Keyboard[ScanCode::sc_mouse_left] ||
			in_is_binding_pressed(BindingId::e_bi_attack))
		{
			target_level = cursor_target_floor;

			draw_cursor = true;
		}

		for (auto i = static_cast<int>(ScanCode::sc_1);
			i <= static_cast<int>(ScanCode::sc_0);
			++i)
		{
			if (Keyboard[i])
			{
				target_level = i - static_cast<int>(ScanCode::sc_1) + 1;

				cursor_target_floor = target_level;
				draw_cursor = true;

				break;
			}
		}

		if (target_level >= 1 && target_level != gamestate.mapon)
		{
			sd_play_player_item_sound(ELEV_BUTTONSND);
			draw_button = true;
			is_button_pressed = true;
			button_index = target_level - 1;

			if (!gamestuff.level[target_level].locked)
			{
				result = target_level;
			}
			else if (gamestate.numkeys[kt_red] > 0 &&
				target_level == (last_unlocked_map + 1))
			{
				result = target_level;

				use_delay = true;
				draw_message = true;
				draw_current_floor = false;
				message = &messages[1];

				gamestate.numkeys[kt_red] = 0;
			}
			else
			{
				use_delay = true;
				draw_message = true;
				draw_current_floor = false;

				if (target_level == (last_unlocked_map + 1))
				{
					draw_locked_floor = false;
					message = &messages[3];
				}
				else
				{
					draw_locked_floor = true;
					message = &messages[2];
				}
			}
		}

		if (draw_message)
		{
			draw_message = false;

			VWB_DrawPic(24, ref_top_bar_height + 10, TELEPORT_TEXT_BG);

			fontcolor = 0x97;
			pi.script[0] = message->c_str();
			pi.flags = TPF_CACHE_NO_GFX | TPF_USE_CURRENT;
			pi.cur_x = 0xFFFF;
			pi.cur_y = 0xFFFF;
			TP_InitScript(&pi);
			TP_Presenter(&pi);

			if (draw_current_floor)
			{
				fontnumber = 3;
				fontcolor = 0x38;

				px = 167;
				py = ref_top_bar_height + 10;

				if (gamestate.mapon < 1 || gamestate.mapon > 10)
				{
					auto error_message = std::string{};
					error_message.reserve(64);
					error_message += "Current floor number out of range. (";
					error_message += std::to_string(gamestate.mapon);
					error_message += ')';
					BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
				}

				USL_DrawString(floor_number_strings[gamestate.mapon - 1]);
			}

			if (draw_locked_floor)
			{
				fontnumber = 3;
				fontcolor = 0x38;

				px = 82;
				py = ref_top_bar_height + 10;

				USL_DrawString(floor_number_strings[target_level - 1]);
			}

			if (draw_button)
			{
				draw_button = false;

				const auto base_x = 264;
				const auto base_y = ref_top_bar_height + 98;
				const auto step_x = 24;
				const auto step_y = 20;

				const auto x = base_x + (step_x * (button_index % 2));
				const auto y = base_y - (step_y * (button_index / 2));

				auto base_pic =
					is_button_pressed ?
					TELEPORT1ONPIC :
					TELEPORT1OFFPIC;

				VWB_DrawPic(
					x,
					y,
					base_pic + button_index);
			}
		}

		if (draw_cursor)
		{
			draw_cursor = false;

			const auto base_x = 264;
			const auto base_y = ref_top_bar_height + 98;
			const auto step_x = 24;
			const auto step_y = 20;

			for (int i_button = 0; i_button < 10; ++i_button)
			{
				auto color = 0;

				if (cursor_current_floor == (i_button + 1))
				{
					if (cursor_current_floor == cursor_target_floor)
					{
						color = 0x1F;
					}
					else
					{
						color = 0x18;
					}
				}
				else if (cursor_target_floor > 0 && cursor_target_floor == (i_button + 1))
				{
					color = 0xAF;
				}
				else
				{
					color = 0x82;
				}

				const auto x = base_x + (step_x * (i_button % 2));
				const auto y = base_y - (step_y * (i_button / 2));

				vwb_rect(x, y, 17, 17, static_cast<std::uint8_t>(color));
			}
		}

		CycleColors();
		VW_UpdateScreen();

		if (screenfaded)
		{
			VW_FadeIn();
		}

		if (draw_stats)
		{
			draw_stats = false;
			fontnumber = 2;
			fontcolor = 0xAF;
			static_cast<void>(ShowStats(
				167,
				ref_top_bar_height + 76,
				ss_normal,
				&gamestuff.level[gamestate.mapon].stats));
		}

		if (use_delay)
		{
			use_delay = false;
			draw_message = true;
			draw_current_floor = true;
			draw_locked_floor = false;
			draw_button = true;
			draw_cursor = true;
			is_button_pressed = false;
			message = &messages[0];

			IN_UserInput(210);
		}
	}

	IN_ClearKeysDown();

	return result;
}

int ps_input_floor()
{
	const auto& assets_info = get_assets_info();

	const auto RADAR_FLAGS = OV_KEYS;
	const auto MAX_TELEPORTS = 20;
	const auto MAX_MOVE_DELAY = 10;

	int buttonPic = 0;
	int buttonY = 0;
	int rt_code = -2;
	int tpNum = gamestate.mapon;
	int lastTpNum = tpNum;

	static const int teleX[MAX_TELEPORTS] =
	{
		16, 40, 86, 23, 44, 62, 83, 27, 118, 161, 161, 161, 213, 213, 184, 205, 226, 256, 276, 276,
	};

	static const int teleY[MAX_TELEPORTS] =
	{
		13, 26, 9, 50, 50, 50, 50, 62, 42, 17, 26, 35, 41, 50, 62, 62, 62, 10, 10, 30
	};

	int moveActive = 0;
	objtype old_player;
	bool locked = false;
	bool buttonsDrawn = false;

	ClearMemory();

	const auto old_vid_is_hud = vid_is_hud;
	vid_is_hud = true;
	vid_set_ui_mask_3d(false);
	VW_FadeOut();
	vid_is_hud = old_vid_is_hud;

	CacheDrawPic(0, 0, TELEPORTBACKTOPPIC);
	CacheDrawPic(0, 12 * 8, TELEPORTBACKBOTPIC);
	DisplayTeleportName(static_cast<std::int8_t>(tpNum), locked);
	CacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);
	VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1ONPIC + tpNum);

	old_player = *player;
	player->angle = 90;
	player->x = 32.5;
	player->y = player->x;

	ov_buffer.resize(4096);

	ShowStats(0, 0, ss_justcalc, &gamestuff.level[gamestate.mapon].stats);
	ov_stats = gamestuff.level[gamestate.mapon].stats;
	ShowOverhead(TOV_X, TOV_Y, 32, 0, RADAR_FLAGS);
	SaveOverheadChunk(tpNum);

	px = 115;
	py = 188;
	fontcolor = 0xAF;
	fontnumber = 2;
	ShPrint(if_help, 0, false);

	controlx = 0;
	controly = 0;

	IN_ClearKeysDown();

	while (rt_code == -2)
	{
		CalcTics();

		// BBi
		in_handle_events();

		if (Keyboard[ScanCode::sc_left_arrow])
		{
			controlx = -1;
		}
		else if (Keyboard[ScanCode::sc_right_arrow])
		{
			controlx = 1;
		}
		else
		{
			controlx = 0;
		}

		if (Keyboard[ScanCode::sc_up_arrow])
		{
			controly = -1;
		}
		else if (Keyboard[ScanCode::sc_down_arrow])
		{
			controly = 1;
		}
		else
		{
			controly = 0;
		}

		if (Keyboard[ScanCode::sc_escape] || buttonstate[bt_strafe])
		{
			rt_code = -1; // ABORT

			LoadLocationText(static_cast<std::int16_t>(
				gamestate.mapon + (assets_info.get_levels_per_episode() * gamestate.episode)));

			break;
		}
		else if (Keyboard[ScanCode::sc_return] || buttonstate[bt_attack])
		{
			if (locked)
			{
				if (!sd_is_player_no_way_sound_playing())
				{
					sd_play_player_no_way_sound(NOWAYSND);
				}
			}
			else
			{
				rt_code = tpNum; // ACCEPT

				// Flash selection
				//
				for (int loop = 0; loop < 10; ++loop)
				{
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
		if (moveActive != 0)
		{
			moveActive -= static_cast<int>(tics);

			if (moveActive < 0)
			{
				moveActive = 0;
			}
		}

		// Move to NEXT / PREV teleport?
		//
		buttonY = 0;

		if (controlx > 0 || controly > 0)
		{
			if (moveActive == 0 && tpNum < (MAX_TELEPORTS - 1))
			{
				tpNum += 1; // MOVE NEXT
				moveActive = MAX_MOVE_DELAY;
			}

			buttonPic = TELEDNONPIC;
			buttonY = 104;
		}
		else if (controlx < 0 || controly < 0)
		{
			if (moveActive == 0 && tpNum)
			{
				tpNum -= 1; // MOVE PREV
				moveActive = MAX_MOVE_DELAY;
			}

			buttonPic = TELEUPONPIC;
			buttonY = 91;
		}

		// Light buttons?
		//
		if (buttonY != 0)
		{
			VWB_DrawMPic(34, 91, TELEUPOFFPIC);
			VWB_DrawMPic(270, 91, TELEUPOFFPIC);
			VWB_DrawMPic(34, 104, TELEDNOFFPIC);
			VWB_DrawMPic(270, 104, TELEDNOFFPIC);

			VWB_DrawMPic(34, buttonY, buttonPic);
			VWB_DrawMPic(270, buttonY, buttonPic);
			buttonsDrawn = true;
		}
		else
			// Unlight buttons?
			//
			if (buttonsDrawn)
			{
				VWB_DrawMPic(34, 91, TELEUPOFFPIC);
				VWB_DrawMPic(270, 91, TELEUPOFFPIC);
				VWB_DrawMPic(34, 104, TELEDNOFFPIC);
				VWB_DrawMPic(270, 104, TELEDNOFFPIC);
				buttonsDrawn = false;
			}

		// Change visual information
		//
		if (tpNum != lastTpNum)
		{
			locked = gamestuff.level[tpNum].locked;
			DisplayTeleportName(static_cast<std::int8_t>(tpNum), locked);

			VWB_DrawMPic(teleX[lastTpNum], teleY[lastTpNum], TELEPORT1OFFPIC + lastTpNum);
			VWB_DrawMPic(teleX[tpNum], teleY[tpNum], TELEPORT1ONPIC + tpNum);

			LoadOverheadChunk(tpNum);
			ShowOverheadChunk();

			if (ov_noImage)
			{
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

		if (locked)
		{
			ShowOverhead(TOV_X, TOV_Y, 32, -1, RADAR_FLAGS);
		}

		CycleColors();
		VW_UpdateScreen();

		if (screenfaded)
		{
			VW_FadeIn();
			ShowStats(235, 138, ss_normal, &ov_stats);
			IN_ClearKeysDown();

			controlx = 0;
			controly = 0;
		}
	}

	vid_set_ui_mask(false);

	VW_FadeOut();

	*player = old_player;
	UnCacheLump(TELEPORT_LUMP_START, TELEPORT_LUMP_END);

	DrawPlayScreen(false);
	IN_ClearKeysDown();

	return rt_code;
}

int InputFloor()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		return aog_input_floor();
	}
	else
	{
		const auto old_vid_is_hud = vid_is_hud;
		vid_is_hud = false;
		const auto result = ps_input_floor();
		vid_is_hud = old_vid_is_hud;

		return result;
	}
}

void ShowOverheadChunk()
{
	VL_MemToScreen(ov_buffer.data(), 64, 64, TOV_X, TOV_Y);
	ShowStats(235, 138, ss_quick, &ov_stats);
}

void LoadOverheadChunk(
	int tpNum)
{
	// Find and load chunk
	//
	g_playtemp.set_position(0);

	const auto chunk_four_cc = bstone::sg_make_overlay_four_cc(tpNum);

	bool is_succeed = true;

	if (bstone::sg_find_chunk(chunk_four_cc, g_playtemp) > 0)
	{
		auto archiver = bstone::Archiver{};

		try
		{
			archiver.open(g_playtemp);

			archiver.read_uint8_array(ov_buffer.data(), 4096);
			ov_stats.unarchive(archiver);
			archiver.read_checksum();
		}
		catch (const std::exception&)
		{
			is_succeed = false;
		}
	}
	else
	{
		is_succeed = false;
	}

	if (!is_succeed)
	{
		std::fill(ov_buffer.begin(), ov_buffer.end(), VgaBuffer::value_type{0x52});
		std::fill_n(reinterpret_cast<std::uint8_t*>(&ov_stats), sizeof(statsInfoType), std::uint8_t{});
	}

	ov_noImage = !is_succeed;
}

void SaveOverheadChunk(
	int tpNum)
{
	// Remove level chunk from file
	//
	const auto chunk_four_cc = bstone::sg_make_overlay_four_cc(tpNum);
	char chunk_four_cc_chars[4] = {};
	*reinterpret_cast<std::uint32_t*>(chunk_four_cc_chars) = bstone::endian::to_little(chunk_four_cc.get_value());

	bstone::sg_delete_chunk(chunk_four_cc, g_playtemp);

	// Prepare buffer
	//
	VL_ScreenToMem(ov_buffer.data(), 64, 64, TOV_X, TOV_Y);

	// Write chunk ID, SIZE, and IMAGE
	//
	g_playtemp.seek(0, bstone::StreamOrigin::end);
	g_playtemp.write(chunk_four_cc_chars, 4);
	g_playtemp.skip(4);

	const auto beg_offset = g_playtemp.get_position();

	auto archiver = bstone::Archiver{};
	archiver.open(g_playtemp);

	archiver.write_uint8_array(ov_buffer.data(), 4096);
	ov_stats.archive(archiver);
	archiver.write_checksum();

	const auto end_offset = g_playtemp.get_position();
	const auto chunk_size = static_cast<std::int32_t>(end_offset - beg_offset);
	g_playtemp.skip(-(chunk_size + 4));
	archiver.write_int32(chunk_size);
}

void DisplayTeleportName(
	std::int8_t tpNum,
	bool locked)
{
	const char* s;
	int w;
	int h;

	if (locked)
	{
		fontcolor = 0xf5;
		s = "-- TELEPORT DISABLED --";
	}
	else
	{
		fontcolor = 0x57;
		LoadLocationText(tpNum);
		s = LocationText;
	}
	VW_MeasurePropString(s, &w, &h);
	py = 103;
	px = static_cast<std::int16_t>(160 - w / 2);
	VW_Bar(54, 101, 212, 9, 0x52);
	ShPrint(s, 0, false);
}

void CacheDrawPic(
	int x,
	int y,
	int pic)
{
	CA_CacheGrChunk(static_cast<std::int16_t>(pic));
	VWB_DrawPic(x, y, pic);
	UNCACHEGRCHUNK(static_cast<std::uint16_t>(pic));
}


// ===========================================================================
//
// MISSION STATISTICS CODE
//
// ===========================================================================

const int BAR_W = 48;
const int BAR_H = 5;

const int PERC_W = 13;
const int PERC_H = 5;

bool show_stats_quick;


std::int16_t ShowStats(
	std::int16_t bx,
	std::int16_t by,
	ss_type type,
	statsInfoType* stats)
{
	std::int16_t floor, total = 0, mission = 0, p1, p2, p3, loop, maxPerFloor;

	// Define max points per floor...
	//
	if (stats->total_points || stats->total_inf || stats->total_enemy)
	{
		maxPerFloor = 300;
	}
	else
	{
		maxPerFloor = 0;
	}

	// Setup to test for bypassing stats.
	//
	LastScan = ScanCode::sc_none;

	if (type == ss_quick)
	{
		show_stats_quick = true;
	}
	else
	{
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
	const auto& assets_info = get_assets_info();

	by += (assets_info.is_ps() ? 13 : 12);
	floor = p1 + p2 + p3;
	ShowRatio(bx, by, bx + 52, by, maxPerFloor, floor, type);

	// Show OVERALL MISSION ratio.
	//
	by += 7;
	stats->overall_floor = floor;

	const auto stats_levels_per_episode = assets_info.get_stats_levels_per_episode();

	for (loop = 0; loop < stats_levels_per_episode; loop++)
	{
		total += 300;
		mission += gamestuff.level[loop].stats.overall_floor;
	}
	mission = ShowRatio(bx, by, bx + 52, by, total, mission, type);

	if (show_stats_quick)
	{
		VW_UpdateScreen();
	}

	return mission;
}

std::uint8_t ShowRatio(
	std::int16_t bx,
	std::int16_t by,
	std::int16_t nx,
	std::int16_t ny,
	std::int32_t total,
	std::int32_t perc,
	ss_type type)
{
	std::int8_t numbars;
	std::int8_t maxperc;
	std::int8_t percentage = 1, loop;

	// Catch those nasty divide-by-zeros!
	//
	if (total)
	{
		maxperc = static_cast<std::int8_t>(LRATIO(100, total, perc, 10));
		numbars = static_cast<std::int8_t>(LRATIO(48, 100, maxperc, 10));
	}
	else
	{
		if (type != ss_justcalc)
		{
			fontcolor = 0x57;
			VW_Bar(bx, by, BAR_W, BAR_H, 0);
			VW_Bar(nx, ny, PERC_W + 6, PERC_H, 0);
			PrintX = nx;
			PrintY = ny;
			US_Print("N/A");
		}
		return 100;
	}

	if (type == ss_justcalc)
	{
		return maxperc;
	}

	PrintY = ny;
	fontcolor = 0xaf;
	fontnumber = 2;

	VW_Bar(bx, by, BAR_W, BAR_H, 0x07);
	PrintStatPercent(nx, ny, 0);
	for (loop = 0; loop < numbars; loop++)
	{
		in_handle_events();

		if (LastScan != ScanCode::sc_none)
		{
			show_stats_quick = true;
		}

		// Print one line of bar
		//
		VL_Vlin(bx++, by, BAR_H, 0xc8);

		// Keep up with current percentage
		//
		if (loop == numbars - 1)
		{
			percentage = maxperc;
		}
		else
		{
			percentage += 2;
		}

		PrintStatPercent(nx, ny, percentage);

		if (!show_stats_quick)
		{
			if (!(loop % 2))
			{
				sd_play_player_item_sound(STATS1SND);
			}
			VW_WaitVBL(1);
			VW_UpdateScreen();
		}
	}

	if (!show_stats_quick && numbars)
	{
		sd_play_player_item_sound(STATS2SND);

		while (sd_is_playing_any_ui_sound() && LastScan == ScanCode::sc_none)
		{
			in_handle_events();
		}
	}

	return maxperc;
}

void PrintStatPercent(
	std::int16_t nx,
	std::int16_t ny,
	std::int8_t percentage)
{
	if (percentage < 10)
	{
		PrintX = nx + 9;
	}
	else if (percentage < 100)
	{
		PrintX = nx + 4;
	}
	else
	{
		PrintX = nx - 1;
	}

	VW_Bar(nx, ny, PERC_W + 5, PERC_H, 0);
	US_PrintUnsigned(percentage);
	US_Print("%");
}

bool PerfectStats()
{
	if ((gamestuff.level[gamestate.mapon].stats.total_points ==
		gamestuff.level[gamestate.mapon].stats.accum_points) &&
		(gamestuff.level[gamestate.mapon].stats.total_inf ==
			gamestuff.level[gamestate.mapon].stats.accum_inf) &&
			(gamestuff.level[gamestate.mapon].stats.total_enemy ==
				gamestuff.level[gamestate.mapon].stats.accum_enemy))
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

void B_GAliFunc()
{
	extern const char* B_GAlienDead2;

	if (gamestate.episode == 5)
	{
		DisplayInfoMsg(B_GAlienDead2, MP_PINBALL_BONUS, 7 * 60, MT_BONUS);
	}
}

void B_EManFunc()
{
	sd_play_player_item_sound(EXTRA_MANSND);
	fontnumber = 2;
	LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	ShadowPrintLocationText(sp_normal);
}

void B_MillFunc()
{
	GiveAmmo(99);
	HealSelf(99);
}

void B_RollFunc()
{
	B_MillFunc();
	gamestate.score_roll_wait = SCORE_ROLL_WAIT;
}


const char* B_GAlienDead2 =
	"^FC57    GUARDIAN ALIEN\r"
	"      DESTROYED!\r\r"
	"^FCA6 FIND AND DESTROY ALL\r"
	"PROJECTION GENERATORS!"
;

const char* B_GAlienDead =
	"^FC57    GUARDIAN ALIEN\r"
	"      DESTROYED!\r\r"
	"^FCA6   FIND THE EXIT TO\r"
	"COMPLETE THIS MISSION"
;

const char* B_ScoreRolled =
	"^FC57\rROLLED SCORE DISPLAY!\r"
	"^FCA6   FULL AMMO BONUS!\r"
	"  FULL HEALTH BONUS!\r"
	"1,000,000 POINT BONUS!"
;

const char* B_OneMillion =
	"^FC57\r     GREAT SCORE!\r"
	"^FCA6   FULL AMMO BONUS!\r"
	"  FULL HEALTH BONUS!\r"
	"1,000,000 POINT BONUS!"
;

const char* B_ExtraMan =
	"^FC57\r\r     GREAT SCORE!\r"
	"^FCA6  EXTRA LIFE BONUS!\r"
;

const char* B_EnemyDestroyed =
	"^FC57\r\r ALL ENEMY DESTROYED!\r"
	"^FCA6  50,000 POINT BONUS!\r"
;

const char* B_TotalPoints =
	"^FC57\r\r ALL POINTS COLLECTED!\r"
	"^FCA6  50,000 POINT BONUS!\r"
;

const char* B_InformantsAlive =
	"^FC57\r\r ALL INFORMANTS ALIVE!\r"
	"^FCA6  50,000 POINT BONUS!\r"
;


const PinballBonusInfos PinballBonus =
{
	//                                       Special
	//  BonusText           Points   Recur? Function
	// -----------------------------------------------------

	PinballBonusInfo{B_GAlienDead, 0, false, B_GAliFunc},
	PinballBonusInfo{B_ScoreRolled, 1000000l, true, B_RollFunc},
	PinballBonusInfo{B_OneMillion, 1000000l, false, B_MillFunc},
	PinballBonusInfo{B_ExtraMan, 0, true, B_EManFunc},
	PinballBonusInfo{B_EnemyDestroyed, 50000l, false, nullptr},
	PinballBonusInfo{B_TotalPoints, 50000l, false, nullptr},
	PinballBonusInfo{B_InformantsAlive, 50000l, false, nullptr},
};

void DisplayPinballBonus()
{
	const auto is_victory = (playstate == ex_victorious);

	// Check queue for bonuses
	//
	const auto count = static_cast<int>(sizeof(gamestuff.level[0].bonus_queue) * 8);

	for (int loop = 0; loop < count; ++loop)
	{
		if ((BONUS_QUEUE & (1 << loop)) != 0 && (is_victory || LastMsgPri < MP_PINBALL_BONUS))
		{
			if (!is_victory)
			{
				// Start this bonus!
				//
				sd_play_player_item_sound(ROLL_SCORESND);
				DisplayInfoMsg(PinballBonus[loop].BonusText, MP_PINBALL_BONUS, 7 * 60, MT_BONUS);
			}

			// Add to "shown" ... Remove from "queue"
			//
			if (!PinballBonus[loop].Recurring)
			{
				BONUS_SHOWN |= (1 << loop);
			}

			BONUS_QUEUE &= ~(1 << loop);

			// Give points and execute special function.
			//
			GivePoints(PinballBonus[loop].Points, false);

			if (PinballBonus[loop].func)
			{
				PinballBonus[loop].func();
			}
		}
	}
}

void CheckPinballBonus(
	std::int32_t points)
{
	const auto score_before = gamestate.score;
	const auto score_after = gamestate.score + points;

	// Check SCORE ROLLED bonus
	//
	if (score_before <= MAX_DISPLAY_SCORE && score_after > MAX_DISPLAY_SCORE)
	{
		ActivatePinballBonus(B_SCORE_ROLLED);
	}

	// Check ONE MILLION bonus
	//
	if (score_before < 500000l && score_after >= 500000l)
	{
		ActivatePinballBonus(B_ONE_MILLION);
	}

	// Check EXTRA MAN bonus
	//
	if (score_after >= gamestate.nextextra)
	{
		gamestate.nextextra += EXTRAPOINTS;
		if (gamestate.lives < MAX_EXTRA_LIVES)
		{
			gamestate.lives++;
			ActivatePinballBonus(B_EXTRA_MAN);
		}
	}

	// Check TOTAL ENEMY bonus
	//
	// FIXME
	// https://github.com/bibendovsky/bstone/issues/86
	// https://github.com/bibendovsky/bstone/pull/176
	if (gamestuff.level[gamestate.mapon].stats.total_enemy <=  
		gamestuff.level[gamestate.mapon].stats.accum_enemy)
	{
		ActivatePinballBonus(B_ENEMY_DESTROYED);
	}

	// Check TOTAL POINTS bonus
	//
	// FIXME
	// https://github.com/bibendovsky/bstone/issues/86
	// https://github.com/bibendovsky/bstone/pull/176
	if (gamestuff.level[gamestate.mapon].stats.total_points <= 
		gamestuff.level[gamestate.mapon].stats.accum_points)
	{
		ActivatePinballBonus(B_TOTAL_POINTS);
	}

	// Check INFORMANTS ALIVE bonus
	//
	// FIXME
	// https://github.com/bibendovsky/bstone/issues/86
	// https://github.com/bibendovsky/bstone/pull/176
	if ((gamestuff.level[gamestate.mapon].stats.total_inf <=
		gamestuff.level[gamestate.mapon].stats.accum_inf) && // All informants alive?
		(gamestuff.level[gamestate.mapon].stats.total_inf) && // Any informants in level?
		((BONUS_SHOWN & (B_TOTAL_POINTS | B_ENEMY_DESTROYED)) ==
		(B_TOTAL_POINTS | B_ENEMY_DESTROYED))) // Got ENEMY and POINTS bonuses?
	{
		ActivatePinballBonus(B_INFORMANTS_ALIVE);
	}

	// Display bonuses?
	//
	if (BONUS_QUEUE)
	{
		DisplayPinballBonus();
	}
}


/*
=============================================================================

 PLAYER CONTROL

=============================================================================
*/

void SpawnPlayer(
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir)
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		if (gamestuff.level[gamestate.mapon].ptilex &&
			gamestuff.level[gamestate.mapon].ptiley)
		{
			tilex = gamestuff.level[gamestate.mapon].ptilex;
			tiley = gamestuff.level[gamestate.mapon].ptiley;
			dir = 1 + (gamestuff.level[gamestate.mapon].pangle / 90);
		}
	}

	player->obclass = playerobj;
	player->active = ac_yes;
	player->tilex = static_cast<std::uint8_t>(tilex);
	player->tiley = static_cast<std::uint8_t>(tiley);

	player->areanumber = GetAreaNumber(player->tilex, player->tiley);

	player->x = tilex + 0.5;
	player->y = tiley + 0.5;
	player->state = &s_player;
	player->angle = (1 - dir) * 90;
	if (player->angle < 0)
	{
		player->angle += ANGLES;
	}
	player->flags = FL_NEVERMARK;
	Thrust(0, 0); // set some variables

	InitAreas();

	InitWeaponBounce();
}

void GunAttack(
	objtype* ob)
{
	if (gamestate.weapon != wp_autocharge)
	{
		MakeAlertNoise(ob);
	}

	auto skip = false;

	switch (gamestate.weapon)
	{
	case wp_autocharge:
		skip = true;
		sd_play_player_weapon_sound(ATKAUTOCHARGESND);
		break;

	case wp_pistol:
		skip = true;
		sd_play_player_weapon_sound(ATKCHARGEDSND);
		break;

	case wp_burst_rifle:
		sd_play_player_weapon_sound(ATKBURSTRIFLESND);
		break;

	case wp_ion_cannon:
		sd_play_player_weapon_sound(ATKIONCANNONSND);
		break;

	default:
		break;
	}

	static const auto object_radius = 0.5;

	const auto theta = bstone::math::deg_to_rad(player->angle);

	const auto theta_cos = std::cos(theta);
	const auto theta_sin = std::sin(theta);

	const auto x_1 = player->x;
	const auto y_1 = (MAPSIZE - 1) - player->y;

	//
	// find potential targets
	//

	auto view_dist = std::numeric_limits<double>::max();

	objtype* closest = nullptr;

	while (true)
	{
		const auto old_closest = closest;

		auto closest_x = 0.0;
		auto closest_y = 0.0;

		for (auto check = ob->next; check; check = check->next)
		{
			if ((check->flags & FL_SHOOTABLE) == 0 || (check->flags & FL_VISIBLE) == 0)
			{
				continue;
			}

			const auto x_0 = check->x;
			const auto y_0 = (MAPSIZE - 1) - check->y;

			const auto dx_0_1 = x_0 - x_1;
			const auto dy_0_1 = y_0 - y_1;

			//
			// How to calculate a distance from a line defined by two points to the given one.
			// (http://wikipedia.org/wiki/Distance_from_a_point_to_a_line)
			//
			//                              |(y2 - y1) x0 - (x2 - x1) y0 + x2 y1 - y2 x1|
			// distance(P1, P2, (x0, y0)) = ---------------------------------------------
			//                                   sqrt((y2 - y1) ^ 2 + (x2 - x1) ^ 2)
			//
			// where:
			//    P1 = (x1, y1), P2 = (x2, y2) - two points which defines the line;
			//    (x0, y0) - a point to calculate the distance to.
			//
			//
			// Our case:
			//
			// ->   ->   ->   ------->   ----------------------->
			// P2 = P1 +  u = (x1, y1) + (cos(theta), sin(theta))
			//
			// where: (x1, y1) - player's position; theta - player's direction as angle.
			//
			// So the distance is:
			//
			// distance(x1, y1, a, (x0, y0)) = |sin(theta) (x0 - x1) - cos(theta) (y0 - y1)|
			//
			const auto distance_to_hitscan_line = std::abs((theta_sin * dx_0_1) - (theta_cos * dy_0_1));

			if (distance_to_hitscan_line > object_radius)
			{
				continue;
			}

			// Squared distance to an object from the player.
			const auto sqr_distance_to_object = (dx_0_1 * dx_0_1) + (dy_0_1 * dy_0_1);

			if (sqr_distance_to_object < view_dist)
			{
				if (skip && (check->obclass == hang_terrotobj))
				{
					continue;
				}

				view_dist = sqr_distance_to_object;

				closest = check;

				closest_x = x_0;
				closest_y = y_0;
			}
		}

		if (closest == old_closest)
		{
			return; // no more targets, all missed
		}

		const auto direction_to_closest_x = closest_x - x_1;
		const auto direction_to_closest_y = closest_y - y_1;

		const auto dot = (direction_to_closest_x * theta_cos) + (direction_to_closest_y * theta_sin);

		if (dot <= 0)
		{
			// The closest target should be in front of the player.
			continue;
		}

		//
		// trace a line from player to enemey
		//
		if (CheckLine(closest, player))
		{
			break;
		}
	}

	//
	// hit something
	//

	const auto dx = std::abs(closest->tilex - player->tilex);
	const auto dy = std::abs(closest->tiley - player->tiley);

	auto dist = (dx > dy ? dx : dy);

	auto damage = 0;

	if (dist < 2)
	{
		damage = US_RndT() / 2; // 4
	}
	else if (dist < 4)
	{
		damage = US_RndT() / 4; // 6
	}
	else
	{
		if ((US_RndT() / 12) < dist)
		{
			// missed
			return;
		}

		damage = US_RndT() / 4; // 6
	}

	DamageActor(closest, static_cast<std::uint16_t>(damage), player);
}

void T_Attack(
	objtype* ob)
{
	atkinf_t* cur;
	std::int16_t x;

	if (noShots)
	{
		ob->state = &s_player;
		gamestate.attackframe = gamestate.weaponframe = 0;
		return;
	}

	if (gamestate.weapon == wp_autocharge)
	{
		UpdateAmmoMsg();
	}

	if (buttonstate[bt_use] && !buttonheld[bt_use])
	{
		buttonstate[bt_use] = false;
	}

	if (buttonstate[bt_attack] && !buttonheld[bt_attack])
	{
		buttonstate[bt_attack] = false;
	}

	ControlMovement(ob);

	player->tilex = static_cast<std::uint8_t>(player->x); // scale to tile values
	player->tiley = static_cast<std::uint8_t>(player->y);

	//
	// change frame and fire
	//
	const auto& assets_info = get_assets_info();

	gamestate.attackcount -= tics;
	if (gamestate.attackcount <= 0)
	{
		cur = &attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe];
		switch (cur->attack)
		{
		case -1:
			ob->state = &s_player;

			if (!gamestate.ammo)
			{
				if (gamestate.weapon != wp_autocharge)
				{
					gamestate.weapon = wp_autocharge;
					DrawWeapon();
					DisplayInfoMsg(EnergyPackDepleted, MP_NO_MORE_AMMO, DISPLAY_MSG_STD_TIME << 1, MT_OUT_OF_AMMO);
				}
			}
			else
			{
				if (!(gamestate.useable_weapons & (1 << gamestate.weapon)))
				{
					gamestate.weapon = wp_autocharge;
					DrawWeapon();
					DisplayInfoMsg(NotEnoughEnergyForWeapon, MP_NO_MORE_AMMO, DISPLAY_MSG_STD_TIME << 1, MT_OUT_OF_AMMO);
				}
			}
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case -2:
			ob->state = &s_player;
			if (!gamestate.plasma_detonators)
			{
				// Check to see what weapons are possible.
				//
				const auto n_x = static_cast<std::int16_t>(assets_info.is_ps() ? wp_bfg_cannon : wp_grenade);

				for (x = n_x; x >= wp_autocharge; x--)
				{
					if (gamestate.useable_weapons & (1 << x))
					{
						gamestate.weapon = static_cast<std::int8_t>(x);
						break;
					}
				}

				DrawWeapon();
			}
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case 4:
			if (!gamestate.ammo)
			{
				break;
			}
			if (buttonstate[bt_attack])
			{
				gamestate.attackframe -= 2;
			}

		case 0:
			if (gamestate.weapon == wp_grenade)
			{
				if (!objfreelist)
				{
					DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
					gamestate.attackframe++;
				}
			}
			break;

		case 1:
			if (!gamestate.ammo)
			{ // can only happen with chain gun
				gamestate.attackframe++;
				break;
			}
			GunAttack(ob);
			if (!godmode)
			{
				gamestate.ammo--;
			}
			if (!assets_info.is_ps())
			{
				DrawWeapon();
			}
			else
			{
				DrawAmmo(false);
			}
			break;

		case 2:
			if (gamestate.weapon_wait)
			{
				break;
			}
			GunAttack(ob);
			gamestate.weapon_wait = AUTOCHARGE_WAIT;
			if (!assets_info.is_ps())
			{
				DrawWeapon();
			}
			else
			{
				DrawAmmo(false);
			}
			break;

		case 3:
			if (gamestate.ammo && buttonstate[bt_attack])
			{
				gamestate.attackframe -= 2;
			}
			break;

		case 6:
			if (gamestate.ammo && buttonstate[bt_attack])
			{
				if (objfreelist)
				{
					gamestate.attackframe -= 2;
				}
			}
			else if (gamestate.ammo == 0)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
			}
			break;

		case 5:
			if (!objfreelist)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
				gamestate.attackframe++;
			}
			else
			{
				if (LastMsgType == MT_MALFUNCTION)
				{
					MsgTicsRemain = 1; // Clear Malfuction Msg before anim

				}
				if (!godmode)
				{
					if (gamestate.ammo >= GRENADE_ENERGY_USE)
					{
						gamestate.ammo -= GRENADE_ENERGY_USE;
						if (!assets_info.is_ps())
						{
							DrawWeapon();
						}
						else
						{
							DrawAmmo(false);
						}
					}
					else
					{
						gamestate.attackframe++;
					}
				}

				sd_play_player_weapon_sound(ATKGRENADESND);
				SpawnProjectile(ob, grenadeobj);
				MakeAlertNoise(ob);
			}
			break;

		case 7:
			TryDropPlasmaDetonator();
			DrawAmmo(false);
			break;

		case 8:
			if (gamestate.plasma_detonators && buttonstate[bt_attack])
			{
				gamestate.attackframe -= 2;
			}
			break;

		case 9:
			if (!objfreelist)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
				gamestate.attackframe++;
			}
			else
			{
				if (LastMsgType == MT_MALFUNCTION)
				{
					MsgTicsRemain = 1; // Clear Malfuction Msg before anim

				}
				if (!godmode)
				{
					if (gamestate.ammo >= BFG_ENERGY_USE)
					{
						gamestate.ammo -= BFG_ENERGY_USE;
						DrawAmmo(false);
					}
					else
					{
						gamestate.attackframe++;
					}
				}

				sd_play_player_weapon_sound(ATKIONCANNONSND);
				SpawnProjectile(ob, bfg_shotobj);
				MakeAlertNoise(ob);
			}
			break;

		case 10:
			if (gamestate.ammo && buttonstate[bt_attack])
			{
				if (objfreelist)
				{
					gamestate.attackframe -= 2;
				}
			}
			else if (gamestate.ammo == 0)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction, MP_WEAPON_MALFUNCTION, MT_MALFUNCTION);
			}
			break;
		}

		gamestate.attackcount += cur->tics;
		gamestate.attackframe++;
		gamestate.weaponframe =
			attackinfo[static_cast<int>(gamestate.weapon)][gamestate.attackframe].frame;
	}

	try_to_grab_bonus_items();
}

void T_Player(
	objtype* ob)
{
	CheckWeaponChange();

	if (gamestate.weapon == wp_autocharge)
	{
		UpdateAmmoMsg();
	}

	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		if (tryDetonatorDelay > tics)
		{
			tryDetonatorDelay -= tics;
		}
		else
		{
			tryDetonatorDelay = 0;
		}
	}

	if (buttonstate[bt_use])
	{
		bool play_hit_wall_sound;

		Cmd_Use(play_hit_wall_sound);

		if (play_hit_wall_sound)
		{
			sd_play_player_hit_wall_sound(HITWALLSND);
		}
	}

	if (buttonstate[bt_attack] && !buttonheld[bt_attack])
	{
		Cmd_Fire();
	}

	ControlMovement(ob);
	HandleWeaponBounce();

	player->tilex = static_cast<std::uint8_t>(player->x); // scale to tile values
	player->tiley = static_cast<std::uint8_t>(player->y);

	try_to_grab_bonus_items();
}

void RunBlakeRun()
{
	vid_is_hud = true;

	constexpr auto BLAKE_SPEED = MOVESCALE * 50;

	// Spawn Blake and set pointer.
	//
	SpawnPatrol(en_blake, player->tilex, player->tiley, static_cast<std::int16_t>(player->dir >> 1));

	auto blake = new_actor;

	// Blake object starts one tile behind player object.
	//
	switch (blake->dir)
	{
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

	default:
		break;
	}

	// Align Blake on the middle of the tile.
	//
	blake->x = blake->tilex + 0.5;
	blake->y = blake->tiley + 0.5;
	blake->tilex = static_cast<std::uint8_t>(blake->x);
	auto startx = static_cast<int>(blake->tilex);
	blake->tiley = static_cast<std::uint8_t>(blake->y);
	auto starty = static_cast<int>(blake->tiley);

	// Run, Blake, Run!
	//
	int dx;
	int dy;

	do
	{
		// Calc movement in X and Y directions.
		//
		const auto xmove = BLAKE_SPEED * costable[player->angle];
		const auto ymove = -BLAKE_SPEED * sintable[player->angle];

		// Move, animate, and redraw.
		//
		if (ClipMove(blake, xmove, ymove))
		{
			break;
		}

		DoActor(blake);
		ThreeDRefresh();

		// Calc new tile X/Y.
		//
		blake->tilex = static_cast<std::uint8_t>(blake->x);
		blake->tiley = static_cast<std::uint8_t>(blake->y);

		// Evaluate distance from start.
		//
		dx = std::abs(blake->tilex - startx);
		dy = std::abs(blake->tiley - starty);

		// BBi
		in_handle_events();
	} while (dx < 6 && dy < 6);

	vid_is_hud = false;
}

void SW_HandleActor(
	objtype* obj)
{
	if (!obj->active)
	{
		obj->active = ac_yes;
	}

	switch (obj->obclass)
	{
	case rentacopobj:
	case gen_scientistobj:
	case swatobj:
	case goldsternobj:
	case proguardobj:
		if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK)))
		{
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

	case floatingbombobj:
	case volatiletransportobj:
		if (obj->flags & FL_STATIONARY)
		{
			KillActor(obj);
		}
		else if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK)))
		{
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
		if (!(obj->flags & (FL_ATTACKMODE | FL_FIRSTATTACK)))
		{
			FirstSighting(obj);
		}
		break;

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
	std::uint16_t tilex,
	std::uint16_t tiley)
{
	const auto& assets_info = get_assets_info();

	switch (stat->itemnumber)
	{
	case bo_clip:
	case bo_clip2:
		if (assets_info.is_ps())
		{
			SpawnCusExplosion(
				tilex + 0.5,
				tiley + 0.5,
				SPR_CLIP_EXP1,
				7,
				30 + (US_RndT() & 0x27),
				explosionobj
			);
		}
		stat->shapenum = -1;
		stat->itemnumber = bo_nothing;

		vid_hw_on_remove_static(*stat);

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
//              and last tilex & tiley coords.  This is useful for other
//              actors toggling barrier switches.
//
// RETURNS: Boolean: TRUE  - Remove switch from map
//      FALSE - Keep switch in map
//
// -------------------------------------------------------------------------
bool OperateSmartSwitch(
	std::uint16_t tilex,
	std::uint16_t tiley,
	std::int8_t Operation,
	bool Force)
{
	enum what_is_it
	{
		wit_NOTHING,
		wit_DOOR,
		wit_WALL,
		wit_STATIC,
		wit_ACTOR
	}; // what_is_it

	what_is_it WhatItIs;
	objtype* obj;
	statobj_t* stat = nullptr;
	std::uint8_t tile, DoorNum = 0;
	std::uint16_t iconnum;

	//
	// Get some information about what
	// this switch is pointing to.
	//

	tile = tilemap[tilex][tiley];
	obj = actorat[tilex][tiley];
	iconnum = mapsegs[1][farmapylookup[tiley] + tilex];
	WhatItIs = wit_NOTHING;

	//
	// Deterimine if the switch points to an
	// actor, door, wall, static or is Special.
	//

	if (obj < objlist)
	{
		if (obj == (objtype*)1 && tile == 0)
		{
			// We have a SOLID static!

			WhatItIs = wit_STATIC;
		}
		else
		{
			if (tile)
			{
				//
				// We have a wall of some type (maybe a door).
				//

				if (tile & 0x80)
				{
					// We have a door

					WhatItIs = wit_DOOR;
					DoorNum = tile & 0x3f;
				}
				else
				{
					// We have a wall

					WhatItIs = wit_WALL;
				}
			}
			else
			{
				stat = FindStatic(tilex, tiley);

				if (stat)
				{
					WhatItIs = wit_STATIC;
				}
			}
		}
	}
	else
	{
		if (obj < &objlist[MAXACTORS])
		{
			// We have an actor.

			WhatItIs = wit_ACTOR;
		}
		else
		{
			WhatItIs = wit_NOTHING;
		}
	}

	//
	// Ok... Now do that voodoo that you do so well...
	//

	switch (WhatItIs)
	{
		//
		// Handle Doors
		//
	case wit_DOOR:
		if (doorobjlist[DoorNum].action == dr_jammed)
		{
			return false;
		}

		doorobjlist[DoorNum].lock = kt_none;
		OpenDoor(DoorNum);

		vid_hw_on_update_door_lock(DoorNum);

		return false;


		//
		// Handle Actors
		//
	case wit_ACTOR:
		if (!(obj->flags & FL_DEADGUY))
		{
			SW_HandleActor(obj);
		}
		return true;


		//
		// Handle Walls
		//
	case wit_WALL:
	{
		if (Force || player_oldtilex != player->tilex || player_oldtiley != player->tiley)
		{
			switch (tile)
			{
			case OFF_SWITCH:
				if (Operation == ST_TURN_OFF)
				{
					return false;
				}

				ActivateWallSwitch(iconnum, tilex, tiley);
				break;

			case ON_SWITCH:
				if (Operation == ST_TURN_ON)
				{
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
		stat = FindStatic(tilex, tiley);

		if (!stat)
		{
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

constexpr auto wb_MaxPoint = 10.0;
constexpr auto wb_MidPoint = 6.0;
constexpr auto wb_MinPoint = 2.0;
constexpr auto wb_MaxGoalDist = wb_MaxPoint - wb_MidPoint;

constexpr auto wb_MaxOffset = wb_MaxPoint + 2.0;
constexpr auto wb_MinOffset = wb_MinPoint - 2.0;

extern double bounceOffset;

double bounceVel;
double bounceDest;
std::int16_t bounceOk;

void InitWeaponBounce()
{
	bounceOffset = wb_MidPoint;
	bounceDest = wb_MaxPoint;
	bounceVel = 0;
	bounceOk = 0;
}

void HandleWeaponBounce()
{
	int bounceSpeed;

	bounceSpeed = 90 - ((20 - viewsize) * 6);

	if (bounceOk)
	{
		if (bounceOffset < bounceDest)
		{
			bounceVel += sintable[bounceSpeed] / 2.0;
			bounceOffset += bounceVel;

			if (bounceOffset > bounceDest)
			{
				bounceDest = wb_MinPoint;
				bounceVel /= 4.0;
			}
		}
		else if (bounceOffset > bounceDest)
		{
			bounceVel -= sintable[bounceSpeed] / 4.0;
			bounceOffset += bounceVel;

			if (bounceOffset < bounceDest)
			{
				bounceDest = wb_MaxPoint;
				bounceVel /= 4.0;
			}
		}
	}
	else
	{
		if (bounceOffset > wb_MidPoint)
		{
			bounceOffset -= 2.0;

			if (bounceOffset < wb_MidPoint)
			{
				bounceOffset = wb_MidPoint;
			}
		}
		else if (bounceOffset < wb_MidPoint)
		{
			bounceOffset += 2.0;

			if (bounceOffset > wb_MidPoint)
			{
				bounceOffset = wb_MidPoint;
			}
		}

		bounceDest = wb_MaxPoint;
		bounceVel = 0;
	}

	if (bounceOffset > wb_MaxOffset)
	{
		bounceOffset = wb_MaxOffset;
	}
	else if (bounceOffset < wb_MinOffset)
	{
		bounceOffset = wb_MinOffset;
	}
}


namespace
{


void try_to_grab_bonus_items()
{
	const auto item_radius = 0.25;
	const auto item_tile_offset = 0.5;
	const auto player_radius = PLAYERSIZE;
	const auto min_distance = item_radius + player_radius;
	const auto min_sqr_distance = min_distance * min_distance;

	const auto player_x = player->x;
	const auto player_y = player->y;

	for (auto item = statobjlist.data(); item != laststatobj; ++item)
	{
		if (item->shapenum < 0 || (item->flags & FL_BONUS) == 0)
		{
			continue;
		}

		const auto item_x = static_cast<double>(item->tilex) + item_tile_offset;
		const auto item_y = static_cast<double>(item->tiley) + item_tile_offset;

		const auto dx = item_x - player_x;
		const auto dy = item_y - player_y;

		const auto sqr_distance = (dx * dx) + (dy * dy);

		if (sqr_distance > min_sqr_distance)
		{
			continue;
		}

		GetBonus(item);
	}
}


}


double player_get_weapon_bounce_offset()
{
	return bounceOffset;
}
