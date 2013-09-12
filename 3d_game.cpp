// 3D_GAME.C

#include "3d_def.h"

#ifdef MSVC
#pragma hdrstop
#endif

#ifdef MYPROFILE
#include <ctime>
#endif

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define LOCATION_TEXT_COLOR 0xAF
extern char prep_msg[];
extern char LS_current,LS_total;
void Died (void);
void PM_SetMainMemPurge(Sint16 level);
void InitGoldsternInfo(void);
void InitDoorList (void);
void InitStaticList (void);
void ConnectBarriers(void);
void DrawHealth (void);
void DrawKeys (void);
void DrawWeapon (void);
void DrawScore(void);
void InitInfoArea(void);
void ForceUpdateStatusBar(void);
void UpdateStatusBar(void);
boolean LoadLevel(int levelnum);
void SetPlaneViewSize (void);
Sint16 CalcAngle(objtype* from_obj, objtype* to_obj);
void FinishPaletteShifts (void);
void CA_CacheScreen (Sint16 chunk);
void VH_UpdateScreen();
void DoActor (objtype *ob);
boolean LevelInPlaytemp(char levelnum);
void PreloadUpdate(Uint16 current, Uint16 total);
void PreloadGraphics(void);
boolean SaveLevel(int levelnum);
Sint16 NextBuffer();
void CheckHighScore (Sint32 score,Uint16 other);


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

#if IN_DEVELOPMENT
Sint16 db_count=0;
#ifdef DEBUG_STATICS
classtype debug_bonus[2][800];
#endif
#endif

fargametype gamestuff;
gametype gamestate;
boolean		ingame,fizzlein;
Uint16	latchpics[NUMLATCHPICS];
eaWallInfo eaList[MAXEAWALLS];
char NumEAWalls;

tilecoord_t GoldieList[GOLDIE_MAX_SPAWNS];	 
GoldsternInfo_t GoldsternInfo;

extern Uint16 scan_value;


//
// ELEVATOR BACK MAPS - REMEMBER (-1)!!
//
// int ElevatorBackTo[]={1,1,7,3,5,3};

