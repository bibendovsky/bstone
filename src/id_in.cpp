/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


//
//      ID Engine
//      ID_IN.c - Input Manager
//      v1.0d1
//      By Jason Blochowiak
//

//
//      This module handles dealing with the various input devices
//
//      Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//                              User Mgr (for command line parms)
//
//      Globals:
//              LastScan - The keyboard scan code of the last key pressed
//              LastASCII - The ASCII value of the last key pressed
//      DEBUG - there are more globals
//


#include <cstring>

#include "SDL_events.h"
#include "SDL_version.h"

#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vl.h"


#define KeyInt 9 // The keyboard ISR number

//
// mouse constants
//
#define MReset 0
#define MButtons 3
#define MDelta 11

#define MouseInt 0x33


/*
=============================================================================

										GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
bool MousePresent;
bool NGinstalled = false;


//      Global variables
ControlType ControlTypeUsed; // JAM - added
KeyboardState Keyboard;
bool Paused;
char LastASCII;
ScanCode LastScan;

KeyboardDef KbdDefs = {
	ScanCode::sc_control,
	ScanCode::sc_alt,
	ScanCode::sc_home,
	ScanCode::sc_up_arrow,
	ScanCode::sc_page_up,
	ScanCode::sc_left_arrow,
	ScanCode::sc_right_arrow,
	ScanCode::sc_end,
	ScanCode::sc_down_arrow,
	ScanCode::sc_page_down
}; // KeyboardDef KbdDefs

ControlType Controls[MaxPlayers];

std::uint32_t MouseDownCount;

bool allcaps = false;

Bindings in_bindings;
bool in_is_mouse_grabbed = false;
static bool in_last_is_mouse_grabbed = false;

/*
=============================================================================

										LOCAL VARIABLES

=============================================================================
*/

bool IN_Started;

static Direction DirTable[] = // Quick lookup for total direction
{
	dir_NorthWest, dir_North, dir_NorthEast,
	dir_West, dir_None, dir_East,
	dir_SouthWest, dir_South, dir_SouthEast
};


//      Internal routines

// BBi
bool in_grab_mouse(
	bool grab)
{
	if (grab == in_is_mouse_grabbed)
	{
		return grab;
	}

	auto sdl_result = SDL_SetRelativeMouseMode(
		grab ? SDL_TRUE : SDL_FALSE);

	if (sdl_result == 0)
	{
		in_is_mouse_grabbed = grab;
	}
	else
	{
		in_is_mouse_grabbed = false;
	}

	return in_is_mouse_grabbed;
}

