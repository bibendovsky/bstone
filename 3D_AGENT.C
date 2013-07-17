// 3D_AGENT.C

#include "3D_DEF.H"
#pragma hdrstop

#include <ctype.h>

#include "jm_tp.h"

void InitWeaponBounce(void);
void HandleWeaponBounce(void);

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

//#define ACTIVATE_TERMINAL

#define MAXMOUSETURN			10

#define MOVESCALE				150l
#define BACKMOVESCALE		100l
#define ANGLESCALE			20
#define MAX_DA 				100

#define MAX_TERM_COMMAND_LEN	31

// Max Score displayable

#define MAX_DISPLAY_SCORE 			(9999999L)
#define SCORE_ROLL_WAIT				(60*10)			// Tics

// IFDEF switches

//#define NO_STATUS_BAR

// ECG scroll rate (delay).

#define HEALTH_SCROLL_RATE		7
#define HEALTH_PULSE				70

// Text "InfoArea" defines
#define INFOAREA_X				3
#define INFOAREA_Y				((unsigned)200-STATUSLINES+3)
#define INFOAREA_W				109
#define INFOAREA_H				37

#define INFOAREA_BCOLOR			0x01
#define INFOAREA_CCOLOR    	0x1A
#define INFOAREA_TCOLOR			0xA6
#define INFOAREA_TSHAD_COLOR	0x04			// Text Shadow Color

#define GRENADE_ENERGY_USE		4
#define BFG_ENERGY_USE			(GRENADE_ENERGY_USE<<1)


#define NUM_AMMO_SEGS			21


#define AMMO_SMALL_FONT_NUM_WIDTH	5

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

extern boolean noShots;
extern short bounceOk;

short tryDetonatorDelay=0;

//
// player state info
//
long		thrustspeed;

//unsigned	plux,pluy;			// player coordinates scaled to unsigned

int			anglefrac;

objtype		*LastAttacker;

boolean 	PlayerInvisable = false;

char LocationText[MAX_LOCATION_DESC_LEN];

#ifdef ACTIVATE_TERMINAL
char term_com_name[13]= {"TERM_CMD."};
char term_msg_name[13]= {"TERM_MSG."};
#endif

unsigned player_oldtilex;
unsigned player_oldtiley;


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void writeTokenStr(char far *str);

void ShowOverheadChunk(void);
void LoadOverheadChunk(short tpNum);
void SaveOverheadChunk(short tpNum);
void DisplayTeleportName(char tpNum, boolean locked);

void ForceUpdateStatusBar(void);

void UpdateRadarGuage(void);
void DrawLedStrip(short x,short y,short frac,short max);
void DisplayPinballBonus(void);
void CheckPinballBonus(long points);
byte LevelCompleted(void);
void	T_Player (objtype *ob);
void	T_Attack (objtype *ob);

statetype s_player = {0,0,0,&T_Player,NULL,NULL};
statetype s_attack = {0,0,0,&T_Attack,NULL,NULL};

long	playerxmove,playerymove;

atkinf_t far attackinfo[7][14] =

