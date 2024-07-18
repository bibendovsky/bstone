/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// POSIX API string.

#ifndef _WIN32

#include <algorithm>
#include <memory>

#include "bstone_char_traits.h"
#include "bstone_configurations.h"
#include "bstone_exception.h"
#include "bstone_posix_string.h"
#include "bstone_ts_auto_arena_resource.h"

namespace bstone {

namespace {

MemoryResource& get_posix_string_memory_resource()
{
	struct Initializer
	{
		Initializer(TsAutoArenaResource& memory_resource)
		{
			memory_resource.reserve(posix_string_capacity, get_default_memory_resource());
		}
	};

	static TsAutoArenaResource memory_resource{};
	static Initializer initializer{memory_resource};
	return memory_resource;
}

} // namespace

// ==========================================================================

PosixString::PosixString()
	:
	Base{get_posix_string_memory_resource()}
{}

PosixString::PosixString(MemoryResource& memory_resource)
	:
	Base{memory_resource}
{}

PosixString::PosixString(std::intptr_t capacity)
	:
	Base{capacity, get_posix_string_memory_resource()}
{}

PosixString::PosixString(std::intptr_t capacity, MemoryResource& memory_resource)
	:
	Base{capacity, memory_resource}
{}

PosixString::PosixString(const char* string)
	:
	PosixString{string, get_posix_string_memory_resource()}
{}

PosixString::PosixString(const char* string, MemoryResource& memory_resource)
	:
	Base{memory_resource}
{
	if (string == nullptr)
	{
		return;
	}

	const auto size = char_traits::get_size(string);
	auto posix_string = Base{size + 1, memory_resource};
	std::copy_n(string, size, posix_string.get_data());
	swap(posix_string);
	set_size(size);
}

} // namespace bstone

#endif // _WIN32
