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

namespace bstone::sys {

// A game the launcher can offer.
struct LauncherItem
{
	std::string title;
	std::string detail;
	// Artwork read from the game the user provided; empty when it has none.
	const void* art_pixels;
	int art_width;
	int art_height;
};

enum class LauncherAction
{
	quit,
	play,
	add_source,
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
	static LauncherResult run(
		std::span<const LauncherItem> items,
		const char* empty_message,
		const char* add_source_note,
		LauncherAddSourceFunc add_source_func,
		void* user_data);
};

} // namespace bstone::sys

#endif // BSTONE_SYS_LAUNCHER_INCLUDED
