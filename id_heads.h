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


#ifndef BSTONE_ID_HEADS_INCLUDED
#define BSTONE_ID_HEADS_INCLUDED


// ID_GLOB.H


#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "SDL.h"

#include "bstone_binary_reader.h"
#include "bstone_binary_writer.h"
#include "bstone_endian.h"
#include "bstone_file_stream.h"
#include "bstone_format_string.h"
#include "bstone_log.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_memory_stream.h"
#include "bstone_string_helper.h"
#include "bstone_cl_args.h"

// FIXME
// BBi Enable only if necessary!
#if 0
#pragma GCC diagnostic ignored "-fpermissive"
#endif // 0

#define __ID_GLOB__

// --------------------------------------------------------------------------

#define DEBUG_VALUE
#define CEILING_FLOOR_COLORS


// #define CARMACIZED
#define WOLF
#define FREE_FUNCTIONS (0)
#define FREE_DATA (0)
#define DEMOS_ENABLED (0)
#define RESTART_PICTURE_PAUSE (0)
#define GEORGE_CHEAT (0)

#define FORCE_FILE_CLOSE (1) // true - forces all files closed once they are used

//
// GAME VERSION TYPES
//

#define SHAREWARE_VERSION 0x0001
#define MISSIONS_1_THR_3 0x0003
#define MISSIONS_4_THR_6 0x0004
#define MISSIONS_1_THR_6 0x0007

//
// CURRENT GAME VERSION DEFINE - Change this define according to the
// game release versions 1,1-3,4-6, or 1-6.

#define GAME_VERSION (MISSIONS_1_THR_6)
// #define GAME_VERSION (MISSIONS_1_THR_3)
// #define GAME_VERSION (SHAREWARE_VERSION)


#define TECH_SUPPORT_VERSION (0)
#define IN_DEVELOPMENT (0)

#define ERROR_LOG "ERROR.LOG" // Text filename for critical memory errors
#define DUAL_SWAP_FILES (0) // (GAME_VERSION != SHAREWARE_VERSION) // Support for Shadowed and NonShadowed page files

extern int8_t signonv1;
#define introscn signonv1

#ifndef SPEAR

#include "audio.h"
#include "gfxv.h"

#else

#include "GFXV_SOD.H"
#include "AUDIOSOD.H"
#include "MAPSSOD.H"

#endif

// -----------------


#define GREXT "VGA"

//
//      ID Engine
//      Types.h - Generic types, #defines, etc.
//      v1.0d1
//

struct Point {
    int16_t x;
    int16_t y;
}; // Point

struct Rect {
    Point ul;
    Point lr;
}; // Rect

#define nil (0l)


#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"

#include "jm_tp.h"
#include "jm_debug.h"

#include "movie.h"


extern void CalcMemFree();

//
// replacing refresh manager with custom routines
//

#define PORTTILESWIDE 20 // all drawing takes place inside a
#define PORTTILESHIGH 13 // non displayed port of this size

#define UPDATEWIDE PORTTILESWIDE
#define UPDATEHIGH PORTTILESHIGH

#define MAXTICS 10
#define DEMOTICS 4

#define UPDATETERMINATE 0x0301

extern uint16_t mapwidth, mapheight, tics, realtics;
extern bool compatability;

extern uint8_t* updateptr;
extern uint16_t uwidthtable[UPDATEHIGH];
extern uint16_t blockstarts[UPDATEWIDE * UPDATEHIGH];

extern uint8_t fontcolor, backcolor;

#define SETFONTCOLOR(f, b) fontcolor = static_cast<uint8_t>(f); backcolor = static_cast<uint8_t>(b);

#include "3d_menu.h"


#define CA_FarRead(h, d, s) IO_FarRead((int16_t)h, (uint8_t*)d, (int32_t)s)


// BBi
const int UPDATESIZE = UPDATEWIDE * UPDATEHIGH;

extern uint8_t update[UPDATESIZE];


extern uint8_t* vga_memory;
extern bstone::ClArgs g_args;
extern uint8_t update[UPDATESIZE];


void pre_quit();

void Quit();

template<typename... TArgs>
void Quit(
    const std::string& format,
    TArgs... args)
{
    ::pre_quit();

    if (!format.empty()) {
        bstone::Log::write_critical(format, args...);
    }

    ::exit(1);
}
// BBi


#endif // BSTONE_ID_HEADS_INCLUDED