{
{ {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },     // Auto charge
{ {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },     // Pistol
{ {6,0,1},{6,1,2},{5,3,3},{5,-1,4} },		// Pulse
{ {6,0,1},{6,1,2},{3,4,3},{3,-1,4} },		// ION
{ {6,0,1},{6,5,2},{6,6,3},{6,-1,4} },
{ {6,0,1},{6,9,2},{6,10,3},{6,-1,4} },
{ {5,7,0},{5,8,0},{2,-2,0},{0,0,0} },
};

//int	strafeangle[9] = {0,90,180,270,45,135,225,315,0};

#define GD0 0x55
#define YD0 0x35
#define RD0 0x15

#define GD1 0x53
#define YD1 0x33
#define RD1 0x13

char  far DimAmmo[2][22] = {{GD0,GD0,GD0,GD0,GD0,GD0,GD0,YD0,YD0,YD0,YD0,YD0,YD0,YD0,RD0,RD0,RD0,RD0,RD0,RD0,RD0,RD0},
									 {GD1,GD1,GD1,GD1,GD1,GD1,GD1,YD1,YD1,YD1,YD1,YD1,YD1,YD1,RD1,RD1,RD1,RD1,RD1,RD1,RD1,RD1}};

#define GL0 0x58
#define YL0 0x38
#define RL0 0x18

#define GL1 0x56
#define YL1 0x36
#define RL1 0x16

char  far LitAmmo[2][22] = {{GL0,GL0,GL0,GL0,GL0,GL0,GL0,YL0,YL0,YL0,YL0,YL0,YL0,YL0,RL0,RL0,RL0,RL0,RL0,RL0,RL0,RL0},
									 {GL1,GL1,GL1,GL1,GL1,GL1,GL1,YL1,YL1,YL1,YL1,YL1,YL1,YL1,RL1,RL1,RL1,RL1,RL1,RL1,RL1,RL1}};


#define IA_MAX_LINE			30
typedef struct InfoArea_Struct
{
	int	x,y;
	int	text_color;
	int	backgr_color;
	int 	left_margin;
	char	delay;
	char numanims;
	char framecount;
} InfoArea_Struct;

unsigned LastMsgPri = 0;
short MsgTicsRemain = 0;
classtype LastInfoAttacker = nothing;
int LastInfoAttacker_Cloaked = 0;
infomsg_type LastMsgType = MT_NOTHING;
InfoArea_Struct InfoAreaSetup;

char DrawRadarGuage_COUNT = 3;
char DrawAmmoNum_COUNT = 3;
char DrawAmmoPic_COUNT = 3;
//char DrawPDAmmoPic_COUNT = 3;
char DrawScoreNum_COUNT = 3;
char DrawWeaponPic_COUNT = 3;
char DrawKeyPics_COUNT = 3;
char DrawHealthNum_COUNT = 3;

char DrawInfoArea_COUNT = 3;
char InitInfoArea_COUNT = 3;
char ClearInfoArea_COUNT = 3;

void DrawWeapon (void);
void GiveWeapon (int weapon);
void	GiveAmmo (int ammo);
void DrawGAmmoNum(void);
void DrawMAmmoNum(void);
void DrawPDAmmoMsg(void);
void ComputeAvailWeapons(void);
void SW_HandleActor(objtype *obj);
void SW_HandleStatic(statobj_t *stat,unsigned tilex, unsigned tiley);

//===========================================================================

//----------

byte ShowRatio(short bx, short by, short px, short py, long total, long perc, ss_type type);
void Attack (void);
void Use (void);
void Search (objtype *ob);
void SelectWeapon (void);
void SelectItem (void);

//----------

void SpawnPlayer (int tilex, int tiley, int dir);
void Thrust (int angle, long speed);
boolean TryMove (objtype *ob);
void T_Player (objtype *ob);

boolean ClipMove (objtype *ob, long xmove, long ymove);

void SocketToggle(boolean TurnOn);
void CheckStatsBonus(void);

void	T_Stand (objtype *ob);

/*
=============================================================================

						CONTROL STUFF

=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-6 change weapons
=
=
======================
*/


void CheckWeaponChange (void)
{
	int	i,buttons,last;

	for (i=wp_autocharge;i<=wp_bfg_cannon;i++)
	{
		if (buttonstate[bt_ready_autocharge+i-wp_autocharge])
		{
			if (gamestate.useable_weapons & (1<<i))
			{
				gamestate.weapon = gamestate.chosenweapon = i;

				DISPLAY_TIMED_MSG(WeaponAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
				DrawWeapon();
				return;
			}
			else
				DISPLAY_TIMED_MSG(WeaponNotAvailMsg, MP_WEAPON_AVAIL, MT_GENERAL);
		}
	}
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement (objtype *ob)
{
	long	oldx,oldy;
	int		angle,maxxmove;
	int		angleunits;
	long	speed;

	thrustspeed = 0;

	oldx = player->x;
	oldy = player->y;

//
// side to side move
//

	if ((buttonstate[bt_strafe]) && !(gamestate.turn_around))
	{
	//
	// strafing
	//
	//
		if (controlx > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,controlx*MOVESCALE);	// move to left
		}
		else if (controlx < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-controlx*MOVESCALE);	// move to right
		}
	}
	else
	{
		if (gamestate.turn_around)
		{
			controlx = 100*tics;
			if (gamestate.turn_around < 0)
				controlx = -controlx;
		}

	//
	// not strafing
	//
		anglefrac += controlx;
		angleunits = anglefrac/ANGLESCALE;
		anglefrac -= angleunits*ANGLESCALE;
		ob->angle -= angleunits;

		if (ob->angle >= ANGLES)
			ob->angle -= ANGLES;
		if (ob->angle < 0)
			ob->angle += ANGLES;

		if (gamestate.turn_around)
		{
			boolean done=false;

			if (gamestate.turn_around > 0)
			{
				gamestate.turn_around -= angleunits;
				if (gamestate.turn_around <= 0)
					done=true;
			}
			else
			{
				gamestate.turn_around -= angleunits;
				if (gamestate.turn_around >= 0)
					done=true;
			}

			if (done)
			{
				gamestate.turn_around=0;
				ob->angle = gamestate.turn_angle;
			}
		}
	}


//
// forward/backwards move
//
	if (controly < 0)
	{
		Thrust (ob->angle,-controly*MOVESCALE);	// move forwards
	}
	else if (controly > 0)
	{
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,controly*BACKMOVESCALE);		// move backwards
	}
	else
		if (bounceOk)
			bounceOk--;

	if (controly)
		bounceOk = 8;
	else
		if (bounceOk)
			bounceOk--;

	ob->dir = ((ob->angle + 22) % 360)/45;

//
// calculate total move
//
	playerxmove = player->x - oldx;
	playerymove = player->y - oldy;
}

/*
=============================================================================

					STATUS WINDOW STUFF

=============================================================================
*/

#define STATUSDRAWPIC(x, y, picnum)			JLatchDrawPic((x),(y+(200-STATUSLINES)),(picnum))


/*
==================
=
= StatusDrawPic
=
==================
*/
void StatusAllDrawPic(unsigned x, unsigned y, unsigned picnum)
{
	unsigned	temp;

#ifdef PAGEFLIP

	temp = bufferofs;
	bufferofs = PAGE1START+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = PAGE2START+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = PAGE3START+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = temp;

#else

	temp = bufferofs;
	bufferofs = screenloc[0]+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = screenloc[1]+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = screenloc[2]+(200-STATUSLINES)*SCREENWIDTH;
	JLatchDrawPic (x,y,picnum);
	bufferofs = temp;

#endif

}


void JLatchDrawPic (unsigned x, unsigned y, unsigned picnum)
{
	unsigned wide, height, source;

	x <<= 3;
	wide = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;
	source = latchpics[2+picnum-LATCHPICS_LUMP_START];
	VL_LatchToScreen (source,wide/4,height,x,y);
}



/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/
void	LatchNumber (int x, int y, int width, long number)
{
	unsigned	length,wide=0,c;
	char	str[20];

	ltoa(number,str,10);

	length =	strlen(str);

	while ((length<width) && (wide < width))
	{
		STATUSDRAWPIC(x,y,N_BLANKPIC);
		x++;
		wide++;
		length++;
	}

	c = 0;

	while (wide<width)
	{
		STATUSDRAWPIC (x,y,str[c]-'0'+ N_0PIC);
		x++;
		c++;
		wide++;
	}
}




//===========================================================================
//
//
//								SCORE DISPLAY ROUTINES
//
//
//===========================================================================




//--------------------------------------------------------------------------
// DrawHealth()
//
// PURPOSE : Marks the Health_NUM to be refreshed durring the next
//				 StatusBarRefresh.
//--------------------------------------------------------------------------
void DrawHealth (void)
{
	char *ptr = gamestate.health_str;

	itoa(gamestate.health,gamestate.health_str,10);
	while (*ptr)
		*ptr++ -= '0';

	DrawHealthNum_COUNT=3;
}

//--------------------------------------------------------------------------
// DrawHealthNum()
//--------------------------------------------------------------------------
void DrawHealthNum(void)
{
	char loop,num;
	short check=100;

	DrawHealthNum_COUNT--;

	for (loop=num=0; loop<3; loop++,check /= 10)
		if (gamestate.health < check)
			JLatchDrawPic(16+loop,162,NG_BLANKPIC);
		else
			JLatchDrawPic(16+loop,162,gamestate.health_str[num++]+NG_0PIC);
}

//---------------------------------------------------------------------------
// TakeDamage()
//---------------------------------------------------------------------------
void	TakeDamage (int points, objtype *attacker)
{
	LastAttacker = attacker;

	if (gamestate.flags & GS_ATTACK_INFOAREA)
		if (attacker)
		{
			if ((LastMsgType == MT_ATTACK) && (LastInfoAttacker == attacker->obclass))
				MsgTicsRemain = DISPLAY_MSG_STD_TIME;
			else
			{
				if (DISPLAY_TIMED_MSG(ActorInfoMsg[attacker->obclass-rentacopobj],MP_TAKE_DAMAGE,MT_ATTACK))
            {
					LastInfoAttacker = attacker->obclass;
               LastInfoAttacker_Cloaked = attacker->flags2 & FL2_CLOAKED;
            }
			}
		}

	if (godmode)
		return;

	if (gamestate.difficulty==gd_baby)
	  points>>=2;

	gamestate.health -= points;

	if (gamestate.health<=0)
	{
		gamestate.health = 0;
		playstate = ex_died;
		killerobj = attacker;
		if (killerobj)
			killerobj->flags |= FL_FREEZE;
	}

	StartDamageFlash (points);
	DrawHealth();
}

//---------------------------------------------------------------------------
// HealSelf()
//---------------------------------------------------------------------------
void	HealSelf(int points)
{
	gamestate.health += points;
	if (gamestate.health > 100)
		gamestate.health = 100;

	DrawHealth ();
}




//===========================================================================
//
//
//								SCORE DISPLAY ROUTINES
//
//
//===========================================================================

//--------------------------------------------------------------------------
// DrawScore()
//
// PURPOSE : Marks the Score to be refreshed durring the next
//				 StatusBarRefresh.
//--------------------------------------------------------------------------
void	DrawScore(void)
{
	DrawScoreNum_COUNT = 3;
}

extern unsigned char music_num;

//--------------------------------------------------------------------------
// DrawScoreNum()
//
// NOTE : Could do some sort of "scrolling" animation on LED screen with
//			 chars and a simple table.....
//--------------------------------------------------------------------------
void	DrawScoreNum(void)
{
	#define Y	3
	#define X	32

	if (gamestate.tic_score > MAX_DISPLAY_SCORE)
	{
		if (gamestate.score_roll_wait)
		{
			JLatchDrawPic(X+0,(200-STATUSLINES)+Y,N_BLANKPIC);
			JLatchDrawPic(X+1,(200-STATUSLINES)+Y,N_DASHPIC);
			JLatchDrawPic(X+2,(200-STATUSLINES)+Y,N_RPIC);
			JLatchDrawPic(X+3,(200-STATUSLINES)+Y,N_OPIC);
			JLatchDrawPic(X+4,(200-STATUSLINES)+Y,N_LPIC);
			JLatchDrawPic(X+5,(200-STATUSLINES)+Y,N_LPIC);
			JLatchDrawPic(X+6,(200-STATUSLINES)+Y,N_DASHPIC);
		}
		else
		{
			LatchNumber(X,Y,7,gamestate.tic_score%(MAX_DISPLAY_SCORE+1));
		}
	}
	else
	{
		if (gamestate.flags & GS_TICS_FOR_SCORE)
			LatchNumber(X,Y,7,realtics);
		else
		if (gamestate.flags & GS_MUSIC_TEST)
			LatchNumber(X,Y,7,music_num);
		else
			LatchNumber(X,Y,7,gamestate.tic_score);
	}
}

//--------------------------------------------------------------------------
// UpdateScore()
//--------------------------------------------------------------------------
void UpdateScore(void)
{
	long score_diff, temp_tics;
	boolean RollSound;

	score_diff = gamestate.score - gamestate.tic_score;

	if (score_diff)
	{
		if (score_diff > 1500)
			temp_tics = score_diff>>2;
		else
			temp_tics = tics<<3;

		if (score_diff > temp_tics)
			gamestate.tic_score += temp_tics;
		else
			gamestate.tic_score = gamestate.score;

		DrawScore();
	}


	if (gamestate.score_roll_wait)
	{
		if ((gamestate.score_roll_wait-=tics) <= 0)
		{
			gamestate.score_roll_wait = 0;
		}
		DrawScore();
	}
}

//--------------------------------------------------------------------------
// GivePoints()
//
// .score 		= Holds real score
// .tic_score  = Holds displayed score (tic'ing toward .score)
//
//--------------------------------------------------------------------------
void GivePoints(long points,boolean add_to_stats)
{
// Add score to statistics.
//
	if (add_to_stats)
		gamestuff.level[gamestate.mapon].stats.accum_points += points;

// Check for bonuses!
//
	CheckPinballBonus(points);

// Add points to score
//
	gamestate.score += points;
}

//===========================================================================
//
//
//                      SECURITY KEY DISPLAY ROUTINES
//
//
//===========================================================================



//---------------------------------------------------------------------------
// DrawKeys()
//
// PURPOSE : Marks the security key pics to be refreshed during the next
//				 StatusBarRefresh.
//---------------------------------------------------------------------------
void DrawKeys (void)
{
	DrawKeyPics_COUNT = 3;
}

//---------------------------------------------------------------------------
// DrawKeyPics()
//---------------------------------------------------------------------------
void DrawKeyPics(void)
{
	char loop;

	DrawKeyPics_COUNT--;

	for (loop=0; loop<NUMKEYS; loop++)
		if (gamestate.numkeys[loop])
			JLatchDrawPic(15+2*loop,179,RED_KEYPIC+loop);
		else
			JLatchDrawPic(15+2*loop,179,NO_KEYPIC);
}

//---------------------------------------------------------------------------
// GiveKey
//---------------------------------------------------------------------------
void GiveKey (int key)
{
	gamestate.numkeys[key]++;
	DrawKeys();
}

//---------------------------------------------------------------------------
// TakeKey
//---------------------------------------------------------------------------
void TakeKey (int key)
{
	gamestate.numkeys[key]--;
	DrawKeys();
}


//===========================================================================
//
//
//                      	WEAPON DISPLAY ROUTINES
//
//
//===========================================================================


//---------------------------------------------------------------------------
// DrawWeapon()
//
// PURPOSE : Marks the Weapon pics to be refreshed durring the next
//				 StatusBarRefresh.
//---------------------------------------------------------------------------
void DrawWeapon(void)
{
	DrawWeaponPic_COUNT=3;
	DrawAmmo(true);
}

//---------------------------------------------------------------------------
// DrawWeaponPic()
//---------------------------------------------------------------------------
void DrawWeaponPic(void)
{
	if (gamestate.weapon == -1)
		return;

	JLatchDrawPic(31,176,WEAPON1PIC+gamestate.weapon);

	DrawWeaponPic_COUNT--;
}

//---------------------------------------------------------------------------
// GiveWeapon()
//---------------------------------------------------------------------------
void GiveWeapon (int weapon)
{
	GiveAmmo (6);

	if (!(gamestate.weapons & (1<<weapon)))
	{
		gamestate.weapons |= (1<<weapon);

		if (gamestate.weapon < weapon)
		{
			gamestate.weapon = gamestate.chosenweapon = weapon;
			DrawWeapon();
		}
	}
}

//===========================================================================
//
//
//                      	AMMO DISPLAY ROUTINES
//
//
//===========================================================================

//---------------------------------------------------------------------------
// DrawAmmo()
//
// PURPOSE : Marks the AMMO NUM & AMMO PIC (if necessary) to be refreshed
//				 durring the next StatusBarRefresh.
//
// NOTE : This re-computes the number of LEDs to be lit.
//---------------------------------------------------------------------------
void DrawAmmo(boolean ForceRefresh)
{
	int temp;
   unsigned ammo,max_ammo;

   ComputeAvailWeapons();

   //
   // Which weapon are we needing a refresh for?
   //

	switch (gamestate.weapon)
	{
//      case wp_plasma_detonators:
//			DrawAmmoPic_COUNT = 3;
//			DrawAmmoNum_COUNT = 0;
//		return;

		case wp_autocharge:
			DrawAmmoPic_COUNT = 3;
			DrawAmmoNum_COUNT = 0;
		return;

      default:
      	ammo = gamestate.ammo;
         max_ammo = MAX_AMMO;
      break;
	}

	if (ammo)
	{
		temp = (ammo*NUM_AMMO_SEGS)/max_ammo;
		if (!temp)
			temp = 1;
	}
	else
		temp = 0;

	gamestate.ammo_leds = temp;

	if ((temp != gamestate.lastammo_leds) || ForceRefresh)
	{
		gamestate.lastammo_leds = temp;
		DrawAmmoPic_COUNT = 3;
	}

	DrawAmmoNum_COUNT=3;
}



//---------------------------------------------------------------------------
// DrawAmmoNum()
//---------------------------------------------------------------------------
void DrawAmmoNum(void)
{
	if (gamestate.weapon == -1)
		return;

	fontnumber = 2;
	fontcolor = 0x9D;

	PrintX = 252;
	PrintY = 200-STATUSLINES+38;

#if 0
	switch (gamestate.weapon)
	{
		case wp_plasma_detonators:
		case wp_autocharge:
      break;

      default:
		   DrawGAmmoNum();
      break;
	}
#else

   DrawGAmmoNum();

#endif

	DrawAmmoNum_COUNT--;
}



//---------------------------------------------------------------------------
// DrawGAmmoNum()
//---------------------------------------------------------------------------
void DrawGAmmoNum(void)
{
	char	buffer[32];

	if (gamestate.ammo <100)
	{
		PrintX+=AMMO_SMALL_FONT_NUM_WIDTH;
		if (gamestate.ammo <10)
			PrintX+=AMMO_SMALL_FONT_NUM_WIDTH;
	}

	JLatchDrawPic(31,184,W1_CORNERPIC+gamestate.weapon);

   px = PrintX;
   py = PrintY;
	VW_DrawPropString(ultoa(gamestate.ammo,buffer,10));
	VW_DrawPropString("%");
}

//---------------------------------------------------------------------------
// DrawAmmoPic()
//---------------------------------------------------------------------------
void DrawAmmoPic(void)
{
	switch (gamestate.weapon)
	{
		case wp_autocharge:
			DrawAmmoMsg();
      break;

//		case wp_plasma_detonators:
//			DrawPDAmmoMsg();
//     break;

      default:
			DrawAmmoGuage();
      break;
   }

	DrawAmmoPic_COUNT--;
}

//---------------------------------------------------------------------------
// DrawAmmoMsg() -
//---------------------------------------------------------------------------
void DrawAmmoMsg(void)
{
	if (gamestate.weapon_wait)
		JLatchDrawPic(30,(200-STATUSLINES),WAITPIC);
	else
		JLatchDrawPic(30,(200-STATUSLINES),READYPIC);
}

//---------------------------------------------------------------------------
// DrawPDAmmoMsg() -
//---------------------------------------------------------------------------
void DrawPDAmmoMsg(void)
{
	if (gamestate.plasma_detonators)
		JLatchDrawPic(30,(200-STATUSLINES),READYPIC);
	else
		JLatchDrawPic(30,(200-STATUSLINES),WAITPIC);
}


//---------------------------------------------------------------------------
// UpdateAmmoMsg() -
//---------------------------------------------------------------------------
void UpdateAmmoMsg(void)
{
	if (gamestate.weapon_wait)
		if ((gamestate.weapon_wait -= tics)<=0)
		{
			gamestate.weapon_wait = 0;
			DrawAmmoPic_COUNT = 3;
		}
}

//---------------------------------------------------------------------------
// DrawAmmoGuage()
//---------------------------------------------------------------------------
void DrawAmmoGuage(void)
{
	DrawLedStrip(243,155,gamestate.ammo_leds,NUM_AMMO_SEGS);
}

//---------------------------------------------------------------------------
// UpdateRadarGuage()
//---------------------------------------------------------------------------
void UpdateRadarGuage(void)
{
	int temp;

	if (gamestate.rpower)
	{
		temp = ((long)gamestate.rpower*NUM_AMMO_SEGS)/MAX_RADAR_ENERGY;

		if (temp > NUM_AMMO_SEGS)
			temp = NUM_AMMO_SEGS;

		if (!temp)
			temp = 1;
	}
	else
		temp = 0;

	gamestate.radar_leds = temp;

	if (temp != gamestate.lastradar_leds)
		gamestate.lastradar_leds = temp;

	DrawRadarGuage_COUNT=3;
}

//---------------------------------------------------------------------------
// DrawRadarGuage()
//---------------------------------------------------------------------------
void DrawRadarGuage(void)
{
	char zoom;

	DrawLedStrip(235,155,gamestate.radar_leds,NUM_AMMO_SEGS);

	if (gamestate.rpower)
		zoom = gamestate.rzoom;
	else
		zoom = 0;

	JLatchDrawPic(22,152,ONEXZOOMPIC+zoom);
}

//---------------------------------------------------------------------------
// DrawLedStrip()
//---------------------------------------------------------------------------
void DrawLedStrip(short x,short y,short frac,short max)
{
	int ypos;
	unsigned amount;
	char leds;

	leds = frac;

	if (leds)
		amount = max-leds;
	else
		amount = max;

// Draw dim LEDs.
//
	for (ypos = 0;ypos < amount;ypos++)
	{
		VW_Hlin (x,x+4,y++,DimAmmo[0][amount]);
		VW_Hlin (x,x+4,y++,DimAmmo[1][amount]);
	}

// Draw lit LEDs.
//
	for (;ypos<NUM_AMMO_SEGS;ypos++)
	{
		VW_Hlin (x,x+4,y++,LitAmmo[0][amount]);
		VW_Hlin (x,x+4,y++,LitAmmo[1][amount]);
	}
}


//---------------------------------------------------------------------------
// GiveAmmo()
//---------------------------------------------------------------------------
void	GiveAmmo (int ammo)
{

#if MP_NO_MORE_AMMO > MP_BONUS
	if (LastMsgType == MT_OUT_OF_AMMO)
	{
		MsgTicsRemain = 1;
		LastMsgType = MT_CLEAR;
	}
#endif

	gamestate.ammo += ammo;
	if (gamestate.ammo > MAX_AMMO)
	{
		gamestate.ammo = MAX_AMMO;
	}

	DrawAmmo(false);

   if (gamestate.weapon != gamestate.chosenweapon)
	{
	   if (gamestate.useable_weapons & (1<<gamestate.chosenweapon))
		{
			gamestate.weapon = gamestate.chosenweapon;
			DrawWeapon ();
     	}
	}

	SD_PlaySound (GETAMMOSND);


#if 0
#if MP_NO_MORE_AMMO > MP_BONUS
	if (LastMsgType == MT_OUT_OF_AMMO)
	{
		MsgTicsRemain = 1;
		LastMsgType = MT_CLEAR;
	}
#endif

	gamestate.ammo += ammo;
	if (gamestate.ammo > MAX_AMMO)
	{
		gamestate.ammo = MAX_AMMO;
	}

   // JIM - This needs to be optomized.

   if (gamestate.weapon != gamestate.chosenweapon)
	{
	   if (!((gamestate.chosenweapon == wp_grenade) && (gamestate.ammo < GRENADE_ENERGY_USE)) ||
		    !((gamestate.chosenweapon == wp_bfg_cannon) && (gamestate.ammo < BFG_ENERGY_USE)))
		{
			gamestate.weapon = gamestate.chosenweapon;
			DrawWeapon ();
     	}
	}

	DrawAmmo(false);
	SD_PlaySound (GETAMMOSND);
#endif
}


//---------------------------------------------------------------------------
//ComputeAvailWeapons()
//
// This function creates a Bit MASK for gamestate.weapons according to what
// weapon is available for useage due to ammo avail.
//
//---------------------------------------------------------------------------
void ComputeAvailWeapons(void)
{

   //
   // Determine what ammo ammounts we have avail
   //

   if (gamestate.ammo)
   {
	   if (gamestate.ammo >= BFG_ENERGY_USE)
			gamestate.useable_weapons = (1<<wp_bfg_cannon)
			                          | (1<<wp_grenade)
			                          | (1<<wp_ion_cannon)
			                          | (1<<wp_burst_rifle)
			                          | (1<<wp_pistol)
                                   | (1<<wp_autocharge);
		else
		   if (gamestate.ammo >= GRENADE_ENERGY_USE)
				gamestate.useable_weapons = (1<<wp_grenade)
				                          | (1<<wp_ion_cannon)
			                             | (1<<wp_burst_rifle)
			                             | (1<<wp_pistol)
                                      | (1<<wp_autocharge);
         else
				gamestate.useable_weapons = (1<<wp_ion_cannon)
												  | (1<<wp_burst_rifle)
				                          | (1<<wp_pistol)
		                                | (1<<wp_autocharge);
   }
   else
		gamestate.useable_weapons = (1<<wp_autocharge);

   //
   // Do special weapons.
   //

//   if (gamestate.plasma_detonators)
//		gamestate.useable_weapons |= (1<<wp_plasma_detonators);

	//
   // mask off with the weapons being carried.
   //

	gamestate.useable_weapons &= gamestate.weapons;

}




//---------------------------------------------------------------------------
// TakePlasmaDetonator()
//---------------------------------------------------------------------------
void	TakePlasmaDetonator(int count)
{
	if (gamestate.plasma_detonators < count)
		gamestate.plasma_detonators = 0;
	else
		gamestate.plasma_detonators -= count;
}

//---------------------------------------------------------------------------
// GivePlasmaDetonator()
//---------------------------------------------------------------------------
void GivePlasmaDetonator(int count)
{
	gamestate.plasma_detonators += count;

	if (gamestate.plasma_detonators > MAX_PLASMA_DETONATORS)
		gamestate.plasma_detonators = MAX_PLASMA_DETONATORS;

//   if (gamestate.chosenweapon == wp_plasma_detonators)
//	{
//		gamestate.weapon = gamestate.chosenweapon;
//		DrawWeapon ();
//	}

	ComputeAvailWeapons();
}


//---------------------------------------------------------------------------
// GiveToken()
//---------------------------------------------------------------------------
void	GiveToken (int tokens)
{
#if MP_NO_MORE_TOKENS > MP_BONUS
	if (LastMsgType == MT_NO_MO_FOOD_TOKENS)
	{
		MsgTicsRemain = 1;
		LastMsgType = MT_CLEAR;
	}
#endif

	gamestate.tokens += tokens;
	if (gamestate.tokens > MAX_TOKENS)
	{
		gamestate.tokens = MAX_TOKENS;
	}

	SD_PlaySound (GOTTOKENSND);
}

//===========================================================================
//
//
//                      	 INFO AREA ROUTINES
//
//
//===========================================================================

//--------------------------------------------------------------------------
// DisplayInfoMsg() - Returns if Higher Pri message is holding.
//
// SEE MACROS:	 DISPLAY_TIMED_MSG() & DISPLAY_MSG()					-- Def.h
//
//		 DISPLAY_TIMED_MSG(msg,pri,type) - For E-Z Timed Msgs (std. display time)
//     DISPLAY_MSG(msg,pri,type)		 - For E-Z NON-Timed Msgs.
//--------------------------------------------------------------------------
boolean DisplayInfoMsg(char far *Msg,msg_priorities Priority,short DisplayTime,short MsgType)
{
	if (Priority >= LastMsgPri)
	{
		if (Priority == MP_max_val)			// "System" msgs
			LastMsgPri = MP_min_val;
		else
			LastMsgPri = Priority;

#pragma warn -pia
		if (MsgTicsRemain = DisplayTime)
			StatusAllDrawPic(0,40,BRI_LIGHTPIC);
#pragma warn +pia

		gamestate.msg = Msg;

      DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;

      LastMsgType = MsgType;

      if (LastMsgType != MT_ATTACK)
      	LastInfoAttacker_Cloaked = 0;

		return(true);
	}
	else
		return(false);
}


//--------------------------------------------------------------------------
// ClearInfoArea()
//--------------------------------------------------------------------------
void ClearInfoArea(void)
{
	unsigned i,old_ofs;

#if IN_DEVELOPMENT
	if (gamestate.flags & GS_SHOW_OVERHEAD)
		return;
#endif

	if (ClearInfoArea_COUNT)
	   ClearInfoArea_COUNT--;

	InfoAreaSetup.x = InfoAreaSetup.left_margin;
	InfoAreaSetup.y = INFOAREA_Y;
	InfoAreaSetup.framecount = InfoAreaSetup.numanims = 0;

	JLatchDrawPic(0,200-STATUSLINES,INFOAREAPIC);
}


//--------------------------------------------------------------------------
// InitInfoArea()
//--------------------------------------------------------------------------
void InitInfoArea(void)
{
	InfoAreaSetup.left_margin = INFOAREA_X;
	InfoAreaSetup.text_color = INFOAREA_TCOLOR;
	InfoAreaSetup.backgr_color = INFOAREA_BCOLOR;
	InitInfoArea_COUNT--;

	ClearInfoArea();
}


//--------------------------------------------------------------------------
// UpdateInfoArea()
//--------------------------------------------------------------------------
void UpdateInfoArea(void)
{

	if (InfoAreaSetup.numanims)
	{
		AnimatePage();
	}

	if (InitInfoArea_COUNT)
		InitInfoArea();
	else
	if (ClearInfoArea_COUNT)
		ClearInfoArea();

	if (DrawInfoArea_COUNT)
		DrawInfoArea();
}

//---------------------------------------------------------------------------
// UpdateInfoAreaClock() - This routine is called ONLY ONCE per refresh
//								   to update the InfoArea Clock and to release
//								   any messages that have expired.
//---------------------------------------------------------------------------
void UpdateInfoAreaClock(void)
{

  	if (playstate == ex_title || playstate == ex_victorious)
		return;

	//
	// Check for existing timed messages
	//

	if (LastMsgPri && MsgTicsRemain)
	{
		//
		// Tic' that 'Puppy' down - Yea!
		//

		if ((MsgTicsRemain -= tics) <= 0)
		{
			// Message has expired.
			DisplayNoMoMsgs();
		}
	}

}

//---------------------------------------------------------------------------
// DisplayTokens()
//---------------------------------------------------------------------------
char default_msg[] = {   "\r    NO MESSAGES."
						  "^FCA8\r    FOOD TOKENS:      "
						  "                                 "
						  };

char needDetonator_msg[]="\r\r^FC39 FIND THE DETONATOR!";

char haveDetonator_msg[]="\r\r^FC39DESTROY SECURITY CUBE!";

char destroyGoldfire_msg[]="\r\r^FC39  DESTROY GOLDFIRE!";

void DisplayNoMoMsgs(void)
{
	char buffer[9];

	LastMsgPri = MP_min_val;

	if (BONUS_QUEUE)
	{
		DisplayPinballBonus();
		return;
	}

	MsgTicsRemain = 0;
	StatusAllDrawPic (0,40,DIM_LIGHTPIC);
	sprintf((char *)&default_msg[40],"%-d",gamestate.tokens);
	if (gamestuff.level[gamestate.mapon+1].locked)
	{
		switch (gamestate.mapon)
		{
			case 19:
				strcat(default_msg,destroyGoldfire_msg);
			break;

			case 20:
			case 21:
			case 22:
			case 23:
			break;

			default:
				if (gamestate.plasma_detonators)
					strcat(default_msg,haveDetonator_msg);
				else
					strcat(default_msg,needDetonator_msg);
			break;
		}
	}

	DisplayInfoMsg(default_msg,MP_max_val,0,MT_NOTHING);
}

//--------------------------------------------------------------------------
// DrawInfoArea()
//
//
// Active control codes:
//
//  ^ANnn			- define animation
//  ^FCnn			- set font color
//  ^LMnnn			- set left margin (if 'nnn' == "fff" uses current x)
//  ^EP				- end of page (waits for 'M' to read MORE)
//  ^PXnnn			- move x to coordinate 'n'
//  ^PYnnn			- move y to coordinate 'n'
//  ^SHnnn			- display shape 'n' at current x,y
//  ^BGn	 			- set background color
//  ^DM				- Default Margins
//
// Other info:
//
// All 'n' values are hex numbers (0 - f), case insensitive.
// The number of N's listed is the number of digits REQUIRED by that control
// code. (IE: ^LMnnn MUST have 3 values! --> 003, 1a2, 01f, etc...)
//
// If a line consists only of control codes, the cursor is NOT advanced
// to the next line (the ending <CR><LF> is skipped). If just ONE non-control
// code is added, the number "8" for example, then the "8" is displayed
// and the cursor is advanced to the next line.
//
// The text presenter now handles sprites, but they are NOT masked! Also,
// sprite animations will be difficult to implement unless all frames are
// of the same dimensions.
//
//--------------------------------------------------------------------------

char far *HandleControlCodes(char far *first_ch);


void DrawInfoArea(void)
{
	#define IA_FONT_HEIGHT	6

//	short length,i;
	char far *first_ch;
	char far *scan_ch,temp;
	unsigned old_ofs;

#if IN_DEVELOPMENT
	if (gamestate.flags & GS_SHOW_OVERHEAD)
		return;
#endif

	DrawInfoArea_COUNT--;

	if (!*gamestate.msg)
		return;

	first_ch = gamestate.msg;

	fontnumber = 2;
	fontcolor = InfoAreaSetup.text_color;

	while (first_ch && *first_ch)
	{

		if (*first_ch != TP_CONTROL_CHAR)
		{
			scan_ch = first_ch;

			while ((*scan_ch) && (*scan_ch != '\n') && (*scan_ch != TP_RETURN_CHAR) && (*scan_ch != TP_CONTROL_CHAR))
				scan_ch++;

			// print current line
			//

			temp = *scan_ch;
			*scan_ch = 0;

			if (*first_ch != TP_RETURN_CHAR)
			{
				int i;
				char temp_color;

				temp_color = fontcolor;
				fontcolor = INFOAREA_TSHAD_COLOR;

				px = InfoAreaSetup.x+1;
				py = InfoAreaSetup.y+1;
				VW_DrawPropString(first_ch);
				fontcolor = temp_color;

				px = InfoAreaSetup.x;
				py = InfoAreaSetup.y;
				VW_DrawPropString(first_ch);
			}

			*scan_ch = temp;
			first_ch = scan_ch;

			// skip SPACES / RETURNS at end of line
			//

			if ((*first_ch==' ') || (*first_ch==TP_RETURN_CHAR))
				first_ch++;

			// TP_CONTROL_CHARs don't advance to next character line
			//

			if (*scan_ch != TP_CONTROL_CHAR)
			{
				InfoAreaSetup.x = InfoAreaSetup.left_margin;
				InfoAreaSetup.y += IA_FONT_HEIGHT;
			}
			else
				InfoAreaSetup.x = px;
		}
		else
			first_ch = HandleControlCodes(first_ch);
	}
}

//---------------------------------------------------------------------------
// HandleControlCodes()
//---------------------------------------------------------------------------
char far *HandleControlCodes(char far *first_ch)
{
	spritetabletype far *spr;
//	piShapeInfo far *shape_info;
	piShapeInfo far *shape;
	piAnimInfo far *anim;
	unsigned shapenum;
	short length,width;
	char far *s;

	first_ch++;

#ifndef TP_CASE_SENSITIVE
	*first_ch=toupper(*first_ch);
	*(first_ch+1)=toupper(*(first_ch+1));
#endif

	switch (*((unsigned far *)first_ch)++)
	{

		// INIT ANIMATION ---------------------------------------------------
		//
				case TP_CNVT_CODE('A','N'):
					shapenum = TP_VALUE(first_ch,2);
					first_ch += 2;
					_fmemcpy(&piAnimList[InfoAreaSetup.numanims],&piAnimTable[shapenum],sizeof(piAnimInfo));
					anim = &piAnimList[InfoAreaSetup.numanims++];
					shape = &piShapeTable[anim->baseshape+anim->frame];	// BUG!! (assumes "pia_shapetable")
//					spr = &spritetable[shape->shapenum-STARTSPRITES];

					anim->y=InfoAreaSetup.y;
					anim->x=DrawShape(InfoAreaSetup.x,InfoAreaSetup.y,shape->shapenum,shape->shapetype);
               InfoAreaSetup.framecount = 3;
					InfoAreaSetup.left_margin = InfoAreaSetup.x;
				break;

		// DRAW SHAPE -------------------------------------------------------
		//
				case TP_CNVT_CODE('S','H'):

					// NOTE : This needs to handle the left margin....

					shapenum = TP_VALUE(first_ch,3);
					first_ch += 3;
					shape = &piShapeTable[shapenum];
//					spr = &spritetable[shape->shapenum-STARTSPRITES];

					DrawShape(InfoAreaSetup.x,InfoAreaSetup.y,shape->shapenum,shape->shapetype);
					InfoAreaSetup.left_margin = InfoAreaSetup.x;
				break;

		// FONT COLOR -------------------------------------------------------
		//
				case TP_CNVT_CODE('F','C'):
					InfoAreaSetup.text_color = fontcolor = TP_VALUE(first_ch,2);
					first_ch += 2;
				break;

		// BACKGROUND COLOR -------------------------------------------------
		//
				case TP_CNVT_CODE('B','G'):
					InfoAreaSetup.backgr_color = TP_VALUE(first_ch,2);
					first_ch += 2;
				break;

		// DEFAULT MARGINS -------------------------------------------------
		//
				case TP_CNVT_CODE('D','M'):
					InfoAreaSetup.left_margin = INFOAREA_X;
				break;

		// LEFT MARGIN ------------------------------------------------------
		//
				case TP_CNVT_CODE('L','M'):
					shapenum = TP_VALUE(first_ch,3);
					first_ch += 3;
					if (shapenum == 0xfff)
						InfoAreaSetup.left_margin = InfoAreaSetup.x;
					else
						InfoAreaSetup.left_margin = shapenum;
				break;

#ifdef UNLOCK_FLOORS
		// UNLOCK FLOOR ----------------------------------------------------
		//
				case TP_CNVT_CODE('U','F'):
					shapenum = TP_VALUE(first_ch++,1);
					gamestuff.level[shapenum].locked=false;
				break;
#endif
	}

	return(first_ch);

}

//--------------------------------------------------------------------------
// DrawShape()
//--------------------------------------------------------------------------
short DrawShape(short x, short y, short shapenum, pisType shapetype)
{
	short width;
	unsigned i,old_ofs,shade;

//	width=TP_BoxAroundShape(x,y,shapenum,shapetype);

	//
	// If Image is Cloaked... Shade the image
	//
	if (LastInfoAttacker_Cloaked)
		shade = 35;				// 63 == BLACK | 0 == NO SHADING
	else
		shade = 0;

	switch (shapetype)
	{
		case pis_scaled:
//			old_ofs = bufferofs;
//			for (i=0;i<3;i++)
//			{
//				bufferofs = screenloc[i];
//				VWB_Bar(x,y,37,37,InfoAreaSetup.backgr_color);
				VW_Bar(x,y,37,37,InfoAreaSetup.backgr_color);				// JTR changed
				MegaSimpleScaleShape(x+19,y+20,shapenum,37,shade);
//			}
//			bufferofs = old_ofs;
			width = 37;
		break;

#if NUMPICS
		case pis_latchpic:
			x = (x+7) & 0xFFF8;
//			old_ofs = bufferofs;
//			for (i=0;i<3;i++)
//			{
//				bufferofs = screenloc[i];
				JLatchDrawPic(x>>3,y,shapenum);
//			}
//			bufferofs = old_ofs;
		break;

		case pis_pic:
			x = (x+7) & 0xFFF8;
			width = pictable[shapenum-STARTPICS].width;
			CA_MarkGrChunk(shapenum);
			CA_CacheMarks();
//			old_ofs = bufferofs;
//			for (i=0;i<3;i++)
//			{
//				bufferofs = screenloc[i];
				VWB_DrawPic(x,y,shapenum);
//			}
//			bufferofs = old_ofs;
			UNCACHEGRCHUNK(shapenum);
		break;
#endif

#if NUMSPRITES && 0
		case pis_sprite:
//			VW_geDrawSprite(x,y-(spr->orgy>>G_P_SHIFT),shapenum,shapetype == pis_sprite2x);
		break;
#endif
	}

	InfoAreaSetup.x += width;
	return(x);
}

//--------------------------------------------------------------------------
// AnimatePage()
//--------------------------------------------------------------------------
void AnimatePage(void)
{
	piAnimInfo far *anim=piAnimList;
	piShapeInfo far *shape;

	// Dec Timers
	//

	anim->delay += tics;

	if (anim->delay >= anim->maxdelay)
	{
		InfoAreaSetup.framecount = 3;
		anim->delay = 0;
	}

	// Test framecount - Do we need to draw a shape?
	//

	if (InfoAreaSetup.framecount)
	{
		// Draw shapes

		switch (anim->animtype)
		{
			case pia_shapetable:
				shape = &piShapeTable[anim->baseshape+anim->frame];
				DrawShape(anim->x,anim->y,shape->shapenum,shape->shapetype);
			break;

			case pia_grabscript:
				shape = &piShapeTable[anim->baseshape];
				DrawShape(anim->x,anim->y,shape->shapenum+anim->frame,shape->shapetype);
			break;
		}

		// Dec frame count

		InfoAreaSetup.framecount--;
		if (!InfoAreaSetup.framecount)
		{
			// Have drawn all pages... Inc Frame count

			anim->frame++;
			if (anim->frame == anim->maxframes)
				anim->frame = 0;
		}
	}

}

#if 0

//--------------------------------------------------------------------------
// AnimatePage()
//--------------------------------------------------------------------------
void AnimatePage(short numanims)
{
	piAnimInfo far *anim=piAnimList;
	piShapeInfo far *shape;

	anim->delay += tics;

	if (anim->delay >= anim->maxdelay)
	{
		anim->delay = 0;
		anim->frame++;

		if (anim->frame == anim->maxframes)
			anim->frame = 0;

		switch (anim->animtype)
		{
			case pia_shapetable:
				shape = &piShapeTable[anim->baseshape+anim->frame];
				DrawShape(anim->x,anim->y,shape->shapenum,shape->shapetype);
			break;

			case pia_grabscript:
				shape = &piShapeTable[anim->baseshape];
				DrawShape(anim->x,anim->y,shape->shapenum+anim->frame,shape->shapetype);
			break;
		}
	}
}

#endif

//===========================================================================
//
//
//                     	 STATUS BAR REFRESH ROUTINES
//
//
//===========================================================================


//---------------------------------------------------------------------------
// UpdateStatusBar()
//---------------------------------------------------------------------------
void UpdateStatusBar(void)
{
	if (playstate == ex_title || playstate == ex_victorious)
		return;

#ifdef NO_STATUS_BAR
	return;
#endif


	//
	// Call specific status bar managers
	//

	UpdateScore();
	UpdateInfoArea();

	//
	// Refresh Status Area
	//

	if (DrawAmmoPic_COUNT)
		DrawAmmoPic();

//	if (DrawScoreNum_COUNT)
		DrawScoreNum();

	if (DrawWeaponPic_COUNT)
		DrawWeaponPic();

	if (DrawRadarGuage_COUNT)
		DrawRadarGuage();

//	if (DrawAmmoNum_COUNT)
		DrawAmmoNum();

	if (DrawKeyPics_COUNT)
		DrawKeyPics();

	if (DrawHealthNum_COUNT)
		DrawHealthNum();

	if (gamestate.flags & (GS_TICS_FOR_SCORE))
   	DrawScore();

}

//---------------------------------------------------------------------------
// ForceUpdateStatusBar() - Force Draw status bar onto ALL display pages
//---------------------------------------------------------------------------
void ForceUpdateStatusBar(void)
{
	unsigned old_ofs,i;

	old_ofs = bufferofs;

	DrawScore();
	DrawWeapon();
	DrawKeys();
	DrawHealth();
	UpdateRadarGuage();

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		UpdateStatusBar();
   }

   bufferofs = old_ofs;
}


