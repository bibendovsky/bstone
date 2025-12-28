/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_sdl3_subsystem.h"

#include "bstone_exception.h"
#include "bstone_sys_exception_sdl3.h"
#include <utility>

namespace bstone {
namespace sys {

Sdl3Subsystem::Sdl3Subsystem() noexcept = default;

Sdl3Subsystem::Sdl3Subsystem(Uint32 sdl_flags)
try
	:
	sdl_flags_{sdl_flags}
{
	switch (sdl_flags_)
	{
		case 0: return;

		case SDL_INIT_AUDIO:
		case SDL_INIT_VIDEO:
		case SDL_INIT_JOYSTICK:
		case SDL_INIT_HAPTIC:
		case SDL_INIT_GAMEPAD:
		case SDL_INIT_EVENTS:
		case SDL_INIT_SENSOR:
			break;

		default: BSTONE_THROW_STATIC_SOURCE("Unknown subsystem.");
	}

	sdl3_ensure_result(SDL_InitSubSystem(sdl_flags));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3Subsystem::Sdl3Subsystem(Sdl3Subsystem&& rhs) noexcept
	:
	sdl_flags_{rhs.sdl_flags_}
{
	rhs.sdl_flags_ = 0;
}

Sdl3Subsystem::~Sdl3Subsystem()
{
	if (sdl_flags_ == 0)
	{
		return;
	}

	SDL_QuitSubSystem(sdl_flags_);
}

void Sdl3Subsystem::swap(Sdl3Subsystem& rhs) noexcept
{
	std::swap(sdl_flags_, rhs.sdl_flags_);
}

} // namespace sys
} // namespace bstone
