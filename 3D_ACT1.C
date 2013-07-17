// 3D_ACT1.C

#include "3D_DEF.H"
#pragma hdrstop


//===========================================================================
//
//							PROTOTYPES
//
//===========================================================================

void OpenDoor (int door);
void CloseDoor (int door);
void PlaceItemNearTile(int itemtype, int tilex, int tiley);


//===========================================================================
//
//							LOCALS
//
//===========================================================================


concession_t ConHintList={0};


/*
=============================================================================

							STATICS

=============================================================================
*/
statobj_t	statobjlist[MAXSTATS],*laststatobj;

stattype far statinfo[] =

{
{SPR_STAT_0,bo_water_puddle},		// Water Puddle					SPR1V
{SPR_STAT_1,block},			 		// Containment Canister
{SPR_STAT_2,block},				  	// Lunch Table
{SPR_STAT_3,block},			  		// Floor Lamp
{SPR_STAT_4,block},					// Lab Table
{SPR_STAT_5,block},					// Pillar
{SPR_STAT_6},					 		// Blood Puddle
{SPR_STAT_7},							// Piss Puddle

{SPR_STAT_8,block},					// Ficus Tree						SPR2V
{SPR_STAT_9},							// Half-Eaten Corpse
{SPR_STAT_10,block},		 			// Water Fountain
{SPR_STAT_11,block},		  			// Plant 1
{SPR_STAT_12,block},					// Vase
{SPR_STAT_13,block},		 			// General Table
{SPR_STAT_14},				  			// Ceiling Light
{SPR_STAT_15,block},		  			// General Chair

{SPR_STAT_16,block}, 				// Kitchen Trash 					SPR3V
{SPR_STAT_17},				  			// Office Trash
{SPR_STAT_18,block},		  			// Plant 2
{SPR_STAT_19,block},					// Gurney No-Blood
{SPR_STAT_20},							// Indirect Half-Sphere
{SPR_STAT_21},							// Exit Sign
{SPR_STAT_22},							// Transporter
{SPR_STAT_23,block},					// Body Can

{SPR_STAT_24,bo_pistol},			// PISTOL							SPR4V
{SPR_STAT_25,block},					// Statue

{SPR_STAT_31,bo_clip},				// Charge Unit

{SPR_STAT_27,bo_burst_rifle},		// Auto-Burst Rifle
{SPR_STAT_28,bo_ion_cannon},		// Particle Charged ION
{SPR_STAT_29,bo_firstaid},			// First Aid
{SPR_VSPIKE8,block},					// Static VSPIKE

{SPR_STAT_26,bo_clip2},				// Big Charge pack/clip

{SPR_STAT_32,bo_red_key},			// Red Key  						SPR5V
{SPR_STAT_33,bo_yellow_key},		// Yellow Key
{SPR_STAT_34,bo_bfg_cannon},	   // BFG Cannon
{SPR_STAT_35,bo_blue_key},			// Blue Key
{SPR_STAT_36},							// OPEN
{SPR_STAT_37,block},					// Office Desk
{SPR_STAT_38,block},	 				// Office Chair
{SPR_STAT_39,block},					// Security Desk

{SPR_STAT_40,bo_water},				// Full Water Bowl				SPR7V
{SPR_STAT_41},							// Empty Water Bowl
{SPR_STAT_42,bo_chicken},			// Chicken Leg
{SPR_STAT_43},							// Chicken Bone
{SPR_STAT_44,bo_ham},				// Ham
{SPR_STAT_45},							// Ham Bone
{SPR_STAT_46,bo_grenade},			// Grande Launcher
{SPR_STAT_47},							// Video Game Machine

{SPR_VPOST8,block},					// Static VPOST

//				-- VARIOUS --

{SPR_GURNEY_MUT_READY,block},		// 49 Gurney Mutant
{SPR_LCAN_ALIEN_READY,block},		// 50 Large	Alien Canister
{SPR_SCAN_ALIEN_READY,block},		// 51 Small Alien Canister

{SPR_GURNEY_MUT_EMPTY,block},		// 52 Gurney Mutant
{SPR_LCAN_ALIEN_EMPTY,block},		// 53 Large	Alien Canister
{SPR_SCAN_ALIEN_EMPTY,block},	   // 54 Small Alien Canister

{SPR_OFC_DEAD},						// 55 Dead Gen Sci.

{0},										// 56 Spacer

{SPR_AIR_VENT,bo_plainvent},		// 57 Plain air vent
{SPR_AIR_VENT,bo_bloodvent},		// 58 Blood air vent
{SPR_AIR_VENT,bo_watervent},		// 59 Water air vent
{SPR_GRATE},							// 60 Floor Grate
{SPR_STEAM_PIPE},						// 61 Steam Pipe

{SPR_STAT_48,bo_money_bag},		// 62 money bag
{SPR_STAT_49,bo_loot},				// 63 loot
{SPR_STAT_50,bo_gold},				// 64 gold
{SPR_STAT_51,bo_bonus},				// 65 bonus

{SPR_STAT_52, block},				// 66 Greek Post
{SPR_STAT_53, block},				// 67 Red/Blue post
{SPR_STAT_54, block},				// 68 Red HiTech Post
{SPR_STAT_55},							// 69 Ceiling Lamp #2
{SPR_STAT_56},							// 70 Ceiling Lamp #3
{SPR_STAT_57},							// 71 Body Parts
{SPR_STAT_58},							// 72 OR Lamp
{SPR_STAT_59,block},					// 73 Office Sink
{SPR_STAT_57,0},						// EMPTY - Copy of 71 - Body Parts...
{SPR_CANDY_BAR,bo_candybar},		// 75 candy bar
{SPR_SANDWICH,bo_sandwich},		// 76 sandwich
{SPR_CRATE_1,block},					// 77 Crate #1
{SPR_CRATE_2,block},					// 78 Crate #2
{SPR_CRATE_3,block},					// 79 Crate #3
{SPR_STAT_61,block},					//	80 Table
{SPR_STAT_62,block},					//	81 Chair
{SPR_STAT_63,block},					//	82 Stool
{SPR_STAT_64},							//	83 Gore

{SPR_STAT_65,bo_gold3},				// Gold 3
{SPR_STAT_66,bo_gold2},				// Gold 2
{SPR_STAT_67,bo_gold1},				// Gold 1

{SPR_STAT_68,block},					//
{SPR_STAT_69,block},					//
{SPR_STAT_70,block},					//
{SPR_STAT_71,block},					//
{SPR_STAT_72,block},					//
{SPR_STAT_73},							//
{SPR_STAT_74},							//
{SPR_STAT_75},							//
{SPR_STAT_76},							//

{SPR_RENT_DEAD},						//
{SPR_PRO_DEAD},						//
{SPR_SWAT_DEAD},						//
{SPR_GSCOUT_DEAD},					//
{SPR_FSCOUT_DEAD},					//
{SPR_MUTHUM1_DEAD},
{SPR_MUTHUM2_DEAD},
{SPR_LCAN_ALIEN_DEAD},
{SPR_SCAN_ALIEN_DEAD},
{SPR_GURNEY_MUT_DEAD},
{SPR_TERROT_DEAD},
{SPR_POD_DIE3},
{SPR_STAT_77,bo_coin},				// Concession Machine Money
{SPR_STAT_78,bo_coin5},				// Concession Machine Money
{SPR_STAT_79},							// Auto-Charge Pistol

{SPR_DOORBOMB,bo_plasma_detonator},			// Plasma Detonator		
{SPR_RUBBLE},								// Door Rubble
{SPR_AUTOMAPPER,bo_automapper1},		// Auto Mapper Bonus #1
{SPR_BONZI_TREE,block},					// BonziTree
{SPR_POT_PLANT,block},					// Yellow Potted Plant
{SPR_TUBE_PLANT,block},					// Tube Plant
{SPR_HITECH_CHAIR,block},				// Hi Tech table and chair
{SPR_DEAD_RENT},						// Dead AOG: Rent A Cop
{SPR_DEAD_PRO},						// Dead AOG: Pro Guard
{SPR_DEAD_SWAT},						// Dead AOG: Swat Guad


{-1}										// terminator

};


