/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_stb_image_encoder.h"

#include <cassert>
#include <cstdio>

#include <algorithm>
#include <memory>

#include "bstone_stb_image_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC bstone::stb_cxx_malloc
#define STBIW_FREE bstone::stb_cxx_free
#define STBIW_REALLOC_SIZED bstone::stb_cxx_realloc_sized
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#include "bstone_exception.h"

namespace bstone {

StbImageEncoder::~StbImageEncoder() = default;

void StbImageEncoder::encode_24(
	const std::uint8_t* src_buffer,
	int src_width,
	int src_height,
	std::uint8_t* dst_buffer,
	int max_dst_buffer_size,
	int& dst_size)
try {
	if (src_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null source buffer.");
	}

	if (src_width <= 0 || src_height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Dimensions are out of range.");
	}

	if (dst_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null destination buffer.");
	}

	if (max_dst_buffer_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Max dst buffer size out of range.");
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
		stride_bytes);

	if (stbiw_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("STBIW failed.");
	}

	if (size_ <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Empty image.");
	}

	if (size_ > max_size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not enough room in dst buffer.");
	}

	dst_size = size_;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void StbImageEncoder::stb_write_func_proxy(void* context, void* data, int size) noexcept
{
	static_cast<StbImageEncoder*>(context)->stb_write_func(data, size);
}

void StbImageEncoder::stb_write_func(void* data, int size) noexcept
{
	if (data == nullptr || size <= 0 || size > max_size_)
	{
		return;
	}

	std::copy_n(static_cast<const std::uint8_t*>(data), size, dst_buffer_);
	size_ = size;
}

} // namespace bstone
