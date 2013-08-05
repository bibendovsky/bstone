// 3D_DEBUG.C

#include "3D_DEF.H"
#pragma hdrstop
#include <BIOS.H>

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/



#define VIEWTILEX	(viewwidth/16)
#define VIEWTILEY	(viewheight/16)

#define MAX_WARP_LEVEL	23

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

boolean ForceLoadDefault=false;

int DebugKeys (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

boolean PP_step=false;
#if IN_DEVELOPMENT
/*
================
=
= PicturePause
=
================
*/
void PicturePause (void)
{
	int			i;
	byte		p;
	unsigned	x;
	byte		far	*dest,far *src;
	memptr		buffer;

#if RESTART_PICTURE_PAUSE
	PP_step=false;
#endif

	VW_ColorBorder (15);
	FinishPaletteShifts ();

	LastScan = 0;
	while (!LastScan);

#if RESTART_PICTURE_PAUSE

	if (LastScan != sc_Enter)
	{
		switch (LastScan)
		{
			case sc_O:
			case sc_Space:
			case sc_Control:
			case sc_Alt:
			break;

			default:
				PP_step=true;
			break;
		}

		VW_ColorBorder (0);
		return;
	}

	VW_ColorBorder (1);
	VW_SetScreen (0,0);

// Save screen to buffer.
//
	ClearMemory ();
	MM_GetPtr (&buffer,64000);

	for (p=0;p<4;p++)
	{
		src = MK_FP(0xa000,displayofs);
		dest = (byte far *)buffer+p;
		VGAREADMAP(p);
		for (x=0;x<16000;x++,dest+=4)
			*dest = *src++;
	}

// Switch to mode 13h VGA.
//
asm	mov	ax,0x13
asm	int	0x10

// Restore buffer to screen.
//
	VL_SetPalette (0,256,&vgapal);
	dest = MK_FP(0xa000,0);
	_fmemcpy (dest,buffer,64000);
	MM_FreePtr(&buffer);

// Enable keyboard and wait for grab.
//
	while (Keyboard[sc_Enter]);
	IN_Shutdown ();
	bioskey(0);

// Restart play
//
	IN_Startup();
	VL_SetVGAPlaneMode();
	VL_SetPalette (0,256,&vgapal);
	LoadLatchMem();
	playstate=ex_transported;
	DrawPlayScreen(false);
	DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;
	playstate=ex_stillplaying;
	PM_CheckMainMem();
	PP_step=true;
	return;

#else
	VW_ColorBorder (0);
	return;
#endif
}

#endif
int	maporgx;
int	maporgy;
enum {mapview,tilemapview,actoratview,visview}	viewtype;

void ViewMap (void);

//===========================================================================

/*
==================
=
= DebugMemory
=
==================
*/
void DebugMemory (void)
{
	int	i,temp;
	char    scratch[80],str[10];
	long	mem;
	spritetype _seg	*block;

	CenterWindow (22,15);

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total     :");
	US_PrintUnsigned (mminfo.mainmem/1024);
	US_Print ("k\nFree      :");
	US_PrintUnsigned (MM_UnusedMemory()/1024);
	US_Print ("k\nWith purge:");
	US_PrintUnsigned (MM_TotalFree()/1024);
	US_Print ("k\nTics      :");
	US_PrintUnsigned (tics);
	US_Print ("\nReal Tics :");
	US_PrintUnsigned (realtics);

#ifdef CEILING_FLOOR_COLORS
   if (gamestate.flags & GS_DRAW_CEILING)
#endif
	{
		US_Print ("\n\nCeiling TEX: ");
		US_PrintUnsigned (CeilingTile-START_TEXTURES);

  		US_Print(" Floor TEX: ");
		US_PrintUnsigned(FloorTile-START_TEXTURES);
	}
#ifdef CEILING_FLOOR_COLORS
   else
   {
		US_Print("\n\nTop COL: ");
		US_PrintUnsigned(TopColor&0xff);

		US_Print(" Bottom COL: ");
		US_PrintUnsigned(BottomColor&0xff);
   }
#endif

	if (gamestate.flags & GS_LIGHTING)
   {
		US_Print ("\nShade div :");
		US_PrintUnsigned (normalshade_div);

		US_Print ("\nShade max :");
		US_PrintUnsigned (shade_max);
	}

   VW_UpdateScreen();
	IN_Ack ();

	temp = bufferofs;
	WindowW = 253;
  	WindowH = 8;
   fontnumber = 2;

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		LatchDrawPic(0,0,TOP_STATUSBARPIC);
		ShadowPrintLocationText(sp_normal);
	}

	bufferofs = temp;
}

