/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_EXCEPTION_SDL2_INCLUDED
#define BSTONE_SYS_EXCEPTION_SDL2_INCLUDED

#include "bstone_exception.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl2_fail();

// ==========================================================================

int sdl2_ensure_result(int sdl_result);

template<typename T>
T* sdl2_ensure_result(T* sdl_result)
try {
	if (sdl_result == nullptr)
	{
		sdl2_fail();
	}

	return sdl_result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_EXCEPTION_SDL2_INCLUDED