/*
=============================================================================

							MOVEMENT

=============================================================================
*/



/*
===================
=
= GetBonus
=
===================
*/

unsigned far static_points[]={	100,		// money bag
											500,     // loot
											250,     // gold1
											500,     // gold2
											750,     // gold3
											1000,    // major gold!
											5000     // bonus
};

unsigned far static_health[][3] =
{
	{100,HEALTH2SND,-1},			 					// Full Heal
	{ 30,HEALTH1SND,-1},			 					// First Aid
	{ 20,HEALTH1SND,SPR_STAT_45},					// Steak
	{ 15,HEALTH1SND,SPR_STAT_43},					// Chicken Leg
	{ 10,HEALTH1SND,SPR_SANDWICH_WRAPER},	 	// Sandwich
	{  8,HEALTH1SND,SPR_CANDY_WRAPER},	 		// Candy Bar
	{  5,HEALTH1SND,SPR_STAT_41},    			// Water bowl
	{  5,HEALTH1SND,-1},    						// Water puddle
};

extern char far bonus_msg24[];
extern char far bonus_msg25[];

void GetBonus (statobj_t *check)
{
	boolean givepoints=false;
	short shapenum = -1,possible;

	switch (check->itemnumber)
	{
	case	bo_red_key:
	case	bo_yellow_key:
	case	bo_blue_key:
	{
		unsigned keynum = check->itemnumber - bo_red_key;

		if (gamestate.numkeys[keynum] >= MAXKEYS)
			return;

		GiveKey(keynum);
		SD_PlaySound(GETKEYSND);
		TravelTable[check->tilex][check->tiley] &= ~TT_KEYS;
		break;
	}

	case	bo_money_bag:
		SD_PlaySound (BONUS1SND);
		givepoints=true;
		break;

	case	bo_loot:
		SD_PlaySound (BONUS2SND);
		givepoints=true;
		break;


	case	bo_gold1:
	case	bo_gold2:
	case	bo_gold3:
	case	bo_gold:
		SD_PlaySound (BONUS3SND);
		givepoints=true;
		break;


	case	bo_bonus:
		SD_PlaySound (BONUS4SND);
		givepoints=true;
		break;

	case bo_water_puddle:
		if (gamestate.health > 15)
			return;
	case bo_fullheal:
	case bo_firstaid:
	case bo_ham:			// STEAK
	case bo_chicken:
	case bo_sandwich:
	case bo_candybar:
	case bo_water:
		if (gamestate.health == 100)
			return;
		SD_PlaySound (static_health[check->itemnumber-bo_fullheal][1]);
		HealSelf (static_health[check->itemnumber-bo_fullheal][0]);
		check->flags &= ~FL_BONUS;
		shapenum = static_health[check->itemnumber-bo_fullheal][2];
	break;

	case	bo_clip:
		if (gamestate.ammo == MAX_AMMO)
			return;
		GiveAmmo (8);
		bonus_msg7[45] = '8';
		break;

	case	bo_clip2:
   	{
			unsigned char ammo;

			if (gamestate.ammo == MAX_AMMO)
				return;

			ammo = 1+(US_RndT() & 0x7);
      	bonus_msg7[45] = '0'+ammo;
			GiveAmmo (ammo);
		}
		break;

	case	bo_plasma_detonator:
		TravelTable[check->tilex][check->tiley] &= ~TT_KEYS;
		GivePlasmaDetonator(1);
		SD_PlaySound (GETDETONATORSND);
		break;

	case	bo_pistol:
		SD_PlaySound (GETPISTOLSND);
		GiveWeapon(wp_pistol);
		break;

	case	bo_burst_rifle:
		SD_PlaySound (GETBURSTRIFLESND);
		GiveWeapon (wp_burst_rifle);
		break;

	case	bo_ion_cannon:
		SD_PlaySound (GETIONCANNONSND);
		GiveWeapon (wp_ion_cannon);
		break;

	case	bo_grenade:
		SD_PlaySound (GETCANNONSND);
		GiveWeapon (wp_grenade);
		break;


	case	bo_bfg_cannon:
		SD_PlaySound (GETCANNONSND);
		GiveWeapon (wp_bfg_cannon);
		break;


	case bo_coin:
		if (gamestate.tokens == MAX_TOKENS)
			return;
		GiveToken(1);

		writeTokenStr(bonus_msg24);
	break;

	case bo_coin5:
		if (gamestate.tokens == MAX_TOKENS)
			return;
		GiveToken(5);

		writeTokenStr(bonus_msg25);
	break;

		case bo_automapper1:
			if (gamestate.rpower > MAX_RADAR_ENERGY-(RADAR_PAK_VALUE/8))
				return;
			gamestate.rpower += RADAR_PAK_VALUE;
         SD_PlaySound(RADAR_POWERUPSND);
			UpdateRadarGuage();
		break;
	}

	if (givepoints)
	{
		GivePoints(static_points[check->itemnumber-bo_money_bag],true);
#if IN_DEVELOPMENT
#ifdef DEBUG_STATICS
		debug_bonus[1][db_count++] = static_points[check->itemnumber-bo_money_bag];
#endif
#endif
	}

	DISPLAY_TIMED_MSG(BonusMsg[check->itemnumber-1],MP_BONUS,MT_BONUS);
	StartBonusFlash ();
	check->shapenum = shapenum;			// remove from list if shapenum == -1
	check->itemnumber = bo_nothing;
}

