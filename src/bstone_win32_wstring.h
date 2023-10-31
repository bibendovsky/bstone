/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows API wide string.

#if defined(_WIN32)

#include "bstone_api_string.h"
#include "bstone_int.h"

namespace bstone {

class Win32WString final : public ApiString<wchar_t>
{
public:
	using Base = ApiString<wchar_t>;

public:
	Win32WString();
	explicit Win32WString(MemoryResource& memory_resource);
	explicit Win32WString(std::intptr_t capacity);
	Win32WString(std::intptr_t capacity, MemoryResource& memory_resource);
	Win32WString(const char* u8_string);
	Win32WString(const char* u8_string, MemoryResource& memory_resource);
};

} // namespace bstone

#endif // _WIN32
