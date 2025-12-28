/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_EXCEPTION_SDL3_INCLUDED
#define BSTONE_SYS_EXCEPTION_SDL3_INCLUDED

#include "bstone_exception.h"

namespace bstone {
namespace sys {

[[noreturn]] void sdl3_fail();

// ==========================================================================

void sdl3_ensure_result(bool sdl_result);
int sdl3_ensure_result(int sdl_result);

template<typename T>
T* sdl3_ensure_result(T* sdl_result)
try {
	if (sdl_result == nullptr)
	{
		sdl3_fail();
	}

	return sdl_result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_EXCEPTION_SDL3_INCLUDED
