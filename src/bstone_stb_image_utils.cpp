/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_stb_image_utils.h"

#include <algorithm>
#include <memory>


namespace bstone
{


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


} // bstone
