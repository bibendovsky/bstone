/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Steam content path.
//
// Steam records its library folders and installed games in VDF text files laid
// out identically on every platform, so the search is shared: find the Steam
// root, read `libraryfolders.vdf` for the libraries, then read
// `appmanifest_<appid>.acf` in each library for the game's install directory.
// Only locating the Steam root itself differs per platform.
//


#include "bstone_content_path.h"

#include <cstddef>
#include <cstdint>

#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bstone_fs_utils.h"
#include "bstone_steam_manifest.h"
#include "bstone_sys_file.h"
#include "bstone_sys_special_path.h"

#ifdef _WIN32
#include "bstone_win32_registry_key.h"
#endif // _WIN32


namespace bstone
{


namespace
{


// Steam application identifiers.
constexpr auto aog_app_id = "358190";
constexpr auto ps_app_id = "358310";
constexpr auto pack_app_id = "238050"; // The Apogee Throwback Pack.

// A VDF file of this size is not one of Steam's.
constexpr auto max_vdf_size = 4 * 1024 * 1024;

// Steam standardised on the lower case spelling, but older installations still
// use the other one and either is accepted.
constexpr const char* steamapps_dir_names[] = {"steamapps", "SteamApps"};

// A game's own files live in a sub-directory of the Steam install directory, and
// the name differs between the games, between the standalone titles and the
// pack, and between the desktop and the macOS depots. All of them are tried
// everywhere: a Proton installation on Linux has the Windows layout, and a
// library folder can be carried between machines.
constexpr const char* aog_sub_dirs[] =
{
	"Blake Stone - Aliens of Gold",
	"Blake Stone AOG.app/Contents/Resources/BlakestoneAOG",
	"",
};

constexpr const char* ps_sub_dirs[] =
{
	"Blake Stone - Planet Strike",
	"Blake Stone PS.app/Contents/Resources/BlakestonePS",
	"",
};

constexpr const char* pack_aog_sub_dirs[] =
{
	"Blake Stone",
	"Blake Stone AOG.app/Contents/Resources/BlakestoneAOG",
	"",
};

constexpr const char* pack_ps_sub_dirs[] =
{
	"Planet Strike",
	"Blake Stone PS.app/Contents/Resources/BlakestonePS",
	"",
};

// A manifest only records what Steam believes; the directory it points at may
// have been emptied by hand. Probing for an asset the game cannot run without
// both proves the directory exists and tells the two games apart.
constexpr const char* aog_marker_files[] = {"AUDIOHED.BS6", "AUDIOHED.BS1"};
constexpr const char* ps_marker_files[] = {"AUDIOHED.VSI"};


bool file_exists(const std::string& path)
{
	auto file = sys::File{};
	return file.open(path.c_str(), sys::FileMode::read);
}

bool read_text_file(const std::string& path, std::string& text)
{
	auto file = sys::File{};

	if (!file.open(path.c_str(), sys::FileMode::read))
	{
		return false;
	}

	const auto size = file.get_size();

	if (size <= 0 || size > max_vdf_size)
	{
		return false;
	}

	text.resize(static_cast<std::size_t>(size));

	return file.read_exactly(&text[0], static_cast<int>(size));
}

void add_steam_root(std::vector<std::string>& roots, std::string root)
{
	if (root.empty())
	{
		return;
	}

	fs_utils::normalize_separators_inplace(root);
	fs_utils::trim_non_root_trailing_separator_inplace(root);

	for (const auto& existing_root : roots)
	{
		if (existing_root == root)
		{
			return;
		}
	}

	roots.emplace_back(std::move(root));
}

void add_home_relative_steam_root(std::vector<std::string>& roots, const char* sub_path)
{
	char home_chars[2048];
	const auto home_size = sys::SpecialPath::get_home_path(home_chars,

		static_cast<int>(sizeof(home_chars)));

	if (home_size <= 0)
	{
		return;
	}

	add_steam_root(roots, fs_utils::append_path(std::string{home_chars, static_cast<std::size_t>(home_size)}, sub_path));
}

#ifdef _WIN32
bool get_registry_string(
	win32::RegistryRootKeyType root_key_type,
	const char* sub_key_name,
	const char* value_name,
	std::string& value)
{
	constexpr win32::RegistryOpenFlags open_flags_set[] =
	{
		win32::RegistryOpenFlags::read | win32::RegistryOpenFlags::wow64_64,
		win32::RegistryOpenFlags::read | win32::RegistryOpenFlags::wow64_32,
	};

	for (const auto open_flags : open_flags_set)
	{
		auto registry_key = win32::RegistryKey{};

		if (!registry_key.try_open(sub_key_name, root_key_type, open_flags))
		{
			continue;
		}

		value.resize(win32::RegistryKey::max_string_length);
		auto length = std::intptr_t{};

		if (registry_key.try_get_string(
			value_name,
			&value[0],
			win32::RegistryKey::max_string_length,
			length))
		{
			value.resize(static_cast<std::size_t>(length));
			return true;
		}
	}

	value.clear();
	return false;
}
#endif // _WIN32

std::vector<std::string> make_steam_roots()
{
	auto roots = std::vector<std::string>{};
	roots.reserve(8);

#ifdef _WIN32
	// The running client keeps this current, including for a relocated
	// installation, so it comes first. Its value uses forward slashes, unlike the
	// machine-wide one, hence the normalisation in add_steam_root.
	auto registry_value = std::string{};

	if (get_registry_string(
		win32::RegistryRootKeyType::current_user,
		"Software\\Valve\\Steam",
		"SteamPath",
		registry_value))
	{
		add_steam_root(roots, registry_value);
	}

	if (get_registry_string(
		win32::RegistryRootKeyType::local_machine,
		"SOFTWARE\\Valve\\Steam",
		"InstallPath",
		registry_value))
	{
		add_steam_root(roots, registry_value);
	}

	add_steam_root(roots, "C:\\Program Files (x86)\\Steam");
#elif defined(__APPLE__)
	add_home_relative_steam_root(roots, "Library/Application Support/Steam");
#else
	// The first is the real directory on a current installation; the next two are
	// normally symbolic links to it, and are listed for older layouts. The Flatpak
	// build moved its data to .local/share and left `data` as a link behind.
	add_home_relative_steam_root(roots, ".local/share/Steam");
	add_home_relative_steam_root(roots, ".steam/steam");
	add_home_relative_steam_root(roots, ".steam/root");
	add_home_relative_steam_root(roots, ".steam/debian-installation");
	add_home_relative_steam_root(roots, ".var/app/com.valvesoftware.Steam/.local/share/Steam");
	add_home_relative_steam_root(roots, ".var/app/com.valvesoftware.Steam/data/Steam");
	// The Snap package remaps the home directory, so its data sits outside the
	// locations above.
	add_home_relative_steam_root(roots, "snap/steam/common/.local/share/Steam");
#endif // _WIN32

	return roots;
}

// Reads the library list. Steam regenerates the copy under `config` at start-up;
// installations predating that only have the one under `steamapps`.
bool read_library_folders(const std::string& steam_root, std::string& text)
{
	if (read_text_file(fs_utils::append_path(steam_root, "config/libraryfolders.vdf"), text))
	{
		return true;
	}

	for (const auto steamapps_dir_name : steamapps_dir_names)
	{
		const auto path = fs_utils::append_path(
			fs_utils::append_path(steam_root, steamapps_dir_name),
			"libraryfolders.vdf");

		if (read_text_file(path, text))
		{
			return true;
		}
	}

	return false;
}

std::vector<std::string> make_library_paths(const std::string& steam_root)
{
	auto library_paths = std::vector<std::string>{};
	library_paths.reserve(8);

	// Only the current layout lists the Steam root among the libraries, so add it
	// unconditionally and let the de-duplication drop the repeat.
	add_steam_root(library_paths, steam_root);

	auto text = std::string{};

	if (!read_library_folders(steam_root, text))
	{
		return library_paths;
	}

	for (auto& path : parse_steam_library_paths(text))
	{
		add_steam_root(library_paths, std::move(path));
	}

	return library_paths;
}

// Returns the install directory recorded for an application, when it is present
// and Steam considers it installed.
bool find_app_install_dir(
	const std::string& library_path,
	const char* app_id,
	std::string& install_path)
{
	for (const auto steamapps_dir_name : steamapps_dir_names)
	{
		const auto steamapps_path = fs_utils::append_path(library_path, steamapps_dir_name);
		const auto manifest_path = fs_utils::append_path(
			steamapps_path,
			std::string{"appmanifest_"} + app_id + ".acf");

		auto text = std::string{};

		if (!read_text_file(manifest_path, text))
		{
			continue;
		}

		auto install_dir = std::string{};

		if (!parse_steam_install_dir(text, install_dir))
		{
			continue;
		}

		install_path = fs_utils::append_path(
			fs_utils::append_path(steamapps_path, "common"),
			install_dir);

		return true;
	}

	return false;
}

bool has_marker_file(const std::string& path, const char* const* marker_files, std::size_t marker_file_count)
{
	for (auto i = std::size_t{}; i < marker_file_count; ++i)
	{
		if (file_exists(fs_utils::append_path(path, marker_files[i])))
		{
			return true;
		}
	}

	return false;
}

// Picks the sub-directory of an installation that actually holds the game.
bool find_game_dir(
	const std::string& install_path,
	const char* const* sub_dirs,
	std::size_t sub_dir_count,
	const char* const* marker_files,
	std::size_t marker_file_count,
	std::string& game_path)
{
	for (auto i = std::size_t{}; i < sub_dir_count; ++i)
	{
		auto path = sub_dirs[i][0] != '\0' ?
			fs_utils::append_path(install_path, sub_dirs[i]) :
			install_path;

		if (has_marker_file(path, marker_files, marker_file_count))
		{
			game_path = std::move(path);
			return true;
		}
	}

	return false;
}


// True when "path" is the directory "prefix", or something inside it. The two
// come from different places, so both are spelled with the one separator
// before being compared - the portable one, since it is also what the boundary
// between the prefix and the rest is tested against.
bool is_within(const std::string& path, const std::string& prefix)
{
	if (prefix.empty() || path.size() < prefix.size())
	{
		return false;
	}

	auto normalized_path = path;
	auto normalized_prefix = prefix;
	fs_utils::normalize_separators_portable_inplace(normalized_path);
	fs_utils::normalize_separators_portable_inplace(normalized_prefix);

	if (normalized_path.compare(0, normalized_prefix.size(), normalized_prefix) != 0)
	{
		return false;
	}

	return normalized_path.size() == normalized_prefix.size() ||
		normalized_path[normalized_prefix.size()] == '/';
}


} // namespace


std::string make_steam_art_path(const std::string& game_path)
{
	if (game_path.empty())
	{
		return std::string{};
	}

	// The pack shares one entry for both games, so its artwork is the same
	// whichever of the two was found inside it.
	constexpr const char* app_ids[] = {aog_app_id, ps_app_id, pack_app_id};

	for (const auto& steam_root : make_steam_roots())
	{
		for (const auto& library_path : make_library_paths(steam_root))
		{
			for (const auto app_id : app_ids)
			{
				auto install_path = std::string{};

				if (!find_app_install_dir(library_path, app_id, install_path) ||
					!is_within(game_path, install_path))
				{
					continue;
				}

				// Only the logo is stored as PNG; the rest of what the client
				// caches is JPEG, which BStone cannot read.
				const auto art_path = fs_utils::append_path(
					fs_utils::append_path(
						fs_utils::append_path(steam_root, "appcache/librarycache"),
						app_id),
					"logo.png");

				if (file_exists(art_path))
				{
					return art_path;
				}
			}
		}
	}

	return std::string{};
}


AssetPath make_steam_content_path()
{
	auto result = AssetPath{};
	result.provider = ContentPathProvider::steam;

	for (const auto& steam_root : make_steam_roots())
	{
		for (const auto& library_path : make_library_paths(steam_root))
		{
			auto install_path = std::string{};

			if (result.aog.empty() &&
				find_app_install_dir(library_path, aog_app_id, install_path))
			{
				find_game_dir(
					install_path,
					aog_sub_dirs, std::size(aog_sub_dirs),
					aog_marker_files, std::size(aog_marker_files),
					result.aog);
			}

			if (result.ps.empty() &&
				find_app_install_dir(library_path, ps_app_id, install_path))
			{
				find_game_dir(
					install_path,
					ps_sub_dirs, std::size(ps_sub_dirs),
					ps_marker_files, std::size(ps_marker_files),
					result.ps);
			}

			// The pack carries both games, and only fills in what the standalone
			// titles did not provide.
			if ((result.aog.empty() || result.ps.empty()) &&
				find_app_install_dir(library_path, pack_app_id, install_path))
			{
				if (result.aog.empty())
				{
					find_game_dir(
						install_path,
						pack_aog_sub_dirs, std::size(pack_aog_sub_dirs),
						aog_marker_files, std::size(aog_marker_files),
						result.aog);
				}

				if (result.ps.empty())
				{
					find_game_dir(
						install_path,
						pack_ps_sub_dirs, std::size(pack_ps_sub_dirs),
						ps_marker_files, std::size(ps_marker_files),
						result.ps);
				}
			}

			if (!result.aog.empty() && !result.ps.empty())
			{
				return result;
			}
		}
	}

	return result;
}


} // bstone
