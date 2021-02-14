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


#ifndef BSTONE_WIN32_REGISTRY_INCLUDED
#define BSTONE_WIN32_REGISTRY_INCLUDED


#include <memory>
#include <string>


namespace bstone
{


enum class Win32RegistryViewType
{
	system,
	wow64_32,
}; // Win32RegistryViewType


struct Win32RegistryKeyStringValueResult
{
	bool is_valid_;
	std::string value_;


	Win32RegistryKeyStringValueResult()
		:
		is_valid_{},
		value_{}
	{
	}

	explicit Win32RegistryKeyStringValueResult(
		std::string&& string)
		:
		is_valid_{true},
		value_{std::move(string)}
	{
	}

	explicit Win32RegistryKeyStringValueResult(
		const std::string& string)
		:
		is_valid_{true},
		value_{string}
	{
	}

	explicit operator bool() const noexcept
	{
		return is_valid_;
	}
}; // Win32RegistryKeyStringValueResult


class Win32RegistryKey
{
public:
	Win32RegistryKey() = default;

	virtual ~Win32RegistryKey() = default;


	virtual Win32RegistryKeyStringValueResult get_string(
		const std::string& value_name) const = 0;
}; // Win32RegistryKey

using Win32RegistryKeyUPtr = std::unique_ptr<Win32RegistryKey>;


enum class Win32RegistryRootKeyType
{
	none,
	machine,
}; // Win32RegistryRootKeyType


Win32RegistryKeyUPtr make_win32_registry_key(
	const Win32RegistryViewType view_type,
	const Win32RegistryRootKeyType root_key_type,
	const std::string& sub_key_name_utf8);


} // bstone


#endif // !BSTONE_WIN32_REGISTRY_INCLUDED
