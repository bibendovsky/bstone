#ifndef _JM_ERROR_H_
#define _JM_ERROR_H_



enum ERROR_Units {

JM_IO_ERROR=0x01,	  				// JAM Units
JM_TP_ERROR,
JM_AN_ERROR,

D3_MAIN_ERROR=0x81,				// ID Units
D3_STATE_ERROR,
D3_ACT1_ERROR,
D3_ACT2_ERROR,
D3_GAME_ERROR,
D3_AGENT_ERROR,
D3_SCALE_ERROR,
ID_US1_ERROR,
D3_PLAY_ERROR,
D3_DEBUG_ERROR,
ID_IN_ERROR,
ID_VL_ERROR,
ID_VH_ERROR,
ID_CA_ERROR,
ID_PM_ERROR,
ID_MM_ERROR,
ID_SD_ERROR,
DRAW2_ERROR,
MENU_ERROR,		 
};


enum ERROR_Msgs {


// UNIT: JM_IO
//
IO_LOADFILE_NO_LZW=0x01,  		// No code for LZW compression.
IO_LOADFILE_UNKNOWN,		     	// Unknown compression type.
IO_COPYFILE_OPEN_SRC,			// IO_CopyFile(): Error opening source file.
IO_COPYFILE_OPEN_DEST,			// IO_COpyFile(): Error opening destination file.


// UNIT: JM_TP
//
TP_PRESENTER_EP_RECURSE=0x01,	// Use of ^EP when recursing.
TP_PRESENTER_LONG_TEXT,      	// String too long to print.
TP_CACHEPAGE_ANIM_OF,        	// Too many anims on one page.
TP_INITSCRIPT_PAGES_OF,      	// Too many pages in presenter.
TP_DISPLAY_STR_NUM_BAD,			// String number exceeds max array size.
TP_CANT_FIND_XX_TERMINATOR,	// Can't find the ^XX doc terminator string.

// UNIT: JM_AN
//
AN_BAD_ANIM_FILE=0x01,			// Animation file is corrupt or truncated.
HANDLEPAGE_BAD_CODE,				// Unrecognized anim code.
AN_SHOW_FRAME_NULL,				// MOVIE_ShowFrame() was passed a NULL ptr


// UNIT: 3D_MAIN
//
READINFO_BAD_DECOMP=0x01, 		// Bad decompression during game load.
WRITEINFO_BIGGER_BUF,	    	// Save game compression buffer too small.
SAVELEVEL_DISKERR,	      	// Disk error while changing levels.
CHECKDISK_GDFREE,		      	// Error in _dos_getdiskfree call.
INITACCESS_NOFILE,				// Error loading 'ACCESS.xxx' file.
PREDEMO_NOJAM,						// JAM animation (IANIM.BSx) does not exist.
INITPLAYTEMP_OPEN_ERR,			// InitPlaytemp(): Error opening PLAYTEMP file.
CHECK_FILENAME_TOO_LONG,		// CheckValidity(): Filename is too long.


// UNIT: 3D_STATE
//
TRYWALK_BAD_DIR=0x01,    		// Illegal direction passed.
MOVEOBJ_BAD_DIR,		       	// Illegal direction passed.
SIGHTPLAYER_IN_ATKMODE,      	// Actor is in ATTACKMODE.


// UNIT: 3D_ACT1
//
SPAWNSTATIC_TOO_MANY=0x01,   	// Too many static objects.
PLACEITEMTYPE_NO_TYPE,       	// Couldn't find type.
SPAWNDOOR_TOO_MANY,	      	// Too many doors in level.
SPAWNCONCESSION_TOO_MANY,	   // Too many concession machines in level.
UNRECOGNIZED_CON_CODE,  		// Concession Code (Upper Byte) is unrecognized (Hint Or Food PAL!)
INVALID_CACHE_MSG_NUM,			// Invalid 'Cached Message' number
HINT_BUFFER_OVERFLOW,			// Cached Hint Message is to Long for allocated space
CACHEMSG_TOO_MANY,				// Too many messages in current level.
LINKAREA_BAD_LINK,				// Invalid linkable area.
CHECKLINKED_BAD_LINK,			// Linked door is linked to a non-door.
SPAWNCON_CACHE_MSG_OVERFLOW,	// Too many 'cached msgs' loaded. (CONCESSIONS)
TRANSFORM_AREA1_OUT_OF_RANGE,	// Area1 out of table range.
TRANSFORM_AREA2_OUT_OF_RANGE,	// Area2 out of table range.
SPAWNSTATIC_ON_WALL,				// Static spawned on a wall.
NO_DOORBOMB_SPARES,				// Could not find Fision/Plasma Detonator reserve object.
CANT_FIND_LEVELCOMPUTER,		// Cound not find level computer - Need to have one pal!
CANT_FIND_RESERVE_STATIC,		// Count not find a reserved static at location 0,0 with shape num == 1

// UNIT: 3D_ACT2
//
T_PATH_HIT_WALL=0x01,			// Actor walked out of map.
TOGGLE_BARRIER_NULL,				// ToggleBarrier() was passed a NULL ptr!
TOGGLE_NON_BARRIER,				// ToggleBarrier() was passed a non-barrier object pointer! Oh...Gubs!
BAD_BARRIER_THINK,				// barrier is ON but thinking using T_BarrierTransistion!
ACTOR_ON_WALL,						// An actor has been spawned on an invalid area number
TOO_MANY_DETONATORS,				// Too many Fission/Plasma Detonators are placed in this map! You can only have one!
CANT_FIND_HIDDEN_OBJ,			// Unable to find a "Hidden Actor" at location 0,0
SWITCH_DOES_NOT_MATCH,			// Barrier ON/OFF Switch does not match the connecting barrier object.

// UNIT: 3D_GAME
//
SETUPGAME_BAD_MAP_SIZE=0x01,	// Map not 64 x 64.
SETUPGAME_MAX_EA_WALLS,			// Too many Electro-Alien walls in level.
SETUPGAME_MAX_GOLDIE_SPAWNS,	// Too many Dr. Goldfire Spawn sites in level.
SCANINFO_CACHE_MSG_OVERFLOW,	// Too many 'cached msgs' loaded. (INFORMANTS)
SCANINFO_INVALID_ACTOR,			// Invalid actor in shareware version!
NO_SPACE_ANIM,						// SPACE Animation file (SANIM.BSx) does not exist.
NO_GROUND_ANIM,					// GROUND Animation file (GANIM.BSx) does not exists.
CEILING_TILE_OUT_OF_RANGE,		// CEILING TILE/TEXTURE IS OUT OF RANGE
FLOOR_TILE_OUT_OF_RANGE,		// FLOOR TILE/TEXTURE IS OUT OF RANGE
TOO_MANY_FAST_GOLD_SPAWNS,		// Too many FAST Goldfire spawn sites in map.
UNSUPPORTED_BARRIER_LINK,		// Barrier Link has a NON 0xF8FF value - No longer supported in retail!
NO_DETONATORS_IN_LEVEL,			// No Fision/Plasma Detonator in level! - Go put one in the map PAL!

// UNIT: 3D_AGENT
//
INTERROGATE_LONG_MSG=0x01,		// Interrogation message too long.
BAD_TERMINAL_MSG_NUM,			// Bad Cached Terminal Message number
SWITCH_TABLE_OVERFLOW,			// Too many DIFFERENT(coords) barriers hooked up to switches
UPDATE_SWITCH_BAD_LEVEL,		// Bad level number passed to UpdateBarrierTable
GETAREANUMBER_ON_WALL,			// Can't GetAreaNumber() of object on wall!
BAD_SMART_TRIGGER_PTR,			// A Smart Floor Trigger points to nothing!
INVALID_SMART_SWITCH_LINK,		// A Smart floor trigger points to a non-supported actor
BARRIER_SWITCH_NOT_CONNECTED,	// A barrier switch was not connect to any barriers.
NORMAL_SHADE_TOO_BIG,			// Shade Div (normalshade) TOO large!  Must be 1-12!
SHADEMAX_VALUE_BAD,				// Shade Max (shademax) Value BAD! Must be 5-63!

// UNIT: 3D_SCALE
//
BADSCALE_ERROR=0x01,				// Bad scale called.


// UNIT: ID_US_1
//
US_CPRINTLINE_WIDTH=0x01,		// String exceeds width.


// UNIT: 3D_PLAY
//
POLLCONTROLS_DEMO_OV=0x01,		// Demo buffer overflowed.
GETNEWACTOR_NO_FREE_SPOTS,		// No free spots in objlist.
REMOVEOBJ_REMOVED_PLAYER,		// Tried to remove the player.

// UNIT: 3D_DEBUG
//

// UNIT: ID_IN
//
IN_READCONTROL_PLAY_EXC=0x01,	// Demo playback exceeded.
IN_READCONTROL_BUF_OV,       	// Demo buffer overflow.


// UNIT: ID_VL
//
VL_MUNGEPIC_NO_DIV_FOUR=0x01,	// Not divisible by 4.

// UNIT: ID_VH
//

// UNIT: ID_CA
//
CA_SETUPAUDIO_CANT_OPEN=0x01,	// Can't open audio file.
CA_UPLEVEL_PAST_MAX,	       	// Up past level 7.
CA_DOWNLEVEL_PAST_MIN,       	// Down past level 0.


// UNIT: ID_PM
//
PML_MAPEMS_MAPPING_FAIL=0x01,	// Page mapping failed.
PML_SHUTDOWNEMS_FREE,       	// Error freeing EMS.
PML_XMSCOPY_ZERO_ADRS,       	// Zero address.
PML_XMSCOPY_COPY_ERROR,	    	// Error on copy.
PML_SHUTDOWNXMS_FREE,	    	// Error freeing XMS.
PML_STARTUPMAINMEM_LOW,	     	// Not enough main memory.
PML_READFROMFILE_NULL,	      // Null pointer.
PML_READFROMFILE_ZERO,	      // Zero offset.
PML_READFROMFILE_SEEK,	    	// Seek failed.
PML_READFROMFILE_READ,       	// Read failed.
PML_OPENPAGEFILE_OPEN,       	// Unable to open page file.
PML_OPENPAGEFILE_OFF,	    	// Offset read failed.
PML_OPENPAGEFILE_LEN,	    	// Length read failed.
PML_GETEMSADDRESS_FIND,	    	// EMS find failed.
PML_GIVELRUPAGE_SEARCH,	    	// LRU search failed.
PML_PUTPAGEINXMS_NO_LRU,     	// No XMS LRU.
PML_TRANSFERPAGE_IDENT,    	// Identity replacement.
PML_TRANSFERPAGE_KILL,	    	// Killing locked page.
PML_TRANSFERPAGE_REUSE,	    	// Reusing non-existent page.
PML_TRANSFERPAGE_ZERO,	     	// Zero replacement.
PML_GETPAGEBUFFER_LIED,	      // MainPagesAvail lied.
PML_GETPAGEBUFFER_PURGE,    	// Purged main block.
PML_GETPAGEBUFFER_SEARC,    	// Search failed.
PML_GETPAGEFROMXMS_SEG,	    	// Non segment pointer.
PM_GETPAGE_BAD_PAGE,		    	// Invalid page request.
PM_GETPAGE_SPARSE_PAGE,	    	// Tried to load a sparse page.
PM_SETPAGELOCK_NON_SND,	    	// Locking/unlocking non-sound page.
PM_PRELOAD_PAGES_OV,	       	// More pages than chunks in file.
PM_PRELOAD_XMS_FAIL,	       	// XMS buffer failed.
PM_PRELOAD_EXCEED,		     	// Exceeded XMS pages.
PM_PRELOAD_TOO_LONG,	       	// Page too long.
PM_STARTUP_NO_MAIN_EMS,	     	// No main or EMS.


// UNIT: ID_MM
//
MML_USESPACE_TWO_BLOCKS=0x01,	// Segment spans two blocks.
MML_CLEARBLOCK_NO_PURGE_BLKS,	// No purgable blocks.
MM_GETPTR_OUT_OF_MEMORY,		// Out of memory.
MM_FREEPTR_BLOCK_NOT_FOUND,	// Block not found.
MM_SETPURGE_BLOCK_NOT_FOUND,	// Block not found.
MM_SETLOCK_BLOCK_NOT_FOUND,	// Block not found.
MM_SHOWMEMORY_BLK_CORRUPT,		// Memory block order corrupted.
MM_DUMPDATA_FILE_ERROR,			// Couldn't open MMDUMP.TXT.
MM_DUMPDATA_FILE_CREATED,		// MMDUMP.TXT created.
MM_GETMEM_WITH_PAGEMANAGER,	// MM_GetPtr() was called while PageManager was installed

// UNIT: ID_SD
//
SD_PLAYSOUND_ZERO_LEN=0x01,	// Zero length sound.
SD_PLAYSOUND_PRI_NO_SOUND,		// Priority without a sound.
SD_PLAYSOUND_UNCACHED,			// Uncached sound.
SD_STARTUP_BAD_DMA,				// Unsupported DMA value in BLASTER.
SD_STARTUP_BAD_INTERRUPT,		// Unsupported interrupt value in BLASTER.
SD_STARTUP_BAD_ADDRESS,			// Unsupported address value in BLASTER.
SDL_ALPLAYSOUND_BAD_INST,		// Bad instrument.
SD_PLAYDIGITIZED_BAD_SOUND,	// Bad sound number.
SD_SETPOSITION_BAD_POSITION,	// Illegal position.
SDL_STARTSB_BAD_INTERRUPT,		// Illegal or unsupported interrupt number for SoundBlaster.
SDL_SBSETDMA_BAD_DMA,			// Invalid SoundBlaster DMA channel.


// UNIT: 3D_DRAW2
//
NULL_FUNC_PTR_PASSED=0x01,			// Null Function Pointer passed - MapRowPtr was NOT initalized!!!

// UNIT: 3D_MENU
//
CACHE_MESSAGE_NO_END_MARKER,  // A cached message was NOT terminated with "^XX"
};


