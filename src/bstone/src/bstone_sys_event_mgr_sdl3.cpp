/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_event_mgr_sdl3.h"

#include "SDL3/SDL.h"

#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl3.h"
#include "bstone_sys_event_mgr_null.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_sdl3_subsystem.h"

namespace bstone {
namespace sys {

namespace {

class Sdl3EventMgr final : public EventMgr
{
public:
	Sdl3EventMgr(Logger& logger);
	Sdl3EventMgr(const Sdl3EventMgr&) = delete;
	Sdl3EventMgr& operator=(const Sdl3EventMgr&) = delete;
	~Sdl3EventMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	bool is_initialized_{};
	Sdl3Subsystem sdl3_subsystem_{};

private:
	bool do_is_initialized() const noexcept override;

	bool do_poll_event(Event& e) override;

private:
	static KeyboardKey map_key_code(SDL_Keycode sdl_key_code) noexcept;
	static unsigned int map_mouse_buttons_mask(Uint32 sdl_buttons_mask) noexcept;
	static int map_mouse_button(int sdl_button) noexcept;
	static MouseWheelDirection map_mouse_wheel_direction(SDL_MouseWheelDirection sdl_direction) noexcept;

	static bool handle_event(const SDL_KeyboardEvent& sdl_e, KeyboardEvent& e) noexcept;
	static bool handle_event(const SDL_MouseMotionEvent& sdl_e, MouseMotionEvent& e) noexcept;
	static bool handle_event(const SDL_MouseButtonEvent& sdl_e, MouseButtonEvent& e) noexcept;
	static bool handle_event(const SDL_MouseWheelEvent& sdl_e, MouseWheelEvent& e) noexcept;
	static bool handle_event(const SDL_WindowEvent& sdl_e, WindowEvent& e) noexcept;
	static bool handle_event(const SDL_Event& sdl_e, Event& e) noexcept;
};

// ==========================================================================

using Sdl3EventMgrPool = SinglePoolResource<Sdl3EventMgr>;
Sdl3EventMgrPool sdl3_event_mgr_pool{};

// ==========================================================================

Sdl3EventMgr::Sdl3EventMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL event manager.");

