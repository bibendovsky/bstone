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


#ifndef BSTONE_MOVIE_INCLUDED
#define BSTONE_MOVIE_INCLUDED


#include <vector>
#include "jm_vl.h"


struct anim_frame
{
	std::uint16_t code;
	std::int32_t block_num;
	std::int32_t recsize;
}; // anim_frame


struct anim_chunk
{
	std::uint16_t opt;
	std::uint16_t offset;
	std::uint16_t length;
}; // anim_chunk


// -------------------------------------------------------------------------
//   MovieStuff Anim Stucture...
//
//
//  fname -- File Name of the Anim to be played..
//  rep -- Number of repetitions to play the anim
//  ticdelay -- Tic wait between frames
//  maxmembuffer -- Maximum ammount to use as a ram buffer
//  start_line -- Starting line of screen to copy to other pages
//  end_line -- Ending line  "   "   "   "   "   "   "   "
//
struct MovieStuff_t
{
	AssetsCRefString file_name;
	std::int8_t rep;
	std::int8_t ticdelay;

	std::uint32_t max_mem_buffer;

	std::int16_t start_line;
	std::int16_t end_line;
	const void* palette;
}; // MovieStuff_t



// =========================================================================
//
// EXTERNS
//
// =========================================================================

using Movies = std::vector<MovieStuff_t>;

extern void* displaybuffer;
extern Movies movies;

// ===========================================================================
//
// Prototypes
//
// ===========================================================================

void MOVIE_ShowFrame(
	char* inpic);
bool MOVIE_Play(
	MovieStuff_t* MovieStuff);
void SetupMovie(
	MovieStuff_t* MovieStuff);
void ShutdownMovie();


#endif // BSTONE_MOVIE_INCLUDED
