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

#include <algorithm>
#include <memory>

#include "audio.h"
#include "jm_lzh.h"
#include "id_heads.h"
#include "id_pm.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "gfxv.h"

#include "bstone_binary_writer.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_system.h"
#include "bstone_logger.h"
#include "bstone_rgb_palette.h"
#include "bstone_sdl2_types.h"
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
AudioSegments audiosegs;
GrSegments grsegs;
GrSegmentSizes grsegs_sizes_;

GrNeeded grneeded;
std::uint8_t ca_levelbit, ca_levelnum;

std::int16_t profilehandle, debughandle;

void InitDigiMap();

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
extern std::uint8_t audiohead;
extern std::uint8_t audiodict;


void CA_CannotOpen(
	const std::string& string);

GrStarts grstarts; // array of offsets in egagraph, -1 for sparse

AudioStarts audiostarts; // array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode* grhuffman;
#else
huffnode grhuffman[255];
#endif

#ifdef AUDIOHEADERLINKED
huffnode* audiohuffman;
#else
huffnode audiohuffman[255];
#endif

bstone::FileStream grhandle; // handle to EGAGRAPH
bstone::FileStream maphandle; // handle to MAPTEMP / GAMEMAPS
bstone::FileStream audiohandle; // handle to AUDIOT / AUDIO

std::int32_t chunkcomplen;
std::int32_t chunkexplen;

bool old_is_sound_enabled;

static Buffer ca_buffer;

static const int BUFFERSIZE = 0x10000;


// BBi
std::string ca_make_padded_asset_number_string(
	const int number)
{
	constexpr auto max_padded_numer_string_length = 8;

	auto result = std::to_string(number);
	result.insert(0, max_padded_numer_string_length - result.size(), '0');
	return result;
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
	ca_open_resource(Assets::get_map_data_base_name(), maphandle);
#endif
}

void CloseMapFile()
{
	maphandle.close();
}

void OpenAudioFile()
{
#ifndef AUDIOHEADERLINKED
	ca_open_resource(Assets::get_audio_data_base_name(), audiohandle);
#else
	// TODO Remove or fix
	if ((audiohandle = open("AUDIO."EXTENSION,
		O_RDONLY | O_BINARY, S_IREAD)) == -1)
	{
		CA_ERROR(SETUPAUDIO_CANT_OPEN);
	}
#endif
}

void CloseAudioFile()
{
	audiohandle.close();
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
	CloseAudioFile();
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
	CAL_SetupAudioFile();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;

	ca_buffer.reserve(BUFFERSIZE);
}

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/
void CA_CacheAudioChunk(
	std::int16_t chunk)
{
	std::int32_t pos;
	std::int32_t compressed;
#ifdef AUDIOHEADERLINKED
	std::int32_t expanded;
	memptr bigbufferseg;
	std::uint8_t* source;
#endif

	if (!audiosegs[chunk].empty())
	{
		return; // already in memory
	}

	//
	// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
	// a larger buffer
	//
	pos = audiostarts[chunk];
	compressed = audiostarts[chunk + 1] - pos;

	OpenAudioFile();

	audiohandle.set_position(pos);

#ifndef AUDIOHEADERLINKED

	audiosegs[chunk].resize(compressed);
	audiohandle.read(audiosegs[chunk].data(), compressed);

#else

	if (compressed <= BUFFERSIZE)
	{
		CA_FarRead(audiohandle, bufferseg, compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg, compressed);
		if (mmerror)
		{
			CloseAudioFile();
			return;
		}
		MM_SetLock(&bigbufferseg, true);
		CA_FarRead(audiohandle, bigbufferseg, compressed);
		source = bigbufferseg;
	}

	expanded = *(std::int32_t*)source;
	source += 4; // skip over length
	MM_GetPtr(&(memptr)audiosegs[chunk], expanded);
	if (mmerror)
	{
		goto done;
	}
	CAL_HuffExpand(source, audiosegs[chunk], expanded, audiohuffman, false);

done:
	if (compressed > BUFFERSIZE)
	{
		MM_FreePtr(&bigbufferseg);
	}
#endif

	CloseAudioFile();
}

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/
void CA_LoadAllSounds()
{
	auto start = 0;

	if (old_is_sound_enabled)
	{
		start = STARTADLIBSOUNDS;
	}

	if (sd_is_sound_enabled_)
	{
		start = STARTADLIBSOUNDS;
	}
	else
	{
		return;
	}

	for (int i = 0; i < NUMSOUNDS; ++i)
	{
		CA_CacheAudioChunk(start++);
	}

	old_is_sound_enabled = sd_is_sound_enabled_;
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
		Quit("There are no assets for level index " + std::to_string(mapnum) + '.');
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
		Quit("Up past level 7.");
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
		Quit("Down past level 0.");
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
	Quit("Can't open " + string + "!\n");
}

void UNCACHEGRCHUNK(
	int chunk)
{
	grsegs[chunk] = std::move(GrSegment{});

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
		Quit("Invalid script.");
	}

	if (strip_xx)
	{
		length -= 3;
	}

	return std::string(script, length);
}

void initialize_ca_constants()
{
	const auto& assets_info = AssetsInfo{};

	const auto total_level_count = assets_info.get_episode_count() * assets_info.get_levels_per_episode();

	mapheaderseg.resize(total_level_count);
}

bool ca_is_resource_exists(
	const std::string& file_name)
{
	const auto path = data_dir_ + file_name;

	auto is_open = false;

	is_open = bstone::FileStream::is_exists(path);

	if (!is_open)
	{
		auto&& file_name_lc = bstone::StringHelper::to_lower_ascii(file_name);
		const auto path_lc = data_dir_ + file_name_lc;

		is_open = bstone::FileStream::is_exists(path_lc);
	}

	return is_open;
}

bool ca_open_resource_non_fatal(
	const std::string& data_dir,
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream)
{
	const auto file_name = file_name_without_ext + file_extension;
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
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream)
{
	if (!mod_dir_.empty())
	{
		const auto mod_dir_result = ca_open_resource_non_fatal(
			mod_dir_, file_name_without_ext, file_extension, file_stream);

		if (mod_dir_result)
		{
			return true;
		}
	}

	const auto data_dir_result = ca_open_resource_non_fatal(
		data_dir_, file_name_without_ext, file_extension, file_stream);

	return data_dir_result;
}

