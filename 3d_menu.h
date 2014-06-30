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


//
// WL_MENU.H
//
#include "id_heads.h"

#define GAME_DESCRIPTION_LEN	31


//
// Menu Color Defines
//

#define HIGHLIGHT_BOX_COLOR			0x52			// Box behind text for cursor
#define HIGHLIGHT_TEXT_COLOR			0x59			// Text color for text on cursor
#define HIGHLIGHT_DISABLED_COLOR		0x56			// Text color for text on cursor for a turned off item
#define HIGHLIGHT_DEACTIAVED_COLOR  0x55


#define ENABLED_TEXT_COLOR			0x56
#define DISABLED_TEXT_COLOR		0x53
#define DEACTIAVED_TEXT_COLOR		0x52

#define INSTRUCTIONS_TEXT_COLOR	0x53

#define TERM_BACK_COLOR				0x02
#define TERM_SHADOW_COLOR			0x01

//
// Clearable Menu Terminal Area
//
#define SCREEN_X						32
#define SCREEN_Y						28
#define SCREEN_W						244
#define SCREEN_H						132


#define BORDCOLOR		(0x78)
#define BORD2COLOR	(0x74)
#define DEACTIVE		(0x72)
#define BKGDCOLOR		(0x76)
#define STRIPE			0x2c

#define MenuFadeOut()	VL_FadeOut(0,255,40,44,44,10)


#define READCOLOR		0x4a
#define READHCOLOR	0x47
#define VIEWCOLOR		0x7f
#define TEXTCOLOR		WHITE
#define HIGHLIGHT		0x13

#define MenuFadeIn()	VL_FadeIn(0,255,vgapal,10)

#define MENUSONG  	LASTLAFF_MUS
#define ROSTER_MUS   HISCORE_MUS
#define TEXTSONG		TOHELL_MUS

#define QUITSUR	"Are you sure you want\n"\
						"to quit this great game? (Y/N)"

#define CURGAME "   Continuing past this\n"\
					 "  point will end the game\n"\
					 " you're currently playing.\n"\
					 "\n"\
					 " Start a NEW game? (Y/N)"

#if 0
#define CURGAME	"You are currently in\n"\
						"a game. Continuing will\n"\
						"erase the old game.\n"\
						"Start a NEW game? (Y/N)"
#endif

#define GAMESVD	"There's already a game\n"\
						"saved at this position.\n"\
						"\n"\
						"    Overwrite? (Y/N)"



#define SENSITIVE	60
#define CENTER		SENSITIVE*2

#define MENU_X	111
#define MENU_Y	50

#define SM_X	121
#define SM_Y	84
#define SM_W	54

#define CTL_X	100
#define CTL_Y	70

#define LSM_X	85
#define LSM_Y	55
#define LSM_W	144
#define LSM_H	10*13+10

#define NM_X	71
#define NM_Y	66

#define NE_X	58
#define NE_Y	54

#define CST_X		77
#define CST_Y		60

#define CST_START	77
#define CST_SPC	41

#define LSA_X	96
#define LSA_Y	80
#define LSA_W	130
#define LSA_H	42

typedef enum mm_labels {MM_NEW_MISSION,
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
								MM_LOGOFF
} mm_labels;

// CP_Switch() menu labels
//

enum sw_labels {
	SW_LIGHTING,
	SW_REBA_ATTACK_INFO,
	SW_CEILING,
	SW_FLOORS,

    // BBi
    SW_NO_WALL_HIT_SOUND
}; // enum sw_labels



//
//  ActiveType flags for menu options (SEE CP_itemtype.active)
//
enum activetypes {
	AT_DISABLED = 0,
	AT_ENABLED,
	AT_READIT,
	AT_NON_SELECTABLE				// Menu Bar/Separator - Not a selectable item.
}; // enum activetypes

//
// TYPEDEFS
//
struct CP_cursortype {
	Uint8 x;
	char y_ofs;
	Uint8 width;
	Uint8 height;
	char on;
}; // struct CP_cursortype

struct CP_iteminfo {
	Uint8 x;
	Uint8 y;
	Uint8 amount;
	char curpos;
	Uint8 indent;
	Uint8 y_spacing;

	CP_cursortype cursor;
}; // struct CP_iteminfo


