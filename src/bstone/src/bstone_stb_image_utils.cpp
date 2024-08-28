/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_stb_image_utils.h"

#include <algorithm>
#include <new>

namespace bstone {

void* stb_cxx_malloc(std::size_t size) noexcept
{
	return ::operator new(size, std::nothrow);
}

void stb_cxx_free(void* ptr) noexcept
{
	::operator delete(ptr);
}

void* stb_cxx_realloc_sized(void* ptr, std::size_t old_size, std::size_t new_size) noexcept
{
	if (ptr == nullptr)
	{
		return stb_cxx_malloc(new_size);
	}

	if (new_size <= old_size)
	{
		return ptr;
	}

	auto new_ptr = stb_cxx_malloc(new_size);

	if (new_ptr == nullptr)
	{
		return nullptr;
	}

	const auto src_data = static_cast<const char*>(ptr);
	const auto dst_data = static_cast<char*>(new_ptr);
	const auto copy_count = std::min(old_size, new_size);
	std::copy_n(src_data, copy_count, dst_data);

	stb_cxx_free(ptr);

	return new_ptr;
}

} // namespace bstone
