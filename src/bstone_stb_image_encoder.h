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


#ifndef BSTONE_STB_IMAGE_ENCODER_INCLUDED
#define BSTONE_STB_IMAGE_ENCODER_INCLUDED


#include "bstone_image_encoder.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class StbImageEncoder :
	public ImageEncoder
{
public:
	void encode_24(
		const std::uint8_t* src_buffer,
		int src_width,
		int src_height,
		std::uint8_t* dst_buffer,
		int max_dst_buffer_size,
		int& dst_size) override;


private:
	std::uint8_t* dst_buffer_{};
	int size_{};
	int max_size_{};


	static void stb_write_func_proxy(
		void* context,
		void* data,
		int size);

	void stb_write_func(
		void* data,
		int size);
}; // StbImageEncoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_STB_IMAGE_ENCODER_INCLUDED
