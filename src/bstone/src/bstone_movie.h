/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_MOVIE_INCLUDED
#define BSTONE_MOVIE_INCLUDED

#include "bstone_logger.h"
#include <cstdint>

namespace bstone {

// NOTE - This enum list is ORDERED!
enum class MovieId
{
	intro,
	final,
	final_2, // AOG Episode 3 / 5
	final_3, // AOG Episode 6
};

bool movie_play(MovieId movie_id, const std::uint8_t* palette, Logger& logger);

} // namespace bstone

#endif // BSTONE_MOVIE_INCLUDED