void ScanInfoPlane (void);
void SetupGameLevel (void);
void DrawPlayScreen (boolean InitInfoMsg);
void LoadLatchMem (void);
void GameLoop (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



//
// NOTE: This array indexs the "statinfo" array in ACT1.C and is indexed
//		   upon tile number/values.
//

char ExpCrateShapes[] =	 
{
	42,			// Chicken Leg
	44,			// Ham/Steak
	26,			// Clip
	24,			// Pistol
	27,			// Pulse
	28,			// ION
   46,			// Grenade
   62,			// Money Bag
   63,			// Loot
   64,			// Gold
   65,			// Bonus
   71,			// Gore 1
   74,			// Gore 2
	32,			// red key
	33,			// yel key
   52-23,		// grn key
   35,			// blu key
   488-375,		// gld key
};





//===========================================================================
//===========================================================================


/*
==========================
=
= SetSoundLoc - Given the location of an object (in terms of global
=	coordinates, held in globalsoundx and globalsoundy), munges the values
=	for an approximate distance from the left and right ear, and puts
=	those values into leftchannel and rightchannel.
=
= JAB
=
==========================
*/

	fixed	globalsoundx,globalsoundy;
	Sint16		leftchannel,rightchannel;
#define ATABLEMAX 15
Uint8 righttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
Uint8 lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

void
SetSoundLoc(fixed gx,fixed gy)
{
	fixed	xt,yt;
	Sint16		x,y;

//
// translate point to view centered coordinates
//
	gx -= viewx;
	gy -= viewy;

//
// calculate newx
//
	xt = FixedByFrac(gx,viewcos);
	yt = FixedByFrac(gy,viewsin);
	x = (xt - yt) >> TILESHIFT;

//
// calculate newy
//
	xt = FixedByFrac(gx,viewsin);
	yt = FixedByFrac(gy,viewcos);
	y = (yt + xt) >> TILESHIFT;

	if (y >= ATABLEMAX)
		y = ATABLEMAX - 1;
	else if (y <= -ATABLEMAX)
		y = -ATABLEMAX;
	if (x < 0)
		x = -x;
	if (x >= ATABLEMAX)
		x = ATABLEMAX - 1;
	leftchannel  =  lefttable[x][y + ATABLEMAX];
	rightchannel = righttable[x][y + ATABLEMAX];

#if 0
	CenterWindow(8,1);
	US_PrintSigned(leftchannel);
	US_Print(",");
	US_PrintSigned(rightchannel);
	VW_UpdateScreen();
#endif
}

/*
==========================
=
= SetSoundLocGlobal - Sets up globalsoundx & globalsoundy and then calls
=	UpdateSoundLoc() to transform that into relative channel volumes. Those
=	values are then passed to the Sound Manager so that they'll be used for
=	the next sound played (if possible).
=
= JAB
=
==========================
*/
void PlaySoundLocGlobal(Uint16 s,fixed gx,fixed gy)
{
	SetSoundLoc(gx,gy);
	SD_PositionSound(leftchannel,rightchannel);
	if (SD_PlaySound(static_cast<soundnames>(s)))
	{
		globalsoundx = gx;
		globalsoundy = gy;
	}
}

void UpdateSoundLoc(void)
{
	if (SoundPositioned)
	{
		SetSoundLoc(globalsoundx,globalsoundy);
		SD_SetPosition(leftchannel,rightchannel);
	}
}

/*
**	JAB End
*/


/*
==========================
=
= ClearMemory
=
==========================
*/

void ClearMemory (void)
{
// FIXME
#if 0
	PM_UnlockMainMem();
#endif // 0

	SD_StopDigitized();

// FIXME
#if 0
	MM_SortMem ();
#endif // 0
}

#if 0

//-------------------------------------------------------------------------
// FreeAllMemory() - This is an attempt to FREE All possible memory
//							for memory hungry routines (ie. MOVIE_Play())
//
// NOTE: If any sounds, music, etc are currently playing, they will
//			adbruply cut off.
//-------------------------------------------------------------------------
void FreeAllMemory(void)
{
	StopMusic();
	SD_StopSound();
	ClearMemory();
}

#endif

#ifdef TRACK_ENEMY_COUNT
Sint16 numEnemy[gold_morphingobj];
#endif

/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/

#define INVALID_ACTOR_ERR Quit("Invalid actor: %d %d",x,y)

void ScanInfoPlane (void)
{
	Uint16	x,y;
	Sint16			tile;
	Uint16	*start;
	boolean gotlight = false,gottextures = false;

#ifdef CEILING_FLOOR_COLORS
	boolean gotcolors=false;
#endif

#if IN_DEVELOPMENT
	db_count=0;
#endif
	detonators_spawned = 0;

#ifdef TRACK_ENEMY_COUNT
memset(numEnemy,0,sizeof(numEnemy));
#endif

	new_actor=NULL;
	start = mapsegs[1];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			sci_mCacheInfo *ci;
			scientist_t *st;
			Uint8 tilehi,tilelo,block;


         tile = *start++;
         //
         // Check for tiles/icons to ignore...
         //
         switch ((Uint16)*(mapsegs[0]+farmapylookup[y]+x))
         {
         	case DOORTRIGGERTILE:
				case SMART_OFF_TRIGGER:
				case SMART_ON_TRIGGER:
            	// Ignore all values/icons on top of these tiles...
           	continue;
         }
			tilehi = (tile&0xff00)>>8;
			tilelo = (tile&0xff);

			if ((*start & 0xff00) == 0xfa00)
				scan_value = *start & 0x00ff;
			else
				scan_value = 0xffff;

			switch (tilehi)
			{


//				case 0xff:											// Special background
//					continue;
//				break;

#ifdef CEILING_FLOOR_COLORS
				case 0xfe:								// Top/Bottom colors
					if (gotcolors)
						break;
					x++;
					tile = *start++;
					TopColor = tile&0xff00;
					TopColor |= TopColor>>8;
					BottomColor = tile&0xff;
					BottomColor |= BottomColor<<8;
					gotcolors=true;
					continue;
				break;
#else
				case 0xfe:								// Top/Bottom colors
					x++;
					tile = *start++;
               // Give error
					continue;
				break;
#endif


				case 0xFB:								// Global Ceiling/Floor textures
					if (gottextures)
						break;
					x++;
					tile = *start++;

					CeilingTile = START_TEXTURES+((tile&0xff00)>>8);
					if (CeilingTile > NUM_TILES-1)
						GAME_ERROR(CEILING_TILE_OUT_OF_RANGE);

					FloorTile = START_TEXTURES+(tile&0xff);
               if (FloorTile > NUM_TILES-1)
						GAME_ERROR(FLOOR_TILE_OUT_OF_RANGE);

					gottextures = true;
					continue;
				break;


				case 0xf5:									// IntraLevel warp
					*(start-1) = *start;					// Move Coord right on top
					*start = 0;
					continue;
				break;

#if 0
				case 0xF6:									// Lighting effects
					if (gotlight)
						break;
					x++;
					tile = *start++;
					normalshade_div = (tile&0xff00)>>8 ;
					if (normalshade_div > 12)
						AGENT_ERROR(NORMAL_SHADE_TOO_BIG);

					shade_max = tile&0xff;
					if (shade_max > 63 || shade_max < 5)
						AGENT_ERROR(SHADEMAX_VALUE_BAD);

					normalshade=(3*(maxscale>>2))/normalshade_div;
					gotlight = true;
					continue;
				break;
#endif

				case 0xfa:
					continue;

				case 0xf1:		// Informant messages
				case 0xf2:		// "Nice" scientist messages
				case 0xf3:		// "Mean" scientist messages
					switch (tilehi)
					{
						case 0xf1:
							block=INFORMANT_HINTS;
							st=&InfHintList;
						break;

						case 0xf2:
							block=NICE_SCIE_HINTS;
							st=&NiceSciList;
						break;

						case 0xf3:
							block=MEAN_SCIE_HINTS;
							st=&MeanSciList;
						break;
					}

					ci = &st->smInfo[st->NumMsgs];
					ci->mInfo.local_val = 0xff;
					ci->mInfo.global_val = tilelo;
					if (!ReuseMsg((mCacheInfo *)ci,st->NumMsgs,sizeof(sci_mCacheInfo)))
					{
						CacheMsg((mCacheInfo *)ci,block,ci->mInfo.global_val);
						ci->mInfo.local_val = InfHintList.NumMsgs;
					}

					if (++st->NumMsgs > MAX_CACHE_MSGS)
						GAME_ERROR(SCANINFO_CACHE_MSG_OVERFLOW);

					ci->areanumber=GetAreaNumber(x,y);

					if (ci->areanumber >= NUMAREAS)
						ci->areanumber = 0xff;
					continue;
				break;

				case 0:
					if (!tilelo)
						continue;
				break;
			}

			//
			// SPECIAL SPAWN CODING FOR BLASTABLE CRATES...
			//

			if (tile >=432 && tile <=485)
			{
#if GAME_VERSION != SHAREWARE_VERSION
				if (tile>=468)
				{
					SpawnOffsetObj(en_crate3,x,y);
					new_actor->temp2 = ExpCrateShapes[tile - 468];

// FIXME
#if 0
					new_actor->temp3 = (Uint16)ReserveStatic();
#endif // 0

                    new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if ((tile >= 475) && (tile <= 478))
						tile=(tile-475)+bo_money_bag;
					else
						tile=0;
				}
				else
				if (tile >=450)
				{
					SpawnOffsetObj(en_crate2,x,y);
					new_actor->temp2 = ExpCrateShapes[tile - 450];

// FIXME
#if 0
					new_actor->temp3 = (Uint16)ReserveStatic();
#endif // 0

                    new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if ((tile >= 457) && (tile <= 460))
						tile=(tile-457)+bo_money_bag;
					else
						tile=0;
				}
				else
#endif
#if GAME_VERSION == SHAREWARE_VERSION
#if IN_DEVELOPMENT
				if (tile >= 450)
					INVALID_ACTOR_ERR;
				else
#endif
#endif
				if (tile >= 432)
				{
					SpawnOffsetObj(en_crate1,x,y);
					new_actor->temp2 = ExpCrateShapes[tile - 432];

// FIXME
#if 0
					new_actor->temp3 = (Uint16)ReserveStatic();
#endif // 0

                    new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if ((tile >= 439) && (tile <= 442))
						tile=(tile-439)+bo_money_bag;
					else
						tile=0;
				}

				if (tile)
				{
					if (tile > bo_loot)
						tile += 3;
					tile -= bo_money_bag;
					AddTotalPoints(static_points[tile]);
#if IN_DEVELOPMENT
#ifdef DEBUG_STATICS
					debug_bonus[0][db_count++] = static_points[tile];
#endif
#endif
				}

				continue;
			}

			switch (tile)
			{
			case 19:
			case 20:
			case 21:
			case 22:
				SpawnPlayer(x,y,NORTH+tile-19);
			break;

			case 85:					// Money bag
			case 86:					// Loot
			case 87:					// Gold
			case 88:					// Bonus
					AddTotalPoints(static_points[statinfo[tile-23].type-bo_money_bag]);
#if IN_DEVELOPMENT
#ifdef DEBUG_STATICS
					debug_bonus[0][db_count++] = static_points[statinfo[tile-23].type-bo_money_bag];
#endif
#endif

			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:


#if GAME_VERSION != SHAREWARE_VERSION
			case 30:									// Yellow Puddle
#endif

			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 38:

			case 39:
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:

			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:

			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:

			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:
         case 71:					// BFG Weapon		  
			case 72:					// Gurney Mutant
			case 73:             // Large Canister
			case 74:             // Small Canister
			case 75:             // Empty Gurney
			case 76:					// Empty Large Canister
			case 77:					// Empty Small Canister
			case 78:					// Dead Gen. Sci.

			case 80:
			case 83:					// Floor Grate
			case 84:					// Floor Pipe
				SpawnStatic(x,y,tile-23);
			break;

			case 399:				// gold 1
			case 400:				// gold 2
			case 401:				// gold 3
					AddTotalPoints(static_points[statinfo[tile-315].type-bo_money_bag]);
#if IN_DEVELOPMENT
#ifdef DEBUG_STATICS
					debug_bonus[0][db_count++] = static_points[statinfo[tile-315].type-bo_money_bag];
#endif
#endif

			case 381:
			case 382:
			case 383:
			case 384:
			case 385:
			case 386:
			case 387:
			case 388:
//			case 389:
			case 390:      		// candy bar
			case 391:				// sandwich

			case 395:				// Table
			case 396:				// Chair
			case 397:				// Stool
			case 398:				// Gore

			case 402:				//
			case 403:				//
			case 404:				//
			case 405:				//
			case 406:				//
			case 407:				//
			case 408:				//
			case 409:				//
			case 410:				//
			case 411:				//
			case 412:				//
			case 413:				//
			case 414:				//
			case 415:				//
			case 416:				//
			case 417:				//
			case 418:				//
			case 419:				//
			case 420:				//
			case 421:				//
			case 422:				//
			case 423:				// bo_coin
			case 424:				// bo_coin5
				SpawnStatic(x,y,tile-315);
			break;

			case 486:				// Plasma Detonator
				SpawnHiddenOfs(en_plasma_detonator_reserve, x, y);			// Spawn a reserve  
				SpawnStatic(x,y,486-375);
			break;

			case 487:				// Door rubble
         case 488:				// AutoMapper Bonus #1
         case 489:				// BonziTree
         case 490:				// Yellow Potted plant
         case 491:				// Tube Plant
         case 492:				// HiTech Chair
         case 493:				// AOG: Rent A Cop - Dead.
         case 494:				// AOG: Pro Guard - Dead.
			case 495:				// AOG: Swat Guard - Dead.
				SpawnStatic(x,y,tile-375);
			break;


			case 393:				// crate 2
			case 394:				// crate 3
#if GAME_VERSION == SHAREWARE_VERSION
#if IN_DEVELOPMENT
				INVALID_ACTOR_ERR;
#endif
#endif
			case 392:				// crate 1
				SpawnStatic(x,y,tile-315);
			break;

			case 81:
			case 82:
				SpawnOffsetObj(static_cast<enemy_t>(en_bloodvent+tile-81),x,y);
			break;


			//
			// GREEN OOZE
			//

			case 208:
				if (gamestate.difficulty<gd_hard)
					break;
			case 207:
				if (gamestate.difficulty<gd_medium)
					break;
			case 206:
				SpawnOffsetObj(en_green_ooze,x,y);
			break;


			//
			// BLACK OOZE
			//

			case 212:
				if (gamestate.difficulty<gd_hard)
					break;
			case 211:
				if (gamestate.difficulty<gd_medium)
					break;
			case 210:
				SpawnOffsetObj(en_black_ooze,x,y);
			break;



		// Flickering Light
		//
			case 79:
				SpawnOffsetObj(en_flickerlight,x,y);
            new_actor->lighting = LAMP_ON_SHADING;
			break;



		// Arc Barrier
		//
			case 174:
         case 175:
         	//
            // 174=off,175=on
            //
				SpawnBarrier(en_post_barrier,x,y,tile-174);
			break;

		// Plasma Post Barrier
		//
			case 138:
         case 139:
         	//
            // 138=off,139=on
            //
				SpawnBarrier(en_arc_barrier,x,y,tile-138);
			break;


		//
      // VPOST Barrier
      //

         //
         // Switchable
         //
			case 563:	// On
			case 562:   // Off
				SpawnBarrier(en_vpost_barrier,x,y,tile-562);
				break;


         //
         // Cycle
         //
      	case 567:
				if (gamestate.difficulty<gd_hard)
					break;
         case 566:
				if (gamestate.difficulty<gd_medium)
					break;
			case 565:
				SpawnBarrier(en_vpost_barrier,x,y,0);
				break;

		//
      // VSPIKE Barrier
      //

      	//
         // Switchable
         //
			case 426:	// On
			case 425:	// Off
				SpawnBarrier(en_vspike_barrier,x,y,tile-425);
				break;


			//
         // Cycle
         //
			case 430:
				if (gamestate.difficulty<gd_hard)
					break;
			case 429:
				if (gamestate.difficulty<gd_medium)
					break;
			case 428:
				SpawnBarrier(en_vspike_barrier,x,y,0);
				break;


		//
		// STEAM GRATE
		//

			case 178:
				SpawnStand(en_steamgrate,x,y,0);
			break;

		//
		// STEAM PIPE
		//

			case 179:
				SpawnStand(en_steampipe,x,y,0);
			break;


		//
		// GOLDFIRE SPAWN SITES
		//
			case 124:
         	if (!loadedgame)
            {
					if (GoldsternInfo.SpawnCnt == GOLDIE_MAX_SPAWNS)
						GAME_ERROR(SETUPGAME_MAX_GOLDIE_SPAWNS);
					GoldsternInfo.flags = GS_FIRSTTIME;
					if (gamestate.mapon==9)
						GoldsternInfo.WaitTime = 60;
					else
						GoldsternInfo.WaitTime = MIN_GOLDIE_FIRST_WAIT + Random(MAX_GOLDIE_FIRST_WAIT-MIN_GOLDIE_FIRST_WAIT);
					GoldieList[GoldsternInfo.SpawnCnt].tilex=x;
					GoldieList[GoldsternInfo.SpawnCnt].tiley=y;
					GoldsternInfo.SpawnCnt++;

					if (gamestate.mapon == GOLD_MORPH_LEVEL)
					{
						AddTotalPoints(actor_points[goldsternobj-rentacopobj]);
						AddTotalEnemy(1);
#ifdef TRACK_ENEMY_COUNT
numEnemy[goldsternobj]++;
#endif
					}
				}
				break;

		//
		// GOLDFIRE SPAWN - IMMEDEATLY
		//

			case 141:
				if (!loadedgame)
				{
					if (GoldsternInfo.GoldSpawned)
						GAME_ERROR(TOO_MANY_FAST_GOLD_SPAWNS);

					if (GoldsternInfo.SpawnCnt == GOLDIE_MAX_SPAWNS)
						GAME_ERROR(SETUPGAME_MAX_GOLDIE_SPAWNS);

					GoldieList[GoldsternInfo.SpawnCnt].tilex=x;
					GoldieList[GoldsternInfo.SpawnCnt].tiley=y;

					GoldsternInfo.LastIndex = GoldsternInfo.SpawnCnt++;
					GoldsternInfo.flags = GS_COORDFOUND;

					SpawnStand(en_goldstern, x, y, 0);
					GoldsternInfo.GoldSpawned = true;
					new_actor = NULL;
				}
				break;

//
// P wall
//
//			case 98:
//				if (!loadedgame)
//				  gamestate.secrettotal++;
//				break;



		//
		// SECURITY LIGHT
		//
			case 160:
				SpawnOffsetObj(en_security_light,x,y);
				break;



		//
		// Rotating Cube
		//

			case 177:
				SpawnOffsetObj(en_rotating_cube,x,y);
				new_actor = NULL;
			break;

		//
		// RENT-A-COP
		//
			case 180:
			case 181:
			case 182:
			case 183:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 144:
			case 145:
			case 146:
			case 147:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 108:
			case 109:
			case 110:
			case 111:
				SpawnStand(en_rentacop,x,y,tile-108);
				break;


			case 184:
			case 185:
			case 186:
			case 187:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 148:
			case 149:
			case 150:
			case 151:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 112:
			case 113:
			case 114:
			case 115:
				SpawnPatrol(en_rentacop,x,y,tile-112);
				break;


//			case 124:
//				SpawnDeadGuard (x,y);
//				break;






		//
		// officer
		//
			case 188:
			case 189:
			case 190:
			case 191:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 152:
			case 153:
			case 154:
			case 155:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 116:
			case 117:
			case 118:
			case 119:
				SpawnStand(en_gen_scientist,x,y,tile-116);
				if (new_actor->flags & FL_INFORMANT)
				{
					AddTotalInformants(1);
					new_actor=NULL;
				}
			break;


			case 192:
			case 193:
			case 194:
			case 195:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 156:
			case 157:
			case 158:
			case 159:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 120:
			case 121:
			case 122:
			case 123:
				SpawnPatrol(en_gen_scientist,x,y,tile-120);
				if (new_actor->flags & FL_INFORMANT)
				{
					AddTotalInformants(1);
					new_actor=NULL;
				}
			break;


		//
		//	PROGUARD
		//
			case 198:
			case 199:
			case 200:
			case 201:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 162:
			case 163:
			case 164:
			case 165:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 126:
			case 127:
			case 128:
			case 129:
				SpawnStand(en_proguard,x,y,tile-126);
				break;


			case 202:
			case 203:
			case 204:
			case 205:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 166:
			case 167:
			case 168:
			case 169:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 130:
			case 131:
			case 132:
			case 133:
				SpawnPatrol(en_proguard,x,y,tile-130);
				break;



			case 312:
				if (gamestate.difficulty<gd_hard)
					break;

			case 311:
				if (gamestate.difficulty<gd_medium)
					break;

			case 310:
				SpawnStand(en_electro_alien,x,y,0);
				new_actor=NULL;
			break;


		//
		//		FLOATING BOMB - Stationary
		//

			case 364:
			case 365:
			case 366:
			case 367:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 346:
			case 347:
			case 348:
			case 349:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 328:
			case 329:
			case 330:
			case 331:
				SpawnStand(en_floatingbomb,x,y,tile-328);
				new_actor->flags |= FL_STATIONARY;
				break;


		//
		//		FLOATING BOMB - Start Stationary
		//

			case 296:
			case 297:
			case 298:
			case 299:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 278:
			case 279:
			case 280:
			case 281:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 260:
			case 261:
			case 262:
			case 263:
				SpawnStand(en_floatingbomb,x,y,tile-260);
				break;


		//
		//		FLOATING BOMB - Start Moving
		//

			case 300:
			case 301:
			case 302:
			case 303:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 282:
			case 283:
			case 284:
			case 285:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 264:
			case 265:
			case 266:
			case 267:
				SpawnPatrol(en_floatingbomb,x,y,tile-264);
				break;


		//
		//		VOLATILE MAT. TRANSPORT - Stationary
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 350:
			case 351:
			case 352:
			case 353:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 332:
			case 333:
			case 334:
			case 335:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 314:
			case 315:
			case 316:
			case 317:
				SpawnStand(en_volatiletransport,x,y,tile-314);
			break;
#else
#if IN_DEVELOPMENT
			case 350:
			case 351:
			case 352:
			case 353:
			case 332:
			case 333:
			case 334:
			case 335:
			case 314:
			case 315:
			case 316:
			case 317:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif



		//
		//		Black Ooze
		//
			case 313:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 295:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 277:
				SpawnOffsetObj(en_black2_ooze,x,y);
			break;



		//
		//		Green Ooze
		//
			case 322:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 304:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 286:
				SpawnOffsetObj(en_green2_ooze,x,y);
			break;



		//
		//		VOLATILE MAT. TRANSPORT - Moving
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 354:
			case 355:
			case 356:
			case 357:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 336:
			case 337:
			case 338:
			case 339:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 318:
			case 319:
			case 320:
			case 321:
				SpawnPatrol(en_volatiletransport,x,y,tile-318);
			break;
#else
#if IN_DEVELOPMENT
			case 354:
			case 355:
			case 356:
			case 357:
			case 336:
			case 337:
			case 338:
			case 339:
			case 318:
			case 319:
			case 320:
			case 321:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif

		//
		// Genetic Guard
		//

			case 143:
				if (gamestate.difficulty<gd_hard)
					break;
			case 142:
				if (gamestate.difficulty<gd_medium)
					break;
			case 214:
				SpawnOffsetObj(en_genetic_guard,x,y);
				break;


		//
		// Cyborg Warrior
		//
			case 603:
				if (gamestate.difficulty<gd_hard)
					break;
			case 585:
				if (gamestate.difficulty<gd_medium)
					break;
			case 250:
				SpawnOffsetObj(en_cyborg_warrior,x,y);
				break;


		//
		// Spider Mutant
		//
			case 601:
				if (gamestate.difficulty<gd_hard)
					break;
			case 583:
				if (gamestate.difficulty<gd_medium)
					break;
			case 232:
				SpawnOffsetObj(en_spider_mutant,x,y);
				break;

		//
		// Acid Dragon
		//
			case 605:
				if (gamestate.difficulty<gd_hard)
					break;
			case 587:
				if (gamestate.difficulty<gd_medium)
					break;
			case 268:
				SpawnOffsetObj(en_acid_dragon,x,y);
				break;

		//
		// Breather beast
		//
			case 602:
				if (gamestate.difficulty<gd_hard)
					break;
			case 584:
				if (gamestate.difficulty<gd_medium)
					break;
			case 233:
				SpawnOffsetObj(en_breather_beast,x,y);
				break;

		//
				// Mech Guardian
//
			case 606:
				if (gamestate.difficulty<gd_hard)
					break;
			case 588:
				if (gamestate.difficulty<gd_medium)
					break;
			case 269:
				SpawnOffsetObj(en_mech_guardian,x,y);
				break;

		//
		// Reptilian Warrior,
		//
			case 604:
				if (gamestate.difficulty<gd_hard)
					break;
			case 586:
				if (gamestate.difficulty<gd_medium)
					break;
			case 251:
				SpawnOffsetObj(en_reptilian_warrior,x,y);
				break;


		//
		// Mutant Human type 1
		//

			case 105:
				if (gamestate.difficulty<gd_hard)
					break;
			case 104:
				if (gamestate.difficulty<gd_medium)
					break;
			case 103:
				SpawnOffsetObj(en_mutant_human1,x,y);
				break;


		//
		// Mutant Human type 2
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 125:
				if (gamestate.difficulty<gd_hard)
					break;
			case 107:
				if (gamestate.difficulty<gd_medium)
					break;
			case 106:
				SpawnOffsetObj(en_mutant_human2,x,y);
			break;
#else
#if IN_DEVELOPMENT
			case 125:
			case 107:
			case 106:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif



		//
		// Small Canister Alien - CONTAINED
		//

			case 136:
				if (gamestate.difficulty<gd_hard)
				{
					SpawnStatic(x,y,74-23);
					break;
				}
			case 135:
				if (gamestate.difficulty<gd_medium)
				{
					SpawnStatic(x,y,74-23);
					break;
				}
			case 134:
				SpawnOffsetObj(en_scan_wait_alien,x,y);
			break;



		//
		// Large Canister Alien - CONTAINED
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 172:
				if (gamestate.difficulty<gd_hard)
				{
					SpawnStatic(x,y,73-23);
					break;
				}
			case 171:
				if (gamestate.difficulty<gd_medium)
				{
					SpawnStatic(x,y,73-23);
					break;
				}
			case 170:
				SpawnOffsetObj(en_lcan_wait_alien,x,y);
			break;
#else
#if IN_DEVELOPMENT
			case 172:
			case 171:
			case 170:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif


		//
		// Gurney Mutant - ASLEEP
		//

#if GAME_VERSION != SHAREWARE_VERSION
			case 161:
				if (gamestate.difficulty<gd_hard)
				{
					SpawnStatic(x,y,72-23);
					break;
				}
			case 173:
				if (gamestate.difficulty<gd_medium)
				{
					SpawnStatic(x,y,72-23);
					break;
				}
			case 137:
				SpawnOffsetObj(en_gurney_wait,x,y);
				break;
#else
#if IN_DEVELOPMENT
			case 161:
			case 173:
			case 137:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif

		//
		// Small Canister Alien - ACTIVE/WALKING
		//

			case 288:
				if (gamestate.difficulty<gd_hard)
					break;
			case 289:
				if (gamestate.difficulty<gd_medium)
					break;
			case 290:
				SpawnOffsetObj(en_scan_alien,x,y);
				break;


		//
		// Large Canister Alien - ACTIVE/WALKING
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 270:
				if (gamestate.difficulty<gd_hard)
					break;
			case 271:
				if (gamestate.difficulty<gd_medium)
					break;
			case 272:
				SpawnOffsetObj(en_lcan_alien,x,y);
			break;
#else
#if IN_DEVELOPMENT
			case 270:
			case 271:
			case 272:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif

		//
		// Gurney Mutant - AWAKE
		//
#if GAME_VERSION != SHAREWARE_VERSION
			case 275:
				if (gamestate.difficulty<gd_hard)
					break;
			case 274:
				if (gamestate.difficulty<gd_medium)
					break;
			case 273:
				SpawnOffsetObj(en_gurney,x,y);
			break;
#else
#if IN_DEVELOPMENT
			case 275:
			case 274:
			case 273:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif


#if GAME_VERSION != SHAREWARE_VERSION
			case 293:
				if (gamestate.difficulty<gd_hard)
					break;
			case 292:
				if (gamestate.difficulty<gd_medium)
					break;
			case 291:
				SpawnStand(en_liquid,x,y,0);
			break;
#else
#if IN_DEVELOPMENT
			case 293:
			case 292:
			case 291:
				INVALID_ACTOR_ERR;
			break;
#endif
#endif


		// P.O.D. Alien Egg
		//
			case 294:
				if (gamestate.difficulty<gd_hard)
					scan_value = 0xff;

			case 276:
				if (gamestate.difficulty<gd_medium)
					scan_value = 0xff;

			case 306:
				SpawnOffsetObj(en_podegg,x,y);
				if (scan_value == 0xff)
					new_actor->obclass = deadobj;
				else
				{
					AddTotalPoints(actor_points[podobj-rentacopobj]);
					AddTotalEnemy(1);
#ifdef TRACK_ENEMY_COUNT
numEnemy[podobj]++;
#endif
				}
				scan_value=0xffff;
			break;


		// Morphing Brown/LBlue Post -> Spider Mutant
		//
			case 610:
				if (gamestate.difficulty<gd_hard)
					scan_value = 0xff;

			case 609:
				if (gamestate.difficulty<gd_medium)
					scan_value = 0xff;

			case 608:
				if (scan_value == 0xff)
					SpawnStatic(x,y,402-315);
				else
				{
					AddTotalPoints(actor_points[en_spider_mutant]);
					AddTotalEnemy(1);
					SpawnOffsetObj(en_morphing_spider_mutant,x,y);
#ifdef TRACK_ENEMY_COUNT
numEnemy[new_actor->obclass]++;
#endif
				}
				scan_value=0xffff;
			break;


		// Morphing Gray/Green Post -> Reptilian Warrior
		//
			case 592:
				if (gamestate.difficulty<gd_hard)
					scan_value = 0xff;

			case 591:
				if (gamestate.difficulty<gd_medium)
					scan_value = 0xff;

			case 590:
				if (scan_value == 0xff)
					SpawnStatic(x,y,403-315);
				else
				{
					AddTotalPoints(actor_points[en_reptilian_warrior]);
					AddTotalEnemy(1);
					SpawnOffsetObj(en_morphing_reptilian_warrior,x,y);
#ifdef TRACK_ENEMY_COUNT
numEnemy[new_actor->obclass]++;
#endif
				}
				scan_value=0xffff;
			break;



		// Morphing Statue -> Blue Boy
		//
			case 628:
				if (gamestate.difficulty<gd_hard)
					scan_value = 0xff;

			case 627:
				if (gamestate.difficulty<gd_medium)
					scan_value = 0xff;

			case 626:
				if (scan_value == 0xff)
					SpawnStatic(x,y,48-23);
				else
				{
					AddTotalPoints(actor_points[en_mutant_human2]);
					AddTotalEnemy(1);
					SpawnOffsetObj(en_morphing_mutanthuman2,x,y);
#ifdef TRACK_ENEMY_COUNT
numEnemy[new_actor->obclass]++;
#endif
				}
				scan_value=0xffff;
			break;



		// P.O.D. Alien
		//
			case 309:
				if (gamestate.difficulty<gd_hard)
					break;

			case 308:
				if (gamestate.difficulty<gd_medium)
					break;

			case 307:
				SpawnOffsetObj(en_pod,x,y);
			break;


		// Electro-Sphere - Vertical Hover
		//
			case 360:
				if (gamestate.difficulty<gd_hard)
					break;

			case 342:
				if (gamestate.difficulty<gd_medium)
					break;

			case 324:
				SpawnOffsetObj(en_vertsphere,x,y);
			break;

		// Electro-Sphere - Horizontal Hover
		//
			case 361:
				if (gamestate.difficulty<gd_hard)
					break;

			case 343:
				if (gamestate.difficulty<gd_medium)
					break;

			case 325:
				SpawnOffsetObj(en_horzsphere,x,y);
			break;

		// Electro-Sphere - Diagonal Hover
		//
			case 362:
				if (gamestate.difficulty<gd_hard)
					break;

			case 344:
				if (gamestate.difficulty<gd_medium)
					break;

			case 326:
				SpawnOffsetObj(en_diagsphere,x,y);
			break;




		//
		// Stationary SWAT Guards
		//

			case 252:
			case 253:
			case 254:
			case 255:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 234:
			case 235:
			case 236:
			case 237:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 216:
			case 217:
			case 218:
			case 219:
				SpawnStand(en_swat,x,y,tile-216);
				break;


		//
		// Roaming SWAT Guards
		//

			case 256:
			case 257:
			case 258:
			case 259:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 238:
			case 239:
			case 240:
			case 241:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 220:
			case 221:
			case 222:
			case 223:
				SpawnPatrol(en_swat,x,y,tile-220);
				break;

		//
		// STATIONARY HANGING TURRETS
		//
			case 368:
			case 369:
			case 370:
			case 371:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 126;
			case 242:
			case 243:
			case 244:
			case 245:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 224:
			case 225:
			case 226:
			case 227:
				SpawnStand(en_hang_terrot,x,y,tile-224);
				new_actor->flags |= FL_STATIONARY;
				break;


		//
		// ROTATING HANGING TURRETS
		//
			case 372:
			case 373:
			case 374:
			case 375:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 126;
			case 246:
			case 247:
			case 248:
			case 249:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 228:
			case 229:
			case 230:
			case 231:
				SpawnStand(en_hang_terrot,x,y,tile-228);
				break;



//--------------------------
// PATH OBJECTS
//--------------------------
			//
			// Swat Guards
			//

			case 540:
			case 541:
			case 542:
			case 543:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 522:
			case 523:
			case 524:
			case 525:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 504:
			case 505:
			case 506:
			case 507:
				SpawnPatrol(en_swat,x,y,tile-504);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

			//
			//		VOLATILE MAT. TRANSPORT
			//

			case 548:
			case 549:
			case 550:
			case 551:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 530:
			case 531:
			case 532:
			case 533:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 512:
			case 513:
			case 514:
			case 515:
				SpawnPatrol(en_volatiletransport,x,y,tile-512);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

			//
			//	FLOATING BOMB -
			//

			case 544:
			case 545:
			case 546:
			case 547:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 526:
			case 527:
			case 528:
			case 529:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 508:
			case 509:
			case 510:
			case 511:
				SpawnPatrol(en_floatingbomb,x,y,tile-508);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

			//
			// PRO GUARD
			//

			case 594:
			case 595:
			case 596:
			case 597:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 576:
			case 577:
			case 578:
			case 579:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 558:
			case 559:
			case 560:
			case 561:
				SpawnPatrol(en_proguard,x,y,tile-558);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;


			//
			// RENT-A-COP
			//

			case 552:
			case 553:
			case 554:
			case 555:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 534:
			case 535:
			case 536:
			case 537:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 516:
			case 517:
			case 518:
			case 519:
				SpawnPatrol(en_rentacop,x,y,tile-516);
				new_actor->flags &= ~FL_RANDOM_TURN;
			break;


//-----------------------
// BOSS ACTORS
//-----------------------

			case 630:	// FINAL BOSS 1
         case 631:	// FINAL BOSS 2
         case 632:	// FINAL BOSS 3
         case 633:	// FINAL BOSS 4
         	SpawnOffsetObj(static_cast<enemy_t>(en_final_boss1+tile-630),x,y);
			break;

			}

		// If "new_actor" is an object that gives points, add those points to level total...
		//
		// "new_actor" is cleared to keep from re-adding points from the previous actor!
		//
			if (new_actor && (new_actor->obclass >= rentacopobj) && (new_actor->obclass<crate1obj))
			{
				classtype obclass=new_actor->obclass;

				switch (obclass)
				{
					case lcan_wait_alienobj:
					case scan_wait_alienobj:
					case gurney_waitobj:
						obclass++;
					break;

					default:
					break;
				}

				AddTotalPoints(actor_points[obclass-rentacopobj]);
				AddTotalEnemy(1);
#ifdef TRACK_ENEMY_COUNT
numEnemy[new_actor->obclass]++;
#endif
				new_actor = NULL;
			}

		// Skip past FA code...
		//
			if (scan_value != 0xffff)
			{
				x++;
				start++;
			}
		}

	if (!loadedgame)
		gamestuff.level[gamestate.mapon].stats.accum_inf =	gamestuff.level[gamestate.mapon].stats.total_inf;

#if IN_DEVELOPMENT
	db_count=0;
#endif
}

