/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_render.h"
#include "bstone_exception.h"
#include "bstone_nhsz_memory_pool.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_limits.h"
#include "bstone_sys_sdl_texture.h"
#include "bstone_sys_sdl_texture_lock.h"

namespace bstone {
namespace sys {

namespace {

struct SdlTextureDeleter
{
	void operator()(SDL_Texture* sdl_texture)
	{
		SDL_DestroyTexture(sdl_texture);
	}
};

using SdlTextureUPtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

// ==========================================================================

class SdlTexture final : public Texture
{
public:
	SdlTexture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param);
	SdlTexture(const SdlTexture&) = delete;
	SdlTexture& operator=(const SdlTexture&) = delete;
	~SdlTexture() override;

	static void* operator new(std::size_t count);
	static void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;
	SDL_Renderer& sdl_renderer_;
	SdlTextureUPtr sdl_texture_{};

private:
	void do_set_blend_mode(TextureBlendMode mode) override;
	void do_copy(const R2Rect* texture_rect, const R2Rect* target_rect) override;

	TextureLockUPtr do_make_lock(const R2Rect* rect) override;

private:
	static SDL_BlendMode map_blend_mode(TextureBlendMode blend_mode);
	static SDL_PixelFormatEnum map_pixel_format(PixelFormat pixel_format);
	static SDL_TextureAccess map_access(TextureAccess texture_access);

};

// ==========================================================================

using SdlTexturePool = NhszMemoryPool<SdlTexture, limits::max_textures>;

SdlTexturePool sdl_texture_pool{};

// ==========================================================================

SdlTexture::SdlTexture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
try
	:
	logger_{logger},
	sdl_renderer_{sdl_renderer}
{
	const auto sdl_pixel_format = map_pixel_format(param.pixel_format);
	const auto sdl_access = map_access(param.access);

	sdl_texture_ = SdlTextureUPtr{sdl_ensure_result(SDL_CreateTexture(
		&sdl_renderer,
		sdl_pixel_format,
		sdl_access,
		param.width,
		param.height))};
}
BSTONE_STATIC_THROW_NESTED_FUNC

SdlTexture::~SdlTexture()
{
}

void* SdlTexture::operator new(std::size_t count)
try
{
	return sdl_texture_pool.allocate(count);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void SdlTexture::operator delete(void* ptr) noexcept
{
	sdl_texture_pool.deallocate(ptr);
}

void SdlTexture::do_set_blend_mode(TextureBlendMode blend_mode)
try
{
	const auto sdl_blend_mode = map_blend_mode(blend_mode);
	sdl_ensure_result(SDL_SetTextureBlendMode(sdl_texture_.get(), sdl_blend_mode));
}
BSTONE_STATIC_THROW_NESTED_FUNC

void SdlTexture::do_copy(const R2Rect* texture_rect, const R2Rect* target_rect)
try
{
	sdl_ensure_result(SDL_RenderCopy(
		&sdl_renderer_,
		sdl_texture_.get(),
		reinterpret_cast<const SDL_Rect*>(texture_rect),
		reinterpret_cast<const SDL_Rect*>(target_rect)));
}
BSTONE_STATIC_THROW_NESTED_FUNC

TextureLockUPtr SdlTexture::do_make_lock(const R2Rect* rect)
try
{
	return make_sdl_texture_lock(*sdl_texture_, rect);
}
BSTONE_STATIC_THROW_NESTED_FUNC

SDL_BlendMode SdlTexture::map_blend_mode(TextureBlendMode blend_mode)
try
{
	switch (blend_mode)
	{
		case TextureBlendMode::none: return SDL_BLENDMODE_NONE;
		case TextureBlendMode::blend: return SDL_BLENDMODE_BLEND;
		default: BSTONE_STATIC_THROW("Unknown blend mode.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

SDL_PixelFormatEnum SdlTexture::map_pixel_format(PixelFormat pixel_format)
try
{
	switch (pixel_format)
	{
		case PixelFormat::a8r8g8b8: return SDL_PIXELFORMAT_ARGB8888;
		default: BSTONE_STATIC_THROW("Unknown pixel format.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

SDL_TextureAccess SdlTexture::map_access(TextureAccess texture_access)
try
{
	switch (texture_access)
	{
		case TextureAccess::streaming: return SDL_TEXTUREACCESS_STREAMING;
		default: BSTONE_STATIC_THROW("Unknown access.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace

// ==========================================================================

TextureUPtr make_sdl_texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
try
{
	return std::make_unique<SdlTexture>(logger, sdl_renderer, param);
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
