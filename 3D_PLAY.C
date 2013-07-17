// 3D_PLAY.C

#include "3D_DEF.H"
#pragma hdrstop


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define sc_Question	0x35

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


unsigned char music_num=0;

#if LOOK_FOR_DEAD_GUYS
objtype *DeadGuys[MAXACTORS];
unsigned char NumDeadGuys;
#endif

boolean		madenoise;					// true when shooting or screaming
unsigned char alerted = 0,alerted_areanum;


exit_t		playstate;

boolean PowerBall = false;

#if TECH_SUPPORT_VERSION
int			bordertime,DebugOk = true,InstantWin = 0,InstantQuit = 0;
#else
int			bordertime,DebugOk = false,InstantWin = 0,InstantQuit = 0;
#endif

unsigned ExtraRadarFlags	= 0;



#if IN_DEVELOPMENT

int TestQuickSave = 0, TestAutoMapper = 0;

#endif

objtype 	objlist[MAXACTORS],*new,*player,*lastobj,
			*objfreelist,*killerobj;

unsigned	farmapylookup[MAPSIZE];
byte		*nearmapylookup[MAPSIZE];

boolean		singlestep=false,godmode;	//,noclip;
int			extravbls = 0;

byte		tilemap[MAPSIZE][MAPSIZE];	// wall values only
byte		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

//
// replacing refresh manager
//
unsigned	mapwidth,mapheight,tics,realtics;
boolean		compatability,usedummy=false, nevermark = false;
byte		*updateptr;
unsigned	mapwidthtable[64];
unsigned	uwidthtable[UPDATEHIGH];
unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];
byte		update[UPDATESIZE];

//
// control info
//
boolean		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
int			joystickport;
int			dirscan[4] = {sc_UpArrow,sc_RightArrow,sc_DownArrow,sc_LeftArrow};
int			buttonscan[NUMBUTTONS] =
				{sc_Control,sc_Alt,sc_RShift,sc_Space,sc_1,sc_2,sc_3,sc_4,sc_5,sc_6,sc_7};
int			buttonmouse[4]={bt_attack,bt_strafe,bt_use,bt_nobutton};
int			buttonjoy[4]={bt_attack,bt_strafe,bt_use,bt_run};

int			viewsize;

boolean		buttonheld[NUMBUTTONS];

boolean		demorecord,demoplayback;
char		far *demoptr, far *lastdemoptr;
memptr		demobuffer;

// Light sourcing flag

byte lightson;

//
// curent user input
//
int			controlx,controly;		// range from -100 to 100 per tic
boolean		buttonstate[NUMBUTTONS];


//===========================================================================


void	CenterWindow(word w,word h);
void 	InitObjList (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void StartMusic(boolean preload);
void	PlayLoop (void);
void SpaceEntryExit(boolean entry);
void FinishPaletteShifts (void);
void ShowQuickInstructions(void);
void CleanDrawPlayBorder(void);
void PopupAutoMap(void);


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


objtype dummyobj;

//
// LIST OF SONGS FOR EACH LEVEL
//

int far songs[]=
{
	MAJMIN_MUS,              // 0
	STICKS_MUS,              // 1
	MOURNING_MUS,            // 2
	LURKING_MUS,             // 3
	CIRCLES_MUS,             // 4
	TIME_MUS,                // 5
	TOHELL_MUS,              // 6
	FORTRESS_MUS,            // 7
	GIVING_MUS,              // 8
	HARTBEAT_MUS,            // 9
	MOURNING_MUS,            // 10
	MAJMIN_MUS,              // 11
	VACCINAP_MUS,            // 12
	LURKING_MUS,             // 13
	MONASTRY_MUS,            // 14
	TOMBP_MUS,               // 15
	DARKNESS_MUS,            // 16
	MOURNING_MUS,            // 17
	SERPENT_MUS,             // 18
	TIME_MUS,                // 19
	CATACOMB_MUS,            // 20
	PLOT_MUS,					 // 21
	GIVING_MUS,              // 22
	VACCINAP_MUS,            // 23
};

/*
=============================================================================

						  USER CONTROL

=============================================================================
*/


#define BASEMOVE		35
#define RUNMOVE			70
#define BASETURN		35
#define RUNTURN			70

#define JOYSCALE		2

/*
===================
=
= PollKeyboardButtons
=
===================
*/

void PollKeyboardButtons (void)
{
	int		i;

	for (i=0;i<NUMBUTTONS;i++)
		if (Keyboard[buttonscan[i]])
			buttonstate[i] = true;
}


/*
===================
=
= PollMouseButtons
=
===================
*/

void PollMouseButtons (void)
{
	int	buttons;

	buttons = IN_MouseButtons ();

	if (buttons&1)
		buttonstate[buttonmouse[0]] = true;
	if (buttons&2)
		buttonstate[buttonmouse[1]] = true;
	if (buttons&4)
		buttonstate[buttonmouse[2]] = true;
}



/*
===================
=
= PollJoystickButtons
=
===================
*/

void PollJoystickButtons (void)
{
	int	buttons;

	buttons = IN_JoyButtons ();

	if (joystickport && !joypadenabled)
	{
		if (buttons&4)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&8)
			buttonstate[buttonjoy[1]] = true;
	}
	else
	{
		if (buttons&1)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&2)
			buttonstate[buttonjoy[1]] = true;
		if (joypadenabled)
		{
			if (buttons&4)
				buttonstate[buttonjoy[2]] = true;
			if (buttons&8)
				buttonstate[buttonjoy[3]] = true;
		}
	}
}


