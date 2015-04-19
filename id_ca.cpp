/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


// ID_CA.C

// this has been customized for BLAKE STONE

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_heads.h"

#ifdef MSVC
#pragma hdrstop
#endif


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/

/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

Uint16 rlew_tag;

Sint16 mapon;

Uint16* mapsegs[MAPPLANES];
maptype* mapheaderseg[NUMMAPS];
AudioSegments audiosegs;
GrSegments grsegs;

GrNeeded grneeded;
Uint8 ca_levelbit, ca_levelnum;

Sint16 profilehandle, debughandle;

std::string audioname = "AUDIO.";

/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

extern Sint32 CGAhead;
extern Sint32 EGAhead;
extern Uint8 CGAdict;
extern Uint8 EGAdict;
extern Uint8 maphead;
extern Uint8 mapdict;
extern Uint8 audiohead;
extern Uint8 audiodict;


std::string extension; // Need a string, not constant to change cache files
std::string gheadname = GREXT "HEAD.";
std::string gfilename = GREXT "GRAPH.";
std::string gdictname = GREXT "DICT.";
std::string mheadname = "MAPHEAD.";
std::string mfilename = "MAPTEMP.";
std::string aheadname = "AUDIOHED.";
std::string afilename = "AUDIOT.";

void CA_CannotOpen(
    const std::string& string);

Sint32* grstarts; // array of offsets in egagraph, -1 for sparse
Sint32* audiostarts; // array of offsets in audio / audiot

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

Sint32 chunkcomplen, chunkexplen;

SDMode oldsoundmode;

// BBi
int ca_gr_last_expanded_size;


void CAL_CarmackExpand(
    Uint16* source,
    Uint16* dest,
    Uint16 length);


#ifdef THREEBYTEGRSTARTS
// #define      GRFILEPOS(c) (*(long *)(((Uint8 *)grstarts)+(c)*3)&0xffffff)
Sint32 GRFILEPOS(
    Sint16 c)
{
    Sint32 value;
    Sint16 offset;

    offset = c * 3;

    value = *(Sint32*)(((Uint8*)grstarts) + offset);

    value &= 0x00ffffffl;

    if (value == 0xffffffl) {
        value = -1;
    }

    return value;
}
#else
#define GRFILEPOS(c) (grstarts[c])
#endif

/*
=============================================================================

                                           LOW LEVEL ROUTINES

=============================================================================
*/

#if 0

/*
============================
=
= CA_OpenDebug / CA_CloseDebug
=
= Opens a binary file with the handle "debughandle"
=
============================
*/

void CA_OpenDebug()
{
    unlink("DEBUG.TXT");
    debughandle = open("DEBUG.TXT", O_CREAT | O_WRONLY | O_TEXT);
}

void CA_CloseDebug()
{
    close(debughandle);
}

#endif



// -----------------------------------------------------------------------
// OpenGrFile()
// ------------------------------------------------------------------------
void OpenGrFile()
{
    auto fname = ::gfilename + ::extension;

    ::grhandle.open(fname);

    if (!::grhandle.is_open()) {
        ::CA_CannotOpen(fname);
    }
}



// -----------------------------------------------------------------------
// CloseGrFile()
// ------------------------------------------------------------------------
void CloseGrFile()
{
    ::grhandle.close();
}


// -----------------------------------------------------------------------
// OpenMapFile()
// ------------------------------------------------------------------------
void OpenMapFile()
{
    std::string fname;

#ifdef CARMACIZED
    strcpy(fname, "GAMEMAPS.");
    strcat(fname, extension);

    if ((maphandle = open(fname,
                          O_RDONLY | O_BINARY, S_IREAD)) == -1)
    {
        CA_CannotOpen(fname);
    }
#else
    fname = ::mfilename + ::extension;

    ::maphandle.open(fname);

    if (!::maphandle.is_open()) {
        ::CA_CannotOpen(fname);
    }
#endif
}


// -----------------------------------------------------------------------
// CloseMapFile()
// ------------------------------------------------------------------------
void CloseMapFile()
{
    ::maphandle.close();
}



