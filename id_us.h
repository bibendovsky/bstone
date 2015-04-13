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
    Sint32 score;
    Uint16 completed;
    Uint16 episode;
    Uint16 ratio;
}; // struct HighScore

#define MaxGameName 32
#define MaxSaveGames 6

struct SaveGame {
    char signature[4];
    Uint16* oldtest;
    boolean present;
    char name[MaxGameName + 1];
}; // struct SaveGame

#define MaxString 128 // Maximum input string size

// Record used to save & restore screen windows
struct WindowRec {
    Sint16 x;
    Sint16 y;
    Sint16 w;
    Sint16 h;
    Sint16 px;
    Sint16 py;
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
    Uint16 cursor_color;
    Uint16 font_number;
}; // struct US_CursorStruct

//      Hack import for TED launch support
// extern       boolean         tedlevel;
// extern       Uint16          tedlevelnum;
extern void TEDDeath();

extern boolean ingame, // Set by game code if a game is in progress
               abortgame, // Set if a game load failed
               loadedgame, // Set if the current game was loaded
               NoWait,
               HighScoresDirty;
extern char* abortprogram; // Set to error msg if program is dying
extern GameDiff restartgame; // Normally gd_Continue, else starts game
extern Uint16 PrintX, PrintY; // Current printing location in the window
extern Uint16 WindowX, WindowY, // Current location of window
              WindowW, WindowH; // Current size of window

extern boolean Button0, Button1,
               CursorBad;
extern Sint16 CursorX, CursorY;

extern void (* USL_MeasureString)(const char*, int*, int*),
(*USL_DrawString)(const char*);

extern boolean (* USL_SaveGame)(Sint16), (*USL_LoadGame)(Sint16);
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
US_SetLoadSaveHooks(boolean (* load)(Sint16),
                    boolean (* save)(Sint16),
                    void (* reset)()),
US_TextScreen(),
US_UpdateTextScreen(),
US_FinishTextScreen(),
US_DrawWindow(Uint16 x, Uint16 y, Uint16 w, Uint16 h),
US_CenterWindow(Uint16, Uint16),
US_SaveWindow(WindowRec * win),
US_RestoreWindow(WindowRec * win),
US_ClearWindow(),
US_SetPrintRoutines(void (* measure)(char*, Uint16*, Uint16*),
                    void (* print)(char*)),
US_PrintCentered(const char* s),
US_CPrint(const char* s),
US_CPrintLine(const char* s),
US_Print(const char* s),
US_PrintUnsigned(Uint32 n),
US_PrintSigned(Sint32 n),
US_StartCursor(),
US_ShutCursor(),
US_CheckHighScore(Sint32 score, Uint16 other),
US_DisplayHighScores(Sint16 which);
extern boolean US_UpdateCursor(),
US_LineInput(Sint16 x, Sint16 y, char* buf, char* def, boolean escok,
             Sint16 maxchars, Sint16 maxwidth);
extern Sint16 US_CheckParm(
    const char* parm,
    const char** strings),

US_RndT();

void USL_PrintInCenter(
    const char* s,
    Rect r);
char* USL_GiveSaveName(
    Uint16 game);
#endif
