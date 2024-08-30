/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows API wide string.

#if defined(_WIN32)

#include "bstone_char_traits.h"
#include "bstone_configurations.h"
#include "bstone_exception.h"
#include "bstone_ts_auto_arena_resource.h"
#include "bstone_utf.h"
#include "bstone_win32_wstring.h"

namespace bstone {

MemoryResource& get_win32_wstring_memory_resource()
{
	struct Initializer
	{
		Initializer(TsAutoArenaResource& memory_resource)
		{
			memory_resource.reserve(win32_wstring_capacity, get_default_memory_resource());
		}
	};

	static TsAutoArenaResource memory_resource{};
	static Initializer initializer{memory_resource};
	return memory_resource;
}

// ==========================================================================

Win32WString::Win32WString()
	:
	Base{get_win32_wstring_memory_resource()}
{}

Win32WString::Win32WString(MemoryResource& memory_resource)
	:
	Base{memory_resource}
{}

Win32WString::Win32WString(std::intptr_t capacity)
	:
	Base{capacity, get_win32_wstring_memory_resource()}
{}

Win32WString::Win32WString(std::intptr_t capacity, MemoryResource& memory_resource)
	:
	Base{capacity, memory_resource}
{}

Win32WString::Win32WString(const char* u8_string)
	:
	Win32WString{u8_string, get_win32_wstring_memory_resource()}
{}

Win32WString::Win32WString(const char* u8_string, MemoryResource& memory_resource)
	:
	Base{memory_resource}
{
	if (u8_string == nullptr)
	{
		return;
	}

	const auto u8_size = char_traits::get_size(u8_string);
	auto u16_string = Base{u8_size, memory_resource};
	const auto u16_chars = u16_string.get_data();

	const auto u16_string_next = utf::u8_to_u16(
		u8_string,
		u8_string + u8_size,
		u16_chars,
		u16_chars + u8_size);

	const auto u16_size = u16_string_next - u16_chars;
	u16_string.set_size(u16_size);
	swap(u16_string);
}

} // namespace bstone

#endif // _WIN32