// -----------------------------------------------------------------------
// OpenAudioFile()
// ------------------------------------------------------------------------
void OpenAudioFile()
{
    std::string fname;

#ifndef AUDIOHEADERLINKED
    fname = ::afilename + ::extension;

    ::audiohandle.open(fname);

    if (!::audiohandle.is_open()) {
        ::CA_CannotOpen(fname);
    }
#else
    if ((audiohandle = open("AUDIO."EXTENSION,
                            O_RDONLY | O_BINARY, S_IREAD)) == -1)
    {
        CA_ERROR(SETUPAUDIO_CANT_OPEN);
    }
#endif
}


// -----------------------------------------------------------------------
// CloseAudioFile()
// ------------------------------------------------------------------------
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
    Sint16 chunk)
{
    grhandle.set_position(GRFILEPOS(chunk));
    grhandle.read(&chunkexplen, sizeof(chunkexplen));

    chunkcomplen = GRFILEPOS(chunk + 1) - GRFILEPOS(chunk) - 4;
}


// MDM - functions removed...

#if 0

/*
==========================
=
= CA_FarRead
=
= Read from a file to a far pointer
=
==========================
*/

boolean CA_FarRead(
    int handle,
    Uint8* dest,
    Sint32 length)
{
    unsigned readlen;

//      if (length>0xffffl)
//              Quit ("CA_FarRead doesn't support 64K reads yet!");

    while (length) {
        if (length > 0xffff) {
            readlen = 0xffff;
        } else {
            readlen = length;
        }

        asm             push ds
        asm             mov bx, [handle]
        asm             mov cx, [readlen]
        asm             mov dx, [WORD PTR dest]
        asm             mov ds, [WORD PTR dest + 2]
        asm             mov ah, 0x3f // READ w/handle
        asm             int 21h
        asm             pop ds
        asm             jnc good
        errno = _AX;
        return false;
good:
        asm             cmp ax, [WORD PTR length]
        asm             je done
        errno = EINVFMT; // user manager knows this is bad read
        return false;
done:
        length -= readlen;
    }

    return true;
}


/*
==========================
=
= CA_FarWrite
=
= Write from a file to a far pointer
=
==========================
*/

boolean CA_FarWrite(
    int handle,
    Uint8* source,
    Sint32 length)
{
    unsigned writelen;

//      if (length>0xffffl)
//              Quit ("CA_FarWrite doesn't support 64K reads yet!");

    while (length) {
        if (length > 0xffff) {
            writelen = 0xffff;
        } else {
            writelen = length;
        }

        asm             push ds
        asm             mov bx, [handle]
        asm             mov cx, [writelen]
        asm             mov dx, [WORD PTR source]
        asm             mov ds, [WORD PTR source + 2]
        asm             mov ah, 0x40 // WRITE w/handle
        asm             int 21h
        asm             pop ds
        asm             jnc good
        errno = _AX;
        return false;
good:
        asm             cmp ax, [WORD PTR length]
        asm             je done
        errno = ENOMEM; // user manager knows this is bad write
        return false;

done:
        length -= writelen;
    }

    return true;
}

/*
==========================
=
= CA_ReadFile
=
= Reads a file into an allready allocated buffer
=
==========================
*/

boolean CA_ReadFile(
    char* filename,
    memptr* ptr)
{
    int handle;
    Sint32 size;

    if ((handle = open(filename, O_RDONLY | O_BINARY, S_IREAD)) == -1) {
        return false;
    }

    size = filelength(handle);
    if (!CA_FarRead(handle, *ptr, size)) {
        close(handle);
        return false;
    }
    close(handle);
    return true;
}


/*
==========================
=
= CA_WriteFile
=
= Writes a file from a memory buffer
=
==========================
*/

boolean CA_WriteFile(
    char* filename,
    void* ptr,
    Sint32 length)
{
    int handle;
    Sint32 size;

    handle = open(filename, O_CREAT | O_BINARY | O_WRONLY,
                  S_IREAD | S_IWRITE | S_IFREG);

    if (handle == -1) {
        return false;
    }

    if (!CA_FarWrite(handle, ptr, length)) {
        close(handle);
        return false;
    }
    close(handle);
    return true;
}



/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

boolean CA_LoadFile(
    char* filename,
    memptr* ptr)
{
    int handle;
    Sint32 size;

    if ((handle = open(filename, O_RDONLY | O_BINARY, S_IREAD)) == -1) {
        return false;
    }

    size = filelength(handle);
    MM_GetPtr(ptr, size);
    if (!CA_FarRead(handle, *ptr, size)) {
        close(handle);
        return false;
    }
    close(handle);
    return true;
}

