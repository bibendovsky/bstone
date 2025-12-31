/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture lock (SDL)

#ifndef BSTONE_SYS_TEXTURE_LOCK_SDL_INCLUDED
#define BSTONE_SYS_TEXTURE_LOCK_SDL_INCLUDED

#include "bstone_sys_rectangle.h"
#include "bstone_sys_texture_sdl.h"
#include "bstone_sys_texture_lock_sdl.h"
#include "SDL3/SDL_render.h"

namespace bstone::sys {

TextureLockUPtr make_texture_lock_sdl(SDL_Texture& sdl_texture, const Rectangle* rect);

} // namespace bstone::sys

#endif // BSTONE_SYS_TEXTURE_LOCK_SDL_INCLUDED