void writeTokenStr(char far *str)
{
	char buffer[3],len;

	len = _fstrlen(str);
	if (gamestate.tokens > 9)
		itoa(gamestate.tokens,buffer,10);
	else
	{
		buffer[0]='0';
		itoa(gamestate.tokens,buffer+1,10);
	}

	_fstrcpy(str+len-2,buffer);
}


/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/


boolean TryMove (objtype *ob)
{
	int			xl,yl,xh,yh,x,y,xx,yy;
	objtype		*check;
	long		deltax,deltay;

	if (ob==player)
	{
		xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
		yl = (ob->y-PLAYERSIZE) >>TILESHIFT;
		xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
		yh = (ob->y+PLAYERSIZE) >>TILESHIFT;
	}
	else
	{
		if (ob->obclass == blakeobj)
		{
			xl = (ob->x-(0x1000l)) >>TILESHIFT;
			yl = (ob->y-(0x1000l)) >>TILESHIFT;
			xh = (ob->x+(0x1000l)) >>TILESHIFT;
			yh = (ob->y+(0x1000l)) >>TILESHIFT;
		}
		else
		{
			xl = (ob->x-(0x7FFFl)) >>TILESHIFT;
			yl = (ob->y-(0x7FFFl)) >>TILESHIFT;
			xh = (ob->x+(0x7FFFl)) >>TILESHIFT;
			yh = (ob->y+(0x7FFFl)) >>TILESHIFT;
		}
   }


//
// check for solid walls
//
#pragma warn -pia


	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			if (check = actorat[x][y])
				if ((check < objlist) || (check->flags & FL_FAKE_STATIC))
					return(false);
		}

#pragma warn +pia

//
// check for actors....
//

	yl-=2;
	yh+=2;
	xl-=2;
	xh+=2;

	// NOTE: xl,yl may go NEGITIVE!
	//	----  xh,yh may exceed 63 (MAPWIDTH-1)

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			xx = x & 0x3f;

			yy = y & 0x3f;

			check = actorat[xx][yy];

			if ((check > objlist) && ((check->flags & (FL_SOLID|FL_FAKE_STATIC)) == FL_SOLID))
			{
				deltax = ob->x - check->x;
				if ((deltax < -MINACTORDIST) || (deltax > MINACTORDIST))
					continue;

				deltay = ob->y - check->y;
				if ((deltay < -MINACTORDIST) || (deltay > MINACTORDIST))
					continue;

				return(false);
			}
		}

	return(true);
}


/*
===================
=
= ClipMove
=
= returns true if object hit a wall
=
===================
*/

boolean ClipMove (objtype *ob, long xmove, long ymove)
{
	long	basex,basey;

	basex = ob->x;
	basey = ob->y;

	ob->x = (basex+xmove);
	ob->y = (basey+ymove);

	if (TryMove(ob))
		return(false);

#if (!BETA_TEST) && IN_DEVELOPMENT
	if ((!(gamestate.flags & GS_CLIP_WALLS)) && (ob == player))
		return(true);
#endif

	if (!SD_SoundPlaying())
		SD_PlaySound (HITWALLSND);

	ob->x = (basex+xmove);
	ob->y = basey;

	if (TryMove (ob))
	   return(true);

	ob->x = basex;
	ob->y = (basey+ymove);


	if (TryMove (ob))
	   return(true);

	ob->x = basex;
	ob->y = basey;

   return(true);
}

//==========================================================================

/*
===================
=
= Thrust
=
===================
*/

void Thrust (int angle, long speed)
{
	extern byte far TravelTable[MAPSIZE][MAPSIZE];
	objtype dumb;
	long xmove,ymove;
	long	slowmax;
	unsigned	offset, far *map[2];
	short dx,dy;
	int dangle;
   boolean ignore_map1;

	thrustspeed += speed;
//
// moving bounds speed
//
	if (speed >= MINDIST*2)
		speed = MINDIST*2-1;

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipMove(player,xmove,ymove);

   player_oldtilex = player->tilex;
   player_oldtiley = player->tiley;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

	player->areanumber=GetAreaNumber(player->tilex,player->tiley);
	areabyplayer[player->areanumber] = true;
	TravelTable[player->tilex][player->tiley] |= TT_TRAVELED;

	offset = farmapylookup[player->tiley]+player->tilex;
	map[0]=mapsegs[0]+offset;
	map[1]=mapsegs[1]+offset;

// Check for trigger tiles.
//
	switch (*map[0])
	{
		case DOORTRIGGERTILE:
			dx = *map[1]>>8;									// x
			dy = *map[1]&255;									// y
         if (OperateSmartSwitch(dx,dy,ST_TOGGLE,false))	// Operate & Check for removeal
         	*map[0] = AREATILE+player->areanumber;	// Remove switch
         ignore_map1 = true;
		break;

      case SMART_OFF_TRIGGER:
      case SMART_ON_TRIGGER:
			dx = *map[1]>>8;
			dy = *map[1]&255;
         OperateSmartSwitch(dx,dy,(*map[0])-SMART_OFF_TRIGGER,false);
         ignore_map1 = true;
      break;

		case WINTIGGERTILE:
			playstate = ex_victorious;
			dumb.x = ((long)gamestate.wintilex<<TILESHIFT)+TILEGLOBAL/2;
			dumb.y = ((long)gamestate.wintiley<<TILESHIFT)+TILEGLOBAL/2;
			dumb.flags = 0;
			dangle=CalcAngle(player,&dumb);
			RotateView(dangle,2);
         ignore_map1 = true;
		break;

      default:
         ignore_map1 = false;
      break;
	}

   if (!ignore_map1)
	{
		// Change sky and ground color on-the-fly.
		//

		offset=*(map[1]+1);					// 'offset' used as temp...
		switch (*map[1])
		{
#ifdef CEILING_FLOOR_COLORS
			case 0xfe00:
				TopColor = offset&0xff00;
				TopColor |= TopColor>>8;
				BottomColor = offset&0xff;
				BottomColor |= BottomColor<<8;
			break;
#else
#if IN_DEVELOPMENT
			case 0xfe00:
         	// Give error
			break;
#endif
#endif

#if 0
			case 0xF600:									// Lighting effects
				normalshade_div = (offset&0xff00) >> 8;
				if (normalshade_div > 12)
					AGENT_ERROR(NORMAL_SHADE_TOO_BIG);
				shade_max = offset&0xff;
				if (shade_max > 63 || shade_max < 5)
					AGENT_ERROR(SHADEMAX_VALUE_BAD);
				normalshade=(3*(maxscale>>2))/normalshade_div;
			break;
#endif
		}
   }

}

extern short an_offset[];

#pragma warn -pia

boolean GAN_HiddenArea;

//------------------------------------------------------------------------
// GetAreaNumber()
//------------------------------------------------------------------------
char GetAreaNumber(char tilex, char tiley)
{
	unsigned offset, far *map, far *ptr[2], loop;
	byte areanumber;

	GAN_HiddenArea = false;

// Are we on a wall?
//
	if (tilemap[tilex][tiley] && (!(tilemap[tilex][tiley]&0xc0)))
		return(127);

// Get initial areanumber from map
//
	offset = farmapylookup[tiley]+tilex;
	ptr[0]=mapsegs[0]+offset;
	ptr[1]=mapsegs[1]+offset;

// Special tile areas must use a valid areanumber tile around it.
//
	if (!(areanumber=ValidAreaTile(ptr[0])))
	{
		for (loop=0; loop<8; loop++)
			if (areanumber=ValidAreaTile(ptr[0]+an_offset[loop]))
				break;

		if (loop==8)
			areanumber = AREATILE;
	}

// Merge hidden areanumbers into non-hidden areanumbers AND pull all
// values down to an indexable range.
//
	if (areanumber >= HIDDENAREATILE)
	{
		GAN_HiddenArea = true;
		areanumber -= HIDDENAREATILE;
	}
	else
		areanumber -= AREATILE;

	return(areanumber);
}

#pragma warn +pia

#pragma warn -rch
#pragma warn -rvl

//------------------------------------------------------------------------
// ValidAreaTile()
//------------------------------------------------------------------------
byte ValidAreaTile(unsigned far *ptr)
{
	switch (*ptr)
	{
		case AREATILE:
		case HIDDENAREATILE:
		case DOORTRIGGERTILE:
		case WINTIGGERTILE:
      case SMART_ON_TRIGGER:
      case SMART_OFF_TRIGGER:
		case AMBUSHTILE:
		case LINC_TILE:
		case CLOAK_AMBUSH_TILE:
		break;

		default:
			if (*ptr > AREATILE)
				return(*ptr);
		break;
	}

	return(0);
}

#pragma warn +rch
#pragma warn +rvl

