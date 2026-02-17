/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window

#include "bstone_sys_window.h"

namespace bstone::sys {

void Window::center()
{
	constinit static const WindowOffset centered_offset = WindowOffset::make_centered();
	set_position(WindowPosition{centered_offset, centered_offset});
}

} // namespace bstone::sys
