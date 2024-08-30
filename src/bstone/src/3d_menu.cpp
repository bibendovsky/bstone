/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cmath>
#include <cstring>

#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"
#include "jm_cio.h"
#include "jm_lzh.h"
#include "jm_tp.h"

#include "bstone_exception.h"
#include "bstone_saved_game.h"
#include "bstone_scope_exit.h"
#include "bstone_r3r_limits.h"


#define GAME_DESCRIPTION_LEN (31)

// Box behind text for cursor
#define HIGHLIGHT_BOX_COLOR (0x52)

#define HIGHLIGHT_DEACTIAVED_COLOR (0x55)

#define DISABLED_TEXT_COLOR (0x53)
#define DEACTIAVED_TEXT_COLOR (0x52)

#define INSTRUCTIONS_TEXT_COLOR (0x53)

#define TERM_SHADOW_COLOR (0x01)

//
// Clearable Menu Terminal Area
//

#define SCREEN_X (32)
#define SCREEN_Y (28)
#define SCREEN_W (244)
#define SCREEN_H (132)

#define BORD2COLOR (0x74)
#define DEACTIVE (0x72)
#define BKGDCOLOR (0x76)


#define READCOLOR (0x4A)
#define READHCOLOR (0x47)
#define TEXTCOLOR (WHITE)


#define SENSITIVE (60)
#define CENTER (SENSITIVE * 2)

#define MENU_X (111)
#define MENU_Y (50)

#define SM_X (121)
#define SM_Y (84)

#define CTL_X (100)
#define CTL_Y (70)

#define LSM_X (85)
#define LSM_Y (55)
#define LSM_W (144)

#define NM_X (71)
#define NM_Y (66)

#define NE_X (58)
#define NE_Y (54)

#define CST_X (77)
#define CST_Y (60)

#define CST_START (77)
#define CST_SPC (41)


// CP_Switch() menu labels
enum sw_labels
{
	SW_LIGHTING,
	SW_REBA_ATTACK_INFO,
	SW_CEILING,
	SW_FLOORS,

	// BBi
	SW_NO_WALL_HIT_SOUND,
	SW_ALWAYS_RUN,
	SW_QUIT_ON_ESCAPE,
	SW_HEART_BEAT_SOUND,
	SW_ROTATED_AUTOMAP,
}; // sw_labels

// BBi
enum sw2_labels
{
	SW2_NO_INTRO_OUTRO,
	SW2_NO_FADE_IN_OR_OUT,
	SW2_NO_WEAPON_BOBBING,
	SW2_VANILLA_FIZZLE_FX,
	SW2_PS_MAP_IN_STATS,
}; // sw2_labels

enum MenuVideoLables
{
	mvl_mode,
	mvl_texturing,
	mvl_widescreen,
	mvl_stretch_ui,
	mvl_filler_color,
};
// BBi


void draw_carousel(
	const int item_index,
	CP_iteminfo* item_i,
	CP_itemtype* items,
	const std::string& text);


namespace
{

std::int16_t COAL_FONT()
{
	return STARTFONT + 4;
}

} // namespace


bool is_full_menu_active = false;


//
// VARIABLES
//
extern std::int16_t SaveGamesAvail[10];
extern std::int16_t StartGame;
extern std::int16_t SoundStatus;


void CA_CacheScreen(
	std::int16_t chunk);

void DrawHighScores();
void ClearMemory();

void DrawTopInfo(
	sp_type type);

void PreloadUpdate(
	std::uint16_t current,
	std::uint16_t total);

bool LoadTheGame(
	const std::string& file_name);

bool IN_CheckAck();

void MenuFadeIn();

void MenuFadeOut();

void SetupControlPanel();

void CleanupControlPanel();

void ControlPanelFree();

void ControlPanelAlloc();

void DrawMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items);

std::int16_t HandleMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	void (*routine)(std::int16_t w));

void DrawWindow(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t wcolor);

void DrawOutline(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t color1,
	std::int16_t color2);

void WaitKeyUp();

void TicDelay(
	std::int16_t count);

void CheckPause();

void ShootSnd();

void DrawGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t* y,
	std::int16_t which,
	std::int16_t basey,
	void(*routine)(std::int16_t w));

void EraseGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t y,
	std::int16_t which);

void SetTextColor(
	CP_itemtype* items,
	std::int16_t hlight);

void DrawMenuGun(
	CP_iteminfo* iteminfo);

void DrawMainMenu();
void DrawSoundMenu();

void DrawLoadSaveScreen(
	std::int16_t loadsave);

void DrawNewEpisode();

void DrawNewGame();

void DrawMouseSens();

void DrawCtlScreen();

void DrawLSAction(
	std::int16_t which);

void PrintLSEntry(
	std::int16_t w,
	std::int16_t color);

void TrackWhichGame(
	std::int16_t w);

void DrawNewGameDiff(
	std::int16_t w);

void CP_BlakeStoneSaga(
	std::int16_t temp1);

void CP_NewGame(
	std::int16_t temp1);

void CP_Sound(
	std::int16_t temp1);

std::int16_t CP_LoadGame(
	std::int16_t quick);

std::int16_t CP_SaveGame(
	std::int16_t quick);

void CP_Control(
	std::int16_t temp1);

void CP_ExitOptions(
	std::int16_t temp1);

void CP_Quit();

std::int16_t CP_EndGame();

bool CP_CheckQuick(
	ScanCode scancode);

void CustomControls(
	std::int16_t temp1);

void MouseSensitivity(
	std::int16_t temp1);

//
// End game message
//

char EndGameStr[] = {"    End current game?\n"
" Are you sure (Y or N)?"};


#define ENDGAMESTR (EndGameStr)

char QuitToDosStr[] = {"      Quit to DOS?\n"
" Are you sure (Y or N)?"};

#define FREEFONT(fontnum) { if (fontnum != STARTFONT + 2 && !grsegs[fontnum].empty()) { UNCACHEGRCHUNK(fontnum); } }


static const char* const CURGAME =
"   Continuing past this\n"
"  point will end the game\n"
" you're currently playing.\n"
"\n"
" Start a NEW game? (Y/N)";

static const char* const GAMESVD =
"There's already a game\n"
"saved at this position.\n"
"\n"
"    Overwrite? (Y/N)";


bool EscPressed = false;

std::int16_t lastmenumusic;

std::int16_t MENUSONG = 0;
std::int16_t ROSTER_MUS = 0;
std::int16_t TEXTSONG = 0;


// ===========================================================================
//
// PRIVATE PROTOTYPES
//
// ===========================================================================

void CP_ReadThis(
	std::int16_t temp1);

void CP_OrderingInfo(
	std::int16_t temp1);

void DrawEpisodePic(
	std::int16_t w);

void DrawAllSoundLights(
	std::int16_t which);

void ReadGameNames();
void FreeMusic();

void CP_GameOptions(
	std::int16_t temp1);

void DrawGopMenu();
void ExitGame();

void CP_Switches(
	std::int16_t temp1);

void cp_switches2(
	std::int16_t);

void DrawSwitchMenu();

void DrawAllSwitchLights(
	std::int16_t which);

void DrawSwitchDescription(
	std::int16_t which);

// BBi
void cp_sound_volume(
	std::int16_t);

void cp_video(
	std::int16_t);
// BBi


extern bool refresh_screen;


// ===========================================================================
//
// LOCAL DATA...
//
// ===========================================================================

CP_iteminfo MainItems = {MENU_X, MENU_Y, 12, MM_NEW_MISSION, 0, 9, {77, 1, 154, 9, 1}};
CP_iteminfo GopItems = {MENU_X, MENU_Y + 25, 6, 0, 0, 9, {77, 1, 154, 9, 1}};
CP_iteminfo SndItems = {SM_X, SM_Y, 6, 0, 0, 8, {87, -1, 144, 7, 1}};
CP_iteminfo LSItems = {LSM_X, LSM_Y, 10, 0, 0, 8, {86, -1, 144, 8, 1}};
CP_iteminfo CtlItems = {CTL_X, CTL_Y, 3, -1, 0, 9, {87, 1, 174, 9, 1}};
CP_iteminfo CusItems = {CST_X, CST_Y + 7, 6, -1, 0, 15, {54, -1, 203, 7, 1}};
CP_iteminfo NewEitems = {NE_X, NE_Y, 6, 0, 0, 16, {43, -2, 119, 16, 1}};
CP_iteminfo NewItems = {NM_X, NM_Y, 4, 1, 0, 16, {60, -2, 105, 16, 1}};
CP_iteminfo SwitchItems = {MENU_X, 0, 0, 0, 0, 9, {87, -1, 132, 7, 1}};

// BBi
CP_iteminfo video_items = {MENU_X, MENU_Y + 30, 5, 0, 0, 9, {77, -1, 154, 7, 1}};
CP_iteminfo video_mode_items = {MENU_X, MENU_Y + 10, 7, 0, 0, 9, {77, -1, 154, 7, 1}};
CP_iteminfo texturing_items = {MENU_X, MENU_Y + 10, 7, 0, 0, 9, {77, -1, 154, 7, 1}};
CP_iteminfo switches2_items = {MENU_X, MENU_Y + 30, 5, 0, 0, 9, {87, -1, 132, 7, 1}};
// BBi


CP_itemtype MainMenu[] = {
	{AT_ENABLED, "NEW MISSION", CP_NewGame, static_cast<std::uint8_t>(COAL_FONT())},
	{AT_READIT, "ORDERING INFO", CP_OrderingInfo},
	{AT_READIT, "INSTRUCTIONS", CP_ReadThis},
	{AT_ENABLED, "STORY", CP_BlakeStoneSaga},
	{AT_DISABLED, "", nullptr},
	{AT_ENABLED, "GAME OPTIONS", CP_GameOptions},
	{AT_ENABLED, "HIGH SCORES", CP_ViewScores},
	{AT_ENABLED, "LOAD MISSION", [](std::int16_t arg) { CP_LoadGame(arg); }},
	{AT_DISABLED, "SAVE MISSION", [](std::int16_t arg) { CP_SaveGame(arg); }},
	{AT_DISABLED, "", nullptr},
	{AT_ENABLED, "BACK TO DEMO", CP_ExitOptions},
	{AT_ENABLED, "LOGOFF", nullptr}
};

CP_itemtype GopMenu[] = {
	// BBi
	{AT_ENABLED, "VIDEO", cp_video},
	// BBi

	{AT_ENABLED, "SOUND", CP_Sound},

	// BBi
	{AT_ENABLED, "SOUND VOLUME", cp_sound_volume},
	// BBi

	{AT_ENABLED, "CONTROLS", CP_Control},
	{AT_ENABLED, "SWITCHES", CP_Switches},

	// BBi
	{AT_ENABLED, "SWITCHES2", cp_switches2},
	// BBi
};

CP_itemtype SndMenu[] =
{
	{AT_ENABLED, "SOUND EFFECTS", 0},
	{AT_ENABLED, "SFX TYPE", 0},
	{AT_ENABLED, "DIGITIZED SFX", 0},
	{AT_ENABLED, "BACKGROUND MUSIC", 0},
	{AT_ENABLED, "DRIVER", 0},
	{AT_ENABLED, "OPL3 TYPE", 0},
};

CP_itemtype CtlMenu[] = {
	{AT_DISABLED, "MOUSE ENABLED", 0},
	{AT_DISABLED, "MOUSE SENSITIVITY", MouseSensitivity},
	{AT_ENABLED, "CUSTOMIZE CONTROLS", CustomControls}
};

CP_itemtype SwitchMenu[] = {
	{AT_ENABLED, "LIGHTING", 0},
	{AT_ENABLED, "REBA ATTACK INFO", 0},
	{AT_ENABLED, "SHOW CEILINGS", 0},
	{AT_ENABLED, "SHOW FLOORS", 0},

	// BBi
	{AT_ENABLED, "NO WALL HIT SOUND", 0},
	{AT_ENABLED, "ALWAYS RUN", 0},
	{AT_ENABLED, "QUIT ON ESCAPE", 0},
	{AT_ENABLED, "HEART BEAT SOUND", 0},
	{AT_ENABLED, "ROTATED AUTOMAP", 0},
};

CP_itemtype switch2_menu[] =
{
	{AT_ENABLED, "SKIP INTRO/OUTRO", 0},
	{AT_ENABLED, "SKIP FADE IN/OUT EFFECT", 0},
	{AT_ENABLED, "NO WEAPON BOBBING", 0},
	{AT_ENABLED, "VANILLA FIZZLE FX", 0},
	{AT_ENABLED, "[PS] MAP IN STATS", 0},
};

CP_itemtype NewEmenu[] = {
	{AT_ENABLED, "MISSION 1:\n"
	"STAR INSTITUTE", 0},

	{AT_NON_SELECTABLE, "MISSION 2:\n"
	"FLOATING FORTRESS", 0},

	{AT_NON_SELECTABLE, "MISSION 3:\n"
	"UNDERGROUND NETWORK", 0},

	{AT_NON_SELECTABLE, "MISSION 4:\n"
	"STAR PORT", 0},

	{AT_NON_SELECTABLE, "MISSION 5:\n"
	"HABITAT II", 0},

	{AT_NON_SELECTABLE, "MISSION 6:\n"
	"SATELLITE DEFENSE", 0},
};

CP_itemtype NewMenu[] = {
	{AT_ENABLED, "LEVEL 1:\n"
	"NOVICE AGENT", 0},
	{AT_ENABLED, "LEVEL 2:\n"
	"SKILLED AGENT", 0},
	{AT_ENABLED, "LEVEL 3:\n"
	"EXPERT AGENT", 0},
	{AT_ENABLED, "LEVEL 4:\n"
	"VETERAN AGENT", 0}
};

CP_itemtype LSMenu[] = {
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0}
};

CP_itemtype CusMenu[] = {
	{AT_ENABLED, "", 0},
	{AT_DISABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_DISABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0}
};

// BBi
CP_itemtype video_mode_menu[] =
{
	{AT_ENABLED, "RENDERER", nullptr},
	{AT_ENABLED, "WINDOW SIZE", nullptr},
	{AT_ENABLED, "VSYNC", nullptr},
	{AT_ENABLED, "ANTI-ALIASING TYPE", nullptr},
	{AT_ENABLED, "ANTI-ALIASING DEGREE", nullptr},
	{AT_DISABLED, "", nullptr},
	{AT_DISABLED, "APPLY", nullptr},
};

enum class TexturingMenuIndices
{
	anisotropy,
	image_2d_filter,
	image_3d_filter,
	mipmap_3d_filter,
	upscale_filter,
	upscale_degree,
	external_textures,
}; // TexturingMenuIndices

CP_itemtype texturing_menu[] =
{
	{AT_ENABLED, "ANISOTROPY", nullptr},
	{AT_ENABLED, "2D IMAGE FILTER", nullptr},
	{AT_ENABLED, "3D IMAGE FILTER", nullptr},
	{AT_ENABLED, "3D MIPMAP FILTER", nullptr},
	{AT_ENABLED, "UPSCALE FILTER", nullptr},
	{AT_ENABLED, "UPSCALE DEGREE", nullptr},
	{AT_ENABLED, "EXTERNAL TEXTURES", nullptr},
};

void video_menu_mode_routine(
	const std::int16_t index);

void texturing_routine(
	const std::int16_t index);

void filler_color_routine(
	const std::int16_t index);

CP_itemtype video_menu[] =
{
	{AT_ENABLED, "MODE", video_menu_mode_routine},
	{AT_ENABLED, "TEXTURING", texturing_routine},
	{AT_ENABLED, "WIDESCREEN", nullptr},
	{AT_ENABLED, "STRETCH UI", nullptr},
	{AT_ENABLED, "FILLER COLOR", filler_color_routine},
};
// BBi


std::int16_t color_hlite[] = {
	HIGHLIGHT_DISABLED_COLOR,
	HIGHLIGHT_TEXT_COLOR,
	READHCOLOR,
	HIGHLIGHT_DEACTIAVED_COLOR,
};

std::int16_t color_norml[] = {
	DISABLED_TEXT_COLOR,
	ENABLED_TEXT_COLOR,
	READCOLOR,
	DEACTIAVED_TEXT_COLOR,
};

std::int16_t EpisodeSelect[6] = {
	1,
	0,
	0,
	0,
	0,
	0,
};

std::int16_t SaveGamesAvail[10];
std::int16_t StartGame;
std::int16_t SoundStatus = 1;
std::int16_t pickquick;
char SaveGameNames[10][GAME_DESCRIPTION_LEN + 1];

static std::uint8_t menu_background_color = 0x00;


