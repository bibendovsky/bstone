// ID_CA.H
//===========================================================================


#ifndef ID_CA_H
#define ID_CA_H


#include "SDL.h"


#define NUM_EPISODES			1
#define MAPS_PER_EPISODE	25
#define MAPS_WITH_STATS		20

#define NUMMAPS				NUM_EPISODES*MAPS_PER_EPISODE
#define MAPPLANES	2

void UNCACHEGRCHUNK(Uint16 chunk);

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
#else
#define FILEPOSSIZE	4
#endif

//===========================================================================

typedef	struct
{
	Sint32		planestart[3];
	Uint16	planelength[3];
	Uint16	width,height;
	char		name[16];
} maptype;

typedef struct
{
  Uint16 bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

typedef struct
{
	Uint16	RLEWtag;
	Sint32		headeroffsets[100];
//	Uint8		tileinfo[];
} mapfiletype;

//===========================================================================

extern	char		audioname[13];

// FIXME
//extern	Uint8 		*tinf;
extern Uint16 rlew_tag;

extern	Sint16			mapon;

extern	Uint16	*mapsegs[MAPPLANES];
extern	maptype		*mapheaderseg[NUMMAPS];
extern	Uint8		*audiosegs[NUMSNDCHUNKS];
extern	void		*grsegs[NUMCHUNKS];

extern	Uint8		grneeded[NUMCHUNKS];
extern	Uint8		ca_levelbit,ca_levelnum;

extern	char		*titleptr[8];

extern	Sint16			profilehandle,debughandle;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern Sint32		*grstarts;	// array of offsets in egagraph, -1 for sparse
extern Sint32		*audiostarts;	// array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern	void	(*drawcachebox)		(char *title, Uint16 numcache);
extern	void	(*updatecachebox)	(void);
extern	void	(*finishcachebox)	(void);

extern Sint16			grhandle;		// handle to EGAGRAPH
extern Sint16			maphandle;		// handle to MAPTEMP / GAMEMAPS
extern Sint16			audiohandle;	// handle to AUDIOT / AUDIO
extern Sint32		chunkcomplen,chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode	*grhuffman;
#else
extern huffnode	grhuffman[255];
#endif

//===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite (Uint16 segment,Uint16 source,Uint16 dest,
	Uint16 width, Uint16 height, Uint16 pixshift);

//===========================================================================

void CA_OpenDebug (void);
void CA_CloseDebug (void);
boolean CA_FarRead (Sint16 handle, Uint8 *dest, Sint32 length);
boolean CA_FarWrite (Sint16 handle, Uint8 *source, Sint32 length);
boolean CA_ReadFile (char *filename, void** ptr);
boolean CA_LoadFile (char *filename, void** ptr);
boolean CA_WriteFile (char *filename, void *ptr, Sint32 length);

Sint32 CA_RLEWCompress (Uint16 *source, Sint32 length, Uint16 *dest,
  Uint16 rlewtag);

void CA_RLEWexpand (Uint16 *source, Uint16 *dest,Sint32 length,
  Uint16 rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (Sint16 chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (Sint16 chunk);
void CA_CacheMap (Sint16 mapnum);

void CA_CacheMarks (void);

void CAL_SetupAudioFile (void);
void CAL_SetupGrFile (void);
void CAL_SetupMapFile (void);
void CAL_HuffExpand (Uint8 *source, Uint8 *dest,
  Sint32 length,huffnode *hufftable, boolean screenhack);

void CloseGrFile(void);
void OpenGrFile(void);


#endif // ID_CA_H
