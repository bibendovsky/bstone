// JM_FREE.C

//
// Warning!
//
// All functions in this source file are designated usable by the memory
// manager after program initialization.
//

#include "id_heads.h"
#ifdef MSVC
#pragma hdrstop
#endif

#include "3d_def.h"


void MML_ClearBlock (void);
Sint16 VL_VideoID (void);
void CA_CannotOpen(char *string);
void CAL_GetGrChunkLength (Sint16 chunk);
void CA_CacheScreen (Sint16 chunk);
void VH_UpdateScreen();
void IN_StartAck(void);
boolean IN_CheckAck (void);
void OpenMapFile(void);
void CloseMapFile(void);
void ClearMemory (void);
void PM_SetMainMemPurge(Sint16 level);
void ShutdownId (void);
void InitRedShifts (void);
void CAL_OptimizeNodes (huffnode *table);
void OpenAudioFile(void);
void ReadConfig();


#define SKIP_CHECKSUMS					(1)
#define SHOW_CHECKSUM					(0)

#if GAME_VERSION == SHAREWARE_VERSION

#define AUDIOT_CHECKSUM			0xFFF87142
#define MAPTEMP_CHECKSUM		0x000370C9
#define VGAGRAPH_CHECKSUM		0xFFFFDE44
#define DIZFILE_CHECKSUM		0x00000879l

#elif GAME_VERSION == MISSIONS_1_THR_3

#define AUDIOT_CHECKSUM			0xFFF87142
#define MAPTEMP_CHECKSUM		0x00084F1F
#define VGAGRAPH_CHECKSUM		0xFFFFDE44
#define DIZFILE_CHECKSUM		0x00000879l

#else


#define AUDIOT_CHECKSUM       0xfff912C9
#define MAPTEMP_CHECKSUM		0x00107739
#define VGAGRAPH_CHECKSUM		0xffff6C9A
#define DIZFILE_CHECKSUM		0x00000879l


#endif

extern int g_argc;
extern char** g_argv;
extern SDL_TimerID sys_timer_id;

void SDL_SBSetDMA(Uint8 channel);
void SDL_SetupDigi(void);

//=========================================================================
//
//								 FAR FREE DATA
//
//=========================================================================

#if FREE_DATA

char JM_FREE_DATA_START[1]={0};

#endif

#if TECH_SUPPORT_VERSION
char EnterBetaCode[]="\n  TECH SUPPORT VERSION!\n\n  NO DISTRIBUTING!";
#elif BETA_TEST
char EnterBetaCode[]="      !BETA VERSION!\n    DO NOT DISTRIBUTE\n UNDER PENALTY OF DEATH\n\n   ENTER BETA CODE";
#endif


const char    * JHParmStrings[] = {"no386","is386",nil};

char show_text1[]="\n     SYSTEM INFO\n";
char show_text2[]="=======================\n\n";
char show_text3[]="-- Memory avail after game is loaded --\n\n";
char show_text4[]="            ** Insufficient memory to run the game **";
char show_text5[]="---- Extra Devices ----\n\n";

static	const char * ParmStrings[] = {"HIDDENCARD",""};

static  Uint8 wolfdigimap[] =
{
			// These first sounds are in the upload version

			ATKIONCANNONSND,					0,
			ATKCHARGEDSND,						1,
			ATKBURSTRIFLESND,             2,
			ATKGRENADESND,                46,

			OPENDOORSND,                  3,
			CLOSEDOORSND,                 4,
			HTECHDOOROPENSND,             5,
			HTECHDOORCLOSESND,            6,

			INFORMANTDEATHSND,            7,
			SCIENTISTHALTSND,             19,
			SCIENTISTDEATHSND,            20,

			GOLDSTERNHALTSND,             8,
			GOLDSTERNLAUGHSND,            24,

			HALTSND,                      9,			// Rent-A-Cop 1st sighting
			RENTDEATH1SND,                10,		// Rent-A-Cop Death

			EXPLODE1SND,                  11,

			GGUARDHALTSND,                12,
			GGUARDDEATHSND,               17,

			PROHALTSND,                   16,
			PROGUARDDEATHSND,             13,

			BLUEBOYDEATHSND,              18,
			BLUEBOYHALTSND,					51,

			SWATHALTSND,                  22,
			SWATDIESND,                   47,

			SCANHALTSND,                  15,
			SCANDEATHSND,                 23,

			PODHATCHSND,                  26,
			PODHALTSND,							50,
			PODDEATHSND,                  49,

			ELECTSHOTSND,                 27,

			DOGBOYHALTSND,                14,
			DOGBOYDEATHSND,               21,
			ELECARCDAMAGESND,             25,
			ELECAPPEARSND,                28,
			ELECDIESND,                   29,

			INFORMDEATH2SND,              39,		// Informant Death #2
			RENTDEATH2SND,                34,      // Rent-A-Cop Death #2
			PRODEATH2SND,                 42,	   // PRO Death #2
			SWATDEATH2SND,                48, 		// SWAT Death #2
			SCIDEATH2SND,					   53,		// Gen. Sci Death #2

			LIQUIDDIESND,                 30,

			GURNEYSND,                    31,
			GURNEYDEATHSND,               41,

			WARPINSND,                    32,
			WARPOUTSND,                   33,

			EXPLODE2SND,                  35,

			LCANHALTSND,                  36,
			LCANDEATHSND,                 37,

//			RENTDEATH3SND,		            38,		// Rent-A-Cop Death #3
			INFORMDEATH3SND,              40,		// Informant Death #3
//			PRODEATH3SND,                 43,      // PRO Death #3
//			SWATDEATH3SND,						52,		// Swat Guard #3
			SCIDEATH3SND,						54,		// Gen. Sci Death #3

			LCANBREAKSND,                 44,
			SCANBREAKSND,                 45,
			CLAWATTACKSND,						56,
			SPITATTACKSND,						55,
			PUNCHATTACKSND,					57,

			LASTSOUND
};


#if 0

static  Uint8 wolfdigimap[] =
{
			// These first sounds are in the upload version

			ATKIONCANNONSND,					0,
			ATKCHARGEDSND,						1,
			ATKBURSTRIFLESND,             2,
			ATKGRENADESND,                46,

			OPENDOORSND,                  3,
			CLOSEDOORSND,                 4,
			HTECHDOOROPENSND,             5,
			HTECHDOORCLOSESND,            6,

			INFORMANTDEATHSND,            7,
			SCIENTISTHALTSND,             19,
			SCIENTISTDEATHSND,            20,

			GOLDSTERNHALTSND,             8,
			GOLDSTERNLAUGHSND,            24,

			HALTSND,                      9,		// Rent-A-Cop 1st sighting
			RENTDEATH1SND,                10,		// Rent-A-Cop Death

			EXPLODE1SND,                  11,

			GGUARDHALTSND,                12,
			GGUARDDEATHSND,               17,

			PROHALTSND,                   16,
			PROGUARDDEATHSND,             13,

			BLUEBOYDEATHSND,              18,
			BLUEBOYHALTSND,					51,

			SWATHALTSND,                  22,
			SWATDIESND,                   47,

			SCANHALTSND,                  15,
			SCANDEATHSND,                 23,

			PODHATCHSND,                  26,
			PODHALTSND,							50,
			PODDEATHSND,                  49,

			ELECTSHOTSND,                 27,

			DOGBOYHALTSND,                14,
			DOGBOYDEATHSND,               21,
			ELECARCDAMAGESND,             25,
			ELECAPPEARSND,                28,
			ELECDIESND,                   29,

			INFORMDEATH2SND,              39,		// Informant Death #2
			RENTDEATH2SND,                34,      // Rent-A-Cop Death #2
			PRODEATH2SND,                 42,	   // PRO Death #2
			SWATDEATH2SND,                48, 		// SWAT Death #2
			SCIDEATH2SND,					   53,		// Gen. Sci Death #2

			LIQUIDDIESND,                 30,

			GURNEYSND,                    31,
			GURNEYDEATHSND,               41,

			WARPINSND,                    32,
			WARPOUTSND,                   33,

			EXPLODE2SND,                  35,

			LCANHALTSND,                  36,
			LCANDEATHSND,                 37,

//			RENTDEATH3SND,		            38,		// Rent-A-Cop Death #3
			INFORMDEATH3SND,              40,		// Informant Death #3
//			PRODEATH3SND,                 43,      // PRO Death #3
//			SWATDEATH3SND,						52,		// Swat Guard #3
			SCIDEATH3SND,						54,		// Gen. Sci Death #3

			LCANBREAKSND,                 44,
			SCANBREAKSND,                 45,
			CLAWATTACKSND,						56,
			SPITATTACKSND,						55,
			PUNCHATTACKSND,					57,

			LASTSOUND
};