// Use ERROR() when passing "Unit:  Msg:" info -- this keeps Quit()
// flexible and optimizes _DATA usage.
//
// Also, other xxx_ERROR macros are included.
//
extern char QuitMsg[];
#define ERROR(Unit,Error)		Quit(QuitMsg,Unit,Error)

#define IO_ERROR(Error)			ERROR(JM_IO_ERROR,Error)
#define TP_ERROR(Error)			ERROR(JM_TP_ERROR,Error)
#define AN_ERROR(Error)			ERROR(JM_AN_ERROR,Error)

#define MAIN_ERROR(Error)		ERROR(D3_MAIN_ERROR,Error)
#define STATE_ERROR(Error)		ERROR(D3_STATE_ERROR,Error)
#define DEBUG_ERROR(Error)		ERROR(D3_DEBUG_ERROR,Error)
#define PLAY_ERROR(Error)		ERROR(D3_PLAY_ERROR,Error)
#define ACT1_ERROR(Error)		ERROR(D3_ACT1_ERROR,Error)
#define ACT2_ERROR(Error)		ERROR(D3_ACT2_ERROR,Error)
#define GAME_ERROR(Error)		ERROR(D3_GAME_ERROR,Error)
#define AGENT_ERROR(Error)		ERROR(D3_AGENT_ERROR,Error)
#define SCALE_ERROR(Error)		ERROR(D3_SCALE_ERROR,Error)
#define US1_ERROR(Error)		ERROR(ID_US1_ERROR,Error)
#define IN_ERROR(Error)			ERROR(ID_IN_ERROR,Error)
#define CA_ERROR(Error)			ERROR(ID_CA_ERROR,Error)
#define PM_ERROR(Error)			ERROR(ID_PM_ERROR,Error)
#define VH_ERROR(Error)			ERROR(ID_VH_ERROR,Error)
#define SD_ERROR(Error)			ERROR(ID_SD_ERROR,Error)
#define MM_ERROR(Error)			ERROR(ID_MM_ERROR,Error)
#define DRAW2_ERROR(Error)		ERROR(DRAW2_ERROR,Error)
#define MENU_ERROR(Error)		ERROR(MENU_ERROR,Error)

#endif
