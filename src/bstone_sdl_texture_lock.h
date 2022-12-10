/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SDL texture lock.

#ifndef BSTONE_SDL_TEXTURE_LOCK_INCLUDED
#define BSTONE_SDL_TEXTURE_LOCK_INCLUDED

#include "SDL_render.h"

namespace bstone {

class SdlTextureLock
{
public:
	explicit SdlTextureLock(SDL_Texture* sdl_texture, const SDL_Rect* rect = nullptr);
	SdlTextureLock(const SdlTextureLock& rhs) = delete;
	SdlTextureLock(SdlTextureLock&& rhs) noexcept;
	SdlTextureLock& operator=(const SdlTextureLock& rhs) = delete;
	~SdlTextureLock();

	void* get_pixels() const noexcept;

	template<typename T>
	T get_pixels() const noexcept
	{
		return static_cast<T>(get_pixels());
	}

	int get_pitch() const noexcept;

private:
	SDL_Texture* sdl_texture_{};
	void* pixels_{};
	int pitch_{};
};

} // namespace bstone

#endif // !BSTONE_SDL_TEXTURE_LOCK_INCLUDED
