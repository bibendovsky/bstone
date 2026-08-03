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
	//
	// "parent_window" is the sys::Window handle to hang the dialog off, so it
	// belongs to the window that asked instead of appearing on its own.
	static std::string show(const char* title, const char* default_path, void* parent_window);

	// Where the dialog should start, for someone who has not chosen before.
	static const char* get_default_location();
};

} // namespace bstone::sys

#endif // BSTONE_SYS_FOLDER_DIALOG_INCLUDED
