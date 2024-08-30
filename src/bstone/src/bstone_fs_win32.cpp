/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if defined(_WIN32)

#include <windows.h>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_fs.h"
#include "bstone_utf.h"
#include "bstone_win32_wstring.h"

namespace bstone {
namespace fs {

std::intptr_t get_working_directory(char* buffer, std::intptr_t buffer_size)
{
	BSTONE_ASSERT(buffer_size >= 0);

	const auto w_size_with_null = GetCurrentDirectoryW(0, nullptr);

	if (w_size_with_null == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current directory's path length.");
	}

	if (w_size_with_null > 0x7FFFFFFF)
	{
		BSTONE_THROW_STATIC_SOURCE("Current directory path too long.");
	}

	auto w_buffer = Win32WString{static_cast<std::intptr_t>(w_size_with_null)};
	const auto w_size = GetCurrentDirectoryW(w_size_with_null, w_buffer.get_data());

	if (w_size == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current directory.");
	}

	const auto u8_next = utf::u8_to_u16(
		w_buffer.get_data(),
		w_buffer.get_data() + w_size,
		buffer,
		buffer + buffer_size);

	const auto u8_size = u8_next - buffer;

	if (u8_size == buffer_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Destination buffer too small.");
	}

	buffer[u8_size] = '\0';
	return u8_size;
}

// ==========================================================================

void set_working_directory(const char* path)
{
	const auto w_path = Win32WString{path};
	const auto set_current_directory_result = SetCurrentDirectoryW(w_path.get_data());

	if (set_current_directory_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set current directory.");
	}
}

// ==========================================================================

void create_directory(const char* path)
{
	const auto w_path = Win32WString{path};
	const auto create_directory_result = CreateDirectoryW(w_path.get_data(), nullptr);

	if (create_directory_result == 0)
	{
		const auto error_code = GetLastError();

		if (error_code != ERROR_ALREADY_EXISTS)
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to create a directory.");
		}
	}
}

// ==========================================================================

void create_directories(const char* path)
{
	const auto u8_size_with_null = char_traits::get_size(path) + 1;
	auto w_path = Win32WString{u8_size_with_null};

	const auto w_next = utf::u8_to_u16(
		path,
		path + u8_size_with_null,
		w_path.get_data(),
		w_path.get_data() + u8_size_with_null);

	for (auto w_chars = w_path.get_data(); w_chars != w_next; ++w_chars)
	{
		auto& w_char = *w_chars;

		if (w_char != native_separator && w_char != '\0')
		{
			continue;
		}

		const auto w_old_char = w_char;
		w_char = L'\0';

		const auto create_directory_result = CreateDirectoryW(w_path.get_data(), nullptr);

		if (create_directory_result == 0)
		{
			const auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
			{
				BSTONE_THROW_STATIC_SOURCE("Failed to create a sub-directory");
			}
		}

		w_char = w_old_char;
	}
}

// ==========================================================================

void rename(const char* old_path, const char* new_path)
{
	const auto w_old_path = Win32WString{old_path};
	const auto w_new_path = Win32WString{new_path};
	const auto move_file_result = MoveFileW(w_old_path.get_data(), w_new_path.get_data());

	if (move_file_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to rename.");
	}
}

// ==========================================================================

namespace {

void win32_remove(const char* path, bool ignore_if_not_found)
{
	const auto w_path = Win32WString{path};
	const auto get_file_attributes_result = GetFileAttributesW(w_path.get_data());

	auto is_failed = false;

	if (get_file_attributes_result != INVALID_FILE_ATTRIBUTES)
	{
		const auto is_directory = ((get_file_attributes_result & FILE_ATTRIBUTE_DIRECTORY) != 0);

		if (is_directory)
		{
			const auto remove_directory_result = RemoveDirectoryW(w_path.get_data());
			is_failed = remove_directory_result == 0;
		}
		else
		{
			const auto delete_file_result = DeleteFileW(w_path.get_data());
			is_failed = delete_file_result == 0;
		}
	}
	else
	{
		is_failed = true;
	}

	if (is_failed)
	{
		if (ignore_if_not_found)
		{
			const auto error_code = GetLastError();

			switch (error_code)
			{
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND:
					return;
			}
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to remove.");
	}
}

} // namespace

// ==========================================================================

void remove_if_exists(const char* path)
{
	return win32_remove(path, true);
}

// ==========================================================================

void remove(const char* path)
{
	return win32_remove(path, false);
}

// ==========================================================================

bool is_directory_exists(const char* path)
{
	const auto w_path = Win32WString{path};
	const auto file_attributes = GetFileAttributesW(w_path.get_data());

	if (file_attributes == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return (file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// ==========================================================================

bool is_regular_file_exists(const char* path)
{
	const auto w_path = Win32WString{path};
	const auto file_attributes = GetFileAttributesW(w_path.get_data());

	if (file_attributes == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return (file_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

} // namespace fs
} // namespace bstone

#endif // _WIN32