#endif // MDM END

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
    Uint8* source,
    Uint8* destination,
    Sint32 length,
    huffnode* hufftable)
{
    Uint8 val = *source++;
    Uint8 mask = 1;
    Uint16 nodeval;

    huffnode* headptr = &hufftable[254]; // head node is always node 254

    Uint8* dst = destination;
    Uint8* end = dst + length;

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
            dst[0] = static_cast<Uint8>(nodeval);
            ++dst;
            huffptr = headptr;
        } else {
            huffptr = &hufftable[nodeval - 256];
        }
    }
}

void ca_huff_expand_on_screen(
    Uint8* source,
    huffnode* hufftable)
{
    Uint8 val = *source++;
    Uint8 mask = 1;
    Uint16 nodeval;

    huffnode* headptr = &hufftable[254]; // head node is always node 254
    huffnode* huffptr = headptr;

    for (int p = 0; p < 4; ++p) {
        int x = p;
        int y = 0;

        while (y < k_ref_height) {
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
                VL_Plot(x, y, static_cast<Uint8>(nodeval));
                huffptr = headptr;

                x += 4;

                if (x >= k_ref_width) {
                    x = p;
                    ++y;
                }
            } else {
                huffptr = &hufftable[nodeval - 256];
            }
        }
    }
}

/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG 0xa7
#define FARTAG 0xa8


#ifdef CARMACIZED

void CAL_CarmackExpand(
    Uint16* source,
    Uint16* dest,
    Uint16 length)
{
    Uint16 ch, chhigh, count, offset;
    Uint16* copyptr, * inptr, * outptr;

    length /= 2;

    inptr = source;
    outptr = dest;

    while (length) {
        ch = *inptr++;
        chhigh = ch >> 8;
        if (chhigh == NEARTAG) {
            count = ch & 0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *((Uint8*)inptr)++;
                *outptr++ = ch;
                length--;
            } else {
                offset = *((Uint8*)inptr)++;
                copyptr = outptr - offset;
                length -= count;
                while (count--) {
                    *outptr++ = *copyptr++;
                }
            }
        } else if (chhigh == FARTAG) {
            count = ch & 0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *((Uint8*)inptr)++;
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


#if 0

/*
======================
=
= CA_RLEWcompress
=
======================
*/
Sint32 CA_RLEWCompress(
    Uint16* source,
    Sint32 length,
    Uint16* dest,
    Uint16 rlewtag)
{
    Sint32 complength;
    Uint16 value, count, i;
    Uint16* start, * end;

    start = dest;

    end = source + (length + 1) / 2;

//
// compress it
//
    do {
        count = 1;
        value = *source++;
        while (*source == value && source < end) {
            count++;
            source++;
        }
        if (count > 3 || value == rlewtag) {
            //
            // send a tag / count / value string
            //
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        } else {
            //
            // send word without compressing
            //
            for (i = 1; i <= count; i++) {
                *dest++ = value;
            }
        }

    } while (source < end);

    complength = 2 * (dest - start);
    return complength;
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
    Uint16* source,
    Uint16* dest,
    Sint32 length,
    Uint16 rlewtag)
{
    Uint16 i;
    Uint16 value;
    Uint16 count;
    const Uint16* end = &dest[length / 2];

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
}

// ===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk(
    Sint16 chunk)
{
    Sint32 pos, compressed;
#ifdef AUDIOHEADERLINKED
    Sint32 expanded;
    memptr bigbufferseg;
    Uint8* source;
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

#if FORCE_FILE_CLOSE
    OpenAudioFile();
#endif

    audiohandle.set_position(pos);

#ifndef AUDIOHEADERLINKED

    audiosegs[chunk] = new Uint8[compressed];
    audiohandle.read(audiosegs[chunk], compressed);

#else

    if (compressed <= BUFFERSIZE) {
        CA_FarRead(audiohandle, bufferseg, compressed);
        source = bufferseg;
    } else {
        MM_GetPtr(&bigbufferseg, compressed);
        if (mmerror) {
#if FORCE_FILE_CLOSE
            CloseAudioFile();
#endif
            return;
        }
        MM_SetLock(&bigbufferseg, true);
        CA_FarRead(audiohandle, bigbufferseg, compressed);
        source = bigbufferseg;
    }

    expanded = *(Sint32*)source;
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

#if FORCE_FILE_CLOSE
    CloseAudioFile();
#endif
}

// ===========================================================================

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
    Uint16 start = 0, i;

    switch (oldsoundmode) {
    case sdm_Off:
        break;

    case sdm_AdLib:
        start = STARTADLIBSOUNDS;
        break;
    }

cachein:

    switch (SoundMode) {
    case sdm_Off:
        return;

    case sdm_AdLib:
        start = STARTADLIBSOUNDS;
        break;
    }

    for (i = 0; i < NUMSOUNDS; i++, start++) {
        CA_CacheAudioChunk(start);
    }

    oldsoundmode = SoundMode;
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
    Sint16 chunk,
    Uint8* source)
{
    Sint32 expanded;


    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS) {
        //
        // expanded sizes of tile8/16/32 are implicit
        //

#define BLOCK 64
#define MASKBLOCK 128

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
        expanded = bstone::Endian::le(*reinterpret_cast<Sint32*>(source));
        source += 4; // skip over length
    }

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
    grsegs[chunk] = new char[expanded];

    CAL_HuffExpand(source, static_cast<Uint8*>(grsegs[chunk]), expanded, grhuffman);

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
    Sint16 chunk)
{
    Sint32 pos, compressed;
    Uint8* bigbufferseg = NULL;
    Uint8* source;
    Sint16 next;

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


    grhandle.set_position(pos);

    if (compressed <= BUFFERSIZE) {
        grhandle.read(bufferseg, compressed);
        source = static_cast<Uint8*>(bufferseg);
    } else {
        bigbufferseg = new Uint8[compressed];
        grhandle.read(bigbufferseg, compressed);
        source = bigbufferseg;
    }

    CAL_ExpandGrChunk(chunk, source);

    if (compressed > BUFFERSIZE) {
        delete [] bigbufferseg;
        bigbufferseg = NULL;
    }


}



