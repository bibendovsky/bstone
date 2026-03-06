/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture (SDL)

#include "bstone_sys_texture_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include "bstone_sys_texture_lock_sdl.h"
#include "SDL3/SDL_render.h"

namespace bstone::sys {

namespace {

class TextureSdl final : public Texture
{
public:
	TextureSdl(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param);
	TextureSdl(const TextureSdl&) = delete;
	TextureSdl& operator=(const TextureSdl&) = delete;
	~TextureSdl() override;

	void set_blend_mode(TextureBlendMode mode) override;
	void copy(const FRect* texture_rect, const FRect* target_rect) override;
	TextureLockUPtr make_lock() override;

private:
	SDL_Renderer& sdl_renderer_;
	SDL_Texture* sdl_texture_{};

	static SDL_BlendMode map_blend_mode(TextureBlendMode blend_mode);
	static SDL_PixelFormat map_pixel_format(PixelFormat pixel_format);
	static SDL_TextureAccess map_access(TextureAccess texture_access);
};

// ======================================

TextureSdl::TextureSdl([[maybe_unused]] Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
	:
	sdl_renderer_{sdl_renderer}
{
	const SDL_PixelFormat sdl_pixel_format = map_pixel_format(param.pixel_format);
	const SDL_TextureAccess sdl_texture_access = map_access(param.access);
	SDL_Texture* sdl_texture = SDL_CreateTexture(
		&sdl_renderer,
		sdl_pixel_format,
		sdl_texture_access,
		param.width,
		param.height);
	if (sdl_texture == nullptr)
	{
		sdl::fail("SDL_CreateTexture");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_texture]()
		{
			if (sdl_texture != nullptr)
			{
				SDL_DestroyTexture(sdl_texture);
			}
		});
	if (!SDL_SetTextureScaleMode(sdl_texture, SDL_SCALEMODE_NEAREST))
	{
		sdl::fail("SDL_SetTextureScaleMode");
	}
	sdl_texture_ = sdl_texture;
	sdl_texture = nullptr;
}

TextureSdl::~TextureSdl()
{
	SDL_DestroyTexture(sdl_texture_);
}

void TextureSdl::set_blend_mode(TextureBlendMode blend_mode)
{
	const SDL_BlendMode sdl_blend_mode = map_blend_mode(blend_mode);
	if (!SDL_SetTextureBlendMode(sdl_texture_, sdl_blend_mode))
	{
		sdl::fail("SDL_SetTextureBlendMode");
	}
}

void TextureSdl::copy(const FRect* texture_rect, const FRect* target_rect)
{
	if (!SDL_RenderTexture(
		&sdl_renderer_,
		sdl_texture_,
		reinterpret_cast<const SDL_FRect*>(texture_rect),
		reinterpret_cast<const SDL_FRect*>(target_rect)))
	{
		sdl::fail("SDL_RenderTexture");
	}
}

TextureLockUPtr TextureSdl::make_lock()
{
	return make_texture_lock_sdl(*sdl_texture_, nullptr);
}

SDL_BlendMode TextureSdl::map_blend_mode(TextureBlendMode blend_mode)
{
	switch (blend_mode)
	{
		case TextureBlendMode::none: return SDL_BLENDMODE_NONE;
		case TextureBlendMode::blend: return SDL_BLENDMODE_BLEND;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown blend mode.");
	}
}

SDL_PixelFormat TextureSdl::map_pixel_format(PixelFormat pixel_format)
{
	switch (pixel_format)
	{
		case PixelFormat::b8g8r8a8: return SDL_PIXELFORMAT_BGRA32;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
}

SDL_TextureAccess TextureSdl::map_access(TextureAccess texture_access)
{
	switch (texture_access)
	{
		case TextureAccess::streaming: return SDL_TEXTUREACCESS_STREAMING;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown access.");
	}
}

} // namespace

// ==========================================================================

TextureUPtr make_texture_sdl(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
{
	return std::make_unique<TextureSdl>(logger, sdl_renderer, param);
}

} // namespace bstone::sys
