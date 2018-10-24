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


#ifndef BSTONE_3D_MENU_INCLUDED
#define BSTONE_3D_MENU_INCLUDED


#include "id_heads.h"


#define GAME_DESCRIPTION_LEN (31)


//
// Menu Color Defines
//

// Box behind text for cursor
#define HIGHLIGHT_BOX_COLOR (0x52)

// Text color for text on cursor
#define HIGHLIGHT_TEXT_COLOR (0x59)

// Text color for text on cursor for a turned off item
#define HIGHLIGHT_DISABLED_COLOR (0x56)

#define HIGHLIGHT_DEACTIAVED_COLOR (0x55)


#define ENABLED_TEXT_COLOR (0x56)
#define DISABLED_TEXT_COLOR (0x53)
#define DEACTIAVED_TEXT_COLOR (0x52)

#define INSTRUCTIONS_TEXT_COLOR (0x53)

#define TERM_BACK_COLOR (0x02)
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

void MenuFadeOut();

#define READCOLOR (0x4A)
#define READHCOLOR (0x47)
#define VIEWCOLOR (0x7F)
#define TEXTCOLOR (WHITE)
#define HIGHLIGHT (0x13)


inline void MenuFadeIn()
{
	::VL_FadeIn(0, 255, ::vgapal, 10);
}


extern std::int16_t MENUSONG;
extern std::int16_t ROSTER_MUS;
extern std::int16_t TEXTSONG;


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


enum mm_labels
{
	MM_NEW_MISSION,
	MM_ORDERING_INFO,
	MM_READ_THIS,
	MM_BLAKE_STONE_SAGA,
	MM_BLANK1,
	MM_GAME_OPTIONS,
	MM_VIEW_SCORES,
	MM_LOAD_MISSION,
	MM_SAVE_MISSION,
	MM_BLANK2,
	MM_BACK_TO_DEMO,
	MM_LOGOFF,
}; // mm_labels

// CP_Switch() menu labels
enum sw_labels
{
	SW_LIGHTING,
	SW_REBA_ATTACK_INFO,
	SW_CEILING,
	SW_FLOORS,

	// BBi
	SW_NO_WALL_HIT_SOUND,
	SW_MODERN_CONTROLS,
	SW_ALWAYS_RUN,
	SW_QUIT_ON_ESCAPE,
	SW_HEART_BEAT_SOUND,
	SW_ROTATED_AUTOMAP,
}; // sw_labels

// BBi
enum sw2_labels
{
	SW2_NO_INTRO_OUTRO,
}; // sw2_labels

enum MenuVideoLables
{
	mvl_widescreen,
	mvl_stretch_ui,
};
// BBi

//  ActiveType flags for menu options (SEE CP_itemtype.active)
enum activetypes
{
	AT_DISABLED = 0,
	AT_ENABLED,
	AT_READIT,
	AT_NON_SELECTABLE, // Menu Bar/Separator - Not a selectable item.
}; // activetypes


struct CP_cursortype
{
	std::uint8_t x;
	std::int8_t y_ofs;
	std::uint8_t width;
	std::uint8_t height;
	std::int8_t on;
}; // CP_cursortype

struct CP_iteminfo
{
	std::uint8_t x;
	std::uint8_t y;
	std::uint8_t amount;
	std::int8_t curpos;
	std::uint8_t indent;
	std::uint8_t y_spacing;

	CP_cursortype cursor;
}; // CP_iteminfo

struct CP_itemtype
{
	activetypes active;
	char string[36];

	void(*routine)(
		std::int16_t temp1);

	std::uint8_t fontnumber; // Font to print text in
	std::uint8_t height; // Hight of text (Y_Offset from previous line)
}; // CP_itemtype

struct CustomCtrls
{
	std::int16_t allowed[4];
}; // CustomCtrls

extern CP_itemtype MainMenu[];
extern CP_itemtype NewEMenu[];
extern CP_iteminfo MainItems;


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
	void(*routine)(std::int16_t w));

void ClearMScreen();

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

void ReadAnyControl(
	ControlInfo* ci);

void TicDelay(
	std::int16_t count);

void CacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend);

void UnCacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend);

void StartCPMusic(
	std::int16_t song);

std::int16_t Confirm(
	const char* string);

void Message(
	const char* string);

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

void DefineMouseBtns();
void DefineJoyBtns();
void DefineKeyBtns();
void DefineKeyMove();

void EnterCtrlData(
	std::int16_t index,
	CustomCtrls* cust,
	void(*DrawRtn)(std::int16_t),
	void(*PrintRtn)(std::int16_t),
	std::int16_t type);

void DrawMainMenu();
void DrawSoundMenu();

void DrawLoadSaveScreen(
	std::int16_t loadsave);

void DrawNewEpisode();
void DrawNewGame();

void DrawMouseSens();
void DrawCtlScreen();
void DrawCustomScreen();

void DrawLSAction(
	std::int16_t which);

void DrawCustMouse(
	std::int16_t hilight);

void DrawCustJoy(
	std::int16_t hilight);

void DrawCustKeybd(
	std::int16_t hilight);

void DrawCustKeys(
	std::int16_t hilight);

void PrintCustMouse(
	std::int16_t i);

void PrintCustJoy(
	std::int16_t i);

void PrintCustKeybd(
	std::int16_t i);

void PrintCustKeys(
	std::int16_t i);

void PrintLSEntry(
	std::int16_t w,
	std::int16_t color);

void TrackWhichGame(
	std::int16_t w);

void DrawNewGameDiff(
	std::int16_t w);

void FixupCustom(
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

void CP_ViewScores(
	std::int16_t temp1);

std::int16_t CP_EndGame();

bool CP_CheckQuick(
	ScanCode scancode);

void CustomControls(
	std::int16_t temp1);

void MouseSensitivity(
	std::int16_t temp1);

void DrawMenuTitle(
	const char* title);

void CheckForEpisodes();

void HelpPresenter(
	const char* fname,
	bool continuekeys,
	std::uint16_t id_cache,
	bool startmusic);

void ShadowPrint(
	const char* string,
	std::int16_t x,
	std::int16_t y);

//
// VARIABLES
//
extern std::int16_t SaveGamesAvail[10];
extern std::int16_t StartGame;
extern std::int16_t SoundStatus;
extern char SaveGameNames[10][GAME_DESCRIPTION_LEN + 1];

// FOR INPUT TYPES
enum MenuInputType
{
	MOUSE,
	JOYSTICK,
	KEYBOARDBTNS,
	KEYBOARDMOVE
}; // MenuInputType

//
// WL_INTER
//
struct LRstruct
{
	std::int16_t kill, secret, treasure;
	std::int32_t time;
}; // LRstruct

extern LRstruct LevelRatios[];


#endif // BSTONE_3D_MENU_INCLUDED
