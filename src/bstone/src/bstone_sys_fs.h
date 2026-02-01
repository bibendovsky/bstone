/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system management

#ifndef BSTONE_SYS_FS_INCLUDED
#define BSTONE_SYS_FS_INCLUDED

namespace bstone::sys {

constexpr char native_separator =
#ifdef _WIN32
	'\\'
#else
	'/'
#endif
;

// =====================================

int get_working_directory(char* buffer, int buffer_size);

void create_directories(const char* path);

void rename_path(const char* old_path, const char* new_path);

void remove_path_if_exists(const char* path);
void remove_path(const char* path);

bool is_directory_exists(const char* path);
bool is_regular_file_exists(const char* path);

} // namespace bstone::sys

#endif // BSTONE_SYS_FS_INCLUDED
