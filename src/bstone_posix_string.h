/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// POSIX API string.

#ifndef _WIN32

#include <cstdint>

#include "bstone_api_string.h"

namespace bstone {

class PosixString final : public ApiString<char>
{
public:
	using Base = ApiString<char>;

public:
	PosixString();
	explicit PosixString(MemoryResource& memory_resource);
	explicit PosixString(std::intptr_t capacity);
	PosixString(std::intptr_t capacity, MemoryResource& memory_resource);
	PosixString(const char* string);
	PosixString(const char* string, MemoryResource& memory_resource);
};

} // namespace bstone

#endif // _WIN32
