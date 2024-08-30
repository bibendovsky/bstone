/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_3D_MENU_INCLUDED
#define BSTONE_3D_MENU_INCLUDED


#include <string>

#include "id_in.h"


//
// Menu Color Defines
//

// Text color for text on cursor
#define HIGHLIGHT_TEXT_COLOR (0x59)

// Text color for text on cursor for a turned off item
#define HIGHLIGHT_DISABLED_COLOR (0x56)


#define ENABLED_TEXT_COLOR (0x56)

#define TERM_BACK_COLOR (0x02)

#define HIGHLIGHT (0x13)


extern std::int16_t MENUSONG;
extern std::int16_t ROSTER_MUS;
extern std::int16_t TEXTSONG;


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
	using CarouselFunc = void(*)(
		const int item_index,
		const bool is_left,
		const bool is_right);

	activetypes active{AT_DISABLED};
	std::string string{};

	void (*routine)(
		std::int16_t temp1) = nullptr;

	std::uint8_t fontnumber{}; // Font to print text in
	std::uint8_t height{}; // Hight of text (Y_Offset from previous line)

	CarouselFunc carousel_func_{};
}; // CP_itemtype

struct CustomCtrls
{
	std::int16_t allowed[4];
}; // CustomCtrls

extern CP_itemtype MainMenu[];
extern CP_itemtype NewEMenu[];
extern CP_iteminfo MainItems;


void ClearMScreen();

void ReadAnyControl(
	ControlInfo* ci);

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

void DrawMenuTitle(
	const char* title);

void HelpPresenter(
	const char* fname,
	bool continuekeys,
	std::uint16_t id_cache,
	bool startmusic);

void ShadowPrint(
	const char* string,
	std::int16_t x,
	std::int16_t y);

void CP_ViewScores(
	std::int16_t temp1);

void menu_enable_all_episodes();


#endif // BSTONE_3D_MENU_INCLUDED