//--------------------------------------------------------------------------
// AddTotalPoints()
//--------------------------------------------------------------------------
void AddTotalPoints(Uint16 points)
{
	if (loadedgame)
		return;

	gamestuff.level[gamestate.mapon].stats.total_points += points;
}

//--------------------------------------------------------------------------
// AddTotalInformants()
//--------------------------------------------------------------------------
void AddTotalInformants(char informants)
{
	if (loadedgame)
		return;

	gamestuff.level[gamestate.mapon].stats.total_inf += informants;
}

//--------------------------------------------------------------------------
// AddTotalEnemy()
//--------------------------------------------------------------------------
void AddTotalEnemy(Uint16 enemies)
{
	if (loadedgame)
		return;

	gamestuff.level[gamestate.mapon].stats.total_enemy += enemies;	  
}

//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/

Sint16 an_offset[8] = {1,-1,64,-64,-65,-63,63,65};

void SetupGameLevel (void)
{
	extern boolean ForceLoadDefault;
   boolean switchon = false;
	sci_mCacheInfo *ci = InfHintList.smInfo;
	Sint16	x,y;
	Uint16	*map,tile,icon;
	keytype lock;
	Uint16	*map1,*map2;
	Sint16 count;

	if (!loadedgame)
	{
		gamestate.flags |= GS_CLIP_WALLS;
		InitGoldsternInfo();
	}

	if (demoplayback || demorecord)
		US_InitRndT (false);
	else
		US_InitRndT (true);

//
// load the level
//
	CA_CacheMap (gamestate.mapon+MAPS_PER_EPISODE*gamestate.episode);
	mapon-=gamestate.episode*MAPS_PER_EPISODE;

	mapwidth = mapheaderseg[mapon]->width;
	mapheight = mapheaderseg[mapon]->height;

	if (mapwidth != 64 || mapheight != 64)
		GAME_ERROR(SETUPGAME_BAD_MAP_SIZE);

	LoadLocationText(gamestate.mapon+MAPS_PER_EPISODE*gamestate.episode);

//
// copy the wall data to a data segment array
//
	memset (TravelTable,0,sizeof(TravelTable));
	memset (gamestate.barrier_table,0xff,sizeof(gamestate.barrier_table));
	memset (gamestate.old_barrier_table,0xff,sizeof(gamestate.old_barrier_table));
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	memset (wallheight,0,sizeof(wallheight));

	map = mapsegs[0];
	map2 = mapsegs[1];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			icon = *map2++;
			tile = *map++;

			if (tile<AREATILE)
			{
			// solid wall
				tilemap[x][y] = tile;

//				if (tile != AMBUSHTILE)
//					(unsigned)actorat[x][y] = tile;

				switch (tile)
				{
					case AMBUSHTILE:
					case RKEY_TILE:
					case YKEY_TILE:
					case BKEY_TILE:
					case BFG_TILE:
					case ION_TILE:
					case DETONATOR_TILE:
					case CLOAK_TILE:
					case LINC_TILE:
					case CLOAK_AMBUSH_TILE:
					break;

					default:
// FIXME
#if 0
						(Uint16)actorat[x][y] = tile;
#endif // 0

                        actorat[x][y] = (objtype*)tile;
					break;
				}

			}

			if (tile<64 || icon==PUSHABLETILE)
				TravelTable[x][y] |= TT_TRAVELED;
		}

