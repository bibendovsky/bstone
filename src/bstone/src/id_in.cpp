/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
#include <iterator>
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vl.h"
#include "bstone_ascii.h"
#include "bstone_char_conv.h"
#include "bstone_globals.h"
#include "bstone_sys_keyboard_key.h"

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

auto in_is_lalt_pressed = false;
auto in_is_ralt_pressed = false;
auto in_is_lctrl_pressed = false;
auto in_is_rctrl_pressed = false;
auto in_is_lshift_pressed = false;
auto in_is_rshift_pressed = false;
auto in_is_lgui_pressed = false;
auto in_is_rgui_pressed = false;

auto in_is_caps_lock_pressed = false;
auto in_is_scroll_lock_pressed = false;
auto in_is_num_lock_pressed = false;

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

	bstone::globals::sys_mouse_mgr->set_relative_mode(grab);
	in_is_mouse_grabbed = grab;
	return in_is_mouse_grabbed;
}

// Internal routines

// TODO
#if 0
#ifdef __vita__
auto in_mouse_dx = 0;
auto in_mouse_dy = 0;
#endif
#else
namespace {
auto in_mouse_dx = 0;
auto in_mouse_dy = 0;
} // namespace
#endif

namespace {

ScanCode in_keyboard_map_to_bstone(bstone::sys::KeyboardKey key_code)
{
	switch (key_code)
	{
		case bstone::sys::KeyboardKey::enter:
		case bstone::sys::KeyboardKey::kp_enter:
			return ScanCode::sc_return;

		case bstone::sys::KeyboardKey::esc: return ScanCode::sc_escape;

		case bstone::sys::KeyboardKey::space:
		case bstone::sys::KeyboardKey::kp_space:
			return ScanCode::sc_space;

		case bstone::sys::KeyboardKey::minus: return ScanCode::sc_minus;
		case bstone::sys::KeyboardKey::equals: return ScanCode::sc_equals;

		case bstone::sys::KeyboardKey::backspace:
		case bstone::sys::KeyboardKey::kp_backspace:
			return ScanCode::sc_backspace;

		case bstone::sys::KeyboardKey::tab:
		case bstone::sys::KeyboardKey::kp_tab:
			return ScanCode::sc_tab;

		case bstone::sys::KeyboardKey::left_alt:
		case bstone::sys::KeyboardKey::right_alt:
			return ScanCode::sc_alt;

		case bstone::sys::KeyboardKey::left_bracket:
		case bstone::sys::KeyboardKey::kp_left_brace:
			return ScanCode::sc_left_bracket;

		case bstone::sys::KeyboardKey::right_bracket:
		case bstone::sys::KeyboardKey::kp_right_brace:
			return ScanCode::sc_right_bracket;

		case bstone::sys::KeyboardKey::left_ctrl:
		case bstone::sys::KeyboardKey::right_ctrl:
			return ScanCode::sc_control;

		case bstone::sys::KeyboardKey::caps_lock: return ScanCode::sc_caps_lock;
		case bstone::sys::KeyboardKey::num_lock: return ScanCode::sc_num_lock;
		case bstone::sys::KeyboardKey::scroll_lock: return ScanCode::sc_scroll_lock;
		case bstone::sys::KeyboardKey::left_shift: return ScanCode::sc_left_shift;
		case bstone::sys::KeyboardKey::right_shift: return ScanCode::sc_right_shift;
		case bstone::sys::KeyboardKey::up: return ScanCode::sc_up_arrow;
		case bstone::sys::KeyboardKey::kp_8: return in_is_num_lock_pressed ? ScanCode::sc_up_arrow : ScanCode::sc_8;
		case bstone::sys::KeyboardKey::down: return ScanCode::sc_down_arrow;
		case bstone::sys::KeyboardKey::kp_2: return in_is_num_lock_pressed ? ScanCode::sc_down_arrow : ScanCode::sc_2;
		case bstone::sys::KeyboardKey::left: return ScanCode::sc_left_arrow;
		case bstone::sys::KeyboardKey::kp_4: return in_is_num_lock_pressed ? ScanCode::sc_left_arrow : ScanCode::sc_4;
		case bstone::sys::KeyboardKey::right: return ScanCode::sc_right_arrow;
		case bstone::sys::KeyboardKey::kp_6: return in_is_num_lock_pressed ? ScanCode::sc_right_arrow : ScanCode::sc_6;
		case bstone::sys::KeyboardKey::insert: return ScanCode::sc_insert;
		case bstone::sys::KeyboardKey::kp_0: return in_is_num_lock_pressed ? ScanCode::sc_insert : ScanCode::sc_0;
		case bstone::sys::KeyboardKey::del: return ScanCode::sc_delete;
		case bstone::sys::KeyboardKey::kp_comma: return in_is_num_lock_pressed ? ScanCode::sc_delete : ScanCode::sc_comma;
		case bstone::sys::KeyboardKey::home: return ScanCode::sc_home;
		case bstone::sys::KeyboardKey::kp_7: return in_is_num_lock_pressed ? ScanCode::sc_home : ScanCode::sc_7;
		case bstone::sys::KeyboardKey::end: return ScanCode::sc_end;
		case bstone::sys::KeyboardKey::kp_1: return in_is_num_lock_pressed ? ScanCode::sc_end : ScanCode::sc_1;
		case bstone::sys::KeyboardKey::page_up: return ScanCode::sc_page_up;
		case bstone::sys::KeyboardKey::kp_9: return in_is_num_lock_pressed ? ScanCode::sc_page_up : ScanCode::sc_9;
		case bstone::sys::KeyboardKey::page_down: return ScanCode::sc_page_down;
		case bstone::sys::KeyboardKey::kp_3: return in_is_num_lock_pressed ? ScanCode::sc_page_down : ScanCode::sc_3;

		case bstone::sys::KeyboardKey::slash:
		case bstone::sys::KeyboardKey::kp_divide:
			return ScanCode::sc_slash;

		case bstone::sys::KeyboardKey::backslash: return ScanCode::sc_backslash;
		case bstone::sys::KeyboardKey::semicolon: return ScanCode::sc_semicolon;
		case bstone::sys::KeyboardKey::quote: return ScanCode::sc_quote;
		case bstone::sys::KeyboardKey::period: return ScanCode::sc_period;
		case bstone::sys::KeyboardKey::f1: return ScanCode::sc_f1;
		case bstone::sys::KeyboardKey::f2: return ScanCode::sc_f2;
		case bstone::sys::KeyboardKey::f3: return ScanCode::sc_f3;
		case bstone::sys::KeyboardKey::f4: return ScanCode::sc_f4;
		case bstone::sys::KeyboardKey::f5: return ScanCode::sc_f5;
		case bstone::sys::KeyboardKey::f6: return ScanCode::sc_f6;
		case bstone::sys::KeyboardKey::f7: return ScanCode::sc_f7;
		case bstone::sys::KeyboardKey::f8: return ScanCode::sc_f8;
		case bstone::sys::KeyboardKey::f9: return ScanCode::sc_f9;
		case bstone::sys::KeyboardKey::f10: return ScanCode::sc_f10;
		case bstone::sys::KeyboardKey::f11: return ScanCode::sc_f11;
		case bstone::sys::KeyboardKey::f12: return ScanCode::sc_f12;
		case bstone::sys::KeyboardKey::prt_scr: return ScanCode::sc_print_screen;
		case bstone::sys::KeyboardKey::pause: return ScanCode::sc_pause;
		case bstone::sys::KeyboardKey::backtick: return ScanCode::sc_back_quote;
		case bstone::sys::KeyboardKey::n1: return ScanCode::sc_1;
		case bstone::sys::KeyboardKey::n2: return ScanCode::sc_2;
		case bstone::sys::KeyboardKey::n3: return ScanCode::sc_3;
		case bstone::sys::KeyboardKey::n4: return ScanCode::sc_4;
		case bstone::sys::KeyboardKey::n5: return ScanCode::sc_5;
		case bstone::sys::KeyboardKey::n6: return ScanCode::sc_6;
		case bstone::sys::KeyboardKey::n7: return ScanCode::sc_7;
		case bstone::sys::KeyboardKey::n8: return ScanCode::sc_8;
		case bstone::sys::KeyboardKey::n9: return ScanCode::sc_9;
		case bstone::sys::KeyboardKey::n0: return ScanCode::sc_0;

		case bstone::sys::KeyboardKey::a:
		case bstone::sys::KeyboardKey::kp_a:
			return ScanCode::sc_a;

		case bstone::sys::KeyboardKey::b:
		case bstone::sys::KeyboardKey::kp_b:
			return ScanCode::sc_b;

		case bstone::sys::KeyboardKey::c:
		case bstone::sys::KeyboardKey::kp_c:
			return ScanCode::sc_c;

		case bstone::sys::KeyboardKey::d:
		case bstone::sys::KeyboardKey::kp_d:
			return ScanCode::sc_d;

		case bstone::sys::KeyboardKey::e:
		case bstone::sys::KeyboardKey::kp_e:
			return ScanCode::sc_e;

		case bstone::sys::KeyboardKey::f:
		case bstone::sys::KeyboardKey::kp_f:
			return ScanCode::sc_f;

		case bstone::sys::KeyboardKey::g: return ScanCode::sc_g;
		case bstone::sys::KeyboardKey::h: return ScanCode::sc_h;
		case bstone::sys::KeyboardKey::i: return ScanCode::sc_i;
		case bstone::sys::KeyboardKey::j: return ScanCode::sc_j;
		case bstone::sys::KeyboardKey::k: return ScanCode::sc_k;
		case bstone::sys::KeyboardKey::l: return ScanCode::sc_l;
		case bstone::sys::KeyboardKey::m: return ScanCode::sc_m;
		case bstone::sys::KeyboardKey::n: return ScanCode::sc_n;
		case bstone::sys::KeyboardKey::o: return ScanCode::sc_o;
		case bstone::sys::KeyboardKey::p: return ScanCode::sc_p;
		case bstone::sys::KeyboardKey::q: return ScanCode::sc_q;
		case bstone::sys::KeyboardKey::r: return ScanCode::sc_r;
		case bstone::sys::KeyboardKey::s: return ScanCode::sc_s;
		case bstone::sys::KeyboardKey::t: return ScanCode::sc_t;
		case bstone::sys::KeyboardKey::u: return ScanCode::sc_u;
		case bstone::sys::KeyboardKey::v: return ScanCode::sc_v;
		case bstone::sys::KeyboardKey::w: return ScanCode::sc_w;
		case bstone::sys::KeyboardKey::x: return ScanCode::sc_x;
		case bstone::sys::KeyboardKey::y: return ScanCode::sc_y;
		case bstone::sys::KeyboardKey::z: return ScanCode::sc_z;
		case bstone::sys::KeyboardKey::kp_minus: return ScanCode::sc_kp_minus;
		case bstone::sys::KeyboardKey::kp_plus: return ScanCode::sc_kp_plus;
		default: return ScanCode::sc_none;
	}
}

char in_keyboard_map_to_char(const bstone::sys::KeyboardEvent& e)
{
	auto is_caps = false;
	auto key_code = e.key;

	if (in_is_lalt_pressed ||
		in_is_ralt_pressed ||
		in_is_lctrl_pressed ||
		in_is_rctrl_pressed ||
		in_is_lgui_pressed ||
		in_is_rgui_pressed)
	{
		return '\0';
	}

	switch (key_code)
	{
		case bstone::sys::KeyboardKey::esc: return '\x1B';
		case bstone::sys::KeyboardKey::backspace: return '\b';
		case bstone::sys::KeyboardKey::tab: return '\t';
		case bstone::sys::KeyboardKey::enter: return '\n';
		case bstone::sys::KeyboardKey::space: return ' ';
		case bstone::sys::KeyboardKey::del: return '\x7F';
		default: break;
	}

	const auto is_shift = (in_is_lshift_pressed || in_is_rshift_pressed);

	if (allcaps)
	{
		is_caps = true;
	}
	else
	{
		if (in_is_caps_lock_pressed)
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
			case bstone::sys::KeyboardKey::n1: return '!';
			case bstone::sys::KeyboardKey::n2: return '@';
			case bstone::sys::KeyboardKey::n3: return '#';
			case bstone::sys::KeyboardKey::n4: return '$';
			case bstone::sys::KeyboardKey::n5: return '%';
			case bstone::sys::KeyboardKey::n6: return '^';
			case bstone::sys::KeyboardKey::n7: return '&';
			case bstone::sys::KeyboardKey::n8: return '*';
			case bstone::sys::KeyboardKey::n9: return '(';
			case bstone::sys::KeyboardKey::n0: return ')';
			case bstone::sys::KeyboardKey::minus: return '_';
			case bstone::sys::KeyboardKey::equals: return '+';
			case bstone::sys::KeyboardKey::left_bracket: return '{';
			case bstone::sys::KeyboardKey::right_bracket: return '}';
			case bstone::sys::KeyboardKey::semicolon: return ':';
			case bstone::sys::KeyboardKey::quote: return '"';
			case bstone::sys::KeyboardKey::backtick: return '~';
			case bstone::sys::KeyboardKey::backslash: return '|';
			case bstone::sys::KeyboardKey::comma: return '<';
			case bstone::sys::KeyboardKey::period: return '>';
			case bstone::sys::KeyboardKey::slash: return '?';
			default: break;
		}
	}
	else
	{
		switch (key_code)
		{
			case bstone::sys::KeyboardKey::n1: return '1';
			case bstone::sys::KeyboardKey::n2: return '2';
			case bstone::sys::KeyboardKey::n3: return '3';
			case bstone::sys::KeyboardKey::n4: return '4';
			case bstone::sys::KeyboardKey::n5: return '5';
			case bstone::sys::KeyboardKey::n6: return '6';
			case bstone::sys::KeyboardKey::n7: return '7';
			case bstone::sys::KeyboardKey::n8: return '8';
			case bstone::sys::KeyboardKey::n9: return '9';
			case bstone::sys::KeyboardKey::n0: return '0';
			case bstone::sys::KeyboardKey::minus: return '-';
			case bstone::sys::KeyboardKey::equals: return '=';
			case bstone::sys::KeyboardKey::left_bracket: return '[';
			case bstone::sys::KeyboardKey::right_bracket: return ']';
			case bstone::sys::KeyboardKey::semicolon: return ';';
			case bstone::sys::KeyboardKey::quote: return '\'';
			case bstone::sys::KeyboardKey::backtick: return '`';
			case bstone::sys::KeyboardKey::backslash: return '\\';
			case bstone::sys::KeyboardKey::comma: return ',';
			case bstone::sys::KeyboardKey::period: return '.';
			case bstone::sys::KeyboardKey::slash: return '/';
			default: break;
		}
	}