/*
===================
=
= PollKeyboardMove
=
===================
*/

void PollKeyboardMove (void)
{
	if (buttonstate[bt_run])
	{
		if (Keyboard[dirscan[di_north]])
			controly -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += RUNMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += RUNMOVE*tics;
	}
	else
	{
		if (Keyboard[dirscan[di_north]])
			controly -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += BASEMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += BASEMOVE*tics;
	}
}


/*
===================
=
= PollMouseMove
=
===================
*/


boolean pollMouseUsed=false;

void PollMouseMove (void)
{
	int	mousexmove,mouseymove;

	Mouse(MDelta);
	mousexmove = _CX;
	mouseymove = _DX;

// Double speed when shift is pressed.
//
	if (Keyboard[sc_LShift] || Keyboard[sc_RShift] || buttonstate[bt_run])
	{
		controly += (mouseymove*20/(13-mouseadjustment))*4;
		controlx += (mousexmove*10/(13-mouseadjustment))/2;
	}

	controlx += mousexmove*10/(13-mouseadjustment);
	controly += mouseymove*20/(13-mouseadjustment);

	if (mousexmove || mouseymove)
		pollMouseUsed=true;
	else
		pollMouseUsed=false;
}


/*
===================
=
= PollJoystickMove
=
===================
*/

void PollJoystickMove (void)
{
	int	joyx,joyy;

	INL_GetJoyDelta(joystickport,&joyx,&joyy);

	if (joystickprogressive)
	{
		if (joyx > 64)
			controlx += (joyx-64)*JOYSCALE*tics;
		else if (joyx < -64)
			controlx -= (-joyx-64)*JOYSCALE*tics;
		if (joyy > 64)
			controlx += (joyy-64)*JOYSCALE*tics;
		else if (joyy < -64)
			controly -= (-joyy-64)*JOYSCALE*tics;
	}
	else if (buttonstate[bt_run])
	{
		if (joyx > 64)
			controlx += RUNMOVE*tics;
		else if (joyx < -64)
			controlx -= RUNMOVE*tics;
		if (joyy > 64)
			controly += RUNMOVE*tics;
		else if (joyy < -64)
			controly -= RUNMOVE*tics;
	}
	else
	{
		if (joyx > 64)
			controlx += BASEMOVE*tics;
		else if (joyx < -64)
			controlx -= BASEMOVE*tics;
		if (joyy > 64)
			controly += BASEMOVE*tics;
		else if (joyy < -64)
			controly -= BASEMOVE*tics;
	}
}


/*
===================
=
= PollControls
=
= Gets user or demo input, call once each frame
=
= controlx		set between -100 and 100 per tic
= controly
= buttonheld[]	the state of the buttons LAST frame
= buttonstate[]	the state of the buttons THIS frame
=
===================
*/

void PollControls (void)
{
	int		max,min,i;
	byte	buttonbits;

	controlx = 0;
	controly = 0;
	memcpy (buttonheld,buttonstate,sizeof(buttonstate));
	memset (buttonstate,0,sizeof(buttonstate));

#ifdef MYPROFILE
	controlx = 100;			// just spin in place
	return;
#endif

	if (demoplayback)
	{
	//
	// read commands from demo buffer
	//
		buttonbits = *demoptr++;
		for (i=0;i<NUMBUTTONS;i++)
		{
			buttonstate[i] = buttonbits&1;
			buttonbits >>= 1;
		}

		controlx = *demoptr++;
		controly = *demoptr++;
		tics = *demoptr++;

		while (TimeCount-lasttimecount < tics)
		;
		lasttimecount = TimeCount;

		if (demoptr == lastdemoptr)
			playstate = ex_completed;		// demo is done

		controlx *= (int)tics;
		controly *= (int)tics;


		return;
	}

//
// get timing info for last frame
//
	CalcTics ();

//
// get button states
//
	PollKeyboardButtons ();

	if (mouseenabled)
		PollMouseButtons ();

	if (joystickenabled)
		PollJoystickButtons ();

#if 0
if (buttonstate[bt_run])
	VL_ColorBorder (1);
else
	VL_ColorBorder (0);
#endif

//
// get movements
//
	PollKeyboardMove ();

	if (mouseenabled)
		PollMouseMove ();

	if (joystickenabled)
		PollJoystickMove ();

//
// bound movement to a maximum
//
	max = 100*tics;
	min = -max;
	if (controlx > max)
		controlx = max;
	else if (controlx < min)
		controlx = min;

	if (controly > max)
		controly = max;
	else if (controly < min)
		controly = min;

#ifdef DEMOS_EXTERN

	if (demorecord)
	{
	//
	// save info out to demo buffer
	//
		controlx /= (int)tics;
		controly /= (int)tics;

		buttonbits = 0;

		for (i=NUMBUTTONS-1;i>=0;i--)
		{
			buttonbits <<= 1;
			if (buttonstate[i])
				buttonbits |= 1;
		}

		*demoptr++ = buttonbits;
		*demoptr++ = controlx;
		*demoptr++ = controly;
		*demoptr++ = tics;

		if (demoptr >= lastdemoptr)
			PLAY_ERROR(POLLCONTROLS_DEMO_OV);

		controlx *= (int)tics;
		controly *= (int)tics;
	}

#endif			

}



