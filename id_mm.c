// NEWMM.C

/*
=============================================================================

		   ID software memory manager
		   --------------------------

Primary coder: John Carmack

RELIES ON
---------
Quit (char *error) function


WORK TO DO
----------
MM_SizePtr to change the size of a given pointer

Multiple purge levels utilized

EMS / XMS unmanaged routines

=============================================================================
*/

#include "ID_HEADS.H"
#include <STDARG.H>
#include <dos.h>
#pragma hdrstop

#pragma warn -pro
#pragma warn -use

/*
=============================================================================

							LOCAL INFO

=============================================================================
*/

//#define LOCKBIT			0x80	// if set in attributes, block cannot be moved
#define PURGEBITS			3		// 0-3 level, 0= unpurgable, 3= purge first
#define PURGEMASK			0xfffc
#define BASEATTRIBUTES	0		// unlocked, non purgable

#define MAXUMBS			10


//#define GETNEWBLOCK {if(!(mmnew=mmfree))Quit("MM_GETNEWBLOCK: No free blocks!")\
//	;mmfree=mmfree->next;}

//#define GETNEWBLOCK {if(!mmfree)MML_ClearBlock();mmnew=mmfree;mmfree=mmfree->next;}

#if IN_DEVELOPMENT
	#define FREEBLOCK(x) {*x->useptr=NULL;x->next=mmfree;mmfree=x;blockcount--;}
#else
	#define FREEBLOCK(x) {*x->useptr=NULL;x->next=mmfree;mmfree=x;}
#endif

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//unsigned __SEGS_AVAILABLE__;

#if IN_DEVELOPMENT
unsigned long __PUR_MEM_AVAIL__;
unsigned long __FREE_MEM_AVAIL__;
#endif

int errorfile=-1;						// jdebug

mminfotype	mminfo;
memptr		bufferseg;
boolean		mmerror;
#if IN_DEVELOPMENT
boolean 		clearblock_error=false;			// mdebug
#endif