static const std::string& get_saved_game_base_name()
try {
	static auto base_name = std::string();
	static auto is_initialized = false;

	if (!is_initialized)
	{
		is_initialized = true;

		const auto& assets_info = get_assets_info();

		base_name = "bstone_";

		if (assets_info.is_aog_sw())
		{
			base_name += "aog_sw";
		}
		else if (assets_info.is_aog_full())
		{
			base_name += "aog_full";
		}
		else if (assets_info.is_ps())
		{
			base_name += "ps";
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid game type.");
		}

		base_name += "_saved_game_";
	}

	return base_name;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////

#ifndef CACHE_KEY_DATA

using ScanCodes = std::vector<ScanCode>;
using ScanNames = std::vector<std::string>;

// Scan code names with single chars
static ScanNames scan_names = {
	"?", "?", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "?", "?",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "|", "?", "A", "S",
	"D", "F", "G", "H", "J", "K", "L", ";", "\"", "?", "?", "?", "Z", "X", "C", "V",
	"B", "N", "M", ",", ".", "/", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "\xF", "?", "-", "\x15", "5", "\x11", "+", "?",
	"\x13", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
}; // scan_names

// Scan codes with >1 char names
static ScanCodes ext_scan_codes = {
	ScanCode::sc_escape,
	ScanCode::sc_backspace,
	ScanCode::sc_tab,
	ScanCode::sc_control,
	ScanCode::sc_left_shift,
	ScanCode::sc_space,
	ScanCode::sc_caps_lock,
	ScanCode::sc_f1,
	ScanCode::sc_f2,
	ScanCode::sc_f3,
	ScanCode::sc_f4,
	ScanCode::sc_f5,
	ScanCode::sc_f6,
	ScanCode::sc_f7,
	ScanCode::sc_f8,
	ScanCode::sc_f9,
	ScanCode::sc_f10,
	ScanCode::sc_f11,
	ScanCode::sc_f12,
	ScanCode::sc_scroll_lock,
	ScanCode::sc_return,
	ScanCode::sc_right_shift,
	ScanCode::sc_print_screen,
	ScanCode::sc_alt,
	ScanCode::sc_home,
	ScanCode::sc_page_up,
	ScanCode::sc_end,
	ScanCode::sc_page_down,
	ScanCode::sc_insert,
	ScanCode::sc_delete,
	ScanCode::sc_num_lock,
	ScanCode::sc_up_arrow,
	ScanCode::sc_down_arrow,
	ScanCode::sc_left_arrow,
	ScanCode::sc_right_arrow,
	ScanCode::sc_none,
}; // ExtScanCodes

static ScanNames ext_scan_names = {
	"ESC", "BKSP", "TAB", "CTRL", "LSHFT", "SPACE", "CAPSLK", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
	"F10", "F11", "F12", "SCRLK", "ENTER", "RSHFT", "PRTSC", "ALT",
	"HOME", "PGUP", "END", "PGDN", "INS", "DEL", "NUMLK", "UP",
	"DOWN", "LEFT", "RIGHT", "",
}; // ext_scan_names

#else

std::uint8_t* ScanNames, *ExtScanNames;
ScanCode* ExtScanCodes;

#endif

using SpecialKeys = std::vector<ScanCode>;

static SpecialKeys special_keys = {
	ScanCode::sc_back_quote,
	ScanCode::sc_equals,
	ScanCode::sc_minus,
	ScanCode::sc_l,
	ScanCode::sc_p,
	ScanCode::sc_m,
	ScanCode::sc_s,
	ScanCode::sc_i,
	ScanCode::sc_q,
	ScanCode::sc_w,
	ScanCode::sc_e,
	ScanCode::sc_return,
	ScanCode::sc_1,
	ScanCode::sc_2,
	ScanCode::sc_3,
	ScanCode::sc_4,
	ScanCode::sc_5,
	ScanCode::sc_tab,
}; // special_keys


// BBi
namespace
{


void menu_play_sound(
	int sound_index)
{
	sd_play_ui_sound(sound_index);
}

void menu_play_esc_pressed_sound()
{
	menu_play_sound(ESCPRESSEDSND);
}

void menu_play_no_way_sound()
{
	menu_play_sound(NOWAYSND);
}

void menu_play_move_gun_1_sound()
{
	menu_play_sound(MOVEGUN1SND);
}


using BindsNames = std::map<ScanCode, const char*>;


enum BindsFindDirection
{
	e_bfd_forward,
	e_bfd_backward
}; // BindsFindDirection

enum BindsFindFrom
{
	e_bff_current,
	e_bff_not_current
}; // BindingFindFrom


class BindsItem
{
public:
	std::string name;
	int name_width;
	Binding* binding;

	BindsItem(
		std::string new_name = std::string(),
		int new_name_width = 0,
		Binding* new_binding = nullptr) :
		name(new_name),
		name_width(new_name_width),
		binding(new_binding)
	{
	}
}; // BindsItem

using BindsItems = std::vector<BindsItem>;

static BindsItems binds = {
	{"MOVEMENT", 0, nullptr, },
	{"FORWARD", 0, &in_bindings[e_bi_forward], },
	{"BACKWARD", 0, &in_bindings[e_bi_backward], },
	{"LEFT", 0, &in_bindings[e_bi_left], },
	{"RIGHT", 0, &in_bindings[e_bi_right], },
	{"STRAFE", 0, &in_bindings[e_bi_strafe], },
	{"STRAFE LEFT", 0, &in_bindings[e_bi_strafe_left], },
	{"STRAFE RIGHT", 0, &in_bindings[e_bi_strafe_right], },
	{"QUICK LEFT", 0, &in_bindings[e_bi_quick_left], },
	{"QUICK RIGHT", 0, &in_bindings[e_bi_quick_right], },
	{"TURN AROUND", 0, &in_bindings[e_bi_turn_around], },
	{"RUN", 0, &in_bindings[e_bi_run], },
	{"", 0, nullptr, },

	{"WEAPONS", 0, nullptr, },
	{"ATTACK", 0, &in_bindings[e_bi_attack], },
	{"AUTO CHARGE PISTOL", 0, &in_bindings[e_bi_weapon_1], },
	{"SLOW FIRE PROTECTOR", 0, &in_bindings[e_bi_weapon_2], },
	{"RAPID ASSAULT WEAPON", 0, &in_bindings[e_bi_weapon_3], },
	{"DUAL NEUTRON DISRUPTOR", 0, &in_bindings[e_bi_weapon_4], },
	{"PLASMA DISCHARGE UNIT", 0, &in_bindings[e_bi_weapon_5], },
	{"ANTI-PLASMA CANNON (PS)", 0, &in_bindings[e_bi_weapon_6], },
	{"FISSION DETONATOR (PS)", 0, &in_bindings[e_bi_weapon_7], },
	{"", 0, nullptr, },
	{"CYCLE NEXT", 0, &in_bindings[e_bi_cycle_next_weapon], },
	{"CYCLE PREVIOUS", 0, &in_bindings[e_bi_cycle_previous_weapon], },
	{"", 0, nullptr, },

	{"INTERACTION", 0, nullptr, },
	{"USE", 0, &in_bindings[e_bi_use], },
	{"", 0, nullptr, },

	{"HUD", 0, nullptr, },
	{"STATS", 0, &in_bindings[e_bi_stats], },
	{"MAGNIFY RADAR (PS)", 0, &in_bindings[e_bi_radar_magnify], },
	{"MINIFY RADAR (PS)", 0, &in_bindings[e_bi_radar_minify], },
	{"", 0, nullptr, },

	{"MENU", 0, nullptr, },
	{"HELP", 0, &in_bindings[e_bi_help], },
	{"SAVE", 0, &in_bindings[e_bi_save], },
	{"LOAD", 0, &in_bindings[e_bi_load], },
	{"SOUND OPTIONS", 0, &in_bindings[e_bi_sound], },
	{"CONTROLS", 0, &in_bindings[e_bi_controls], },
	{"END GAME", 0, &in_bindings[e_bi_end_game], },
	{"QUICK SAVE", 0, &in_bindings[e_bi_quick_save], },
	{"QUICK LOAD", 0, &in_bindings[e_bi_quick_load], },
	{"QUICK EXIT", 0, &in_bindings[e_bi_quick_exit], },
	{"", 0, nullptr, },

	{"OPTIONS", 0, nullptr, },
	{"ATTACK INFO", 0, &in_bindings[e_bi_attack_info], },
	{"LIGHTNING", 0, &in_bindings[e_bi_lightning], },
	{"SOUND", 0, &in_bindings[e_bi_sfx], },
	{"MUSIC", 0, &in_bindings[e_bi_music], },
	{"CEILING", 0, &in_bindings[e_bi_ceiling], },
	{"FLOORING", 0, &in_bindings[e_bi_flooring], },
	{"HEART BEAT (AOG)", 0, &in_bindings[e_bi_heart_beat], },
	{"", 0, nullptr, },

	{"MISC", 0, nullptr, },
	{"PAUSE", 0, &in_bindings[e_bi_pause], },
	{"(UN)GRAB MOUSE", 0, &in_bindings[e_bi_grab_mouse], },
	{"TAKE SCREENSHOT", 0, &in_bindings[e_bi_take_screenshot], },
}; // binds


const int k_binds_max_per_window = 14;
const int k_binds_text_keys_gap = 3;
const int k_binds_line_spacing = 1;
const int k_binds_top = 28;

const std::uint8_t k_binds_category_color = 0x4A;
const std::uint8_t k_binds_text_color = 0x56;
const std::uint8_t k_binds_key_text_color = 0x7F;
const std::uint8_t k_binds_key_bar_default_color = 0x03;
const std::uint8_t k_binds_key_bar_active_color = 0x31;
const std::uint8_t k_binds_key_bar_assign_color = 0x14;

int binds_count;
int binds_window;
int binds_window_offset;
int binds_key_index;
int binds_key_width;
int binds_max_text_width;
int binds_text_height;
int binds_text_x;
int binds_key_x[k_max_binding_keys];
bool binds_is_assigning = false;

BindsNames binds_names;


void binds_initialize_menu()
{
	static bool is_initialized = false;

	if (is_initialized)
	{
		return;
	}

	binds_count = 0;
	binds_window = 0;
	binds_window_offset = 0;
	binds_key_index = 0;
	binds_key_width = 0;
	binds_max_text_width = 0;
	binds_text_height = 0;
	binds_is_assigning = false;

	bool has_bindings = false;

	fontnumber = 2;

	for (auto& bind : binds)
	{
		++binds_count;

		if (!bind.name.empty())
		{
			int width = 0;
			int height = 0;
			VW_MeasurePropString(bind.name.c_str(), &width, &height);

			bind.name_width = width;

			if (width > binds_max_text_width)
			{
				binds_max_text_width = width;
			}

			if (height > binds_text_height)
			{
				binds_text_height = height;
			}

			has_bindings = true;
		}
	}


	if (!has_bindings)
	{
		BSTONE_THROW_STATIC_SOURCE("No bindings.");
	}

	binds_names.clear();
	binds_names[ScanCode::sc_return] = "ENTER";
	binds_names[ScanCode::sc_space] = "SPACE";
	binds_names[ScanCode::sc_minus] = "-";
	binds_names[ScanCode::sc_equals] = "=";
	binds_names[ScanCode::sc_backspace] = "BACKSPACE";
	binds_names[ScanCode::sc_tab] = "TAB";
	binds_names[ScanCode::sc_alt] = "ALT";
	binds_names[ScanCode::sc_left_bracket] = "[";
	binds_names[ScanCode::sc_right_bracket] = "]";
	binds_names[ScanCode::sc_control] = "CTRL";
	binds_names[ScanCode::sc_caps_lock] = "CAPS LOCK";
	binds_names[ScanCode::sc_num_lock] = "NUM LOCK";
	binds_names[ScanCode::sc_scroll_lock] = "SCROLL LOCK";
	binds_names[ScanCode::sc_left_shift] = "L-SHIFT";
	binds_names[ScanCode::sc_right_shift] = "R-SHIFT";
	binds_names[ScanCode::sc_up_arrow] = "UP";
	binds_names[ScanCode::sc_down_arrow] = "DOWN";
	binds_names[ScanCode::sc_left_arrow] = "LEFT";
	binds_names[ScanCode::sc_right_arrow] = "RIGHT";
	binds_names[ScanCode::sc_insert] = "INS";
	binds_names[ScanCode::sc_delete] = "DEL";
	binds_names[ScanCode::sc_home] = "HOME";
	binds_names[ScanCode::sc_end] = "END";
	binds_names[ScanCode::sc_page_up] = "PGUP";
	binds_names[ScanCode::sc_page_down] = "PGDN";
	binds_names[ScanCode::sc_slash] = "/";
	binds_names[ScanCode::sc_f1] = "F1";
	binds_names[ScanCode::sc_f2] = "F2";
	binds_names[ScanCode::sc_f3] = "F3";
	binds_names[ScanCode::sc_f4] = "F4";
	binds_names[ScanCode::sc_f5] = "F5";
	binds_names[ScanCode::sc_f6] = "F6";
	binds_names[ScanCode::sc_f7] = "F7";
	binds_names[ScanCode::sc_f8] = "F8";
	binds_names[ScanCode::sc_f9] = "F9";
	binds_names[ScanCode::sc_f10] = "F10";
	binds_names[ScanCode::sc_f11] = "F11";
	binds_names[ScanCode::sc_f12] = "F12";
	binds_names[ScanCode::sc_print_screen] = "PRT SCR";
	binds_names[ScanCode::sc_pause] = "PAUSE";
	binds_names[ScanCode::sc_back_quote] = "BACK QUOTE";
	binds_names[ScanCode::sc_semicolon] = ";";
	binds_names[ScanCode::sc_quote] = "'";
	binds_names[ScanCode::sc_backslash] = "\\";
	binds_names[ScanCode::sc_comma] = ",";
	binds_names[ScanCode::sc_period] = ".";

	binds_names[ScanCode::sc_1] = "1";
	binds_names[ScanCode::sc_2] = "2";
	binds_names[ScanCode::sc_3] = "3";
	binds_names[ScanCode::sc_4] = "4";
	binds_names[ScanCode::sc_5] = "5";
	binds_names[ScanCode::sc_6] = "6";
	binds_names[ScanCode::sc_7] = "7";
	binds_names[ScanCode::sc_8] = "8";
	binds_names[ScanCode::sc_9] = "9";
	binds_names[ScanCode::sc_0] = "0";

	binds_names[ScanCode::sc_a] = "A";
	binds_names[ScanCode::sc_b] = "B";
	binds_names[ScanCode::sc_c] = "C";
	binds_names[ScanCode::sc_d] = "D";
	binds_names[ScanCode::sc_e] = "E";
	binds_names[ScanCode::sc_f] = "F";
	binds_names[ScanCode::sc_g] = "G";
	binds_names[ScanCode::sc_h] = "H";
	binds_names[ScanCode::sc_i] = "I";
	binds_names[ScanCode::sc_j] = "J";
	binds_names[ScanCode::sc_k] = "K";
	binds_names[ScanCode::sc_l] = "L";
	binds_names[ScanCode::sc_m] = "M";
	binds_names[ScanCode::sc_n] = "N";
	binds_names[ScanCode::sc_o] = "O";
	binds_names[ScanCode::sc_p] = "P";
	binds_names[ScanCode::sc_q] = "Q";
	binds_names[ScanCode::sc_r] = "R";
	binds_names[ScanCode::sc_s] = "S";
	binds_names[ScanCode::sc_t] = "T";
	binds_names[ScanCode::sc_u] = "U";
	binds_names[ScanCode::sc_v] = "V";
	binds_names[ScanCode::sc_w] = "W";
	binds_names[ScanCode::sc_x] = "X";
	binds_names[ScanCode::sc_y] = "Y";
	binds_names[ScanCode::sc_z] = "Z";

	binds_names[ScanCode::sc_kp_minus] = "KP MINUS";
	binds_names[ScanCode::sc_kp_plus] = "KP PLUS";

	binds_names[ScanCode::sc_mouse_left] = "MOUSE 1";
	binds_names[ScanCode::sc_mouse_middle] = "MOUSE 2";
	binds_names[ScanCode::sc_mouse_right] = "MOUSE 3";
	binds_names[ScanCode::sc_mouse_x1] = "MOUSE 4";
	binds_names[ScanCode::sc_mouse_x2] = "MOUSE 5";

	binds_names[ScanCode::sc_mouse_wheel_down] = "MWHEEL DOWN";
	binds_names[ScanCode::sc_mouse_wheel_up] = "MWHEEL UP";

	for (const auto& binds_name : binds_names)
	{
		int width = 0;
		int height = 0;
		VW_MeasurePropString(binds_name.second, &width, &height);

		if (width > binds_key_width)
		{
			binds_key_width = width;
		}
	}

	int max_keys_width = k_max_binding_keys * (binds_key_width + 1);
	int max_text_width = 2 + binds_max_text_width;
	int max_width = max_keys_width + k_binds_text_keys_gap + max_text_width;

	int text_x = (SCREEN_W - max_width) / 2;

	int base_key_x = text_x + max_text_width + k_binds_text_keys_gap;

	binds_text_x = text_x;

	for (int i = 0; i < k_max_binding_keys; ++i)
	{
		binds_key_x[i] = base_key_x + (i * (binds_key_width + 1));
	}

	is_initialized = true;
}

bool binds_advance_to_item(
	BindsFindDirection direction)
{
	switch (direction)
	{
	case e_bfd_forward:
		if ((binds_window + binds_window_offset + 1) < binds_count)
		{
			++binds_window_offset;

			if (binds_window_offset == k_binds_max_per_window)
			{
				++binds_window;
				--binds_window_offset;
			}

			return true;
		}
		else
		{
			return false;
		}

	case e_bfd_backward:
		if ((binds_window + binds_window_offset) > 0)
		{
			--binds_window_offset;

			if (binds_window_offset < 0)
			{
				--binds_window;
				binds_window_offset = 0;
			}

			return true;
		}
		else
		{
			return false;
		}

	default:
		return false;
	}
}

bool binds_find_item(
	BindsFindDirection direction,
	BindsFindFrom from)
{
	if (from == e_bff_not_current)
	{
		if (!binds_advance_to_item(direction))
		{
			return false;
		}
	}

	while (true)
	{
		if (binds[binds_window + binds_window_offset].binding)
		{
			return true;
		}

		if (!binds_advance_to_item(direction))
		{
			return false;
		}
	}

	return false;
}

bool binds_assign_new_key(
	ScanCode key,
	Binding& binding)
{
	auto it = binds_names.find(LastScan);

	if (it == binds_names.end())
	{
		return false;
	}

	for (int b = 0; b < k_max_bindings; ++b)
	{
		for (int k = 0; k < k_max_binding_keys; ++k)
		{
			if (in_bindings[b][k] == key)
			{
				in_bindings[b][k] = ScanCode::sc_none;
			}
		}
	}

	binding[binds_key_index] = key;

	return true;
}

void binds_remove_binding()
{
	auto& item = binds[binds_window + binds_window_offset];
	(*item.binding)[binds_key_index] = ScanCode::sc_none;
}

void binds_draw_item_text(
	int item_index)
{
	auto& item = binds[binds_window + item_index];

	if (item.name.empty())
	{
		return;
	}

	int x = SCREEN_X + binds_text_x;
	int y = SCREEN_Y + k_binds_top +
		(item_index * (binds_text_height + k_binds_line_spacing));

	int text_left_offset = 0;
	int text_width = 0;

	if (item.binding)
	{
		text_width = item.name_width;
		text_left_offset = binds_max_text_width - item.name_width;

		if (text_left_offset < 0)
		{
			text_left_offset = 0;
			text_width = binds_max_text_width;
		}
	}
	else
	{
		text_width = SCREEN_W;
	}

	PrintX = static_cast<std::int16_t>(x + text_left_offset);
	PrintY = static_cast<std::int16_t>(y);
	WindowX = PrintX;
	WindowY = PrintY;
	WindowW = static_cast<std::int16_t>(text_width);
	WindowH = static_cast<std::int16_t>(binds_text_height);

	if (item.binding)
	{
		fontcolor = k_binds_text_color;
		US_Print(item.name.c_str());
	}
	else
	{
		fontcolor = k_binds_category_color;
		US_PrintCentered(item.name.c_str());
	}
}

void binds_draw_keys(
	int item_index)
{
	const auto& item = binds[binds_window + item_index];

	if (!item.binding)
	{
		return;
	}

	int y = SCREEN_Y + k_binds_top +
		(item_index * (binds_text_height + k_binds_line_spacing));

	bool is_current = (item_index == binds_window_offset);

	for (int k = 0; k < k_max_binding_keys; ++k)
	{
		std::uint8_t color;
		ScanCode key;
		const char* key_name;

		bool is_active = is_current && (binds_key_index == k);

		if (is_active)
		{
			color =
				binds_is_assigning ?
				k_binds_key_bar_assign_color :
				k_binds_key_bar_active_color;
		}
		else
		{
			color = k_binds_key_bar_default_color;
		}

		int x = SCREEN_X + binds_key_x[k];

		VWB_Bar(
			x,
			y,
			binds_key_width,
			binds_text_height,
			color);

		PrintX = static_cast<std::int16_t>(x);
		PrintY = static_cast<std::int16_t>(y);
		WindowX = PrintX;
		WindowY = PrintY;
		WindowW = static_cast<std::int16_t>(binds_key_width);
		WindowH = static_cast<std::int16_t>(binds_text_height);

		if (!(is_active && binds_is_assigning))
		{
			key = (*item.binding)[k];

			if (key != ScanCode::sc_none)
			{
				key_name = "???";

				auto name_it = binds_names.find(key);

				if (name_it != binds_names.end())
				{
					key_name = name_it->second;
				}

				fontcolor = k_binds_key_text_color;
				US_PrintCentered(key_name);
			}
		}
	}
}

void binds_draw()
{
	bool found_item = false;

	found_item = binds_find_item(e_bfd_forward, e_bff_current);

	if (!found_item)
	{
		found_item = binds_find_item(e_bfd_backward, e_bff_current);
	}

	ClearMScreen();
	DrawMenuTitle("CUSTOMIZE CONTROLS");

	DrawInstructions(
		binds_is_assigning ? IT_CONTROLS_ASSIGNING_KEY : IT_CONTROLS);

	fontnumber = 2;

	for (int i = 0; i < k_binds_max_per_window; ++i)
	{
		int item_index = binds_window + i;

		if (item_index == binds_count)
		{
			break;
		}

		binds_draw_item_text(i);
		binds_draw_keys(i);
	}
}

void binds_draw_menu()
{
	bool is_up_pressed = false;
	bool is_down_pressed = false;
	bool is_left_pressed = false;
	bool is_right_pressed = false;
	bool is_pgdn_pressed = false;
	bool is_pgup_pressed = false;
	bool is_enter_pressed = false;
	bool is_delete_pressed = false;
	bool is_escape_pressed = false;

	bool handle_up = false;
	bool handle_down = false;
	bool handle_left = false;
	bool handle_right = false;
	bool handle_pgdn = false;
	bool handle_pgup = false;
	bool handle_enter = false;
	bool handle_delete = false;
	bool handle_escape = false;

	bool fade_in = true;

	CA_CacheGrChunk(STARTFONT + 2);
	CA_CacheGrChunk(STARTFONT + 4);
	binds_initialize_menu();

	binds_is_assigning = false;

	while (true)
	{
		binds_draw();
		VW_UpdateScreen();

		if (fade_in)
		{
			fade_in = false;
			MenuFadeIn();
		}

		in_handle_events();

		if (binds_is_assigning)
		{
			LastScan = ScanCode::sc_none;
			bool quit = false;

			while (!quit)
			{
				LastScan = ScanCode::sc_none;
				in_handle_events();

				if (Keyboard[ScanCode::sc_escape])
				{
					quit = true;
					menu_play_esc_pressed_sound();
				}
				else if (LastScan != ScanCode::sc_none)
				{
					auto& item = binds[binds_window + binds_window_offset];

					if (binds_assign_new_key(LastScan, *item.binding))
					{
						ShootSnd();
						quit = true;
					}
					else
					{
						menu_play_no_way_sound();
					}
				}
			}

			is_escape_pressed = true;
			binds_is_assigning = false;
		}
		else
		{
			if (Keyboard[ScanCode::sc_up_arrow])
			{
				if (!is_up_pressed)
				{
					handle_up = true;
					is_up_pressed = true;
				}
			}
			else
			{
				is_up_pressed = false;
			}

			if (Keyboard[ScanCode::sc_down_arrow])
			{
				if (!is_down_pressed)
				{
					handle_down = true;
					is_down_pressed = true;
				}
			}
			else
			{
				is_down_pressed = false;
			}

			if (Keyboard[ScanCode::sc_left_arrow])
			{
				if (!is_left_pressed)
				{
					handle_left = true;
					is_left_pressed = true;
				}
			}
			else
			{
				is_left_pressed = false;
			}

			if (Keyboard[ScanCode::sc_right_arrow])
			{
				if (!is_right_pressed)
				{
					handle_right = true;
					is_right_pressed = true;
				}
			}
			else
			{
				is_right_pressed = false;
			}

			if (Keyboard[ScanCode::sc_page_down])
			{
				if (!is_pgdn_pressed)
				{
					handle_pgdn = true;
					is_pgdn_pressed = true;
				}
			}
			else
			{
				is_pgdn_pressed = false;
			}

			if (Keyboard[ScanCode::sc_page_up])
			{
				if (!is_pgup_pressed)
				{
					handle_pgup = true;
					is_pgup_pressed = true;
				}
			}
			else
			{
				is_pgup_pressed = false;
			}

			if (Keyboard[ScanCode::sc_return])
			{
				if (!is_enter_pressed)
				{
					handle_enter = true;
					is_enter_pressed = true;
				}
			}
			else
			{
				is_enter_pressed = false;
			}

			if (Keyboard[ScanCode::sc_delete])
			{
				if (!is_delete_pressed)
				{
					handle_delete = true;
					is_delete_pressed = true;
				}
			}
			else
			{
				is_delete_pressed = false;
			}

			if (Keyboard[ScanCode::sc_escape])
			{
				if (!is_escape_pressed)
				{
					handle_escape = true;
					is_escape_pressed = true;
				}
			}
			else
			{
				is_escape_pressed = false;
			}

			if (handle_up)
			{
				handle_up = false;
				binds_find_item(e_bfd_backward, e_bff_not_current);
			}

			if (handle_down)
			{
				handle_down = false;
				binds_find_item(e_bfd_forward, e_bff_not_current);
			}

			if (handle_left)
			{
				handle_left = false;

				if (binds_key_index == 1)
				{
					binds_key_index = 0;
				}
			}

			if (handle_right)
			{
				handle_right = false;

				if (binds_key_index == 0)
				{
					binds_key_index = 1;
				}
			}

			if (handle_pgdn)
			{
				handle_pgdn = false;

				for (int i = 0; i < k_binds_max_per_window; ++i)
				{
					binds_find_item(e_bfd_forward, e_bff_not_current);
				}
			}

			if (handle_pgup)
			{
				handle_pgup = false;

				for (int i = 0; i < k_binds_max_per_window; ++i)
				{
					binds_find_item(e_bfd_backward, e_bff_not_current);
				}
			}

			if (handle_enter)
			{
				handle_enter = false;
				binds_is_assigning = true;
			}

			if (handle_delete)
			{
				handle_delete = false;
				binds_remove_binding();
				ShootSnd();
			}

			if (handle_escape)
			{
				handle_escape = false;
				menu_play_esc_pressed_sound();
				break;
			}
		}
	}

	MenuFadeOut();
}


} // namespace
// BBi

void HelpScreens()
{
	HelpPresenter(nullptr, false, HELPTEXT, true);
}

void HelpPresenter(
	const char* fname,
	bool continue_keys,
	std::uint16_t id_cache,
	bool startmusic)
{
	const int FULL_VIEW_WIDTH = 19;

	PresenterInfo pi;
	int oldwidth;

	pi = PresenterInfo{};

	pi.flags = TPF_SHOW_PAGES;
	if (continue_keys)
	{
		pi.flags |= TPF_CONTINUE;
	}

	VW_FadeOut();

	// Change view size to MAX! (scaler clips shapes on smaller views)
	//
	oldwidth = viewwidth / 16;
	if (oldwidth != FULL_VIEW_WIDTH)
	{
		NewViewSize();
	}

	// Draw help border
	//
	CacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);
	VWB_DrawPic(0, 0, H_TOPWINDOWPIC);
	VWB_DrawPic(0, 8, H_LEFTWINDOWPIC);
	VWB_DrawPic(312, 8, H_RIGHTWINDOWPIC);
	VWB_DrawPic(8, 176, H_BOTTOMINFOPIC);
	UnCacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);

	// Setup for text presenter
	//
	pi.xl = 8;
	pi.yl = 8;
	pi.xh = 311;
	pi.yh = 175;
	pi.ltcolor = 0x7b;
	pi.bgcolor = 0x7d;
	pi.dkcolor = 0x7f;
	pi.shcolor = 0x00;
	pi.fontnumber = 4;

	if (continue_keys)
	{
		pi.infoline = (char*)" UP / DN - PAGES       ENTER - CONTINUES         ESC - EXITS";
	}
	else
	{
		pi.infoline = (char*)"           UP / DN - PAGES            ESC - EXITS";
	}

	if (startmusic)
	{
		StartCPMusic(static_cast<std::int16_t>(TEXTSONG));
	}

	// Load, present, and free help text.
	//
	TP_LoadScript(fname, &pi, id_cache);
	TP_Presenter(&pi);
	TP_FreeScript(&pi, id_cache);

	MenuFadeOut();

	// Reset view size
	//
	if (oldwidth != FULL_VIEW_WIDTH)
	{
		NewViewSize();
	}

	if (startmusic && TPscan == ScanCode::sc_escape)
	{
		StartCPMusic(MENUSONG);
	}

	IN_ClearKeysDown();
}

