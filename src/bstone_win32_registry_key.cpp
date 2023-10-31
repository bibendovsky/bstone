/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows registry key primitive.

#if defined(_WIN32)

#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "bstone_exception.h"
#include "bstone_win32_registry_key.h"
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

DWORD map_access_type(RegistryAccessType access_type)
{
	const auto is_create = (access_type & RegistryAccessType::create) != RegistryAccessType::none;

	auto access = DWORD{};

	if ((access_type & RegistryAccessType::read) != RegistryAccessType::none)
	{
		access |= KEY_READ;
	}

	if (is_create || (access_type & RegistryAccessType::write) != RegistryAccessType::none)
	{
		access |= KEY_WRITE;
	}

	if ((access_type & RegistryAccessType::wow64_32) != RegistryAccessType::none)
	{
		access |= KEY_WOW64_32KEY;
	}

	if ((access_type & RegistryAccessType::wow64_64) != RegistryAccessType::none)
	{
		access |= KEY_WOW64_64KEY;
	}

	return access;
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
};

LSTATUS registry_key_open(HKEY root_key, LPCWSTR subkey_name, DWORD access, HKEY& subkey)
{
	return RegOpenKeyExW(
		root_key,
		subkey_name,
		0,
		access,
		&subkey);
};

} // namespace

// ==========================================================================

void RegistryKeyHandleDeleter::operator()(RegistryKeyHandle* handle) const
{
	RegCloseKey(reinterpret_cast<HKEY>(handle));
}

// ==========================================================================

RegistryKey::RegistryKey(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryAccessType access_type)
{
	open(subkey_name, root_key_type, access_type);
}

void RegistryKey::open(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryAccessType access_type)
{
	close();

	const auto root_key = map_root_key_type(root_key_type);
	const auto access = map_access_type(access_type);
	const auto is_create = (access_type & RegistryAccessType::create) != RegistryAccessType::none;

	const auto u16_subkey_name = Win32WString{subkey_name};
	const auto reg_func = is_create ? registry_key_create : registry_key_open;

	auto subkey = HKEY{};
	const auto win32_result = reg_func(root_key, u16_subkey_name.get_data(), access, subkey);

	if (win32_result != ERROR_SUCCESS)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to open a key.");
	}

	handle_ = RegistryKeyHandleUPtr{reinterpret_cast<RegistryKeyHandle*>(subkey)};
}

void RegistryKey::close()
{
	handle_ = nullptr;
}

bool RegistryKey::is_open() const noexcept
{
	return handle_ != nullptr;
}

bool RegistryKey::has_string(const char* name) const
{
	ensure_is_open();

	auto u16_name = Win32WString{name};
	auto value_type = DWORD{};

	const auto win32_result = RegQueryValueExW(
		reinterpret_cast<HKEY>(handle_.get()),
		u16_name.get_data(),
		nullptr,
		&value_type,
		nullptr,
		nullptr);

	if (win32_result != ERROR_SUCCESS)
	{
		return false;
	}

	if (value_type != REG_SZ)
	{
		return false;
	}

	return true;
}

std::intptr_t RegistryKey::get_string(const char* name, char* buffer, std::intptr_t buffer_size) const
{
	ensure_is_open();

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
		BSTONE_THROW_STATIC_SOURCE("Failed to get a string value.");
	}

	if (value_type != REG_SZ)
	{
		BSTONE_THROW_STATIC_SOURCE("Not a string value.");
	}

	if ((value_byte_count % 2) != 0)
	{
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
		BSTONE_THROW_STATIC_SOURCE("RegQueryValueExW failed.");
	}

	if (value_type != REG_SZ)
	{
		BSTONE_THROW_STATIC_SOURCE("Value type mismatch.");
	}

	if (value_byte_count != value_byte_count_2)
	{
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
		return 0;
	}

	if (buffer[u8_size - 1] == '\0')
	{
		return u8_size - 1;
	}

	if (u8_size == buffer_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer overflow.");
	}

	buffer[u8_size] = '\0';

	return u8_size;
}

void RegistryKey::set_string(const char* name, const char* value) const
{
	ensure_is_open();

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
		BSTONE_THROW_STATIC_SOURCE("Failed to write a string value.");
	}
}

void RegistryKey::delete_value(const char* name) const
{
	ensure_is_open();

	auto u16_name = Win32WString{name};
	const auto win32_result = RegDeleteValueW(reinterpret_cast<HKEY>(handle_.get()), u16_name.get_data());

	if (win32_result != ERROR_SUCCESS)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to delete a value.");
	}
}

bool RegistryKey::has_key(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryAccessType access_type)
{
	const auto root_key = map_root_key_type(root_key_type);
	const auto access = map_access_type(access_type);
	const auto u16_subkey_name = Win32WString{subkey_name};

	auto subkey = HKEY{};

	const auto win32_result = RegOpenKeyExW(
		root_key,
		u16_subkey_name.get_data(),
		0,
		(access & (KEY_WOW64_32KEY | KEY_WOW64_64KEY)) | KEY_READ,
		&subkey);

	if (win32_result != ERROR_SUCCESS)
	{
		if (win32_result == ERROR_ACCESS_DENIED)
		{
			return true;
		}

		return false;
	}

	RegCloseKey(subkey);

	return true;
}

void RegistryKey::delete_key(
	const char* subkey_name,
	RegistryRootKeyType root_key_type,
	RegistryAccessType access_type)
{
	const auto root_key = map_root_key_type(root_key_type);
	const auto access = map_access_type(access_type);
	const auto u16_subkey_name = Win32WString{subkey_name};

	const auto win32_result = RegDeleteKeyExW(
		root_key,
		u16_subkey_name.get_data(),
		access & (KEY_WOW64_32KEY | KEY_WOW64_64KEY),
		0);

	if (win32_result != ERROR_SUCCESS)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to delete a key.");
	}
}

void RegistryKey::ensure_is_open() const
{
	if (handle_ == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Closed key.");
	}
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
