/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// WIN32 wide string.

#if defined(_WIN32)

#include "bstone_char_traits.h"
#include "bstone_configurations.h"
#include "bstone_exception.h"
#include "bstone_ts_auto_arena_memory_resource.h"
#include "bstone_utf.h"
#include "bstone_win32_wstring.h"

namespace bstone {

MemoryResource& get_win32_wstring_memory_resource()
{
	struct Initializer
	{
		Initializer(TsAutoArenaMemoryResource& memory_resource)
		{
			memory_resource.reserve(low_level_api_u16string_capacity, get_default_memory_resource());
		}
	};

	static TsAutoArenaMemoryResource memory_resource{};
	static Initializer initializer{memory_resource};

	return memory_resource;
}

// ==========================================================================

Win32WString::StorageDeleter::StorageDeleter(MemoryResource& memory_resource)
	:
	memory_resource_{&memory_resource}
{}

void Win32WString::StorageDeleter::operator()(wchar_t* ptr) const
{
	memory_resource_->deallocate(ptr);
}

// --------------------------------------------------------------------------

Win32WString::Win32WString(const char* u8_string)
try
	:
	storage_{make_storage(u8_string, size_)}
{} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

IntP Win32WString::get_size() const noexcept
{
	return size_;
}

const wchar_t* Win32WString::get_data() const noexcept
{
	return storage_.get();
}

wchar_t* Win32WString::get_data() noexcept
{
	return storage_.get();
}

auto Win32WString::make_storage(const char* u8_string, IntP& u16_size) -> Storage
{
	auto& memory_resource = get_win32_wstring_memory_resource();

	const auto u8_size = char_traits::get_size(u8_string);
	const auto u16_capacity = u8_size + 1;
	const auto u16_byte_count = u16_capacity * static_cast<IntP>(sizeof(wchar_t));

	const auto u16_string = static_cast<wchar_t*>(memory_resource.allocate(u16_byte_count));
	auto storage = Storage{u16_string, StorageDeleter{memory_resource}};

	const auto u16_string_next = utf::u8_to_u16(
		u8_string,
		u8_string + u8_size + 1,
		u16_string,
		u16_string + u16_capacity);

	u16_size = u16_string_next - u16_string - 1;

	return storage;
}

} // namespace bstone

#endif // _WIN32
