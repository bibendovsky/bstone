/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Warning!
//
// All functions in this source file are designated usable by the memory
// manager after program initialization.
//


#include <cmath>

#include <algorithm>
#include <iostream>

#include "SDL_messagebox.h"

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_pm.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"

#include "bstone_logger.h"
#include "bstone_version.h"
#include "bstone_content_path.h"
#include "bstone_file_system.h"


extern SpanStart spanstart;
extern StepScale stepscale;
extern BaseDist basedist;
extern PlaneYLookup planeylookup;
extern MirrorOfs mirrorofs;


void CA_CannotOpen(
	const std::string& string);

void CAL_GetGrChunkLength(
	std::int16_t chunk);

void CA_CacheScreen(
	std::int16_t chunk);

void VH_UpdateScreen();
void IN_StartAck();
bool IN_CheckAck();
void OpenMapFile();
void CloseMapFile();
void ClearMemory();
void ShutdownId();
void InitRedShifts();

void CAL_OptimizeNodes(
	huffnode* table);

void OpenAudioFile();
void ReadConfig();
void read_high_scores();

void initialize_sprites();
void initialize_states();
void initialize_audio_constants();
void initialize_songs();
void initialize_gfxv_contants();
void initialize_tp_shape_table();
void initialize_tp_animation_table();
void initialize_static_info_constants();
void initialize_weapon_constants();
void initialize_grenade_shape_constants();
void initialize_static_health_table();
void initialize_boss_constants();
void initialize_messages();
void initialize_ca_constants();

void sd_setup_digi();


static std::uint8_t wolfdigimap[] = {
	// These first sounds are in the upload version

	ATKIONCANNONSND, 0,
	ATKCHARGEDSND, 1,
	ATKBURSTRIFLESND, 2,
	ATKGRENADESND, 46,

	OPENDOORSND, 3,
	CLOSEDOORSND, 4,
	HTECHDOOROPENSND, 5,
	HTECHDOORCLOSESND, 6,

	INFORMANTDEATHSND, 7,
	SCIENTISTHALTSND, 19,
	SCIENTISTDEATHSND, 20,

	GOLDSTERNHALTSND, 8,
	GOLDSTERNLAUGHSND, 24,

	HALTSND, 9, // Rent-A-Cop 1st sighting
	RENTDEATH1SND, 10, // Rent-A-Cop Death

	EXPLODE1SND, 11,

	GGUARDHALTSND, 12,
	GGUARDDEATHSND, 17,

	PROHALTSND, 16,
	PROGUARDDEATHSND, 13,

	BLUEBOYDEATHSND, 18,
	BLUEBOYHALTSND, 51,

	SWATHALTSND, 22,
	SWATDIESND, 47,

	SCANHALTSND, 15,
	SCANDEATHSND, 23,

	PODHATCHSND, 26,
	PODHALTSND, 50,
	PODDEATHSND, 49,

	ELECTSHOTSND, 27,

	DOGBOYHALTSND, 14,
	DOGBOYDEATHSND, 21,
	ELECARCDAMAGESND, 25,
	ELECAPPEARSND, 28,
	ELECDIESND, 29,

	INFORMDEATH2SND, 39, // Informant Death #2
	RENTDEATH2SND, 34, // Rent-A-Cop Death #2
	PRODEATH2SND, 42, // PRO Death #2
	SWATDEATH2SND, 48, // SWAT Death #2
	SCIDEATH2SND, 53, // Gen. Sci Death #2

	LIQUIDDIESND, 30,

	GURNEYSND, 31,
	GURNEYDEATHSND, 41,

	WARPINSND, 32,
	WARPOUTSND, 33,

	EXPLODE2SND, 35,

	LCANHALTSND, 36,
	LCANDEATHSND, 37,

	// RENTDEATH3SND, 38, // Rent-A-Cop Death #3
	INFORMDEATH3SND, 40, // Informant Death #3
	PRODEATH3SND, 43, // PRO Death #3
	SWATDEATH3SND, 52, // Swat Guard #3
	SCIDEATH3SND, 54, // Gen. Sci Death #3

	LCANBREAKSND, 44,
	SCANBREAKSND, 45,
	CLAWATTACKSND, 56,
	SPITATTACKSND, 55,
	PUNCHATTACKSND, 57,

	LASTSOUND,
};


extern const std::uint8_t colormap[16896];
const std::uint8_t* lightsource;


