/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"


void INL_GetJoyDelta(
	std::uint16_t joy,
	int* dx,
	int* dy);

void UpdateRadarGuage();
void ClearMemory();

void GiveWeapon(
	int weapon);

void DrawWeapon();
void DrawHealth();
void DrawKeys();
void DrawScore();
void ForceUpdateStatusBar();
void ClearSplitVWB();
void RedrawStatusAreas();
void PreloadGraphics();
void TryDropPlasmaDetonator();
void IN_StartAck();
bool IN_CheckAck();
void MoveDoors();
void MovePWalls();
void ConnectAreas();
void UpdateSoundLoc();


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/


std::uint8_t music_num = 0;

#if LOOK_FOR_DEAD_GUYS
objtype* DeadGuys[MAXACTORS];
std::uint8_t NumDeadGuys;
#endif

bool madenoise; // true when shooting or screaming
std::uint8_t alerted = 0;
std::uint8_t alerted_areanum;


exit_t playstate;

bool DebugOk = false;
std::int16_t InstantWin = 0;
std::int16_t InstantQuit = 0;

std::uint16_t ExtraRadarFlags = 0;


objtype objlist[MAXACTORS];
objtype* new_actor;
objtype* player;
objtype* lastobj;
objtype* objfreelist;
objtype* killerobj;

std::uint16_t farmapylookup[MAPSIZE];
std::uint8_t* nearmapylookup[MAPSIZE];

bool singlestep = false;
bool godmode; // ,noclip;

std::uint8_t tilemap[MAPSIZE][MAPSIZE]; // wall values only !!! Used in saved game.
std::uint8_t spotvis[MAPSIZE][MAPSIZE];
objtype* actorat[MAPSIZE][MAPSIZE];


//
// replacing refresh manager
//
std::uint16_t mapwidth, mapheight, tics, realtics;
bool compatability;
bool usedummy = false;
bool nevermark = false;
std::uint8_t* updateptr;
std::uint16_t mapwidthtable[64];
std::uint16_t uwidthtable[UPDATEHIGH];
std::uint16_t blockstarts[UPDATEWIDE * UPDATEHIGH];
std::uint8_t update[UPDATESIZE];

//
// control info
//
bool mouseenabled;
bool joystickenabled;
bool joypadenabled;
bool joystickprogressive;
std::int16_t joystickport;

ScanCodes dirscan;
ScanCodes buttonscan;
Buttons buttonmouse;
Buttons buttonjoy;

const int viewsize = 20;

bool buttonheld[NUMBUTTONS];

bool demorecord;
bool demoplayback;
char* demoptr;
char* lastdemoptr;
void* demobuffer;

// Light sourcing flag

std::uint8_t lightson;

//
// curent user input
//
int controlx;
int controly; // range from -100 to 100 per tic
#ifdef __vita__
int control2x; //left stick horizontal axis
#endif
bool buttonstate[NUMBUTTONS];
int strafe_value = 0;


void RemoveObj(
	objtype* gone);

void PollControls();
void StopMusic();

void StartMusic(
	bool preload);

void PlayLoop();

void FinishPaletteShifts();
void ShowQuickInstructions();
void CleanDrawPlayBorder();

void PopupAutoMap(
	bool is_shift_pressed);


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/


objtype dummyobj;

//
// LIST OF SONGS FOR EACH LEVEL
//

using Songs = std::vector<int>;

Songs songs;

void initialize_songs()
{
	const auto& assets_info = AssetsInfo{};

	if (!assets_info.is_ps())
	{
		songs = {
			// Episode 1
			INCNRATN_MUS,
			DRKHALLA_MUS,
			JUNGLEA_MUS,
			RACSHUFL_MUS,
			DRKHALLA_MUS,
			HIDINGA_MUS,
			JUNGLEA_MUS,
			RACSHUFL_MUS,
			HIDINGA_MUS,
			DRKHALLA_MUS,
			INCNRATN_MUS,

			// Episode 2
			FREEDOMA_MUS,
			DRKHALLA_MUS,
			STRUTA_MUS,
			INTRIGEA_MUS,
			MEETINGA_MUS,
			DRKHALLA_MUS,
			INCNRATN_MUS,
			RACSHUFL_MUS,
			JUNGLEA_MUS,
			GENEFUNK_MUS,
			THEME_MUS,

			// Episode 3
			LEVELA_MUS,
			HIDINGA_MUS,
			STRUTA_MUS,
			THEME_MUS,
			RACSHUFL_MUS,
			INCNRATN_MUS,
			GOLDA_MUS,
			JUNGLEA_MUS,
			DRKHALLA_MUS,
			THEWAYA_MUS,
			FREEDOMA_MUS,

			// Episode 4
			HIDINGA_MUS,
			DRKHALLA_MUS,
			GENEFUNK_MUS,
			JUNGLEA_MUS,
			INCNRATN_MUS,
			GOLDA_MUS,
			HIDINGA_MUS,
			JUNGLEA_MUS,
			DRKHALLA_MUS,
			THEWAYA_MUS,
			RUMBAA_MUS,

			// Episode 5
			RACSHUFL_MUS,
			SEARCHNA_MUS,
			JUNGLEA_MUS,
			HIDINGA_MUS,
			GENEFUNK_MUS,
			MEETINGA_MUS,
			S2100A_MUS,
			THEME_MUS,
			INCNRATN_MUS,
			DRKHALLA_MUS,
			THEWAYA_MUS,

			// Episode 6
			TIMEA_MUS,
			RACSHUFL_MUS,
			GENEFUNK_MUS,
			HIDINGA_MUS,
			S2100A_MUS,
			THEME_MUS,
			THEWAYA_MUS,
			JUNGLEA_MUS,
			MEETINGA_MUS,
			DRKHALLA_MUS,
			INCNRATN_MUS,
		};
	}
	else
	{
		songs = {
			MAJMIN_MUS,
			STICKS_MUS,
			MOURNING_MUS,
			LURKING_MUS,
			CIRCLES_MUS,
			TIME_MUS,
			TOHELL_MUS,
			FORTRESS_MUS,
			GIVING_MUS,
			HARTBEAT_MUS,
			MOURNING_MUS,
			MAJMIN_MUS,
			VACCINAP_MUS,
			LURKING_MUS,
			MONASTRY_MUS,
			TOMBP_MUS,
			DARKNESS_MUS,
			MOURNING_MUS,
			SERPENT_MUS,
			TIME_MUS,
			CATACOMB_MUS,
			PLOT_MUS,
			GIVING_MUS,
			VACCINAP_MUS,
		};
	}
}


