/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Minimal reader for Valve Data Format (VDF, also known as KeyValues) text.
//
// Supports only what Steam's `libraryfolders.vdf` and `appmanifest_*.acf` actually
// use: quoted and bare tokens, nested objects, `//` comments and the `\\`, `\"`,
// `\n`, `\t` escapes. Binary VDF, `#base`/`#include` and platform conditionals are
// out of scope.

#ifndef BSTONE_VDF_INCLUDED
#define BSTONE_VDF_INCLUDED

#include <string>
#include <string_view>
#include <vector>

namespace bstone {

// A node is either an object (it has children) or a leaf (it has a value).
struct VdfNode
{
	std::string name;
	std::string value; // Meaningful only for a leaf.
	std::vector<VdfNode> children;

	bool is_object() const noexcept;

	// Finds a direct child by name. Names are compared case-insensitively (ASCII),
	// as KeyValues does, which also makes the caller immune to Steam's renaming of
	// the root key from "LibraryFolders" to "libraryfolders".
	// Returns null when there is no such child.
	const VdfNode* find_child(std::string_view child_name) const noexcept;

	// Value of a direct leaf child, or an empty view when there is no such child.
	std::string_view find_value(std::string_view child_name) const noexcept;
};

// Parses VDF text into `root`, whose children are the top-level entries.
//
// Returns false if the text is malformed, in which case `root` is left empty.
// Never throws, so a corrupt file on disk is reported rather than propagated.
bool parse_vdf(std::string_view text, VdfNode& root) noexcept;

} // namespace bstone

#endif // BSTONE_VDF_INCLUDED
