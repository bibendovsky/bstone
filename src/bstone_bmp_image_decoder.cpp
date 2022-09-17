/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "bstone_bmp_image_decoder.h"

#include <cstdint>

#include <memory>
#include <string>

#include "SDL_pixels.h"
#include "SDL_surface.h"

#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_sdl_types.h"
#include "bstone_sdl_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class BmpImageDecoderException :
	public Exception
{
public:
	explicit BmpImageDecoderException(
		const char* message) noexcept
		:
		Exception{"BMP_IMAGE_DECODER", message}
	{
	}
}; // BmpImageDecoderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


void BmpImageDecoder::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
try
{
	dst_width = 0;
	dst_height = 0;

	if (src_data == nullptr)
	{
		fail("Null source data.");
	}

	if (src_data_size <= 0)
	{
		fail("Source data size out of range.");
	}

	const auto sdl_rw_ops = SDL_RWFromConstMem(src_data, src_data_size);

	if (sdl_rw_ops == nullptr)
	{
		fail_sdl();
	}

	const auto sdl_src_surface = SdlSurfaceUPtr{SDL_LoadBMP_RW(sdl_rw_ops, true)};

	if (sdl_src_surface == nullptr)
	{
		fail_sdl();
	}

	dst_width = sdl_src_surface->w;
	dst_height = sdl_src_surface->h;

	const auto dst_area = dst_width * dst_height;

	if (static_cast<int>(dst_buffer.size()) < dst_area)
	{
		dst_buffer.resize(dst_area);
	}

	constexpr auto dst_pixel_format = (Endian::is_little() ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_RGBA8888);

	if (SDL_MUSTLOCK(sdl_src_surface.get()))
	{
		SdlEnsureResult{SDL_LockSurface(sdl_src_surface.get())};
	}

	if (SDL_ISPIXELFORMAT_INDEXED(sdl_src_surface->format->format))
	{
		decode_paletted(sdl_src_surface.get(), dst_pixel_format, dst_buffer);
	}
	else
	{
		decode_non_paletted(sdl_src_surface.get(), dst_pixel_format, dst_buffer);
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void BmpImageDecoder::fail(
	const char* message)
{
	throw BmpImageDecoderException{message};
}

[[noreturn]]
void BmpImageDecoder::fail_nested(
	const char* message)
{
	std::throw_with_nested(BmpImageDecoderException{message});
}

void BmpImageDecoder::decode_non_paletted(
	SDL_Surface* src_sdl_surface,
	Uint32 dst_sdl_pixel_format,
	Rgba8Buffer& dst_buffer)
try
{
	SdlEnsureResult{SDL_ConvertPixels(
		src_sdl_surface->w,
		src_sdl_surface->h,
		src_sdl_surface->format->format,
		src_sdl_surface->pixels,
		src_sdl_surface->pitch,
		dst_sdl_pixel_format,
		dst_buffer.data(),
		4 * src_sdl_surface->w
	)};
}
catch (...)
{
	fail_nested(__func__);
}

void BmpImageDecoder::decode_paletted(
	SDL_Surface* src_sdl_surface,
	Uint32 dst_sdl_pixel_format,
	Rgba8Buffer& dst_buffer)
try
{
	const auto dst_sdl_surface = SdlSurfaceUPtr{SDL_ConvertSurfaceFormat(src_sdl_surface, dst_sdl_pixel_format, 0)};

	SdlEnsureResult{dst_sdl_surface.get()};

	if (dst_sdl_surface->pitch != (4 * src_sdl_surface->w))
	{
		fail("Unsupported pitch value.");
	}

	if (SDL_MUSTLOCK(dst_sdl_surface.get()))
	{
		SdlEnsureResult{SDL_LockSurface(dst_sdl_surface.get())};
	}

	std::uninitialized_copy_n(
		static_cast<const Rgba8*>(dst_sdl_surface->pixels),
		src_sdl_surface->w * src_sdl_surface->h,
		dst_buffer.data()
	);
}
catch (...)
{
	fail_nested(__func__);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
