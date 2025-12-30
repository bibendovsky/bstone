/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 2D renderer (SDL)

#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_string_builder.h"
#include "bstone_sys_texture_sdl.h"
#include "bstone_sys_renderer_sdl.h"
#include <climits>
#include <format>
#include <string>
#include <vector>
#include "SDL3/SDL_rect.h"
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

static_assert(
	sizeof(SDL_Rect) == sizeof(RendererViewport) &&
		offsetof(SDL_Rect, x) == offsetof(RendererViewport, x) &&
		offsetof(SDL_Rect, y) == offsetof(RendererViewport, y) &&
		offsetof(SDL_Rect, w) == offsetof(RendererViewport, width) &&
		offsetof(SDL_Rect, h) == offsetof(RendererViewport, height),
	"Unsupported RendererViewport type.");

// ======================================

class RendererSdl final : public Renderer
{
public:
	RendererSdl(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param);
	RendererSdl(const RendererSdl&) = delete;
	RendererSdl& operator=(const RendererSdl&) = delete;
	~RendererSdl() override;

private:
	using FRectBuffer = std::vector<SDL_FRect>;

	Logger& logger_;
	SDL_Renderer* sdl_renderer_;
	FRectBuffer frect_buffer_{};

	const char* do_get_name() const override;
	void do_set_viewport(const RendererViewport* viewport) override;
	void do_clear() override;
	void do_set_draw_color(Color color) override;
	void do_fill(std::span<const Rectangle> rects) override;
	void do_present() override;
	void do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch) override;
	TextureUPtr do_make_texture(const TextureInitParam& param) override;

	[[noreturn]] static void fail_sdl_func(const char* func_name);
	static SDL_PixelFormat map_pixel_format(PixelFormat pixel_format);
	void log_info(SDL_Renderer* sdl_renderer);
};

// --------------------------------------

RendererSdl::RendererSdl(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
	:
	logger_{logger}
{
	logger_.log_information("Creating SDL renderer.");
	SDL_PropertiesID sdl_properties_id = SDL_CreateProperties();
	SDL_SetPointerProperty(sdl_properties_id, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, &sdl_window);
	SDL_SetNumberProperty(sdl_properties_id, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, param.is_vsync);
	SDL_Renderer* sdl_renderer = SDL_CreateRendererWithProperties(sdl_properties_id);
	SDL_DestroyProperties(sdl_properties_id);
	if (sdl_renderer == nullptr)
	{
		fail_sdl_func("SDL_CreateRendererWithProperties");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_renderer]()
		{
			if (sdl_renderer != nullptr)
			{
				SDL_DestroyRenderer(sdl_renderer);
			}
		});
	log_info(sdl_renderer);
	logger_.log_information("SDL renderer has created.");
	sdl_renderer_ = sdl_renderer;
	sdl_renderer = nullptr;
}

RendererSdl::~RendererSdl()
{
	logger_.log_information("Shut down SDL renderer.");
	SDL_DestroyRenderer(sdl_renderer_);
}

const char* RendererSdl::do_get_name() const
{
	const SDL_PropertiesID sdl_properties_id = SDL_GetRendererProperties(sdl_renderer_);
	if (sdl_properties_id == 0)
	{
		fail_sdl_func("SDL_GetRendererProperties");
	}
	return SDL_GetStringProperty(sdl_properties_id, SDL_PROP_RENDERER_NAME_STRING, "");
}

void RendererSdl::do_set_viewport(const RendererViewport* viewport)
{
	if (!SDL_SetRenderViewport(sdl_renderer_, reinterpret_cast<const SDL_Rect*>(viewport)))
	{
		fail_sdl_func("SDL_SetRenderViewport");
	}
}

void RendererSdl::do_clear()
{
	if (!SDL_RenderClear(sdl_renderer_))
	{
		fail_sdl_func("SDL_RenderClear");
	}
}

void RendererSdl::do_set_draw_color(Color color)
{
	if (!SDL_SetRenderDrawColor(sdl_renderer_, color.r, color.g, color.b, color.a))
	{
		fail_sdl_func("SDL_SetRenderDrawColor");
	}
}

