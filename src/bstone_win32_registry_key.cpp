/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows registry key primitive.

#if defined(_WIN32)

#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#include "bstone_win32_registry_key.h"

#include <windows.h>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_win32_advapi32_symbols.h"
#include "bstone_utf.h"
#include "bstone_win32_wstring.h"

namespace bstone {
namespace win32 {

// ==========================================================================

namespace {

HKEY map_root_key_type(RegistryRootKeyType root_key_type)
{
	switch (root_key_type)
	{
		case RegistryRootKeyType::local_machine: return HKEY_LOCAL_MACHINE;
		case RegistryRootKeyType::current_user: return HKEY_CURRENT_USER;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown root key type.");
	}
}

DWORD map_open_flags(RegistryOpenFlags open_flags)
{
	const auto is_create = (open_flags & RegistryOpenFlags::create) != RegistryOpenFlags::none;
	const auto is_read = (open_flags & RegistryOpenFlags::read) != RegistryOpenFlags::none;
	const auto is_write = is_create || (open_flags & RegistryOpenFlags::write) != RegistryOpenFlags::none;
	const auto is_wow64_32 = (open_flags & RegistryOpenFlags::wow64_32) != RegistryOpenFlags::none;
	const auto is_wow64_64 = (open_flags & RegistryOpenFlags::wow64_64) != RegistryOpenFlags::none;

	if (!is_read && !is_write)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected read or write access.");
	}

	if (is_wow64_32 && is_wow64_64)
	{
		BSTONE_THROW_STATIC_SOURCE("Only one view type allowed.");
	}

	return DWORD
	{
		(is_read ? KEY_READ : 0U) |
		(is_write ? KEY_WRITE : 0U) |
		(is_wow64_32 ? KEY_WOW64_32KEY : 0U) |
		(is_wow64_64 ? KEY_WOW64_64KEY : 0U) |
		0U
	};
}

DWORD map_wow64_open_flags(RegistryOpenFlags open_flags)
{
	const auto is_wow64_32 = (open_flags & RegistryOpenFlags::wow64_32) != RegistryOpenFlags::none;
	const auto is_wow64_64 = (open_flags & RegistryOpenFlags::wow64_64) != RegistryOpenFlags::none;

	if (is_wow64_32 && is_wow64_64)
	{
		BSTONE_THROW_STATIC_SOURCE("Only one view type allowed.");
	}

	return DWORD
	{
		(is_wow64_32 ? KEY_WOW64_32KEY : 0U) |
		(is_wow64_64 ? KEY_WOW64_64KEY : 0U) |
		0U
	};
}

LSTATUS registry_key_create(HKEY root_key, LPCWSTR subkey_name, DWORD access, HKEY& subkey)
{
	return RegCreateKeyExW(
		root_key,
		subkey_name,
		0,
		nullptr,
		0,
		access,
		nullptr,
		&subkey,
		nullptr);
}

LSTATUS registry_key_open(HKEY root_key, LPCWSTR subkey_name, DWORD access, HKEY& subkey)
{
	return RegOpenKeyExW(
		root_key,
		subkey_name,
		0,
		access,
		&subkey);
}

} // namespace

// ==========================================================================

void RegistryKeyHandleDeleter::operator()(RegistryKeyHandle* handle) const noexcept
{
	RegCloseKey(reinterpret_cast<HKEY>(handle));
}

// ==========================================================================

RegistryKey::RegistryKey() noexcept = default;

RegistryKey::RegistryKey(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags)
{
	try_or_open(subkey_name, root_key_type, open_flags, false);
}

bool RegistryKey::try_open(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags)
{
	return try_or_open(subkey_name, root_key_type, open_flags, true);
}

void RegistryKey::open(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags)
{
	try_or_open(subkey_name, root_key_type, open_flags, false);
}

void RegistryKey::close() noexcept
{
	handle_ = nullptr;
}

bool RegistryKey::is_open() const noexcept
{
	return handle_ != nullptr;
}

bool RegistryKey::try_get_string(
	const char* name,
	char* buffer,
	std::intptr_t buffer_size,
	std::intptr_t& written_size) const
{
	return try_or_get_string(name, buffer, buffer_size, true, written_size);
}

std::intptr_t RegistryKey::get_string(const char* name, char* buffer, std::intptr_t buffer_size) const
{
	auto result = std::intptr_t{};
	try_or_get_string(name, buffer, buffer_size, false, result);
	return result;
}

bool RegistryKey::try_set_string(const char* name, const char* value) const
{
	return try_or_set_string(name, value, true);
}

void RegistryKey::set_string(const char* name, const char* value) const
{
	try_or_set_string(name, value, false);
}

bool RegistryKey::try_delete_value(const char* name) const
{
	return try_or_delete_value(name, true);
}

void RegistryKey::delete_value(const char* name) const
{
	try_or_delete_value(name, false);
}

bool RegistryKey::try_delete_key(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags)
{
	return try_or_delete_key(subkey_name, root_key_type, open_flags, true);
}

void RegistryKey::delete_key(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags)
{
	try_or_delete_key(subkey_name, root_key_type, open_flags, false);
}

bool RegistryKey::try_or_open(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags,
	bool is_ignore_errors)
{
	close();

	const auto root_key = map_root_key_type(root_key_type);
	const auto access = map_open_flags(open_flags);
	const auto is_create = (open_flags & RegistryOpenFlags::create) != RegistryOpenFlags::none;

	const auto u16_subkey_name = Win32WString{subkey_name};
	const auto reg_func = is_create ? registry_key_create : registry_key_open;

	auto subkey = HKEY{};
	const auto win32_result = reg_func(root_key, u16_subkey_name.get_data(), access, subkey);

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to open a key.");
	}