/*
===============
=
= InitStaticList
=
===============
*/

void InitStaticList (void)
{
	laststatobj = &statobjlist[0];
}

//---------------------------------------------------------------------------
// FindStatic()
//
// FUNCTION: Searches the stat obj list and returns ptr to a static obj
//				 at a particular tile x & tile y coords.
//
// RETURNS: Ptr == Pointer to static obj.
//          NULL == No static found.
//---------------------------------------------------------------------------
statobj_t *FindStatic(unsigned tilex, unsigned tiley)
{
	statobj_t	*spot;

	for (spot=statobjlist;spot!=laststatobj;spot++)
		if (spot->shapenum != -1 && spot->tilex == tilex && spot->tiley == tiley)
         	return(spot);

   return(NULL);
}

//---------------------------------------------------------------------------
// FindEmptyStatic()
//
// FUNCTION: Searches the stat obj list and returns ptr to an empty
//				 static object.
//
// RETURNS: Ptr == Pointer to empty static obj.
//          NULL == static objlist full.
//---------------------------------------------------------------------------
statobj_t *FindEmptyStatic(void)
{
	statobj_t	*spot;

	for (spot=&statobjlist[0] ; ; spot++)
	{
		if (spot==laststatobj)
		{
			if (spot == &statobjlist[MAXSTATS])
         	return(NULL);
			laststatobj++;						// space at end
			break;
		}

		if (spot->shapenum == -1)				// -1 is a free spot
			break;
	}

   return(spot);
}


/*
===============
=
= SpawnStatic
=
===============
*/

void SpawnStatic (int tilex, int tiley, int type)
{
	statobj_t	*spot;

#if 0
#if IN_DEVELOPMENT
	if (tilemap[tilex][tiley])
		Quit("Static spawned on wall.  %d,%d",tilex,tiley);
//		ACT1_ERROR(SPAWNSTATIC_ON_WALL);
#endif
#endif

	if (!(spot = FindEmptyStatic()))
		return;

	spot->shapenum = statinfo[type].picnum;
	spot->tilex = tilex;
	spot->tiley = tiley;
	spot->visspot = &spotvis[tilex][tiley];
 	spot->flags = 0;

#if IN_DEVELOPMENT
#if GAME_VERSION == SHAREWARE_VERSION
	if (!spot->shapenum)
		Quit("Invalid static: %d %d",tilex,tiley);
#endif
#endif


   switch (spot->shapenum)
   {
#if GAME_VERSION != SHAREWARE_VERSION
   	case SPR_STAT_3:			// floor lamp
#endif
      case SPR_STAT_14:			// ceiling light
#if GAME_VERSION != SHAREWARE_VERSION
      case SPR_STAT_20:			//
#endif
      case SPR_STAT_47:
      case SPR_STAT_51:
      case SPR_STAT_55:
      case SPR_STAT_56:
		   spot->lighting = LAMP_ON_SHADING;
      break;

		default:
		   spot->lighting = 0;
      break;
   }


	switch (statinfo[type].type)
	{
		case block:
			(unsigned)actorat[tilex][tiley] = 1;		// consider it a blocking tile
		break;

		case	bo_red_key:
		case	bo_yellow_key:
		case	bo_blue_key:
		case  bo_plasma_detonator:
			TravelTable[tilex][tiley] |= TT_KEYS;

		case	bo_gold1:
		case	bo_gold2:
		case	bo_gold3:
		case	bo_gold:
		case	bo_bonus:
		case	bo_money_bag:
		case	bo_loot:

		case	bo_fullheal:
		case	bo_firstaid:
		case	bo_clip:
		case	bo_clip2:
		case	bo_burst_rifle:
		case	bo_ion_cannon:
		case 	bo_grenade:
		case 	bo_bfg_cannon:		
		case  bo_pistol:
		case  bo_chicken:
		case  bo_ham:
		case  bo_water:
		case  bo_water_puddle:
		case  bo_sandwich:
		case  bo_candybar:
		case  bo_coin:
		case  bo_coin5:
		case  bo_automapper1:
			spot->flags = FL_BONUS;
			spot->itemnumber = statinfo[type].type;
		break;
	}

	spot->areanumber=GetAreaNumber(spot->tilex,spot->tiley);

	spot++;

	if (spot == &statobjlist[MAXSTATS])
		ACT1_ERROR(SPAWNSTATIC_TOO_MANY);
}



//---------------------------------------------------------------------------
// ReserveStatic()
//
// Reserves a static object at location 0,0 (unseen).  This function is
// used to gaurantee that a static will be available.
//---------------------------------------------------------------------------
statobj_t  *ReserveStatic(void)
{
	statobj_t	*spot;

	if (!(spot = FindEmptyStatic()))
		ACT1_ERROR(SPAWNSTATIC_TOO_MANY);

	// Mark as Used.

	spot->shapenum = 1;
	spot->tilex = 0;
	spot->tiley = 0;
	spot->visspot = &spotvis[0][0];

   return(spot);
}


//---------------------------------------------------------------------------
// FindReservedStatic()
//
// Finds a Reserved static object at location 0,0 (unseen).  This function is
// used to gaurantee that a static will be available.
//---------------------------------------------------------------------------
statobj_t  *FindReservedStatic(void)
{
	statobj_t	*spot;

	for (spot=&statobjlist[0];spot < &statobjlist[MAXSTATS];spot++)
	{
		if (spot->shapenum == 1 && (!spot->tilex) && (!spot->tiley))				// -1 is a free spot
			return(spot);
	}

   return(NULL);
}



//---------------------------------------------------------------------------
// UseReservedStatic()
//
// Finds a Reserved static object and moves it to a new location with new
// attributes.
//
// This function acts like PlaceItemType - But REQUIRES a reserved
// static.  Before using this function, make sure that you have already
// reserved a static to be used using ReserveStatic();
//---------------------------------------------------------------------------
statobj_t *UseReservedStatic(int itemtype, int tilex, int tiley)
{
	statobj_t	*spot;
	int			type;

   if (!(spot = FindReservedStatic()))
		ACT1_ERROR(CANT_FIND_RESERVE_STATIC);

	//
	// find the item number
	//

	for (type=0;;type++)
	{
		if (statinfo[type].picnum == -1)		// End of Static List...
			ACT1_ERROR(PLACEITEMTYPE_NO_TYPE);

		if (statinfo[type].type == itemtype)	// Bingo, Found it!
			break;
	}

	//
	// place it
	//

   switch (type)
   {
		case	bo_red_key:
		case	bo_yellow_key:
		case	bo_blue_key:
			TravelTable[tilex][tiley] |= TT_KEYS;
      break;
   }

	spot->shapenum = statinfo[type].picnum;
	spot->tilex = tilex;
	spot->tiley = tiley;
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = FL_BONUS;
	spot->itemnumber = statinfo[type].type;

	spot->areanumber=GetAreaNumber(spot->tilex,spot->tiley);

#if IN_DEVELOPMENT
		if (spot->areanumber >= NUMAREAS)
			Quit("Static Spawned on a wall at %d %d",spot->tilex,spot->tiley);
#endif

	return(spot);
}

//--------------------------------------------------------------------------
// PlaceReservedItemNearTile()
//--------------------------------------------------------------------------
char far pint_xy[8][2]={{-1,-1},{-1, 0},{-1, 1},
								{ 0,-1},        { 0, 1},
								{ 1,-1},{ 1, 0},{ 1, 1}};

