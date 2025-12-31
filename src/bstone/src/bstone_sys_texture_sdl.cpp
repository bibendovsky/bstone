/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture (SDL)

#include "bstone_sys_texture_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sys_texture_lock_sdl.h"
#include <format>
#include <string>
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

private:
	Logger& logger_;
	SDL_Renderer& sdl_renderer_;
	SDL_Texture* sdl_texture_{};

	void do_set_blend_mode(TextureBlendMode mode) override;
	void do_copy(const Rectangle* texture_rect, const Rectangle* target_rect) override;
	TextureLockUPtr do_make_lock(const Rectangle* rect) override;

	[[noreturn]] static void fail_sdl_func(const char* func_name);
	static SDL_BlendMode map_blend_mode(TextureBlendMode blend_mode);
	static SDL_PixelFormat map_pixel_format(PixelFormat pixel_format);
	static SDL_TextureAccess map_access(TextureAccess texture_access);
};

// ======================================

TextureSdl::TextureSdl(Logger& logger, SDL_Renderer& sdl_renderer, const TextureInitParam& param)
	:
	logger_{logger},
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
		fail_sdl_func("SDL_CreateTexture");
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
		fail_sdl_func("SDL_SetTextureScaleMode");
	}
	sdl_texture_ = sdl_texture;
	sdl_texture = nullptr;
}

TextureSdl::~TextureSdl()
{
	SDL_DestroyTexture(sdl_texture_);
}

void TextureSdl::do_set_blend_mode(TextureBlendMode blend_mode)
{
	const SDL_BlendMode sdl_blend_mode = map_blend_mode(blend_mode);
	if (!SDL_SetTextureBlendMode(sdl_texture_, sdl_blend_mode))
	{
		fail_sdl_func("SDL_SetTextureBlendMode");
	}
}

void TextureSdl::do_copy(const Rectangle* texture_rect, const Rectangle* target_rect)
{
	SDL_FRect sdl_texture_rect;
	const SDL_FRect* sdl_texture_rect_ptr;
	if (texture_rect != nullptr)
	{
		sdl_texture_rect = SDL_FRect{
			.x = static_cast<float>(texture_rect->x),
			.y = static_cast<float>(texture_rect->y),
			.w = static_cast<float>(texture_rect->width),
			.h = static_cast<float>(texture_rect->height),
		};
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
		sdl_target_rect = SDL_FRect{
			.x = static_cast<float>(target_rect->x),
			.y = static_cast<float>(target_rect->y),
			.w = static_cast<float>(target_rect->width),
			.h = static_cast<float>(target_rect->height),
		};
		sdl_target_rect_ptr = &sdl_target_rect;
	}
	else
	{
		sdl_target_rect_ptr = nullptr;
	}
	if (!SDL_RenderTexture(
		&sdl_renderer_,
		sdl_texture_,
		sdl_texture_rect_ptr,
		sdl_target_rect_ptr))
	{
		fail_sdl_func("SDL_RenderTexture");
	}
}

TextureLockUPtr TextureSdl::do_make_lock(const Rectangle* rect)
{
	return make_texture_lock_sdl(*sdl_texture_, rect);
}

[[noreturn]] void TextureSdl::fail_sdl_func(const char* func_name)
{
	const std::string message = std::format("[{}] {}", func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
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