namespace
{


int get_vgahead_offset_count()
{
	const auto& assets_info = AssetsInfo{};

	auto file_stream = bstone::FileStream{};
	const auto& base_name = Assets::get_gfx_header_base_name();
	const auto& file_extension = assets_info.get_extension();
	const auto is_open = ca_open_resource_non_fatal(base_name, file_extension, file_stream);

	if (!is_open)
	{
		return 0;
	}

	auto file_size = file_stream.get_size();

	if ((file_size % FILEPOSSIZE) != 0)
	{
		return 0;
	}

	return static_cast<int>(file_size / FILEPOSSIZE);
}

bool check_vgahead_offset_count()
{
	const auto& assets_info = AssetsInfo{};
	const auto offset_count = get_vgahead_offset_count();
	return offset_count == assets_info.get_gfx_header_offset_count();
}

class AddSearchPathException :
	public bstone::Exception
{
public:
	explicit AddSearchPathException(
		const char* const message)
		:
		bstone::Exception{std::string{"[ADD_SEARCH_PATH] "} + message}
	{
	}
}; // AddSearchPathException

struct SearchPath
{
	std::string source_name_;
	std::string path_;
}; // SearchPath

using SearchPaths = std::vector<SearchPath>;

using Games = std::vector<Game>;

struct FoundContent
{
	Game game_;
	AssetsVersion version_;
	const SearchPath* search_path_;
}; // FoundContent

using FoundContents = std::vector<FoundContent>;


void add_search_path(
	const std::string& source_name,
	const std::string& path,
	SearchPaths& search_paths)
{
	if (source_name.empty())
	{
		throw AddSearchPathException{"Empty source name."};
	}

	if (path.empty())
	{
		throw AddSearchPathException{"Empty path."};
	}

	search_paths.emplace_back();
	auto& search_path = search_paths.back();
	search_path.source_name_ = source_name;
	search_path.path_ = path;
}

bool has_content(
	const SearchPath& search_path,
	const AssetsCRefStrings& base_names,
	const std::string& extension)
{
	for (const auto& base_name : base_names)
	{
		const auto& path = bstone::file_system::append_path(search_path.path_, base_name.get() + extension);

		if (!bstone::file_system::has_file(path))
		{
			return false;
		}
	}

	return true;
}

bool has_content(
	const SearchPath& search_path,
	const AssetsCRefStrings& base_names,
	const std::string& extension,
	const AssetsBaseNameToHashMap& hash_map)
{
	if (!has_content(search_path, base_names, extension))
	{
		return false;
	}

	for (const auto& base_name : base_names)
	{
		const auto& hash = ca_calculate_hash(
			search_path.path_,
			base_name,
			extension
		);

		if (hash.empty())
		{
			return false;
		}

		if (hash_map.count(base_name) != 1)
		{
			return false;
		}

		if (hash_map.at(base_name) != hash)
		{
			return false;
		}
	}

	return true;
}

FoundContent find_aog_content(
	const SearchPath& search_path)
{
	const auto& assets = Assets{};

	auto result = FoundContent{};
	result.game_ = Game::aog;
	result.search_path_ = &search_path;

	// v2.1
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_full_base_names(),
			assets.get_aog_full_extension(),
			assets.get_aog_full_v2_1_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_full_v2_1;
			return result;
		}
	}

	// v3.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_full_base_names(),
			assets.get_aog_full_extension(),
			assets.get_aog_full_v3_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_full_v3_0;
			return result;
		}
	}

	// v2.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_full_base_names(),
			assets.get_aog_full_extension(),
			assets.get_aog_full_v2_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_full_v2_0;
			return result;
		}
	}

	// v1.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_full_base_names(),
			assets.get_aog_full_extension(),
			assets.get_aog_full_v1_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_full_v1_0;
			return result;
		}
	}

	return FoundContent{};
}

FoundContent find_aog_sw_content(
	const SearchPath& search_path)
{
	const auto& assets = Assets{};

	auto result = FoundContent{};
	result.game_ = Game::aog_sw;
	result.search_path_ = &search_path;

	// v2.1
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_sw_base_names(),
			assets.get_aog_sw_extension(),
			assets.get_aog_sw_v2_1_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_sw_v2_1;
			return result;
		}
	}

	// v3.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_sw_base_names(),
			assets.get_aog_sw_extension(),
			assets.get_aog_sw_v3_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_sw_v3_0;
			return result;
		}
	}

	// v2.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_sw_base_names(),
			assets.get_aog_sw_extension(),
			assets.get_aog_sw_v2_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_sw_v2_0;
			return result;
		}
	}

	// v1.0
	{
		const auto is_match = has_content(
			search_path,
			assets.get_aog_sw_base_names(),
			assets.get_aog_sw_extension(),
			assets.get_aog_sw_v1_0_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::aog_sw_v1_0;
			return result;
		}
	}

	return FoundContent{};
}

