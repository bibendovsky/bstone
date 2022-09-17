/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// SDL texture lock.
//


#include "bstone_sdl_texture_lock.h"

#include "bstone_exception.h"
#include "bstone_sdl_exception.h"


namespace bstone
{


namespace
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlTextureLockException :
	public Exception
{
public:
	explicit SdlTextureLockException(
		const char* message) noexcept
		:
		Exception{"SDL_TEXTURE_LOCK", message}
	{
	}
}; // SdlTextureLockException

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


[[noreturn]]
void fail(
	const char* message)
{
	throw SdlTextureLockException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(SdlTextureLockException{message});
}


} // namespace


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SdlTextureLock::SdlTextureLock(
	::SDL_Texture* sdl_texture,
	const ::SDL_Rect* rect)
try
{
	if (!sdl_texture)
	{
		fail("Null texture.");
	}

	bstone::ensure_sdl_result(::SDL_LockTexture(
		sdl_texture,
		rect,
		&pixels_,
		&pitch_
	));

	sdl_texture_ = sdl_texture;
}
catch (...)
{
	fail_nested("Failed to lock a texture.");
}

SdlTextureLock::SdlTextureLock(
	SdlTextureLock&& rhs) noexcept
	:
	sdl_texture_{rhs.sdl_texture_}
{
	rhs.sdl_texture_ = nullptr;
}

SdlTextureLock::~SdlTextureLock()
{
	::SDL_UnlockTexture(sdl_texture_);
}

void* SdlTextureLock::get_pixels() const noexcept
{
	return pixels_;
}

int SdlTextureLock::get_pitch() const noexcept
{
	return pitch_;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // bstone