void PlaceReservedItemNearTile(int itemtype, int tilex, int tiley)
{
	char loop;

	for (loop=0; loop<8; loop++)
	{
		char x=tilex+pint_xy[loop][1], y=tiley+pint_xy[loop][0];

		if (!tilemap[x][y])
		{
			if (actorat[x][y] == (objtype *)1)		// Check for a SOLID static
	        	continue;

			UseReservedStatic(itemtype,x,y);
			return;
		}
	}

	UseReservedStatic(itemtype,tilex,tiley);
}



/*
===============
=
= PlaceItemType
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
===============
*/

void PlaceItemType (int itemtype, int tilex, int tiley)
{
	int			type;
	statobj_t	*spot;

//
// find the item number
//
	for (type=0 ;  ; type++)
	{
		if (statinfo[type].picnum == -1)		// end of list
			ACT1_ERROR(PLACEITEMTYPE_NO_TYPE);
		if (statinfo[type].type == itemtype)
			break;
	}

//
// find a spot in statobjlist to put it in
//
	if (!(spot = FindEmptyStatic()))
   	return;

//
// place it
//
	spot->shapenum = statinfo[type].picnum;
	spot->tilex = tilex;
	spot->tiley = tiley;
	spot->visspot = &spotvis[tilex][tiley];
	spot->flags = FL_BONUS;
	spot->itemnumber = statinfo[type].type;

	spot->areanumber=GetAreaNumber(spot->tilex,spot->tiley);

#if IN_DEVELOPMENT
		if (spot->areanumber >= NUMAREAS)
			Quit("Item Spawned on a wall at %d %d",spot->tilex,spot->tiley);
#endif

}

//--------------------------------------------------------------------------
// PlaceItemNearTile()
//--------------------------------------------------------------------------
void PlaceItemNearTile(int itemtype, int tilex, int tiley)
{
// [0] is the y offset
// [1] is the x offset
//
	char loop;

	for (loop=0; loop<8; loop++)
	{
		char x=tilex+pint_xy[loop][1], y=tiley+pint_xy[loop][0];

		if (!tilemap[x][y])
		{
			if (actorat[x][y] == (objtype *)1)		// Check for a SOLID static
	        	continue;

			PlaceItemType(itemtype,x,y);
			return;
		}
	}

	PlaceItemType(itemtype,tilex,tiley);
}




//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// ExplodeStatics()
//
//  NOTES: Explodes statics in a one tile radius from a given tile x and tile y
//
//--------------------------------------------------------------------------
void ExplodeStatics(int tilex, int tiley)
{
	statobj_t *statobj, *spot;
   objtype *obj;						
   int y_diff,x_diff;
	boolean remove;

	for (spot=&statobjlist[0] ; spot != laststatobj ; spot++)
		if (spot->shapenum != -1)
      {
      	y_diff = spot->tiley - tiley;
         y_diff = ABS(y_diff);

      	x_diff = spot->tilex - tilex;
         x_diff = ABS(x_diff);

			if (x_diff < 2 && y_diff <2)
         {
				remove = false;

            //
            // Test for specific statics..
            //

			  	switch (spot->itemnumber)
			   {
					//
					// Check for Clips
					//
					case bo_clip:
					case bo_clip2:
						remove = true;
						SpawnCusExplosion((((fixed)spot->tilex)<<TILESHIFT)+0x7FFF,
												(((fixed)spot->tiley)<<TILESHIFT)+0x7FFF,
												SPR_CLIP_EXP1, 7, 3+(US_RndT()&0x3),explosionobj);
					break;
				}


				//
				// Check to see if we remove it.
				//
				if (remove)
				{
					// Remove static
					spot->shapenum = -1;
					spot->itemnumber = bo_nothing;
				}
			}
		}
}



/*
=============================================================================

							DOORS

doorobjlist[] holds most of the information for the doors

doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
	this is directly accessed by AsmRefresh during rendering

The number of doors is limited to 64 because a spot in tilemap holds the
	door number in the low 6 bits, with the high bit meaning a door center
	and bit 6 meaning a door side tile

Open doors conect two areas, so sounds will travel between them and sight
	will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
	An area is true if it connects with the player's current spor.

=============================================================================
*/

#define DOORWIDTH	0x7800
#define OPENTICS	300

doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;
int			doornum;

unsigned	doorposition[MAXDOORS];		// leading edge of door 0=closed
										// 0xffff = fully open

byte		far areaconnect[NUMAREAS][NUMAREAS];

boolean		areabyplayer[NUMAREAS];


/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void RecursiveConnect (int areanumber)
{
	int	i;

	for (i=0;i<NUMAREAS;i++)
	{
		if (areaconnect[areanumber][i] && !areabyplayer[i])
		{
			areabyplayer[i] = true;
			RecursiveConnect (i);
		}
	}
}


void ConnectAreas (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	areabyplayer[player->areanumber] = true;
	RecursiveConnect (player->areanumber);
}


void InitAreas (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	areabyplayer[player->areanumber] = true;
}



/*
===============
=
= InitDoorList
=
===============
*/

void InitDoorList (void)
{
	memset (areabyplayer,0,sizeof(areabyplayer));
	_fmemset (areaconnect,0,sizeof(areaconnect));

	lastdoorobj = &doorobjlist[0];
	doornum = 0;
}


/*
===============
=
= SpawnDoor
=
===============
*/

void SpawnDoor (int tilex, int tiley, boolean vertical, keytype lock, door_t type)
{
	int	areanumber;
	unsigned	far *map[2];

	map[0] = mapsegs[0] + farmapylookup[tiley]+tilex;
	map[1] = mapsegs[1] + farmapylookup[tiley]+tilex;

	if (doornum==64)
		ACT1_ERROR(SPAWNDOOR_TOO_MANY);

	doorposition[doornum] = 0;		// doors start out fully closed
	lastdoorobj->tilex = tilex;
	lastdoorobj->tiley = tiley;
	lastdoorobj->vertical = vertical;
	lastdoorobj->lock = lock;
	lastdoorobj->type = type;
	lastdoorobj->action = dr_closed;
	lastdoorobj->flags = DR_BLASTABLE;	// JIM - Do something with this! jdebug

	if (vertical)
	{
		lastdoorobj->areanumber[0]=GetAreaNumber(tilex+1,tiley);
		lastdoorobj->areanumber[1]=GetAreaNumber(tilex-1,tiley);
	}
	else
	{
		lastdoorobj->areanumber[0]=GetAreaNumber(tilex,tiley-1);
		lastdoorobj->areanumber[1]=GetAreaNumber(tilex,tiley+1);
	}


	(unsigned)actorat[tilex][tiley] = doornum | 0x80;	// consider it a solid wall

//
// make the door tile a special tile, and mark the adjacent tiles
// for door sides
//
	tilemap[tilex][tiley] = doornum | 0x80;

	if (vertical)
	{
		if (*(map[0]-mapwidth-1) == TRANSPORTERTILE)
			*map[0] = GetAreaNumber(tilex+1,tiley);
		else
			*map[0] = GetAreaNumber(tilex-1,tiley);
		tilemap[tilex][tiley-1] |= 0x40;
		tilemap[tilex][tiley+1] |= 0x40;
	}
	else
	{
		*map[0] = GetAreaNumber(tilex,tiley-1);
		tilemap[tilex-1][tiley] |= 0x40;
		tilemap[tilex+1][tiley] |= 0x40;
	}

	doornum++;
	lastdoorobj++;
}

//===========================================================================



