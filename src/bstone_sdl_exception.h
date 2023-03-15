/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Ensures SDL result.

#ifndef BSTONE_SDL_EXCEPTION_INCLUDED
#define BSTONE_SDL_EXCEPTION_INCLUDED

namespace bstone {

[[noreturn]] void fail_sdl();

// ==========================================================================

int sdl_ensure_result(int sdl_result);

template<typename T>
T* sdl_ensure_result(T* sdl_result)
{
	if (sdl_result == nullptr)
	{
		fail_sdl();
	}

	return sdl_result;
}

} // bstone


#endif // !BSTONE_SDL_EXCEPTION_INCLUDED