// ==========================================================================

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
    Sint16 chunk)
{
    Sint32 pos, compressed, expanded;
    Uint8* bigbufferseg;
    Uint8* source;
    Sint16 next;


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

    bigbufferseg = new Uint8[compressed];
    grhandle.read(bigbufferseg, compressed);
    source = bigbufferseg;

    expanded = *(Sint32*)source;
    source += 4; // skip over length

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
    ca_huff_expand_on_screen(source, grhuffman);

    delete [] bigbufferseg;
    bigbufferseg = NULL;
}

// ==========================================================================

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
    Sint16 mapnum)
{
    Sint32 pos, compressed;
    Sint16 plane;
    Uint16** dest;
    Uint16* bigbufferseg = NULL;
    Uint16 size;
    Uint16* source;
#ifdef CARMACIZED
    memptr buffer2seg;
    Sint32 expanded;
#endif

    mapon = mapnum;

#if FORCE_FILE_CLOSE
    OpenMapFile();
#endif

//
// load the planes into the allready allocated buffers
//
    size = 64 * 64 * 2;

    for (plane = 0; plane < MAPPLANES; plane++) {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];

        dest = &mapsegs[plane];

        maphandle.set_position(pos);
        if (compressed <= BUFFERSIZE) {
            source = static_cast<Uint16*>(bufferseg);
        } else {
            bigbufferseg = new Uint16[compressed / 2];
            source = bigbufferseg;
        }

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
        CAL_CarmackExpand(source, (Uint16*)buffer2seg, expanded);
        CA_RLEWexpand(((Uint16*)buffer2seg) + 1, *dest, size,
                      ((mapfiletype*)tinf)->RLEWtag);
        MM_FreePtr(&buffer2seg);

#else
        //
        // unRLEW, skipping expanded length
        //
        CA_RLEWexpand(source + 1, *dest, size,
                      rlew_tag);
#endif

        if (compressed > BUFFERSIZE) {
            delete [] bigbufferseg;
            bigbufferseg = NULL;
        }
    }

#if FORCE_FILE_CLOSE
    CloseMapFile();
#endif


}

// ===========================================================================

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
        CA_ERROR(CA_UPLEVEL_PAST_MAX);
    }

    ca_levelbit <<= 1;
    ca_levelnum++;
}