void US_ControlPanel(
	ScanCode scancode)
{
	is_full_menu_active = (scancode != ScanCode::sc_f7 && scancode != ScanCode::sc_f10);
	sd_pause_scene_sfx(true);

	const auto guard_flag = bstone::make_scope_exit(
		[&]()
		{
			is_full_menu_active = false;
			sd_pause_scene_sfx(false);
		});

	// BBi
	const auto& assets_info = get_assets_info();

	menu_background_color = (
		(assets_info.is_aog_sw_v3_0() || assets_info.is_aog_full_v3_0()) ?
		0x04 :
		TERM_BACK_COLOR);


	std::int16_t which;

	if (ingame)
	{
		if (CP_CheckQuick(scancode))
		{
			return;
		}
	}

	SetupControlPanel();

	StartCPMusic(MENUSONG);

	//
	// F-KEYS FROM WITHIN GAME
	//
	auto finish = false;

	switch (scancode)
	{
	case ScanCode::sc_f1:
		CleanupControlPanel();
		HelpScreens();
		return;

	case ScanCode::sc_f2:
		CP_SaveGame(0);
		finish = true;
		break;

	case ScanCode::sc_f3:
		CP_LoadGame(0);
		finish = true;
		break;

	case ScanCode::sc_f4:
		CP_Sound(0);
		finish = true;
		break;

	case ScanCode::sc_f6:
		CP_Control(0);
		finish = true;
		break;

	default:
		break;
	}

	if (finish)
	{
		CleanupControlPanel();
		return;
	}

	DrawMainMenu();
	MenuFadeIn();
	StartGame = 0;

	//
	// MAIN MENU LOOP
	//
	do
	{
		which = HandleMenu(&MainItems, &MainMenu[0], nullptr);

		switch (which)
		{
		case MM_VIEW_SCORES:
			if (!MainMenu[MM_VIEW_SCORES].routine)
			{
				if (CP_EndGame())
				{
					StartGame = 1;
				}
			}

			DrawMainMenu();
			MenuFadeIn();
			break;

		case -1:
			// on hit ESC on main menu
			if (ingame && !gp_quit_on_escape())
			{
				// return to game if quit on escape not enabled
				StartGame = 1;
			}
			else
			{
				CP_Quit();
			}
			break;

		case MM_LOGOFF:
			CP_Quit();
			break;

		default:
			if (!StartGame)
			{
				DrawMainMenu();
				MenuFadeIn();
			}
		}

		//
		// "EXIT OPTIONS" OR "NEW GAME" EXITS
		//
	} while (!StartGame);

	//
	// DEALLOCATE EVERYTHING
	//
	CleanupControlPanel();
	if (!loadedgame)
	{
		StopMusic();
	}


	//
	// CHANGE MAINMENU ITEM
	//
	if (startgame || loadedgame)
	{
		MainMenu[MM_VIEW_SCORES].routine = nullptr;
		MainMenu[MM_VIEW_SCORES].string = "END GAME";
	}

	if (ingame && loadedgame)
	{
		refresh_screen = false;
	}


#ifdef CACHE_KEY_DATA
	FREEFONT(SCANNAMES_DATA);
	FREEFONT(EXTSCANNAMES_DATA);
	FREEFONT(EXTSCANCODES_DATA);
#endif
}

void DrawMainMenu()
{
	ControlPanelFree();
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ControlPanelAlloc();

	ClearMScreen();
	DrawMenuTitle("MAIN OPTIONS");
	DrawInstructions(IT_STANDARD);

	//
	// CHANGE "MISSION" AND "DEMO"
	//
	auto& back_to_demo_mi = MainMenu[MM_BACK_TO_DEMO];

	if (ingame)
	{
		back_to_demo_mi.string = "BACK TO MISSION";
		back_to_demo_mi.active = AT_READIT;
	}
	else
	{
		back_to_demo_mi.string.clear();
		back_to_demo_mi.active = AT_DISABLED;
	}

	fontnumber = 4; // COAL

	DrawMenu(&MainItems, &MainMenu[0]);

	VW_UpdateScreen();
}

void CP_ReadThis(
	std::int16_t)
{
	ControlPanelFree();
	HelpScreens();
	ControlPanelAlloc();
}

void CP_OrderingInfo(
	std::int16_t)
{
	ControlPanelFree();
	HelpPresenter("", false, ORDERTEXT, true);
	ControlPanelAlloc();
}

void CP_BlakeStoneSaga(
	std::int16_t)
{
	ControlPanelFree();
	HelpPresenter("", false, SAGATEXT, true);
	ControlPanelAlloc();
}

// --------------------------------------------------------------------------
// CP_CheckQuick() - CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
// --------------------------------------------------------------------------
bool CP_CheckQuick(
	ScanCode scancode)
{
	auto string = std::string{};
	string.reserve(100);

	switch (scancode)
	{
		// END GAME
		//
	case ScanCode::sc_f7:
		// BBi
#if 0
		VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

		CA_CacheGrChunk(STARTFONT + 1);

		WindowH = 160;
		if (Confirm(ENDGAMESTR))
		{
			playstate = ex_died;
			pickquick = gamestate.lives = 0;
		}

		WindowH = 200;
		fontnumber = 4;
		return true;

		// QUICKSAVE
		//
	case ScanCode::sc_f8:
		if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick)
		{
			string = "Quick Save will overwrite:\n\"";

			CA_CacheGrChunk(STARTFONT + 1);

			string += SaveGameNames[static_cast<int>(LSItems.curpos)];
			string += "\"?";

			// BBi
#if 0
			VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

			if (Confirm(string.c_str()))
			{
				CA_CacheGrChunk(STARTFONT + 1);
				CP_SaveGame(1);
				fontnumber = 4;
			}
			else
			{
				refresh_screen = false;
			}
		}
		else
		{
			CA_CacheGrChunk(STARTFONT + 1);

			vid_is_hud = true;
			VW_FadeOut();
			vid_is_hud = false;

			StartCPMusic(MENUSONG);

			pickquick = CP_SaveGame(0);

			lasttimecount = TimeCount;
			in_clear_mouse_deltas();
		}

		return true;

		// QUICKLOAD
		//
	case ScanCode::sc_f9:
		if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick)
		{
			string = "Quick Load:\n\"";

			CA_CacheGrChunk(STARTFONT + 1);

			string += SaveGameNames[static_cast<int>(LSItems.curpos)];
			string += "\"?";

			// BBi
#if 0
			VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

			if (Confirm(string.c_str()))
			{
				CP_LoadGame(1);
			}
			else
			{
				refresh_screen = false;
				return true;
			}

			fontnumber = 4;
		}
		else
		{
			CA_CacheGrChunk(STARTFONT + 1);

			vid_is_hud = true;
			VW_FadeOut();
			vid_is_hud = false;

			StartCPMusic(MENUSONG);

			pickquick = CP_LoadGame(0);

			lasttimecount = TimeCount;
			in_clear_mouse_deltas();
		}

		if (pickquick)
		{
			refresh_screen = false;
		}
		return true;

		// QUIT
		//
	case ScanCode::sc_f10:
		CA_CacheGrChunk(STARTFONT + 1);

		// BBi
