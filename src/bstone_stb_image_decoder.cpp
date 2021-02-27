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


void* cpp_malloc(
	std::size_t size) noexcept;

void cpp_free(
	void* ptr) noexcept;

void* cpp_realloc(
	void* ptr,
	std::size_t new_size) noexcept;


#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC cpp_malloc
#define STBI_FREE cpp_free
#define STBI_REALLOC cpp_realloc
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

#include "bstone_exception.h"


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void* cpp_malloc(
	std::size_t size) noexcept
{
	auto block = new (std::nothrow) char[sizeof(std::size_t) + size];

	if (block == nullptr)
	{
		return nullptr;
	}

	auto header = reinterpret_cast<std::size_t*>(block);
	*header = size;

	return header + 1;
}

void cpp_free(
	void* ptr) noexcept
{
	if (ptr == nullptr)
	{
		return;
	}

	delete[] reinterpret_cast<char*>(static_cast<std::size_t*>(ptr) - 1);
}

void* cpp_realloc(
	void* ptr,
	std::size_t new_size) noexcept
{
	if (ptr == nullptr)
	{
		return cpp_malloc(new_size);
	}

	const auto old_size = (reinterpret_cast<std::size_t*>(ptr))[-1];

	if (old_size >= new_size)
	{
		return ptr;
	}

	auto new_data = cpp_malloc(new_size);

	if (new_data == nullptr)
	{
		return nullptr;
	}

	const auto old_data = static_cast<const char*>(ptr);

	std::uninitialized_copy_n(
		old_data,
		std::min(old_size, new_size),
		static_cast<char*>(new_data)
	);

	cpp_free(ptr);

	return new_data;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


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
