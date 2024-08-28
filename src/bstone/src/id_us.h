/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
//      ID Engine
//      ID_US.h - Header file for the User Manager
//      v1.0d1
//      By Jason Blochowiak
//

#ifndef BSTONE_ID_US_INCLUDED
#define BSTONE_ID_US_INCLUDED

#include <cstdint>

#include <vector>

constexpr auto MaxHighName = 57;
constexpr auto MaxScores = 10;

struct HighScore
{
	char name[MaxHighName + 1]; // !!! Used in saved game.
	std::int32_t score; // !!! Used in saved game.
	std::uint16_t completed; // !!! Used in saved game.
	std::uint16_t episode; // !!! Used in saved game.
	std::uint16_t ratio; // !!! Used in saved game.
};

constexpr auto MaxGameName = 32;
constexpr auto MaxSaveGames = 6;

struct SaveGame
{
	char name[MaxGameName + 1];
};

constexpr auto MaxString = 128; // Maximum input string size

// Record used to save & restore screen windows
struct WindowRec
{
	int x;
	int y;
	int w;
	int h;

	int px;
	int py;
};

enum GameDiff
{
	gd_Continue,
	gd_Easy,
	gd_Normal,
	gd_Hard,
};

// Custom Cursor struct type for US_LineInput()

// JAM - Custom Cursor Support
struct US_CursorStruct
{
	char cursor_char;
	char do_not_use; // Space holder for ASCZ string
	int cursor_color;
	int font_number;
};

struct Point
{
	int x;
	int y;
};

struct Rect
{
	Point ul;
	Point lr;
};

extern bool ingame; // Set by game code if a game is in progress
extern bool loadedgame; // Set if the current game was loaded
extern GameDiff restartgame; // Normally gd_Continue, else starts game
extern int PrintX;
extern int PrintY; // Current printing location in the window
extern int WindowX;
extern int WindowY; // Current location of window
extern int WindowW;
extern int WindowH; // Current size of window

extern void (*USL_MeasureString)(const char*, int*, int*);
extern void (*USL_DrawString)(const char*);

extern bool (*USL_SaveGame)(int);
extern bool (*USL_LoadGame)(int);
extern void (*USL_ResetGame)();
extern SaveGame Games[MaxSaveGames];

using HighScores = std::vector<HighScore>;
extern HighScores Scores;

extern US_CursorStruct US_CustomCursor; // JAM
extern bool use_custom_cursor; // JAM

void US_HomeWindow();

void US_Startup();
void US_Shutdown();
void US_InitRndT(bool randomize);
void US_DrawWindow(int x, int y, int w, int h);
void US_CenterWindow(int w, int h);
void US_SaveWindow(WindowRec* win);
void US_RestoreWindow(WindowRec* win);
void US_ClearWindow();
void US_PrintCentered(const char* s);
void US_CPrint(const char* s);
void US_CPrintLine(const char* s);
void US_Print(const char* s);
void US_PrintUnsigned(std::uint32_t n);

void US_PrintF64(double f64);

bool US_LineInput(
	int x,
	int y,
	char* buf,
	char* def,
	bool escok,
	int maxchars,
	int maxwidth);

int US_RndT();

void USL_PrintInCenter(const char* s, Rect r);

#endif // BSTONE_ID_US_INCLUDED
