/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture (SDL)

#ifndef BSTONE_SYS_TEXTURE_SDL_INCLUDED
#define BSTONE_SYS_TEXTURE_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_texture.h"
#include "SDL3/SDL_render.h"

namespace bstone::sys {

TextureUPtr make_texture_sdl(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param);

} // namespace bstone::sys

#endif // BSTONE_SYS_TEXTURE_SDL_INCLUDED