//==========================================================================



///////////////////////////////////////////////////////////////////////////
//
//	CenterWindow() - Generates a window of a given width & height in the
//		middle of the screen
//
///////////////////////////////////////////////////////////////////////////

#define MAXX	320
#define MAXY	160

void	CenterWindow(word w,word h)
{
	FixOfs ();
	US_DrawWindow(((MAXX / 8) - w) / 2,((MAXY / 8) - h) / 2,w,h);
}

//===========================================================================


/*
=====================
=
= CheckKeys
=
=====================
*/

extern boolean PP_step,sqActive;	
extern int pickquick;

boolean refresh_screen;
#if (GAME_VERSION != SHAREWARE_VERSION) || GEORGE_CHEAT
byte jam_buff_cmp[]={sc_J,sc_A,sc_M};
byte jam_buff[sizeof(jam_buff_cmp)];
#endif

char far PAUSED_MSG[]="^ST1^CEGame Paused\r^CEPress any key to resume.^XX";

void CheckKeys (void)
{
	boolean one_eighty=false;
	int		i;
	byte	scan;
	unsigned	temp;
	static boolean Plus_KeyReleased;
	static boolean Minus_KeyReleased;
	static boolean I_KeyReleased;
	static boolean S_KeyReleased;

#if IN_DEVELOPMENT || BETA_TEST
//	if (DebugOk && (Keyboard[sc_P] || PP_step))
//		PicturePause ();
#endif


	if (screenfaded || demoplayback)	// don't do anything with a faded screen
		return;

	scan = LastScan;


#if IN_DEVELOPMENT
#ifdef ACTIVATE_TERMINAL
	if (Keyboard[sc_9] && Keyboard[sc_0])
		ActivateTerminal(true);
#endif
#endif

	//
	// SECRET CHEAT CODE: 'JAM'
	//

#if GAME_VERSION != SHAREWARE_VERSION
	if (Keyboard[sc_J] || Keyboard[sc_A] || Keyboard[sc_M])
	{
		if (jam_buff[sizeof(jam_buff_cmp)-1] != LastScan)
		{
			memcpy(jam_buff,jam_buff+1,sizeof(jam_buff_cmp)-1);
			jam_buff[sizeof(jam_buff_cmp)-1] = LastScan;
		}
	}
#endif

	CheckMusicToggle();

	if (gamestate.rpower)
	{
		if (Keyboard[sc_Plus] || Keyboard[sc_kpPlus])
		{
			if (Plus_KeyReleased && gamestate.rzoom<2)
			{
				UpdateRadarGuage();
				gamestate.rzoom++;
				Plus_KeyReleased=false;
			}
		}
		else
			Plus_KeyReleased=true;

		if (Keyboard[sc_Minus] || Keyboard[sc_kpMinus])
		{
			if (Minus_KeyReleased && gamestate.rzoom)
			{
				UpdateRadarGuage();
				gamestate.rzoom--;
				Minus_KeyReleased=false;
			}
		}
		else
			Minus_KeyReleased=true;
	}

	if (Keyboard[sc_S])
	{
		if (S_KeyReleased)
		{
			if ((SoundMode != sdm_Off) || (DigiMode!=sds_Off))
			{
				if (SoundMode != sdm_Off)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_Off);
				}

				if (DigiMode!=sds_Off)
					SD_SetDigiDevice(sds_Off);

				_fmemcpy((char far *)&SoundOn[55],"OFF.",4);
			}
			else
			{
				ClearMemory();
				if (SoundBlasterPresent || AdLibPresent)
					SD_SetSoundMode(sdm_AdLib);
				else
					SD_SetSoundMode(sdm_PC);

				if (SoundBlasterPresent)
					SD_SetDigiDevice(sds_SoundBlaster);
				else
				if (SoundSourcePresent)
					SD_SetDigiDevice(sds_SoundSource);
				else
					SD_SetDigiDevice(sds_Off);

				CA_LoadAllSounds();
				PM_CheckMainMem();

				_fmemcpy((char far *)&SoundOn[55],"ON. ",4);
			}

			DISPLAY_TIMED_MSG(SoundOn,MP_BONUS,MT_GENERAL);
			S_KeyReleased=false;
		}
	}
	else
		S_KeyReleased=true;

	if (Keyboard[sc_Enter])
	{
#if (GAME_VERSION != SHAREWARE_VERSION) || GEORGE_CHEAT
		char loop;

		if ((!memcmp(jam_buff,jam_buff_cmp,sizeof(jam_buff_cmp))))
		{
			jam_buff[0]=0;

			for (loop=0; loop<NUMKEYS; loop++)
				if (gamestate.numkeys[loop] < MAXKEYS)
					gamestate.numkeys[loop]=1;

			gamestate.health = 100;
			gamestate.ammo = MAX_AMMO;
			gamestate.rpower = MAX_RADAR_ENERGY;

			if (!DebugOk)
			{
				gamestate.score = 0;
				gamestate.nextextra = EXTRAPOINTS;
			}

			gamestate.TimeCount += 42000L;

			for (loop=0; loop<NUMWEAPONS-1; loop++)
				GiveWeapon(loop);

			DrawWeapon();
			DrawHealth();
			DrawKeys();
			DrawScore();
			DISPLAY_TIMED_MSG("\r\r     YOU CHEATER!",MP_INTERROGATE,MT_GENERAL);
			ForceUpdateStatusBar();

			ClearMemory ();
			ClearSplitVWB ();
			VW_ScreenToScreen (displayofs,bufferofs,80,160);

			Message("\n NOW you're jammin'!! \n");

			PM_CheckMainMem ();
			IN_ClearKeysDown();
			IN_Ack();

			CleanDrawPlayBorder();
		}
		else
#endif
			one_eighty=true;
	}