#if 0
		VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

		WindowX = WindowY = 0;
		WindowW = 320;
		WindowH = 160;
		if (Confirm(QuitToDosStr))
		{
			ExitGame();
		}

		refresh_screen = false;
		WindowH = 200;
		fontnumber = 4;

		return true;

	default:
		return false;
	}
}

std::int16_t CP_EndGame()
{
	if (!Confirm(ENDGAMESTR))
	{
		return 0;
	}

	pickquick = gamestate.lives = 0;
	playstate = ex_died;
	InstantQuit = 1;

	return 1;
}

void CP_ViewScores(
	std::int16_t)
{
	fontnumber = 4;

	StartCPMusic(static_cast<std::int16_t>(ROSTER_MUS));

	DrawHighScores();
	VW_UpdateScreen();
	MenuFadeIn();
	fontnumber = 1;

	IN_Ack();

	StartCPMusic(MENUSONG);

	MenuFadeOut();
}

void CP_NewGame(
	std::int16_t)
{
	std::int16_t which, episode = 0;

	DrawMenuTitle("Difficulty Level");
	DrawInstructions(IT_STANDARD);

	const auto& assets_info = get_assets_info();

firstpart:

	if (!assets_info.is_ps())
	{
		DrawNewEpisode();
		do
		{
			which = HandleMenu(&NewEitems, &NewEmenu[0], DrawEpisodePic);
			switch (which)
			{
			case -1:
				MenuFadeOut();
				return;

			default:
				if (!EpisodeSelect[which])
				{
					menu_play_no_way_sound();
					CacheMessage(READTHIS_TEXT);
					IN_ClearKeysDown();
					IN_Ack();
					VL_Bar(35, 69, 250, 62, menu_background_color);
					DrawNewEpisode();
					which = 0;
				}
				else
				{
					episode = which;
					which = 1;
				}
				break;
			}

		} while (!which);

		ShootSnd();
	}
	else
	{
		episode = 0;
	}

	//
	// ALREADY IN A GAME?
	//
	if (!assets_info.is_ps() && ingame)
	{
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}
	}

secondpart:

	MenuFadeOut();
	if (ingame)
	{
		CA_CacheScreen(BACKGROUND_SCREENPIC);
	}
	DrawNewGame();
	which = HandleMenu(&NewItems, &NewMenu[0], DrawNewGameDiff);

	if (which < 0)
	{
		MenuFadeOut();

		if (!assets_info.is_ps())
		{
			goto firstpart;
		}
		else
		{
			return;
		}
	}

	ShootSnd();
	MenuFadeOut();
	ControlPanelFree();

	if (Breifing(BT_INTRO, episode))
	{
		CA_CacheScreen(BACKGROUND_SCREENPIC);
		ControlPanelAlloc();
		goto secondpart;
	}

	StartGame = 1;
	NewGame(which, episode);

	//
	// CHANGE "READ THIS!" TO NORMAL COLOR
	//
	MainMenu[MM_READ_THIS].active = AT_ENABLED;
}

void DrawMenuTitle(
	const char* title)
{
	fontnumber = 3;
	CA_CacheGrChunk(STARTFONT + fontnumber);

	PrintX = WindowX = 32;
	PrintY = WindowY = 32;
	WindowW = 244;
	WindowH = 20;

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(title);

	WindowX = 32 - 1;
	WindowY = 32 - 1;

	SETFONTCOLOR(ENABLED_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(title);

	FREEFONT(STARTFONT + fontnumber);
}

const std::int16_t INSTRUCTIONS_Y_POS = 154 + 10;

// ---------------------------------------------------------------------------
// DrawInstructions() - Draws instructions centered at the bottom of
//      the view screen.
//
// NOTES: Original font number or font color is not maintained.
// ---------------------------------------------------------------------------
void DrawInstructions(
	inst_type Type)
{
	const char* instr[MAX_INSTRUCTIONS] = {
		"UP/DN SELECTS - ENTER CHOOSES - ESC EXITS",
		"PRESS ANY KEY TO CONTINUE",
		"ENTER YOUR NAME AND PRESS ENTER",
		"RT/LF ARROW SELECTS - ENTER CHOOSES",

		// BBi
		"UP/DN SELECTS - LF/RT CHANGES - ESC EXITS",
		"ARROWS SELECTS - ENTER CHOOSES - DEL REMOVES",
		"ESC EXITS"
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = INSTRUCTIONS_Y_POS;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);
}

void DrawNewEpisode()
{
	ClearMScreen();

	DrawMenuTitle("CHOOSE A MISSION");
	DrawInstructions(IT_STANDARD);

	PrintY = 51;
	WindowX = 58;

	fontnumber = 2; // six point font
	DrawMenu(&NewEitems, &NewEmenu[0]);

	DrawEpisodePic(NewEitems.curpos);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();

}

void DrawNewGame()
{
	ClearMScreen();
	DrawMenuTitle("DIFFICULTY LEVEL");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2; // six point font
	DrawMenu(&NewItems, &NewMenu[0]);

	DrawNewGameDiff(NewItems.curpos);

	px = 48;
	py = INSTRUCTIONS_Y_POS - 24;
	ShPrint("        HIGHER DIFFICULTY LEVELS CONTAIN", TERM_SHADOW_COLOR, false);

	px = 48;
	py += 6;
	ShPrint("            MORE, STRONGER ENEMIES", TERM_SHADOW_COLOR, false);


	VW_UpdateScreen();

	MenuFadeIn();
	WaitKeyUp();
}

void DrawNewGameDiff(
	std::int16_t w)
{
	VWB_DrawPic(192, 77, w + C_BABYMODEPIC);
}

void DrawEpisodePic(
	std::int16_t w)
{
	VWB_DrawPic(176, 72, w + C_EPISODE1PIC);
}

void CP_GameOptions(
	std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawGopMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&GopItems, &GopMenu[0], nullptr);

		if (which != -1)
		{
			DrawGopMenu();
			MenuFadeIn();
		}

	} while (which >= 0);

	MenuFadeOut();
}

void DrawGopMenu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME OPTIONS");
	DrawInstructions(IT_STANDARD);

	fontnumber = 4; // COAL

	DrawMenu(&GopItems, &GopMenu[0]);

	VW_UpdateScreen();
}

void ChangeSwaps()
{
	WindowX = WindowY = 0;
	WindowW = 320;
	WindowH = 200;
	Message(Computing);

	ClearMemory();
	ControlPanelAlloc();

	IN_UserInput(50);
	IN_ClearKeysDown();

}

void CP_Switches(
	std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawSwitchMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&SwitchItems, &SwitchMenu[0], DrawAllSwitchLights);

		switch (which)
		{
		case SW_LIGHTING:
			gp_no_shading(!gp_no_shading());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_REBA_ATTACK_INFO:
			gp_hide_attacker_info(!gp_hide_attacker_info());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_CEILING:
			gp_is_ceiling_solid(!gp_is_ceiling_solid());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_FLOORS:
			gp_is_flooring_solid(!gp_is_flooring_solid());
			ShootSnd();
			DrawSwitchMenu();
			break;

		// BBi
		case SW_NO_WALL_HIT_SOUND:
			gp_no_wall_hit_sfx(!gp_no_wall_hit_sfx());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_ALWAYS_RUN:
			gp_is_always_run(!gp_is_always_run());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_QUIT_ON_ESCAPE:
			gp_quit_on_escape(!gp_quit_on_escape());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_HEART_BEAT_SOUND:
			gp_use_heart_beat_sfx(!gp_use_heart_beat_sfx());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_ROTATED_AUTOMAP:
			am_rotatable(!am_rotatable());
			ShootSnd();
			DrawSwitchMenu();
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void DrawSwitchMenu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME SWITCHES");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2;

	DrawMenu(&SwitchItems, &SwitchMenu[0]);
	DrawAllSwitchLights(SwitchItems.curpos);

	VW_UpdateScreen();
}

void DrawAllSwitchLights(
	std::int16_t which)
{
	std::int16_t i;
	std::uint16_t Shape;

	for (i = 0; i < SwitchItems.amount; i++)
	{
		if (SwitchMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (SwitchItems.cursor.on)
			{
				if (i == which)
				{ // Is the cursor sitting on this pic?
					Shape += 2;
				}
			}

			switch (i)
			{
			case SW_LIGHTING:
				if (!gp_no_shading())
				{
					Shape++;
				}
				break;

			case SW_REBA_ATTACK_INFO:
				if (!gp_hide_attacker_info())
				{
					Shape++;
				}
				break;

			case SW_CEILING:
				if (!gp_is_ceiling_solid())
				{
					Shape++;
				}
				break;

			case SW_FLOORS:
				if (!gp_is_flooring_solid())
				{
					Shape++;
				}
				break;

			// BBi
			case SW_NO_WALL_HIT_SOUND:
				if (gp_no_wall_hit_sfx())
				{
					++Shape;
				}
				break;

			case SW_ALWAYS_RUN:
				if (gp_is_always_run())
				{
					++Shape;
				}
				break;

			case SW_QUIT_ON_ESCAPE:
				if (gp_quit_on_escape())
				{
					++Shape;
				}
				break;

			case SW_HEART_BEAT_SOUND:
				if (gp_use_heart_beat_sfx())
				{
					++Shape;
				}
				break;

			case SW_ROTATED_AUTOMAP:
				if (am_rotatable())
				{
					++Shape;
				}
				break;
			}

			VWB_DrawPic(SwitchItems.x - 16, SwitchItems.y + i * SwitchItems.y_spacing - 1, Shape);
		}
	}

	DrawSwitchDescription(which);

}

void DrawSwitchDescription(
	std::int16_t which)
{
	const char* instr[] = {
		"TOGGLES LIGHT SOURCING IN HALLWAYS",
		"TOGGLES DETAILED ATTACKER INFO",
		"TOGGLES CEILING MAPPING",
		"TOGGLES FLOOR MAPPING",

		// BBi
		"TOGGLES WALL HIT SOUND",
		"TOGGLES ALWAYS RUN MODE",
		"ESC QUITS INSTEAD OF RETURNING TO GAME",
		"TOGGLES HEART BEAT SOUND WITH EKG",
		"TOGGLES <TAB>/<SHIFT+TAB> FUNCTIONS",
	};

	const auto& assets_info = get_assets_info();

	fontnumber = 2;

	WindowX = 48;
	WindowY = (assets_info.is_ps() ? 134 : 144);
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);
}

struct SoundDriverItem
{
	AudioDriverType type{};
	std::string name{};
}; // SoundDriverItem

constexpr auto sound_driver_count = 3;

const SoundDriverItem sound_drivers[sound_driver_count] =
{
	SoundDriverItem{AudioDriverType::auto_detect, "AUTO-DETECT"},
	SoundDriverItem{AudioDriverType::system, "SYSTEM"},
	SoundDriverItem{AudioDriverType::openal, "OPENAL"},
};

struct SoundOpl3TypeItem
{
	bstone::Opl3Type type{};
	std::string name{};
}; // SoundOpl3TypeItem

constexpr auto sound_opl3_type_count = 2;

const SoundOpl3TypeItem sound_opl3_types[sound_opl3_type_count] =
{
	SoundOpl3TypeItem{bstone::Opl3Type::dbopl, "DBOPL"},
	SoundOpl3TypeItem{bstone::Opl3Type::nuked, "NUKED"},
};

