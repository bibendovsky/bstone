/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Interpretation of Steam's bookkeeping files.
//
// These read the text of `libraryfolders.vdf` and `appmanifest_<appid>.acf`,
// which Steam writes identically on every platform. They touch no files of their
// own, so locating and reading those files is left to the caller.

#ifndef BSTONE_STEAM_MANIFEST_INCLUDED
#define BSTONE_STEAM_MANIFEST_INCLUDED

#include <string>
#include <string_view>
#include <vector>

namespace bstone {

struct SteamManifest
{
	// Extracts the library folder paths from `libraryfolders.vdf`.
	//
	// Handles both layouts Steam has used: the current one maps a numbered key to
	// an object carrying "path", the legacy one maps it straight to the path.
	// Neither the Steam root nor any book-keeping key is returned, so the caller
	// adds the root itself.
	//
	// Returns an empty list for text that is not a library folder file.
	static std::vector<std::string> parse_library_paths(std::string_view vdf_text);

	// Extracts the install directory from `appmanifest_<appid>.acf`.
	//
	// Fails unless Steam considers the application installed, and unless the
	// recorded directory is a bare name - anything else is a corrupt or
	// hand-edited manifest trying to point outside the library.
	static bool parse_install_dir(std::string_view acf_text, std::string& install_dir);
};

} // namespace bstone

#endif // BSTONE_STEAM_MANIFEST_INCLUDED
