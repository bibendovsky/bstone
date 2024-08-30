/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"

#include "bstone_exception_utils.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_version.h"
#include "bstone_content_path.h"
#include "bstone_fs_utils.h"
#include "bstone_sys_message_box.h"


extern SpanStart spanstart;
extern BaseDist basedist;
extern PlaneYLookup planeylookup;
extern MirrorOfs mirrorofs;


void CA_CannotOpen(
	const std::string& string);

void CAL_GetGrChunkLength(
	std::int16_t chunk);

void CA_CacheScreen(
	std::int16_t chunk);

void IN_StartAck();
bool IN_CheckAck();
void OpenMapFile();
void CloseMapFile();
void ClearMemory();
void ShutdownId();
void InitRedShifts();

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
void initialize_hit_point_table();
void initialize_boss_constants();
void initialize_messages();
void initialize_ca_constants();

void InitPlaytemp();


extern const std::uint8_t colormap[16896];
const std::uint8_t* lightsource;


const std::string& get_message_box_title()
{
	static const auto result = std::string{} + "BStone v" + bstone::get_version().string_short;

	return result;
}


namespace
{

int get_vgahead_offset_count()
{
	const auto& assets_info = get_assets_info();
	const auto& assets_resource = assets_info.find_resource(AssetsResourceType::vgahead);

	auto file_stream = bstone::FileStream{};
	const auto is_open = ca_open_resource_non_fatal(assets_resource.file_name, file_stream);

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
	const auto& assets_info = get_assets_info();
	const auto offset_count = get_vgahead_offset_count();
	return offset_count == assets_info.get_gfx_header_offset_count();
}


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
try {
	if (source_name.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty source name.");
	}

	if (path.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty path.");
	}

	search_paths.emplace_back();
	auto& search_path = search_paths.back();
	search_path.source_name_ = source_name;
	search_path.path_ = path;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool has_resources(
	const SearchPath& search_path,
	const AssetsResources& assets_resources)
{
	auto file_stream = bstone::FileStream{};

	for (const auto& assets_resource : assets_resources)
	{
		const auto is_resource_open = ca_open_resource_non_fatal(
			search_path.path_,
			assets_resource.file_name,
			file_stream
		);

		if (!is_resource_open)
		{
			return false;
		}
	}

	return true;
}

bool has_content(
	const SearchPath& search_path,
	const AssetsResources& assets_resources)
{
	if (!has_resources(search_path, assets_resources))
	{
		return false;
	}

	for (const auto& assets_resource : assets_resources)
	{
		const auto& hash = ca_calculate_hash(
			search_path.path_,
			assets_resource.file_name
		);

		if (hash != assets_resource.hash_string)
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
			assets.get_aog_full_v2_1_resources()
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
			assets.get_aog_full_v3_0_resources()
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
			assets.get_aog_full_v2_0_resources()
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
			assets.get_aog_full_v1_0_resources()
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
			assets.get_aog_sw_v2_1_resources()
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
			assets.get_aog_sw_v3_0_resources()
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
			assets.get_aog_sw_v2_0_resources()
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
			assets.get_aog_sw_v1_0_resources()
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
			assets.get_ps_resources()
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
	switch (game)
	{
		case Game::aog:
			return find_aog_content(search_path);

		case Game::aog_sw:
			return find_aog_sw_content(search_path);

		case Game::ps:
			return find_ps_content(search_path);

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported game.");
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
			BSTONE_THROW_STATIC_SOURCE("Unsupported game.");
	}
}

const FoundContent* choose_content(
	const FoundContents& found_contents)
{
	if (found_contents.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("No content.");
	}

	if (found_contents.size() == 1)
	{
		return found_contents.data();
	}

	bstone::globals::logger->log_information("Found multiple contents.");

	using Button = bstone::sys::MessageBoxButton;
	using LocalButtons = std::vector<Button>;

	auto buttons = LocalButtons{};
	buttons.reserve(found_contents.size() + 1);

	auto button_id = 0;

	for (const auto& found_content : found_contents)
	{
		buttons.emplace_back();
		auto& button = buttons.back();
		button.id = button_id++;
		button.text = get_content_acronym(found_content.game_);
	}

	{
		buttons.emplace_back();
		auto& button = buttons.back();
		button.id = -1;
		button.flags = bstone::sys::MessageBoxButtonFlags::default_for_escape_key;
		button.text = "Cancel";
	}

	auto descriptor = bstone::sys::MessageBoxInitParam{};
	descriptor.title = get_message_box_title().c_str();
	descriptor.message = "Select content to play.";
	descriptor.type = bstone::sys::MessageBoxType::information;
	descriptor.buttons = bstone::make_const_span(buttons.data(), static_cast<std::intptr_t>(buttons.size()));

	bstone::globals::logger->log_information("Waiting for user response.");
	const auto selected_button_id = bstone::sys::MessageBox::show(descriptor);

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
	auto& assets_info = get_assets_info();

	switch (found_content.game_)
	{
		case Game::aog:
			switch (found_content.version_)
			{
				case AssetsVersion::aog_full_v1_0:
					assets_info.set_resources(assets.get_aog_full_v1_0_resources());
					break;

				case AssetsVersion::aog_full_v2_0:
					assets_info.set_resources(assets.get_aog_full_v2_0_resources());
					break;

				case AssetsVersion::aog_full_v2_1:
					assets_info.set_resources(assets.get_aog_full_v2_1_resources());
					break;

				case AssetsVersion::aog_full_v3_0:
					assets_info.set_resources(assets.get_aog_full_v3_0_resources());
					break;

				default:
					BSTONE_THROW_STATIC_SOURCE("Unsupported game version.");
			}

			break;

		case Game::aog_sw:
			switch (found_content.version_)
			{
				case AssetsVersion::aog_sw_v1_0:
					assets_info.set_resources(assets.get_aog_sw_v1_0_resources());
					break;

				case AssetsVersion::aog_sw_v2_0:
					assets_info.set_resources(assets.get_aog_sw_v2_0_resources());
					break;

				case AssetsVersion::aog_sw_v2_1:
					assets_info.set_resources(assets.get_aog_sw_v2_1_resources());
					break;

				case AssetsVersion::aog_sw_v3_0:
					assets_info.set_resources(assets.get_aog_sw_v3_0_resources());
					break;

				default:
					BSTONE_THROW_STATIC_SOURCE("Unsupported game version.");
			}
			break;

		case Game::ps:
			assets_info.set_resources(assets.get_ps_resources());
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported game.");
	}

	assets_info.set_version(found_content.version_);
}

void log_found_content(
	const FoundContent& found_content)
{
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
			BSTONE_THROW_STATIC_SOURCE("Unsupported version.");
	}

	auto name_and_version = std::string{"\"Blake Stone: "};
	name_and_version += (title.empty() ? "???" : title);
	name_and_version += '\"';

	if (!version.empty())
	{
		name_and_version += ' ';
		name_and_version += version;
	}

	bstone::globals::logger->log_information(("Found " + name_and_version + '.').c_str());
	bstone::globals::logger->log_information(("Content source: " + found_content.search_path_->source_name_).c_str());
}

void find_contents()
{
	bstone::globals::logger->log_information("Looking for game content.");

	// Games to find.
	//
	const auto force_aog = g_args.has_option("aog");
	const auto force_aog_sw = g_args.has_option("aog_sw");
	const auto force_ps = g_args.has_option("ps");

	const auto forced_game_count = force_aog + force_aog_sw + force_ps;

	if (forced_game_count > 1)
	{
		BSTONE_THROW_STATIC_SOURCE("Multiple game modes defined.");
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
	auto search_paths = SearchPaths{};

	if (is_data_dir_custom_)
	{
		add_search_path("custom dir", data_dir_, search_paths);
	}

	{
		const auto working_full_dir = bstone::fs_utils::append_path_separator(
			bstone::fs_utils::get_working_dir());

		add_search_path("working dir", working_full_dir, search_paths);
	}

	{
		const auto profile_full_dir = get_profile_dir();

		add_search_path("profile dir", profile_full_dir, search_paths);
	}
		
	{
		const auto source_name = std::string{"GOG"};
		const auto content_path = bstone::make_content_path(bstone::ContentPathProvider::gog);

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
		const auto source_name = std::string{"Steam"};
		const auto content_path = bstone::make_content_path(bstone::ContentPathProvider::steam);

		if (!content_path.aog_.empty())
		{
			add_search_path(source_name, content_path.aog_, search_paths);
		}

		if (!content_path.ps_.empty())
		{
			add_search_path(source_name, content_path.ps_, search_paths);
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
		BSTONE_THROW_STATIC_SOURCE("Content not found.");
	}

	const auto content = choose_content(found_contents);

	if (content == nullptr)
	{
		bstone::globals::logger->log_information("Cancelled by user.");

		Quit();
	}

	set_assets_info(*content);

	data_dir_ = bstone::fs_utils::normalize_path(
		bstone::fs_utils::append_path_separator(
			content->search_path_->path_));

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
void BuildTables()
{
	//
	// calculate fine tangents
	//

	for (auto i = 0; i < (FINEANGLES / 8); ++i)
	{
		const auto tang = std::tan((i + 0.5) / radtoint);

		finetangent[i] = tang;
		finetangent[(FINEANGLES / 4) - 1 - i] = 1.0 / tang;
	}

	//
	// costable overlays sintable with a quarter phase shift
	// ANGLES is assumed to be divisable by four
	//
	// The low word of the value is the fraction, the high bit is the sign bit,
	// bits 16-30 should be 0
	//

	auto angle = 0.0;
	auto anglestep = bstone::math::pi() / 2.0 / ANGLEQUAD;

	for (auto i = 0; i <= ANGLEQUAD; ++i)
	{
		const auto value = std::sin(angle);

		sintable[i] = value;
		sintable[i + ANGLES] = value;
		sintable[ANGLES / 2 - i] = value;

		sintable[ANGLES - i] = -value;
		sintable[ANGLES / 2 + i] = -value;

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

	BaseDist().swap(basedist);
	basedist.resize(k_half_height);

	PlaneYLookup().swap(planeylookup);
	planeylookup.resize(k_half_height);

	MirrorOfs().swap(mirrorofs);
	mirrorofs.resize(k_half_height);
}

void CAL_SetupGrFile()
{
	if (!check_vgahead_offset_count())
	{
		BSTONE_THROW_STATIC_SOURCE("Mismatch GFX header offset count.");
	}

	bstone::FileStream handle;

	//
	// load ???dict.ext (huffman dictionary for graphics files)
	//

	ca_open_resource(AssetsResourceType::vgadict, handle);
	handle.read(&grhuffman, sizeof(grhuffman));

	//
	// load the data offsets from ???head.ext
	//
	int grstarts_size = (NUMCHUNKS + 1) * FILEPOSSIZE;

	grstarts.resize((grstarts_size + 3) / 4);

	ca_open_resource(AssetsResourceType::vgahead, handle);
	handle.read(grstarts.data(), grstarts_size);

	//
	// Open the graphics file, leaving it open until the game is finished
	//
	ca_open_resource(AssetsResourceType::vgagraph, grhandle);

	//
	// load the pic and sprite headers into the arrays in the data segment
	//
	pictable.resize(NUMPICS);
	CAL_GetGrChunkLength(STRUCTPIC); // position file pointer

	auto compseg = Buffer{};
	compseg.resize(chunkcomplen);

	grhandle.read(compseg.data(), chunkcomplen);

	CAL_HuffExpand(
		compseg.data(),
		reinterpret_cast<std::uint8_t*>(pictable.data()),
		NUMPICS * sizeof(pictabletype),
		grhuffman);
}

static void cal_setup_map_data_file()
{
	auto& assets_info = get_assets_info();

	auto has_mod = false;

	if (!mod_dir_.empty())
	{
		const auto& modded_hash = ca_calculate_hash(mod_dir_, AssetsResourceType::maptemp);

		if (!modded_hash.empty())
		{
			has_mod = true;

			const auto are_official_levels = Assets::are_official_levels(modded_hash);

			if (are_official_levels && modded_hash != assets_info.get_levels_hash_string())
			{
				BSTONE_THROW_STATIC_SOURCE("Mismatch official levels are not allowed in the mod directory.");
			}

			assets_info.set_levels_hash(modded_hash);
		}
	}

	if (!has_mod)
	{
		const auto& assets_resource = assets_info.find_resource(AssetsResourceType::maptemp);
		assets_info.set_levels_hash(assets_resource.hash_string);
	}

	OpenMapFile();
}

void CAL_SetupMapFile()
{
	std::int16_t i;
	bstone::FileStream handle;
	std::int32_t pos;
	auto header = mapfiletype{};
	maptype* map_header;

	cal_setup_map_data_file();

	//
	// load maphead.ext (offsets and tileinfo for map file)
	//

	ca_open_resource(AssetsResourceType::maphead, handle);
	handle.read(&header.RLEWtag, sizeof(header.RLEWtag));
	handle.read(&header.headeroffsets, sizeof(header.headeroffsets));

	rlew_tag = header.RLEWtag;

	//
	// load all map header
	//
	const auto& assets_info = get_assets_info();

	const auto total_levels = assets_info.get_total_levels();

	for (i = 0; i < total_levels; ++i)
	{
		pos = header.headeroffsets[i];

		if (pos < 0)
		{
			continue;
		}

		mapheaderseg[i] = maptype{};
		map_header = &mapheaderseg[i];

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
		mapsegs[i].resize(64 * 64);
	}
}


// --------------------- Other general functions ------------------------

void CheckForEpisodes()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_full())
	{
		menu_enable_all_episodes();
	}
}


extern const char* MainStrs[];
extern char bc_buffer[];


void PreDemo()
{
	if (gp_no_intro_outro() || gp_no_screens())
	{
		return;
	}

	vid_is_movie = true;

	VL_SetPaletteIntensity(vgapal, 0);

	const auto& assets_info = get_assets_info();

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
		VL_SetPalette(0, 256, grsegs[PIRACYPALETTE].data());
		VL_SetPaletteIntensity(grsegs[PIRACYPALETTE].data(), 0);
		VW_UpdateScreen();

		VL_FadeOut(0, 255, 0, 0, 25, 20);
		VL_FadeIn(0, 255, grsegs[PIRACYPALETTE].data(), 30);

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

	sd_start_music(APOGFNFM_MUS, false);

	// Cache and set palette.  AND  Fade it in!
	//
	CA_CacheGrChunk(APOGEEPALETTE);
	VL_SetPalette(0, 256, grsegs[APOGEEPALETTE].data());
	VL_SetPaletteIntensity(grsegs[APOGEEPALETTE].data(), 0);
	VW_UpdateScreen();
	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 0, 0, 0, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 25, 29, 53, 20);
	}
	VL_FadeIn(0, 255, grsegs[APOGEEPALETTE].data(), 30);

	// Wait for end of fanfare
	//
	if (sd_is_music_enabled())
	{
		IN_StartAck();
		while (sd_is_music_playing() && (!IN_CheckAck()))
		{
		}
	}
	else
	{
		IN_UserInput(TickBase * 6);
	}

	IN_ClearKeysDown();

	sd_music_off();

	// Free palette and music.  AND  Restore palette
	//
	UNCACHEGRCHUNK(APOGEEPALETTE);

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
	sd_start_music(TITLE_LOOP_MUSIC);

	IN_ClearKeysDown();

	// Show JAM logo
	//
	if (!DoMovie(MovieId::intro))
	{
		BSTONE_THROW_STATIC_SOURCE("JAM animation (IANIM.xxx) does not exist.");
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
		constexpr auto extract_all_option_name_sv = bstone::StringView{"extract_all"};

		if (g_args.has_option(extract_all_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_all_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_all(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_vga_palette_option_name_sv = bstone::StringView{"extract_vga_palette"};

		if (g_args.has_option(extract_vga_palette_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_vga_palette_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_vga_palette(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_walls_option_name_sv = bstone::StringView{"extract_walls"};

		if (g_args.has_option(extract_walls_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_walls_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_walls(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_sprites_option_name_sv = bstone::StringView{"extract_sprites"};

		if (g_args.has_option(extract_sprites_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_sprites_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_sprites(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_musics_option_name_sv = bstone::StringView{"extract_music"};

		if (g_args.has_option(extract_musics_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_musics_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_music(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_sfx_option_name_sv = bstone::StringView{"extract_sfx"};

		if (g_args.has_option(extract_sfx_option_name_sv))
		{
			const auto extract_dir_sv = g_args.get_option_value(extract_sfx_option_name_sv);
			const auto extract_dir = std::string{extract_dir_sv.cbegin(), extract_dir_sv.cend()};
			ca_extract_sfx(extract_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_texts_option_name_sv = bstone::StringView{"extract_texts"};

		if (g_args.has_option(extract_texts_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_texts_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_texts(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_levels_option_name_sv = bstone::StringView{"extract_levels"};

		if (g_args.has_option(extract_levels_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_levels_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_levels(dst_dir);
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
	bstone::globals::page_mgr = bstone::make_page_mgr();

	check_for_extract_options();
	read_high_scores();
	InitPlaytemp();

	sd_startup();
	IN_Startup();
	VW_Startup();
	US_Startup();

	VL_SetPalette(0, 256, vgapal);

	//
	// build some tables
	//

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

	in_grab_mouse(true);
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
	const auto version_string = bstone::get_version().string;
	const auto message = std::string{} + "BStone v" + version_string + '.';

	// Standard output.
	//
	std::cout << message << '\n';

	// Message box.
	//
	try
	{
		bstone::sys::MessageBox::show_simple(
			get_message_box_title().c_str(),
			message.c_str(),
			bstone::sys::MessageBoxType::information);
	}
	catch (...)
	{
		const auto error_message = bstone::get_nested_message();
		bstone::globals::logger->log_error(error_message.c_str());
	}
}

namespace {

void deserialize_cvars_from_cli(const bstone::Cl& args, bstone::CVarMgr& cvar_mgr)
{
	for (const auto& option : args.get_options())
	{
		const auto cvar = cvar_mgr.find(option.name);

		if (cvar == nullptr)
		{
			continue;
		}

		if (option.args.get_size() != 1)
		{
			auto message = std::string{};
			message += "Expected one argument for option \"";
			message.append(option.name.get_data(), static_cast<std::size_t>(option.name.get_size()));
			message += "\".";
			bstone::globals::logger->log_warning(message.c_str());
			continue;
		}

		cvar->set_string(option.args.get_front());
	}
}

} // namespace

void freed_main()
{
	if (g_args.has_option("version"))
	{
		output_version();
		Quit();
	}

	vid_initialize_cvars(*bstone::globals::cvar_mgr);
	sd_initialize_cvars(*bstone::globals::cvar_mgr);
	gp_initialize_cvars(*bstone::globals::cvar_mgr);
	am_initialize_cvars(*bstone::globals::cvar_mgr);
	in_initialize_cvars(*bstone::globals::cvar_mgr);
	in_initialize_ccmds(*bstone::globals::ccmd_mgr);

	// Setup for APOGEECD thingie.
	//
	InitDestPath();
	find_contents();

	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information(("Data path: \"" + data_dir_ + "\"").c_str());
	bstone::globals::logger->log_information(("Mod path: \"" + mod_dir_ + "\"").c_str());
	bstone::globals::logger->log_information(("Profile path: \"" + get_profile_dir() + "\"").c_str());
	bstone::globals::logger->log_information(("Screenshot path: \"" + get_screenshot_dir() + "\"").c_str());

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
	ReadConfig();
	deserialize_cvars_from_cli(g_args, *bstone::globals::cvar_mgr);

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
	initialize_hit_point_table();
	initialize_messages();
	initialize_ca_constants();

	gamestuff.initialize();
	old_gamestuff = gamestuff;

	gamestate.initialize();
	old_gamestate = gamestate;

	if (g_args.has_option("cheats"))
	{
		DebugOk = true;
	}

	InitGame();

	PreDemo();
}
