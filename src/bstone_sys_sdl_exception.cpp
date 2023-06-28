/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_error.h"
#include "bstone_sys_sdl_exception.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl_fail()
{
	BSTONE_THROW_DYNAMIC_SOURCE(SDL_GetError());
}

int sdl_ensure_result(int sdl_result)
BSTONE_BEGIN_FUNC_TRY
	if (sdl_result != 0)
	{
		sdl_fail();
	}

	return sdl_result;
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
