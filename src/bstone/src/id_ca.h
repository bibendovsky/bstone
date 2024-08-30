/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_ID_CA_INCLUDED
#define BSTONE_ID_CA_INCLUDED


#include <memory>

#include "3d_def.h"
#include "bstone_file_stream.h"
#include "bstone_span.h"


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
	std::uint16_t width;
	std::uint16_t height;
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


enum class AssetsResourceType
{
	none,
	audiohed,
	audiot,
	eanim,
	ganim,
	ianim,
	maphead,
	maptemp,
	sanim,
	vgadict,
	vgagraph,
	vgahead,
	vswap,
}; // AssetsResourceType

struct AssetsResource
{
	AssetsResourceType type{};
	const char* file_name{};
	const char* hash_string{};
}; // AssetsResource

using AssetsResources = bstone::Span<const AssetsResource>;

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


class AssetsInfo
{
public:
	AssetsVersion get_version() const;

	void set_version(
		const AssetsVersion version);


	const AssetsResources& get_resources() const noexcept;

	void set_resources(
		const AssetsResources& resources) noexcept;

	const AssetsResource& find_resource(
		AssetsResourceType resource_type) const;


	const std::string& get_levels_hash_string() const;

	void set_levels_hash(
		const std::string& levels_hash);


	const std::string& get_base_path_name() const noexcept;


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

	int get_barrier_switches_per_level() const noexcept;

	int get_max_barrier_switches_per_level_bits() const noexcept;

	bool is_secret_level(
		const int level_number) const;

	int secret_floor_get_index(
		const int level_number) const;


private:
	static const char* get_resource_type_string(
		AssetsResourceType resource_type) noexcept;


	AssetsVersion version_{};
	AssetsResources resources_{};
	std::string levels_hash_{};
	std::string base_path_name_{};
	int gfx_header_offset_count_{};
	bool are_modded_levels_{};
	int episode_count_{};
	int levels_per_episode_{};
	int stats_levels_per_episode_{};
	int total_levels_{};
	int barrier_switches_per_level_{};
	int max_barrier_switches_per_level_bits_{};
}; // AssetsInfo


AssetsInfo& get_assets_info();


class Assets
{
public:
	static constexpr auto max_size = 4000000;


	static bool are_official_levels(
		const std::string& levels_hash);


	static const AssetsResources& get_aog_sw_v1_0_resources();

	static const AssetsResources& get_aog_sw_v2_0_resources();

	static const AssetsResources& get_aog_sw_v2_1_resources();

	static const AssetsResources& get_aog_sw_v3_0_resources();


	static const AssetsResources& get_aog_full_v1_0_resources();

	static const AssetsResources& get_aog_full_v2_0_resources();

	static const AssetsResources& get_aog_full_v2_1_resources();

	static const AssetsResources& get_aog_full_v3_0_resources();


	static const AssetsResources& get_ps_resources();


	static const AssetsResources& get_all_resources();


private:
	// Aliens Of Gold (shareware)

	static constexpr const char* audiohed_bs1 = "AUDIOHED.BS1";
	static constexpr const char* audiot_bs1 = "AUDIOT.BS1";
	static constexpr const char* ianim_bs1 = "IANIM.BS1";
	static constexpr const char* maphead_bs1 = "MAPHEAD.BS1";
	static constexpr const char* maptemp_bs1 = "MAPTEMP.BS1";
	static constexpr const char* sanim_bs1 = "SANIM.BS1";
	static constexpr const char* vgadict_bs1 = "VGADICT.BS1";
	static constexpr const char* vgagraph_bs1 = "VGAGRAPH.BS1";
	static constexpr const char* vgahead_bs1 = "VGAHEAD.BS1";
	static constexpr const char* vswap_bs1 = "VSWAP.BS1";

	// Aliens Of Gold (registered)

