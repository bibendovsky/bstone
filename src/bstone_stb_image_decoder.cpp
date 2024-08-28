/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_stb_image_decoder.h"

#include <cassert>

#include <algorithm>
#include <memory>

#include "bstone_stb_image_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC bstone::stb_cxx_malloc
#define STBI_FREE bstone::stb_cxx_free
#define STBI_REALLOC_SIZED bstone::stb_cxx_realloc_sized
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

#include "bstone_exception.h"

namespace bstone {

StbImageDecoder::~StbImageDecoder() = default;

void StbImageDecoder::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
try {
	dst_width = 0;
	dst_height = 0;

	auto stb_comp = 0;

	const auto stb_bytes = stbi_load_from_memory(
		static_cast<const stbi_uc*>(src_data),
		src_data_size,
		&dst_width,
		&dst_height,
		&stb_comp,
		4);

	if (stb_bytes == nullptr)
	{
		BSTONE_THROW_DYNAMIC_SOURCE(stbi_failure_reason());
	}

	const auto dst_area = dst_width * dst_height;

	if (dst_buffer.size() < static_cast<std::size_t>(dst_area))
	{
		dst_buffer.resize(dst_area);
	}

	std::copy_n(reinterpret_cast<const Rgba8*>(stb_bytes), dst_area, dst_buffer.begin());
	STBI_FREE(stb_bytes);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
