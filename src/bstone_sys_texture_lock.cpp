/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_texture_lock.h"

namespace bstone {
namespace sys {

void* TextureLock::get_pixels() const noexcept
{
	return do_get_pixels();
}

int TextureLock::get_pitch() const noexcept
{
	return do_get_pitch();
}

} // namespace sys
} // namespace bstone