/*
=============================================================================

 USER CONTROL

=============================================================================
*/

const int BASEMOVE = 35;
const int RUNMOVE = 70;
const int BASETURN = 35;
const int RUNTURN = 70;

const int JOYSCALE = 2;


void PollKeyboardButtons()
{
	if (in_use_modern_bindings)
	{
		if (in_is_binding_pressed(e_bi_attack))
		{
			buttonstate[bt_attack] = true;
		}

		if (in_is_binding_pressed(e_bi_strafe))
		{
			buttonstate[bt_strafe] = true;
		}

		if (in_is_binding_pressed(e_bi_run))
		{
			buttonstate[bt_run] = true;
		}

		if (in_is_binding_pressed(e_bi_use))
		{
			buttonstate[bt_use] = true;
		}

		if (in_is_binding_pressed(e_bi_weapon_1))
		{
			buttonstate[bt_ready_autocharge] = true;
		}

		if (in_is_binding_pressed(e_bi_weapon_2))
		{
			buttonstate[bt_ready_pistol] = true;
		}

		if (in_is_binding_pressed(e_bi_weapon_3))
		{
			buttonstate[bt_ready_burst_rifle] = true;
		}

		if (in_is_binding_pressed(e_bi_weapon_4))
		{
			buttonstate[bt_ready_ion_cannon] = true;
		}

		if (in_is_binding_pressed(e_bi_weapon_5))
		{
			buttonstate[bt_ready_grenade] = true;
		}

		const auto& assets_info = AssetsInfo{};

		if (assets_info.is_ps())
		{
			if (in_is_binding_pressed(e_bi_weapon_6))
			{
				buttonstate[bt_ready_bfg_cannon] = true;
			}

			if (in_is_binding_pressed(e_bi_weapon_7))
			{
				buttonstate[bt_ready_plasma_detonators] = true;
			}
		}
	}
	else
	{
		for (int i = 0; i < NUMBUTTONS; ++i)
		{
			if (Keyboard[buttonscan[i]])
			{
				buttonstate[i] = true;
			}
		}
	}
}

void PollMouseButtons()
{
	if (in_use_modern_bindings)
	{
		return;
	}

	std::uint8_t buttons = IN_MouseButtons();

	if ((buttons & 1) != 0)
	{
		buttonstate[buttonmouse[0]] = true;
	}

	if ((buttons & 2) != 0)
	{
		buttonstate[buttonmouse[1]] = true;
	}

	if ((buttons & 4) != 0)
	{
		buttonstate[buttonmouse[2]] = true;
	}
}

void PollJoystickButtons()
{
	std::int16_t buttons;

	buttons = IN_JoyButtons();

	if (joystickport && !joypadenabled)
	{
		if (buttons & 4)
		{
			buttonstate[buttonjoy[0]] = true;
		}
		if (buttons & 8)
		{
			buttonstate[buttonjoy[1]] = true;
		}
	}
	else
	{
		if (buttons & 1)
		{
			buttonstate[buttonjoy[0]] = true;
		}
		if (buttons & 2)
		{
			buttonstate[buttonjoy[1]] = true;
		}
		if (joypadenabled)
		{
			if (buttons & 4)
			{
				buttonstate[buttonjoy[2]] = true;
			}
			if (buttons & 8)
			{
				buttonstate[buttonjoy[3]] = true;
			}
		}
	}
}

void PollKeyboardMove()
{
	bool is_running = in_is_binding_pressed(e_bi_run);

	if (g_always_run)
	{
		is_running = !is_running;
	}

	int value = tics * (is_running ? RUNMOVE : BASEMOVE);

	if (in_is_binding_pressed(e_bi_forward))
	{
		controly -= value;
	}

	if (in_is_binding_pressed(e_bi_backward))
	{
		controly += value;
	}

	if (in_is_binding_pressed(e_bi_left))
	{
		controlx -= value;
	}

	if (in_is_binding_pressed(e_bi_right))
	{
		controlx += value;
	}

	strafe_value = 0;

	if (in_is_binding_pressed(e_bi_strafe))
	{
		if (in_is_binding_pressed(e_bi_left))
		{
			strafe_value = -value;
		}
		else if (in_is_binding_pressed(e_bi_right))
		{
			strafe_value = value;
		}
	}
	else if (in_is_binding_pressed(e_bi_strafe_left))
	{
		strafe_value = -value;
	}
	else if (in_is_binding_pressed(e_bi_strafe_right))
	{
		strafe_value = value;
	}
}

void PollMouseMove()
{
	int mousexmove;
	int mouseymove;

	::in_get_mouse_deltas(mousexmove, mouseymove);
	::in_clear_mouse_deltas();

	auto is_running = ::in_is_binding_pressed(e_bi_run);

	if (g_always_run)
	{
		is_running = !is_running;
	}

	const auto move_scale = 1.0F + (::mouseadjustment / 6.0F);

	auto delta_x = static_cast<float>(mousexmove);
	auto delta_y = static_cast<float>(mouseymove);

	if (is_running)
	{
		delta_x *= 1.5F;
		delta_y *= 5.0F;
	}

	if (::in_use_modern_bindings)
	{
		delta_y = 0.0F;
	}

	delta_x *= move_scale;
	delta_y *= move_scale;

	::controlx += static_cast<int>(delta_x);
	::controly += static_cast<int>(delta_y);
}