/*
=============================================================================

								ACTIONS

=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire (void)
{
	if (noShots)
		return;

	if ((gamestate.weapon == wp_autocharge) && (gamestate.weapon_wait))
		return;

	buttonheld[bt_attack] = true;

	gamestate.weaponframe = 0;

	player->state = &s_attack;

	gamestate.attackframe = 0;
	gamestate.attackcount = attackinfo[gamestate.weapon][gamestate.attackframe].tics;
	gamestate.weaponframe =	attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

//===========================================================================


void Cmd_Use (void)
{
	objtype 	*check;
	int			checkx,checky,doornum,dir;
	unsigned iconnum;
	unsigned offset,new_level;
	unsigned char static interrogate_delay=0;
	boolean tryDetonator = false;

// Find which cardinal direction the player is facing
//
	if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8)
	{
		checkx = player->tilex + 1;
		checky = player->tiley;
		dir = di_east;
	}
	else if (player->angle < 3*ANGLES/8)
	{
		checkx = player->tilex;
		checky = player->tiley-1;
		dir = di_north;
	}
	else if (player->angle < 5*ANGLES/8)
	{
		checkx = player->tilex - 1;
		checky = player->tiley;
		dir = di_west;
	}
	else
	{
		checkx = player->tilex;
		checky = player->tiley + 1;
		dir = di_south;
	}

	doornum = tilemap[checkx][checky];
	iconnum = *(mapsegs[1]+farmapylookup[checky]+checkx);

// Test for a pushable wall
//
	if (iconnum == PUSHABLETILE)
	{
		PushWall (checkx,checky,dir);
	}
	else
	if (!buttonheld[bt_use])
	{
	// Test for doors / elevator
	//
		if ((doornum & 0x80) && ((pwallx != checkx) || (pwally != checky)))
		{
			buttonheld[bt_use] = true;
			OperateDoor(doornum & ~0x80);
		}
		else
	// Test for special tile types...
	//
		switch (doornum & 63)
		{
		// Test for 'display elevator buttons'
		//
			case TRANSPORTERTILE:
			{
				short new_floor;

				if ((new_floor=InputFloor()) != -1 && new_floor != gamestate.mapon)
				{
					int angle = player->angle;

					gamestuff.level[gamestate.mapon].ptilex = player->tilex;
					gamestuff.level[gamestate.mapon].ptiley = player->tiley;
					angle = player->angle - 180;
					if (angle < 0)
						angle += ANGLES;
					gamestuff.level[gamestate.mapon].pangle = angle;

					playstate=ex_transported;
					gamestate.lastmapon=gamestate.mapon;
					gamestate.mapon=new_floor-1;
				}
				else
					DrawPlayScreen(false);
			}
			break;

			case DIRECTTRANSPORTTILE:
				switch (iconnum & 0xff00)
				{
					case 0xf400:
						playstate = ex_transported;
						gamestate.lastmapon=gamestate.mapon;
						gamestate.mapon=(iconnum & 0xff)-1;
					break;

					default:
						// Stay in current level warp to new location

						playstate = ex_transported;
						Warped();
						playstate = ex_stillplaying;

						player->tilex = (iconnum >> 8);
						player->tiley = iconnum & 0xff;
						player->x = ((long)player->tilex<<TILESHIFT)+TILEGLOBAL/2;
						player->y = ((long)player->tiley<<TILESHIFT)+TILEGLOBAL/2;

						DrawWarpIn();
					break;
				}
			break;

			//
			// Test for Wall Switch Activation
			//
			case OFF_SWITCH:
			case ON_SWITCH:
				ActivateWallSwitch(iconnum,checkx,checky);
			break;


			// Test for Concession Machines
			//

			case FOODTILE:
			case SODATILE:
				OperateConcession((int)actorat[checkx][checky]);
			break;

			default:
				tryDetonator = true;
			break;
		}
	}
	else
	if (!interrogate_delay)
	{
		#define	INTERROGATEDIST	(MINACTORDIST)
		#define	MDIST					2
		#define	INTG_ANGLE			45

		char x,y;
		objtype *intg_ob=NULL,*ob;
		long dx,dy,dist,intg_dist=INTERROGATEDIST+1;

		for (y=-MDIST;y<MDIST+1;y++)
			for (x=-MDIST;x<MDIST+1;x++)
			{
				if ((!tilemap[player->tilex+x][player->tiley+y]) &&
					 (actorat[player->tilex+x][player->tiley+y] >= objlist))
					ob = actorat[player->tilex+x][player->tiley+y];
				else
					continue;
				dx = player->x - ob->x;
				dx = LABS(dx);
				dy = player->y - ob->y;
				dy = LABS(dy);
				dist = dx<dy ? dx:dy;
				if ((ob->obclass==gen_scientistobj) &&
					 ((ob->flags&(FL_FRIENDLY|FL_VISABLE))==(FL_FRIENDLY|FL_VISABLE)) &&
					 (dist < intg_dist))
				{
					short angle=CalcAngle(player,ob);

					angle = ABS(player->angle-angle);
					if (angle > INTG_ANGLE/2)
						continue;

					intg_ob=ob;
					intg_dist=dist;
				}
			}

		if (intg_ob)
		{
			if (Interrogate(intg_ob))
				interrogate_delay=20;		// Informants have 1/3 sec delay
			else
				interrogate_delay=120;		// Non-informants have 2 sec delay
		}
		else
			tryDetonator = true;
	}
	else
	{
		if (tics < interrogate_delay)
			interrogate_delay-=tics;
		else
			interrogate_delay=0;

		tryDetonator = true;
	}

	if (tryDetonator)
	{
		if ((!tryDetonatorDelay) && gamestate.plasma_detonators)
		{
			TryDropPlasmaDetonator();
			tryDetonatorDelay = 60;
		}
	}
	else
		tryDetonatorDelay = 60;

	if (!buttonheld[bt_use])
		interrogate_delay=0;
}

//==========================================================================
//
//                           INTERROGATE CODE
//
//==========================================================================

#define MSG_BUFFER_LEN 150

char far msg[MSG_BUFFER_LEN+1];

memptr InfAreaMsgs[MAX_INF_AREA_MSGS];
byte NumAreaMsgs,LastInfArea;
short FirstGenInfMsg,TotalGenInfMsgs;

scientist_t InfHintList;		// Informant messages
scientist_t NiceSciList;		// Non-informant, non-pissed messages
scientist_t MeanSciList;		// Non-informant, pissed messages

char far int_interrogate[]="INTERROGATE:",
	  far int_informant[]=" ^FC3aINFORMANT^FCa6",
	  far int_rr[]="\r\r",
	  far int_xx[]="^XX",
	  far int_haveammo[]=" HEY BLAKE,\r TAKE MY CHARGE PACK!",
	  far int_havetoken[]=" HEY BLAKE,\r TAKE MY FOOD TOKENS!";

//--------------------------------------------------------------------------
// Interrogate()
//--------------------------------------------------------------------------
boolean Interrogate(objtype *ob)
{
	boolean rt_value=true;
	char far *msgptr=NULL;

	_fstrcpy(msg,int_interrogate);

	if (ob->flags & FL_INFORMANT)						// Informant
	{
		short msgnum;

		_fstrcat(msg,int_informant);

		if (ob->flags & FL_INTERROGATED)
		{
			if ((ob->flags & FL_HAS_AMMO) && (gamestate.ammo != MAX_AMMO))
			{
				GiveAmmo((US_RndT()%8)+1);
				ob->flags &= ~FL_HAS_AMMO;
				msgptr=int_haveammo;
			}
			else
			if ((ob->flags & FL_HAS_TOKENS) && (gamestate.tokens != MAX_TOKENS))
			{
				GiveToken(5);
				ob->flags &= ~FL_HAS_TOKENS;
				msgptr=int_havetoken;
			}
		}

		if (!msgptr)
		{
		// If new areanumber OR no 'area msgs' have been compiled, compile
		// a list of all special messages for this areanumber.
		//
			if ((LastInfArea==0xff) || (LastInfArea!=ob->areanumber))
			{
				sci_mCacheInfo *ci = InfHintList.smInfo;

				NumAreaMsgs=0;
				for (;ci->areanumber != 0xff;ci++)
					if (ci->areanumber == ob->areanumber)
						InfAreaMsgs[NumAreaMsgs++]=InfHintList.smInfo[ci->mInfo.local_val].mInfo.mSeg;

				LastInfArea=ob->areanumber;
			}

		// Randomly select an informant hint, either: specific to areanumber
		// or general hint...
		//
			if (NumAreaMsgs)
			{
				if (ob->ammo != ob->areanumber)
					ob->s_tilex = 0xff;
				ob->ammo = ob->areanumber;
				if (ob->s_tilex == 0xff)
					ob->s_tilex=Random(NumAreaMsgs);
				msgptr=InfAreaMsgs[ob->s_tilex];
			}
			else
			{
				if (ob->s_tiley == 0xff)
					ob->s_tiley=FirstGenInfMsg+Random(TotalGenInfMsgs);
				msgptr=InfHintList.smInfo[ob->s_tiley].mInfo.mSeg;
			}

		// Still no msgptr? This is a shared message! Use smInfo[local_val]
		// for this message.
		//
			if (!msgptr)
				msgptr=InfHintList.smInfo[InfHintList.smInfo[ob->s_tiley].mInfo.local_val].mInfo.mSeg;

			ob->flags |= FL_INTERROGATED;		// Scientist has been interrogated
		}
	}
	else														// Non-Informant
	{
		scientist_t *st;

		rt_value=false;
		if ((ob->flags & FL_MUST_ATTACK) || (US_RndT()&1))		// Mean
		{
			ob->flags &= ~FL_FRIENDLY;		  	// Make him attack!
			ob->flags |= FL_INTERROGATED;		//  "    "     "
			st = &MeanSciList;
		}
		else																	// Nice
		{
			ob->flags |= FL_MUST_ATTACK;		// Make him mean!
			st = &NiceSciList;
		}

		msgptr=st->smInfo[Random(st->NumMsgs)].mInfo.mSeg;
	}

	if (msgptr)
	{
		_fstrcat(msg,int_rr);
		_fstrcat(msg,msgptr);
		_fstrcat(msg,int_xx);
		if (_fstrlen(msg) > MSG_BUFFER_LEN)
			AGENT_ERROR(INTERROGATE_LONG_MSG);
		DisplayInfoMsg(msg,MP_INTERROGATE,DISPLAY_MSG_STD_TIME*2,MT_GENERAL);
		SD_PlaySound(INTERROGATESND);
	}

	return(rt_value);
}

//==========================================================================
//
//                            ELEVATOR CODE
//
//==========================================================================


extern boolean pollMouseUsed;

char far if_help[]="UP/DN MOVES SELECTOR - ENTER ACTIVATES";
char far if_noImage[]="   AREA\n"
							 "  UNMAPPED\n"
							 "\n"
							 "\n"
							 " PRESS ENTER\n"
							 " TO TELEPORT";

statsInfoType ov_stats;
memptr ov_buffer;
boolean ov_noImage=false;

#define TOV_X  16
#define TOV_Y	132

//--------------------------------------------------------------------------
// InputFloor
//--------------------------------------------------------------------------
short InputFloor(void)
{
	#define RADAR_FLAGS		OV_KEYS
	#define MAX_TELEPORTS 	20
	#define MAX_MOVE_DELAY	10

	short buttonPic,buttonY;
	short rt_code=-2,tpNum=gamestate.mapon,lastTpNum=tpNum;
	short teleX[MAX_TELEPORTS]={16,40,86,23,44,62,83,27,118,161,161,161,213,213,184,205,226,256,276,276};
	short teleY[MAX_TELEPORTS]={13,26, 9,50,50,50,50,62, 42, 17, 26, 35, 41, 50, 62, 62, 62, 10, 10, 30};
	char moveActive=0;
	objtype old_player;
	boolean locked=false,buttonsDrawn=false;

	ClearMemory();
	VW_FadeOut();

	CacheDrawPic(0,0,TELEPORTBACKTOPPIC);
	CacheDrawPic(0,12*8,TELEPORTBACKBOTPIC);
	DisplayTeleportName(tpNum,locked);
	CacheLump(TELEPORT_LUMP_START,TELEPORT_LUMP_END);
	VWB_DrawMPic(teleX[tpNum],teleY[tpNum],TELEPORT1ONPIC+tpNum);

	memcpy(&old_player,player,sizeof(objtype));
	player->angle = 90;
	player->x = player->y = ((long)32<<TILESHIFT)+(TILEGLOBAL/2);

	MM_GetPtr(&ov_buffer,4096);
	ShowStats(0,0,ss_justcalc,&gamestuff.level[gamestate.mapon].stats);
	_fmemcpy(&ov_stats,&gamestuff.level[gamestate.mapon].stats,sizeof(statsInfoType));
	ShowOverhead(TOV_X,TOV_Y,32,0,RADAR_FLAGS);
	SaveOverheadChunk(tpNum);

	px = 115;
	py = 188;
	fontcolor = 0xaf;
	fontnumber = 2;
	ShPrint(if_help,0,false);

	controlx = controly = 0;
	IN_ClearKeysDown();
	while (rt_code == -2)
	{
	// Handle ABORT and ACCEPT
	//
//		if (!screenfaded)
//			PollControls();

		CalcTics();
		if (Keyboard[sc_LeftArrow])
			controlx = -1;
		else
			if (Keyboard[sc_RightArrow])
				controlx = 1;
			else
				controlx = 0;

		if (Keyboard[sc_UpArrow])
			controly = -1;
		else
			if (Keyboard[sc_DownArrow])
				controly = 1;
			else
				controly = 0;

		if (Keyboard[sc_Escape] || buttonstate[bt_strafe])
		{
			rt_code=-1;													// ABORT

			LoadLocationText(gamestate.mapon+MAPS_PER_EPISODE*gamestate.episode);
			break;
		}
		else
			if (Keyboard[sc_Enter] || buttonstate[bt_attack])
			{
				if (locked)
				{
					if (!SD_SoundPlaying())
						SD_PlaySound(NOWAYSND);
				}
				else
				{
					char loop;

					rt_code=tpNum;											// ACCEPT

				// Flash selection
				//
					for (loop=0; loop<10; loop++)
					{
						VWB_DrawMPic(teleX[tpNum],teleY[tpNum],TELEPORT1OFFPIC+tpNum);
						VW_UpdateScreen();
						VW_WaitVBL(4);

						VWB_DrawMPic(teleX[tpNum],teleY[tpNum],TELEPORT1ONPIC+tpNum);
						VW_UpdateScreen();
						VW_WaitVBL(4);
					}

					break;
				}
			}

		CheckMusicToggle();

	// Handle delay
	//
		if (moveActive)
		{
			moveActive -= tics;
			if (moveActive<0)
				moveActive=0;
		}

	// Move to NEXT / PREV teleport?
	//
		buttonY=0;
		if (controlx>0 || controly>0)
		{
			if (!moveActive && tpNum<MAX_TELEPORTS-1)
			{
				tpNum++;												// MOVE NEXT
				moveActive=MAX_MOVE_DELAY;
			}

			buttonPic = TELEDNONPIC;
			buttonY = 104;
		}
		else
			if (controlx<0 || controly<0)
			{
				if (!moveActive && tpNum)
				{
					tpNum--;											// MOVE PREV
					moveActive=MAX_MOVE_DELAY;
				}

				buttonPic = TELEUPONPIC;
				buttonY = 91;
			}

	// Light buttons?
	//
		if (buttonY)
		{
			VWB_DrawMPic(34,91,TELEUPOFFPIC);
			VWB_DrawMPic(270,91,TELEUPOFFPIC);
			VWB_DrawMPic(34,104,TELEDNOFFPIC);
			VWB_DrawMPic(270,104,TELEDNOFFPIC);

			VWB_DrawMPic(34,buttonY,buttonPic);
			VWB_DrawMPic(270,buttonY,buttonPic);
			buttonsDrawn=true;
		}
		else
		// Unlight buttons?
		//
			if (buttonsDrawn)
			{
				VWB_DrawMPic(34,91,TELEUPOFFPIC);
				VWB_DrawMPic(270,91,TELEUPOFFPIC);
				VWB_DrawMPic(34,104,TELEDNOFFPIC);
				VWB_DrawMPic(270,104,TELEDNOFFPIC);
				buttonsDrawn=false;
			}

	// Change visual information
	//
		if (tpNum != lastTpNum)
		{
			locked = gamestuff.level[tpNum].locked;
			DisplayTeleportName(tpNum,locked);

			VWB_DrawMPic(teleX[lastTpNum],teleY[lastTpNum],TELEPORT1OFFPIC+lastTpNum);
			VWB_DrawMPic(teleX[tpNum],teleY[tpNum],TELEPORT1ONPIC+tpNum);

			LoadOverheadChunk(tpNum);
			ShowOverheadChunk();
			if (ov_noImage)
			{
				fontcolor = 0x57;
				WindowX = WindowW = TOV_X;
				WindowY = WindowH = TOV_Y;
				WindowW += 63;
				WindowH += 63;
				PrintX = TOV_X+5;
				PrintY = TOV_Y+13;
				US_Print(if_noImage);
			}
			lastTpNum = tpNum;
		}

		if (locked)
		{
			ShowOverhead(TOV_X,TOV_Y,32,-1,RADAR_FLAGS);
			VW_MarkUpdateBlock(TOV_X,TOV_Y,79,195);
		}

		CycleColors();
		VW_UpdateScreen();
		if (screenfaded)
		{
			VW_FadeIn();
			ShowStats(235,138,ss_normal,&ov_stats);
			IN_ClearKeysDown();
			controlx = controly = 0;
		}
	}

#if 0
	for (buttonY=63; buttonY>=0; buttonY -= 2)
	{
		char shps[]={TELEPORT1ONPIC,TELEPORT1OFFPIC};

		if (rt_code != -1)
			VWB_DrawMPic(teleX[tpNum],teleY[tpNum],shps[(buttonY&4)>>2]+tpNum);

		if (locked)
		{
			ShowOverhead(TOV_X,TOV_Y,32,-locked,RADAR_FLAGS);
			VW_MarkUpdateBlock(TOV_X,TOV_Y,79,195);
		}

		CycleColors();
		VL_SetPaletteIntensity(0,255,&vgapal,buttonY);
		VW_UpdateScreen();
	}
#else
	VW_FadeOut();
#endif

	MM_FreePtr(&ov_buffer);
	memcpy(player,&old_player,sizeof(objtype));
	UnCacheLump(TELEPORT_LUMP_START,TELEPORT_LUMP_END);

	PM_CheckMainMem();
	DrawPlayScreen(false);
	IN_ClearKeysDown();

	return(rt_code);
}

//--------------------------------------------------------------------------
// ShowOverheadChunk()
//--------------------------------------------------------------------------
void ShowOverheadChunk(void)
{
	VL_MemToScreen(ov_buffer,64,64,TOV_X,TOV_Y);
	VW_MarkUpdateBlock(TOV_X,TOV_Y,79,195);
	ShowStats(235,138,ss_quick,&ov_stats);
}

//--------------------------------------------------------------------------
// LoadOverheadChunk()
//--------------------------------------------------------------------------
void LoadOverheadChunk(short tpNum)
{
	int handle;
	long offset;
	char chunk[5]="OVxx";

// Open PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	if ((handle=open(tempPath,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		MAIN_ERROR(SAVELEVEL_DISKERR);

// Find and load chunk
//
	sprintf(&chunk[2],"%02x",tpNum);
	if (FindChunk(handle,chunk))
	{
		ov_noImage=false;
		IO_FarRead(handle,ov_buffer,4096);
		IO_FarRead(handle,(char far *)&ov_stats,sizeof(statsInfoType));
	}
	else
	{
		ov_noImage=true;
		_fmemset(ov_buffer,0x52,4096);
		memset(&ov_stats,0,sizeof(statsInfoType));
	}

// Close file
//
	close(handle);
}

//--------------------------------------------------------------------------
// SaveOverheadChunk()
//--------------------------------------------------------------------------
void SaveOverheadChunk(short tpNum)
{
	int handle;
	long cksize=4096+sizeof(statsInfoType);
	char chunk[5]="OVxx";

// Open PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	if ((handle=open(tempPath,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		MAIN_ERROR(SAVELEVEL_DISKERR);

// Remove level chunk from file
//
	sprintf(&chunk[2],"%02x",tpNum);
	DeleteChunk(handle,chunk);

// Prepare buffer
//
	VL_ScreenToMem(ov_buffer, 64, 64, TOV_X, TOV_Y);

// Write chunk ID, SIZE, and IMAGE
//
	write(handle,chunk,4);
	IO_FarWrite(handle,(char far *)&cksize,sizeof(cksize));
	IO_FarWrite(handle,ov_buffer,4096);
	IO_FarWrite(handle,(char far *)&ov_stats,sizeof(statsInfoType));

// Close file
//
	close(handle);
}

//--------------------------------------------------------------------------
// DisplayTeleportName()
//--------------------------------------------------------------------------
void DisplayTeleportName(char tpNum, boolean locked)
{
	char far *s;
	word w,h;

	if (locked)
	{
		fontcolor = 0xf5;
		s = "-- TELEPORT DISABLED --";
	}
	else
	{
		fontcolor = 0x57;
		LoadLocationText(tpNum);
		s = LocationText;
	}
	VW_MeasurePropString(s,&w,&h);
	py = 103;
	px = 160-w/2;
	VW_Bar(54,101,212,9,0x52);
	VW_MarkUpdateBlock(54,101,265,108);
	ShPrint(s,0,false);
}

//--------------------------------------------------------------------------
// CacheDrawPic()
//--------------------------------------------------------------------------
void CacheDrawPic(short x, short y, short pic)
{
	CA_CacheGrChunk(pic);
	VWB_DrawPic(x,y,pic);
	UNCACHEGRCHUNK(pic);
}

//===========================================================================
//
//						     MISSION STATISTICS CODE
//
//===========================================================================

	#define BAR_W			48
	#define BAR_H			5

	#define BAR1_COLOR	0xe0
	#define BAR2_COLOR	0x30
	#define BAR3_COLOR	0x10

	#define PERC_W			13
	#define PERC_H			5

boolean show_stats_quick;

//--------------------------------------------------------------------------
// ShowStats()
//--------------------------------------------------------------------------
short ShowStats(short bx, short by, ss_type type, statsInfoType far *stats)
{
	short floor,total=0,mission=0,p1,p2,p3,loop,maxPerFloor;

// Define max points per floor...
//
	if (stats->total_points || stats->total_inf || stats->total_enemy)
		maxPerFloor = 300;
	else
		maxPerFloor = 0;

// Setup to test for bypassing stats.
//
	LastScan=0;

	if (type == ss_quick)
		show_stats_quick=true;
	else
		show_stats_quick=false;

// Show ratio for each statistic:
//
//      TOTAL POINTS, INFORMANTS ALIVE, ENEMY DESTROYED,
//      OVERALL FLOOR, OVERALL MISSION
//

// Show TOTAL POINTS ratio.
//
	p1=ShowRatio(bx,by,bx+52,by,stats->total_points,stats->accum_points,type);

// Show INFORMANTS ALIVE ratio.
//
	by += 7;
	p2=ShowRatio(bx,by,bx+52,by,stats->total_inf,stats->accum_inf,type);

// Show ENEMY DESTROYED ratio.
//
	by += 7;
	p3=ShowRatio(bx,by,bx+52,by,stats->total_enemy,stats->accum_enemy,type);

// Show OVERALL FLOOR ratio.
//
	by += 13;
	floor=p1+p2+p3;
	ShowRatio(bx,by,bx+52,by,maxPerFloor,floor,type);

// Show OVERALL MISSION ratio.
//
	by += 7;
	stats->overall_floor=floor;
	for (loop=0; loop<MAPS_WITH_STATS; loop++)
	{
		total+=300;
		mission+=gamestuff.level[loop].stats.overall_floor;
	}
	mission=ShowRatio(bx,by,bx+52,by,total,mission,type);

	if (show_stats_quick)
		VW_UpdateScreen();

	return(mission);
}

//--------------------------------------------------------------------------
// ShowRatio()
//--------------------------------------------------------------------------
byte ShowRatio(short bx, short by, short nx, short ny, long total, long perc, ss_type type)
{
	char numbars;
	char maxperc;
	char percentage=1,loop;

//	if (perc > total)
//		perc = total;

// Catch those nasty divide-by-zeros!
//
	if (total)
	{
		maxperc=LRATIO(100,total,perc,10);
		numbars=LRATIO(48,100,maxperc,10);
	}
	else
	{
		if (type != ss_justcalc)
		{
			fontcolor = 0x57;
			VW_Bar(bx,by,BAR_W,BAR_H,0);
			VW_MarkUpdateBlock(bx,by,bx+(BAR_W-1),by+(BAR_H-1));
			VW_Bar(nx,ny,PERC_W+6,PERC_H,0);
			PrintX=nx; PrintY=ny;
			US_Print("N/A");
		}
		return(100);
	}

	if (type == ss_justcalc)
		return(maxperc);

	PrintY=ny;
	fontcolor=0xaf;
	fontnumber=2;

	VW_Bar(bx,by,BAR_W,BAR_H,0x07);
	PrintStatPercent(nx,ny,0);
	VW_MarkUpdateBlock(bx,by,bx+(BAR_W-1),by+(BAR_H-1));
	for (loop=0; loop<numbars; loop++)
	{
		if (LastScan)
			show_stats_quick=true;

	// Print one line of bar
	//
		VW_MarkUpdateBlock(bx,by,bx,by+(BAR_H-1));
		VL_Vlin(bx++,by,BAR_H,0xc8);

	// Keep up with current percentage
	//
		if (loop==numbars-1)
			percentage = maxperc;
		else
			percentage += 2;

		PrintStatPercent(nx,ny,percentage);

		if (!show_stats_quick)
		{
			if (!(loop%2))
				SD_PlaySound(STATS1SND);
			VW_WaitVBL(1);
			VW_UpdateScreen();
		}
	}

	if (!show_stats_quick && numbars)
	{
		SD_PlaySound(STATS2SND);
		while (SD_SoundPlaying() && !LastScan);
	}

	return(maxperc);
}

//--------------------------------------------------------------------------
// PrintStatPercent()
//--------------------------------------------------------------------------
void PrintStatPercent(short nx, short ny, char percentage)
{
	if (percentage < 10)
		PrintX=nx+9;
	else
	if (percentage < 100)
		PrintX=nx+4;
	else
		PrintX=nx-1;

	VW_Bar(nx,ny,PERC_W+5,PERC_H,0);
	US_PrintUnsigned(percentage);
	US_Print("%");
}

//--------------------------------------------------------------------------
// PerfectStats()
//--------------------------------------------------------------------------
boolean PerfectStats()
{
	if ((gamestuff.level[gamestate.mapon].stats.total_points == gamestuff.level[gamestate.mapon].stats.accum_points) &&
		 (gamestuff.level[gamestate.mapon].stats.total_inf == gamestuff.level[gamestate.mapon].stats.accum_inf) &&
		 (gamestuff.level[gamestate.mapon].stats.total_enemy == gamestuff.level[gamestate.mapon].stats.accum_enemy))
		return(true);

	return(false);
}

//===========================================================================
//
//  				           PINBALL BONUS DISPLAY CODE
//
//===========================================================================

//--------------------------------------------------------------------------
// B_GAliFunc()
//--------------------------------------------------------------------------
void B_GAliFunc()
{
	extern char far B_GAlienDead2[];

	if (gamestate.episode == 5)
		DisplayInfoMsg(B_GAlienDead2,MP_PINBALL_BONUS,7*60,MT_BONUS);
}

//--------------------------------------------------------------------------
// B_EManFunc()
//--------------------------------------------------------------------------
void B_EManFunc()
{
	unsigned temp,i;

	SD_PlaySound(EXTRA_MANSND);
	fontnumber = 2;

	temp = bufferofs;

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		JLatchDrawPic(0,0,TOP_STATUSBARPIC);
		ShadowPrintLocationText(sp_normal);
	}

	bufferofs = temp;
}

//--------------------------------------------------------------------------
// B_MillFunc()
//--------------------------------------------------------------------------
void B_MillFunc()
{
	GiveAmmo(99);
	HealSelf(99);
}

//--------------------------------------------------------------------------
// B_RollFunc()
//--------------------------------------------------------------------------
void B_RollFunc()
{
	B_MillFunc();
	gamestate.score_roll_wait	= SCORE_ROLL_WAIT;
}

//--------------------------------------------------------------------------
// Pinball Bonus Text
//--------------------------------------------------------------------------

char far B_GAlienDead2[] = "^FC57    GUARDIAN ALIEN\r"
										  "      DESTROYED!\r\r"
									"^FCA6 FIND AND DESTROY ALL\r"
										  "PROJECTION GENERATORS!";

char far B_GAlienDead[] =  "^FC57    GUARDIAN ALIEN\r"
										 "      DESTROYED!\r\r"
								  "^FCA6   FIND THE EXIT TO\r"
										 "COMPLETE THIS MISSION";

char far B_ScoreRolled[] = "^FC57\rROLLED SCORE DISPLAY!\r"
									"^FCA6   FULL AMMO BONUS!\r"
										  "  FULL HEALTH BONUS!\r"
										  "1,000,000 POINT BONUS!";

char far B_OneMillion[]  = "^FC57\r     GREAT SCORE!\r"
									"^FCA6   FULL AMMO BONUS!\r"
										  "  FULL HEALTH BONUS!\r"
										  "1,000,000 POINT BONUS!";

char far B_ExtraMan[]  = "^FC57\r\r     GREAT SCORE!\r"
									"^FCA6  EXTRA LIFE BONUS!\r";

char far B_EnemyDestroyed[]  = "^FC57\r\r ALL ENEMY DESTROYED!\r"
									  "^FCA6  50,000 POINT BONUS!\r";

char far B_TotalPoints[]  =  "^FC57\r\r ALL POINTS COLLECTED!\r"
									"^FCA6  50,000 POINT BONUS!\r";

char far B_InformantsAlive[]  = "^FC57\r\r ALL INFORMANTS ALIVE!\r"
										"^FCA6  50,000 POINT BONUS!\r";

//--------------------------------------------------------------------------
// Pinball Bonus Table
//--------------------------------------------------------------------------
PinballBonusInfo far PinballBonus[]={

//                                        Special
//  BonusText           Points   Recur?	Function
//-----------------------------------------------------
	{B_GAlienDead,			0,			false,	B_GAliFunc},
	{B_ScoreRolled,		1000000l,true,		B_RollFunc},
	{B_OneMillion,		  	1000000l,false,	B_MillFunc},
	{B_ExtraMan,	 		0,			true,		B_EManFunc},
	{B_EnemyDestroyed,	50000l,	false,	NULL},
	{B_TotalPoints,		50000l,	false,	NULL},
	{B_InformantsAlive,	50000l,	false,	NULL},

};

//--------------------------------------------------------------------------
// DisplayPinballBonus()
//--------------------------------------------------------------------------
void DisplayPinballBonus()
{
	char loop;

// Check queue for bonuses
//
	for (loop=0; loop<sizeof(gamestuff.level[0].bonus_queue)*8; loop++)
		if ((BONUS_QUEUE & (1<<loop)) && (LastMsgPri < MP_PINBALL_BONUS))
		{
		// Start this bonus!
		//
			SD_PlaySound(ROLL_SCORESND);
			DisplayInfoMsg(PinballBonus[loop].BonusText,MP_PINBALL_BONUS,7*60,MT_BONUS);

		// Add to "shown" ... Remove from "queue"
		//
			if (!PinballBonus[loop].Recurring)
				BONUS_SHOWN |= (1<<loop);
			BONUS_QUEUE &= ~(1<<loop);

		// Give points and execute special function.
		//
			GivePoints(PinballBonus[loop].Points,false);
			if (PinballBonus[loop].func)
				PinballBonus[loop].func();
		}
}

//--------------------------------------------------------------------------
// CheckPinballBonus()
//--------------------------------------------------------------------------
void CheckPinballBonus(long points)
{
	long score_before=gamestate.score,
		  score_after=gamestate.score+points;

// Check SCORE ROLLED bonus
//
	if (score_before <= MAX_DISPLAY_SCORE && score_after > MAX_DISPLAY_SCORE)
		ActivatePinballBonus(B_SCORE_ROLLED);

// Check ONE MILLION bonus
//
	if	(score_before < 500000l && score_after >= 500000l)
		ActivatePinballBonus(B_ONE_MILLION);

// Check EXTRA MAN bonus
//
	if (score_after >= gamestate.nextextra)
	{
		gamestate.nextextra += EXTRAPOINTS;
		if (gamestate.lives < MAX_EXTRA_LIVES)
		{
			gamestate.lives++;
			ActivatePinballBonus(B_EXTRA_MAN);
		}
	}

// Check TOTAL ENEMY bonus
//
	if (gamestuff.level[gamestate.mapon].stats.total_enemy == gamestuff.level[gamestate.mapon].stats.accum_enemy)
		ActivatePinballBonus(B_ENEMY_DESTROYED);

// Check TOTAL POINTS bonus
//
	if (gamestuff.level[gamestate.mapon].stats.total_points == gamestuff.level[gamestate.mapon].stats.accum_points)
		ActivatePinballBonus(B_TOTAL_POINTS);

// Check INFORMANTS ALIVE bonus
//
	if ((gamestuff.level[gamestate.mapon].stats.total_inf == gamestuff.level[gamestate.mapon].stats.accum_inf) &&	// All informants alive?
		  (gamestuff.level[gamestate.mapon].stats.total_inf) &&																							// Any informants in level?
		  ((BONUS_SHOWN & (B_TOTAL_POINTS|B_ENEMY_DESTROYED)) == (B_TOTAL_POINTS|B_ENEMY_DESTROYED)))														// Got ENEMY and POINTS bonuses?
		ActivatePinballBonus(B_INFORMANTS_ALIVE);

// Display bonuses?
//
	if (BONUS_QUEUE)
		DisplayPinballBonus();
}

//===========================================================================
//
//
//								COMPUTER TERMINAL ROUTINES
//
//
//===========================================================================

#ifdef ACTIVATE_TERMINAL

#define TERM_BUFFERED_DISPLAY
#define TERM_VIEW_WIDTH					246
#define TERM_VIEW_HEIGHT				95
//#define TERM_BACK_COLOR					2			// Defined in 3d)menu.h
#define TERM_BACK_XOFS					8
#define TERM_BACK_YOFS					22
#define TERM_BACK_WIDTH 				304
#define TERM_BACK_HEIGHT				124

#define TERM_BCOLOR						3		// Dark Grey
#define TERM_TCOLOR						88		// Green MONO text color 87=LOW intensity
#define TERM_TSHAD_COLOR				0     // "Shadow" color

#define TERM_SCREEN_XOFS				(TERM_BACK_XOFS+19)
#define TERM_SCREEN_YOFS				(TERM_BACK_YOFS+14)

static unsigned far tcursor_x = TERM_SCREEN_XOFS,
					 far tcursor_y = TERM_SCREEN_YOFS;


char TERM_sound_on = 1;


char far *Commands[TC_LAST];

memptr TermMessages = NULL;
memptr TermCommands = NULL;

#define FreeTerminalCommands()	MM_FreePtr(&TermCommands)
#define FreeTerminalMessages()	MM_FreePtr(&TermMessages)
#define LoadTerminalText()			IO_LoadFile(term_msg_name,&TermMessages)
//#define LoadTerminalText()			IO_LoadFile("TERM_MSG.TXT",&TermMessages)

//---------------------------------------------------------------------------
//
// LoadTerminalCommands()
//
// Caches in the commands in TERM_COMMANDS grsegs and sparces the file for
// commands - Assigning Commands[] to the beginning of each command and
// null terminating the command.  Each command is seperated with ^XX.  Leading
// returns&linefeeds are skipped.
//
// NOTE: This expects that TC_LAST in the enum list of commands is concurrent
//       with the grseg TERM_COMMANDS.
//
//---------------------------------------------------------------------------
void LoadTerminalCommands(void)
{
	char far *Message;
	unsigned char pos;

//	IO_LoadFile("TERM_CMD.TXT",&TermCommands);
	IO_LoadFile(term_com_name,&TermCommands);
	Message = TermCommands;

	for (pos = 0;pos<TC_LAST;pos++)
   {
      // Bump past any leading returns/linefeeds

      while (*Message == '\n' || *Message == '\r')
      	Message++;

      // Assign ptrs

      Commands[pos] = Message;

		if (!(Message = _fstrstr(Message,int_xx)))
			ACT1_ERROR(INVALID_CACHE_MSG_NUM);

      *Message = 0;	// Null Terminate String
		Message += 3;	// Bump to start of next Message
	}
}




boolean term_cursor_vis = true;
boolean shadow_text = true;

#if 1

PresenterInfo Terminal_PI;

//---------------------------------------------------------------------------
// TerminalPrint()
//---------------------------------------------------------------------------
void TerminalPrint(char far *msg, boolean FastPrint)
{
	Terminal_PI.print_delay = !FastPrint;
	Terminal_PI.script[0] = msg;
	TP_Presenter(&Terminal_PI);
}

#else

//---------------------------------------------------------------------------
// TerminalPrint()
//
// NOTE : Terminal Control Chars
//
//          @ - Square Box (IE. Cursor)
//---------------------------------------------------------------------------

void TerminalPrint(char far *msg,boolean FastPrint)
{
	#define TERM_PRINT_DELAY			1
	fontstruct _seg *font;
	char buf[2] = {0,0};
	char old_color,old_color2;
	char fontheight;

	font = (fontstruct _seg *)grsegs[STARTFONT+fontnumber];
	fontheight = font->height;

	while (msg && *msg)
	{
		buf[0] = *msg++;

		if (buf[0] == '^')
		{
			//
			//  Handle Control Codes
			//

			switch (*((unsigned far *)msg)++)
			{
				// FONT COLOR
				//
				case TP_CNVT_CODE('F','C'):
					fontcolor = TP_VALUE(msg,2);
					msg += 2;
				break;

				// BELL
				//
				case TP_CNVT_CODE('B','E'):
					SD_PlaySound(TERM_BEEPSND);
					SD_WaitSoundDone();
				break;

				// HIDE CURSOR
				//
				case TP_CNVT_CODE('H','I'):
					px = tcursor_x;
					py = tcursor_y;
					old_color = fontcolor;
					fontcolor = TERM_BCOLOR;
					VW_DrawPropString("@");
					fontcolor = old_color;
				break;


				// PAUSE
				//
				case TP_CNVT_CODE('P','A'):
					VW_WaitVBL(30);
				break;


            // END OF MSG
            //

            case TP_CNVT_CODE('X','X'):
            	msg = NULL;
            break;

			}
		}
		else
		{
			//
			// Process Text Char (Like print it!)
			//

			bufferofs = displayofs;

			if (term_cursor_vis)
			{
				px = tcursor_x;
				py = tcursor_y;

				old_color = fontcolor;		// Store Cursor Color
				fontcolor = TERM_BCOLOR;

				VW_DrawPropString("@");

				fontcolor = old_color;
			}

			if (buf[0] != '\n')
			{
				// Blast "Shadow" on screen

				if (shadow_text)
				{
					px = tcursor_x+1;
					py = tcursor_y+1;
					old_color2 = fontcolor;					// STORE	Old Colr
					fontcolor =	TERM_TSHAD_COLOR;
					VW_DrawPropString(buf);
					fontcolor = old_color2;					// RESTORE Old Colr
				}

				// Blast normal Text color to screen

				px = tcursor_x;
				py = tcursor_y;
				VW_DrawPropString(buf);

				if (sound_on)
					if (buf[0] != ' ')
						SD_PlaySound(TERM_TYPESND);

				tcursor_x = px;

				if (term_cursor_vis)
				{
					VW_DrawPropString("@");
				}
			}
			else
			{
				if (tcursor_y > 90+TERM_SCREEN_XOFS)
					VL_ScreenToScreen(displayofs+((TERM_SCREEN_YOFS+fontheight)*SCREENWIDTH)+(TERM_SCREEN_XOFS/4),
											displayofs+TERM_SCREEN_YOFS*SCREENWIDTH+TERM_SCREEN_XOFS/4,
											(248/4),93);
				else
					tcursor_y += fontheight;

				tcursor_x = TERM_SCREEN_XOFS;
			}

			if (!FastPrint)
				VL_WaitVBL(TERM_PRINT_DELAY);

			VW_UpdateScreen();
		}
	}
}

#endif




//---------------------------------------------------------------------------
// CacheTerminalPrint()
//
// This prints a message in the TERM_MESSAGES grsegs which MUST
// already be loaded into memory.
//---------------------------------------------------------------------------
void CacheTerminalPrint(short MsgNum,boolean FastPrint)
{
	char far *Message;

	Message = TermMessages;

// Search for end of MsgNum-1 (Start of our message)
//
#pragma warn -pia
	while (MsgNum--)
	{
		if (!(Message = _fstrstr(Message,int_xx)))
			AGENT_ERROR(BAD_TERMINAL_MSG_NUM);
		Message += 3;	// Bump to start of next Message
	}
#pragma warn +pia

// Move past LFs and CRs that follow "^XX"
//
//	while ((*Message=='\n') || (*Message=='\r'))
//		Message++;

	Message += 2;		// Move past LF and CR that follows "^XX"	  

	TerminalPrint(Message,FastPrint);
}



char far TERM_MSG[]="^ST1^CEEnter commands and press ENTER.\r^CEPress ESC to exit terminal.^XX";

//---------------------------------------------------------------------------
// ActivateTerminal()
//---------------------------------------------------------------------------
void ActivateTerminal(boolean skiplink)
{
   #define MAX_INPUT		30
   char buffer[MAX_INPUT];
	boolean temp_caps = allcaps,ExitMoFo;
	unsigned oldwidth;
	US_CursorStruct TermCursor = {'@',0,0x58,2};			// Holds Font#, etc.
   short msgnum;


// Setup for text presenter
//
	memset(&Terminal_PI,0,sizeof(Terminal_PI));
	Terminal_PI.flags=TPF_USE_CURRENT|TPF_SHOW_CURSOR|TPF_SCROLL_REGION;
	Terminal_PI.xl=21;
	Terminal_PI.yl=32;
	Terminal_PI.xh=277;
	Terminal_PI.yh=134;
	Terminal_PI.ltcolor=255;
	Terminal_PI.bgcolor=TERM_BCOLOR;
	Terminal_PI.dkcolor=255;
	Terminal_PI.shcolor=TERM_TSHAD_COLOR;
	Terminal_PI.fontnumber=2;
	Terminal_PI.cur_x = -1;
	Terminal_PI.print_delay=1;


	#ifndef TERM_BUFFERED_DISPLAY
	bufferofs = displayofs;
	#endif

	ClearMemory();

	oldwidth = viewwidth/16;
	if (oldwidth != FULL_VIEW_WIDTH)
		NewViewSize(FULL_VIEW_WIDTH);


	DrawPlayScreen(false);

	StopMusic();

	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT+FONT2);						// Medium font

	BMAmsg(TERM_MSG);

	CacheDrawPic(TERM_BACK_XOFS,TERM_BACK_YOFS,TERMINAL_SCREENPIC);

   LoadTerminalText();
   LoadTerminalCommands();

	#ifdef TERM_BUFFERED_DISPLAY
	VW_UpdateScreen();
	#endif


	fontnumber = 2;
	allcaps = true;
	fontcolor = TERM_TCOLOR;
	tcursor_x = TERM_SCREEN_XOFS;
	tcursor_y = TERM_SCREEN_YOFS;

	//
	// Set up custom cursor
	//

	use_custom_cursor = true;
	US_CustomCursor = TermCursor;

	//
	// Start term stuff..
	//

	VW_FadeIn();

   ExitMoFo = false;

	TerminalPrint("^ST1^XX",false);	 
	if (!skiplink)
   {
	   CacheTerminalPrint(TM_LINK,false);

	   if (Keyboard[sc_H] & Keyboard[sc_O] & Keyboard[sc_T])
   	{
	   	CacheTerminalPrint(TM_CHEATER,false);
	   }
   	else
	   {
			VW_WaitVBL(1*60 + (US_RndT() % 60*2));	 

			if (gamestate.TimeCount & 0x1000)
			{
				CacheTerminalPrint(TM_LINK_BAD,false);
				IN_Ack();
				ExitMoFo = true;
			}
			else
			{
				CacheTerminalPrint(TM_LINK_OK,false);
			}
   	}
   }


	IN_ClearKeysDown();

	while (!ExitMoFo)
	{
		backcolor = TERM_BCOLOR;
		CacheTerminalPrint(TM_READY,false);

		if (US_LineInput(px+1,py,buffer,nil,true,MAX_INPUT,240+TERM_SCREEN_XOFS-px))
		{
			CacheTerminalPrint(TM_RETURN,false);
			if (*buffer)
			switch (msgnum = US_CheckParm(buffer,Commands))
			{
         	case TC_HINT:
//         	case TC_GOLDSTERN:
            case TC_JAM:
            case TC_HELP:
            case TC_APOGEE:
            case TC_THANKS:
				case TC_GOOBERS:
//          case TC_BSTONE:
				case TC_JERRY:
				case TC_MIKE:
				case TC_JIM:
					CacheTerminalPrint(msgnum,false);
				break;


				case TC_EXIT:
				case TC_QUIT:
				case TC_OFF:
				case TC_BYE:
					ExitMoFo = true;
				break;

            case TC_STAR:
            	CacheTerminalPrint(TM_STAR,false);
            break;

				case TC_JOSHUA:
					CacheTerminalPrint(TM_JOSHUA,false);
//            	PowerBall = 1;
            break;

            case TC_BLUEPRINT:
					FloorCheat(255);
				break;

				case TC_SOUND:
					TERM_sound_on ^= 1;
					CacheTerminalPrint(TM_SOUND_OFF+TERM_sound_on,false);
				break;

				case TC_ARRIVAL_GOLDSTERN:
				{
					if (GoldsternInfo.GoldSpawned)
						CacheTerminalPrint(TM_GOLDSTERN_ARRIVED,false);
					else
					if (GoldsternInfo.flags == GS_COORDFOUND)
					{
						CacheTerminalPrint(TM_GOLDSTERN_WILL_AR,false);
						sprintf(buffer," %d^XX",GoldsternInfo.WaitTime/60);
						TerminalPrint(buffer,false);
						CacheTerminalPrint(TM_SECONDS,false);
					}
					else
					{
						if (GoldsternInfo.WaitTime)
						{
							CacheTerminalPrint(TM_GOLDSTERN_NO_PICK,false);
							sprintf(buffer," %d^XX",GoldsternInfo.WaitTime/60);
							TerminalPrint(buffer,false);
							CacheTerminalPrint(TM_SECONDS,false);
						}
						else
							CacheTerminalPrint(TM_GOLDSTERN_NO_INFO,false);
					}
				}
				break;

				case TC_DEACTIVATE_SECURITY:
				{
					objtype *obj;

					CacheTerminalPrint(TM_RESET_SECURITY,false);
					for (obj = player;obj;obj = obj->next)
					{
						if (obj->obclass == security_lightobj)
						{
							obj->temp1 = 0;
                     obj->flags &= ~FL_ALERTED;
                  }
					}
				}
				break;

				case TC_SATALITE_STATUS:
					{
						CacheTerminalPrint(TM_VITALS1,false);
						TerminalPrint(buffer,false);

						CacheTerminalPrint(TM_VITALS2,false);
						sprintf(buffer, " %d\r\n^XX", gamestate.VitalsRemain);
						TerminalPrint(buffer,false);
					}
					break;

				case TC_PROFILE:
					CacheTerminalPrint(TM_PROFILE_WHO,false);
					if (US_LineInput(px+1,py,buffer,nil,true,MAX_INPUT,246+TERM_SCREEN_XOFS-px))
					{
						CacheTerminalPrint(TM_RETURN,false);
						if (*buffer)
							switch (US_CheckParm(buffer,Commands))
							{
								case TC_GOLDSTERN:
									CacheTerminalPrint(TM_PROFILE_GOLDSTERN,false);
								break;

								case TC_BSTONE:
									CacheTerminalPrint(TM_PROFILE_BLAKE,false);
								break;

								case TC_SSTONE:
									CacheTerminalPrint(TM_PROFILE_SARA,false);
								break;

								default:
									CacheTerminalPrint(TM_PROFILE_UNKNOWN,false);
								break;
							}
					}
				break;

				default:
					CacheTerminalPrint(TM_UNRECOGNIZED_COMMAND,false);
				break;
			}
		}
		else
		{
			// User pressed escape....

			ExitMoFo = true;
		}

		#ifdef TERM_BUFFERED_DISPLAY
			VW_UpdateScreen();
		#endif
	}

	//
	// Free everything cached in...Exit terminal
	//

   FreeTerminalCommands();
   FreeTerminalMessages();

	UNCACHEGRCHUNK(STARTFONT+1);
	NewViewSize(oldwidth);


	StartMusic(false);
	PM_CheckMainMem();

	DrawPlayScreen(false);

	IN_ClearKeysDown();
	allcaps = temp_caps;
	use_custom_cursor = false;

}


//---------------------------------------------------------------------------
// FloorCheat()
//---------------------------------------------------------------------------
void FloorCheat(unsigned RadarFlags)
{
	#define FC_EMBED_COLOR(ColorCodes)	{_fstrncpy(&pbuffer[pos],ColorCodes,5);pos+=5;}
	#define FC_NORM_COLOR()					FC_EMBED_COLOR("^FC57")

	unsigned x,y,pos;
	objtype *actor;
   char far *pbuffer;
   memptr buffer;

	MM_GetPtr(&buffer,512);

	pbuffer = buffer;

	CacheTerminalPrint(TM_BLUEPRINTS,false);

	shadow_text = term_cursor_vis = false;
	Terminal_PI.flags &= ~TPF_SHOW_CURSOR;

	//
	// Cache in the "Radar Font"
	//

	CA_CacheGrChunk(STARTFONT+5);
//	fontnumber = 5;
	Terminal_PI.fontnumber = 5;			

	//
	// Display the radar/floor-plans

//	TerminalPrint("\r\n^XX",true);
	for (y=0;y<64;y++)
	{
		pos = 0;
		for (x=0;x<64;x++)
		{
			//
			// Get wall/actor && Check for force placement of player on radar..
			//

			if (DebugOk && x == player->tilex && y == player->tiley)
				actor = player;
			else
				actor = actorat[x][y];

			//
			//  Check for walls

			if (!TravelTable[x][y])		// Map only shows where you've seen!
			{
				pbuffer[pos++]='!';
			}
			else
			if (((unsigned)actor && (unsigned)actor<108) ||			// 108 == LAST WALL TILE

#if IN_DEVELOPMENT

				(*(mapsegs[0]+farmapylookup[y]+x) >= HIDDENAREATILE && (!DebugOk)) ||

#endif

				(((unsigned)actor & 0x80) && actor<objlist && (!DebugOk))) 	// Treat doors as walls in NoDebug
			{
				// Mark Wall piece
				//
				pbuffer[pos++] = '!';
			}
			else
			{
				// Not a wall Piece
				//
				if (((RadarFlags & RS_PERSONNEL_TRACKER) && actor >= objlist) && (!(actor->flags & FL_DEADGUY)))
				{
					switch (actor->obclass)
					{
						case playerobj:
							if (RadarFlags & RS_PERSONNEL_TRACKER)
							{
								//
								// Mark map piece as the "player"
								//
								FC_EMBED_COLOR("^FC0F");		//  WHITE
								pbuffer[pos++] = '!';
								FC_NORM_COLOR();
							}
							else
								pbuffer[pos++] = ' ';
						break;

						case security_lightobj:
							if (RadarFlags & RS_SECURITY_STATUS)
							{
								//
								// Mark map piece as "Alerted Security Lamp"
								//
								if (actor->temp1)
								{
									FC_EMBED_COLOR("^FC1C");			// Red
								}
								else
								{
									FC_EMBED_COLOR("^FC5C");			// Green
								}

								pbuffer[pos++] = '!';
								FC_NORM_COLOR();
								break;
							}
							else
								pbuffer[pos++] = ' ';
							break;

						case lcan_wait_alienobj:
						case scan_wait_alienobj:
						case hang_terrotobj:
						case gurney_waitobj:
								pbuffer[pos++] = ' ';
						break;

						case goldsternobj:
							if (RadarFlags & RS_GOLDSTERN_TRACKER)
							{
								//
								// Mark map piece as "goldstern"
								//
								FC_EMBED_COLOR("^FC38");		//  Yellow ...or.. err, like gold!
								pbuffer[pos++] = '!';
								FC_NORM_COLOR();
								break;
							}
							else
								pbuffer[pos++] = ' ';
							break;

						default:
							if (RadarFlags & RS_PERSONNEL_TRACKER)
							{
								//
								// Mark map piece as a "general object"
								//
								FC_EMBED_COLOR("^FC18");		// Red
								pbuffer[pos++] = '!';
								FC_NORM_COLOR();
							}
							else
								pbuffer[pos++] = ' ';
						break;
					}
				}
				else
					pbuffer[pos++] = ' ';

			}

		}

//		pbuffer[pos++] = '\n';
		_fstrcpy(pbuffer+pos,"\r\n^XX");
		pbuffer[pos+5] = 0;

		TerminalPrint(pbuffer,true);
	}


	TerminalPrint("\r\n\r\n\r\n\r\n^XX",true);
	MM_FreePtr(&buffer);

	UNCACHEGRCHUNK(STARTFONT+5);
	Terminal_PI.fontnumber = 2;
	TerminalPrint("\r\n^XX",true);
	Terminal_PI.flags |= TPF_SHOW_CURSOR;

}

#endif


/*
=============================================================================

							PLAYER CONTROL

=============================================================================
*/




