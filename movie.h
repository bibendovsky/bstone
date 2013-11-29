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


#ifndef _MOVIE_H_
#define _MOVIE_H_


#include "jm_vl.h"

//==========================================================================
//
//  UNIT:  MOVIE.H
//
//==========================================================================


struct anim_frame {
    Uint16 code;
    Sint32 block_num;
    Sint32 recsize;
}; // struct anim_frame


struct anim_chunk {
    Uint16 opt;
    Uint16 offset;
    Uint16 length;
}; // struct anim_chunk


//-------------------------------------------------------------------------
//   MovieStuff Anim Stucture...
//
//
//  fname 			-- File Name of the Anim to be played..
//	 rep				-- Number of repetitions to play the anim
//	 ticdelay		-- Tic wait between frames
//  maxmembuffer 	-- Maximum ammount to use as a ram buffer
//  start_line 	-- Starting line of screen to copy to other pages
//  end_line   	-- Ending line  "   "   "   "   "   "   "   "
//
struct MovieStuff_t {
    char FName[13];
    char rep;
    char ticdelay;

    Uint32 MaxMemBuffer;

    Sint16 start_line;
    Sint16 end_line;
    const void* palette;
}; // struct MovieStuff_t



//=========================================================================
//
//											EXTERNS
//
//=========================================================================

extern void* displaybuffer;
extern MovieStuff_t Movies[];

//===========================================================================
//
//								     Prototypes
//
//===========================================================================

void MOVIE_ShowFrame (char *inpic);
boolean MOVIE_Play(MovieStuff_t *MovieStuff);
void SetupMovie(MovieStuff_t *MovieStuff);
void ShutdownMovie(void);

#endif