FoundContent find_ps_content(
	const SearchPath& search_path)
{
	const auto& assets = Assets{};

	auto result = FoundContent{};
	result.game_ = Game::ps;
	result.search_path_ = &search_path;

	{
		const auto is_match = has_content(
			search_path,
			assets.get_ps_base_names(),
			assets.get_ps_extension(),
			assets.get_ps_base_name_to_hash_map()
		);

		if (is_match)
		{
			result.version_ = AssetsVersion::ps;
			return result;
		}
	}


	return FoundContent{};
}

FoundContent find_assets(
	const Game game,
	const SearchPath& search_path)
{
	const auto& assets = Assets{};

	switch (game)
	{
		case Game::aog:
			return find_aog_content(search_path);

		case Game::aog_sw:
			return find_aog_sw_content(search_path);

		case Game::ps:
			return find_ps_content(search_path);

		default:
			Quit("Unsupported game.");
	}
}

const char* get_content_acronym(
	const Game game)
{
	switch (game)
	{
		case Game::aog:
			return "AOG";

		case Game::aog_sw:
			return "AOG (SW)";

		case Game::ps:
			return "PS";

		default:
			Quit("Unsupported game.");
	}
}

const FoundContent* choose_content(
	const FoundContents& found_contents)
{
	if (found_contents.empty())
	{
		Quit("No content.");
	}

	if (found_contents.size() == 1)
	{
		return found_contents.data();
	}

	bstone::logger_->write("Found multiple contents.");

	using SdlButton = SDL_MessageBoxButtonData;
	using SdlButtons = std::vector<SdlButton>;

	auto sdl_buttons = SdlButtons{};
	sdl_buttons.reserve(found_contents.size() + 1);

	auto button_id = 0;

	for (const auto& found_content : found_contents)
	{
		sdl_buttons.emplace_back();
		auto& sdl_button = sdl_buttons.back();
		sdl_button.flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		sdl_button.buttonid = button_id++;
		sdl_button.text = get_content_acronym(found_content.game_);
	}

	{
		sdl_buttons.emplace_back();
		auto& sdl_button = sdl_buttons.back();
		sdl_button.buttonid = -1;
		sdl_button.text = "Cancel";
	}

	auto sdl_message_box_data = SDL_MessageBoxData{};
	sdl_message_box_data.flags = SDL_MESSAGEBOX_INFORMATION;
	sdl_message_box_data.title = "BStone";
	sdl_message_box_data.message = "Select content to play.";
	sdl_message_box_data.numbuttons = static_cast<int>(sdl_buttons.size());
	sdl_message_box_data.buttons = sdl_buttons.data();

	auto selected_button_id = -1;

	bstone::logger_->write("Waiting for user response.");

	const auto sdl_result = SDL_ShowMessageBox(&sdl_message_box_data, &selected_button_id);

	if (sdl_result != 0)
	{
		const auto sdl_error = SDL_GetError();
		Quit(std::string{"Failed to show message box. "} + sdl_error);
	}

	if (selected_button_id < 0)
	{
		return nullptr;
	}

	return &found_contents[selected_button_id];
}

void set_assets_info(
	const FoundContent& found_content)
{
	const auto& assets = Assets{};
	auto assets_info = AssetsInfo{};

	switch (found_content.game_)
	{
		case Game::aog:
			assets_info.set_base_names(assets.get_aog_full_base_names());
			assets_info.set_extension(assets.get_aog_full_extension());

			switch (found_content.version_)
			{
				case AssetsVersion::aog_full_v1_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_full_v1_0_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_full_v2_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_full_v2_0_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_full_v2_1:
					assets_info.set_base_name_to_hash_map(assets.get_aog_full_v2_1_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_full_v3_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_full_v3_0_base_name_to_hash_map());
					break;

				default:
					Quit("Unsupported game version.");
			}

			break;

		case Game::aog_sw:
			assets_info.set_base_names(assets.get_aog_sw_base_names());
			assets_info.set_extension(assets.get_aog_sw_extension());

			switch (found_content.version_)
			{
				case AssetsVersion::aog_sw_v1_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_sw_v1_0_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_sw_v2_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_sw_v2_0_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_sw_v2_1:
					assets_info.set_base_name_to_hash_map(assets.get_aog_sw_v2_1_base_name_to_hash_map());
					break;

				case AssetsVersion::aog_sw_v3_0:
					assets_info.set_base_name_to_hash_map(assets.get_aog_sw_v3_0_base_name_to_hash_map());
					break;

				default:
					Quit("Unsupported game version.");
			}
			break;

		case Game::ps:
			assets_info.set_base_names(assets.get_ps_base_names());
			assets_info.set_extension(assets.get_ps_extension());
			assets_info.set_base_name_to_hash_map(assets.get_ps_base_name_to_hash_map());
			break;

		default:
			Quit("Unsupported game.");
	}

	assets_info.set_version(found_content.version_);
}