struct CP_itemtype {
	activetypes active;
	char string[36];
	void (* routine)(Sint16 temp1);
	Uint8 fontnumber;							// Font to print text in
	Uint8 height;								// Hight of text (Y_Offset from previous line)
}; // struct CP_itemtype


struct CustomCtrls {
	Sint16 allowed[4];
}; // struct CustomCtrls

extern CP_itemtype MainMenu[],NewEMenu[];
extern CP_iteminfo MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);
void ControlPanelFree(void);
void ControlPanelAlloc(void);

void DrawMenu(CP_iteminfo *item_i,CP_itemtype *items);			
Sint16 HandleMenu(CP_iteminfo *item_i,CP_itemtype *items,void (*routine)(Sint16 w));	
void ClearMScreen(void);
void DrawWindow(Sint16 x,Sint16 y,Sint16 w,Sint16 h,Sint16 wcolor);
void DrawOutline(Sint16 x,Sint16 y,Sint16 w,Sint16 h,Sint16 color1,Sint16 color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(Sint16 count);
void CacheLump(Sint16 lumpstart,Sint16 lumpend);
void UnCacheLump(Sint16 lumpstart,Sint16 lumpend);
void StartCPMusic(Sint16 song);
Sint16  Confirm(const char *string);
void Message(const char *string);
void CheckPause(void);
void ShootSnd(void);
void CheckSecretMissions(void);

void DrawGun(CP_iteminfo *item_i,CP_itemtype *items,Sint16 x,Sint16 *y,Sint16 which,Sint16 basey,void (*routine)(Sint16 w));	
void DrawHalfStep(Sint16 x,Sint16 y,Sint16 y_spacing);
void EraseGun(CP_iteminfo *item_i,CP_itemtype *items,Sint16 x,Sint16 y,Sint16 which);	  
void SetTextColor(CP_itemtype *items,Sint16 hlight);
void DrawMenuGun(CP_iteminfo *iteminfo);
void DrawStripes(Sint16 y);

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(Sint16 index,CustomCtrls *cust,void (*DrawRtn)(Sint16),void (*PrintRtn)(Sint16),Sint16 type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(Sint16 loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(Sint16 view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(Sint16 which);
void DrawCustMouse(Sint16 hilight);
void DrawCustJoy(Sint16 hilight);
void DrawCustKeybd(Sint16 hilight);
void DrawCustKeys(Sint16 hilight);
void PrintCustMouse(Sint16 i);
void PrintCustJoy(Sint16 i);
void PrintCustKeybd(Sint16 i);
void PrintCustKeys(Sint16 i);

void PrintLSEntry(Sint16 w,Sint16 color);
void TrackWhichGame(Sint16 w);
void DrawNewGameDiff(Sint16 w);
void FixupCustom(Sint16 w);

void CP_BlakeStoneSaga(Sint16 temp1);
void CP_NewGame(Sint16 temp1);
void CP_Sound(Sint16 temp1);
Sint16  CP_LoadGame(Sint16 quick);
Sint16  CP_SaveGame(Sint16 quick);
void CP_Control(Sint16 temp1);
void CP_ExitOptions(Sint16 temp1);
void CP_Quit(void);
void CP_ViewScores(Sint16 temp1);
Sint16  CP_EndGame(void);
Sint16  CP_CheckQuick(Uint16 scancode);
void CustomControls(Sint16 temp1);
void MouseSensitivity(Sint16 temp1);

void DrawMenuTitle(const char *title);
void CheckForEpisodes(void);
void HelpPresenter(const char *fname,boolean continuekeys, Uint16 id_cache, boolean startmusic);
void ShadowPrint(const char *string, Sint16 x, Sint16 y);

//
// VARIABLES
//
extern Sint16 SaveGamesAvail[10],StartGame,SoundStatus;
extern char SaveGameNames[10][GAME_DESCRIPTION_LEN+1];

// FOR INPUT TYPES
enum MenuInputType {
    MOUSE,
    JOYSTICK,
    KEYBOARDBTNS,
    KEYBOARDMOVE
}; // enum MenuInputType

//
// WL_INTER
//
struct LRstruct {
	Sint16 kill,secret,treasure;
	Sint32 time;
}; // struct LRstruct

extern LRstruct LevelRatios[];

void Write (Sint16 x,Sint16 y,char *string);
