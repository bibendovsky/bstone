#ifndef _MOVIE_H_
#define _MOVIE_H_


#include "JM_VL.H"

//==========================================================================
//
//  UNIT:  MOVIE.H
//
//==========================================================================


typedef struct
{
	unsigned code;
   long block_num;
   long recsize;
}anim_frame;


typedef struct
{
	unsigned short opt;
   unsigned short offset;
   unsigned short length;
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

   unsigned long MaxMemBuffer;

	short start_line;
	short end_line;
   memptr palette;

} MovieStuff_t;



//=========================================================================
//
//											EXTERNS
//
//=========================================================================

extern memptr displaybuffer;
extern MovieStuff_t Movies[];

//===========================================================================
//
//								     Prototypes
//
//===========================================================================

void MOVIE_ShowFrame (char huge *inpic);
boolean MOVIE_Play(MovieStuff_t *MovieStuff);
void SetupMovie(MovieStuff_t *MovieStuff);
void ShutdownMovie(void);

#endif