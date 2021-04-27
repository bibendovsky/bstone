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


#ifndef BSTONE_SDL_TEXTURE_LOCK_INCLUDED
#define BSTONE_SDL_TEXTURE_LOCK_INCLUDED


#include "SDL_render.h"


namespace bstone
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlTextureLock
{
public:
	explicit SdlTextureLock(
		::SDL_Texture* sdl_texture,
		const ::SDL_Rect* rect = nullptr);

	SdlTextureLock(
		const SdlTextureLock& rhs) = delete;

	SdlTextureLock(
		SdlTextureLock&& rhs) noexcept;

	SdlTextureLock& operator=(
		const SdlTextureLock& rhs) = delete;

	~SdlTextureLock();


	void* get_pixels() const noexcept;

	template<
		typename T
	>
	T get_pixels() const noexcept
	{
		return static_cast<T>(get_pixels());
	}

	int get_pitch() const noexcept;


private:
	::SDL_Texture* sdl_texture_{};
	void* pixels_{};
	int pitch_{};
}; // SdlTextureLock

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // bstone


#endif // !BSTONE_SDL_TEXTURE_LOCK_INCLUDED