// Handle quick turning!
//
	if (!gamestate.turn_around)
	{
	// 90 degrees left
	//
		if (Keyboard[sc_Q])
		{
			gamestate.turn_around = -90;
			gamestate.turn_angle = player->angle + 90;
			if (gamestate.turn_angle > 359)
				gamestate.turn_angle -= ANGLES;
		}

	// 180 degrees right
	//
		if ((Keyboard[sc_W]) || (one_eighty))
		{
			gamestate.turn_around = 180;
			gamestate.turn_angle = player->angle + 180;
			if (gamestate.turn_angle > 359)
				gamestate.turn_angle -= ANGLES;
		}

	// 90 degrees right
	//
		if (Keyboard[sc_E])
		{
			gamestate.turn_around = 90;
			gamestate.turn_angle = player->angle - 90;
			if (gamestate.turn_angle < 0)
				gamestate.turn_angle += ANGLES;
		}
	}

//
// pause key weirdness can't be checked as a scan code
//
	if (Paused || Keyboard[sc_P])
	{
		SD_MusicOff();
		fontnumber = 4;
		BMAmsg(PAUSED_MSG);
		IN_Ack();
		IN_ClearKeysDown();
		fontnumber = 2;
		RedrawStatusAreas();
		SD_MusicOn();
		Paused = false;
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		return;
	}

#if IN_DEVELOPMENT
	if (TestQuickSave)
	{
//   	TestQuickSave--;
		scan = sc_F8;
	}

	if (TestAutoMapper)
		PopupAutoMap();