void log_found_content(
	const FoundContent& found_content)
{
	const auto& assets_info = AssetsInfo{};

	static const auto aog_title = std::string{"Aliens Of Gold"};
	static const auto aog_sw_title = std::string{"Aliens Of Gold (shareware)"};
	static const auto ps_title = std::string{"Planet Strike"};

	static const auto v1_0 = std::string{"v1.0"};
	static const auto v2_0 = std::string{"v2.0"};
	static const auto v2_1 = std::string{"v2.1"};
	static const auto v3_0 = std::string{"v3.0"};

	auto title = std::string{};
	auto version = std::string{};

	switch (found_content.version_)
	{
		case AssetsVersion::aog_sw_v1_0:
			title = aog_sw_title;
			version = v1_0;
			break;

		case AssetsVersion::aog_sw_v2_0:
			title = aog_sw_title;
			version = v2_0;
			break;

		case AssetsVersion::aog_sw_v2_1:
			title = aog_sw_title;
			version = v2_1;
			break;

		case AssetsVersion::aog_sw_v3_0:
			title = aog_sw_title;
			version = v3_0;
			break;

		case AssetsVersion::aog_full_v1_0:
			title = aog_title;
			version = v1_0;
			break;

		case AssetsVersion::aog_full_v2_0:
			title = aog_title;
			version = v2_0;
			break;

		case AssetsVersion::aog_full_v2_1:
			title = aog_title;
			version = v2_1;
			break;

		case AssetsVersion::aog_full_v3_0:
			title = aog_title;
			version = v3_0;
			break;

		case AssetsVersion::ps:
			title = ps_title;
			break;

		default:
			Quit("Unsupported version.");
	}

	auto name_and_version = std::string{"\"Blake Stone: "};
	name_and_version += (title.empty() ? "???" : title);
	name_and_version += '\"';

	if (!version.empty())
	{
		name_and_version += ' ';
		name_and_version += version;
	}

	bstone::logger_->write("Found " + name_and_version + '.');
	bstone::logger_->write("Content source: " + found_content.search_path_->source_name_);
}

void find_contents()
{
	bstone::logger_->write("Looking for game content.");

	// Games to find.
	//
	const auto force_aog = g_args.has_option("aog");
	const auto force_aog_sw = g_args.has_option("aog_sw");
	const auto force_ps = g_args.has_option("ps");

	const auto forced_game_count = force_aog + force_aog_sw + force_ps;

	if (forced_game_count > 1)
	{
		Quit("Multiple game modes defined.");
	}

	auto games_to_find = Games{};

	if (forced_game_count == 0)
	{
		games_to_find = {Game::aog, Game::aog_sw, Game::ps};
	}
	else
	{
		if (force_aog)
		{
			games_to_find = {Game::aog};
		}
		else if (force_aog_sw)
		{
			games_to_find = {Game::aog_sw};
		}
		else
		{
			games_to_find = {Game::ps};
		}
	}


	// Paths to search.
	//
	const auto& working_full_dir = bstone::file_system::append_path_separator(
		bstone::file_system::get_working_dir());

	const auto& data_full_dir = bstone::file_system::append_path_separator(
		data_dir_.empty() ? working_full_dir : bstone::file_system::resolve_path(data_dir_));

	auto search_paths = SearchPaths{};

	if (data_full_dir != working_full_dir)
	{
		add_search_path("custom dir", data_full_dir, search_paths);
	}
	else
	{
		{
			add_search_path("working dir", working_full_dir, search_paths);
		}

		{
			const auto& source_name = std::string{"GOG"};
			const auto& content_path = bstone::make_content_path(bstone::ContentPathProvider::gog);

			if (!content_path.aog_.empty())
			{
				add_search_path(source_name, content_path.aog_, search_paths);
			}

			if (!content_path.ps_.empty())
			{
				add_search_path(source_name, content_path.ps_, search_paths);
			}
		}

		{
			const auto& source_name = std::string{"Steam"};
			const auto& content_path = bstone::make_content_path(bstone::ContentPathProvider::steam);

			if (!content_path.aog_.empty())
			{
				add_search_path(source_name, content_path.aog_, search_paths);
			}

			if (!content_path.ps_.empty())
			{
				add_search_path(source_name, content_path.ps_, search_paths);
			}
		}
	}


	// Find contents.
	//
	auto found_contents = FoundContents{};

	for (const auto& search_path : search_paths)
	{
		for (const auto game : games_to_find)
		{
			const auto already_found = std::any_of(
				found_contents.cbegin(),
				found_contents.cend(),
				[game](const auto& item)
				{
					return item.game_ == game;
				}
			);

			if (already_found)
			{
				continue;
			}

			const auto& found_content = find_assets(game, search_path);

			if (found_content.game_ != Game::none &&
				found_content.search_path_ != nullptr &&
				found_content.version_ != AssetsVersion::none)
			{
				found_contents.push_back(found_content);
			}
		}
	}

	if (found_contents.empty())
	{
		Quit("Content not found.");
	}

	const auto content = choose_content(found_contents);

	if (content == nullptr)
	{
		bstone::logger_->write("Cancelled by user.");

		Quit();
	}

	set_assets_info(*content);

	data_dir_ = content->search_path_->path_;

	log_found_content(*content);
}


} // namespace