void PollJoystickMove()
{
	int joyx;
	int joyy;

	INL_GetJoyDelta(joystickport, &joyx, &joyy);

	if (joystickprogressive)
	{
		if (joyx > 64)
		{
			controlx += (joyx - 64) * JOYSCALE * tics;
		}
		else if (joyx < -64)
		{
			controlx -= (-joyx - 64) * JOYSCALE * tics;
		}
		if (joyy > 64)
		{
			controlx += (joyy - 64) * JOYSCALE * tics;
		}
		else if (joyy < -64)
		{
			controly -= (-joyy - 64) * JOYSCALE * tics;
		}
	}
	else if (buttonstate[bt_run])
	{
		if (joyx > 64)
		{
			controlx += RUNMOVE * tics;
		}
		else if (joyx < -64)
		{
			controlx -= RUNMOVE * tics;
		}
		if (joyy > 64)
		{
			controly += RUNMOVE * tics;
		}
		else if (joyy < -64)
		{
			controly -= RUNMOVE * tics;
		}
	}
	else
	{
		if (joyx > 64)
		{
			controlx += BASEMOVE * tics;
		}
		else if (joyx < -64)
		{
			controlx -= BASEMOVE * tics;
		}
		if (joyy > 64)
		{
			controly += BASEMOVE * tics;
		}
		else if (joyy < -64)
		{
			controly -= BASEMOVE * tics;
		}
	}
}

/*
===================
=
= PollControls
=
= Gets user or demo input, call once each frame
=
= controlx              set between -100 and 100 per tic
= controly
= buttonheld[]  the state of the buttons LAST frame
= buttonstate[] the state of the buttons THIS frame
=
===================
*/
void PollControls()
{
	std::uint8_t buttonbits;

	controlx = 0;
	controly = 0;
	memcpy(buttonheld, buttonstate, sizeof(buttonstate));
	memset(buttonstate, 0, sizeof(buttonstate));

	if (demoplayback)
	{
		//
		// read commands from demo buffer
		//
		buttonbits = *demoptr++;
		for (int i = 0; i < NUMBUTTONS; ++i)
		{
			buttonstate[i] = ((buttonbits & 1) != 0);
			buttonbits >>= 1;
		}

		controlx = *demoptr++;
		controly = *demoptr++;
		tics = *demoptr++;

		while (TimeCount - lasttimecount < tics)
		{
		}
		lasttimecount = TimeCount;

		if (demoptr == lastdemoptr)
		{
			playstate = ex_completed; // demo is done

		}
		controlx *= tics;
		controly *= tics;


		return;
	}

	//
	// get timing info for last frame
	//
	CalcTics();

	// BBi
	::in_handle_events();
	//
	// get button states
	//
	PollKeyboardButtons();

	if (mouseenabled)
	{
		PollMouseButtons();
	}

	if (joystickenabled)
	{
		PollJoystickButtons();
	}

	//
	// get movements
	//
	PollKeyboardMove();

	if (mouseenabled)
	{
		PollMouseMove();
	}

	if (joystickenabled)
	{
		PollJoystickMove();
	}

	//
	// bound movement to a maximum
	//

	const auto max_control = 1000000;
	const auto min_control = -max_control;

	if (controlx > max_control)
	{
		controlx = max_control;
	}
	else if (controlx < min_control)
	{
		controlx = min_control;
	}

	if (controly > max_control)
	{
		controly = max_control;
	}
	else if (controly < min_control)
	{
		controly = min_control;
	}
#ifdef __vita__
	if (control2x > max_control)
	{
		control2x = max_control;
	}
	else if (control2x < min_control)
	{
		control2x = min_control;
	}
#endif

}


extern bool PP_step;
extern bool sqActive;
extern std::int16_t pickquick;

bool refresh_screen;

using JamBuffCmp = std::vector<ScanCode>;

JamBuffCmp jam_buff_cmp = {
	ScanCode::sc_j,
	ScanCode::sc_a,
	ScanCode::sc_m,
}; // jam_buff_cmp

JamBuffCmp jam_buff = {
	ScanCode::sc_none,
	ScanCode::sc_none,
	ScanCode::sc_none,
}; // jam_buff

char PAUSED_MSG[] = "^ST1^CEGame Paused\r^CEPress any key to resume.^XX";


namespace
{


void check_heart_beat_key()
{
	const auto& assets_info = AssetsInfo{};

	if (!assets_info.is_aog())
	{
		return;
	}


	static bool is_key_released;

	if (::in_is_binding_pressed(e_bi_heart_beat))
	{
		if (is_key_released)
		{
			::g_heart_beat_sound = !::g_heart_beat_sound;

			const auto& message = (
				::g_heart_beat_sound ?
				::ekg_heartbeat_enabled :
				::ekg_heartbeat_disabled);

			DISPLAY_TIMED_MSG(message.c_str(), MP_BONUS, MT_GENERAL);
			is_key_released = false;
		}
	}
	else
	{
		is_key_released = true;
	}
}


} // namespace


