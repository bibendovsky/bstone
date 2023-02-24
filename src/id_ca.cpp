/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
#include <exception>
#include <memory>

#include "jm_cio.h"
#include "jm_lzh.h"
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
#include "bstone_file_system.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_memory_stream.h"
#include "bstone_sha1.h"
#include "bstone_sprite_cache.h"
#include "bstone_string_helper.h"

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

		expanded = bstone::Endian::little(*reinterpret_cast<std::int32_t*>(source));
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
void CA_CacheMap(
	std::int16_t mapnum)
{
	if (mapheaderseg[mapnum].name[0] == '\0')
	{
		::fail("There are no assets for level index " + std::to_string(mapnum) + '.');
	}

	std::int32_t pos;
	std::int32_t compressed;
	std::int16_t plane;
	std::uint16_t size;
	std::uint16_t* source;
#ifdef CARMACIZED
	memptr buffer2seg;
	std::int32_t expanded;
#endif

	mapon = mapnum;

	//
	// load the planes into the already allocated buffers
	//
	size = MAPSIZE * MAPSIZE * MAPPLANES;

	for (plane = 0; plane < MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum].planestart[plane];
		compressed = mapheaderseg[mapnum].planelength[plane];

		const auto dest = mapsegs[plane].data();

		maphandle.set_position(pos);
		ca_buffer.resize(compressed);
		source = reinterpret_cast<std::uint16_t*>(ca_buffer.data());

		maphandle.read(source, compressed);

#ifdef CARMACIZED
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		expanded = *source;
		source++;
		MM_GetPtr(&buffer2seg, expanded);
		CAL_CarmackExpand(source, (std::uint16_t*)buffer2seg, expanded);
		CA_RLEWexpand(((std::uint16_t*)buffer2seg) + 1, *dest, size,
			((mapfiletype*)tinf)->RLEWtag);
		MM_FreePtr(&buffer2seg);

#else
		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand(source + 1, dest, size, rlew_tag);
#endif
	}
}

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
		::fail("Up past level 7.");
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
		::fail("Down past level 0.");
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
	::fail("Can't open " + string + "!\n");
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
		::fail("Invalid script.");
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
	const auto path = bstone::file_system::append_path(data_dir, file_name);

	auto is_open = false;

	is_open = file_stream.open(path);

	if (!is_open)
	{
		auto&& file_name_lc = bstone::StringHelper::to_lower_ascii(file_name);
		const auto path_lc = bstone::file_system::append_path(data_dir, file_name_lc);

		is_open = file_stream.open(path_lc);
	}

	return is_open;
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

	static auto sha1 = bstone::Sha1{};
	sha1.reset();

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

	if (!sha1.is_valid())
	{
		return {};
	}

	return sha1.to_string();
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
	bstone::logger_->write();
	bstone::logger_->write("Calculating resource hashes...");

	const auto& assets_info = get_assets_info();

	for (const auto& resource : assets_info.get_resources())
	{
		const auto& sha1_string = ca_calculate_hash(resource.file_name);

		if (sha1_string.empty())
		{
			continue;
		}

		bstone::logger_->write(std::string{resource.file_name} + ": " + sha1_string);
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
			::fail("No assets information.");
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
			::fail("No assets information.");
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
			::fail("No assets information.");
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
			::fail("No assets information.");
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
			::fail("No assets information.");
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

	::fail("Resource of type \"" + resource_type_string + "\" not found.");
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
		::fail("No assets information.");
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
		::fail("No assets information.");
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
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs1(), "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs1(), "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs1(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs1(), "55b9eb9ed555b0f249c2cefd54ecc3f511bfcd55"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs1(), "293464e7143ff7e6faf5f5b20799e76a394d65bf"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs1(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs1(), "b54d48d35d095e27a3c9130cfa59ed9c4f05abe7"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs1(), "d1fa19131281d30787125f946881652be876f57a"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs1(), "cc44c362e9e2c9f7b9fb2e4ba31331dde42a1e96"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs1(), "37412ca9139562fc31330d72470822de4d9ceb4a"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs1(), "0c3de403b524107809fa9308b730d60e8e41ba93"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs1(), "3d4e8b62aa4683671027513ad9720f76f176ba5b"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs1(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs1(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs1(), "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs1(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs1(), "058fe0b59c7aa020bf4e7509103892d4c7459aa6"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs1(), "e14b6172b6ab7568d3736f087f07a0df5eac5dad"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs1(), "6c0e273df7fd3940c038fc20a5bdfb81cd50e7ef"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs1(), "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs1(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs1(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs1(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs1(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs1(), "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs1(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs1(), "ed1ab61ad2529e046f966d9c1627508f76693ef8"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs1(), "b59cbe3793b4612b06254ece48bf8e961ab6f528"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs1(), "0635a4a2b823eef2904ed1d590d98362fb16621d"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs1(), "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_sw_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs1(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs1(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs1(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs1(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs1(), "bbb3cd6ab3e742eada427862504eba06437036d0"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs1(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs1(), "537676ddcafeee415c22bb9d00097b25bc7e13c5"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs1(), "9f9f4c40c9637af472ce5e1a360e51364c3a418a"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs1(), "e7fb8f241b8fa94739c3bd09ea05c1afc7bbfc95"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs1(), "dbde907ba1110bef445d6daae283c4520d6951b4"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v1_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs6(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs6(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, get_eanim_bs6(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, get_ganim_bs6(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs6(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs6(), "6f19a144d8985e90f096fc1c67ade58e9051235c"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs6(), "4d00c5f5c843f99a266bd938648192a6eff17b5a"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs6(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs6(), "d4f81ace1701a7338d43ce07723c2adaafdc837c"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs6(), "02dc27d4810e3ffa26540b310eac6091f5e16de0"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs6(), "639ec9e7a81ad83fc5b5c557cf4fc5fa28b9676b"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs6(), "0a700732ccbc72f95318a6226a7e1ad78ac713bb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs6(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs6(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, get_eanim_bs6(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, get_ganim_bs6(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs6(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs6(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs6(), "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs6(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs6(), "e83b690836c9edf9ef60f6189b8384fb2319b735"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs6(), "67a679e3b107db8685ba5ff1643a38f9291b00bf"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs6(), "48b18caa86151610957b64b207cf2a2977ef7d57"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs6(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v2_1_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs6(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs6(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, get_eanim_bs6(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, get_ganim_bs6(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs6(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs6(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs6(), "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs6(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs6(), "e4ae3ef9a3ac158a832092b7b5487227337c6f13"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs6(), "6868e6cc4f8cb9160a218c5bce680a11f64c675a"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs6(), "1a3864cd12de4b8fd6be023b73d2d79e91b9018d"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs6(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_aog_full_v3_0_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_bs6(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_bs6(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},
		AssetsResource{AssetsResourceType::eanim, get_eanim_bs6(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		AssetsResource{AssetsResourceType::ganim, get_ganim_bs6(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_bs6(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_bs6(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_bs6(), "8b48e7eb859382a4c84948c5a62899194288e853"},
		AssetsResource{AssetsResourceType::sanim, get_sanim_bs6(), "22bf818465da0f32eef9611de936cba9966b14aa"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_bs6(), "60da35e506d57753f83cca5f232a76dd1cf074ba"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_bs6(), "001037fafdff124befb5437c563d0b9c613b3c00"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_bs6(), "ff165ca5924d738853587a7e4ab1cd239e10e359"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_bs6(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_ps_resources()
{
	static const AssetsResource resources[] =
	{
		AssetsResource{AssetsResourceType::audiohed, get_audiohed_vsi(), "2ed9a587926b2b455f8176788f595d03f6359171"},
		AssetsResource{AssetsResourceType::audiot, get_audiot_vsi(), "ecac8a800c5d021c8b68d170066c8859df9bd79a"},
		AssetsResource{AssetsResourceType::eanim, get_eanim_vsi(), "b668330e56ceb069c1b972cefddd8a3d618a14af"},
		AssetsResource{AssetsResourceType::ianim, get_ianim_vsi(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		AssetsResource{AssetsResourceType::maphead, get_maphead_vsi(), "c3fb0a9b81e0240d12c952fe5b57b78c1108aa48"},
		AssetsResource{AssetsResourceType::maptemp, get_maptemp_vsi(), "acd03031d526eeaee64072518adf6814f96e2a37"},
		AssetsResource{AssetsResourceType::vgadict, get_vgadict_vsi(), "061692d166f68d7e0b81568725439078cc728f33"},
		AssetsResource{AssetsResourceType::vgagraph, get_vgagraph_vsi(), "e619a3e6245f79888da5436df138d63204d2caba"},
		AssetsResource{AssetsResourceType::vgahead, get_vgahead_vsi(), "ad5fed9de5d1d82145df3ff2286e4bc62cb80e2a"},
		AssetsResource{AssetsResourceType::vswap, get_vswap_vsi(), "4113ce83c42e69dc5cb20de79f41971a100f876e"},
	};

	static const auto assets_resources = AssetsResources{resources};

	return assets_resources;
}

const AssetsResources& Assets::get_all_resources()
{
	static const auto all_resources = make_all_resources();

	static const auto all_assets_resources = AssetsResources{
		all_resources.data(),
		static_cast<bstone::Int>(all_resources.size())
	};

	return all_assets_resources;
}

// -------------------------------------------------------------------------
// Aliens Of Gold (shareware)

const char* Assets::get_audiohed_bs1() noexcept
{
	return "AUDIOHED.BS1";
}

const char* Assets::get_audiot_bs1() noexcept
{
	return "AUDIOT.BS1";
}

const char* Assets::get_ianim_bs1() noexcept
{
	return "IANIM.BS1";
}

const char* Assets::get_maphead_bs1() noexcept
{
	return "MAPHEAD.BS1";
}

const char* Assets::get_maptemp_bs1() noexcept
{
	return "MAPTEMP.BS1";
}

const char* Assets::get_sanim_bs1() noexcept
{
	return "SANIM.BS1";
}

const char* Assets::get_vgadict_bs1() noexcept
{
	return "VGADICT.BS1";
}

const char* Assets::get_vgagraph_bs1() noexcept
{
	return "VGAGRAPH.BS1";
}

const char* Assets::get_vgahead_bs1() noexcept
{
	return "VGAHEAD.BS1";
}

const char* Assets::get_vswap_bs1() noexcept
{
	return "VSWAP.BS1";
}

// Aliens Of Gold (shareware)
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Aliens Of Gold (full)

const char* Assets::get_audiohed_bs6() noexcept
{
	return "AUDIOHED.BS6";
}

const char* Assets::get_audiot_bs6() noexcept
{
	return "AUDIOT.BS6";
}

const char* Assets::get_eanim_bs6() noexcept
{
	return "EANIM.BS6";
}

const char* Assets::get_ganim_bs6() noexcept
{
	return "GANIM.BS6";
}

const char* Assets::get_ianim_bs6() noexcept
{
	return "IANIM.BS6";
}

const char* Assets::get_maphead_bs6() noexcept
{
	return "MAPHEAD.BS6";
}

const char* Assets::get_maptemp_bs6() noexcept
{
	return "MAPTEMP.BS6";
}

const char* Assets::get_sanim_bs6() noexcept
{
	return "SANIM.BS6";
}

const char* Assets::get_vgadict_bs6() noexcept
{
	return "VGADICT.BS6";
}

const char* Assets::get_vgagraph_bs6() noexcept
{
	return "VGAGRAPH.BS6";
}

const char* Assets::get_vgahead_bs6() noexcept
{
	return "VGAHEAD.BS6";
}

const char* Assets::get_vswap_bs6() noexcept
{
	return "VSWAP.BS6";
}

// Aliens Of Gold (full)
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Planet Strike

const char* Assets::get_audiohed_vsi() noexcept
{
	return "AUDIOHED.VSI";
}

const char* Assets::get_audiot_vsi() noexcept
{
	return "AUDIOT.VSI";
}

const char* Assets::get_eanim_vsi() noexcept
{
	return "EANIM.VSI";
}

const char* Assets::get_ianim_vsi() noexcept
{
	return "IANIM.VSI";
}

const char* Assets::get_maphead_vsi() noexcept
{
	return "MAPHEAD.VSI";
}

const char* Assets::get_maptemp_vsi() noexcept
{
	return "MAPTEMP.VSI";
}

const char* Assets::get_vgadict_vsi() noexcept
{
	return "VGADICT.VSI";
}

const char* Assets::get_vgagraph_vsi() noexcept
{
	return "VGAGRAPH.VSI";
}

const char* Assets::get_vgahead_vsi() noexcept
{
	return "VGAHEAD.VSI";
}

const char* Assets::get_vswap_vsi() noexcept
{
	return "VSWAP.VSI";
}

// Planet Strike
// -------------------------------------------------------------------------

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

// ==========================================================================
// ImageExtractor

class ImageExtractor
{
public:
	ImageExtractor();

	void extract_vga_palette(const std::string& destination_dir);
	void extract_walls(const std::string& destination_dir);
	void extract_sprites(const std::string& destination_dir);

private:
	class Exception : public bstone::Exception
	{
	public:
		explicit Exception(const char* message)
			:
			bstone::Exception{"BSTONE_IMAGE_EXTRACTOR", message}
		{}

		~Exception() override = default;
	};

private:
	static constexpr auto max_bit_depth = 32;

	static constexpr auto palette_1bpp_size = 1 << 1;
	static constexpr auto palette_4bpp_size = 1 << 4;
	static constexpr auto palette_8bpp_size = 1 << 8;
	static constexpr auto max_palette_size = palette_8bpp_size;

	static constexpr auto max_width = 320;
	static constexpr auto max_height = 200;

	static constexpr auto wall_width = 64;
	static constexpr auto wall_height = 64;

	static constexpr auto sprite_width = bstone::Sprite::dimension;
	static constexpr auto sprite_height = bstone::Sprite::dimension;

	using Palette = std::array<std::uint32_t, max_palette_size>; // 0xAARRGGBB

	using ColorBuffer = std::vector<std::uint8_t>;
	using LineBuffer = std::array<std::uint8_t, max_width>;

private:
	static constexpr const std::uint8_t padding_bytes[3] = {};

private:
	int sprite_count_{};
	std::string destination_dir_{};
	std::string destination_path_{};
	bstone::SpriteCache sprite_cache_{};
	int width_{};
	int height_{};
	int area_{};
	int bit_depth_{};
	int palette_size_{};
	int stride_{};
	Palette src_palette_{};
	Palette dst_palette_{};
	ColorBuffer color_buffer_{};
	LineBuffer line_buffer_{};
	std::uint8_t* colors8_;
	std::uint32_t* colors32_; // 0xAARRGGBB

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);

private:
	void initialize_colors();
	void initialize_src_palette();
	void remap_indexed_image();

	void decode_default_palette() noexcept;
	void decode_wall_page(const std::uint8_t* src_colors) noexcept;
	void decode_sprite_page(const bstone::Sprite& sprite) noexcept;

	void save_bmp_rgb_palette(bstone::BinaryWriter& binary_writer);
	void save_bmp_rgbx_palette(bstone::BinaryWriter& binary_writer);
	void save_bmp_palette(bstone::BinaryWriter& binary_writer);

	void save_bmp_1bpp_bits(bstone::BinaryWriter& binary_writer);
	void save_bmp_4bpp_bits(bstone::BinaryWriter& binary_writer);
	void save_bmp_8bpp_bits(bstone::BinaryWriter& binary_writer);
	void save_bmp_32bpp_bits(bstone::BinaryWriter& binary_writer);

	void save_bmp(const std::string& path);
	void save_image(const std::string& name_prefix, int image_index);

	void extract_wall(int wall_index);
	void extract_sprite(int sprite_index);
}; // ImageExtractor

ImageExtractor::ImageExtractor()
{
	initialize_colors();
	initialize_src_palette();
}

void ImageExtractor::extract_vga_palette(const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting VGA palette.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	decode_default_palette();

	const auto file_name = bstone::file_system::append_path(destination_dir_, "vga_palette.bmp");
	save_bmp(file_name);

	bstone::logger_->write(">>> ================");
}

void ImageExtractor::extract_walls(const std::string& destination_dir)
{
	const auto wall_count = bstone::globals::page_mgr->get_wall_count();

	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting walls.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");
	bstone::logger_->write("File count: " + std::to_string(wall_count));

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	for (auto i = 0; i < wall_count; ++i)
	{
		extract_wall(i);
	}

	bstone::logger_->write(">>> ================");
}

void ImageExtractor::extract_sprites(const std::string& destination_dir)
{
	sprite_count_ = bstone::globals::page_mgr->get_sprite_count();

	if (sprite_count_ < 0)
	{
		sprite_count_ = 0;
	}

	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting sprites.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");
	bstone::logger_->write("File count: " + std::to_string(sprite_count_));

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	for (auto i = 1; i < sprite_count_; ++i)
	{
		extract_sprite(i);
	}

	bstone::logger_->write(">>> ================");
}

[[noreturn]] void ImageExtractor::fail(const char* message)
{
	throw Exception{message};
}

[[noreturn]] void ImageExtractor::fail_nested(const char* message)
{
	std::throw_with_nested(Exception{message});
}

void ImageExtractor::initialize_colors()
try
{
	color_buffer_.resize(max_width * max_height * (((max_bit_depth + 7) / 8)) * 8);
	colors8_ = color_buffer_.data();
	colors32_ = reinterpret_cast<std::uint32_t*>(color_buffer_.data());
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::initialize_src_palette()
try
{
	auto src_colors = vgapal; // {0xRR, 0xGG, 0xBB}

	for (auto& dst_color : src_palette_)
	{
		const auto r = (255U * (*src_colors++)) / 63U;
		const auto g = (255U * (*src_colors++)) / 63U;
		const auto b = (255U * (*src_colors++)) / 63U;
		dst_color = 0xFF000000U | (r << 16) | (g << 8) | b;
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::remap_indexed_image()
{
	using UsedColors = std::bitset<max_palette_size>;
	auto used_colors = UsedColors{};

	for (auto i = 0; i < area_; ++i)
	{
		used_colors.set(colors8_[i]);
	}

	const auto palette_size = used_colors.count();

	if (static_cast<int>(palette_size) == max_palette_size)
	{
		// Uses all palette colors. No need to re-map.
		//
		bit_depth_ = 8;
		dst_palette_ = src_palette_;
	}
	else
	{
		using IndexMap = std::array<unsigned char, max_palette_size>;
		auto index_map = IndexMap{};
		auto index = 0;

		for (auto i = 0; i < max_palette_size; ++i)
		{
			if (used_colors[i])
			{
				index_map[i] = static_cast<std::uint8_t>(index);
				dst_palette_[index] = src_palette_[i];
				++index;
			}
		}

		for (auto i = 0; i < area_; ++i)
		{
			const auto old_index = colors8_[i];
			const auto new_index = index_map[old_index];
			colors8_[i] = new_index;
		}

		if (palette_size <= (1U << 1))
		{
			bit_depth_ = 1;
		}
		else if (palette_size <= (1U << 4))
		{
			bit_depth_ = 4;
		}
		else
		{
			bit_depth_ = 8;
		}
	}

	palette_size_ = static_cast<int>(palette_size);
	stride_ = bstone::bmp::calculate_stride(width_, bit_depth_);
}

void ImageExtractor::decode_default_palette() noexcept
{
	width_ = 16;
	height_ = 16;
	area_ = width_ * height_;
	bit_depth_ = 0;
	palette_size_ = 0;
	stride_ = 0;

	for (auto i = 0; i < area_; ++i)
	{
		colors8_[i] = static_cast<std::uint8_t>(i);
	}

	remap_indexed_image();
}

void ImageExtractor::decode_wall_page(const std::uint8_t* src_colors) noexcept
{
	width_ = wall_width;
	height_ = wall_height;
	area_ = width_ * height_;
	bit_depth_ = 0;
	palette_size_ = 0;
	stride_ = 0;

	auto dst_colors = colors8_;

	for (auto w = 0; w < width_; ++w)
	{
		for (auto h = 0; h < height_; ++h)
		{
			const auto color_index = *src_colors++;
			const auto dst_index = (h * width_) + w;
			dst_colors[dst_index] = color_index;
		}
	}

	remap_indexed_image();
}

void ImageExtractor::decode_sprite_page(const bstone::Sprite& sprite) noexcept
{
	width_ = sprite_width;
	height_ = sprite_height;
	area_ = width_ * height_;
	bit_depth_ = 32;
	palette_size_ = 0;
	stride_ = bstone::bmp::calculate_stride(width_, bit_depth_);

	auto dst_colors = colors32_;
	const auto left = sprite.get_left();
	const auto right = sprite.get_right();
	const auto top = sprite.get_top();
	const auto bottom = sprite.get_bottom();

	for (auto w = 0; w < width_; ++w)
	{
		auto column = static_cast<const std::int16_t*>(nullptr);

		if (w >= left && w <= right)
		{
			column = sprite.get_column(w - left);
		}

		for (auto h = 0; h < height_; ++h)
		{
			auto dst_color = std::uint32_t{};

			if (column && h >= top && h <= bottom)
			{
				const auto color_index = column[h - top];

				if (color_index >= 0)
				{
					dst_color = src_palette_[color_index];
				}
			}

			const auto dst_index = (h * width_) + w;
			dst_colors[dst_index] = dst_color;
		}
	}
}

void ImageExtractor::save_bmp_rgb_palette(bstone::BinaryWriter& binary_writer)
try
{
	struct Bgr
	{
		std::uint8_t b;
		std::uint8_t g;
		std::uint8_t r;
	};

	using PaletteBgr = std::array<Bgr, max_palette_size>;
	auto palette_bgr = PaletteBgr{};

	for (auto i = 0; i < palette_size_; ++i)
	{
		const auto& src_color = dst_palette_[i];
		auto& dst_color = palette_bgr[i];
		dst_color.b = src_color & 0xFFU;
		dst_color.g = (src_color >> 8) & 0xFFU;
		dst_color.r = (src_color >> 16) & 0xFFU;
	}

	binary_writer.write(palette_bgr.data(), 3 * palette_size_);
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_rgbx_palette(bstone::BinaryWriter& binary_writer)
try
{
	if (bstone::Endian::is_big())
	{
		for (auto i = 0; i < palette_size_; ++i)
		{
			bstone::Endian::little(dst_palette_[i]);
		}
	}

	binary_writer.write(dst_palette_.data(), 4 * palette_size_);
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_palette(bstone::BinaryWriter& binary_writer)
try
{
	if (palette_size_ == (1 << bit_depth_))
	{
		save_bmp_rgb_palette(binary_writer);
	}
	else
	{
		save_bmp_rgbx_palette(binary_writer);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_1bpp_bits(bstone::BinaryWriter& binary_writer)
try
{
	constexpr auto initial_mask = 0x80U;

	const auto line_size = ((width_ + 7) / 8) * 8;
	const auto padding_size = stride_ - line_size;
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		auto mask = 0U;
		auto line_byte = line_buffer_.data() - 1;

		for (auto w = 0; w < width_; ++w)
		{
			if (mask == 0U)
			{
				mask = initial_mask;
				*(++line_byte) = 0U;
			}

			if (src_colors[w] != 0U)
			{
				*line_byte |= mask;
			}
		}

		for (auto i = 0; i < padding_size; ++i)
		{
			*line_byte++ = 0U;
		}

		binary_writer.write(line_buffer_.data(), line_size);
		src_colors -= width_;
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_4bpp_bits(bstone::BinaryWriter& binary_writer)
try
{
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		std::uninitialized_fill_n(line_buffer_.begin(), stride_, 0U);

		auto nibble_index = 0U;
		auto line_byte = line_buffer_.data();

		for (auto w = 0; w < width_; ++w)
		{
			const auto src_color = src_colors[w];
			*line_byte |= src_color << (4 * (nibble_index ^ 1U));

			if (nibble_index != 0U)
			{
				++line_byte;
			}

			nibble_index ^= 1U;
		}

		binary_writer.write(line_buffer_.data(), stride_);
		src_colors -= width_;
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_8bpp_bits(bstone::BinaryWriter& binary_writer)
try
{
	const auto padding_size = stride_ - width_;
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		binary_writer.write(src_colors, width_);
		binary_writer.write(padding_bytes, padding_size);
		src_colors -= width_;
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp_32bpp_bits(bstone::BinaryWriter& binary_writer)
try
{
	if (bstone::Endian::is_big())
	{
		for (auto i = 0; i < area_; ++i)
		{
			bstone::Endian::little_i(*colors32_++);
		}
	}

	const auto bits_byte_count = stride_ * height_;
	binary_writer.write(colors32_, bits_byte_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_bmp(const std::string& path)
try
{
	auto file_stream = bstone::FileStream{path, bstone::StreamOpenMode::write};
	auto binary_writer = bstone::BinaryWriter{&file_stream};

	const auto is_core_header =
		palette_size_ == palette_1bpp_size ||
		palette_size_ == palette_4bpp_size ||
		palette_size_ == palette_8bpp_size;

	const auto is_bpp32 = (bit_depth_ == 32);

	const auto info_header_size =
		is_bpp32 ? bstone::bmp::bitmapv4header_size : (
			is_core_header ? bstone::bmp::bitmapcoreheader_size :
				bstone::bmp::bitmapinfoheader_size);

	const auto bits_offset =
		bstone::bmp::bitmapfileheader_size +
		info_header_size +
		(is_bpp32 ? 0 : (palette_size_ * (is_core_header ? 3 : 4)));

	const auto bits_byte_count = stride_ * height_;
	const auto file_size = bits_offset + bits_byte_count;
	const auto compression = (is_bpp32 ? bstone::bmp::bi_bitfields : bstone::bmp::bi_rgb);

	// -------------------------------------------------------------------------
	// BITMAPFILEHEADER

	// bfType
	binary_writer.write_u16(bstone::Endian::little(std::uint16_t{bstone::bmp::type_bm}));

	// bfSize
	binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(file_size)));

	// bfReserved1
	binary_writer.write_u16(bstone::Endian::little(std::uint16_t{0}));

	// bfReserved2
	binary_writer.write_u16(bstone::Endian::little(std::uint16_t{0}));

	// bfOffBits
	binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(bits_offset)));

	// -------------------------------------------------------------------------
	// BITMAPCOREHEADER

	if (is_core_header)
	{
		// bcSize
		binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(info_header_size)));

		// bcWidth
		binary_writer.write_u16(bstone::Endian::little(static_cast<std::uint16_t>(width_)));

		// bcHeight
		binary_writer.write_u16(bstone::Endian::little(static_cast<std::uint16_t>(height_)));

		// bcPlanes
		binary_writer.write_u16(bstone::Endian::little(std::uint16_t{bstone::bmp::plane_count}));

		// bcBitCount
		binary_writer.write_u16(bstone::Endian::little(static_cast<std::uint16_t>(bit_depth_)));
	}

	// -------------------------------------------------------------------------
	// BITMAPINFOHEADER

	if (!is_core_header)
	{
		// biSize
		binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(info_header_size)));

		// biWidth
		binary_writer.write_s32(bstone::Endian::little(width_));

		// biHeight
		binary_writer.write_s32(bstone::Endian::little(is_bpp32 ? -height_ : height_));

		// biPlanes
		binary_writer.write_u16(bstone::Endian::little(std::uint16_t{bstone::bmp::plane_count}));

		// biBitCount
		binary_writer.write_u16(bstone::Endian::little(static_cast<std::uint16_t>(bit_depth_)));

		// biCompression
		binary_writer.write_u32(bstone::Endian::little(compression));

		// biSizeImage
		binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(bits_byte_count)));

		// biXPelsPerMeter
		binary_writer.write_s32(bstone::Endian::little(std::int32_t{0}));

		// biYPelsPerMeter
		binary_writer.write_s32(bstone::Endian::little(std::int32_t{0}));

		// biClrUsed
		binary_writer.write_u32(bstone::Endian::little(static_cast<std::uint32_t>(palette_size_)));

		// biClrImportant
		binary_writer.write_u32(bstone::Endian::little(std::uint32_t{0}));
	}

	// ----------------------------------------------------------------------
	// BITMAPV4HEADER

	if (is_bpp32)
	{
		constexpr std::uint8_t endpoints[bstone::bmp::ciexyztriple_size] = {};

		// bV4RedMask
		binary_writer.write_u32(bstone::Endian::little(0x00FF0000U));

		// bV4GreenMask
		binary_writer.write_u32(bstone::Endian::little(0x0000FF00U));

		// bV4BlueMask
		binary_writer.write_u32(bstone::Endian::little(0x000000FFU));

		// bV4AlphaMask
		binary_writer.write_u32(bstone::Endian::little(0xFF000000U));

		// bV4CSType
		binary_writer.write_u32(bstone::Endian::little(bstone::bmp::lcs_calibrated_rgb));

		// bV4Endpoints
		binary_writer.write(endpoints);

		// bV4GammaRed
		binary_writer.write_u32(bstone::Endian::little(0));

		// bV4GammaGreen
		binary_writer.write_u32(bstone::Endian::little(0));

		// bV4GammaBlue
		binary_writer.write_u32(bstone::Endian::little(0));
	}

	// -------------------------------------------------------------------------
	// Palette

	if (bit_depth_ <= 8)
	{
		save_bmp_palette(binary_writer);
	}

	// -------------------------------------------------------------------------
	// Colors

	switch (bit_depth_)
	{
		case 1: save_bmp_1bpp_bits(binary_writer); break;
		case 4: save_bmp_4bpp_bits(binary_writer); break;
		case 8: save_bmp_8bpp_bits(binary_writer); break;
		case 32: save_bmp_32bpp_bits(binary_writer); break;
		default: fail("Unknown bit depth.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::save_image(const std::string& name_prefix, int image_index)
try
{
	const auto& wall_index_string = ca_make_padded_asset_number_string(image_index);

	const auto& file_name = bstone::file_system::append_path(
		destination_dir_,
		name_prefix + wall_index_string + ".bmp");

	save_bmp(file_name);
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::extract_wall(int wall_index)
try
{
	const auto wall_page = bstone::globals::page_mgr->get(wall_index);

	if (wall_page == nullptr)
	{
		const auto error_message = std::string{} + "No wall page #" + std::to_string(wall_index) + ".";
		fail(error_message.c_str());
	}

	decode_wall_page(wall_page);
	save_image("wall_", wall_index);
}
catch (...)
{
	fail_nested(__func__);
}

void ImageExtractor::extract_sprite(int sprite_index)
try
{
	const auto cache_sprite_index = sprite_index;
	const auto sprite = sprite_cache_.cache(cache_sprite_index);
	decode_sprite_page(*sprite);
	save_image("sprite_", cache_sprite_index);
}
catch (...)
{
	fail_nested(__func__);
}

// ImageExtractor
// ==========================================================================


void ca_extract_vga_palette(const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};
	images_extractor.extract_vga_palette(destination_dir);
}

void ca_extract_walls(const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};
	images_extractor.extract_walls(destination_dir);
}

void ca_extract_sprites(const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};
	images_extractor.extract_sprites(destination_dir);
}

void ca_extract_music(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting music.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	auto audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
	auto audio_extractor = bstone::make_audio_extractor(*audio_content_mgr);

	const auto normalized_dst_dir = bstone::file_system::normalize_path(destination_dir);
	audio_extractor->extract_music(normalized_dst_dir);

	bstone::logger_->write(">>> ================");
}

void ca_extract_sfx(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting sfx.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	auto audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
	auto audio_extractor = bstone::make_audio_extractor(*audio_content_mgr);

	const auto normalized_dst_dir = bstone::file_system::normalize_path(destination_dir);
	audio_extractor->extract_sfx(normalized_dst_dir);

	bstone::logger_->write(">>> ================");
}

// ==========================================================================
// TextExtractor
//

class TextExtractorException :
	public bstone::Exception
{
public:
	explicit TextExtractorException(
		const std::string& message) noexcept
		:
		Exception{"TEXT_EXTRACTOR", message.c_str()}
	{
	}
}; // TextExtractorException


namespace
{


[[noreturn]]
void text_extractor_fail(
	const char* message)
{
	throw TextExtractorException{message};
}

[[noreturn]]
void text_extractor_fail(
	int number,
	const char* message)
{
	const auto error_message = std::string{} + "[Text #" + std::to_string(number) + "] " + message;

	text_extractor_fail(error_message.c_str());
}


} // namespace


class TextExtractor
{
public:
	TextExtractor();


	void extract_text(
		const std::string& dst_dir);


private:
	struct TextNumber
	{
		bool is_compressed_;
		int number_;
	}; // TextNumber


	using TextNumbers = std::vector<TextNumber>;
	using Buffer = std::vector<std::uint8_t>;


	TextNumbers text_numbers_;
	Buffer buffer_;


	void initialize_text();

	CompHeader_t deserialize_header(
		const int number,
		const std::uint8_t* const data);

	void extract_text(
		const std::string& dst_dir,
		const TextNumber& number);
}; // TextExtractor


TextExtractor::TextExtractor()
	:
	text_numbers_{}
{
	initialize_text();
}

void TextExtractor::extract_text(
	const std::string& dst_dir)
{
	bstone::logger_->write("File count: " + std::to_string(text_numbers_.size()));

	for (const auto text_number : text_numbers_)
	{
		extract_text(dst_dir, text_number);
	}
}

void TextExtractor::initialize_text()
{
	const auto& assets_info = get_assets_info();
	const auto is_compressed = assets_info.is_aog_sw_v2_x() || assets_info.is_ps();

	text_numbers_.reserve(50);

	text_numbers_.emplace_back(TextNumber{false, INFORMANT_HINTS});
	text_numbers_.emplace_back(TextNumber{false, NICE_SCIE_HINTS});
	text_numbers_.emplace_back(TextNumber{false, MEAN_SCIE_HINTS});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W1});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I1});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W2});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I2});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W3});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I3});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W4});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I4});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W5});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I5});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W6});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I6});
	text_numbers_.emplace_back(TextNumber{false, LEVEL_DESCS});
	text_numbers_.emplace_back(TextNumber{is_compressed, POWERBALLTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, TICSTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, MUSICTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, RADARTEXT});
	text_numbers_.emplace_back(TextNumber{false, HELPTEXT});
	text_numbers_.emplace_back(TextNumber{false, SAGATEXT});
	text_numbers_.emplace_back(TextNumber{false, LOSETEXT});
	text_numbers_.emplace_back(TextNumber{false, ORDERTEXT});
	text_numbers_.emplace_back(TextNumber{false, CREDITSTEXT});
	text_numbers_.emplace_back(TextNumber{false, MUSTBE386TEXT});
	text_numbers_.emplace_back(TextNumber{false, QUICK_INFO1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, QUICK_INFO2_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADINFO_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CALJOY1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CALJOY2_TEXT});
	text_numbers_.emplace_back(TextNumber{false, READTHIS_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG0_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG4_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG5_TEXT});
	text_numbers_.emplace_back(TextNumber{false, FLOORMSG_TEXT});
	text_numbers_.emplace_back(TextNumber{false, YOUWIN_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CHANGEVIEW_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADCHECKSUMTEXT});
	text_numbers_.emplace_back(TextNumber{false, DIZ_ERR_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADLEVELSTEXT});
	text_numbers_.emplace_back(TextNumber{false, BADSAVEGAME_TEXT});

	std::sort(
		text_numbers_.begin(),
		text_numbers_.end(),
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.number_ < rhs.number_;
		}
	);

	const auto non_zero_number_it = std::find_if(
		text_numbers_.begin(),
		text_numbers_.end(),
		[](const auto item)
		{
			return item.number_ != 0;
		}
	);

	if (non_zero_number_it == text_numbers_.end())
	{
		text_extractor_fail("Empty list.");
	}

	text_numbers_.erase(text_numbers_.begin(), non_zero_number_it);
}

CompHeader_t TextExtractor::deserialize_header(
	const int number,
	const std::uint8_t* const data)
{
	auto stream = bstone::MemoryStream
	{
		CompHeader_t::class_size,
		0,
		data
	};

	auto reader = bstone::BinaryReader{&stream};

	auto result = CompHeader_t{};

	reader.read(result.NameId, 4);
	result.OriginalLen = bstone::Endian::little(reader.read_u32());
	result.CompType = static_cast<ct_TYPES>(bstone::Endian::little(reader.read_u16()));
	result.CompressLen = bstone::Endian::little(reader.read_u32());

	const auto four_cc = std::string{result.NameId, 4};

	if (four_cc != JAMP)
	{
		text_extractor_fail(number, "Unsupported FOURCC.");
	}

	return result;
}

void TextExtractor::extract_text(
	const std::string& dst_dir,
	const TextNumber& text_number)
{
	const auto number = text_number.number_;

	CA_CacheGrChunk(static_cast<std::int16_t>(number));

	auto text_data = grsegs[number].data();
	auto text_size = grsegs_sizes_[number];

	if (text_number.is_compressed_)
	{
		constexpr auto header_size = CompHeader_t::class_size;

		if (text_size < header_size)
		{
			text_extractor_fail(number, "Header too small.");
		}

		constexpr auto max_uncompressed_size = 4'096;

		const auto compressed_header = deserialize_header(number, text_data);
		const auto pure_data_size = text_size - header_size;

		if (compressed_header.CompressLen > static_cast<std::uint32_t>(pure_data_size) ||
			compressed_header.OriginalLen > max_uncompressed_size)
		{
			text_extractor_fail(number, "Length(s) out of range.");
		}

		if (compressed_header.CompType != ct_LZH)
		{
			text_extractor_fail(number, "Expected LZH compression type.");
		}

		buffer_.resize(compressed_header.OriginalLen);

		const auto decoded_size = LZH_Decompress(
			text_data + header_size,
			buffer_.data(),
			compressed_header.OriginalLen,
			compressed_header.CompressLen
		);

		buffer_.resize(decoded_size);

		text_data = buffer_.data();
		text_size = decoded_size;
	}
	else
	{
		text_data = grsegs[number].data();
	}

	const auto& number_string = ca_make_padded_asset_number_string(number);

	const auto& file_name = bstone::file_system::append_path(
		dst_dir,
		"text_" + number_string + ".txt"
	);

	auto file_stream = bstone::FileStream{file_name, bstone::StreamOpenMode::write};

	if (!file_stream.is_open())
	{
		text_extractor_fail(number, ("Failed to open \"" + file_name + "\" for writing.").c_str());
	}

	if (!file_stream.write(text_data, text_size))
	{
		text_extractor_fail(number, "Write I/O error.");
	}
}

//
// TextExtractor
// ==========================================================================

void ca_extract_texts(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting text.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	auto text_extractor = TextExtractor{};
	text_extractor.extract_text(bstone::file_system::normalize_path(destination_dir));

	bstone::logger_->write(">>> ================");
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
}


namespace
{


class CaResourceException :
	public bstone::Exception
{
public:
	explicit CaResourceException(
		const char* message) noexcept
		:
		bstone::Exception{"CA_RESOURCE", message}
	{
	}
}; // CaResourceException


[[noreturn]]
void ca_resource_fail(
	const char* message)
{
	throw CaResourceException{message};
}

[[noreturn]]
void ca_resource_fail_nested(
	const char* message)
{
	std::throw_with_nested(CaResourceException{message});
}


} // namespace


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

void ca_make_resource_path_name(
	const std::string& resource_name,
	std::string& path_name)
try
{
	if (resource_name.empty())
	{
		ca_resource_fail("Empty name.");
	}

	path_name = (mod_dir_.empty() ? data_dir_ : mod_dir_);

	const auto& assets_info = get_assets_info();
	path_name = bstone::file_system::append_path(path_name, assets_info.get_base_path_name());
	path_name = bstone::file_system::append_path(path_name, resource_name);
}
catch (...)
{
	ca_resource_fail_nested(__func__);
}

void ca_make_sprite_resource_path_name(
	int sprite_id,
	std::string& path_name)
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw())
	{
		sprite_id = ca_map_aog_sw_sprite_id_to_aog_full(sprite_id);
	}

	ca_make_resource_path_name("sprite_" + ca_make_padded_asset_number_string(sprite_id), path_name);
}

void ca_make_wall_resource_path_name(
	int wall_id,
	std::string& path_name)
{
	ca_make_resource_path_name("wall_" + ca_make_padded_asset_number_string(wall_id), path_name);
}
