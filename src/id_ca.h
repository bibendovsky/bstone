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


#ifndef BSTONE_ID_CA_INCLUDED
#define BSTONE_ID_CA_INCLUDED


#include <unordered_map>
#include "3d_def.h"
#include "bstone_file_stream.h"


const int MAPPLANES = 2;


void UNCACHEGRCHUNK(
	int chunk);

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE 3
#else
#define FILEPOSSIZE 4
#endif

// ===========================================================================

struct maptype
{
	std::int32_t planestart[3];
	std::uint16_t planelength[3];
	std::uint16_t width, height;
	char name[16];
}; // maptype

struct huffnode
{
	std::uint16_t bit0, bit1; // 0-255 is a character, > is a pointer to a node
}; // huffnode

struct mapfiletype
{
	std::uint16_t RLEWtag;
	std::int32_t headeroffsets[100];
	//      std::uint8_t           tileinfo[];
}; // mapfiletype


using AssetsCRefString = std::reference_wrapper<const std::string>;

using AssetsCRefStrings = std::vector<AssetsCRefString>;

using AssetsBaseNameToHashMap = std::unordered_map<
	AssetsCRefString,
	std::string,
	std::hash<std::string>,
	std::equal_to<std::string>>;


enum class AssetsVersion
{
	none,
	aog_sw_v1_0, // Aliens Of Gold (shareware, v1.0)
	aog_sw_v2_0, // Aliens Of Gold (shareware, v2.0)
	aog_sw_v2_1, // Aliens Of Gold (shareware, v2.1)
	aog_sw_v3_0, // Aliens Of Gold (shareware, v3.0)
	aog_full_v1_0, // Aliens Of Gold (full, v1.0)
	aog_full_v2_0, // Aliens Of Gold (full, v2.0)
	aog_full_v2_1, // Aliens Of Gold (full, v2.1)
	aog_full_v3_0, // Aliens Of Gold (full, v3.0)
	ps, // Planet Strike (full, v1.0/v1.1)
}; // AssetsVersion


struct AssetsInfo final
{
public:
	AssetsVersion get_version() const;

	void set_version(
		const AssetsVersion version);


	const std::string& get_extension() const;

	void set_extension(
		const std::string& extension);


	const AssetsCRefStrings& get_base_names() const;

	void set_base_names(
		const AssetsCRefStrings& base_names);


	const AssetsBaseNameToHashMap& get_base_name_to_hash_map() const;

	void set_base_name_to_hash_map(
		const AssetsBaseNameToHashMap& base_name_to_hash_map);


	const std::string& get_levels_hash_string() const;

	void set_levels_hash(
		const std::string& levels_hash);


	int get_gfx_header_offset_count() const;

	bool are_modded_levels() const;


	bool is_aog_full_v1_0() const;

	bool is_aog_full_v2_0() const;

	bool is_aog_full_v2_1() const;

	bool is_aog_full_v2_x() const;

	bool is_aog_full_v3_0() const;

	bool is_aog_full() const;

	bool is_aog_sw_v1_0() const;

	bool is_aog_sw_v2_0() const;

	bool is_aog_sw_v2_1() const;

	bool is_aog_sw_v2_x() const;

	bool is_aog_sw_v3_0() const;

	bool is_aog_sw() const;

	bool is_aog() const;

	bool is_ps() const;

	int get_episode_count() const;

	int get_levels_per_episode() const;

	int get_stats_levels_per_episode() const;

	int get_total_levels() const;

	bool is_secret_level(
		const int level_number) const;


private:
	static std::string empty_extension_;
	static AssetsVersion version_;
	static AssetsCRefString extension_;
	static AssetsCRefStrings base_names_;
	static AssetsBaseNameToHashMap base_name_to_hash_map_;
	static std::string levels_hash_;
	static int gfx_header_offset_count_;
	static bool are_modded_levels_;
	static int episode_count_;
	static int levels_per_episode_;
	static int stats_levels_per_episode_;
	static int total_levels_;
	static int min_secret_level_index_;
	static int max_secret_level_index_;
}; // AssetsInfo


struct Assets final
{
	static constexpr auto max_size = 4000000;


	static const std::string& get_audio_header_base_name();
	static const std::string& get_audio_data_base_name();

	static const std::string& get_map_header_base_name();
	static const std::string& get_map_data_base_name();

	static const std::string& get_gfx_dictionary_base_name();
	static const std::string& get_gfx_header_base_name();
	static const std::string& get_gfx_data_base_name();

