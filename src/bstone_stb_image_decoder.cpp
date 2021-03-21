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
		const char* message)
		:
		Exception{std::string{"[STB_IMAGE_DECODER] "} + message}
	{
	}
}; // StbImageDecoderException


} // detail

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void StbImageDecoder::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
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

	if (stb_bytes == nullptr)
	{
		const auto message = stbi_failure_reason();
		throw detail::StbImageDecoderException{message};
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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
