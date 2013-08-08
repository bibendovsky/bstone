#ifndef _ID_HEADS_H_
#define _ID_HEADS_H_


// ID_GLOB.H


#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#define __ID_GLOB__

//--------------------------------------------------------------------------


#define DEBUG_VALUE
#define CEILING_FLOOR_COLORS


//#define CARMACIZED
#define WOLF
#define FREE_FUNCTIONS					(0)
#define FREE_DATA							(0)
#define DEMOS_ENABLED					(1)
#define RESTART_PICTURE_PAUSE			(0)
#define GEORGE_CHEAT						(0)

#define FORCE_FILE_CLOSE				(1)		// true - forces all files closed once they are used

//
// GAME VERSION TYPES
//

#define SHAREWARE_VERSION			0x0001
#define MISSIONS_1_THR_3			0x0003
#define MISSIONS_4_THR_6			0x0004
#define MISSIONS_1_THR_6			0x0007

//
// CURRENT GAME VERSION DEFINE - Change this define according to the
//											game release versions 1,1-3,4-6, or 1-6.

#define GAME_VERSION					(MISSIONS_1_THR_6)
//#define GAME_VERSION      			(MISSIONS_1_THR_3)
//#define GAME_VERSION					(SHAREWARE_VERSION)


#define TECH_SUPPORT_VERSION		(0)
#define IN_DEVELOPMENT				(0)

#define ERROR_LOG						"ERROR.LOG"			// Text filename for critical memory errors
#define DUAL_SWAP_FILES				(0)				//(GAME_VERSION != SHAREWARE_VERSION)				// Support for Shadowed and NonShadowed page files

extern  char            signonv1;
#define introscn        signonv1

#ifndef SPEAR

#include "gfxv_vsi.h"
#include "audiovsi.h"

// FIXME Got "unexpected end of file" error
//#include "mapsvsi.h"

#else

#include "GFXV_SOD.H"
#include "AUDIOSOD.H"
#include "MAPSSOD.H"

#endif

//-----------------


#define GREXT   "VGA"

//
//      ID Engine
//      Types.h - Generic types, #defines, etc.
//      v1.0d1
//

#define false (0)
#define true (1)
typedef short boolean;
typedef unsigned        char            byte;
typedef unsigned        short                     word;
typedef unsigned        long            longword;
typedef byte *                                  Ptr;

typedef struct
		{
			short     x,y;
		} Point;
typedef struct
		{
			Point   ul,lr;
		} Rect;

#define nil     (0l)


#include "id_mm.h"
#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"

#include "jm_tp.h"
#include "jm_debug.h"
#include "jm_error.h"

#include "movie.h"

void    Quit (char *error,...);             // defined in user program

extern void CalcMemFree(void);

//
// replacing refresh manager with custom routines
//

#define PORTTILESWIDE           20      // all drawing takes place inside a
#define PORTTILESHIGH           13              // non displayed port of this size

#define UPDATEWIDE                      PORTTILESWIDE
#define UPDATEHIGH                      PORTTILESHIGH

#define MAXTICS                         10
#define DEMOTICS                        4

#define UPDATETERMINATE 0x0301

extern  unsigned short        mapwidth,mapheight,tics,realtics;
extern  boolean         compatability;

extern  byte            *updateptr;
extern  unsigned short        uwidthtable[UPDATEHIGH];
extern  unsigned short        blockstarts[UPDATEWIDE*UPDATEHIGH];

extern  byte            fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

#include "3d_menu.h"


#define CA_FarRead(h,d,s)	IO_FarRead((short)h,(byte *)d,(long)s)


// BBi
extern unsigned char* vga_memory;


#endif