void		(* beforesort) (void);
void		(* aftersort) (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

boolean		mmstarted;

void far	*farheap;
void		*nearheap;

mmblocktype	far mmblocks[MAXBLOCKS]
			,far *mmhead,far *mmfree,far *mmrover,far *mmnew;

boolean		bombonerror;

//unsigned	totalEMSpages,freeEMSpages,EMSpageframe,EMSpagesmapped,EMShandle;

void		(* XMSaddr) (void);		// far pointer to XMS driver

#if IN_DEVELOPMENT
unsigned blockcount = 0;
#endif


//unsigned	numUMBs,UMBbase[MAXUMBS];		// jdebug

//==========================================================================

//
// local prototypes
//

boolean		MML_CheckForEMS (void);
void 		MML_ShutdownEMS (void);
void 		MM_MapEMS (void);
boolean 	MML_CheckForXMS (void);
void 		MML_ShutdownXMS (void);
void		MML_UseSpace (unsigned segstart, unsigned seglength);
void 		MML_ClearBlock (void);

//==========================================================================

void PrintAllocated(long amount);		// mdebug

/*
======================
=
= MML_CheckForXMS
=
= Check for XMM driver
=
=======================
*/

#if 0	

boolean MML_CheckForXMS (void)
{
	numUMBs = 0;

asm {
	mov	ax,0x4300
	int	0x2f				// query status of installed diver
	cmp	al,0x80
	je	good
	}
	return false;
good:
	return true;
}

#endif 




#if 0	
/*
======================
=
= MML_SetupXMS
=
= Try to allocate all upper memory block
=
=======================
*/

void MML_SetupXMS (void)
{
	unsigned	base,size;

asm	{
	mov	ax,0x4310
	int	0x2f
	mov	[WORD PTR XMSaddr],bx
	mov	[WORD PTR XMSaddr+2],es		// function pointer to XMS driver
	}

getmemory:
asm	{
	mov	ah,XMS_ALLOCUMB
	mov	dx,0xffff					// try for largest block possible
	call	[DWORD PTR XMSaddr]
	or	ax,ax
	jnz	gotone

	cmp	bl,0xb0						// error: smaller UMB is available
	jne	done;

	mov	ah,XMS_ALLOCUMB
	call	[DWORD PTR XMSaddr]		// DX holds largest available UMB
	or	ax,ax
	jz	done						// another error...
	}

gotone:
asm	{
	mov	[base],bx
	mov	[size],dx
	}
	MML_UseSpace (base,size);
	mminfo.XMSmem += size*16;
	UMBbase[numUMBs] = base;
	numUMBs++;
	if (numUMBs < MAXUMBS)
		goto getmemory;

done:;
}


#endif


#if 0

/*
======================
=
= MML_ShutdownXMS
=
======================
*/

void MML_ShutdownXMS (void)
{
	int	i;
	unsigned	base;

	for (i=0;i<numUMBs;i++)
	{
		base = UMBbase[i];

asm	mov	ah,XMS_FREEUMB
asm	mov	dx,[base]
asm	call	[DWORD PTR XMSaddr]
	}
}

#endif


//==========================================================================

/*
======================
=
= MML_UseSpace
=
= Marks a range of paragraphs as usable by the memory manager
= This is used to mark space for the near heap, far heap, ems page frame,
= and upper memory blocks
=
======================
*/

void MML_UseSpace (unsigned segstart, unsigned seglength)
{
	mmblocktype far *scan,far *last;
	unsigned	oldend;
	long		extra;

	scan = last = mmhead;
	mmrover = mmhead;		// reset rover to start of memory

//
// search for the block that contains the range of segments
//
	while (scan->start+scan->length < segstart)
	{
		last = scan;
		scan = scan->next;
	}

//
// take the given range out of the block
//
	oldend = scan->start + scan->length;
	extra = oldend - (segstart+seglength);
	if (extra < 0)
		MM_ERROR(MML_USESPACE_TWO_BLOCKS);

	if (segstart == scan->start)
	{
		last->next = scan->next;			// unlink block
		FREEBLOCK(scan);
		scan = last;
	}
	else
		scan->length = segstart-scan->start;	// shorten block

	if (extra > 0)
	{
		GETNEWBLOCK;
		mmnew->useptr = NULL;

		mmnew->next = scan->next;
		scan->next = mmnew;
		mmnew->start = segstart+seglength;
		mmnew->length = extra;
		mmnew->attributes = LOCKBIT;
	}
}

//==========================================================================

/*
====================
=
= MML_ClearBlock
=
= We are out of blocks, so free a purgable block
=
====================
*/

//
// mdebug
//

#if IN_DEVELOPMENT

char far cb_text[]="\n\n"
						 " YOU'VE JUST FOUND THE 90:02 ERROR!\n"
						 "\n"
						 " TAKE NOTE OF WHAT -JUST- HAPPEND BEFORE\n"
						 " THE ERROR AND -WHERE- YOU ARE.\n"
						 "\n"
						 " REPORT ALL INFO TO JAM PRODUCTIONS\n"
						 " VIA THE BETA CONFERENCE. THANKS!        PRESS A KEY\n\n";
#endif

//
// jdebug
//

#if IN_DEVELOPMENT

//-----------------------------------------------------------------------
// OpenErrorFile() - Opens a TEXT error file
//
// RETURNS : TRUE  - Able to open file.
//				 FALSE - Unable to open file.
//-----------------------------------------------------------------------
boolean OpenErrorFile(void)
{
	if (errorfile == -1)
		errorfile = open(ERROR_LOG, O_APPEND | O_CREAT | O_TEXT,S_IWRITE);

   return(errorfile != -1);
}

//-----------------------------------------------------------------------
// CloseErrorFile()
//-----------------------------------------------------------------------
void CloseErrorFile(void)
{
	if (errorfile != -1)
   {
		close(errorfile);
   	errorfile = -1;
   }
}

//-----------------------------------------------------------------------
// ErrorOut - Outputs a text message to the ErrorLog and Echos to mono monitor
//-----------------------------------------------------------------------
void ErrorOut(char *msg, ...)
{
	char buffer[100],*ptr;
//	va_list ap;

	va_list(ap);

	va_start(ap,msg);

	vsprintf(buffer,msg,ap);

   OpenErrorFile();

	if (errorfile != -1)
		write(errorfile,buffer,strlen(buffer));

	CloseErrorFile();

	fmprint(buffer);
}

#endif

void MML_ClearBlock (void)
{
	mmblocktype far *scan,far *last;

#if IN_DEVELOPMENT
	mprintf("\nMML_ClearBlock()\n");	// jdebug

// jdebug begin
#if !BETA_TEST
   while(Keyboard[sc_L]);
	while(!Keyboard[sc_L]);
#endif
// jdebug end
#endif

	scan = mmhead->next;

#if IN_DEVELOPMENT
	ErrorOut("mmhead->next %X:%X\n",FP_SEG(scan),FP_OFF(scan));			// jdebug
#endif

	while (scan)
	{
#if IN_DEVELOPMENT
		ErrorOut("scan %X:%X\n",FP_SEG(scan),FP_OFF(scan));			// jdebug
#endif

		if (!(scan->attributes&LOCKBIT) && (scan->attributes&PURGEBITS) )
		{
#if IN_DEVELOPMENT
			mprintf(" AVAIL BLOCK FOUND!\n");		// jdebug
#endif
			MM_FreePtr(scan->useptr);
			return;
		}
// jdebug begin
#if IN_DEVELOPMENT
		else
		{
			if  (scan->attributes & LOCKBIT)
				ErrorOut(" LOCKED\n");

			switch (scan->attributes&PURGEBITS)
			{
				case 0:
					ErrorOut(" UNPURGEABLE\n");
				break;

				default:
					ErrorOut(" PURGEABLE\n");
				break;
			}
		}
#endif
// jdebug end

		scan = scan->next;
	}

#if IN_DEVELOPMENT

//
// mdebug
//

	if (screenfaded)
		VW_FadeIn();
	bufferofs=displayofs;
	CenterWindow(32,14);
	fontnumber = 2;
	fontcolor = 0x9A;
	US_Print(cb_text);
//	clearblock_error=true;
	VW_UpdateScreen();
	LastScan=0;
	while (!LastScan);
#else
	MM_ERROR(MML_CLEARBLOCK_NO_PURGE_BLKS);		// mdebug -- uncomment me!
#endif
}

//==========================================================================

/*
===================
=
= MM_Startup
=
= Grabs all space from turbo with malloc/farmalloc
= Allocates bufferseg misc buffer
=
===================
*/


//==========================================================================

/*
====================
=
= MM_Shutdown
=
= Frees all conventional, EMS, and XMS allocated
=
====================
*/

void MM_Shutdown (void)
{
  if (!mmstarted)
	return;

  farfree (farheap);
  free (nearheap);
//  MML_ShutdownXMS ();
}

//==========================================================================

/*
====================
=
= MM_GetPtr
=
= Allocates an unlocked, unpurgable block
=
====================
*/

#if IN_DEVELOPMENT

char far gp_text[]=" WRITE DOWN THE FOLLOWING INFO, TOO:\n"
						 "\n"
						 " MM_GETPTR SIZE: ";

char far *gp_fartext=NULL;			// mdebug

char far *jr_fartext = NULL;				// jim/mdebug

#endif

void MM_GetPtr (memptr *baseptr,unsigned long size)
{
	mmblocktype far *scan,far *lastscan,far *endscan
				,far *purge,far *next;
	int			search;
	unsigned	needed,startseg;

	needed = (size+15)/16;		// convert size from bytes to paragraphs

	GETNEWBLOCK;				// fill in start and next after a spot is found

#if IN_DEVELOPMENT

//
// mdebug
//

	if (clearblock_error)
	{
		US_Print(gp_text);
		US_PrintUnsigned(size);
		US_Print("\n\n");
		if (gp_fartext)
			US_Print(gp_fartext);
		if (jr_fartext)
			US_Print(jr_fartext);
		LastScan=0;
		while (!LastScan);
		clearblock_error=false;
		gp_fartext=false;
		Quit(0);
	}

#endif

	mmnew->length = needed;
	mmnew->useptr = baseptr;
	mmnew->attributes = BASEATTRIBUTES;

	for (search = 0; search<3; search++)
	{
	//
	// first search:	try to allocate right after the rover, then on up
	// second search: 	search from the head pointer up to the rover
	// third search:	compress memory, then scan from start
		if (search == 1 && mmrover == mmhead)
			search++;

		switch (search)
		{
		case 0:
			lastscan = mmrover;
			scan = mmrover->next;
			endscan = NULL;
			break;
		case 1:
			lastscan = mmhead;
			scan = mmhead->next;
			endscan = mmrover;
			break;
		case 2:
			MM_SortMem ();
			lastscan = mmhead;
			scan = mmhead->next;
			endscan = NULL;
			break;
		}

		startseg = lastscan->start + lastscan->length;

		while (scan != endscan)
		{
			if (scan->start - startseg >= needed)
			{
			//
			// got enough space between the end of lastscan and
			// the start of scan, so throw out anything in the middle
			// and allocate the new block
			//
				purge = lastscan->next;
				lastscan->next = mmnew;
				mmnew->start = *(unsigned *)baseptr = startseg;
				mmnew->next = scan;
				while ( purge != scan)
				{	// free the purgable block
					next = purge->next;
					FREEBLOCK(purge);
					purge = next;		// purge another if not at scan
				}
				mmrover = mmnew;
//				__SEGS_AVAILABLE__ -= needed;
#if IN_DEVELOPMENT && 0
				PrintAllocated(needed);		// mdebug
#endif
				return;	// good allocation!
			}

			//
			// if this block is purge level zero or locked, skip past it
			//
			if ( (scan->attributes & LOCKBIT)
				|| !(scan->attributes & PURGEBITS) )
			{
				lastscan = scan;
				startseg = lastscan->start + lastscan->length;
			}


			scan=scan->next;		// look at next line
		}
	}

	FREEBLOCK(mmnew);

	if (bombonerror)
	{

extern char configname[];

//	mprintf("\n\nOUT OF MEMORY:\n");
//	mprintf("blocks needed: %d   (%ld)\n",needed,needed<<4);

		unlink(configname);
		MM_ERROR(MM_GETPTR_OUT_OF_MEMORY);
	}
	else
		mmerror = true;
}

//==========================================================================

/*
====================
=
= MM_FreePtr
=
= Allocates an unlocked, unpurgable block
=
====================
*/

void MM_FreePtr (memptr *baseptr)
{
	long value;		// mdebug

	mmblocktype far *scan,far *last;

	last = mmhead;
	scan = last->next;

	if (baseptr == mmrover->useptr)	// removed the last allocated block
		mmrover = mmhead;

	while (scan->useptr != baseptr && scan)
	{
		last = scan;
		scan = scan->next;
	}

	if (!scan)
		MM_ERROR(MM_FREEPTR_BLOCK_NOT_FOUND);

#if IN_DEVELOPMENT && 0
	value = scan->length;		// mdebug
	PrintAllocated(-value);		// mdebug
#endif

	last->next = scan->next;
//	__SEGS_AVAILABLE__ += scan->length;

	FREEBLOCK(scan);
}
//==========================================================================

/*
=====================
=
= MM_SetPurge
=
= Sets the purge level for a block (locked blocks cannot be made purgable)
=
=====================
*/

void MM_SetPurge (memptr *baseptr, int purge)
{
	mmblocktype far *start;

	start = mmrover;

	do
	{
		if (mmrover->useptr == baseptr)
			break;

		mmrover = mmrover->next;

		if (!mmrover)
			mmrover = mmhead;
		else if (mmrover == start)
			MM_ERROR(MM_SETPURGE_BLOCK_NOT_FOUND);

	} while (1);

	mmrover->attributes &= ~PURGEBITS;
	mmrover->attributes |= purge;
}

//==========================================================================

/*
=====================
=
= MM_SetLock
=
= Locks / unlocks the block
=
=====================
*/

void MM_SetLock (memptr *baseptr, boolean locked)
{
	mmblocktype far *start;

	start = mmrover;

	do
	{
		if (mmrover->useptr == baseptr)
			break;

		mmrover = mmrover->next;

		if (!mmrover)
			mmrover = mmhead;
		else if (mmrover == start)
			MM_ERROR(MM_SETLOCK_BLOCK_NOT_FOUND);

	} while (1);

	mmrover->attributes &= ~LOCKBIT;
	mmrover->attributes |= locked*LOCKBIT;
}

//==========================================================================

/*
=====================
=
= MM_SortMem
=
= Throws out all purgable stuff and compresses movable blocks
=
=====================
*/

void MM_SortMem (void)
{
	mmblocktype far *scan,far *last,far *next;
	unsigned	start,length,source,dest;
	int			playing;

	//
	// lock down a currently playing sound
	//
	playing = SD_SoundPlaying ();
	if (playing)
	{
		switch (SoundMode)
		{
		case sdm_PC:
			playing += STARTPCSOUNDS;
			break;
		case sdm_AdLib:
			playing += STARTADLIBSOUNDS;
			break;
		}
		MM_SetLock(&(memptr)audiosegs[playing],true);
	}


	SD_StopSound();

	if (beforesort)
		beforesort();

	scan = mmhead;

	last = NULL;		// shut up compiler warning

	while (scan)
	{
		if (scan->attributes & LOCKBIT)
		{
		//
		// block is locked, so try to pile later blocks right after it
		//
			start = scan->start + scan->length;
		}
		else
		{
			if (scan->attributes & PURGEBITS)
			{
			//
			// throw out the purgable block
			//
				next = scan->next;
				FREEBLOCK(scan);
				last->next = next;
				scan = next;
				continue;
			}
			else
			{
			//
			// push the non purgable block on top of the last moved block
			//
				if (scan->start != start)
				{
					length = scan->length;
					source = scan->start;
					dest = start;
					while (length > 0xf00)
					{
						movedata(source,0,dest,0,0xf00*16);
						length -= 0xf00;
						source += 0xf00;
						dest += 0xf00;
					}
					movedata(source,0,dest,0,length*16);

					scan->start = start;
					*(unsigned *)scan->useptr = start;
				}
				start = scan->start + scan->length;
			}
		}

		last = scan;
		scan = scan->next;		// go to next block
	}

	mmrover = mmhead;

	if (aftersort)
		aftersort();

	if (playing)
		MM_SetLock(&(memptr)audiosegs[playing],false);
}


//==========================================================================

/*
=====================
=
= MM_ShowMemory
=
=====================
*/

#if 0

void MM_ShowMemory (void)
{
	mmblocktype far *scan;
	unsigned color,temp,x,y;
	long	end,owner;
	char    scratch[80],str[10];

	temp = bufferofs;
	bufferofs = displayofs;
	scan = mmhead;

	end = -1;

	while (scan)
	{
		if (scan->attributes & PURGEBITS)
			color = 5;		// dark purple = purgable
		else
			color = 9;		// medium blue = non purgable
		if (scan->attributes & LOCKBIT)
			color = 12;		// red = locked
		if (scan->start<=end)
			MM_ERROR(MM_SHOWMEMORY_BLK_CORRUPT);
		end = scan->length-1;
		y = scan->start/320;
		x = scan->start%320;
		VW_Hlin(x,x+end,y,color);
		VW_Plot(x,y,15);
		if (scan->next && scan->next->start > end+1)
			VW_Hlin(x+end+1,x+(scan->next->start-scan->start),y,0);	// black = free

		scan = scan->next;
	}

	VW_FadeIn ();
	IN_Ack();

	bufferofs = temp;
}


#endif 

//==========================================================================

/*
=====================
=
= MM_DumpData
=
=====================
*/

#if 0

void MM_DumpData (void)
{
	mmblocktype far *scan,far *best;
	long	lowest,oldlowest;
	unsigned	owner;
	char	lock,purge;
	FILE	*dumpfile;


	free (nearheap);
	dumpfile = fopen ("MMDUMP.TXT","w");
	if (!dumpfile)
		MM_ERROR(MM_DUMPDATA_FILE_ERROR);

	lowest = -1;
	do
	{
		oldlowest = lowest;
		lowest = 0xffff;

		scan = mmhead;
		while (scan)
		{
			owner = (unsigned)scan->useptr;

			if (owner && owner<lowest && owner > oldlowest)
			{
				best = scan;
				lowest = owner;
			}

			scan = scan->next;
		}

		if (lowest != 0xffff)
		{
			if (best->attributes & PURGEBITS)
				purge = 'P';
			else
				purge = '-';
			if (best->attributes & LOCKBIT)
				lock = 'L';
			else
				lock = '-';
			fprintf (dumpfile,"0x%p (%c%c) = %u\n"
			,(unsigned)lowest,lock,purge,best->length);
		}

	} while (lowest != 0xffff);

	fclose (dumpfile);
	MM_ERROR(MM_DUMPDATA_FILE_CREATED);
}

#endif


//==========================================================================


/*
======================
=
= MM_UnusedMemory
=
= Returns the total free space without purging
=
======================
*/

long MM_UnusedMemory (void)
{
	unsigned free;
	mmblocktype far *scan;

	free = 0;
	scan = mmhead;

	while (scan->next)
	{
		free += scan->next->start - (scan->start + scan->length);
		scan = scan->next;
	}

	return free*16l;
}

//==========================================================================


/*
======================
=
= MM_TotalFree
=
= Returns the total free space with purging
=
======================
*/

long MM_TotalFree (void)
{
	unsigned free;
	mmblocktype far *scan;

	free = 0;
	scan = mmhead;

	while (scan->next)
	{
		if ((scan->attributes&PURGEBITS) && !(scan->attributes&LOCKBIT))
			free += scan->length;
		free += scan->next->start - (scan->start + scan->length);
		scan = scan->next;
	}

	return free*16l;
}

/*
======================
=
= MM_LargestAvail
=
= Returns the Largest free space with purging
=
======================
*/

long MM_LargestAvail (void)
{
	unsigned largest,ammount;
	mmblocktype far *scan;

	largest = 0;
	scan = mmhead;

	while (scan->next)
	{
		if ((scan->attributes&PURGEBITS) && !(scan->attributes&LOCKBIT))
      {
			ammount = scan->length;
	      if (largest < ammount)
   	   	largest = ammount;
      }

		ammount = scan->next->start - (scan->start + scan->length);

      if (largest < ammount)
      	largest = ammount;

		scan = scan->next;
	}

	return largest*16l;
}

//==========================================================================

/*
=====================
=
= MM_BombOnError
=
=====================
*/

void MM_BombOnError (boolean bomb)
{
	bombonerror = bomb;
}


