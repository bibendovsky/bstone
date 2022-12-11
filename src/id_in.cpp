/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
#include <unordered_map>
#include "SDL.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vl.h"
#include "bstone_char_conv.h"
#include "bstone_logger.h"

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

// Global variables

ControlType ControlTypeUsed; // JAM - added
KeyboardState Keyboard;
bool Paused;
char LastASCII;
ScanCode LastScan;

KeyboardDef KbdDefs =
{
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
};

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


namespace {

// in_is_mouse_enabled

constexpr auto in_is_mouse_enabled_cvar_name = bstone::StringView{"in_is_mouse_enabled"};
constexpr auto in_is_mouse_enabled_cvar_default = true;

auto in_is_mouse_enabled_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	in_is_mouse_enabled_cvar_name,
	bstone::CVarFlags::archive,
	in_is_mouse_enabled_cvar_default};

// in_mouse_sensitivity

constexpr auto in_mouse_sensitivity_cvar_name = bstone::StringView{"in_mouse_sensitivity"};
constexpr auto in_mouse_sensitivity_cvar_min = min_mouse_sensitivity;
constexpr auto in_mouse_sensitivity_cvar_max = max_mouse_sensitivity;
constexpr auto in_mouse_sensitivity_cvar_default = default_mouse_sensitivity;

auto in_mouse_sensitivity_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	in_mouse_sensitivity_cvar_name,
	bstone::CVarFlags::archive,
	in_mouse_sensitivity_cvar_default,
	in_mouse_sensitivity_cvar_min,
	in_mouse_sensitivity_cvar_max};

} // namespace

void in_initialize_cvars(bstone::CVarMgr& cvar_mgr)
{
	cvar_mgr.add(in_is_mouse_enabled_cvar);
	cvar_mgr.add(in_mouse_sensitivity_cvar);
}

bool in_grab_mouse(bool grab)
{
	if (grab == in_is_mouse_grabbed)
	{
		return grab;
	}

	const auto sdl_result = SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);

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

// Internal routines

namespace {

ScanCode in_keyboard_map_to_bstone(SDL_Keycode key_code, SDL_Keymod key_mod)
{
	// FIXME There is no reliable way to check for numlock state in SDL.
	auto is_numlock_active = true;

	switch (key_code)
	{
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			return ScanCode::sc_return;

		case SDLK_ESCAPE: return ScanCode::sc_escape;

		case SDLK_SPACE:
		case SDLK_KP_SPACE:
			return ScanCode::sc_space;

		case SDLK_MINUS: return ScanCode::sc_minus;
		case SDLK_EQUALS: return ScanCode::sc_equals;

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

		case SDLK_CAPSLOCK: return ScanCode::sc_caps_lock;
		case SDLK_NUMLOCKCLEAR: return ScanCode::sc_num_lock;
		case SDLK_SCROLLLOCK: return ScanCode::sc_scroll_lock;
		case SDLK_LSHIFT: return ScanCode::sc_left_shift;
		case SDLK_RSHIFT: return ScanCode::sc_right_shift;
		case SDLK_UP: return ScanCode::sc_up_arrow;
		case SDLK_KP_8: return is_numlock_active ? ScanCode::sc_up_arrow : ScanCode::sc_8;
		case SDLK_DOWN: return ScanCode::sc_down_arrow;
		case SDLK_KP_2: return is_numlock_active ? ScanCode::sc_down_arrow : ScanCode::sc_2;
		case SDLK_LEFT: return ScanCode::sc_left_arrow;
		case SDLK_KP_4: return is_numlock_active ? ScanCode::sc_left_arrow : ScanCode::sc_4;
		case SDLK_RIGHT: return ScanCode::sc_right_arrow;
		case SDLK_KP_6: return is_numlock_active ? ScanCode::sc_right_arrow : ScanCode::sc_6;
		case SDLK_INSERT: return ScanCode::sc_insert;
		case SDLK_KP_0: return is_numlock_active ? ScanCode::sc_insert : ScanCode::sc_0;
		case SDLK_DELETE: return ScanCode::sc_delete;
		case SDLK_KP_COMMA: return is_numlock_active ? ScanCode::sc_delete : ScanCode::sc_comma;
		case SDLK_HOME: return ScanCode::sc_home;
		case SDLK_KP_7: return is_numlock_active ? ScanCode::sc_home : ScanCode::sc_7;
		case SDLK_END: return ScanCode::sc_end;
		case SDLK_KP_1: return is_numlock_active ? ScanCode::sc_end : ScanCode::sc_1;
		case SDLK_PAGEUP: return ScanCode::sc_page_up;
		case SDLK_KP_9: return is_numlock_active ? ScanCode::sc_page_up : ScanCode::sc_9;
		case SDLK_PAGEDOWN: return ScanCode::sc_page_down;
		case SDLK_KP_3: return is_numlock_active ? ScanCode::sc_page_down : ScanCode::sc_3;

		case SDLK_SLASH:
		case SDLK_KP_DIVIDE:
			return ScanCode::sc_slash;

		case SDLK_BACKSLASH: return ScanCode::sc_backslash;
		case SDLK_SEMICOLON: return ScanCode::sc_semicolon;
		case SDLK_QUOTE: return ScanCode::sc_quote;
		case SDLK_PERIOD: return ScanCode::sc_period;
		case SDLK_F1: return ScanCode::sc_f1;
		case SDLK_F2: return ScanCode::sc_f2;
		case SDLK_F3: return ScanCode::sc_f3;
		case SDLK_F4: return ScanCode::sc_f4;
		case SDLK_F5: return ScanCode::sc_f5;
		case SDLK_F6: return ScanCode::sc_f6;
		case SDLK_F7: return ScanCode::sc_f7;
		case SDLK_F8: return ScanCode::sc_f8;
		case SDLK_F9: return ScanCode::sc_f9;
		case SDLK_F10: return ScanCode::sc_f10;
		case SDLK_F11: return ScanCode::sc_f11;
		case SDLK_F12: return ScanCode::sc_f12;
		case SDLK_PRINTSCREEN: return ScanCode::sc_print_screen;
		case SDLK_PAUSE: return ScanCode::sc_pause;
		case SDLK_BACKQUOTE: return ScanCode::sc_back_quote;
		case SDLK_1: return ScanCode::sc_1;
		case SDLK_2: return ScanCode::sc_2;
		case SDLK_3: return ScanCode::sc_3;
		case SDLK_4: return ScanCode::sc_4;
		case SDLK_5: return ScanCode::sc_5;
		case SDLK_6: return ScanCode::sc_6;
		case SDLK_7: return ScanCode::sc_7;
		case SDLK_8: return ScanCode::sc_8;
		case SDLK_9: return ScanCode::sc_9;
		case SDLK_0: return ScanCode::sc_0;

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

		case SDLK_g: return ScanCode::sc_g;
		case SDLK_h: return ScanCode::sc_h;
		case SDLK_i: return ScanCode::sc_i;
		case SDLK_j: return ScanCode::sc_j;
		case SDLK_k: return ScanCode::sc_k;
		case SDLK_l: return ScanCode::sc_l;
		case SDLK_m: return ScanCode::sc_m;
		case SDLK_n: return ScanCode::sc_n;
		case SDLK_o: return ScanCode::sc_o;
		case SDLK_p: return ScanCode::sc_p;
		case SDLK_q: return ScanCode::sc_q;
		case SDLK_r: return ScanCode::sc_r;
		case SDLK_s: return ScanCode::sc_s;
		case SDLK_t: return ScanCode::sc_t;
		case SDLK_u: return ScanCode::sc_u;
		case SDLK_v: return ScanCode::sc_v;
		case SDLK_w: return ScanCode::sc_w;
		case SDLK_x: return ScanCode::sc_x;
		case SDLK_y: return ScanCode::sc_y;
		case SDLK_z: return ScanCode::sc_z;
		case SDLK_KP_MINUS: return ScanCode::sc_kp_minus;
		case SDLK_KP_PLUS: return ScanCode::sc_kp_plus;
		default: return ScanCode::sc_none;
	}
}

char in_keyboard_map_to_char(const SDL_KeyboardEvent& e)
{
	auto flags = e.keysym.mod;
	auto is_caps = false;
	auto is_shift = false;
	auto key_code = e.keysym.sym;

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
			case SDLK_1: return '!';
			case SDLK_2: return '@';
			case SDLK_3: return '#';
			case SDLK_4: return '$';
			case SDLK_5: return '%';
			case SDLK_6: return '^';
			case SDLK_7: return '&';
			case SDLK_8: return '*';
			case SDLK_9: return '(';
			case SDLK_0: return ')';
			case SDLK_MINUS: return '_';
			case SDLK_EQUALS: return '+';
			case SDLK_LEFTBRACKET: return '{';
			case SDLK_RIGHTBRACKET: return '}';
			case SDLK_SEMICOLON: return ':';
			case SDLK_QUOTE: return '"';
			case SDLK_BACKQUOTE: return '~';
			case SDLK_BACKSLASH: return '|';
			case SDLK_COMMA: return '<';
			case SDLK_PERIOD: return '>';
			case SDLK_SLASH: return '?';
			default: break;
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
	// Keys which depends on Caps MutexLock & L/R Shift
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
			return static_cast<char>(is_caps ? SDL_toupper(key_code) : key_code);
	}

