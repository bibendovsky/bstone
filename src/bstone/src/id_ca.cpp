/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_ca.h"

#include <cassert>
#include <cmath>

#include <array>
#include <algorithm>
#include <memory>

#include "id_heads.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "gfxv.h"

#include "bstone_audio_content_mgr.h"
#include "bstone_audio_extractor.h"
#include "bstone_binary_reader.h"
#include "bstone_binary_writer.h"
#include "bstone_bmp_image_common.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_image_extractor.h"
#include "bstone_level_extractor.h"
#include "bstone_logger.h"
#include "bstone_memory_stream.h"
#include "bstone_rlew_decoder.h"
#include "bstone_sha1.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"
#include "bstone_text_extractor.h"

#include "bstone_opl3.h"
#include "bstone_audio_decoder.h"


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

std::uint16_t rlew_tag;

std::int16_t mapon;

MapSegments mapsegs;
MapHeaderSegments mapheaderseg;
GrSegments grsegs;
GrSegmentSizes grsegs_sizes_;

GrNeeded grneeded;
std::uint8_t ca_levelbit, ca_levelnum;

std::int16_t profilehandle, debughandle;


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

extern std::int32_t CGAhead;
extern std::int32_t EGAhead;
extern std::uint8_t CGAdict;
extern std::uint8_t EGAdict;
extern std::uint8_t maphead;
extern std::uint8_t mapdict;


void CA_CannotOpen(
	const std::string& string);

GrStarts grstarts; // array of offsets in egagraph, -1 for sparse

#ifdef GRHEADERLINKED
huffnode* grhuffman;
#else
huffnode grhuffman[255];
#endif

bstone::FileStream grhandle; // handle to EGAGRAPH
bstone::FileStream maphandle; // handle to MAPTEMP / GAMEMAPS

std::int32_t chunkcomplen;
std::int32_t chunkexplen;

static Buffer ca_buffer;

static const int BUFFERSIZE = 0x10000;


// BBi
std::string ca_make_padded_asset_number_string(
	int number)
{
	return bstone::StringHelper::make_left_padded_with_zero(number, 8);
}

void CAL_CarmackExpand(
	std::uint16_t* source,
	std::uint16_t* dest,
	std::uint16_t length);


#ifdef THREEBYTEGRSTARTS
std::int32_t GRFILEPOS(
	std::int16_t c)
{
	std::int32_t value;
	std::int16_t offset;

	offset = c * 3;

	value = *(std::int32_t*)(((std::uint8_t*)grstarts.data()) + offset);

	value &= 0x00ffffffl;

	if (value == 0xffffffl)
	{
		value = -1;
	}

	return value;
}
#else
#define GRFILEPOS(c) (grstarts[c])
#endif

void CloseGrFile()
{
	grhandle.close();
}

void OpenMapFile()
{
	// TODO Remove or fix
#ifdef CARMACIZED
	strcpy(fname, "GAMEMAPS.");
	strcat(fname, extension);

	if ((maphandle = open(fname,
		O_RDONLY | O_BINARY, S_IREAD)) == -1)
	{
		CA_CannotOpen(fname);
	}
#else
	ca_open_resource(AssetsResourceType::maptemp, maphandle);
#endif
}

void CloseMapFile()
{
	maphandle.close();
}


/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/
void CAL_GetGrChunkLength(
	std::int16_t chunk)
{
	grhandle.set_position(GRFILEPOS(chunk));
	grhandle.read(&chunkexplen, sizeof(chunkexplen));

	chunkcomplen = GRFILEPOS(chunk + 1) - GRFILEPOS(chunk) - 4;
}


/*
============================================================================

				COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
= If screenhack, the data is decompressed in four planes directly
= to the screen
=
======================
*/
void CAL_HuffExpand(
	std::uint8_t* source,
	std::uint8_t* destination,
	std::int32_t length,
	huffnode* hufftable)
{
	std::uint8_t val = *source++;
	std::uint8_t mask = 1;
	std::uint16_t nodeval;

	huffnode* headptr = &hufftable[254]; // head node is always node 254

	std::uint8_t* dst = destination;
	std::uint8_t* end = dst + length;

	huffnode* huffptr = headptr;

	while (dst < end)
	{
		if ((val & mask) == 0)
		{
			nodeval = huffptr->bit0;
		}
		else
		{
			nodeval = huffptr->bit1;
		}

		if (mask == 0x80)
		{
			val = *source++;
			mask = 1;
		}
		else
		{
			mask <<= 1;
		}

		if (nodeval < 256)
		{
			dst[0] = static_cast<std::uint8_t>(nodeval);
			++dst;
			huffptr = headptr;
		}
		else
		{
			huffptr = &hufftable[nodeval - 256];
		}
	}
}