#endif

char cinfo_text[]=	"\n"
								"Planet Strike\n"
								"Copyright (c) 1993 - JAM Productions, Inc.\n"
								"All rights reserved.\n";

#if BETA_TEST
char dver_text[]="Download the latest version pal!";
#endif


#if FREE_DATA

char JM_FREE_DATA_END[1]={0};

#endif

extern const Uint8 colormap[16896];
const Uint8 * lightsource;

//=========================================================================
//
//								 FREE FUNCTIONS
//
//=========================================================================



#if FREE_FUNCTIONS

// This function is used as a label for the start of the
// functions used by the memory manager.
//
void JM_FREE_START()
{
}

#endif

// ------------------ ID Software 'startup' functions ---------------------

// FIXME Moved to id_ca.c
#if 0
/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/
void CA_Startup (void)
{
#ifdef PROFILE
	unlink ("PROFILE.TXT");
	profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif

	CAL_SetupMapFile ();
	CAL_SetupGrFile ();
	CAL_SetupAudioFile ();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;
}
#endif // 0


// FIXME Moved to id_in.c
#if 0

//#if !IN_DEVELOPMENT

extern boolean IN_Started;
extern char * IN_ParmStrings[];
extern boolean INL_StartJoy(Uint16 joy);
extern boolean INL_StartMouse(void);
extern void INL_StartKbd(void);

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
boolean
INL_StartJoy(Uint16 joy)
{
	Uint16		x,y;

	IN_GetJoyAbs(joy,&x,&y);

	if
	(
		((x == 0) || (x > MaxJoyValue - 10))
	||	((y == 0) || (y > MaxJoyValue - 10))
	)
		return(false);
	else
	{
		IN_SetupJoy(joy,0,x * 2,0,y * 2);
		return(true);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	boolean	checkjoys,checkmouse,checkNG;
	Uint16	i;

	if (IN_Started)
		return;

	checkjoys = true;
	checkmouse = true;
	checkNG = false;
	for (i = 1;i < g_argc;i++)
	{
		switch (US_CheckParm(g_argv[i],IN_ParmStrings))
		{
			case 0:
				checkjoys = false;
			break;

			case 1:
				checkmouse = false;
			break;

			case 2:
				checkNG = true;
			break;
		}
	}

	if (checkNG)
	{
#if 0
		#define WORD_CODE(c1,c2)	((c2)|(c1<<8))

		NGjoy(0xf0);
		if ((_AX==WORD_CODE('S','G')) && _BX)
			NGinstalled=true;
#endif // 0
	}

	INL_StartKbd();
	MousePresent = checkmouse? INL_StartMouse() : false;

	for (i = 0;i < MaxJoys;i++)
		JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;

	IN_Started = true;
}
#endif // 0

//#endif


// FIXME Moved to id_mm.h
#if 0
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

extern boolean mmstarted,bombonerror;
extern mmblocktype  mmblocks[], *mmhead, *mmfree,
						 *mmnew, *mmrover;
extern void *farheap;
extern void	*nearheap;

void MM_Startup (void)
{
	int i;
	unsigned 	Sint32 length;
	void 	*start;
	unsigned 	segstart,seglength,endfree;

	if (mmstarted)
		MM_Shutdown ();


	mmstarted = true;
	bombonerror = true;
//
// set up the linked list (everything in the free list;
//
	mmhead = NULL;
	mmfree = &mmblocks[0];
	for (i=0;i<MAXBLOCKS-1;i++)
		mmblocks[i].next = &mmblocks[i+1];
	mmblocks[i].next = NULL;

//
// locked block of all memory until we punch out free space
//
	GETNEWBLOCK;
	mmhead = mmnew;				// this will allways be the first node
	mmnew->start = 0;
	mmnew->length = 0xffff;
	mmnew->attributes = LOCKBIT;
	mmnew->next = NULL;
	mmrover = mmhead;


//
// get all available near conventional memory segments
//

// FIXME
#if 0
	length=coreleft();
#endif // 0
    length = 64 * 1024;

	start = (void *)(nearheap = malloc(length));

	length -= 16-(FP_OFF(start)&15);
	length -= SAVENEARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = FP_SEG(start)+(FP_OFF(start)+15)/16;
	MML_UseSpace (segstart,seglength);
	mminfo.nearheap = length;



//
// get all available far conventional memory segments
//

// FIXME
#if 0
	length=farcoreleft();
#endif // 0
    length = 1 * 1024 * 1024;

	start = farheap = malloc(length);
	length -= 16-(FP_OFF(start)&15);
	length -= SAVEFARHEAP;
	seglength = length / 16;			// now in paragraphs
	segstart = FP_SEG(start)+(FP_OFF(start)+15)/16;
	MML_UseSpace (segstart,seglength);
	mminfo.farheap = length;
	mminfo.mainmem = mminfo.nearheap + mminfo.farheap;

//	__SEGS_AVAILABLE__ = (Sint32)(mminfo.EMSmem + mminfo.XMSmem + mminfo.mainmem) / 16;

//
// allocate the misc buffer
//
	mmrover = mmhead;		// start looking for space after low block

	MM_GetPtr (&bufferseg,BUFFERSIZE);
}
#endif // 0

// FIXME
#if 0
//
//	PML_StartupEMS() - Sets up EMS for Page Mgr's use
//		Checks to see if EMS driver is present
//      Verifies that EMS hardware is present
//		Make sure that EMS version is 3.2 or later
//		If there's more than our minimum (2 pages) available, allocate it (up
//			to the maximum we need)
//


#if !DUAL_SWAP_FILES

char	EMMDriverName[9] = "EMMXXXX0";
extern Uint16		EMSAvail,EMSPagesAvail,EMSHandle,
					EMSPageFrame,EMSPhysicalPage;
extern EMSListStruct	EMSList[];


boolean
PML_StartupEMS(void)
{
	int		i;
	Sint32	size;

	EMSPresent = false;			// Assume that we'll fail
	EMSAvail = 0;

#if 0
	_DX = Uint16EMMDriverName;
	_AX = 0x3d00;
	geninterrupt(0x21);			// try to open EMMXXXX0 device
asm	jnc	gothandle
	goto error;

gothandle:
	_BX = _AX;
	_AX = 0x4400;
	geninterrupt(0x21);			// get device info
asm	jnc	gotinfo;
	goto error;

gotinfo:
asm	and	dx,0x80
	if (!_DX)
		goto error;

	_AX = 0x4407;
	geninterrupt(0x21);			// get status
asm	jc	error
	if (!_AL)
		goto error;

	_AH = 0x3e;
	geninterrupt(0x21);			// close handle

	_AH = EMS_STATUS;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;				// make sure EMS hardware is present

	_AH = EMS_VERSION;
	geninterrupt(EMS_INT);
	if (_AH || (_AL < 0x32))	// only work on EMS 3.2 or greater (silly, but...)
		goto error;

	_AH = EMS_GETFRAME;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;				// find the page frame address
	EMSPageFrame = _BX;

	_AH = EMS_GETPAGES;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;
	if (_BX < 2)
		goto error;         	// Require at least 2 pages (32k)
	EMSAvail = _BX;

	// Don't hog all available EMS
	size = EMSAvail * (Sint32)EMSPageSize;
	if (size - (EMSPageSize * 2) > (ChunksInFile * (Sint32)PMPageSize))
	{
		size = (ChunksInFile * (Sint32)PMPageSize) + EMSPageSize;
		EMSAvail = size / EMSPageSize;
	}

	_AH = EMS_ALLOCPAGES;
	_BX = EMSAvail;
	geninterrupt(EMS_INT);
	if (_AH)
		goto error;
	EMSHandle = _DX;

	mminfo.EMSmem += EMSAvail * (Sint32)EMSPageSize;

	// Initialize EMS mapping cache
	for (i = 0;i < EMSFrameCount;i++)
		EMSList[i].baseEMSPage = -1;

	EMSPresent = true;			// We have EMS

error:
#endif // 0

	return(EMSPresent);
}

//
//	PML_StartupXMS() - Starts up XMS for the Page Mgr's use
//		Checks for presence of an XMS driver
//		Makes sure that there's at least a page of XMS available
//		Allocates any remaining XMS (rounded down to the nearest page size)
//

extern boolean			XMSPresent;
extern Uint16			XMSAvail,XMSPagesAvail,XMSHandle;
extern Uint32		XMSDriver;
extern int				XMSProtectPage;

boolean
PML_StartupXMS(void)
{
	XMSPresent = false;					// Assume failure
	XMSAvail = 0;

#if 0
asm	mov	ax,0x4300
asm	int	XMS_INT         				// Check for presence of XMS driver
	if (_AL != 0x80)
		goto error;

asm	mov	ax,0x4310
asm	int	XMS_INT							// Get address of XMS driver
asm	mov	[WORD PTR XMSDriver],bx
asm	mov	[WORD PTR XMSDriver+2],es		// function pointer to XMS driver

	XMS_CALL(XMS_QUERYFREE);			// Find out how much XMS is available
	XMSAvail = _AX;
	if (!_AX)				// AJR: bugfix 10/8/92
		goto error;

	XMSAvail &= ~(PMPageSizeKB - 1);	// Round off to nearest page size
	if (XMSAvail < (PMPageSizeKB * 2))	// Need at least 2 pages
		goto error;

	_DX = XMSAvail;
	XMS_CALL(XMS_ALLOC);				// And do the allocation
	XMSHandle = _DX;
	if (!_AX)				// AJR: bugfix 10/8/92
	{
		XMSAvail = 0;
		goto error;
	}

	mminfo.XMSmem += XMSAvail * 1024;

	XMSPresent = true;
error:
#endif // 0

	return(XMSPresent);
}

//
//
//	PML_StartupMainMem() - Allocates as much main memory as is possible for
//		the Page Mgr. The memory is allocated as non-purgeable, so if it's
//		necessary to make requests of the Memory Mgr, PM_UnlockMainMem()
//		needs to be called.
//

extern boolean			MainPresent;
extern void*			MainMemPages[PMMaxMainMem];
extern PMBlockAttr		MainMemUsed[PMMaxMainMem];
extern int				MainPagesAvail;

void
PML_StartupMainMem(void)
{
// FIXME
#if 0
	int		i,n;
	void*	*p;

	MainPagesAvail = 0;
	MM_BombOnError(false);
	for (i = 0,p = MainMemPages;i < PMMaxMainMem;i++,p++)
	{
		MM_GetPtr(p,PMPageSize);
		if (mmerror)
			break;

		MainPagesAvail++;
		MainMemUsed[i] = pmba_Allocated;
	}
	MM_BombOnError(true);
	if (mmerror)
		mmerror = false;
	if (MainPagesAvail < PMMinMainMem)
		PM_ERROR(PML_STARTUPMAINMEM_LOW);
	MainPresent = true;
#endif // 0
}
#endif // 0

// FIXME Moved to id_pm.c
#if 0
//
//	PM_Startup() - Start up the Page Mgr
//

extern boolean PMStarted;
extern char * PM_ParmStrings[];

void
PM_Startup(void)
{
	boolean	nomain,noems,noxms;
	int		i;

	if (PMStarted)
		return;

	nomain = noems = noxms = false;
	for (i = 1;i < g_argc;i++)
	{
		switch (US_CheckParm(g_argv[i],PM_ParmStrings))
		{
		case 0:
			nomain = true;
			break;
		case 1:
			noems = true;
			break;
		case 2:
			noxms = true;
			break;
		}
	}

	OpenPageFile();

	if (!noems)
		PML_StartupEMS();
	if (!noxms)
		PML_StartupXMS();

	if (nomain && !EMSPresent)
		PM_ERROR(PM_STARTUP_NO_MAIN_EMS);
	else
		PML_StartupMainMem();

	PM_Reset();

	PMStarted = true;
}
#endif // 0

#endif


///////////////////////////////////////////////////////////////////////////
//
//	US_Startup() - Starts the User Mgr
//
///////////////////////////////////////////////////////////////////////////

extern boolean US_Started;
extern const char * US_ParmStrings[];
extern const char * US_ParmStrings2[];

// BBi
Uint32 sys_timer_callback(Uint32 interval, void* param)
{
    ++TimeCount;
    return interval;
}
// BBi

void
US_Startup(void)
{
	Sint16	i,n;

    // BBi
    int sdl_result;
    // BBi

	if (US_Started)
		return;

    // BBi
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "SDL: %s", "Initializing timer...");

    sdl_result = SDL_InitSubSystem(SDL_INIT_TIMER);

    if (sdl_result != 0)
        Quit("%s", SDL_GetError());

    sys_timer_id = SDL_AddTimer(1000 / 70, sys_timer_callback, NULL);

    if (sys_timer_id == 0)
        Quit("%s", SDL_GetError());
    // BBi

	US_InitRndT(true);		// Initialize the random number generator

	for (i = 1;i < g_argc;i++)
	{
		switch (US_CheckParm(g_argv[i],US_ParmStrings2))
		{
		case 0:
			compatability = true;
			break;
		case 1:
			compatability = false;
			break;
		}
	}

	// Check for TED launching here
	for (i = 1;i < g_argc;i++)
	{
		n = US_CheckParm(g_argv[i],US_ParmStrings);
		switch(n)
		{
		 case 0:
#if 0
			tedlevelnum = atoi(g_argv[i + 1]);
//		   if (tedlevelnum >= 0)
			  tedlevel = true;
#endif
			break;

//		 case 1:
//			NoWait = true;
//			break;
		}
	}

	US_Started = true;
}

