/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Artwork shipped alongside a game the user provided.

#ifndef BSTONE_GAME_ART_INCLUDED
#define BSTONE_GAME_ART_INCLUDED

#include <string>
#include <vector>

#include "bstone_rgb8.h"

namespace bstone {

struct GameArt
{
	int width;
	int height;
	Rgba8Buffer pixels;

	bool is_empty() const
	{
		return width <= 0 || height <= 0;
	}
};

// Looks for the artwork a store put next to the game - the icon of the
// application holding it - and decodes it.
//
// Nothing here is distributed with BStone: the artwork is read from the copy
// of the game the user pointed at, and only for as long as it is shown.
GameArt find_game_art(const std::string& game_path);

} // namespace bstone

#endif // BSTONE_GAME_ART_INCLUDED
