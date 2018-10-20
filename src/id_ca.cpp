/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "id_heads.h"
#include "bstone_sha1.h"


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

uint16_t rlew_tag;

int16_t mapon;

uint16_t* mapsegs[MAPPLANES];
MapHeaderSegments mapheaderseg;
AudioSegments audiosegs;
GrSegments grsegs;

GrNeeded grneeded;
uint8_t ca_levelbit, ca_levelnum;

int16_t profilehandle, debughandle;

int NUM_EPISODES = 0;
int MAPS_PER_EPISODE = 0;
int MAPS_WITH_STATS = 0;

int NUMMAPS = 0;

bool is_aog_full();
bool is_aog_sw();
bool is_ps();


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

extern int32_t CGAhead;
extern int32_t EGAhead;
extern uint8_t CGAdict;
extern uint8_t EGAdict;
extern uint8_t maphead;
extern uint8_t mapdict;
extern uint8_t audiohead;
extern uint8_t audiodict;


void CA_CannotOpen(
    const std::string& string);

int32_t* grstarts; // array of offsets in egagraph, -1 for sparse
int32_t* audiostarts; // array of offsets in audio / audiot

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

int32_t chunkcomplen;
int32_t chunkexplen;

bool old_is_sound_enabled;

static Buffer ca_buffer;

static const int BUFFERSIZE = 0x10000;


// BBi
int ca_gr_last_expanded_size;
int map_compressed_size = 0;
std::string map_sha1_string;

void CAL_CarmackExpand(
    uint16_t* source,
    uint16_t* dest,
    uint16_t length);


#ifdef THREEBYTEGRSTARTS
int32_t GRFILEPOS(
    int16_t c)
{
    int32_t value;
    int16_t offset;

    offset = c * 3;

    value = *(int32_t*)(((uint8_t*)grstarts) + offset);

    value &= 0x00ffffffl;

    if (value == 0xffffffl) {
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
    ::ca_open_resource(Assets::map_data_base_name, ::maphandle);
#endif
}

void CloseMapFile()
{
    ::maphandle.close();
}

void OpenAudioFile()
{
#ifndef AUDIOHEADERLINKED
    ::ca_open_resource(Assets::audio_data_base_name, ::audiohandle);
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
    int16_t chunk)
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
    uint8_t* source,
    uint8_t* destination,
    int32_t length,
    huffnode* hufftable)
{
    uint8_t val = *source++;
    uint8_t mask = 1;
    uint16_t nodeval;

    huffnode* headptr = &hufftable[254]; // head node is always node 254

    uint8_t* dst = destination;
    uint8_t* end = dst + length;

    huffnode* huffptr = headptr;

    while (dst < end) {
        if ((val & mask) == 0) {
            nodeval = huffptr->bit0;
        } else {
            nodeval = huffptr->bit1;
        }

        if (mask == 0x80) {
            val = *source++;
            mask = 1;
        } else {
            mask <<= 1;
        }

        if (nodeval < 256) {
            dst[0] = static_cast<uint8_t>(nodeval);
            ++dst;
            huffptr = headptr;
        } else {
            huffptr = &hufftable[nodeval - 256];
        }
    }
}

