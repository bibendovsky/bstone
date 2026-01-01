/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Special path primitives (SDL)

#include "bstone_sys_special_path.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include <algorithm>
#include <format>
#include <string>
#include "SDL3/SDL_filesystem.h"

namespace bstone::sys {

std::intptr_t SpecialPath::get_user_specific_data_path(
	const char* organization_name,
	const char* application_name,
	char* buffer,
	std::intptr_t buffer_size)
{
	char* const sdl_path = SDL_GetPrefPath(organization_name, application_name);
	if (sdl_path == nullptr)
	{
		const std::string message = std::format("[{}] {}", "SDL_GetPrefPath", SDL_GetError());
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	const auto scope_exit = make_scope_exit(
		[sdl_path]()
		{
			SDL_free(sdl_path);
		});
	const std::intptr_t path_size = static_cast<std::intptr_t>(std::string::traits_type::length(sdl_path));
	if (path_size >= buffer_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
	}
	std::copy_n(sdl_path, path_size + 1, buffer);
	return path_size;
}

} // namespace bstone::sys
