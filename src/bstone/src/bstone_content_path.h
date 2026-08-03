/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Content path.
//


#ifndef BSTONE_CONTENT_PATH_INCLUDED
#define BSTONE_CONTENT_PATH_INCLUDED


#include <string>


namespace bstone
{


enum class ContentPathProvider
{
	none,
	gog,
	steam,
}; // ContentPathProvider


struct AssetPath
{
	ContentPathProvider provider;

	std::string aog;
	std::string ps;
}; // AssetProvider


AssetPath make_content_path(
	ContentPathProvider type);

// Path to the artwork Steam already downloaded for the game installed at
// "game_path", or an empty string when the game is not a Steam one or the
// client has no artwork cached for it.
//
// The client keeps this beside its own data rather than with the game, so it
// cannot be found by looking around the game itself.
std::string make_steam_art_path(const std::string& game_path);


} // bstone


#endif // !BSTONE_CONTENT_PATH_INCLUDED
