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
	static const auto value_name = std::string{"path"};

	static const auto sub_key_base_name = std::string{"SOFTWARE\\GOG.com\\Games\\"};
	static const auto aog_sub_key_name = sub_key_base_name + "1207658728";
	static const auto ps_sub_key_name = sub_key_base_name + "1207658729";

	// AOG
	{
		auto win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::wow64_32,
			Win32RegistryRootKeyType::machine,
			aog_sub_key_name
		);

		if (win32_registry_key)
		{
			const auto& string_result = win32_registry_key->get_string(value_name);

			if (string_result)
			{
				result.aog_ = string_result.value_;
			}
		}
	}

	// PS
	{
		auto win32_registry_key = make_win32_registry_key(
			Win32RegistryViewType::wow64_32,
			Win32RegistryRootKeyType::machine,
			ps_sub_key_name
		);

		if (win32_registry_key)
		{
			const auto& string_result = win32_registry_key->get_string(value_name);

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