// ------------------ ID Software 'startup' functions ---------------------


/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const float radtoint = static_cast<float>(FINEANGLES / 2 / PI);


void BuildTables()
{
	std::int16_t i;
	float angle, anglestep;
	double tang;
	fixed value;


	//
	// calculate fine tangents
	//

	for (i = 0; i < FINEANGLES / 8; i++)
	{
		tang = tan((i + 0.5) / radtoint);
		finetangent[i] = static_cast<std::int32_t>(tang * TILEGLOBAL);
		finetangent[FINEANGLES / 4 - 1 - i] = static_cast<std::int32_t>(1 / tang * TILEGLOBAL);
	}

	//
	// costable overlays sintable with a quarter phase shift
	// ANGLES is assumed to be divisable by four
	//
	// The low word of the value is the fraction, the high bit is the sign bit,
	// bits 16-30 should be 0
	//

	angle = 0.0F;
	anglestep = static_cast<float>(PI / 2 / ANGLEQUAD);
	for (i = 0; i <= ANGLEQUAD; i++)
	{
		value = static_cast<fixed>(GLOBAL1 * sin(static_cast<double>(angle)));
		sintable[i] =
			sintable[i + ANGLES] =
			sintable[ANGLES / 2 - i] = value;
		sintable[ANGLES - i] =
			sintable[ANGLES / 2 + i] = value | 0x80000000l;
		angle += anglestep;
	}

	lightsource = colormap;
}

// Map tile values to scaled pics
void SetupWalls()
{
	//
	// Hey! Yea You! This is where you can VERY easly setup to use a
	// specific 'BANK' of wall graphics.... JTR
	//

	for (int i = 1; i < MAXWALLTILES; ++i)
	{
		horizwall[i] = static_cast<std::int16_t>((i - 1) * 2);
		vertwall[i] = horizwall[i] + 1;
	}

	WallHeight().swap(wallheight);
	wallheight.resize(vga_width);


	const int k_half_height = vga_height / 2;

	SpanStart().swap(spanstart);
	spanstart.resize(k_half_height);

	StepScale().swap(stepscale);
	stepscale.resize(k_half_height);

	BaseDist().swap(basedist);
	basedist.resize(k_half_height);

	PlaneYLookup().swap(planeylookup);
	planeylookup.resize(k_half_height);

	MirrorOfs().swap(mirrorofs);
	mirrorofs.resize(k_half_height);
}

void InitDigiMap()
{
	char* map;

	for (map = reinterpret_cast<char*>(wolfdigimap); *map != LASTSOUND; map += 2)
	{
		sd_digi_map_[static_cast<int>(map[0])] = map[1];
	}
}

void CAL_SetupAudioFile()
{
	bstone::FileStream handle;

	//
	// load maphead.ext (offsets and tileinfo for map file)
	//
#ifndef AUDIOHEADERLINKED
	ca_open_resource(Assets::get_audio_header_base_name(), handle);
	auto length = static_cast<std::int32_t>(handle.get_size());
	audiostarts = new std::int32_t[length / 4];
	handle.read(audiostarts, length);
	handle.close();
#else
	// TODO Remove or fix
	audiohuffman = (huffnode*)&audiodict;
	CAL_OptimizeNodes(audiohuffman);
	audiostarts = (std::int32_t*)FP_SEG(&audiohead);
#endif

	//
	// open the data file
	//
	OpenAudioFile();
}

