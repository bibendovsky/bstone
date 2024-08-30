/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_MOVIE_INCLUDED
#define BSTONE_MOVIE_INCLUDED


#include <cstdint>


// NOTE - This enum list is ORDERED!
enum MovieId
{
	intro,
	final,
	final_2, // AOG Episode 3 / 5
	final_3, // AOG Episode 6
}; // MovieId


bool movie_play(
	const MovieId movie_id,
	const std::uint8_t* const palette);


#endif // BSTONE_MOVIE_INCLUDED