void CheckKeys()
{
	bool one_eighty = false;
	ScanCode scan;
	static bool Plus_KeyReleased;
	static bool Minus_KeyReleased;
	static bool I_KeyReleased;
	static bool S_KeyReleased;

	if (screenfaded || demoplayback)
	{          // don't do anything with a faded screen
		return;
	}

	scan = LastScan;

	//
	// SECRET CHEAT CODE: 'JAM'
	//

	const auto& assets_info = AssetsInfo{};

	if (!assets_info.is_aog_sw())
	{
		if (Keyboard[ScanCode::sc_j] || Keyboard[ScanCode::sc_a] || Keyboard[ScanCode::sc_m])
		{
			if (jam_buff.back() != LastScan)
			{
				for (auto i = 1; i < 3; ++i)
				{
					jam_buff[i - 1] = jam_buff[i];
				}

				jam_buff.back() = LastScan;
			}
		}
	}

	CheckMusicToggle();

	if (gamestate.rpower)
	{
		if (in_is_binding_pressed(e_bi_radar_magnify))
		{
			if (Plus_KeyReleased && gamestate.rzoom < 2)
			{
				UpdateRadarGuage();
				gamestate.rzoom++;
				Plus_KeyReleased = false;
			}
		}
		else
		{
			Plus_KeyReleased = true;
		}

		if (in_is_binding_pressed(e_bi_radar_minify))
		{
			if (Minus_KeyReleased && gamestate.rzoom)
			{
				UpdateRadarGuage();
				gamestate.rzoom--;
				Minus_KeyReleased = false;
			}
		}
		else
		{
			Minus_KeyReleased = true;
		}
	}

	if (in_is_binding_pressed(e_bi_sfx))
	{
		if (S_KeyReleased)
		{
			bool is_enabled = false;

			if (::sd_is_sound_enabled)
			{
				::SD_WaitSoundDone();
				::SD_EnableSound(false);

				is_enabled = false;
			}
			else
			{
				::ClearMemory();

				if (::sd_has_audio)
				{
					::SD_EnableSound(true);
				}
				else
				{
					::SD_EnableSound(false);
				}

				::CA_LoadAllSounds();

				is_enabled = true;
			}

			DISPLAY_TIMED_MSG(
				is_enabled ? ::SoundOn : ::SoundOff,
				MP_BONUS,
				MT_GENERAL);

			S_KeyReleased = false;
		}
	}
	else
	{
		S_KeyReleased = true;
	}

	if (!assets_info.is_aog_sw() && Keyboard[ScanCode::sc_return])
	{
		std::int8_t loop;

		if (jam_buff == jam_buff_cmp)
		{
			jam_buff[0] = ScanCode::sc_none;

			for (loop = 0; loop < NUMKEYS; loop++)
			{
				if (gamestate.numkeys[static_cast<int>(loop)] < MAXKEYS)
				{
					gamestate.numkeys[static_cast<int>(loop)] = 1;
				}
			}

			gamestate.health = 100;
			gamestate.ammo = MAX_AMMO;
			gamestate.rpower = MAX_RADAR_ENERGY;

			if (!DebugOk)
			{
				gamestate.score = 0;
				gamestate.nextextra = EXTRAPOINTS;
			}

			gamestate.TimeCount += 42000L;

			for (loop = 0; loop < NUMWEAPONS - 1; loop++)
			{
				GiveWeapon(loop);
			}

			DrawWeapon();
			DrawHealth();
			DrawKeys();
			DrawScore();
			DISPLAY_TIMED_MSG("\r\r     YOU CHEATER!", MP_INTERROGATE, MT_GENERAL);
			ForceUpdateStatusBar();

			ClearMemory();
			ClearSplitVWB();

			// BBi
#if 0
			VW_ScreenToScreen(PAGE1START, ::bufferofs, 320, 160);
#endif

			Message("\n NOW you're jammin'!! \n");

			IN_ClearKeysDown();
			IN_Ack();

			CleanDrawPlayBorder();
		}
		else if (!in_use_modern_bindings)
		{
			one_eighty = true;
		}
	}

	// Handle quick turning!
	//
	if (!gamestate.turn_around)
	{
		// 90 degrees left
		//
		if (in_is_binding_pressed(e_bi_quick_left))
		{
			gamestate.turn_around = -90;
			gamestate.turn_angle = player->angle + 90;
			if (gamestate.turn_angle > 359)
			{
				gamestate.turn_angle -= ANGLES;
			}
		}

		// 180 degrees right
		//
		if (in_is_binding_pressed(e_bi_turn_around) || one_eighty)
		{
			gamestate.turn_around = 180;
			gamestate.turn_angle = player->angle + 180;
			if (gamestate.turn_angle > 359)
			{
				gamestate.turn_angle -= ANGLES;
			}
		}

		// 90 degrees right
		//
		if (in_is_binding_pressed(e_bi_quick_right))
		{
			gamestate.turn_around = 90;
			gamestate.turn_angle = player->angle - 90;
			if (gamestate.turn_angle < 0)
			{
				gamestate.turn_angle += ANGLES;
			}
		}
	}

	//
	// pause key weirdness can't be checked as a scan code
	//
	if (in_is_binding_pressed(e_bi_pause))
	{
		SD_MusicOff();
		fontnumber = 4;
		BMAmsg(PAUSED_MSG);
		IN_Ack();
		IN_ClearKeysDown();
		fontnumber = 2;
		RedrawStatusAreas();
		SD_MusicOn();
		Paused = false;
		::in_clear_mouse_deltas();
		return;
	}

	scan = ScanCode::sc_none;

	if (Keyboard[ScanCode::sc_escape])
	{
		scan = ScanCode::sc_escape;
	}
	else if (in_is_binding_pressed(e_bi_help))
	{
		scan = ScanCode::sc_f1;
	}
	if (in_is_binding_pressed(e_bi_save))
	{
		scan = ScanCode::sc_f2;
	}
	else if (in_is_binding_pressed(e_bi_load))
	{
		scan = ScanCode::sc_f3;
	}
	else if (in_is_binding_pressed(e_bi_sound))
	{
		scan = ScanCode::sc_f4;
	}
	else if (in_is_binding_pressed(e_bi_controls))
	{
		scan = ScanCode::sc_f6;
	}
	else if (in_is_binding_pressed(e_bi_end_game))
	{
		scan = ScanCode::sc_f7;
	}
	else if (in_is_binding_pressed(e_bi_quick_save))
	{
		scan = ScanCode::sc_f8;
	}
	else if (in_is_binding_pressed(e_bi_quick_load))
	{
		scan = ScanCode::sc_f9;
	}
	else if (in_is_binding_pressed(e_bi_quick_exit))
	{
		scan = ScanCode::sc_f10;
	}

	switch (scan)
	{
	case ScanCode::sc_f7: // END GAME
	case ScanCode::sc_f10: // QUIT TO DOS
		::vid_is_hud = true;
		FinishPaletteShifts();
		ClearMemory();
		US_ControlPanel(scan);
		CleanDrawPlayBorder();
		::vid_is_hud = false;
		return;

	case ScanCode::sc_f2: // SAVE MISSION
	case ScanCode::sc_f8: // QUICK SAVE
		// Make sure there's room to save...
		//
		::vid_is_hud = true;
		ClearMemory();
		FinishPaletteShifts();
		::vid_is_hud = false;

		if (!CheckDiskSpace(DISK_SPACE_NEEDED, CANT_SAVE_GAME_TXT, cds_id_print))
		{
			CleanDrawPlayBorder();
			break;
		}

	case ScanCode::sc_f1: // HELP
	case ScanCode::sc_f3: // LOAD MISSION
	case ScanCode::sc_f4: // SOUND MENU
	case ScanCode::sc_f5: //      RESIZE VIEW
	case ScanCode::sc_f6: // CONTROLS MENU
	case ScanCode::sc_f9: // QUICK LOAD
	case ScanCode::sc_escape: // MAIN MENU
	{
		refresh_screen = true;
		if (scan < ScanCode::sc_f8)
		{
			::vid_is_hud = true;
			VW_FadeOut();
			::vid_is_hud = false;
		}
		StopMusic();
		ClearMemory();
		ClearSplitVWB();
		US_ControlPanel(scan);
		if (refresh_screen)
		{
			const auto old_loadedgame = loadedgame;
			loadedgame = false;
			DrawPlayScreen(false);
			loadedgame = old_loadedgame;
		}
		ClearMemory();
		if (!sqActive || !loadedgame)
		{
			StartMusic(false);
		}
		IN_ClearKeysDown();
		if (loadedgame)
		{
			PreloadGraphics();
			loadedgame = false;
			DrawPlayScreen(false);
		}
		else if (!refresh_screen)
		{
			CleanDrawPlayBorder();
		}
		if (!sqActive)
		{
			StartMusic(false);
		}
		return;
	}

	default:
		break;
	}

	scan = ScanCode::sc_none;

	if (in_is_binding_pressed(e_bi_stats))
	{
		PopupAutoMap(Keyboard[ScanCode::sc_left_shift] || Keyboard[ScanCode::sc_right_shift]);
	}

	if (Keyboard[ScanCode::sc_back_quote])
	{
		Keyboard[ScanCode::sc_back_quote] = 0;

		if (assets_info.is_ps())
		{
			::TryDropPlasmaDetonator();
		}
	}


	if ((DebugOk || gamestate.flags & GS_MUSIC_TEST) && (Keyboard[ScanCode::sc_backspace]))
	{
		std::uint8_t old_num = music_num;

		if (gamestate.flags & GS_MUSIC_TEST)
		{
			if (Keyboard[ScanCode::sc_left_arrow])
			{
				if (music_num)
				{
					music_num--;
				}
				Keyboard[ScanCode::sc_left_arrow] = false;
			}
			else if (Keyboard[ScanCode::sc_right_arrow])
			{
				if (music_num < LASTMUSIC - 1)
				{
					music_num++;
				}
				Keyboard[ScanCode::sc_right_arrow] = false;
			}

			if (old_num != music_num)
			{
				ClearMemory();

				delete[] audiosegs[STARTMUSIC + old_num];
				audiosegs[STARTMUSIC + old_num] = nullptr;

				StartMusic(false);
				DrawScore();
			}
		}

		if (old_num == music_num)
		{
			fontnumber = 4;
			SETFONTCOLOR(0, 15);

			::vid_is_hud = true;

			if (DebugKeys())
			{
				CleanDrawPlayBorder();
			}

			::vid_is_hud = false;

			::in_clear_mouse_deltas();

			lasttimecount = TimeCount;
			return;
		}
	}

	if (in_is_binding_pressed(e_bi_attack_info))
	{
		if (I_KeyReleased)
		{
			gamestate.flags ^= GS_ATTACK_INFOAREA;
			if (gamestate.flags & GS_ATTACK_INFOAREA)
			{
				DISPLAY_TIMED_MSG(attacker_info_enabled, MP_ATTACK_INFO, MT_GENERAL);
			}
			else
			{
				DISPLAY_TIMED_MSG(attacker_info_disabled, MP_ATTACK_INFO, MT_GENERAL);
			}
			I_KeyReleased = false;
		}
	}
	else
	{
		I_KeyReleased = true;
	}

	if (in_is_binding_pressed(e_bi_ceiling))
	{
		gamestate.flags ^= GS_DRAW_CEILING;
		in_reset_binding_state(e_bi_ceiling);
	}

	if (in_is_binding_pressed(e_bi_flooring))
	{
		ThreeDRefresh();
		ThreeDRefresh();

		gamestate.flags ^= GS_DRAW_FLOOR;

		in_reset_binding_state(e_bi_flooring);
	}

	if (in_is_binding_pressed(e_bi_lightning))
	{
		in_reset_binding_state(e_bi_lightning);
		gamestate.flags ^= GS_LIGHTING;
	}

	check_heart_beat_key();
}


