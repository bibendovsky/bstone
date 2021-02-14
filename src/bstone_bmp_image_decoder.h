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
