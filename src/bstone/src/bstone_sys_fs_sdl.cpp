/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system management (SDL)

#include "bstone_sys_fs.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include <algorithm>
#include "SDL3/SDL_filesystem.h"

namespace bstone::sys {

int get_working_directory(char* buffer, int buffer_size)
try
{
	char* const sdl_directoy = SDL_GetCurrentDirectory();
	if (sdl_directoy == nullptr)
	{
		sdl::fail("SDL_GetCurrentDirectory");
	}
	const auto scope_exit = make_scope_exit(
		[sdl_directoy]()
		{
			SDL_free(sdl_directoy);
		});
	const std::size_t sdl_directoy_length = SDL_strlen(sdl_directoy);
	if (static_cast<std::size_t>(buffer_size) < sdl_directoy_length)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
	}
	std::copy_n(sdl_directoy, sdl_directoy_length, buffer);
	return static_cast<int>(sdl_directoy_length);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void create_directories(const char* path)
try
{
	if (!SDL_CreateDirectory(path))
	{
		sdl::fail("SDL_CreateDirectory");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void rename_path(const char* old_path, const char* new_path)
try
{
	if (!SDL_RenamePath(old_path, new_path))
	{
		sdl::fail("SDL_RenamePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void remove_path_if_exists(const char* path)
try
{
	if (!SDL_GetPathInfo(path, nullptr))
	{
		return;
	}
	if (!SDL_RemovePath(path))
	{
		sdl::fail("SDL_RemovePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void remove_path(const char* path)
try
{
	if (!SDL_RemovePath(path))
	{
		sdl::fail("SDL_RemovePath");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool is_directory_exists(const char* path)
{
	if (FileType file_type;
		get_file_type(path, file_type))
		return file_type == FileType::directory;
	return false;
}

bool is_regular_file_exists(const char* path)
{
	if (FileType file_type;
		get_file_type(path, file_type))
		return file_type == FileType::file;
	return false;
}

bool get_file_type(const char* path, FileType& file_type)
{
	file_type = FileType::none;
	if (SDL_PathInfo sdl_path_info;
		SDL_GetPathInfo(path, &sdl_path_info))
	{
		switch(sdl_path_info.type)
		{
			case SDL_PATHTYPE_NONE:
				file_type = FileType::none;
				break;
			case SDL_PATHTYPE_FILE:
				file_type = FileType::file;
				break;
			case SDL_PATHTYPE_DIRECTORY:
				file_type = FileType::directory;
				break;
			case SDL_PATHTYPE_OTHER:
				file_type = FileType::other;
				break;
			default:
				BSTONE_ASSERT(false && "Unknown path type.");
				return false;
		}
		return true;
	}
	return false;
}

// =====================================

namespace {

struct EnumDirCallbackPayload
{
	void* user_data;
	EnumDirCallback callback;
};

SDL_EnumerationResult SDLCALL sdl_enum_dir_callback(void* userdata, const char* dirname, const char* fname)
{
	BSTONE_ASSERT(userdata != nullptr);
	const EnumDirCallbackPayload* payload = static_cast<const EnumDirCallbackPayload*>(userdata);
	const EnumDirCallbackResult result = payload->callback(payload->user_data, dirname, fname);
	switch (result)
	{
		case EnumDirCallbackResult::resume:
			return SDL_ENUM_CONTINUE;
		case EnumDirCallbackResult::success:
			return SDL_ENUM_SUCCESS;
		case EnumDirCallbackResult::failure:
			return SDL_ENUM_FAILURE;
		default:
			BSTONE_ASSERT(false && "Unknown result.");
			return SDL_ENUM_FAILURE;
	}
}

} // namespace

bool enumerate_directory(const char* path, EnumDirCallback callback, void* user_data)
{
	EnumDirCallbackPayload payload{
		.user_data = user_data,
		.callback = callback};
	return SDL_EnumerateDirectory(path, sdl_enum_dir_callback, &payload);
}

} // namespace bstone::sys