	//
	// Keys which depends on Caps Lock and Left or Right Shift.
	//

	switch (key_code)
	{
		case bstone::sys::KeyboardKey::a: return is_caps ? 'A' : 'a';
		case bstone::sys::KeyboardKey::b: return is_caps ? 'B' : 'b';
		case bstone::sys::KeyboardKey::c: return is_caps ? 'C' : 'c';
		case bstone::sys::KeyboardKey::d: return is_caps ? 'D' : 'd';
		case bstone::sys::KeyboardKey::e: return is_caps ? 'E' : 'e';
		case bstone::sys::KeyboardKey::f: return is_caps ? 'F' : 'f';
		case bstone::sys::KeyboardKey::g: return is_caps ? 'G' : 'g';
		case bstone::sys::KeyboardKey::h: return is_caps ? 'H' : 'h';
		case bstone::sys::KeyboardKey::i: return is_caps ? 'I' : 'i';
		case bstone::sys::KeyboardKey::j: return is_caps ? 'J' : 'j';
		case bstone::sys::KeyboardKey::k: return is_caps ? 'K' : 'k';
		case bstone::sys::KeyboardKey::l: return is_caps ? 'L' : 'l';
		case bstone::sys::KeyboardKey::m: return is_caps ? 'M' : 'm';
		case bstone::sys::KeyboardKey::n: return is_caps ? 'N' : 'n';
		case bstone::sys::KeyboardKey::o: return is_caps ? 'O' : 'o';
		case bstone::sys::KeyboardKey::p: return is_caps ? 'P' : 'p';
		case bstone::sys::KeyboardKey::q: return is_caps ? 'Q' : 'q';
		case bstone::sys::KeyboardKey::r: return is_caps ? 'R' : 'r';
		case bstone::sys::KeyboardKey::s: return is_caps ? 'S' : 's';
		case bstone::sys::KeyboardKey::t: return is_caps ? 'T' : 't';
		case bstone::sys::KeyboardKey::u: return is_caps ? 'U' : 'u';
		case bstone::sys::KeyboardKey::v: return is_caps ? 'V' : 'v';
		case bstone::sys::KeyboardKey::w: return is_caps ? 'W' : 'w';
		case bstone::sys::KeyboardKey::x: return is_caps ? 'X' : 'x';
		case bstone::sys::KeyboardKey::y: return is_caps ? 'Y' : 'y';
		case bstone::sys::KeyboardKey::z: return is_caps ? 'Z' : 'z';
		default: break;
	}

