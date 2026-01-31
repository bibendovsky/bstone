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
#include "bstone_win32_utility.h"
#include "bstone_win32_wstring.h"
#include <climits>

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

struct RegKeyCreateOrOpenResult
{
	HKEY hkey;
	LSTATUS lstatus;

	explicit operator bool() const
	{
		return lstatus == ERROR_SUCCESS;
	}
};

RegKeyCreateOrOpenResult registry_key_create(HKEY root_key, LPCWSTR subkey_name, DWORD access)
{
	HKEY hkey;
	const LSTATUS lstatus = RegCreateKeyExW(
		/* hKey                 */ root_key,
		/* lpSubKey             */ subkey_name,
		/* Reserved             */ 0,
		/* lpClass              */ nullptr,
		/* dwOptions            */ 0,
		/* samDesired           */ access,
		/* lpSecurityAttributes */ nullptr,
		/* phkResult            */ &hkey,
		/* lpdwDisposition      */ nullptr);
	return RegKeyCreateOrOpenResult{.hkey = hkey, .lstatus = lstatus,};
}

RegKeyCreateOrOpenResult registry_key_open(HKEY root_key, LPCWSTR subkey_name, DWORD access)
{
	HKEY hkey;
	const LSTATUS lstatus = RegOpenKeyExW(
		/* hKey       */ root_key,
		/* lpSubKey   */ subkey_name,
		/* ulOptions  */ 0,
		/* samDesired */ access,
		/* phkResult  */ &hkey);
	return RegKeyCreateOrOpenResult{.hkey = hkey, .lstatus = lstatus,};
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
	const HKEY root_key = map_root_key_type(root_key_type);
	const DWORD access = map_open_flags(open_flags);
	const bool is_create = (open_flags & RegistryOpenFlags::create) != RegistryOpenFlags::none;
	const auto reg_func = is_create ? registry_key_create : registry_key_open;
	RegKeyCreateOrOpenResult create_or_open_result;
	if (subkey_name != nullptr)
	{
		const StackOrHeapWString<512> u16_subkey_name{subkey_name};
		if (u16_subkey_name == nullptr)
		{
			if (is_ignore_errors)
			{
				return false;
			}
			BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
		}
		create_or_open_result = reg_func(root_key, u16_subkey_name, access);
	}
	else
	{
		create_or_open_result = reg_func(root_key, nullptr, access);
	}
	if (!create_or_open_result)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("Failed to open a key.");
	}
	handle_ = RegistryKeyHandleUPtr{reinterpret_cast<RegistryKeyHandle*>(create_or_open_result.hkey)};
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
	if (buffer_size == 0)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("Buffer size too small.");
	}
	DWORD value_type;
	LSTATUS win32_result;
	const StackOrHeapWString<128> u16_name{name};
	if (u16_name == nullptr)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
	}
	// Get value size.
	DWORD value_byte_count;
	win32_result = RegQueryValueExW(
		/* hKey        */ reinterpret_cast<HKEY>(handle_.get()),
		/* lpValueName */ u16_name,
		/* lpReserved  */ nullptr,
		/* lpType      */ &value_type,
		/* lpData      */ nullptr,
		/* lpcbData    */ &value_byte_count);
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
	if (value_byte_count == 0)
	{
		*buffer = '\0';
		return true;
	}
	if ((value_byte_count % 2) != 0)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("Odd value size in bytes.");
	}
	if (value_byte_count > INT_MAX)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("Value size too big.");
	}
	// Get the value.
	const int u16_value_size = static_cast<int>(value_byte_count / 2);
	StackOrHeapWString<512> u16_value{u16_value_size};
	if (u16_value == nullptr)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
	}
	DWORD value_byte_count_2 = value_byte_count;
	win32_result = RegQueryValueExW(
		/* hKey        */ reinterpret_cast<HKEY>(handle_.get()),
		/* lpValueName */ u16_name,
		/* lpReserved  */ nullptr,
		/* lpType      */ &value_type,
		/* lpData      */ reinterpret_cast<LPBYTE>(u16_value.get()),
		/* lpcbData    */ &value_byte_count_2);
	if (win32_result != ERROR_SUCCESS)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("RegQueryValueExW");
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
	const int u8_size = utf16_to_utf8(u16_value, u16_value_size, buffer, static_cast<int>(buffer_size));
	if (u8_size < 0)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("utf16_to_utf8");
	}
	if (buffer[u8_size - 1] == '\0')
	{
		written_size = static_cast<std::intptr_t>(u8_size - 1);
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
	const StackOrHeapWString<128> u16_name{name};
	const StackOrHeapWString<512> u16_value{value};
	if (u16_name == nullptr || u16_value == nullptr)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
	}
	const int u16_byte_count = 2 * (u16_value.get_size() + 1);
	const LSTATUS win32_result = RegSetValueExW(
		/* hKey        */ reinterpret_cast<HKEY>(handle_.get()),
		/* lpValueName */ u16_name,
		/* Reserved    */ 0,
		/* dwType      */ REG_SZ,
		/* lpData      */ reinterpret_cast<const BYTE*>(u16_value.get()),
		/* cbData      */ static_cast<DWORD>(u16_byte_count));
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
	const StackOrHeapWString<128> u16_name{name};
	if (u16_name == nullptr)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
	}
	if (const LSTATUS win32_result = RegDeleteValueW(reinterpret_cast<HKEY>(handle_.get()), u16_name);
		win32_result != ERROR_SUCCESS)
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
	const HKEY root_key = map_root_key_type(root_key_type);
	const DWORD access = map_wow64_open_flags(open_flags);
	const StackOrHeapWString<512> u16_subkey_name{subkey_name};
	if (u16_subkey_name == nullptr)
	{
		if (is_ignore_errors)
		{
			return false;
		}
		BSTONE_THROW_STATIC_SOURCE("StackOrHeapWString");
	}
	const RegDeleteKeyExWFunc reg_delete_key_ex_w = AdvApi32Symbols::get_reg_delete_key_ex_w();
	LSTATUS win32_result;
	if (reg_delete_key_ex_w != nullptr)
	{
		win32_result = reg_delete_key_ex_w(
			/* hKey       */ root_key,
			/* lpSubKey   */ u16_subkey_name.get(),
			/* samDesired */ access & (KEY_WOW64_32KEY | KEY_WOW64_64KEY),
			/* Reserved   */ 0);
	}
	else
	{
		win32_result = RegDeleteKeyW(
			/* hKey     */ root_key,
			/* lpSubKey */ u16_subkey_name);
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
