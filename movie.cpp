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


//===========================================================================
//
//
//
//
//===========================================================================


#include "3d_def.h"
#include "an_codes.h"


boolean IN_CheckAck();
void VH_UpdateScreen();


//===========================================================================
//
//
//
//
//===========================================================================


void VL_LatchToScreen(int source, int width, int height, int x, int y);


//#define  DRAW_TO_FRONT

//
// Various types for various purposes...
//

enum FADES {
    FADE_NONE,
    FADE_SET,
    FADE_IN,
    FADE_OUT,
    FADE_IN_FRAME,
    FADE_OUT_FRAME
}; // enum FADES


enum MOVIE_FLAGS {
    MV_NONE,
    MV_FILL,
    MV_SKIP,
    MV_READ
}; // enum MOVIE_FLAGS


//===========================================================================
//
//											VARIABLES
//
//===========================================================================


// Movie File variables

bstone::FileStream Movie_FHandle;

// Fade Variables

FADES fade_flags, fi_type, fo_type;
Uint8	fi_rate,fo_rate;

// MOVIE_GetFrame & MOVIE_LoadBuffer variables

char* MovieBuffer;					// Ptr to Allocated Memory for Buffer
Uint32 BufferLen;			// Len of MovieBuffer (Ammount of RAM allocated)
Uint32 PageLen;				// Len of data loaded into MovieBuffer
char * BufferPtr;				// Ptr to next frame in MovieBuffer
char * NextPtr;   				// Ptr Ofs to next frame after BufferOfs

boolean MorePagesAvail;				// More Pages avail on disk?

//

MOVIE_FLAGS  movie_flag;
boolean ExitMovie;
boolean EverFaded;
Sint32 seek_pos;
char movie_reps;
ControlInfo ci;
const void* movie_palette;


//
// MOVIE Definations for external movies
//
// NOTE: This list is ordered according to mv_???? enum list.
//


MovieStuff_t Movies[] =
{
	{{"IANIM."},1,3,0,0,200},				//mv_intro
	{{"EANIM."},1,30,0,0,200},				//mv_final
};



//===========================================================================
//
//										LOCAL PROTO TYPES
//
//===========================================================================

void JM_MemToScreen(void);
void JM_ClearVGAScreen(Uint8 fill);
void FlipPages(void);
boolean CheckFading(void);
boolean CheckPostFade(void);


//===========================================================================
//
//										   FUNCTIONS
//
//===========================================================================


//---------------------------------------------------------------------------
// SetupMovie() - Inits all the internal routines for the Movie Presenter
//
//
//
//---------------------------------------------------------------------------
void SetupMovie(MovieStuff_t *MovieStuff)
{
#ifdef DRAW_TO_FRONT
	bufferofs=displayofs;
#endif

   movie_reps = MovieStuff->rep;
	movie_flag = MV_FILL;
   LastScan = 0;										  
   PageLen = 0;
   MorePagesAvail = true;
	ExitMovie = false;
	EverFaded = false;
	IN_ClearKeysDown();

   movie_palette = MovieStuff->palette;
	JM_VGALinearFill(screenloc[0],3*80*208,0);

	VL_FillPalette (0,0,0);
   LastScan = 0;

   // Find out how much memory we have to work with..

    BufferLen = 65535;
   BufferLen -= 65535;						// HACK: Save some room for sounds - This is cludgey

   if (BufferLen < 64256)
   	BufferLen = 64256;

    MovieBuffer = new char[BufferLen];
}


//---------------------------------------------------------------------------
// void ShutdownMovie(void)
//---------------------------------------------------------------------------
void ShutdownMovie(void)
{
    delete [] MovieBuffer;
    MovieBuffer = NULL;

   Movie_FHandle.close();
}

//---------------------------------------------------------------------------
// void JM_DrawBlock()
//
// dest_offset = Correct offset value for memory location for Paged/Latch mem
//
// byte_offset = Offset for the image to be drawn - This address is NOT
//					  a calculated Paged/Latch value but a byte offset in
//					  conventional memory.
//
// source		= Source image of graphic to be blasted to latch memory.  This
//					  pic is NOT 'munged'
//
// length		= length of the source image in bytes
//---------------------------------------------------------------------------
void JM_DrawBlock(
    int dest_offset,
    int byte_offset,
    const char* source,
    int length)
{
    int x = byte_offset % k_ref_width;
    int y = byte_offset / k_ref_width;

    for (int i = 0; i < length; ++i) {
        VL_Plot(x, y, static_cast<Uint8>(source[i]));

        ++x;

        if (x == k_ref_width) {
            x = 0;
            ++y;
        }
    }
}



