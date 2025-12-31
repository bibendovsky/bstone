/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_version.h"

#include "bstone_configurations.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_generic_pool_resource.h"

#include "bstone_sys_exception_sdl.h"
#include "bstone_sys_limits_sdl.h"
#include "bstone_sys_texture_sdl.h"
#include "bstone_sys_texture_lock_sdl.h"

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

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	SDL_Renderer& sdl_renderer_;
	SdlTextureUPtr sdl_texture_{};

private:
	void do_set_blend_mode(TextureBlendMode mode) override;
	void do_copy(const Rectangle* texture_rect, const Rectangle* target_rect) override;

	TextureLockUPtr do_make_lock(const Rectangle* rect) override;

private:
	static MemoryResource& get_memory_resource();

	static SDL_BlendMode map_blend_mode(TextureBlendMode blend_mode);
	static SDL_PixelFormat map_pixel_format(PixelFormat pixel_format);
	static SDL_TextureAccess map_access(TextureAccess texture_access);

};

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlTexture::~SdlTexture()
{
}

void* SdlTexture::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlTexture::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void SdlTexture::do_set_blend_mode(TextureBlendMode blend_mode)
try {
	const auto sdl_blend_mode = map_blend_mode(blend_mode);
	sdl_ensure_result(SDL_SetTextureBlendMode(sdl_texture_.get(), sdl_blend_mode));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlTexture::do_copy(const Rectangle* texture_rect, const Rectangle* target_rect)
try {
	SDL_FRect sdl_texture_rect;
	const SDL_FRect* sdl_texture_rect_ptr;
	if (texture_rect != nullptr)
	{
		sdl_texture_rect.x = static_cast<float>(texture_rect->x);
		sdl_texture_rect.y = static_cast<float>(texture_rect->y);
		sdl_texture_rect.w = static_cast<float>(texture_rect->width);
		sdl_texture_rect.h = static_cast<float>(texture_rect->height);
		sdl_texture_rect_ptr = &sdl_texture_rect;
	}
	else
	{
		sdl_texture_rect_ptr = nullptr;
	}
	SDL_FRect sdl_target_rect;
	const SDL_FRect* sdl_target_rect_ptr;
	if (target_rect != nullptr)
	{
		sdl_target_rect.x = static_cast<float>(target_rect->x);
		sdl_target_rect.y = static_cast<float>(target_rect->y);
		sdl_target_rect.w = static_cast<float>(target_rect->width);
		sdl_target_rect.h = static_cast<float>(target_rect->height);
		sdl_target_rect_ptr = &sdl_target_rect;
	}
	else
	{
		sdl_target_rect_ptr = nullptr;
	}
	sdl_ensure_result(SDL_RenderTexture(
		&sdl_renderer_,
		sdl_texture_.get(),
		sdl_texture_rect_ptr,
		sdl_target_rect_ptr));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureLockUPtr SdlTexture::do_make_lock(const Rectangle* rect)
try {
	return make_texture_lock_sdl(*sdl_texture_, rect);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& SdlTexture::get_memory_resource()
{
	struct Initializer
	{
		Initializer(GenericPoolResource& generic_memory_pool)
		{
			generic_memory_pool.reserve(
				static_cast<std::intptr_t>(sizeof(SdlTexture)),
				sys_max_textures,
				get_default_memory_resource());
		}
	};

	static GenericPoolResource generic_memory_pool{};
	static const Initializer initializer{generic_memory_pool};

	return generic_memory_pool;
}

SDL_BlendMode SdlTexture::map_blend_mode(TextureBlendMode blend_mode)
try {
	switch (blend_mode)
	{
		case TextureBlendMode::none: return SDL_BLENDMODE_NONE;
		case TextureBlendMode::blend: return SDL_BLENDMODE_BLEND;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown blend mode.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SDL_PixelFormat SdlTexture::map_pixel_format(PixelFormat pixel_format)
try {
	switch (pixel_format)
	{
		case PixelFormat::b8g8r8a8: return SDL_PIXELFORMAT_BGRA32;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SDL_TextureAccess SdlTexture::map_access(TextureAccess texture_access)
try {
	switch (texture_access)
	{
		case TextureAccess::streaming: return SDL_TEXTUREACCESS_STREAMING;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown access.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

TextureUPtr make_sdl_texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
try {
	return std::make_unique<SdlTexture>(logger, sdl_renderer, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