//
// spawn doors
//
	InitActorList ();			// start spawning things with a clean slate
	InitDoorList ();

	InitMsgCache((mCacheList *)&ConHintList,sizeof(ConHintList),sizeof(ConHintList.cmInfo[0]));
	InitMsgCache((mCacheList *)&InfHintList,sizeof(InfHintList),sizeof(InfHintList.smInfo[0]));
	InitMsgCache((mCacheList *)&NiceSciList,sizeof(NiceSciList),sizeof(InfHintList.smInfo[0]));
	InitMsgCache((mCacheList *)&MeanSciList,sizeof(MeanSciList),sizeof(InfHintList.smInfo[0]));

	InitStaticList ();

	map = mapsegs[0];
	map1 = mapsegs[1];

    NumEAWalls = 0;
    alerted = 0;
	LastInfoAttacker = nothing;

	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			lock = static_cast<keytype>(*map1);

			if (*map==30)
			{
				gamestate.wintilex=x+1;
				gamestate.wintiley=y;
			}

			if (tile >= 88 && tile <= 105)
			{
				//
				// KEYS
				//

				switch (lock)
				{
					case 55:
					case 56:
						lock = static_cast<keytype>(kt_red+lock-55);
						*map1 = 0;
						break;

					case 58:
						lock = kt_blue;
						*map1 = 0;
						break;

					default:
						lock = kt_none;
				}

				//
				// DOOR
				//

				switch (tile)
				{

					case 88:
					case 89:
						SpawnDoor (x,y,!(tile%2),lock,dr_bio);
						break;

					case 90:
					case 91:
						SpawnDoor (x,y,!(tile%2),lock,dr_normal);
						break;

					case 92:
					case 93:
						SpawnDoor (x,y,!(tile%2),lock,dr_prison);
						break;

					case 94:
					case 95:
						SpawnDoor (x,y,!(tile%2),lock,dr_elevator);
						break;

					case 96:
					case 97:
						SpawnDoor (x,y,!(tile%2),lock,dr_high_security);
						break;

					case 98:							// oneway left  - Vert
					case 99:							// oneway up    - Horz
					case 100:						// oneway right - Vert
					case 101:						// oneway down  - Horz
						SpawnDoor (x,y,!(tile%2),lock,static_cast<door_t>(dr_oneway_left+(tile-98)));
						break;

				  case 102:
				  case 103:
						SpawnDoor (x,y,!(tile%2),lock,dr_office);
						break;

				  case 104:
				  case 105:
						SpawnDoor (x,y,!(tile%2),lock,dr_space);
						break;


				}
			}
			else
			switch (tile)
			{
				case SODATILE:
					if (!loadedgame)
					{
						SpawnConcession(x,y,lock,CT_BEVS);
						*map1 = 0;
					}
				break;



				case FOODTILE:
					if (!loadedgame)
					{
						SpawnConcession(x,y,lock,CT_FOOD);
						*map1 = 0;
					}
				break;

				case EATILE:
					eaList[NumEAWalls].tilex=x;
					eaList[NumEAWalls].tiley=y;
					eaList[NumEAWalls].aliens_out=0;
					if ((lock & 0xff00) == 0xfa00)
						eaList[NumEAWalls].delay=60*(lock&0xff);
					else
						eaList[NumEAWalls].delay=60*8+Random(60*22);
					if (NumEAWalls++ == MAXEAWALLS)
						GAME_ERROR(SETUPGAME_MAX_EA_WALLS);
				break;

				case ON_SWITCH:
            	switchon = true;
				case OFF_SWITCH:
					*(map1) = 0xf800 | UpdateBarrierTable((*(map1)>>8),*(map1)&0xff,switchon);

               // Init for next time.

	            switchon = false;
				break;
			}

			map1++;
		}


