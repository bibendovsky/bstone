/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED
#define BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED

#include "SDL3/SDL.h"

namespace bstone {
namespace sys {

class SdlSubsystem
{
public:
	SdlSubsystem() noexcept;
	explicit SdlSubsystem(Uint32 sdl_flags);
	SdlSubsystem(SdlSubsystem&& rhs) noexcept;
	~SdlSubsystem();

	void swap(SdlSubsystem& rhs) noexcept;

private:
	Uint32 sdl_flags_{};
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_SUBSYSTEM_INCLUDED