//--------------------------------------------------------------------------
// CheckLinkedDoors
//--------------------------------------------------------------------------
void CheckLinkedDoors(short door, short door_dir)
{
	static short LinkCheck=0;
	static short base_tilex;
	static short base_tiley;

	short tilex=doorobjlist[door].tilex,
			tiley=doorobjlist[door].tiley,
			next_tilex=0,
			next_tiley=0;

// Find next door in link.
//
//	if ((*(mapsegs[1]+(farmapylookup[tiley]+tilex+1)) & 0xff00) == 0xf900)
	if (*(mapsegs[1]+(farmapylookup[tiley]+tilex)))
	{
//		unsigned value=*(mapsegs[1]+(farmapylookup[tiley]+tilex+2));
		unsigned value=*(mapsegs[1]+(farmapylookup[tiley]+tilex));

	// Define the next door in the link.
	//
		next_tilex = (value & 0xff00)>>8;
		next_tiley = value & 0xff;

	// Is this the head of the link?
	//
		if (!LinkCheck)
		{
			base_tilex=tilex;
			base_tiley=tiley;
		}
	}

	LinkCheck++;

// Recursively open/close linked doors.
//
	if ((next_tilex) &&
		 (next_tiley) &&
		 ((next_tilex != base_tilex) || (next_tiley != base_tiley))
		)
	{
		short door=tilemap[next_tilex][next_tiley] & ~0x80;

		switch (door_dir)
		{
			case dr_opening:
				doorobjlist[door].lock = kt_none;
				OpenDoor(door);
			break;

			case dr_closing:
				doorobjlist[door].lock = kt_none;
				CloseDoor(door);
			break;
		}
	}

	LinkCheck--;
}


/*
=====================
=
= OpenDoor
=
=====================
*/

void OpenDoor (int door)
{

  	if (doorobjlist[door].action == dr_jammed)
   	return;												

	if (doorobjlist[door].action == dr_open)
		doorobjlist[door].ticcount = 0;			// reset open time
	else
		doorobjlist[door].action = dr_opening;	// start it opening

	CheckLinkedDoors(door,dr_opening);

}


/*
=====================
=
= CloseDoor
=
=====================
*/

void CloseDoor (int door)
{
	int	tilex,tiley,area;
	objtype *check;

  	if (doorobjlist[door].action == dr_jammed)
   	return;											

//
// don't close on anything solid
//
	tilex = doorobjlist[door].tilex;
	tiley = doorobjlist[door].tiley;

	if (actorat[tilex][tiley])
		return;

	if (player->tilex == tilex && player->tiley == tiley)
		return;

	if (doorobjlist[door].vertical)
	{
		if ( player->tiley == tiley )
		{
			if ( ((player->x+MINDIST) >>TILESHIFT) == tilex )
				return;
			if ( ((player->x-MINDIST) >>TILESHIFT) == tilex )
				return;
		}
		check = actorat[tilex-1][tiley];
		if (check && ((check->x+MINDIST) >> TILESHIFT) == tilex )
			return;
		check = actorat[tilex+1][tiley];
		if (check && ((check->x-MINDIST) >> TILESHIFT) == tilex )
			return;
	}
	else if (!doorobjlist[door].vertical)
	{
		if (player->tilex == tilex)
		{
			if ( ((player->y+MINDIST) >>TILESHIFT) == tiley )
				return;
			if ( ((player->y-MINDIST) >>TILESHIFT) == tiley )
				return;
		}
		check = actorat[tilex][tiley-1];
		if (check && ((check->y+MINDIST) >> TILESHIFT) == tiley )
			return;
		check = actorat[tilex][tiley+1];
		if (check && ((check->y-MINDIST) >> TILESHIFT) == tiley )
			return;
	}


//
// play door sound if in a connected area
//
	area = GetAreaNumber(doorobjlist[door].tilex,doorobjlist[door].tiley);
	if (areabyplayer[area])
	{
		switch(doorobjlist[door].type)
		{
			case dr_bio:
			case dr_office:
			case dr_space:
			case dr_normal:
				PlaySoundLocTile(HTECHDOORCLOSESND,doorobjlist[door].tilex,doorobjlist[door].tiley);
			break;

			default:
				PlaySoundLocTile(CLOSEDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);
			break;
		}
	}

	doorobjlist[door].action = dr_closing;
//
// make the door space solid
//
	(unsigned)actorat[tilex][tiley]
		= door | 0x80;

	CheckLinkedDoors(door,dr_closing);

}

/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

char far od_oneway[]="\r\r   DOOR LOCKED FROM\r      THIS SIDE.\r^XX";
char far od_locked[]="\r\r   DOOR PERMANENTLY\r        LOCKED.\r^XX";
char far od_reddenied[]="\r\r      RED LEVEL\r    ACCESS DENIED!\r^XX";
char far od_yellowdenied[]="\r\r     YELLOW LEVEL\r    ACCESS DENIED!\r^XX";
char far od_bluedenied[]="\r\r      BLUE LEVEL\r    ACCESS DENIED!\r^XX";
char far od_granted[]="\r\r    ACCESS GRANTED\r    DOOR UNLOCKED.\r^XX";
char far od_operating[]="\r\r    OPERATING DOOR.\r^XX";

void OperateDoor (int door)
{
	int	lock;
	boolean OperateMsg,oneway = false;


	//
	// Check for wrong way on a ONEWAY door.
	//

	switch (doorobjlist[door].type)
	{
		case dr_oneway_left:
			if (player->tilex < doorobjlist[door].tilex)
				oneway = true;
			break;

		case dr_oneway_right:
			if (player->tilex > doorobjlist[door].tilex)
				oneway = true;
			break;

		case dr_oneway_up:
			if (player->tiley < doorobjlist[door].tiley)
				oneway = true;
			break;

		case dr_oneway_down:
			if (player->tiley > doorobjlist[door].tiley)
				oneway = true;
			break;
	}

	if (oneway)
	{
		if (doorobjlist[door].action == dr_closed)
		{
			DISPLAY_TIMED_MSG(od_oneway,MP_DOOR_OPERATE,MT_GENERAL);
			SD_PlaySound(NOWAYSND);
		}

		return;
	}

	//
	// Check for possibly being locked
	//

	lock = doorobjlist[door].lock;
	if (lock != kt_none)
	{
		if (!(gamestate.numkeys[lock-kt_red]))
		{
				SD_PlaySound(NOWAYSND);
				switch (lock)
				{
					case kt_red:
						DISPLAY_TIMED_MSG(od_reddenied,MP_DOOR_OPERATE,MT_GENERAL);
					break;

					case kt_yellow:
						DISPLAY_TIMED_MSG(od_yellowdenied,MP_DOOR_OPERATE,MT_GENERAL);
					break;

					case kt_blue:
						DISPLAY_TIMED_MSG(od_bluedenied,MP_DOOR_OPERATE,MT_GENERAL);
					break;

					default:
						DISPLAY_TIMED_MSG(od_locked,MP_DOOR_OPERATE,MT_GENERAL);
					break;
				}

			return;
		}
		else
		{
			TakeKey(lock-kt_red);
			DISPLAY_TIMED_MSG(od_granted,MP_DOOR_OPERATE,MT_GENERAL);
			doorobjlist[door].lock = kt_none;		// UnLock door
		}
	}
	else
		DISPLAY_TIMED_MSG(od_operating,MP_DOOR_OPERATE,MT_GENERAL);

	switch (doorobjlist[door].action)
	{
		case dr_closed:
		case dr_closing:
			OpenDoor (door);
			break;

		case dr_open:
		case dr_opening:
			CloseDoor (door);
			break;
	}
}



//--------------------------------------------------------------------------
// BlockDoorOpen()
//--------------------------------------------------------------------------
void BlockDoorOpen(int door)
{
	doorobjlist[door].action = dr_jammed;
	doorobjlist[door].ticcount = 0;
	doorposition[door] = 0xffff;
	doorobjlist[door].lock = kt_none;
	doorobjlist[door].flags &= ~DR_BLASTABLE;

	actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
//	tilemap[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;

	TransformAreas(doorobjlist[door].tilex,doorobjlist[door].tiley,1);
//   SpawnStatic(doorobjlist[door].tilex, doorobjlist[door].tiley, DOOR_RUBBLE);
}