#endif

	switch (scan)
	{
		case sc_F7:							// END GAME
		case sc_F10:						// QUIT TO DOS
			FinishPaletteShifts();
			ClearMemory();
			US_ControlPanel(scan);
			PM_CheckMainMem();
			CleanDrawPlayBorder();
			return;

		case sc_F2:							// SAVE MISSION
		case sc_F8:							// QUICK SAVE
		// Make sure there's room to save...
		//
			ClearMemory();
			FinishPaletteShifts();
			if (!CheckDiskSpace(DISK_SPACE_NEEDED,CANT_SAVE_GAME_TXT,cds_id_print))
			{
				PM_CheckMainMem();
				CleanDrawPlayBorder();
				break;
			}

		case sc_F1:							// HELP
		case sc_F3:							// LOAD MISSION
		case sc_F4:							// SOUND MENU
		case sc_F5:							//	RESIZE VIEW
		case sc_F6:							// CONTROLS MENU
		case sc_F9:							// QUICK LOAD
		case sc_Escape:					// MAIN MENU
			refresh_screen=true;
			if (scan < sc_F8)
				VW_FadeOut();
			StopMusic();
			ClearMemory();
			ClearSplitVWB();
			US_ControlPanel(scan);
			if (refresh_screen)
			{
				boolean old=loadedgame;

				loadedgame=false;
				DrawPlayScreen(false);
				loadedgame=old;
			}
			ClearMemory();
			if (!sqActive || !loadedgame)
				StartMusic(false);
			PM_CheckMainMem();
			IN_ClearKeysDown();
			if (loadedgame)
			{
				PreloadGraphics();
				loadedgame=false;
				DrawPlayScreen(false);
			}
			else
				if (!refresh_screen)
					CleanDrawPlayBorder();
			if (!sqActive)
				StartMusic(false);
			return;
	}

	if (Keyboard[sc_Tab])
		PopupAutoMap();

  	if (Keyboard[sc_Tilde])
   {
      Keyboard[sc_Tilde] = 0;
   	TryDropPlasmaDetonator();
   }


	if ((DebugOk || gamestate.flags & GS_MUSIC_TEST) && (Keyboard[sc_BackSpace]))
	{
		unsigned char old_num=music_num;

		if (gamestate.flags & GS_MUSIC_TEST)
		{
			if (Keyboard[sc_LeftArrow])
			{
				if (music_num)
					music_num--;
				Keyboard[sc_LeftArrow]=false;
			}
			else
			if (Keyboard[sc_RightArrow])
			{
				if (music_num < LASTMUSIC-1)
					music_num++;
				Keyboard[sc_RightArrow]=false;
			}

			if (old_num != music_num)
			{
				ClearMemory();
				MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + old_num]);
				StartMusic(false);
				PM_CheckMainMem();
				DrawScore();
			}
		}

		if (old_num == music_num)
		{
			fontnumber=4;
			SETFONTCOLOR(0,15);
			if (DebugKeys())
         {
				CleanDrawPlayBorder();
         }

			if (MousePresent)
				Mouse(MDelta);	// Clear accumulated mouse movement
			lasttimecount = TimeCount;
			return;
		}
	}

	if (Keyboard[sc_I])
	{
		if (I_KeyReleased)
		{
			gamestate.flags ^= GS_ATTACK_INFOAREA;
			if (gamestate.flags & GS_ATTACK_INFOAREA)
				DISPLAY_TIMED_MSG(attacker_info_enabled,MP_ATTACK_INFO,MT_GENERAL);
			else
				DISPLAY_TIMED_MSG(attacker_info_disabled,MP_ATTACK_INFO,MT_GENERAL);
			I_KeyReleased = false;
		}
	}
	else
		I_KeyReleased = true;


#ifdef CEILING_FLOOR_COLORS
	if (Keyboard[sc_C])
	{
		gamestate.flags ^= GS_DRAW_CEILING;
		Keyboard[sc_C] = 0;
	}

	if (Keyboard[sc_F])
	{
		ThreeDRefresh();
		ThreeDRefresh();

		gamestate.flags ^= GS_DRAW_FLOOR;

		Keyboard[sc_F] = 0;
#if DUAL_SWAP_FILES
		ChangeSwapFiles(true);
#endif
	}
#endif

	if (Keyboard[sc_L])
	{
		Keyboard[sc_L]=0;
		gamestate.flags ^= GS_LIGHTING;
	}
}


//-------------------------------------------------------------------------
// CheckMusicToggle()
//-------------------------------------------------------------------------
void CheckMusicToggle(void)
{
	static boolean M_KeyReleased;

	if (Keyboard[sc_M])
	{
		if (M_KeyReleased
#if GAME_VERSION != SHAREWARE_VERSION
			 && ((jam_buff[0] != sc_J) || (jam_buff[1] != sc_A))
#endif
			)
		{
			if (!AdLibPresent)
			{
				DISPLAY_TIMED_MSG(NoAdLibCard,MP_BONUS,MT_GENERAL);
				SD_PlaySound(NOWAYSND);
            return;
			}
			else
			if (MusicMode != smm_Off)
			{
				SD_SetMusicMode(smm_Off);
				_fmemcpy((char far *)&MusicOn[58],"OFF.",4);
			}
			else
			{
				SD_SetMusicMode(smm_AdLib);
				StartMusic(false);
				_fmemcpy((char far *)&MusicOn[58],"ON. ",4);
			}

			DISPLAY_TIMED_MSG(MusicOn,MP_BONUS,MT_GENERAL);
			M_KeyReleased=false;
		}
	}
	else
		M_KeyReleased=true;
}


char far Computing[] = {"Computing..."};

#if DUAL_SWAP_FILES
//--------------------------------------------------------------------------
// ChangeSwapFiles()
//
// PURPOSE: To chance out swap files durring game play -
//
// ASSUMES: PageManager is installed.
//
//--------------------------------------------------------------------------

void ChangeSwapFiles(boolean display)
{
	ClearMemory();

   if (display)
   {
		WindowX=WindowY=0;
		WindowW=320;
		WindowH=200;
	   Message(Computing);
   }

	PM_Shutdown();
	PM_Startup ();

	PM_CheckMainMem();

   if (display)
   {
	   IN_UserInput(50);
		CleanDrawPlayBorder();
		IN_ClearKeysDown();
   }
}
#endif


//--------------------------------------------------------------------------
// OpenPageFile()
//--------------------------------------------------------------------------
void OpenPageFile(void)
{
#if DUAL_SWAP_FILES

	if (gamestate.flags & GS_DRAW_FLOOR || (!ShadowsAvail))
   {
   	PML_OpenPageFile(PageFileName);
      FileUsed = sd_NO_SHADOWS;
   }
   else
   {
   	PML_OpenPageFile(AltPageFileName);
      FileUsed = sd_SHADOWS;
   }

#else
  	PML_OpenPageFile(PageFileName);
#endif
}

