/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_error.h"
#include "bstone_sys_exception_sdl2.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl2_fail()
{
	BSTONE_THROW_DYNAMIC_SOURCE(SDL_GetError());
}

int sdl2_ensure_result(int sdl_result)
try {
	if (sdl_result != 0)
	{
		sdl2_fail();
	}

	return sdl_result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
