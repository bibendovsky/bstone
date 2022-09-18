/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sdl_utils.h"

#include <cassert>

#include "bstone_sdl_exception.h"


namespace bstone
{
namespace sdl
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void fill_window_black(
	::SDL_Window* sdl_window)
{
	assert(sdl_window);

	const auto sdl_window_surface = ensure_sdl_result(::SDL_GetWindowSurface(sdl_window));

	const auto black_color = ::SDL_MapRGB(sdl_window_surface->format, 0, 0, 0);
	ensure_sdl_result(::SDL_FillRect(sdl_window_surface, nullptr, black_color));
	ensure_sdl_result(::SDL_UpdateWindowSurface(sdl_window));
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // sdl
} // bstone
