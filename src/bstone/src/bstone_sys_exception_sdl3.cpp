/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL3/SDL_error.h"
#include "bstone_sys_exception_sdl3.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl3_fail()
{
	BSTONE_THROW_DYNAMIC_SOURCE(SDL_GetError());
}

void sdl3_ensure_result(bool sdl_result)
{
	if (!sdl_result)
	{
		sdl3_fail();
	}
}

int sdl3_ensure_result(int sdl_result)
try {
	if (sdl_result != 0)
	{
		sdl3_fail();
	}

	return sdl_result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
