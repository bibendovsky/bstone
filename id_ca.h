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


// ID_CA.H
// ===========================================================================


#ifndef ID_CA_H
#define ID_CA_H


#include "SDL.h"


#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
#define NUM_EPISODES (6)
#else
#define NUM_EPISODES (1)
#endif
#define MAPS_PER_EPISODE (15)
#define MAPS_WITH_STATS (11)
#else
#define NUM_EPISODES (1)
#define MAPS_PER_EPISODE (25)
#define MAPS_WITH_STATS (20)
#endif

#define NUMMAPS NUM_EPISODES * MAPS_PER_EPISODE
#define MAPPLANES 2

void UNCACHEGRCHUNK(
    Uint16 chunk);

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE 3
#else
#define FILEPOSSIZE 4
#endif

// ===========================================================================

struct maptype {
    Sint32 planestart[3];
    Uint16 planelength[3];
    Uint16 width, height;
    char name[16];
}; // struct maptype

struct huffnode {
    Uint16 bit0, bit1; // 0-255 is a character, > is a pointer to a node
}; // struct huffnode

struct mapfiletype {
    Uint16 RLEWtag;
    Sint32 headeroffsets[100];
//      Uint8           tileinfo[];
}; // struct mapfiletype

// ===========================================================================

using AudioSegments = std::vector<uint8_t*>;
using GrSegments = std::vector<void*>;
using GrNeeded = std::vector<uint8_t>;


extern std::string audioname;

extern Uint16 rlew_tag;

extern Sint16 mapon;

extern Uint16* mapsegs[MAPPLANES];
extern maptype* mapheaderseg[NUMMAPS];
extern AudioSegments audiosegs;
extern GrSegments grsegs;

extern GrNeeded grneeded;
extern Uint8 ca_levelbit, ca_levelnum;

extern char* titleptr[8];

extern Sint16 profilehandle, debughandle;

extern std::string extension;
extern std::string gheadname;
extern std::string gfilename;
extern std::string gdictname;
extern std::string mheadname;
extern std::string mfilename;
extern std::string aheadname;
extern std::string afilename;

extern Sint32* grstarts; // array of offsets in egagraph, -1 for sparse
extern Sint32* audiostarts; // array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern void (* drawcachebox)(
    char* title,
    Uint16 numcache);
extern void (* updatecachebox)();
extern void (* finishcachebox)();

extern bstone::FileStream grhandle;
extern bstone::FileStream maphandle;
extern bstone::FileStream audiohandle;

extern Sint32 chunkcomplen, chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode* grhuffman;
#else
extern huffnode grhuffman[255];
#endif

// ===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite(
    Uint16 segment,
    Uint16 source,
    Uint16 dest,
    Uint16 width,
    Uint16 height,
    Uint16 pixshift);

// ===========================================================================

void CA_OpenDebug();
void CA_CloseDebug();

boolean CA_ReadFile(
    char* filename,
    void** ptr);
boolean CA_LoadFile(
    char* filename,
    void** ptr);
boolean CA_WriteFile(
    char* filename,
    void* ptr,
    Sint32 length);

Sint32 CA_RLEWCompress(
    Uint16* source,
    Sint32 length,
    Uint16* dest,
    Uint16 rlewtag);

void CA_RLEWexpand(
    Uint16* source,
    Uint16* dest,
    Sint32 length,
    Uint16 rlewtag);

void CA_Startup();
void CA_Shutdown();

void CA_SetGrPurge();
void CA_CacheAudioChunk(
    Sint16 chunk);
void CA_LoadAllSounds();

void CA_UpLevel();
void CA_DownLevel();

void CA_SetAllPurge();

void CA_ClearMarks();
void CA_ClearAllMarks();

#define CA_MarkGrChunk(chunk) grneeded[chunk] |= ca_levelbit

void CA_CacheGrChunk(
    Sint16 chunk);
void CA_CacheMap(
    Sint16 mapnum);

void CA_CacheMarks();

void CAL_SetupAudioFile();
void CAL_SetupGrFile();
void CAL_SetupMapFile();

void CAL_HuffExpand(
    Uint8* source,
    Uint8* dest,
    Sint32 length,
    huffnode* hufftable);

void ca_huff_expand_on_screen(
    Uint8* source,
    huffnode* hufftable);

void CloseGrFile();
void OpenGrFile();

// BBi
extern int ca_gr_last_expanded_size;

std::string ca_load_script(
    int chunk_id,
    bool strip_xx = false);


#endif // ID_CA_H
