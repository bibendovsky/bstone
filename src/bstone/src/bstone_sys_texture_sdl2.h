/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_TEXTURE_SDL2_INCLUDED
#define BSTONE_SYS_TEXTURE_SDL2_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_texture.h"

struct SDL_Renderer;

namespace bstone {
namespace sys {

TextureUPtr make_sdl2_texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_TEXTURE_SDL2_INCLUDED
