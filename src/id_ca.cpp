/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include <cassert>
#include <algorithm>
#include <memory>
#include "SDL_video.h"
#include "id_ca.h"
#include "audio.h"
#include "id_heads.h"
#include "id_pm.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "gfxv.h"
#include "bstone_endian.h"
#include "bstone_log.h"
#include "bstone_sha1.h"
#include "bstone_string_helper.h"


using namespace std::string_literals;


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

std::uint16_t rlew_tag;

std::int16_t mapon;

std::uint16_t* mapsegs[MAPPLANES];
MapHeaderSegments mapheaderseg;
AudioSegments audiosegs;
GrSegments grsegs;

GrNeeded grneeded;
std::uint8_t ca_levelbit, ca_levelnum;

std::int16_t profilehandle, debughandle;

int NUM_EPISODES = 0;
int MAPS_PER_EPISODE = 0;
int MAPS_WITH_STATS = 0;

int NUMMAPS = 0;


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

std::int32_t* grstarts; // array of offsets in egagraph, -1 for sparse
std::int32_t* audiostarts; // array of offsets in audio / audiot

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
int ca_gr_last_expanded_size;

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

	value = *(std::int32_t*)(((std::uint8_t*)grstarts) + offset);

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
	::grhandle.close();
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
	::ca_open_resource(Assets::get_map_data_base_name(), ::maphandle);
#endif
}

void CloseMapFile()
{
	::maphandle.close();
}