//--------------------------------------------------------------------------
// PopupAutoMap()
//--------------------------------------------------------------------------
void PopupAutoMap()
{
	#define BASE_X	64
	#define BASE_Y	44

	ThreeDRefresh();
	ThreeDRefresh();

	SD_StopSound();
	ClearMemory();
	CacheDrawPic(BASE_X,BASE_Y,AUTOMAPPIC);

	ShowStats(BASE_X+101,BASE_Y+22,ss_quick,&gamestuff.level[gamestate.mapon].stats);

	while (Keyboard[sc_Tilde])
		CalcTics();

#if GAME_VERSION != SHAREWARE_VERSION && IN_DEVELOPMENT
//	if (DebugOk && PP_step)
//		PicturePause();
#endif

	IN_StartAck ();
	while (!IN_CheckAck ())
		CalcTics();

	PM_CheckMainMem();
	CleanDrawPlayBorder();
	IN_ClearKeysDown();
}


//===========================================================================

/*
#############################################################################

				  The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/


/*
=========================
=
= InitActorList
=
= Call to clear out the actor object lists returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/

int	objcount;

void InitActorList (void)
{
	int	i;

//
// init the actor lists
//
#if LOOK_FOR_DEAD_GUYS
	NumDeadGuys=0;
	memset(DeadGuys,0,sizeof(DeadGuys));
#endif

	memset(statobjlist,0,sizeof(statobjlist));
	for (i=0;i<MAXACTORS;i++)
	{
		objlist[i].prev = &objlist[i+1];
		objlist[i].next = NULL;
	}

	objlist[MAXACTORS-1].prev = NULL;

	objfreelist = &objlist[0];
	lastobj = NULL;

	objcount = 0;

//
// give the player the first free spots
//
	GetNewActor ();
	player = new;
}

//===========================================================================

/*
=========================
=
= GetNewActor
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out ot
= return a dummy object pointer that will never get used
=
=========================
*/

void GetNewActor (void)
{
	if (objcount >= MAXACTORS-1)
	{
		objtype *obj=player->next;

		while (obj)
		{
			if ((obj->flags & (FL_DEADGUY|FL_VISABLE)) == FL_DEADGUY)
			{
				RemoveObj(obj);
				obj = NULL;
			}
			else
				obj = obj->next;
		}
	}

	if (!objfreelist)
		if (usedummy)
      {
			new = &dummyobj;
			memset (new,0,sizeof(*new));
      }
		else
			PLAY_ERROR(GETNEWACTOR_NO_FREE_SPOTS);
   else
   {
		new = objfreelist;
		objfreelist = new->prev;

		memset (new,0,sizeof(*new));

		if (lastobj)
			lastobj->next = new;

		new->prev = lastobj;	// new->next is allready NULL from memset

//		new->active = false;
		lastobj = new;

		objcount++;
	}
}


//===========================================================================

/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/

void RemoveObj (objtype *gone)
{
	objtype **spotat;

   if (gone == &dummyobj)
   	return;

	if (gone == player)
		PLAY_ERROR(REMOVEOBJ_REMOVED_PLAYER);

	gone->state = NULL;

//
// fix the next object's back link
//
	if (gone == lastobj)
		lastobj = (objtype *)gone->prev;
	else
		gone->next->prev = gone->prev;

//
// fix the previous object's forward link
//
	gone->prev->next = gone->next;

//
// add it back in to the free list
//
	gone->prev = objfreelist;
	objfreelist = gone;

	objcount--;
}

/*
=============================================================================

						MUSIC STUFF

=============================================================================
*/


/*
=================
=
= StopMusic
=
=================
*/

void StopMusic(void)
{
	int	i;

	SD_MusicOff();
	for (i = 0;i < LASTMUSIC;i++)
		if (audiosegs[STARTMUSIC + i])
			MM_FreePtr(&((memptr)audiosegs[STARTMUSIC + i]));
}

//==========================================================================

//-------------------------------------------------------------------------
// StartMusic()
//		o preload = true, music is cached but not started
//-------------------------------------------------------------------------
void StartMusic(boolean preload)
{
	musicnames	musicchunk;

	SD_MusicOff();
#if IN_DEVELOPMENT || GAME_VERSION != SHAREWARE_VERSION || TECH_SUPPORT_VERSION
	if (gamestate.flags & GS_MUSIC_TEST)
		musicchunk=music_num;
	else
#endif
	if (playstate==ex_victorious)
		musicchunk = FORTRESS_MUS;
	else
		musicchunk = songs[gamestate.mapon+gamestate.episode*MAPS_PER_EPISODE];

	if (!audiosegs[STARTMUSIC+musicchunk])
	{
		MM_BombOnError(false);
		CA_CacheAudioChunk(STARTMUSIC + musicchunk);
		MM_BombOnError(true);
	}

	if (mmerror)
		mmerror = false;
	else
	{
		MM_SetLock(&((memptr)audiosegs[STARTMUSIC + musicchunk]),true);
		if (!preload)
			SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + musicchunk]);
	}
}