	static constexpr const char* audiohed_bs6 = "AUDIOHED.BS6";
	static constexpr const char* audiot_bs6 = "AUDIOT.BS6";
	static constexpr const char* eanim_bs6 = "EANIM.BS6";
	static constexpr const char* ganim_bs6 = "GANIM.BS6";
	static constexpr const char* ianim_bs6 = "IANIM.BS6";
	static constexpr const char* maphead_bs6 = "MAPHEAD.BS6";
	static constexpr const char* maptemp_bs6 = "MAPTEMP.BS6";
	static constexpr const char* sanim_bs6 = "SANIM.BS6";
	static constexpr const char* vgadict_bs6 = "VGADICT.BS6";
	static constexpr const char* vgagraph_bs6 = "VGAGRAPH.BS6";
	static constexpr const char* vgahead_bs6 = "VGAHEAD.BS6";
	static constexpr const char* vswap_bs6 = "VSWAP.BS6";

	// Planet Strike

	static constexpr const char* audiohed_vsi = "AUDIOHED.VSI";
	static constexpr const char* audiot_vsi = "AUDIOT.VSI";
	static constexpr const char* eanim_vsi = "EANIM.VSI";
	static constexpr const char* ianim_vsi = "IANIM.VSI";
	static constexpr const char* maphead_vsi = "MAPHEAD.VSI";
	static constexpr const char* maptemp_vsi = "MAPTEMP.VSI";
	static constexpr const char* vgadict_vsi = "VGADICT.VSI";
	static constexpr const char* vgagraph_vsi = "VGAGRAPH.VSI";
	static constexpr const char* vgahead_vsi = "VGAHEAD.VSI";
	static constexpr const char* vswap_vsi = "VSWAP.VSI";


	using AllResources = std::vector<AssetsResource>;

	static AllResources make_all_resources();
}; // Assets

// ===========================================================================

using GrSegment = Buffer;
using GrSegments = std::vector<GrSegment>;

using GrNeeded = Buffer;
using GrSegmentSizes = std::vector<int>;

using MapHeaderSegments = std::vector<maptype>;


extern std::uint16_t rlew_tag;

extern std::int16_t mapon;

using MapSegment = std::vector<std::uint16_t>;
using MapSegments = std::array<MapSegment, MAPPLANES>;
extern MapSegments mapsegs;

extern MapHeaderSegments mapheaderseg;
extern GrSegments grsegs;
extern GrSegmentSizes grsegs_sizes_;

extern GrNeeded grneeded;
extern std::uint8_t ca_levelbit, ca_levelnum;

extern char* titleptr[8];

extern std::int16_t profilehandle, debughandle;

using GrStarts = std::vector<std::int32_t>;
extern GrStarts grstarts; // array of offsets in egagraph, -1 for sparse

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

void CA_UpLevel();
void CA_DownLevel();

#define CA_MarkGrChunk(chunk) grneeded[chunk] |= ca_levelbit

void CA_CacheGrChunk(
	std::int16_t chunk);
void CA_CacheMap(int mapnum);

void CA_CacheMarks();

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
std::string ca_load_script(
	int chunk_id,
	bool strip_xx = false);

bool ca_open_resource_non_fatal(
	const std::string& data_dir,
	const std::string& file_name,
	bstone::FileStream& file_stream);

bool ca_open_resource_non_fatal(
	const std::string& file_name,
	bstone::FileStream& file_stream);

void ca_open_resource(
	AssetsResourceType assets_resource_type,
	bstone::FileStream& file_stream);

std::string ca_calculate_hash(
	const std::string& data_dir,
	const std::string& file_name);

std::string ca_calculate_hash(
	const std::string& data_dir,
	AssetsResourceType assets_resource_type);

void ca_calculate_hashes();

void ca_extract_vga_palette(
	const std::string& destination_dir);

void ca_extract_walls(
	const std::string& destination_dir);

void ca_extract_sprites(
	const std::string& destination_dir);

void ca_extract_music(
	const std::string& destination_dir);

void ca_extract_sfx(
	const std::string& destination_dir);

void ca_extract_texts(
	const std::string& destination_dir);

void ca_extract_levels(const std::string& destination_dir);

void ca_extract_all(
	const std::string& destination_dir);


void ca_make_resource_path(const std::string& resource_name,
	std::string& data_path,
	std::string& mod_path);

void ca_make_sprite_resource_path_name(
	int sprite_id,
	std::string& data_path,
	std::string& mod_path);

void ca_make_wall_resource_path_name(
	int wall_id,
	std::string& data_path,
	std::string& mod_path);

std::string ca_make_padded_asset_number_string(int number);

#endif // BSTONE_ID_CA_INCLUDED
