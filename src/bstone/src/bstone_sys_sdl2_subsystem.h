/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_SDL2_SUBSYSTEM_INCLUDED
#define BSTONE_SYS_SDL2_SUBSYSTEM_INCLUDED

#include "SDL.h"

namespace bstone {
namespace sys {

class Sdl2Subsystem
{
public:
	Sdl2Subsystem() noexcept;
	explicit Sdl2Subsystem(Uint32 sdl_flags);
	Sdl2Subsystem(Sdl2Subsystem&& rhs) noexcept;
	~Sdl2Subsystem();

	void swap(Sdl2Subsystem& rhs) noexcept;

private:
	Uint32 sdl_flags_{};
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL2_SUBSYSTEM_INCLUDED