void digitized_sfx_carousel(
	int item_index,
	bool,
	bool)
{
	const auto old_sfx_type = sd_cfg_get_sfx_type();

	auto new_sfx_type = AudioSfxType{};

	switch (old_sfx_type)
	{
		case AudioSfxType::adlib:
			new_sfx_type = AudioSfxType::pc_speaker;
			break;

		case AudioSfxType::pc_speaker:
		default:
			new_sfx_type = AudioSfxType::adlib;
			break;
	}

	sd_cfg_set_sfx_type(new_sfx_type);
	sd_apply_sfx_type();

	DrawSoundMenu();
	DrawAllSoundLights(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

const char* get_sfx_type_string(
	AudioSfxType sfx_type)
{
	switch (sfx_type)
	{
		case AudioSfxType::pc_speaker:
			return "PC SPEAKER";

		case AudioSfxType::adlib:
		default:
			return "ADLIB";
	}
}

auto sound_driver_index = 0;

void initialize_sound_driver_index()
{
	switch (sd_get_audio_driver_type())
	{
		case AudioDriverType::system:
			sound_driver_index = 1;
			break;

		case AudioDriverType::openal:
			sound_driver_index = 2;
			break;

		case AudioDriverType::auto_detect:
		default:
			sound_driver_index = 0;
			break;
	}
}

void sound_driver_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	const auto delta = (is_left ? -1 : (is_right ? 1 : 0));

	sound_driver_index += delta;

	if (sound_driver_index < 0)
	{
		sound_driver_index = sound_driver_count - 1;
	}
	else if (sound_driver_index >= sound_driver_count)
	{
		sound_driver_index = 0;
	}

	sd_set_audio_driver_type(sound_drivers[sound_driver_index].type);
	sd_shutdown();
	sd_startup();

	DrawSoundMenu();
	DrawAllSoundLights(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

auto sound_opl3_type_index = 0;

void initialize_sound_opl3_type_index()
{
	switch (sd_get_opl3_type())
	{
		case bstone::Opl3Type::dbopl:
		default:
			sound_opl3_type_index = 0;
			break;

		case bstone::Opl3Type::nuked:
			sound_opl3_type_index = 1;
			break;
	}
}

void sound_opl3_type_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	const auto delta = (is_left ? -1 : (is_right ? 1 : 0));

	sound_opl3_type_index += delta;

	if (sound_opl3_type_index < 0)
	{
		sound_opl3_type_index = sound_opl3_type_count - 1;
	}
	else if (sound_opl3_type_index >= sound_opl3_type_count)
	{
		sound_opl3_type_index = 0;
	}

	sd_set_opl3_type(sound_opl3_types[sound_opl3_type_index].type);
	sd_shutdown();
	sd_startup();

	DrawSoundMenu();
	DrawAllSoundLights(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void CP_Sound(
	std::int16_t)
{
	initialize_sound_driver_index();
	initialize_sound_opl3_type_index();
	SndMenu[1].carousel_func_ = digitized_sfx_carousel;
	SndMenu[4].carousel_func_ = sound_driver_carousel;
	SndMenu[5].carousel_func_ = sound_opl3_type_carousel;

	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawSoundMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&SndItems, &SndMenu[0], DrawAllSoundLights);
		//
		// HANDLE MENU CHOICES
		//
		switch (which)
		{
			//
			// SOUND EFFECTS / DIGITIZED SOUND
			//
		case 0:
			sd_wait_sound_done();
			sd_enable_sound(!sd_is_sound_enabled());

			DrawSoundMenu();

			if (sd_is_sound_enabled())
			{
				ShootSnd();
			}

			break;

		case 2:
			sd_wait_sound_done();
			sd_cfg_set_is_sfx_digitized(!sd_cfg_get_is_sfx_digitized());
			apply_digitized_sfx();
			DrawSoundMenu();
			ShootSnd();
			break;

		case 3:
			sd_enable_music(!sd_is_music_enabled());

			if (sd_is_music_enabled())
			{
				StartCPMusic(MENUSONG);
			}

			DrawSoundMenu();
			ShootSnd();

			break;

		default:
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void DrawSoundMenu()
{
	//
	// DRAW SOUND MENU
	//

	ClearMScreen();
	DrawMenuTitle("SOUND SETTINGS");
	DrawInstructions(IT_STANDARD);

	//
	// IF NO ADLIB, NON-CHOOSENESS!
	//

	if (!sd_has_audio())
	{
		SndMenu[0].active = AT_DISABLED;
		SndMenu[3].active = AT_DISABLED;
	}

	fontnumber = 2;
	DrawMenu(&SndItems, &SndMenu[0]);

	DrawAllSoundLights(SndItems.curpos);

	VW_UpdateScreen();
}

void DrawAllSoundLights(
	std::int16_t which)
{
	std::int16_t i;
	std::uint16_t Shape;

	for (i = 0; i < SndItems.amount; i++)
	{
		if (SndMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (SndItems.cursor.on)
			{
				if (i == which)
				{ // Is the cursor sitting on this pic?
					Shape += 2;
				}
			}

			switch (i)
			{
			case 0:
				if (sd_is_sound_enabled())
				{
					++Shape;
				}
				break;

			case 1:
				draw_carousel(
					i,
					&SndItems,
					SndMenu,
					get_sfx_type_string(sd_cfg_get_sfx_type())
				);
				continue;

			case 2:
				if (sd_cfg_get_is_sfx_digitized())
				{
					++Shape;
				}
				break;

			case 3:
				if (sd_is_music_enabled())
				{
					++Shape;
				}
				break;

			case 4:
				draw_carousel(
					i,
					&SndItems,
					SndMenu,
					sound_drivers[sound_driver_index].name
				);
				continue;

			case 5:
				draw_carousel(
					i,
					&SndItems,
					SndMenu,
					sound_opl3_types[sound_opl3_type_index].name
				);
				continue;

			default:
				continue;
			}

			VWB_DrawPic(SndItems.x - 16, SndItems.y + i * SndItems.y_spacing - 1, Shape);
		}
	}
}

char LOADSAVE_GAME_MSG[2][25] = {
	"^ST1^CELoading Game\r^XX",
	"^ST1^CESaving Game\r^XX"
};

// --------------------------------------------------------------------------
// DrawLSAction() - DRAW LOAD/SAVE IN PROGRESS
// --------------------------------------------------------------------------
void DrawLSAction(
	std::int16_t which)
{
	std::int8_t total[] = {19, 19};

	VW_FadeOut();
	screenfaded = true;
	DrawTopInfo(static_cast<sp_type>(sp_loading + which));

	VL_Bar(
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
		BLACK);

	DisplayPrepingMsg(LOADSAVE_GAME_MSG[which]);

	if (which)
	{
		PreloadUpdate(1, 1); // GFX: bar is full when saving...

	}
	WindowY = 181;
}

std::int16_t CP_LoadGame(
	std::int16_t quick)
{
	std::int16_t which;
	std::int16_t exit = 0;

	//
	// QUICKLOAD?
	//
	if (quick)
	{
		which = LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			DrawLSAction(0); // Testing...

			auto name_path = get_profile_dir() + name;

			loadedgame = LoadTheGame(name_path);
			return loadedgame;
		}
	}

restart:

	DrawLoadSaveScreen(0);

	do
	{
		which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);

		if (which >= 0 && SaveGamesAvail[which])
		{
			ShootSnd();

			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			auto name_path = get_profile_dir() + name;

			DrawLSAction(0);

			if (!LoadTheGame(name_path))
			{
				exit = 0;
				StartGame = 0;
				loadedgame = 0;
				playstate = ex_abort;
				goto restart;
			}

			loadedgame = true;
			StartGame = true;

			ShootSnd();

			//
			// CHANGE "READ THIS!" TO NORMAL COLOR
			//
			MainMenu[MM_READ_THIS].active = AT_ENABLED;
			exit = 1;
			break;
		}
	} while (which >= 0);

	if (which == -1)
	{
		MenuFadeOut();
	}

	if (loadedgame)
	{
		refresh_screen = false;
	}

	return exit;
}


///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(
	std::int16_t w)
{
	static std::int16_t lastgameon = 0;

	PrintLSEntry(lastgameon, ENABLED_TEXT_COLOR);
	PrintLSEntry(w, HIGHLIGHT_TEXT_COLOR);

	lastgameon = w;
}

void DrawLoadSaveScreen(
	std::int16_t loadsave)
{
	std::int16_t i;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();

	fontnumber = 1;

	if (!loadsave)
	{
		DrawMenuTitle("Load Mission");
	}
	else
	{
		DrawMenuTitle("Save Mission");
	}

	DrawInstructions(IT_STANDARD);

	for (i = 0; i < 10; i++)
	{
		PrintLSEntry(i, ENABLED_TEXT_COLOR);
	}

	fontnumber = 4;
	DrawMenu(&LSItems, &LSMenu[0]);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}

// --------------------------------------------------------------------------
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
// --------------------------------------------------------------------------
void PrintLSEntry(
	std::int16_t w,
	std::int16_t color)
{
	SETFONTCOLOR(color, BKGDCOLOR);
	DrawOutline(LSM_X + LSItems.indent, LSM_Y + w * LSItems.y_spacing - 2, LSM_W - LSItems.indent, 8, color, color);

	fontnumber = 2;

	PrintX = LSM_X + LSItems.indent + 2;
	PrintY = LSM_Y + w * LSItems.y_spacing;

	if (SaveGamesAvail[w])
	{
		US_Print(SaveGameNames[w]);
	}
	else
	{
		US_Print("       ----- EMPTY -----");
	}

	fontnumber = 1;
}

std::int16_t CP_SaveGame(
	std::int16_t quick)
{
	std::int16_t which, exit = 0;
	char input[GAME_DESCRIPTION_LEN + 1];
	bool temp_caps = allcaps;
	US_CursorStruct TermCursor = {'@', 0, HIGHLIGHT_TEXT_COLOR, 2};

	allcaps = true;
	use_custom_cursor = true;
	US_CustomCursor = TermCursor;

	//
	// QUICKSAVE?
	//
	if (quick)
	{
		which = LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			DrawLSAction(1); // Testing...
			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			auto name_path = get_profile_dir() + name;

			SaveTheGame(name_path, &SaveGameNames[which][0]);

			return 1;
		}
	}

	DrawLoadSaveScreen(1);

	do
	{
		which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);
		if (which >= 0)
		{
			//
			// OVERWRITE EXISTING SAVEGAME?
			//
			if (SaveGamesAvail[which])
			{
				if (!Confirm(GAMESVD))
				{
					DrawLoadSaveScreen(1);
					continue;
				}
				else
				{
					DrawLoadSaveScreen(1);
					PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
					VW_UpdateScreen();
				}
			}

			ShootSnd();

			strcpy(input, &SaveGameNames[which][0]);

			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			fontnumber = 2;
			VWB_Bar(LSM_X + LSItems.indent + 1, LSM_Y + which * LSItems.y_spacing - 1, LSM_W - LSItems.indent - 1, 7, HIGHLIGHT_BOX_COLOR);
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, HIGHLIGHT_BOX_COLOR);
			VW_UpdateScreen();


			if (US_LineInput(LSM_X + LSItems.indent + 2, LSM_Y + which * LSItems.y_spacing, input, input, true, GAME_DESCRIPTION_LEN, LSM_W - LSItems.indent - 10))
			{
				SaveGamesAvail[which] = 1;
				strcpy(&SaveGameNames[which][0], input);

				DrawLSAction(1);

				auto name_path = get_profile_dir() + name;
				SaveTheGame(name_path, input);

				ShootSnd();
				exit = 1;
			}
			else
			{
				VWB_Bar(
					LSM_X + LSItems.indent + 1,
					LSM_Y + which * LSItems.y_spacing - 1,
					LSM_W - LSItems.indent - 1,
					7,
					menu_background_color);

				PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
				VW_UpdateScreen();
				menu_play_esc_pressed_sound();
				continue;
			}

			fontnumber = 1;
			break;
		}

	} while (which >= 0);

	MenuFadeOut();
	use_custom_cursor = false;
	allcaps = temp_caps;
	return exit;
}

void CP_ExitOptions(
	std::int16_t)
{
	StartGame = 1;
}

void CP_Control(
	std::int16_t)
{
#ifdef __vita__
	MouseSensitivity(0);
	return;
#endif

	enum
	{
		MOUSEENABLE,
		MOUSESENS,
		CUSTOMIZE,
	};

	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);

	DrawCtlScreen();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&CtlItems, &CtlMenu[0], nullptr);
		switch (which)
		{
		case MOUSEENABLE:
			in_set_is_mouse_enabled(!in_is_mouse_enabled());
			DrawCtlScreen();
			CusItems.curpos = -1;
			ShootSnd();
			break;

		case MOUSESENS:
		case CUSTOMIZE:
			DrawCtlScreen();
			MenuFadeIn();
			WaitKeyUp();
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void DrawMousePos()
{
	const int thumb_width = 16;
	const int track_width = 160;
	const int slide_width = track_width - thumb_width;
	const int max_mouse_delta = max_mouse_sensitivity - min_mouse_sensitivity;

	VWB_Bar(
		74,
		92,
		track_width,
		8,
		HIGHLIGHT_BOX_COLOR);

	DrawOutline(
		73,
		91,
		track_width + 1,
		9,
		ENABLED_TEXT_COLOR,
		ENABLED_TEXT_COLOR);

	VWB_Bar(
		74 + ((slide_width * in_get_mouse_sensitivity()) / max_mouse_delta),
		92,
		thumb_width,
		8,
		HIGHLIGHT_TEXT_COLOR);
}

void DrawMouseSens()
{
	ClearMScreen();
#ifdef __vita__
	DrawMenuTitle("JOYSTICK SENSITIVITY");
#else
	DrawMenuTitle("MOUSE SENSITIVITY");
#endif
	DrawInstructions(IT_MOUSE_SEN);

	fontnumber = 4;

	SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
	PrintX = 36;
	PrintY = 91;
	US_Print("SLOW");
	PrintX = 242;
	US_Print("FAST");

	DrawMousePos();

	VW_UpdateScreen();
	MenuFadeIn();
}

void MouseSensitivity(
	std::int16_t)
{
	ControlInfo ci;
	std::int16_t exit = 0;

	const auto oldMA = in_get_mouse_sensitivity();

	DrawMouseSens();
	do
	{
		ReadAnyControl(&ci);
		switch (ci.dir)
		{
		case dir_North:
		case dir_West:
			if (in_get_mouse_sensitivity() > 0)
			{
				in_set_mouse_sensitivity(in_get_mouse_sensitivity() - 1);
				DrawMousePos();
				VW_UpdateScreen();
				menu_play_move_gun_1_sound();

				while (Keyboard[ScanCode::sc_left_arrow])
				{
					in_handle_events();
				}

				WaitKeyUp();
			}
			break;

		case dir_South:
		case dir_East:
			if (in_get_mouse_sensitivity() < max_mouse_sensitivity)
			{
				in_set_mouse_sensitivity(in_get_mouse_sensitivity() + 1);
				DrawMousePos();
				VW_UpdateScreen();
				menu_play_move_gun_1_sound();

				while (Keyboard[ScanCode::sc_right_arrow])
				{
					in_handle_events();
				}

				WaitKeyUp();
			}
			break;

		default:
			break;
		}

		if (ci.button0 || Keyboard[ScanCode::sc_space] || Keyboard[ScanCode::sc_return])
		{
			exit = 1;
		}
		else if (ci.button1 || Keyboard[ScanCode::sc_escape])
		{
			exit = 2;
		}

	} while (!exit);

	if (exit == 2)
	{
		in_set_mouse_sensitivity(oldMA);
		menu_play_esc_pressed_sound();
	}
	else
	{
		ShootSnd();
	}

	WaitKeyUp();
	MenuFadeOut();
}

// --------------------------------------------------------------------------
// DrawCtlScreen() - DRAW CONTROL MENU SCREEN
// --------------------------------------------------------------------------
void DrawCtlScreen()
{
	const std::int16_t Y_CTL_PIC_OFS = 3;

	std::int16_t i;
	std::int16_t x;
	std::int16_t y;

	ClearMScreen();
	DrawMenuTitle("CONTROL");
	DrawInstructions(IT_STANDARD);

	WindowX = 0;
	WindowW = 320;
	SETFONTCOLOR(TEXTCOLOR, BKGDCOLOR);

	if (MousePresent)
	{
		CtlMenu[0].active = AT_ENABLED;
		CtlMenu[1].active = AT_ENABLED;
	}

	CtlMenu[1].active = static_cast<activetypes>(in_is_mouse_enabled());

	fontnumber = 4;
	DrawMenu(&CtlItems, &CtlMenu[0]);

	x = CTL_X + CtlItems.indent - 24;
	y = CTL_Y + Y_CTL_PIC_OFS;
	if (in_is_mouse_enabled())
	{
		VWB_DrawPic(x, y, C_SELECTEDPIC);
	}
	else
	{
		VWB_DrawPic(x, y, C_NOTSELECTEDPIC);
	}

	//
	// PICK FIRST AVAILABLE SPOT
	//

	if (CtlItems.curpos < 0 || !CtlMenu[static_cast<int>(CtlItems.curpos)].active)
	{
		for (i = 0; i < CtlItems.amount; ++i)
		{
			if (CtlMenu[i].active)
			{
				CtlItems.curpos = static_cast<std::int8_t>(i);
				break;
			}
		}
	}

	DrawMenuGun(&CtlItems);
	VW_UpdateScreen();
}

void CustomControls(
	std::int16_t)
{
	binds_draw_menu();
}

void CP_Quit()
{
	if (Confirm(QuitToDosStr))
	{
		ExitGame();
	}

	DrawMainMenu();
}

// ---------------------------------------------------------------------------
// Clear Menu screens to dark red
// ---------------------------------------------------------------------------
void ClearMScreen()
{
	VWB_Bar(SCREEN_X, SCREEN_Y, SCREEN_W, SCREEN_H, menu_background_color);
}

// ---------------------------------------------------------------------------
// Un/Cache a LUMP of graphics
// ---------------------------------------------------------------------------
void CacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend)
{
	std::int16_t i;

	for (i = lumpstart; i <= lumpend; i++)
	{
		CA_CacheGrChunk(i);
	}
}

void UnCacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend)
{
	std::int16_t i;

	for (i = lumpstart; i <= lumpend; i++)
	{
		FREEFONT(i);
	}
}

void DrawWindow(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t wcolor)
{
	VWB_Bar(x, y, w, h, static_cast<std::uint8_t>(wcolor));
	DrawOutline(x, y, w, h, BORD2COLOR, DEACTIVE);
}

void DrawOutline(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t color1,
	std::int16_t color2)
{
	VWB_Hlin(x, x + w, y, static_cast<std::uint8_t>(color2));
	VWB_Vlin(y, y + h, x, static_cast<std::uint8_t>(color2));
	VWB_Hlin(x, x + w, y + h, static_cast<std::uint8_t>(color1));
	VWB_Vlin(y, y + h, x + w, static_cast<std::uint8_t>(color1));
}

void SetupControlPanel()
{
	const auto& assets_info = get_assets_info();

	// BBi
	SwitchItems.amount = (assets_info.is_ps() ? 7 : 9);
	SwitchItems.y = MENU_Y + (assets_info.is_ps() ? 11 : 3);
	// BBi

	ControlPanelAlloc();

	fontnumber = 2;

	WindowH = 200;

	if (ingame)
	{
		MainMenu[MM_SAVE_MISSION].active = AT_ENABLED;
	}

	ReadGameNames();
}

void ReadGameNames()
{
	for (int i = 0; i < 10; ++i)
	{
		auto name = get_saved_game_base_name();
		name += static_cast<char>('0' + i);

		auto name_path = get_profile_dir() + name;

		bstone::FileStream stream{};

		if (!stream.try_open(name_path.c_str()))
		{
			continue;
		}

		SaveGamesAvail[i] = 1;

		int chunk_size = bstone::sg_find_chunk(bstone::SgKnownFourCc::desc(), stream);

		if (chunk_size > 0)
		{
			char temp[GAME_DESCRIPTION_LEN + 1];

			std::fill_n(
				temp,
				GAME_DESCRIPTION_LEN,
				'\0');

			auto temp_size = std::min(GAME_DESCRIPTION_LEN, chunk_size);

			stream.read(temp, temp_size);

			strcpy(&SaveGameNames[i][0], temp);
		}
		else
		{
			strcpy(&SaveGameNames[i][0], "DESCRIPTION LOST");
		}
	}
}

void CleanupControlPanel()
{
	if (!loadedgame)
	{
		FreeMusic();
	}
	ControlPanelFree();
	fontnumber = 4;
}

// ---------------------------------------------------------------------------
// ControlPanelFree() - This FREES the control panel lump from memory
//      and REALLOCS the ScaledDirectory
// ---------------------------------------------------------------------------
void ControlPanelFree()
{
	UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
	NewViewSize();
}

// ---------------------------------------------------------------------------
// ControlPanelAlloc() - This CACHEs the control panel lump into memory
//      and FREEs the ScaledDirectory.
// ---------------------------------------------------------------------------
void ControlPanelAlloc()
{
	CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
}

// ---------------------------------------------------------------------------
// ShadowPrint() - Shadow Prints given text @ a given x & y in default font
//
// NOTE: Font MUST already be loaded
// ---------------------------------------------------------------------------
void ShadowPrint(
	const char* strng,
	std::int16_t x,
	std::int16_t y)
{
	std::int16_t old_bc, old_fc;

	old_fc = fontcolor;
	old_bc = backcolor;

	PrintX = x + 1;
	PrintY = y + 1;

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_Print(strng);

	PrintX = x;
	PrintY = y;
	SETFONTCOLOR(old_fc, old_bc);
	US_Print(strng);
}