void ca_open_resource(
	const std::string& file_name_without_ext,
	bstone::FileStream& file_stream)
{
	auto assets_info = AssetsInfo{};

	const auto is_open = ca_open_resource_non_fatal(file_name_without_ext, assets_info.get_extension(), file_stream);

	if (!is_open)
	{
		const auto path = data_dir_ + file_name_without_ext + assets_info.get_extension();

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
	const std::string& base_name,
	const std::string& extension)
{
	auto file_stream = bstone::FileStream{};

	if (!ca_open_resource_non_fatal(base_name, extension, file_stream))
	{
		return {};
	}

	return ca_calculate_hash(file_stream);
}

std::string ca_calculate_hash(
	const std::string& data_dir,
	const std::string& base_name,
	const std::string& extension)
{
	auto file_stream = bstone::FileStream{};

	if (!ca_open_resource_non_fatal(data_dir, base_name, extension, file_stream))
	{
		return {};
	}

	return ca_calculate_hash(file_stream);
}

void ca_calculate_hashes()
{
	bstone::logger_->write();
	bstone::logger_->write("Calculating resource hashes...");

	auto sha1 = bstone::Sha1{};

	for (const auto& base_name : Assets::get_base_names())
	{
		for (const auto& extension : Assets::get_extensions())
		{
			const auto& sha1_string = ca_calculate_hash(base_name, extension);

			if (sha1_string.empty())
			{
				continue;
			}

			bstone::logger_->write(base_name.get() + extension.get() + ": " + sha1_string);
		}
	}
}


std::string AssetsInfo::empty_extension_;

AssetsVersion AssetsInfo::version_;
AssetsCRefString AssetsInfo::extension_ = empty_extension_;
AssetsCRefStrings AssetsInfo::base_names_;
AssetsBaseNameToHashMap AssetsInfo::base_name_to_hash_map_;
int AssetsInfo::gfx_header_offset_count_;
std::string AssetsInfo::levels_hash_;
bool AssetsInfo::are_modded_levels_;
int AssetsInfo::episode_count_;
int AssetsInfo::levels_per_episode_;
int AssetsInfo::stats_levels_per_episode_;
int AssetsInfo::total_levels_;
int AssetsInfo::barrier_switches_per_level_;
int AssetsInfo::max_barrier_switches_per_level_bits_;


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
			Quit("No assets information.");
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
			Quit("No assets information.");
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
			Quit("No assets information.");
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
			Quit("No assets information.");
		}
	}

	total_levels_ = episode_count_ * levels_per_episode_;
}

const std::string& AssetsInfo::get_extension() const
{
	return extension_;
}

void AssetsInfo::set_extension(
	const std::string& extension)
{
	extension_ = extension;
}

const AssetsCRefStrings& AssetsInfo::get_base_names() const
{
	return base_names_;
}

void AssetsInfo::set_base_names(
	const AssetsCRefStrings& base_names)
{
	base_names_ = base_names;
}

const AssetsBaseNameToHashMap& AssetsInfo::get_base_name_to_hash_map() const
{
	return base_name_to_hash_map_;
}

void AssetsInfo::set_base_name_to_hash_map(
	const AssetsBaseNameToHashMap& base_name_to_hash_map)
{
	base_name_to_hash_map_ = base_name_to_hash_map;
}

const std::string& AssetsInfo::get_levels_hash_string() const
{
	return levels_hash_;
}

void AssetsInfo::set_levels_hash(
	const std::string& levels_hash)
{
	levels_hash_ = levels_hash;

	static const auto all_levels_hashes = AssetsCRefStrings
	{
		Assets::get_aog_full_v1_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v2_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v2_1_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v3_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),

		Assets::get_aog_sw_v1_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v2_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v2_1_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v3_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),

		Assets::get_ps_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
	}; // all_levels_hashes

	are_modded_levels_ = !Assets::are_official_levels(levels_hash_);
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
		Quit("No assets information.");
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
		Quit("No assets information.");
	}
}


const std::string& Assets::get_audio_header_base_name()
{
	static const auto audio_header_base_name = std::string{"AUDIOHED"};

	return audio_header_base_name;
}

const std::string& Assets::get_audio_data_base_name()
{
	static const auto audio_data_base_name = std::string{"AUDIOT"};

	return audio_data_base_name;
}

const std::string& Assets::get_map_header_base_name()
{
	static const auto map_header_base_name = std::string{"MAPHEAD"};

	return map_header_base_name;
}

const std::string& Assets::get_map_data_base_name()
{
	static const auto map_data_base_name = std::string{"MAPTEMP"};

	return map_data_base_name;
}

const std::string& Assets::get_gfx_dictionary_base_name()
{
	static const auto gfx_dictionary_base_name = std::string{"VGADICT"};

	return gfx_dictionary_base_name;
}

const std::string& Assets::get_gfx_header_base_name()
{
	static const auto gfx_header_base_name = std::string{"VGAHEAD"};

	return gfx_header_base_name;
}

const std::string& Assets::get_gfx_data_base_name()
{
	static const auto gfx_data_base_name = std::string{"VGAGRAPH"};

	return gfx_data_base_name;
}

const std::string& Assets::get_page_file_base_name()
{
	static const auto page_file_base_name = std::string{"VSWAP"};

	return page_file_base_name;
}

const std::string& Assets::get_episode_6_fmv_base_name()
{
	static const auto episode_6_fmv_base_name = std::string{"EANIM"};

	return episode_6_fmv_base_name;
}

const std::string& Assets::get_episode_3_5_fmv_base_name()
{
	static const auto episode_3_5_fmv_base_name = std::string{"GANIM"};

	return episode_3_5_fmv_base_name;
}

const std::string& Assets::get_intro_fmv_base_name()
{
	static const auto get_intro_fmv_base_name = std::string{"IANIM"};

	return get_intro_fmv_base_name;
}

const std::string& Assets::get_episode_2_4_fmv_base_name()
{
	static const auto get_episode_2_4_fmv_base_name = std::string{"SANIM"};

	return get_episode_2_4_fmv_base_name;
}

const std::string& Assets::get_aog_sw_extension()
{
	static const auto get_aog_sw_extension = std::string{".BS1"};

	return get_aog_sw_extension;
}

const std::string& Assets::get_aog_full_extension()
{
	static const auto get_aog_full_extension = std::string{".BS6"};

	return get_aog_full_extension;
}

const std::string& Assets::get_ps_extension()
{
	static const auto get_ps_extension = std::string{".VSI"};

	return get_ps_extension;
}

const AssetsCRefStrings& Assets::get_extensions()
{
	static const auto extensions = AssetsCRefStrings
	{
		get_aog_sw_extension(),
		get_aog_full_extension(),
		get_ps_extension(),
	}; // extensions

	return extensions;
}

const AssetsCRefStrings& Assets::get_base_names()
{
	static const auto base_names = AssetsCRefStrings
	{
		get_audio_header_base_name(),
		get_audio_data_base_name(),

		get_map_header_base_name(),
		get_map_data_base_name(),

		get_gfx_dictionary_base_name(),
		get_gfx_header_base_name(),
		get_gfx_data_base_name(),

		get_page_file_base_name(),

		get_episode_6_fmv_base_name(),
		get_episode_3_5_fmv_base_name(),
		get_intro_fmv_base_name(),
		get_episode_2_4_fmv_base_name(),
	}; // base_names

	return base_names;
}

const AssetsCRefStrings& Assets::get_aog_sw_base_names()
{
	static const auto aog_sw_base_names = AssetsCRefStrings
	{
		get_audio_header_base_name(),
		get_audio_data_base_name(),

		get_map_header_base_name(),
		get_map_data_base_name(),

		get_gfx_dictionary_base_name(),
		get_gfx_header_base_name(),
		get_gfx_data_base_name(),

		get_page_file_base_name(),

		get_intro_fmv_base_name(),
		get_episode_2_4_fmv_base_name(),
	}; // aog_sw_base_names

	return aog_sw_base_names;
}

const AssetsCRefStrings& Assets::get_aog_full_base_names()
{
	static const auto aog_full_base_names = AssetsCRefStrings
	{
		get_audio_header_base_name(),
		get_audio_data_base_name(),

		get_map_header_base_name(),
		get_map_data_base_name(),

		get_gfx_dictionary_base_name(),
		get_gfx_header_base_name(),
		get_gfx_data_base_name(),

		get_page_file_base_name(),

		get_episode_6_fmv_base_name(),
		get_episode_3_5_fmv_base_name(),
		get_intro_fmv_base_name(),
		get_episode_2_4_fmv_base_name(),
	}; // aog_full_base_names

	return aog_full_base_names;
}

