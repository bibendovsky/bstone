//
//	ID Engine
//	ID_SD.c - Sound Manager for Wolfenstein 3D
//	v1.3 (revised for **********, screwed with for Blake Stone)
//	By Jason Blochowiak
//

//
//	This module handles dealing with generating sound on the appropriate
//		hardware
//
//	Depends on: User Mgr (for parm checking)
//
//	Globals:
//		For User Mgr:
//			SoundSourcePresent - Sound Source thingie present?
//			SoundBlasterPresent - SoundBlaster card present?
//			AdLibPresent - AdLib card present?
//			SoundMode - What device is used for sound effects
//				(Use SM_SetSoundMode() to set)
//			MusicMode - What device is used for music
//				(Use SM_SetMusicMode() to set)
//			DigiMode - What device is used for digitized sound effects
//				(Use SM_SetDigiDevice() to set)
//
//		For Cache Mgr:
//			NeedsDigitized - load digitized sounds?
//			NeedsMusic - load music?
//

#ifdef MSVC
#pragma hdrstop		// Wierdo thing with MUSE
#endif

#ifdef	_MUSE_      // Will be defined in ID_Types.h
#include "id_sd.h"
#else
#include "id_heads.h"
#endif

#ifdef MSVC
#pragma	hdrstop
#endif

// BBi
#include "3d_def.h"

#include "bstone_audio_mixer.h"
#include "bstone_memory_binary_reader.h"

#ifdef	nil
#undef	nil
#endif
#define	nil	0

#define	SDL_SoundFinished()	{SoundNumber = HITWALLSND; SoundPriority = 0;}

// Macros for SoundBlaster stuff
// FIXME
#if 0
#define	sbOut(n,b)				outportb((n) + sbLocation,b)
#define	sbIn(n)					inportb((n) + sbLocation)
#endif // 0

#define sbOut(n,b)
#define sbIn(n) (0)


#define	sbSimpleWriteDelay()	while (sbIn(sbWriteStat) & 0x80);
#define	sbReadDelay()			while (sbIn(sbDataAvail) & 0x80);

// Macros for AdLib stuff
// FIXME
#if 0
#define	selreg(n)	outportb(alFMAddr,n)
#define	writereg(n)	outportb(alFMData,n)
#define	readstat()	inportb(alFMStatus)
#endif // 0

#define selreg(n)
#define writereg(n)
#define readstat()


//	Imports from ID_SD_A.ASM
extern	void			SDL_SetDS(void);
extern	void SDL_t0FastAsmService(void),
						SDL_t0SlowAsmService(void);

//	Imports from ID_SDD.C
#undef	NUMSOUNDS
#undef	NUMSNDCHUNKS
#undef	STARTPCSOUNDS
#undef	STARTADLIBSOUNDS
#undef	STARTDIGISOUNDS
#undef	STARTMUSIC

extern	Uint16	sdStartPCSounds;
extern	Uint16	sdStartALSounds;
extern	Sint16		sdLastSound;
extern	Sint16		DigiMap[];

extern int g_argc;
extern char** g_argv;

//	Global variables
	boolean		SoundSourcePresent,
				AdLibPresent,
				SoundBlasterPresent,SBProPresent,
				NeedsDigitized,NeedsMusic,
				SoundPositioned;
	SDMode		SoundMode;
	SMMode		MusicMode;
	SDSMode		DigiMode;
	volatile Uint32	TimeCount;
	Uint16		HackCount;

// FIXME
#if 0
	Uint16		*SoundTable;	// Really * _seg *SoundTable, but that don't work
#endif // 0

    Uint8** SoundTable;

	boolean		ssIsTandy;
	Uint16		ssPort = 2;

//	Internal variables
static	boolean			SD_Started;
		boolean			nextsoundpos;
		Uint32		TimerDivisor,TimerCount;
static	const char * 	ParmStrings[] =
						{
							"noal",
							"nosb",
							"nopro",
							"noss",
							"sst",
							"ss1",
							"ss2",
							"ss3",
							nil
						};
static	void			(*SoundUserHook)(void);
		soundnames		SoundNumber,DigiNumber;
		Uint16			SoundPriority,DigiPriority;
		Sint16				LeftPosition,RightPosition;
		void (*t0OldService)(void);
		Sint32			LocalTime;
		Uint16			TimerRate;

        Uint16				NumDigi;
		Uint16				*DigiList;

// FIXME
#if 0
		Uint16				DigiLeft,DigiPage;
		Uint16				DigiLastStart,DigiLastEnd;
		boolean	DigiPlaying;
static	boolean	DigiMissed,DigiLastSegment;
static	void*		DigiNextAddr;
static	Uint16		DigiNextLen;
		Uint32	DigiFailSafe;
		Uint32			DigiFailTriggered;
#endif // 0

//	SoundBlaster variables
static	boolean					sbNoCheck,sbNoProCheck;

// FIXME
#if 0
		Uint16					SBResetCount;
static	volatile boolean		sbSamplePlaying;
static	Uint8					sbPIC1Mask,sbPIC2Mask;
static	Uint8					sbOldIntMask = -1,sbOldIntMask2 = -1;
static	volatile Uint8			*sbNextSegPtr;
static	Uint8					sbDMA = 1,
								sbDMAa1 = 0x83,sbDMAa2 = 2,sbDMAa3 = 3,
								sba1Vals[] = {0x87,0x83,0,0x82},
								sba2Vals[] = {0,2,0,6},
								sba3Vals[] = {1,3,0,7};
static	Sint16						sbLocation = -1,sbInterrupt = 7,sbIntVec = 0xf,
								sbIntVectors[] = {-1,-1,0xa,0xb,-1,0xd,-1,0xf,-1,-1,0x72};
static	volatile Uint8			sbLastTimeValue;
static	volatile Uint32		sbNextSegLen;
static	volatile SampledSound	*sbSamples;
static	void 		(*sbOldIntHand)(void);
static	Uint8					sbpOldFMMix,sbpOldVOCMix;
#endif // 0

//	SoundSource variables
		boolean				ssNoCheck;

// FIXME
#if 0
		boolean				ssActive;
		Uint16				ssControl,ssStatus,ssData;
		Uint8				ssOn,ssOff;
		Uint16				ssVol;
		Uint8				ssVolTable[256];
		volatile Uint8		*ssSample;
		volatile Uint32	ssLengthLeft;
#endif // 0

//	PC Sound variables
// FIXME
#if 0
		volatile Uint8	pcLastSample,*pcSound;
		Uint32		pcLengthLeft;
		Uint16			pcSoundLookup[255];
#endif // 0

//	AdLib variables
		boolean			alNoCheck;

// FIXME
#if 0
		Uint8			*alSound;
		Uint16			alBlock;
		Uint32		alLengthLeft;
		Uint32		alTimeCount;
		Instrument		alZeroInst;

// This table maps channel numbers to carrier and modulator op cells
static	Uint8			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
						pmodifiers[5] = {16,17,18,20,21};

//	Sequencer variables
static	Uint16			alFXReg;
static	ActiveTrack		*tracks[sqMaxTracks],
						mytracks[sqMaxTracks];
static	Uint16			sqMode,sqFadeStep;
#endif // 0

        boolean sqActive;
        Uint16* sqHack;
        Uint16 sqHackLen;

// FIXME
#if 0
        Uint16* sqHackPtr;
        Uint16 sqHackSeqLen;
		Sint32			sqHackTime;
#endif // 0

		boolean			sqPlayedOnce;

//	Internal routines
		void			SDL_DigitizedDone(void);

// BBi
static int music_index = -1;
static bstone::AudioMixer mixer;