// -------------------------------------------------------------------------
// CheckMusicToggle()
// -------------------------------------------------------------------------
void CheckMusicToggle()
{
	static bool M_KeyReleased;

	if (in_is_binding_pressed(e_bi_music))
	{
		bool toggle = M_KeyReleased;

		const auto& assets_info = AssetsInfo{};

		if (!assets_info.is_aog_sw())
		{
			if (jam_buff[0] == ScanCode::sc_j &&
				jam_buff[1] == ScanCode::sc_a &&
				jam_buff[2] == ScanCode::sc_m)
			{
				toggle = false;
			}
		}

		if (toggle)
		{
			bool is_enabled = false;

			if (!::sd_has_audio)
			{
				DISPLAY_TIMED_MSG(NoAdLibCard, MP_BONUS, MT_GENERAL);

				::sd_play_player_sound(NOWAYSND, bstone::ActorChannel::no_way);
				return;
			}
			else if (::sd_is_music_enabled)
			{
				::SD_EnableMusic(false);
				is_enabled = false;
			}
			else
			{
				::SD_EnableMusic(true);
				StartMusic(false);
				is_enabled = true;
			}

			DISPLAY_TIMED_MSG(
				is_enabled ? MusicOn : MusicOff,
				MP_BONUS,
				MT_GENERAL);

			M_KeyReleased = false;
		}
	}
	else
	{
		M_KeyReleased = true;
	}
}