static ScanCode in_keyboard_map_to_bstone(
	SDL_Keycode key_code,
	SDL_Keymod key_mod)
{
	static_cast<void>(key_mod);

	// FIXME There is no reliable way to check for numlock state in SDL2.
	bool is_numlock_active = true;

	switch (key_code)
	{
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		return ScanCode::sc_return;

	case SDLK_ESCAPE:
		return ScanCode::sc_escape;

	case SDLK_SPACE:
	case SDLK_KP_SPACE:
		return ScanCode::sc_space;

	case SDLK_MINUS:
		return ScanCode::sc_minus;

	case SDLK_EQUALS:
		return ScanCode::sc_equals;

	case SDLK_BACKSPACE:
	case SDLK_KP_BACKSPACE:
		return ScanCode::sc_backspace;

	case SDLK_TAB:
	case SDLK_KP_TAB:
		return ScanCode::sc_tab;

	case SDLK_LALT:
	case SDLK_RALT:
		return ScanCode::sc_alt;

	case SDLK_LEFTBRACKET:
	case SDLK_KP_LEFTBRACE:
		return ScanCode::sc_left_bracket;

	case SDLK_RIGHTBRACKET:
	case SDLK_KP_RIGHTBRACE:
		return ScanCode::sc_right_bracket;

	case SDLK_LCTRL:
	case SDLK_RCTRL:
		return ScanCode::sc_control;

	case SDLK_CAPSLOCK:
		return ScanCode::sc_caps_lock;

	case SDLK_NUMLOCKCLEAR:
		return ScanCode::sc_num_lock;

	case SDLK_SCROLLLOCK:
		return ScanCode::sc_scroll_lock;

	case SDLK_LSHIFT:
		return ScanCode::sc_left_shift;

	case SDLK_RSHIFT:
		return ScanCode::sc_right_shift;

	case SDLK_UP:
		return ScanCode::sc_up_arrow;

	case SDLK_KP_8:
		if (is_numlock_active)
		{
			return ScanCode::sc_up_arrow;
		}
		else
		{
			return ScanCode::sc_8;
		}

	case SDLK_DOWN:
		return ScanCode::sc_down_arrow;

	case SDLK_KP_2:
		if (is_numlock_active)
		{
			return ScanCode::sc_down_arrow;
		}
		else
		{
			return ScanCode::sc_2;
		}

	case SDLK_LEFT:
		return ScanCode::sc_left_arrow;

	case SDLK_KP_4:
		if (is_numlock_active)
		{
			return ScanCode::sc_left_arrow;
		}
		else
		{
			return ScanCode::sc_4;
		}

	case SDLK_RIGHT:
		return ScanCode::sc_right_arrow;

	case SDLK_KP_6:
		if (is_numlock_active)
		{
			return ScanCode::sc_right_arrow;
		}
		else
		{
			return ScanCode::sc_6;
		}

	case SDLK_INSERT:
		return ScanCode::sc_insert;

	case SDLK_KP_0:
		if (is_numlock_active)
		{
			return ScanCode::sc_insert;
		}
		else
		{
			return ScanCode::sc_0;
		}

	case SDLK_DELETE:
		return ScanCode::sc_delete;

	case SDLK_KP_COMMA:
		if (is_numlock_active)
		{
			return ScanCode::sc_delete;
		}
		else
		{
			return ScanCode::sc_comma;
		}

	case SDLK_HOME:
		return ScanCode::sc_home;

	case SDLK_KP_7:
		if (is_numlock_active)
		{
			return ScanCode::sc_home;
		}
		else
		{
			return ScanCode::sc_7;
		}

	case SDLK_END:
		return ScanCode::sc_end;

	case SDLK_KP_1:
		if (is_numlock_active)
		{
			return ScanCode::sc_end;
		}
		else
		{
			return ScanCode::sc_1;
		}

	case SDLK_PAGEUP:
		return ScanCode::sc_page_up;

	case SDLK_KP_9:
		if (is_numlock_active)
		{
			return ScanCode::sc_page_up;
		}
		else
		{
			return ScanCode::sc_9;
		}

	case SDLK_PAGEDOWN:
		return ScanCode::sc_page_down;

	case SDLK_KP_3:
		if (is_numlock_active)
		{
			return ScanCode::sc_page_down;
		}
		else
		{
			return ScanCode::sc_3;
		}

	case SDLK_SLASH:
	case SDLK_KP_DIVIDE:
		return ScanCode::sc_slash;

	case SDLK_BACKSLASH:
		return ScanCode::sc_backslash;

	case SDLK_SEMICOLON:
		return ScanCode::sc_semicolon;

	case SDLK_QUOTE:
		return ScanCode::sc_quote;

	case SDLK_PERIOD:
		return ScanCode::sc_period;

	case SDLK_F1:
		return ScanCode::sc_f1;

	case SDLK_F2:
		return ScanCode::sc_f2;

	case SDLK_F3:
		return ScanCode::sc_f3;

	case SDLK_F4:
		return ScanCode::sc_f4;

	case SDLK_F5:
		return ScanCode::sc_f5;

	case SDLK_F6:
		return ScanCode::sc_f6;

	case SDLK_F7:
		return ScanCode::sc_f7;

	case SDLK_F8:
		return ScanCode::sc_f8;

	case SDLK_F9:
		return ScanCode::sc_f9;

	case SDLK_F10:
		return ScanCode::sc_f10;

	case SDLK_F11:
		return ScanCode::sc_f11;

	case SDLK_F12:
		return ScanCode::sc_f12;

	case SDLK_PRINTSCREEN:
		return ScanCode::sc_print_screen;

	case SDLK_PAUSE:
		return ScanCode::sc_pause;

	case SDLK_BACKQUOTE:
		return ScanCode::sc_back_quote;

	case SDLK_1:
		return ScanCode::sc_1;

	case SDLK_2:
		return ScanCode::sc_2;

	case SDLK_3:
		return ScanCode::sc_3;

	case SDLK_4:
		return ScanCode::sc_4;

	case SDLK_5:
		return ScanCode::sc_5;

	case SDLK_6:
		return ScanCode::sc_6;

	case SDLK_7:
		return ScanCode::sc_7;

	case SDLK_8:
		return ScanCode::sc_8;

	case SDLK_9:
		return ScanCode::sc_9;

	case SDLK_0:
		return ScanCode::sc_0;

	case SDLK_a:
	case SDLK_KP_A:
		return ScanCode::sc_a;

	case SDLK_b:
	case SDLK_KP_B:
		return ScanCode::sc_b;

	case SDLK_c:
	case SDLK_KP_C:
		return ScanCode::sc_c;

	case SDLK_d:
	case SDLK_KP_D:
		return ScanCode::sc_d;

	case SDLK_e:
	case SDLK_KP_E:
		return ScanCode::sc_e;

	case SDLK_f:
	case SDLK_KP_F:
		return ScanCode::sc_f;

	case SDLK_g:
		return ScanCode::sc_g;

	case SDLK_h:
		return ScanCode::sc_h;

	case SDLK_i:
		return ScanCode::sc_i;

	case SDLK_j:
		return ScanCode::sc_j;

	case SDLK_k:
		return ScanCode::sc_k;

	case SDLK_l:
		return ScanCode::sc_l;

	case SDLK_m:
		return ScanCode::sc_m;

	case SDLK_n:
		return ScanCode::sc_n;

	case SDLK_o:
		return ScanCode::sc_o;

	case SDLK_p:
		return ScanCode::sc_p;

	case SDLK_q:
		return ScanCode::sc_q;

	case SDLK_r:
		return ScanCode::sc_r;

	case SDLK_s:
		return ScanCode::sc_s;

	case SDLK_t:
		return ScanCode::sc_t;

	case SDLK_u:
		return ScanCode::sc_u;

	case SDLK_v:
		return ScanCode::sc_v;

	case SDLK_w:
		return ScanCode::sc_w;

	case SDLK_x:
		return ScanCode::sc_x;

	case SDLK_y:
		return ScanCode::sc_y;

	case SDLK_z:
		return ScanCode::sc_z;

	case SDLK_KP_MINUS:
		return ScanCode::sc_kp_minus;

	case SDLK_KP_PLUS:
		return ScanCode::sc_kp_plus;

	default:
		return ScanCode::sc_none;
	}
}