void SpawnPlayer (int tilex, int tiley, int dir)
{
	if (gamestuff.level[gamestate.mapon].ptilex &&
		 gamestuff.level[gamestate.mapon].ptiley)
	{
		tilex = gamestuff.level[gamestate.mapon].ptilex;
		tiley = gamestuff.level[gamestate.mapon].ptiley;
		dir = 1+(gamestuff.level[gamestate.mapon].pangle/90);
	}

	player->obclass = playerobj;
	player->active = true;
	player->tilex = tilex;
	player->tiley = tiley;

	player->areanumber=GetAreaNumber(player->tilex,player->tiley);

	player->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->angle = (1-dir)*90;
	if (player->angle<0)
		player->angle += ANGLES;
	player->flags = FL_NEVERMARK;
	Thrust (0,0);				// set some variables

	InitAreas ();

	InitWeaponBounce();
}


//===========================================================================

//------------------------------------------------------------------------
// GunAttack()
//------------------------------------------------------------------------
void	GunAttack (objtype *ob)
{
	objtype *check,*closest,*oldclosest;
	int		damage;
	int		dx,dy,dist;
	long	viewdist;
   boolean skip = false;

	if (gamestate.weapon != wp_autocharge)
	{
		MakeAlertNoise(ob);
	}

	switch (gamestate.weapon)
	{
		case wp_autocharge:
			SD_PlaySound (ATKAUTOCHARGESND);
         skip = true;
		break;

		case wp_pistol:
			SD_PlaySound (ATKCHARGEDSND);
         skip = true;
		break;

		case wp_burst_rifle:
			SD_PlaySound (ATKBURSTRIFLESND);
		break;

		case wp_ion_cannon:
			SD_PlaySound (ATKIONCANNONSND);
		break;

	}

	//
	// find potential targets
	//

	viewdist = 0x7fffffffl;
	closest = NULL;

	while (1)
	{
		oldclosest = closest;

		for (check=ob->next ; check ; check=check->next)
			if ((check->flags & FL_SHOOTABLE) &&
				 (check->flags & FL_VISABLE) &&
				 (abs(check->viewx-centerx) < shootdelta))
			{
				if (check->transx < viewdist)
				{
            	if ((skip && (check->obclass == hang_terrotobj))
					   || (check->flags2 & FL2_NOTGUNSHOOTABLE))
               	continue;

					viewdist = check->transx;
					closest = check;
				}
			}

		if (closest == oldclosest)
			return;						// no more targets, all missed

	//
	// trace a line from player to enemey
	//
		if (CheckLine(closest,player))
			break;
	}

	//
	// hit something
	//

	dx = abs(closest->tilex - player->tilex);
	dy = abs(closest->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	if (dist<2)
		damage = US_RndT() / 2;			// 4
	else if (dist<4)
		damage = US_RndT() / 4;			// 6
	else
	{
		if ( (US_RndT() / 12) < dist)		// missed
			return;
		damage = US_RndT() / 4;			// 6
	}

	DamageActor (closest,damage,player);
}


//===========================================================================




//===========================================================================


/*
===============
=
= T_Attack
=
===============
*/


void	T_Attack (objtype *ob)
{
	atkinf_t	far *cur;
	int x, wp_start;

	if (noShots)
	{
		ob->state = &s_player;
		gamestate.attackframe = gamestate.weaponframe = 0;
		return;
	}

	if (gamestate.weapon == wp_autocharge)
			UpdateAmmoMsg();

	if ( buttonstate[bt_use] && !buttonheld[bt_use] )
		buttonstate[bt_use] = false;

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		buttonstate[bt_attack] = false;

	ControlMovement (ob);

	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

//
// change frame and fire
//
	gamestate.attackcount -= tics;
	if (gamestate.attackcount <= 0)
	{
		cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
		switch (cur->attack)
		{
		case -1:
			ob->state = &s_player;

			if (!gamestate.ammo)
			{
				if (gamestate.weapon != wp_autocharge)
				{
					gamestate.weapon = wp_autocharge;
					DrawWeapon();
					DisplayInfoMsg(EnergyPackDepleted,MP_NO_MORE_AMMO,DISPLAY_MSG_STD_TIME<<1,MT_OUT_OF_AMMO);
				}
			}
			else
			{
				if (!(gamestate.useable_weapons & (1<<gamestate.weapon)))
				{
					gamestate.weapon = wp_autocharge;
					DrawWeapon();
					DisplayInfoMsg(NotEnoughEnergyForWeapon,MP_NO_MORE_AMMO,DISPLAY_MSG_STD_TIME<<1,MT_OUT_OF_AMMO);
				}
			};
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case -2:
			ob->state = &s_player;
			if (!gamestate.plasma_detonators)
			{
				// Check to see what weapons are possible.
				//

				for (x=wp_bfg_cannon;x>=wp_autocharge;x--)
				{
					if (gamestate.useable_weapons & (1<<x))
					{
						gamestate.weapon = x;
                  break;
               }
            }

				DrawWeapon();
//				DisplayInfoMsg(pd_switching,MP_NO_MORE_AMMO,DISPLAY_MSG_STD_TIME<<1,MT_OUT_OF_AMMO);
			}
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case 4:
			if (!gamestate.ammo)
				break;
			if (buttonstate[bt_attack])
				gamestate.attackframe -= 2;

      case 0:
         if (gamestate.weapon == wp_grenade)
	         if (!objfreelist)
   	      {
					DISPLAY_TIMED_MSG(WeaponMalfunction,MP_WEAPON_MALFUNCTION,MT_MALFUNCTION);
	            gamestate.attackframe++;
				}
      	break;

		case 1:
			if (!gamestate.ammo)
			{	// can only happen with chain gun
				gamestate.attackframe++;
				break;
			}
			GunAttack (ob);
			if (!godmode)
				gamestate.ammo--;
			DrawAmmo(false);
			break;

		case 2:
			if (gamestate.weapon_wait)
				break;
			GunAttack (ob);
			gamestate.weapon_wait	= AUTOCHARGE_WAIT;
			DrawAmmo(false);
			break;

		case 3:
			if (gamestate.ammo && buttonstate[bt_attack])
				gamestate.attackframe -= 2;
			break;

		case 6:
			if (gamestate.ammo && buttonstate[bt_attack])
				if (objfreelist)
					gamestate.attackframe -= 2;
            else
            {
					DISPLAY_TIMED_MSG(WeaponMalfunction,MP_WEAPON_MALFUNCTION,MT_MALFUNCTION);
            }
			break;

		case 5:
			if (!objfreelist)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction,MP_WEAPON_MALFUNCTION,MT_MALFUNCTION);
				gamestate.attackframe++;
			}
			else
			{
				if (LastMsgType == MT_MALFUNCTION)
					MsgTicsRemain = 1;		// Clear Malfuction Msg before anim

				if (!godmode)
				{
					if (gamestate.ammo >= GRENADE_ENERGY_USE)
					{
						gamestate.ammo-=GRENADE_ENERGY_USE;
						DrawAmmo(false);
					}
					else
						gamestate.attackframe++;
				}
				SD_PlaySound(ATKGRENADESND);
				SpawnProjectile(ob,grenadeobj);
				MakeAlertNoise(ob);
			}
		break;


		case 7:
	   	TryDropPlasmaDetonator();
     		DrawAmmo(false);
	      break;

      case 8:
			if (gamestate.plasma_detonators && buttonstate[bt_attack])
				gamestate.attackframe -= 2;
			break;

		case 9:
			if (!objfreelist)
			{
				DISPLAY_TIMED_MSG(WeaponMalfunction,MP_WEAPON_MALFUNCTION,MT_MALFUNCTION);
				gamestate.attackframe++;
			}
			else
			{
				if (LastMsgType == MT_MALFUNCTION)
					MsgTicsRemain = 1;		// Clear Malfuction Msg before anim

				if (!godmode)
				{
					if (gamestate.ammo >= BFG_ENERGY_USE)
					{
						gamestate.ammo-=BFG_ENERGY_USE;
						DrawAmmo(false);
					}
					else
						gamestate.attackframe++;
				}
				SD_PlaySound(ATKIONCANNONSND);				// JTR - this needs to change
				SpawnProjectile(ob,bfg_shotobj);
				MakeAlertNoise(ob);
			}
		break;

		case 10:
			if (gamestate.ammo && buttonstate[bt_attack])
				if (objfreelist)
					gamestate.attackframe -= 2;
            else
            {
					DISPLAY_TIMED_MSG(WeaponMalfunction,MP_WEAPON_MALFUNCTION,MT_MALFUNCTION);
            }
		break;

		}

		gamestate.attackcount += cur->tics;
		gamestate.attackframe++;
		gamestate.weaponframe =
			attackinfo[gamestate.weapon][gamestate.attackframe].frame;
	}
}


