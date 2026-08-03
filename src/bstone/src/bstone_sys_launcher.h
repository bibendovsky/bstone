/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Launcher panel

#ifndef BSTONE_SYS_LAUNCHER_INCLUDED
#define BSTONE_SYS_LAUNCHER_INCLUDED

#include <span>
#include <string>
#include <vector>

namespace bstone::sys {

// One release of a game, and where the copy of it was found. Two copies of the
// same release are one of these: there is nothing to choose between them.
struct LauncherRelease
{
	// How the release is known - "2.1", or empty for a game that had only one.
	std::string label;
	// Marks a release that is not the full game.
	std::string qualifier;
	std::string source;
	// Where the copy that would be played sits.
	std::string path;
	// What the caller gets back for this release.
	int item_index;
	// Which release to start on when several were found, lowest first. It
	// decides the choice, not the order they are shown in.
	int preference;
};

// A game the launcher can offer, with every release of it that was found.
struct LauncherItem
{
	std::string title;
	// Artwork read from the game the user provided; empty when it has none.
	const void* art_pixels;
	int art_width;
	int art_height;
	std::vector<LauncherRelease> releases;
};

enum class LauncherAction
{
	quit,
	play,
	add_source,
	get_shareware,
};

struct LauncherResult
{
	LauncherAction action;
	int item_index;
};

// Invoked while the launcher window is still up, so the folder dialog belongs
// to it. "parent_window" is the launcher's window handle.
using LauncherAddSourceFunc = bool (*)(void* user_data, void* parent_window);

struct Launcher
{
	// Runs the launcher panel in its own window until the user picks a game,
	// asks to add a source, or leaves. The panel draws with its own embedded
	// font, so it works before any game files have been found.
	// "shareware_url" offers the user a way to get a game when they have none;
	// pass nullptr once they have one, so the offer goes away.
	static LauncherResult run(
		std::span<const LauncherItem> items,
		const char* empty_message,
		const char* add_source_note,
		const char* shareware_url,
		LauncherAddSourceFunc add_source_func,
		void* user_data);
};

} // namespace bstone::sys

#endif // BSTONE_SYS_LAUNCHER_INCLUDED