/*
=============================================================================

					PALETTE SHIFTING STUFF

=============================================================================
*/

#define NUMREDSHIFTS	6
#define REDSTEPS		8

#define NUMWHITESHIFTS	3
#define WHITESTEPS		20
#define WHITETICS		6


byte	far redshifts[NUMREDSHIFTS][768];
byte	far whiteshifts[NUMREDSHIFTS][768];

int		damagecount,bonuscount;
boolean	palshifted;

extern 	byte	far	vgapal;

/*
=====================
=
= InitRedShifts
=
=====================
*/

void InitRedShifts (void)
{
	byte	far *workptr, far *baseptr;
	int		i,j,delta;


//
// fade through intermediate frames
//
	for (i=1;i<=NUMREDSHIFTS;i++)
	{
		workptr = (byte far *)&redshifts[i-1][0];
		baseptr = &vgapal;

		for (j=0;j<=255;j++)
		{
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			delta = -*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			delta = -*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
		}
	}

	for (i=1;i<=NUMWHITESHIFTS;i++)
	{
		workptr = (byte far *)&whiteshifts[i-1][0];
		baseptr = &vgapal;

		for (j=0;j<=255;j++)
		{
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			delta = 62-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			delta = 0-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
		}
	}
}


/*
=====================
=
= ClearPaletteShifts
=
=====================
*/

void ClearPaletteShifts (void)
{
	bonuscount = damagecount = 0;
}


/*
=====================
=
= StartBonusFlash
=
=====================
*/

void StartBonusFlash (void)
{
	bonuscount = NUMWHITESHIFTS*WHITETICS;		// white shift palette
}


/*
=====================
=
= StartDamageFlash
=
=====================
*/

void StartDamageFlash (int damage)
{
	damagecount += damage;
}


/*
=====================
=
= UpdatePaletteShifts
=
=====================
*/

void UpdatePaletteShifts (void)
{
	int	red,white;

	if (bonuscount)
	{
		white = bonuscount/WHITETICS +1;
		if (white>NUMWHITESHIFTS)
			white = NUMWHITESHIFTS;
		bonuscount -= tics;
		if (bonuscount < 0)
			bonuscount = 0;
	}
	else
		white = 0;


	if (damagecount)
	{
		red = damagecount/10 +1;
		if (red>NUMREDSHIFTS)
			red = NUMREDSHIFTS;

		damagecount -= tics;
		if (damagecount < 0)
			damagecount = 0;
	}
	else
		red = 0;

	if (red)
	{
		VW_WaitVBL(1);
		VL_SetPalette (0,256,redshifts[red-1]);
		palshifted = true;
	}
	else if (white)
	{
		VW_WaitVBL(1);
		VL_SetPalette (0,256,whiteshifts[white-1]);	
		palshifted = true;
	}
	else if (palshifted)
	{
		VW_WaitVBL(1);
		VL_SetPalette (0,256,&vgapal);		// back to normal
		palshifted = false;
	}
}


/*
=====================
=
= FinishPaletteShifts
=
= Resets palette to normal if needed
=
=====================
*/

void FinishPaletteShifts (void)
{
	if (palshifted)
	{
		palshifted = 0;
		VW_WaitVBL(1);
		VL_SetPalette (0,256,&vgapal);
	}
}


/*
=============================================================================

						CORE PLAYLOOP

=============================================================================
*/


/*
=====================
=
= DoActor
=
=====================
*/

void DoActor (objtype *ob)
{
	void (*think)(objtype *);
	objtype *actor;


	if (ob->flags & FL_FREEZE)
		return;

#pragma warn -pia
	if (ob->flags & FL_BARRIER)
	{
		actor = actorat[ob->tilex][ob->tiley];
		if (BARRIER_STATE(ob) == bt_ON)
		{
			if (actor)
			{
				short damage = 0;

				actor->flags |= FL_BARRIER_DAMAGE;
				if ((US_RndT() < 0x7f) && (actor->flags & FL_SHOOTABLE))
				{
					switch (ob->obclass)
					{
						case arc_barrierobj:			// arc barrier - Mild Damage
							damage = 500;			// 100
						break;

						case post_barrierobj:		// post barrier - Butt kicker
							damage = 500;
						break;
					}
					DamageActor(actor,damage,ob);
				}
			}
		}
		else
			if (actor)
				actor->flags &= ~FL_BARRIER_DAMAGE;
	}
#pragma warn +pia

	if (!ob->active && !areabyplayer[ob->areanumber])
		return;

	if (!(ob->flags&(FL_NONMARK|FL_NEVERMARK)) )
		actorat[ob->tilex][ob->tiley] = NULL;

//
// non transitional object
//

	if (!ob->ticcount)
	{
		think =	ob->state->think;
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		if (!(ob->flags&FL_NEVERMARK) )
		{
			if (ob->flags&FL_NONMARK)
			{
				if (actorat[ob->tilex][ob->tiley])
				{
					return;
				}
			}
			actorat[ob->tilex][ob->tiley] = ob;
		}
		return;
	}

//
// transitional object
//
	ob->ticcount-=tics;
	while ( ob->ticcount <= 0)
	{
		think = ob->state->action;			// end of state action
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		ob->state = ob->state->next;

		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}

		if (!ob->state->tictime)
		{
			ob->ticcount = 0;
			goto think;
		}

		ob->ticcount += ob->state->tictime;
	}

