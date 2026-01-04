/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Image decoder (SDL)

#include "bstone_exception.h"
#include "bstone_image_decoder.h"
#include "bstone_scope_exit.h"
#include <algorithm>
#include <format>
#include <string>
#include "SDL3/SDL_surface.h"

namespace bstone {

namespace {

class ImageDecoderSdl final : public ImageDecoder
{
public:
	explicit ImageDecoderSdl(ImageDecoderType image_decoder_type);
	~ImageDecoderSdl() override = default;

	void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) override;

private:
	using SdlLoadFunc = SDL_Surface* (SDLCALL *)(SDL_IOStream* src, bool closeio);

	SdlLoadFunc sdl_load_func_;

	[[noreturn]] static void fail_sdl_func(const char* sdl_func_name);
};

// --------------------------------------

ImageDecoderSdl::ImageDecoderSdl(ImageDecoderType image_decoder_type)
{
	switch (image_decoder_type)
	{
		case ImageDecoderType::bmp:
			sdl_load_func_ = SDL_LoadBMP_IO;
			break;
		case ImageDecoderType::png:
			sdl_load_func_ = SDL_LoadPNG_IO;
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported image decoder type.");
	}
}

void ImageDecoderSdl::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
{
	SDL_IOStream* sdl_io_stream = nullptr;
	SDL_Surface* sdl_surface = nullptr;
	SDL_Surface* sdl_surface2 = nullptr;
	const auto scope_exit = make_scope_exit(
		[&sdl_io_stream, &sdl_surface, &sdl_surface2]()
		{
			SDL_CloseIO(sdl_io_stream);
			SDL_DestroySurface(sdl_surface);
			SDL_DestroySurface(sdl_surface2);
		});
	sdl_io_stream = SDL_IOFromConstMem(src_data, static_cast<std::size_t>(src_data_size));
	if (sdl_io_stream == nullptr)
	{
		fail_sdl_func("SDL_IOFromConstMem");
	}
	sdl_surface = sdl_load_func_(sdl_io_stream, false);
	if (sdl_surface == nullptr)
	{
		fail_sdl_func("SDL_SDL_LoadXXX_IO");
	}
	const int dst_pitch = sdl_surface->w * 4;
	const std::size_t area_size_t = static_cast<std::size_t>(sdl_surface->w * sdl_surface->h);
	if (dst_buffer.size() < area_size_t)
	{
		dst_buffer.clear();
		dst_buffer.resize(area_size_t);
	}
	if (SDL_GetSurfacePalette(sdl_surface) != nullptr)
	{
		sdl_surface2 = SDL_ConvertSurface(sdl_surface, SDL_PIXELFORMAT_RGBA32);
		if (sdl_surface2 == nullptr)
		{
			fail_sdl_func("SDL_ConvertSurface");
		}
		if (sdl_surface2->pitch != sdl_surface2->w * 4)
		{
			BSTONE_THROW_STATIC_SOURCE("Unsupported SDL surface pitch.");
		}
		std::copy_n(static_cast<const Rgba8*>(sdl_surface2->pixels), area_size_t, dst_buffer.begin());
	}
	else
	{
		if (!SDL_ConvertPixels(
			sdl_surface->w,
			sdl_surface->h,
			sdl_surface->format,
			sdl_surface->pixels,
			sdl_surface->pitch,
			SDL_PIXELFORMAT_RGBA32,
			dst_buffer.data(),
			dst_pitch))
		{
			fail_sdl_func("SDL_ConvertPixels");
		}
	}
	dst_width = sdl_surface->w;
	dst_height = sdl_surface->h;
}

[[noreturn]] void ImageDecoderSdl::fail_sdl_func(const char* sdl_func_name)
{
	const std::string message = std::format("[{}] {}", sdl_func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

} // namespace

// ======================================

ImageDecodeUPtr make_image_decoder(ImageDecoderType image_decoder_type)
{
	return std::make_unique<ImageDecoderSdl>(image_decoder_type);
}

} // namespace bstone
