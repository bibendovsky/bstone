/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Locating a game's files under a folder the user chose.

#include "bstone_game_source.h"

#include <algorithm>
#include <cstring>

#include "bstone_ascii.h"
#include "bstone_fs_utils.h"
#include "bstone_sys_fs.h"

namespace bstone {

namespace {

// Deep enough for the deepest layout seen - a storefront bundle wrapping a
// DOS-emulator bundle, eight levels from the folder the user can pick - with
// room to spare for one holding several games.
constexpr int max_search_depth = 10;
constexpr int max_visited_directories = 4096;

// The file that says a folder holds a game, for each of the two games and the
// shareware release. Matching one of these is what tells the folders apart.
constexpr const char* marker_file_names[] =
{
	"AUDIOHED.BS6", // Aliens of Gold.
	"AUDIOHED.BS1", // Aliens of Gold, shareware.
	"AUDIOHED.VSI", // Planet Strike.
};

struct SearchState
{
	GameSourcePaths paths;
	int visited_directories;
};

bool is_marker_file_name(const char* file_name)
{
	for (const char* marker_file_name : marker_file_names)
	{
		const char* lhs = file_name;
		const char* rhs = marker_file_name;

		while (*lhs != '\0' && *rhs != '\0' && ascii::to_upper(*lhs) == *rhs)
		{
			++lhs;
			++rhs;
		}

		if (*lhs == '\0' && *rhs == '\0')
		{
			return true;
		}
	}

	return false;
}

struct DirectoryScan
{
	bool has_marker;
	std::vector<std::string> sub_directory_names;
};

sys::EnumDirCallbackResult scan_callback(void* user_data, const char* directory_path, const char* file_name)
{
	auto& scan = *static_cast<DirectoryScan*>(user_data);

	if (is_marker_file_name(file_name))
	{
		scan.has_marker = true;
		return sys::EnumDirCallbackResult::resume;
	}

	const std::string entry_path = fs_utils::append_path(directory_path, file_name);

	if (sys::is_directory_exists(entry_path.c_str()))
	{
		scan.sub_directory_names.emplace_back(file_name);
	}

	return sys::EnumDirCallbackResult::resume;
}

void search(const std::string& path, int depth, SearchState& state)
{
	if (depth > max_search_depth || state.visited_directories >= max_visited_directories)
	{
		return;
	}

	++state.visited_directories;
	auto scan = DirectoryScan{};

	if (!sys::enumerate_directory(path.c_str(), scan_callback, &scan))
	{
		return;
	}

	if (scan.has_marker)
	{
		// A folder holding a game's files is an answer, not a place to search
		// under: what is below it belongs to that game.
		state.paths.emplace_back(path);
		return;
	}

	// Deterministic order, so the same folder always yields the same list.
	std::sort(scan.sub_directory_names.begin(), scan.sub_directory_names.end());

	for (const std::string& sub_directory_name : scan.sub_directory_names)
	{
		search(fs_utils::append_path(path, sub_directory_name), depth + 1, state);
	}
}

} // namespace

GameSourcePaths find_game_sources(const std::string& path)
{
	if (path.empty() || !sys::is_directory_exists(path.c_str()))
	{
		return GameSourcePaths{};
	}

	auto state = SearchState{};
	search(path, 0, state);
	return std::move(state.paths);
}

const char* get_game_source_label(const std::string& path)
{
	if (path.find("/steamapps/") != std::string::npos)
	{
		return "Steam";
	}

	if (path.find("/GOG Games/") != std::string::npos)
	{
		return "GOG";
	}

	// A GOG application carries the id file its installer dropped; look for it
	// beside the bundle the game sits in.
	std::string probe_path = path;

	while (!probe_path.empty())
	{
		const std::size_t app_pos = probe_path.rfind(".app");

		if (app_pos == std::string::npos)
		{
			break;
		}

		probe_path.resize(app_pos + 4);
		const std::string resources_path = fs_utils::append_path(probe_path, "Contents/Resources");
		bool is_gog = false;
		sys::enumerate_directory(
			resources_path.c_str(),
			[](void* user_data, const char*, const char* file_name) -> sys::EnumDirCallbackResult
			{
				if (std::strncmp(file_name, ".goggame-", 9) == 0)
				{
					*static_cast<bool*>(user_data) = true;
				}

				return sys::EnumDirCallbackResult::resume;
			},
			&is_gog);

		if (is_gog)
		{
			return "GOG";
		}

		probe_path.resize(app_pos);
	}

	return "Folder";
}

std::string make_display_path(const std::string& path)
{
	// Everything below an application bundle is that bundle's business, and
	// the whole chain is far too long to read.
	const std::size_t app_pos = path.find(".app/");
	return app_pos != std::string::npos ? path.substr(0, app_pos + 4) : path;
}

const char* get_game_source_prompt()
{
	return
		"BStone automatically searches Steam and GOG installations.\n"
		"You can also choose a folder to search for supported game files.";
}

const char* get_game_source_dialog_note()
{
#if defined(__APPLE__)
	return
		"On macOS, you may choose the Applications folder. "
		"BStone will search inside game applications automatically.";
#elif defined(_WIN32)
	return "Choose where a game was installed, or a folder holding its files.";
#else
	return
		"Steam installs are searched already. For GOG, run the offline "
		"installer and choose where it installed - GOG Games in your home "
		"folder by default.";
#endif
}

} // namespace bstone