// ===========================================================================

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
        CA_ERROR(CA_DOWNLEVEL_PAST_MIN);
    }
    ca_levelbit >>= 1;
    ca_levelnum--;
    CA_CacheMarks();
}

// ===========================================================================


#if 0

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/

void CA_ClearMarks()
{
    Sint16 i;

    for (i = 0; i < NUMCHUNKS; i++) {
        grneeded[i] &= ~ca_levelbit;
    }
}

#endif


// ===========================================================================

#if 0

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/

void CA_ClearAllMarks()
{
    _fmemset(grneeded, 0, sizeof(grneeded));
    ca_levelbit = 1;
    ca_levelnum = 0;
}

#endif


// ===========================================================================



#if 0
/*
======================
=
= CA_FreeGraphics
=
======================
*/
void CA_SetGrPurge()
{
    Sint16 i;

//
// free graphics
//
    CA_ClearMarks();

    for (i = 0; i < NUMCHUNKS; i++) {
        if (grsegs[i]) {
            MM_SetPurge(&(memptr)grsegs[i], 3);
        }
    }
}
#endif


#if 0

/*
======================
=
= CA_SetAllPurge
=
= Make everything possible purgable
=
======================
*/

void CA_SetAllPurge()
{
    Sint16 i;


//
// free sounds
//
    for (i = 0; i < NUMSNDCHUNKS; i++) {
        if (audiosegs[i]) {
            MM_SetPurge(&(memptr)audiosegs[i], 3);
        }
    }

//
// free graphics
//
    CA_SetGrPurge();
}

#endif


// ===========================================================================

/*
======================
=
= CA_CacheMarks
=
======================
*/
#define MAXEMPTYREAD 1024

void CA_CacheMarks()
{
    Sint16 i, next, numcache;
    Sint32 pos, endpos, nextpos, nextendpos, compressed;
    Sint32 bufferstart, bufferend; // file position of general buffer
    Uint8* source;
    Uint8* bigbufferseg = NULL;

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

            if (compressed <= BUFFERSIZE) {
                if (bufferstart <= pos
                    && bufferend >= endpos)
                {
                    // data is allready in buffer
                    source = (Uint8*)bufferseg + (pos - bufferstart);
                } else {
                    // load buffer with a new block from disk
                    // try to get as many of the needed blocks in as possible
                    while (next < NUMCHUNKS) {
                        while (next < NUMCHUNKS &&
                               !(grneeded[next] & ca_levelbit && !grsegs[next]))
                        {
                            next++;
                        }
                        if (next == NUMCHUNKS) {
                            continue;
                        }

                        nextpos = GRFILEPOS(next);
                        while (GRFILEPOS(++next) == -1) { // skip past any sparse tiles
                        }
                        nextendpos = GRFILEPOS(next);
                        if (nextpos - endpos <= MAXEMPTYREAD
                            && nextendpos - pos <= BUFFERSIZE)
                        {
                            endpos = nextendpos;
                        } else {
                            next = NUMCHUNKS; // read pos to posend
                        }
                    }

                    grhandle.set_position(pos);
                    grhandle.read(bufferseg, endpos - pos);
                    bufferstart = pos;
                    bufferend = endpos;
                    source = static_cast<Uint8*>(bufferseg);
                }
            } else {
                // big chunk, allocate temporary buffer
                bigbufferseg = new Uint8[compressed];
                grhandle.set_position(pos);
                grhandle.read(bigbufferseg, compressed);
                source = bigbufferseg;
            }

            CAL_ExpandGrChunk(i, source);

            if (compressed > BUFFERSIZE) {
                delete [] bigbufferseg;
                bigbufferseg = NULL;
            }

        }
    }
}

void CA_CannotOpen(
    const std::string& string)
{
    auto str = "Can't open " + string + "!\n";
    ::Quit(str.c_str());
}

void UNCACHEGRCHUNK(
    Uint16 chunk)
{
    delete [] static_cast<char*>(grsegs[chunk]);
    grsegs[chunk] = NULL;

    grneeded[chunk] &= ~ca_levelbit;
}

std::string ca_load_script(
    int chunk_id,
    bool strip_xx)
{
    ::CA_CacheGrChunk(static_cast<Sint16>(chunk_id));

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
