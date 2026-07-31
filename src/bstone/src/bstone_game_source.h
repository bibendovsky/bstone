/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Locating a game's files under a folder the user chose.

#ifndef BSTONE_GAME_SOURCE_INCLUDED
#define BSTONE_GAME_SOURCE_INCLUDED

#include <string>
#include <vector>

namespace bstone {

using GameSourcePaths = std::vector<std::string>;

// Looks for the folders holding a game's files at or below the given one.
//
// The files are rarely where a person would point: a storefront on macOS buries
// them inside an application bundle - four levels down for one, eight for
// another - and a bundle cannot even be opened from a folder dialog. So take
// whatever the user chose, be it one game, a folder of games, or the folder the
// files themselves are in, and search down from it.
//
// The walk is bounded in both depth and directories visited, so choosing a whole
// drive gives up rather than hanging.
GameSourcePaths find_game_sources(const std::string& path);

// What to tell the user before the folder dialog opens, spelled for the
// platform they are on and how the game is sold for it.
const char* get_game_source_prompt();

} // namespace bstone

#endif // BSTONE_GAME_SOURCE_INCLUDED