// ---------------------------------------------------------------------------
// HandleMenu() - Handle moving gun around a menu
// ---------------------------------------------------------------------------
std::int16_t HandleMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	void(*routine)(std::int16_t w))
{
#define box_on item_i->cursor.on
	std::int8_t key;
	static std::int16_t redrawitem = 1;

	std::int16_t i, x, y, basey, exit, which, flash_tics;
	ControlInfo ci;

	which = item_i->curpos;
	x = item_i->x;
	basey = item_i->y;
	y = basey + which * item_i->y_spacing;
	box_on = 1;
	DrawGun(item_i, items, x, &y, which, basey, routine);

	SetTextColor(items + which, 1);

	if (redrawitem)
	{
		ShadowPrint(
			(items + which)->string.c_str(),
			item_i->x + item_i->indent,
			item_i->y + which * item_i->y_spacing);
	}

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
	{
		routine(which);
	}

	VW_UpdateScreen();

	flash_tics = 40;
	exit = 0;
	TimeCount = 0;
	IN_ClearKeysDown();

	do
	{
		CalcTics();
		flash_tics -= tics;

		CycleColors();

		//
		// CHANGE GUN SHAPE
		//

		if (flash_tics <= 0)
		{
			flash_tics = 40;

			box_on ^= 1;

			if (box_on)
			{
				DrawGun(item_i, items, x, &y, which, basey, routine);
			}
			else
			{
				EraseGun(item_i, items, x, y, which);
				if (routine)
				{
					routine(which);
				}
			}


			VW_UpdateScreen();
		}

		CheckPause();


		//
		// SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
		//

		key = LastASCII;
		if (key)
		{
			std::int16_t ok = 0;

			if (key >= 'a')
			{
				key -= 'a' - 'A';
			}

			for (i = which + 1; i < item_i->amount; i++)
			{
				if ((items + i)->active && (items + i)->string[0] == key)
				{
					EraseGun(item_i, items, x, y, which);
					which = i;
					item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing
					box_on = 1;
					DrawGun(item_i, items, x, &y, which, basey, routine);
					VW_UpdateScreen();

					ok = 1;
					IN_ClearKeysDown();
					break;
				}
			}

			//
			// DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
			//

			if (!ok)
			{
				for (i = 0; i < which; i++)
				{
					if ((items + i)->active && (items + i)->string[0] == key)
					{
						EraseGun(item_i, items, x, y, which);
						which = i;
						item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing
						box_on = 1;
						DrawGun(item_i, items, x, &y, which, basey, routine);
						VW_UpdateScreen();

						IN_ClearKeysDown();
						break;
					}
				}
			}
		}

		//
		// GET INPUT
		//

		ReadAnyControl(&ci);

		switch (ci.dir)
		{
			// ------------------------
			// MOVE UP
			//
		case dir_North:
			EraseGun(item_i, items, x, y, which);

			do
			{
				if (!which)
				{
					which = item_i->amount - 1;
				}
				else
				{
					which--;
				}

			} while (!(items + which)->active);

			item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing

			box_on = 1;
			DrawGun(item_i, items, x, &y, which, basey, routine);

			VW_UpdateScreen();

			TicDelay(20);
			break;

			// --------------------------
			// MOVE DOWN
			//
		case dir_South:
			EraseGun(item_i, items, x, y, which);

			do
			{
				if (which == item_i->amount - 1)
				{
					which = 0;
				}
				else
				{
					which++;
				}
			} while (!(items + which)->active);

			item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing

			box_on = 1;
			DrawGun(item_i, items, x, &y, which, basey, routine);

			VW_UpdateScreen();

			TicDelay(20);
			break;

			// Carousel (left).
			//
			case dir_West:
			{
				auto carousel_func = items[which].carousel_func_;

				if (carousel_func)
				{
					carousel_func(which, true, false);
				}

				break;
			}

			// Carousel (right).
			//
			case dir_East:
			{
				auto carousel_func = items[which].carousel_func_;

				if (carousel_func)
				{
					carousel_func(which, false, true);
				}

				break;
			}

		default:
			break;
		}

		if (ci.button0 || Keyboard[ScanCode::sc_space] || Keyboard[ScanCode::sc_return])
		{
			exit = 1;
		}

		if (ci.button1 || Keyboard[ScanCode::sc_escape])
		{
			exit = 2;
		}

	} while (!exit);

	IN_ClearKeysDown();

	//
	// ERASE EVERYTHING
	//

	box_on = 0;
	redrawitem = 1;
	EraseGun(item_i, items, x, y, which);

	if (routine)
	{
		routine(which);
	}

	VW_UpdateScreen();

	item_i->curpos = static_cast<std::int8_t>(which);

	switch (exit)
	{
	case 1:
		//
		// CALL THE ROUTINE
		//
		if ((items + which)->routine)
		{
			// Make sure there's room to save when CP_SaveGame() is called.
			//
			if (reinterpret_cast<std::size_t>(items[which].routine) == reinterpret_cast<std::size_t>(CP_SaveGame))
			{
				if (!CheckDiskSpace(DISK_SPACE_NEEDED, CANT_SAVE_GAME_TXT, cds_menu_print))
				{
					return which;
				}
			}

			//
			// ALREADY IN A GAME?
			//
			const auto& assets_info = get_assets_info();

			if (assets_info.is_ps() && ingame && ((items + which)->routine == CP_NewGame))
			{
				if (!Confirm(CURGAME))
				{
					MenuFadeOut();
					return 0;
				}
			}

			ShootSnd();
			MenuFadeOut();
			(items + which)->routine(0);
		}
		return which;

	case 2:
		menu_play_esc_pressed_sound();
		return -1;
	}

	return 0;     // JUST TO SHUT UP THE ERROR MESSAGES!
}

// ---------------------------------------------------------------------------
// ERASE GUN & DE-HIGHLIGHT STRING
// ---------------------------------------------------------------------------
void EraseGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t y,
	std::int16_t which)
{
	static_cast<void>(x);

	VWB_Bar(item_i->cursor.x, y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, menu_background_color);
	SetTextColor(items + which, 0);

	ShadowPrint((items + which)->string.c_str(), item_i->x + item_i->indent, y);
}

// ---------------------------------------------------------------------------
// DrawGun() - DRAW GUN AT NEW POSITION
// ---------------------------------------------------------------------------
void DrawGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t* y,
	std::int16_t which,
	std::int16_t basey,
	void(*routine)(std::int16_t w))
{
	static_cast<void>(x);

	*y = basey + which * item_i->y_spacing;

	VWB_Bar(item_i->cursor.x, *y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, HIGHLIGHT_BOX_COLOR);
	SetTextColor(items + which, 1);

	ShadowPrint(
		(items + which)->string.c_str(),
		item_i->x + item_i->indent,
		item_i->y + which * item_i->y_spacing);

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
	{
		routine(which);
	}
}

// ---------------------------------------------------------------------------
// TicDelay() - DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
// ---------------------------------------------------------------------------
void TicDelay(
	std::int16_t count)
{
	ControlInfo ci;

	TimeCount = 0;

	do
	{
		ReadAnyControl(&ci);
	} while (TimeCount < count && ci.dir != dir_None);
}

// ---------------------------------------------------------------------------
// DrawMenu() - Draw a menu
//
//       This also calculates the Y position of the current items in the
//                      CP_itemtype structures.
// ---------------------------------------------------------------------------
void DrawMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items)
{
	std::int16_t i, which = item_i->curpos;

	WindowX = PrintX = item_i->x + item_i->indent;
	WindowY = PrintY = item_i->y;

	WindowW = 320;
	WindowH = 200;

	for (i = 0; i < item_i->amount; i++)
	{
		SetTextColor(items + i, which == i);

		ShadowPrint(
			(items + i)->string.c_str(),
			static_cast<std::int16_t>(WindowX),
			item_i->y + i * item_i->y_spacing);
	}
}

// ---------------------------------------------------------------------------
// SET TEXT COLOR (HIGHLIGHT OR NO)
// ---------------------------------------------------------------------------
void SetTextColor(
	CP_itemtype* items,
	std::int16_t hlight)
{
	if (hlight)
	{
		SETFONTCOLOR(color_hlite[items->active], TERM_BACK_COLOR);
	}
	else
	{
		SETFONTCOLOR(color_norml[items->active], TERM_BACK_COLOR);
	}
}

// ---------------------------------------------------------------------------
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
// ---------------------------------------------------------------------------
void WaitKeyUp()
{
	for (auto quit = false; !quit; )
	{
		ControlInfo ci;

		ReadAnyControl(&ci);

		quit = !(
			ci.button0 != 0 ||
			ci.button1 != 0 ||
			ci.button2 != 0 ||
			ci.button3 != 0 ||
			Keyboard[ScanCode::sc_space] ||
			Keyboard[ScanCode::sc_return] ||
			Keyboard[ScanCode::sc_escape]);
	}
}

// ---------------------------------------------------------------------------
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
// ---------------------------------------------------------------------------
void ReadAnyControl(
	ControlInfo* ci)
{
	IN_ReadControl(0, ci);

	//
	// UNDO some of the ControlInfo vars that were init
	// with IN_ReadControl() for the mouse...
	//
	if (ControlTypeUsed == ctrl_Mouse)
	{
		//
		// Clear directions & buttons (if enabled or not)
		//
		ci->dir = dir_None;
		ci->button0 = 0;
		ci->button1 = 0;
		ci->button2 = 0;
		ci->button3 = 0;
	}

	if (in_is_mouse_enabled())
	{
		int mousex;
		int mousey;

		// READ MOUSE MOTION COUNTERS
		// RETURN DIRECTION
		// HOME MOUSE
		// CHECK MOUSE BUTTONS

		in_get_mouse_deltas(mousex, mousey);
		in_clear_mouse_deltas();

		const int DELTA_THRESHOLD = 10;

		if (mousey < -DELTA_THRESHOLD)
		{
			ci->dir = dir_North;
		}
		else if (mousey > DELTA_THRESHOLD)
		{
			ci->dir = dir_South;
		}

		if (mousex < -DELTA_THRESHOLD)
		{
			ci->dir = dir_West;
		}
		else if (mousex > DELTA_THRESHOLD)
		{
			ci->dir = dir_East;
		}

		int buttons = IN_MouseButtons();

		if (buttons != 0)
		{
			ci->button0 = buttons & 1;
			ci->button1 = buttons & 2;
			ci->button2 = buttons & 4;
			ci->button3 = false;
		}
	}
}

////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
std::int16_t Confirm(
	const char* string)
{
	std::int16_t xit = 0, x, y, tick = 0, whichsnd[2] = {ESCPRESSEDSND, SHOOTSND};


	Message(string);

	// Next two lines needed for flashing cursor ...
	//
	SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
	CA_CacheGrChunk(STARTFONT + fontnumber);

	IN_ClearKeysDown();

	//
	// BLINK CURSOR
	//
	x = static_cast<std::int16_t>(PrintX);
	y = static_cast<std::int16_t>(PrintY);
	TimeCount = 0;
	do
	{
		if (TimeCount >= 10)
		{
			switch (tick)
			{
			case 0:
				VWB_Bar(x, y, 8, 13, BORDER_MED_COLOR);
				break;

			case 1:
				PrintX = x;
				PrintY = y;
				US_Print("_");
			}

			VW_UpdateScreen();
			tick ^= 1;
			TimeCount = 0;
		}

		// BBi
		IN_CheckAck();
	} while (!Keyboard[ScanCode::sc_y] && !Keyboard[ScanCode::sc_n] && !Keyboard[ScanCode::sc_escape]);


	if (Keyboard[ScanCode::sc_y])
	{
		xit = 1;
		ShootSnd();
	}

	while (Keyboard[ScanCode::sc_y] || Keyboard[ScanCode::sc_n] || Keyboard[ScanCode::sc_escape])
	{
		IN_CheckAck();
	}

	IN_ClearKeysDown();
	menu_play_sound(whichsnd[xit]);
	FREEFONT(STARTFONT + fontnumber);

	return xit;
}

// ---------------------------------------------------------------------------
// PRINT A MESSAGE IN A WINDOW
// ---------------------------------------------------------------------------
void Message(
	const char* string)
{
	std::int16_t h = 0, w = 0, mw = 0;
	std::size_t i;
	fontstruct* font;

	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT + 1);

	font = reinterpret_cast<fontstruct*>(grsegs[STARTFONT + fontnumber].data());

	h = font->height;
	for (i = 0; i < strlen(string); i++)
	{
		if (string[i] == '\n')
		{
			if (w > mw)
			{
				mw = w;
			}
			w = 0;
			h += font->height;
		}
		else
		{
			w += font->width[static_cast<int>(string[i])];
		}
	}

	if (w + 10 > mw)
	{
		mw = w + 10;
	}

	const auto OldPrintY = PrintY = (WindowH / 2) - h / 2;
	const auto OldPrintX = PrintX = WindowX = 160 - mw / 2;

	// bump down and to right for shadow

	PrintX++;
	PrintY++;
	WindowX++;

	BevelBox(
		static_cast<std::int16_t>(WindowX - 6),
		static_cast<std::int16_t>(PrintY - 6),
		mw + 10,
		h + 10,
		BORDER_HI_COLOR,
		BORDER_MED_COLOR,
		BORDER_LO_COLOR);

	SETFONTCOLOR(BORDER_LO_COLOR, BORDER_MED_COLOR);
	US_Print(string);

	PrintY = OldPrintY;
	WindowX = PrintX = OldPrintX;

	SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
	US_Print(string);

	FREEFONT(STARTFONT + 1);

	VW_UpdateScreen();
}

// --------------------------------------------------------------------------
// Searches for an "^XX" and replaces with a 0 (NULL)
// --------------------------------------------------------------------------
void TerminateStr(char* pos)
{
	pos = strstr(pos, "^XX");

	if (pos != nullptr)
	{
		*pos = '\0';
	}
}

// ---------------------------------------------------------------------------
// Caches and prints a message in a window.
// ---------------------------------------------------------------------------
void CacheMessage(
	std::uint16_t MessageNum)
{
	char* string;

	CA_CacheGrChunk(MessageNum);
	string = (char*)grsegs[MessageNum].data();

	TerminateStr(string);

	Message(string);

	FREEFONT(MessageNum);
}

void StartCPMusic(
	std::int16_t song)
{
	int chunk;

	lastmenumusic = song;

	sd_music_off();
	chunk = song;
	sd_start_music(chunk);
}

void FreeMusic()
{
	sd_music_off();
}


#ifdef CACHE_KEY_DATA
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
std::uint8_t far* IN_GetScanName(
	ScanCode scan)
{
	std::uint8_t* p;
	ScanCode* s;

	for (s = ExtScanCodes, p = ExtScanNames; *s; p += 7, s++)
	{
		if (*s == scan)
		{
			return (std::uint8_t*)p;
		}
	}

	return (std::uint8_t*)(ScanNames + (scan << 1));
}
#else
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
const std::string& IN_GetScanName(
	ScanCode scan)
{
	for (auto i = 0; ext_scan_codes[i] != ScanCode::sc_none; ++i)
	{
		if (ext_scan_codes[i] == scan)
		{
			return ext_scan_names[i];
		}
	}

	return scan_names[static_cast<int>(scan)];
}
#endif

// ---------------------------------------------------------------------------
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
// ---------------------------------------------------------------------------
void CheckPause()
{
	if (Paused)
	{
		switch (SoundStatus)
		{
		case 0:
			sd_music_on(true);
			break;

		case 1:
			sd_music_off();
			break;
		}

		SoundStatus ^= 1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused = false;
	}
}

// -------------------------------------------------------------------------
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
// -------------------------------------------------------------------------
void DrawMenuGun(
	CP_iteminfo* iteminfo)
{
	std::int16_t x, y;

	x = iteminfo->cursor.x;
	y = iteminfo->y + iteminfo->curpos * iteminfo->y_spacing + iteminfo->cursor.y_ofs;

	VWB_Bar(x, y, iteminfo->cursor.width, iteminfo->cursor.height, HIGHLIGHT_BOX_COLOR);
}

void ShootSnd()
{
	sd_play_ui_sound(SHOOTSND);
}

void ShowPromo()
{
	vid_is_movie = true;

	const auto PROMO_MUSIC = HIDINGA_MUS;

	// Load and start music
	//
	sd_start_music(PROMO_MUSIC);

	// Show promo screen 1
	//
	MenuFadeOut();
	CA_CacheScreen(PROMO1PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase * 20);

	// Show promo screen 2
	//
	MenuFadeOut();
	CA_CacheScreen(PROMO2PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase * 20);

	// Music off and freed!
	//
	StopMusic();

	vid_is_movie = false;
}

void ExitGame()
{
	VW_FadeOut();

	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw_v3_0() && !gp_no_intro_outro() && !gp_no_screens())
	{
		ShowPromo();
	}

	sd_music_off();
	sd_stop_sfx_sound();
	Quit();
}

// BBi
int volume_index = 0;

struct Volume
{
	using Get = int (*)();
	Get get;

	using Set = void (*)(int);
	Set set;
};

Volume const volumes[2] = {
	{sd_get_sfx_volume, sd_set_sfx_volume},
	{sd_get_music_volume, sd_set_music_volume}};

void draw_volume_control(
	int index,
	int volume,
	bool is_enabled)
{
	std::int16_t slider_color = is_enabled ? ENABLED_TEXT_COLOR : DISABLED_TEXT_COLOR;

	std::int16_t outline_color = is_enabled ? HIGHLIGHT_TEXT_COLOR : DEACTIAVED_TEXT_COLOR;

	int y = 82 + (index * 40);

	VWB_Bar(74, static_cast<std::int16_t>(y), 160, 8, HIGHLIGHT_BOX_COLOR);

	DrawOutline(
		73,
		static_cast<std::int16_t>(y - 1),
		161,
		9,
		outline_color,
		outline_color
	);

	VWB_Bar(
		static_cast<std::int16_t>(74 + (((160 - 16) * volume) / sd_max_volume)),
		static_cast<std::int16_t>(y),
		16,
		8,
		static_cast<std::uint8_t>(slider_color)
	);
}

void draw_volume_controls()
{
	for (int i = 0; i < 2; ++i)
	{
		draw_volume_control(i, volumes[i].get(), i == volume_index);
	}
}

