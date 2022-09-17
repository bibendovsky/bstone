/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_SDL_UTILS_INCLUDED
#define BSTONE_SDL_UTILS_INCLUDED


#include "SDL_video.h"


namespace bstone
{
namespace sdl
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void fill_window_black(
	::SDL_Window* sdl_window);

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // sdl
} // bstone


#endif // !BSTONE_SDL_UTILS_INCLUDED
