/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_SDL3_SUBSYSTEM_INCLUDED
#define BSTONE_SYS_SDL3_SUBSYSTEM_INCLUDED

#include "SDL3/SDL.h"

namespace bstone {
namespace sys {

class Sdl3Subsystem
{
public:
	Sdl3Subsystem() noexcept;
	explicit Sdl3Subsystem(Uint32 sdl_flags);
	Sdl3Subsystem(Sdl3Subsystem&& rhs) noexcept;
	~Sdl3Subsystem();

	void swap(Sdl3Subsystem& rhs) noexcept;

private:
	Uint32 sdl_flags_{};
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL3_SUBSYSTEM_INCLUDED
