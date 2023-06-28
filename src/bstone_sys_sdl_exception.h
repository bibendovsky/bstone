/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_EXCEPTION_INCLUDED)
#define BSTONE_SYS_SDL_EXCEPTION_INCLUDED

#include "bstone_exception.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl_fail();

// ==========================================================================

int sdl_ensure_result(int sdl_result);

template<typename T>
T* sdl_ensure_result(T* sdl_result)
BSTONE_BEGIN_FUNC_TRY
	if (sdl_result == nullptr)
	{
		sdl_fail();
	}

	return sdl_result;
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_EXCEPTION_INCLUDED
