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


#include <cstdint>
#include <string>
#include "bstone_cl_args.h"
#include "bstone_log.h"


//
// replacing refresh manager with custom routines
//

#define PORTTILESWIDE (20) // all drawing takes place inside a
#define PORTTILESHIGH (13) // non displayed port of this size

#define UPDATEWIDE (PORTTILESWIDE)
#define UPDATEHIGH (PORTTILESHIGH)

#define TickBase (70) // 70Hz per tick - used as a base for timer 0
#define MAXTICS (10)

extern std::uint16_t mapwidth;
extern std::uint16_t mapheight;
extern std::uint16_t tics;
extern std::uint16_t realtics;

extern std::uint8_t* updateptr;
extern std::uint16_t uwidthtable[UPDATEHIGH];
extern std::uint16_t blockstarts[UPDATEWIDE * UPDATEHIGH];

extern std::uint8_t fontcolor;
extern std::uint8_t backcolor;

inline void SETFONTCOLOR(
	int foreground_color,
	int background_color)
{
	::fontcolor = static_cast<std::uint8_t>(foreground_color);
	::backcolor = static_cast<std::uint8_t>(background_color);
}


const int UPDATESIZE = UPDATEWIDE * UPDATEHIGH;

extern std::uint8_t update[UPDATESIZE];


extern std::uint8_t* vga_memory;
extern bstone::ClArgs g_args;
extern std::uint8_t update[UPDATESIZE];


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

std::uint32_t sys_get_timer_ticks();
// BBi


#endif // BSTONE_ID_HEADS_INCLUDED