//---------------------------------------------------------------------------
// MOVIE_ShowFrame() - Shows an animation frame
//
// PARAMETERS: pointer to animpic
//---------------------------------------------------------------------------
void MOVIE_ShowFrame (char *inpic)
{
   anim_chunk *ah;

   if (inpic == NULL)
      return;

   for (;;)
   {
      ah = (anim_chunk *)inpic;

      if (ah->opt == 0)
			break;

      inpic += sizeof(anim_chunk);
		JM_DrawBlock(bufferofs, ah->offset, (char *)inpic, ah->length);
      inpic += ah->length;
   }
}



//---------------------------------------------------------------------------
// MOVIE_LoadBuffer() - Loads the RAM Buffer full of graphics...
//
// RETURNS:  true  	- MORE Pages avail on disk..
//				 false   - LAST Pages from disk..
//
// PageLen = Length of data loaded into buffer
//
//---------------------------------------------------------------------------
boolean MOVIE_LoadBuffer()
{
    anim_frame blk;
    long chunkstart;
    char* frame;
    Uint32 free_space;

    NextPtr = BufferPtr = frame = MovieBuffer;
    free_space = BufferLen;

    while (free_space) {
        chunkstart = static_cast<long>(Movie_FHandle.get_position());

        if (Movie_FHandle.read(
            &blk.code,
            sizeof(blk.code)) != sizeof(blk.code))
        {
            AN_ERROR(AN_BAD_ANIM_FILE);
        }

        if (Movie_FHandle.read(
            &blk.block_num,
            sizeof(blk.block_num)) != sizeof(blk.block_num))
        {
            AN_ERROR(AN_BAD_ANIM_FILE);
        }

        if (Movie_FHandle.read(
            &blk.recsize,
            sizeof(blk.recsize)) != sizeof(blk.recsize))
        {
            AN_ERROR(AN_BAD_ANIM_FILE);
        }

        if (blk.code == AN_END_OF_ANIM)
            return false;

        if (free_space >= (blk.recsize + sizeof(anim_frame))) {
            memcpy(frame, &blk, sizeof(anim_frame));

            free_space -= sizeof(anim_frame);
            frame += sizeof(anim_frame);
            PageLen += sizeof(anim_frame);

            if (Movie_FHandle.read(frame, blk.recsize) != blk.recsize)
                AN_ERROR(AN_BAD_ANIM_FILE);

            free_space -= blk.recsize;
            frame += blk.recsize;
            PageLen += blk.recsize;
        } else {
            Movie_FHandle.seek(chunkstart);
            free_space = 0;
        }
    }

    return true;
}


//---------------------------------------------------------------------------
// MOVIE_GetFrame() - Returns pointer to next Block/Screen of animation
//
// PURPOSE: This function "Buffers" the movie presentation from allocated
//				ram.  It loads and buffers incomming frames of animation..
//
// RETURNS:  0 - Ok
//				 1 - End Of File
//---------------------------------------------------------------------------
Sint16 MOVIE_GetFrame()
{
   anim_frame blk;

	if (PageLen == 0)
   {
    	if (MorePagesAvail)
	      MorePagesAvail = MOVIE_LoadBuffer();
      else
      	return(1);
	}

   BufferPtr = NextPtr;
	memcpy(&blk, BufferPtr, sizeof(anim_frame));
   PageLen-=sizeof(anim_frame);
   PageLen-=blk.recsize;
   NextPtr = BufferPtr+sizeof(anim_frame)+blk.recsize;
	return(0);
}



//---------------------------------------------------------------------------
// MOVIE_HandlePage() - This handles the current page of data from the
//								ram buffer...
//
// PURPOSE: Process current Page of anim.
//
//
// RETURNS:
//
//---------------------------------------------------------------------------
void MOVIE_HandlePage(MovieStuff_t *MovieStuff)
{
	anim_frame blk;
	char *frame;
   Uint16 wait_time;

	memcpy(&blk,BufferPtr,sizeof(anim_frame));
	BufferPtr+=sizeof(anim_frame);
   frame = BufferPtr;

	IN_ReadControl(0,&ci);

   switch (blk.code)
   {

      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_SOUND:				// Sound Chunk
		{
      	Uint16 sound_chunk;
         sound_chunk = *(Uint16 *)frame;

        ::sd_play_player_sound(
            static_cast<soundnames>(sound_chunk),
            bstone::AC_ITEM);

         BufferPtr+=blk.recsize;
      }
      break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

#if 0
		case MV_CNVT_CODE('P','M'):				// Play Music
		{
      	unsigned song_chunk;
         song_chunk = *(unsigned *)frame;
         SD_MusicOff();

			if (!audiosegs[STARTMUSIC+musicchunk])
			{
//				MM_BombOnError(false);
				CA_CacheAudioChunk(STARTMUSIC + musicchunk);
//				MM_BombOnError(true);
			}

			if (mmerror)
				mmerror = false;
			else
			{
				MM_SetLock(&((memptr)audiosegs[STARTMUSIC + musicchunk]),true);
				SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC + musicchunk]);
			}

         BufferPtr+=blk.recsize;
      }
      break;