//===========================================================================

/*
===============
=
= T_Player
=
===============
*/

void	T_Player (objtype *ob)
{
	CheckWeaponChange ();

	if (gamestate.weapon == wp_autocharge)
		UpdateAmmoMsg();

	if (tryDetonatorDelay > tics)
		tryDetonatorDelay -= tics;
	else
		tryDetonatorDelay = 0;

	if ( buttonstate[bt_use] )
	{
		Cmd_Use();
		SD_PlaySound(HITWALLSND);
	}

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		Cmd_Fire ();

	ControlMovement (ob);
	HandleWeaponBounce();


//	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in unsigned
//	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;
}

#if 0
//-------------------------------------------------------------------------
// RunBlakeRun()
//-------------------------------------------------------------------------
void RunBlakeRun()
{
	#define	BLAKE_SPEED	(MOVESCALE*50)

	long xmove,ymove,speed;
	objtype *blake;
	short startx,starty,dx,dy;

// Spawn Blake and set pointer.
//
	SpawnPatrol(en_blake,player->tilex,player->tiley,player->dir>>1);
	blake=new;

// Blake object starts one tile behind player object.
//
	switch (blake->dir)
	{
		case north:
			blake->tiley+=2;
		break;

		case south:
			blake->tiley-=2;
		break;

		case east:
			blake->tilex-=2;
		break;

		case west:
			blake->tilex+=2;
		break;
	}

// Align Blake on the middle of the tile.
//
	blake->x = ((long)blake->tilex<<TILESHIFT)+TILEGLOBAL/2;
	blake->y = ((long)blake->tiley<<TILESHIFT)+TILEGLOBAL/2;
	startx=blake->tilex = blake->x >> TILESHIFT;
	starty=blake->tiley = blake->y >> TILESHIFT;

// Run, Blake, Run!
//
	do
	{
	// Calc movement in X and Y directions.
	//
		xmove = FixedByFrac(BLAKE_SPEED,costable[player->angle]);
		ymove = -FixedByFrac(BLAKE_SPEED,sintable[player->angle]);

	// Move, animate, and redraw.
	//
		if (ClipMove(blake,xmove,ymove))
			break;
		DoActor(blake);
		ThreeDRefresh();

	// Calc new tile X/Y.
	//
		blake->tilex = blake->x >> TILESHIFT;
		blake->tiley = blake->y >> TILESHIFT;

	// Evaluate distance from start.
	//
		dx=blake->tilex-startx;
		dx=ABS(dx);
		dy=blake->tiley-starty;
		dy=ABS(dy);

	} while ((dx < 6) && (dy < 6));
}

