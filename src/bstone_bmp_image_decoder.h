/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_BMP_IMAGE_DECODER_INCLUDED
#define BSTONE_BMP_IMAGE_DECODER_INCLUDED


#include "bstone_image_decoder.h"

#include "SDL.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class BmpImageDecoder :
	public ImageDecoder
{
public:
	void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) override;


private:
	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void decode_non_paletted(
		SDL_Surface* src_sdl_surface,
		Uint32 dst_sdl_pixel_format,
		Rgba8Buffer& dst_buffer);

	static void decode_paletted(
		SDL_Surface* src_sdl_surface,
		Uint32 dst_sdl_pixel_format,
		Rgba8Buffer& dst_buffer);
}; // BmpImageDecoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_BMP_IMAGE_DECODER_INCLUDED