	return '\0';
}

auto in_is_take_screenshot_key_pressed = false;

void in_handle_keyboard(const SDL_KeyboardEvent& e)
{
	const auto key_code = e.keysym.sym;
	const auto key_mod = SDL_GetModState();
	const auto key = in_keyboard_map_to_bstone(key_code, key_mod);

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


	auto is_pressed = false;

	switch (key)
	{
#ifndef __vita__
		// (vita) TranslateControllerEvent() does not currently affect the output of SDL_GetModState()
		case ScanCode::sc_alt: is_pressed = ((key_mod & KMOD_ALT) != 0); break;
#endif
		case ScanCode::sc_control: is_pressed = ((key_mod & KMOD_CTRL) != 0); break;
		default: is_pressed = (e.state == SDL_PRESSED); break;
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

void in_handle_mouse_buttons( const SDL_MouseButtonEvent& e)
{
	auto key = ScanCode::sc_none;
	auto is_pressed = (e.state == SDL_PRESSED);

	switch (e.button)
	{
		case SDL_BUTTON_LEFT: key = ScanCode::sc_mouse_left; break;
		case SDL_BUTTON_MIDDLE: key = ScanCode::sc_mouse_middle; break;
		case SDL_BUTTON_RIGHT: key = ScanCode::sc_mouse_right; break;
		case SDL_BUTTON_X1: key = ScanCode::sc_mouse_x1; break;
		case SDL_BUTTON_X2: key = ScanCode::sc_mouse_x2; break;
		default: break;
	}

	if (key != ScanCode::sc_none)
	{
		auto apply_key = true;

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

} // namespace

#ifndef __vita__
namespace {
int in_mouse_dx;
int in_mouse_dy;
} // namespace
#else
int in_mouse_dx;
int in_mouse_dy;
#endif

namespace {

void in_handle_mouse_motion( const SDL_MouseMotionEvent& e)
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

void in_handle_mouse_wheel( const SDL_MouseWheelEvent& e)
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

void in_handle_mouse(const SDL_Event& e)
{
	switch (e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		in_handle_mouse_buttons(e.button);
		break;

	case SDL_MOUSEMOTION: in_handle_mouse_motion(e.motion); break;
	case SDL_MOUSEWHEEL: in_handle_mouse_wheel(e.wheel); break;
	default: break;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//              mouse driver
//
///////////////////////////////////////////////////////////////////////////
void INL_GetMouseDelta(int* x, int* y)
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
int INL_GetMouseButtons()
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

} // namespace

///////////////////////////////////////////////////////////////////////////
//
//      INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
void INL_StartKbd()
{
	IN_ClearKeysDown();
}

namespace {

///////////////////////////////////////////////////////////////////////////
//
//      INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
void INL_ShutKbd()
{
}

} // namespace

///////////////////////////////////////////////////////////////////////////
//
//      INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
bool INL_StartMouse()
{
	return true;
}

void INL_ShutMouse()
{
}

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
	INL_ShutMouse();
	in_gc_shutdown();
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

namespace {

void in_handle_window(const SDL_WindowEvent& e)
{
	auto reset_state = false;

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
			in_grab_mouse(false);
			sd_mute(true);
			break;

		default: break;
	}

	if (reset_state)
	{
		in_reset_state();
	}
}

void in_gc_handle_axis(const SDL_ControllerAxisEvent& e);
void in_gc_handle_button(const SDL_ControllerButtonEvent& e);
void in_gc_handle_device(const SDL_ControllerDeviceEvent& e);
void in_gc_handle_touchpad(const SDL_ControllerTouchpadEvent& e);
void in_gc_handle_sensor(const SDL_ControllerSensorEvent& e);

} // namespace

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

			case SDL_CONTROLLERAXISMOTION: in_gc_handle_axis(e.caxis); break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				in_gc_handle_button(e.cbutton);
				break;

			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				in_gc_handle_device(e.cdevice);
				break;

			case SDL_CONTROLLERTOUCHPADDOWN:
			case SDL_CONTROLLERTOUCHPADMOTION:
			case SDL_CONTROLLERTOUCHPADUP:
				in_gc_handle_touchpad(e.ctouchpad);
				break;

			case SDL_CONTROLLERSENSORUPDATE:
				in_gc_handle_sensor(e.csensor);
				break;

			case SDL_WINDOWEVENT: in_handle_window(e.window); break;
			case SDL_QUIT: Quit();
			default: break;
		}
	}

	if (in_is_take_screenshot_key_pressed)
	{
		vid_schedule_take_screenshot();
	}
}

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
void IN_ReadControl(std::int16_t player_index, ControlInfo* control_info)
{
	in_handle_events();

	auto realdelta = false;
	auto dx = 0;
	auto dy = 0;
	auto mx = motion_None;
	auto my = motion_None;
	KeyboardDef* def;
	auto buttons = 0;

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
#endif

			realdelta = false;

			if (mx != motion_None || my != motion_None || buttons != 0)
			{
				ControlTypeUsed = ctrl_Keyboard;
			}
		}

		// Handle mouse input...
		//
		if (MousePresent && (ControlTypeUsed == ctrl_None))
		{
			INL_GetMouseDelta(&dx, &dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;

			if (dx != 0 || dy != 0 || buttons != 0)
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
	auto result = '\0';

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

namespace {
using BtnState = std::bitset<8>;
BtnState btnstate;
} // namespace

void IN_StartAck()
{
	//
	// get initial state of everything
	//
	IN_ClearKeysDown();
	btnstate.reset();

	auto buttons = 0;

	if (MousePresent)
	{
		buttons |= IN_MouseButtons();
	}

	for (auto i = 0; i < 8; ++i, buttons >>= 1)
	{
		if ((buttons & 1) != 0)
		{
			btnstate[i] = true;
		}
	}
}

bool IN_CheckAck()
{
	in_handle_events();

	//
	// see if something has been pressed
	//
	if (LastScan != ScanCode::sc_none)
	{
		return true;
	}

	auto buttons = 0;

	if (MousePresent)
	{
		buttons |= IN_MouseButtons();
	}

	for (auto i = 0; i < 8; ++i, buttons >>= 1)
	{
		if ((buttons & 1) != 0)
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
bool IN_UserInput(std::uint32_t delay)
{
	auto lasttime = TimeCount.load(std::memory_order_acquire);

	IN_StartAck();

	do
	{
		VL_WaitVBL(1);

		if (IN_CheckAck())
		{
			return true;
		}
	} while ((TimeCount.load(std::memory_order_acquire) - lasttime) < delay);

	return false;
}

std::uint8_t IN_MouseButtons()
{
	return static_cast<std::uint8_t>(INL_GetMouseButtons());
}

bool in_is_mouse_enabled() noexcept
{
	return in_is_mouse_enabled_cvar.get_bool();
}

void in_set_is_mouse_enabled(bool is_enabled)
{
	in_is_mouse_enabled_cvar.set_bool(is_enabled);
}

int in_get_mouse_sensitivity() noexcept
{
	return in_mouse_sensitivity_cvar.get_int32();
}

void in_set_mouse_sensitivity(int sensitivity)
{
	in_mouse_sensitivity_cvar.set_int32(sensitivity);
}

void IN_Startup()
{
	if (IN_Started)
	{
		return;
	}

	INL_StartKbd();
	MousePresent = INL_StartMouse();

#if FIXMENOW
#ifdef __vita__
	// Vita joysticks are treated separately from other kinds of joystick
	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		SDL_Init(SDL_INIT_JOYSTICK);
	}

	SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);
#endif
#else
	in_gc_startup();
#endif

	IN_Started = true;
}

// BBi
void in_get_mouse_deltas(int& dx, int& dy)
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

void in_clear_bindings()
{
	for (auto& binding_slots : in_bindings)
	{
		for (auto& binding_slot : binding_slots)
		{
			binding_slot = ScanCode::sc_none;
		}
	}
}

void in_set_default_bindings()
{
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

bool in_is_binding_pressed(BindingId binding_id)
{
	const auto& binding = in_bindings[binding_id];

	return
		(binding[0] != ScanCode::sc_none && Keyboard[binding[0]]) ||
		(binding[1] != ScanCode::sc_none && Keyboard[binding[1]]);
}

void in_reset_binding_state(BindingId binding_id)
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

	for (auto i = 0; i < NUMBUTTONS; ++i)
	{
		buttonstate[i] = false;
		buttonheld[i] = false;
	}

	in_clear_mouse_deltas();
}

namespace {

constexpr auto forward_sv = bstone::StringView{"forward"};
constexpr auto backward_sv = bstone::StringView{"backward"};
constexpr auto left_sv = bstone::StringView{"left"};
constexpr auto right_sv = bstone::StringView{"right"};
constexpr auto strafe_sv = bstone::StringView{"strafe"};
constexpr auto strafe_left_sv = bstone::StringView{"strafe_left"};
constexpr auto strafe_right_sv = bstone::StringView{"strafe_right"};
constexpr auto quick_left_sv = bstone::StringView{"quick_left"};
constexpr auto quick_right_sv = bstone::StringView{"quick_right"};
constexpr auto turn_around_sv = bstone::StringView{"turn_around"};
constexpr auto run_sv = bstone::StringView{"run"};

constexpr auto attack_sv = bstone::StringView{"attack"};
constexpr auto weapon_1_sv = bstone::StringView{"weapon_1"};
constexpr auto weapon_2_sv = bstone::StringView{"weapon_2"};
constexpr auto weapon_3_sv = bstone::StringView{"weapon_3"};
constexpr auto weapon_4_sv = bstone::StringView{"weapon_4"};
constexpr auto weapon_5_sv = bstone::StringView{"weapon_5"};
constexpr auto weapon_6_sv = bstone::StringView{"weapon_6"};
constexpr auto weapon_7_sv = bstone::StringView{"weapon_7"};

constexpr auto use_sv = bstone::StringView{"use"};

constexpr auto stats_sv = bstone::StringView{"stats"};
constexpr auto radar_magnify_sv = bstone::StringView{"radar_magnify"};
constexpr auto radar_minify_sv = bstone::StringView{"radar_minify"};

constexpr auto help_sv = bstone::StringView{"help"};
constexpr auto save_sv = bstone::StringView{"save"};
constexpr auto load_sv = bstone::StringView{"load"};
constexpr auto sound_sv = bstone::StringView{"sound"};
constexpr auto controls_sv = bstone::StringView{"controls"};
constexpr auto end_game_sv = bstone::StringView{"end_game"};
constexpr auto quick_save_sv = bstone::StringView{"quick_save"};
constexpr auto quick_load_sv = bstone::StringView{"quick_load"};
constexpr auto quick_exit_sv = bstone::StringView{"quick_exit"};

constexpr auto attack_info_sv = bstone::StringView{"attack_info"};
constexpr auto lighting_sv = bstone::StringView{"lighting"};
constexpr auto sfx_sv = bstone::StringView{"sfx"};
constexpr auto music_sv = bstone::StringView{"music"};
constexpr auto ceiling_sv = bstone::StringView{"ceiling"};
constexpr auto floor_sv = bstone::StringView{"floor"};
constexpr auto heart_beat_sv = bstone::StringView{"heart_beat"};

constexpr auto pause_sv = bstone::StringView{"pause"};
constexpr auto grab_mouse_sv = bstone::StringView{"grab_mouse"};

constexpr auto previous_weapon_sv = bstone::StringView{"previous_weapon"};
constexpr auto next_weapon_sv = bstone::StringView{"next_weapon"};

constexpr auto screenshot_sv = bstone::StringView{"screenshot"};

struct InBindingIdNameToIdMapItem
{
	const bstone::StringView& name_sv;
	BindingId binding_id;
};

constexpr InBindingIdNameToIdMapItem in_binding_id_name_to_id_map[] =
{
	InBindingIdNameToIdMapItem{forward_sv, e_bi_forward},
	InBindingIdNameToIdMapItem{backward_sv, e_bi_backward},
	InBindingIdNameToIdMapItem{left_sv, e_bi_left},
	InBindingIdNameToIdMapItem{right_sv, e_bi_right},
	InBindingIdNameToIdMapItem{strafe_sv, e_bi_strafe},
	InBindingIdNameToIdMapItem{strafe_left_sv, e_bi_strafe_left},
	InBindingIdNameToIdMapItem{strafe_right_sv, e_bi_strafe_right},
	InBindingIdNameToIdMapItem{quick_left_sv, e_bi_quick_left},
	InBindingIdNameToIdMapItem{quick_right_sv, e_bi_quick_right},
	InBindingIdNameToIdMapItem{turn_around_sv, e_bi_turn_around},
	InBindingIdNameToIdMapItem{run_sv, e_bi_run},

	InBindingIdNameToIdMapItem{attack_sv, e_bi_attack},
	InBindingIdNameToIdMapItem{weapon_1_sv, e_bi_weapon_1},
	InBindingIdNameToIdMapItem{weapon_2_sv, e_bi_weapon_2},
	InBindingIdNameToIdMapItem{weapon_3_sv, e_bi_weapon_3},
	InBindingIdNameToIdMapItem{weapon_4_sv, e_bi_weapon_4},
	InBindingIdNameToIdMapItem{weapon_5_sv, e_bi_weapon_5},
	InBindingIdNameToIdMapItem{weapon_6_sv, e_bi_weapon_6},
	InBindingIdNameToIdMapItem{weapon_7_sv, e_bi_weapon_7},

	InBindingIdNameToIdMapItem{use_sv, e_bi_use},

	InBindingIdNameToIdMapItem{stats_sv, e_bi_stats},
	InBindingIdNameToIdMapItem{radar_magnify_sv, e_bi_radar_magnify},
	InBindingIdNameToIdMapItem{radar_minify_sv, e_bi_radar_minify},

	InBindingIdNameToIdMapItem{help_sv, e_bi_help},
	InBindingIdNameToIdMapItem{save_sv, e_bi_save},
	InBindingIdNameToIdMapItem{load_sv, e_bi_load},
	InBindingIdNameToIdMapItem{sound_sv, e_bi_sound},
	InBindingIdNameToIdMapItem{controls_sv, e_bi_controls},
	InBindingIdNameToIdMapItem{end_game_sv, e_bi_end_game},
	InBindingIdNameToIdMapItem{quick_save_sv, e_bi_quick_save},
	InBindingIdNameToIdMapItem{quick_load_sv, e_bi_quick_load},
	InBindingIdNameToIdMapItem{quick_exit_sv, e_bi_quick_exit},

	InBindingIdNameToIdMapItem{attack_info_sv, e_bi_attack_info},
	InBindingIdNameToIdMapItem{lighting_sv, e_bi_lightning},
	InBindingIdNameToIdMapItem{sfx_sv, e_bi_sfx},
	InBindingIdNameToIdMapItem{music_sv, e_bi_music},
	InBindingIdNameToIdMapItem{ceiling_sv, e_bi_ceiling},
	InBindingIdNameToIdMapItem{floor_sv, e_bi_flooring},
	InBindingIdNameToIdMapItem{heart_beat_sv, e_bi_heart_beat},

	InBindingIdNameToIdMapItem{pause_sv, e_bi_pause},
	InBindingIdNameToIdMapItem{grab_mouse_sv, e_bi_grab_mouse},

	InBindingIdNameToIdMapItem{previous_weapon_sv, e_bi_cycle_previous_weapon},
	InBindingIdNameToIdMapItem{next_weapon_sv, e_bi_cycle_next_weapon},

	InBindingIdNameToIdMapItem{screenshot_sv, e_bi_take_screenshot},
};

class InBindingException : public bstone::Exception
{
public:
	explicit InBindingException(const char* message) noexcept
		:
		Exception{"BSTONE_IN_BINDING", message}
	{}
};

BindingId in_binding_name_to_id(bstone::StringView name_sv)
try
{
	for (const auto& map_item_id : in_binding_id_name_to_id_map)
	{
		if (map_item_id.name_sv == name_sv)
		{
			return map_item_id.binding_id;
		}
	}

	auto message = std::string{};
	message += "Unknown binding name \"";
	message.append(name_sv.get_data(), static_cast<std::size_t>(name_sv.get_size()));
	message += "\".";
	throw InBindingException{message.c_str()};
}
catch (...)
{
	std::throw_with_nested(__func__);
}

bstone::StringView in_binding_id_to_name(BindingId binding_id)
try
{
	for (const auto& map_item_id : in_binding_id_name_to_id_map)
	{
		if (map_item_id.binding_id == binding_id)
		{
			return map_item_id.name_sv;
		}
	}

	throw InBindingException{"Unknown binding ID."};
}
catch (...)
{
	std::throw_with_nested(__func__);
}

// --------------------------------------------------------------------------

class InClearBindingsException : public bstone::Exception
{
public:
	explicit InClearBindingsException(const char* message) noexcept
		:
		Exception{"BSTONE_IN_CLEAR_BINDINGS", message}
	{}
};

class InClearBindingsCCmdAction final : public bstone::CCmdAction
{
public:
	InClearBindingsCCmdAction() noexcept = default;
	~InClearBindingsCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try
	{
		if (!args.is_empty())
		{
			fail("Too many arguments.");
		}

		in_clear_bindings();
	}
	catch (...)
	{
		fail_nested(__func__);
	}

private:
	[[noreturn]] static void fail(const char* message)
	{
		throw InClearBindingsException{message};
	}

	[[noreturn]] static void fail_nested(const char* message)
	{
		std::throw_with_nested(InClearBindingsException{message});
	}
};

constexpr auto in_clear_bindings_sv = bstone::StringView{"in_clear_bindings"};
auto in_clear_bindings_ccmd_action = InClearBindingsCCmdAction{};
auto in_clear_bindings_ccmd = bstone::CCmd{in_clear_bindings_sv, in_clear_bindings_ccmd_action};

// --------------------------------------------------------------------------

bstone::Int in_parse_binding_slot_index(bstone::StringView slot_index_name_sv)
try
{
	const auto slot_index = bstone::char_conv::from_chars<bstone::Int>(slot_index_name_sv.to_span());

	if (slot_index < 0 || slot_index > k_max_binding_keys)
	{
		auto message = std::string{};
		message += "Slot index \"";
		message.append(slot_index_name_sv.get_data(), static_cast<std::size_t>(slot_index_name_sv.get_size()));
		message += "\" out of range.";
		throw InBindingException{message.c_str()};
	}

	return slot_index;
}
catch (...)
{
	std::throw_with_nested(InBindingException{__func__});
}

// --------------------------------------------------------------------------

class InClearBindingException : public bstone::Exception
{
public:
	explicit InClearBindingException(const char* message) noexcept
		:
		Exception{"BSTONE_IN_CLEAR_BINDING", message}
	{}
};

class InClearBindingCCmdAction final : public bstone::CCmdAction
{
public:
	InClearBindingCCmdAction() noexcept = default;
	~InClearBindingCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try
	{
		if (args.get_size() != 2)
		{
			fail("Invalid argument count.");
		}

		const auto binding_id = in_binding_name_to_id(args[0]);
		const auto slot_index = in_parse_binding_slot_index(args[1]);
		in_bindings[binding_id][slot_index] = ScanCode::sc_none;
	}
	catch (...)
	{
		fail_nested(__func__);
	}

private:
	[[noreturn]] static void fail(const char* message)
	{
		throw InClearBindingsException{message};
	}

	[[noreturn]] static void fail_nested(const char* message)
	{
		std::throw_with_nested(InClearBindingsException{message});
	}
};

constexpr auto in_clear_binding_sv = bstone::StringView{"in_clear_binding"};
auto in_clear_binding_ccmd_action = InClearBindingCCmdAction{};
auto in_clear_binding_ccmd = bstone::CCmd{in_clear_binding_sv, in_clear_binding_ccmd_action};

// --------------------------------------------------------------------------

constexpr auto in_return_sv = bstone::StringView{"return"};
constexpr auto in_escape_sv = bstone::StringView{"escape"};
constexpr auto in_space_sv = bstone::StringView{"space"};
constexpr auto in_minus_sv = bstone::StringView{"minus"};
constexpr auto in_equals_sv = bstone::StringView{"equals"};
constexpr auto in_backspace_sv = bstone::StringView{"backspace"};
constexpr auto in_tab_sv = bstone::StringView{"tab"};
constexpr auto in_alt_sv = bstone::StringView{"alt"};
constexpr auto in_left_bracket_sv = bstone::StringView{"left_bracket"};
constexpr auto in_right_bracket_sv = bstone::StringView{"right_bracket"};
constexpr auto in_control_sv = bstone::StringView{"control"};
constexpr auto in_caps_lock_sv = bstone::StringView{"caps_lock"};
constexpr auto in_num_lock_sv = bstone::StringView{"num_lock"};
constexpr auto in_scroll_lock_sv = bstone::StringView{"scroll_lock"};
constexpr auto in_left_shift_sv = bstone::StringView{"left_shift"};
constexpr auto in_right_shift_sv = bstone::StringView{"right_shift"};
constexpr auto in_up_arrow_sv = bstone::StringView{"up_arrow"};
constexpr auto in_down_arrow_sv = bstone::StringView{"down_arrow"};
constexpr auto in_left_arrow_sv = bstone::StringView{"left_arrow"};
constexpr auto in_right_arrow_sv = bstone::StringView{"right_arrow"};
constexpr auto in_insert_sv = bstone::StringView{"insert"};
constexpr auto in_delete_sv = bstone::StringView{"delete"};
constexpr auto in_home_sv = bstone::StringView{"home"};
constexpr auto in_end_sv = bstone::StringView{"end"};
constexpr auto in_page_up_sv = bstone::StringView{"page_up"};
constexpr auto in_page_down_sv = bstone::StringView{"page_down"};
constexpr auto in_slash_sv = bstone::StringView{"slash"};
constexpr auto in_f1_sv = bstone::StringView{"f1"};
constexpr auto in_f2_sv = bstone::StringView{"f2"};
constexpr auto in_f3_sv = bstone::StringView{"f3"};
constexpr auto in_f4_sv = bstone::StringView{"f4"};
constexpr auto in_f5_sv = bstone::StringView{"f5"};
constexpr auto in_f6_sv = bstone::StringView{"f6"};
constexpr auto in_f7_sv = bstone::StringView{"f7"};
constexpr auto in_f8_sv = bstone::StringView{"f8"};
constexpr auto in_f9_sv = bstone::StringView{"f9"};
constexpr auto in_f10_sv = bstone::StringView{"f10"};
constexpr auto in_f11_sv = bstone::StringView{"f11"};
constexpr auto in_f12_sv = bstone::StringView{"f12"};
constexpr auto in_print_screen_sv = bstone::StringView{"print_screen"};
constexpr auto in_pause_sv = bstone::StringView{"pause"};
constexpr auto in_back_quote_sv = bstone::StringView{"back_quote"};
constexpr auto in_semicolon_sv = bstone::StringView{"semicolon"};
constexpr auto in_quote_sv = bstone::StringView{"quote"};
constexpr auto in_backslash_sv = bstone::StringView{"backslash"};
constexpr auto in_comma_sv = bstone::StringView{"comma"};
constexpr auto in_period_sv = bstone::StringView{"period"};

constexpr auto in_1_sv = bstone::StringView{"1"};
constexpr auto in_2_sv = bstone::StringView{"2"};
constexpr auto in_3_sv = bstone::StringView{"3"};
constexpr auto in_4_sv = bstone::StringView{"4"};
constexpr auto in_5_sv = bstone::StringView{"5"};
constexpr auto in_6_sv = bstone::StringView{"6"};
constexpr auto in_7_sv = bstone::StringView{"7"};
constexpr auto in_8_sv = bstone::StringView{"8"};
constexpr auto in_9_sv = bstone::StringView{"9"};
constexpr auto in_0_sv = bstone::StringView{"0"};

constexpr auto in_a_sv = bstone::StringView{"a"};
constexpr auto in_b_sv = bstone::StringView{"b"};
constexpr auto in_c_sv = bstone::StringView{"c"};
constexpr auto in_d_sv = bstone::StringView{"d"};
constexpr auto in_e_sv = bstone::StringView{"e"};
constexpr auto in_f_sv = bstone::StringView{"f"};
constexpr auto in_g_sv = bstone::StringView{"g"};
constexpr auto in_h_sv = bstone::StringView{"h"};
constexpr auto in_i_sv = bstone::StringView{"i"};
constexpr auto in_j_sv = bstone::StringView{"j"};
constexpr auto in_k_sv = bstone::StringView{"k"};
constexpr auto in_l_sv = bstone::StringView{"l"};
constexpr auto in_m_sv = bstone::StringView{"m"};
constexpr auto in_n_sv = bstone::StringView{"n"};
constexpr auto in_o_sv = bstone::StringView{"o"};
constexpr auto in_p_sv = bstone::StringView{"p"};
constexpr auto in_q_sv = bstone::StringView{"q"};
constexpr auto in_r_sv = bstone::StringView{"r"};
constexpr auto in_s_sv = bstone::StringView{"s"};
constexpr auto in_t_sv = bstone::StringView{"t"};
constexpr auto in_u_sv = bstone::StringView{"u"};
constexpr auto in_v_sv = bstone::StringView{"v"};
constexpr auto in_w_sv = bstone::StringView{"w"};
constexpr auto in_x_sv = bstone::StringView{"x"};
constexpr auto in_y_sv = bstone::StringView{"y"};
constexpr auto in_z_sv = bstone::StringView{"z"};

constexpr auto in_kp_minus_sv = bstone::StringView{"kp_minus"};
constexpr auto in_kp_plus_sv = bstone::StringView{"kp_plus"};

constexpr auto in_mouse_left_sv = bstone::StringView{"mouse_left"};
constexpr auto in_mouse_middle_sv = bstone::StringView{"mouse_middle"};
constexpr auto in_mouse_right_sv = bstone::StringView{"mouse_right"};
constexpr auto in_mouse_x1_sv = bstone::StringView{"mouse_x1"};
constexpr auto in_mouse_x2_sv = bstone::StringView{"mouse_x2"};

constexpr auto in_mouse_wheel_down_sv = bstone::StringView{"mouse_wheel_down"};
constexpr auto in_mouse_wheel_up_sv = bstone::StringView{"mouse_wheel_up"};

struct InScanCodeNameToIdMapItem
{
	const bstone::StringView& name_sv;
	ScanCode scan_code;
};

constexpr InScanCodeNameToIdMapItem in_scan_code_name_to_id_map[] =
{
	{in_return_sv, ScanCode::sc_return},
	{in_escape_sv, ScanCode::sc_escape},
	{in_space_sv, ScanCode::sc_space},
	{in_minus_sv, ScanCode::sc_minus},
	{in_equals_sv, ScanCode::sc_equals},
	{in_backspace_sv, ScanCode::sc_backspace},
	{in_tab_sv, ScanCode::sc_tab},
	{in_alt_sv, ScanCode::sc_alt},
	{in_left_bracket_sv, ScanCode::sc_left_bracket},
	{in_right_bracket_sv, ScanCode::sc_right_bracket},
	{in_control_sv, ScanCode::sc_control},
	{in_caps_lock_sv, ScanCode::sc_caps_lock},
	{in_num_lock_sv, ScanCode::sc_num_lock},
	{in_scroll_lock_sv, ScanCode::sc_scroll_lock},
	{in_left_shift_sv, ScanCode::sc_left_shift},
	{in_right_shift_sv, ScanCode::sc_right_shift},
	{in_up_arrow_sv, ScanCode::sc_up_arrow},
	{in_down_arrow_sv, ScanCode::sc_down_arrow},
	{in_left_arrow_sv, ScanCode::sc_left_arrow},
	{in_right_arrow_sv, ScanCode::sc_right_arrow},
	{in_insert_sv, ScanCode::sc_insert},
	{in_delete_sv, ScanCode::sc_delete},
	{in_home_sv, ScanCode::sc_home},
	{in_end_sv, ScanCode::sc_end},
	{in_page_up_sv, ScanCode::sc_page_up},
	{in_page_down_sv, ScanCode::sc_page_down},
	{in_slash_sv, ScanCode::sc_slash},
	{in_f1_sv, ScanCode::sc_f1},
	{in_f2_sv, ScanCode::sc_f2},
	{in_f3_sv, ScanCode::sc_f3},
	{in_f4_sv, ScanCode::sc_f4},
	{in_f5_sv, ScanCode::sc_f5},
	{in_f6_sv, ScanCode::sc_f6},
	{in_f7_sv, ScanCode::sc_f7},
	{in_f8_sv, ScanCode::sc_f8},
	{in_f9_sv, ScanCode::sc_f9},
	{in_f10_sv, ScanCode::sc_f10},
	{in_f11_sv, ScanCode::sc_f11},
	{in_f12_sv, ScanCode::sc_f12},
	{in_print_screen_sv, ScanCode::sc_print_screen},
	{in_pause_sv, ScanCode::sc_pause},
	{in_back_quote_sv, ScanCode::sc_back_quote},
	{in_semicolon_sv, ScanCode::sc_semicolon},
	{in_quote_sv, ScanCode::sc_quote},
	{in_backslash_sv, ScanCode::sc_backslash},
	{in_comma_sv, ScanCode::sc_comma},
	{in_period_sv, ScanCode::sc_period},

	{in_1_sv, ScanCode::sc_1},
	{in_2_sv, ScanCode::sc_2},
	{in_3_sv, ScanCode::sc_3},
	{in_4_sv, ScanCode::sc_4},
	{in_5_sv, ScanCode::sc_5},
	{in_6_sv, ScanCode::sc_6},
	{in_7_sv, ScanCode::sc_7},
	{in_8_sv, ScanCode::sc_8},
	{in_9_sv, ScanCode::sc_9},
	{in_0_sv, ScanCode::sc_0},

	{in_a_sv, ScanCode::sc_a},
	{in_b_sv, ScanCode::sc_b},
	{in_c_sv, ScanCode::sc_c},
	{in_d_sv, ScanCode::sc_d},
	{in_e_sv, ScanCode::sc_e},
	{in_f_sv, ScanCode::sc_f},
	{in_g_sv, ScanCode::sc_g},
	{in_h_sv, ScanCode::sc_h},
	{in_i_sv, ScanCode::sc_i},
	{in_j_sv, ScanCode::sc_j},
	{in_k_sv, ScanCode::sc_k},
	{in_l_sv, ScanCode::sc_l},
	{in_m_sv, ScanCode::sc_m},
	{in_n_sv, ScanCode::sc_n},
	{in_o_sv, ScanCode::sc_o},
	{in_p_sv, ScanCode::sc_p},
	{in_q_sv, ScanCode::sc_q},
	{in_r_sv, ScanCode::sc_r},
	{in_s_sv, ScanCode::sc_s},
	{in_t_sv, ScanCode::sc_t},
	{in_u_sv, ScanCode::sc_u},
	{in_v_sv, ScanCode::sc_v},
	{in_w_sv, ScanCode::sc_w},
	{in_x_sv, ScanCode::sc_x},
	{in_y_sv, ScanCode::sc_y},
	{in_z_sv, ScanCode::sc_z},

	{in_kp_minus_sv, ScanCode::sc_kp_minus},
	{in_kp_plus_sv, ScanCode::sc_kp_plus},

	{in_mouse_left_sv, ScanCode::sc_mouse_left},
	{in_mouse_middle_sv, ScanCode::sc_mouse_middle},
	{in_mouse_right_sv, ScanCode::sc_mouse_right},
	{in_mouse_x1_sv, ScanCode::sc_mouse_x1},
	{in_mouse_x2_sv, ScanCode::sc_mouse_x2},

	{in_mouse_wheel_down_sv, ScanCode::sc_mouse_wheel_down},
	{in_mouse_wheel_up_sv, ScanCode::sc_mouse_wheel_up},
};

ScanCode in_scan_code_name_to_id(bstone::StringView name_sv)
try
{
	for (const auto& map_item : in_scan_code_name_to_id_map)
	{
		if (map_item.name_sv == name_sv)
		{
			return map_item.scan_code;
		}
	}

	auto message = std::string{};
	message += "Unknown scan code name \"";
	message.append(name_sv.get_data(), static_cast<std::size_t>(name_sv.get_size()));
	message += "\".";
	throw InBindingException{message.c_str()};
}
catch (...)
{
	std::throw_with_nested(InBindingException{__func__});
}

bstone::StringView in_scan_code_id_to_name(ScanCode scan_code)
try
{
	for (const auto& map_item : in_scan_code_name_to_id_map)
	{
		if (map_item.scan_code == scan_code)
		{
			return map_item.name_sv;
		}
	}

	throw InBindingException{"Unknown scan code name."};
}
catch (...)
{
	std::throw_with_nested(InBindingException{__func__});
}

class InBindException : public bstone::Exception
{
public:
	explicit InBindException(const char* message) noexcept
		:
		Exception{"BSTONE_IN_BIND", message}
	{}
};

class InBindCCmdAction final : public bstone::CCmdAction
{
public:
	InBindCCmdAction() noexcept = default;
	~InBindCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try
	{
		if (args.get_size() != 3)
		{
			fail("Invalid argument count.");
		}

		const auto scan_code = in_scan_code_name_to_id(args[0]);
		const auto binding_id = in_binding_name_to_id(args[1]);
		const auto slot_index = in_parse_binding_slot_index(args[2]);

		// Unbind existing bindings.
		//
		for (auto& binding : in_bindings)
		{
			for (auto& bindings_slot : binding)
			{
				if (bindings_slot == scan_code)
				{
					bindings_slot = ScanCode::sc_none;
				}
			}
		}

		// Bind it.
		//
		in_bindings[binding_id][slot_index] = scan_code;
	}
	catch (...)
	{
		fail_nested(__func__);
	}

private:
	[[noreturn]] static void fail(const char* message)
	{
		throw InBindException{message};
	}

	[[noreturn]] static void fail_nested(const char* message)
	{
		std::throw_with_nested(InBindException{message});
	}
};

constexpr auto in_bind_sv = bstone::StringView{"in_bind"};
auto in_bind_ccmd_action = InBindCCmdAction{};
auto in_bind_ccmd = bstone::CCmd{in_bind_sv, in_bind_ccmd_action};

bool in_has_any_binding()
{
	for (const auto& binding_slots : in_bindings)
	{
		for (const auto& binding_slot : binding_slots)
		{
			if (binding_slot != ScanCode::sc_none)
			{
				return true;
			}
		}
	}

	return false;
}

} // namespace

void in_initialize_ccmds(bstone::CCmdMgr& ccmd_mgr)
{
	ccmd_mgr.add(in_clear_bindings_ccmd);
	ccmd_mgr.add(in_clear_binding_ccmd);
	ccmd_mgr.add(in_bind_ccmd);
}

void in_serialize_bindings(bstone::TextWriter& text_writer)
{
	if (!in_has_any_binding())
	{
		in_set_default_bindings();
	}

	auto text_buffer = std::string{};
	text_buffer.reserve(1024);

	text_buffer = '\n';
	text_writer.write(text_buffer);

	// Clear bindings.
	{
		text_buffer.assign(
			in_clear_bindings_sv.get_data(),
			static_cast<std::size_t>(in_clear_bindings_sv.get_size()));

		text_buffer += '\n';
		text_writer.write(text_buffer);
	}

	// Write out bindings.
	{
		constexpr auto slot_index_max_chars = 11;
		char slot_index_chars[slot_index_max_chars];
		const auto slot_index_chars_span = bstone::make_span(slot_index_chars);

		auto raw_binding_id = static_cast<bstone::Int>(BindingId{});

		for (const auto& binding_slots : in_bindings)
		{
			const auto binding_id = static_cast<BindingId>(raw_binding_id);
			const auto binding_name_sv = in_binding_id_to_name(binding_id);
			auto slot_index = 0;

			for (const auto& binding_slot : binding_slots)
			{
				if (binding_slot != ScanCode::sc_none)
				{
					const auto scan_code_sv = in_scan_code_id_to_name(binding_slot);

					const auto slot_index_char_count = bstone::char_conv::to_chars(
						slot_index,
						slot_index_chars_span,
						10);

					text_buffer.clear();

					text_buffer.append(
						in_bind_sv.get_data(),
						static_cast<std::size_t>(in_bind_sv.get_size()));

					text_buffer += " \"";

					text_buffer.append(
						scan_code_sv.get_data(),
						static_cast<std::size_t>(scan_code_sv.get_size()));

					text_buffer += "\" \"";

					text_buffer.append(
						binding_name_sv.get_data(),
						static_cast<std::size_t>(binding_name_sv.get_size()));

					text_buffer += "\" \"";

					text_buffer.append(
						slot_index_chars,
						static_cast<std::size_t>(slot_index_char_count));

					text_buffer += "\"\n";
					text_writer.write(text_buffer);
				}

				slot_index += 1;
			}

			raw_binding_id += 1;
		}
	}
}

namespace {

constexpr auto in_gc_unknown_string = "<UNKNOWN>";
constexpr auto in_gc_log_prefix = "[INGC]";
using InGcMap = std::unordered_map<SDL_JoystickID, SDL_GameController*>;

auto in_gc_is_subsystem_started = false;
auto in_gc_map = InGcMap{};
char in_gc_number_chars[11];

void in_gc_log_append_number(int number, std::string& string)
{
	const auto char_count = bstone::char_conv::to_chars(
		number, bstone::make_span(in_gc_number_chars), 10);

	string.append(in_gc_number_chars, static_cast<std::size_t>(char_count));
}

void in_gc_log(bstone::LoggerMessageKind message_type, const std::string& message)
{
	auto log_message = std::string{};
	log_message += in_gc_log_prefix;
	log_message += ' ';
	log_message += message;
	bstone::logger_->write(message_type, log_message);
}

void in_gc_log_info(const std::string& message)
{
	in_gc_log(bstone::LoggerMessageKind::information, message);
}

void in_gc_log_warning(const std::string& message)
{
	in_gc_log(bstone::LoggerMessageKind::warning, message);
}

void in_gc_log_error(const std::string& message)
{
	in_gc_log(bstone::LoggerMessageKind::error, message);
}

void in_gc_log_sdl_error()
{
	const auto sdl_error_message = SDL_GetError();
	auto message = std::string{};
	message += in_gc_log_prefix;
	message += "[SDL] ";
	message += (sdl_error_message != nullptr ? sdl_error_message : "Generic failure.");
	bstone::logger_->write_error(message);
}

void in_gc_log_instance_id(SDL_JoystickID instance_id, std::string& log_buffer)
{
	log_buffer.clear();
	log_buffer += "  Instance ID: ";
	in_gc_log_append_number(instance_id, log_buffer);
	in_gc_log_info(log_buffer);
}

void in_gc_log_name(SDL_GameController& game_controller, std::string& log_buffer)
{
	const auto name = SDL_GameControllerName(&game_controller);
	log_buffer.clear();
	log_buffer += "  Name: ";
	log_buffer += (name != nullptr ? name : in_gc_unknown_string);
	in_gc_log_info(log_buffer);
}

void in_gc_log_path(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 24)
	const auto path = SDL_GameControllerPath(&game_controller);
	log_buffer.clear();
	log_buffer += "  Path: ";
	log_buffer += (path != nullptr ? path : in_gc_unknown_string);
	in_gc_log_info(log_buffer);
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 12)
const char* in_gc_get_type_string(SDL_GameControllerType type)
{
	switch (type)
	{
		case SDL_CONTROLLER_TYPE_XBOX360: return "XBox 360";
		case SDL_CONTROLLER_TYPE_XBOXONE: return "XBox One";
		case SDL_CONTROLLER_TYPE_PS3: return "PS3";
		case SDL_CONTROLLER_TYPE_PS4: return "PS4";
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO: return "Nintendo Switch Pro";
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case SDL_CONTROLLER_TYPE_VIRTUAL: return "Virtual";
		case SDL_CONTROLLER_TYPE_PS5: return "PS5";
#endif
#if SDL_VERSION_ATLEAST(2, 0, 16)
		case SDL_CONTROLLER_TYPE_AMAZON_LUNA: return "Amazon Luna";
		case SDL_CONTROLLER_TYPE_GOOGLE_STADIA: return "Google Stadia";
#endif
#if SDL_VERSION_ATLEAST(2, 24, 0)
		case SDL_CONTROLLER_TYPE_NVIDIA_SHIELD: return "Nvidia Shield";

		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
			return "Nintendo Switch Joy-Con";
#endif

		default: return in_gc_unknown_string;
	}
}
#endif

void in_gc_log_type(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 12)
	const auto type = SDL_GameControllerGetType(&game_controller);
	const auto type_string = in_gc_get_type_string(type);
	log_buffer.clear();
	log_buffer += "  Type: ";
	log_buffer += type_string;
	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_player_index(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	const auto player_index = SDL_GameControllerGetPlayerIndex(&game_controller);
	log_buffer.clear();
	log_buffer += "  Player index: ";

	if (player_index >= 0)
	{
		in_gc_log_append_number(player_index, log_buffer);
	}
	else
	{
		log_buffer += in_gc_unknown_string;
	}

	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_vendor(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 6)
	const auto vendor = SDL_GameControllerGetVendor(&game_controller);
	log_buffer.clear();
	log_buffer += "  USB vendor ID: ";

	if (vendor != 0)
	{
		in_gc_log_append_number(vendor, log_buffer);
	}
	else
	{
		log_buffer += in_gc_unknown_string;
	}

	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_product(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 6)
	const auto product = SDL_GameControllerGetProduct(&game_controller);
	log_buffer.clear();
	log_buffer += "  USB product ID: ";

	if (product != 0)
	{
		in_gc_log_append_number(product, log_buffer);
	}
	else
	{
		log_buffer += in_gc_unknown_string;
	}

	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_product_version(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 6)
	const auto product_version = SDL_GameControllerGetProductVersion(&game_controller);
	log_buffer.clear();
	log_buffer += "  Product version: ";

	if (product_version != 0)
	{
		in_gc_log_append_number(product_version, log_buffer);
	}
	else
	{
		log_buffer += in_gc_unknown_string;
	}

	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_firmware_version(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 24, 0)
	const auto firmware_version = SDL_GameControllerGetFirmwareVersion(&game_controller);
	log_buffer.clear();
	log_buffer += "  Firmware version: ";

	if (firmware_version != 0)
	{
		in_gc_log_append_number(firmware_version, log_buffer);
	}
	else
	{
		log_buffer += in_gc_unknown_string;
	}

	in_gc_log_info(log_buffer);
#endif
}

void in_gc_log_serial(SDL_GameController& game_controller, std::string& log_buffer)
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
	const auto serial = SDL_GameControllerGetSerial(&game_controller);
	log_buffer.clear();
	log_buffer += "  Serial: ";
	log_buffer += (serial != nullptr ? serial : in_gc_unknown_string);
	in_gc_log_info(log_buffer);
#endif
}

void in_gc_add(int joystick_index)
{
	auto log_buffer = std::string{};
	log_buffer.reserve(256);

	log_buffer.clear();
	log_buffer += "Adding controller with joystick index ";
	in_gc_log_append_number(joystick_index, log_buffer);
	log_buffer += '.';
	in_gc_log_info(log_buffer);

	const auto map_iter = in_gc_map.find(joystick_index);

	if (map_iter != in_gc_map.cend())
	{
		in_gc_log_warning("Already added.");
		return;
	}

	const auto game_controller = SDL_GameControllerOpen(joystick_index);

	if (game_controller == nullptr)
	{
		in_gc_log_error("Failed to open a controller.");
		return;
	}

	const auto joystick = SDL_GameControllerGetJoystick(game_controller);

	if (joystick == nullptr)
	{
		SDL_GameControllerClose(game_controller);
		in_gc_log_error("Failed to get a joystick object.");
		return;
	}

	const auto joystick_id = SDL_JoystickInstanceID(joystick);

	if (joystick_id < 0)
	{
		SDL_GameControllerClose(game_controller);
		in_gc_log_error("Failed to get a joystick ID.");
		return;
	}

	in_gc_map.emplace(joystick_id, game_controller);

	in_gc_log_info("Added controller:");
	log_buffer.clear();
	in_gc_log_instance_id(joystick_id, log_buffer);
	in_gc_log_name(*game_controller, log_buffer);
	in_gc_log_path(*game_controller, log_buffer);
	in_gc_log_type(*game_controller, log_buffer);
	in_gc_log_player_index(*game_controller, log_buffer);
	in_gc_log_vendor(*game_controller, log_buffer);
	in_gc_log_product(*game_controller, log_buffer);
	in_gc_log_product_version(*game_controller, log_buffer);
	in_gc_log_firmware_version(*game_controller, log_buffer);
	in_gc_log_serial(*game_controller, log_buffer);
}

void in_gc_remove(SDL_JoystickID joystick_id)
{
	auto log_buffer = std::string{};
	log_buffer.reserve(256);
	log_buffer += "Removing controller with instance ID ";
	in_gc_log_append_number(joystick_id, log_buffer);
	log_buffer += '.';
	in_gc_log_info(log_buffer);

	const auto map_iter = in_gc_map.find(joystick_id);

	if (map_iter == in_gc_map.cend())
	{
		in_gc_log_error("Not internally registered.");
		return;
	}

	SDL_GameControllerClose(map_iter->second);
	in_gc_map.erase(map_iter);
	in_gc_log_info("Removed.");
}

void in_gc_remap(SDL_JoystickID joystick_id)
{
	auto log_buffer = std::string{};
	log_buffer.reserve(256);
	log_buffer += "Remapped controller with instance ID ";
	in_gc_log_append_number(joystick_id, log_buffer);
	log_buffer += '.';
	in_gc_log_info(log_buffer);
}

void in_gc_handle_axis(const SDL_ControllerAxisEvent& e)
{
	assert(e.type == SDL_CONTROLLERAXISMOTION);
	// TODO
}

void in_gc_handle_button(const SDL_ControllerButtonEvent& e)
{
	assert(e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP);
	// TODO
}

void in_gc_handle_device_added(const SDL_ControllerDeviceEvent& e)
{
	assert(e.type == SDL_CONTROLLERDEVICEADDED);
	in_gc_add(e.which);
}

void in_gc_handle_device_removed(const SDL_ControllerDeviceEvent& e)
{
	assert(e.type == SDL_CONTROLLERDEVICEREMOVED);
	in_gc_remove(e.which);
}

void in_gc_handle_device_remapped(const SDL_ControllerDeviceEvent& e)
{
	assert(e.type == SDL_CONTROLLERDEVICEREMAPPED);
	in_gc_remap(e.which);
}

void in_gc_handle_device(const SDL_ControllerDeviceEvent& e)
{
	switch (e.type)
	{
		case SDL_CONTROLLERDEVICEADDED: in_gc_handle_device_added(e); break;
		case SDL_CONTROLLERDEVICEREMOVED: in_gc_handle_device_removed(e); break;
		case SDL_CONTROLLERDEVICEREMAPPED: in_gc_handle_device_remapped(e); break;
		default: break;
	}
}

void in_gc_handle_touchpad_down(const SDL_ControllerTouchpadEvent& e)
{
	assert(e.type == SDL_CONTROLLERTOUCHPADDOWN);
	// TODO
}

void in_gc_handle_touchpad_motion(const SDL_ControllerTouchpadEvent& e)
{
	assert(e.type == SDL_CONTROLLERTOUCHPADMOTION);
	// TODO
}

void in_gc_handle_touchpad_up(const SDL_ControllerTouchpadEvent& e)
{
	assert(e.type == SDL_CONTROLLERTOUCHPADUP);
	// TODO
}

void in_gc_handle_touchpad(const SDL_ControllerTouchpadEvent& e)
{
	switch (e.type)
	{
		case SDL_CONTROLLERTOUCHPADDOWN: in_gc_handle_touchpad_down(e); break;
		case SDL_CONTROLLERTOUCHPADMOTION: in_gc_handle_touchpad_motion(e); break;
		case SDL_CONTROLLERTOUCHPADUP: in_gc_handle_touchpad_up(e); break;
		default: break;
	}
}

void in_gc_handle_sensor(const SDL_ControllerSensorEvent& e)
{
	assert(e.type == SDL_CONTROLLERSENSORUPDATE);
	// TODO
}

} // namespace

void in_gc_startup()
{
	in_gc_log_info("Starting up.");

	if (in_gc_is_subsystem_started)
	{
		bstone::logger_->write_warning("Already started.");
		return;
	}

	const auto sdl_result = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

	if (sdl_result != 0)
	{
		in_gc_log_sdl_error();
		return;
	}

	in_gc_is_subsystem_started = true;
	in_gc_log_info("Started up.");
}

void in_gc_shutdown()
{
	in_gc_log_info("Shutting down.");
	in_gc_is_subsystem_started = true;
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
	in_gc_map.clear();
	in_gc_log_info("Shutted down.");
}
