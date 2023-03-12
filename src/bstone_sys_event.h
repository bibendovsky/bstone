/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_EVENT_INCLUDED)
#define BSTONE_SYS_EVENT_INCLUDED

#include "bstone_sys_mouse.h"
#include "bstone_sys_virtual_key.h"

namespace bstone {
namespace sys {

enum class EventType
{
	none = 0,
	keyboard,
	mouse_motion,
	mouse_button,
	mouse_wheel,
	window,
	quit,
};

struct KeyboardEvent
{
	bool is_pressed;
	VirtualKey key;
	int repeat_count;
	int window_id;
};

struct MouseMotionEvent
{
	int window_id;
	int x;
	int y;
	int delta_x;
	int delta_y;
	unsigned int buttons_mask;
};

struct MouseButtonEvent
{
	bool is_pressed;
	int window_id;
	int x;
	int y;
	int button_index;
	int click_count;
};

enum class MouseWheelDirection
{
	none = 0,
	normal,
	flipped,
};

struct MouseWheelEvent
{
	int window_id;
	int x;
	int y;
	MouseWheelDirection direction;
};

enum class WindowEventType
{
	none = 0,
	keyboard_focus_gained,
	keyboard_focus_lost,
};

struct WindowEvent
{
	WindowEventType type;
	unsigned int id;
};

union EventData
{
	KeyboardEvent keyboard;
	MouseMotionEvent mouse_motion;
	MouseButtonEvent mouse_button;
	MouseWheelEvent mouse_wheel;
	WindowEvent window;
};

struct Event
{
	EventType type;
	unsigned int timestamp;
	EventData data;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_EVENT_INCLUDED
