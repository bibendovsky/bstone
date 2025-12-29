/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Single SDL sub-system.

#include "bstone_sys_sdl_subsystem.h"
#include "bstone_exception.h"
#include <bit>
#include <format>
#include <string>
#include <utility>

namespace bstone::sys {

SdlSubsystem::SdlSubsystem(SDL_InitFlags sdl_init_flags)
{
	if (!std::has_single_bit(sdl_init_flags))
	{
		const std::string message = std::format("[{}] Expected a single flag. (flags={})", __func__, sdl_init_flags);
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	if (!SDL_InitSubSystem(sdl_init_flags))
	{
		const std::string message = std::format("[{}] {} (flags={})", "SDL_InitSubSystem", SDL_GetError(), sdl_init_flags);
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	sdl_init_flags_ = sdl_init_flags;
}

SdlSubsystem::~SdlSubsystem()
{
	SDL_QuitSubSystem(sdl_init_flags_);
}

void SdlSubsystem::swap(SdlSubsystem& sdl_subsystem)
{
	std::swap(sdl_init_flags_, sdl_subsystem.sdl_init_flags_);
}

} // namespace bstone::sys