char Computing[] = {"Computing..."};


void PopupAutoMap(
	bool is_shift_pressed)
{
	::vid_is_hud = true;

	const auto& assets_info = AssetsInfo{};

	const std::int16_t BASE_X = (assets_info.is_ps() ? 64 : 40);
	const std::int16_t BASE_Y = 44;

	ThreeDRefresh();
	ThreeDRefresh();

	SD_StopSound();
	ClearMemory();
	CacheDrawPic(BASE_X, BASE_Y, AUTOMAPPIC);

	if (assets_info.is_aog())
	{
		bool show_whole_map = true;
		int overlay_flags = OV_KEYS;

		if (is_shift_pressed)
		{
			show_whole_map = !show_whole_map;
		}

		if (g_rotated_automap)
		{
			show_whole_map = !show_whole_map;
		}

		if (show_whole_map)
		{
			overlay_flags |= OV_WHOLE_MAP;
		}

		::ShowOverhead(BASE_X + 4, BASE_Y + 4, 32, 0, overlay_flags);

		ShowStats(BASE_X + 157, BASE_Y + 25, ss_quick, &gamestuff.level[gamestate.mapon].stats);
	}
	else
	{
		ShowStats(BASE_X + 101, BASE_Y + 22, ss_quick, &gamestuff.level[gamestate.mapon].stats);
	}

	while (Keyboard[ScanCode::sc_back_quote])
	{
		CalcTics();

		if (!assets_info.is_ps())
		{
			::CycleColors();
			::in_handle_events();
		}
	}

	IN_StartAck();
	while (!IN_CheckAck())
	{
		CalcTics();

		if (!assets_info.is_ps())
		{
			::CycleColors();
			::in_handle_events();
		}
	}

	CleanDrawPlayBorder();
	IN_ClearKeysDown();

	::vid_is_hud = false;
}


/*
#############################################################################

								  The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/


int objcount;

/*
=========================
=
= InitActorList
=
= Call to clear out the actor object lists returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/
void InitActorList()
{
	std::int16_t i;

	//
	// init the actor lists
	//
#if LOOK_FOR_DEAD_GUYS
	NumDeadGuys = 0;
	memset(DeadGuys, 0, sizeof(DeadGuys));
#endif

	memset(statobjlist, 0, sizeof(statobjlist));
	for (i = 0; i < MAXACTORS; i++)
	{
		objlist[i].prev = &objlist[i + 1];
		objlist[i].next = nullptr;
	}

	objlist[MAXACTORS - 1].prev = nullptr;

	objfreelist = &objlist[0];
	lastobj = nullptr;

	objcount = 0;

	//
	// give the player the first free spots
	//
	GetNewActor();
	player = new_actor;
}

/*
=========================
=
= GetNewActor
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out ot
= return a dummy object pointer that will never get used
=
=========================
*/
void GetNewActor()
{
	if (objcount >= MAXACTORS - 1)
	{
		objtype* obj = player->next;

		while (obj)
		{
			if ((obj->flags & (FL_DEADGUY | FL_VISABLE)) == FL_DEADGUY)
			{
				RemoveObj(obj);
				obj = nullptr;
			}
			else
			{
				obj = obj->next;
			}
		}
	}

	if (!objfreelist)
	{
		if (usedummy)
		{
			new_actor = &dummyobj;
			memset(new_actor, 0, sizeof(*new_actor));
		}
		else
		{
			::Quit("No free spots in objlist.");
		}
	}
	else
	{
		new_actor = objfreelist;
		objfreelist = new_actor->prev;

		memset(new_actor, 0, sizeof(*new_actor));

		if (lastobj)
		{
			lastobj->next = new_actor;
		}

		new_actor->prev = lastobj; // new_actor->next is allready nullptr from memset

		lastobj = new_actor;

		objcount++;
	}
}

/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/
void RemoveObj(
	objtype* gone)
{
	if (gone == &dummyobj)
	{
		return;
	}

	if (gone == player)
	{
		::Quit("Tried to remove the player.");
	}

	gone->state = nullptr;

	//
	// fix the next object's back link
	//
	if (gone == lastobj)
	{
		lastobj = (objtype*)gone->prev;
	}
	else
	{
		gone->next->prev = gone->prev;
	}

	//
	// fix the previous object's forward link
	//
	gone->prev->next = gone->next;

	//
	// add it back in to the free list
	//
	gone->prev = objfreelist;
	objfreelist = gone;

	objcount--;
}

void StopMusic()
{
	SD_MusicOff();
}