void RendererSdl::do_fill(std::span<const Rectangle> rects)
{
	if (rects.size() > INT_MAX)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many rectangles.");
	}
	frect_buffer_.clear();
	frect_buffer_.reserve(rects.size());
	for (const Rectangle& rect : rects)
	{
		frect_buffer_.emplace_back(SDL_FRect{
			.x = static_cast<float>(rect.x),
			.y = static_cast<float>(rect.y),
			.w = static_cast<float>(rect.width),
			.h = static_cast<float>(rect.height),
		});
	}
	if (!SDL_RenderFillRects(
		sdl_renderer_,
		frect_buffer_.data(),
		static_cast<int>(frect_buffer_.size())))
	{
		fail_sdl_func("SDL_RenderFillRects");
	}
}

void RendererSdl::do_present()
{
	if (!SDL_RenderPresent(sdl_renderer_))
	{
		fail_sdl_func("SDL_RenderPresent");
	}
}

void RendererSdl::do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch)
{
	if (pixel_format != PixelFormat::r8g8b8)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported destination pixel format.");
	}
	SDL_Surface* sdl_surface = SDL_RenderReadPixels(sdl_renderer_, reinterpret_cast<const SDL_Rect*>(rect));
	if (sdl_surface == nullptr)
	{
		fail_sdl_func("SDL_RenderReadPixels");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_surface]()
		{
			SDL_DestroySurface(sdl_surface);
		});
	int width;
	int height;
	if (rect != nullptr)
	{
		width = rect->width;
		height = rect->height;
	}
	else
	{
		width = sdl_surface->w;
		height = sdl_surface->h;
	}
	if (SDL_MUSTLOCK(sdl_surface))
	{
		if (!SDL_LockSurface(sdl_surface))
		{
			fail_sdl_func("SDL_LockSurface");
		}
	}
	if (!SDL_ConvertPixels(
		width,
		height,
		sdl_surface->format,
		sdl_surface->pixels,
		sdl_surface->pitch,
		SDL_PIXELFORMAT_RGB24,
		pixels,
		pitch))
	{
		fail_sdl_func("SDL_ConvertPixels");
	}
}

TextureUPtr RendererSdl::do_make_texture(const TextureInitParam& param)
{
	return make_sdl_texture(logger_, *sdl_renderer_, param);
}

[[noreturn]] void RendererSdl::fail_sdl_func(const char* func_name)
{
	const std::string message = std::format("[{}] {}", func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

SDL_PixelFormat RendererSdl::map_pixel_format(PixelFormat pixel_format)
{
	switch (pixel_format)
	{
		case PixelFormat::r8g8b8: return SDL_PIXELFORMAT_RGB24;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
}

void RendererSdl::log_info(SDL_Renderer* sdl_renderer)
{
	StringBuilder formatter{};
	formatter.reserve(512);
	formatter.reset_indent();
	formatter.add_line("Properties:");
	formatter.increase_indent();
	if (
		const SDL_PropertiesID sdl_properties_id = SDL_GetRendererProperties(sdl_renderer);
		sdl_properties_id != 0)
	{
		const char* const name = SDL_GetStringProperty(sdl_properties_id, SDL_PROP_RENDERER_NAME_STRING, "???");
		formatter.add_indented_line("Name: {}", name);
		const Sint64 vsync = SDL_GetNumberProperty(sdl_properties_id, SDL_PROP_RENDERER_VSYNC_NUMBER, 0);
		formatter.add_indented_line("VSync: {}", vsync);
		const Sint64 max_texture_size = SDL_GetNumberProperty(sdl_properties_id, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
		formatter.add_indented_line("Max texture dimension: {}", max_texture_size);
		SDL_PixelFormat empty_format_list[1] = {SDL_PIXELFORMAT_UNKNOWN};
		const SDL_PixelFormat* formats = static_cast<const SDL_PixelFormat*>(SDL_GetPointerProperty(
			sdl_properties_id,
			SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER,
			empty_format_list));
		formatter.add_indented_line("Texture formats:");
		formatter.increase_indent();
		for (const SDL_PixelFormat* format_ptr = formats; *format_ptr != SDL_PIXELFORMAT_UNKNOWN; ++format_ptr)
		{
			const char* const format_name = SDL_GetPixelFormatName(*format_ptr);
			formatter.add_indented_line(format_name);
		}
	}
	else
	{
		formatter.add_indented_line("ERROR: {}", SDL_GetError());
	}
	logger_.log_information(formatter.get_string().c_str());
}

} // namespace

// ======================================

RendererUPtr make_renderer_sdl(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
{
	return std::make_unique<RendererSdl>(logger, sdl_window, param);
}

} // namespace bstone::sys
