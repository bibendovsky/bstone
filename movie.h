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
