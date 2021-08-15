/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// SDL texture lock.
//


#include "bstone_sdl_texture_lock.h"

#include "bstone_exception.h"
#include "bstone_sdl2_exception.h"


namespace bstone
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


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SdlTextureLock::SdlTextureLock(
	::SDL_Texture* sdl_texture,
	const ::SDL_Rect* rect)
try
{
	if (!sdl_texture)
	{
		throw SdlTextureLockException{"Null texture."};
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
	std::throw_with_nested(SdlTextureLockException{"Failed to lock a texture."});
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