#endif

//-------------------------------------------------------------------------
// SW_HandleActor() - Handle all actors connected to a smart switch.
//-------------------------------------------------------------------------
void SW_HandleActor(objtype *obj)
{
   if (!obj->active)
   	obj->active = ac_yes;

  	switch (obj->obclass)
   {
		case rentacopobj:
		case gen_scientistobj:
		case swatobj:
		case goldsternobj:
		case proguardobj:
         if (!(obj->flags & (FL_ATTACKMODE|FL_FIRSTATTACK)))
         	FirstSighting(obj);
      break;

		case morphing_spider_mutantobj:
   	case morphing_reptilian_warriorobj:
		case morphing_mutanthuman2obj:
      case crate1obj:
      case crate2obj:
      case crate3obj:
      case podeggobj:
			KillActor(obj);
      break;

		case gurney_waitobj:
		case scan_wait_alienobj:
		case lcan_wait_alienobj:
      break;

//		case electrosphereobj:
//    break;

		case floatingbombobj:
		case volatiletransportobj:
			if (obj->flags & FL_STATIONARY)
         	KillActor(obj);
         else
         if (!(obj->flags & (FL_ATTACKMODE|FL_FIRSTATTACK)))
  	      	FirstSighting(obj);
      break;

		case spider_mutantobj:
		case breather_beastobj:
		case cyborg_warriorobj:
		case reptilian_warriorobj:
		case acid_dragonobj:
		case mech_guardianobj:
		case liquidobj:
		case genetic_guardobj:
		case mutant_human1obj:
		case mutant_human2obj:
		case lcan_alienobj:
		case scan_alienobj:
		case gurneyobj:
		case podobj:
		case final_boss1obj:
		case final_boss2obj:
		case final_boss3obj:
		case final_boss4obj:
         if (!(obj->flags & (FL_ATTACKMODE|FL_FIRSTATTACK)))
         	FirstSighting(obj);
      break;

//		case electroobj:
//		case liquidobj:
//    break;

		case post_barrierobj:
		case arc_barrierobj:
      break;
   }
}


//-------------------------------------------------------------------------
// SW_HandleStatic() - Handle all statics connected to a smart switch.
//-------------------------------------------------------------------------
void SW_HandleStatic(statobj_t *stat, unsigned tilex, unsigned tiley)
{
	switch (stat->itemnumber)
   {
     	case bo_clip:
     	case bo_clip2:
			SpawnCusExplosion((((fixed)tilex)<<TILESHIFT)+0x7FFF,
									(((fixed)tiley)<<TILESHIFT)+0x7FFF,
									SPR_CLIP_EXP1, 7, 30+(US_RndT()&0x27),explosionobj);
			stat->shapenum = -1;
			stat->itemnumber = bo_nothing;
      break;
   }
}


//-------------------------------------------------------------------------
// OperateSmartSwitch() - Operates a Smart Switch
//
// PARAMETERS:
//			tilex - Tile X coord that the Smart switch points to.
//			tiley - Tile Y coord that the Smart switch points to.
//       force - Force switch operation.  Will not check the players current
//               and last tilex & tiley coords.  This is usefull for other
//               actors toggling barrier switches.
//
// RETURNS: Boolean: TRUE  - Remove switch from map
//							FALSE - Keep switch in map
//
//-------------------------------------------------------------------------
boolean OperateSmartSwitch(unsigned tilex, unsigned tiley, char Operation, boolean Force)
{
	typedef enum
   {
   	wit_NOTHING,
		wit_DOOR,
		wit_WALL,
		wit_STATIC,
		wit_ACTOR,
   } what_is_it;

	what_is_it WhatItIs;
   objtype *obj;
   statobj_t *stat;
   unsigned char tile, DoorNum;
   unsigned iconnum;

	//
   // Get some information about what
   // this switch is pointing to.
   //

   tile = tilemap[tilex][tiley];
   obj = actorat[tilex][tiley];
	iconnum = *(mapsegs[1]+farmapylookup[tiley]+tilex);
   WhatItIs  = wit_NOTHING;

   //
   // Deterimine if the switch points to an
	// actor, door, wall, static or is Special.
   //

   if (obj < objlist)
	{
		if (obj == (objtype *)1 && tile == 0)
		{
			// We have a SOLID static!

         WhatItIs = wit_STATIC;
  	   }
     	else
		{
      	if (tile)
         {
         	//
            // We have a wall of some type (maybe a door).
            //

	         if (tile & 0x80)
   	      {
	         	// We have a door

	   	      WhatItIs = wit_DOOR;
         	   DoorNum = tile & 0x7f;
	         }
   	      else
      	   {
         		// We have a wall

	   	      WhatItIs = wit_WALL;
   	      }
         }
			else
         {
#pragma warn -pia
         	if (stat = FindStatic(tilex,tiley))
            	WhatItIs = wit_STATIC;
#pragma warn +pia
         }
      }
	}
	else
   {
		if (obj < &objlist[MAXACTORS])
		{
	     	// We have an actor.

         WhatItIs = wit_ACTOR;
		}
		else
		   WhatItIs  = wit_NOTHING;
   }

   //
   // Ok... Now do that voodoo that you do so well...
   //

   switch (WhatItIs)
   {
   	//
      // Handle Doors
      //
   	case wit_DOOR:
	      if (doorobjlist[DoorNum].action == dr_jammed)
   	      return(false);

	     	doorobjlist[DoorNum].lock = kt_none;
			OpenDoor(DoorNum);
     	return(false);


   	//
      // Handle Actors
      //
		case wit_ACTOR:
			if (!(obj->flags & FL_DEADGUY))
   	   	SW_HandleActor(obj);
		return(true);


   	//
      // Handle Walls
      //
      case wit_WALL:
      {
        	if (Force || player_oldtilex != player->tilex || player_oldtiley != player->tiley)
		      switch (tile)
   		   {
					case OFF_SWITCH:
	     	      	if (Operation == ST_TURN_OFF)
         	      	return(false);

						ActivateWallSwitch(iconnum, tilex, tiley);
					break;

					case ON_SWITCH:
   	         	if (Operation == ST_TURN_ON)
      	         	return(false);
						ActivateWallSwitch(iconnum, tilex, tiley);
					break;
   		   }
		}
      return(false);


   	//
		// Handle Statics
		//
		case wit_STATIC:
			SW_HandleStatic(stat,tilex,tiley);
		return (true);


		//
		// Handle NON connected smart switches...
		//
		case wit_NOTHING:
			// Actor (or something) that was to be triggered has
			// moved... SSSOOOoo, Remove the switch.
		return(true);
	}

	return(false);
}

//==========================================================================
//
//                         WEAPON BOUNCE CODE
//
//==========================================================================

#define wb_MaxPoint		((long)10 << TILESHIFT)
#define wb_MidPoint		((long)6 << TILESHIFT)
#define wb_MinPoint		((long)2 << TILESHIFT)
#define wb_MaxGoalDist	(wb_MaxPoint - wb_MidPoint)

#define wb_MaxOffset		(wb_MaxPoint+((long)2 << TILESHIFT))
#define wb_MinOffset		(wb_MinPoint-((long)2 << TILESHIFT))

extern fixed bounceOffset;

fixed bounceVel,bounceDest;
short bounceOk;

//--------------------------------------------------------------------------
// InitWeaponBounce()
//--------------------------------------------------------------------------
void InitWeaponBounce()
{
	bounceOffset = wb_MidPoint;
	bounceDest = wb_MaxPoint;
	bounceVel = bounceOk = 0;
}

//--------------------------------------------------------------------------
// HandleWeaponBounce()
//--------------------------------------------------------------------------
void HandleWeaponBounce()
{
	short bounceSpeed;

	bounceSpeed = 90-((20-viewsize)*6);

	if (bounceOk)
	{
		if (bounceOffset < bounceDest)
		{
			bounceVel += (sintable[bounceSpeed]+1) >> 1;
			bounceOffset += bounceVel;
			if (bounceOffset > bounceDest)
			{
				bounceDest = wb_MinPoint;
				bounceVel >>= 2;
			}
		}
		else
		if (bounceOffset > bounceDest)
		{
			bounceVel -= sintable[bounceSpeed] >> 2;
			bounceOffset += bounceVel;

			if (bounceOffset < bounceDest)
			{
				bounceDest = wb_MaxPoint;
				bounceVel >>= 2;
			}
		}
	}
	else
	{
		if (bounceOffset > wb_MidPoint)
		{
			bounceOffset -= ((long)2<<TILESHIFT);
			if (bounceOffset < wb_MidPoint)
				bounceOffset = wb_MidPoint;
		}
		else
		if (bounceOffset < wb_MidPoint)
		{
			bounceOffset += ((long)2<<TILESHIFT);
			if (bounceOffset > wb_MidPoint)
				bounceOffset = wb_MidPoint;
		}

		bounceDest = wb_MaxPoint;
		bounceVel = 0;
	}

	if (bounceOffset > wb_MaxOffset)
		bounceOffset = wb_MaxOffset;
	else
		if (bounceOffset < wb_MinOffset)
			bounceOffset = wb_MinOffset;
}

