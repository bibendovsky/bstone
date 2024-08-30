/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <algorithm>
#include <memory>

#include <SDL_filesystem.h>
#include <SDL_stdinc.h>

#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_special_path.h"

namespace bstone {
namespace sys {

namespace {

struct Sdl2PrefPathDeleter
{
	void operator()(void* ptr) const noexcept
	{
		SDL_free(ptr);
	}
};

} // namespace

std::intptr_t SpecialPath::get_user_specific_data_path(
	const char* organization_name,
	const char* application_name,
	char* buffer,
	std::intptr_t buffer_size)
{
	using SdlPrefPathUPtr = std::unique_ptr<char[], Sdl2PrefPathDeleter>;
	const auto sdl_path = SdlPrefPathUPtr{sdl2_ensure_result(SDL_GetPrefPath(organization_name, application_name))};
	const auto path_size = char_traits::get_size(sdl_path.get());

	if (path_size >= buffer_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
	}

	std::copy_n(sdl_path.get(), path_size + 1, buffer);
	return path_size;
}

} // namespace sys
} // namespace bstone
