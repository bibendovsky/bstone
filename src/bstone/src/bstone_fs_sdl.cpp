/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system management (SDL)

#include "bstone_fs.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include <string>
#include "SDL3/SDL_filesystem.h"

namespace bstone::fs {

namespace {

[[noreturn]] void fail_sdl_func(const char* sdl_func_name)
{
	const std::size_t sdl_func_name_length = std::string::traits_type::length(sdl_func_name);
	const char* const sdl_error_message = SDL_GetError();
	const std::size_t sdl_error_message_length = std::string::traits_type::length(sdl_error_message);
	std::string message{};
	message.reserve(sdl_func_name_length + sdl_error_message_length + 3);
	message += '[';
	message += sdl_func_name;
	message += "] ";
	message += sdl_error_message;
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

} // namespace

std::intptr_t get_working_directory(char* buffer, std::intptr_t buffer_size)
try
{
	if (buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null buffer.");
	}
	if (buffer_size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid buffer size.");
	}
	char* const sdl_directoy = SDL_GetCurrentDirectory();
	if (sdl_directoy == nullptr)
	{
		fail_sdl_func("SDL_GetCurrentDirectory");
	}
	const auto scope_exit = make_scope_exit(
		[sdl_directoy]()
		{
			SDL_free(sdl_directoy);
		});
	const std::size_t sdl_directoy_length = std::string::traits_type::length(sdl_directoy);
	if (static_cast<std::size_t>(buffer_size) < sdl_directoy_length)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
	}
	std::string::traits_type::copy(buffer, sdl_directoy, sdl_directoy_length);
	return sdl_directoy_length;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void create_directories(const char* path)
try
{
	if (!SDL_CreateDirectory(path))
	{
		fail_sdl_func("SDL_CreateDirectory");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void rename(const char* old_path, const char* new_path)
try
{
	if (!SDL_RenamePath(old_path, new_path))
	{
		fail_sdl_func("SDL_RenamePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void remove_if_exists(const char* path)
try
{
	if (!SDL_GetPathInfo(path, nullptr))
	{
		return;
	}
	if (!SDL_RemovePath(path))
	{
		fail_sdl_func("SDL_RemovePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void remove(const char* path)
try
{
	if (!SDL_RemovePath(path))
	{
		fail_sdl_func("SDL_RemovePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool is_directory_exists(const char* path)
try
{
	SDL_PathInfo sdl_path_info;
	if (!SDL_GetPathInfo(path, &sdl_path_info))
	{
		return false;
	}
	return sdl_path_info.type == SDL_PATHTYPE_DIRECTORY;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool is_regular_file_exists(const char* path)
try
{
	SDL_PathInfo sdl_path_info;
	if (!SDL_GetPathInfo(path, &sdl_path_info))
	{
		return false;
	}
	return sdl_path_info.type == SDL_PATHTYPE_FILE;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone::fs