/*
=======================
=
= VL_Startup	// WOLFENSTEIN HACK
=
=======================
*/


void	VL_Startup (void)
{
	Sint16 i,videocard;

#if 0
	asm	cld;
#endif // 0

	videocard = VL_VideoID ();
	for (i = 1;i < g_argc;i++)
		if (US_CheckParm(g_argv[i],ParmStrings) == 0)
		{
			videocard = 5;
			break;
		}

	if (videocard != 5)
		Quit ("Improper video card! Try the -HIDDENCARD command line parameter!");

}

// FIXME Moved to id_vl.c
#if 0
#if !RESTART_PICTURE_PAUSE

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void	VL_SetVGAPlaneMode (void)
{
// FIXME
#if 0
asm	mov	ax,0x13
asm	int	0x10
	VL_DePlaneVGA ();
	VGAMAPMASK(15);
	VL_SetLineWidth (40);
#endif // 0

    const size_t VGA_MEM_SIZE = 4 * 64 * 1024;

    free(vga_memory);
    vga_memory = (Uint8*)malloc(VGA_MEM_SIZE);
    memset(vga_memory, 0, VGA_MEM_SIZE);
}
#endif // 0

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo (Uint8 color)
{
// FIXME
#if 0
asm	mov	dx,GC_INDEX
asm	mov	al,GC_MODE
asm	out	dx,al
asm	inc	dx
asm	in	al,dx
asm	and	al,0xfc				// write mode 0 to store directly to video
asm	out	dx,al

asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax				// write through all four planes

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,[color]
asm	mov	ah,al
asm	mov	cx,0x8000			// 0x8000 words, clearing 8 video bytes/word
asm	xor	di,di
asm	rep	stosw
#endif // 0
}