//===========================================================================

/*
==================
=
= CountObjects
=
==================
*/

void CountObjects (void)
{
	int	i,total,count,active,inactive,doors;
	objtype	*obj;

	CenterWindow (16,7);
	active = inactive = count = doors = 0;

	US_Print ("Total statics :");
	total = laststatobj-&statobjlist[0];
	US_PrintUnsigned (total);

	US_Print ("\nIn use statics:");
	for (i=0;i<total;i++)
		if (statobjlist[i].shapenum != -1)
			count++;
		else
			doors++;	//debug
	US_PrintUnsigned (count);

	US_Print ("\nDoors         :");
	US_PrintUnsigned (doornum);

	for (obj=player->next;obj;obj=obj->next)
	{
		if (obj->active)
			active++;
		else
			inactive++;
	}

	US_Print ("\nTotal actors  :");
	US_PrintUnsigned (active+inactive);

	US_Print ("\nActive actors :");
	US_PrintUnsigned (active);

	VW_UpdateScreen();
	IN_Ack ();
}


//------------------------------------------------------------------------
// CountTotals
//------------------------------------------------------------------------
void CountTotals(void)
{
	CenterWindow (20,11);

   US_Print ("  CURRENT MAP TOTALS\n");

	US_Print ("\nTotal Enemy:\n");
	US_PrintUnsigned (gamestuff.level[gamestate.mapon].stats.total_enemy);

	US_Print ("\nTotal Points:\n");
	US_PrintUnsigned (gamestuff.level[gamestate.mapon].stats.total_points);

	US_Print ("\nTotal Informants:\n");
	US_PrintUnsigned (gamestuff.level[gamestate.mapon].stats.total_inf);

	VW_UpdateScreen();
	IN_Ack ();
}


//------------------------------------------------------------------------
// ShowMap()
//------------------------------------------------------------------------
void ShowMap(void)
{
   objtype old_player;

   memcpy(&old_player,player,sizeof(objtype));
	player->angle = 90;
	player->x = player->y = ((long)32<<TILESHIFT)+(TILEGLOBAL/2);

	CenterWindow (20,11);

   US_CPrint("CURRENT MAP\n\n ");

	ShowOverhead(160-32,py,32,0,OV_ACTORS|OV_SHOWALL|OV_KEYS|OV_PUSHWALLS);
   VW_UpdateScreen();

   memcpy(player,&old_player,sizeof(objtype));
	IN_Ack ();
}


//===========================================================================

//===========================================================================

#if IN_DEVELOPMENT
/*
================
=
= ShapeTest
=
================
*/