	handle_ = RegistryKeyHandleUPtr{reinterpret_cast<RegistryKeyHandle*>(subkey)};
	return true;
}

bool RegistryKey::try_or_get_string(
	const char* name,
	char* buffer,
	std::intptr_t buffer_size,
	bool is_ignore_errors,
	std::intptr_t& written_size) const
{
	BSTONE_ASSERT(is_open());

	written_size = 0;

	auto value_type = DWORD{};
	auto win32_result = LSTATUS{};

	const auto u16_name = Win32WString{name};

	// Get value size.
	//
	auto value_byte_count = DWORD{};

	win32_result = RegQueryValueExW(
		reinterpret_cast<HKEY>(handle_.get()),
		u16_name.get_data(),
		nullptr,
		&value_type,
		nullptr,
		&value_byte_count);

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to get a string value.");
	}

	if (value_type != REG_SZ)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Not a string value.");
	}

	if ((value_byte_count % 2) != 0)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Invalid value size in bytes.");
	}

	// Get the value.
	//
	const auto u16_value_size = static_cast<std::intptr_t>(value_byte_count / 2);
	auto u16_value = Win32WString{u16_value_size};
	const auto u16_value_begin = u16_value.get_data();
	auto value_byte_count_2 = DWORD{value_byte_count};

	win32_result = RegQueryValueExW(
		reinterpret_cast<HKEY>(handle_.get()),
		u16_name.get_data(),
		nullptr,
		&value_type,
		reinterpret_cast<LPBYTE>(u16_value_begin),
		&value_byte_count_2);

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("RegQueryValueExW failed.");
	}

	if (value_type != REG_SZ)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Value type mismatch.");
	}

	if (value_byte_count != value_byte_count_2)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Value size mismatch.");
	}

	const auto u8_next = utf::u16_to_u8(
		u16_value_begin,
		u16_value_begin + u16_value_size,
		buffer,
		buffer + buffer_size);

	auto u8_size = u8_next - buffer;

	if (u8_size == 0)
	{
		*buffer = '\0';
		return true;
	}

	if (buffer[u8_size - 1] == '\0')
	{
		written_size = u8_size - 1;
		return true;
	}

	if (u8_size == buffer_size)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Buffer overflow.");
	}

	buffer[u8_size] = '\0';
	written_size = u8_size;

	return true;
}

bool RegistryKey::try_or_set_string(const char* name, const char* value, bool is_ignore_errors) const
{
	BSTONE_ASSERT(is_open());

	auto u16_name = Win32WString{name};
	const auto u16_value = Win32WString{value};
	const auto u16_byte_count = 2 * (u16_value.get_size() + 1);
	auto win32_size = static_cast<DWORD>(u16_byte_count);

	const auto win32_result = RegSetValueExW(
		reinterpret_cast<HKEY>(handle_.get()),
		u16_name.get_data(),
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(u16_value.get_data()),
		win32_size);

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to write a string value.");
	}

	return true;
}

bool RegistryKey::try_or_delete_value(const char* name, bool is_ignore_errors) const
{
	BSTONE_ASSERT(is_open());

	auto u16_name = Win32WString{name};
	const auto win32_result = RegDeleteValueW(reinterpret_cast<HKEY>(handle_.get()), u16_name.get_data());

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to delete a value.");
	}

	return true;
}

bool RegistryKey::try_or_delete_key(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryOpenFlags open_flags,
	bool is_ignore_errors)
{
	const auto root_key = map_root_key_type(root_key_type);
	const auto access = map_wow64_open_flags(open_flags);
	const auto u16_subkey_name = Win32WString{subkey_name};
	const auto reg_delete_key_ex_w = AdvApi32Symbols::get_reg_delete_key_ex_w();

	auto win32_result = LSTATUS{};

	if (reg_delete_key_ex_w != nullptr)
	{
		win32_result = reg_delete_key_ex_w(
			root_key,
			u16_subkey_name.get_data(),
			access & (KEY_WOW64_32KEY | KEY_WOW64_64KEY),
			0);
	}
	else
	{
		win32_result = RegDeleteKeyW(root_key, u16_subkey_name.get_data());
	}

	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to delete a key.");
	}

	return true;
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