//--------------------------------------------------------------------------
// TryBlastDoor()
//
//--------------------------------------------------------------------------
void TryBlastDoor(char door)
{
	switch (doorobjlist[door].type)
   {
   	case dr_oneway_left:
		case dr_oneway_up:
		case dr_oneway_right:
		case dr_oneway_down:
   	break;

		default:
			if (doorposition[door] < 0x7fff &&
			    doorobjlist[door].action != dr_jammed &&
				 doorobjlist[door].lock == kt_none)
			{
				BlockDoorOpen(door);
				SpawnCusExplosion((((fixed)doorobjlist[door].tilex)<<TILESHIFT)+0x7FFF,
								      (((fixed)doorobjlist[door].tiley)<<TILESHIFT)+0x7FFF,
								      SPR_EXPLOSION_1,4,3,doorexplodeobj);
			}
      break;
   }
}


//--------------------------------------------------------------------------
// BlastNearDoors()
//
//--------------------------------------------------------------------------
void BlastNearDoors(int tilex, int tiley)
{
	unsigned char door;
	char far *doorptr;
	int x,y;

	doorptr = (char far *)&tilemap[tilex][tiley];

	for (x=-1;x<2;x++)
		for (y=-64;y<128;y+=64)
		{
			if ((door = *(doorptr+x+y)) & 0x80)
			{
				door &= ~0x80;
				TryBlastDoor(door);
			}
		}
}



//--------------------------------------------------------------------------
// DropPlasmaDetonator()  - Will move a Chaff from reserve to the player location.
//--------------------------------------------------------------------------
void DropPlasmaDetonator(void)
{
	objtype *obj;

#pragma warn -pia

	if (obj = MoveHiddenOfs(plasma_detonator_reserveobj,plasma_detonatorobj,player->x,player->y))
	{
   	obj->flags |= FL_SHOOTABLE;

		DISPLAY_TIMED_MSG(pd_dropped,MP_DOOR_OPERATE,MT_GENERAL);
		SD_PlaySound(ROBOT_SERVOSND);		// jdebug-This sound will probly change.
      TakePlasmaDetonator(1);
      return;
	}

#pragma warn +pia

	ACT1_ERROR(NO_DOORBOMB_SPARES);
}



//--------------------------------------------------------------------------
// TryDropPlasmaDetonator()  - Will check to see if player is close enough to
//										 drop a detonator.
//--------------------------------------------------------------------------
void TryDropPlasmaDetonator(void)
{
	#define MAX_RANGE_DIST        2
	objtype *obj;
   short distx,disty,distance;


   if (!gamestuff.level[gamestate.mapon+1].locked)
   {
		DISPLAY_TIMED_MSG(pd_floornotlocked, MP_DETONATOR, MT_GENERAL);
      return;
   }

   if (gamestate.mapon > 19)
   {
		DISPLAY_TIMED_MSG(pd_no_computer, MP_DETONATOR, MT_GENERAL);
   	return;
   }

   if (!gamestate.plasma_detonators)
   {
		DISPLAY_TIMED_MSG(pd_donthaveany, MP_DETONATOR, MT_GENERAL);
      return;
   }

   if (!(obj = FindObj(rotating_cubeobj,-1,-1)))
   	ACT1_ERROR(CANT_FIND_LEVELCOMPUTER);

   if (obj->areanumber != player->areanumber)
   {
		DISPLAY_TIMED_MSG(pd_notnear,MP_DETONATOR,MT_GENERAL);
      return;
   }

   distx = player->tilex - obj->tilex;
   distx = ABS(distx);
   disty = player->tiley - obj->tiley;
   disty = ABS(disty);
   distance = distx>disty ? distx:disty;

   if (distance > MAX_RANGE_DIST)
   {
		DISPLAY_TIMED_MSG(pd_getcloser,MP_DETONATOR,MT_GENERAL);
      return;
   }
   else
   	DropPlasmaDetonator();
}


//===========================================================================

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/

void DoorOpen (int door)
{
	if ( (doorobjlist[door].ticcount += tics) >= OPENTICS)
		CloseDoor (door);
}

#define USE_TRANSFORMAREAS

#ifdef USE_TRANSFORMAREAS

//--------------------------------------------------------------------------
// TransformAreas()
//--------------------------------------------------------------------------
int TransformAreas(char tilex, char tiley, char xform)
{
	short xofs,yofs;
	byte		area1,area2;
	unsigned	far	*map,offset;

// Is this walkway:  Horizontal?   Vertical?   Error?
//
	if ((tilemap[tilex][tiley+1]) && (tilemap[tilex][tiley-1]))
	{
		xofs = 1;
		yofs = 0;
	}
	else
		if ((tilemap[tilex+1][tiley]) && (tilemap[tilex-1][tiley]))
		{
			xofs = 0;
			yofs = 1;
		}
		else
			ACT1_ERROR(LINKAREA_BAD_LINK);

// Define the two areas...
//
	area1=GetAreaNumber(tilex+xofs,tiley+yofs);
	if (area1 >= NUMAREAS)
		ACT1_ERROR(TRANSFORM_AREA1_OUT_OF_RANGE);

	area2=GetAreaNumber(tilex-xofs,tiley-yofs);
	if (area2 >= NUMAREAS)
		ACT1_ERROR(TRANSFORM_AREA2_OUT_OF_RANGE);

// Connect these two areas.
//
	areaconnect[area1][area2] += xform;
	areaconnect[area2][area1] += xform;
	ConnectAreas ();

	return(area1);
}

#endif

/*
===============
=
= DoorOpening
=
===============
*/

void DoorOpening (int door)
{
	int		area1,area2;
	unsigned	far	*map;
	long	position;

	position = doorposition[door];
	if (!position)
	{
		area1=TransformAreas(doorobjlist[door].tilex,doorobjlist[door].tiley,1);

		if (areabyplayer[area1])
		{
//			PlaySoundLocTile(OPENDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);	// JAB
			switch(doorobjlist[door].type)
			{
				case dr_bio:
				case dr_office:
				case dr_space:
				case dr_normal:
					PlaySoundLocTile(HTECHDOOROPENSND,doorobjlist[door].tilex,doorobjlist[door].tiley);
				break;

				default:
					PlaySoundLocTile(OPENDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);
				break;
			}
		}
	}

//
// slide the door by an adaptive amount
//
	position += tics<<10;
	if (position >= 0xffff)
	{
	//
	// door is all the way open
	//
		position = 0xffff;
		doorobjlist[door].ticcount = 0;
		doorobjlist[door].action = dr_open;
		actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
	}

	doorposition[door] = position;
}

/*
===============
=
= DoorClosing
=
===============
*/
void DoorClosing (int door)
{
	int		area1,area2,move;
	unsigned	far	*map;
	long	position;
	int		tilex,tiley;

	tilex = doorobjlist[door].tilex;
	tiley = doorobjlist[door].tiley;

	if ( ((unsigned)actorat[tilex][tiley] != (door | 0x80))
	|| (player->tilex == tilex && player->tiley == tiley) )
	{			// something got inside the door
		OpenDoor (door);
		return;
	};

	position = doorposition[door];

//
// slide the door by an adaptive amount
//
	position -= tics<<10;
	if (position <= 0)
	{
#ifdef USE_TRANSFORMAREAS
		position = 0;
		doorobjlist[door].action = dr_closed;
		TransformAreas(doorobjlist[door].tilex,doorobjlist[door].tiley,-1);
#else
	//
	// door is closed all the way, so disconnect the areas
	//
		position = 0;

		doorobjlist[door].action = dr_closed;

		map = mapsegs[0] + farmapylookup[doorobjlist[door].tiley]
			+doorobjlist[door].tilex;

		if (doorobjlist[door].vertical)
		{
			area1 =	*(map+1);
			area2 =	*(map-1);
		}
		else
		{
			area1 =	*(map-mapwidth);
			area2 =	*(map+mapwidth);
		}
		area1 -= AREATILE;
		if (area1 >= HIDDENAREATILE-AREATILE)
			area1 -= HIDDENAREATILE-AREATILE;
		area2 -= AREATILE;
		if (area2 >= HIDDENAREATILE-AREATILE)
			area2 -= HIDDENAREATILE-AREATILE;
		areaconnect[area1][area2]--;
		areaconnect[area2][area1]--;
		ConnectAreas ();
#endif
	}

	doorposition[door] = position;
}