/*
=================
=
= VL_DePlaneVGA
=
=================
*/

void VL_DePlaneVGA (void)
{
// FIXME
#if 0
//
// change CPU addressing to non linear mode
//

//
// turn off chain 4 and odd/even
//
	outportb (SC_INDEX,SC_MEMMODE);
	outportb (SC_INDEX+1,(inportb(SC_INDEX+1)&~8)|4);

	outportb (SC_INDEX,SC_MAPMASK);		// leave this set throughought

//
// turn off odd/even and set write mode 0
//
	outportb (GC_INDEX,GC_MODE);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~0x13);

//
// turn off chain
//
	outportb (GC_INDEX,GC_MISCELLANEOUS);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~2);

//
// clear the entire buffer space, because int 10h only did 16 k / plane
//
	VL_ClearVideo (0);

//
// change CRTC scanning from doubleword to byte mode, allowing >64k scans
//
	outportb (CRTC_INDEX,CRTC_UNDERLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)&~0x40);

	outportb (CRTC_INDEX,CRTC_MODE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)|0x40);
#endif // 0
}

/*
====================
=
= VL_SetLineWidth
=
= Line witdh is in WORDS, 40 words is normal width for vgaplanegr
=
====================
*/

void VL_SetLineWidth (Uint16 width)
{
	Sint16 i,offset;

// FIXME
#if 0
//
// set wide virtual screen
//
	outport (CRTC_INDEX,CRTC_OFFSET+width*256);
#endif // 0

//
// set up lookup tables
//
	linewidth = width*2;

	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += linewidth;
	}
}

#endif


/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const   float   radtoint = static_cast<float>(FINEANGLES/2/PI);

void BuildTables (void)
{
  Sint16           i;
  float         angle,anglestep;
  double        tang;
  fixed         value;


//
// calculate fine tangents
//

	for (i=0;i<FINEANGLES/8;i++)
	{
		tang = tan( (i+0.5)/radtoint);
		finetangent[i] = static_cast<Sint32>(tang*TILEGLOBAL);
		finetangent[FINEANGLES/4-1-i] = static_cast<Sint32>(1/tang*TILEGLOBAL);
	}

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//
// The low word of the value is the fraction, the high bit is the sign bit,
// bits 16-30 should be 0
//

  angle = 0.0F;
  anglestep = static_cast<float>(PI/2/ANGLEQUAD);
  for (i=0;i<=ANGLEQUAD;i++)
  {
	value=static_cast<fixed>(GLOBAL1*sin(static_cast<double>(angle)));
	sintable[i]=
	  sintable[i+ANGLES]=
	  sintable[ANGLES/2-i] = value;
	sintable[ANGLES-i]=
	  sintable[ANGLES/2+i] = value | 0x80000000l;
	angle += anglestep;
  }

// FIXME
#if 0
// Fix ColorMap
    temp = (Uint8*)malloc(16896);
  memcpy(temp,colormap,16896);
  lightsource=(Uint8 *)(((Sint32)colormap + 255)&~0xff);
  memcpy(lightsource,temp,16384);
    free(temp);
#endif // 0

    lightsource = colormap;
}

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls (void)
{
	Sint16     i;

	//
	// Hey! Yea You! This is where you can VERY easly setup to use a
	// specific 'BANK' of wall graphics.... JTR
	//

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}




/*
=====================
=
= InitDigiMap
=
=====================
*/
// 3D_GAME.C

void InitDigiMap (void)
{
	char *map;

	for (map = reinterpret_cast<char*>(wolfdigimap);*map != LASTSOUND;map += 2)
		DigiMap[map[0]] = map[1];
}



/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	bstone::FileStream handle;
	Sint32 length;
	char fname[13];

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
	strcpy(fname,aheadname);
	strcat(fname,extension);

    handle.open(fname);
	if (!handle.is_open())
		CA_CannotOpen(fname);

	length = static_cast<Sint32>(handle.get_size());
    audiostarts = (Sint32*)malloc(length);
	handle.read(audiostarts, length);
	handle.close();
#else
	audiohuffman = (huffnode *)&audiodict;
	CAL_OptimizeNodes (audiohuffman);
	audiostarts = (Sint32 *)FP_SEG(&audiohead);
#endif

//
// open the data file
//
	OpenAudioFile();
}



/*
======================
=
= CAL_SetupGrFile
=
======================
*/

