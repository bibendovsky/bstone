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


#ifndef BSTONE_ID_CA_INCLUDED
#define BSTONE_ID_CA_INCLUDED


#include <cstdint>
#include "3d_def.h"


extern int NUM_EPISODES;
extern int MAPS_PER_EPISODE;
extern int MAPS_WITH_STATS;

extern int NUMMAPS;

const int MAPPLANES = 2;


void UNCACHEGRCHUNK(
    uint16_t chunk);

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE 3
#else
#define FILEPOSSIZE 4
#endif

// ===========================================================================

struct maptype {
    int32_t planestart[3];
    uint16_t planelength[3];
    uint16_t width, height;
    char name[16];
}; // maptype

struct huffnode {
    uint16_t bit0, bit1; // 0-255 is a character, > is a pointer to a node
}; // huffnode

struct mapfiletype {
    uint16_t RLEWtag;
    int32_t headeroffsets[100];
//      uint8_t           tileinfo[];
}; // mapfiletype

// ===========================================================================

using AudioSegments = std::vector<uint8_t*>;
using GrSegments = std::vector<void*>;
using GrNeeded = std::vector<uint8_t>;
using MapHeaderSegments = std::vector<maptype*>;


extern std::string audioname;

extern uint16_t rlew_tag;

extern int16_t mapon;

extern uint16_t* mapsegs[MAPPLANES];
extern MapHeaderSegments mapheaderseg;
extern AudioSegments audiosegs;
extern GrSegments grsegs;

extern GrNeeded grneeded;
extern uint8_t ca_levelbit, ca_levelnum;

extern char* titleptr[8];

extern int16_t profilehandle, debughandle;

extern std::string extension;
extern std::string gheadname;
extern std::string gfilename;
extern std::string gdictname;
extern std::string mheadname;
extern std::string mfilename;
extern std::string aheadname;
extern std::string afilename;

extern int32_t* grstarts; // array of offsets in egagraph, -1 for sparse
extern int32_t* audiostarts; // array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern void (* drawcachebox)(
    char* title,
    uint16_t numcache);
extern void (* updatecachebox)();
extern void (* finishcachebox)();

extern bstone::FileStream grhandle;
extern bstone::FileStream maphandle;
extern bstone::FileStream audiohandle;

extern int32_t chunkcomplen, chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode* grhuffman;
#else
extern huffnode grhuffman[255];
#endif

// ===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite(
    uint16_t segment,
    uint16_t source,
    uint16_t dest,
    uint16_t width,
    uint16_t height,
    uint16_t pixshift);

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
    int32_t length);

int32_t CA_RLEWCompress(
    uint16_t* source,
    int32_t length,
    uint16_t* dest,
    uint16_t rlewtag);

void CA_RLEWexpand(
    uint16_t* source,
    uint16_t* dest,
    int32_t length,
    uint16_t rlewtag);

void CA_Startup();
void CA_Shutdown();

void CA_SetGrPurge();
void CA_CacheAudioChunk(
    int16_t chunk);
void CA_LoadAllSounds();

void CA_UpLevel();
void CA_DownLevel();

void CA_SetAllPurge();

void CA_ClearMarks();
void CA_ClearAllMarks();

#define CA_MarkGrChunk(chunk) grneeded[chunk] |= ca_levelbit

void CA_CacheGrChunk(
    int16_t chunk);
void CA_CacheMap(
    int16_t mapnum);

void CA_CacheMarks();

void CAL_SetupAudioFile();
void CAL_SetupGrFile();
void CAL_SetupMapFile();

void CAL_HuffExpand(
    uint8_t* source,
    uint8_t* dest,
    int32_t length,
    huffnode* hufftable);

void ca_huff_expand_on_screen(
    uint8_t* source,
    huffnode* hufftable);

void CloseGrFile();
void OpenGrFile();

// BBi
extern int ca_gr_last_expanded_size;

std::string ca_load_script(
    int chunk_id,
    bool strip_xx = false);


#endif // BSTONE_ID_CA_INCLUDED
