/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Process management (SDL)

#include "bstone_process.h"
#include "bstone_sdl.h"
#include "SDL3/SDL_misc.h"

namespace bstone::process {

void open_file_or_url(const char* url)
{
	if (!SDL_OpenURL(url))
	{
		sdl::fail("SDL_OpenURL");
	}
}

} // namespace bstone::process
