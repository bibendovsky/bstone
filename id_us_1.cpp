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


#include "id_heads.h"


// BBi
SDL_TimerID sys_timer_id = 0;
// BBi

#define VW_UpdateScreen() VH_UpdateScreen()
void VH_UpdateScreen();


//      Global variables
char* abortprogram;
//              boolean         NoWait;
uint16_t PrintX, PrintY;
uint16_t WindowX, WindowY, WindowW, WindowH;

US_CursorStruct US_CustomCursor; // JAM
boolean use_custom_cursor = false; // JAM

//      Internal variables
#define ConfigVersion 1

const char* US_ParmStrings[] = { "TEDLEVEL", "NOWAIT", nullptr },
          * US_ParmStrings2[] = { "COMP", "NOCOMP", nullptr };
boolean US_Started;

boolean Button0, Button1,
        CursorBad;
int16_t CursorX, CursorY;

void (* USL_MeasureString)(
    const char*,
    int*,
    int*) = VW_MeasurePropString,
(*USL_DrawString)(const char*) = VWB_DrawPropString;

SaveGame Games[MaxSaveGames];

HighScore Scores[MaxScores] = {
    { "JAM PRODUCTIONS INC.", 10000, 1, 0 },
    { "", 10000, 1, 0 },
    { "JERRY JONES", 10000, 1, 0 },
    { "MICHAEL MAYNARD", 10000, 1, 0 },
    { "JAMES T. ROW", 10000, 1, 0 },
    { "", 10000, 1, 0 },
    { "", 10000, 1, 0 },
    { "TO REGISTER CALL", 10000, 1, 0 },
    { " 1-800-GAME123", 10000, 1, 0 },
    { "", 10000, 1, 0 },
};

//      Internal routines

//      Public routines