#pragma warn -pia
void ShapeTest (void)
{
extern	word	NumDigi;
extern	word	_seg *DigiList;
static	char	buf[10];

	boolean			done;
	ScanCode		scan;
	int				i,j,k,x;
	int				sound;
	longword		l;
	memptr			addr;
	PageListStruct	far *page;

	CenterWindow(20,16);
	VW_UpdateScreen();
	for (i = 0,done = false;!done;)
	{
		US_ClearWindow();
		sound = -1;

		page = &PMPages[i];
		US_Print(" Page #");
		US_PrintUnsigned(i);
		if (i < PMSpriteStart)
			US_Print(" (Wall)");
		else if (i < PMSoundStart)
			US_Print(" (Sprite)");
		else if (i == ChunksInFile - 1)
			US_Print(" (Sound Info)");
		else
			US_Print(" (Sound)");

		US_Print("\n XMS: ");
		if (page->xmsPage != -1)
			US_PrintUnsigned(page->xmsPage);
		else
			US_Print("No");

		US_Print("\n Main: ");
		if (page->mainPage != -1)
			US_PrintUnsigned(page->mainPage);
		else if (page->emsPage != -1)
		{
			US_Print("EMS ");
			US_PrintUnsigned(page->emsPage);
		}
		else
			US_Print("No");

		US_Print("\n Last hit: ");
		US_PrintUnsigned(page->lastHit);

		US_Print("\n Address: ");
		addr = PM_GetPageAddress(i);
		sprintf(buf,"0x%04x",(word)addr);
		US_Print(buf);

		if (addr)
		{
			if (i < PMSpriteStart)
			{
			//
			// draw the wall
			//
				bufferofs += 32*SCREENWIDTH;
				postx = 128;
				postwidth = 1;
				postsource = ((long)((unsigned)addr))<<16;
				for (x=0;x<64;x++,postx++,postsource+=64)
				{
					wallheight[postx] = 256;
					FarScalePost ();
				}
				bufferofs -= 32*SCREENWIDTH;
			}
			else if (i < PMSoundStart)
			{
			//
			// draw the sprite
			//
				bufferofs += 32*SCREENWIDTH;
				SimpleScaleShape (160, i-PMSpriteStart, 64);
				bufferofs -= 32*SCREENWIDTH;
			}
			else if (i == ChunksInFile - 1)
			{
				US_Print("\n\n Number of sounds: ");
				US_PrintUnsigned(NumDigi);
				for (l = j = k = 0;j < NumDigi;j++)
				{
					l += DigiList[(j * 2) + 1];
					k += (DigiList[(j * 2) + 1] + (PMPageSize - 1)) / PMPageSize;
				}
				US_Print("\n Total bytes: ");
				US_PrintUnsigned(l);
				US_Print("\n Total pages: ");
				US_PrintUnsigned(k);
			}
			else
			{
				byte far *dp = (byte far *)MK_FP(addr,0);

	 			sound = i-PMSoundStart;
				US_Print("\n Sound #");
				US_PrintUnsigned(sound);

				for (j = 0;j < page->length;j += 32)
				{
					byte v = dp[j];
					int v2 = (unsigned)v;
					v2 -= 128;
					v2 /= 4;
					if (v2 < 0)
						VWB_Vlin(WindowY + WindowH - 32 + v2,
								WindowY + WindowH - 32,
								WindowX + 8 + (j / 32),BLACK);
					else
						VWB_Vlin(WindowY + WindowH - 32,
								WindowY + WindowH - 32 + v2,
								WindowX + 8 + (j / 32),BLACK);
				}
			}
		}

		VW_UpdateScreen();

		while (!(scan = LastScan))
			SD_Poll();

		IN_ClearKey(scan);
		switch (scan)
		{
		case sc_LeftArrow:
			if (i)
				i--;
			break;
		case sc_RightArrow:
			if (++i >= ChunksInFile)
				i--;
			break;
		case sc_W:	// Walls
			i = 0;
			break;
		case sc_S:	// Sprites
			i = PMSpriteStart;
			break;
		case sc_D:	// Digitized
			i = PMSoundStart;
			break;
		case sc_I:	// Digitized info
			i = ChunksInFile - 1;
			break;
		case sc_L:	// Load all pages
			for (j = 0;j < ChunksInFile;j++)
				PM_GetPage(j);
			break;
		case sc_P:
			if (sound != -1)
				SD_PlaySound(sound);
			break;
		case sc_Escape:
			done = true;
			break;
		case sc_Enter:
			PM_GetPage(i);
			break;
		}
	}
	SD_StopDigitized();

	RedrawStatusAreas();
}
#pragma warn +pia

#endif


//===========================================================================


//---------------------------------------------------------------------------
// IncRange - Incs a value to a MAX value (including max value)
//
// NOTE: Assumes that 0 is the lowest value
//---------------------------------------------------------------------------
unsigned IncRange(unsigned Value,unsigned MaxValue)
{
	if (Value == MaxValue)
   	Value = 0;
   else
   	Value++;

   return (Value);
}

//---------------------------------------------------------------------------
// DecRange - Decs a value to 0 and resets to MAX_VALUE
//
// NOTE: Assumes that 0 is the lowest value
//---------------------------------------------------------------------------
unsigned DecRange(unsigned Value,unsigned MaxValue)
{
	if (Value == 0)
		Value = MaxValue;
	else
		Value--;

	return (Value);
}



/*
================
=
= DebugKeys
=
================
*/

