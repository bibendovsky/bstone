/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SDL texture lock.

#include <utility>
#include "bstone_exception.h"
#include "bstone_sdl_exception.h"
#include "bstone_sdl_texture_lock.h"

namespace bstone {

namespace {

class SdlTextureLockException : public Exception
{
public:
	explicit SdlTextureLockException(const char* message) noexcept
		:
		Exception{"SDL_TEXTURE_LOCK", message}
	{}
};

[[noreturn]] void fail(const char* message)
{
	throw SdlTextureLockException{message};
}

[[noreturn]] void fail_nested(const char* message)
{
	std::throw_with_nested(SdlTextureLockException{message});
}

} // namespace

// ==========================================================================

SdlTextureLock::SdlTextureLock(SDL_Texture* sdl_texture, const SDL_Rect* rect)
try
{
	if (sdl_texture == nullptr)
	{
		fail("Null texture.");
	}

	bstone::sdl_ensure_result(SDL_LockTexture(sdl_texture, rect, &pixels_, &pitch_));
	sdl_texture_ = sdl_texture;
}
catch (...)
{
	fail_nested(__func__);
}

SdlTextureLock::SdlTextureLock(SdlTextureLock&& rhs) noexcept
{
	std::swap(sdl_texture_, rhs.sdl_texture_);
}

SdlTextureLock::~SdlTextureLock()
{
	SDL_UnlockTexture(sdl_texture_);
}

void* SdlTextureLock::get_pixels() const noexcept
{
	return pixels_;
}

int SdlTextureLock::get_pitch() const noexcept
{
	return pitch_;
}

} // namespace bstone
