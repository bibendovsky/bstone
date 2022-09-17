/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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


namespace
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class StbImageEncoderException :
	public Exception
{
public:
	explicit StbImageEncoderException(
		const char* message) noexcept
		:
		Exception{"STB_IMAGE_ENCODER", message}
	{
	}
}; // StbImageEncoderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


[[noreturn]]
void fail(
	const char* message)
{
	throw StbImageEncoderException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(StbImageEncoderException{message});
}


} // namespace


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void StbImageEncoder::encode_24(
	const std::uint8_t* src_buffer,
	int src_width,
	int src_height,
	std::uint8_t* dst_buffer,
	int max_dst_buffer_size,
	int& dst_size)
try
{
	if (!src_buffer)
	{
		fail("Null src buffer.");
	}

	if (src_width <= 0 || src_height <= 0)
	{
		fail("Dimensions are out of range.");
	}

	if (!dst_buffer)
	{
		fail("Null dst buffer.");
	}

	if (max_dst_buffer_size <= 0)
	{
		fail("Max dst buffer size out of range.");
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
		fail("STBIW failed.");
	}

	if (size_ <= 0)
	{
		fail("Empty image.");
	}

	if (size_ > max_size_)
	{
		fail("Not enough room in dst buffer.");
	}

	dst_size = size_;
}
catch (...)
{
	fail_nested(__func__);
}

void StbImageEncoder::stb_write_func_proxy(
	void* context,
	void* data,
	int size) noexcept
{
	static_cast<StbImageEncoder*>(context)->stb_write_func(data, size);
}

void StbImageEncoder::stb_write_func(
	void* data,
	int size) noexcept
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