// FIXME
#if 0
void CAL_SetupGrFile (void)
{
	char fname[13];
	int handle;
	void* compseg;

#ifdef GRHEADERLINKED

	grhuffman = (huffnode *)&EGAdict;
	grstarts = (Sint32 *)FP_SEG(&EGAhead);

	CAL_OptimizeNodes (grhuffman);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	strcpy(fname,gdictname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	read(handle, &grhuffman, sizeof(grhuffman));
	close(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
    grstarts = (Sint32*)malloc((NUMCHUNKS + 1) * FILEPOSSIZE);

	strcpy(fname,gheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	CA_FarRead(handle, (void*)grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);

	close(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
	OpenGrFile();

//
// load the pic and sprite headers into the arrays in the data segment
//
    pictable = (pictabletype*)malloc(NUMPICS * sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
    compseg = malloc(chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (Uint8 *)pictable,NUMPICS*sizeof(pictabletype),grhuffman,false);
	free(compseg);


// MDM begin
#if 0
	MM_GetPtr(&(memptr)picmtable,NUMPICM*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPICM);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (Uint8 *)picmtable,NUMPICS*sizeof(pictabletype),grhuffman,false);
	MM_FreePtr(&compseg);
#endif
// MDM end

}
#endif // 0

void CAL_SetupGrFile()
{
    char fname[13];
    bstone::FileStream handle;
    void* compseg;

    //
    // load ???dict.ext (huffman dictionary for graphics files)
    //

    strcpy(fname, gdictname);
    strcat(fname, extension);

    handle.open(fname);

    if (!handle.is_open())
        CA_CannotOpen(fname);

    handle.read(&grhuffman, sizeof(grhuffman));

    //
    // load the data offsets from ???head.ext
    //
    grstarts = (Sint32*)malloc((NUMCHUNKS + 1) * FILEPOSSIZE);

    strcpy(fname, gheadname);
    strcat(fname, extension);

    handle.open(fname);

    if (!handle.is_open())
        CA_CannotOpen(fname);

    handle.read(grstarts, (NUMCHUNKS + 1) * FILEPOSSIZE);


    //
    // Open the graphics file, leaving it open until the game is finished
    //
    OpenGrFile();

    //
    // load the pic and sprite headers into the arrays in the data segment
    //
    pictable = (pictabletype*)malloc(NUMPICS * sizeof(pictabletype));
    CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
    compseg = malloc(chunkcomplen);
    grhandle.read(compseg, chunkcomplen);

    CAL_HuffExpand(
        static_cast<Uint8*>(compseg),
        (Uint8*)pictable,
        NUMPICS * sizeof(pictabletype),
        grhuffman,
        false);

    free(compseg);
}



/*
======================
=
= CAL_SetupMapFile
=
======================
*/

// FIXME
#if 0
void CAL_SetupMapFile (void)
{
	int	i;
	int handle;
	Sint32 length,pos;
	char fname[13];

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef MAPHEADERLINKED
	strcpy(fname,mheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	length = filelength(handle);
    tinf = (Uint8*)malloc(length);
	CA_FarRead(handle, tinf, length);
	close(handle);
#else

	tinf = (Uint8 *)FP_SEG(&maphead);

#endif

//
// open the data file
//
	OpenMapFile();

//
// load all map header
//
	for (i=0;i<NUMMAPS;i++)
	{
		pos = ((mapfiletype	*)tinf)->headeroffsets[i];
		if (pos<0)						// $FFFFFFFF start is a sparse map
			continue;

        mapheaderseg[i] = (maptype*)malloc(sizeof(maptype));
		lseek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,(void*)mapheaderseg[i],sizeof(maptype));
	}

//
// allocate space for 3 64*64 planes
//
	for (i=0;i<MAPPLANES;i++)
	{
        mapsegs[i] = (unsigned*)malloc(2 * 64 * 64);
	}

#if FORCE_FILE_CLOSE
   CloseMapFile();
#endif

}
#endif // 0

void CAL_SetupMapFile()
{
    Sint16 i;
    bstone::FileStream handle;
    Sint32 pos;
    char fname[13];
    mapfiletype header;
    maptype* map_header;

    //
    // load maphead.ext (offsets and tileinfo for map file)
    //

    strcpy(fname, mheadname);
    strcat(fname, extension);

    handle.open(fname);

    if (!handle.is_open())
        CA_CannotOpen(fname);

    handle.read(&header.RLEWtag, sizeof(header.RLEWtag));
    handle.read(&header.headeroffsets, sizeof(header.headeroffsets));

    rlew_tag = header.RLEWtag;

    //
    // open the data file
    //
    OpenMapFile();

    //
    // load all map header
    //
    for (i = 0; i < NUMMAPS; ++i) {
        pos = header.headeroffsets[i];

        if (pos < 0)
            continue;

        mapheaderseg[i] = (maptype*)malloc(sizeof(maptype));
        map_header = mapheaderseg[i];

        maphandle.set_position(pos);

        maphandle.read(
            &map_header->planestart,
            sizeof(map_header->planestart));

        maphandle.read(
            &map_header->planelength,
            sizeof(map_header->planelength));

        maphandle.read(
            &map_header->width,
            sizeof(map_header->width));

        maphandle.read(
            &map_header->height,
            sizeof(map_header->height));

        maphandle.read(
            &map_header->name,
            sizeof(map_header->name));
    }

    //
    // allocate space for 3 64*64 planes
    //
    for (i = 0; i < MAPPLANES; ++i)
        mapsegs[i] = (Uint16*)malloc(2 * 64 * 64);

#if FORCE_FILE_CLOSE
    CloseMapFile();
#endif

}


// --------------------- Other general functions ------------------------

// FIXME
// BBi Moved to 3d_main.cpp
#if 0
/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig()
{
	int                     file;
	SDMode          sd;
	SMMode          sm;
	SDSMode         sds;

	boolean config_found=false;
	Uint16 flags=gamestate.flags;
	MakeDestPath(configname);

	if ( (file = open(tempPath,O_BINARY | O_RDONLY)) != -1)
	{
	//
	// valid config file
	//
        int i;
        Sint16 enum_buffer;

        for (i = 0; i < MaxScores; ++i) {
            HighScore* score = &Scores[i];

            read(file, score->name, sizeof(score->name));
            read(file, &score->score, sizeof(score->score));
            read(file, &score->completed, sizeof(score->completed));
            read(file, &score->episode, sizeof(score->episode));
            read(file, &score->ratio, sizeof(score->ratio));
        }

        read(file, &enum_buffer, sizeof(enum_buffer));
        sd = (SDMode)enum_buffer;

        read(file, &enum_buffer, sizeof(enum_buffer));
        sm = (SMMode)enum_buffer;

        read(file, &enum_buffer, sizeof(enum_buffer));
        sds = (SDSMode)enum_buffer;


		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&viewsize,sizeof(viewsize));
		read(file,&mouseadjustment,sizeof(mouseadjustment));

		read(file,&flags,sizeof(flags));		// Use temp so we don't destroy pre-sets.
		flags &= GS_HEARTB_SOUND|GS_ATTACK_INFOAREA|GS_LIGHTING|GS_DRAW_CEILING|GS_DRAW_FLOOR;			 	// Mask out the useful flags!

		gamestate.flags |= flags;				// Must "OR", some flags are already set.
		close(file);

#if 0
#ifdef CEILING_FLOOR_COLORS
		if (gamestate.flags & GS_DRAW_FLOOR)
		{
			PM_Shutdown();
			PM_Startup ();
			PM_UnlockMainMem ();
		}
#endif
#endif

		if (sd == sdm_AdLib &&
			(!AdLibPresent || !SoundBlasterPresent))
		{
			sd = sdm_PC;
			sd = sdm_Off;
		}

		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
			(sds == sds_SoundSource && !SoundSourcePresent))
			sds = sds_Off;

		if (!MousePresent)
			mouseenabled = false;
		if (!JoysPresent[joystickport])
			joystickenabled = false;

		MainMenu[6].active=AT_ENABLED;
		MainItems.curpos=0;

		config_found=true;
	}

	if ((!config_found) || (!viewsize))
	{
	//
	// no config file, so select by hardware
	//
		if (SoundBlasterPresent || AdLibPresent)
		{
			sd = sdm_AdLib;
			sm = smm_AdLib;
		}
		else
		{
			sd = sdm_PC;
			sm = smm_Off;
		}

		if (SoundBlasterPresent)
			sds = sds_SoundBlaster;
		else if (SoundSourcePresent)
			sds = sds_SoundSource;
		else
			sds = sds_Off;

		if (MousePresent)
			mouseenabled = true;

		joystickenabled = false;
		joypadenabled = false;
		joystickport = 0;
		joystickprogressive = false;

		viewsize = 17;
		mouseadjustment=5;
		gamestate.flags |= GS_HEARTB_SOUND|GS_ATTACK_INFOAREA;

#ifdef CEILING_FLOOR_COLORS
		gamestate.flags |= GS_DRAW_CEILING|GS_DRAW_FLOOR|GS_LIGHTING;
#else
		gamestate.flags |= GS_LIGHTING;
#endif

#if 0
		PM_Shutdown();
		PM_Startup ();
		PM_UnlockMainMem ();
#endif
	}

	SD_SetMusicMode (sm);
	SD_SetSoundMode (sd);
	SD_SetDigiDevice (sds);

}
#endif // 0

/*
========================
=
= Patch386
=
= Patch ldiv to use 32 bit instructions
=
========================
*/

void Patch386 (void)
{
extern void jabhack2(void);
extern int  CheckIs386(void);

	Sint16     i;

	for (i = 1;i < g_argc;i++)
		switch (US_CheckParm(g_argv[i],JHParmStrings))
      {
      	case 0:
				IsA386 = false;
			return;

         case 1:
         	IsA386 = true;
         return;
		}

	if (CheckIs386())
	{
		IsA386 = true;
		jabhack2();
	}
	else
		IsA386 = false;
}


#if 0

//------------------------------------------------------------------------
// CheckIs386()
//------------------------------------------------------------------------
int CheckIs386()
{

asm	pushf						// Save flag registers, we use them here
asm	xor	ax,ax				// Clear AX and...
asm	push	ax					// ...push it onto the stack
asm	popf						// Pop 0 into flag registers (all bits to 0),
asm	pushf						// attempting to set bits 12-15 of flags to 0's
asm	pop	ax					// Recover the save flags
asm	and	ax,08000h		// If bits 12-15 of flags are set to
asm	cmp	ax,08000h		// zero then it's 8088/86 or 80188/186
asm	jz	not386

asm	mov	ax,07000h		// Try to set flag bits 12-14 to 1's
asm	push	ax					// Push the test value onto the stack
asm	popf						// Pop it into the flag register
asm	pushf						// Push it back onto the stack
asm	pop	ax					// Pop it into AX for check
asm	and	ax,07000h		// if bits 12-14 are cleared then
asm	jz	not386				// the chip is an 80286

asm	mov	ax,1				// We now assume it's a 80386 or better
asm	popf
asm	retf

not386:
asm	xor	ax,ax
asm	popf
asm	retf

}

#endif


#define CHECK_FOR_EPISODES

extern CP_itemtype NewEmenu[];
extern Sint16 EpisodeSelect[];


//-------------------------------------------------------------------------
// CheckForEpisodes() - CHECK FOR VERSION/EXTESION
//-------------------------------------------------------------------------
void CheckForEpisodes(void)
{
	Sint16 i;

#if (GAME_VERSION != SHAREWARE_VERSION)
    bstone::FileStream stream("VSWAP.VSI");

    if (stream.is_open())
        strcpy(extension, "VSI");
#else
    bstone::FileStream stream("VSWAP.FSW");

    if (stream.is_open())
        strcpy(extension, "FSW");
#endif
    else {
        printf("No Fire Strike data files found!");
        exit(0);
    }

	for (i=0;i<mv_NUM_MOVIES;i++)
		strcat(Movies[i].FName,extension);

#ifdef ACTIVATE_TERMINAL
	strcat(term_com_name,extension);
	strcat(term_msg_name,extension);
#endif

// FIXME
#if 0
	strcat(configname,extension);
	strcat(SaveName,extension);
#endif // 0

	strcat(PageFileName,extension);
	strcat(audioname,extension);
	strcat(demoname,extension);

#if DUAL_SWAP_FILES
	strcat(AltPageFileName,extension);
	ShadowsAvail = (!findfirst(AltPageFileName,&f,FA_ARCH));
#endif
}


#if 0
//
// ORGINAL used in Aliens of Gold
//
//-------------------------------------------------------------------------
// CheckForEpisodes() - CHECK FOR EPISODES
//-------------------------------------------------------------------------
void CheckForEpisodes(void)
{
	struct ffblk f;
	Sint16 i;

#if (GAME_VERSION != SHAREWARE_VERSION)
#ifdef CHECK_FOR_EPISODES
	if (!findfirst("*.BS6",&f,FA_ARCH))
#else
	if (!findfirst("*.BS1",&f,FA_ARCH))
#endif
	{
#ifdef CHECK_FOR_EPISODES
		strcpy(extension,"BS6");
#else
		strcpy(extension,"BS1");
#endif
		NewEmenu[1].active =
		NewEmenu[2].active =
		NewEmenu[3].active =
		NewEmenu[4].active =
		NewEmenu[5].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] =
		EpisodeSelect[4] =
		EpisodeSelect[5] = AT_ENABLED;
	}
	else
#ifdef CHECK_FOR_EPISODES
		if (!findfirst("*.BS3",&f,FA_ARCH))
#else
		if (!findfirst("*.BS1",&f,FA_ARCH))
#endif
		{
#ifdef CHECK_FOR_EPISODES
			strcpy(extension,"BS3");
#else
			strcpy(extension,"BS1");
#endif
			NewEmenu[1].active =
			NewEmenu[2].active =
			EpisodeSelect[1] =
			EpisodeSelect[2] = AT_ENABLED;
		}
		else
#endif
			if (!findfirst("*.BS1",&f,FA_ARCH))
			{
				strcpy(extension,"BS1");
			}
			else
			{
				printf("No Blake Stone data files found!");
				exit(0);
			}

#if BETA_TEST
// This enables specific missions for beta testers.
//
	NewEmenu[1].active =
	EpisodeSelect[1] =
	NewEmenu[2].active =
	EpisodeSelect[2] =
	NewEmenu[3].active =
	EpisodeSelect[3] =
	NewEmenu[4].active =
	EpisodeSelect[4] =
	NewEmenu[5].active =
	EpisodeSelect[5] = AT_ENABLED;
#endif

	for (i=0;i<mv_NUM_MOVIES;i++)
		strcat(Movies[i].FName,extension);

#ifdef ACTIVATE_TERMINAL
	strcat(term_com_name,extension);
	strcat(term_msg_name,extension);
#endif

	strcat(configname,extension);
	_fstrcat(SaveName,extension);
	strcat(PageFileName,extension);
	strcat(audioname,extension);
	strcat(demoname,extension);

#if DUAL_SWAP_FILES
	strcat(AltPageFileName,extension);
	ShadowsAvail = (!findfirst(AltPageFileName,&f,FA_ARCH));
#endif
}
#endif


extern const char * MainStrs[];
extern char bc_buffer[];

//------------------------------------------------------------------------
// PreDemo()
//------------------------------------------------------------------------
void PreDemo()
{
#if TECH_SUPPORT_VERSION

	fontnumber=4;
	SETFONTCOLOR(0,15*3);
	CenterWindow (26,7);
	US_Print(EnterBetaCode);
	VW_UpdateScreen();
	CA_LoadAllSounds();
	PM_CheckMainMem();
	SD_PlaySound(INFORMDEATH2SND);		// Nooooo!
	IN_UserInput(TickBase*20);
	ClearMemory();

#elif BETA_TEST

	boolean param=false;

	for (i=1; i<g_argc; i++)
		switch (US_CheckParm(g_argv[i],MainStrs))
		{
			case 13:
				param=true;
			break;
		}

	if (!param)
	{
		char buffer[15] = {0};

		fontnumber=4;
		CenterWindow (26,7);
		US_Print(EnterBetaCode);
		VW_UpdateScreen();
		SETFONTCOLOR(0,15*3);
		US_LineInput(24*8,92,buffer,buffer,true,14,100);
		if (_fstricmp(buffer,bc_buffer))
			Quit("Bad beta code!");
	}
#endif



#if GAME_VERSION == SHAREWARE_VERSION
#if IN_DEVELOPMENT || GEORGE_CHEAT
	if (!MS_CheckParm("nochex"))
#endif
	{
#if  (!SKIP_CHECKSUMS)
//	CheckValidity("MAPTEMP.",MAPTEMP_CHECKSUM,"LEVELS");
	CheckValidity("MAPTEMP.",MAPTEMP_CHECKSUM);
#endif
	}
#else
#if  (!SKIP_CHECKSUMS)
	if (ChecksumFile("FILE_ID.DIZ",0) != DIZFILE_CHECKSUM)
		gamestate.flags |= GS_BAD_DIZ_FILE;
#endif
#endif

	VL_SetPaletteIntensity(0,255,vgapal,0);

	if (!(gamestate.flags & GS_NOWAIT))
	{
#if (0)				// GAME_VERSION != SHAREWARE_VERSION
//---------------------
// Anti-piracy screen
//---------------------
	// Cache pic
	//
		CA_CacheScreen(PIRACYPIC);

	// Cache and set palette.  AND  Fade it in!
	//
		CA_CacheGrChunk(PIRACYPALETTE);
		VL_SetPalette (0,256,grsegs[PIRACYPALETTE]);
		VL_SetPaletteIntensity(0,255,grsegs[PIRACYPALETTE],0);
		VW_UpdateScreen();

		VL_FadeOut (0, 255, 0, 0, 25, 20);
		VL_FadeIn(0,255,grsegs[PIRACYPALETTE],30);

	// Wait a little
	//
		IN_UserInput(TickBase*20);

	// Free palette
	//
		UNCACHEGRCHUNK(PIRACYPALETTE);

		VL_FadeOut (0, 255, 0, 0, 25, 20);
		VW_FadeOut();

	// Cleanup screen for upcoming SetPalette call
	//
		{
		Uint16 old_bufferofs=bufferofs;

		bufferofs=displayofs;
		VL_Bar(0,0,320,200,0);
		bufferofs=old_bufferofs;
		}
#endif

//---------------------
// Apogee presents
//---------------------
	// Cache pic
	//
		CA_CacheScreen(APOGEEPIC);

	// Load and start music
	//
		CA_CacheAudioChunk(STARTMUSIC+APOGFNFM_MUS);

// FIXME
#if 0
		SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+APOGFNFM_MUS]);
#endif // 0

        ::SD_StartMusic(APOGFNFM_MUS);

	// Cache and set palette.  AND  Fade it in!
	//
		CA_CacheGrChunk(APOGEEPALETTE);
		VL_SetPalette (0,256,static_cast<const Uint8*>(grsegs[APOGEEPALETTE]));
		VL_SetPaletteIntensity(0,255,static_cast<const Uint8*>(grsegs[APOGEEPALETTE]),0);
		VW_UpdateScreen();
		VL_FadeOut (0, 255, 25, 29, 53, 20);
		VL_FadeIn(0,255,static_cast<const Uint8*>(grsegs[APOGEEPALETTE]),30);

	// Wait for end of fanfare
	//
		if (MusicMode==smm_AdLib)
		{
			IN_StartAck();
			while ((!sqPlayedOnce) && (!IN_CheckAck()));
      }
		else
			IN_UserInput(TickBase*6);

		SD_MusicOff();

	// Free palette and music.  AND  Restore palette
	//
		UNCACHEGRCHUNK(APOGEEPALETTE);

        free(audiosegs[STARTMUSIC + APOGFNFM_MUS]);
        audiosegs[STARTMUSIC + APOGFNFM_MUS] = NULL;

      // Do A Blue Flash!

      VL_FadeOut (0, 255, 25, 29, 53, 20);
      VL_FadeOut (0, 255, 0, 	0,  0,  30);

//---------------------
// JAM logo intro
//---------------------
	// Load and start music
	//
		CA_CacheAudioChunk(STARTMUSIC+TITLE_LOOP_MUSIC);

// FIXME
#if 0
		SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]);