void ca_huff_expand_on_screen(
	std::uint8_t* source,
	huffnode* hufftable)
{
	std::uint8_t val = *source++;
	std::uint8_t mask = 1;
	std::uint16_t nodeval;

	huffnode* headptr = &hufftable[254]; // head node is always node 254
	huffnode* huffptr = headptr;

	for (int p = 0; p < 4; ++p)
	{
		int x = p;
		int y = 0;

		while (y < vga_ref_height)
		{
			if ((val & mask) == 0)
			{
				nodeval = huffptr->bit0;
			}
			else
			{
				nodeval = huffptr->bit1;
			}

			if (mask == 0x80)
			{
				val = *source++;
				mask = 1;
			}
			else
			{
				mask <<= 1;
			}

			if (nodeval < 256)
			{
				VL_Plot(x, y, static_cast<std::uint8_t>(nodeval));
				huffptr = headptr;

				x += 4;

				if (x >= vga_ref_width)
				{
					x = p;
					++y;
				}
			}
			else
			{
				huffptr = &hufftable[nodeval - 256];
			}
		}
	}
}

#ifdef CARMACIZED
/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/
void CAL_CarmackExpand(
	std::uint16_t* source,
	std::uint16_t* dest,
	std::uint16_t length)
{
#define NEARTAG 0xa7
#define FARTAG 0xa8

	std::uint16_t ch, chhigh, count, offset;
	std::uint16_t* copyptr, *inptr, *outptr;

	length /= 2;

	inptr = source;
	outptr = dest;

	while (length)
	{
		ch = *inptr++;
		chhigh = ch >> 8;
		if (chhigh == NEARTAG)
		{
			count = ch & 0xff;
			if (!count)
			{ // have to insert a word containing the tag byte
				ch |= *((std::uint8_t*)inptr)++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *((std::uint8_t*)inptr)++;
				copyptr = outptr - offset;
				length -= count;
				while (count--)
				{
					*outptr++ = *copyptr++;
				}
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch & 0xff;
			if (!count)
			{ // have to insert a word containing the tag byte
				ch |= *((std::uint8_t*)inptr)++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
				{
					*outptr++ = *copyptr++;
				}
			}
		}
		else
		{
			*outptr++ = ch;
			length--;
		}
	}
}

#endif

/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/
void CA_RLEWexpand(
	std::uint16_t* source,
	std::uint16_t* dest,
	std::int32_t length,
	std::uint16_t rlewtag)
{
	std::uint16_t i;
	std::uint16_t value;
	std::uint16_t count;
	const std::uint16_t* end = &dest[length / 2];

	do
	{
		value = *source++;

		if (value != rlewtag)
		{
			*dest++ = value;
		}
		else
		{
			count = *source++;
			value = *source++;

			for (i = 0; i < count; ++i)
			{
				*dest++ = value;
			}
		}
	} while (dest < end);
}

/*
=============================================================================

 CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/
void CA_Shutdown()
{
#ifdef PROFILE
	if (profilehandle != -1)
	{
		close(profilehandle);
		profilehandle = -1;
	}
#endif

	CloseMapFile();
	CloseGrFile();
}

/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/
void CA_Startup()
{
#ifdef PROFILE
	unlink("PROFILE.TXT");
	profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif

	CAL_SetupMapFile();
	CAL_SetupGrFile();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;

	ca_buffer.reserve(BUFFERSIZE);
}

// ===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk(
	std::int16_t chunk,
	std::uint8_t* source)
{
	std::int32_t expanded;

	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
		//
		// expanded sizes of tile8/16/32 are implicit
		//

		const int BLOCK = 64;
		const int MASKBLOCK = 128;

		if (chunk < STARTTILE8M)
		{ // tile 8s are all in one chunk!
			expanded = BLOCK * NUMTILE8;
		}
		else if (chunk < STARTTILE16)
		{
			expanded = MASKBLOCK * NUMTILE8M;
		}
		else if (chunk < STARTTILE16M)
		{      // all other tiles are one/chunk
			expanded = BLOCK * 4;
		}
		else if (chunk < STARTTILE32)
		{
			expanded = MASKBLOCK * 4;
		}
		else if (chunk < STARTTILE32M)
		{
			expanded = BLOCK * 16;
		}
		else
		{
			expanded = MASKBLOCK * 16;
		}
	}
	else
	{
		//
		// everything else has an explicit size longword
		//

		expanded = bstone::endian::to_little(*reinterpret_cast<std::int32_t*>(source));
		source += 4; // skip over length
	}

	//
	// allocate final space, decompress it, and free bigbuffer
	// Sprites need to have shifts made and various other junk
	//
	grsegs[chunk].resize(expanded);

	CAL_HuffExpand(source, grsegs[chunk].data(), expanded, grhuffman);

	grsegs_sizes_[chunk] = expanded;
}

/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/
void CA_CacheGrChunk(
	std::int16_t chunk)
{
	std::int32_t pos, compressed;
	std::uint8_t* source;
	std::int16_t next;

	grneeded[chunk] |= ca_levelbit; // make sure it doesn't get removed

	if (!grsegs[chunk].empty())
	{
		return; // already in memory

	}

	//
	// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
	// a larger buffer
	//
	pos = GRFILEPOS(chunk);
	if (pos < 0)
	{ // $FFFFFFFF start is a sparse tile
		return;
	}

	next = chunk + 1;
	while (GRFILEPOS(next) == -1)
	{ // skip past any sparse tiles
		next++;
	}

	compressed = GRFILEPOS(next) - pos;


	grhandle.set_position(pos);

	ca_buffer.resize(compressed);
	grhandle.read(ca_buffer.data(), compressed);
	source = ca_buffer.data();

	CAL_ExpandGrChunk(chunk, source);
}


/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/
void CA_CacheScreen(
	std::int16_t chunk)
{
	std::int32_t pos, compressed;
	std::uint8_t* source;
	std::int16_t next;


	//
	// load the chunk into a buffer
	//
	pos = GRFILEPOS(chunk);
	next = chunk + 1;
	while (GRFILEPOS(next) == -1)
	{ // skip past any sparse tiles
		next++;
	}
	compressed = GRFILEPOS(next) - pos;

	grhandle.set_position(pos);

	ca_buffer.resize(compressed);
	grhandle.read(ca_buffer.data(), compressed);
	source = ca_buffer.data();

	source += 4; // skip over length

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	ca_huff_expand_on_screen(source, grhuffman);
}

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/
void CA_CacheMap(int mapnum)
try
{
	if (mapheaderseg[mapnum].name[0] == '\0')
	{
		BSTONE_THROW_DYNAMIC_SOURCE(("Level " + std::to_string(mapnum) + " not found.").c_str());
	}

	mapon = static_cast<std::int16_t>(mapnum);

	constexpr auto plane_size = MAPSIZE * MAPSIZE;

	for (auto plane = 0; plane < MAPPLANES; ++plane)
	{
		const auto plane_offset = mapheaderseg[mapnum].planestart[plane];
		maphandle.set_position(plane_offset);

		const auto src_count = mapheaderseg[mapnum].planelength[plane];

		if (ca_buffer.size() < src_count)
		{
			ca_buffer.resize(src_count);
		}

		const auto src_bytes = ca_buffer.data();
		maphandle.read_exactly(src_bytes, src_count);

		const auto dst_words = mapsegs[plane].data();
		bstone::RlewDecoder::decode(rlew_tag, src_bytes, src_count, dst_words, plane_size);
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/
void CA_UpLevel()
{
	if (ca_levelnum == 7)
	{
		BSTONE_THROW_STATIC_SOURCE("Up past level 7.");
	}

	ca_levelbit <<= 1;
	ca_levelnum++;
}

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/
void CA_DownLevel()
{
	if (!ca_levelnum)
	{
		BSTONE_THROW_STATIC_SOURCE("Down past level 0.");
	}

	ca_levelbit >>= 1;
	ca_levelnum--;
	CA_CacheMarks();
}

void CA_CacheMarks()
{
	const int MAXEMPTYREAD = 1024;

	std::int16_t i;
	std::int16_t next;
	std::int16_t numcache;
	std::int32_t pos;
	std::int32_t endpos;
	std::int32_t nextpos;
	std::int32_t nextendpos;
	std::int32_t compressed;
	std::int32_t bufferstart;
	std::int32_t bufferend; // file position of general buffer
	std::uint8_t* source;

	numcache = 0;
	//
	// go through and make everything not needed purgable
	//
	for (i = 0; i < NUMCHUNKS; i++)
	{
		if (grneeded[i] & ca_levelbit)
		{
			if (!grsegs[i].empty())
			{ // its already in memory, make
			}
			else
			{
				numcache++;
			}
		}
	}

	if (!numcache)
	{ // nothing to cache!
		return;
	}


	//
	// go through and load in anything still needed
	//
	bufferstart = bufferend = 0; // nothing good in buffer now

	for (i = 0; i < NUMCHUNKS; i++)
	{
		if ((grneeded[i] & ca_levelbit) && grsegs[i].empty())
		{
			pos = GRFILEPOS(i);
			if (pos < 0)
			{
				continue;
			}

			next = i + 1;
			while (GRFILEPOS(next) == -1)
			{ // skip past any sparse tiles
				next++;
			}

			compressed = GRFILEPOS(next) - pos;
			endpos = pos + compressed;

			if (bufferstart <= pos && bufferend >= endpos)
			{
				// data is already in buffer
				source = ca_buffer.data() + (pos - bufferstart);
			}
			else
			{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
				while (next < NUMCHUNKS)
				{
					while (next < NUMCHUNKS &&
						!(grneeded[next] & ca_levelbit && grsegs[next].empty()))
					{
						++next;
					}

					if (next == NUMCHUNKS)
					{
						continue;
					}

					nextpos = GRFILEPOS(next);

					while (GRFILEPOS(++next) == -1)
					{
						// skip past any sparse tiles
					}

					nextendpos = GRFILEPOS(next);

					if ((nextpos - endpos) <= MAXEMPTYREAD
						&& (nextendpos - pos) <= BUFFERSIZE)
					{
						endpos = nextendpos;
					}
					else
					{
						next = NUMCHUNKS; // read pos to posend
					}
				}

				grhandle.set_position(pos);
				ca_buffer.resize(endpos - pos);
				grhandle.read(ca_buffer.data(), endpos - pos);
				bufferstart = pos;
				bufferend = endpos;
				source = ca_buffer.data();
			}

			CAL_ExpandGrChunk(i, source);
		}
	}
}

void CA_CannotOpen(
	const std::string& string)
{
	BSTONE_THROW_DYNAMIC_SOURCE(("Can't open " + string + "!\n").c_str());
}

void UNCACHEGRCHUNK(
	int chunk)
{
	grsegs[chunk] = GrSegment{};

	grneeded[chunk] &= ~ca_levelbit;
}

std::string ca_load_script(
	int chunk_id,
	bool strip_xx)
{
	CA_CacheGrChunk(static_cast<std::int16_t>(chunk_id));

	const auto script = reinterpret_cast<const char*>(grsegs[chunk_id].data());

	int length = 0;

	for (int i = 0; script[i] != '\x1A'; ++i)
	{
		if (script[i] == '^' && script[i + 1] == 'X' && script[i + 2] == 'X')
		{
			length = i + 3;
		}
	}

	if (length == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid script.");
	}

	if (strip_xx)
	{
		length -= 3;
	}

	return std::string(script, length);
}

void initialize_ca_constants()
{
	const auto& assets_info = get_assets_info();

	const auto total_level_count = assets_info.get_episode_count() * assets_info.get_levels_per_episode();

	mapheaderseg.resize(total_level_count);
}

bool ca_open_resource_non_fatal(
	const std::string& data_dir,
	const std::string& file_name,
	bstone::FileStream& file_stream)
{
	const auto path = bstone::fs_utils::append_path(data_dir, file_name);

	if (file_stream.try_open(path.c_str()))
	{
		return true;
	}

	auto&& file_name_lc = bstone::StringHelper::to_lower_ascii(file_name);
	const auto path_lc = bstone::fs_utils::append_path(data_dir, file_name_lc);

	if (file_stream.try_open(path_lc.c_str()))
	{
		return true;
	}

	return false;
}

bool ca_open_resource_non_fatal(
	const std::string& file_name,
	bstone::FileStream& file_stream)
{
	if (!mod_dir_.empty())
	{
		const auto mod_dir_result = ca_open_resource_non_fatal(mod_dir_, file_name, file_stream);

		if (mod_dir_result)
		{
			return true;
		}
	}

	const auto data_dir_result = ca_open_resource_non_fatal(data_dir_, file_name, file_stream);

	return data_dir_result;
}

void ca_open_resource(
	AssetsResourceType assets_resource_type,
	bstone::FileStream& file_stream)
{
	const auto& assets_info = get_assets_info();
	const auto& assets_resource = assets_info.find_resource(assets_resource_type);

	const auto is_open = ca_open_resource_non_fatal(assets_resource.file_name, file_stream);

	if (!is_open)
	{
		const auto path = data_dir_ + assets_resource.file_name;

		CA_CannotOpen(path);
	}
}

std::string ca_calculate_hash(
	bstone::FileStream& file_stream)
{
	const auto stream_size = file_stream.get_size();

	if (stream_size <= 0 || stream_size > Assets::max_size)
	{
		return {};
	}

	constexpr auto buffer_size = 16384;
	const auto file_size = static_cast<int>(stream_size);

	static auto buffer = Buffer{};
	buffer.resize(buffer_size);

	auto sha1 = bstone::Sha1{};

	auto remain_size = file_size;

	while (remain_size > 0)
	{
		const auto read_count = std::min(remain_size, buffer_size);
		const auto read_result = file_stream.read(buffer.data(), read_count);

		if (read_result != read_count)
		{
			return {};
		}

		sha1.process(buffer.data(), read_count);

		remain_size -= read_count;
	}

	sha1.finish();

	return bstone::array_to_hex_string(sha1.get_digest());
}

std::string ca_calculate_hash(
	const std::string& file_name)
{
	auto file_stream = bstone::FileStream{};

	if (!ca_open_resource_non_fatal(file_name, file_stream))
	{
		return {};
	}

	return ca_calculate_hash(file_stream);
}

std::string ca_calculate_hash(
	const std::string& data_dir,
	const std::string& file_name)
{
	auto file_stream = bstone::FileStream{};

	if (!ca_open_resource_non_fatal(data_dir, file_name, file_stream))
	{
		return {};
	}

	return ca_calculate_hash(file_stream);
}

std::string ca_calculate_hash(
	const std::string& data_dir,
	AssetsResourceType assets_resource_type)
{
	const auto& assets_info = get_assets_info();
	const auto& assets_resource = assets_info.find_resource(assets_resource_type);

	return ca_calculate_hash(data_dir, assets_resource.file_name);
}

void ca_calculate_hashes()
{
	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("Calculating resource hashes...");

	const auto& assets_info = get_assets_info();

	for (const auto& resource : assets_info.get_resources())
	{
		const auto& sha1_string = ca_calculate_hash(resource.file_name);

		if (sha1_string.empty())
		{
			continue;
		}

		bstone::globals::logger->log_information((std::string{resource.file_name} + ": " + sha1_string).c_str());
	}
}


AssetsVersion AssetsInfo::get_version() const
{
	return version_;
}

void AssetsInfo::set_version(
	const AssetsVersion version)
{
	version_ = version;

	switch (version_)
	{
	case AssetsVersion::aog_sw_v1_0:
		gfx_header_offset_count_ = 200;
		break;

	case AssetsVersion::aog_sw_v3_0:
	case AssetsVersion::aog_full_v1_0:
		gfx_header_offset_count_ = 213;
		break;

	case AssetsVersion::aog_sw_v2_0:
	case AssetsVersion::aog_sw_v2_1:
		gfx_header_offset_count_ = 211;
		break;

	case AssetsVersion::aog_full_v2_0:
	case AssetsVersion::aog_full_v2_1:
		gfx_header_offset_count_ = 224;
		break;

	case AssetsVersion::aog_full_v3_0:
		gfx_header_offset_count_ = 226;
		break;

	case AssetsVersion::ps:
		gfx_header_offset_count_ = 249;
		break;

	default:
		gfx_header_offset_count_ = 0;
		break;
	}


	{
		if (is_aog_full())
		{
			episode_count_ = 6;
		}
		else if (is_aog_sw() || is_ps())
		{
			episode_count_ = 1;
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("No assets information.");
		}
	}

	{
		if (is_aog())
		{
			levels_per_episode_ = 15;
		}
		else if (is_ps())
		{
			levels_per_episode_ = 24;
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("No assets information.");
		}
	}

	{
		if (is_aog())
		{
			stats_levels_per_episode_ = 11;
		}
		else if (is_ps())
		{
			stats_levels_per_episode_ = 20;
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("No assets information.");
		}
	}

	{
		if (is_aog())
		{
			barrier_switches_per_level_ = 5;
			max_barrier_switches_per_level_bits_ = 3;
		}
		else if (is_ps())
		{
			barrier_switches_per_level_ = 40;
			max_barrier_switches_per_level_bits_ = 6;
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("No assets information.");
		}
	}

	total_levels_ = episode_count_ * levels_per_episode_;

	{
		if (is_aog())
		{
			base_path_name_ = "aog";
		}
		else if (is_ps())
		{
			base_path_name_ = "ps";
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("No assets information.");
		}
	}
}

const std::string& AssetsInfo::get_levels_hash_string() const
{
	return levels_hash_;
}

const AssetsResources& AssetsInfo::get_resources() const noexcept
{
	return resources_;
}

void AssetsInfo::set_resources(
	const AssetsResources& resources) noexcept
{
	resources_ = resources;
}

const AssetsResource& AssetsInfo::find_resource(
	AssetsResourceType resource_type) const
{
	for (const auto& resource : resources_)
	{
		if (resource.type == resource_type)
		{
			return resource;
		}
	}

	const auto resource_type_string = std::string{get_resource_type_string(resource_type)};

	BSTONE_THROW_DYNAMIC_SOURCE(("Resource of type \"" + resource_type_string + "\" not found.").c_str());
}

void AssetsInfo::set_levels_hash(
	const std::string& levels_hash)
{
	levels_hash_ = levels_hash;

	are_modded_levels_ = !Assets::are_official_levels(levels_hash_);
}

const std::string& AssetsInfo::get_base_path_name() const noexcept
{
	return base_path_name_;
}

int AssetsInfo::get_gfx_header_offset_count() const
{
	return gfx_header_offset_count_;
}

bool AssetsInfo::are_modded_levels() const
{
	return are_modded_levels_;
}

bool AssetsInfo::is_aog_full_v1_0() const
{
	return version_ == AssetsVersion::aog_full_v1_0;
}

bool AssetsInfo::is_aog_full_v2_0() const
{
	return version_ == AssetsVersion::aog_full_v2_0;
}

bool AssetsInfo::is_aog_full_v2_1() const
{
	return version_ == AssetsVersion::aog_full_v2_1;
}

bool AssetsInfo::is_aog_full_v2_x() const
{
	return is_aog_full_v2_0() || is_aog_full_v2_1();
}

bool AssetsInfo::is_aog_full_v3_0() const
{
	return version_ == AssetsVersion::aog_full_v3_0;
}

bool AssetsInfo::is_aog_full() const
{
	return is_aog_full_v1_0() || is_aog_full_v2_x() || is_aog_full_v3_0();
}

bool AssetsInfo::is_aog_sw_v1_0() const
{
	return version_ == AssetsVersion::aog_sw_v1_0;
}

bool AssetsInfo::is_aog_sw_v2_0() const
{
	return version_ == AssetsVersion::aog_sw_v2_0;
}

bool AssetsInfo::is_aog_sw_v2_1() const
{
	return version_ == AssetsVersion::aog_sw_v2_1;
}

bool AssetsInfo::is_aog_sw_v2_x() const
{
	return is_aog_sw_v2_0() || is_aog_sw_v2_1();
}

bool AssetsInfo::is_aog_sw_v3_0() const
{
	return version_ == AssetsVersion::aog_sw_v3_0;
}

bool AssetsInfo::is_aog_sw() const
{
	return is_aog_sw_v1_0() || is_aog_sw_v2_x() || is_aog_sw_v3_0();
}

bool AssetsInfo::is_aog() const
{
	return is_aog_full() || is_aog_sw();
}

bool AssetsInfo::is_ps() const
{
	return version_ == AssetsVersion::ps;
}

int AssetsInfo::get_episode_count() const
{
	return episode_count_;
}

int AssetsInfo::get_levels_per_episode() const
{
	return levels_per_episode_;
}

int AssetsInfo::get_stats_levels_per_episode() const
{
	return stats_levels_per_episode_;
}

int AssetsInfo::get_total_levels() const
{
	return total_levels_;
}

int AssetsInfo::get_barrier_switches_per_level() const noexcept
{
	return barrier_switches_per_level_;
}

int AssetsInfo::get_max_barrier_switches_per_level_bits() const noexcept
{
	return max_barrier_switches_per_level_bits_;
}

bool AssetsInfo::is_secret_level(
	const int level_number) const
{
	if (is_aog())
	{
		return level_number <= 0 || level_number >= 10;
	}
	else if (is_ps())
	{
		return level_number > 19;
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("No assets information.");
	}
}

int AssetsInfo::secret_floor_get_index(
	const int level_number) const
{
	if (is_aog())
	{
		if (level_number <= 0)
		{
			return 0;
		}

		if (level_number >= 10)
		{
			return level_number - 10 + 1;
		}

		return -1;
	}
	else if (is_ps())
	{
		if (level_number >= 20)
		{
			return level_number - 20;
		}

		return -1;
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("No assets information.");
	}
}

const char* AssetsInfo::get_resource_type_string(
	AssetsResourceType resource_type) noexcept
{
	switch (resource_type)
	{
		case AssetsResourceType::audiohed:
			return "audiohed";

		case AssetsResourceType::audiot:
			return "audiot";

		case AssetsResourceType::eanim:
			return "eanim";

		case AssetsResourceType::ganim:
			return "ganim";

		case AssetsResourceType::ianim:
			return "ianim";

		case AssetsResourceType::maphead:
			return "maphead";

		case AssetsResourceType::maptemp:
			return "maptemp";

		case AssetsResourceType::sanim:
			return "sanim";

		case AssetsResourceType::vgadict:
			return "vgadict";

		case AssetsResourceType::vgagraph:
			return "vgagraph";

		case AssetsResourceType::vgahead:
			return "vgahead";

		case AssetsResourceType::vswap:
			return "vswap";

		case AssetsResourceType::none:
		default:
			return "???";
	}
}

AssetsInfo& get_assets_info()
{
	static auto assets_info = AssetsInfo{};
	return assets_info;
}

bool Assets::are_official_levels(
	const std::string& levels_hash)
{
	for (const auto& resource : get_all_resources())
	{
		if (resource.hash_string == levels_hash)
		{
			return true;
		}
	}

	return false;
}

const AssetsResources& Assets::get_aog_sw_v1_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "55b9eb9ed555b0f249c2cefd54ecc3f511bfcd55"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "293464e7143ff7e6faf5f5b20799e76a394d65bf"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "b54d48d35d095e27a3c9130cfa59ed9c4f05abe7"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "d1fa19131281d30787125f946881652be876f57a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "cc44c362e9e2c9f7b9fb2e4ba31331dde42a1e96"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "37412ca9139562fc31330d72470822de4d9ceb4a"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "058fe0b59c7aa020bf4e7509103892d4c7459aa6"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "e14b6172b6ab7568d3736f087f07a0df5eac5dad"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "6c0e273df7fd3940c038fc20a5bdfb81cd50e7ef"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "ed1ab61ad2529e046f966d9c1627508f76693ef8"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "b59cbe3793b4612b06254ece48bf8e961ab6f528"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "0635a4a2b823eef2904ed1d590d98362fb16621d"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs1, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs1, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs1, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs1, "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs1, "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs1, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs1, "537676ddcafeee415c22bb9d00097b25bc7e13c5"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs1, "9f9f4c40c9637af472ce5e1a360e51364c3a418a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs1, "e7fb8f241b8fa94739c3bd09ea05c1afc7bbfc95"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs1, "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v1_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "6f19a144d8985e90f096fc1c67ade58e9051235c"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "4d00c5f5c843f99a266bd938648192a6eff17b5a"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "d4f81ace1701a7338d43ce07723c2adaafdc837c"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "02dc27d4810e3ffa26540b310eac6091f5e16de0"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "639ec9e7a81ad83fc5b5c557cf4fc5fa28b9676b"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "0a700732ccbc72f95318a6226a7e1ad78ac713bb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "e83b690836c9edf9ef60f6189b8384fb2319b735"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "67a679e3b107db8685ba5ff1643a38f9291b00bf"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "48b18caa86151610957b64b207cf2a2977ef7d57"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "e4ae3ef9a3ac158a832092b7b5487227337c6f13"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "6868e6cc4f8cb9160a218c5bce680a11f64c675a"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "1a3864cd12de4b8fd6be023b73d2d79e91b9018d"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_bs6, "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, audiot_bs6, "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, eanim_bs6, "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, ganim_bs6, "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, ianim_bs6, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_bs6, "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_bs6, "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, sanim_bs6, "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_bs6, "60da35e506d57753f83cca5f232a76dd1cf074ba"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_bs6, "001037fafdff124befb5437c563d0b9c613b3c00"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_bs6, "ff165ca5924d738853587a7e4ab1cd239e10e359"},
		AssetsResource{AssetsResourceType::vswap, vswap_bs6, "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_ps_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, audiohed_vsi, "2ed9a587926b2b455f8176788f595d03f6359171"},
		AssetsResource{AssetsResourceType::audiot, audiot_vsi, "ecac8a800c5d021c8b68d170066c8859df9bd79a"},
		AssetsResource{AssetsResourceType::eanim, eanim_vsi, "b668330e56ceb069c1b972cefddd8a3d618a14af"},
		AssetsResource{AssetsResourceType::ianim, ianim_vsi, "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, maphead_vsi, "c3fb0a9b81e0240d12c952fe5b57b78c1108aa48"},
		AssetsResource{AssetsResourceType::maptemp, maptemp_vsi, "acd03031d526eeaee64072518adf6814f96e2a37"},
		AssetsResource{AssetsResourceType::vgadict, vgadict_vsi, "061692d166f68d7e0b81568725439078cc728f33"},
		AssetsResource{AssetsResourceType::vgagraph, vgagraph_vsi, "e619a3e6245f79888da5436df138d63204d2caba"},
		AssetsResource{AssetsResourceType::vgahead, vgahead_vsi, "ad5fed9de5d1d82145df3ff2286e4bc62cb80e2a"},
		AssetsResource{AssetsResourceType::vswap, vswap_vsi, "4113ce83c42e69dc5cb20de79f41971a100f876e"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_all_resources()
{
	static const auto all_resources = make_all_resources();

	static const auto all_assets_resources = AssetsResources{
		all_resources.data(),
		static_cast<std::intptr_t>(all_resources.size())
	};

	return all_assets_resources;
}

Assets::AllResources Assets::make_all_resources()
{
	auto all_resources = AllResources{};
	all_resources.reserve(40);

	all_resources.insert(all_resources.end(), get_aog_sw_v1_0_resources().begin(), get_aog_sw_v1_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v2_0_resources().begin(), get_aog_sw_v2_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v2_1_resources().begin(), get_aog_sw_v2_1_resources().end());
	all_resources.insert(all_resources.end(), get_aog_sw_v3_0_resources().begin(), get_aog_sw_v3_0_resources().end());

	all_resources.insert(all_resources.end(), get_aog_full_v1_0_resources().begin(), get_aog_full_v1_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v2_0_resources().begin(), get_aog_full_v2_0_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v2_1_resources().begin(), get_aog_full_v2_1_resources().end());
	all_resources.insert(all_resources.end(), get_aog_full_v3_0_resources().begin(), get_aog_full_v3_0_resources().end());

	all_resources.insert(all_resources.end(), get_ps_resources().begin(), get_ps_resources().end());

	return all_resources;
}

void ca_extract_vga_palette(const std::string& destination_dir)
{
	auto images_extractor = bstone::ImageExtractor{};
	images_extractor.extract_vga_palette(destination_dir);
}

void ca_extract_walls(const std::string& destination_dir)
{
	auto images_extractor = bstone::ImageExtractor{};
	images_extractor.extract_walls(destination_dir);
}

void ca_extract_sprites(const std::string& destination_dir)
{
	auto images_extractor = bstone::ImageExtractor{};
	images_extractor.extract_sprites(destination_dir);
}

void ca_extract_music(
	const std::string& destination_dir)
{
	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("<<< ================");
	bstone::globals::logger->log_information("Extracting music.");
	bstone::globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());

	auto audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
	auto audio_extractor = bstone::make_audio_extractor(*audio_content_mgr);

	const auto normalized_dst_dir = bstone::fs_utils::normalize_path(destination_dir);
	audio_extractor->extract_music(normalized_dst_dir);

	bstone::globals::logger->log_information(">>> ================");
}

void ca_extract_sfx(
	const std::string& destination_dir)
{
	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("<<< ================");
	bstone::globals::logger->log_information("Extracting sfx.");
	bstone::globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());

	auto audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
	auto audio_extractor = bstone::make_audio_extractor(*audio_content_mgr);

	const auto normalized_dst_dir = bstone::fs_utils::normalize_path(destination_dir);
	audio_extractor->extract_sfx(normalized_dst_dir);

	bstone::globals::logger->log_information(">>> ================");
}

void ca_extract_texts(
	const std::string& destination_dir)
{
	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("<<< ================");
	bstone::globals::logger->log_information("Extracting text.");
	bstone::globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());

	auto text_extractor = bstone::TextExtractor{};
	text_extractor.extract_text(bstone::fs_utils::normalize_path(destination_dir));

	bstone::globals::logger->log_information(">>> ================");
}

void ca_extract_levels(const std::string& destination_dir)
{
	auto level_extractor = bstone::LevelExtractor{};
	level_extractor.extract_levels(destination_dir);
}

void ca_extract_all(
	const std::string& destination_dir)
{
	ca_extract_vga_palette(destination_dir);
	ca_extract_walls(destination_dir);
	ca_extract_sprites(destination_dir);
	ca_extract_music(destination_dir);
	ca_extract_sfx(destination_dir);
	ca_extract_texts(destination_dir);
	ca_extract_levels(destination_dir);
}

int ca_map_aog_sw_sprite_id_to_aog_full(
	int aog_sw_sprite_id)
{
	constexpr auto min_id = 1;
	constexpr auto max_id = 552;

	if (aog_sw_sprite_id < min_id || aog_sw_sprite_id > max_id)
	{
		return 0;
	}

	using Map = std::array<int, max_id + 1>;

	static const auto map = Map
	{
		0, 1, 3, 9, 10, 11, 12, 13, 14, 15,
		16, 19, 25, 27, 28, 29, 30, 32, 33, 34,
		35, 36, 37, 39, 41, 42, 43, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
		57, 58, 61, 64, 65, 66, 67, 69, 70, 71,
		72, 73, 81, 82, 83, 84, 85, 86, 87, 88,
		89, 90, 91, 92, 93, 94, 95, 96, 97, 98,
		99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
		109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
		119, 120, 121, 122, 123, 124, 125, 126, 127, 128,

		129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
		139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
		149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
		159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
		169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
		179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
		189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
		199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
		209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
		219, 220, 221, 222, 223, 224, 225, 226, 227, 228,

		229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
		239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
		249, 250, 251, 252, 253, 254, 255, 256, 257, 258,
		259, 260, 261, 262, 263, 264, 265, 266, 267, 268,
		269, 270, 271, 272, 273, 274, 275, 276, 277, 278,
		279, 280, 281, 282, 283, 284, 285, 286, 287, 288,
		289, 290, 291, 292, 293, 294, 295, 296, 297, 298,
		299, 300, 301, 302, 303, 304, 305, 306, 307, 308,
		309, 310, 311, 312, 313, 314, 315, 316, 317, 318,
		319, 320, 321, 322, 323, 324, 325, 326, 327, 328,

		329, 330, 331, 332, 333, 334, 335, 336, 337, 338,
		339, 340, 341, 342, 343, 344, 345, 346, 347, 348,
		349, 350, 351, 352, 353, 354, 355, 356, 357, 358,
		359, 401, 402, 403, 404, 405, 406, 407, 408, 433,
		434, 435, 436, 437, 438, 439, 440, 441, 442, 443,
		444, 445, 459, 460, 461, 462, 463, 464, 465, 466,
		467, 468, 469, 470, 471, 472, 473, 474, 475, 476,
		477, 478, 479, 480, 481, 482, 483, 484, 485, 486,
		487, 488, 489, 490, 491, 492, 493, 494, 495, 496,
		497, 498, 499, 500, 501, 502, 503, 504, 505, 506,

		507, 508, 509, 510, 511, 512, 513, 514, 515, 516,
		517, 518, 519, 520, 521, 522, 523, 524, 525, 526,
		527, 528, 529, 530, 531, 532, 533, 534, 535, 536,
		574, 575, 576, 577, 578, 579, 580, 581, 582, 583,
		584, 585, 586, 587, 588, 589, 590, 591, 592, 593,
		594, 635, 636, 637, 638, 639, 640, 641, 642, 643,
		644, 645, 646, 647, 648, 649, 650, 651, 652, 653,
		654, 655, 656, 657, 658, 659, 660, 661, 662, 663,
		664, 665, 666, 667, 668, 669, 670, 671, 672, 673,
		674, 675, 676, 677, 678, 679, 680, 681, 682, 683,

		684, 685, 686, 687, 688, 689, 690, 691, 692, 693,
		694, 695, 696, 697, 698, 699, 700, 701, 702, 703,
		704, 705, 706, 707, 708, 709, 716, 717, 718, 719,
		720, 721, 722, 723, 724, 725, 726, 727, 728, 729,
		730, 731, 732, 733, 734, 735, 736, 737, 738, 739,
		740, 741, 742,
	};

	return map[aog_sw_sprite_id];
}

void ca_make_resource_path(
	const std::string& resource_name,
	std::string& data_path,
	std::string& mod_path)
try {
	if (resource_name.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty name.");
	}

	const auto& assets_info = get_assets_info();

	data_path.clear();

	if (!data_dir_.empty())
	{
		data_path = bstone::fs_utils::append_path(data_dir_, assets_info.get_base_path_name());
		data_path = bstone::fs_utils::append_path(data_path, resource_name);
	}

	mod_path.clear();

	if (!mod_dir_.empty())
	{
		mod_path = bstone::fs_utils::append_path(mod_dir_, assets_info.get_base_path_name());
		mod_path = bstone::fs_utils::append_path(mod_path, resource_name);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ca_make_sprite_resource_path_name(
	int sprite_id,
	std::string& data_path,
	std::string& mod_path)
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw())
	{
		sprite_id = ca_map_aog_sw_sprite_id_to_aog_full(sprite_id);
	}

	const auto id_string = ca_make_padded_asset_number_string(sprite_id);
	ca_make_resource_path("sprite_" + id_string, data_path, mod_path);
}

void ca_make_wall_resource_path_name(
	int wall_id,
	std::string& data_path,
	std::string& mod_path)
{
	const auto id_string = ca_make_padded_asset_number_string(wall_id);
	ca_make_resource_path("wall_" + id_string, data_path, mod_path);
}