#endif


      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_FADE_IN_FRAME:				// Fade In Page
        	VL_FadeIn(0,255,(const Uint8*)movie_palette,30);
			fade_flags = FADE_NONE;
         EverFaded = true;
			screenfaded = false;
      break;



      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_FADE_OUT_FRAME:				// Fade Out Page
			VW_FadeOut();
			screenfaded = true;
			fade_flags = FADE_NONE;
      break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_PAUSE:				// Pause
		{
      	Uint16 vbls;
         vbls = *(Uint16 *)frame;
			IN_UserInput(vbls);
         BufferPtr+=blk.recsize;

         // BBi
         // FIXME Clear entire input state.
         LastScan = 0;
         ci.button0 = 0;
         ci.button1 = 0;
         // BBi
      }
      break;



      //-------------------------------------------
      //
      //
      //-------------------------------------------

   	case AN_PAGE:				// Graphics Chunk
#if 1
         if (movie_flag == MV_FILL)
         {
            // First page comming in.. Fill screen with fill color...
            //
//            movie_flag = MV_READ;	// Set READ flag to skip the first frame on an anim repeat
            movie_flag = MV_NONE;	// Set READ flag to skip the first frame on an anim repeat
			  	JM_VGALinearFill(screenloc[0],3*80*208,*frame);
            frame++;
         }
         else
#endif 
				VL_LatchToScreen(displayofs+ylookup[MovieStuff->start_line], 320>>2, MovieStuff->end_line-MovieStuff->start_line, 0, MovieStuff->start_line);

         MOVIE_ShowFrame(frame);

#if 0  
         if (movie_flag == MV_READ)
         {
         	seek_pos = tell(Movie_FHandle);
            movie_flag = MV_NONE;
         }
#endif 
   		FlipPages();

         if (TimeCount < static_cast<Uint32>(MovieStuff->ticdelay))
         {
	         wait_time = static_cast<Uint16>(MovieStuff->ticdelay - TimeCount);
				VL_WaitVBL(wait_time);
         }
         else
				VL_WaitVBL(1);

			TimeCount = 0;

			if ((!screenfaded) && (ci.button0 || ci.button1 || LastScan))
			{
				ExitMovie = true;
				if (EverFaded)					// This needs to be a passed flag...
				{
					VW_FadeOut();
					screenfaded = true;
				}
			}
		break;


#if 0
      //-------------------------------------------
      //
      //
      //-------------------------------------------

		case AN_PRELOAD_BEGIN:			// These are NOT handled YET!
		case AN_PRELOAD_END:
		break;

#endif
      //-------------------------------------------
      //
      //
      //-------------------------------------------

		case AN_END_OF_ANIM:
			ExitMovie = true;
		break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

		default:
			AN_ERROR(HANDLEPAGE_BAD_CODE);
		break;
   }
}


//---------------------------------------------------------------------------
// MOVIE_Play() - Playes an Animation
//
// RETURNS: true  - Movie File was found and "played"
//				false - Movie file was NOT found!
//---------------------------------------------------------------------------
boolean MOVIE_Play(MovieStuff_t *MovieStuff)
{
	// Init our Movie Stuff...
   //

   SetupMovie(MovieStuff);

   // Start the anim process
   //

   Movie_FHandle.open(MovieStuff->FName);
   if (!Movie_FHandle.is_open())
     	return(false);

   while (movie_reps && (!ExitMovie))
	{
#if 0 	
      if (movie_flag == MV_SKIP)
	   	if (lseek(Movie_FHandle, seek_pos, SEEK_SET) == -1)
         	return(false);
#endif
	   for (;!ExitMovie;)
   	{
      	if (MOVIE_GetFrame())
         	break;

         MOVIE_HandlePage(MovieStuff);
      }

      movie_reps--;
      movie_flag = MV_SKIP;
   }

   ShutdownMovie();

   return(true);
}




//--------------------------------------------------------------------------
// FlipPages()
//---------------------------------------------------------------------------
void FlipPages(void)
{
    displayofs = bufferofs;

    VL_RefreshScreen();

    bufferofs += SCREENSIZE;

    if (bufferofs > static_cast<int>(PAGE3START))
        bufferofs = PAGE1START;
}