	return '\0';
}

void in_handle_keyboard(const bstone::sys::KeyboardEvent& e)
{
	switch (e.key)
	{
		case bstone::sys::KeyboardKey::left_alt: in_is_lalt_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::right_alt: in_is_ralt_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::left_ctrl: in_is_lctrl_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::right_ctrl: in_is_rctrl_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::left_shift: in_is_lshift_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::right_shift: in_is_rshift_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::left_gui: in_is_lgui_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::right_gui: in_is_rgui_pressed = e.is_pressed; break;

		case bstone::sys::KeyboardKey::caps_lock: in_is_caps_lock_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::scroll_lock: in_is_scroll_lock_pressed = e.is_pressed; break;
		case bstone::sys::KeyboardKey::num_lock: in_is_num_lock_pressed = e.is_pressed; break;

		default: break;
	}

	const auto key = in_keyboard_map_to_bstone(e.key);

	if (key == ScanCode::sc_none)
	{
		return;
	}

	// Check for special keys
	if (e.is_pressed)
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
// TODO
#if 0
#ifndef __vita__
		// (vita) TranslateControllerEvent() does not currently affect the output of SDL_GetModState()
		case ScanCode::sc_alt: is_pressed = ((key_mod & KMOD_ALT) != 0); break;
#endif
#else
		case ScanCode::sc_alt: is_pressed = in_is_lalt_pressed || in_is_ralt_pressed; break;
#endif
		case ScanCode::sc_control: is_pressed = in_is_lctrl_pressed || in_is_rctrl_pressed; break;
		default: is_pressed = e.is_pressed; break;
	}

	Keyboard[key] = is_pressed;

	if (is_pressed)
	{
		LastScan = key;
		const auto key_char = in_keyboard_map_to_char(e);

		if (key_char != '\0')
		{
			LastASCII = key_char;
		}
	}
}

