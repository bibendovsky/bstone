/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Image encoder (SDL)

#include "bstone_exception.h"
#include "bstone_image_encoder.h"
#include "bstone_scope_exit.h"
#include <format>
#include <string>
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

private:
	[[noreturn]] static void fail_sdl_func(const char* sdl_func_name);
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
			if (sdl_surface != nullptr)
			{
				SDL_DestroySurface(sdl_surface);
			}
			if (sdl_io_stream != nullptr)
			{
				SDL_CloseIO(sdl_io_stream);
			}
		});
	sdl_surface = SDL_CreateSurfaceFrom(
		src_width,
		src_height,
		SDL_PIXELFORMAT_RGB24,
		const_cast<std::uint8_t*>(src_buffer),
		src_width * 3);
	if (sdl_surface == nullptr)
	{
		fail_sdl_func("SDL_CreateSurfaceFrom");
	}
	sdl_io_stream = SDL_IOFromMem(dst_buffer, dst_buffer_max_size);
	if (sdl_io_stream == nullptr)
	{
		fail_sdl_func("SDL_IOFromMem");
	}
	if (!SDL_SavePNG_IO(sdl_surface, sdl_io_stream, false))
	{
		fail_sdl_func("SDL_SavePNG_IO");
	}
	dst_size = static_cast<int>(SDL_TellIO(sdl_io_stream));
}

[[noreturn]] void PngImageEncoderSdl::fail_sdl_func(const char* sdl_func_name)
{
	const std::string message = std::format("[{}] {}", sdl_func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
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
