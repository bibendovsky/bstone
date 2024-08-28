/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
//      ID Engine
//      ID_US_1.c - User Manager - General routines
//      v1.1d1
//      By Jason Blochowiak
//      Hacked up for Catacomb 3D
//

//
//      This module handles dealing with user input & feedback
//
//      Depends on: Input Mgr, View Mgr, some variables from the Sound, Caching,
//              and Refresh Mgrs, Memory Mgr for background save/restore
//
//      Globals:
//              ingame - Flag set by game indicating if a game is in progress
//      abortgame - Flag set if the current game should be aborted (if a load
//                      game fails)
//              loadedgame - Flag set if a game was loaded
//              abortprogram - Normally nil, this points to a terminal error message
//                      if the program needs to abort
//              restartgame - Normally set to gd_Continue, this is set to one of the
//                      difficulty levels if a new game should be started
//              PrintX, PrintY - Where the User Mgr will print (global coords)
//              WindowX,WindowY,WindowW,WindowH - The dimensions of the current
//                      window
//

#include "id_us.h"

#include <cstring>

#include <atomic>
#include <iterator>
#include <tuple>

#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_char_conv.h"
#include "bstone_game_timer.h"
#include "bstone_logger.h"


// Global variables

bstone::GameTimer TimeCount; // Global time in ticks

int PrintX;
int PrintY;
int WindowX;
int WindowY;
int WindowW;
int WindowH;

US_CursorStruct US_CustomCursor; // JAM
bool use_custom_cursor = false; // JAM

// Internal variables

bool US_Started;

void (*USL_MeasureString)(const char*, int*, int*) = VW_MeasurePropString;
void (*USL_DrawString)(const char*) = VWB_DrawPropString;

SaveGame Games[MaxSaveGames];

HighScores Scores;

// Shuts down the User Mgr
void US_Shutdown()
{
	if (!US_Started)
	{
		return;
	}

	// BBi
	TimeCount.stop();
	// BBi

	US_Started = false;
}

// Prints a string in the current window. Newlines are supported.
void US_Print(const char* s)
{
	std::vector<char> buffer(s, s + std::string::traits_type::length(s) + 1);
	s = &buffer[0];

	char c;
	const char* se;
	int w;
	int h;

	while (*s != '\0')
	{
		se = s;

		while ((c = *se) != '\0' && c != '\n')
		{
			se++;
		}

		*const_cast<char*>(se) = '\0';

		USL_MeasureString(s, &w, &h);
		px = static_cast<std::int16_t>(PrintX);
		py = static_cast<std::int16_t>(PrintY);
		USL_DrawString(s);

		s = se;

		if (c != '\0')
		{
			*const_cast<char*>(se) = c;
			s++;

			PrintX = WindowX;
			PrintY = PrintY + h;
		}
		else
		{
			PrintX = PrintX + w;
		}
	}
}

// Prints an unsigned long
void US_PrintUnsigned(std::uint32_t n)
{
	constexpr auto max_chars = 12;
	char chars[max_chars] = {};
	bstone::to_chars(n, std::begin(chars), std::end(chars));
	US_Print(chars);
}

void US_PrintF64(double f64)
{
	const auto& buffer = std::to_string(f64);
	US_Print(buffer.c_str());
}

// Prints a string in the center of the given rect
void USL_PrintInCenter(const char* s, Rect r)
{
	int w;
	int h;
	int rw;
	int rh;

	USL_MeasureString(s, &w, &h);
	rw = r.lr.x - r.ul.x;
	rh = r.lr.y - r.ul.y;

	px = static_cast<std::int16_t>(r.ul.x + ((rw - w) / 2));
	py = static_cast<std::int16_t>(r.ul.y + ((rh - h) / 2));
	USL_DrawString(s);
}

// Prints a string centered in the current window.
void US_PrintCentered(const char* s)
{
	Rect r;

	r.ul.x = WindowX;
	r.ul.y = WindowY;
	r.lr.x = r.ul.x + WindowW;
	r.lr.y = r.ul.y + WindowH;

	USL_PrintInCenter(s, r);
}

// Prints a string centered on the current line and
// advances to the next line. Newlines are not supported.
void US_CPrintLine(const char* s)
{
	int w;
	int h;

	USL_MeasureString(s, &w, &h);

	if (w > WindowW)
	{
		BSTONE_THROW_STATIC_SOURCE("String exceeds width.");
	}

	px = static_cast<std::int16_t>(WindowX + ((WindowW - w) / 2));
	py = static_cast<std::int16_t>(PrintY);
	USL_DrawString(s);
	PrintY = PrintY + h;
}

