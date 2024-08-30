/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
//      ID Engine
//      ID_IN.h - Header file for Input Manager
//      v1.0d1
//      By Jason Blochowiak
//

#ifndef BSTONE_ID_IN_INCLUDED
#define BSTONE_ID_IN_INCLUDED

#ifdef  __DEBUG__
#define __DEBUG_InputMgr__
#endif

#include <cstdint>
#include <bitset>
// TODO
#if 0
#ifdef __vita__
#include "SDL.h"
void TranslateControllerEvent(SDL_Event *ev);
void TranslateTouchEvent(SDL_Event *ev);
void TranslateAnalogEvent(SDL_Event *ev);
#endif
#endif
#include "bstone_ccmd_mgr.h"
#include "bstone_cvar_mgr.h"
#include "bstone_text_writer.h"

#define MaxPlayers 4
#define MaxKbds 2
#define NumCodes 128

enum class ScanCode
{
	sc_none = 0,

	sc_return = 0x1C,
	sc_escape = 0x01,
	sc_space = 0x39,
	sc_minus = 0x0C,
	sc_equals = 0x0D,
	sc_backspace = 0x0E,
	sc_tab = 0x0F,
	sc_alt = 0x38,
	sc_left_bracket = 0x1A,
	sc_right_bracket = 0x1B,
	sc_control = 0x1D,
	sc_caps_lock = 0x3A,
	sc_num_lock = 0x45,
	sc_scroll_lock = 0x46,
	sc_left_shift = 0x2A,
	sc_right_shift = 0x36,
	sc_up_arrow = 0x48,
	sc_down_arrow = 0x50,
	sc_left_arrow = 0x4B,
	sc_right_arrow = 0x4D,
	sc_insert = 0x52,
	sc_delete = 0x53,
	sc_home = 0x47,
	sc_end = 0x4F,
	sc_page_up = 0x49,
	sc_page_down = 0x51,
	sc_slash = 0x35,
	sc_f1 = 0x3B,
	sc_f2 = 0x3C,
	sc_f3 = 0x3D,
	sc_f4 = 0x3E,
	sc_f5 = 0x3F,
	sc_f6 = 0x40,
	sc_f7 = 0x41,
	sc_f8 = 0x42,
	sc_f9 = 0x43,
	sc_f10 = 0x44,
	sc_f11 = 0x57,
	sc_f12 = 0x59,
	sc_print_screen = 0x37,
	sc_pause = 0x54,
	sc_back_quote = 0x29,
	sc_semicolon = 0x27,
	sc_quote = 0x28,
	sc_backslash = 0x2B,
	sc_comma = 0x33,
	sc_period = 0x34,

	sc_1 = 0x02,
	sc_2 = 0x03,
	sc_3 = 0x04,
	sc_4 = 0x05,
	sc_5 = 0x06,
	sc_6 = 0x07,
	sc_7 = 0x08,
	sc_8 = 0x09,
	sc_9 = 0x0a,
	sc_0 = 0x0b,

	sc_a = 0x1E,
	sc_b = 0x30,
	sc_c = 0x2E,
	sc_d = 0x20,
	sc_e = 0x12,
	sc_f = 0x21,
	sc_g = 0x22,
	sc_h = 0x23,
	sc_i = 0x17,
	sc_j = 0x24,
	sc_k = 0x25,
	sc_l = 0x26,
	sc_m = 0x32,
	sc_n = 0x31,
	sc_o = 0x18,
	sc_p = 0x19,
	sc_q = 0x10,
	sc_r = 0x13,
	sc_s = 0x1F,
	sc_t = 0x14,
	sc_u = 0x16,
	sc_v = 0x2F,
	sc_w = 0x11,
	sc_x = 0x2D,
	sc_y = 0x15,
	sc_z = 0x2C,

	sc_kp_minus = 0x4A,
	sc_kp_plus = 0x4E,

	sc_mouse_left = 0x64,
	sc_mouse_middle = 0x65,
	sc_mouse_right = 0x66,
	sc_mouse_x1 = 0x67,
	sc_mouse_x2 = 0x68,

	sc_mouse_wheel_down = 0x69,
	sc_mouse_wheel_up = 0x6A,
};

#define key_None 0
#define key_Return 0x0d
#define key_Enter key_Return
#define key_Escape 0x1b
#define key_Space 0x20
#define key_BackSpace 0x08
#define key_Tab 0x09
#define key_Delete 0x7f
#define key_UnderScore 0x0c

// Stuff for the mouse
#define MReset 0
#define MButtons 3
#define MDelta 11

#define MouseInt 0x33

enum Demo
{
	demo_Off,
	demo_Record,
	demo_Playback,
	demo_PlayDone
};

enum ControlType
{
	ctrl_None, // JAM - added
	ctrl_Keyboard,
	ctrl_Keyboard1 = ctrl_Keyboard,
	ctrl_Keyboard2,
	ctrl_Mouse
};

