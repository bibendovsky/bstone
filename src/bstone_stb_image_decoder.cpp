/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "bstone_stb_image_decoder.h"

#include <cassert>

#include <algorithm>
#include <memory>


#include "bstone_stb_image_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC bstone::cpp_malloc
#define STBI_FREE bstone::cpp_free
#define STBI_REALLOC bstone::cpp_realloc
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

#include "bstone_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

namespace detail
{


class StbImageDecoderException :
	public Exception
{
public:
	explicit StbImageDecoderException(
		const char* message) noexcept
		:
		Exception{"STB_IMAGE_DECODER", message}
	{
	}
}; // StbImageDecoderException


} // detail


namespace
{


[[noreturn]]
void fail(
	const char* message)
{
	throw detail::StbImageDecoderException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(detail::StbImageDecoderException{message});
}


}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void StbImageDecoder::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
try
{
	dst_width = 0;
	dst_height = 0;

	auto stb_comp = 0;

	const auto stb_bytes = stbi_load_from_memory(
		static_cast<const stbi_uc*>(src_data),
		src_data_size,
		&dst_width,
		&dst_height,
		&stb_comp,
		4
	);

	if (!stb_bytes)
	{
		fail(stbi_failure_reason());
	}

	const auto dst_area = dst_width * dst_height;

	if (dst_buffer.size() < static_cast<std::size_t>(dst_area))
	{
		dst_buffer.resize(dst_area);
	}

	std::uninitialized_copy_n(
		reinterpret_cast<const Rgba8*>(stb_bytes),
		dst_area,
		dst_buffer.begin()
	);

	STBI_FREE(stb_bytes);
}
catch (...)
{
	fail_nested(__func__);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
