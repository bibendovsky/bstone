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


#include "bstone_stb_image_encoder.h"

#include <cassert>
#include <cstdio>

#include <algorithm>
#include <memory>


#include "bstone_stb_image_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC bstone::cpp_malloc
#define STBIW_FREE bstone::cpp_free
#define STBIW_REALLOC bstone::cpp_realloc
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#include "bstone_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class StbImageEncoderException :
	public Exception
{
public:
	explicit StbImageEncoderException(
		const char* message)
		:
		Exception{std::string{"[STB_IMAGE_ENCODER] "} + message}
	{
	}
}; // StbImageEncoderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void StbImageEncoder::encode_24(
	const std::uint8_t* src_buffer,
	int src_width,
	int src_height,
	std::uint8_t* dst_buffer,
	int max_dst_buffer_size,
	int& dst_size)
{
	if (!src_buffer)
	{
		throw StbImageEncoderException{"Null src buffer."};
	}

	if (src_width <= 0 || src_height <= 0)
	{
		throw StbImageEncoderException{"Dimensions are out of range."};
	}

	if (!dst_buffer)
	{
		throw StbImageEncoderException{"Null dst buffer."};
	}

	if (max_dst_buffer_size <= 0)
	{
		throw StbImageEncoderException{"Max dst buffer size out of range."};
	}

	dst_size = 0;

	dst_buffer_ = dst_buffer;
	size_ = 0;
	max_size_ = max_dst_buffer_size;

	const auto stride_bytes = (((3 * src_width) + 3) / 4) * 4;

	const auto stbiw_result = stbi_write_png_to_func(
		stb_write_func_proxy,
		this,
		src_width,
		src_height,
		3,
		src_buffer,
		stride_bytes
	);

	if (!stbiw_result)
	{
		throw StbImageEncoderException{"STBIW failed."};
	}

	if (size_ <= 0)
	{
		throw StbImageEncoderException{"Empty image."};
	}

	if (size_ > max_size_)
	{
		throw StbImageEncoderException{"Not enough room in dst buffer."};
	}

	dst_size = size_;
}

void StbImageEncoder::stb_write_func_proxy(
	void* context,
	void* data,
	int size)
{
	static_cast<StbImageEncoder*>(context)->stb_write_func(data, size);
}

void StbImageEncoder::stb_write_func(
	void* data,
	int size)
{
	if (!data || size <= 0 || size > max_size_)
	{
		return;
	}

	std::uninitialized_copy_n(
		static_cast<const std::uint8_t*>(data),
		size,
		dst_buffer_
	);

	size_ = size;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