static char in_keyboard_map_to_char(
	const SDL_KeyboardEvent& e)
{
	std::uint16_t flags = e.keysym.mod;
	bool is_caps = false;
	bool is_shift = false;
	SDL_Keycode key_code = e.keysym.sym;

	if ((flags & (
		KMOD_LCTRL |
		KMOD_RCTRL |
		KMOD_LALT |
		KMOD_RALT |
		KMOD_LGUI |
		KMOD_RGUI |
		KMOD_MODE)) != 0)
	{
		return '\0';
	}

	switch (key_code)
	{
	case SDLK_ESCAPE:
	case SDLK_BACKSPACE:
	case SDLK_TAB:
	case SDLK_RETURN:
	case SDLK_SPACE:
	case SDLK_DELETE:
		return static_cast<char>(key_code);
	}


	is_shift = ((flags & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0);

	if (allcaps)
	{
		is_caps = true;
	}
	else
	{
		if ((flags & KMOD_CAPS) != 0)
		{
			is_caps = !is_caps;
		}

		if (is_shift)
		{
			is_caps = !is_caps;
		}
	}

	//
	// Keys which depends on L/R Shift
	//

	if (is_shift)
	{
		switch (key_code)
		{
		case SDLK_1:
			return '!';

		case SDLK_2:
			return '@';

		case SDLK_3:
			return '#';

		case SDLK_4:
			return '$';

		case SDLK_5:
			return '%';

		case SDLK_6:
			return '^';

		case SDLK_7:
			return '&';

		case SDLK_8:
			return '*';

		case SDLK_9:
			return '(';

		case SDLK_0:
			return ')';

		case SDLK_MINUS:
			return '_';

		case SDLK_EQUALS:
			return '+';

		case SDLK_LEFTBRACKET:
			return '{';

		case SDLK_RIGHTBRACKET:
			return '}';

		case SDLK_SEMICOLON:
			return ':';

		case SDLK_QUOTE:
			return '"';

		case SDLK_BACKQUOTE:
			return '~';

		case SDLK_BACKSLASH:
			return '|';

		case SDLK_COMMA:
			return '<';

		case SDLK_PERIOD:
			return '>';

		case SDLK_SLASH:
			return '?';
		}
	}
	else
	{
		switch (key_code)
		{
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
		case SDLK_0:
		case SDLK_MINUS:
		case SDLK_EQUALS:
		case SDLK_LEFTBRACKET:
		case SDLK_RIGHTBRACKET:
		case SDLK_SEMICOLON:
		case SDLK_QUOTE:
		case SDLK_BACKQUOTE:
		case SDLK_BACKSLASH:
		case SDLK_COMMA:
		case SDLK_PERIOD:
		case SDLK_SLASH:
			return static_cast<char>(key_code);
		}
	}


	//
	// Keys which depends on Caps Lock & L/R Shift
	//

	switch (key_code)
	{
	case SDLK_a:
	case SDLK_b:
	case SDLK_c:
	case SDLK_d:
	case SDLK_e:
	case SDLK_f:
	case SDLK_g:
	case SDLK_h:
	case SDLK_i:
	case SDLK_j:
	case SDLK_k:
	case SDLK_l:
	case SDLK_m:
	case SDLK_n:
	case SDLK_o:
	case SDLK_p:
	case SDLK_q:
	case SDLK_r:
	case SDLK_s:
	case SDLK_t:
	case SDLK_u:
	case SDLK_v:
	case SDLK_w:
	case SDLK_x:
	case SDLK_y:
	case SDLK_z:
		return is_caps ? static_cast<char>(SDL_toupper(key_code)) :
			static_cast<char>(key_code);
	}

	return '\0';
}

static auto in_is_take_screenshot_key_pressed = false;

static void in_handle_keyboard(
	const SDL_KeyboardEvent& e)
{
	SDL_Keycode key_code = e.keysym.sym;
	SDL_Keymod key_mod = SDL_GetModState();
	ScanCode key = in_keyboard_map_to_bstone(key_code, key_mod);

	if (key == ScanCode::sc_none)
	{
		return;
	}

	// Check for special keys
	if (e.state == SDL_PRESSED)
	{
		const auto& grab_mouse_binding = in_bindings[e_bi_grab_mouse];

		if (grab_mouse_binding[0] == key || grab_mouse_binding[1] == key)
		{
			in_grab_mouse(!in_is_mouse_grabbed);
		}

		const auto& take_screenshot_binding = in_bindings[e_bi_take_screenshot];

		if (take_screenshot_binding[0] == key || take_screenshot_binding[1] == key)
		{
			vid_schedule_take_screenshot();
		}
	}


	bool is_pressed;

	switch (key)
	{
#ifndef __vita__
// (vita) TranslateControllerEvent() does not currently affect the output of SDL_GetModState()
	case ScanCode::sc_alt:
		is_pressed = ((key_mod & KMOD_ALT) != 0);
		break;
#endif
	case ScanCode::sc_control:
		is_pressed = ((key_mod & KMOD_CTRL) != 0);
		break;

	default:
		is_pressed = (e.state == SDL_PRESSED);
		break;
	}

	Keyboard[key] = is_pressed;

	if (is_pressed)
	{
		LastScan = key;

		char key_char = in_keyboard_map_to_char(e);

		if (key_char != '\0')
		{
			LastASCII = key_char;
		}
	}
}

static void in_handle_mouse_buttons(
	const SDL_MouseButtonEvent& e)
{
	ScanCode key = ScanCode::sc_none;
	bool is_pressed = (e.state == SDL_PRESSED);

	switch (e.button)
	{
	case SDL_BUTTON_LEFT:
		key = ScanCode::sc_mouse_left;
		break;

	case SDL_BUTTON_MIDDLE:
		key = ScanCode::sc_mouse_middle;
		break;

	case SDL_BUTTON_RIGHT:
		key = ScanCode::sc_mouse_right;
		break;

	case SDL_BUTTON_X1:
		key = ScanCode::sc_mouse_x1;
		break;

	case SDL_BUTTON_X2:
		key = ScanCode::sc_mouse_x2;
		break;
	}

	if (key != ScanCode::sc_none)
	{
		bool apply_key = true;

		if (!in_is_mouse_grabbed)
		{
			if (in_grab_mouse(true))
			{
				apply_key = false;
			}
		}

		if (apply_key)
		{
			Keyboard[key] = is_pressed;

			if (is_pressed)
			{
				LastScan = key;
			}
		}
	}
}

#ifndef __vita__
static int in_mouse_dx;
static int in_mouse_dy;
#else
int in_mouse_dx;
int in_mouse_dy;
#endif

static void in_handle_mouse_motion(
	const SDL_MouseMotionEvent& e)
{
	if (in_is_mouse_grabbed)
	{
		in_mouse_dx += e.xrel;
		in_mouse_dy += e.yrel;
	}
	else
	{
		in_mouse_dx = 0;
		in_mouse_dy = 0;
	}
}

static void in_handle_mouse_wheel(
	const SDL_MouseWheelEvent& e)
{
	if (!in_is_mouse_grabbed)
	{
		return;
	}

	auto vertical_value = e.y;

#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (e.direction == SDL_MOUSEWHEEL_FLIPPED)
	{
		vertical_value = -vertical_value;
	}
#endif // SDL_VERSION_ATLEAST(2, 0, 4)

	auto scan_code = ScanCode{};

	if (vertical_value < 0)
	{
		scan_code = ScanCode::sc_mouse_wheel_down;
	}
	else if (vertical_value > 0)
	{
		scan_code = ScanCode::sc_mouse_wheel_up;
	}

	if (scan_code != ScanCode::sc_none)
	{
		Keyboard[scan_code] = true;
		LastScan = scan_code;
	}
}

static void in_handle_mouse(
	const SDL_Event& e)
{
	switch (e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		in_handle_mouse_buttons(e.button);
		break;

	case SDL_MOUSEMOTION:
		in_handle_mouse_motion(e.motion);
		break;

	case SDL_MOUSEWHEEL:
		in_handle_mouse_wheel(e.wheel);
		break;
	}
}
// BBi

///////////////////////////////////////////////////////////////////////////
//
//      INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//              mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void INL_GetMouseDelta(
	int* x,
	int* y)
{
	*x = in_mouse_dx;
	*y = in_mouse_dy;
}

