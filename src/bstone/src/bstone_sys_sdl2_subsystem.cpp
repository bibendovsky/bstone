/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_sdl2_subsystem.h"

#include "bstone_exception.h"
#include "bstone_utility.h"
#include "bstone_sys_exception_sdl2.h"

namespace bstone {
namespace sys {

Sdl2Subsystem::Sdl2Subsystem() noexcept = default;

Sdl2Subsystem::Sdl2Subsystem(Uint32 sdl_flags)
try
	:
	sdl_flags_{sdl_flags}
{
	switch (sdl_flags_)
	{
		case 0: return;

		case SDL_INIT_TIMER:
		case SDL_INIT_AUDIO:
		case SDL_INIT_VIDEO:
		case SDL_INIT_JOYSTICK:
		case SDL_INIT_HAPTIC:
		case SDL_INIT_GAMECONTROLLER:
		case SDL_INIT_EVENTS:
		case SDL_INIT_SENSOR:
			break;

		default: BSTONE_THROW_STATIC_SOURCE("Unknown subsystem.");
	}

	sdl2_ensure_result(SDL_InitSubSystem(sdl_flags));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2Subsystem::Sdl2Subsystem(Sdl2Subsystem&& rhs) noexcept
	:
	sdl_flags_{rhs.sdl_flags_}
{
	rhs.sdl_flags_ = 0;
}

Sdl2Subsystem::~Sdl2Subsystem()
{
	if (sdl_flags_ == 0)
	{
		return;
	}

	SDL_QuitSubSystem(sdl_flags_);
}

void Sdl2Subsystem::swap(Sdl2Subsystem& rhs) noexcept
{
	swop(sdl_flags_, rhs.sdl_flags_);
}

} // namespace sys
} // namespace bstone
