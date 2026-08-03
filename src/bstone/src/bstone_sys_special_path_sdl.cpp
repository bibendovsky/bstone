/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Special path primitives (SDL)

#include "bstone_sys_special_path.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include <cstddef>
#include <climits>
#include <algorithm>
#include <string>
#include "SDL3/SDL_filesystem.h"

namespace bstone::sys {

int SpecialPath::get_user_specific_data_path(
	const char* organization_name,
	const char* application_name,
	char* buffer,
	int buffer_size)
{
	char* const sdl_path = SDL_GetPrefPath(organization_name, application_name);
	if (sdl_path == nullptr)
	{
		sdl::fail("SDL_GetPrefPath");
	}
	const auto scope_exit = make_scope_exit(
		[sdl_path]()
		{
			SDL_free(sdl_path);
		});
	const std::size_t path_size = std::string::traits_type::length(sdl_path);
	if (path_size >= static_cast<std::size_t>(buffer_size))
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
	}
	std::copy_n(sdl_path, path_size + 1, buffer);
	return static_cast<int>(path_size);
}

int SpecialPath::get_home_path(char* buffer, int buffer_size) noexcept
{
	if (buffer == nullptr || buffer_size <= 0)
	{
		return 0;
	}
	// Owned by SDL, unlike SDL_GetPrefPath's result, so it must not be freed.
	const char* const sdl_path = SDL_GetUserFolder(SDL_FOLDER_HOME);
	if (sdl_path == nullptr)
	{
		return 0;
	}
	std::size_t path_size = std::string::traits_type::length(sdl_path);
	// SDL guarantees a trailing separator; drop it so the result composes with
	// the path helpers the same way every other path in the code base does.
	while (path_size > 0 && (sdl_path[path_size - 1] == '/' || sdl_path[path_size - 1] == '\\'))
	{
		path_size -= 1;
	}
	if (path_size == 0 || path_size >= static_cast<std::size_t>(buffer_size))
	{
		return 0;
	}
	std::copy_n(sdl_path, path_size, buffer);
	buffer[path_size] = '\0';
	return static_cast<int>(path_size);
}

} // namespace bstone::sys