#endif // 0

        ::SD_StartMusic(TITLE_LOOP_MUSIC);

	// Show JAM logo
	//
		if (!DoMovie(mv_intro,0))
			MAIN_ERROR(PREDEMO_NOJAM);

		if (PowerBall)
      {
      	Sint16 i;

			for (i=0;i<60 && (!DebugOk);i++)
   	   {
      		VL_WaitVBL(1);

				if (Keyboard[sc_LShift] && Keyboard[sc_RShift])
		      {
					CA_LoadAllSounds();

					SD_MusicOff();

// FIXME
#if 0
				   SD_PlaySound(SHOOTDOORSND);
#endif // 0

                   ::sd_play_player_sound(SHOOTDOORSND, bstone::AC_ITEM);

		         SD_WaitSoundDone();

				   ClearMemory();
		      	DebugOk = 1;

					CA_CacheAudioChunk(STARTMUSIC+TITLE_LOOP_MUSIC);

// FIXME
#if 0
					SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]);
#endif // 0

                    ::SD_StartMusic(TITLE_LOOP_MUSIC);
      		}
         }
      }

//---------------------
// PC-13
//---------------------
		VL_Bar(0,0,320,200,0x14);
		VW_MarkUpdateBlock(0,0,319,199);
		CacheDrawPic(0,64,PC13PIC);
		VW_UpdateScreen();
		VW_FadeIn();
		IN_UserInput(TickBase*2);

      // Do A Red Flash!

      VL_FadeOut (0, 255, 39, 0, 0, 20);
		VW_FadeOut();
	}
}

