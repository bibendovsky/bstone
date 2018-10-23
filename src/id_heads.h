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


#ifndef BSTONE_ID_HEADS_INCLUDED
#define BSTONE_ID_HEADS_INCLUDED


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
#include "bstone_crc32.h"
#include "bstone_endian.h"
#include "bstone_file_stream.h"
#include "bstone_format_string.h"
#include "bstone_log.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_memory_stream.h"
#include "bstone_scope_guard.h"
#include "bstone_string_helper.h"
#include "bstone_cl_args.h"

#include "audio.h"
#include "gfxv.h"
#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "jm_tp.h"
#include "movie.h"


//
// replacing refresh manager with custom routines
//

#define PORTTILESWIDE (20) // all drawing takes place inside a
#define PORTTILESHIGH (13) // non displayed port of this size

#define UPDATEWIDE (PORTTILESWIDE)
#define UPDATEHIGH (PORTTILESHIGH)

#define TickBase (70) // 70Hz per tick - used as a base for timer 0
#define MAXTICS (10)

extern uint16_t mapwidth;
extern uint16_t mapheight;
extern uint16_t tics;
extern uint16_t realtics;

extern uint8_t* updateptr;
extern uint16_t uwidthtable[UPDATEHIGH];
extern uint16_t blockstarts[UPDATEWIDE * UPDATEHIGH];

extern uint8_t fontcolor;
extern uint8_t backcolor;

inline void SETFONTCOLOR(
    int foreground_color,
    int background_color)
{
    ::fontcolor = static_cast<uint8_t>(foreground_color);
    ::backcolor = static_cast<uint8_t>(background_color);
}


#include "3d_menu.h"


// BBi
using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;


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

	if (!format.empty())
	{
		bstone::Log::write_critical(format, std::forward<TArgs>(args)...);
	}

	std::exit(1);
}

uint32_t sys_get_timer_ticks();
// BBi


#endif // BSTONE_ID_HEADS_INCLUDED