/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/
void MoveDoors (void)
{
	int		door;

	for (door = 0 ; door < doornum ; door++)
		switch (doorobjlist[door].action)
		{
		case dr_open:
			DoorOpen (door);
			break;

		case dr_opening:
			DoorOpening(door);
			break;

		case dr_closing:
			DoorClosing(door);
			break;
		}
}

/*
=============================================================================

						PUSHABLE WALLS

=============================================================================
*/

unsigned	pwallstate;
unsigned	pwallpos;			// amount a pushable wall has been moved (0-63)
unsigned	pwallx=0,pwally=0;
int pwalldir,pwalldist;

/*
===============
=
= PushWall
=
===============
*/

void PushWall (int checkx, int checky, int dir)
{
	int		oldtile;

	if (pwallstate)
	  return;

	TransformAreas(checkx,checky,1);		  

	oldtile = tilemap[checkx][checky];
	if (!oldtile)
		return;

	switch (dir)
	{
	case di_north:
		if (actorat[checkx][checky-1])
		{
			return;
		}
		(unsigned)actorat[checkx][checky-1] =
		tilemap[checkx][checky-1] = oldtile;
		break;

	case di_east:
		if (actorat[checkx+1][checky])
		{
			return;
		}
		(unsigned)actorat[checkx+1][checky] =
		tilemap[checkx+1][checky] = oldtile;
		break;

	case di_south:
		if (actorat[checkx][checky+1])
		{
			return;
		}
		(unsigned)actorat[checkx][checky+1] =
		tilemap[checkx][checky+1] = oldtile;
		break;

	case di_west:
		if (actorat[checkx-1][checky])
		{
			return;
		}
		(unsigned)actorat[checkx-1][checky] =
		tilemap[checkx-1][checky] = oldtile;
		break;
	}

	pwalldist=2;
	pwallx = checkx;
	pwally = checky;
	pwalldir = dir;
	pwallstate = 1;
	pwallpos = 0;
	tilemap[pwallx][pwally] |= 0xc0;
	*(mapsegs[1]+farmapylookup[pwally]+pwallx) = 0;	// remove P tile info

	SD_PlaySound (PUSHWALLSND);
}

/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls (void)
{
	int		oldblock,oldtile;

	if (!pwallstate)
		return;

	oldblock = pwallstate/128;

	pwallstate += tics*4;

	if (pwallstate/128 != oldblock)
	{
		unsigned areanumber;

		pwalldist--;

	// block crossed into a new block
		oldtile = tilemap[pwallx][pwally] & 63;

		//
		// the tile can now be walked into
		//
		tilemap[pwallx][pwally] = 0;
		(unsigned)actorat[pwallx][pwally] = 0;
		areanumber=GetAreaNumber(player->tilex,player->tiley);
		if (GAN_HiddenArea)
			areanumber += HIDDENAREATILE;
		else
			areanumber += AREATILE;
		*(mapsegs[0]+farmapylookup[pwally]+pwallx) = areanumber;

		//
		// see if it should be pushed farther
		//
		if (!pwalldist)
		{
		//
		// the block has been pushed two tiles
		//
			pwallstate = 0;
			return;
		}
		else
		{
			switch (pwalldir)
			{
			case di_north:
				pwally--;
				if (actorat[pwallx][pwally-1])
				{
					pwallstate = 0;
					return;
				}
				(unsigned)actorat[pwallx][pwally-1] =
				tilemap[pwallx][pwally-1] = oldtile;
				break;

			case di_east:
				pwallx++;
				if (actorat[pwallx+1][pwally])
				{
					pwallstate = 0;
					return;
				}
				(unsigned)actorat[pwallx+1][pwally] =
				tilemap[pwallx+1][pwally] = oldtile;
				break;

			case di_south:
				pwally++;
				if (actorat[pwallx][pwally+1])
				{
					pwallstate = 0;
					return;
				}
				(unsigned)actorat[pwallx][pwally+1] =
				tilemap[pwallx][pwally+1] = oldtile;
				break;

			case di_west:
				pwallx--;
				if (actorat[pwallx-1][pwally])
				{
					pwallstate = 0;
					return;
				}
				(unsigned)actorat[pwallx-1][pwally] =
				tilemap[pwallx-1][pwally] = oldtile;
				break;
			}

			tilemap[pwallx][pwally] = oldtile | 0xc0;
		}
	}


	pwallpos = (pwallstate/2)&63;
}

//==========================================================================
//
//                    'SPECIAL MESSAGE' CACHING SYSTEM
//
// When creating special 'types' of message caching structures, make sure
// all 'special data' is placed at the end of the BASIC message structures.
// In memory, BASIC INFO should appear first. ex:
//
// mCacheList
// ---> NumMsgs
// ---> mCacheInfo
//      ---> local_val
//      ---> global_val
//      ---> mSeg
//
// ... all special data follows ...
//
//==========================================================================

//--------------------------------------------------------------------------
// InitMsgCache()
//--------------------------------------------------------------------------
void InitMsgCache(mCacheList *mList,unsigned listSize, unsigned infoSize)
{
	FreeMsgCache(mList,infoSize);
	memset(mList,0,listSize);
}

//--------------------------------------------------------------------------
// FreeMsgCache()
//--------------------------------------------------------------------------
void FreeMsgCache(mCacheList *mList, unsigned infoSize)
{
	mCacheInfo *ci=mList->mInfo;
	char *ch_ptr;

	while (mList->NumMsgs--)
	{
		if (ci->mSeg)
			MM_FreePtr(&ci->mSeg);

		ch_ptr = (char far *)ci;
		ch_ptr += infoSize;
		ci = (mCacheInfo *)ch_ptr;
	}
}

extern char far int_xx[];

//---------------------------------------------------------------------------
// CacheMsg()
//
// Caches the specific message in FROM a given 'grsegs' block TO the
// next available message segment pointer.
//---------------------------------------------------------------------------
void CacheMsg(mCacheInfo *ci, unsigned SegNum, unsigned MsgNum)
{
//	char far *Message, far *EndOfMsg, far *hint_buffer;
//	unsigned char pos=0;

// Alloc memory for message and cache-in seg
//
	MM_GetPtr(&ci->mSeg,MAX_CACHE_MSG_LEN);
//	hint_buffer = ci->mSeg;

// Load message into CachInfo Message Seg.
//
	LoadMsg(ci->mSeg,SegNum,MsgNum,MAX_CACHE_MSG_LEN);
}

//---------------------------------------------------------------------------
// LoadMsg()
//
// Loads the specific message in FROM a given 'grsegs' block TO the
// the memory address provided.  Memory allocation and handleing prior and
// after this function usage is responsiblity of the calling function(s).
//
// PARAMS:  hint_buffer 	- Destination address to store message
//			   SegNum		   - GrSeg for messages in VGAGRAPH.BS?
//			   MsgNum			- Message number to load
//				MaxMsgLen		- Max len of cache msg (Len of hint_buffer)
//
// RETURNS : Returns the length of the loaded message
//---------------------------------------------------------------------------
short LoadMsg(char far *hint_buffer, unsigned SegNum, unsigned MsgNum, unsigned MaxMsgLen)
{
	char far *Message, far *EndOfMsg;
	short pos=0;

	CA_CacheGrChunk(SegNum);
	Message = grsegs[SegNum];

// Search for end of MsgNum-1 (Start of our message)
//
#pragma warn -pia
	while (--MsgNum)
	{
		if (!(Message = _fstrstr(Message,int_xx)))
			ACT1_ERROR(INVALID_CACHE_MSG_NUM);

		Message += 3;	// Bump to start of next Message
	}
#pragma warn +pia

// Move past LFs and CRs that follow "^XX"
//
	while ((*Message=='\n') || (*Message=='\r'))
		Message++;

// Find the end of the message
//
	if (!(EndOfMsg = _fstrstr(Message,int_xx)))
		ACT1_ERROR(INVALID_CACHE_MSG_NUM);
	EndOfMsg += 3;

// Copy to a temp buffer
//
	while (Message != EndOfMsg)
	{
		if (*Message != '\n')
			hint_buffer[pos++] = *Message;

		if (pos >= MaxMsgLen)
			ACT1_ERROR(HINT_BUFFER_OVERFLOW);

		Message++;
	}

	hint_buffer[pos] = 0;		// Null Terminate
	UNCACHEGRCHUNK(SegNum);

	return(pos);
}


