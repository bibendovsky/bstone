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
	static const auto value_name = std::string{"InstallLocation"};

	static const auto sub_key_base_name = std::string{
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "};

	static const auto aog_sub_key_name = sub_key_base_name + "358190";
	static const auto ps_sub_key_name = sub_key_base_name + "358310";
	static const auto tatp_sub_key_name = sub_key_base_name + "238050";

	// AOG
	{
		auto win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::system,
			Win32RegistryRootKeyType::machine,
			aog_sub_key_name
		);

		if (win32_registry_key)
		{
			const auto& string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				static const auto aog_sub_dir = std::string{"Blake Stone - Aliens of Gold"};

				result.aog_ = bstone::file_system::append_path(string_result.value_, aog_sub_dir);
			}
		}
	}

	// PS
	{
		auto win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::system,
			Win32RegistryRootKeyType::machine,
			ps_sub_key_name
		);

		if (win32_registry_key)
		{
			const auto& string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				static const auto ps_sub_dir = std::string{"Blake Stone - Planet Strike"};

				result.ps_ = bstone::file_system::append_path(string_result.value_, ps_sub_dir);
			}
		}
	}

	// The Apogee Throwback Pack
	{
		auto win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::system,
			Win32RegistryRootKeyType::machine,
			tatp_sub_key_name
		);

		if (win32_registry_key)
		{
			const auto& string_result = win32_registry_key->get_string(value_name);

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
