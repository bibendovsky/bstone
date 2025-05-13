/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SDL2 version utility

#include "bstone_sys_sdl2_version.h"

bool operator<(const SDL_version& a, const SDL_version& b)
{
	return
		a.major < b.major ||
		(a.major == b.major && (a.minor < b.minor || a.minor == b.minor && a.patch < b.patch));
}
