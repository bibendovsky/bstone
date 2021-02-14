/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Windows registry manager.
//


#include "bstone_win32_registry.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "bstone_encoding.h"
#include "bstone_exception.h"
#include "bstone_unique_resource.h"


namespace bstone
{


class Win32RegistryKeyException :
	public Exception
{
public:
	explicit Win32RegistryKeyException(
		const char* const message)
		:
		Exception{std::string{"[WIN32_REG_KEY] "} + message}
	{
	}
}; // Win32RegistryKeyException


void Win32RegistryKeyDeleter(
	const HKEY& resource) noexcept
{
	RegCloseKey(resource);
}

using Win32RegistryKeyResource = UniqueResource<HKEY, Win32RegistryKeyDeleter>;

class Win32RegistryKeyImpl final :
	public Win32RegistryKey
{
public:
	explicit Win32RegistryKeyImpl(
		Win32RegistryKeyResource&& key_resource);

	~Win32RegistryKeyImpl() override;


	Win32RegistryKeyStringValueResult get_string(
		const std::string& value_name) const override;


private:
	Win32RegistryKeyResource key_resource_;
}; // Win32RegistryKey


Win32RegistryKeyImpl::Win32RegistryKeyImpl(
	Win32RegistryKeyResource&& key_resource)
	:
	key_resource_{std::move(key_resource)}
{
}

Win32RegistryKeyImpl::~Win32RegistryKeyImpl() = default;

Win32RegistryKeyStringValueResult Win32RegistryKeyImpl::get_string(
	const std::string& value_name) const
{
	const auto& value_name_utf16 = utf8_to_utf16(value_name);

	auto win32_value_type = DWORD{};
	auto win32_value_size = DWORD{};

	const auto win32_result_probe = RegQueryValueExW(
		key_resource_.get(),
		reinterpret_cast<LPCWSTR>(value_name_utf16.c_str()),
		nullptr,
		&win32_value_type,
		nullptr,
		&win32_value_size
	);

	switch (win32_result_probe)
	{
		case ERROR_SUCCESS:
		case ERROR_MORE_DATA:
			break;

		case ERROR_FILE_NOT_FOUND:
		default:
			return Win32RegistryKeyStringValueResult{};
	}

	if (win32_value_type != REG_SZ)
	{
		return Win32RegistryKeyStringValueResult{};
	}

	if (win32_value_size < 2)
	{
		return Win32RegistryKeyStringValueResult{""};
	}

	const auto value_length = win32_value_size / sizeof(char16_t);

	auto value_utf16 = std::u16string{};
	value_utf16.resize(value_length);

	const auto win32_result = RegQueryValueExW(
		key_resource_.get(),
		reinterpret_cast<LPCWSTR>(value_name_utf16.c_str()),
		nullptr,
		&win32_value_type,
		reinterpret_cast<LPBYTE>(&value_utf16[0]),
		&win32_value_size
	);

	if (win32_result != ERROR_SUCCESS)
	{
		return Win32RegistryKeyStringValueResult{};
	}

	value_utf16.resize(value_utf16.size() - 1);

	return Win32RegistryKeyStringValueResult{utf16_to_utf8(value_utf16)};
}


Win32RegistryKeyUPtr make_win32_registry_key(
	const Win32RegistryViewType view_type,
	const Win32RegistryRootKeyType root_key_type,
	const std::string& sub_key_name_utf8)
{
	auto win32_access = DWORD{KEY_READ};

	switch (view_type)
	{
		case Win32RegistryViewType::system:
			break;

		case Win32RegistryViewType::wow64_32:
			win32_access |= KEY_WOW64_32KEY;
			break;

		default:
			throw Win32RegistryKeyException{"Unsupported view type."};
	}

	auto win32_root_key = HKEY{};

	switch (root_key_type)
	{
		case Win32RegistryRootKeyType::machine:
			win32_root_key = HKEY_LOCAL_MACHINE;
			break;

		default:
			throw Win32RegistryKeyException{"Unsupported root key type."};
	}

	if (sub_key_name_utf8.empty())
	{
		throw Win32RegistryKeyException{"Empty sub-key name."};
	}

	const auto& sub_key_name_utf16 = utf8_to_utf16(sub_key_name_utf8);

	auto win32_key = HKEY{};

	const auto win32_result = RegOpenKeyExW(
		win32_root_key,
		reinterpret_cast<LPCWSTR>(sub_key_name_utf16.c_str()),
		0,
		win32_access,
		&win32_key
	);

	auto win32_key_resource = Win32RegistryKeyResource{win32_key};

	if (win32_result != ERROR_SUCCESS || !win32_key_resource)
	{
		return nullptr;
	}

	return std::make_unique<Win32RegistryKeyImpl>(std::move(win32_key_resource));
}


} // bstone
