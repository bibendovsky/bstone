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


#ifndef BSTONE_MOVIE_INCLUDED
#define BSTONE_MOVIE_INCLUDED


#include "jm_vl.h"

// ==========================================================================
//
//  UNIT:  MOVIE.H
//
// ==========================================================================


struct anim_frame {
    uint16_t code;
    int32_t block_num;
    int32_t recsize;
}; // anim_frame


struct anim_chunk {
    uint16_t opt;
    uint16_t offset;
    uint16_t length;
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
struct MovieStuff_t {
    char FName[13];
    int8_t rep;
    int8_t ticdelay;

    uint32_t MaxMemBuffer;

    int16_t start_line;
    int16_t end_line;
    const void* palette;
}; // MovieStuff_t



// =========================================================================
//
// EXTERNS
//
// =========================================================================

extern void* displaybuffer;
extern MovieStuff_t Movies[];

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