///////////////////////////////////////////////////////////////////////////
//
//      INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//              mouse driver
//
///////////////////////////////////////////////////////////////////////////
static int INL_GetMouseButtons()
{
	in_handle_events();

	int result = 0;

	if (Keyboard[ScanCode::sc_mouse_left])
	{
		result |= 1;
	}

	if (Keyboard[ScanCode::sc_mouse_middle])
	{
		result |= 4;
	}

	if (Keyboard[ScanCode::sc_mouse_right])
	{
		result |= 2;
	}

	if (Keyboard[ScanCode::sc_mouse_x1])
	{
		result |= 8;
	}

	if (Keyboard[ScanCode::sc_mouse_x2])
	{
		result |= 16;
	}

	return result;
}


///////////////////////////////////////////////////////////////////////////
//
//      INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
void INL_StartKbd()
{
	IN_ClearKeysDown();
}

///////////////////////////////////////////////////////////////////////////
//
//      INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void INL_ShutKbd()
{
}

///////////////////////////////////////////////////////////////////////////
//
//      INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
bool INL_StartMouse()
{
	return true;
}

// BBi
static void INL_ShutMouse()
{
	// SDL_ShowCursor(SDL_TRUE);
}
// BBi


///////////////////////////////////////////////////////////////////////////
//
//      IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Shutdown()
{
	if (!IN_Started)
	{
		return;
	}

	INL_ShutKbd();

	// BBi
	INL_ShutMouse();

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void IN_ClearKeysDown()
{
	LastScan = ScanCode::sc_none;
	LastASCII = key_None;
	Keyboard.reset();
}

// BBi
static void in_handle_window(
	const SDL_WindowEvent& e)
{
	bool reset_state = false;

	switch (e.event)
	{
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		reset_state = true;

		if (in_last_is_mouse_grabbed)
		{
			in_last_is_mouse_grabbed = in_grab_mouse(true);
		}

		sd_mute(false);

		break;

	case SDL_WINDOWEVENT_FOCUS_LOST:
		reset_state = true;
		in_last_is_mouse_grabbed = in_is_mouse_grabbed;
		static_cast<void>(in_grab_mouse(false));
		sd_mute(true);

		break;
	}

	if (reset_state)
	{
		in_reset_state();
	}
}

void in_handle_events()
{
	in_is_take_screenshot_key_pressed = false;

	SDL_Event e;

	SDL_PumpEvents();

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
#ifdef __vita__
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			TranslateControllerEvent(&e);
			break;
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
			TranslateTouchEvent(&e);
			break;
		case SDL_JOYAXISMOTION:
			TranslateAnalogEvent(&e);
			break;
#endif
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			in_handle_keyboard(e.key);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
			in_handle_mouse(e);
			break;

		case SDL_WINDOWEVENT:
			in_handle_window(e.window);
			break;

		case SDL_QUIT:
			Quit();
		}
	}

	if (in_is_take_screenshot_key_pressed)
	{
		vid_schedule_take_screenshot();
	}
}
// BBi

