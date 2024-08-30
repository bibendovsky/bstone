/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system primitives.

#ifndef BSTONE_FS_INCLUDED
#define BSTONE_FS_INCLUDED

#include <cstdint>

namespace bstone {
namespace fs {

constexpr char native_separator =
#if defined(_WIN32)
	'\\'
#else
	'/'
#endif
;

// ==========================================================================

std::intptr_t get_working_directory(char* buffer, std::intptr_t buffer_size);
void set_working_directory(const char* path);

void create_directory(const char* path);
void create_directories(const char* path);

void rename(const char* old_path, const char* new_path);

void remove_if_exists(const char* path);
void remove(const char* path);

bool is_directory_exists(const char* path);
bool is_regular_file_exists(const char* path);

} // namespace fs
} // namespace bstone

#endif // BSTONE_FS_INCLUDED