void CAL_SetupGrFile()
{
	if (!check_vgahead_offset_count())
	{
		Quit("Mismatch GFX header offset count.");
	}

	bstone::FileStream handle;
	std::uint8_t* compseg;

	//
	// load ???dict.ext (huffman dictionary for graphics files)
	//

	ca_open_resource(Assets::get_gfx_dictionary_base_name(), handle);
	handle.read(&grhuffman, sizeof(grhuffman));

	//
	// load the data offsets from ???head.ext
	//
	int grstarts_size = (NUMCHUNKS + 1) * FILEPOSSIZE;

	grstarts = new std::int32_t[(grstarts_size + 3) / 4];

	ca_open_resource(Assets::get_gfx_header_base_name(), handle);
	handle.read(grstarts, grstarts_size);

	//
	// Open the graphics file, leaving it open until the game is finished
	//
	ca_open_resource(Assets::get_gfx_data_base_name(), grhandle);

	//
	// load the pic and sprite headers into the arrays in the data segment
	//
	pictable = new pictabletype[NUMPICS];
	CAL_GetGrChunkLength(STRUCTPIC); // position file pointer
	compseg = new std::uint8_t[chunkcomplen];
	grhandle.read(compseg, chunkcomplen);

	CAL_HuffExpand(
		compseg,
		reinterpret_cast<std::uint8_t*>(pictable),
		NUMPICS * sizeof(pictabletype),
		grhuffman);

	delete[] compseg;
}

static void cal_setup_map_data_file()
{
	auto assets_info = AssetsInfo{};

	auto has_mod = false;

	if (!mod_dir_.empty())
	{
		const auto& modded_hash = ca_calculate_hash(
			mod_dir_, Assets::get_map_data_base_name(), assets_info.get_extension());

		if (!modded_hash.empty())
		{
			has_mod = true;

			const auto are_official_levels = Assets::are_official_levels(modded_hash);

			if (are_official_levels && modded_hash != assets_info.get_levels_hash_string())
			{
				Quit("Mismatch official levels are not allowed in the mod directory.");
			}

			assets_info.set_levels_hash(modded_hash);
		}
	}

	if (!has_mod)
	{
		const auto& official_hash = assets_info.get_base_name_to_hash_map().at(Assets::get_map_data_base_name());

		assets_info.set_levels_hash(official_hash);
	}

	OpenMapFile();
}

void CAL_SetupMapFile()
{
	std::int16_t i;
	bstone::FileStream handle;
	std::int32_t pos;
	mapfiletype header;
	maptype* map_header;

	cal_setup_map_data_file();

	//
	// load maphead.ext (offsets and tileinfo for map file)
	//

	ca_open_resource(Assets::get_map_header_base_name(), handle);
	handle.read(&header.RLEWtag, sizeof(header.RLEWtag));
	handle.read(&header.headeroffsets, sizeof(header.headeroffsets));

	rlew_tag = header.RLEWtag;

	//
	// load all map header
	//
	const auto& assets_info = AssetsInfo{};

	const auto total_levels = assets_info.get_total_levels();

	for (i = 0; i < total_levels; ++i)
	{
		pos = header.headeroffsets[i];

		if (pos < 0)
		{
			continue;
		}

		mapheaderseg[i] = new maptype();
		map_header = mapheaderseg[i];

		maphandle.set_position(pos);

		maphandle.read(
			&map_header->planestart,
			sizeof(map_header->planestart));

		maphandle.read(
			&map_header->planelength,
			sizeof(map_header->planelength));

		maphandle.read(
			&map_header->width,
			sizeof(map_header->width));

		maphandle.read(
			&map_header->height,
			sizeof(map_header->height));

		maphandle.read(
			&map_header->name,
			sizeof(map_header->name));
	}

	//
	// allocate space for 3 64*64 planes
	//
	for (i = 0; i < MAPPLANES; ++i)
	{
		mapsegs[i] = new std::uint16_t[64 * 64];
	}
}


// --------------------- Other general functions ------------------------

extern CP_itemtype NewEmenu[];
extern std::int16_t EpisodeSelect[];


void CheckForEpisodes()
{
	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog_full())
	{
		for (int i = 1; i < 6; ++i)
		{
			NewEmenu[i].active = AT_ENABLED;
			EpisodeSelect[i] = 1;
		}
	}
}


extern const char* MainStrs[];
extern char bc_buffer[];