//
// spawn actors
//

	ScanInfoPlane ();
	ConnectBarriers();

// Init informant stuff
//
	count = InfHintList.NumMsgs;
	LastInfArea=0xff;
	FirstGenInfMsg=0;
	for (;(ci->areanumber != 0xff) && (count--);ci++)
		FirstGenInfMsg++;
	TotalGenInfMsgs=InfHintList.NumMsgs-FirstGenInfMsg;


	//
	// Take out the special tiles that were not used...
	//

	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			switch (tile)
			{
         	case AMBUSHTILE:
				case RKEY_TILE:
				case YKEY_TILE:
				case BKEY_TILE:
				case BFG_TILE:
				case ION_TILE:
				case DETONATOR_TILE:
				case CLOAK_TILE:
				case LINC_TILE:
				case CLOAK_AMBUSH_TILE:
					tilemap[x][y] = 0;
// FIXME
#if 0
					if ( (Uint16)actorat[x][y] == AMBUSHTILE)
#endif // 0

                    if (actorat[x][y] == (objtype*)AMBUSHTILE)
						actorat[x][y] = NULL;
					*(map-1) = GetAreaNumber(x,y);
            break;
			}
		}

//
// have the caching manager load and purge stuff to make sure all marks
// are in memory
//
	CA_LoadAllSounds ();

