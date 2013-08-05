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
	void (* routine)(int temp1);
	unsigned char fontnumber;							// Font to print text in
	unsigned char height;								// Hight of text (Y_Offset from previous line)
} CP_itemtype;


typedef struct
{
	int allowed[4];
} CustomCtrls;

extern CP_itemtype far MainMenu[],far NewEMenu[];
extern CP_iteminfo far MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);
void ControlPanelFree(void);
void ControlPanelAlloc(void);

void DrawMenu(CP_iteminfo far *item_i,CP_itemtype far *items);			
int HandleMenu(CP_iteminfo far *item_i,CP_itemtype far *items,void (*routine)(int w));	
void ClearMScreen(void);
void DrawWindow(int x,int y,int w,int h,int wcolor);
void DrawOutline(int x,int y,int w,int h,int color1,int color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(int count);
void CacheLump(int lumpstart,int lumpend);
void UnCacheLump(int lumpstart,int lumpend);
void StartCPMusic(int song);
int  Confirm(char far *string);
void Message(char far *string);
void CheckPause(void);
void ShootSnd(void);
void CheckSecretMissions(void);

void DrawGun(CP_iteminfo far *item_i,CP_itemtype far *items,int x,int *y,int which,int basey,void (*routine)(int w));	
void DrawHalfStep(int x,int y,int y_spacing);
void EraseGun(CP_iteminfo far *item_i,CP_itemtype far *items,int x,int y,int which);	  
void SetTextColor(CP_itemtype far *items,int hlight);
void DrawMenuGun(CP_iteminfo far *iteminfo);
void DrawStripes(int y);

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(int index,CustomCtrls *cust,void (*DrawRtn)(int),void (*PrintRtn)(int),int type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(int loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(int view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(int which);
void DrawCustMouse(int hilight);
void DrawCustJoy(int hilight);
void DrawCustKeybd(int hilight);
void DrawCustKeys(int hilight);
void PrintCustMouse(int i);
void PrintCustJoy(int i);
void PrintCustKeybd(int i);
void PrintCustKeys(int i);

void PrintLSEntry(int w,int color);
void TrackWhichGame(int w);
void DrawNewGameDiff(int w);
void FixupCustom(int w);

void CP_BlakeStoneSaga(void);
void CP_NewGame(void);
void CP_Sound(void);
int  CP_LoadGame(int quick);
int  CP_SaveGame(int quick);
void CP_Control(void);
void CP_ChangeView(void);
void CP_ExitOptions(void);
void CP_Quit(void);
void CP_ViewScores(void);
int  CP_EndGame(void);
int  CP_CheckQuick(unsigned scancode);
void CustomControls(void);
void MouseSensitivity(void);

void DrawMenuTitle(char *title);
void CheckForEpisodes(void);
void HelpPresenter(char *fname,boolean continuekeys, unsigned id_cache, boolean startmusic);
void ShadowPrint(char far *string, int x, int y);

//
// VARIABLES
//
extern int SaveGamesAvail[10],StartGame,SoundStatus;
extern char far SaveGameNames[10][GAME_DESCRIPTION_LEN+1], far SaveName[13];

enum {MOUSE,JOYSTICK,KEYBOARDBTNS,KEYBOARDMOVE};	// FOR INPUT TYPES

//
// WL_INTER
//
typedef struct {
		int kill,secret,treasure;
		long time;
		} LRstruct;

extern LRstruct LevelRatios[];

void Write (int x,int y,char *string);
