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
//      ID Engine
//      ID_US.h - Header file for the User Manager
//      v1.0d1
//      By Jason Blochowiak
//

#ifndef __ID_US__
#define __ID_US__

#ifdef  __DEBUG__
#define __DEBUG_UserMgr__
#endif

// #define      HELPTEXTLINKED

#define MaxX 320
#define MaxY 200

#define MaxHelpLines 500

#define MaxHighName 57
#define MaxScores 10

struct HighScore {
    char name[MaxHighName + 1];
    int32_t score;
    uint16_t completed;
    uint16_t episode;
    uint16_t ratio;
}; // struct HighScore

#define MaxGameName 32
#define MaxSaveGames 6

struct SaveGame {
    char signature[4];
    uint16_t* oldtest;
    boolean present;
    char name[MaxGameName + 1];
}; // struct SaveGame

#define MaxString 128 // Maximum input string size

// Record used to save & restore screen windows
struct WindowRec {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int16_t px;
    int16_t py;
}; // struct WindowRec;

enum GameDiff {
    gd_Continue,
    gd_Easy,
    gd_Normal,
    gd_Hard
}; // enum GameDiff

// Custom Cursor struct type for US_LineInput()

// JAM - Custom Cursor Support
struct US_CursorStruct {
    char cursor_char;
    char do_not_use; // Space holder for ASCZ string
    uint16_t cursor_color;
    uint16_t font_number;
}; // struct US_CursorStruct

//      Hack import for TED launch support
// extern       boolean         tedlevel;
// extern       uint16_t          tedlevelnum;
extern void TEDDeath();

extern boolean ingame, // Set by game code if a game is in progress
               abortgame, // Set if a game load failed
               loadedgame, // Set if the current game was loaded
               NoWait,
               HighScoresDirty;
extern char* abortprogram; // Set to error msg if program is dying
extern GameDiff restartgame; // Normally gd_Continue, else starts game
extern uint16_t PrintX, PrintY; // Current printing location in the window
extern uint16_t WindowX, WindowY, // Current location of window
              WindowW, WindowH; // Current size of window

extern boolean Button0, Button1,
               CursorBad;
extern int16_t CursorX, CursorY;

extern void (* USL_MeasureString)(const char*, int*, int*),
(*USL_DrawString)(const char*);

extern boolean (* USL_SaveGame)(int16_t), (*USL_LoadGame)(int16_t);
extern void (* USL_ResetGame)();
extern SaveGame Games[MaxSaveGames];
extern HighScore Scores[];

extern US_CursorStruct US_CustomCursor; // JAM
extern boolean use_custom_cursor; // JAM

#define US_HomeWindow() { PrintX = WindowX; PrintY = WindowY; }

extern void US_Startup(),
US_Setup(),
US_Shutdown(),
US_InitRndT(boolean randomize),
US_SetLoadSaveHooks(boolean (* load)(int16_t),
                    boolean (* save)(int16_t),
                    void (* reset)()),
US_TextScreen(),
US_UpdateTextScreen(),
US_FinishTextScreen(),
US_DrawWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h),
US_CenterWindow(uint16_t, uint16_t),
US_SaveWindow(WindowRec * win),
US_RestoreWindow(WindowRec * win),
US_ClearWindow(),
US_SetPrintRoutines(void (* measure)(char*, uint16_t*, uint16_t*),
                    void (* print)(char*)),
US_PrintCentered(const char* s),
US_CPrint(const char* s),
US_CPrintLine(const char* s),
US_Print(const char* s),
US_PrintUnsigned(uint32_t n),
US_PrintSigned(int32_t n),
US_StartCursor(),
US_ShutCursor(),
US_CheckHighScore(int32_t score, uint16_t other),
US_DisplayHighScores(int16_t which);
extern boolean US_UpdateCursor(),
US_LineInput(int16_t x, int16_t y, char* buf, char* def, boolean escok,
             int16_t maxchars, int16_t maxwidth);
extern int16_t US_CheckParm(
    const char* parm,
    const char** strings),

US_RndT();

void USL_PrintInCenter(
    const char* s,
    Rect r);
char* USL_GiveSaveName(
    uint16_t game);
#endif
