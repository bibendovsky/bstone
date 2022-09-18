/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Steam content path.
//


#include "bstone_content_path.h"

#include "bstone_file_system.h"

#ifdef _WIN32
#include "bstone_win32_registry.h"
#endif // _WIN32


namespace bstone
{
namespace detail
{


AssetPath make_steam_content_path()
{
	auto result = AssetPath{};
	result.provider_ = ContentPathProvider::steam;

#ifdef _WIN32
	const auto get_registry_key = [](const std::string& subkey_name) -> Win32RegistryKeyUPtr
	{
		auto win32_registry_key = Win32RegistryKeyUPtr{};

		win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::wow64_64,
			Win32RegistryRootKeyType::machine,
			subkey_name);

		if (!win32_registry_key)
		{
			win32_registry_key = make_win32_registry_key(
				Win32RegistryViewType::wow64_32,
				Win32RegistryRootKeyType::machine,
				subkey_name);
		}

		return win32_registry_key;
	};

	static const auto value_name = std::string{"InstallLocation"};

	static const auto sub_key_base_name = std::string{
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "};

	static const auto aog_sub_key_name = sub_key_base_name + "358190";
	static const auto ps_sub_key_name = sub_key_base_name + "358310";
	static const auto tatp_sub_key_name = sub_key_base_name + "238050";

	// AOG
	{
		const auto win32_registry_key = get_registry_key(aog_sub_key_name);

		if (win32_registry_key)
		{
			const auto string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				static const auto aog_sub_dir = std::string{"Blake Stone - Aliens of Gold"};
				result.aog_ = bstone::file_system::append_path(string_result.value_, aog_sub_dir);
			}
		}
	}

	// PS
	{
		const auto win32_registry_key = get_registry_key(ps_sub_key_name);

		if (win32_registry_key)
		{
			const auto string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				static const auto ps_sub_dir = std::string{"Blake Stone - Planet Strike"};
				result.ps_ = bstone::file_system::append_path(string_result.value_, ps_sub_dir);
			}
		}
	}

	// The Apogee Throwback Pack
	{
		const auto win32_registry_key = get_registry_key(tatp_sub_key_name);

		if (win32_registry_key)
		{
			const auto string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				const auto& base_path = string_result.value_;

				if (result.aog_.empty())
				{
					static const auto aog_dir = std::string{"Blake Stone"};
					result.aog_ = file_system::append_path(base_path, aog_dir);
				}

				if (result.ps_.empty())
				{
					static const auto ps_dir = std::string{"Planet Strike"};
					result.ps_ = file_system::append_path(base_path, ps_dir);
				}
			}
		}
	}
#endif // _WIN32

	return result;
}


} // detail
} // bstone