//
// Check and make sure a detonator is in a 'locked' level.
//

   if (gamestate.mapon < 20 && (!detonators_spawned) && gamestuff.level[gamestate.mapon+1].locked)
   	GAME_ERROR(NO_DETONATORS_IN_LEVEL);
}


//------------------------------------------------------------------------
// LoadLocationText()
//------------------------------------------------------------------------
void LoadLocationText(Sint16 textNum)
{
	char *temp;

	LoadMsg(LocationText,LEVEL_DESCS,textNum+1,MAX_LOCATION_DESC_LEN);
	temp = strstr(LocationText,"^XX");
	if (temp)
		*temp = 0;
}


//==========================================================================




/*
===================
=
= DrawPlayBorder
=
===================
*/
void DrawPlayBorder (void)
{
	Sint16	xl,yl;

	if (viewwidth == 320)
	{
		VWB_Bar (0,TOP_STRIP_HEIGHT,320,200-STATUSLINES-TOP_STRIP_HEIGHT,0);    // JTR - Changed
//		VW_Bar (0,TOP_STRIP_HEIGHT,320,200-STATUSLINES-TOP_STRIP_HEIGHT,0);
		return;
	}

	xl = 160-viewwidth/2;
	yl = (200-STATUSLINES+TOP_STRIP_HEIGHT-viewheight)/2;

	BevelBox(0,TOP_STRIP_HEIGHT,320,200-STATUSLINES-TOP_STRIP_HEIGHT,BORDER_HI_COLOR,BORDER_MED_COLOR,BORDER_LO_COLOR);
	BevelBox(xl-1,yl-1,viewwidth+2,viewheight+2,BORDER_LO_COLOR,0,BORDER_HI_COLOR);
}


//--------------------------------------------------------------------------
// BMAmsg() - These messages are displayed by the Text Presenter!
//--------------------------------------------------------------------------
void BMAmsg(const char *msg)
{
	#define 	BMAx1	0						// outer bevel
	#define	BMAy1	152
	#define	BMAw1	320
	#define	BMAh1	48

	#define 	BMAx2	(BMAx1+7)			// inner bevel
	#define	BMAy2	(BMAy1+4)
	#define	BMAw2	(BMAw1-14)
	#define	BMAh2	(BMAh1-8)

	BevelBox(BMAx1,BMAy1,BMAw1,BMAh1,BORDER_HI_COLOR,BORDER_MED_COLOR,BORDER_LO_COLOR);
	BevelBox(BMAx2,BMAy2,BMAw2,BMAh2,BORDER_LO_COLOR,BORDER_MED_COLOR,BORDER_HI_COLOR);

	if (msg)
	{
		PresenterInfo pi;
		fontstruct *font=(fontstruct *)grsegs[STARTFONT+fontnumber];
		char numlines=1;
		const char *p=msg;
		Sint16 cheight;

		memset(&pi,0,sizeof(pi));
		pi.flags = TPF_CACHE_NO_GFX;
		pi.script[0]=p;
		while (*p)
			if (*p++ == TP_RETURN_CHAR)
				numlines++;
		cheight=font->height*numlines+1+(TP_MARGIN*2);

		pi.xl=BMAx2+1;
		pi.yl=BMAy2+(BMAh2-cheight)/2;
		pi.xh=pi.xl+BMAw2-3;
		pi.yh=pi.yl+cheight-1;
		pi.bgcolor = BORDER_MED_COLOR;
		pi.ltcolor = BORDER_HI_COLOR;
		fontcolor = BORDER_TEXT_COLOR;
		pi.shcolor = pi.dkcolor = BORDER_LO_COLOR;
		pi.fontnumber=fontnumber;
		TP_InitScript(&pi);
		TP_Presenter(&pi);
	}
}

//----------------------------------------------------------------------
// CacheBMAmsg() - Caches in a Message Number and displays it using
//						 BMAmsg()
//----------------------------------------------------------------------
void CacheBMAmsg(Uint16 MsgNum)
{
	char *string, *pos;

	CA_CacheGrChunk(MsgNum);
   string = (char*)grsegs[MsgNum];

   pos = strstr(string,"^XX");
   *(pos+3) = 0;

   BMAmsg(string);

	UNCACHEGRCHUNK(MsgNum);

}

//--------------------------------------------------------------------------
// BevelBox()
//--------------------------------------------------------------------------
void BevelBox(Sint16 xl, Sint16 yl, Sint16 w, Sint16 h, Uint8 hi, Uint8 med, Uint8 lo)
{
	Sint16 xh=xl+w-1,yh=yl+h-1;
	Uint8 hc;

	VWB_Bar (xl,yl,w,h,med);				// inside

#if 0
	hc = (hi-lo+1)/2;
	hc = ABS(hc);
	if (hi > lo)
		hc = hi-hc;
	else
		hc = hi+hc;
#else
	hc = med+1;
#endif

	VWB_Hlin (xl,xh,yl,hi);					// top
	VWB_Hlin (xl,xh,yh,lo);					// bottom
	VWB_Vlin (yl,yh,xl,hi);					// left
	VWB_Vlin (yl,yh,xh,lo);					// right
	VWB_Plot (xl,yh,hc);						// lower-left
	VWB_Plot (xh,yl,hc);						// upper-right
}

//--------------------------------------------------------------------------
// ShadowPrintLocationText()
//--------------------------------------------------------------------------
void ShadowPrintLocationText(sp_type type)
{
	const char *DemoMsg="-- DEMO --";
    const char *DebugText= "-- DEBUG MODE ENABLED --";
	const char *s,*ls_text[3]={"-- LOADING --","-- SAVING --","-- CHANGE VIEW SIZE --"};
    char str[8];
	Uint16 w,h;

// Used for all fields...
//
	py = 5;
	fontcolor = 0xaf;

// Print LOCATION info...
//
	switch (type)
	{
		case sp_normal:
		// Print LEVEL info...
		//
			px = 13;
			if (gamestate.mapon>19)
				ShPrint(" SECRET ",0,false);
			else
			{
				ShPrint(" AREA: ",0,false);
				if (!type)
					ShPrint(itoa(gamestate.mapon+1,str,10),0,false);
			}

		// Print LIVES info...
		//
			px = 267;
			ShPrint("LIVES: ",0,false);
			if (!type)
				ShPrint(itoa(gamestate.lives,str,10),0,false);

		// Print location text
      //

			if (demoplayback || demorecord)
         	s = DemoMsg;
         else
#if IN_DEVELOPMENT
         	if (DebugOk)
#else
         	if (DebugOk || (gamestate.flags & (GS_QUICKRUN|GS_STARTLEVEL|GS_TICS_FOR_SCORE|GS_MUSIC_TEST|GS_SHOW_OVERHEAD)))
#endif
					s=DebugText;
            else
					s=LocationText;
		break;

		case sp_changeview:
		case sp_loading:
		case sp_saving:
			s=ls_text[type-sp_loading];
		break;

		default:
		break;
	}

	VW_MeasurePropString(s,&w,&h);
	px = 160-w/2;
	ShPrint(s,0,false);
}

//--------------------------------------------------------------------------
// DrawTopInfo()
//--------------------------------------------------------------------------
void DrawTopInfo(sp_type type)
{
	char old=fontnumber;

	LatchDrawPic(0,0,TOP_STATUSBARPIC);
	fontnumber=2;
	ShadowPrintLocationText(type);
	fontnumber=old;
}

/*
===================
=
= DrawPlayScreen
=
===================
*/
void DrawPlayScreen (boolean InitInfoMsg)
{
	Sint16	i;
	Uint16 temp;

	if (loadedgame)
		return;

	if (playstate != ex_transported)
		VW_FadeOut ();

	temp = bufferofs;
	WindowW = 253;
  	WindowH = 8;
   fontnumber = 2;

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder();

		JLatchDrawPic(0,200-STATUSLINES,STATUSBARPIC);
		LatchDrawPic(0,0,TOP_STATUSBARPIC);

		ShadowPrintLocationText(sp_normal);
	}

	bufferofs = temp;

	DrawHealth();
	DrawKeys();
	DrawWeapon();
	DrawScore();

	InitInfoArea();

	if (InitInfoMsg)
		DISPLAY_MSG("R.E.B.A.\rAGENT: BLAKE STONE\rALL SYSTEMS READY.", MP_max_val, MT_NOTHING);
	else
		DisplayNoMoMsgs();

	ForceUpdateStatusBar();
}

//---------------------------------------------------------------------------
// void DrawWarpIn(void)
//---------------------------------------------------------------------------
void DrawWarpIn(void)
{
	Sint16	i;
	Uint16 temp;

	temp = bufferofs;
	InitInfoArea();
	DisplayInfoMsg("\r\r    TRANSPORTING...",MP_POWERUP,2*60,MT_GENERAL);

	DrawHealth();
	DrawKeys();
	DrawWeapon();
	DrawScore();
	WindowW = 253;
  	WindowH = 8;
   fontnumber = 2;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		VW_Bar ((320-viewwidth)/2,(200-STATUSLINES-viewheight+TOP_STRIP_HEIGHT)/2,viewwidth,viewheight,BLACK);
		JLatchDrawPic(0,200-STATUSLINES,STATUSBARPIC);
		LatchDrawPic(0,0,TOP_STATUSBARPIC);

		ShadowPrintLocationText(sp_normal);
		UpdateStatusBar();
	}

	bufferofs = temp;

	SD_PlaySound(WARPINSND);

	fizzlein = true;

	ThreeDRefresh();

}