think:
	//
	// think
	//
	think =	ob->state->think;
	if (think)
	{
		think (ob);
		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}
	}

	if ( !(ob->flags&FL_NEVERMARK) )
	{
		if (ob->flags&FL_NONMARK)
		{
			if (actorat[ob->tilex][ob->tiley])
			{
				return;
			}
		}
		actorat[ob->tilex][ob->tiley] = ob;
	}
	return;
}

//==========================================================================


/*
===================
=
= PlayLoop
=
===================
*/

extern boolean ShowQuickMsg;


void PlayLoop (void)
{
	boolean reset_areas=false;
	int		give;
	objtype *obj;

	playstate = TimeCount = lasttimecount = 0;
	framecount = frameon = 0;
	pwallstate = anglefrac = 0;
	memset (buttonstate,0,sizeof(buttonstate));
	ClearPaletteShifts ();
   ForceUpdateStatusBar();

	if (MousePresent)
		Mouse(MDelta);	// Clear accumulated mouse movement

	tics = 1;			// for first time through
	if (demoplayback)
		IN_StartAck ();

	do
	{
		PollControls();

//
// actor thinking
//
		madenoise = false;

		if (alerted)
			alerted--;

		MoveDoors ();
		MovePWalls ();

		for (obj = player;obj;obj = obj->next)
		{
			if ((obj != player) && (Keyboard[sc_6] || Keyboard[sc_7]) && Keyboard[sc_8] && DebugOk)
			{
				if (!reset_areas)
					memset(areabyplayer,1,sizeof(areabyplayer));
				reset_areas=true;

				if ((((!(obj->flags & FL_INFORMANT)) && (obj->flags & FL_SHOOTABLE))) ||
					 (obj->obclass == liquidobj && !(obj->flags & FL_DEADGUY)))
					DamageActor(obj,1000,player);
			}
			else
				if (reset_areas)
				{
					ConnectAreas();
					reset_areas=false;
				}
			DoActor (obj);
		}

		if (NumEAWalls)
			CheckSpawnEA();

		if ((!GoldsternInfo.GoldSpawned) && GoldsternInfo.SpawnCnt)
			CheckSpawnGoldstern();

		UpdatePaletteShifts ();


		ThreeDRefresh ();

		gamestate.TimeCount+=tics;

		SD_Poll ();
		UpdateSoundLoc();	// JAB

		if (screenfaded & !playstate)
			VW_FadeIn();

	// Display first-time instructions.
	//
		if (ShowQuickMsg)
			ShowQuickInstructions();

		CheckKeys();

		if (demoplayback && demoptr == lastdemoptr)
			playstate = ex_title;

//
// debug aids
//
		if (singlestep)
		{
			VW_WaitVBL(14);
			lasttimecount = TimeCount;
		}
		if (extravbls)
			VW_WaitVBL(extravbls);

		if ((demoplayback) && (IN_CheckAck()))
		{
			IN_ClearKeysDown ();
			playstate = ex_abort;
		}


	}while (!playstate && !startgame);

	if (playstate != ex_died)
		FinishPaletteShifts ();

	gamestate.flags &= ~GS_VIRGIN_LEVEL;
}

//--------------------------------------------------------------------------
// ShowQuickInstructions()
//--------------------------------------------------------------------------
void ShowQuickInstructions()
{
	ShowQuickMsg=false;

	if ((demoplayback) || (gamestate.mapon) || (gamestate.flags & GS_QUICKRUN))
		return;

	ThreeDRefresh();
	ThreeDRefresh();
	ClearMemory();
	WindowX=0; WindowY=16; WindowW=320; WindowH=168;
	CacheMessage(QUICK_INFO1_TEXT);
	VW_WaitVBL(120);
	CacheMessage(QUICK_INFO2_TEXT);
	IN_Ack();
	IN_ClearKeysDown();
	PM_CheckMainMem();
	CleanDrawPlayBorder();
}

//--------------------------------------------------------------------------
// CleanDrawPlayBorder()
//--------------------------------------------------------------------------
void CleanDrawPlayBorder()
{
	DrawPlayBorder();
	ThreeDRefresh();
	DrawPlayBorder();
	ThreeDRefresh();
	DrawPlayBorder();
}

