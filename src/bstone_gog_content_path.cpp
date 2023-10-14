/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
		constexpr win32::RegistryAccessType access_types[] =
		{
			win32::RegistryAccessType::none,
			win32::RegistryAccessType::wow64_64,
			win32::RegistryAccessType::wow64_32
		};

		auto has_found_access_type = false;
		auto access_type = win32::RegistryAccessType::none;

		for (const auto i_access_type : access_types)
		{
			const auto has_key = win32::RegistryKey::has_key(
				subkey_name.c_str(),
				win32::RegistryRootKeyType::local_machine,
				i_access_type);

			if (!has_key)
			{
				continue;
			}

			has_found_access_type = true;
			access_type = i_access_type;
			break;
		}

		if (!has_found_access_type)
		{
			return false;
		}

		try
		{
			registry_key.open(
				subkey_name.c_str(),
				win32::RegistryRootKeyType::local_machine,
				access_type | win32::RegistryAccessType::read);

			return true;
		}
		catch (...)
		{
			return false;
		}
	};

	const auto get_registry_string = [](
		win32::RegistryKey& registry_key,
		const std::string& value_name,
		std::string& value) -> bool
	{
		if (!registry_key.has_string(value_name.c_str()))
		{
			return false;
		}

		value.resize(win32::RegistryKey::max_string_length);

		try
		{
			const auto length = registry_key.get_string(
				value_name.c_str(),
				&value[0],
				win32::RegistryKey::max_string_length);

			value.resize(static_cast<std::size_t>(length));

			return true;
		}
		catch (...)
		{
			return false;
		}
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
