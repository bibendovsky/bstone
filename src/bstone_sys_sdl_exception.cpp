/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_error.h"
#include "bstone_sys_sdl_exception.h"

namespace bstone {
namespace sys {

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

[[noreturn]] void sdl_fail()
{
	throw SdlException{};
}

int sdl_ensure_result(int sdl_result)
try
{
	if (sdl_result != 0)
	{
		sdl_fail();
	}

	return sdl_result;
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