//
// IN_ReadControl() was modified to allow a single player to use any input
// device at any time without having to physically choose a different
// input device.
//
// PROBLEM: This technique will not work for multi-player games.
//
// Basically, this function should use "player" when in "multi-player"
// mode and ignore "player" when in single player mode.
//
// Anyway, it's not a major problem until we start working with
// with multi-player games.
//
//                                                       - Mike
//

///////////////////////////////////////////////////////////////////////////
//
//      IN_ReadControl() - Reads the device associated with the specified
//              player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void IN_ReadControl(
	std::int16_t player_index,
	ControlInfo* control_info)
{
	static_cast<void>(player_index);

	bool realdelta = false;
	std::uint16_t buttons;
	int dx;
	int dy;
	Motion mx, my;
	KeyboardDef* def;

	// BBi
	in_handle_events();

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

	{

		// JAM begin
		ControlTypeUsed = ctrl_None;

		// Handle keyboard input...
		//
		if (ControlTypeUsed == ctrl_None)
		{
			def = &KbdDefs;

			if (Keyboard[def->upleft])
			{
				mx = motion_Left, my = motion_Up;
			}
			else if (Keyboard[def->upright])
			{
				mx = motion_Right, my = motion_Up;
			}
			else if (Keyboard[def->downleft])
			{
				mx = motion_Left, my = motion_Down;
			}
			else if (Keyboard[def->downright])
			{
				mx = motion_Right, my = motion_Down;
			}

			if (Keyboard[def->up])
			{
				my = motion_Up;
			}
			else if (Keyboard[def->down])
			{
				my = motion_Down;
			}

			if (Keyboard[def->left])
			{
				mx = motion_Left;
			}
			else if (Keyboard[def->right])
			{
				mx = motion_Right;
			}

// Enter/Space/Escape should be enough to enter/leave menu item. [#179]
#if 0
			if (Keyboard[def->button0])
			{
				buttons += 1 << 0;
			}
			if (Keyboard[def->button1])
			{
				buttons += 1 << 1;
			}
#endif // 0

			realdelta = false;
			if (mx || my || buttons)
			{
				ControlTypeUsed = ctrl_Keyboard;
			}
		}

		// Handle mouse input...
		//
		if (MousePresent && (ControlTypeUsed == ctrl_None))
		{
			INL_GetMouseDelta(&dx, &dy);
			buttons = static_cast<std::uint16_t>(INL_GetMouseButtons());
			realdelta = true;
			if (dx || dy || buttons)
			{
				ControlTypeUsed = ctrl_Mouse;
			}
		}
		// JAM end
	}

	if (realdelta)
	{
		mx = (dx < 0) ? motion_Left : ((dx > 0) ? motion_Right : motion_None);
		my = (dy < 0) ? motion_Up : ((dy > 0) ? motion_Down : motion_None);
	}
	else
	{
		dx = mx * 127;
		dy = my * 127;
	}

	control_info->x = dx;
	control_info->xaxis = mx;
	control_info->y = dy;
	control_info->yaxis = my;
	control_info->button0 = buttons & (1 << 0);
	control_info->button1 = buttons & (1 << 1);
	control_info->button2 = buttons & (1 << 2);
	control_info->button3 = buttons & (1 << 3);
	control_info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//      IN_WaitForASCII() - Waits for an ASCII char, then clears LastASCII and
//              returns the ASCII value
//
///////////////////////////////////////////////////////////////////////////
char IN_WaitForASCII()
{
	char result;

	while ((result = LastASCII) == '\0')
	{
		in_handle_events();
	}
	LastASCII = '\0';
	return result;
}

///////////////////////////////////////////////////////////////////////////
//
//      IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

using BtnState = std::bitset<8>;
BtnState btnstate;

void IN_StartAck()
{
	std::uint16_t i, buttons;

	//
	// get initial state of everything
	//
	IN_ClearKeysDown();
	btnstate.reset();

	buttons = 0;

	if (MousePresent)
	{
		buttons |= IN_MouseButtons();
	}

	for (i = 0; i < 8; i++, buttons >>= 1)
	{
		if (buttons & 1)
		{
			btnstate[i] = true;
		}
	}
}

bool IN_CheckAck()
{
	std::uint16_t i, buttons;

	in_handle_events();
	//
	// see if something has been pressed
	//
	if (LastScan != ScanCode::sc_none)
	{
		return true;
	}

	buttons = 0;

	if (MousePresent)
	{
		buttons |= IN_MouseButtons();
	}

	for (i = 0; i < 8; i++, buttons >>= 1)
	{
		if (buttons & 1)
		{
			if (!btnstate[i])
			{
				return true;
			}
		}
		else
		{
			btnstate[i] = false;
		}
	}

	return false;
}

void IN_Ack()
{
	IN_StartAck();

	while (!IN_CheckAck())
	{
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      IN_UserInput() - Waits for the specified delay time (in ticks) or the
//              user pressing a key or a mouse button. If the clear flag is set, it
//              then either clears the key or waits for the user to let the mouse
//              button up.
//
///////////////////////////////////////////////////////////////////////////
bool IN_UserInput(
	std::uint32_t delay)
{
	std::uint32_t lasttime;

	lasttime = TimeCount;
	IN_StartAck();
	do
	{
		VL_WaitVBL(1);
		if (IN_CheckAck())
		{
			return true;
		}
	} while (TimeCount - lasttime < delay);
	return false;
}

std::uint8_t IN_MouseButtons()
{
	return static_cast<std::uint8_t>(INL_GetMouseButtons());
}

void IN_Startup()
{
	if (IN_Started)
	{
		return;
	}

	INL_StartKbd();
	MousePresent = INL_StartMouse();

#ifdef __vita__
	// Vita joysticks are treated separately from other kinds of joystick
	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		SDL_Init(SDL_INIT_JOYSTICK);
	}
	SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);
#endif

	IN_Started = true;
}

// BBi
void in_get_mouse_deltas(
	int& dx,
	int& dy)
{
	INL_GetMouseDelta(&dx, &dy);
}

void in_clear_mouse_deltas()
{
#ifndef __vita__
	in_mouse_dx = 0;
	in_mouse_dy = 0;
#endif
}

void in_set_default_bindings()
{
	for (int b = 0; b < k_max_bindings; ++b)
	{
		for (int k = 0; k < k_max_binding_keys; ++k)
		{
			in_bindings[b][k] = ScanCode::sc_none;
		}
	}

	in_bindings[e_bi_forward][0] = ScanCode::sc_w;
	in_bindings[e_bi_backward][0] = ScanCode::sc_s;
	in_bindings[e_bi_left][0] = ScanCode::sc_left_arrow;
	in_bindings[e_bi_right][0] = ScanCode::sc_right_arrow;
	in_bindings[e_bi_strafe][0] = ScanCode::sc_alt;
	in_bindings[e_bi_strafe_left][0] = ScanCode::sc_a;
	in_bindings[e_bi_strafe_right][0] = ScanCode::sc_d;
	in_bindings[e_bi_run][0] = ScanCode::sc_left_shift;

	in_bindings[e_bi_attack][0] = ScanCode::sc_control;
	in_bindings[e_bi_attack][1] = ScanCode::sc_mouse_left;
	in_bindings[e_bi_weapon_1][0] = ScanCode::sc_1;
	in_bindings[e_bi_weapon_2][0] = ScanCode::sc_2;
	in_bindings[e_bi_weapon_3][0] = ScanCode::sc_3;
	in_bindings[e_bi_weapon_4][0] = ScanCode::sc_4;
	in_bindings[e_bi_weapon_5][0] = ScanCode::sc_5;
	in_bindings[e_bi_weapon_6][0] = ScanCode::sc_6;
	in_bindings[e_bi_weapon_7][0] = ScanCode::sc_back_quote;

	in_bindings[e_bi_cycle_next_weapon][0] = ScanCode::sc_e;
	in_bindings[e_bi_cycle_next_weapon][1] = ScanCode::sc_mouse_wheel_up;

	in_bindings[e_bi_cycle_previous_weapon][0] = ScanCode::sc_q;
	in_bindings[e_bi_cycle_previous_weapon][1] = ScanCode::sc_mouse_wheel_down;

	in_bindings[e_bi_use][0] = ScanCode::sc_space;
	in_bindings[e_bi_use][1] = ScanCode::sc_mouse_right;

	in_bindings[e_bi_stats][0] = ScanCode::sc_tab;
	in_bindings[e_bi_radar_magnify][0] = ScanCode::sc_equals;
	in_bindings[e_bi_radar_minify][0] = ScanCode::sc_minus;

	in_bindings[e_bi_help][0] = ScanCode::sc_f1;
	in_bindings[e_bi_save][0] = ScanCode::sc_f2;
	in_bindings[e_bi_load][0] = ScanCode::sc_f3;
	in_bindings[e_bi_sound][0] = ScanCode::sc_f4;
	in_bindings[e_bi_controls][0] = ScanCode::sc_f6;
	in_bindings[e_bi_end_game][0] = ScanCode::sc_f7;
	in_bindings[e_bi_quick_save][0] = ScanCode::sc_f8;
	in_bindings[e_bi_quick_load][0] = ScanCode::sc_f9;
	in_bindings[e_bi_quick_exit][0] = ScanCode::sc_f10;

	in_bindings[e_bi_pause][0] = ScanCode::sc_p;
	in_bindings[e_bi_pause][1] = ScanCode::sc_pause;

	in_bindings[e_bi_grab_mouse][0] = ScanCode::sc_u;

	in_bindings[e_bi_take_screenshot][0] = ScanCode::sc_f5;
}

bool in_is_binding_pressed(
	BindingId binding_id)
{
	const Binding& binding = in_bindings[binding_id];

	return (binding[0] != ScanCode::sc_none && Keyboard[binding[0]]) ||
		(binding[1] != ScanCode::sc_none && Keyboard[binding[1]]);
}

void in_reset_binding_state(
	BindingId binding_id)
{
	const auto& binding = in_bindings[binding_id];

	if (binding[0] != ScanCode::sc_none)
	{
		Keyboard[binding[0]] = false;
	}

	if (binding[1] != ScanCode::sc_none)
	{
		Keyboard[binding[1]] = false;
	}
}

void in_reset_state()
{
	LastASCII = '\0';
	LastScan = ScanCode::sc_none;

	Keyboard.reset();

	for (int i = 0; i < NUMBUTTONS; ++i)
	{
		buttonstate[i] = false;
		buttonheld[i] = false;
	}

	in_clear_mouse_deltas();
}
// BBi
