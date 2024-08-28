/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef _WIN32

#include <cstdint>

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_fs.h"
#include "bstone_posix_string.h"

namespace bstone {
namespace fs {

std::intptr_t get_working_directory(char* buffer, std::intptr_t buffer_size)
{
	const auto getcwd_result = getcwd(buffer, buffer_size);

	if (getcwd_result == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current directory.");
	}

	return char_traits::get_size(buffer);
}

// ==========================================================================

void set_working_directory(const char* path)
{
	const auto chdir_result = chdir(path);

	if (chdir_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set current directory.");
	}
}

// ==========================================================================

void create_directory(const char* path)
{
	const auto umask_mode = umask(0);
	umask(umask_mode);
	const auto mode = static_cast<mode_t>((S_IRWXU | S_IRWXG | S_IRWXO) & ~umask_mode);
	const auto mkdir_result = mkdir(path, mode);

	if (mkdir_result != 0)
	{
		if (errno != EEXIST)
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to create a directory.");
		}
	}
}

// ==========================================================================

void create_directories(const char* path)
{
	auto posix_path = PosixString{path};
	const auto size_with_null = posix_path.get_size() + 1;
	auto got_umask = false;
	auto umask_mode = mode_t{};
	auto posix_chars = posix_path.get_data();

	for (auto i = std::intptr_t{}; i < size_with_null; ++i)
	{
		if (i == 0 || (posix_chars[i] != native_separator && posix_chars[i] != '\0'))
		{
			continue;
		}

		const auto old_char = posix_chars[i];
		posix_chars[i] = '\0';

		if (!got_umask)
		{
			got_umask = true;
			umask_mode = umask(0);
			umask(umask_mode);
		}

		const auto mode = static_cast<mode_t>((S_IRWXU | S_IRWXG | S_IRWXO) & (~umask_mode));
		const auto mkdir_result = mkdir(posix_chars, mode);

		if (mkdir_result != 0)
		{
			if (errno != EEXIST)
			{
				BSTONE_THROW_STATIC_SOURCE("Failed to create a sub-directory.");
			}
		}

		posix_chars[i] = old_char;
	}
}

// ==========================================================================

void rename(const char* old_path, const char* new_path)
{
	const auto rename_result = ::rename(old_path, new_path);

	if (rename_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to rename.");
	}
}

// ==========================================================================

void remove_if_exists(const char* path)
{
	const auto remove_result = ::remove(path);

	if (remove_result != 0)
	{
		switch (errno)
		{
			case ENOENT: break;
			default: BSTONE_THROW_STATIC_SOURCE("Failed to remove.");
		}
	}
}

// ==========================================================================

void remove(const char* path)
{
	const auto remove_result = ::remove(path);

	if (remove_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to remove.");
	}
}

// ==========================================================================

bool is_directory_exists(const char* path)
{
	struct stat posix_stat;

	if (stat(path, &posix_stat) != 0)
	{
		return false;
	}

	return S_ISDIR(posix_stat.st_mode) != 0;
}

// ==========================================================================

bool is_regular_file_exists(const char* path)
{
	struct stat posix_stat;

	if (stat(path, &posix_stat) != 0)
	{
		return false;
	}

	return S_ISREG(posix_stat.st_mode) != 0;
}

} // namespace fs
} // namespace bstone

#endif // _WIN32
