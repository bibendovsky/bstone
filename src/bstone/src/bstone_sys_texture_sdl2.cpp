/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_render.h"
#include "SDL_version.h"

#include "bstone_configurations.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_generic_pool_resource.h"

#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_limits_sdl2.h"
#include "bstone_sys_texture_sdl2.h"
#include "bstone_sys_texture_lock_sdl2.h"

namespace bstone {
namespace sys {

namespace {

struct Sdl2TextureDeleter
{
	void operator()(SDL_Texture* sdl_texture)
	{
		SDL_DestroyTexture(sdl_texture);
	}
};

using Sdl2TextureUPtr = std::unique_ptr<SDL_Texture, Sdl2TextureDeleter>;

// ==========================================================================

class Sdl2Texture final : public Texture
{
public:
	Sdl2Texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param);
	Sdl2Texture(const Sdl2Texture&) = delete;
	Sdl2Texture& operator=(const Sdl2Texture&) = delete;
	~Sdl2Texture() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	SDL_Renderer& sdl_renderer_;
	Sdl2TextureUPtr sdl_texture_{};

private:
	void do_set_blend_mode(TextureBlendMode mode) override;
	void do_copy(const Rectangle* texture_rect, const Rectangle* target_rect) override;

	TextureLockUPtr do_make_lock(const Rectangle* rect) override;

private:
	static MemoryResource& get_memory_resource();

	static SDL_BlendMode map_blend_mode(TextureBlendMode blend_mode);
	static SDL_PixelFormatEnum map_pixel_format(PixelFormat pixel_format);
	static SDL_TextureAccess map_access(TextureAccess texture_access);

};

// ==========================================================================

Sdl2Texture::Sdl2Texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
try
	:
	logger_{logger},
	sdl_renderer_{sdl_renderer}
{
	const auto sdl_pixel_format = map_pixel_format(param.pixel_format);
	const auto sdl_access = map_access(param.access);

	sdl_texture_ = Sdl2TextureUPtr{sdl2_ensure_result(SDL_CreateTexture(
		&sdl_renderer,
		sdl_pixel_format,
		sdl_access,
		param.width,
		param.height))};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2Texture::~Sdl2Texture()
{
}

void* Sdl2Texture::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Texture::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void Sdl2Texture::do_set_blend_mode(TextureBlendMode blend_mode)
try {
	const auto sdl_blend_mode = map_blend_mode(blend_mode);
	sdl2_ensure_result(SDL_SetTextureBlendMode(sdl_texture_.get(), sdl_blend_mode));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Texture::do_copy(const Rectangle* texture_rect, const Rectangle* target_rect)
try {
	sdl2_ensure_result(SDL_RenderCopy(
		&sdl_renderer_,
		sdl_texture_.get(),
		reinterpret_cast<const SDL_Rect*>(texture_rect),
		reinterpret_cast<const SDL_Rect*>(target_rect)));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureLockUPtr Sdl2Texture::do_make_lock(const Rectangle* rect)
try {
	return make_sdl2_texture_lock(*sdl_texture_, rect);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& Sdl2Texture::get_memory_resource()
{
	struct Initializer
	{
		Initializer(GenericPoolResource& generic_memory_pool)
		{
			generic_memory_pool.reserve(
				static_cast<std::intptr_t>(sizeof(Sdl2Texture)),
				sys_max_textures,
				get_default_memory_resource());
		}
	};

	static GenericPoolResource generic_memory_pool{};
	static const Initializer initializer{generic_memory_pool};

	return generic_memory_pool;
}

SDL_BlendMode Sdl2Texture::map_blend_mode(TextureBlendMode blend_mode)
try {
	switch (blend_mode)
	{
		case TextureBlendMode::none: return SDL_BLENDMODE_NONE;
		case TextureBlendMode::blend: return SDL_BLENDMODE_BLEND;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown blend mode.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SDL_PixelFormatEnum Sdl2Texture::map_pixel_format(PixelFormat pixel_format)
try {
	switch (pixel_format)
	{
		case PixelFormat::b8g8r8a8: return
#if SDL_VERSION_ATLEAST(2, 0, 5)
			SDL_PIXELFORMAT_BGRA32
#else
	#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
			SDL_PIXELFORMAT_ARGB8888
	#else
			SDL_PIXELFORMAT_BGRA8888
	#endif
#endif
			;

		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SDL_TextureAccess Sdl2Texture::map_access(TextureAccess texture_access)
try {
	switch (texture_access)
	{
		case TextureAccess::streaming: return SDL_TEXTUREACCESS_STREAMING;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown access.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

TextureUPtr make_sdl2_texture(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
try {
	return std::make_unique<Sdl2Texture>(logger, sdl_renderer, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
