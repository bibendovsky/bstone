#ifndef _MOVIE_H_
#define _MOVIE_H_


#include "jm_vl.h"

//==========================================================================
//
//  UNIT:  MOVIE.H
//
//==========================================================================


typedef struct
{
	Uint16 code;
   Sint32 block_num;
   Sint32 recsize;
}anim_frame;


typedef struct
{
	Uint16 opt;
   Uint16 offset;
   Uint16 length;
} anim_chunk;


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
typedef struct
{
	char FName[13];
	char rep;
	char ticdelay;

   Uint32 MaxMemBuffer;

	Sint16 start_line;
	Sint16 end_line;
    const void* palette;

} MovieStuff_t;



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