void PreDemo()
{
	if (g_no_intro_outro || g_no_screens)
	{
		return;
	}

	vid_is_movie = true;

	VL_SetPaletteIntensity(0, 255, vgapal, 0);

	const auto& assets_info = AssetsInfo{};

	if (assets_info.is_aog_full())
	{
		// ---------------------
		// Anti-piracy screen
		// ---------------------

		// Cache pic
		//
		CA_CacheScreen(PIRACYPIC);

		// Cache and set palette.  AND  Fade it in!
		//
		CA_CacheGrChunk(PIRACYPALETTE);
		VL_SetPalette(0, 256, static_cast<const std::uint8_t*>(grsegs[PIRACYPALETTE]));
		VL_SetPaletteIntensity(0, 255, static_cast<const std::uint8_t*>(grsegs[PIRACYPALETTE]), 0);
		VW_UpdateScreen();

		VL_FadeOut(0, 255, 0, 0, 25, 20);
		VL_FadeIn(0, 255, static_cast<const std::uint8_t*>(grsegs[PIRACYPALETTE]), 30);

		// Wait a little
		//
		IN_UserInput(TickBase * 20);

		// Free palette
		//
		UNCACHEGRCHUNK(PIRACYPALETTE);

		VL_FadeOut(0, 255, 0, 0, 25, 20);
		VW_FadeOut();

		// Cleanup screen for upcoming SetPalette call
		//
		VL_Bar(0, 0, 320, 200, 0);
	}

	// ---------------------
	// Apogee presents
	// ---------------------

	// Cache pic
	//
	CA_CacheScreen(APOGEEPIC);

	// Load and start music
	//
	CA_CacheAudioChunk(STARTMUSIC + APOGFNFM_MUS);

	sd_start_music(APOGFNFM_MUS);

	// Cache and set palette.  AND  Fade it in!
	//
	CA_CacheGrChunk(APOGEEPALETTE);
	VL_SetPalette(0, 256, static_cast<const std::uint8_t*>(grsegs[APOGEEPALETTE]));
	VL_SetPaletteIntensity(0, 255, static_cast<const std::uint8_t*>(grsegs[APOGEEPALETTE]), 0);
	VW_UpdateScreen();
	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 0, 0, 0, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 25, 29, 53, 20);
	}
	VL_FadeIn(0, 255, static_cast<const std::uint8_t*>(grsegs[APOGEEPALETTE]), 30);

	// Wait for end of fanfare
	//
	if (sd_is_music_enabled_)
	{
		IN_StartAck();
		while ((!sd_sq_played_once_) && (!IN_CheckAck()))
		{
		}
	}
	else
	{
		IN_UserInput(TickBase * 6);
	}

	sd_music_off();

	// Free palette and music.  AND  Restore palette
	//
	UNCACHEGRCHUNK(APOGEEPALETTE);

	delete[] audiosegs[STARTMUSIC + APOGFNFM_MUS];
	audiosegs[STARTMUSIC + APOGFNFM_MUS] = nullptr;

	if (assets_info.is_ps())
	{
		// Do A Blue Flash!
		VL_FadeOut(0, 255, 25, 29, 53, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 0, 0, 0, 30);
	}

	// ---------------------
	// JAM logo intro
	// ---------------------

	// Load and start music
	//
	CA_CacheAudioChunk(STARTMUSIC + TITLE_LOOP_MUSIC);
	sd_start_music(TITLE_LOOP_MUSIC);

	// Show JAM logo
	//
	if (!DoMovie(MovieId::intro))
	{
		Quit("JAM animation (IANIM.xxx) does not exist.");
	}

	// ---------------------
	// PC-13
	// ---------------------
	VL_Bar(0, 0, 320, 200, 0x14);
	CacheDrawPic(0, 64, PC13PIC);
	VW_UpdateScreen();
	VW_FadeIn();
	IN_UserInput(TickBase * 2);

	// Do A Red Flash!

	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 39, 0, 0, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 0, 0, 0, 20);
	}

	VW_FadeOut();

	vid_is_movie = false;
}

