/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// GOG content path.
//


#include "bstone_content_path.h"

#ifdef _WIN32
#include "bstone_win32_registry_key.h"
#endif // _WIN32


namespace bstone
{
namespace detail
{


AssetPath make_gog_content_path()
{
	auto result = AssetPath{};
	result.provider_ = ContentPathProvider::gog;

#ifdef _WIN32
	const auto open_registry_key = [](
		const std::string& subkey_name,
		win32::RegistryKey& registry_key) -> bool
	{
		constexpr win32::RegistryOpenFlags open_flags_set[] =
		{
			win32::RegistryOpenFlags::read | win32::RegistryOpenFlags::wow64_64,
			win32::RegistryOpenFlags::read | win32::RegistryOpenFlags::wow64_32
		};

		for (const auto open_flags : open_flags_set)
		{
			if (registry_key.try_open(
				subkey_name.c_str(),
				win32::RegistryRootKeyType::local_machine,
				open_flags))
			{
				return true;
			}
		}

		return false;
	};

	const auto get_registry_string = [](
		win32::RegistryKey& registry_key,
		const std::string& value_name,
		std::string& value) -> bool
	{
		value.resize(win32::RegistryKey::max_string_length);
		auto length = std::intptr_t{};

		if (!registry_key.try_get_string(
			value_name.c_str(),
			&value[0],
			win32::RegistryKey::max_string_length,
			length))
		{
			return false;
		}

		value.resize(static_cast<std::size_t>(length));

		return true;
	};

	static const auto value_name = std::string{"path"};

	static const auto sub_key_base_name = std::string{"SOFTWARE\\GOG.com\\Games\\"};
	static const auto aog_sub_key_name = sub_key_base_name + "1207658728";
	static const auto ps_sub_key_name = sub_key_base_name + "1207658729";

	auto registry_key = win32::RegistryKey{};

	auto value = std::string{};
	value.reserve(win32::RegistryKey::max_string_length);

	// AOG
	{
		if (open_registry_key(aog_sub_key_name, registry_key))
		{
			if (get_registry_string(registry_key, value_name, value))
			{
				result.aog_ = value;
			}
		}
	}

	// PS
	{
		if (open_registry_key(ps_sub_key_name, registry_key))
		{
			if (get_registry_string(registry_key, value_name, value))
			{
				result.ps_ = value;
			}
		}
	}
#endif // _WIN32

	return result;
}


} // detail
} // bstone