//------------------------------------------------------------------------
// InitGame()
//------------------------------------------------------------------------
void InitGame (void)
{
	Sint16                     i,x,y;
	Uint16        *blockstart;
//long mmsize;

	MM_Startup ();                  // so the signon screen can be freed
#if IN_DEVELOPMENT || GEORGE_CHEAT || SHOW_CHECKSUM
	if (MS_CheckParm("checksum"))
	{
		ShowChecksums();
		MM_Shutdown();
		exit(0);
	}
#endif
	CA_Startup ();

// Any problems with this version of the game?
//
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
	if (!MS_CheckParm("nochex"))
#endif

#if !SKIP_CHECKSUMS
		CheckValidity("MAPTEMP.",MAPTEMP_CHECKSUM);

#if GAME_VERSION != SHAREWARE_VERSION
	if (ChecksumFile("FILE_ID.DIZ",0) != DIZFILE_CHECKSUM)
		gamestate.flags |= GS_BAD_DIZ_FILE;
#endif
#endif

	VL_SetVGAPlaneMode ();
	VL_SetPalette (0,256,vgapal);

	VW_Startup ();
	IN_Startup ();
	PM_Startup ();
	SD_Startup ();
	US_Startup ();

	if (CheckForSpecialCode(POWERBALLTEXT))
#if IN_DEVELOPMENT
		DebugOk = true;
#else
		PowerBall = true;
#endif

	if (CheckForSpecialCode(TICSTEXT))
		gamestate.flags |= GS_TICS_FOR_SCORE;

	if (CheckForSpecialCode(MUSICTEXT))
		gamestate.flags |= GS_MUSIC_TEST;

	if (CheckForSpecialCode(RADARTEXT))
		gamestate.flags |= GS_SHOW_OVERHEAD;

// FIXME
#if 0
//mmsize = mminfo.mainmem;

#if (!IN_DEVELOPMENT)
	if (mminfo.mainmem < MIN_MEM_NEEDED)
	{
	 void* screen;
	 CA_CacheGrChunk (ERRORSCREEN);
	 screen = grsegs[ERRORSCREEN];
	 ShutdownId();

	 movedata ((Uint16)screen,7+7*160,0xb800,0,17*160);
	 gotoxy (1,23);

	 exit(0);
	}
#endif
#endif // 0

#if IN_DEVELOPMENT
	//
	// Clear Monocrome
	//
	_fmemset(MK_FP(0xb000,0x0000),0,4000);
#endif

//
// build some tables
//
	InitDigiMap ();

	for (i=0;i<MAPSIZE;i++)
	{
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;
		farmapylookup[i] = i*64;
	}

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	updateptr = &update[0];

	bufferofs = 0;
	displayofs = 0;
	ReadConfig ();

//
// draw intro screen stuff
//
//	if (!(gamestate.flags & GS_QUICKRUN))
//		IntroScreen ();

//
// load in and lock down some basic chunks
//

	LoadFonts();

	LoadLatchMem ();
	BuildTables ();          // trig tables
	SetupWalls ();
	NewViewSize (viewsize);

//
// initialize variables
//
	InitRedShifts ();
}

//-------------------------------------------------------------------------
// ShowSystem()
//-------------------------------------------------------------------------
void ShowSystem()
{
	char avail[2][8]={"None","Present"};

	MM_Startup ();
	CA_Startup ();
	IN_Startup ();
	PM_Startup ();
	SD_Startup ();

	fprint(show_text1);
	fprint(show_text2);
	fprint(show_text3);

// FIXME
#if 0
	printf("     MAIN: %ld",mminfo.nearheap+mminfo.farheap);
	if (mminfo.nearheap+mminfo.farheap < MIN_MEM_NEEDED)
		fprint(show_text4);
	printf("\n");
	printf("      EMS: %ld\n",4l*EMSPagesAvail*1024l);
	printf("      XMS: %ld\n\n",4l*XMSPagesAvail*1024l);
#endif // 0

	fprint(show_text5);

	printf("        Mouse: %s\n",avail[MousePresent]);
	printf("     Joystick: %s\n",avail[JoysPresent[0]||JoysPresent[1]]);
	printf("        AdLib: %s\n",avail[AdLibPresent&&!SoundBlasterPresent]);
	printf("Sound Blaster: %s\n",avail[SoundBlasterPresent]);
	printf(" Sound Source: %s\n\n",avail[SoundSourcePresent]);
	fprint(show_text2);

	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ();
}

