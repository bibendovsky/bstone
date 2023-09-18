/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_file_system.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <windows.h>
#else
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif // _WIN32

#include "bstone_encoding.h"
#include "bstone_exception.h"


namespace bstone
{
namespace file_system
{


namespace detail
{


char get_separator() noexcept
{
	return
#if _WIN32
		'\\'
#else
		'/'
#endif // _WIN32
		;
}


} // detail


std::string normalize_path(
	const std::string& path)
{
	auto result = path;

	for (auto& ch : result)
	{
		if ((ch == '/' || ch == '\\') && ch != detail::get_separator())
		{
			ch = detail::get_separator();
		}
	}

	return result;
}

std::string append_path_separator(
	const std::string& path)
{
	auto result = path;

	if (!result.empty() && result.back() != detail::get_separator())
	{
		result += detail::get_separator();
	}

	return result;
}

std::string append_path(
	const std::string& path,
	const std::string& sub_path)
{
	if (path.empty() && sub_path.empty())
	{
		return std::string{};
	}

	if (path.empty())
	{
		return sub_path;
	}
	else if (sub_path.empty())
	{
		return path;
	}
	else
	{
		auto result = std::string{};
		result.reserve(path.size() + 1 + sub_path.size());
		result.assign(path);

		if (result.back() != detail::get_separator())
		{
			result += detail::get_separator();
		}

		result += sub_path;

		return result;
	}
}

void replace_extension(
	std::string& path_name,
	const std::string& new_extension)
try {
	if (path_name.empty() || new_extension.empty())
	{
		return;
	}

	if (new_extension.front() != '.')
	{
		BSTONE_THROW_STATIC_SOURCE("An extension should start with a dot.");
	}

	const auto separator_pos = path_name.find_last_of("\\/");
	const auto dot_pos = path_name.find('.', separator_pos);

	if (dot_pos != std::string::npos)
	{
		if (dot_pos == 0 || dot_pos == (path_name.size() - 1))
		{
			return;
		}

		path_name.resize(dot_pos);
	}

	path_name += new_extension;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string get_working_dir()
try {
#if _WIN32
	const auto utf16_string_size = GetCurrentDirectoryW(0, nullptr);

	if (utf16_string_size == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("GetCurrentDirectoryW");
	}

	auto utf16_dir = std::u16string{};
	utf16_dir.resize(utf16_string_size - 1);

	const auto win32_result = GetCurrentDirectoryW(
		utf16_string_size,
		reinterpret_cast<LPWSTR>(&utf16_dir[0])
	);

	if (win32_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("GetCurrentDirectoryW");
	}

	utf16_dir.resize(win32_result);

	return u16_to_u8(utf16_dir);
#else
	const auto max_size = pathconf(".", _PC_PATH_MAX);

	if (max_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Unknown max path length.");
	}

	constexpr auto initial_size = 256;
	constexpr auto size_delta = 256;

	auto size = initial_size;
	auto result = std::string{};

	while (true)
	{
		result.resize(size - 1);

		const auto posix_result = getcwd(&result[0], size);

		if (posix_result != nullptr)
		{
			break;
		}

		switch (errno)
		{
			case ERANGE:
				break;

			case EACCES:
				BSTONE_THROW_STATIC_SOURCE("No access.");

			default:
				BSTONE_THROW_STATIC_SOURCE("Generic error.");
		}

		size += size_delta;

		if (size > max_size)
		{
			BSTONE_THROW_STATIC_SOURCE("Path too long.");
		}
	}

	result.resize(std::string::traits_type::length(result.c_str()));

	return result;
#endif // _WIN32
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string resolve_path(
	const std::string& path)
try {
	if (path.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty path.");
	}

#ifdef _WIN32
	const auto& utf16_path = u8_to_u16(path);

	const auto utf16_full_path_size = GetFullPathNameW(
		reinterpret_cast<LPCWSTR>(utf16_path.c_str()),
		0,
		nullptr,
		nullptr
	);

	if (utf16_full_path_size == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("GetFullPathNameW");
	}

	auto utf16_full_path = std::u16string{};
	utf16_full_path.resize(utf16_full_path_size - 1);

	const auto win32_result = GetFullPathNameW(
		reinterpret_cast<LPCWSTR>(utf16_path.c_str()),
		utf16_full_path_size,
		reinterpret_cast<LPWSTR>(&utf16_full_path[0]),
		nullptr
	);

	if (win32_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("GetFullPathNameW");
	}

	utf16_full_path.resize(win32_result);

	return u16_to_u8(utf16_full_path);
#else
	const auto max_size = pathconf(".", _PC_PATH_MAX);

	if (max_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Unknown max path length.");
	}

	auto result = std::string{};
	result.resize(max_size - 1);

	const auto posix_result = realpath(path.c_str(), &result[0]);

	if (posix_result == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("realpath");
	}

	result.resize(std::string::traits_type::length(result.c_str()));

	return result;
#endif // _WIN32
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool has_file(
	const std::string& path)
{
	if (path.empty())
	{
		return false;
	}

#if _WIN32
	const auto& utf16_path = u8_to_u16(path);

	WIN32_FIND_DATAW win32_fd;

	const auto win32_result = FindFirstFileW(
		reinterpret_cast<LPCWSTR>(utf16_path.c_str()),
		&win32_fd
	);

	if (win32_result == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	static_cast<void>(FindClose(win32_result));

	return true;
#else
	struct stat posix_stat;

	const auto posix_result = stat(path.c_str(), &posix_stat);

	if (posix_result != 0)
	{
		return false;
	}

	return S_ISREG(posix_stat.st_mode) != 0;
#endif // _WIN32
}

void rename(
	const std::string& old_path,
	const std::string& new_path)
try {
	if (old_path.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty old path.");
	}

	if (new_path.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty new path.");
	}

#if _WIN32
	const auto& old_path_utf16 = u8_to_u16(old_path);
	const auto& new_path_utf16 = u8_to_u16(new_path);

	const auto win32_move_file_result = MoveFileExW(
		reinterpret_cast<LPCWSTR>(old_path_utf16.c_str()),
		reinterpret_cast<LPCWSTR>(new_path_utf16.c_str()),
		MOVEFILE_REPLACE_EXISTING
	);

	if (win32_move_file_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("MoveFileExW");
	}
#else
	const auto posix_rename_result = ::rename(old_path.c_str(), new_path.c_str());

	if (posix_rename_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("rename");
	}
#endif // _WIN32
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED


} // file_system
} // bstone
