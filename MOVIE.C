//===========================================================================
//
//
//
//
//===========================================================================

#include <io.h>
#include <alloc.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <FCNTL.H>

#include "3d_def.h"
#include "jm_io.h"
#include "an_codes.h"

//===========================================================================
//
//
//
//
//===========================================================================


//#define  DRAW_TO_FRONT

//
// Various types for various purposes...
//

typedef enum
{
  	FADE_NONE,
   FADE_SET,
   FADE_IN,
   FADE_OUT,
   FADE_IN_FRAME,
   FADE_OUT_FRAME,
} FADES;


typedef enum
{
   MV_NONE,
	MV_FILL,
   MV_SKIP,
   MV_READ,

} MOVIE_FLAGS;


//===========================================================================
//
//											VARIABLES
//
//===========================================================================


// Movie File variables

int Movie_FHandle;

// Fade Variables

FADES fade_flags, fi_type, fo_type;
byte	fi_rate,fo_rate;

// MOVIE_GetFrame & MOVIE_LoadBuffer variables

memptr MovieBuffer;					// Ptr to Allocated Memory for Buffer
unsigned long BufferLen;			// Len of MovieBuffer (Ammount of RAM allocated)
unsigned long PageLen;				// Len of data loaded into MovieBuffer
char huge * BufferPtr;				// Ptr to next frame in MovieBuffer
char huge * NextPtr;   				// Ptr Ofs to next frame after BufferOfs

boolean MorePagesAvail;				// More Pages avail on disk?

//

MOVIE_FLAGS  movie_flag;
boolean ExitMovie;
boolean EverFaded;
long seek_pos;
char movie_reps;
ControlInfo ci;
memptr movie_palette;


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
void JM_ClearVGAScreen(byte fill);
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

	BufferLen = MM_LargestAvail();
   BufferLen -= 65535;						// HACK: Save some room for sounds - This is cludgey

   if (BufferLen < 64256)
   	BufferLen = 64256;

	MM_GetPtr(&MovieBuffer,BufferLen);
	PM_CheckMainMem();
}


//---------------------------------------------------------------------------
// void ShutdownMovie(void)
//---------------------------------------------------------------------------
void ShutdownMovie(void)
{
	MM_FreePtr(&MovieBuffer);
   close (Movie_FHandle);
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
void JM_DrawBlock(unsigned dest_offset,unsigned byte_offset,char far *source,unsigned length)
{
	byte numplanes;
   byte mask,plane;
	char huge *dest_ptr;
	char huge *source_ptr;
   char huge *dest;
   char huge *end_ptr;
   unsigned count,total_len;


   end_ptr = source+length;

   //
   // Byte offset to determine our starting page to write to...
   //

   mask = 1<<(byte_offset & 3);

   //
   // Check to see if we are writting more than 4 bytes (to loop pages...)
   //

   if (length >= 4)
   	numplanes = 4;
   else
   	numplanes = length;

   //
   // Compute our DEST memory location
   //

   dest = MK_FP(0xA000,dest_offset+(byte_offset>>2));

   //
   // Move that memory.
   //

	for (plane = 0; plane<numplanes; plane++)
	{
   	dest_ptr = dest;
	   source_ptr = source+plane;

		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
      {
			mask = 1;
         dest++;
      }

		for (count=0;count<length,source_ptr < end_ptr;count+=4,dest_ptr++,source_ptr+=4)
      	*dest_ptr = *source_ptr;
	}
}



//---------------------------------------------------------------------------
// MOVIE_ShowFrame() - Shows an animation frame
//
// PARAMETERS: pointer to animpic
//---------------------------------------------------------------------------
void MOVIE_ShowFrame (char huge *inpic)
{
   anim_chunk huge *ah;

   if (inpic == NULL)
      return;

   for (;;)
   {
      ah = (anim_chunk huge *)inpic;

      if (ah->opt == 0)
			break;

      inpic += sizeof(anim_chunk);
		JM_DrawBlock(bufferofs, ah->offset, (char far *)inpic, ah->length);
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
	char huge *frame;
   unsigned long free_space;

   NextPtr = BufferPtr = frame = MK_FP(MovieBuffer,0);
   free_space = BufferLen;

	while (free_space)
   {
   	chunkstart = tell(Movie_FHandle);

	   if (!IO_FarRead(Movie_FHandle, (byte far *)&blk, sizeof(anim_frame)))
			AN_ERROR(AN_BAD_ANIM_FILE);

      if (blk.code == AN_END_OF_ANIM)
      	return(false);

		if (free_space>=(blk.recsize+sizeof(anim_frame)))
      {
			_fmemcpy(frame, (byte far *)&blk, sizeof(anim_frame));

      	free_space -= sizeof(anim_frame);
   	   frame += sizeof(anim_frame);
         PageLen += sizeof(anim_frame);

		   if (!IO_FarRead(Movie_FHandle, (byte far *)frame, blk.recsize))
				AN_ERROR(AN_BAD_ANIM_FILE);

         free_space -= blk.recsize;
         frame += blk.recsize;
         PageLen += blk.recsize;
      }
      else
      {
	      lseek(Movie_FHandle,chunkstart,SEEK_SET);
         free_space = 0;
      }
   }

   return(true);
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
short MOVIE_GetFrame()
{
	unsigned ReturnVal;
   anim_frame blk;

	if (PageLen == 0)
   {
    	if (MorePagesAvail)
	      MorePagesAvail = MOVIE_LoadBuffer(Movie_FHandle);
      else
      	return(1);
	}

   BufferPtr = NextPtr;
	_fmemcpy(&blk, BufferPtr, sizeof(anim_frame));
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
	char huge *frame;
   unsigned wait_time;

	_fmemcpy(&blk,BufferPtr,sizeof(anim_frame));
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
      	unsigned sound_chunk;
         sound_chunk = *(unsigned far *)frame;
      	SD_PlaySound(sound_chunk);
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
         song_chunk = *(unsigned far *)frame;
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
				SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + musicchunk]);
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
        	VL_FadeIn(0,255,MK_FP(movie_palette,0),30);
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
      	unsigned vbls;
         vbls = *(unsigned far *)frame;
			IN_UserInput(vbls);
         BufferPtr+=blk.recsize;
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

         if (TimeCount < MovieStuff->ticdelay)
         {
	         wait_time = MovieStuff->ticdelay - TimeCount;
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

   if ((Movie_FHandle = open(MovieStuff->FName, O_RDONLY|O_BINARY, S_IREAD)) == -1)	  
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
      	if (MOVIE_GetFrame(Movie_FHandle))
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

#ifndef DRAW_TO_FRONT

	displayofs = bufferofs;

	asm	cli
	asm	mov	cx,[displayofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
#if 0
	asm	dec	dx
	asm	mov	al,0dh		// start address low register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,cl
	asm	out	dx,al		// set the low byte
#endif
	asm	sti

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;

#endif

}
