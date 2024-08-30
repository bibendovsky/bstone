/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_ID_HEADS_INCLUDED
#define BSTONE_ID_HEADS_INCLUDED


#include <array>
#include <cstdint>
#include <string>


#include "bstone_cl.h"


struct QuitException{};


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

void SETFONTCOLOR(
	const int foreground_color,
	const int background_color);

const int UPDATESIZE = UPDATEWIDE * UPDATEHIGH;
using Update = std::array<std::uint8_t, UPDATESIZE>;
extern Update update;


extern bstone::Cl g_args;


[[noreturn]]
void Quit();
// BBi


#endif // BSTONE_ID_HEADS_INCLUDED
