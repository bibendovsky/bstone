/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Folder dialog

#ifndef BSTONE_SYS_FOLDER_DIALOG_INCLUDED
#define BSTONE_SYS_FOLDER_DIALOG_INCLUDED

#include <string>

namespace bstone::sys {

struct FolderDialog
{
	// Asks the user for a folder and blocks until they answer. Returns an empty
	// string if they cancelled, or if the platform has no folder dialog.
	static std::string show(const char* title, const char* default_path);
};

} // namespace bstone::sys

#endif // BSTONE_SYS_FOLDER_DIALOG_INCLUDED