	auto sdl3_subsystem = Sdl3Subsystem{SDL_INIT_EVENTS};
	sdl3_subsystem.swap(sdl3_subsystem_);
	is_initialized_ = true;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3EventMgr::~Sdl3EventMgr()
{
	logger_.log_information("Shut down SDL event manager.");
}

void* Sdl3EventMgr::operator new(std::size_t size)
try {
	return sdl3_event_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3EventMgr::operator delete(void* ptr)
{
	sdl3_event_mgr_pool.deallocate(ptr);
}

bool Sdl3EventMgr::do_is_initialized() const noexcept
{
	return is_initialized_;
}

bool Sdl3EventMgr::do_poll_event(Event& e)
{
	BSTONE_ASSERT(is_initialized_);

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

KeyboardKey Sdl3EventMgr::map_key_code(SDL_Keycode sdl_key_code) noexcept
{
	switch (sdl_key_code)
	{
		case SDLK_0: return KeyboardKey::n0;
		case SDLK_1: return KeyboardKey::n1;
		case SDLK_2: return KeyboardKey::n2;
		case SDLK_3: return KeyboardKey::n3;
		case SDLK_4: return KeyboardKey::n4;
		case SDLK_5: return KeyboardKey::n5;
		case SDLK_6: return KeyboardKey::n6;
		case SDLK_7: return KeyboardKey::n7;
		case SDLK_8: return KeyboardKey::n8;
		case SDLK_9: return KeyboardKey::n9;

		case SDLK_A: return KeyboardKey::a;
		case SDLK_B: return KeyboardKey::b;
		case SDLK_C: return KeyboardKey::c;
		case SDLK_D: return KeyboardKey::d;
		case SDLK_E: return KeyboardKey::e;
		case SDLK_F: return KeyboardKey::f;
		case SDLK_G: return KeyboardKey::g;
		case SDLK_H: return KeyboardKey::h;
		case SDLK_I: return KeyboardKey::i;
		case SDLK_J: return KeyboardKey::j;
		case SDLK_K: return KeyboardKey::k;
		case SDLK_L: return KeyboardKey::l;
		case SDLK_M: return KeyboardKey::m;
		case SDLK_N: return KeyboardKey::n;
		case SDLK_O: return KeyboardKey::o;
		case SDLK_P: return KeyboardKey::p;
		case SDLK_Q: return KeyboardKey::q;
		case SDLK_R: return KeyboardKey::r;
		case SDLK_S: return KeyboardKey::s;
		case SDLK_T: return KeyboardKey::t;
		case SDLK_U: return KeyboardKey::u;
		case SDLK_V: return KeyboardKey::v;
		case SDLK_W: return KeyboardKey::w;
		case SDLK_X: return KeyboardKey::x;
		case SDLK_Y: return KeyboardKey::y;
		case SDLK_Z: return KeyboardKey::z;

		case SDLK_F1: return KeyboardKey::f1;
		case SDLK_F2: return KeyboardKey::f2;
		case SDLK_F3: return KeyboardKey::f3;
		case SDLK_F4: return KeyboardKey::f4;
		case SDLK_F5: return KeyboardKey::f5;
		case SDLK_F6: return KeyboardKey::f6;
		case SDLK_F7: return KeyboardKey::f7;
		case SDLK_F8: return KeyboardKey::f8;
		case SDLK_F9: return KeyboardKey::f9;
		case SDLK_F10: return KeyboardKey::f10;
		case SDLK_F11: return KeyboardKey::f11;
		case SDLK_F12: return KeyboardKey::f12;

		case SDLK_LEFT: return KeyboardKey::left;
		case SDLK_RIGHT: return KeyboardKey::right;
		case SDLK_DOWN: return KeyboardKey::down;
		case SDLK_UP: return KeyboardKey::up;

		case SDLK_INSERT: return KeyboardKey::insert;
		case SDLK_DELETE: return KeyboardKey::del;
		case SDLK_HOME: return KeyboardKey::home;
		case SDLK_END: return KeyboardKey::end;
		case SDLK_PAGEUP: return KeyboardKey::page_up;
		case SDLK_PAGEDOWN: return KeyboardKey::page_down;

		case SDLK_KP_0: return KeyboardKey::kp_0;
		case SDLK_KP_1: return KeyboardKey::kp_1;
		case SDLK_KP_2: return KeyboardKey::kp_2;
		case SDLK_KP_3: return KeyboardKey::kp_3;
		case SDLK_KP_4: return KeyboardKey::kp_4;
		case SDLK_KP_6: return KeyboardKey::kp_6;
		case SDLK_KP_7: return KeyboardKey::kp_7;
		case SDLK_KP_8: return KeyboardKey::kp_8;
		case SDLK_KP_9: return KeyboardKey::kp_9;
		case SDLK_KP_A: return KeyboardKey::kp_a;
		case SDLK_KP_B: return KeyboardKey::kp_b;
		case SDLK_KP_C: return KeyboardKey::kp_c;
		case SDLK_KP_D: return KeyboardKey::kp_d;
		case SDLK_KP_E: return KeyboardKey::kp_e;
		case SDLK_KP_F: return KeyboardKey::kp_f;

		case SDLK_KP_TAB: return KeyboardKey::kp_tab;
		case SDLK_KP_SPACE: return KeyboardKey::kp_space;
		case SDLK_KP_PLUS: return KeyboardKey::kp_plus;
		case SDLK_KP_MINUS: return KeyboardKey::kp_minus;
		case SDLK_KP_LEFTBRACE: return KeyboardKey::kp_left_brace;
		case SDLK_KP_RIGHTBRACE: return KeyboardKey::kp_right_brace;
		case SDLK_KP_ENTER: return KeyboardKey::kp_enter;
		case SDLK_KP_DIVIDE: return KeyboardKey::kp_divide;
		case SDLK_KP_COMMA: return KeyboardKey::kp_comma;
		case SDLK_KP_BACKSPACE: return KeyboardKey::kp_backspace;

		case SDLK_CAPSLOCK: return KeyboardKey::caps_lock;
		case SDLK_SCROLLLOCK: return KeyboardKey::scroll_lock;
		case SDLK_NUMLOCKCLEAR: return KeyboardKey::num_lock;

		case SDLK_LALT: return KeyboardKey::left_alt;
		case SDLK_RALT: return KeyboardKey::right_alt;
		case SDLK_LCTRL: return KeyboardKey::left_ctrl;
		case SDLK_RCTRL: return KeyboardKey::right_ctrl;
		case SDLK_LSHIFT: return KeyboardKey::left_shift;
		case SDLK_RSHIFT: return KeyboardKey::right_shift;
		case SDLK_LGUI: return KeyboardKey::left_gui;
		case SDLK_RGUI: return KeyboardKey::right_gui;

		case SDLK_BACKSLASH: return KeyboardKey::backslash;
		case SDLK_BACKSPACE: return KeyboardKey::backspace;
		case SDLK_GRAVE: return KeyboardKey::backtick;
		case SDLK_COMMA: return KeyboardKey::comma;
		case SDLK_EQUALS: return KeyboardKey::equals;
		case SDLK_ESCAPE: return KeyboardKey::esc;
		case SDLK_MINUS: return KeyboardKey::minus;
		case SDLK_PAUSE: return KeyboardKey::pause;
		case SDLK_PERIOD: return KeyboardKey::period;
		case SDLK_PRINTSCREEN: return KeyboardKey::prt_scr;
		case SDLK_APOSTROPHE: return KeyboardKey::quote;
		case SDLK_LEFTBRACKET: return KeyboardKey::left_bracket;
		case SDLK_RIGHTBRACKET: return KeyboardKey::right_bracket;
		case SDLK_RETURN: return KeyboardKey::enter;
		case SDLK_SEMICOLON: return KeyboardKey::semicolon;
		case SDLK_SLASH: return KeyboardKey::slash;
		case SDLK_SPACE: return KeyboardKey::space;
		case SDLK_TAB: return KeyboardKey::tab;

		default: return KeyboardKey::none;
	}
}

unsigned int Sdl3EventMgr::map_mouse_buttons_mask(Uint32 sdl_buttons_mask) noexcept
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

int Sdl3EventMgr::map_mouse_button(int sdl_button) noexcept
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

MouseWheelDirection Sdl3EventMgr::map_mouse_wheel_direction(SDL_MouseWheelDirection sdl_direction) noexcept
{
	switch (sdl_direction)
	{
		case SDL_MOUSEWHEEL_NORMAL: return MouseWheelDirection::normal;
		case SDL_MOUSEWHEEL_FLIPPED: return MouseWheelDirection::flipped;

		default: return MouseWheelDirection::none;
	}
}

bool Sdl3EventMgr::handle_event(const SDL_KeyboardEvent& sdl_e, KeyboardEvent& e) noexcept
{
	const auto virtual_key = map_key_code(sdl_e.key);

	if (virtual_key == KeyboardKey::none)
	{
		return false;
	}

	e.is_pressed = sdl_e.down;
	e.key = virtual_key;
	e.repeat_count = sdl_e.repeat;
	e.window_id = sdl_e.windowID;
	e.type = EventType::keyboard;
	return true;
}

bool Sdl3EventMgr::handle_event(const SDL_MouseMotionEvent& sdl_e, MouseMotionEvent& e) noexcept
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

bool Sdl3EventMgr::handle_event(const SDL_MouseButtonEvent& sdl_e, MouseButtonEvent& e) noexcept
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

	e.is_pressed = sdl_e.down;
	e.x = sdl_e.x;
	e.y = sdl_e.y;
	e.button_index = button_index;
	e.click_count = sdl_e.clicks;
	e.window_id = sdl_e.windowID;
	e.type = EventType::mouse_button;
	return true;
}

bool Sdl3EventMgr::handle_event(const SDL_MouseWheelEvent& sdl_e, MouseWheelEvent& e) noexcept
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
	e.direction = direction;
	e.window_id = sdl_e.windowID;
	e.type = EventType::mouse_wheel;
	return true;
}

bool Sdl3EventMgr::handle_event(const SDL_WindowEvent& sdl_e, WindowEvent& e) noexcept
{
	auto is_handled = true;
	e.id = sdl_e.windowID;

	switch (sdl_e.type)
	{
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			e.event_type = WindowEventType::keyboard_focus_gained;
			break;

		case SDL_EVENT_WINDOW_FOCUS_LOST:
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

bool Sdl3EventMgr::handle_event(const SDL_Event& sdl_e, Event& e) noexcept
{
	switch (sdl_e.type)
	{
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			return handle_event(sdl_e.key, e.keyboard);

		case SDL_EVENT_MOUSE_MOTION:
			return handle_event(sdl_e.motion, e.mouse_motion);

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			return handle_event(sdl_e.button, e.mouse_button);

		case SDL_EVENT_MOUSE_WHEEL:
			return handle_event(sdl_e.wheel, e.mouse_wheel);

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			return handle_event(sdl_e.window, e.window);

		default:
			return false;
	}
}

} // namespace

// ==========================================================================

EventMgrUPtr make_sdl3_event_mgr(Logger& logger)
try
{
	return std::make_unique<Sdl3EventMgr>(logger);
}
catch (...)
{
	return make_null_event_mgr(logger);
}

} // namespace sys
} // namespace bstone