const AssetsCRefStrings& Assets::get_ps_base_names()
{
	static const auto ps_base_names = AssetsCRefStrings
	{
		get_audio_header_base_name(),
		get_audio_data_base_name(),

		get_map_header_base_name(),
		get_map_data_base_name(),

		get_gfx_dictionary_base_name(),
		get_gfx_header_base_name(),
		get_gfx_data_base_name(),

		get_page_file_base_name(),

		get_episode_6_fmv_base_name(),
		get_intro_fmv_base_name(),
	}; // ps_base_names

	return ps_base_names;
}


const AssetsBaseNameToHashMap& Assets::get_aog_sw_v1_0_base_name_to_hash_map()
{
	static auto aog_sw_v1_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "0c3de403b524107809fa9308b730d60e8e41ba93"},
		{get_audio_data_base_name(), "3d4e8b62aa4683671027513ad9720f76f176ba5b"},

		{get_map_header_base_name(), "55b9eb9ed555b0f249c2cefd54ecc3f511bfcd55"},
		{get_map_data_base_name(), "293464e7143ff7e6faf5f5b20799e76a394d65bf"},

		{get_gfx_dictionary_base_name(), "b54d48d35d095e27a3c9130cfa59ed9c4f05abe7"},
		{get_gfx_header_base_name(), "cc44c362e9e2c9f7b9fb2e4ba31331dde42a1e96"},
		{get_gfx_data_base_name(), "d1fa19131281d30787125f946881652be876f57a"},

		{get_page_file_base_name(), "37412ca9139562fc31330d72470822de4d9ceb4a"},

		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_sw_v1_0_base_name_to_hash_map

	return aog_sw_v1_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v2_0_base_name_to_hash_map()
{
	static auto aog_sw_v2_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "0c3de403b524107809fa9308b730d60e8e41ba93"},
		{get_audio_data_base_name(), "3d4e8b62aa4683671027513ad9720f76f176ba5b"},

		{get_map_header_base_name(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		{get_map_data_base_name(), "bbb3cd6ab3e742eada427862504eba06437036d0"},

		{get_gfx_dictionary_base_name(), "058fe0b59c7aa020bf4e7509103892d4c7459aa6"},
		{get_gfx_header_base_name(), "6c0e273df7fd3940c038fc20a5bdfb81cd50e7ef"},
		{get_gfx_data_base_name(), "e14b6172b6ab7568d3736f087f07a0df5eac5dad"},

		{get_page_file_base_name(), "dbde907ba1110bef445d6daae283c4520d6951b4"},

		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_sw_v2_0_base_name_to_hash_map

	return aog_sw_v2_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v2_1_base_name_to_hash_map()
{
	static auto aog_sw_v2_1_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		{get_map_data_base_name(), "bbb3cd6ab3e742eada427862504eba06437036d0"},

		{get_gfx_dictionary_base_name(), "ed1ab61ad2529e046f966d9c1627508f76693ef8"},
		{get_gfx_header_base_name(), "0635a4a2b823eef2904ed1d590d98362fb16621d"},
		{get_gfx_data_base_name(), "b59cbe3793b4612b06254ece48bf8e961ab6f528"},

		{get_page_file_base_name(), "dbde907ba1110bef445d6daae283c4520d6951b4"},

		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_sw_v2_1_base_name_to_hash_map

	return aog_sw_v2_1_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v3_0_base_name_to_hash_map()
{
	static auto aog_sw_v3_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "04a8e4e7c360f6033dc70f7b09edbc4a6447e462"},
		{get_map_data_base_name(), "bbb3cd6ab3e742eada427862504eba06437036d0"},

		{get_gfx_dictionary_base_name(), "537676ddcafeee415c22bb9d00097b25bc7e13c5"},
		{get_gfx_header_base_name(), "e7fb8f241b8fa94739c3bd09ea05c1afc7bbfc95"},
		{get_gfx_data_base_name(), "9f9f4c40c9637af472ce5e1a360e51364c3a418a"},

		{get_page_file_base_name(), "dbde907ba1110bef445d6daae283c4520d6951b4"},

		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_sw_v3_0_base_name_to_hash_map

	return aog_sw_v3_0_base_name_to_hash_map;
}


const AssetsBaseNameToHashMap& Assets::get_aog_full_v1_0_base_name_to_hash_map()
{
	static auto aog_full_v1_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "6f19a144d8985e90f096fc1c67ade58e9051235c"},
		{get_map_data_base_name(), "4d00c5f5c843f99a266bd938648192a6eff17b5a"},

		{get_gfx_dictionary_base_name(), "d4f81ace1701a7338d43ce07723c2adaafdc837c"},
		{get_gfx_header_base_name(), "639ec9e7a81ad83fc5b5c557cf4fc5fa28b9676b"},
		{get_gfx_data_base_name(), "02dc27d4810e3ffa26540b310eac6091f5e16de0"},

		{get_page_file_base_name(), "0a700732ccbc72f95318a6226a7e1ad78ac713bb"},

		{get_episode_6_fmv_base_name(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		{get_episode_3_5_fmv_base_name(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_full_v1_0_base_name_to_hash_map

	return aog_full_v1_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v2_0_base_name_to_hash_map()
{
	static auto aog_full_v2_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		{get_map_data_base_name(), "8b48e7eb859382a4c84948c5a62899194288e853"},

		{get_gfx_dictionary_base_name(), "e83b690836c9edf9ef60f6189b8384fb2319b735"},
		{get_gfx_header_base_name(), "48b18caa86151610957b64b207cf2a2977ef7d57"},
		{get_gfx_data_base_name(), "67a679e3b107db8685ba5ff1643a38f9291b00bf"},

		{get_page_file_base_name(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},

		{get_episode_6_fmv_base_name(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		{get_episode_3_5_fmv_base_name(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_full_v2_0_base_name_to_hash_map

	return aog_full_v2_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v2_1_base_name_to_hash_map()
{
	static auto aog_full_v2_1_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		{get_map_data_base_name(), "8b48e7eb859382a4c84948c5a62899194288e853"},

		{get_gfx_dictionary_base_name(), "e4ae3ef9a3ac158a832092b7b5487227337c6f13"},
		{get_gfx_header_base_name(), "1a3864cd12de4b8fd6be023b73d2d79e91b9018d"},
		{get_gfx_data_base_name(), "6868e6cc4f8cb9160a218c5bce680a11f64c675a"},

		{get_page_file_base_name(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},

		{get_episode_6_fmv_base_name(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		{get_episode_3_5_fmv_base_name(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_full_v2_1_base_name_to_hash_map

	return aog_full_v2_1_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v3_0_base_name_to_hash_map()
{
	static auto aog_full_v3_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2"},
		{get_audio_data_base_name(), "b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25"},

		{get_map_header_base_name(), "028f624e150f84ffc8336336cb0ecea0932cc22d"},
		{get_map_data_base_name(), "8b48e7eb859382a4c84948c5a62899194288e853"},

		{get_gfx_dictionary_base_name(), "60da35e506d57753f83cca5f232a76dd1cf074ba"},
		{get_gfx_header_base_name(), "ff165ca5924d738853587a7e4ab1cd239e10e359"},
		{get_gfx_data_base_name(), "001037fafdff124befb5437c563d0b9c613b3c00"},

		{get_page_file_base_name(), "6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb"},

		{get_episode_6_fmv_base_name(), "af5af59ad7ed17517d87b3ece3c0cac23ade535b"},
		{get_episode_3_5_fmv_base_name(), "9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e"},
		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
		{get_episode_2_4_fmv_base_name(), "22bf818465da0f32eef9611de936cba9966b14aa"},
	}; // aog_full_v3_0_base_name_to_hash_map

	return aog_full_v3_0_base_name_to_hash_map;
}


const AssetsBaseNameToHashMap& Assets::get_ps_base_name_to_hash_map()
{
	static auto ps_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{get_audio_header_base_name(), "2ed9a587926b2b455f8176788f595d03f6359171"},
		{get_audio_data_base_name(), "ecac8a800c5d021c8b68d170066c8859df9bd79a"},

		{get_map_header_base_name(), "c3fb0a9b81e0240d12c952fe5b57b78c1108aa48"},
		{get_map_data_base_name(), "acd03031d526eeaee64072518adf6814f96e2a37"},

		{get_gfx_dictionary_base_name(), "061692d166f68d7e0b81568725439078cc728f33"},
		{get_gfx_header_base_name(), "ad5fed9de5d1d82145df3ff2286e4bc62cb80e2a"},
		{get_gfx_data_base_name(), "e619a3e6245f79888da5436df138d63204d2caba"},

		{get_page_file_base_name(), "4113ce83c42e69dc5cb20de79f41971a100f876e"},

		{get_episode_6_fmv_base_name(), "b668330e56ceb069c1b972cefddd8a3d618a14af"},
		{get_intro_fmv_base_name(), "ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd"},
	}; // ps_base_name_to_hash_map

	return ps_base_name_to_hash_map;
}

bool Assets::are_official_levels(
	const std::string& levels_hash)
{
	static const auto all_levels_hashes = AssetsCRefStrings
	{
		Assets::get_aog_full_v1_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v2_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v2_1_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_full_v3_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),

		Assets::get_aog_sw_v1_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v2_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v2_1_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
		Assets::get_aog_sw_v3_0_base_name_to_hash_map().at(Assets::get_map_data_base_name()),

		Assets::get_ps_base_name_to_hash_map().at(Assets::get_map_data_base_name()),
	}; // all_levels_hashes

	const auto result = std::any_of(
		all_levels_hashes.cbegin(),
		all_levels_hashes.cend(),
		[&](const std::string& item)
	{
		return item == levels_hash;
	}
	);

	return result;
}


// ==========================================================================
// ImageExtractor
//

class ImageExtractor
{
public:
	bool is_initialized() const;

	bool initialize();

	void uninitialize();

	void extract_vga_palette(
		const std::string& destination_dir);

	void extract_walls(
		const std::string& destination_dir);

	void extract_sprites(
		const std::string& destination_dir);


private:
	using Palette = std::vector<SDL_Color>;


	bool is_initialized_;
	int sprite_count_;
	bstone::SdlSurfaceUPtr sdl_surface_16x16x8_;
	bstone::SdlSurfaceUPtr sdl_surface_64x64x8_;
	bstone::SdlSurfaceUPtr sdl_surface_64x64x32_;
	std::string destination_dir_;
	std::string destination_path_;
	bstone::SpriteCache sprite_cache_;
	Palette vga_palette_;


	void set_palette(
		bstone::SdlSurfacePtr sdl_surface,
		const std::uint8_t* const vga_palette);

	void set_palette(
		bstone::SdlSurfacePtr sdl_surface,
		const Palette& palette);

	void uninitialize_vga_palette();

	void initialize_vga_palette();

	void uninitialize_surface_64x64x8();

	bool initialize_surface_16x16x8();

	void uninitialize_surface_16x16x8();

	bool initialize_surface_64x64x8();

	void uninitialize_surface_64x64x32();

	bool initialize_surface_64x64x32();

	void convert_wall_page_into_surface(
		const std::uint8_t* const src_indices);

	void convert_sprite_page_into_surface(
		const bstone::Sprite& sprite);

	bool save_image(
		const std::string& name_prefix,
		const int image_index,
		bstone::SdlSurfacePtr sdl_surface);

	bool extract_wall(
		const int wall_index);

	bool extract_sprite(
		const int sprite_index);
}; // ImageExtractor


bool ImageExtractor::is_initialized() const
{
	return is_initialized_;
}

bool ImageExtractor::initialize()
{
	if (!initialize_surface_16x16x8())
	{
		return false;
	}

	if (!initialize_surface_64x64x8())
	{
		return false;
	}

	if (!initialize_surface_64x64x32())
	{
		return false;
	}

	initialize_vga_palette();

	is_initialized_ = true;

	return true;
}

void ImageExtractor::uninitialize()
{
	is_initialized_ = false;

	uninitialize_surface_64x64x8();
	uninitialize_surface_64x64x32();
	uninitialize_vga_palette();
}

void ImageExtractor::extract_vga_palette(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting VGA palette.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	if (!is_initialized_)
	{
		bstone::logger_->write_error("Not initialized.");

		return;
	}

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	set_palette(sdl_surface_16x16x8_.get(), vga_palette_);

	const auto pitch = sdl_surface_16x16x8_->pitch;

	auto dst_indices = static_cast<std::uint8_t*>(sdl_surface_16x16x8_->pixels);

	auto src_index = std::uint8_t{};

	for (int h = 0; h < 16; ++h)
	{
		for (int w = 0; w < 16; ++w)
		{
			const auto dst_index = (h * pitch) + w;

			dst_indices[dst_index] = src_index;

			src_index += 1;
		}
	}

	const auto& file_name = bstone::file_system::append_path(
		destination_dir_,
		"vga_palette.bmp"
	);

	const auto sdl_result = SDL_SaveBMP(sdl_surface_16x16x8_.get(), file_name.c_str());

	if (sdl_result != 0)
	{
		auto error_message = "Failed to save VGA palette into \"" + file_name + "\". ";
		error_message += SDL_GetError();

		bstone::logger_->write_error(error_message);

		return;
	}

	bstone::logger_->write(">>> ================");
}

void ImageExtractor::extract_walls(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting walls.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");
	bstone::logger_->write("File count: " + std::to_string(PMSpriteStart));

	if (!is_initialized_)
	{
		bstone::logger_->write_error("Not initialized.");

		return;
	}

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	set_palette(sdl_surface_64x64x8_.get(), vga_palette_);

	for (int i = 0; i < PMSpriteStart; ++i)
	{
		extract_wall(i);
	}

	bstone::logger_->write(">>> ================");
}

void ImageExtractor::extract_sprites(
	const std::string& destination_dir)
{
	sprite_count_ = ChunksInFile - PMSpriteStart - 1;

	if (sprite_count_ < 0)
	{
		sprite_count_ = 0;
	}

	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting sprites.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");
	bstone::logger_->write("File count: " + std::to_string(sprite_count_));

	if (!is_initialized_)
	{
		bstone::logger_->write_error("Not initialized.");

		return;
	}

	destination_dir_ = bstone::file_system::normalize_path(destination_dir);

	for (int i = 0; i < sprite_count_; ++i)
	{
		extract_sprite(i);
	}

	bstone::logger_->write(">>> ================");
}

void ImageExtractor::set_palette(
	bstone::SdlSurfacePtr sdl_surface,
	const std::uint8_t* const vga_palette)
{
	assert(sdl_surface);
	assert(vga_palette);
	assert(sdl_surface->format);
	assert(sdl_surface->format->palette);
	assert(sdl_surface->format->palette->ncolors == bstone::RgbPalette::get_max_color_count());

	auto& sdl_palette = *sdl_surface->format->palette;

	for (int i = 0; i < bstone::RgbPalette::get_max_color_count(); ++i)
	{
		const auto src_color = vga_palette + (i * 3);
		auto& dst_color = sdl_palette.colors[i];

		dst_color.r = static_cast<Uint8>((255 * src_color[0]) / 63);
		dst_color.g = static_cast<Uint8>((255 * src_color[1]) / 63);
		dst_color.b = static_cast<Uint8>((255 * src_color[2]) / 63);
		dst_color.a = 255;
	}
}

void ImageExtractor::set_palette(
	bstone::SdlSurfacePtr sdl_surface,
	const Palette& palette)
{
	assert(sdl_surface);
	assert(palette.size() == bstone::RgbPalette::get_max_color_count());
	assert(sdl_surface->format);
	assert(sdl_surface->format->palette);
	assert(sdl_surface->format->palette->ncolors == bstone::RgbPalette::get_max_color_count());

	std::uninitialized_copy_n(
		palette.cbegin(),
		bstone::RgbPalette::get_max_color_count(),
		sdl_surface->format->palette->colors
	);
}

void ImageExtractor::uninitialize_vga_palette()
{
	vga_palette_.clear();
}

void ImageExtractor::initialize_vga_palette()
{
	vga_palette_.resize(bstone::RgbPalette::get_max_color_count());

	const auto src_colors = vgapal;

	for (int i = 0; i < bstone::RgbPalette::get_max_color_count(); ++i)
	{
		const auto src_color = src_colors + (i * 3);
		auto& dst_color = vga_palette_[i];

		dst_color.r = static_cast<Uint8>((255 * src_color[0]) / 63);
		dst_color.g = static_cast<Uint8>((255 * src_color[1]) / 63);
		dst_color.b = static_cast<Uint8>((255 * src_color[2]) / 63);
		dst_color.a = 255;
	}
}

void ImageExtractor::uninitialize_surface_64x64x8()
{
	sdl_surface_64x64x8_ = nullptr;
}

bool ImageExtractor::initialize_surface_16x16x8()
{
	auto sdl_surface = SDL_CreateRGBSurfaceWithFormat(
		0, // flags
		16, // width
		16, // height
		8, // depth
		SDL_PIXELFORMAT_INDEX8 // format
	);

	if (!sdl_surface)
	{
		auto error_message = std::string{"Failed to create SDL surface 16x16x8bit. "};
		error_message += SDL_GetError();

		bstone::logger_->write_error(error_message);

		return false;
	}

	sdl_surface_16x16x8_ = bstone::SdlSurfaceUPtr{sdl_surface};

	return true;
}

void ImageExtractor::uninitialize_surface_16x16x8()
{
	sdl_surface_16x16x8_ = nullptr;
}

bool ImageExtractor::initialize_surface_64x64x8()
{
	auto sdl_surface = SDL_CreateRGBSurfaceWithFormat(
		0, // flags
		64, // width
		64, // height
		8, // depth
		SDL_PIXELFORMAT_INDEX8 // format
	);

	if (!sdl_surface)
	{
		auto error_message = std::string{"Failed to create SDL surface 64x64x8bit. "};
		error_message += SDL_GetError();

		bstone::logger_->write_error(error_message);

		return false;
	}

	sdl_surface_64x64x8_ = bstone::SdlSurfaceUPtr{sdl_surface};

	return true;
}

void ImageExtractor::uninitialize_surface_64x64x32()
{
	sdl_surface_64x64x32_ = nullptr;
}

bool ImageExtractor::initialize_surface_64x64x32()
{
	auto sdl_surface = SDL_CreateRGBSurfaceWithFormat(
		0, // flags
		64, // width
		64, // height
		32, // depth
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		SDL_PIXELFORMAT_ABGR8888 // format
#else // SDL_BYTEORDER == SDL_LIL_ENDIAN
		SDL_PIXELFORMAT_RGBA8888 // format
#endif // SDL_BYTEORDER == SDL_LIL_ENDIAN
	);

	if (!sdl_surface)
	{
		auto error_message = std::string{"Failed to create SDL surface 64x64x8bit. "};
		error_message += SDL_GetError();

		bstone::logger_->write_error(error_message);

		return false;
	}

	sdl_surface_64x64x32_ = bstone::SdlSurfaceUPtr{sdl_surface};

	return true;
}

void ImageExtractor::convert_wall_page_into_surface(
	const std::uint8_t* const src_indices)
{
	assert(src_indices);

	const auto pitch = sdl_surface_64x64x8_->pitch;

	auto dst_indices = static_cast<std::uint8_t*>(sdl_surface_64x64x8_->pixels);

	auto src_index = 0;

	for (int w = 0; w < 64; ++w)
	{
		for (int h = 0; h < 64; ++h)
		{
			const auto dst_index = (h * pitch) + w;

			dst_indices[dst_index] = src_indices[src_index];

			++src_index;
		}
	}
}

void ImageExtractor::convert_sprite_page_into_surface(
	const bstone::Sprite& sprite)
{
	const auto pitch = sdl_surface_64x64x32_->pitch / 4;

	auto dst_colors = static_cast<SDL_Color*>(sdl_surface_64x64x32_->pixels);

	const auto left = sprite.get_left();
	const auto right = sprite.get_right();
	const auto top = sprite.get_top();
	const auto bottom = sprite.get_bottom();

	for (int w = 0; w < 64; ++w)
	{
		const std::int16_t* column = nullptr;

		if (w >= left && w <= right)
		{
			column = sprite.get_column(w - left);
		}

		for (int h = 0; h < 64; ++h)
		{
			auto dst_color = SDL_Color{};

			if (column && h >= top && h <= bottom)
			{
				const auto color_index = column[h - top];

				if (color_index >= 0)
				{
					dst_color = vga_palette_[color_index];
				}
			}

			const auto dst_index = (h * pitch) + w;

			dst_colors[dst_index] = dst_color;
		}
	}
}

bool ImageExtractor::save_image(
	const std::string& name_prefix,
	const int image_index,
	bstone::SdlSurfacePtr sdl_surface)
{
	const auto image_index_digits = 8;

	const auto& wall_index_string = ca_make_padded_asset_number_string(image_index);

	const auto& file_name = bstone::file_system::append_path(
		destination_dir_,
		name_prefix + wall_index_string + ".bmp"
	);

	const auto sdl_result = SDL_SaveBMP(sdl_surface, file_name.c_str());

	if (sdl_result != 0)
	{
		auto error_message = "Failed to save an image into \"" + file_name + "\". ";
		error_message += SDL_GetError();

		bstone::logger_->write_error(error_message);

		return false;
	}

	return true;
}

bool ImageExtractor::extract_wall(
	const int wall_index)
{
	if (wall_index < 0 && wall_index >= PMSpriteStart)
	{
		bstone::logger_->write_error("Wall index out of range.");

		return false;
	}

	const auto wall_page = static_cast<const std::uint8_t*>(PM_GetPage(wall_index));

	if (!wall_page)
	{
		bstone::logger_->write_error("No wall page #" + std::to_string(wall_index) + ".");

		return false;
	}

	convert_wall_page_into_surface(wall_page);

	if (!save_image("wall_", wall_index, sdl_surface_64x64x8_.get()))
	{
		return false;
	}

	return true;
}

bool ImageExtractor::extract_sprite(
	const int sprite_index)
{
	const auto cache_sprite_index = sprite_index + 1;

	auto sprite = bstone::SpriteCPtr{};

	try
	{
		sprite = sprite_cache_.cache(cache_sprite_index);
	}
	catch (const std::runtime_error& ex)
	{
		auto error_message = "Failed to cache a sprite #" + std::to_string(sprite_index) + ". ";
		error_message += ex.what();

		bstone::logger_->write_error(error_message);

		return false;
	}

	convert_sprite_page_into_surface(*sprite);

	if (!save_image("sprite_", cache_sprite_index, sdl_surface_64x64x32_.get()))
	{
		return false;
	}

	return true;
}

//
// ImageExtractor
// ==========================================================================


void ca_extract_vga_palette(
	const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};

	if (!images_extractor.initialize())
	{
		return;
	}

	images_extractor.extract_vga_palette(destination_dir);
}

void ca_extract_walls(
	const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};

	if (!images_extractor.initialize())
	{
		return;
	}

	images_extractor.extract_walls(destination_dir);
}

void ca_extract_sprites(
	const std::string& destination_dir)
{
	auto images_extractor = ImageExtractor{};

	if (!images_extractor.initialize())
	{
		return;
	}

	images_extractor.extract_sprites(destination_dir);
}

// ==========================================================================
// AudioExtractor
//

class AudioExtractorException :
	public bstone::Exception
{
public:
	explicit AudioExtractorException(
		const char* const message)
		:
		bstone::Exception{std::string{"[DBG_MUS_DMPR] "} + message}
	{
	}
}; // AudioExtractorException

class AudioExtractorTrackException :
	public bstone::Exception
{
public:
	explicit AudioExtractorTrackException(
		const int track_number,
		const char* const message)
		:
		bstone::Exception{std::string{"[DBG_MUS_DMPR][TRK #"} + std::to_string(track_number) + "] " + message}
	{
	}

	explicit AudioExtractorTrackException(
		const int track_number,
		const std::string& message)
		:
		bstone::Exception{std::string{"[DBG_MUS_DMPR][TRK #"} + std::to_string(track_number) + "] " + message}
	{
	}
}; // AudioExtractorTrackException


class AudioExtractor
{
public:
	AudioExtractor();

	void extract_music(
		const std::string& destination_dir);

	void extract_sfx(
		const std::string& destination_dir);


private:
	static constexpr auto wav_prefix_size = 44;


	using Sample = std::int16_t;
	using MusicNumbers = std::vector<int>;
	using DecodeBuffer = std::vector<Sample>;

	AssetsInfo assets_info_;

	MusicNumbers music_numbers_;
	DecodeBuffer decode_buffer_;


	bool write_wav_header(
		const int data_size,
		const int bit_depth,
		const int sample_rate,
		bstone::Stream& stream);

	void initialize_music();

	void extract_music(
		const std::string& destination_dir,
		const int number);

	void initialize_sfx();

	void write_adlib_sfx(
		const int number,
		const SfxInfo& sfx_info,
		bstone::Stream& stream);

	void write_pcm_sfx(
		const int number,
		const SfxInfo& sfx_info,
		bstone::Stream& stream);

	void extract_sfx(
		const std::string& destination_dir,
		const int number);
}; // AudioExtractor


AudioExtractor::AudioExtractor()
	:
	assets_info_{},
	music_numbers_{},
	decode_buffer_{}
{
	decode_buffer_.resize(bstone::opl3_fixed_frequency);
}

void AudioExtractor::extract_music(
	const std::string& destination_dir)
{
	initialize_music();

	bstone::logger_->write("File count: " + std::to_string(music_numbers_.size()));

	for (const auto music_number : music_numbers_)
	{
		extract_music(destination_dir, music_number);
	}
}

void AudioExtractor::extract_sfx(
	const std::string& destination_dir)
{
	initialize_sfx();

	bstone::logger_->write("File count: " + std::to_string(NUMSOUNDS));

	for (int i = 0; i < NUMSOUNDS; ++i)
	{
		extract_sfx(destination_dir, i);
	}
}

bool AudioExtractor::write_wav_header(
	const int data_size,
	const int bit_depth,
	const int sample_rate,
	bstone::Stream& stream)
{
	const auto aligned_data_size = ((data_size + 1) / 2) * 2;
	const auto wav_size = aligned_data_size + wav_prefix_size;

	const auto audio_format = 1; // PCM
	const auto channel_count = 1;
	const auto byte_depth = bit_depth / 8;
	const auto byte_rate = sample_rate * channel_count * byte_depth;
	const auto block_align = channel_count * byte_depth;

	auto writer = bstone::BinaryWriter{&stream};

	auto result = true;

	result &= writer.write_u32(bstone::Endian::big(0x52494646)); // "RIFF"

	// riff_chunk_size = = "file size" - "chunk id" + "chunk size".
	const auto riff_chunk_size = static_cast<std::uint32_t>(wav_size - 4 - 4);
	result &= writer.write_u32(bstone::Endian::little(riff_chunk_size)); // Chunk size.

	result &= writer.write_u32(bstone::Endian::big(0x57415645)); // "WAVE"
	result &= writer.write_u32(bstone::Endian::big(0x666D7420)); // "fmt "
	result &= writer.write_u32(bstone::Endian::little(16)); // Format size.
	result &= writer.write_u16(bstone::Endian::little(audio_format)); // Audio format.
	result &= writer.write_u16(bstone::Endian::little(channel_count)); // Channel count.
	result &= writer.write_u32(bstone::Endian::little(sample_rate)); // Sample rate.
	result &= writer.write_u32(bstone::Endian::little(byte_rate)); // Byte rate.
	result &= writer.write_u16(bstone::Endian::little(block_align)); // Block align.
	result &= writer.write_u16(bstone::Endian::little(bit_depth)); // Bits per sample.
	result &= writer.write_u32(bstone::Endian::big(0x64617461)); // "data"
	result &= writer.write_u32(bstone::Endian::little(data_size)); // Data size.

	return result;
}

void AudioExtractor::initialize_music()
{
	if (LASTMUSIC <= 0 || STARTMUSIC <= 0)
	{
		throw AudioExtractorException{"Assets information not initialized."};
	}

	music_numbers_.reserve(LASTMUSIC + 1);

	music_numbers_.emplace_back(APOGFNFM_MUS);
	music_numbers_.emplace_back(THEME_MUS);

	if (assets_info_.is_aog())
	{
		music_numbers_.emplace_back(S2100A_MUS);
		music_numbers_.emplace_back(GOLDA_MUS);
		//music_numbers_.emplace_back(APOGFNFM_MUS);
		music_numbers_.emplace_back(DRKHALLA_MUS);
		music_numbers_.emplace_back(FREEDOMA_MUS);
		music_numbers_.emplace_back(GENEFUNK_MUS);
		music_numbers_.emplace_back(TIMEA_MUS);
		music_numbers_.emplace_back(HIDINGA_MUS);
		music_numbers_.emplace_back(INCNRATN_MUS);
		music_numbers_.emplace_back(JUNGLEA_MUS);
		music_numbers_.emplace_back(LEVELA_MUS);
		music_numbers_.emplace_back(MEETINGA_MUS);
		music_numbers_.emplace_back(STRUTA_MUS);
		music_numbers_.emplace_back(RACSHUFL_MUS);
		music_numbers_.emplace_back(RUMBAA_MUS);
		music_numbers_.emplace_back(SEARCHNA_MUS);
		music_numbers_.emplace_back(THEWAYA_MUS);
		music_numbers_.emplace_back(INTRIGEA_MUS);
	}

	if (assets_info_.is_ps())
	{
		music_numbers_.emplace_back(CATACOMB_MUS);
		music_numbers_.emplace_back(STICKS_MUS);
		music_numbers_.emplace_back(PLOT_MUS);
		music_numbers_.emplace_back(CIRCLES_MUS);
		music_numbers_.emplace_back(LASTLAFF_MUS);
		music_numbers_.emplace_back(TOHELL_MUS);
		music_numbers_.emplace_back(FORTRESS_MUS);
		music_numbers_.emplace_back(GIVING_MUS);
		music_numbers_.emplace_back(HARTBEAT_MUS);
		music_numbers_.emplace_back(LURKING_MUS);
		music_numbers_.emplace_back(MAJMIN_MUS);
		music_numbers_.emplace_back(VACCINAP_MUS);
		music_numbers_.emplace_back(DARKNESS_MUS);
		music_numbers_.emplace_back(MONASTRY_MUS);
		music_numbers_.emplace_back(TOMBP_MUS);
		music_numbers_.emplace_back(TIME_MUS);
		music_numbers_.emplace_back(MOURNING_MUS);
		music_numbers_.emplace_back(SERPENT_MUS);
		music_numbers_.emplace_back(HISCORE_MUS);
	}

	std::sort(music_numbers_.begin(), music_numbers_.end());
}

void AudioExtractor::extract_music(
	const std::string& destination_dir,
	const int number)
{
	const auto music_index = STARTMUSIC + number;
	CA_CacheAudioChunk(music_index);

	const auto& number_string = ca_make_padded_asset_number_string(number);

	const auto& file_name = bstone::file_system::append_path(
		destination_dir,
		"music_" + number_string + ".wav"
	);

	auto file_stream = bstone::FileStream{file_name, bstone::StreamOpenMode::write};

	if (!file_stream.is_open())
	{
		throw AudioExtractorTrackException{number, "Failed to open \"" + file_name + "\" for writing."};
	}

	auto audio_decoder = bstone::make_audio_decoder(
		bstone::AudioDecoderType::adlib_music,
		bstone::Opl3Type::dbopl
	);

	if (!audio_decoder)
	{
		throw AudioExtractorTrackException{number, "Failed to create AdLib music decoder."};
	}

	const auto music_data = audiosegs[music_index].data();
	const auto music_data_size = sd_get_adlib_music_data_size(music_data);

	auto param = bstone::AudioDecoderInitParam{};
	param.src_raw_data_ = music_data;
	param.src_raw_size_ = music_data_size;
	param.dst_rate_ = bstone::opl3_fixed_frequency;

	if (!audio_decoder->initialize(param))
	{
		throw AudioExtractorTrackException{number, "Failed to initialize AdLib music decoder."};
	}

	if (!file_stream.set_position(wav_prefix_size))
	{
		throw AudioExtractorTrackException{number, "I/O error on \"" + file_name + "\"."};
	}

	constexpr auto sample_size = static_cast<int>(sizeof(Sample));
	constexpr auto bit_depth = sample_size * 8;

	auto data_size = 0;
	auto sample_count = 0;
	auto abs_max_sample = 0;

	while (true)
	{
		const auto decoded_count = audio_decoder->decode(
			bstone::opl3_fixed_frequency,
			decode_buffer_.data()
		);

		if (decoded_count == 0)
		{
			break;
		}

		for (int i = 0; i < decoded_count; ++i)
		{
			const auto sample = static_cast<int>(decode_buffer_[i]);

			abs_max_sample = std::max(std::abs(sample), abs_max_sample);
		}

		const auto decoded_size = decoded_count * sample_size;

		if (!file_stream.write(decode_buffer_.data(), decoded_size))
		{
			throw AudioExtractorTrackException{number, "I/O error on \"" + file_name + "\"."};
		}

		data_size += decoded_size;
		sample_count += decoded_count;
	}

	if (!file_stream.set_position(0))
	{
		throw AudioExtractorTrackException{number, "I/O error on \"" + file_name + "\"."};
	}

	if (!write_wav_header(data_size, bit_depth, bstone::opl3_fixed_frequency, file_stream))
	{
		throw AudioExtractorTrackException{number, "I/O error on \"" + file_name + "\"."};
	}

	const auto volume_factor = 32'768.0 / abs_max_sample;
	const auto volume_factor_string = (data_size > 0 ? std::to_string(volume_factor) : "-");

	auto sha1 = bstone::Sha1{};
	sha1.process(music_data, music_data_size);
	sha1.finish();
	const auto sha1_string = (data_size > 0 ? sha1.to_string() : "-");

	bstone::logger_->write(
		"Track " + std::to_string(number) + ". " +
			"Sample rate: " + std::to_string(bstone::opl3_fixed_frequency) + ". " +
			"Sample count: " + std::to_string(sample_count) + ". " +
			"SHA1: " + sha1_string + ". " +
			"Volume factor: " + volume_factor_string + "."
	);
}

void AudioExtractor::initialize_sfx()
{
	for (int i = 0; i < NUMSOUNDS; ++i)
	{
		CA_CacheAudioChunk(STARTADLIBSOUNDS + i);
	}

	sd_setup_extracting();

	InitDigiMap();
}

void AudioExtractor::write_adlib_sfx(
	const int number,
	const SfxInfo& sfx_info,
	bstone::Stream& stream)
{
	auto audio_decoder = bstone::make_audio_decoder(
		bstone::AudioDecoderType::adlib_sfx,
		bstone::Opl3Type::dbopl
	);

	if (!audio_decoder)
	{
		throw AudioExtractorTrackException{number, "Failed to create AdLib music decoder."};
	}

	auto param = bstone::AudioDecoderInitParam{};
	param.src_raw_data_ = sfx_info.data_;
	param.src_raw_size_ = sfx_info.size_;
	param.dst_rate_ = bstone::opl3_fixed_frequency;

	if (!audio_decoder->initialize(param))
	{
		throw AudioExtractorTrackException{number, "Failed to initialize AdLib music decoder."};
	}

	if (!stream.set_position(wav_prefix_size))
	{
		throw AudioExtractorTrackException{number, "Seek I/O error."};
	}

	constexpr auto sample_size = static_cast<int>(sizeof(Sample));
	constexpr auto bit_depth = sample_size * 8;

	auto data_size = 0;
	auto sample_count = 0;
	auto abs_max_sample = 0;

	while (true)
	{
		const auto decoded_count = audio_decoder->decode(
			bstone::opl3_fixed_frequency,
			decode_buffer_.data()
		);

		if (decoded_count == 0)
		{
			break;
		}

		for (int i = 0; i < decoded_count; ++i)
		{
			const auto sample = static_cast<int>(decode_buffer_[i]);

			abs_max_sample = std::max(std::abs(sample), abs_max_sample);
		}

		const auto decoded_size = decoded_count * sample_size;

		if (!stream.write(decode_buffer_.data(), decoded_size))
		{
			throw AudioExtractorTrackException{number, "Write I/O error."};
		}

		data_size += decoded_size;
		sample_count += decoded_count;
	}

	if (!stream.set_position(0))
	{
		throw AudioExtractorTrackException{number, "Seek I/O error."};
	}

	if (!write_wav_header(data_size, bit_depth, bstone::opl3_fixed_frequency, stream))
	{
		throw AudioExtractorTrackException{number, "Write I/O error."};
	}

	const auto volume_factor = 32'768.0 / abs_max_sample;
	const auto volume_factor_string = (data_size > 0 ? std::to_string(volume_factor) : "-");

	auto sha1 = bstone::Sha1{};
	sha1.process(sfx_info.data_, sfx_info.size_);
	sha1.finish();
	const auto sha1_string = (data_size > 0 ? sha1.to_string() : "-");

	bstone::logger_->write(
		"Track " + std::to_string(number) + ". " +
			"Sample rate: " + std::to_string(bstone::opl3_fixed_frequency) + ". " +
			"Sample count: " + std::to_string(sample_count) + ". " +
			"SHA1: " + sha1_string + ". " +
			"Volume factor: " + volume_factor_string + "."
	);
}

void AudioExtractor::write_pcm_sfx(
	const int number,
	const SfxInfo& sfx_info,
	bstone::Stream& stream)
{
	constexpr auto sample_size = 1;
	constexpr auto bit_depth = sample_size * 8;
	const auto data_size = (sfx_info.data_ ? sfx_info.size_ : 0);

	if (!write_wav_header(data_size, bit_depth, bstone::audio_decoder_pcm_fixed_frequency, stream))
	{
		throw AudioExtractorTrackException{number, "Write I/O error."};
	}

	if (!stream.write(sfx_info.data_, data_size))
	{
		throw AudioExtractorTrackException{number, "Write I/O error."};
	}

	if ((data_size % 2) != 0)
	{
		if (!stream.write_octet(0))
		{
			throw AudioExtractorTrackException{number, "Write I/O error."};
		}
	}

	auto abs_max_sample = 0;

	if (data_size > 0)
	{
		const auto pcm_u8_data = static_cast<const std::uint8_t*>(sfx_info.data_);

		for (int i = 0; i < data_size; ++i)
		{
			abs_max_sample = std::max(std::abs(pcm_u8_data[i] - 128), abs_max_sample);
		}
	}

	const auto volume_factor = 128.0 / abs_max_sample;
	const auto volume_factor_string = (data_size > 0 ? std::to_string(volume_factor) : "-");

	auto sha1 = bstone::Sha1{};
	sha1.process(sfx_info.data_, data_size);
	sha1.finish();
	const auto sha1_string = (data_size > 0 ? sha1.to_string() : "-");

	bstone::logger_->write(
		"Track " + std::to_string(number) + ". " +
			"Sample rate: " + std::to_string(bstone::audio_decoder_pcm_fixed_frequency) + ". " +
			"Sample count: " + std::to_string(data_size) + ". " +
			"SHA1: " + sha1_string + ". " +
			"Volume factor: " + volume_factor_string + "."
	);
}

void AudioExtractor::extract_sfx(
	const std::string& destination_dir,
	const int number)
{
	const auto& sfx_info = sd_get_sfx_info(number);

	const auto& number_string = ca_make_padded_asset_number_string(number);

	const auto& file_name = bstone::file_system::append_path(
		destination_dir,
		"sfx_" + number_string + ".wav"
	);

	auto file_stream = bstone::FileStream{file_name, bstone::StreamOpenMode::write};

	if (!file_stream.is_open())
	{
		throw AudioExtractorTrackException{number, "Failed to open \"" + file_name + "\" for writing."};
	}

	if (sfx_info.is_digitized_)
	{
		write_pcm_sfx(number, sfx_info, file_stream);
	}
	else
	{
		write_adlib_sfx(number, sfx_info, file_stream);
	}
}

//
// AudioExtractor
// ==========================================================================

void ca_extract_music(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting music.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	auto audio_extractor = AudioExtractor{};
	audio_extractor.extract_music(bstone::file_system::normalize_path(destination_dir));

	bstone::logger_->write(">>> ================");
}

void ca_extract_sfx(
	const std::string& destination_dir)
{
	bstone::logger_->write();
	bstone::logger_->write("<<< ================");
	bstone::logger_->write("Extracting sfx.");
	bstone::logger_->write("Destination dir: \"" + destination_dir + "\"");

	auto audio_extractor = AudioExtractor{};
	audio_extractor.extract_sfx(bstone::file_system::normalize_path(destination_dir));

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
		const std::string& message)
		:
		Exception{std::string{"[DBG_TXT_DMPR] "} + message}
	{
	}

	explicit TextExtractorException(
		const int number,
		const std::string& message)
		:
		Exception{std::string{"[DBG_TXT_DMPR][Text #"} + std::to_string(number) + "] " + message}
	{
	}
}; // TextExtractorException


class TextExtractor
{
public:
	TextExtractor();


	void extract_text(
		const std::string& dst_dir);


private:
	enum class CompressionType :
		std::uint16_t
	{
		none = 0,
		lzw = 1,
		lzh = 2,
	}; // CompressionType

	struct TextNumber
	{
		bool is_compressed_;
		int number_;
	}; // TextNumber

#pragma pack(push, 1)
	struct CompressedHeader
	{
		char name_id_[4];
		std::uint32_t uncompressed_size_;
		CompressionType compression_type_;
		std::uint32_t compressed_size_;
	}; // CompressedHeader
#pragma pack(pop)

	using TextNumbers = std::vector<TextNumber>;
	using Buffer = std::vector<std::uint8_t>;


	TextNumbers text_numbers_;
	Buffer buffer_;


	void initialize_text();

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
	text_numbers_.emplace_back(TextNumber{true, POWERBALLTEXT});
	text_numbers_.emplace_back(TextNumber{true, TICSTEXT});
	text_numbers_.emplace_back(TextNumber{true, MUSICTEXT});
	text_numbers_.emplace_back(TextNumber{true, RADARTEXT});
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
		throw TextExtractorException{"Empty list."};
	}

	text_numbers_.erase(text_numbers_.begin(), non_zero_number_it);
}

void TextExtractor::extract_text(
	const std::string& dst_dir,
	const TextNumber& text_number)
{
	const auto number = text_number.number_;

	CA_CacheGrChunk(number);

	auto text_data = grsegs[number].data();
	auto text_size = grsegs_sizes_[number];

	if (text_number.is_compressed_)
	{
		constexpr auto max_uncompressed_size = 256;

		const auto header_size = sizeof(CompressedHeader);
		const auto compressed_header = reinterpret_cast<CompressedHeader*>(text_data);
		const auto pure_data_size = text_size - header_size;

		if (text_size <= header_size ||
			compressed_header->compressed_size_ > pure_data_size ||
			compressed_header->uncompressed_size_ > max_uncompressed_size)
		{
			throw TextExtractorException{number, "Damaged compression header."};
		}

		if (compressed_header->compression_type_ != CompressionType::lzh)
		{
			throw TextExtractorException{number, "Expected LZH compression type."};
		}

		buffer_.resize(compressed_header->uncompressed_size_);

		if (!LZH_Startup())
		{
			throw TextExtractorException{number, "Failed to initialized LZH decoder."};
		}

		const auto decoded_size = LZH_Decompress(
			reinterpret_cast<const char*>(text_data) + header_size,
			buffer_.data(),
			compressed_header->uncompressed_size_,
			compressed_header->compressed_size_
		);

		LZH_Shutdown();

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
		throw TextExtractorException{number, "Failed to open \"" + file_name + "\" for writing."};
	}

	if (!file_stream.write(text_data, text_size))
	{
		throw TextExtractorException{number, "Write I/O error."};
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