// -------------------------------------------------------------------------
// StartMusic()
//              o preload = true, music is cached but not started
// -------------------------------------------------------------------------
void StartMusic(
	bool preload)
{
	int musicchunk;

	SD_MusicOff();

	const auto& assets_info = AssetsInfo{};

	if (!assets_info.is_ps())
	{
		const auto level_count = assets_info.get_stats_levels_per_episode();
		const auto level_number = ::gamestate.mapon % level_count;

		musicchunk = ::songs[level_number + (::gamestate.episode * level_count)];
	}
	else
	{
		if (::playstate == ex_victorious)
		{
			musicchunk = FORTRESS_MUS;
		}
		else
		{
			const auto level_count = assets_info.get_levels_per_episode();
			const auto map_number = ::gamestate.mapon % level_count;

			musicchunk = ::songs[map_number];
		}
	}

	if (!audiosegs[STARTMUSIC + musicchunk])
	{
		CA_CacheAudioChunk(static_cast<std::int16_t>(STARTMUSIC + musicchunk));
	}

	{
		if (!preload)
		{
			::SD_StartMusic(musicchunk);
		}
	}
}


const int NUMREDSHIFTS = 6;
const int REDSTEPS = 8;

const int NUMWHITESHIFTS = 3;
const int WHITESTEPS = 20;
const int WHITETICS = 6;


std::uint8_t redshifts[NUMREDSHIFTS][768];
std::uint8_t whiteshifts[NUMREDSHIFTS][768];

int damagecount;
int bonuscount;
bool palshifted;


void InitRedShifts()
{
	//
	// fade through intermediate frames
	//
	for (int i = 1; i <= NUMREDSHIFTS; i++)
	{
		std::uint8_t* workptr = &redshifts[i - 1][0];
		const std::uint8_t* baseptr = vgapal;

		for (int j = 0; j <= 255; ++j)
		{
			int delta = 64 - baseptr[0];

			workptr[0] = static_cast<std::uint8_t>(
				baseptr[0] + ((delta * i) / REDSTEPS));

			delta = -baseptr[1];

			workptr[1] = static_cast<std::uint8_t>(
				baseptr[1] + ((delta * i) / REDSTEPS));

			delta = -baseptr[2];

			workptr[2] = static_cast<std::uint8_t>(
				baseptr[2] + ((delta * i) / REDSTEPS));

			baseptr += 3;
			workptr += 3;
		}
	}

	for (int i = 1; i <= NUMWHITESHIFTS; i++)
	{
		std::uint8_t* workptr = &whiteshifts[i - 1][0];
		const std::uint8_t* baseptr = vgapal;

		for (int j = 0; j <= 255; ++j)
		{
			int delta = 64 - baseptr[0];

			workptr[0] = static_cast<std::uint8_t>(
				baseptr[0] + ((delta * i) / WHITESTEPS));

			delta = 62 - baseptr[1];

			workptr[1] = static_cast<std::uint8_t>(
				baseptr[1] + ((delta * i) / WHITESTEPS));

			delta = 0 - baseptr[2];

			workptr[2] = static_cast<std::uint8_t>(
				baseptr[2] + ((delta * i) / WHITESTEPS));

			baseptr += 3;
			workptr += 3;
		}
	}
}

void ClearPaletteShifts()
{
	bonuscount = 0;
	damagecount = 0;
}

void StartBonusFlash()
{
	// white shift palette
	bonuscount = NUMWHITESHIFTS * WHITETICS;
}

void StartDamageFlash(
	int damage)
{
	damagecount += damage;
}

void UpdatePaletteShifts()
{
	int red = 0;
	int white = 0;

	if (bonuscount > 0)
	{
		white = (bonuscount / WHITETICS) + 1;

		if (white > NUMWHITESHIFTS)
		{
			white = NUMWHITESHIFTS;
		}

		bonuscount -= tics;

		if (bonuscount < 0)
		{
			bonuscount = 0;
		}
	}
	else
	{
		white = 0;
	}

	if (damagecount > 0)
	{
		red = (damagecount / 10) + 1;

		if (red > NUMREDSHIFTS)
		{
			red = NUMREDSHIFTS;
		}

		damagecount -= tics;

		if (damagecount < 0)
		{
			damagecount = 0;
		}
	}
	else
	{
		red = 0;
	}

	if (red > 0)
	{
		// BBi
#if 0
		VW_WaitVBL(1);
#endif
		VL_SetPalette(0, 256, redshifts[red - 1]);
		palshifted = true;
	}
	else if (white > 0)
	{
		// BBi
#if 0
		VW_WaitVBL(1);
#endif
		VL_SetPalette(0, 256, whiteshifts[white - 1]);
		palshifted = true;
	}
	else if (palshifted)
	{
		// BBi
#if 0
		VW_WaitVBL(1);
#endif
		VL_SetPalette(0, 256, vgapal); // back to normal
		palshifted = false;
	}
}

void FinishPaletteShifts()
{
	if (palshifted)
	{
		palshifted = false;
		// BBi
#if 0
		VW_WaitVBL(1);
#endif
		VL_SetPalette(0, 256, vgapal);
		::VL_RefreshScreen();
	}
}