	static const std::string& get_page_file_base_name();

	static const std::string& get_episode_6_fmv_base_name();
	static const std::string& get_episode_3_5_fmv_base_name();
	static const std::string& get_intro_fmv_base_name();
	static const std::string& get_episode_2_4_fmv_base_name();

	static const std::string& get_aog_sw_extension();
	static const std::string& get_aog_full_extension();
	static const std::string& get_ps_extension();


	static const AssetsCRefStrings& get_extensions();

	static const AssetsCRefStrings& get_base_names();


	static const AssetsCRefStrings& get_aog_sw_base_names();

	static const AssetsCRefStrings& get_aog_full_base_names();

	static const AssetsCRefStrings& get_ps_base_names();


	static const AssetsBaseNameToHashMap& get_aog_sw_v1_0_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_sw_v2_0_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_sw_v2_1_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_sw_v3_0_base_name_to_hash_map();


	static const AssetsBaseNameToHashMap& get_aog_full_v1_0_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_full_v2_0_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_full_v2_1_base_name_to_hash_map();

	static const AssetsBaseNameToHashMap& get_aog_full_v3_0_base_name_to_hash_map();


	static const AssetsBaseNameToHashMap& get_ps_base_name_to_hash_map();


	static bool are_official_levels(
		const std::string& levels_hash);
}; // Assets

// ===========================================================================

using AudioSegments = std::vector<std::uint8_t*>;
using GrSegments = std::vector<void*>;
using GrNeeded = std::vector<std::uint8_t>;
using MapHeaderSegments = std::vector<maptype*>;


extern std::uint16_t rlew_tag;

extern std::int16_t mapon;

extern std::uint16_t* mapsegs[MAPPLANES];
extern MapHeaderSegments mapheaderseg;
extern AudioSegments audiosegs;
extern GrSegments grsegs;

extern GrNeeded grneeded;
extern std::uint8_t ca_levelbit, ca_levelnum;

extern char* titleptr[8];

extern std::int16_t profilehandle, debughandle;

extern std::int32_t* grstarts; // array of offsets in egagraph, -1 for sparse
extern std::int32_t* audiostarts; // array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern void(*drawcachebox)(
	char* title,
	std::uint16_t numcache);
extern void(*updatecachebox)();
extern void(*finishcachebox)();

extern bstone::FileStream grhandle;
extern bstone::FileStream maphandle;
extern bstone::FileStream audiohandle;

extern std::int32_t chunkcomplen;
extern std::int32_t chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode* grhuffman;
#else
extern huffnode grhuffman[255];
#endif


// ===========================================================================

void CA_RLEWexpand(
	std::uint16_t* source,
	std::uint16_t* dest,
	std::int32_t length,
	std::uint16_t rlewtag);

void CA_Startup();
void CA_Shutdown();

void CA_CacheAudioChunk(
	std::int16_t chunk);
void CA_LoadAllSounds();

void CA_UpLevel();
void CA_DownLevel();

#define CA_MarkGrChunk(chunk) grneeded[chunk] |= ca_levelbit

void CA_CacheGrChunk(
	std::int16_t chunk);
void CA_CacheMap(
	std::int16_t mapnum);

void CA_CacheMarks();

void CAL_SetupAudioFile();
void CAL_SetupGrFile();
void CAL_SetupMapFile();

void CAL_HuffExpand(
	std::uint8_t* source,
	std::uint8_t* dest,
	std::int32_t length,
	huffnode* hufftable);

void ca_huff_expand_on_screen(
	std::uint8_t* source,
	huffnode* hufftable);

void CloseGrFile();

// BBi
extern int ca_gr_last_expanded_size;

std::string ca_load_script(
	int chunk_id,
	bool strip_xx = false);

bool ca_is_resource_exists(
	const std::string& file_name);

bool ca_open_resource_non_fatal(
	const std::string& data_dir,
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream);

bool ca_open_resource_non_fatal(
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream);

void ca_open_resource(
	const std::string& file_name_without_ext,
	bstone::FileStream& file_stream);

std::string ca_calculate_hash(
	const std::string& data_dir,
	const std::string& base_name,
	const std::string& extension);

void ca_dump_hashes();

void ca_dump_walls_images(
	const std::string& destination_dir);

void ca_dump_sprites_images(
	const std::string& destination_dir);


#endif // BSTONE_ID_CA_INCLUDED
