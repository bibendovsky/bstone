/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Ensures SDL result.

#include "SDL_error.h"
#include "bstone_exception.h"
#include "bstone_sdl_exception.h"

namespace bstone {

namespace {

class SdlException : public Exception
{
public:
	explicit SdlException() noexcept
		:
		Exception{"SDL", SDL_GetError()}
	{}
};

} // namespace

// ==========================================================================

[[noreturn]] void fail_sdl()
{
	throw SdlException{};
}

void sdl_ensure_result(int sdl_result)
{
	if (sdl_result != 0)
	{
		fail_sdl();
	}
}

} // namespace bstone
