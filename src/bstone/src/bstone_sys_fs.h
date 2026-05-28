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

enum class FileType
{
	none = 0,
	file,
	directory,
	other,
};

// =====================================

int get_working_directory(char* buffer, int buffer_size);

void create_directories(const char* path);

void rename_path(const char* old_path, const char* new_path);

void remove_path_if_exists(const char* path);
void remove_path(const char* path);

bool is_directory_exists(const char* path);
bool is_regular_file_exists(const char* path);
bool get_file_type(const char* path, FileType& file_type);

// =====================================

enum EnumDirCallbackResult
{
	resume,
	success,
	failure,
};

using EnumDirCallback = EnumDirCallbackResult (*)(
	void* user_data,
	const char* directory_path,
	const char* file_name);

bool enumerate_directory(const char* path, EnumDirCallback callback, void* user_data);

} // namespace bstone::sys

#endif // BSTONE_SYS_FS_INCLUDED
