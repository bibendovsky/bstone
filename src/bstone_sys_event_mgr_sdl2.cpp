/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include "SDL.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_memory_resource.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_event_mgr_sdl2.h"
#include "bstone_sys_exception_sdl2.h"

#define BSTONE_SDL_2_0_4 SDL_VERSION_ATLEAST(2, 0, 4)

namespace bstone {
namespace sys {

namespace {

class Sdl2EventMgr final : public EventMgr
{
public:
	Sdl2EventMgr(Logger& logger);
	Sdl2EventMgr(const Sdl2EventMgr&) = delete;
	Sdl2EventMgr& operator=(const Sdl2EventMgr&) = delete;
	~Sdl2EventMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;

private:
	static VirtualKey map_key_code(SDL_Keycode sdl_key_code);
	static unsigned int map_mouse_buttons_mask(Uint32 sdl_buttons_mask);
	static int map_mouse_button(int sdl_button);
#if BSTONE_SDL_2_0_4
	static MouseWheelDirection map_mouse_wheel_direction(SDL_MouseWheelDirection sdl_direction);
#endif

	static bool handle_event(const SDL_KeyboardEvent& sdl_e, KeyboardEvent& e);
	static bool handle_event(const SDL_MouseMotionEvent& sdl_e, MouseMotionEvent& e);
	static bool handle_event(const SDL_MouseButtonEvent& sdl_e, MouseButtonEvent& e);
	static bool handle_event(const SDL_MouseWheelEvent& sdl_e, MouseWheelEvent& e);
	static bool handle_event(const SDL_WindowEvent& sdl_e, WindowEvent& e);
	static bool handle_event(const SDL_Event& sdl_e, Event& e);

	bool do_poll_event(Event& e) override;
};

// ==========================================================================

using Sdl2EventMgrPool = SinglePoolMemoryResource<Sdl2EventMgr>;
Sdl2EventMgrPool sdl2_event_mgr_pool{};

// ==========================================================================

Sdl2EventMgr::Sdl2EventMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL event manager.");

	sdl2_ensure_result(SDL_InitSubSystem(SDL_INIT_EVENTS));