void in_handle_mouse_buttons(const bstone::sys::MouseButtonEvent& e)
{
	auto key = ScanCode::sc_none;
	auto is_pressed = e.is_pressed;

	switch (e.button_index)
	{
		case bstone::sys::MouseButtonIndex::left: key = ScanCode::sc_mouse_left; break;
		case bstone::sys::MouseButtonIndex::middle: key = ScanCode::sc_mouse_middle; break;
		case bstone::sys::MouseButtonIndex::right: key = ScanCode::sc_mouse_right; break;
		case bstone::sys::MouseButtonIndex::x1: key = ScanCode::sc_mouse_x1; break;
		case bstone::sys::MouseButtonIndex::x2: key = ScanCode::sc_mouse_x2; break;
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

void in_handle_mouse_motion(const bstone::sys::MouseMotionEvent& e)
{
	if (in_is_mouse_grabbed)
	{
		in_mouse_dx += e.delta_x;
		in_mouse_dy += e.delta_y;
	}
	else
	{
		in_mouse_dx = 0;
		in_mouse_dy = 0;
	}
}

void in_handle_mouse_wheel(const bstone::sys::MouseWheelEvent& e)
{
	if (!in_is_mouse_grabbed)
	{
		return;
	}

	const auto vertical_value = (e.direction == bstone::sys::MouseWheelDirection::normal ? e.y : -e.y);

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

auto in_is_take_screenshot_key_pressed = false;

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
	bstone::globals::sys_mouse_mgr = nullptr;
	bstone::globals::sys_event_mgr = nullptr;
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

void in_handle_window(const bstone::sys::WindowEvent& e)
{
	auto reset_state = false;

	switch (e.event_type)
	{
		case bstone::sys::WindowEventType::keyboard_focus_gained:
			reset_state = true;

			if (in_last_is_mouse_grabbed)
			{
				in_last_is_mouse_grabbed = in_grab_mouse(true);
			}

			sd_mute(false);
			break;

		case bstone::sys::WindowEventType::keyboard_focus_lost:
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

} // namespace

void in_handle_events()
{
	in_is_take_screenshot_key_pressed = false;
	auto e = bstone::sys::Event{};

	while (bstone::globals::sys_event_mgr->poll_event(e))
	{
		switch (e.common.type)
		{
// TODO
#if 0
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
#endif
			case bstone::sys::EventType::keyboard:
				in_handle_keyboard(e.keyboard);
				break;

			case bstone::sys::EventType::mouse_motion:
				in_handle_mouse_motion(e.mouse_motion);
				break;

			case bstone::sys::EventType::mouse_button:
				in_handle_mouse_buttons(e.mouse_button);
				break;

			case bstone::sys::EventType::mouse_wheel:
				in_handle_mouse_wheel(e.mouse_wheel);
				break;

			case bstone::sys::EventType::window:
				in_handle_window(e.window);
				break;

			case bstone::sys::EventType::quit:
				Quit();

			default:
				break;
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
void IN_ReadControl(std::int16_t, ControlInfo* control_info)
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
bool IN_UserInput(std::int32_t delay)
{
	const auto lasttime = static_cast<bstone::GameTimerTicks>(TimeCount);

	IN_StartAck();

	do
	{
		VL_WaitVBL(1);

		if (IN_CheckAck())
		{
			return true;
		}
	} while ((TimeCount - lasttime) < delay);

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

// TODO
#if 0
#ifdef __vita__
	// Vita joysticks are treated separately from other kinds of joystick
	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		SDL_Init(SDL_INIT_JOYSTICK);
	}

	SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);
#endif
#endif

	bstone::globals::sys_event_mgr = &bstone::globals::sys_system_mgr->get_event_mgr();

	if (!bstone::globals::sys_event_mgr->is_initialized())
	{
		BSTONE_THROW_STATIC_SOURCE("No event subsystem.");
	}

	bstone::globals::sys_mouse_mgr = &bstone::globals::sys_video_mgr->get_mouse_mgr();

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

	in_is_lalt_pressed = false;
	in_is_ralt_pressed = false;
	in_is_lctrl_pressed = false;
	in_is_rctrl_pressed = false;
	in_is_lshift_pressed = false;
	in_is_rshift_pressed = false;
	in_is_lgui_pressed = false;
	in_is_rgui_pressed = false;

	in_is_caps_lock_pressed = false;
	in_is_scroll_lock_pressed = false;
	in_is_num_lock_pressed = false;
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

BindingId in_binding_name_to_id(bstone::StringView name_sv)
try {
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
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bstone::StringView in_binding_id_to_name(BindingId binding_id)
try {
	for (const auto& map_item_id : in_binding_id_name_to_id_map)
	{
		if (map_item_id.binding_id == binding_id)
		{
			return map_item_id.name_sv;
		}
	}

	BSTONE_THROW_STATIC_SOURCE("Unknown binding ID.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// --------------------------------------------------------------------------

class InClearBindingsCCmdAction final : public bstone::CCmdAction
{
public:
	InClearBindingsCCmdAction() noexcept = default;
	~InClearBindingsCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try {
		if (!args.is_empty())
		{
			BSTONE_THROW_STATIC_SOURCE("Too many arguments.");
		}

		in_clear_bindings();
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
};

constexpr auto in_clear_bindings_sv = bstone::StringView{"in_clear_bindings"};
auto in_clear_bindings_ccmd_action = InClearBindingsCCmdAction{};
auto in_clear_bindings_ccmd = bstone::CCmd{in_clear_bindings_sv, in_clear_bindings_ccmd_action};

// --------------------------------------------------------------------------

std::intptr_t in_parse_binding_slot_index(bstone::StringView slot_index_name_sv)
try {
	auto slot_index = std::intptr_t{};
	bstone::from_chars(slot_index_name_sv.cbegin(), slot_index_name_sv.cend(), slot_index);

	if (slot_index < 0 || slot_index > k_max_binding_keys)
	{
		auto message = std::string{};
		message += "Slot index \"";
		message.append(slot_index_name_sv.get_data(), static_cast<std::size_t>(slot_index_name_sv.get_size()));
		message += "\" out of range.";
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}

	return slot_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// --------------------------------------------------------------------------

class InClearBindingCCmdAction final : public bstone::CCmdAction
{
public:
	InClearBindingCCmdAction() noexcept = default;
	~InClearBindingCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try {
		if (args.get_size() != 2)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid argument count.");
		}

		const auto binding_id = in_binding_name_to_id(args[0]);
		const auto slot_index = in_parse_binding_slot_index(args[1]);
		in_bindings[binding_id][slot_index] = ScanCode::sc_none;
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
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
try {
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
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bstone::StringView in_scan_code_id_to_name(ScanCode scan_code)
try {
	for (const auto& map_item : in_scan_code_name_to_id_map)
	{
		if (map_item.scan_code == scan_code)
		{
			return map_item.name_sv;
		}
	}

	BSTONE_THROW_STATIC_SOURCE("Unknown scan code name.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

class InBindCCmdAction final : public bstone::CCmdAction
{
public:
	InBindCCmdAction() noexcept = default;
	~InBindCCmdAction() override = default;

private:
	void do_invoke(bstone::CCmdActionArgs args) override
	try {
		if (args.get_size() != 3)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid argument count.");
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
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
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

		auto raw_binding_id = static_cast<std::intptr_t>(BindingId{});

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

					const auto slot_index_char_count = bstone::to_chars(
						slot_index,
						std::begin(slot_index_chars),
						std::end(slot_index_chars)) - slot_index_chars;

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