//-------------------------------------------------------------------------
// scan_atoi()
//-------------------------------------------------------------------------
Uint16 scan_atoi(char *s)
{
	while (*s && (!isdigit(*s)))			// First scans for a digit...
		s++;

	return(static_cast<Uint16>(atoi(s)));							// Then converts to integer...
}

extern const char * MainStrs[];
extern Sint16 starting_episode,starting_level,starting_difficulty;


//-------------------------------------------------------------------------
// freed_main()
//-------------------------------------------------------------------------
void freed_main()
{
	Sint16     i;

// FIXME
#if 0
	struct dosdate_t d;
#endif // 0

// Setup for APOGEECD thingie.
//
	InitDestPath();

// Make sure there's room to play the game
//
	CheckDiskSpace(DISK_SPACE_NEEDED,CANT_PLAY_TXT,cds_dos_print);

// Which version is this? (SHAREWARE? 1-3? 1-6?)
//
	CheckForEpisodes();

	Patch386();

	for (i=1; i<g_argc; i++)
		switch (US_CheckParm(g_argv[i],MainStrs))
		{
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
			case 0:											// quick run
				gamestate.flags |= GS_QUICKRUN;

			case 1:											// no wait
				gamestate.flags |= GS_NOWAIT;
			break;
#endif

			case 2:											// starting level
				gamestate.flags |= GS_STARTLEVEL;
				starting_level=scan_atoi(g_argv[i]);
			break;

			case 3:
				gamestate.flags |= GS_STARTLEVEL;
				starting_episode=scan_atoi(g_argv[i])-1;
			break;

			case 4:
				fprint(cinfo_text);

				printf("\n"
						 "     Version: %s\n"
						 "COMPILE DATE: %s\n\n",
						 __BLAKE_VERSION__,__DATE__);
				exit(0);
			break;

			case 5:
				ShowSystem();
				exit(0);
			break;

#if IN_DEVELOPMENT
#ifdef DEBUG_VALUE
			case 6:
				debug_value=scan_atoi(g_argv[i]);
			break;
#endif
#endif

			case 7:
				gamestate.flags |= GS_TICS_FOR_SCORE;
			break;

			case 8:
//				gamestate.flags |= GS_MEM_FOR_SCORE;
			break;

			case 9:
				PowerBall = 1;
			break;

			case 11:
				gamestate.flags |= GS_STARTLEVEL;
				starting_difficulty=scan_atoi(g_argv[i])-1;
			break;

			case 10:
				gamestate.flags |= GS_MUSIC_TEST;
			break;

			case 12:
				gamestate.flags |= GS_SHOW_OVERHEAD;
			break;
		}


#if BETA_TEST
	//
	// THIS IS FOR BETA ONLY!
	//

	_dos_getdate(&d);
	if ((d.year > BETA_YEAR) ||
		 ((d.year == BETA_YEAR) && (d.month > BETA_MONTH)) ||
		 ((d.year == BETA_YEAR) && (d.month == BETA_MONTH) && (d.day >= BETA_DAY)))
	{
		FILE *out;
		char name[20]="VSWAP.";

		strcat(name,extension);
		out = fopen(name,"w");
		fprintf(out,"\n\n SELF DESTRUCTED \n");
		fclose(out);
		remove("vswap.bs1");
		fprint(dver_text);
		exit(0);
	}
#endif

	InitGame ();

	if (!IsA386)
		Quit("");

	bufferofs=SCREENSIZE;

// FIXME Enable later
#if 0
	PreDemo();
#endif // 0
}


//-------------------------------------------------------------------------
// CheckValidity()
//-------------------------------------------------------------------------
void CheckValidity(char *file, Sint32 valid_checksum)
{
	char filename[13];
	Sint32 checksum;

	if (strlen(file) > 9)
		MAIN_ERROR(CHECK_FILENAME_TOO_LONG);

	strcpy(filename,file);
	strcat(filename,extension);

	checksum=ChecksumFile(filename,0);
	if (checksum != valid_checksum)
#if GAME_VERSION != SHAREWARE_VERSION
		if (strstr(file,"MAP"))
			InvalidLevels();
#else
			BadChecksum();
#endif
}

#define CFC_BUFFERSIZE 65535

void* cfc_buffer;

//-------------------------------------------------------------------------
// ChecksumFile()
//-------------------------------------------------------------------------
Sint32 ChecksumFile(char *file, Sint32 checksum)
{
	#define JUMPSIZE 8

	bstone::FileStream handle;
	Sint32 size,readlen,i;
	char *p;

    cfc_buffer = malloc(CFC_BUFFERSIZE);
	p=static_cast<char*>(cfc_buffer);

    handle.open(file);

	if (!handle.is_open())
	{
		checksum=0;
		goto exit_func;
	}

	size=static_cast<Sint32>(handle.get_size());
	while (size)
	{
		if (size >= CFC_BUFFERSIZE)
			readlen = CFC_BUFFERSIZE;
		else
			readlen = size;

		handle.read(cfc_buffer,readlen);

		for (i=0;i<readlen-JUMPSIZE;i += JUMPSIZE)
			checksum += p[i]^p[i+1];

		size -= readlen;
	}

exit_func:;

    free(cfc_buffer);
    cfc_buffer = NULL;

	return(checksum);
}

#if IN_DEVELOPMENT || GEORGE_CHEAT || SHOW_CHECKSUM


//-------------------------------------------------------------------------
// ShowChecksums()
//-------------------------------------------------------------------------
void ShowChecksums()
{
#if GAME_VERSION == SHAREWARE_VERSION
	char mapname[23]="SWMAPS\\MAPTEMP.";
#elif GAME_VERSION == MISSIONS_1_THR_3
	char mapname[23]="1_3MAPS\\MAPTEMP.";
#else
	char mapname[13]="MAPTEMP.";
#endif
	char gfxname[13]="VGAGRAPH.";
	char audioname[13]="AUDIOT.";
	char dizname[]="FILE_ID.DIZ";

	strcat(gfxname,extension);
	strcat(mapname,extension);
	strcat(audioname,extension);

	printf("\n");
	printf("%s: %lx\n",audioname,ChecksumFile(audioname,0));
	printf("%s: %lx\n",mapname,ChecksumFile(mapname,0));
	printf("%s: %lx\n",gfxname,ChecksumFile(gfxname,0));
	printf("%s: %lx\n",dizname,ChecksumFile(dizname,0));
	printf("\n");
}


#endif

#if GAME_VERSION != SHAREWARE_VERSION

//-------------------------------------------------------------------------
// InvalidLevels()
//-------------------------------------------------------------------------
void InvalidLevels()
{
	char *chunkptr;

	CA_CacheGrChunk(BADLEVELSTEXT);
	chunkptr = static_cast<char*>(grsegs[BADLEVELSTEXT]);
	*(strstr(chunkptr,"^XX"))=0;

	fprint(chunkptr);

// FIXME
#if 0
	while (!bioskey(1));
	bioskey(0);
#endif // 0

	UNCACHEGRCHUNK(BADLEVELSTEXT);
}

#endif

#if GAME_VERSION == SHAREWARE_VERSION

//-------------------------------------------------------------------------
// BadChecksum()
//-------------------------------------------------------------------------
void BadChecksum()
{
	char *chunkptr;

	CA_CacheGrChunk(BADCHECKSUMTEXT);
	chunkptr = grsegs[BADCHECKSUMTEXT];
	*(_fstrstr(chunkptr,"^XX")) = 0;

	CA_Shutdown();
	MM_Shutdown();

	fprint(chunkptr);

	exit(0);
}

#endif


#if FREE_FUNCTIONS

// This function is used as a label for the end of the
// functions used by the memory manager.
//
void JM_FREE_END()
{
}

#endif
