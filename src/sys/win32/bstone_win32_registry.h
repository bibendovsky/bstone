/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	wow64_64,
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
	Win32RegistryKey() noexcept = default;

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