void cp_sound_volume(
	std::int16_t)
{
	ClearMScreen();
	DrawMenuTitle("SOUND VOLUME");
	DrawInstructions(IT_SOUND_VOLUME);

	fontnumber = 4;

	SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);

	PrintX = 150;
	PrintY = 60;
	US_Print("SFX");

	PrintX = 145;
	PrintY = 105;
	US_Print("MUSIC");

	for (int i = 0; i < 2; ++i)
	{
		PrintX = 36;
		PrintY = static_cast<std::uint16_t>(81 + (i * 40));
		US_Print("MUTE");

		PrintX = 242;
		US_Print("LOUD");
	}

	draw_volume_controls();

	VW_UpdateScreen();
	MenuFadeIn();

	ControlInfo ci;

	int old_volumes[2] = {-1, -1};

	for (bool quit = false; !quit; )
	{
		bool update_volumes = false;
		bool redraw_controls = false;

		ReadAnyControl(&ci);

		switch (ci.dir)
		{
		case dir_North:
			if (volume_index == 1)
			{
				redraw_controls = true;
				volume_index = 0;
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_up_arrow])
			{
				in_handle_events();
			}
			break;

		case dir_South:
			if (volume_index == 0)
			{
				redraw_controls = true;
				volume_index = 1;
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_down_arrow])
			{
				in_handle_events();
			}
			break;

		case dir_West:
			if (volumes[volume_index].get() > sd_min_volume)
			{
				redraw_controls = true;
				update_volumes = true;
				volumes[volume_index].set(volumes[volume_index].get() - 1);
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_left_arrow])
			{
				in_handle_events();
			}
			break;

		case dir_East:
			if (volumes[volume_index].get() < sd_max_volume)
			{
				redraw_controls = true;
				update_volumes = true;
				volumes[volume_index].set(volumes[volume_index].get() + 1);
			}

			while (Keyboard[ScanCode::sc_right_arrow])
			{
				in_handle_events();
			}
			break;

		default:
			break;
		}

		if (update_volumes)
		{
			update_volumes = false;

			if (old_volumes[0] != volumes[0].get())
			{
				sd_set_sfx_volume();
				menu_play_move_gun_1_sound();
			}

			if (old_volumes[1] != volumes[1].get())
			{
				sd_set_music_volume();
			}
		}

		if (redraw_controls)
		{
			redraw_controls = false;
			draw_volume_controls();
			VW_UpdateScreen();
		}

		quit = (ci.button1 || Keyboard[ScanCode::sc_escape]);
	}

	menu_play_esc_pressed_sound();
	WaitKeyUp();
	MenuFadeOut();
}

///
void draw_video_descriptions(
	std::int16_t which)
{
	const char* instructions[] = {
		"CHANGES THE VIDEO MODE",
		"CHANGES TEXTURING OPTIONS",
		"TOGGLES BETWEEN WIDESCREEN AND 4X3 MODES",
		"TOGGLES STRETCHING OF USER INTERFACE",
		"SELECTS FILLER'S COLOR",
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = 144;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(
		WindowX,
		WindowY - 1,
		WindowW,
		WindowH,
		menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);

	--WindowX;
	--WindowY;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);
}

void video_draw_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();
	DrawMenuTitle("VIDEO SETTINGS");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&video_items, video_menu);
	VW_UpdateScreen();
}

void video_draw_switch(
	std::int16_t which)
{
	std::uint16_t Shape;

	for (int i = 0; i < video_items.amount; i++)
	{
		if (video_menu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			if (video_items.cursor.on)
			{
				if (i == which)
				{
					Shape += 2;
				}
			}

			switch (i)
			{
				case mvl_mode:
				case mvl_texturing:
				case mvl_filler_color:
					continue;

			case mvl_widescreen:
				if (vid_cfg_is_widescreen())
				{
					Shape++;
				}
				break;

			case mvl_stretch_ui:
				if (vid_cfg_is_ui_stretched())
				{
					Shape++;
				}
				break;

			default:
				break;
			}

			VWB_DrawPic(
				video_items.x - 16,
				video_items.y + (i * video_items.y_spacing) - 1,
				Shape);
		}
	}

	draw_video_descriptions(which);
}

///
void draw_carousel(
	const int item_index,
	CP_iteminfo* item_i,
	CP_itemtype* items,
	const std::string& text)
{
	const int which = item_i->curpos;

	int item_text_width;
	int item_text_height;

	VW_MeasurePropString(
		items[item_index].string.c_str(),
		&item_text_width,
		&item_text_height
	);

	int carousel_text_width;
	int carousel_text_height;

	VW_MeasurePropString(
		text.c_str(),
		&carousel_text_width,
		&carousel_text_height
	);

	const auto max_height = item_i->y_spacing;

	const auto arrow_width = 3;
	const auto arrow_height = 5;
	const auto arrow_y = item_i->y + (item_index * max_height) + (max_height / 2) - (arrow_height / 2);

	const auto left_arrow_x = item_i->x + item_i->indent + item_text_width + 3;

	const auto arrow_color = static_cast<std::uint8_t>(color_norml[items->active]);

	VL_Plot(left_arrow_x + 0, arrow_y - 0, arrow_color);
	VL_Vlin(left_arrow_x + 1, arrow_y - 1, 3, arrow_color);
	VL_Vlin(left_arrow_x + 2, arrow_y - 2, 5, arrow_color);

	WindowW = 320;
	WindowH = 200;

	const auto carousel_text_x = left_arrow_x + arrow_width + 2;

	WindowX = static_cast<std::int16_t>(carousel_text_x);
	WindowY = static_cast<std::int16_t>(item_i->y + (item_index * max_height));

	PrintX = WindowX;
	PrintY = WindowY;

	SetTextColor(items + item_index, item_index == which);

	ShadowPrint(text.c_str(), static_cast<std::int16_t>(WindowX), static_cast<std::int16_t>(WindowY));

	const auto right_arrow_x = carousel_text_x + carousel_text_width + 2;

	VL_Vlin(right_arrow_x + 0, arrow_y - 2, 5, arrow_color);
	VL_Vlin(right_arrow_x + 1, arrow_y - 1, 3, arrow_color);
	VL_Plot(right_arrow_x + 2, arrow_y - 0, arrow_color);
}


int menu_video_mode_renderer_index_;
VidRendererTypes menu_video_mode_renderer_types_;
int menu_video_mode_sizes_index_;
VidWindowSizes menu_video_mode_sizes_;

VideoModeCfg menu_video_mode_cfg_;
VideoModeCfg menu_video_mode_cfg_saved_;

void menu_video_mode_set_renderer_type(
	VideoModeCfg& video_mode_cfg)
{
	video_mode_cfg.renderer_type = menu_video_mode_renderer_types_[menu_video_mode_renderer_index_];
}

void menu_video_mode_set_windowed_size(
	VideoModeCfg& video_mode_cfg)
{
	const auto& windowed_size = menu_video_mode_sizes_[menu_video_mode_sizes_index_];
	video_mode_cfg.width = windowed_size.width;
	video_mode_cfg.height = windowed_size.height;
}

void menu_video_mode_update_apply_button()
{
	const auto is_modified = (menu_video_mode_cfg_ != menu_video_mode_cfg_saved_);

	video_mode_menu[6].active = (is_modified ? AT_ENABLED : AT_DISABLED);
}

int menu_video_mode_aa_factor_adjust(
	const int aa_factor)
{
	auto current_aa_factor = aa_factor;

	if (current_aa_factor < bstone::R3rLimits::min_aa_on())
	{
		current_aa_factor = bstone::R3rLimits::min_aa_on();
	}
	else if (current_aa_factor > bstone::R3rLimits::max_aa())
	{
		current_aa_factor = bstone::R3rLimits::max_aa();
	}

	auto current_pow = 0;

	while ((1 << current_pow) < current_aa_factor)
	{
		++current_pow;
	}

	current_aa_factor = 1 << current_pow;

	return current_aa_factor;
}

const std::string& menu_video_mode_renderer_type_get_string(
	const bstone::RendererType renderer_type)
{
	static const auto auto_detect_string = std::string{"AUTO-DETECT"};
	static const auto software_string = std::string{"SOFTWARE"};

	static const auto gl_2_string = std::string{"GL 2.0"};
	static const auto gl_3_2_c_string = std::string{"GL 3.2 CORE"};
	static const auto gles_2_0_string = std::string{"GLES 2.0"};

	switch (renderer_type)
	{
		case bstone::RendererType::auto_detect:
			return auto_detect_string;

		case bstone::RendererType::software:
			return software_string;

		case bstone::RendererType::gl_2_0:
			return gl_2_string;

		case bstone::RendererType::gl_3_2_core:
			return gl_3_2_c_string;

		case bstone::RendererType::gles_2_0:
			return gles_2_0_string;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}
}

std::string menu_video_mode_size_get_string(
	const VidWindowSize& size)
{
	return std::to_string(size.width) + " X " + std::to_string(size.height);
}

const std::string& menu_video_mode_aa_type_get_string(
	const bstone::R3rAaType aa_type)
{
	static const auto none_string = std::string{"NONE"};
	static const auto msaa_string = std::string{"MSAA"};

	switch (aa_type)
	{
		case bstone::R3rAaType::none:
			return none_string;

		case bstone::R3rAaType::ms:
			return msaa_string;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported AA type.");
	}
}

std::string menu_video_mode_aa_factor_get_string(
	const int aa_factor)
{
	return std::to_string(aa_factor);
}


void draw_video_mode_descriptions(
	std::int16_t which)
{
	static const char* instructions[] =
	{
		"SELECTS THE RENDERER",
		"SELECTS WINDOW SIZE FOR WINDOWED MODE",
		"TOGGLES VERTICAL SYNCHRONIZATION",
		"SELECTS ANTI-ALIASING TYPE",
		"SELECTS ANTI-ALIASING DEGREE",
		"",
		"APPLIES SETTINGS",
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = 144;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(
		WindowX,
		WindowY - 1,
		WindowW,
		WindowH,
		menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);

	--WindowX;
	--WindowY;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);
}

void video_mode_draw_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();
	DrawMenuTitle("VIDEO MODE");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&video_mode_items, video_mode_menu);
	VW_UpdateScreen();

	menu_video_mode_cfg_ = vid_cfg_get_video_mode();

	{
		menu_video_mode_renderer_types_ = vid_get_available_renderer_types();

		if (menu_video_mode_renderer_types_.empty())
		{
			BSTONE_THROW_STATIC_SOURCE("Empty renderer type list.");
		}

		const auto renderer_type_it = std::find(
			menu_video_mode_renderer_types_.cbegin(),
			menu_video_mode_renderer_types_.cend(),
			menu_video_mode_cfg_.renderer_type
		);

		if (renderer_type_it == menu_video_mode_renderer_types_.cend())
		{
			menu_video_mode_renderer_index_ = 0;
		}
		else
		{
			menu_video_mode_renderer_index_ = static_cast<int>(
				renderer_type_it - menu_video_mode_renderer_types_.cbegin());
		}
	}

	{
		menu_video_mode_sizes_index_ = 0;
		menu_video_mode_sizes_ = vid_get_window_size_list();

		const auto index_begin = menu_video_mode_sizes_.cbegin();

		const auto index_it = std::find_if(
			index_begin,
			menu_video_mode_sizes_.cend(),
			[](const auto& item)
			{
				return item.is_current_;
			}
		);

		if (index_it != menu_video_mode_sizes_.cend())
		{
			menu_video_mode_sizes_index_ = static_cast<int>(index_it - index_begin);
		}
	}

	menu_video_mode_cfg_.aa_degree_ = menu_video_mode_aa_factor_adjust(menu_video_mode_cfg_.aa_degree_);
	menu_video_mode_cfg_saved_ = menu_video_mode_cfg_;
}

void video_mode_update_menu()
{
	ClearMScreen();
	DrawMenuTitle("VIDEO MODE");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&video_mode_items, video_mode_menu);
}

void video_mode_draw_switch(
	std::int16_t which)
{
	std::uint16_t Shape;

	const auto renderer_type = menu_video_mode_renderer_types_[menu_video_mode_renderer_index_];
	const auto& renderer_type_string = menu_video_mode_renderer_type_get_string(renderer_type);

	const auto& window_size = menu_video_mode_sizes_[menu_video_mode_sizes_index_];
	const auto window_size_string = menu_video_mode_size_get_string(window_size);

	const auto aa_type_string = menu_video_mode_aa_type_get_string(menu_video_mode_cfg_.aa_type);
	const auto aa_factor_string = menu_video_mode_aa_factor_get_string(menu_video_mode_cfg_.aa_degree_);

	for (int i = 0; i < video_mode_items.amount; ++i)
	{
		if (video_mode_menu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			if (video_mode_items.cursor.on)
			{
				if (i == which)
				{
					Shape += 2;
				}
			}

			switch (i)
			{
				case 0:
					draw_carousel(
						i,
						&video_mode_items,
						video_mode_menu,
						renderer_type_string
					);

					continue;

				case 1:
					draw_carousel(
						i,
						&video_mode_items,
						video_mode_menu,
						window_size_string
					);

					continue;

				case 2:
					if (menu_video_mode_cfg_.is_vsync_)
					{
						++Shape;
					}

					break;

				case 3:
					draw_carousel(
						i,
						&video_mode_items,
						video_mode_menu,
						aa_type_string
					);

					continue;

				case 4:
					draw_carousel(
						i,
						&video_mode_items,
						video_mode_menu,
						aa_factor_string
					);

					continue;

				default:
					continue;
			}

			VWB_DrawPic(
				video_mode_items.x - 16,
				video_mode_items.y + (i * video_mode_items.y_spacing) - 1,
				Shape);
		}
	}

	draw_video_mode_descriptions(which);
}

void video_menu_mode_renderer_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	const auto max_index = static_cast<int>(menu_video_mode_renderer_types_.size());

	const auto delta = (is_left ? -1 : (is_right ? 1 : 0));

	menu_video_mode_renderer_index_ += delta;

	if (menu_video_mode_renderer_index_ < 0)
	{
		menu_video_mode_renderer_index_ = max_index - 1;
	}
	else if (menu_video_mode_renderer_index_ >= max_index)
	{
		menu_video_mode_renderer_index_ = 0;
	}

	menu_video_mode_set_renderer_type(menu_video_mode_cfg_);
	menu_video_mode_update_apply_button();

	video_mode_update_menu();
	video_mode_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void video_menu_mode_window_size_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	const auto max_index = static_cast<int>(menu_video_mode_sizes_.size());

	const auto delta = (is_left ? -1 : (is_right ? 1 : 0));

	menu_video_mode_sizes_index_ += delta;

	if (menu_video_mode_sizes_index_ < 0)
	{
		menu_video_mode_sizes_index_ = max_index - 1;
	}
	else if (menu_video_mode_sizes_index_ >= max_index)
	{
		menu_video_mode_sizes_index_ = 0;
	}

	menu_video_mode_set_windowed_size(menu_video_mode_cfg_);
	menu_video_mode_update_apply_button();

	video_mode_update_menu();
	video_mode_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void video_menu_mode_window_aa_type_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	switch (menu_video_mode_cfg_.aa_type)
	{
		case bstone::R3rAaType::none:
			if (is_left)
			{
				menu_video_mode_cfg_.aa_type = bstone::R3rAaType::ms;
			}
			else if (is_right)
			{
				menu_video_mode_cfg_.aa_type = bstone::R3rAaType::ms;
			}

			break;

		case bstone::R3rAaType::ms:
			if (is_left)
			{
				menu_video_mode_cfg_.aa_type = bstone::R3rAaType::none;
			}
			else if (is_right)
			{
				menu_video_mode_cfg_.aa_type = bstone::R3rAaType::none;
			}

			break;
	}

	menu_video_mode_update_apply_button();

	video_mode_update_menu();
	video_mode_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void video_menu_mode_window_aa_factor_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	auto aa_factor = menu_video_mode_cfg_.aa_degree_;

	if (is_left)
	{
		aa_factor /= 2;
	}
	else if (is_right)
	{
		aa_factor *= 2;
	}

	if (aa_factor < bstone::R3rLimits::min_aa_on())
	{
		aa_factor = bstone::R3rLimits::max_aa();
	}
	else if (aa_factor > bstone::R3rLimits::max_aa())
	{
		aa_factor = bstone::R3rLimits::min_aa_on();
	}

	menu_video_mode_cfg_.aa_degree_ = aa_factor;

	menu_video_mode_update_apply_button();

	video_mode_update_menu();
	video_mode_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void video_menu_mode_routine(
	const std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	video_mode_draw_menu();
	MenuFadeIn();
	WaitKeyUp();

	video_mode_menu[0].carousel_func_ = video_menu_mode_renderer_carousel;
	video_mode_menu[1].carousel_func_ = video_menu_mode_window_size_carousel;

	video_mode_menu[3].carousel_func_ = video_menu_mode_window_aa_type_carousel;
	video_mode_menu[4].carousel_func_ = video_menu_mode_window_aa_factor_carousel;

	do
	{
		which = HandleMenu(&video_mode_items, video_mode_menu, video_mode_draw_switch);

		switch (which)
		{
			case 2:
				menu_video_mode_cfg_.is_vsync_ = !menu_video_mode_cfg_.is_vsync_;
				menu_video_mode_update_apply_button();
				video_mode_update_menu();
				break;

			case 6:
				if (menu_video_mode_cfg_ != menu_video_mode_cfg_saved_)
				{
					menu_video_mode_cfg_saved_ = menu_video_mode_cfg_;
					menu_video_mode_update_apply_button();
					vid_apply_video_mode(menu_video_mode_cfg_);
					video_mode_draw_menu();
				}

				break;

			default:
				break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

///
const std::string& texturing_filter_to_string(
	const bstone::R3rFilterType filter)
{
	static const auto nearest_string = std::string{"NEAREST"};
	static const auto linear_string = std::string{"LINEAR"};

	switch (filter)
	{
		case bstone::R3rFilterType::nearest:
			return nearest_string;

		case bstone::R3rFilterType::linear:
			return linear_string;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported filter.");
	}
}

int texturing_anisotropy_to_pot(
	const int anisotropy)
{
	auto pot_anisotropy = anisotropy;

	if (pot_anisotropy < bstone::R3rLimits::min_anisotropy_off())
	{
		pot_anisotropy = bstone::R3rLimits::min_anisotropy_off();
	}
	else if (pot_anisotropy > bstone::R3rLimits::max_anisotropy())
	{
		pot_anisotropy = bstone::R3rLimits::max_anisotropy();
	}

	auto power = 0;

	while (pot_anisotropy > (1 << power))
	{
		++power;
	}

	return 1 << power;
}

int texturing_normalize_upscale_degree(
	const int upscale_degree)
{
	if (upscale_degree < vid_upscale_min_degree)
	{
		return vid_upscale_min_degree;
	}
	else if (upscale_degree > vid_upscale_max_degree)
	{
		return vid_upscale_max_degree;
	}
	else
	{
		return upscale_degree;
	}
}

void texturing_draw_descriptions(
	std::int16_t which)
{
	static const char* instructions[] =
	{
		"SELECTS DEGREE OF ANISOTROPY FOR 3D ELEMENTS",
		"SELECTS IMAGE FILTER FOR 2D ELEMENTS",
		"SELECTS IMAGE FILTER FOR 3D ELEMENTS",
		"SELECTS MIPMAP FILTER FOR 3D ELEMENTS",
		"SELECTS UPSCALE FILTER FOR 8-BIT TEXTURES",
		"SELECTS DEGREE OF UPSCALE",
		"TOGGLES USE OF EXTERNAL TEXTURES",
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = 144;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(
		WindowX,
		WindowY - 1,
		WindowW,
		WindowH,
		menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);

	--WindowX;
	--WindowY;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);
}

void texturing_draw_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();
	DrawMenuTitle("TEXTURING");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&texturing_items, texturing_menu);
	VW_UpdateScreen();
	vid_cfg_set_3d_texture_anisotropy(texturing_anisotropy_to_pot(vid_cfg_get_3d_texture_anisotropy()));
}

void texturing_update_menu()
{
	ClearMScreen();
	DrawMenuTitle("TEXTURING");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&texturing_items, texturing_menu);
}