#if 0
//---------------------------------------------------------------------------
// CacheMsg()
//
// Caches the specific message in FROM a given 'grsegs' block TO the
// next available message segment pointer.
//---------------------------------------------------------------------------
void CacheMsg(mCacheInfo *ci, unsigned SegNum, unsigned MsgNum)
{
	char far *Message, far *EndOfMsg, far *hint_buffer;
	unsigned char pos=0;

// Alloc memory for message and cache-in seg
//
	MM_GetPtr(&ci->mSeg,MAX_CACHE_MSG_LEN);
	hint_buffer = ci->mSeg;

	CA_CacheGrChunk(SegNum);
	Message = grsegs[SegNum];

// Search for end of MsgNum-1 (Start of our message)
//
#pragma warn -pia
	while (--MsgNum)
	{
		if (!(Message = _fstrstr(Message,int_xx)))
			ACT1_ERROR(INVALID_CACHE_MSG_NUM);

		Message += 3;	// Bump to start of next Message
	}
#pragma warn +pia

// Move past LFs and CRs that follow "^XX"
//
	while ((*Message=='\n') || (*Message=='\r'))
		Message++;

// Find the end of the message
//
	if (!(EndOfMsg = _fstrstr(Message,int_xx)))
		ACT1_ERROR(INVALID_CACHE_MSG_NUM);
	EndOfMsg += 3;

// Copy to a temp buffer
//
	while (Message != EndOfMsg)
	{
		if (*Message != '\n')
			hint_buffer[pos++] = *Message;

		if (pos >= MAX_CACHE_MSG_LEN)
			ACT1_ERROR(HINT_BUFFER_OVERFLOW);

		Message++;
	}

	hint_buffer[pos] = 0;		// Null Terminate
	UNCACHEGRCHUNK(SegNum);
}
#endif
//


/*
=============================================================================

							CONCESSION MACHINES

=============================================================================
*/

//--------------------------------------------------------------------------
// SpawnConcession()
//
// actorat[][] - Holds concession machine number (1 - MAXCONCESSIONS+1)
//--------------------------------------------------------------------------
void SpawnConcession(int tilex, int tiley, unsigned credits,unsigned machinetype)
{
	con_mCacheInfo *ci=&ConHintList.cmInfo[ConHintList.NumMsgs];

	if (ConHintList.NumMsgs >= MAXCONCESSIONS)
		ACT1_ERROR(SPAWNCONCESSION_TOO_MANY);

	if (credits != PUSHABLETILE)
		switch (credits & 0xff00)
		{
			case 0:
#ifdef CON_HINTS
			case 0xFD00:											// Hint Id
				ci->mInfo.global_val = credits & 0xff;
				ci->mInfo.local_val = 0xff;
				ci->operate_cnt = 3+(US_RndT() & 0x03);
				if ((credits != 0xFDFF) && (credits))
					CacheConcessionMsg();
				ci->type = CT_HINT; 							 // Force to Hint Type
			break;
#endif

			case 0xFC00:											// Food Id
				ci->mInfo.local_val = credits & 0xff;
				ci->operate_cnt = 0;
				ci->type = machinetype;
			break;
		}

// Consider it a solid wall (val != 0)
//
	if (++ConHintList.NumMsgs > MAX_CACHE_MSGS)
		ACT1_ERROR(SPAWNCON_CACHE_MSG_OVERFLOW);
	(unsigned)actorat[tilex][tiley] = ConHintList.NumMsgs;

//
// BORLAND SCREWS UP WHEN COMPILING THE LINE BELOW, EVEN THOUGH
// IT SHOULD BE JUST THE SAME AS THE TWO LINES ABOVE...
//
//	(unsigned)actorat[tilex][tiley] = ++ConHintList.NumMsgs;
//
}

#ifdef CON_HINTS

//--------------------------------------------------------------------------
// CacheConcessionMsg()
//--------------------------------------------------------------------------
void CacheConcessionMsg()
{
	mCacheInfo *ci=(mCacheInfo far *)&ConHintList.cmInfo[ConHintList.NumMsgs];

// Make sure we don't overflow list.
//
	if (ConHintList.NumMsgs >= MAX_CACHE_MSGS)
		ACT1_ERROR(CACHEMSG_TOO_MANY);

// Either re-use a message, or cache-in a new one.
//
	if (!ReuseMsg(ci,ConHintList.NumMsgs,sizeof(con_mCacheInfo)))
	{
		// Cache-in new message
		//

		CacheMsg(ci,CONCESSION_HINTS,ci->global_val);
		ci->local_val = ConHintList.NumMsgs;
	}
}

#endif

//--------------------------------------------------------------------------
// ReuseMsg()
//--------------------------------------------------------------------------
boolean ReuseMsg(mCacheInfo *ci, short count, short struct_size)
{
	char *scan_ch=(char *)ci;
	mCacheInfo *scan=(mCacheInfo *)(scan_ch-struct_size);

// Scan through all loaded messages -- see if we're loading one already
// cached-in.
//
	while (count--)
	{
	// Is this message already cached in?
	//
		if (scan->global_val == ci->global_val)
		{
			ci->local_val = scan->local_val;
			return(true);
		}

	// Funky structure decrement... (structures can be any size...)
	//
		scan_ch=(char *)scan;
		scan_ch-=struct_size;
		scan=(mCacheInfo *)scan_ch;
	}

	return(false);
}

//--------------------------------------------------------------------------
// OperateConcession()
//--------------------------------------------------------------------------

extern char far food_msg1[];
extern char far bevs_msg1[];
extern void writeTokenStr(char far *str);

char far OutOrder[] 	= {"\r\r   FOOD UNIT MACHINE\r    IS OUT OF ORDER.^XX"};

void OperateConcession(unsigned concession)
{
	con_mCacheInfo *ci;
	char far *msgptr;
	boolean ok=false;

	ci=&ConHintList.cmInfo[concession-1];

	switch (ci->type)
	{
#ifdef CON_HINTS
		case CT_HINT:
			if (ci->operate_cnt != 0xff)
				if (!ci->operate_cnt--)
					ok=true;
		break;
#endif

		case CT_FOOD:
		case CT_BEVS:
			if (ci->mInfo.local_val)
			{
				if (gamestate.health == 100)
				{
					DISPLAY_TIMED_MSG(noeat_msg1,MP_CONCESSION_OPERATE,MT_GENERAL);
               SD_PlaySound(NOWAYSND);
					return;
				}
				else
					ok=true;
			}
		break;
	}

	if (ok)
	{
		// Whada' ya' want?

		switch (ci->type)
		{
#ifdef CON_HINTS
			case CT_HINT:
				SD_PlaySound(CON_HINTSND);
				if (ci->mInfo.local_val == 0xFF)
					DISPLAY_TIMED_MSG(ConcessionGenHints[US_RndT() % NUM_GEN_HINTS],MP_CONCESSION_OPERATE,MT_GENERAL);
				else
				{
					msgptr = ConHintList.cmInfo[ci->mInfo.local_val].mInfo.mSeg;
					DISPLAY_TIMED_MSG(msgptr,MP_CONCESSION_HINT,MT_GENERAL);
				}
				ci->mInfo.local_val = 0;				// Mark as Out Of Order
			break;
#endif
			case CT_FOOD:
			case CT_BEVS:
				// One token please... Thank you.

				if (!gamestate.tokens)
				{
					DISPLAY_TIMED_MSG(NoFoodTokens,MP_NO_MORE_TOKENS,MT_NO_MO_FOOD_TOKENS);
               SD_PlaySound(NOWAYSND);
					return;
				}
				else
					gamestate.tokens--;

				ci->mInfo.local_val--;
				SD_PlaySound(CONCESSIONSSND);
				switch (ci->type)
				{
					case CT_FOOD:
						writeTokenStr(food_msg1);
						DISPLAY_TIMED_MSG(food_msg1,MP_CONCESSION_OPERATE,MT_GENERAL);
						HealSelf(10);
					break;

					case CT_BEVS:
						writeTokenStr(bevs_msg1);
						DISPLAY_TIMED_MSG(bevs_msg1,MP_CONCESSION_OPERATE,MT_GENERAL);
						HealSelf(7);
					break;
				}
			break;
		}
	}
	else
	{
		DISPLAY_TIMED_MSG(OutOrder,MP_CONCESSION_OUT_ORDER,MT_GENERAL);
		SD_PlaySound(NOWAYSND);
	}
}

