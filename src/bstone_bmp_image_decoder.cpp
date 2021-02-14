/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_bmp_image_decoder.h"

#include <cstdint>

#include <memory>
#include <string>

#include "SDL_pixels.h"
#include "SDL_surface.h"

#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_sdl2_types.h"
#include "bstone_sdl2_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class BmpImageDecoderException :
	public Exception
{
public:
	explicit BmpImageDecoderException(
		const char* message)
		:
		Exception{std::string{"[BMP_IMAGE_DECODER] "} + message}
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
{
	dst_width = 0;
	dst_height = 0;

	if (src_data == nullptr)
	{
		throw BmpImageDecoderException{"Null source data."};
	}

	if (src_data_size <= 0)
	{
		throw BmpImageDecoderException{"Source data size out of range."};
	}

	const auto sdl_rw_ops = SDL_RWFromConstMem(src_data, src_data_size);

	if (sdl_rw_ops == nullptr)
	{
		throw Sdl2Exception{};
	}

	const auto sdl_src_surface = SdlSurfaceUPtr{SDL_LoadBMP_RW(sdl_rw_ops, true)};

	if (sdl_src_surface == nullptr)
	{
		throw Sdl2Exception{};
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
		Sdl2EnsureResult{SDL_LockSurface(sdl_src_surface.get())};
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

void BmpImageDecoder::decode_non_paletted(
	SDL_Surface* src_sdl_surface,
	Uint32 dst_sdl_pixel_format,
	Rgba8Buffer& dst_buffer)
{
	Sdl2EnsureResult{SDL_ConvertPixels(
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

void BmpImageDecoder::decode_paletted(
	SDL_Surface* src_sdl_surface,
	Uint32 dst_sdl_pixel_format,
	Rgba8Buffer& dst_buffer)
{
	const auto dst_sdl_surface = SdlSurfaceUPtr{SDL_ConvertSurfaceFormat(src_sdl_surface, dst_sdl_pixel_format, 0)};

	Sdl2EnsureResult{dst_sdl_surface.get()};

	if (dst_sdl_surface->pitch != (4 * src_sdl_surface->w))
	{
		throw BmpImageDecoderException{"Unsupported pitch value."};
	}

	if (SDL_MUSTLOCK(dst_sdl_surface.get()))
	{
		Sdl2EnsureResult{SDL_LockSurface(dst_sdl_surface.get())};
	}

	std::uninitialized_copy_n(
		static_cast<const Rgba8*>(dst_sdl_surface->pixels),
		src_sdl_surface->w * src_sdl_surface->h,
		dst_buffer.data()
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