//---------------------------------------------------------------------------
// Warped
//---------------------------------------------------------------------------
void Warped(void)
{
	Sint16		iangle;

	DisplayInfoMsg("\r\r\r   TRANSPORTING OUT",MP_POWERUP,7*60,MT_GENERAL);
	gamestate.old_weapons[3] = gamestate.weapon;
	gamestate.weapon = -1;			// take away weapon

	ThreeDRefresh();

	if (screenfaded)
		VW_FadeIn();

	iangle = (((player->dir+4) % 8)>>1) * 90;

	RotateView(iangle,2);

	gamestate.weapon = gamestate.old_weapons[3];
	gamestate.attackframe = gamestate.attackcount =	gamestate.weaponframe = 0;

	bufferofs += screenofs;

	VW_Bar (0,0,viewwidth,viewheight,BLACK);

	IN_ClearKeysDown ();
	SD_PlaySound (WARPINSND);

	FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);

	bufferofs-= screenofs;

	IN_UserInput(100);
	SD_WaitSoundDone ();
}

//==========================================================================

#if GAME_VERSION == SHAREWARE_VERSION
char	demoname[13] = "DEMO?S.";
#else
char	demoname[13] = "DEMO?.";
#endif

#ifdef DEMOS_EXTERN

/*
==================
=
= StartDemoRecord
=
==================
*/

#define MAXDEMOSIZE	16384

void StartDemoRecord (Sint16 levelnumber)
{
	MM_GetPtr (&demobuffer,MAXDEMOSIZE);
	MM_SetLock (&demobuffer,true);
	demoptr = (char *)demobuffer;
	lastdemoptr = demoptr+MAXDEMOSIZE;

	*demoptr = levelnumber;
	demoptr += 4;				// leave space for length
	demorecord = true;
}


/*
==================
=
= FinishDemoRecord
=
==================
*/

void FinishDemoRecord (void)
{
	char str[3];
	Sint32	length,level;

	demorecord = false;

	length = demoptr - (char *)demobuffer;

	demoptr = ((char *)demobuffer)+1;
	*(Uint16 *)demoptr = length;

	VW_FadeIn();

	CenterWindow(24,3);
	PrintY+=6;
	fontnumber = 4;
	US_Print(" Demo number (0-9):");
	VW_UpdateScreen();

	if (US_LineInput (px,py,str,NULL,true,2,0))
	{
		level = atoi (str);
		if (level>=0 && level<=9)
		{
			demoname[4] = '0'+level;
			IO_WriteFile (demoname,(void *)demobuffer,length);
		}
	}

	VW_FadeOut();

	MM_FreePtr (&demobuffer);
}

//==========================================================================

/*
==================
=
= RecordDemo
=
= Fades the screen out, then starts a demo.  Exits with the screen faded
=
==================
*/


#if 0		// JAM's Modified Version - HELP! - Comment out

void RecordDemo (void)
{
	CenterWindow(26,3);
	PrintY+=6;
	fontnumber=4;

	SETFONTCOLOR(0,15);
	VW_FadeOut ();

	NewGame (gd_hard,gamestate.episode);
	StartDemoRecord (gamestate.mapon);

	DrawPlayScreen (true);
	VW_FadeIn ();

	startgame = false;
	demorecord = true;

	LoadLevel(gamestate.mapon);
	StartMusic(false);
	PM_CheckMainMem ();
	fizzlein = true;

	PlayLoop ();

	demoplayback = false;

	StopMusic ();
	VW_FadeOut ();
	ClearMemory ();

	FinishDemoRecord ();
}

#endif

void RecordDemo (void)
{
	char str[3];
	Sint16 level,esc;

	CenterWindow(26,3);
	PrintY+=6;
	fontnumber=4;
	US_Print("  Demo which level(0-23):");
	VW_UpdateScreen();
	VW_FadeIn ();
	esc = !US_LineInput (px,py,str,NULL,true,2,0);
	if (esc)
		return;

	level = atoi (str);
//	level--;
	if (level > 23)
		return;

	SETFONTCOLOR(0,15);
	VW_FadeOut ();

	NewGame (gd_easy,level/MAPS_PER_EPISODE);
	gamestate.mapon = level%MAPS_PER_EPISODE;
	StartDemoRecord (level);

	DrawPlayScreen (true);
	DrawTopInfo(sp_loading);
	DisplayPrepingMsg(prep_msg);
	LS_current = 1;
	LS_total = 20;

	VW_FadeIn ();

	startgame = false;
	demorecord = true;

	LoadLevel(gamestate.mapon);

	VW_FadeOut();
	DrawPlayScreen(true);
	StartMusic(false);
	PM_CheckMainMem ();
	fizzlein = true;

	PlayLoop ();

	demoplayback = false;

	StopMusic ();
	VW_FadeOut ();
	ClearMemory ();

	FinishDemoRecord ();
}

#endif

//==========================================================================

/*
==================
=
= PlayDemo
=
= Fades the screen out, then starts a demo.  Exits with the screen faded
=
==================
*/

void PlayDemo (Sint16 demonumber)
{
//   static int numloops=0;
	Sint16 length,off;

#ifndef DEMOS_EXTERN
// debug: load chunk
#if GAME_VERSION == SHAREWARE_VERSION
	Sint16 dems[4]={T_DEMO0,T_DEMO1,T_DEMO2,T_DEMO3};
#else
	Sint16 dems[6]={T_DEMO0,T_DEMO1,T_DEMO2,T_DEMO3,T_DEMO4,T_DEMO5};
#endif

	CA_CacheGrChunk(dems[demonumber]);
	demoptr = static_cast<char*>(grsegs[dems[demonumber]]);
#else
	demoname[4] = '0'+demonumber;
	IO_LoadFile (demoname,&demobuffer);
	MM_SetLock (&demobuffer,true);
	demoptr = (char *)demobuffer;
#endif

	NewGame (1,0);
	gamestate.mapon = *demoptr++;
	gamestate.difficulty = gd_easy;
	length = *((Uint16*)demoptr);
    demoptr += 2;
	demoptr++;
	lastdemoptr = demoptr-4+length;

	VW_FadeOut ();

	SETFONTCOLOR(0,15);
	DrawPlayScreen (true);
	DrawTopInfo(sp_loading);
	DisplayPrepingMsg(prep_msg);
	LS_current = 1;
	LS_total = 20;
	VW_FadeIn ();

	startgame = false;
	demoplayback = true;

	StartMusic(false);
	LoadLevel(gamestate.mapon);

	VW_FadeOut();
	DrawPlayScreen(true);
	SetPlaneViewSize();
	fizzlein = true;

#ifndef DEMOS_EXTERN
    // FIXME
	//off = FP_OFF(demoptr);
    off = (Sint16)demoptr;

	demoptr = static_cast<char*>(grsegs[dems[demonumber]]);
	demoptr += off;
#endif

	PlayLoop ();

	if (gamestate.health<=0)
		Died();

#ifndef DEMOS_EXTERN
	UNCACHEGRCHUNK(dems[demonumber]);
#else
	MM_FreePtr (&demobuffer);
#endif

	demoplayback = false;
	LS_current = LS_total = -1;

	StopMusic ();
	VW_FadeOut ();
	ClearMemory ();

	playstate = ex_title;
}

//==========================================================================

/*
==================
=
= Died
=
==================
*/

#define DEATHROTATE 2

void Died (void)
{
	Sint16		iangle;

	gamestate.weapon = -1;			// take away weapon
	SD_PlaySound (PLAYERDEATHSND);

	iangle = CalcAngle(player,killerobj);

	RotateView(iangle,DEATHROTATE);

//
// fade to red
//
	FinishPaletteShifts ();

	bufferofs += screenofs;
	VW_Bar (0,0,viewwidth,viewheight,0x17);
	IN_ClearKeysDown ();
	FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);
	bufferofs -= screenofs;
	if (demoplayback)
		return;
	IN_UserInput(100);

	SD_WaitSoundDone();
	StopMusic();

	gamestate.lives--;

	if (gamestate.lives > -1)
	{
		gamestate.health = 100;
		gamestate.weapons = 1<<wp_autocharge;			// |1<<wp_plasma_detonators;
		gamestate.weapon = gamestate.chosenweapon = wp_autocharge;

		gamestate.ammo = STARTAMMO;
		gamestate.attackframe = gamestate.attackcount =
		gamestate.weaponframe = 0;

		gamestate.flags |= (GS_CLIP_WALLS|GS_ATTACK_INFOAREA);

		DrawHealth();
		DrawKeys();
		DrawWeapon();
		DrawScore();
		DrawKeys();
		ForceUpdateStatusBar();
	}
}

//--------------------------------------------------------------------------
// LoseScreen()   - Displays the Goldstern/DamagedReba message...
//--------------------------------------------------------------------------
void LoseScreen(void)
{
	PresenterInfo pi;

   VW_FadeOut();

	memset(&pi,0,sizeof(pi));
	pi.flags=TPF_USE_CURRENT|TPF_SHOW_CURSOR|TPF_SCROLL_REGION|TPF_CONTINUE|TPF_TERM_SOUND|TPF_ABORTABLE;
	pi.xl=14;
	pi.yl=141;
	pi.xh=14+293;
	pi.yh=141+32;
	pi.ltcolor=15;
	pi.bgcolor=0;
	pi.dkcolor=1;
	pi.shcolor=1;
	pi.fontnumber=2;
	pi.cur_x = -1;
	pi.print_delay=2;

	ClearMemory();
	StopMusic();

	CA_CacheScreen(LOSEPIC);
	VW_UpdateScreen();

#ifdef ID_CACHE_LOSE
	TP_LoadScript(NULL,&pi,LOSETEXT);
#else
	TP_LoadScript("LOSE.TXT",&pi,0);
#endif

	// Now Presenting... The Loser Prize.. I nice message directly from Dr.
	// ==============    Goldstern himself!  Oooo Ohhhhh <clap> <clap> ...
	//

	VW_FadeIn();
	TP_Presenter(&pi);
	VW_FadeOut();

#ifdef ID_CACHE_LOSE
	TP_FreeScript(&pi,LOSETEXT);
#else
	TP_FreeScript(&pi,0);
#endif

	screenfaded = true;

	IN_ClearKeysDown();
}

