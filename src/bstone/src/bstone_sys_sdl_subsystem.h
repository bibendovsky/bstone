/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Single SDL sub-system.

#ifndef BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED
#define BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED

#include "SDL3/SDL.h"

namespace bstone::sys {

class SdlSubsystem
{
public:
	SdlSubsystem() = default;
	explicit SdlSubsystem(SDL_InitFlags sdl_init_flags);
	SdlSubsystem(const SdlSubsystem& rhs) = delete;
	SdlSubsystem& operator=(const SdlSubsystem& rhs) = delete;
	~SdlSubsystem();
	void swap(SdlSubsystem& sdl_subsystem);

private:
	SDL_InitFlags sdl_init_flags_;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED
