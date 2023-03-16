/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_render.h"
#include "bstone_exception.h"
#include "bstone_memory_pool_1x.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_texture_lock.h"

namespace bstone {
namespace sys {

namespace {

static_assert(
	sizeof(SDL_Rect) == sizeof(R2Rect) &&
		offsetof(SDL_Rect, x) == offsetof(R2Rect, x) &&
		offsetof(SDL_Rect, y) == offsetof(R2Rect, y) &&
		offsetof(SDL_Rect, w) == offsetof(R2Rect, width) &&
		offsetof(SDL_Rect, h) == offsetof(R2Rect, height),
	"Unsupported R2Rect type.");

// ==========================================================================

class SdlTextureLock final : public TextureLock
{
public:
	SdlTextureLock(SDL_Texture& sdl_texture, const R2Rect* rect);
	SdlTextureLock(const SdlTextureLock& rhs) = delete;
	SdlTextureLock& operator=(const SdlTextureLock& rhs) = delete;
	~SdlTextureLock() override;

	static void* operator new(std::size_t count);
	static void operator delete(void* ptr) noexcept;

private:
	SDL_Texture& sdl_texture_;
	void* pixels_{};
	int pitch_{};

private:
	void* do_get_pixels() const noexcept override;
	int do_get_pitch() const noexcept override;
};

// ==========================================================================

using SdlTextureLockPool = MemoryPool1XT<SdlTextureLock>;

SdlTextureLockPool sdl_texture_lock_pool{};

// ==========================================================================

SdlTextureLock::SdlTextureLock(SDL_Texture& sdl_texture, const R2Rect* rect)
try
	:
	sdl_texture_{sdl_texture}
{
	sdl_ensure_result(SDL_LockTexture(
		&sdl_texture,
		reinterpret_cast<const SDL_Rect*>(rect),
		&pixels_,
		&pitch_));
}
BSTONE_STATIC_THROW_NESTED_FUNC

SdlTextureLock::~SdlTextureLock()
{
	SDL_UnlockTexture(&sdl_texture_);
}

void* SdlTextureLock::operator new(std::size_t count)
try
{
	return sdl_texture_lock_pool.allocate(count);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void SdlTextureLock::operator delete(void* ptr) noexcept
{
	sdl_texture_lock_pool.deallocate(ptr);
}

void* SdlTextureLock::do_get_pixels() const noexcept
{
	return pixels_;
}

int SdlTextureLock::do_get_pitch() const noexcept
{
	return pitch_;
}

} // namespace

// ==========================================================================

TextureLockUPtr make_sdl_texture_lock(SDL_Texture& sdl_texture, const R2Rect* rect)
{
	return std::make_unique<SdlTextureLock>(sdl_texture, rect);
}

} // namespace sys
} // namespace bstone