enum Motion
{
	motion_Left = -1,
	motion_Up = -1,
	motion_None = 0,
	motion_Right = 1,
	motion_Down = 1
};

enum Direction
{
	dir_North,
	dir_NorthEast,
	dir_East,
	dir_SouthEast,
	dir_South,
	dir_SouthWest,
	dir_West,
	dir_NorthWest,
	dir_None
};

enum BindingId
{
	e_bi_forward,
	e_bi_backward,
	e_bi_left,
	e_bi_right,
	e_bi_strafe,
	e_bi_strafe_left,
	e_bi_strafe_right,
	e_bi_quick_left,
	e_bi_quick_right,
	e_bi_turn_around,
	e_bi_run,

	e_bi_attack,
	e_bi_weapon_1,
	e_bi_weapon_2,
	e_bi_weapon_3,
	e_bi_weapon_4,
	e_bi_weapon_5,
	e_bi_weapon_6,
	e_bi_weapon_7,

	e_bi_use,

	e_bi_stats,
	e_bi_radar_magnify,
	e_bi_radar_minify,

	e_bi_help,
	e_bi_save,
	e_bi_load,
	e_bi_sound,
	e_bi_controls,
	e_bi_end_game,
	e_bi_quick_save,
	e_bi_quick_load,
	e_bi_quick_exit,

	e_bi_attack_info,
	e_bi_lightning,
	e_bi_sfx,
	e_bi_music,
	e_bi_ceiling,
	e_bi_flooring,
	e_bi_heart_beat,

	e_bi_pause,
	e_bi_grab_mouse,

	e_bi_cycle_previous_weapon,
	e_bi_cycle_next_weapon,

	e_bi_take_screenshot,

	e_bi_last_entry,
};

constexpr auto k_max_binding_keys = 2;
constexpr auto k_max_bindings = e_bi_last_entry;

using Binding = ScanCode[k_max_binding_keys];
using Bindings = Binding[k_max_bindings];

extern Bindings in_bindings;

void in_set_default_bindings();

struct CursorInfo
{
	int button0;
	int button1;
	int button2;
	int button3;
	int x;
	int y;
	Motion xaxis;
	Motion yaxis;
	Direction dir;
};

using ControlInfo = CursorInfo;

struct KeyboardDef
{
	ScanCode button0;
	ScanCode button1;
	ScanCode upleft;
	ScanCode up;
	ScanCode upright;
	ScanCode left;
	ScanCode right;
	ScanCode downleft;
	ScanCode down;
	ScanCode downright;
};


// Global variables

class KeyboardState
{
private:
	using State = std::bitset<NumCodes>;


public:
	State::reference operator[](int index)
	{
		return state_[index];
	}

	State::reference operator[](ScanCode scan_code)
	{
		return state_[static_cast<std::size_t>(scan_code)];
	}

	void reset()
	{
		state_.reset();
	}

private:
	State state_;
};

extern ControlType ControlTypeUsed; // JAM - added
extern KeyboardState Keyboard;
extern bool MousePresent;
extern bool Paused;
extern char LastASCII;
extern ScanCode LastScan;
extern KeyboardDef KbdDefs;
extern ControlType Controls[MaxPlayers];

extern std::uint8_t* DemoBuffer;
extern std::uint16_t DemoOffset;
extern std::uint16_t DemoSize;

extern bool allcaps;

// Function prototypes

void in_initialize_cvars(bstone::CVarMgr& cvar_mgr);
void in_initialize_ccmds(bstone::CCmdMgr& ccmd_mgr);

bool in_is_mouse_enabled() noexcept;
void in_set_is_mouse_enabled(bool is_enabled);

int in_get_mouse_sensitivity() noexcept;
void in_set_mouse_sensitivity(int sensitivity);

// DEBUG - put names in prototypes

void IN_Startup();
void IN_Shutdown();

void IN_ClearKeysDown();
void IN_ReadCursor(CursorInfo*);
void IN_ReadControl(std::int16_t, ControlInfo*);
void IN_SetControlType(std::int16_t, ControlType);

void IN_Ack();

extern bool IN_UserInput(std::int32_t delay);
extern char IN_WaitForASCII();
extern const std::string& IN_GetScanName(ScanCode);

std::uint8_t IN_MouseButtons();

extern bool in_is_mouse_grabbed;

bool in_grab_mouse(bool grab);
void in_handle_events();
void in_get_mouse_deltas(int& dx, int& dy);
void in_clear_mouse_deltas();
bool in_is_binding_pressed(BindingId binding_id);
void in_reset_binding_state(BindingId binding_id);
void in_reset_state();
void in_serialize_bindings(bstone::TextWriter& text_writer);

#endif // BSTONE_ID_IN_INCLUDED
