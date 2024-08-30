/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <climits>
#include "SDL_rect.h"
#include "SDL_render.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_detail_sdl2.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_texture_sdl2.h"
#include "bstone_sys_renderer_sdl2.h"

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

struct Sdl2RendererDeleter
{
	void operator()(SDL_Renderer* sdl_renderer)
	{
		SDL_DestroyRenderer(sdl_renderer);
	}
};

using Sdl2RendererUPtr = std::unique_ptr<SDL_Renderer, Sdl2RendererDeleter>;

// ==========================================================================

class Sdl2Renderer final : public Renderer
{
public:
	Sdl2Renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param);
	Sdl2Renderer(const Sdl2Renderer&) = delete;
	Sdl2Renderer& operator=(const Sdl2Renderer&) = delete;
	~Sdl2Renderer() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	Sdl2RendererUPtr sdl_renderer_{};

private:
	const char* do_get_name() const override;

	void do_set_viewport(const RendererViewport* viewport) override;

	void do_clear() override;
	void do_set_draw_color(Color color) override;
	void do_fill(Span<const Rectangle> rects) override;
	void do_present() override;

	void do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch) override;

	TextureUPtr do_make_texture(const TextureInitParam& param) override;

private:
	static MemoryResource& get_memory_resource();

	static SDL_PixelFormatEnum map_pixel_format(PixelFormat pixel_format);

	void log_flag(const char* flag, std::string& message);
	void log_flags(Uint32 flags, std::string& message);
	void log_texture_formats(const SDL_RendererInfo& info, std::string& message);
	void log_info();
};

// ==========================================================================

Sdl2Renderer::Sdl2Renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL renderer.");

	const auto sdl_flags = static_cast<Uint32>(
		param.is_vsync ? SDL_RENDERER_PRESENTVSYNC : SDL_RendererFlags{});
	sdl_renderer_ = Sdl2RendererUPtr{sdl2_ensure_result(SDL_CreateRenderer(&sdl_window, -1, sdl_flags))};
	log_info();

	logger_.log_information(">>> SDL renderer started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2Renderer::~Sdl2Renderer()
{
	logger_.log_information("<<< Shut down SDL renderer.");
}

void* Sdl2Renderer::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

const char* Sdl2Renderer::do_get_name() const
try {
	auto sdl_renderer_info = SDL_RendererInfo{};
	sdl2_ensure_result(SDL_GetRendererInfo(sdl_renderer_.get(), &sdl_renderer_info));
	return sdl_renderer_info.name;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::do_set_viewport(const RendererViewport* viewport)
{
	sdl2_ensure_result(SDL_RenderSetViewport(
		sdl_renderer_.get(),
		reinterpret_cast<const SDL_Rect*>(viewport)));
}

void Sdl2Renderer::do_clear()
try {
	sdl2_ensure_result(SDL_RenderClear(sdl_renderer_.get()));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::do_set_draw_color(Color color)
try {
	sdl2_ensure_result(SDL_SetRenderDrawColor(sdl_renderer_.get(), color.r, color.g, color.b, color.a));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::do_fill(Span<const Rectangle> rects)
try {
	if (rects.get_size() > INT_MAX)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many rectangles.");
	}

	sdl2_ensure_result(SDL_RenderFillRects(
		sdl_renderer_.get(),
		reinterpret_cast<const SDL_Rect*>(rects.get_data()),
		static_cast<int>(rects.get_size())));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::do_present()
try {
	SDL_RenderPresent(sdl_renderer_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::do_read_pixels(const Rectangle* rect, PixelFormat pixel_format, void* pixels, int pitch)
try {
	const auto sdl_pixel_format = map_pixel_format(pixel_format);

	sdl2_ensure_result(SDL_RenderReadPixels(
		sdl_renderer_.get(),
		reinterpret_cast<const SDL_Rect*>(rect),
		sdl_pixel_format,
		pixels,
		pitch));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureUPtr Sdl2Renderer::do_make_texture(const TextureInitParam& param)
try {
	return make_sdl2_texture(logger_, *sdl_renderer_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& Sdl2Renderer::get_memory_resource()
{
	static SinglePoolResource<Sdl2Renderer> memory_pool{};

	return memory_pool;
}

SDL_PixelFormatEnum Sdl2Renderer::map_pixel_format(PixelFormat pixel_format)
try {
	switch (pixel_format)
	{
		case PixelFormat::r8g8b8: return SDL_PIXELFORMAT_RGB24;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Renderer::log_flag(const char* name, std::string& message)
{
	message += "    ";
	message += name;
	detail::sdl2_log_eol(message);
}

void Sdl2Renderer::log_flags(Uint32 flags, std::string& message)
{
	message += "  Flags:";
	detail::sdl2_log_eol(message);

	if ((flags & SDL_RENDERER_SOFTWARE) != 0)
	{
		log_flag("software", message);
	}

	if ((flags & SDL_RENDERER_ACCELERATED) != 0)
	{
		log_flag("accelerated", message);
	}

	if ((flags & SDL_RENDERER_PRESENTVSYNC) != 0)
	{
		log_flag("vsync", message);
	}

	if ((flags & SDL_RENDERER_TARGETTEXTURE) != 0)
	{
		log_flag("target texture", message);
	}
}

void Sdl2Renderer::log_texture_formats(const SDL_RendererInfo& info, std::string& message)
{
	if (info.num_texture_formats == 0)
	{
		message += "  No texture formats.";
		detail::sdl2_log_eol(message);
		return;
	}

	message += "  Texture formats:";
	detail::sdl2_log_eol(message);

	for (auto i = decltype(info.num_texture_formats){}; i < info.num_texture_formats; ++i)
	{
		message += "    ";
		detail::sdl2_log_xint(i + 1, message);
		message += ". \"";
		message += SDL_GetPixelFormatName(info.texture_formats[i]);
		message += '"';
		detail::sdl2_log_eol(message);
	}
}

void Sdl2Renderer::log_info()
{
	auto message = std::string{};
	message.reserve(1024);

	auto sdl_info = SDL_RendererInfo{};
	sdl2_ensure_result(SDL_GetRendererInfo(sdl_renderer_.get(), &sdl_info));

	message += "Effective parameters: \"";
	detail::sdl2_log_eol(message);

	message += "  Name: \"";
	message += sdl_info.name;
	message += '"';
	detail::sdl2_log_eol(message);

	message += "  Max texture width: ";
	detail::sdl2_log_xint(sdl_info.max_texture_width, message);
	detail::sdl2_log_eol(message);

	message += "  Max texture height: ";
	detail::sdl2_log_xint(sdl_info.max_texture_height, message);
	detail::sdl2_log_eol(message);

	log_flags(sdl_info.flags, message);
	log_texture_formats(sdl_info, message);

	logger_.log_information(message.c_str());
}

} // namespace

// ==========================================================================

RendererUPtr make_sdl2_renderer(Logger& logger, SDL_Window& sdl_window, const RendererInitParam& param)
try {
	return std::make_unique<Sdl2Renderer>(logger, sdl_window, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