	logger_.log_information(">>> SDL event manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2EventMgr::~Sdl2EventMgr()
{
	logger_.log_information("Shut down SDL event manager.");

	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void* Sdl2EventMgr::operator new(std::size_t size)
try {
	return sdl2_event_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2EventMgr::operator delete(void* ptr)
{
	sdl2_event_mgr_pool.deallocate(ptr);
}

VirtualKey Sdl2EventMgr::map_key_code(SDL_Keycode sdl_key_code)
{
	switch (sdl_key_code)
	{
		case SDLK_0: return vk_0;
		case SDLK_1: return vk_1;
		case SDLK_2: return vk_2;
		case SDLK_3: return vk_3;
		case SDLK_4: return vk_4;
		case SDLK_5: return vk_5;
		case SDLK_6: return vk_6;
		case SDLK_7: return vk_7;
		case SDLK_8: return vk_8;
		case SDLK_9: return vk_9;

		case SDLK_a: return vk_a;
		case SDLK_b: return vk_b;
		case SDLK_c: return vk_c;
		case SDLK_d: return vk_d;
		case SDLK_e: return vk_e;
		case SDLK_f: return vk_f;
		case SDLK_g: return vk_g;
		case SDLK_h: return vk_h;
		case SDLK_i: return vk_i;
		case SDLK_j: return vk_j;
		case SDLK_k: return vk_k;
		case SDLK_l: return vk_l;
		case SDLK_m: return vk_m;
		case SDLK_n: return vk_n;
		case SDLK_o: return vk_o;
		case SDLK_p: return vk_p;
		case SDLK_q: return vk_q;
		case SDLK_r: return vk_r;
		case SDLK_s: return vk_s;
		case SDLK_t: return vk_t;
		case SDLK_u: return vk_u;
		case SDLK_v: return vk_v;
		case SDLK_w: return vk_w;
		case SDLK_x: return vk_x;
		case SDLK_y: return vk_y;
		case SDLK_z: return vk_z;

		case SDLK_F1: return vk_f1;
		case SDLK_F2: return vk_f2;
		case SDLK_F3: return vk_f3;
		case SDLK_F4: return vk_f4;
		case SDLK_F5: return vk_f5;
		case SDLK_F6: return vk_f6;
		case SDLK_F7: return vk_f7;
		case SDLK_F8: return vk_f8;
		case SDLK_F9: return vk_f9;
		case SDLK_F10: return vk_f10;
		case SDLK_F11: return vk_f11;
		case SDLK_F12: return vk_f12;

		case SDLK_LEFT: return vk_left;
		case SDLK_RIGHT: return vk_right;
		case SDLK_DOWN: return vk_down;
		case SDLK_UP: return vk_up;

		case SDLK_INSERT: return vk_insert;
		case SDLK_DELETE: return vk_delete;
		case SDLK_HOME: return vk_home;
		case SDLK_END: return vk_end;
		case SDLK_PAGEUP: return vk_page_up;
		case SDLK_PAGEDOWN: return vk_page_down;

		case SDLK_KP_0: return vk_kp_0;
		case SDLK_KP_1: return vk_kp_1;
		case SDLK_KP_2: return vk_kp_2;
		case SDLK_KP_3: return vk_kp_3;
		case SDLK_KP_4: return vk_kp_4;
		case SDLK_KP_6: return vk_kp_6;
		case SDLK_KP_7: return vk_kp_7;
		case SDLK_KP_8: return vk_kp_8;
		case SDLK_KP_9: return vk_kp_9;
		case SDLK_KP_A: return vk_kp_a;
		case SDLK_KP_B: return vk_kp_b;
		case SDLK_KP_C: return vk_kp_c;
		case SDLK_KP_D: return vk_kp_d;
		case SDLK_KP_E: return vk_kp_e;
		case SDLK_KP_F: return vk_kp_f;

		case SDLK_KP_TAB: return vk_kp_tab;
		case SDLK_KP_SPACE: return vk_kp_space;
		case SDLK_KP_PLUS: return vk_kp_plus;
		case SDLK_KP_MINUS: return vk_kp_minus;
		case SDLK_KP_LEFTBRACE: return vk_kp_lbrace;
		case SDLK_KP_RIGHTBRACE: return vk_kp_rbrace;
		case SDLK_KP_ENTER: return vk_kp_enter;
		case SDLK_KP_DIVIDE: return vk_kp_divide;
		case SDLK_KP_COMMA: return vk_kp_comma;
		case SDLK_KP_BACKSPACE: return vk_kp_backspace;

		case SDLK_CAPSLOCK: return vk_caps_lock;
		case SDLK_SCROLLLOCK: return vk_scroll_lock;
		case SDLK_NUMLOCKCLEAR: return vk_num_lock;

		case SDLK_LALT: return vk_lalt;
		case SDLK_RALT: return vk_ralt;
		case SDLK_LCTRL: return vk_lctrl;
		case SDLK_RCTRL: return vk_rctrl;
		case SDLK_LSHIFT: return vk_lshift;
		case SDLK_RSHIFT: return vk_rshift;
		case SDLK_LGUI: return vk_lgui;
		case SDLK_RGUI: return vk_rgui;

		case SDLK_BACKSLASH: return vk_backslash;
		case SDLK_BACKSPACE: return vk_backspace;
		case SDLK_BACKQUOTE: return vk_backtick;
		case SDLK_COMMA: return vk_comma;
		case SDLK_EQUALS: return vk_equals;
		case SDLK_ESCAPE: return vk_escape;
		case SDLK_MINUS: return vk_minus;
		case SDLK_PAUSE: return vk_pause;
		case SDLK_PERIOD: return vk_period;
		case SDLK_PRINTSCREEN: return vk_prt_scr;
		case SDLK_QUOTE: return vk_quote;
		case SDLK_LEFTBRACKET: return vk_lbracket;
		case SDLK_RIGHTBRACKET: return vk_rbracket;
		case SDLK_RETURN: return vk_return;
		case SDLK_SEMICOLON: return vk_semicolon;
		case SDLK_SLASH: return vk_slash;
		case SDLK_SPACE: return vk_space;
		case SDLK_TAB: return vk_tab;

		default: return vk_none;
	}
}

unsigned int Sdl2EventMgr::map_mouse_buttons_mask(Uint32 sdl_buttons_mask)
{
	auto button_mask = 0U;

	if ((sdl_buttons_mask & SDL_BUTTON_LMASK) != 0)
	{
		button_mask |= MouseButtonMask::left;
	}

	if ((sdl_buttons_mask & SDL_BUTTON_MMASK) != 0)
	{
		button_mask |= MouseButtonMask::middle;
	}

	if ((sdl_buttons_mask & SDL_BUTTON_RMASK) != 0)
	{
		button_mask |= MouseButtonMask::right;
	}

	if ((sdl_buttons_mask & SDL_BUTTON_X1MASK) != 0)
	{
		button_mask |= MouseButtonMask::x1;
	}

	if ((sdl_buttons_mask & SDL_BUTTON_X2MASK) != 0)
	{
		button_mask |= MouseButtonMask::x2;
	}

	return button_mask;
}

int Sdl2EventMgr::map_mouse_button(int sdl_button)
{
	switch (sdl_button)
	{
		case SDL_BUTTON_LEFT: return MouseButtonIndex::left;
		case SDL_BUTTON_MIDDLE: return MouseButtonIndex::middle;
		case SDL_BUTTON_RIGHT: return MouseButtonIndex::right;
		case SDL_BUTTON_X1: return MouseButtonIndex::x1;
		case SDL_BUTTON_X2: return MouseButtonIndex::x2;

		default: return -1;
	}
}

#if BSTONE_SDL_2_0_4
MouseWheelDirection Sdl2EventMgr::map_mouse_wheel_direction(SDL_MouseWheelDirection sdl_direction)
{
	switch (sdl_direction)
	{
		case SDL_MOUSEWHEEL_NORMAL: return MouseWheelDirection::normal;
		case SDL_MOUSEWHEEL_FLIPPED: return MouseWheelDirection::flipped;

		default: return MouseWheelDirection::none;
	}
}
#endif

bool Sdl2EventMgr::handle_event(const SDL_KeyboardEvent& sdl_e, KeyboardEvent& e)
{
	const auto virtual_key = map_key_code(static_cast<SDL_Keycode>(sdl_e.keysym.sym));

	if (virtual_key == vk_none)
	{
		return false;
	}

	e.is_pressed = (sdl_e.state == SDL_PRESSED);
	e.key = virtual_key;
	e.repeat_count = sdl_e.repeat;
	e.window_id = sdl_e.windowID;
	e.type = EventType::keyboard;
	return true;
}

bool Sdl2EventMgr::handle_event(const SDL_MouseMotionEvent& sdl_e, MouseMotionEvent& e)
{
	if (sdl_e.which == SDL_TOUCH_MOUSEID)
	{
		return false;
	}

	e.x = sdl_e.x;
	e.y = sdl_e.y;
	e.delta_x = sdl_e.xrel;
	e.delta_y = sdl_e.yrel;
	e.button_mask = map_mouse_buttons_mask(sdl_e.state);
	e.window_id = sdl_e.windowID;
	e.type = EventType::mouse_motion;
	return true;
}

bool Sdl2EventMgr::handle_event(const SDL_MouseButtonEvent& sdl_e, MouseButtonEvent& e)
{
	if (sdl_e.which == SDL_TOUCH_MOUSEID)
	{
		return false;
	}

	const auto button_index = map_mouse_button(sdl_e.button);

	if (button_index < 0)
	{
		return false;
	}

	e.is_pressed = (sdl_e.state == SDL_PRESSED);
	e.x = sdl_e.x;
	e.y = sdl_e.y;
	e.button_index = button_index;
#if SDL_VERSION_ATLEAST(2, 0, 2)
	e.click_count = sdl_e.clicks;
#else
	e.click_count = 1;
#endif
	e.window_id = sdl_e.windowID;
	e.type = EventType::mouse_button;
	return true;
}

bool Sdl2EventMgr::handle_event(const SDL_MouseWheelEvent& sdl_e, MouseWheelEvent& e)
{
	if (sdl_e.which == SDL_TOUCH_MOUSEID)
	{
		return false;
	}

	const auto direction = map_mouse_wheel_direction(static_cast<SDL_MouseWheelDirection>(sdl_e.direction));

	if (direction == MouseWheelDirection::none)
	{
		return false;
	}

	e.x = sdl_e.x;
	e.y = sdl_e.y;
#if BSTONE_SDL_2_0_4
	e.direction = direction;
#else
	e.direction = MouseWheelDirection::normal;
#endif
	e.window_id = sdl_e.windowID;
	e.type = EventType::mouse_wheel;
	return true;
}

bool Sdl2EventMgr::handle_event(const SDL_WindowEvent& sdl_e, WindowEvent& e)
{
	auto is_handled = true;
	e.id = sdl_e.windowID;

	switch (sdl_e.event)
	{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			e.event_type = WindowEventType::keyboard_focus_gained;
			break;

		case SDL_WINDOWEVENT_FOCUS_LOST:
			e.event_type = WindowEventType::keyboard_focus_lost;
			break;

		default:
			is_handled = false;
			break;
	}

	if (!is_handled)
	{
		return false;
	}

	e.type = EventType::window;
	return true;
}

bool Sdl2EventMgr::handle_event(const SDL_Event& sdl_e, Event& e)
{
	switch (sdl_e.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			return handle_event(sdl_e.key, e.keyboard);

		case SDL_MOUSEMOTION:
			return handle_event(sdl_e.motion, e.mouse_motion);

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			return handle_event(sdl_e.button, e.mouse_button);

		case SDL_MOUSEWHEEL:
			return handle_event(sdl_e.wheel, e.mouse_wheel);

		case SDL_WINDOWEVENT:
			return handle_event(sdl_e.window, e.window);

		default:
			return false;
	}
}

bool Sdl2EventMgr::do_poll_event(Event& e)
{
	auto sdl_e = SDL_Event{};

	while (SDL_PollEvent(&sdl_e))
	{
		if (handle_event(sdl_e, e))
		{
			e.common.timestamp = sdl_e.common.timestamp;
			return true;
		}
	}

	e.common.type = EventType::none;
	return false;
}

} // namespace

// ==========================================================================

EventMgrUPtr make_sdl2_event_mgr(Logger& logger)
try {
	return std::make_unique<Sdl2EventMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