// Prints a string in the current window. Newlines are supported.
void US_CPrint(const char* s)
{
	std::string string(s);

	if (string.back() != '\n')
	{
		string += '\n';
	}

	std::string::size_type line_begin = 0;

	while (true)
	{
		const auto line_end = string.find('\n', line_begin);

		if (line_end == std::string::npos)
		{
			break;
		}

		auto substring = string.substr(line_begin, line_end - line_begin);
		US_CPrintLine(substring.c_str());
		line_begin = line_end + 1;
	}
}

// Clears the current window to white and homes the cursor
void US_ClearWindow()
{
	VWB_Bar(WindowX, WindowY, WindowW, WindowH, 0xEF);
	PrintX = WindowX;
	PrintY = WindowY;
}

// Draws a frame and sets the current window parms
void US_DrawWindow(int x, int y, int w, int h)
{
	WindowX = x;
	WindowY = y;
	WindowW = w;
	WindowH = h;

	PrintX = WindowX;
	PrintY = WindowY;

	const auto sx = x - 8;
	const auto sy = y - 8;
	const auto sw = w + 8;
	const auto sh = h + 8;

	US_ClearWindow();

	auto i = 0;

	VWB_DrawTile8(sx, sy, 0);
	VWB_DrawTile8(sx, sy + sh, 5);

	for (i = sx + 8; i <= sx + sw - 8; i += 8)
	{
		VWB_DrawTile8(i, sy, 1);
		VWB_DrawTile8(i, sy + sh, 6);
	}

	VWB_DrawTile8(i, sy, 2);
	VWB_DrawTile8(i, sy + sh, 7);

	for (i = sy + 8; i <= sy + sh - 8; i += 8)
	{
		VWB_DrawTile8(sx, i, 3);
		VWB_DrawTile8(sx + sw, i, 4);
	}
}

// Generates a window of a given width & height in the middle of the screen
void US_CenterWindow(int w, int h)
{
	const auto w8 = w * 8;
	const auto h8 = h * 8;

	const auto x = (vga_ref_width - w8) / 2;
	const auto y = ref_view_top_y + (ref_view_height - h8) / 2;

	US_DrawWindow(x, y, w8, h8);
}

// Saves the current window parms into a record for later restoration
void US_SaveWindow(WindowRec* win)
{
	win->x = WindowX;
	win->y = WindowY;
	win->w = WindowW;
	win->h = WindowH;

	win->px = PrintX;
	win->py = PrintY;
}

// Sets the current window parms to those held in the record
void US_RestoreWindow(WindowRec* win)
{
	WindowX = win->x;
	WindowY = win->y;
	WindowW = win->w;
	WindowH = win->h;

	PrintX = win->px;
	PrintY = win->py;
}

// XORs the I-bar text cursor. Used by US_LineInput()
static void USL_XORICursor(int x, int y, char* s, int cursor)
{
	static bool status; // VGA doesn't XOR...
	char buf[MaxString];
	int temp;
	int w, h;

	strcpy(buf, s);
	buf[cursor] = '\0';
	USL_MeasureString(buf, &w, &h);

	px = static_cast<std::int16_t>(x + w - 1);
	py = static_cast<std::int16_t>(y);

	VL_WaitVBL(1);

	status = !status;

	if (status)
	{
		USL_DrawString("\x80");
	}
	else
	{
		temp = fontcolor;
		fontcolor = backcolor;
		USL_DrawString("\x80");
		fontcolor = static_cast<std::uint8_t>(temp);
	}

}

// JAM BEGIN - New Function

// Toggle Displays the custom text cursor.
// Used by US_LineInput()
static void USL_CustomCursor(int x, int y, char* s, int cursor)
{
	static bool status; // VGA doesn't XOR...
	char buf[MaxString];
	int temp;
	int w;
	int h;

	strcpy(buf, s);
	buf[cursor] = '\0';
	USL_MeasureString(buf, &w, &h);

	px = static_cast<std::int16_t>(x + w - 1);
	py = static_cast<std::int16_t>(y);

	temp = fontcolor;
	auto temp_font = fontnumber;

	fontnumber = static_cast<std::int16_t>(US_CustomCursor.font_number);

	status = !status;

	if (status)
	{
		fontcolor = static_cast<std::uint8_t>(US_CustomCursor.cursor_color);
	}
	else
	{
		fontcolor = backcolor;
	}

	VL_WaitVBL(1);

	USL_DrawString(&US_CustomCursor.cursor_char);
	fontcolor = static_cast<std::uint8_t>(temp);
	fontnumber = temp_font;

}
// JAM END - New Function

