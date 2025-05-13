/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SDL2 version utility

#ifndef BSTONE_SYS_SDL2_VERSION_INCLUDED
#define BSTONE_SYS_SDL2_VERSION_INCLUDED

#include "SDL_version.h"

bool operator<(const SDL_version& a, const SDL_version& b);

#endif // BSTONE_SYS_SDL2_VERSION_INCLUDED