void texturing_draw_switch(
	std::int16_t which)
{
	std::uint16_t Shape;

	for (int i = 0; i < texturing_items.amount; ++i)
	{
		if (texturing_menu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			if (texturing_items.cursor.on)
			{
				if (i == which)
				{
					Shape += 2;
				}
			}

			switch (i)
			{
				case static_cast<int>(TexturingMenuIndices::anisotropy):
				{
					const auto anisotropy_string = (
						vid_cfg_get_3d_texture_anisotropy() > bstone::R3rLimits::min_anisotropy_off() ?
						std::to_string(vid_cfg_get_3d_texture_anisotropy()) :
						"OFF"
					);

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						anisotropy_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::image_2d_filter):
				{
					const auto& image_2d_filter_string = texturing_filter_to_string(
						vid_cfg_get_2d_texture_filter());

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						image_2d_filter_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::image_3d_filter):
				{
					const auto& image_3d_filter_string = texturing_filter_to_string(
						vid_cfg_get_3d_texture_image_filter());

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						image_3d_filter_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::mipmap_3d_filter):
				{
					const auto& mipmap_3d_filter_string = texturing_filter_to_string(
						vid_cfg_get_3d_texture_mipmap_filter());

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						mipmap_3d_filter_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::upscale_filter):
				{
					const auto upscale_filter_string = (
						vid_cfg_get_texture_upscale_type() == bstone::HwTextureMgrUpscaleFilterType::none ?
						"NONE" :
						"XBRZ"
					);

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						upscale_filter_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::upscale_degree):
				{
					const auto upscale_degree_string = std::to_string(
						texturing_normalize_upscale_degree(
							vid_cfg_get_texture_upscale_xbrz_degree()));

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						upscale_degree_string
					);

					continue;
				}

				case static_cast<int>(TexturingMenuIndices::external_textures):
				{
					const auto external_textures_string = (vid_cfg_is_external_textures_enabled() ? "ON" : "OFF");

					draw_carousel(
						i,
						&texturing_items,
						texturing_menu,
						external_textures_string
					);

					continue;
				}

				default:
					break;
			}

			VWB_DrawPic(
				texturing_items.x - 16,
				texturing_items.y + (i * texturing_items.y_spacing) - 1,
				Shape);
		}
	}

	texturing_draw_descriptions(which);
}

void texturing_anisotropy_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	auto anisotropy = texturing_anisotropy_to_pot(vid_cfg_get_3d_texture_anisotropy());

	if (is_left)
	{
		anisotropy /= 2;
	}
	else if (is_right)
	{
		anisotropy *= 2;
	}

	if (anisotropy < bstone::R3rLimits::min_anisotropy_off())
	{
		anisotropy = bstone::R3rLimits::max_anisotropy();
	}

	if (anisotropy > bstone::R3rLimits::max_anisotropy())
	{
		anisotropy = bstone::R3rLimits::min_anisotropy_off();
	}

	vid_cfg_set_3d_texture_anisotropy(anisotropy);

	vid_apply_anisotropy();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_filter_carousel(
	bstone::R3rFilterType& filter)
{
	switch (filter)
	{
		case bstone::R3rFilterType::nearest:
			filter = bstone::R3rFilterType::linear;
			break;

		case bstone::R3rFilterType::linear:
			filter = bstone::R3rFilterType::nearest;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported filter.");
	}
}

void texturing_2d_image_filter_carousel(
	const int item_index,
	const bool,
	const bool)
{
	auto filter = vid_cfg_get_2d_texture_filter();
	texturing_filter_carousel(filter);
	vid_cfg_set_2d_texture_filter(filter);
	vid_apply_2d_image_filter();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_3d_image_filter_carousel(
	const int item_index,
	const bool,
	const bool)
{
	auto filter = vid_cfg_get_3d_texture_image_filter();
	texturing_filter_carousel(filter);
	vid_cfg_set_3d_texture_image_filter(filter);
	vid_apply_3d_image_filter();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_3d_mipmap_filter_carousel(
	const int item_index,
	const bool,
	const bool)
{
	auto filter = vid_cfg_get_3d_texture_mipmap_filter();
	texturing_filter_carousel(filter);
	vid_cfg_set_3d_texture_mipmap_filter(filter);
	vid_apply_mipmap_filter();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_upscale_filter_carousel(
	const int item_index,
	const bool,
	const bool)
{
	if (vid_cfg_get_texture_upscale_type() == bstone::HwTextureMgrUpscaleFilterType::none)
	{
		vid_cfg_set_texture_upscale_type(bstone::HwTextureMgrUpscaleFilterType::xbrz);
	}
	else if (vid_cfg_get_texture_upscale_type() == bstone::HwTextureMgrUpscaleFilterType::xbrz)
	{
		vid_cfg_set_texture_upscale_type(bstone::HwTextureMgrUpscaleFilterType::none);
	}

	vid_apply_upscale();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_upscale_degree_carousel(
	const int item_index,
	const bool is_left,
	const bool is_right)
{
	auto xbrz_degree = texturing_normalize_upscale_degree(vid_cfg_get_texture_upscale_xbrz_degree());

	if (is_left)
	{
		--xbrz_degree;

		if (xbrz_degree < vid_upscale_min_degree)
		{
			xbrz_degree = vid_upscale_max_degree;
		}
	}
	else if (is_right)
	{
		++xbrz_degree;

		if (xbrz_degree > vid_upscale_max_degree)
		{
			xbrz_degree = vid_upscale_min_degree;
		}
	}

	vid_cfg_set_texture_upscale_xbrz_degree(xbrz_degree);

	vid_apply_upscale();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_external_textures_carousel(
	const int item_index,
	const bool,
	const bool)
{
	vid_cfg_set_is_external_textures_enabled(!vid_cfg_is_external_textures_enabled());

	vid_apply_external_textures();

	texturing_update_menu();
	texturing_draw_switch(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

void texturing_routine(
	const std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	texturing_draw_menu();
	MenuFadeIn();
	WaitKeyUp();

	texturing_menu[static_cast<int>(TexturingMenuIndices::anisotropy)].carousel_func_ =
		texturing_anisotropy_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::image_2d_filter)].carousel_func_ =
		texturing_2d_image_filter_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::image_3d_filter)].carousel_func_ =
		texturing_3d_image_filter_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::mipmap_3d_filter)].carousel_func_ =
		texturing_3d_mipmap_filter_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::upscale_filter)].carousel_func_ =
		texturing_upscale_filter_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::upscale_degree)].carousel_func_ =
		texturing_upscale_degree_carousel;
	texturing_menu[static_cast<int>(TexturingMenuIndices::external_textures)].carousel_func_ =
		texturing_external_textures_carousel;

	do
	{
		which = HandleMenu(&texturing_items, texturing_menu, texturing_draw_switch);
	} while (which >= 0);

	MenuFadeOut();
}

///
void cp_video(
	std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	video_draw_menu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&video_items, video_menu, video_draw_switch);

		switch (which)
		{
			case mvl_mode:
			case mvl_texturing:
				video_draw_menu();
				MenuFadeIn();
				WaitKeyUp();
				break;

		case mvl_widescreen:
#ifndef __vita__
			vid_cfg_set_is_widescreen(!vid_cfg_is_widescreen());
#endif
			ShootSnd();
			video_draw_switch(video_items.curpos);
			vl_update_widescreen();
			SetupWalls();
			NewViewSize();
			SetPlaneViewSize();
			VL_RefreshScreen();
			break;

		case mvl_stretch_ui:
			vid_cfg_set_is_ui_stretched(!vid_cfg_is_ui_stretched());
			ShootSnd();
			video_draw_switch(video_items.curpos);
			VL_RefreshScreen();
			break;

		case mvl_filler_color:
			menu_play_esc_pressed_sound();
			video_draw_menu();
			MenuFadeIn();
			WaitKeyUp();
			break;

		default:
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void draw_switch2_description(
	std::int16_t which)
{
	const char* instr[] =
	{
		"TOGGLES INTRO/OUTRO",
		"TOGGLES FADE IN/OUT EFFECT",
		"TOGGLES WEAPON BOBBING",
		"TOGGLES VANILLA FIZZLE FX",
		"TOGGLES MAP IN STATS (PLANET STRIKE)",
	};

	const auto& assets_info = get_assets_info();

	fontnumber = 2;

	WindowX = 48;
	WindowY = (assets_info.is_ps() ? 134 : 144);
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);
}

void draw_all_switch2_lights(
	std::int16_t which)
{
	std::uint16_t shape;

	for (auto i = 0; i < switches2_items.amount; ++i)
	{
		if (switch2_menu[i].string[0])
		{
			shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (switches2_items.cursor.on)
			{
				// Is the cursor sitting on this pic?
				if (i == which)
				{
					shape += 2;
				}
			}

			switch (i)
			{
			case SW2_NO_INTRO_OUTRO:
				if (gp_no_intro_outro())
				{
					shape += 1;
				}
				break;

			case SW2_NO_FADE_IN_OR_OUT:
				if (gp_no_fade_in_or_out())
				{
					shape += 1;
				}
				break;

			case SW2_NO_WEAPON_BOBBING:
				if (gp_no_weapon_bobbing())
				{
					shape += 1;
				}
				break;

			case SW2_VANILLA_FIZZLE_FX:
				if (gp_vanilla_fizzle_fx())
				{
					shape += 1;
				}
				break;

			case SW2_PS_MAP_IN_STATS:
				if (gp_ps_map_in_stats())
				{
					shape += 1;
				}
				break;
			}

			VWB_DrawPic(switches2_items.x - 16, switches2_items.y + (i * switches2_items.y_spacing) - 1, shape);
		}
	}

	draw_switch2_description(which);
}

void draw_switch2_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME SWITCHES 2");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2;

	DrawMenu(&switches2_items, &switch2_menu[0]);
	draw_all_switch2_lights(switches2_items.curpos);

	VW_UpdateScreen();
}

void cp_switches2(
	std::int16_t)
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	draw_switch2_menu();
	MenuFadeIn();
	WaitKeyUp();

	auto which = 0;

	do
	{
		which = HandleMenu(&switches2_items, &switch2_menu[0], draw_all_switch2_lights);

		switch (which)
		{
		case SW2_NO_INTRO_OUTRO:
			gp_no_intro_outro(!gp_no_intro_outro());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_NO_FADE_IN_OR_OUT:
			gp_no_fade_in_or_out(!gp_no_fade_in_or_out());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_NO_WEAPON_BOBBING:
			gp_no_weapon_bobbing(!gp_no_weapon_bobbing());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_VANILLA_FIZZLE_FX:
			gp_vanilla_fizzle_fx(!gp_vanilla_fizzle_fx());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_PS_MAP_IN_STATS:
			gp_ps_map_in_stats(!gp_ps_map_in_stats());
			ShootSnd();
			draw_switch2_menu();
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}
///

///
constexpr auto filler_cell_width = 9;
constexpr auto filler_cell_height = 8;
constexpr auto filler_cells_x = (vga_ref_width - (16 * filler_cell_width)) / 2;
constexpr auto filler_cells_y = 36;


void draw_filler_color_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	auto color_index = 0;

	auto y = filler_cells_y;

	for (auto h = 0; h < 16; ++h)
	{
		auto x = filler_cells_x;

		for (auto w = 0; w < 16; ++w)
		{
			VL_Bar(x, y, filler_cell_width, filler_cell_height, static_cast<std::uint8_t>(color_index++));

			x += filler_cell_width;
		}

		y += filler_cell_height;
	}
}

constexpr int weight_color(
	int r,
	int g,
	int b) noexcept
{
	return (r << 16) | (g << 8) | b;
}

int find_highlight_filler_color_index(
	int color_index)
{
	const auto src_palette_index = color_index * 3;

	const auto src_r = vgapal[src_palette_index + 0];
	const auto src_g = vgapal[src_palette_index + 1];
	const auto src_b = vgapal[src_palette_index + 2];

	const auto src_weight = weight_color(src_r, src_g, src_b);

	auto max_match_abs_weight_diff = 0;
	auto match_color_index = -1;

	for (auto i = 0; i < 256; ++i)
	{
		if (i == color_index)
		{
			continue;
		}

		const auto match_palette_index = i * 3;

		const auto match_r = vgapal[match_palette_index + 0];
		const auto match_g = vgapal[match_palette_index + 1];
		const auto match_b = vgapal[match_palette_index + 2];

		const auto match_weight = weight_color(match_r, match_g, match_b);
		const auto match_abs_weight_diff = std::abs(match_weight - src_weight);

		if (match_abs_weight_diff > max_match_abs_weight_diff)
		{
			max_match_abs_weight_diff = match_abs_weight_diff;
			match_color_index = i;
		}
	}

	if (match_color_index < 0)
	{
		match_color_index = color_index + 128;

		if (match_color_index > 255)
		{
			match_color_index -= 255;
		}
	}

	return match_color_index;
}

void draw_filler_color_cell(
	int color_index,
	bool is_highlighted)
{
	const auto cell_x = color_index % 16;
	const auto cell_y = color_index / 16;

	const auto x = filler_cells_x + (cell_x * filler_cell_width);
	const auto y = filler_cells_y + (cell_y * filler_cell_height);

	VL_Bar(x, y, filler_cell_width, filler_cell_height, static_cast<std::uint8_t>(color_index));

	if (is_highlighted)
	{
		const auto highlighted_color_index = find_highlight_filler_color_index(color_index);

		vwb_rect(x, y, filler_cell_width, filler_cell_height, highlighted_color_index);
	}
}

void filler_color_routine(
	const std::int16_t)
{
	VL_SetPalette(0, 256, vgapal);
	draw_filler_color_menu();
	MenuFadeIn();
	WaitKeyUp();

	ControlInfo ci;

	auto is_highlighted = false;
	auto highlight_counter = 0;

	auto is_cell_changed = true;
	auto cell_x = vid_cfg_get_filler_color_index() % 16;
	auto cell_y = vid_cfg_get_filler_color_index() / 16;

	while (true)
	{
		CalcTics();
		ReadAnyControl(&ci);

		switch (ci.dir)
		{
			case dir_North:
				is_cell_changed = true;
				cell_y -= 1;
				break;

			case dir_South:
				is_cell_changed = true;
				cell_y += 1;
				break;

			case dir_West:
				is_cell_changed = true;
				cell_x -= 1;
				break;

			case dir_East:
				is_cell_changed = true;
				cell_x += 1;
				break;

			default:
				break;
		}

		if (is_cell_changed)
		{
			is_cell_changed = false;

			is_highlighted = true;

			draw_filler_color_cell(vid_cfg_get_filler_color_index(), false);

			if (cell_x < 0)
			{
				cell_x = 15;
			}
			else if (cell_x > 15)
			{
				cell_x = 0;
			}

			if (cell_y < 0)
			{
				cell_y = 15;
			}
			else if (cell_y > 15)
			{
				cell_y = 0;
			}

			vid_cfg_set_filler_color_index((cell_y * 16) + cell_x);

			IN_ClearKeysDown();

			vid_apply_filler_color();
		}

		if (ci.button1 || Keyboard[ScanCode::sc_escape])
		{
			break;
		}

		draw_filler_color_cell(vid_cfg_get_filler_color_index(), is_highlighted);

		VW_UpdateScreen();

		highlight_counter += 1;

		if (highlight_counter > 10)
		{
			highlight_counter = 0;
			is_highlighted = !is_highlighted;
		}
	}

	MenuFadeOut();
}

///


void MenuFadeOut()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 44, 0, 0, 10);
	}
	else
	{
		VL_FadeOut(0, 255, 40, 44, 44, 10);
	}
}

void MenuFadeIn()
{
	VL_FadeIn(0, 255, vgapal, 10);
}

void menu_enable_all_episodes()
{
	for (auto i = 1; i < 6; ++i)
	{
		NewEmenu[i].active = AT_ENABLED;
		EpisodeSelect[i] = 1;
	}
}
// BBi