#if IN_DEVELOPMENT
char far TestAutoMapperMsg[] = {"AUTOMAPPER TEST\n ENTER COUNT:"};
char far TestQuickSaveMsg[] = {"QUICK SAVE TEST\n ENTER COUNT:"};
#endif


int DebugKeys (void)
{
	char str[3];
	boolean esc;
	int level,i;

   if (Keyboard[sc_A])		// A = Show Actors on AutoMap
	{
   	ExtraRadarFlags ^= OV_ACTORS;
		CenterWindow (24,3);
		if (ExtraRadarFlags & OV_ACTORS)
		  US_PrintCentered ("AUTOMAP: Show Actors ON");
		else
		  US_PrintCentered ("AUTOMAP: Show Actors OFF");
		VW_UpdateScreen();
		IN_Ack();
		return 1;
   }
   else
	if (Keyboard[sc_B])		// B = border color
	{
		CenterWindow(24,3);
		PrintY+=6;
		US_Print(" Border color (0-15):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=15)
				VW_ColorBorder (level);
		}
		return 1;
	}

	if (Keyboard[sc_K])		// K = Map Content totals
	{
		CountTotals();
		return 1;
	}
	else if (Keyboard[sc_C])		// C = count objects
	{
		CountObjects();
		return 1;
	}
	else if (Keyboard[sc_R])		// C = count objects
	{
		ShowMap();
		return 1;
	}
	else if (Keyboard[sc_D])			// D = Dumb/Blind Objects (Player Invisable)
	{
		CenterWindow (19,3);
		PlayerInvisable ^= 1;
		if (PlayerInvisable)
			US_PrintCentered ("Player Invisible!");
		else
			US_PrintCentered ("Player visible");

		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_E])			// E = Win Mission
	{
		CenterWindow (19,3);
		US_PrintCentered ("Instant Wiener!");
      InstantWin = 1;
      playstate = ex_victorious;
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else
	if (Keyboard[sc_F])		// F = facing spot
	{
		CenterWindow (18,5);
		US_Print ("X:");
		US_PrintUnsigned (player->x);
		US_Print ("  ");
		US_PrintUnsigned (player->x>>TILESHIFT);
		US_Print ("\nY:");
		US_PrintUnsigned (player->y);
		US_Print ("  ");
		US_PrintUnsigned (player->y>>TILESHIFT);
		US_Print ("\nA:");
		US_PrintUnsigned (player->angle);
		US_Print ("\nD:");
		US_PrintUnsigned (player->dir);
		VW_UpdateScreen();
		IN_Ack();
		return 1;
	}

	if (Keyboard[sc_G])		// G = god mode
	{
		CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= 1;
		return 1;
	}


	if (Keyboard[sc_H])		// H = hurt self
	{
		IN_ClearKeysDown ();
		TakeDamage (1,NULL);
	}
	else
	if (Keyboard[sc_I])			// I = item cheat
	{
		char loop;
		CenterWindow (12,3);
		US_PrintCentered ("Free items!");
		VW_UpdateScreen();
		HealSelf (99);
		GiveToken(5);

		for (i=wp_autocharge;i<=wp_bfg_cannon;i++)
			if (!(gamestate.weapons & (1<<i)))
			{
				GiveWeapon (i);
				break;
			}

		gamestate.ammo += 50;
		if (gamestate.ammo > MAX_AMMO)
			gamestate.ammo = MAX_AMMO;
		DrawAmmo(true);
      DrawScore();
		IN_Ack ();
		return 1;
	}
	else
	if (Keyboard[sc_M])			// M = memory info
	{
		DebugMemory();
		return 1;
	}
#if IN_DEVELOPMENT
#if (!BETA_TEST)
	else if (Keyboard[sc_N])			// N = no clip
	{
		gamestate.flags ^= GS_CLIP_WALLS;
		CenterWindow (18,3);
		if (gamestate.flags & GS_CLIP_WALLS)
			US_PrintCentered ("NO clipping OFF");
		else
			US_PrintCentered ("No clipping ON");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
#endif
	else if (Keyboard[sc_P])			// P = pause with no screen disruptioon
	{
		PicturePause ();
		return 1;
	}
#endif
	else if (Keyboard[sc_Q])			// Q = fast quit
		Quit (NULL);
#if IN_DEVELOPMENT
	else if (Keyboard[sc_T])			// T = shape test
	{
		ShapeTest ();
		return 1;
	}
#endif
	else if (Keyboard[sc_O])			// O = Show Push Walls
   {
   	ExtraRadarFlags ^= OV_PUSHWALLS;
		CenterWindow (24,3);
		if (ExtraRadarFlags & OV_PUSHWALLS)
		  US_PrintCentered ("AUTOMAP: Show PWalls ON");
		else
		  US_PrintCentered ("AUTOMAP: Show PWalls OFF");
		VW_UpdateScreen();
		IN_Ack();
   	return 1;
   }
	else if (Keyboard[sc_U])			// Unlock All Floors
	{
   	int i;
		CenterWindow (24,3);
	  	US_PrintCentered ("Unlock All Floors!");
		VW_UpdateScreen();
		IN_Ack();

		for (i=0;i<11;i++)
			gamestuff.level[i].locked=false;

      return 1;
	}
	else if (Keyboard[sc_V])			// V = extra VBLs
	{
		CenterWindow(30,3);
		PrintY+=6;
		US_Print("  Add how many extra VBLs(0-8):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=8)
				extravbls = level;
		}
		return 1;
	}
	else
	if (Keyboard[sc_S])			// S = slow motion
	{
		singlestep^=1;
		CenterWindow (18,3);
		if (singlestep)
			US_PrintCentered ("Slow motion ON");
		else
			US_PrintCentered ("Slow motion OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_W])			// W = warp to level
	{
		ForceLoadDefault=Keyboard[sc_LShift]|Keyboard[sc_RShift]|Keyboard[sc_CapsLock];

		CenterWindow(26,5);
		PrintY+=6;
		if (ForceLoadDefault)
			US_Print("         --- LOAD DEFAULT ---\n");
		US_Print("  Current map: ");
		US_PrintUnsigned(gamestate.mapon);
		US_Print("\n  Enter map number: ");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>-1 && level<=MAX_WARP_LEVEL)
			{
				gamestate.lastmapon = gamestate.mapon;
				gamestate.mapon = level-1;
				playstate = ex_warped;
				if (ForceLoadDefault)
					BONUS_QUEUE = BONUS_SHOWN = 0;
			}
		}
		return 1;
	}
	else if (Keyboard[sc_Home])		// Dec top color
	{
#ifdef CEILING_FLOOR_COLORS
		if (gamestate.flags & GS_DRAW_CEILING)
#endif
		{
			CeilingTile = DecRange(CeilingTile,NUM_TILES-1);
			SetPlaneViewSize ();     // Init new textures
			return(1);
		}
#ifdef CEILING_FLOOR_COLORS
		else
		{
			TopColor = DecRange((TopColor&0xff),0xff);
			TopColor |= (TopColor<<8);
		}
#endif
	}
	else if (Keyboard[sc_PgUp])		// Inc top color
	{
#ifdef CEILING_FLOOR_COLORS
		if (gamestate.flags & GS_DRAW_CEILING)
#endif
		{
			CeilingTile = IncRange(CeilingTile,NUM_TILES-1);
			SetPlaneViewSize ();     // Init new textures
			return(1);
		}
#ifdef CEILING_FLOOR_COLORS
		else
		{
			TopColor = IncRange((TopColor&0xff),0xff);
			TopColor |= (TopColor<<8);
		}
#endif
	}
	else if (Keyboard[sc_End])			// Dec bottom color
	{
#ifdef CEILING_FLOOR_COLORS
		if (gamestate.flags & GS_DRAW_FLOOR)
#endif
		{
			FloorTile = DecRange(FloorTile,NUM_TILES-1);
			SetPlaneViewSize ();     // Init new textures
			return(1);
		}
#ifdef CEILING_FLOOR_COLORS
		else
		{
			BottomColor = DecRange((BottomColor&0xff),0xff);
			BottomColor |= (BottomColor<<8);
		}
#endif
	}
	else if (Keyboard[sc_PgDn])		// Inc bottom color
	{
#ifdef CEILING_FLOOR_COLORS
		if (gamestate.flags & GS_DRAW_FLOOR)
#endif
		{
			FloorTile = IncRange(FloorTile,NUM_TILES-1);
			SetPlaneViewSize ();     // Init new textures
			return(1);
		}
#ifdef CEILING_FLOOR_COLORS
		else
		{
			BottomColor = IncRange((BottomColor&0xff),0xff);
			BottomColor |= (BottomColor<<8);
		}
#endif
	}

#if (IN_DEVELOPMENT)
#if !BETA_TEST
	else if (Keyboard[sc_Y])
	{
		GivePoints(100000L,false);
	}
#endif
#endif

	if (gamestate.flags & GS_LIGHTING)			// Shading adjustments
	{
   	if (Keyboard[sc_Plus] && normalshade_div < 12)
      	normalshade_div++;
      else
   	if (Keyboard[sc_Minus] && normalshade_div>1)
      	normalshade_div--;

		normalshade=(3*(maxscale>>2))/normalshade_div;

   	if (Keyboard[sc_RBrace] && shade_max < 63)
      	shade_max++;
      else
   	if (Keyboard[sc_LBrace] && shade_max > 5)
      	shade_max--;
	}

	return 0;
}

#if 0
/*
===================
=
= OverheadRefresh
=
===================
*/

void OverheadRefresh (void)
{
	unsigned	x,y,endx,endy,sx,sy;
	unsigned	tile;


	endx = maporgx+VIEWTILEX;
	endy = maporgy+VIEWTILEY;

	for (y=maporgy;y<endy;y++)
		for (x=maporgx;x<endx;x++)
		{
			sx = (x-maporgx)*16;
			sy = (y-maporgy)*16;

			switch (viewtype)
			{
#if 0
			case mapview:
				tile = *(mapsegs[0]+farmapylookup[y]+x);
				break;

			case tilemapview:
				tile = tilemap[x][y];
				break;

			case visview:
				tile = spotvis[x][y];
				break;
#endif
			case actoratview:
				tile = (unsigned)actorat[x][y];
				break;
			}

			if (tile<MAXWALLTILES)
				LatchDrawTile(sx,sy,tile);
			else
			{
				LatchDrawChar(sx,sy,NUMBERCHARS+((tile&0xf000)>>12));
				LatchDrawChar(sx+8,sy,NUMBERCHARS+((tile&0x0f00)>>8));
				LatchDrawChar(sx,sy+8,NUMBERCHARS+((tile&0x00f0)>>4));
				LatchDrawChar(sx+8,sy+8,NUMBERCHARS+(tile&0x000f));
			}
		}

}


/*
===================
=
= ViewMap
=
===================
*/

void ViewMap (void)
{
	boolean		button0held;

	viewtype = actoratview;
//	button0held = false;


	maporgx = player->tilex - VIEWTILEX/2;
	if (maporgx<0)
		maporgx = 0;
	if (maporgx>MAPSIZE-VIEWTILEX)
		maporgx=MAPSIZE-VIEWTILEX;
	maporgy = player->tiley - VIEWTILEY/2;
	if (maporgy<0)
		maporgy = 0;
	if (maporgy>MAPSIZE-VIEWTILEY)
		maporgy=MAPSIZE-VIEWTILEY;

	do
	{
//
// let user pan around
//
		PollControls ();
		if (controlx < 0 && maporgx>0)
			maporgx--;
		if (controlx > 0 && maporgx<mapwidth-VIEWTILEX)
			maporgx++;
		if (controly < 0 && maporgy>0)
			maporgy--;
		if (controly > 0 && maporgy<mapheight-VIEWTILEY)
			maporgy++;

#if 0
		if (c.button0 && !button0held)
		{
			button0held = true;
			viewtype++;
			if (viewtype>visview)
				viewtype = mapview;
		}
		if (!c.button0)
			button0held = false;
#endif

		OverheadRefresh ();

	} while (!Keyboard[sc_Escape]);

	IN_ClearKeysDown ();
}
#endif



#if IN_DEVELOPMENT
//-------------------------------------------------------------------------
// CalcMemFree()
//-------------------------------------------------------------------------
void CalcMemFree(void)
{
	__PUR_MEM_AVAIL__ = MM_TotalFree();
	__FREE_MEM_AVAIL__ = MM_UnusedMemory();
}
#endif
