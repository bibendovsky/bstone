/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_EVENT_INCLUDED
#define BSTONE_SYS_EVENT_INCLUDED

#include "bstone_sys_keyboard_key.h"
#include "bstone_sys_mouse.h"

namespace bstone {
namespace sys {

enum class EventType
{
	none,
	quit,
	window,
	keyboard,
	mouse_motion,
	mouse_button,
	mouse_wheel,
};

struct CommonEvent
{
	EventType type;
	unsigned int timestamp;
};

struct KeyboardEvent : CommonEvent
{
	bool is_pressed;
	KeyboardKey key;
	int repeat_count;
	unsigned int window_id;
};

struct MouseMotionEvent : CommonEvent
{
	int x;
	int y;
	int delta_x;
	int delta_y;
	unsigned int button_mask;
	unsigned int window_id;
};

struct MouseButtonEvent : CommonEvent
{
	bool is_pressed;
	int x;
	int y;
	int button_index;
	int click_count;
	unsigned int window_id;
};

struct MouseWheelEvent : CommonEvent
{
	int x;
	int y;
	MouseWheelDirection direction;
	unsigned int window_id;
};

enum class WindowEventType
{
	none,
	keyboard_focus_gained,
	keyboard_focus_lost,
};

struct WindowEvent : CommonEvent
{
	WindowEventType event_type;
	unsigned int id;
};

union Event
{
	CommonEvent common;
	KeyboardEvent keyboard;
	MouseMotionEvent mouse_motion;
	MouseButtonEvent mouse_button;
	MouseWheelEvent mouse_wheel;
	WindowEvent window;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_EVENT_INCLUDED
