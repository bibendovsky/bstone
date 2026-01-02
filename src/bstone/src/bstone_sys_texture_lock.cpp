/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture lock

#include "bstone_sys_texture_lock.h"

namespace bstone::sys {

void* TextureLock::get_pixels() const
{
	return do_get_pixels();
}

int TextureLock::get_pitch() const
{
	return do_get_pitch();
}

} // namespace bstone::sys