void DoActor(
	objtype* ob)
{
	void(*think)(
		objtype*);

	objtype* actor;


	if (ob->flags & FL_FREEZE)
	{
		return;
	}

	if (ob->flags & FL_BARRIER)
	{
		actor = actorat[ob->tilex][ob->tiley];
		if (BARRIER_STATE(ob) == bt_ON)
		{
			if (actor)
			{
				std::int16_t damage = 0;

				actor->flags |= FL_BARRIER_DAMAGE;
				if ((US_RndT() < 0x7f) && (actor->flags & FL_SHOOTABLE))
				{
					switch (ob->obclass)
					{
					case arc_barrierobj: // arc barrier - Mild Damage
						damage = 500; // 100
						break;

					case post_barrierobj: // post barrier - Butt kicker
						damage = 500;
						break;

					default:
						break;
					}
					DamageActor(actor, damage, ob);
				}
			}
		}
		else if (actor)
		{
			actor->flags &= ~FL_BARRIER_DAMAGE;
		}
	}

	if (!ob->active && !areabyplayer[ob->areanumber])
	{
		return;
	}

	if (!(ob->flags & (FL_NONMARK | FL_NEVERMARK)))
	{
		actorat[ob->tilex][ob->tiley] = nullptr;
	}

	//
	// non transitional object
	//

	if (!ob->ticcount)
	{
		think = ob->state->think;
		if (think)
		{
			think(ob);
			if (!ob->state)
			{
				RemoveObj(ob);
				return;
			}
		}

		if (!(ob->flags & FL_NEVERMARK))
		{
			if (ob->flags & FL_NONMARK)
			{
				if (actorat[ob->tilex][ob->tiley])
				{
					return;
				}
			}
			actorat[ob->tilex][ob->tiley] = ob;
		}
		return;
	}

	//
	// transitional object
	//
	ob->ticcount -= tics;
	while (ob->ticcount <= 0)
	{
		think = ob->state->action; // end of state action
		if (think)
		{
			think(ob);
			if (!ob->state)
			{
				RemoveObj(ob);
				return;
			}
		}

		ob->state = ob->state->next;

		if (!ob->state)
		{
			RemoveObj(ob);
			return;
		}

		if (!ob->state->tictime)
		{
			ob->ticcount = 0;
			goto think;
		}

		ob->ticcount = static_cast<std::int16_t>(ob->ticcount + ob->state->tictime);
	}

think:
	//
	// think
	//
	think = ob->state->think;
	if (think)
	{
		think(ob);
		if (!ob->state)
		{
			RemoveObj(ob);
			return;
		}
	}

	if (!(ob->flags & FL_NEVERMARK))
	{
		if (ob->flags & FL_NONMARK)
		{
			if (actorat[ob->tilex][ob->tiley])
			{
				return;
			}
		}
		actorat[ob->tilex][ob->tiley] = ob;
	}
	return;
}


extern bool ShowQuickMsg;


void PlayLoop()
{
	bool reset_areas = false;
	objtype* obj;

	lasttimecount = 0;
	TimeCount = 0;
	playstate = ex_stillplaying;

	framecount = frameon = 0;
	pwallstate = anglefrac = 0;
	memset(buttonstate, 0, sizeof(buttonstate));
	ClearPaletteShifts();
	ForceUpdateStatusBar();

	::in_clear_mouse_deltas();

	tics = 1; // for first time through
	if (demoplayback)
	{
		IN_StartAck();
	}

	do
	{
		PollControls();

		//
		// actor thinking
		//
		madenoise = false;

		if (alerted)
		{
			alerted--;
		}

		MoveDoors();
		MovePWalls();

		for (obj = player; obj; obj = obj->next)
		{
			if ((obj != player) && (Keyboard[ScanCode::sc_6] || Keyboard[ScanCode::sc_7]) && Keyboard[ScanCode::sc_8] && DebugOk)
			{
				if (!reset_areas)
				{
					memset(areabyplayer, 1, sizeof(areabyplayer));
				}
				reset_areas = true;

				if ((((!(obj->flags & FL_INFORMANT)) && (obj->flags & FL_SHOOTABLE))) ||
					(obj->obclass == liquidobj && !(obj->flags & FL_DEADGUY)))
				{
					DamageActor(obj, 1000, player);
				}
			}
			else if (reset_areas)
			{
				ConnectAreas();
				reset_areas = false;
			}
			DoActor(obj);
		}

		if (NumEAWalls)
		{
			CheckSpawnEA();
		}

		if ((!GoldsternInfo.GoldSpawned) && GoldsternInfo.SpawnCnt)
		{
			CheckSpawnGoldstern();
		}

		::vid_is_hud = true;
		UpdatePaletteShifts();

		ThreeDRefresh();

		gamestate.TimeCount += tics;

		UpdateSoundLoc();               // JAB

		if (screenfaded & !playstate)
		{
			VW_FadeIn();
		}

		// Display first-time instructions.
		//
		if (ShowQuickMsg)
		{
			ShowQuickInstructions();
		}

		::vid_is_hud = false;

		CheckKeys();

		if (demoplayback && demoptr == lastdemoptr)
		{
			playstate = ex_title;
		}

		//
		// debug aids
		//
		if (singlestep)
		{
			VW_WaitVBL(14);
			lasttimecount = TimeCount;
		}

		if ((demoplayback) && (IN_CheckAck()))
		{
			IN_ClearKeysDown();
			playstate = ex_abort;
		}


	} while (!playstate && !startgame);

	if (playstate != ex_died)
	{
		FinishPaletteShifts();
	}

	gamestate.flags &= ~GS_VIRGIN_LEVEL;
}

void ShowQuickInstructions()
{
	::ShowQuickMsg = false;

	const auto& assets_info = AssetsInfo{};

	if (::demoplayback ||
		(assets_info.is_ps() && (::gamestate.mapon > 0)) ||
		(::gamestate.flags & GS_QUICKRUN) != 0)
	{
		return;
	}

	::ThreeDRefresh();
	::ThreeDRefresh();
	::ClearMemory();
	::WindowX = 0;
	::WindowY = 16;
	::WindowW = 320;
	::WindowH = 168;
	::CacheMessage(QUICK_INFO1_TEXT);

	if (!::IN_UserInput(120))
	{
		::CacheMessage(QUICK_INFO2_TEXT);
		::IN_Ack();
	}

	::IN_ClearKeysDown();
	::CleanDrawPlayBorder();
}

void CleanDrawPlayBorder()
{
	DrawPlayBorder();
	ThreeDRefresh();
	DrawPlayBorder();
	ThreeDRefresh();
	DrawPlayBorder();
}
