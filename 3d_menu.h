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

#define MenuFadeIn()	VL_FadeIn(0,255,&vgapal,10)

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
#define SM_Y	64
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
								MM_LOGOFF,
} mm_labels;

// CP_Switch() menu labels
//

typedef enum
{
	SW_LIGHTING,
	SW_REBA_ATTACK_INFO,
	SW_CEILING,
	SW_FLOORS,

} sw_labels;



//
//  ActiveType flags for menu options (SEE CP_itemtype.active)
//
typedef enum
{
	AT_DISABLED = 0,
	AT_ENABLED,
	AT_READIT,
	AT_NON_SELECTABLE,				// Menu Bar/Separator - Not a selectable item.


} activetypes;

//
// TYPEDEFS
//
typedef struct
{
	unsigned char x;
	char y_ofs;
	unsigned char width;
	unsigned char height;
	char on;
} CP_cursortype;

typedef struct
{
	unsigned char x;
	unsigned char y;
	unsigned char amount;
	char curpos;
	unsigned char indent;
	unsigned char y_spacing;

	CP_cursortype cursor;

} CP_iteminfo;


typedef struct
{
	activetypes active;
	char string[36];
	void (* routine)(short temp1);
	unsigned char fontnumber;							// Font to print text in
	unsigned char height;								// Hight of text (Y_Offset from previous line)
} CP_itemtype;


typedef struct
{
	short allowed[4];
} CustomCtrls;

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
short HandleMenu(CP_iteminfo *item_i,CP_itemtype *items,void (*routine)(short w));	
void ClearMScreen(void);
void DrawWindow(short x,short y,short w,short h,short wcolor);
void DrawOutline(short x,short y,short w,short h,short color1,short color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(short count);
void CacheLump(short lumpstart,short lumpend);
void UnCacheLump(short lumpstart,short lumpend);
void StartCPMusic(short song);
short  Confirm(char *string);
void Message(char *string);
void CheckPause(void);
void ShootSnd(void);
void CheckSecretMissions(void);

void DrawGun(CP_iteminfo *item_i,CP_itemtype *items,short x,short *y,short which,short basey,void (*routine)(short w));	
void DrawHalfStep(short x,short y,short y_spacing);
void EraseGun(CP_iteminfo *item_i,CP_itemtype *items,short x,short y,short which);	  
void SetTextColor(CP_itemtype *items,short hlight);
void DrawMenuGun(CP_iteminfo *iteminfo);
void DrawStripes(short y);

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(short index,CustomCtrls *cust,void (*DrawRtn)(short),void (*PrintRtn)(short),short type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(short loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(short view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(short which);
void DrawCustMouse(short hilight);
void DrawCustJoy(short hilight);
void DrawCustKeybd(short hilight);
void DrawCustKeys(short hilight);
void PrintCustMouse(short i);
void PrintCustJoy(short i);
void PrintCustKeybd(short i);
void PrintCustKeys(short i);

void PrintLSEntry(short w,short color);
void TrackWhichGame(short w);
void DrawNewGameDiff(short w);
void FixupCustom(short w);

void CP_BlakeStoneSaga(void);
void CP_NewGame(void);
void CP_Sound(void);
short  CP_LoadGame(short quick);
short  CP_SaveGame(short quick);
void CP_Control(void);
void CP_ChangeView(void);
void CP_ExitOptions(void);
void CP_Quit(void);
void CP_ViewScores(void);
short  CP_EndGame(void);
short  CP_CheckQuick(unsigned short scancode);
void CustomControls(void);
void MouseSensitivity(void);

void DrawMenuTitle(char *title);
void CheckForEpisodes(void);
void HelpPresenter(char *fname,boolean continuekeys, unsigned short id_cache, boolean startmusic);
void ShadowPrint(char *string, short x, short y);

//
// VARIABLES
//
extern short SaveGamesAvail[10],StartGame,SoundStatus;
extern char SaveGameNames[10][GAME_DESCRIPTION_LEN+1], SaveName[13];

enum {MOUSE,JOYSTICK,KEYBOARDBTNS,KEYBOARDMOVE};	// FOR INPUT TYPES

//
// WL_INTER
//
typedef struct {
		short kill,secret,treasure;
		long time;
		} LRstruct;

extern LRstruct LevelRatios[];

void Write (short x,short y,char *string);