void ca_huff_expand_on_screen(
    uint8_t* source,
    huffnode* hufftable)
{
    uint8_t val = *source++;
    uint8_t mask = 1;
    uint16_t nodeval;

    huffnode* headptr = &hufftable[254]; // head node is always node 254
    huffnode* huffptr = headptr;

    for (int p = 0; p < 4; ++p) {
        int x = p;
        int y = 0;

        while (y < ::vga_ref_height) {
            if ((val & mask) == 0) {
                nodeval = huffptr->bit0;
            } else {
                nodeval = huffptr->bit1;
            }

            if (mask == 0x80) {
                val = *source++;
                mask = 1;
            } else {
                mask <<= 1;
            }

            if (nodeval < 256) {
                VL_Plot(x, y, static_cast<uint8_t>(nodeval));
                huffptr = headptr;

                x += 4;

                if (x >= ::vga_ref_width) {
                    x = p;
                    ++y;
                }
            } else {
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
    uint16_t* source,
    uint16_t* dest,
    uint16_t length)
{
#define NEARTAG 0xa7
#define FARTAG 0xa8

    uint16_t ch, chhigh, count, offset;
    uint16_t* copyptr, * inptr, * outptr;

    length /= 2;

    inptr = source;
    outptr = dest;

    while (length) {
        ch = *inptr++;
        chhigh = ch >> 8;
        if (chhigh == NEARTAG) {
            count = ch & 0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *((uint8_t*)inptr)++;
                *outptr++ = ch;
                length--;
            } else {
                offset = *((uint8_t*)inptr)++;
                copyptr = outptr - offset;
                length -= count;
                while (count--) {
                    *outptr++ = *copyptr++;
                }
            }
        } else if (chhigh == FARTAG) {
            count = ch & 0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *((uint8_t*)inptr)++;
                *outptr++ = ch;
                length--;
            } else {
                offset = *inptr++;
                copyptr = dest + offset;
                length -= count;
                while (count--) {
                    *outptr++ = *copyptr++;
                }
            }
        } else {
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
    uint16_t* source,
    uint16_t* dest,
    int32_t length,
    uint16_t rlewtag)
{
    uint16_t i;
    uint16_t value;
    uint16_t count;
    const uint16_t* end = &dest[length / 2];

    do {
        value = *source++;

        if (value != rlewtag) {
            *dest++ = value;
        } else {
            count = *source++;
            value = *source++;

            for (i = 0; i < count; ++i) {
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
    if (profilehandle != -1) {
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
    int16_t chunk)
{
    int32_t pos;
    int32_t compressed;
#ifdef AUDIOHEADERLINKED
    int32_t expanded;
    memptr bigbufferseg;
    uint8_t* source;
#endif

    if (audiosegs[chunk]) {
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

    audiosegs[chunk] = new uint8_t[compressed];
    audiohandle.read(audiosegs[chunk], compressed);

#else

    if (compressed <= BUFFERSIZE) {
        CA_FarRead(audiohandle, bufferseg, compressed);
        source = bufferseg;
    } else {
        MM_GetPtr(&bigbufferseg, compressed);
        if (mmerror) {
            CloseAudioFile();
            return;
        }
        MM_SetLock(&bigbufferseg, true);
        CA_FarRead(audiohandle, bigbufferseg, compressed);
        source = bigbufferseg;
    }

    expanded = *(int32_t*)source;
    source += 4; // skip over length
    MM_GetPtr(&(memptr)audiosegs[chunk], expanded);
    if (mmerror) {
        goto done;
    }
    CAL_HuffExpand(source, audiosegs[chunk], expanded, audiohuffman, false);

done:
    if (compressed > BUFFERSIZE) {
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
    int16_t start = 0;

    if (::old_is_sound_enabled) {
        start = STARTADLIBSOUNDS;
    }

    if (::sd_is_sound_enabled) {
        start = STARTADLIBSOUNDS;
    } else {
        return;
    }

    for (auto i = 0; i < NUMSOUNDS; ++i, ++start) {
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
    int16_t chunk,
    uint8_t* source)
{
    int32_t expanded;

    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS) {
        //
        // expanded sizes of tile8/16/32 are implicit
        //

        const int BLOCK = 64;
        const int MASKBLOCK = 128;

        if (chunk < STARTTILE8M) { // tile 8s are all in one chunk!
            expanded = BLOCK * NUMTILE8;
        } else if (chunk < STARTTILE16) {
            expanded = MASKBLOCK * NUMTILE8M;
        } else if (chunk < STARTTILE16M) {      // all other tiles are one/chunk
            expanded = BLOCK * 4;
        } else if (chunk < STARTTILE32) {
            expanded = MASKBLOCK * 4;
        } else if (chunk < STARTTILE32M) {
            expanded = BLOCK * 16;
        } else {
            expanded = MASKBLOCK * 16;
        }
    } else {
        //
        // everything else has an explicit size longword
        //
        expanded = bstone::Endian::le(*reinterpret_cast<int32_t*>(source));
        source += 4; // skip over length
    }

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
    grsegs[chunk] = new char[expanded];

    CAL_HuffExpand(source, static_cast<uint8_t*>(grsegs[chunk]), expanded, grhuffman);

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
    int16_t chunk)
{
    int32_t pos, compressed;
    uint8_t* source;
    int16_t next;

    grneeded[chunk] |= ca_levelbit; // make sure it doesn't get removed
    if (grsegs[chunk]) {
        return; // allready in memory

    }
//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
    pos = GRFILEPOS(chunk);
    if (pos < 0) { // $FFFFFFFF start is a sparse tile
        return;
    }

    next = chunk + 1;
    while (GRFILEPOS(next) == -1) { // skip past any sparse tiles
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
    int16_t chunk)
{
    int32_t pos, compressed;
    uint8_t* source;
    int16_t next;


//
// load the chunk into a buffer
//
    pos = GRFILEPOS(chunk);
    next = chunk + 1;
    while (GRFILEPOS(next) == -1) { // skip past any sparse tiles
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
    int16_t mapnum)
{
    int32_t pos, compressed;
    int16_t plane;
    uint16_t** dest;
    uint16_t size;
    uint16_t* source;
#ifdef CARMACIZED
    memptr buffer2seg;
    int32_t expanded;
#endif

    mapon = mapnum;

    OpenMapFile();

    // BBi
    bstone::Sha1 map_sha1;
    ::map_compressed_size = 0;

//
// load the planes into the allready allocated buffers
//
    size = MAPSIZE * MAPSIZE * MAPPLANES;

    for (plane = 0; plane < MAPPLANES; plane++) {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];

        dest = &mapsegs[plane];

        maphandle.set_position(pos);
        ::ca_buffer.resize(compressed);
        source = reinterpret_cast<uint16_t*>(::ca_buffer.data());

        maphandle.read(source, compressed);

        // BBi
        ::map_compressed_size += compressed;
        map_sha1.process(source, compressed);

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
        CAL_CarmackExpand(source, (uint16_t*)buffer2seg, expanded);
        CA_RLEWexpand(((uint16_t*)buffer2seg) + 1, *dest, size,
                      ((mapfiletype*)tinf)->RLEWtag);
        MM_FreePtr(&buffer2seg);

#else
        //
        // unRLEW, skipping expanded length
        //
        CA_RLEWexpand(source + 1, *dest, size,
                      rlew_tag);
#endif
    }

    CloseMapFile();

    // BBi
    map_sha1.finish();
    ::map_sha1_string = map_sha1.to_string();
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
    if (ca_levelnum == 7) {
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
    if (!ca_levelnum) {
        ::Quit("Down past level 0.");
    }

    ca_levelbit >>= 1;
    ca_levelnum--;
    CA_CacheMarks();
}

void CA_CacheMarks()
{
    const int MAXEMPTYREAD = 1024;

    int16_t i;
    int16_t next;
    int16_t numcache;
    int32_t pos;
    int32_t endpos;
    int32_t nextpos;
    int32_t nextendpos;
    int32_t compressed;
    int32_t bufferstart;
    int32_t bufferend; // file position of general buffer
    uint8_t* source;

    numcache = 0;
//
// go through and make everything not needed purgable
//
    for (i = 0; i < NUMCHUNKS; i++) {
        if (grneeded[i] & ca_levelbit) {
            if (grsegs[i]) { // its allready in memory, make
            } else {
                numcache++;
            }
        }
    }

    if (!numcache) { // nothing to cache!
        return;
    }


//
// go through and load in anything still needed
//
    bufferstart = bufferend = 0; // nothing good in buffer now

    for (i = 0; i < NUMCHUNKS; i++) {
        if ((grneeded[i] & ca_levelbit) && !grsegs[i]) {
            pos = GRFILEPOS(i);
            if (pos < 0) {
                continue;
            }

            next = i + 1;
            while (GRFILEPOS(next) == -1) { // skip past any sparse tiles
                next++;
            }

            compressed = GRFILEPOS(next) - pos;
            endpos = pos + compressed;

            if (bufferstart <= pos && bufferend >= endpos) {
                // data is allready in buffer
                source = ::ca_buffer.data() + (pos - bufferstart);
            } else {
                // load buffer with a new block from disk
                // try to get as many of the needed blocks in as possible
                while (next < NUMCHUNKS) {
                    while (next < NUMCHUNKS &&
                            !(grneeded[next] & ca_levelbit && !grsegs[next]))
                    {
                        ++next;
                    }

                    if (next == NUMCHUNKS) {
                        continue;
                    }

                    nextpos = GRFILEPOS(next);

                    while (GRFILEPOS(++next) == -1) {
                        // skip past any sparse tiles
                    }

                    nextendpos = GRFILEPOS(next);

                    if ((nextpos - endpos) <= MAXEMPTYREAD
                        && (nextendpos - pos) <= BUFFERSIZE)
                    {
                        endpos = nextendpos;
                    } else {
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
    delete [] static_cast<char*>(grsegs[chunk]);
    grsegs[chunk] = nullptr;

    grneeded[chunk] &= ~ca_levelbit;
}

std::string ca_load_script(
    int chunk_id,
    bool strip_xx)
{
    ::CA_CacheGrChunk(static_cast<int16_t>(chunk_id));

    const char* script = static_cast<const char*>(grsegs[chunk_id]);

    int length = 0;

    for (int i = 0; script[i] != '\x1A'; ++i) {
        if (script[i] == '^' && script[i + 1] == 'X' && script[i + 2] == 'X') {
            length = i + 3;
        }
    }

    if (length == 0) {
        ::Quit("Invalid script.");
    }

    if (strip_xx) {
        length -= 3;
    }

    return std::string(script, length);
}

void initialize_ca_constants()
{
	const auto& assets_info = AssetsInfo{};

    if (assets_info.is_aog_full()) {
        NUM_EPISODES = 6;
        MAPS_PER_EPISODE = 15;
        MAPS_WITH_STATS = 11;
    } else if (assets_info.is_aog_sw()) {
        NUM_EPISODES = 1;
        MAPS_PER_EPISODE = 15;
        MAPS_WITH_STATS = 11;
    } else if (assets_info.is_ps()) {
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
        const auto file_name_lc = bstone::StringHelper::to_lower(file_name);
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
		const auto file_name_lc = bstone::StringHelper::to_lower(file_name);
		const auto path_lc = ::data_dir + file_name_lc;

		is_open = file_stream.open(path_lc);
	}

	return is_open;
}

bool ca_open_resource_non_fatal(
	const std::string& file_name_without_ext,
	const std::string& file_extension,
	bstone::FileStream& file_stream)
{
	return ca_open_resource_non_fatal(::data_dir, file_name_without_ext, file_extension, file_stream);
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
	const std::string& data_dir,
	const std::string& base_name,
	const std::string& extension)
{
	auto file_stream = bstone::FileStream{};

	if (!::ca_open_resource_non_fatal(data_dir, base_name, extension, file_stream))
	{
		return {};
	}

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

	const auto data_size = bstone::Endian::le(static_cast<std::int32_t>(file_size));

	sha1.process(&data_size, static_cast<int>(sizeof(data_size)));
	sha1.finish();

	if (!sha1.is_valid())
	{
		return {};
	}

	return sha1.to_string();
}

void ca_dump_hashes()
{
	bstone::Log::write();
	bstone::Log::write("Dumping resource hashes...");

	bstone::FileStream file_stream;

	auto buffer = Buffer{};
	buffer.reserve(Assets::max_size);

	auto data_size = std::int32_t{};

	auto sha1 = bstone::Sha1{};

	for (const auto& base_name : Assets::get_base_names())
	{
		for (const auto& extension : Assets::get_extensions())
		{
			const auto open_result = ca_open_resource_non_fatal(base_name, extension, file_stream);

			if (!open_result)
			{
				continue;
			}

			bstone::Log::write();
			bstone::Log::write("{}{}:", base_name, extension);

			const auto stream_size = file_stream.get_size();

			if (stream_size > Assets::max_size)
			{
				bstone::Log::write_error("File size is too big.");
				continue;
			}

			const auto file_size = static_cast<int>(stream_size);

			const auto read_result = file_stream.read(buffer.data(), file_size);

			if (read_result != file_size)
			{
				bstone::Log::write_error("Failed to read data.");
				continue;
			}

			data_size = file_size;
			bstone::Endian::lei(data_size);

			sha1.reset();
			sha1.process(buffer.data(), file_size);
			sha1.process(&data_size, static_cast<int>(sizeof(data_size)));
			sha1.finish();

			const auto& sha1_string = sha1.to_string();

			bstone::Log::write(sha1_string);
		}
	}
}


std::string AssetsInfo::empty_extension_;

AssetsVersion AssetsInfo::version_;
AssetsCRefString AssetsInfo::extension_ = empty_extension_;
AssetsCRefStrings AssetsInfo::base_names_;
AssetsBaseNameToHashMap AssetsInfo::base_name_to_hash_map_;
int AssetsInfo::gfx_header_offset_count_;


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

	case AssetsVersion::aog_sw_v3_0:
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

int AssetsInfo::get_gfx_header_offset_count() const
{
	return gfx_header_offset_count_;
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


const std::string& Assets::audio_header_base_name = "AUDIOHED";
const std::string& Assets::audio_data_base_name = "AUDIOT";

const std::string& Assets::map_header_base_name = "MAPHEAD";
const std::string& Assets::map_data_base_name = "MAPTEMP";

const std::string& Assets::gfx_dictionary_base_name = "VGADICT";
const std::string& Assets::gfx_header_base_name = "VGAHEAD";
const std::string& Assets::gfx_data_base_name = "VGAGRAPH";

const std::string& Assets::page_file_base_name = "VSWAP";

const std::string& Assets::episode_6_fmv_base_name = "EANIM";
const std::string& Assets::episode_3_5_fmv_base_name = "GANIM";
const std::string& Assets::intro_fmv_base_name = "IANIM";
const std::string& Assets::episode_2_4_fmv_base_name = "SANIM";

const std::string& Assets::aog_sw_extension = ".BS1";
const std::string& Assets::aog_full_extension = ".BS6";
const std::string& Assets::ps_extension = ".VSI";


const AssetsCRefStrings& Assets::get_extensions()
{
	static const auto extensions = AssetsCRefStrings
	{
		aog_sw_extension,
		aog_full_extension,
		ps_extension,
	}; // extensions

	return extensions;
}

const AssetsCRefStrings& Assets::get_base_names()
{
	static const auto base_names = AssetsCRefStrings
	{
		audio_header_base_name,
		audio_data_base_name,

		map_header_base_name,
		map_data_base_name,

		gfx_dictionary_base_name,
		gfx_header_base_name,
		gfx_data_base_name,

		page_file_base_name,

		episode_6_fmv_base_name,
		episode_3_5_fmv_base_name,
		intro_fmv_base_name,
		episode_2_4_fmv_base_name,
	}; // base_names

	return base_names;
}

const AssetsCRefStrings& Assets::get_aog_sw_base_names()
{
	static const auto aog_sw_base_names = AssetsCRefStrings
	{
		audio_header_base_name,
		audio_data_base_name,

		map_header_base_name,
		map_data_base_name,

		gfx_dictionary_base_name,
		gfx_header_base_name,
		gfx_data_base_name,

		page_file_base_name,

		intro_fmv_base_name,
		episode_2_4_fmv_base_name,
	}; // aog_sw_base_names

	return aog_sw_base_names;
}

const AssetsCRefStrings& Assets::get_aog_full_base_names()
{
	static const auto aog_full_base_names = AssetsCRefStrings
	{
		audio_header_base_name,
		audio_data_base_name,

		map_header_base_name,
		map_data_base_name,

		gfx_dictionary_base_name,
		gfx_header_base_name,
		gfx_data_base_name,

		page_file_base_name,

		episode_6_fmv_base_name,
		episode_3_5_fmv_base_name,
		intro_fmv_base_name,
		episode_2_4_fmv_base_name,
	}; // aog_full_base_names

	return aog_full_base_names;
}

const AssetsCRefStrings& Assets::get_ps_base_names()
{
	static const auto ps_base_names = AssetsCRefStrings
	{
		audio_header_base_name,
		audio_data_base_name,

		map_header_base_name,
		map_data_base_name,

		gfx_dictionary_base_name,
		gfx_header_base_name,
		gfx_data_base_name,

		page_file_base_name,

		episode_6_fmv_base_name,
		intro_fmv_base_name,
	}; // ps_base_names

	return ps_base_names;
}


const AssetsBaseNameToHashMap& Assets::get_aog_sw_v1_0_base_name_to_hash_map()
{
	static auto aog_sw_v1_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "08f91c4ce58d4ba15a83f06a8bf588a211124b22"},
		{audio_data_base_name, "4d3da5709e903dbedf9564b53c354b00f607a0ff"},

		{map_header_base_name, "ad8807e51704ef0e4b3ced663c0d579592b90a56"},
		{map_data_base_name, "365824414c91947b1e3589da03a6b651da06514a"},

		{gfx_dictionary_base_name, "b70850f93ff827b042dec18c94ce60e8b03b1b3b"},
		{gfx_header_base_name, "403dc7a9819654c9b0aad5fac0ed2bed681685a6"},
		{gfx_data_base_name, "d880466fc01d16b7ab7aeaee35174b8f3cde270a"},

		{page_file_base_name, "d72bb643fbe0fa3289090e81a583732aecd1c788"},

		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_sw_v1_0_base_name_to_hash_map

	return aog_sw_v1_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v2_0_base_name_to_hash_map()
{
	static auto aog_sw_v2_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "08f91c4ce58d4ba15a83f06a8bf588a211124b22"},
		{audio_data_base_name, "4d3da5709e903dbedf9564b53c354b00f607a0ff"},

		{map_header_base_name, "5cc262f2429d57698b6a3a335239bd2cf1f47945"},
		{map_data_base_name, "3e93219bd86584a25f5a58b9318c8f9f626e3795"},

		{gfx_dictionary_base_name, "7ed53d1006fdd50bcce5949fc4544d28153983e8"},
		{gfx_header_base_name, "34df3bbbe37a32b2e5936f481577385b6c218b69"},
		{gfx_data_base_name, "8452d63ac3f66102d70f6afc20786768eed0f22a"},

		{page_file_base_name, "b13126ee04c4a921329e8a420dbbb85de5b5be70"},

		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_sw_v2_0_base_name_to_hash_map

	return aog_sw_v2_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v2_1_base_name_to_hash_map()
{
	static auto aog_sw_v2_1_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "5cc262f2429d57698b6a3a335239bd2cf1f47945"},
		{map_data_base_name, "3e93219bd86584a25f5a58b9318c8f9f626e3795"},

		{gfx_dictionary_base_name, "cfbda96717fc9d36aca347828000bd8739d25500"},
		{gfx_header_base_name, "e40e4311341ec03b48f48ce35eb04d5493be27c9"},
		{gfx_data_base_name, "b98e8f4b1f398429d4a76720e302615fe52caf8f"},

		{page_file_base_name, "b13126ee04c4a921329e8a420dbbb85de5b5be70"},

		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_sw_v2_1_base_name_to_hash_map

	return aog_sw_v2_1_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_sw_v3_0_base_name_to_hash_map()
{
	static auto aog_sw_v3_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "5cc262f2429d57698b6a3a335239bd2cf1f47945"},
		{map_data_base_name, "3e93219bd86584a25f5a58b9318c8f9f626e3795"},

		{gfx_dictionary_base_name, "56c8a8cc4039079261ebc987f80f401df912406d"},
		{gfx_header_base_name, "f589b6ddbfeac281ee2358dfcf9c421ac469fdc9"},
		{gfx_data_base_name, "2f59b7c33c7a1895faf46cf5a7150d12353d8893"},

		{page_file_base_name, "b13126ee04c4a921329e8a420dbbb85de5b5be70"},

		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_sw_v3_0_base_name_to_hash_map

	return aog_sw_v3_0_base_name_to_hash_map;
}


const AssetsBaseNameToHashMap& Assets::get_aog_full_v1_0_base_name_to_hash_map()
{
	static auto aog_full_v1_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "9ac4a06bd7fc25b48852bf4da1ca9decf06e3873"},
		{map_data_base_name, "22a160796397b515348752393d4f7f91f3ce786e"},

		{gfx_dictionary_base_name, "191dc0617f82f9d93ca5464b6af0d4abf614a3ec"},
		{gfx_header_base_name, "2fbd05d76cdd9e13cc14e3a146db5f5388820a6f"},
		{gfx_data_base_name, "76fc99265ddb463e12707eeefff7cf47c74cc18a"},

		{page_file_base_name, "4a6b188381028158ac392965edb753a807111b56"},

		{episode_6_fmv_base_name, "396b0f0b4409056f6b82239a7bd97b65e34b08a5"},
		{episode_3_5_fmv_base_name, "9aac6157c774a739df40b3bbf6043423a1e6d317"},
		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_full_v1_0_base_name_to_hash_map

	return aog_full_v1_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v2_0_base_name_to_hash_map()
{
	static auto aog_full_v2_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "8ee7970b93c7df2d035fbc168efea6081963924a"},
		{map_data_base_name, "e22c5a638b58bc442b127d4b7c81b7fc221059da"},

		{gfx_dictionary_base_name, "125ccb707edeb420aebf502ee8fc49d40171b2c1"},
		{gfx_header_base_name, "1fdb177d8b010670399ed161c071ed09d2e06b03"},
		{gfx_data_base_name, "36fcc86a858efcaf272e60434d4e69994546b1c9"},

		{page_file_base_name, "0ddd940e2cb2e96bdebb46487e3e812e1bbda613"},

		{episode_6_fmv_base_name, "396b0f0b4409056f6b82239a7bd97b65e34b08a5"},
		{episode_3_5_fmv_base_name, "9aac6157c774a739df40b3bbf6043423a1e6d317"},
		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_full_v2_0_base_name_to_hash_map

	return aog_full_v2_0_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v2_1_base_name_to_hash_map()
{
	static auto aog_full_v2_1_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "8ee7970b93c7df2d035fbc168efea6081963924a"},
		{map_data_base_name, "e22c5a638b58bc442b127d4b7c81b7fc221059da"},

		{gfx_dictionary_base_name, "f24edbeec8f9e6988fde4a91b423fd87ca72148e"},
		{gfx_header_base_name, "ef373dd36ff5f03e21a696cd740ef2ce71585f3e"},
		{gfx_data_base_name, "aefd4868e6e59616c149f6ae4fad7bd8fd8c1f90"},

		{page_file_base_name, "0ddd940e2cb2e96bdebb46487e3e812e1bbda613"},

		{episode_6_fmv_base_name, "396b0f0b4409056f6b82239a7bd97b65e34b08a5"},
		{episode_3_5_fmv_base_name, "9aac6157c774a739df40b3bbf6043423a1e6d317"},
		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_full_v2_1_base_name_to_hash_map

	return aog_full_v2_1_base_name_to_hash_map;
}

const AssetsBaseNameToHashMap& Assets::get_aog_full_v3_0_base_name_to_hash_map()
{
	static auto aog_full_v3_0_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "177a680aca41012539a1e3ecfbbee28af9664ebb"},
		{audio_data_base_name, "bff757d712fa23697767591343879271c57684af"},

		{map_header_base_name, "8ee7970b93c7df2d035fbc168efea6081963924a"},
		{map_data_base_name, "e22c5a638b58bc442b127d4b7c81b7fc221059da"},

		{gfx_dictionary_base_name, "482aa07f75bd06c91fbad0239ee6f633800cfabc"},
		{gfx_header_base_name, "818bed8b27d3c703d2e20ad1c91f2bac5a8d1cd9"},
		{gfx_data_base_name, "28dac1c5f4ef19d834737c15ba8f37af5e66edee"},

		{page_file_base_name, "0ddd940e2cb2e96bdebb46487e3e812e1bbda613"},

		{episode_6_fmv_base_name, "396b0f0b4409056f6b82239a7bd97b65e34b08a5"},
		{episode_3_5_fmv_base_name, "9aac6157c774a739df40b3bbf6043423a1e6d317"},
		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
		{episode_2_4_fmv_base_name, "2301d7e4f63858cf15f428dc3da25cfb5077efc5"},
	}; // aog_full_v3_0_base_name_to_hash_map

	return aog_full_v3_0_base_name_to_hash_map;
}


const AssetsBaseNameToHashMap& Assets::get_ps_base_name_to_hash_map()
{
	static auto ps_base_name_to_hash_map = AssetsBaseNameToHashMap
	{
		{audio_header_base_name, "a713f75daf8274375dce0590c0caec6a994022dc"},
		{audio_data_base_name, "6eadc8ac76bb3e20726db6e2584caf50ce36b624"},

		{map_header_base_name, "155c550a1a240631e08e4f8be2686e29bde0549e"},
		{map_data_base_name, "a40c9f6bbad59fe13c55388e265402e55167803a"},

		{gfx_dictionary_base_name, "b74e45d850c92f2066fed2dfd38545cc28680c4e"},
		{gfx_header_base_name, "e02d5d7c1e86812162eb2d86766ad8acf3dfe9be"},
		{gfx_data_base_name, "523973d0df78d439960662a15a867d600720baf1"},

		{page_file_base_name, "ccf70bfb536545e9b2c709ce3ce12e2b1765bc03"},

		{episode_6_fmv_base_name, "055b9c5d5256e4d8d97259da37381882f46c2550"},
		{intro_fmv_base_name, "a29b7557738c8abf9e0c9109f563d02d427be4b2"},
	}; // ps_base_name_to_hash_map

	return ps_base_name_to_hash_map;
}
