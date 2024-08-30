/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_TEXTURE_LOCK_SDL2_INCLUDED
#define BSTONE_SYS_TEXTURE_LOCK_SDL2_INCLUDED

#include "bstone_sys_rectangle.h"
#include "bstone_sys_texture_sdl2.h"
#include "bstone_sys_texture_lock_sdl2.h"

struct SDL_Texture;

namespace bstone {
namespace sys {

TextureLockUPtr make_sdl2_texture_lock(SDL_Texture& sdl_texture, const Rectangle* rect);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_TEXTURE_LOCK_SDL2_INCLUDED
