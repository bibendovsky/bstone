/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture lock (SDL)

#include "bstone_sys_texture_lock_sdl.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include <format>
#include <memory>
#include <string>
#include "SDL3/SDL_render.h"

namespace bstone::sys {

namespace {

static_assert(
	sizeof(SDL_Rect) == sizeof(Rectangle) &&
		offsetof(SDL_Rect, x) == offsetof(Rectangle, x) &&
		offsetof(SDL_Rect, y) == offsetof(Rectangle, y) &&
		offsetof(SDL_Rect, w) == offsetof(Rectangle, width) &&
		offsetof(SDL_Rect, h) == offsetof(Rectangle, height),
	"Unsupported Rectangle type.");

// ======================================

class TextureLockSdl final : public TextureLock
{
public:
	TextureLockSdl(SDL_Texture& sdl_texture, const Rectangle* rect);
	TextureLockSdl(const TextureLockSdl& rhs) = delete;
	TextureLockSdl& operator=(const TextureLockSdl& rhs) = delete;
	~TextureLockSdl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	class Storage;

	SDL_Texture& sdl_texture_;
	void* pixels_{};
	int pitch_{};

	void* do_get_pixels() const noexcept override;
	int do_get_pitch() const noexcept override;
};

// ======================================

class TextureLockSdl::Storage
{
public:
	Storage() {};
	void* allocate(std::size_t size);
	void deallocate(void* pointer);
	static Storage& get_singleton();
private:
	constexpr static std::size_t storage_size = sizeof(TextureLockSdl);
	bool is_allocated_{};
	std::byte storage_[storage_size];
};

// ======================================

TextureLockSdl::TextureLockSdl(SDL_Texture& sdl_texture, const Rectangle* rect)
	:
	sdl_texture_{sdl_texture}
{
	if (!SDL_LockTexture(
		&sdl_texture,
		reinterpret_cast<const SDL_Rect*>(rect),
		&pixels_,
		&pitch_))
	{
		const std::string message = std::format("[{}] {}", "SDL_LockTexture", SDL_GetError());
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
}

TextureLockSdl::~TextureLockSdl()
{
	SDL_UnlockTexture(&sdl_texture_);
}

void* TextureLockSdl::operator new(std::size_t size)
{
	return Storage::get_singleton().allocate(size);
}

void TextureLockSdl::operator delete(void* ptr)
{
	Storage::get_singleton().deallocate(ptr);
}

void* TextureLockSdl::do_get_pixels() const noexcept
{
	return pixels_;
}

int TextureLockSdl::do_get_pitch() const noexcept
{
	return pitch_;
}

// ======================================

void* TextureLockSdl::Storage::allocate(std::size_t size)
{
	BSTONE_ASSERT(size == storage_size);
	if (is_allocated_)
	{
		BSTONE_THROW_STATIC_SOURCE("[TextureLockSdl] Already allocated.");
	}
	is_allocated_ = true;
	return storage_;
}

void TextureLockSdl::Storage::deallocate(void* pointer)
{
	if (pointer != nullptr)
	{
		BSTONE_ASSERT(pointer == storage_);
		is_allocated_ = false;
	}
}

TextureLockSdl::Storage& TextureLockSdl::Storage::get_singleton()
{
	constinit static std::unique_ptr<Storage> storage{};
	if (storage == nullptr)
	{
		storage = std::make_unique<Storage>();
	}
	return *storage;
}

} // namespace

// ======================================

TextureLockUPtr make_texture_lock_sdl(SDL_Texture& sdl_texture, const Rectangle* rect)
{
	return std::make_unique<TextureLockSdl>(sdl_texture, rect);
}

} // namespace bstone::sys
