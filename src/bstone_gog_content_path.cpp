/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// GOG content path.
//


#include "bstone_content_path.h"

#ifdef _WIN32
#include "bstone_win32_registry.h"
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

	static const auto value_name = std::string{"path"};

	static const auto sub_key_base_name = std::string{"SOFTWARE\\GOG.com\\Games\\"};
	static const auto aog_sub_key_name = sub_key_base_name + "1207658728";
	static const auto ps_sub_key_name = sub_key_base_name + "1207658729";

	// AOG
	{
		const auto win32_registry_key = get_registry_key(aog_sub_key_name);

		if (win32_registry_key)
		{
			const auto string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				result.aog_ = string_result.value_;
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
				result.ps_ = string_result.value_;
			}
		}
	}
#endif // _WIN32

	return result;
}


} // detail
} // bstone
