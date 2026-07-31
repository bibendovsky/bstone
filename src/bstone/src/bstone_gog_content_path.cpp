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

#ifdef __APPLE__
#include <string>

#include <sqlite3.h>

#include "bstone_fs_utils.h"
#include "bstone_sys_file.h"
#endif // __APPLE__


namespace bstone
{

#ifdef __APPLE__
namespace
{

struct GogMacProduct
{
	long long product_id;
	// The outer bundle's name drifted between releases; the id file inside
	// stays put and carries the product id the Windows registry uses.
	const char* bundle_names[2];
	const char* id_file_name;
	const char* game_name;
	const char* marker_file_name;
};

constexpr GogMacProduct gog_mac_products[2] =
{
	GogMacProduct{
		1207658728,
		{"Blake Stone - Aliens of Gold.app", "Blake Stone Aliens of Gold.app"},
		".goggame-1207658728.info",
		"Blake Stone Aliens of Gold",
		"AUDIOHED.BS6"},
	GogMacProduct{
		1207658729,
		{"Blake Stone Planet Strike.app", "Blake Stone - Planet Strike.app"},
		".goggame-1207658729.info",
		"Blake Stone Planet Strike",
		"AUDIOHED.VSI"},
};

// The client keeps the machine-wide install registry here, the way the
// Windows client keeps it under HKLM.
constexpr const char* galaxy_db_path = "/Users/Shared/GOG.com/Galaxy/Storage/galaxy-2.0.db";

std::string get_galaxy_installation_path(long long product_id)
{
	std::string path{};
	sqlite3* db = nullptr;

	if (sqlite3_open_v2(galaxy_db_path, &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK)
	{
		sqlite3_stmt* statement = nullptr;

		if (sqlite3_prepare_v2(
			db,
			"SELECT installationPath FROM InstalledBaseProducts WHERE productId = ?;",
			-1,
			&statement,
			nullptr) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, product_id);

			if (sqlite3_step(statement) == SQLITE_ROW)
			{
				const unsigned char* text = sqlite3_column_text(statement, 0);

				if (text != nullptr)
				{
					path = reinterpret_cast<const char*>(text);
				}
			}

			sqlite3_finalize(statement);
		}

		sqlite3_close(db);
	}

	return path;
}

bool file_exists(const std::string& path)
{
	auto file = sys::File{};
	return file.open(path.c_str(), sys::FileMode::read);
}

// The bundle is a launcher app wrapping a Boxer app whose gamebox holds
// the DOS drive with the game's files.
std::string find_data_in_gog_mac_bundle(const std::string& bundle_path, const GogMacProduct& product)
{
	const std::string name = product.game_name;
	const std::string data_path = fs_utils::append_path(
		bundle_path,
		"Contents/Resources/game/" + name + ".app/Contents/Resources/" + name + ".boxer/C " + name + ".harddisk");

	if (file_exists(fs_utils::append_path(data_path, product.marker_file_name)))
	{
		return data_path;
	}

	return std::string{};
}

std::string find_gog_mac_game_path(const GogMacProduct& product)
{
	// The Galaxy client records where it installed the product.
	const std::string galaxy_bundle_path = get_galaxy_installation_path(product.product_id);

	if (!galaxy_bundle_path.empty())
	{
		const std::string data_path = find_data_in_gog_mac_bundle(galaxy_bundle_path, product);

		if (!data_path.empty())
		{
			return data_path;
		}
	}

	// Without the client - the offline installer - the bundle sits in
	// "/Applications", identified by the id file its installer drops.
	for (const char* bundle_name : product.bundle_names)
	{
		const std::string bundle_path = fs_utils::append_path("/Applications", bundle_name);

		if (!file_exists(fs_utils::append_path(bundle_path, fs_utils::append_path("Contents/Resources", product.id_file_name))))
		{
			continue;
		}

		const std::string data_path = find_data_in_gog_mac_bundle(bundle_path, product);

		if (!data_path.empty())
		{
			return data_path;
		}
	}

	return std::string{};
}

} // namespace
#endif // __APPLE__


AssetPath make_gog_content_path()
{
	auto result = AssetPath{};
	result.provider = ContentPathProvider::gog;

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
				result.aog = value;
			}
		}
	}

	// PS
	{
		if (open_registry_key(ps_sub_key_name, registry_key))
		{
			if (get_registry_string(registry_key, value_name, value))
			{
				result.ps = value;
			}
		}
	}
#endif // _WIN32

#ifdef __APPLE__
	result.aog = find_gog_mac_game_path(gog_mac_products[0]);
	result.ps = find_gog_mac_game_path(gog_mac_products[1]);
#endif // __APPLE__

	return result;
}


} // bstone
