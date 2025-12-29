/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL3/SDL_render.h"
#include "bstone_configurations.h"
#include "bstone_exception.h"
#include "bstone_generic_pool_resource.h"
#include "bstone_sys_exception_sdl.h"
#include "bstone_sys_texture_lock_sdl.h"

namespace bstone {
namespace sys {

namespace {

static_assert(
	sizeof(SDL_Rect) == sizeof(Rectangle) &&
		offsetof(SDL_Rect, x) == offsetof(Rectangle, x) &&
		offsetof(SDL_Rect, y) == offsetof(Rectangle, y) &&
		offsetof(SDL_Rect, w) == offsetof(Rectangle, width) &&
		offsetof(SDL_Rect, h) == offsetof(Rectangle, height),
	"Unsupported Rectangle type.");

// ==========================================================================

class SdlTextureLock final : public TextureLock
{
public:
	SdlTextureLock(SDL_Texture& sdl_texture, const Rectangle* rect);
	SdlTextureLock(const SdlTextureLock& rhs) = delete;
	SdlTextureLock& operator=(const SdlTextureLock& rhs) = delete;
	~SdlTextureLock() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	SDL_Texture& sdl_texture_;
	void* pixels_{};
	int pitch_{};

private:
	void* do_get_pixels() const noexcept override;
	int do_get_pitch() const noexcept override;

private:
	static MemoryResource& get_memory_resource();
};

// ==========================================================================

SdlTextureLock::SdlTextureLock(SDL_Texture& sdl_texture, const Rectangle* rect)
try
	:
	sdl_texture_{sdl_texture}
{
	sdl_ensure_result(SDL_LockTexture(
		&sdl_texture,
		reinterpret_cast<const SDL_Rect*>(rect),
		&pixels_,
		&pitch_));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlTextureLock::~SdlTextureLock()
{
	SDL_UnlockTexture(&sdl_texture_);
}

void* SdlTextureLock::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlTextureLock::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void* SdlTextureLock::do_get_pixels() const noexcept
{
	return pixels_;
}

int SdlTextureLock::do_get_pitch() const noexcept
{
	return pitch_;
}

MemoryResource& SdlTextureLock::get_memory_resource()
{
	struct Initializer
	{
		Initializer(GenericPoolResource& generic_memory_pool)
		{
			generic_memory_pool.reserve(
				static_cast<std::intptr_t>(sizeof(SdlTextureLock)),
				sys_max_texture_locks,
				get_default_memory_resource());
		}
	};

	static GenericPoolResource generic_memory_pool{};
	static Initializer initializer{generic_memory_pool};

	return generic_memory_pool;
}

} // namespace

// ==========================================================================

TextureLockUPtr make_sdl_texture_lock(SDL_Texture& sdl_texture, const Rectangle* rect)
{
	return std::make_unique<SdlTextureLock>(sdl_texture, rect);
}

} // namespace sys
} // namespace bstone
