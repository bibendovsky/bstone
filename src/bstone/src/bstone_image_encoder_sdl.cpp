/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Image encoder (SDL)

#include "bstone_exception.h"
#include "bstone_image_encoder.h"
#include "bstone_scope_exit.h"
#include "bstone_sdl.h"
#include "SDL3/SDL_surface.h"

namespace bstone {

namespace {

class PngImageEncoderSdl final : public ImageEncoder
{
public:
	PngImageEncoderSdl() = default;
	~PngImageEncoderSdl() override = default;

	void encode_24(
		const std::uint8_t* src_buffer,
		int src_width,
		int src_height,
		std::uint8_t* dst_buffer,
		int dst_buffer_max_size,
		int& dst_size) override;

	void encode_indexed8_to_file(
		int width,
		int height,
		int palette_color_count,
		const std::uint8_t* rgba_palette,
		const std::uint8_t* pixels,
		const char* file_path) override;

	void encode_rgba8888_to_file(
		int width,
		int height,
		const std::uint8_t* pixels,
		const char* file_path) override;

private:
	void encode_indexed8_or_rgba8888_to_file(
		int width,
		int height,
		int palette_color_count,
		const std::uint8_t* rgba_palette,
		const std::uint8_t* pixels,
		const char* file_path);
};

// --------------------------------------

void PngImageEncoderSdl::encode_24(
	const std::uint8_t* src_buffer,
	int src_width,
	int src_height,
	std::uint8_t* dst_buffer,
	int dst_buffer_max_size,
	int& dst_size)
{
	SDL_Surface* sdl_surface = nullptr;
	SDL_IOStream* sdl_io_stream = nullptr;
	const auto scope_exit = make_scope_exit(
		[&sdl_surface, &sdl_io_stream]()
		{
			SDL_DestroySurface(sdl_surface);
			SDL_CloseIO(sdl_io_stream);
		});
	sdl_surface = SDL_CreateSurfaceFrom(
		src_width,
		src_height,
		SDL_PIXELFORMAT_RGB24,
		const_cast<std::uint8_t*>(src_buffer),
		src_width * 3);
	if (sdl_surface == nullptr)
	{
		sdl::fail("SDL_CreateSurfaceFrom");
	}
	sdl_io_stream = SDL_IOFromMem(dst_buffer, dst_buffer_max_size);
	if (sdl_io_stream == nullptr)
	{
		sdl::fail("SDL_IOFromMem");
	}
	if (!SDL_SavePNG_IO(sdl_surface, sdl_io_stream, false))
	{
		sdl::fail("SDL_SavePNG_IO");
	}
	dst_size = static_cast<int>(SDL_TellIO(sdl_io_stream));
}

void PngImageEncoderSdl::encode_indexed8_to_file(
	int width,
	int height,
	int palette_color_count,
	const std::uint8_t* rgba_palette,
	const std::uint8_t* pixels,
	const char* file_path)
{
	encode_indexed8_or_rgba8888_to_file(
		width,
		height,
		palette_color_count,
		rgba_palette,
		pixels,
		file_path);
}

void PngImageEncoderSdl::encode_rgba8888_to_file(
	int width,
	int height,
	const std::uint8_t* pixels,
	const char* file_path)
{
	encode_indexed8_or_rgba8888_to_file(
		width,
		height,
		0,
		nullptr,
		pixels,
		file_path);
}

void PngImageEncoderSdl::encode_indexed8_or_rgba8888_to_file(
	int width,
	int height,
	int palette_color_count,
	const std::uint8_t* rgba_palette,
	const std::uint8_t* pixels,
	const char* file_path)
{
	const bool has_palette = palette_color_count > 0 && rgba_palette != nullptr;
	const int pitch = has_palette ? width : width * 4;
	const SDL_PixelFormat sdl_surface_format = has_palette ? SDL_PIXELFORMAT_INDEX8 : SDL_PIXELFORMAT_RGBA32;
	SDL_Surface* const sdl_surface = SDL_CreateSurfaceFrom(
		width,
		height,
		sdl_surface_format,
		const_cast<std::uint8_t*>(pixels),
		pitch);
	if (sdl_surface == nullptr)
	{
		sdl::fail("SDL_CreateSurfaceFrom");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_surface]()
		{
			SDL_DestroySurface(sdl_surface);
		});
	if (has_palette)
	{
		SDL_Palette* const sdl_palette = SDL_CreateSurfacePalette(sdl_surface);
		if (sdl_palette == nullptr)
		{
			sdl::fail("SDL_CreateSurfacePalette");
		}
		if (!SDL_SetPaletteColors(sdl_palette, reinterpret_cast<const SDL_Color*>(rgba_palette), 0, palette_color_count))
		{
			sdl::fail("SDL_SetPaletteColors");
		}
	}
	if (!SDL_SavePNG(sdl_surface, file_path))
	{
		sdl::fail("SDL_SavePNG");
	}
}

} // namespace

ImageEncoderUPtr make_image_encoder(ImageEncoderType image_encoder_type)
{
	switch (image_encoder_type)
	{
		case ImageEncoderType::png:
			return std::make_unique<PngImageEncoderSdl>();
		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported image encoder type.");
	}
}

} // namespace bstone