///////////////////////////////////////////////////////////////////////////
//
//      US_Shutdown() - Shuts down the User Mgr
//
///////////////////////////////////////////////////////////////////////////
void US_Shutdown()
{
    if (!US_Started) {
        return;
    }

    // BBi
    SDL_RemoveTimer(sys_timer_id);
    SDL_QuitSubSystem(SDL_INIT_TIMER);
    // BBi

    US_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CheckParm() - checks to see if a string matches one of a set of
//              strings. The check is case insensitive. The routine returns the
//              index of the string that matched, or -1 if no matches were found
//
///////////////////////////////////////////////////////////////////////////
int16_t US_CheckParm(
    const char* parm,
    const char** strings)
{
    char cp, cs;
    const char* p, * s;
    int16_t i;

    while (!isalpha(*parm)) {   // Skip non-alphas
        parm++;
    }

    for (i = 0; *strings && **strings; i++) {
        for (s = *strings++, p = parm, cs = cp = 0; cs == cp; ) {
            cs = *s++;
            if (!cs) {
                return i;
            }
            cp = *p++;

            if (isupper(cs)) {
                cs = static_cast<char>(tolower(cs));
            }
            if (isupper(cp)) {
                cp = static_cast<char>(tolower(cp));
            }
        }
    }
    return -1;
}

//      Window/Printing routines

#if 0

///////////////////////////////////////////////////////////////////////////
//
//      US_SetPrintRoutines() - Sets the routines used to measure and print
//              from within the User Mgr. Primarily provided to allow switching
//              between masked and non-masked fonts
//
///////////////////////////////////////////////////////////////////////////
void US_SetPrintRoutines(
    void (* measure)(char*, uint16_t*, uint16_t*),
    void (* print)(char*))
{
    USL_MeasureString = measure;
    USL_DrawString = print;
}

#endif

///////////////////////////////////////////////////////////////////////////
//
//      US_Print() - Prints a string in the current window. Newlines are
//              supported.
//
///////////////////////////////////////////////////////////////////////////
void US_Print(
    const char* s)
{
    std::vector<char> buffer(
        s, s + std::string::traits_type::length(s) + 1);
    s = &buffer[0];

    char c;
    const char* se;
    int w, h;

    while (*s) {
        se = s;
        while ((c = *se) != '\0' && (c != '\n')) {
            se++;
        }
        *(char*)se = '\0';

        USL_MeasureString(s, &w, &h);
        px = PrintX;
        py = PrintY;
        USL_DrawString(s);

        s = se;
        if (c) {
            *(char*)se = c;
            s++;

            PrintX = WindowX;
            PrintY += h;
        } else {
            PrintX += w;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      US_PrintUnsigned() - Prints an unsigned long
//
///////////////////////////////////////////////////////////////////////////
void US_PrintUnsigned(
    uint32_t n)
{
    char buffer[32];

    US_Print(bstone::C::xitoa(n, buffer, 10));
}


#if 0
///////////////////////////////////////////////////////////////////////////
//
//      US_PrintSigned() - Prints a signed long
//
///////////////////////////////////////////////////////////////////////////
void US_PrintSigned(
    int32_t n)
{
    char buffer[32];

    US_Print(ltoa(n, buffer, 10));
}
#endif


///////////////////////////////////////////////////////////////////////////
//
//      USL_PrintInCenter() - Prints a string in the center of the given rect
//
///////////////////////////////////////////////////////////////////////////
void USL_PrintInCenter(
    const char* s,
    Rect r)
{
    int w, h,
        rw, rh;

    USL_MeasureString(s, &w, &h);
    rw = r.lr.x - r.ul.x;
    rh = r.lr.y - r.ul.y;

    px = r.ul.x + ((rw - w) / 2);
    py = r.ul.y + ((rh - h) / 2);
    USL_DrawString(s);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_PrintCentered() - Prints a string centered in the current window.
//
///////////////////////////////////////////////////////////////////////////
void US_PrintCentered(
    const char* s)
{
    Rect r;

    r.ul.x = WindowX;
    r.ul.y = WindowY;
    r.lr.x = r.ul.x + WindowW;
    r.lr.y = r.ul.y + WindowH;

    USL_PrintInCenter(s, r);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CPrintLine() - Prints a string centered on the current line and
//              advances to the next line. Newlines are not supported.
//
///////////////////////////////////////////////////////////////////////////
void US_CPrintLine(
    const char* s)
{
    int w, h;

    USL_MeasureString(s, &w, &h);

    if (w > WindowW) {
        US1_ERROR(US_CPRINTLINE_WIDTH);
    }
    px = WindowX + ((WindowW - w) / 2);
    py = PrintY;
    USL_DrawString(s);
    PrintY += h;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CPrint() - Prints a string in the current window. Newlines are
//              supported.
//
///////////////////////////////////////////////////////////////////////////
void US_CPrint(
    const char* s)
{
    std::vector<char> buffer(
        s,
        s + std::string::traits_type::length(s) + 1);

    s = &buffer[0];

    char c;
    const char* se;

    while (*s) {
        se = s;
        while ((c = *se) && (c != '\n')) {
            se++;
        }
        *(char*)se = '\0';

        US_CPrintLine(s);

        s = se;
        if (c) {
            *(char*)se = c;
            s++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      US_ClearWindow() - Clears the current window to white and homes the
//              cursor
//
///////////////////////////////////////////////////////////////////////////
void US_ClearWindow()
{
    VWB_Bar(WindowX, WindowY, WindowW, WindowH, 0xEF);
    PrintX = WindowX;
    PrintY = WindowY;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_DrawWindow() - Draws a frame and sets the current window parms
//
///////////////////////////////////////////////////////////////////////////
void US_DrawWindow(
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h)
{
    uint16_t i,
           sx, sy, sw, sh;

    WindowX = x * 8;
    WindowY = y * 8;
    WindowW = w * 8;
    WindowH = h * 8;

    PrintX = WindowX;
    PrintY = WindowY;

    sx = (x - 1) * 8;
    sy = (y - 1) * 8;
    sw = (w + 1) * 8;
    sh = (h + 1) * 8;

    US_ClearWindow();

    VWB_DrawTile8(sx, sy, 0), VWB_DrawTile8(sx, sy + sh, 5);
    for (i = sx + 8; i <= sx + sw - 8; i += 8) {
        VWB_DrawTile8(i, sy, 1), VWB_DrawTile8(i, sy + sh, 6);
    }
    VWB_DrawTile8(i, sy, 2), VWB_DrawTile8(i, sy + sh, 7);

    for (i = sy + 8; i <= sy + sh - 8; i += 8) {
        VWB_DrawTile8(sx, i, 3), VWB_DrawTile8(sx + sw, i, 4);
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CenterWindow() - Generates a window of a given width & height in the
//              middle of the screen
//
///////////////////////////////////////////////////////////////////////////
void US_CenterWindow(
    uint16_t w,
    uint16_t h)
{
    US_DrawWindow(((MaxX / 8) - w) / 2, ((MaxY / 8) - h) / 2, w, h);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_SaveWindow() - Saves the current window parms into a record for
//              later restoration
//
///////////////////////////////////////////////////////////////////////////
void US_SaveWindow(
    WindowRec* win)
{
    win->x = WindowX;
    win->y = WindowY;
    win->w = WindowW;
    win->h = WindowH;

    win->px = PrintX;
    win->py = PrintY;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_RestoreWindow() - Sets the current window parms to those held in the
//              record
//
///////////////////////////////////////////////////////////////////////////
void US_RestoreWindow(
    WindowRec* win)
{
    WindowX = win->x;
    WindowY = win->y;
    WindowW = win->w;
    WindowH = win->h;

    PrintX = win->px;
    PrintY = win->py;
}

//      Input routines

///////////////////////////////////////////////////////////////////////////
//
//      USL_XORICursor() - XORs the I-bar text cursor. Used by US_LineInput()
//
///////////////////////////////////////////////////////////////////////////
static void USL_XORICursor(
    int16_t x,
    int16_t y,
    char* s,
    uint16_t cursor)
{
    static boolean status; // VGA doesn't XOR...
    char buf[MaxString];
    int temp;
    int w, h;

    strcpy(buf, s);
    buf[cursor] = '\0';
    USL_MeasureString(buf, &w, &h);

    px = x + w - 1;
    py = y;

    VL_WaitVBL(1);

    if (status ^= 1) {
        USL_DrawString("\x80");
    } else {
        temp = fontcolor;
        fontcolor = backcolor;
        USL_DrawString("\x80");
        fontcolor = static_cast<uint8_t>(temp);
    }

}


// JAM BEGIN - New Function
///////////////////////////////////////////////////////////////////////////
//
//      USL_CustomCursor() - Toggle Displays the custom text cursor.
//      Used by US_LineInput()
//
///////////////////////////////////////////////////////////////////////////
static void USL_CustomCursor(
    int16_t x,
    int16_t y,
    char* s,
    uint16_t cursor)
{
    static boolean status; // VGA doesn't XOR...
    char buf[MaxString];
    int temp, temp_font;
    int w, h;

    strcpy(buf, s);
    buf[cursor] = '\0';
    USL_MeasureString(buf, &w, &h);

    px = x + w - 1;
    py = y;

    temp = fontcolor;
    temp_font = fontnumber;

    fontnumber = US_CustomCursor.font_number;

    if (status ^= 1) {
        fontcolor = static_cast<uint8_t>(US_CustomCursor.cursor_color);
    } else {
        fontcolor = backcolor;
    }

    VL_WaitVBL(1);

    USL_DrawString(&US_CustomCursor.cursor_char);
    fontcolor = static_cast<uint8_t>(temp);
    fontnumber = temp_font;

}
// JAM END - New Function

///////////////////////////////////////////////////////////////////////////
//
//      US_LineInput() - Gets a line of user input at (x,y), the string defaults
//              to whatever is pointed at by def. Input is restricted to maxchars
//              chars or maxwidth pixels wide. If the user hits escape (and escok is
//              true), nothing is copied into buf, and false is returned. If the
//              user hits return, the current string is copied into buf, and true is
//              returned
//
///////////////////////////////////////////////////////////////////////////
boolean US_LineInput(
    int16_t x,
    int16_t y,
    char* buf,
    char* def,
    boolean escok,
    int16_t maxchars,
    int16_t maxwidth)
{
    bool redraw,
         cursorvis, cursormoved,
         done, result = false;
    ScanCode sc;
    char c,
         s[MaxString], olds[MaxString];
    int i,
        cursor,
        w, h,
        len, temp;
    uint32_t lasttime;

    if (def) {
        strcpy(s, def);
    } else {
        *s = '\0';
    }
    *olds = '\0';
    cursor = static_cast<uint16_t>(strlen(s));
    cursormoved = redraw = true;

    cursorvis = done = false;
    lasttime = TimeCount;
    LastASCII = key_None;
    LastScan = sc_none;

    while (!done) {
        if (cursorvis) {
            if (use_custom_cursor) { // JAM
                USL_CustomCursor(x, y, s, cursor); // JAM
            } else {
                USL_XORICursor(x, y, s, cursor);
            }
        }

        ::in_handle_events();

        sc = LastScan;
        LastScan = sc_none;
        c = LastASCII;
        LastASCII = key_None;

        switch (sc) {
#if 0

        case sc_LeftArrow:
            if (cursor) {
                cursor--;
            }
            c = key_None;
            cursormoved = true;
            redraw = use_custom_cursor; // JAM -
            break;
        case sc_RightArrow:
            if (s[cursor]) {
                cursor++;
            }
            c = key_None;
            cursormoved = true;
            redraw = use_custom_cursor; // JAM -
            break;
        case sc_Home:
            cursor = 0;
            c = key_None;
            cursormoved = true;
            redraw = use_custom_cursor; // JAM -
            break;
        case sc_end:
            cursor = strlen(s);
            c = key_None;
            cursormoved = true;
            break;

#endif

        case sc_return:
            strcpy(buf, s);
            done = true;
            result = true;
            c = key_None;
            break;
        case sc_escape:
            if (escok) {
                done = true;
                result = false;
            }
            c = key_None;
            break;

        case sc_backspace:
            if (cursor) {
                strcpy(s + cursor - 1, s + cursor);
                cursor--;
                redraw = true;
            }
            c = key_None;
            cursormoved = true;
            break;

#if 0
        case sc_Delete:
            if (s[cursor]) {
                strcpy(s + cursor, s + cursor + 1);
                redraw = true;
            }
            c = key_None;
            cursormoved = true;
            break;
#endif

        case 0x4c: // Keypad 5
        case sc_up_arrow:
        case sc_down_arrow:
        case sc_page_up:
        case sc_page_down:
        case sc_insert:
            c = key_None;
            break;
        }

        if (c) {
            len = static_cast<uint16_t>(strlen(s));
            USL_MeasureString(s, &w, &h);

            if (isprint(c) && (len < MaxString - 1)
                && ((!maxchars) || (len < maxchars))
                && ((!maxwidth) || (w < maxwidth)))
            {
                for (i = len + 1; i > cursor; i--) {
                    s[i] = s[i - 1];
                }
                s[cursor++] = c;
                redraw = true;
            }
        }

        if (redraw) {
            px = x;
            py = y;
            temp = fontcolor;
            fontcolor = backcolor;
            USL_DrawString(olds);
            fontcolor = static_cast<uint8_t>(temp);
            strcpy(olds, s);

            px = x;
            py = y;
            USL_DrawString(s);

            redraw = false;
        }

        if (cursormoved) {
            cursorvis = false;
            lasttime = TimeCount - TickBase;

            cursormoved = false;
        }

        if (TimeCount - lasttime > TickBase / 2) {
            lasttime = TimeCount;

            cursorvis ^= true;
        }

        if (cursorvis) {
            if (use_custom_cursor) { // JAM
                USL_CustomCursor(x, y, s, cursor); // JAM
            } else {
                USL_XORICursor(x, y, s, cursor);
            }
        }

        VW_UpdateScreen();
    }

    if (cursorvis) {
        if (use_custom_cursor) { // JAM
            USL_CustomCursor(x, y, s, cursor); // JAM
        } else {
            USL_XORICursor(x, y, s, cursor);
        }
    }

    if (!result) {
        px = x;
        py = y;
        USL_DrawString(olds);
    }
    VW_UpdateScreen();

    IN_ClearKeysDown();
    return result;
}