void check_for_extract_options()
{
	{
		const auto& extract_all_option_name = std::string{"extract_all"};

		if (g_args.has_option(extract_all_option_name))
		{
			const auto& dst_dir = g_args.get_option_value(extract_all_option_name);

			ca_extract_all(dst_dir);

			Quit();
			return;
		}
	}

	{
		const auto& extract_walls_option_name = std::string{"extract_walls"};

		if (g_args.has_option(extract_walls_option_name))
		{
			const auto& dst_dir = g_args.get_option_value(extract_walls_option_name);

			ca_extract_walls(dst_dir);

			Quit();
			return;
		}
	}

	{
		const auto& extract_sprites_option_name = std::string{"extract_sprites"};

		if (g_args.has_option(extract_sprites_option_name))
		{
			const auto& dst_dir = g_args.get_option_value(extract_sprites_option_name);

			ca_extract_sprites(dst_dir);

			Quit();
		}
	}

	{
		const auto& extract_musics_option_name = std::string{"extract_music"};

		if (g_args.has_option(extract_musics_option_name))
		{
			const auto& dst_dir = g_args.get_option_value(extract_musics_option_name);

			ca_extract_music(dst_dir);

			Quit();
		}
	}

	{
		const auto& extract_sfx_option_name = std::string{"extract_sfx"};

		if (g_args.has_option(extract_sfx_option_name))
		{
			const auto& extract_dir = g_args.get_option_value(extract_sfx_option_name);

			ca_extract_sfx(extract_dir);

			Quit();
		}
	}

	{
		const auto& extract_texts_option_name = std::string{"extract_texts"};

		if (g_args.has_option(extract_texts_option_name))
		{
			const auto& dst_dir = g_args.get_option_value(extract_texts_option_name);

			ca_extract_texts(dst_dir);

			Quit();
		}
	}
}

void InitGame()
{
	vid_is_movie = true;

	std::int16_t i, x, y;
	std::uint16_t* blockstart;

	CA_Startup();
	PM_Startup();

	check_for_extract_options();

	ReadConfig();
	read_high_scores();

	VW_Startup();
	IN_Startup();
	sd_startup();
	US_Startup();

	VL_SetPalette(0, 256, vgapal);

	//
	// build some tables
	//

	InitDigiMap();

	for (i = 0; i < MAPSIZE; i++)
	{
		nearmapylookup[i] = &tilemap[0][0] + MAPSIZE * i;
		farmapylookup[i] = i * 64;
	}

	for (i = 0; i < PORTTILESHIGH; i++)
	{
		uwidthtable[i] = UPDATEWIDE * i;
	}

	blockstart = &blockstarts[0];
	for (y = 0; y < UPDATEHIGH; y++)
	{
		for (x = 0; x < UPDATEWIDE; x++)
		{
			*blockstart++ = SCREENWIDTH * 16 * y + x * TILEWIDTH;
		}
	}

	updateptr = &update[0];

	bufferofs = 0;


	//
	// load in and lock down some basic chunks
	//

	LoadFonts();

	LoadLatchMem();
	BuildTables(); // trig tables
	SetupWalls();
	NewViewSize();

	//
	// initialize variables
	//

	InitRedShifts();

	vid_is_movie = false;
}

std::uint16_t scan_atoi(
	const char* s)
{
	while (*s && (!isdigit(*s)))
	{ // First scans for a digit...
		s++;
	}

	return static_cast<std::uint16_t>(atoi(s)); // Then converts to integer...
}


extern const char* MainStrs[];
extern std::int16_t starting_episode, starting_level, starting_difficulty;


static void output_version()
{
	const auto& version_string = bstone::Version::get_string();
	const auto message = "BStone v" + version_string + '.';

	// Standard output.
	//
	std::cout << message << '\n';

	// Message box.
	//
	static_cast<void>(SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_INFORMATION,
		"BStone",
		message.c_str(),
		nullptr)
	);
}

void freed_main()
{
	if (g_args.has_option("version"))
	{
		output_version();
		Quit();
	}

	// Setup for APOGEECD thingie.
	//
	InitDestPath();

	find_contents();

	bstone::logger_->write();
	bstone::logger_->write("Data path: \"" + data_dir_ + "\"");
	bstone::logger_->write("Mod path: \"" + mod_dir_ + "\"");
	bstone::logger_->write("Profile path: \"" + get_profile_dir() + "\"");

	// BBi
	{
		if (g_args.has_option("calculate_hashes"))
		{
			ca_calculate_hashes();
			Quit();
		}
	}

	// Make sure there's room to play the game
	//
	CheckDiskSpace(DISK_SPACE_NEEDED, CANT_PLAY_TXT, cds_dos_print);

	// Which version is this? (SHAREWARE? 1-3? 1-6?)
	//
	CheckForEpisodes();

	// BBi
	initialize_sprites();
	initialize_gfxv_contants();
	initialize_states();
	initialize_tp_shape_table();
	initialize_tp_animation_table();
	initialize_audio_constants();
	initialize_songs();
	initialize_static_info_constants();
	initialize_weapon_constants();
	initialize_grenade_shape_constants();
	initialize_static_health_table();
	initialize_boss_constants();
	initialize_messages();
	initialize_ca_constants();
	gamestuff.initialize();
	gamestate.initialize();

	if (g_args.has_option("no_screens"))
	{
		g_no_screens = true;
	}

	if (g_args.has_option("cheats"))
	{
		DebugOk = true;
	}

	InitGame();

	PreDemo();
}