// Gets a line of user input at (x,y), the string defaults
// to whatever is pointed at by def. Input is restricted to maxchars
// chars or maxwidth pixels wide. If the user hits escape (and escok is
// true), nothing is copied into buf, and false is returned. If the
// user hits return, the current string is copied into buf, and true is returned
bool US_LineInput(
	int x,
	int y,
	char* buf,
	char* def,
	bool escok,
	int maxchars,
	int maxwidth)
{
	bool redraw;
	bool cursorvis;
	bool cursormoved;
	bool done;
	bool result = false;
	ScanCode sc;
	char c;
	char s[MaxString];
	char olds[MaxString];
	int i;
	int cursor;
	int w;
	int h;
	int len;
	int temp;
	std::int32_t lasttime;

	if (def != nullptr)
	{
		strcpy(s, def);
	}
	else
	{
		*s = '\0';
	}

	*olds = '\0';
	cursor = static_cast<int>(strlen(s));
	cursormoved = true;
	redraw = true;

	cursorvis = false;
	done = false;
	lasttime = TimeCount;
	LastASCII = key_None;
	LastScan = ScanCode::sc_none;

	while (!done)
	{
		if (cursorvis)
		{
			if (use_custom_cursor)
			{
				// JAM
				USL_CustomCursor(x, y, s, cursor); // JAM
			}
			else
			{
				USL_XORICursor(x, y, s, cursor);
			}
		}

		in_handle_events();

		sc = LastScan;
		LastScan = ScanCode::sc_none;
		c = LastASCII;
		LastASCII = key_None;

		switch (sc)
		{
			case ScanCode::sc_return:
				strcpy(buf, s);
				done = true;
				result = true;
				c = key_None;
				break;

			case ScanCode::sc_escape:
				if (escok)
				{
					done = true;
					result = false;
				}

				c = key_None;
				break;

			case ScanCode::sc_backspace:
				if (cursor)
				{
					strcpy(s + cursor - 1, s + cursor);
					cursor--;
					redraw = true;
				}

				c = key_None;
				cursormoved = true;
				break;

			case ScanCode::sc_up_arrow:
			case ScanCode::sc_down_arrow:
			case ScanCode::sc_page_up:
			case ScanCode::sc_page_down:
			case ScanCode::sc_insert:
				c = key_None;
				break;

			default:
				break;
		}

		if (c != '\0')
		{
			len = static_cast<int>(strlen(s));
			USL_MeasureString(s, &w, &h);

			if (isprint(c) && (len < MaxString - 1)
				&& ((!maxchars) || (len < maxchars))
				&& ((!maxwidth) || (w < maxwidth)))
			{
				for (i = len + 1; i > cursor; i--)
				{
					s[i] = s[i - 1];
				}

				s[cursor++] = c;
				redraw = true;
			}
		}

		if (redraw)
		{
			px = static_cast<std::int16_t>(x);
			py = static_cast<std::int16_t>(y);
			temp = fontcolor;
			fontcolor = backcolor;
			USL_DrawString(olds);
			fontcolor = static_cast<std::uint8_t>(temp);
			strcpy(olds, s);

			px = static_cast<std::int16_t>(x);
			py = static_cast<std::int16_t>(y);
			USL_DrawString(s);

			redraw = false;
		}

		if (cursormoved)
		{
			cursorvis = false;
			lasttime = TimeCount - TickBase;

			cursormoved = false;
		}

		if (TimeCount - lasttime > TickBase / 2)
		{
			lasttime = TimeCount;

			cursorvis ^= true;
		}

		if (cursorvis)
		{
			if (use_custom_cursor)
			{
				// JAM
				USL_CustomCursor(x, y, s, cursor); // JAM
			}
			else
			{
				USL_XORICursor(x, y, s, cursor);
			}
		}

		VW_UpdateScreen();
	}

	if (cursorvis)
	{
		if (use_custom_cursor)
		{
			// JAM
			USL_CustomCursor(x, y, s, cursor); // JAM
		}
		else
		{
			USL_XORICursor(x, y, s, cursor);
		}
	}

	if (!result)
	{
		px = static_cast<std::int16_t>(x);
		py = static_cast<std::int16_t>(y);
		USL_DrawString(olds);
	}

	VW_UpdateScreen();
	IN_ClearKeysDown();
	return result;
}

void US_Startup()
{
	if (US_Started)
	{
		return;
	}

	// BBi
	TimeCount.start(TickBase);
	// BBi

	US_InitRndT(true); // Initialize the random number generator
	US_Started = true;
}

void SETFONTCOLOR(int foreground_color, int background_color)
{
	fontcolor = static_cast<std::uint8_t>(foreground_color);
	backcolor = static_cast<std::uint8_t>(background_color);
}
