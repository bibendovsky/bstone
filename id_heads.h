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


#ifndef _ID_HEADS_H_
#define _ID_HEADS_H_


// ID_GLOB.H


#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <exception>
#include <memory>
#include <vector>

#include "SDL.h"

#include "bstone_binary_reader.h"
#include "bstone_binary_writer.h"
#include "bstone_c.h"
#include "bstone_endian.h"
#include "bstone_file_stream.h"
#include "bstone_format_string.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_memory_stream.h"
#include "bstone_string_helper.h"
#include "bstone_cl_args.h"

// FIXME
// BBi Enable only if neccesary!
#if 0
#pragma GCC diagnostic ignored "-fpermissive"
#endif // 0

#define __ID_GLOB__

//--------------------------------------------------------------------------

#define DEBUG_VALUE
#define CEILING_FLOOR_COLORS


//#define CARMACIZED
#define WOLF
#define FREE_FUNCTIONS					(0)
#define FREE_DATA							(0)
#define DEMOS_ENABLED					(0)
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

#ifndef BSTONE_AOG
#include "gfxv_vsi.h"
#include "audiovsi.h"

// FIXME Got "unexpected end of file" error
//#include "mapsvsi.h"
#else
#include "gfxv_bs6.h"
#include "audiobs6.h"

// FIXME Got "unexpected end of file" error
//#include "mapsbs6.h"
#endif // BSTONE_AOG

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
typedef Sint16 boolean;

struct Point {
    Sint16 x;
    Sint16 y;
}; // struct Point

struct Rect {
    Point ul;
    Point lr;
}; // struct Rect

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

void    Quit (const char *error,...);             // defined in user program

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

extern  Uint16        mapwidth,mapheight,tics,realtics;
extern  boolean         compatability;

extern  Uint8            *updateptr;
extern  Uint16        uwidthtable[UPDATEHIGH];
extern  Uint16        blockstarts[UPDATEWIDE*UPDATEHIGH];

extern  Uint8            fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=static_cast<Uint8>(f);backcolor=static_cast<Uint8>(b);

#include "3d_menu.h"


#define CA_FarRead(h,d,s)	IO_FarRead((Sint16)h,(Uint8 *)d,(Sint32)s)


// BBi
extern Uint8* vga_memory;
extern bstone::ClArgs g_args;
// BBi

#endif