void OpenAudioFile()
{
#ifndef AUDIOHEADERLINKED
	::ca_open_resource(Assets::get_audio_data_base_name(), ::audiohandle);
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
	::audiohandle.close();
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

		while (y < ::vga_ref_height)
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

				if (x >= ::vga_ref_width)
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

	::ca_buffer.reserve(BUFFERSIZE);
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

	if (audiosegs[chunk])
	{
		return; // allready in memory
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

	audiosegs[chunk] = new std::uint8_t[compressed];
	audiohandle.read(audiosegs[chunk], compressed);

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
	std::int16_t start = 0;

	if (::old_is_sound_enabled)
	{
		start = STARTADLIBSOUNDS;
	}

	if (::sd_is_sound_enabled)
	{
		start = STARTADLIBSOUNDS;
	}
	else
	{
		return;
	}

	for (auto i = 0; i < NUMSOUNDS; ++i, ++start)
	{
		::CA_CacheAudioChunk(start);
	}

	::old_is_sound_enabled = ::sd_is_sound_enabled;
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
	grsegs[chunk] = new char[expanded];

	CAL_HuffExpand(source, static_cast<std::uint8_t*>(grsegs[chunk]), expanded, grhuffman);

	ca_gr_last_expanded_size = expanded;
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
	if (grsegs[chunk])
	{
		return; // allready in memory

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


	::grhandle.set_position(pos);

	::ca_buffer.resize(compressed);
	::grhandle.read(::ca_buffer.data(), compressed);
	source = ::ca_buffer.data();

	::CAL_ExpandGrChunk(chunk, source);
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

	::ca_buffer.resize(compressed);
	grhandle.read(::ca_buffer.data(), compressed);
	source = ::ca_buffer.data();

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
	std::int32_t pos;
	std::int32_t compressed;
	std::int16_t plane;
	std::uint16_t** dest;
	std::uint16_t size;
	std::uint16_t* source;
#ifdef CARMACIZED
	memptr buffer2seg;
	std::int32_t expanded;
#endif

	mapon = mapnum;

	//
	// load the planes into the allready allocated buffers
	//
	size = MAPSIZE * MAPSIZE * MAPPLANES;

	for (plane = 0; plane < MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];

		dest = &mapsegs[plane];

		maphandle.set_position(pos);
		::ca_buffer.resize(compressed);
		source = reinterpret_cast<std::uint16_t*>(::ca_buffer.data());

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
		::CA_RLEWexpand(source + 1, *dest, size, rlew_tag);
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
		::Quit("Up past level 7.");
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
		::Quit("Down past level 0.");
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
			if (grsegs[i])
			{ // its allready in memory, make
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
		if ((grneeded[i] & ca_levelbit) && !grsegs[i])
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
				// data is allready in buffer
				source = ::ca_buffer.data() + (pos - bufferstart);
			}
			else
			{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
				while (next < NUMCHUNKS)
				{
					while (next < NUMCHUNKS &&
						!(grneeded[next] & ca_levelbit && !grsegs[next]))
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
				::ca_buffer.resize(endpos - pos);
				grhandle.read(::ca_buffer.data(), endpos - pos);
				bufferstart = pos;
				bufferend = endpos;
				source = ::ca_buffer.data();
			}

			CAL_ExpandGrChunk(i, source);
		}
	}
}

void CA_CannotOpen(
	const std::string& string)
{
	::Quit("Can't open " + string + "!\n");
}

void UNCACHEGRCHUNK(
	int chunk)
{
	delete[] static_cast<char*>(grsegs[chunk]);
	grsegs[chunk] = nullptr;

	grneeded[chunk] &= ~ca_levelbit;
}

std::string ca_load_script(
	int chunk_id,
	bool strip_xx)
{
	::CA_CacheGrChunk(static_cast<std::int16_t>(chunk_id));

	const char* script = static_cast<const char*>(grsegs[chunk_id]);

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
		::Quit("Invalid script.");
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

	if (assets_info.is_aog_full())
	{
		NUM_EPISODES = 6;
		MAPS_PER_EPISODE = 15;
		MAPS_WITH_STATS = 11;
	}
	else if (assets_info.is_aog_sw())
	{
		NUM_EPISODES = 1;
		MAPS_PER_EPISODE = 15;
		MAPS_WITH_STATS = 11;
	}
	else if (assets_info.is_ps())
	{
		NUM_EPISODES = 1;
		MAPS_PER_EPISODE = 25;
		MAPS_WITH_STATS = 20;
	}

	NUMMAPS = NUM_EPISODES * MAPS_PER_EPISODE;

	mapheaderseg.resize(NUMMAPS);
}

bool ca_is_resource_exists(
	const std::string& file_name)
{
	const auto path = ::data_dir + file_name;

	auto is_open = false;

	is_open = bstone::FileStream::is_exists(path);

	if (!is_open)
	{
		auto&& file_name_lc = bstone::StringHelper::to_lower_ascii(file_name);
		const auto path_lc = ::data_dir + file_name_lc;

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
	const auto path = data_dir + file_name;

	auto is_open = false;

	is_open = file_stream.open(path);

	if (!is_open)
	{
		auto&& file_name_lc = bstone::StringHelper::to_lower_ascii(file_name);
		const auto path_lc = data_dir + file_name_lc;

		is_open = file_stream.open(path_lc);
	}

	return is_open;
}

bool ca_open_resource_non_fatal(
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream)
{
	if (!::mod_dir_.empty())
	{
		const auto mod_dir_result = ca_open_resource_non_fatal(
			::mod_dir_, file_name_without_ext, file_extension, file_stream);

		if (mod_dir_result)
		{
			return true;
		}
	}

	const auto data_dir_result = ca_open_resource_non_fatal(
		::data_dir, file_name_without_ext, file_extension, file_stream);

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
		const auto path = ::data_dir + file_name_without_ext + assets_info.get_extension();

		::CA_CannotOpen(path);
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

	if (!::ca_open_resource_non_fatal(base_name, extension, file_stream))
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

	if (!::ca_open_resource_non_fatal(data_dir, base_name, extension, file_stream))
	{
		return {};
	}

	return ca_calculate_hash(file_stream);
}

void ca_dump_hashes()
{
	bstone::Log::write();
	bstone::Log::write("Dumping resource hashes...");

	auto data_size = std::int32_t{};

	auto sha1 = bstone::Sha1{};

	for (const auto& base_name : Assets::get_base_names())
	{
		for (const auto& extension : Assets::get_extensions())
		{
			const auto& sha1_string = ::ca_calculate_hash(base_name, extension);

			if (sha1_string.empty())
			{
				continue;
			}

			bstone::Log::write(base_name.get() + extension.get() + ": " + sha1_string);
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


const std::string& Assets::get_audio_header_base_name()
{
	static const auto audio_header_base_name = "AUDIOHED"s;

	return audio_header_base_name;
}

const std::string& Assets::get_audio_data_base_name()
{
	static const auto audio_data_base_name = "AUDIOT"s;

	return audio_data_base_name;
}

const std::string& Assets::get_map_header_base_name()
{
	static const auto map_header_base_name = "MAPHEAD"s;

	return map_header_base_name;
}

const std::string& Assets::get_map_data_base_name()
{
	static const auto map_data_base_name = "MAPTEMP"s;

	return map_data_base_name;
}

const std::string& Assets::get_gfx_dictionary_base_name()
{
	static const auto gfx_dictionary_base_name = "VGADICT"s;

	return gfx_dictionary_base_name;
}

const std::string& Assets::get_gfx_header_base_name()
{
	static const auto gfx_header_base_name = "VGAHEAD"s;

	return gfx_header_base_name;
}

const std::string& Assets::get_gfx_data_base_name()
{
	static const auto gfx_data_base_name = "VGAGRAPH"s;

	return gfx_data_base_name;
}

const std::string& Assets::get_page_file_base_name()
{
	static const auto page_file_base_name = "VSWAP"s;

	return page_file_base_name;
}

const std::string& Assets::get_episode_6_fmv_base_name()
{
	static const auto episode_6_fmv_base_name = "EANIM"s;

	return episode_6_fmv_base_name;
}

const std::string& Assets::get_episode_3_5_fmv_base_name()
{
	static const auto episode_3_5_fmv_base_name = "GANIM"s;

	return episode_3_5_fmv_base_name;
}

const std::string& Assets::get_intro_fmv_base_name()
{
	static const auto get_intro_fmv_base_name = "IANIM"s;

	return get_intro_fmv_base_name;
}

const std::string& Assets::get_episode_2_4_fmv_base_name()
{
	static const auto get_episode_2_4_fmv_base_name = "SANIM"s;

	return get_episode_2_4_fmv_base_name;
}

const std::string& Assets::get_aog_sw_extension()
{
	static const auto get_aog_sw_extension = ".BS1"s;

	return get_aog_sw_extension;
}

const std::string& Assets::get_aog_full_extension()
{
	static const auto get_aog_full_extension = ".BS6"s;

	return get_aog_full_extension;
}

const std::string& Assets::get_ps_extension()
{
	static const auto get_ps_extension = ".VSI"s;

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
// ImagesDumper
//

class ImagesDumper
{
public:
	bool is_initialized() const;

	bool initialize();

	void uninitialize();

	void dump_walls(
		const std::string& destination_dir);


private:
	using SdlSurfacePtr = SDL_Surface*;

	struct SdlSurfaceDeleter
	{
		void operator()(
			SdlSurfacePtr ptr)
		{
			if (!ptr)
			{
				return;
			}

			::SDL_FreeSurface(ptr);
		}
	}; // SdlSurfaceDeleter

	using SdlSurfaceUPtr = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;


	bool is_initialized_;
	SdlSurfaceUPtr sdl_surface_64x64x8_;
	std::string destination_dir_;
	std::string destination_path_;


	void set_palette(
		SdlSurfacePtr sdl_surface,
		const std::uint8_t* const vga_palette);

	void normalize_destination_dir();

	void uninitialize_surface_64x64x8();

	bool initialize_surface_64x64x8();

	void convert_wall_page_into_surface(
		const std::uint8_t* const src_indices);

	bool save_wall(
		const int wall_index);

	bool dump_wall(
		const int wall_index);
}; // ImagesDumper


bool ImagesDumper::is_initialized() const
{
	return is_initialized_;
}

bool ImagesDumper::initialize()
{
	if (!initialize_surface_64x64x8())
	{
		return false;
	}

	is_initialized_ = true;

	return true;
}

void ImagesDumper::uninitialize()
{
	is_initialized_ = false;

	uninitialize_surface_64x64x8();
}

void ImagesDumper::dump_walls(
	const std::string& destination_dir)
{
	bstone::Log::write();
	bstone::Log::write("<<< ================");
	bstone::Log::write("Dumping walls.");
	bstone::Log::write("Destination dir: \"" + destination_dir + "\"");
	bstone::Log::write("File count: " + std::to_string(::PMSpriteStart));

	if (!is_initialized_)
	{
		bstone::Log::write_error("Not initialized.");

		return;
	}

	destination_dir_ = destination_dir;
	normalize_destination_dir();

	set_palette(sdl_surface_64x64x8_.get(), ::vgapal);

	for (int i = 0; i < ::PMSpriteStart; ++i)
	{
		dump_wall(i);
	}

	bstone::Log::write(">>> ================");
}

void ImagesDumper::set_palette(
	SdlSurfacePtr sdl_surface,
	const std::uint8_t* const vga_palette)
{
	assert(sdl_surface);
	assert(vga_palette);
	assert(sdl_surface->format);
	assert(sdl_surface->format->palette);
	assert(sdl_surface->format->palette->ncolors == 256);

	auto& sdl_palette = *sdl_surface->format->palette;

	for (int i = 0; i < 256; ++i)
	{
		const auto src_color = vga_palette + (i * 3);
		auto& dst_color = sdl_palette.colors[i];

		dst_color.r = static_cast<Uint8>((255 * src_color[0]) / 63);
		dst_color.g = static_cast<Uint8>((255 * src_color[1]) / 63);
		dst_color.b = static_cast<Uint8>((255 * src_color[2]) / 63);
		dst_color.a = 255;
	}
}

void ImagesDumper::normalize_destination_dir()
{
	if (!destination_dir_.empty())
	{
		constexpr auto native_separator =
#ifdef _WIN32
			'\\'
#else // _WIN32
			'//'
#endif // _WIN32
			;

		const auto last_char = destination_dir_.back();

		if (last_char != '\\' && last_char != '/')
		{
			destination_dir_ += native_separator;
		}
	}
}

void ImagesDumper::uninitialize_surface_64x64x8()
{
	sdl_surface_64x64x8_ = nullptr;
}

bool ImagesDumper::initialize_surface_64x64x8()
{
	auto sdl_surface = ::SDL_CreateRGBSurfaceWithFormat(
		0, // flags
		64, // width
		64, // depth
		8, // depth
		SDL_PIXELFORMAT_INDEX8 // format
	);

	if (!sdl_surface)
	{
		auto error_message = "Failed to create SDL surface 64x64x8bit. "s;
		error_message += ::SDL_GetError();

		bstone::Log::write_error(error_message);

		return false;
	}

	sdl_surface_64x64x8_ = SdlSurfaceUPtr{sdl_surface};

	return true;
}

void ImagesDumper::convert_wall_page_into_surface(
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

bool ImagesDumper::save_wall(
	const int wall_index)
{
	const auto wall_index_digits = 8;

	auto& wall_index_string = std::to_string(wall_index);
	wall_index_string.reserve(wall_index_digits);

	const auto pad_count = wall_index_digits - static_cast<int>(wall_index_string.size());

	for (int i = 0; i < pad_count; ++i)
	{
		wall_index_string.insert(0, 1, '0');
	}

	const auto& file_name = destination_dir_ + wall_index_string + ".bmp";

	const auto sdl_result = ::SDL_SaveBMP(sdl_surface_64x64x8_.get(), file_name.c_str());

	if (sdl_result != 0)
	{
		auto& error_message = "Failed to save a file \"" + file_name + "\". ";
		error_message += ::SDL_GetError();

		bstone::Log::write_error(error_message);

		return false;
	}

	return true;
}

bool ImagesDumper::dump_wall(
	const int wall_index)
{
	if (wall_index < 0 && wall_index >= ::PMSpriteStart)
	{
		bstone::Log::write_error("Wall index out of range.");

		return false;
	}

	const auto wall_page = static_cast<const std::uint8_t*>(::PM_GetPage(wall_index));

	if (!wall_page)
	{
		bstone::Log::write_error("No wall page.");

		return false;
	}

	convert_wall_page_into_surface(wall_page);

	if (!save_wall(wall_index))
	{
		return false;
	}

	return true;
}

//
// ImagesDumper
// ==========================================================================


void ca_dump_walls_images(
	const std::string& destination_dir)
{
	auto images_dumper = ImagesDumper{};

	if (!images_dumper.initialize())
	{
		return;
	}

	images_dumper.dump_walls(destination_dir);
}
