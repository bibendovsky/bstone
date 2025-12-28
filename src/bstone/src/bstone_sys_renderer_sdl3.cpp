/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <climits>
#include <vector>
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_detail_sdl3.h"
#include "bstone_sys_exception_sdl3.h"
#include "bstone_sys_texture_sdl3.h"
#include "bstone_sys_renderer_sdl3.h"

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

static_assert(
	sizeof(SDL_Rect) == sizeof(RendererViewport) &&
		offsetof(SDL_Rect, x) == offsetof(RendererViewport, x) &&
		offsetof(SDL_Rect, y) == offsetof(RendererViewport, y) &&
		offsetof(SDL_Rect, w) == offsetof(RendererViewport, width) &&
		offsetof(SDL_Rect, h) == offsetof(RendererViewport, height),
	"Unsupported RendererViewport type.");

// ==========================================================================

struct Sdl3RendererDeleter
{
	void operator()(SDL_Renderer* sdl_renderer)
	{
		SDL_DestroyRenderer(sdl_renderer);
	}
};

using Sdl3RendererUPtr = std::unique_ptr<SDL_Renderer, Sdl3RendererDeleter>;

// ==========================================================================

class Sdl3Renderer final : public Renderer
{
public:
	Sdl3Renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param);
	Sdl3Renderer(const Sdl3Renderer&) = delete;
	Sdl3Renderer& operator=(const Sdl3Renderer&) = delete;
	~Sdl3Renderer() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	using FRectBuffer = std::vector<SDL_FRect>;

	Logger& logger_;
	Sdl3RendererUPtr sdl_renderer_{};
	FRectBuffer frect_buffer_{};

private:
	const char* do_get_name() const override;

	void do_set_viewport(const RendererViewport* viewport) override;

	void do_clear() override;
	void do_set_draw_color(Color color) override;
	void do_fill(std::span<const Rectangle> rects) override;
	void do_present() override;

	void do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch) override;

	TextureUPtr do_make_texture(const TextureInitParam& param) override;

private:
	static MemoryResource& get_memory_resource();

	static SDL_PixelFormat map_pixel_format(PixelFormat pixel_format);

	void log_flag(const char* flag, std::string& message);
	void log_flags(Uint32 flags, std::string& message);
	void log_info();
};

// ==========================================================================

Sdl3Renderer::Sdl3Renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL renderer.");
	SDL_PropertiesID sdl_properties_id = SDL_CreateProperties();
	SDL_SetPointerProperty(sdl_properties_id, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, &sdl_window);
	SDL_SetNumberProperty(sdl_properties_id, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);
	SDL_Renderer* const sdl_renderer = SDL_CreateRendererWithProperties(sdl_properties_id);
	SDL_DestroyProperties(sdl_properties_id);
	sdl_renderer_ = Sdl3RendererUPtr{sdl3_ensure_result(sdl_renderer)};
	log_info();
	logger_.log_information(">>> SDL renderer started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3Renderer::~Sdl3Renderer()
{
	logger_.log_information("<<< Shut down SDL renderer.");
}

void* Sdl3Renderer::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

const char* Sdl3Renderer::do_get_name() const
try {
	return SDL_GetStringProperty(SDL_GetRendererProperties(sdl_renderer_.get()), SDL_PROP_RENDERER_NAME_STRING, "");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::do_set_viewport(const RendererViewport* viewport)
{
	sdl3_ensure_result(SDL_SetRenderViewport(
		sdl_renderer_.get(),
		reinterpret_cast<const SDL_Rect*>(viewport)));
}

void Sdl3Renderer::do_clear()
try {
	sdl3_ensure_result(SDL_RenderClear(sdl_renderer_.get()));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::do_set_draw_color(Color color)
try {
	sdl3_ensure_result(SDL_SetRenderDrawColor(sdl_renderer_.get(), color.r, color.g, color.b, color.a));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::do_fill(std::span<const Rectangle> rects)
try {
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
	sdl3_ensure_result(SDL_RenderFillRects(
		sdl_renderer_.get(),
		frect_buffer_.data(),
		static_cast<int>(frect_buffer_.size())));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::do_present()
try {
	SDL_RenderPresent(sdl_renderer_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch)
try {
	if (pixel_format != PixelFormat::r8g8b8)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported destination pixel format.");
	}
	SDL_Surface* sdl_surface = nullptr;
	const auto scope_exit = make_scope_exit(
		[&sdl_surface]()
		{
			if (sdl_surface != nullptr)
			{
				SDL_DestroySurface(sdl_surface);
			}
		});
	sdl_surface = sdl3_ensure_result(SDL_RenderReadPixels(sdl_renderer_.get(), reinterpret_cast<const SDL_Rect*>(rect)));
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
		sdl3_ensure_result(SDL_LockSurface(sdl_surface));
	}
	sdl3_ensure_result(SDL_ConvertPixels(
		width,
		height,
		sdl_surface->format,
		sdl_surface->pixels,
		sdl_surface->pitch,
		SDL_PIXELFORMAT_RGB24,
		pixels,
		pitch));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureUPtr Sdl3Renderer::do_make_texture(const TextureInitParam& param)
try {
	return make_sdl3_texture(logger_, *sdl_renderer_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& Sdl3Renderer::get_memory_resource()
{
	static SinglePoolResource<Sdl3Renderer> memory_pool{};

	return memory_pool;
}

SDL_PixelFormat Sdl3Renderer::map_pixel_format(PixelFormat pixel_format)
try {
	switch (pixel_format)
	{
		case PixelFormat::r8g8b8: return SDL_PIXELFORMAT_RGB24;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3Renderer::log_flag(const char* name, std::string& message)
{
	message += "    ";
	message += name;
	detail::sdl3_log_eol(message);
}

void Sdl3Renderer::log_flags(Uint32 flags, std::string& message)
{}

void Sdl3Renderer::log_info()
{}

} // namespace

// ==========================================================================

RendererUPtr make_sdl3_renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
try {
	return std::make_unique<Sdl3Renderer>(logger, sdl_window, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