char xy_offset[8][2]={{0,-1},{0,+1},{-1,0},{+1,0},				// vert / horz
							 {-1,-1},{+1,+1},{-1,+1},{+1,-1}};		// diagnals

//--------------------------------------------------------------------------
// CheckSpawnEA()
//--------------------------------------------------------------------------
void CheckSpawnEA()
{
	objtype temp,*ob;
	char loop,ofs,tx,ty,x_diff,y_diff;

	if (objcount > MAXACTORS-8)
		return;

	for (loop=0; loop<NumEAWalls; loop++)
	{
//		unsigned far *map=mapsegs[0]+farmapylookup[eaList[loop].tiley]+eaList[loop].tilex;
		unsigned far *map1=mapsegs[1]+farmapylookup[eaList[loop].tiley]+eaList[loop].tilex;

	// Limit the number of aliens spawned by each outlet.
	//
		if (eaList[loop].aliens_out > gamestate.difficulty)
			continue;

	// Decrement 'spawn delay' for current outlet.
	//
		if (eaList[loop].delay > tics)
		{
			eaList[loop].delay -= tics;
			continue;
		}

	// Reset to 1 because it's possible that an alien won't be spawned...
	// If NOT, we'll try again on the next refresh.
	// If SO, the delay is set to a true value below.
	//
		eaList[loop].delay=1;

	// Does this wall touch the 'area' that the player is in?
	//
		for (ofs=0; ofs<4; ofs++)
		{
			char nx=eaList[loop].tilex+xy_offset[ofs][0];
			char ny=eaList[loop].tiley+xy_offset[ofs][1];
			char areanumber=GetAreaNumber(nx,ny);

			if ((nx < 0) || (nx > 63) || (ny < 0) || (ny > 63))
				continue;

			if (areanumber != 127 && areabyplayer[areanumber])
				break;
		}

	// Wall doesn't touch player 'area'.
	//
		if (ofs==4)
			continue;

	// Setup tile x,y in temp obj.
	//
		temp.tilex = eaList[loop].tilex+xy_offset[ofs][0];
		temp.tiley = eaList[loop].tiley+xy_offset[ofs][1];

	// Is another actor already on this tile?
	// If so, "continue" if it's alive...
	//
		ob = actorat[temp.tilex][temp.tiley];
		if (ob >= objlist)
			if (!(ob->flags & FL_DEADGUY))
				continue;

	// Is player already on this tile?
	//
		x_diff = player->tilex-temp.tilex;
		y_diff = player->tiley-temp.tiley;
		if (ABS(x_diff)<2 && ABS(y_diff)<2)
			continue;

	// Setup x,y in temp obj and see if obj is in player's view.
	// Actor is released if it's in player's view   OR
	// a random chance to release whether it can be seen or not.
	//
		temp.x = ((fixed)temp.tilex<<TILESHIFT)+((fixed)TILEGLOBAL/2);
		temp.y = ((fixed)temp.tiley<<TILESHIFT)+((fixed)TILEGLOBAL/2);
		if ((!CheckSight(player,&temp)) && (US_RndT() < 200))
			continue;

	// Spawn Electro-Alien!
	//
		usedummy=true;
		SpawnStand(en_electro_alien,temp.tilex,temp.tiley,0);
		SD_PlaySound(ELECAPPEARSND);
		usedummy=false;
		if (new!=&dummyobj)
		{
			eaList[loop].aliens_out++;
			new->temp2=loop;
			PlaySoundLocActor(ELECAPPEARSND,new);
		}

	// Reset spawn delay.
	//
		if ((*map1 & 0xff00) == 0xfa00)
			eaList[loop].delay=60*((*map1)&0xff);
		else
			eaList[loop].delay=60*8+Random(60*22);

		break;
	}
}

//--------------------------------------------------------------------------
// CheckSpawnGoldstern()
//--------------------------------------------------------------------------
void CheckSpawnGoldstern(void)
{
	if (GoldsternInfo.WaitTime > tics)
	{
		//
		// Count down general timer before doing any Goldie Stuff..
		//

		GoldsternInfo.WaitTime -=tics;
	}
	else
	{
		//
		// What Kind of Goldie Stuff needs to be done?
		//

      if (GoldsternInfo.flags == GS_COORDFOUND)
      {
      	unsigned tilex,tiley;

         // See if we can spawn Dr. Goldstern...

         tilex = GoldieList[GoldsternInfo.LastIndex].tilex;
			tiley = GoldieList[GoldsternInfo.LastIndex].tiley;

			if ((!actorat[tilex][tiley]) && ABS(player->tilex-tilex)>1 && ABS(player->tiley-tiley)>1 )
			{
				SpawnStand(en_goldstern, tilex, tiley, 0);
				GoldsternInfo.GoldSpawned = true;
         }
		}
      else
		{
			// Find a new coord to spawn Goldie (GS_NEEDCOORD or GS_FIRSTTIME)

			FindNewGoldieSpawnSite();
		}
	}
}

//---------------------------------------------------------------------------
// FindNewGoldieSpawnSite()
//---------------------------------------------------------------------------
void FindNewGoldieSpawnSite(void)
{
	objtype temp;
	char loop;

	GoldsternInfo.WaitTime = 0;

	for (loop=0; loop<GoldsternInfo.SpawnCnt; loop++)
	{
		// Test for repeats - And avoid them!
		//

      if ((GoldsternInfo.SpawnCnt>1) && (loop == GoldsternInfo.LastIndex))
			continue;

		// Setup tile x,y in temp obj.
		//

		temp.tilex = GoldieList[loop].tilex;
		temp.tiley = GoldieList[loop].tiley;

		// Setup x,y in temp obj and see if obj is in player's view.
		//

		temp.x = ((fixed)temp.tilex<<TILESHIFT)+((fixed)TILEGLOBAL/2);
		temp.y = ((fixed)temp.tiley<<TILESHIFT)+((fixed)TILEGLOBAL/2);
		if (!CheckSight(player,&temp))
			continue;

		// Mark to spawn Dr Goldstern
		//

		GoldsternInfo.LastIndex = loop;
		if (gamestate.mapon==9)
			GoldsternInfo.WaitTime = 60;
		else
		if (GoldsternInfo.flags == GS_FIRSTTIME)
			GoldsternInfo.WaitTime = MIN_GOLDIE_FIRST_WAIT + Random(MAX_GOLDIE_FIRST_WAIT-MIN_GOLDIE_FIRST_WAIT);			// Reinit Delay Timer before spawning on new position
      else
			GoldsternInfo.WaitTime = MIN_GOLDIE_WAIT + Random(MAX_GOLDIE_WAIT-MIN_GOLDIE_WAIT);			// Reinit Delay Timer before spawning on new position

      GoldsternInfo.flags = GS_COORDFOUND;
		break;
	}
}