///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetTimer0() - Sets system timer 0 to the specified speed
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_SetTimer0(Uint16 speed)
{
#ifndef TPROF	// If using Borland's profiling, don't screw with the timer
asm	pushf
asm	cli

	outportb(0x43,0x36);				// Change timer 0
	outportb(0x40,speed);
	outportb(0x40,speed >> 8);
	// Kludge to handle special case for digitized PC sounds
	if (TimerDivisor == (1192030 / (TickBase * 100)))
		TimerDivisor = (1192030 / (TickBase * 10));
	else
		TimerDivisor = speed;

asm	popf
#else
	TimerDivisor = 0x10000;
#endif
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetIntsPerSec() - Uses SDL_SetTimer0() to set the number of
//		interrupts generated by system timer 0 per second
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_SetIntsPerSec(Uint16 ints)
{
	TimerRate = ints;
	SDL_SetTimer0(1192030 / ints);
}
#endif // 0

// FIXME
#if 0
static void
SDL_SetTimerSpeed(void)
{
	Uint16	rate;
	void (*isr)(void);

	if
	(
		(MusicMode == smm_AdLib)
	||	((DigiMode == sds_SoundSource) && DigiPlaying)
	)
	{
		rate = TickBase * 10;
		isr = SDL_t0FastAsmService;
	}
	else
	{
		rate = TickBase * 2;
		isr = SDL_t0SlowAsmService;
	}

	if (rate != TimerRate)
	{
		setvect(8,isr);
		SDL_SetIntsPerSec(rate);
		TimerRate = rate;
	}
}
#endif // 0

//
//	SoundBlaster code
//

///////////////////////////////////////////////////////////////////////////
//
//	sbWriteDelay() - Waits for the card to become ready. If it doesn't
//		become ready, we reset the card, and reprogram the last time value.
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
boolean
sbWriteDelay(void)
{
	int	i;

	// Try to avoid hitting the card while it's doing a DMA transfer
	for (i = 0;i < 256;i++)
		if (sbIn(sbWriteStat) & 0x80)
			break;
	// See if the DSP is available. If it doesn't go active after 1000
	// cycles, assume it hung, and reset the damn thing.
	for (i = 0;i < 1000;i++)
		if (!(sbIn(sbWriteStat) & 0x80))
			break;
	if (i == 1000)								// Assume that the DSP hung
	{
		SBResetCount++;							// Increment our counter

		sbOut(sbReset,true);					// Reset the SoundBlaster DSP
		for (i = 0;i < 9;i++)					// Wait >4usec
			inportb(alFMStatus);

		sbOut(sbReset,false);					// Turn off sb DSP reset
		for (i = 0;i < 100;i++)					// Wait >100usec
			inportb(alFMStatus);

		sbSimpleWriteDelay();
		sbOut(sbWriteCmd,0xd1);					// Turn on DSP speaker

		sbSimpleWriteDelay();					// Reprogram last time value
		sbOut(sbWriteCmd,0x40);
		sbSimpleWriteDelay();
		sbOut(sbWriteData,sbLastTimeValue);

		return(true);
	}
	else
		return(false);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_EnableDMAInt() - Save old interrupt status and unmask DMA interrupt
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SDL_EnableDMAInt(void)
{
	sbOldIntMask = inportb(0x21);
	outportb(0x21,sbOldIntMask & ~sbPIC1Mask);

	if (sbInterrupt >= 8)
	{
		sbOldIntMask2 = inportb(0xa1);
		outportb(0xa1,sbOldIntMask2 & ~sbPIC2Mask);
	}
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_RestoreDMAInt() - Restore DMA interrupt mask bit(s)
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SDL_RestoreDMAInt(void)
{
	Uint8	is;

	is = inportb(0x21);
	if (sbOldIntMask & sbPIC1Mask)
		is |= sbPIC1Mask;
	else
		is &= ~sbPIC1Mask;
	outportb(0x21,is);

	if (sbInterrupt >= 8)
	{
		is = inportb(0xa1);
		if (sbOldIntMask2 & sbPIC2Mask)
			is |= sbPIC2Mask;
		else
			is &= ~sbPIC2Mask;
		outportb(0xa1,is);
	}
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBStopSample() - Stops any active sampled sound and causes DMA
//		requests from the SoundBlaster to cease
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBStopSample(void)
{
	Uint8	is;
	int		i;

asm	pushf
asm	cli

	if (sbSamplePlaying)
	{
		sbSamplePlaying = false;

		sbWriteDelay();
		sbOut(sbWriteCmd,0xd0);	// Turn off DSP DMA
		for (i = 0;i < 256;i++)	// was 80/256
			sbIn(sbWriteStat);

#if 0
		is = inportb(0x21);	// Restore interrupt mask bit
		if (sbOldIntMask & (1 << sbInterrupt))
			is |= (1 << sbInterrupt);
		else
			is &= ~(1 << sbInterrupt);
		outportb(0x21,is);
#endif
	}

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySeg() - Plays a chunk of sampled sound on the SoundBlaster
//	Insures that the chunk doesn't cross a bank boundary, programs the DMA
//	 controller, and tells the SB to start doing DMA requests for DAC
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static Uint32
SDL_SBPlaySeg(volatile Uint8 *data,Uint32 length)
{
	unsigned		datapage;
	Uint32		dataofs,uselen;

	uselen = length;
	datapage = FP_SEG(data) >> 12;
	dataofs = ((FP_SEG(data) & 0xfff) << 4) + FP_OFF(data);
	if (dataofs >= 0x10000)
	{
		datapage++;
		dataofs -= 0x10000;
	}

	if (dataofs + uselen > 0x10000)
		uselen = 0x10000 - dataofs;

	uselen--;

	// Program the DMA controller
asm	pushf
asm	cli
	outportb(0x0a,sbDMA | 4);					// Mask off DMA on channel sbDMA
	outportb(0x0c,0);							// Clear byte ptr flip-flop to lower byte
	outportb(0x0b,sbDMA | 0x48);				// Set transfer mode for D/A conv (Single, address increment, read)
	outportb(sbDMAa2,(Uint8)dataofs);			// Give LSB of address
	outportb(sbDMAa2,(Uint8)(dataofs >> 8));		// Give MSB of address
	outportb(sbDMAa1,(Uint8)datapage);			// Give page of address
	outportb(sbDMAa3,(Uint8)uselen);				// Give LSB of length
	outportb(sbDMAa3,(Uint8)(uselen >> 8));		// Give MSB of length
	outportb(0x0a,sbDMA);						// Re-enable DMA on channel sbDMA

	// Start playing the thing
	sbWriteDelay();
	sbOut(sbWriteCmd,0x14);
	sbWriteDelay();
	sbOut(sbWriteData,(Uint8)uselen);
	sbWriteDelay();
	sbOut(sbWriteData,(Uint8)(uselen >> 8));
asm	popf

	return(uselen + 1);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBService() - Services the SoundBlaster DMA interrupt
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_SBService(void)
{
	Uint32	used;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	sbIn(sbDataAvail);	// Ack interrupt to SB

	if (sbNextSegPtr)
	{
		used = SDL_SBPlaySeg(sbNextSegPtr,sbNextSegLen);
		if (sbNextSegLen <= used)
			sbNextSegPtr = nil;
		else
		{
			sbNextSegPtr += used;
			sbNextSegLen -= used;
		}
	}
	else
	{
//		SDL_SBStopSample();
		sbSamplePlaying = false;
		SDL_DigitizedDone();
	}

	outportb(0x20,0x20);	// Ack interrupt
	if (sbInterrupt >= 8)	// If necessary, ack to cascade PIC
		outportb(0xa0,0x20);

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySample() - Plays a sampled sound on the SoundBlaster. Sets up
//		DMA to play the sound
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBPlaySample(Uint8 *data,Uint32 len)
{
	Uint32	used;

	SDL_SBStopSample();

asm	pushf
asm	cli

	used = SDL_SBPlaySeg(data,len);
	if (len <= used)
		sbNextSegPtr = nil;
	else
	{
		sbNextSegPtr = data + used;
		sbNextSegLen = len - used;
	}

#if 0
	// Save old interrupt status and unmask ours
	sbOldIntMask = inportb(0x21);
	outportb(0x21,sbOldIntMask & ~(1 << sbInterrupt));
#endif

#if 0
	sbWriteDelay();
	sbOut(sbWriteCmd,0xd4);						// Make sure DSP DMA is enabled
#endif

	sbSamplePlaying = true;

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PositionSBP() - Sets the attenuation levels for the left and right
//		channels by using the mixer chip on the SB Pro. This hits a hole in
//		the address map for normal SBs.
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_PositionSBP(Sint16 leftpos,Sint16 rightpos)
{
	Uint8	v;

	if (!SBProPresent)
		return;

	leftpos = 15 - leftpos;
	rightpos = 15 - rightpos;
	v = ((leftpos & 0x0f) << 4) | (rightpos & 0x0f);

asm	pushf
asm	cli

	sbOut(sbpMixerAddr,sbpmVoiceVol);
	sbOut(sbpMixerData,v);

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSB() - Checks to see if a SoundBlaster resides at a
//		particular I/O location
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static boolean
SDL_CheckSB(Sint16 port)
{
	int	i;

	sbLocation = port << 4;		// Initialize stuff for later use

	sbOut(sbReset,true);		// Reset the SoundBlaster DSP
asm	mov	dx,alFMStatus			// Wait >4usec
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx

	sbOut(sbReset,false);		// Turn off sb DSP reset
asm	mov	dx,alFMStatus			// Wait >100usec
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop

	for (i = 0;i < 100;i++)
	{
		if (sbIn(sbDataAvail) & 0x80)		// If data is available...
		{
			if (sbIn(sbReadData) == 0xaa)	// If it matches correct value
				return(true);
			else
			{
				sbLocation = -1;			// Otherwise not a SoundBlaster
				return(false);
			}
		}
	}

	sbLocation = -1;						// Retry count exceeded - fail
	return(false);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	Checks to see if a SoundBlaster is in the system. If the port passed is
//		-1, then it scans through all possible I/O locations. If the port
//		passed is 0, then it uses the default (2). If the port is >0, then
//		it just passes it directly to SDL_CheckSB()
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static boolean
SDL_DetectSoundBlaster(Sint16 port)
{
	Sint16	i;

	if (port == 0)					// If user specifies default, use 2
		port = 2;
	if (port == -1)
	{
		if (SDL_CheckSB(2))			// Check default before scanning
			return(true);

		if (SDL_CheckSB(4))			// Check other SB Pro location before scan
			return(true);

		for (i = 1;i <= 6;i++)		// Scan through possible SB locations
		{
			if ((i == 2) || (i == 4))
				continue;

			if (SDL_CheckSB(i))		// If found at this address,
				return(true);		//	return success
		}
		return(false);				// All addresses failed, return failure
	}
	else
		return(SDL_CheckSB(port));	// User specified address or default
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBSetDMA() - Sets the DMA channel to be used by the SoundBlaster
//		code. Sets up sbDMA, and sbDMAa1-sbDMAa3 (used by SDL_SBPlaySeg()).
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SDL_SBSetDMA(Uint8 channel)
{
	if (channel > 3)
		SD_ERROR(SD_STARTUP_BAD_DMA);

	sbDMA = channel;
	sbDMAa1 = sba1Vals[channel];
	sbDMAa2 = sba2Vals[channel];
	sbDMAa3 = sba3Vals[channel];
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSB() - Turns on the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_StartSB(void)
{
	Uint8	timevalue,test;

	sbIntVec = sbIntVectors[sbInterrupt];
	if (sbIntVec < 0)
		SD_ERROR(SD_STARTUP_BAD_INTERRUPT);

	sbPIC1Mask = 1 << (sbInterrupt & 7);
	sbPIC2Mask = 1 << 2;

	sbOldIntHand = getvect(sbIntVec);	// Get old interrupt handler
	setvect(sbIntVec,SDL_SBService);	// Set mine

	SDL_EnableDMAInt();					// Enable DMA interrupt

	sbWriteDelay();
	sbOut(sbWriteCmd,0xd1);				// Turn on DSP speaker

	// Set the SoundBlaster DAC time constant for 7KHz
	timevalue = 256 - (1000000 / 7000);
	sbLastTimeValue = timevalue;
	sbWriteDelay();
	sbOut(sbWriteCmd,0x40);
	sbWriteDelay();
	sbOut(sbWriteData,timevalue);

	SBProPresent = false;
	if (sbNoProCheck)
		return;

	// Check to see if this is a SB Pro
	sbOut(sbpMixerAddr,sbpmFMVol);
	sbpOldFMMix = sbIn(sbpMixerData);
	sbOut(sbpMixerData,0xbb);
	test = sbIn(sbpMixerData);
	if (test == 0xbb)
	{
		// Boost FM output levels to be equivilent with digitized output
		sbOut(sbpMixerData,0xff);
		test = sbIn(sbpMixerData);
		if (test == 0xff)
		{
			SBProPresent = true;

			// Save old Voice output levels (SB Pro)
			sbOut(sbpMixerAddr,sbpmVoiceVol);
			sbpOldVOCMix = sbIn(sbpMixerData);

			// Turn SB Pro stereo DAC off
			sbOut(sbpMixerAddr,sbpmControl);
			sbOut(sbpMixerData,0);				// 0=off,2=on
		}
	}
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSB() - Turns off the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_ShutSB(void)
{
	SDL_SBStopSample();
	SDL_RestoreDMAInt();				// Restore DMA interrupt

	if (SBProPresent)
	{
		// Restore FM output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmFMVol);
		sbOut(sbpMixerData,sbpOldFMMix);

		// Restore Voice output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmVoiceVol);
		sbOut(sbpMixerData,sbpOldVOCMix);
	}

	setvect(sbIntVec,sbOldIntHand);		// Set vector back
}
#endif // 0

//	Sound Source Code

// FIXME
#if 0
void
SDL_SSSetVol(Uint16 vol)
{
	Sint16		i;
	Sint32	v;

	if (vol == ssVol)
		return;

	ssVol = vol;
#if 1
	v = vol * -128;
	for (i = 0;i < 256;i++)
	{
		ssVolTable[i] = (v >> 8) + 0x80;
		v += vol;
	}
#else
	for (i = 0;i < 256;i++)
		ssVolTable[i] = (((i - 0x80) * vol) >> 8) + 0x80;
#endif
}
#endif // 0

// FIXME
#if 0
static void
SDL_PositionSS(Sint16 leftpos,Sint16 rightpos)
{
	Sint16	pos;

	pos = (leftpos > rightpos)? rightpos : leftpos;
	pos = 15 - pos;
	pos *= 17;
	SDL_SSSetVol(pos);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSStopSample() - Stops a sample playing on the Sound Source
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSStopSample(void)
{
asm	pushf
asm	cli

	(Sint32)ssSample = 0;

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSService() - Handles playing the next sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_SSService(void)
{
	boolean	gotit;
	Uint8	v;

	while (ssSample)
	{
	asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
	asm	in		al,dx
	asm	test	al,0x40
	asm	jnz		done			// Nope - don't push any more data out

		v = *ssSample++;
		v = ssVolTable[v];
		if (!(--ssLengthLeft))
		{
			(Sint32)ssSample = 0;
			SDL_DigitizedDone();
		}

	asm	mov		dx,[ssData]		// Pump the value out
	asm	mov		al,[v]
	asm	out		dx,al

	asm	mov		dx,[ssControl]	// Pulse printer select
	asm	mov		al,[ssOff]
	asm	out		dx,al
	asm	push	ax
	asm	pop		ax
	asm	mov		al,[ssOn]
	asm	out		dx,al

	asm	push	ax				// Delay a short while
	asm	pop		ax
	asm	push	ax
	asm	pop		ax
	}
done:;
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSPlaySample() - Plays the specified sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSPlaySample(Uint8 *data,Uint32 len)
{
asm	pushf
asm	cli

	ssLengthLeft = len;
	ssSample = (volatile Uint8 *)data;

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSS() - Sets up for and turns on the Sound Source
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_StartSS(void)
{
	if (ssPort == 3)
		ssControl = 0x27a;	// If using LPT3
	else if (ssPort == 2)
		ssControl = 0x37a;	// If using LPT2
	else
		ssControl = 0x3be;	// If using LPT1
	ssStatus = ssControl - 1;
	ssData = ssStatus - 1;

	ssOn = 0x04;
	if (ssIsTandy)
		ssOff = 0x0e;				// Tandy wierdness
	else
		ssOff = 0x0c;				// For normal machines

	outportb(ssControl,ssOn);		// Enable SS

	SDL_SSSetVol(0x100);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSS() - Turns off the Sound Source
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_ShutSS(void)
{
	outportb(ssControl,ssOff);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSS() - Checks to see if a Sound Source is present at the
//		location specified by the sound source variables
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static boolean
SDL_CheckSS(void)
{
	boolean		present = false;

	Uint32	lasttime;

	// Turn the Sound Source on and wait awhile (4 ticks)
	SDL_StartSS();

	lasttime = TimeCount;
	while (TimeCount < lasttime + 4)
		;

asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
asm	in		al,dx
asm	test	al,0x40
asm	jnz		checkdone		// Nope - Sound Source not here

asm	mov		cx,32			// Force FIFO overflow (FIFO is 16 bytes)
outloop:
asm	mov		dx,[ssData]		// Pump a neutral value out
asm	mov		al,0x80
asm	out		dx,al

asm	mov		dx,[ssControl]	// Pulse printer select
asm	mov		al,[ssOff]
asm	out		dx,al
asm	push	ax
asm	pop		ax
asm	mov		al,[ssOn]
asm	out		dx,al

asm	push	ax				// Delay a short while before we do this again
asm	pop		ax
asm	push	ax
asm	pop		ax

asm	loop	outloop

asm	mov		dx,[ssStatus]	// Is FIFO overflowed now?
asm	in		al,dx
asm	test	al,0x40
asm	jz		checkdone		// Nope, still not - Sound Source not here

	present = true;			// Yes - it's here!

checkdone:

	SDL_ShutSS();
	return(present);
}
#endif // 0

// FIXME
#if 0
static boolean
SDL_DetectSoundSource(void)
{
	for (ssPort = 1;ssPort <= 3;ssPort++)
		if (SDL_CheckSS())
			return(true);
	return(false);
}
#endif // 0

//
//	PC Sound code
//

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCPlaySound() - Plays the specified sound on the PC speaker
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCPlaySound(PCSound *sound)
{
asm	pushf
asm	cli

	pcLastSample = -1;
	pcLengthLeft = sound->common.length;
	pcSound = sound->data;

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCStopSound() - Stops the current sound playing on the PC Speaker
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCStopSound(void)
{
asm	pushf
asm	cli

	(Sint32)pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker off
asm	and	al,0xfd			// ~2
asm	out	0x61,al

asm	popf

    pcSound = NULL;
}
#endif // 0

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCService() - Handles playing the next sample in a PC sound
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PCService(void)
{
	Uint8	s;
	Uint16	t;

	if (pcSound)
	{
		s = *pcSound++;
		if (s != pcLastSample)
		{
		asm	pushf
		asm	cli

			pcLastSample = s;
			if (s)					// We have a frequency!
			{
				t = pcSoundLookup[s];
			asm	mov	bx,[t]

			asm	mov	al,0xb6			// Write to channel 2 (speaker) timer
			asm	out	43h,al
			asm	mov	al,bl
			asm	out	42h,al			// Low byte
			asm	mov	al,bh
			asm	out	42h,al			// High byte

			asm	in	al,0x61			// Turn the speaker & gate on
			asm	or	al,3
			asm	out	0x61,al
			}
			else					// Time for some silence
			{
			asm	in	al,0x61		  	// Turn the speaker & gate off
			asm	and	al,0xfc			// ~3
			asm	out	0x61,al
			}

		asm	popf
		}

		if (!(--pcLengthLeft))
		{
			SDL_PCStopSound();
			SDL_SoundFinished();
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutPC() - Turns off the pc speaker
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_ShutPC(void)
{
asm	pushf
asm	cli

	pcSound = 0;

asm	in	al,0x61		  	// Turn the speaker & gate off
asm	and	al,0xfc			// ~3
asm	out	0x61,al

asm	popf
}
#endif // 0

//
//	Stuff for digitized sounds
//

// FIXME
#if 0
void* SDL_LoadDigiSegment(Uint16 page)
{
	void*	addr;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	addr = PM_GetSoundPage(page);

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif

	return(addr);
}
#endif // 0

// FIXME
#if 0
void
SDL_PlayDigiSegment(void* addr,Uint16 len)
{
	switch (DigiMode)
	{
	case sds_SoundSource:
		SDL_SSPlaySample(static_cast<Uint8*>(addr),len);
		break;
	case sds_SoundBlaster:
		SDL_SBPlaySample(static_cast<Uint8*>(addr),len);
		break;
    default:
        break;
	}
}
#endif // 0

// FIXME
#if 0
void
SD_StopDigitized(void)
{
asm	pushf
asm	cli

	DigiFailSafe = 0;
	DigiLeft = 0;
	DigiNextAddr = nil;
	DigiNextLen = 0;
	DigiMissed = false;
	DigiPlaying = false;
    DigiPriority = 0;
	DigiNumber = HITWALLSND;
	SoundPositioned = false;
	if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		SDL_SoundFinished();

	switch (DigiMode)
	{
	case sds_SoundSource:
		SDL_SSStopSample();
		break;
	case sds_SoundBlaster:
		SDL_SBStopSample();
		break;
    default:
		break;
	}

asm	popf

	DigiLastStart = 1;
	DigiLastEnd = 0;
}
#endif // 0

// FIXME
#if 0
void
SD_Poll(void)
{
	if (DigiFailSafe && (DigiFailSafe < TimeCount))
	{
		DigiFailTriggered++;
		SD_StopDigitized();
	}

	if (DigiLeft && !DigiNextAddr)
	{
		DigiNextLen = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
		DigiLeft -= DigiNextLen;
		if (!DigiLeft)
			DigiLastSegment = true;
		DigiNextAddr = SDL_LoadDigiSegment(DigiPage++);
	}
	if (DigiMissed && DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
		}
	}
	SDL_SetTimerSpeed();

#if 0
	if (sbSamplePlaying)
	{
	asm	mov	dx,STATUS_REGISTER_1
	asm	in	al,dx
	asm	mov	dx,ATR_INDEX
	asm	mov	al,ATR_OVERSCAN
	asm	out	dx,al
	asm	mov	al,10	// bright green
	asm	out	dx,al
	asm	mov	al,0x20	// normal
	asm	out	dx,al
	}
	else
	{
	asm	mov	dx,STATUS_REGISTER_1
	asm	in	al,dx
	asm	mov	dx,ATR_INDEX
	asm	mov	al,ATR_OVERSCAN
	asm	out	dx,al
	asm	mov	al,0	// black
	asm	out	dx,al
	asm	mov	al,0x20	// normal
	asm	out	dx,al
	}
#endif
}
#endif // 0

// FIXME
#if 0
void
SD_SetPosition(Sint16 leftpos,Sint16 rightpos)
{
	if
	(
		(leftpos < 0)
	||	(leftpos > 15)
	||	(rightpos < 0)
	||	(rightpos > 15)
	||	((leftpos == 15) && (rightpos == 15))
	)
		SD_ERROR(SD_SETPOSITION_BAD_POSITION);

	switch (DigiMode)
	{
	case sds_SoundSource:
		SDL_PositionSS(leftpos,rightpos);
		break;
	case sds_SoundBlaster:
		SDL_PositionSBP(leftpos,rightpos);
		break;
    default:
        break;
	}
}
#endif // 0

// FIXME
#if 0
void
SD_PlayDigitized(Uint16 which,Sint16 leftpos,Sint16 rightpos)
{
	Sint16		i;
	Uint8	timevalue;
	Uint16	pages;
	Uint16	len;
	void*	addr;

	if (!DigiMode)
		return;

	SD_StopDigitized();
	if (which >= NumDigi)
	{
//		SD_ERROR(SD_PLAYDIGITIZED_BAD_SOUND);
		which = 1;
		return;
	}

	SD_SetPosition(leftpos,rightpos);

#if 1
	if (DigiMode == sds_SoundBlaster)
	{
		// Set the SoundBlaster DAC time constant for 7KHz
		timevalue = 256 - (1000000 / 7000);
		sbLastTimeValue = timevalue;
		sbWriteDelay();
		sbOut(sbWriteCmd,0x40);
		sbWriteDelay();
		sbOut(sbWriteData,timevalue);
		for (i = 0;i < 80;i++)
			sbIn(sbWriteStat);
	}
#endif

	DigiPage = DigiList[(which * 2) + 0];
	DigiLeft = DigiList[(which * 2) + 1];

	DigiLastStart = DigiPage;
	pages = ((DigiLeft + (PMPageSize - 1)) / PMPageSize);
	DigiLastEnd = DigiPage + pages;
	// Set up failsafe at ~105% of when the sound should end. This is
	// computed as 7000Hz/70Hz=100, then only dividing by 95 for the slop.
	DigiFailSafe = TimeCount + ((DigiLeft + 100) / 95);

	len = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
	addr = SDL_LoadDigiSegment(DigiPage++);

	DigiPlaying = true;
	DigiLastSegment = false;

	SDL_PlayDigiSegment(addr,len);
	DigiLeft -= len;
	if (!DigiLeft)
		DigiLastSegment = true;

	SD_Poll();
}
#endif // 0

// FIXME
#if 0
void
SDL_DigitizedDone(void)
{
	if (DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
	}
	else
	{
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
			if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
			{
				SDL_SoundFinished();
			}
			else
			{
				DigiNumber = HITWALLSND;
                DigiPriority = 0;
				DigiFailSafe = 0;
			}
			SoundPositioned = false;
		}
		else
			DigiMissed = true;
	}
}
#endif // 0

// FIXME
#if 0
void
SD_SetDigiDevice(SDSMode mode)
{
	boolean	devicenotpresent;

	if (mode == DigiMode)
		return;

	SD_StopDigitized();

	devicenotpresent = false;
	switch (mode)
	{
	case sds_SoundBlaster:
		if (!SoundBlasterPresent)
		{
			if (SoundSourcePresent)
				mode = sds_SoundSource;
			else
				devicenotpresent = true;
		}
		break;
	case sds_SoundSource:
		if (!SoundSourcePresent)
			devicenotpresent = true;
		break;
    default:
        break;
	}

	if (!devicenotpresent)
	{
		if (DigiMode == sds_SoundSource)
			SDL_ShutSS();

		DigiMode = mode;

		if (mode == sds_SoundSource)
			SDL_StartSS();

		SDL_SetTimerSpeed();
	}
}
#endif // 0

void SD_SetDigiDevice(
    SDSMode mode)
{
    if (mode == DigiMode)
        return;

    switch (mode) {
    case sds_SoundBlaster:
    case sds_SoundSource:
        DigiMode = sds_SoundBlaster;
        break;

    default:
        DigiMode = mode;
    }
}

void
SDL_SetupDigi(void)
{
	void*	list;
	const Uint16* p;
	Uint16 pg;
	Sint16		i;

    list = new char[PMPageSize];
	p = (Uint16 *)PM_GetPage(ChunksInFile - 1);
	memcpy(list, p, PMPageSize);
	pg = PMSoundStart;
	for (i = 0;i < static_cast<int>(PMPageSize / (sizeof(Uint16) * 2));i++,p += 2)
	{
		if (pg >= ChunksInFile - 1)
			break;
		pg += (p[1] + (PMPageSize - 1)) / PMPageSize;
	}
    DigiList = new Uint16[i * 2];

// FIXME
#if 0
	memcpy(DigiList, list, i * sizeof(Uint16) * 2);
#endif // 0

    const Uint16* src_list = static_cast<const Uint16*>(list);
    Uint16* dst_list = DigiList;

    for (int j = 0; j < i; ++j) {
        *dst_list++ = SDL_SwapLE16(*src_list++);
        *dst_list++ = SDL_SwapLE16(*src_list++);
    }

    delete [] static_cast<char*>(list);
	NumDigi = i;

	for (i = 0;i < sdLastSound;i++)
		DigiMap[i] = -1;
}

// 	AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//	alOut(n,b) - Puts b in AdLib card register n
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
alOut(Uint8 n,Uint8 b)
{
	int	i;

asm	pushf
asm	cli

	outportb(alFMAddr,n);
	for (i = 0;i < 6;i++)
		inportb(alFMStatus);
	outportb(alFMData,b);

asm	popf

	for (i = 0;i < 35;i++)
		inportb(alFMStatus);
}
#endif // 0

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetInstrument() - Puts an instrument into a generator
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SetInstrument(int track,int which,Instrument *inst,boolean percussive)
{
	Uint8		c,m;

	if (percussive)
	{
		c = pcarriers[which];
		m = pmodifiers[which];
	}
	else
	{
		c = carriers[which];
		m = modifiers[which];
	}

	tracks[track - 1]->inst = *inst;
	tracks[track - 1]->percussive = percussive;

	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);

	// Most percussive instruments only use one cell
	if (c != 0xff)
	{
		alOut(c + alChar,inst->cChar);
		alOut(c + alScale,inst->cScale);
		alOut(c + alAttack,inst->cAttack);
		alOut(c + alSus,inst->cSus);
		alOut(c + alWave,inst->cWave);
	}

	alOut(which + alFeedCon,inst->nConn);	// DEBUG - I think this is right
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALStopSound() - Turns off any sound effects playing through the
//		AdLib card
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALStopSound(void)
{
asm	pushf
asm	cli

	(Sint32)alSound = 0;
	alOut(alFreqH + 0,0);

asm	popf
}
#endif // 0

// FIXME
#if 0
static void
SDL_AlSetFXInst(Instrument *inst)
{
	Uint8		c,m;

	m = modifiers[0];
	c = carriers[0];
	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);
	alOut(c + alChar,inst->cChar);
	alOut(c + alScale,inst->cScale);
	alOut(c + alAttack,inst->cAttack);
	alOut(c + alSus,inst->cSus);
	alOut(c + alWave,inst->cWave);

	// Note: Switch commenting on these lines for old MUSE compatibility
//	alOut(alFeedCon,inst->nConn);
	alOut(alFeedCon,0);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALPlaySound() - Plays the specified sound on the AdLib card
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALPlaySound(AdLibSound *sound)
{
	Instrument	*inst;
	Uint8		*data;

	SDL_ALStopSound();

asm	pushf
asm	cli

	alLengthLeft = sound->common.length;
	data = sound->data;
	data++;
	data--;
	alSound = (Uint8 *)data;
	alBlock = ((sound->block & 7) << 2) | 0x20;
	inst = &sound->inst;

	if (!(inst->mSus | inst->cSus))
	{
	asm	popf
		SD_ERROR(SDL_ALPLAYSOUND_BAD_INST);
	}

	SDL_AlSetFXInst(&alZeroInst);	// DEBUG
	SDL_AlSetFXInst(inst);

asm	popf
}
#endif // 0

#if 0
///////////////////////////////////////////////////////////////////////////
//
// 	SDL_ALSoundService() - Plays the next sample out through the AdLib card
//
///////////////////////////////////////////////////////////////////////////
//static void
void
SDL_ALSoundService(void)
{
	Uint8	s;

	if (alSound)
	{
		s = *alSound++;
		if (!s)
			alOut(alFreqH + 0,0);
		else
		{
			alOut(alFreqL + 0,s);
			alOut(alFreqH + 0,alBlock);
		}

		if (!(--alLengthLeft))
		{
			(Sint32)alSound = 0;
			alOut(alFreqH + 0,0);
			SDL_SoundFinished();
		}
	}
}
#endif

#if 0
void
SDL_ALService(void)
{
	Uint8	a,v;
	Uint16	w;

	if (!sqActive)
		return;

	while (sqHackLen && (sqHackTime <= alTimeCount))
	{
		w = *sqHackPtr++;
		sqHackTime = alTimeCount + *sqHackPtr++;
	asm	mov	dx,[w]
	asm	mov	[a],dl
	asm	mov	[v],dh
		alOut(a,v);
		sqHackLen -= 4;
	}
	alTimeCount++;
	if (!sqHackLen)
	{
		sqHackPtr = (Uint16 *)sqHack;
		sqHackLen = sqHackSeqLen;
		alTimeCount = sqHackTime = 0;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutAL() - Shuts down the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_ShutAL(void)
{
asm	pushf
asm	cli

	alOut(alEffects,0);
	alOut(alFreqH + 0,0);
	SDL_AlSetFXInst(&alZeroInst);
	alSound = 0;

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanAL() - Totally shuts down the AdLib card
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_CleanAL(void)
{
	int	i;

asm	pushf
asm	cli

	alOut(alEffects,0);
	for (i = 1;i < 0xf5;i++)
		alOut(i,0);

asm	popf
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartAL() - Starts up the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_StartAL(void)
{
	alFXReg = 0;
	alOut(alEffects,alFXReg);
	SDL_AlSetFXInst(&alZeroInst);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_DetectAdLib() - Determines if there's an AdLib (or SoundBlaster
//		emulating an AdLib) present
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static boolean
SDL_DetectAdLib(void)
{
	Uint8	status1,status2;
	int		i;

	alOut(4,0x60);	// Reset T1 & T2
	alOut(4,0x80);	// Reset IRQ
	status1 = readstat();
	alOut(2,0xff);	// Set timer 1
	alOut(4,0x21);	// Start timer 1
#if 0
	SDL_Delay(TimerDelay100);
#else
asm	mov	dx,alFMStatus
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop
#endif

	status2 = readstat();
	alOut(4,0x60);
	alOut(4,0x80);

	if (((status1 & 0xe0) == 0x00) && ((status2 & 0xe0) == 0xc0))
	{
		for (i = 1;i <= 0xf5;i++)	// Zero all the registers
			alOut(i,0);

		alOut(1,0x20);	// Set WSE=1
		alOut(8,0);		// Set CSM=0 & SEL=0

		return(true);
	}
	else
		return(false);
}
#endif

static boolean SDL_DetectAdLib()
{
    int sdl_result = 0;

    sdl_result = ::SDL_Init(SDL_INIT_AUDIO);

    return sdl_result == 0;
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_t0Service() - My timer 0 ISR which handles the different timings and
//		dispatches to whatever other routines are appropriate
//
///////////////////////////////////////////////////////////////////////////
static void interrupt
SDL_t0Service(void)
{
static	Uint16	count = 1;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,4	// red
asm	out	dx,al
#endif

	HackCount++;

	if ((MusicMode == smm_AdLib) || (DigiMode == sds_SoundSource))
	{
		SDL_ALService();
		SDL_SSService();
//		if (!(++count & 7))
		if (!(++count % 10))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
//		if (!(count & 3))
		if (!(count % 5))
		{
			switch (SoundMode)
			{
			case sdm_PC:
				SDL_PCService();
				break;
			case sdm_AdLib:
				SDL_ALSoundService();
				break;
			}
		}
	}
	else
	{
		if (!(++count & 1))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
		switch (SoundMode)
		{
		case sdm_PC:
			SDL_PCService();
			break;
		case sdm_AdLib:
			SDL_ALSoundService();
			break;
		}
	}

asm	mov	ax,[WORD PTR TimerCount]
asm	add	ax,[WORD PTR TimerDivisor]
asm	mov	[WORD PTR TimerCount],ax
asm	jnc	myack
	t0OldService();			// If we overflow a word, time to call old int handler
asm	jmp	olddone
myack:;
	outportb(0x20,0x20);	// Ack the interrupt
olddone:;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_ShutDevice(void)
{
	switch (SoundMode)
	{
	case sdm_PC:
		SDL_ShutPC();
		break;
	case sdm_AdLib:
		SDL_ShutAL();
		break;
    default:
        break;
	}
	SoundMode = sdm_Off;
}
#endif // 0

static void SDL_ShutDevice()
{
    SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanDevice() - totally shuts down all sound devices
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_CleanDevice(void)
{
	if ((SoundMode == sdm_AdLib) || (MusicMode == smm_AdLib))
		SDL_CleanAL();
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
static void
SDL_StartDevice(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
		SDL_StartAL();
		break;
    default:
		break;
	}
	SoundNumber = HITWALLSND;
    SoundPriority = 0;
}
#endif // 0

static void SDL_StartDevice()
{
}

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetSoundMode(SDMode mode)
{
	boolean	result = false;
	Uint16	tableoffset = 0;

	SD_StopSound();

#ifndef	_MUSE_
	if ((mode == sdm_AdLib) && !AdLibPresent)
		mode = sdm_PC;

	switch (mode)
	{
	case sdm_Off:
		tableoffset = sdStartPCSounds;
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_PC:
		tableoffset = sdStartPCSounds;
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_AdLib:
		if (AdLibPresent)
		{
			tableoffset = sdStartALSounds;
			NeedsDigitized = false;
			result = true;
		}
		break;
	}
#else
	result = true;
#endif

	if (result && (mode != SoundMode))
	{
		SDL_ShutDevice();
		SoundMode = mode;
#ifndef	_MUSE_
// FIXME
#if 0
		SoundTable = (Uint16 *)(&audiosegs[tableoffset]);
#endif // 0

        SoundTable = &audiosegs[tableoffset];
#endif
		SDL_StartDevice();
	}

// FIXME
#if 0
	SDL_SetTimerSpeed();
#endif //0

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
boolean
SD_SetMusicMode(SMMode mode)
{
	boolean	result = false;

	SD_FadeOutMusic();
	while (SD_MusicPlaying())
		;

	switch (mode)
	{
	case smm_Off:
		NeedsMusic = false;
		result = true;
		break;
	case smm_AdLib:
		if (AdLibPresent)
		{
			NeedsMusic = true;
			result = true;
		}
		break;
	}

	if (result)
		MusicMode = mode;

	SDL_SetTimerSpeed();

	return(result);
}
#endif // 0

boolean SD_SetMusicMode(
    SMMode mode)
{
    boolean result = false;

    SD_MusicOff();

    switch (mode) {
    case smm_Off:
        result = true;

    case smm_AdLib:
        result = AdLibPresent;
    }

    if (result)
        MusicMode = mode;

    return result;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Startup() - starts up the Sound Mgr
//		Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_Startup(void)
{
	Sint16	i;

	if (SD_Started)
		return;

	SDL_SetDS();

	ssIsTandy = false;
	ssNoCheck = false;
	alNoCheck = false;
	sbNoCheck = false;
	sbNoProCheck = false;
#ifndef	_MUSE_
	for (i = 1;i < g_argc;i++)
	{
		switch (US_CheckParm(g_argv[i],(const char**)ParmStrings))
		{
		case 0:						// No AdLib detection
			alNoCheck = true;
			break;
		case 1:						// No SoundBlaster detection
			sbNoCheck = true;
			break;
		case 2:						// No SoundBlaster Pro detection
			sbNoProCheck = true;
			break;
		case 3:
			ssNoCheck = true;		// No Sound Source detection
			break;
		case 4:						// Tandy Sound Source handling
			ssIsTandy = true;
			break;
		case 5:						// Sound Source present at LPT1
			ssPort = 1;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 6:                     // Sound Source present at LPT2
			ssPort = 2;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 7:                     // Sound Source present at LPT3
			ssPort = 3;
			ssNoCheck = SoundSourcePresent = true;
			break;
		}
	}
#endif

	SoundUserHook = 0;

	t0OldService = getvect(8);	// Get old timer 0 ISR

	LocalTime = TimeCount = alTimeCount = 0;

	SD_SetSoundMode(sdm_Off);
	SD_SetMusicMode(smm_Off);

	if (!ssNoCheck)
		SoundSourcePresent = SDL_DetectSoundSource();

	if (!alNoCheck)
	{
		AdLibPresent = SDL_DetectAdLib();
		if (AdLibPresent && !sbNoCheck)
		{
			Sint16 port = -1;
			char *env = getenv("BLASTER");
			if (env)
			{
				Sint32 temp;
				while (*env)
				{
					while (isspace(*env))
						env++;

					switch (toupper(*env))
					{
					case 'A':
						temp = strtol(env + 1,&env,16);
						if
						(
							(temp >= 0x210)
						&&	(temp <= 0x280)
						&&	(!(temp & 0x00f))
						)
							port = (temp - 0x200) >> 4;
						else
							SD_ERROR(SD_STARTUP_BAD_ADDRESS);
						break;
					case 'I':
						temp = strtol(env + 1,&env,10);
						if
						(
							(temp >= 0)
						&&	(temp <= 10)
						&&	(sbIntVectors[temp] != -1)
						)
						{
							sbInterrupt = temp;
							sbIntVec = sbIntVectors[sbInterrupt];
						}
						else
							SD_ERROR(SD_STARTUP_BAD_INTERRUPT);
						break;
					case 'D':
						temp = strtol(env + 1,&env,10);
						if ((temp == 0) || (temp == 1) || (temp == 3))
							SDL_SBSetDMA(temp);
						else
							SD_ERROR(SD_STARTUP_BAD_DMA);
						break;
					default:
						while (isspace(*env))
							env++;
						while (*env && !isspace(*env))
							env++;
						break;
					}
				}
			}
			SoundBlasterPresent = SDL_DetectSoundBlaster(port);
		}
	}

	for (i = 0;i < 255;i++)
		pcSoundLookup[i] = i * 60;

	if (SoundBlasterPresent)
		SDL_StartSB();

	SDL_SetupDigi();

	SD_Started = true;
}
#endif // 0

void SD_Startup()
{
    if (SD_Started)
        return;

    ssIsTandy = false;
    ssNoCheck = false;
    alNoCheck = false;
    sbNoCheck = false;
    sbNoProCheck = false;

#ifndef _MUSE_
    for (int i = 1; i < g_argc; ++i) {
        switch (US_CheckParm(g_argv[i], ParmStrings)) {
        case 0: // No AdLib detection
            alNoCheck = true;
            break;

        case 1: // No SoundBlaster detection
            sbNoCheck = true;
            break;

        case 2: // No SoundBlaster Pro detection
            sbNoProCheck = true;
            break;

        case 3:
            ssNoCheck = true; // No Sound Source detection
            break;

        case 4: // Tandy Sound Source handling
            ssIsTandy = true;
            break;

        case 5: // Sound Source present at LPT1
        case 6: // Sound Source present at LPT2
        case 7: // Sound Source present at LPT3
            // FIXME Print a warning?
            break;
        }
    }
#endif

    SoundUserHook = 0;

    LocalTime = 0;
    TimeCount = 0;

    SD_SetSoundMode(sdm_Off);
    SD_SetMusicMode(smm_Off);

    SoundSourcePresent = false;

    AdLibPresent = false;
    SoundBlasterPresent = false;

    if (!alNoCheck) {
        AdLibPresent = SDL_DetectAdLib();

        if (AdLibPresent && !sbNoCheck)
            SoundBlasterPresent = true;
    }

    if (AdLibPresent)
        mixer.initialize(44100);
    else
        mixer.uninitialize();

    SDL_SetupDigi();

    SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//		the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void
SD_Default(boolean gotit,SDMode sd,SMMode sm)
{
	boolean	gotsd,gotsm;

	gotsd = gotsm = gotit;

	if (gotsd)	// Make sure requested sound hardware is available
	{
		switch (sd)
		{
		case sdm_AdLib:
			gotsd = AdLibPresent;
			break;
        default:
            break;
		}
	}
	if (!gotsd)
	{
		if (AdLibPresent)
			sd = sdm_AdLib;
		else
			sd = sdm_PC;
	}
	if (sd != SoundMode)
		SD_SetSoundMode(sd);


	if (gotsm)	// Make sure requested music hardware is available
	{
		switch (static_cast<SDMode>(sm))
		{
		case sdm_AdLib:
			gotsm = AdLibPresent;
			break;
        default:
			break;
		}
	}
	if (!gotsm)
	{
		if (AdLibPresent)
			sm = smm_AdLib;
	}
	if (sm != MusicMode)
		SD_SetMusicMode(sm);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Shutdown() - shuts down the Sound Mgr
//		Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_Shutdown(void)
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SD_StopSound();
	SDL_ShutDevice();
	SDL_CleanDevice();

	if (SoundBlasterPresent)
		SDL_ShutSB();

	if (SoundSourcePresent)
		SDL_ShutSS();

	asm	pushf
	asm	cli

	SDL_SetTimer0(0);

	setvect(8,t0OldService);

	asm	popf

	SD_Started = false;
}
#endif // 0

void SD_Shutdown() {
    if (!SD_Started)
        return;

    mixer.uninitialize();

    SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetUserHook() - sets the routine that the Sound Mgr calls every 1/70th
//		of a second from its timer 0 ISR
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_SetUserHook(void (* hook)(void))
{
	SoundUserHook = hook;
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SD_PositionSound() - Sets up a stereo imaging location for the next
//		sound to be played. Each channel ranges from 0 to 15.
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_PositionSound(Sint16 leftvol,Sint16 rightvol)
{
	LeftPosition = leftvol;
	RightPosition = rightvol;
	nextsoundpos = true;
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
boolean
SD_PlaySound(soundnames sound)
{
	boolean		ispos;
	SoundCommon	*s;
	int	lp,rp;

	lp = LeftPosition;
	rp = RightPosition;
	LeftPosition = 0;
	RightPosition = 0;

	ispos = nextsoundpos;
	nextsoundpos = false;

	if (sound == -1)
		return(false);

	s = MK_FP(SoundTable[sound],0);
	if ((SoundMode != sdm_Off) && !s)
		SD_ERROR(SD_PLAYSOUND_UNCACHED);

	if ((DigiMode != sds_Off) && (DigiMap[sound] != -1))
	{
		if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		{
			if (s->priority < SoundPriority)
				return(false);

			SDL_PCStopSound();

			SD_PlayDigitized(DigiMap[sound],lp,rp);
			SoundPositioned = ispos;
			SoundNumber = sound;
			SoundPriority = s->priority;
		}
		else
		{
		asm	pushf
		asm	cli
			if (DigiPriority && !DigiNumber)
			{
			asm	popf
				SD_ERROR(SD_PLAYSOUND_PRI_NO_SOUND);
			}
		asm	popf

			if (s->priority < DigiPriority)
				return(false);

			SD_PlayDigitized(DigiMap[sound],lp,rp);
			SoundPositioned = ispos;
			DigiNumber = sound;
			DigiPriority = s->priority;
		}

		return(true);
	}

	if (SoundMode == sdm_Off)
		return(false);
	if (!s->length)
		SD_ERROR(SD_PLAYSOUND_ZERO_LEN);
	if (s->priority < SoundPriority)
		return(false);

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCPlaySound((void *)s);
		break;
	case sdm_AdLib:
		SDL_ALPlaySound((void *)s);
		break;
	}

	SoundNumber = sound;
	SoundPriority = s->priority;

	return(false);
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
Uint16
SD_SoundPlaying(void)
{
	boolean	result = false;

	switch (SoundMode)
	{
	case sdm_PC:
		result = pcSound? true : false;
		break;
	case sdm_AdLib:
		result = alSound? true : false;
		break;
    default:
		break;
	}

	if (result)
		return(SoundNumber);
	else
		return(false);
}
#endif // 0

bool SD_SoundPlaying()
{
    switch (SoundMode) {
    case sdm_PC:
        return false;

    case sdm_AdLib:
        return mixer.is_any_sfx_playing();

    default:
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_StopSound(void)
{
	if (DigiPlaying)
		SD_StopDigitized();

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCStopSound();
		break;
	case sdm_AdLib:
		SDL_ALStopSound();
		break;
    default:
		break;
	}

	SoundPositioned = false;

	SDL_SoundFinished();
}
#endif // 0

void SD_StopSound()
{
    mixer.stop_all_sfx();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_WaitSoundDone(void)
{
	while (SD_SoundPlaying())
		;
}
#endif // 0

void SD_WaitSoundDone()
{
    while (::SD_SoundPlaying())
        ::SDL_Delay(1);
}


///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_MusicOn(void)
{
	sqActive = true;
}
#endif // 0

void SD_MusicOn()
{
    mixer.play_adlib_music(music_index, sqHack, sqHackLen);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_MusicOff(void)
{
	Uint16	i;

	sqActive = false;
	switch (MusicMode)
	{
	case smm_AdLib:
		alFXReg = 0;
		alOut(alEffects,0);
		for (i = 0;i < sqMaxTracks;i++)
			alOut(alFreqH + i + 1,0);
		break;
	}
}
#endif // 0

void SD_MusicOff()
{
    mixer.stop_music();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_StartMusic(MusicGroup *music)
{
	SD_MusicOff();

asm	pushf
asm	cli

	sqPlayedOnce = false;

	if (MusicMode == smm_AdLib)
	{
		sqHackPtr = sqHack = music->values;
		sqHackSeqLen = sqHackLen = music->length;
		sqHackTime = 0;
		alTimeCount = 0;
		SD_MusicOn();
	}
	else
		sqPlayedOnce = true;

asm	popf
}
#endif

void SD_StartMusic(
    int index)
{
    SD_MusicOff();

    sqPlayedOnce = false;

    if (MusicMode == smm_AdLib) {
        music_index = index;

        Uint16* music_data = reinterpret_cast<Uint16*>(
            audiosegs[STARTMUSIC + index]);

        int length = SDL_SwapLE16(music_data[0]) + 2;

        sqHack = music_data;
        sqHackLen = static_cast<Uint16>(length);

        SD_MusicOn();
    } else
        sqPlayedOnce = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//		to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
void
SD_FadeOutMusic(void)
{
	switch (MusicMode)
	{
	case smm_AdLib:
		// DEBUG - quick hack to turn the music off
		SD_MusicOff();
		break;
    default:
        break;
	}
}
#endif // 0

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicPlaying() - returns true if music is currently playing, false if
//		not
//
///////////////////////////////////////////////////////////////////////////

// FIXME
#if 0
boolean
SD_MusicPlaying(void)
{
	boolean	result;

	switch (MusicMode)
	{
	case smm_AdLib:
		result = false;
		// DEBUG - not written
		break;
	default:
		result = false;
	}

	return(result);
}
#endif // 0

// BBi
void sd_play_sound(
    int sound_index,
    const void* actor,
    bstone::ActorType actor_type,
    bstone::ActorChannel actor_channel)
{
    if (sound_index < 0)
        return;

    if (SoundTable == NULL)
        return;

    int actor_index = -1;

    if (actor != NULL) {
        switch (actor_type) {
        case bstone::AT_ACTOR:
            actor_index = static_cast<int>(
                static_cast<const objtype*>(actor) - objlist);
            break;

        case bstone::AT_DOOR:
            actor_index = static_cast<int>(
                static_cast<const doorobj_t*>(actor) - doorobjlist);
            break;

        default:
            return;
        }
    }

    const SoundCommon* sound = reinterpret_cast<SoundCommon*>(
        SoundTable[sound_index]);

    if (sound == NULL)
        return;

    if (SoundMode != sdm_Off && sound == NULL)
        SD_ERROR(SD_PLAYSOUND_UNCACHED);

    int priority = sound->priority;

    int digi_index = DigiMap[sound_index];

    if (DigiMode != sds_Off && digi_index != -1) {
        int digi_page = DigiList[(2 * digi_index) + 0];
        int digi_length = DigiList[(2 * digi_index) + 1];
        const void* digi_data = ::PM_GetSoundPage(digi_page);

        mixer.play_pcm_sound(digi_index, priority, digi_data, digi_length,
            actor_index, actor_type, actor_channel);

        return;
    }

    if (SoundMode == sdm_Off)
        return;

    switch (SoundMode) {
    case sdm_AdLib:
        break;

    default:
        return;
    }

    int data_size = audiostarts[sdStartALSounds + sound_index + 1] -
            audiostarts[sdStartALSounds + sound_index];

    mixer.play_adlib_sound(sound_index, priority, sound, data_size,
        actor_index, actor_type, actor_channel);
}

void sd_play_actor_sound(
    int sound_index,
    const objtype* actor,
    bstone::ActorChannel actor_channel)
{
    sd_play_sound(
        sound_index,
        actor,
        bstone::AT_ACTOR,
        actor_channel);
}

void sd_play_player_sound(
    int sound_index,
    bstone::ActorChannel actor_channel)
{
    sd_play_sound(
        sound_index,
        player,
        bstone::AT_ACTOR,
        actor_channel);
}

void sd_play_door_sound(
    int sound_index,
    const doorobj_t* door)
{
    sd_play_sound(
        sound_index,
        door,
        bstone::AT_DOOR,
        bstone::AC_VOICE);
}

void sd_play_wall_sound(
    int sound_index)
{
    sd_play_sound(
        sound_index,
        NULL,
        bstone::AT_WALL,
        bstone::AC_VOICE);
}

void sd_update_positions()
{
    mixer.update_positions();
}

bool sd_is_player_channel_playing(
    bstone::ActorChannel channel)
{
    return mixer.is_player_channel_playing(channel);
}
// BBi