//--------------------------------------------------------------------------
// RotateView()
//
// PARAMETERS:
//					DestAngle - Destination angle to rotate player->angle to.
//					RotSpeed  - Rotation Speed
//--------------------------------------------------------------------------
void RotateView(Sint16 DestAngle,Uint8 RotSpeed)
{
	Sint16	curangle,clockwise,counter,change;
	objtype *obj;							  
	boolean old_godmode=godmode;		  

	if (player->angle > DestAngle)
	{
		counter = player->angle - DestAngle;
		clockwise = ANGLES-player->angle + DestAngle;
	}
	else
	{
		clockwise = DestAngle - player->angle;
		counter = player->angle + ANGLES-DestAngle;
	}

	godmode = true;		  				 
	curangle = player->angle;

	controly = 0;
	if (clockwise<counter)
	{
	//
	// rotate clockwise
	//
		if (curangle>DestAngle)
			curangle -= ANGLES;
		controlx = -1;				
		do
		{
			change = tics*RotSpeed;
			if (curangle + change > DestAngle)
				change = DestAngle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;

			for (obj = player->next;obj;obj = obj->next)
				DoActor (obj);
			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != DestAngle);
	}
	else
	{
	//
	// rotate counterclockwise
	//
		if (curangle<DestAngle)
			curangle += ANGLES;
		controlx = 1;
		do
		{
			change = -tics*RotSpeed;
			if (curangle + change < DestAngle)
				change = DestAngle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle < 0)
				player->angle += ANGLES;

			for (obj = player->next;obj;obj = obj->next)
				DoActor (obj);
			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != DestAngle);
	}

	controlx = 0;
	player->dir = static_cast<dirtype>(((player->angle + 22) % 360)/45);
	godmode = old_godmode;

}

//==========================================================================


/*
===================
=
= GameLoop
=
===================
*/

void GameLoop (void)
{
	extern boolean sqActive;

	char Score[13];
	boolean	died;
#ifdef MYPROFILE
	clock_t start,end;
#endif

restartgame:

	ClearMemory();
	SETFONTCOLOR(0,15);
	DrawPlayScreen (true);

	died = false;
	do
	{
		extern Sint16 pickquick;

		ingame = true;

		if (died && pickquick)
		{
				char string[]="  Auto Quick Load?  ";

				WindowX=WindowY=0;
				WindowW=320;
				WindowH=152;

				if (Confirm(string))
				{
					playstate=ex_stillplaying;
					DrawPlayBorder();
					VW_UpdateScreen();
					US_ControlPanel(sc_F9);
				}

				DrawPlayBorder();
				VW_UpdateScreen();
		}

		if (!sqActive)
			StartMusic(false);

		if (!(loadedgame || LevelInPlaytemp(gamestate.mapon)))
		{
			gamestate.tic_score = gamestate.score = gamestate.oldscore;
			memcpy(gamestate.numkeys,gamestate.old_numkeys,sizeof(gamestate.numkeys));
			memcpy(gamestate.barrier_table,gamestate.old_barrier_table,sizeof(gamestate.barrier_table));
			gamestate.rpower = gamestate.old_rpower;	
			gamestate.tokens = gamestate.old_tokens;
			gamestate.weapons = gamestate.old_weapons[0];
			gamestate.weapon = gamestate.old_weapons[1];
			gamestate.chosenweapon = gamestate.old_weapons[2];
			gamestate.ammo = gamestate.old_ammo;
			gamestate.plasma_detonators = gamestate.old_plasma_detonators;
			gamestate.boss_key_dropped=gamestate.old_boss_key_dropped;
			memcpy(&gamestuff.level[0],gamestuff.old_levelinfo,sizeof(gamestuff.old_levelinfo));
			DrawKeys();
			DrawScore();
		}

#ifdef MYPROFILE
start = clock();
while (start == clock());
start++;
#endif

		startgame = false;
		if (!loadedgame)
		{
//			ClearMemory();
			if (LS_current==-1)
			{
//				ClearMemory();					
				DrawTopInfo(sp_loading);
				DisplayPrepingMsg(prep_msg);
				LS_current = 1;
				LS_total = 20;
			}
//			ClearMemory();
			LoadLevel(gamestate.mapon);
		}

		LS_current = LS_total = -1;

		SetPlaneViewSize();
		if (loadedgame)
			loadedgame=false;

		if (died)
		{
			WindowY=188;
			PreloadUpdate(1,1);
			died = false;
			DrawPlayScreen(true);
		}
		else
		{
			PreloadGraphics();
			if (playstate == ex_transported)
				DrawWarpIn();
			else
				DrawPlayScreen(false);
		}

		if (!sqActive)
			StartMusic(false);

		PlayLoop ();
		LS_current = LS_total = -1;
		died = false;

		StopMusic ();
		ingame = false;

#ifdef MYPROFILE
end = clock();
strcpy (str,"300 frames in 1/18ths:"); // defined in 3d_main.c
itoa(end-start,str2,10);					// defined in 3d_main.c
strcat (str,str2);							// defined in 3d_main.c
		Quit (str);								// defined in 3d_main.c
#endif

#ifdef DEMOS_EXTERN
		if (demorecord && playstate != ex_warped)
			FinishDemoRecord ();
#endif

		if (startgame || loadedgame)
			goto restartgame;

		switch (playstate)
		{

		case ex_transported:				// Same as ex_completed
			Warped();

		case ex_completed:
		case ex_secretlevel:
		case ex_warped:
#if 0
			gamestate.keys = 0;
			DrawKeys ();
			VW_FadeOut ();
#endif
//         StopMusic();			// JTR
			ClearMemory ();
			gamestate.mapon++;
			ClearNClose();
			DrawTopInfo(sp_loading);
			DisplayPrepingMsg(prep_msg);
			WindowY = 181;
			LS_current=1;
			LS_total=38;
			StartMusic(false);
			SaveLevel(gamestate.lastmapon);

			gamestate.old_rpower = gamestate.rpower;
			gamestate.oldscore = gamestate.score;
			memcpy(gamestate.old_numkeys,gamestate.numkeys,sizeof(gamestate.old_numkeys));
			gamestate.old_tokens = gamestate.tokens;
			memcpy(gamestate.old_barrier_table,gamestate.barrier_table,sizeof(gamestate.old_barrier_table));
			gamestate.old_weapons[0] = gamestate.weapons;
			gamestate.old_weapons[1] = gamestate.weapon;
			gamestate.old_weapons[2] = gamestate.chosenweapon;
			gamestate.old_ammo = gamestate.ammo;
			gamestate.old_boss_key_dropped = gamestate.boss_key_dropped;
			memcpy(gamestuff.old_levelinfo,&gamestuff.level[0],sizeof(gamestuff.old_levelinfo));

#if 0
			if (gamestate.mapon == 9)
				gamestate.mapon = ElevatorBackTo[gamestate.episode];	// back from secret
			else if (playstate == ex_secretlevel)
				gamestate.mapon = 9;
			else
#endif
			break;

		case ex_died:
			if (InstantQuit)
				InstantQuit = false;
			else
			{
				Died ();

				died = true;			// don't "get psyched!"

				if (gamestate.lives > -1)
				{
					ClearMemory();
					break;				// more lives left
				}

				LoseScreen();
			}


		case ex_victorious:
			MainMenu[MM_SAVE_MISSION].active=AT_DISABLED;
			 MainMenu[MM_VIEW_SCORES].routine=&CP_ViewScores;
			strcpy(MainMenu[MM_VIEW_SCORES].string,"HIGH SCORES");

			if (playstate==ex_victorious)
			{
				ThreeDRefresh();
				ThreeDRefresh();
			}

			ClearMemory ();

			if (playstate==ex_victorious)
			{
				char loop;

				fontnumber=1;
				CA_CacheGrChunk(STARTFONT+1);
				memset (update,0,sizeof(update));
				CacheBMAmsg(YOUWIN_TEXT);
				for (loop=0; loop<2; loop++)
				{
					VW_ScreenToScreen(displayofs,bufferofs,320,200);
					NextBuffer();
				}
				UNCACHEGRCHUNK(STARTFONT+1);
				SD_PlaySound(BONUS1SND);
				SD_WaitSoundDone();
				IN_UserInput(5*60);
				ClearMemory();
			}

			VW_FadeOut ();
//       StopMusic();			 // JTR

			sprintf(Score,"%ld",gamestate.score);
			piStringTable[0]=Score;

			if (playstate==ex_victorious)
			{
				CA_CacheGrChunk(ENDINGPALETTE);
//				VL_SetPalette (0,256,grsegs[ENDINGPALETTE]);

				DoMovie(mv_final,grsegs[ENDINGPALETTE]);

				UNCACHEGRCHUNK(ENDINGPALETTE);
				NewViewSize(viewsize);		// Recreates & Allocs the ScaleDirectory
				Breifing(BT_WIN,gamestate.episode);
			}

			CheckHighScore (gamestate.score,gamestate.mapon+1);

			return;

		default:
			ClearMemory ();
			break;
		}

	} while (1);

}

