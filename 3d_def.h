/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


#ifndef BSTONE_3D_DEF_INCLUDED
#define BSTONE_3D_DEF_INCLUDED


#include <cmath>

#include "id_heads.h"
#include "jm_io.h"
#include "jm_cio.h"


#define LOOK_FOR_DEAD_GUYS (0)

#define LIMITED_AMMO


enum class ScanCode;


const int BS_SAVE_VERSION = 4;

extern bool no_screens;

#define GOLD_MORPH_LEVEL (19) // Level which Dr. GoldFire Morphs.

#define VERSION_TEXT_COLOR (0x82)
#define VERSION_TEXT_BKCOLOR (0x80)

#define NO_SHADING (-128)
#define LAMP_ON_SHADING (-12)
#define EXPLOSION_SHADING (-52)

#define DISK_SPACE_NEEDED (1024L * 1024L)

#define BORDER_HI_COLOR (0x85)
#define BORDER_MED_COLOR (0x82)
#define BORDER_LO_COLOR (0x80)
#define BORDER_TEXT_COLOR (0xAF)


// FONT DEFINES
//
#define COAL_FONT (STARTFONT + 4)


/*
=============================================================================

 MACROS

=============================================================================
*/


#define MAX_RADAR_ENERGY (14400)
#define RADAR_PAK_VALUE (60 * 15)

#define MAXKEYS (1)

#define MAPSPOT(x, y, plane) (*(mapsegs[plane] + farmapylookup[y] + x))

#define ABS(x) (static_cast<int16_t>(x) > 0 ? (x) : -(x))
#define LABS(x) (static_cast<int32_t>(x) > 0 ? (x) : -(x))

#define DISPLAY_MSG_STD_TIME (5 * 60) // Tics display len
#define DISPLAY_TIMED_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), DISPLAY_MSG_STD_TIME, (type))
#define DISPLAY_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), 0, (type))


// SMART_ACTORS is the "case" used for certain switch statements.
//

#define SMART_ACTORS proguardobj : \
case rentacopobj : \
case gen_scientistobj


// Barrier Code Stuff

#define MAX_BARRIER_SWITCHES (40) // max number level wall switches


#define SLIDE_TEMP(obj) (ui16_to_actor(obj->hitpoints))

//
// M_BASE1 - represents 100 percent in 1st base
// M_BASE2 - represents 100 percent in 2nd base
// F_BASE2 - fractional portion of 2nd base
// SCALE - arbitrary scaling value (bigger number means more accurate)
//
// RETURNS: F_BASE1 - represents fractional portion of 1st base
//
// ex: RATIO(320,16,8,7)    returns  160
//
// Make sure values used won't overflow a WORD! In general, if largest number
// to be used (320 in ex: above) * (1<<SCALE) is greater than 65535, use
// LRATIO or a lower SCALE. Using a SCALE of 8 in the example above would
// overflow a WORD in some circumstances!
//
// LRATIO is to be used for larger SCALEs, thus, giving you massive accuracy!
//
#define LRATIO(M_BASE1, M_BASE2, F_BASE2, SCALE) (((int32_t)M_BASE1 * (((int32_t)F_BASE2 << SCALE) / M_BASE2)) >> SCALE)


#define MAX_INF_AREA_MSGS (6)
#define MAX_LOCATION_DESC_LEN (45)

#define DOOR_RUBBLE_STATNUM (112) // Door Rubble sprite

#define SpawnExplosion(a, b) SpawnCusExplosion((a), (b), SPR_EXPLOSION_1, 4, 5, explosionobj)
#define SpawnFlash(a, b) SpawnCusExplosion((a), (b), SPR_EXPLOSION_1, 4, 5, deadobj)


/*
=============================================================================

 GLOBAL CONSTANTS

=============================================================================
*/

#define OV_ACTORS (0x0001)
#define OV_SHOWALL (0x0002)
#define OV_KEYS (0x0004)
#define OV_PUSHWALLS (0x0008)

// BBi
#define OV_WHOLE_MAP (0x0010)


#define TT_TRAVELED (0x01)
#define TT_KEYS (0x02)


#define MAXACTORS (150) // max number of nazis, etc / map
#define MAXSTATS (400) // max number of lamps, bonus, etc
#define MAXDOORS (64) // max number of sliding doors
#define MAXCONCESSIONS (15) // max number of concession machines
#define MAXWALLTILES (64) // max number of wall tiles
#define MAXEAWALLS (12) // max electro-alien wall sockets


#define GS_NEEDCOORD (0)
#define GS_FIRSTTIME (1)
#define GS_COORDFOUND (2)
#define GS_NO_MORE (3)

#define GOLDIE_MAX_SPAWNS (10) // Max Number of spawn points for Goldstern
#define MIN_GOLDIE_FIRST_WAIT (5 * 60) // Min wait time for FIRST goldstern (5 Secs)
#define MAX_GOLDIE_FIRST_WAIT (15 * 60) // Max wait time for FIRST goldstern (15 Secs)
#define MIN_GOLDIE_WAIT (30 * 60) // Min wait time for next goldstern (30 Secs)
#define MAX_GOLDIE_WAIT (4 * 60 * 60) // Max wait time for next goldstern (4 Mins)




//
// tile constants
//

#define ICONARROWS (90)
#define PUSHABLETILE (98)
#define EXITTILE (99) // at end of castle
#define AREATILE (108) // first of NUMAREAS floor tiles
#define HIDDENAREATILE (162) // first of hidden floor tiles
#define NUMAREAS (45)
#define DOORTRIGGERTILE (158)

#define SMART_OFF_TRIGGER (159)
#define SMART_ON_TRIGGER (160)

#define TRANSPORTERTILE (21)
#define DIRECTTRANSPORTTILE (32) // Wall to use on direct level transporters
#define SODATILE (15)
#define FOODTILE (18)
#define AMBUSHTILE (106)

#define RKEY_TILE (72)
#define YKEY_TILE (73)
#define BKEY_TILE (74)
#define BFG_TILE (75)
#define ION_TILE (76)
#define DETONATOR_TILE (77)
#define CLOAK_TILE (78)
#define LINC_TILE (79)
#define CLOAK_AMBUSH_TILE (80)

#define EATILE (24)
#define ON_SWITCH (45) // ON Wall Switch -
#define OFF_SWITCH (57) // OFF Wall Switch -
#define WINTIGGERTILE (157) // Win Tile

#define START_TEXTURES (125) // Start of Textures - (Also LAST_WALL_TILE NUM)
#define NUM_TILES (PMSpriteStart)


// ----------------

#define EXTRAPOINTS (400000)

#define MAX_EXTRA_LIVES (4)

#define RUNSPEED (6000)

#define SCREENBWIDE (::vga_ref_width)

#define HEIGHTRATIO (0.41)
#define TOP_STRIP_HEIGHT (16) // Pix height of top strip.

#define PLAYERSIZE (MINDIST) // player radius
#define MINACTORDIST (0x10000L) // minimum dist from player center
// to any actor center

#define NUMLATCHPICS (100)


#define PI (3.141592657)

#define GLOBAL1 (1L << 16)
#define TILEGLOBAL (GLOBAL1)
#define TILESHIFT (16L)
#define UNSIGNEDSHIFT (8)

#define ANGLES (360) // must be divisable by 4
#define ANGLEQUAD (ANGLES / 4)
#define FINEANGLES (3600)

#define MINDIST (0x5800L)

#define MAPSIZE (64) // maps are 64*64 max
#define NORTH (0)
#define EAST (1)
#define SOUTH (2)
#define WEST (3)

#define STATUSLINES (48)

#define SCREENSIZE (SCREENBWIDE * ::vga_ref_height)
#define PAGE1START (0)
#define PAGE2START (SCREENSIZE)
#define PAGE3START (SCREENSIZE * 2U)
#define FREESTART (SCREENSIZE * 3U)

#define STARTAMMO (8)

// Token Definations

#define MAX_TOKENS (25)



// Ammo/Weapon Definations

#define MAX_AMMO (100) // Max ammount of ammo for any weapon
#define AUTOCHARGE_WAIT (50) // Tics wait time for a full charge

#define MAX_PLASMA_DETONATORS (100) // Max number of Plasma Detonators
#define PLASMA_DETONATORS_DELAY (60 * 4) // Number of tics before plasma detonator explodes



// gamestate.flags flag values

#define GS_HEARTB_SOUND (0x0001)
#define GS_DRAW_CEILING (0x0002)
#define GS_CLIP_WALLS (0x0004)
#define GS_DRAW_FLOOR (0x0008)
#define GS_VIRGIN_LEVEL (0x0010)
#define GS_CHECK_STATS_BONUS (0x0020)
#define GS_ATTACK_INFOAREA (0x0040)
#define GS_KILL_INF_WARN (0x0080)
#define GS_SHOW_OVERHEAD (0x0100)
#define GS_BAD_DIZ_FILE (0x0200)
#define GS_MUSIC_TEST (0x0400)
#define GS_LIGHTING (0x0800)
#define GS_TICS_FOR_SCORE (0x1000)
#define GS_NOWAIT (0x2000)
#define GS_STARTLEVEL (0x4000)
#define GS_QUICKRUN (0x8000)

// object flag values - Oh Shit Longs!

#define FL_SHOOTABLE (0x00000001)
#define FL_BONUS (0x00000002)
#define FL_NEVERMARK (0x00000004)
#define FL_VISABLE (0x00000008)
#define FL_ATTACKMODE (0x00000010)
#define FL_FIRSTATTACK (0x00000020)
#define FL_AMBUSH (0x00000040)
#define FL_NONMARK (0x00000080)
#define FL_SOLID (0x00000100)
#define FL_STATIONARY (0x00000200)
#define FL_FRIENDLY (0x00000400)
#define FL_DEADGUY (0x00000800)
#define FL_RUNAWAY (0x00001000)
#define FL_RUNTOSTATIC (0x00002000)
#define FL_OFFSET_STATES (0x00004000)
#define FL_INFORMANT (0x00008000)
#define FL_INTERROGATED (0x00010000)
#define FL_RANDOM_TURN (0x00020000)
#define FL_NO_SLIDE (0x00040000)
#define FL_MUST_ATTACK (0x00080000)
#define FL_ALERTED (0x00100000)
#define FL_FREEZE (0x00200000)
#define FL_HAS_AMMO (0x00400000)
#define FL_PROJ_TRANSPARENT (0x00800000)
#define FL_PROJ_CHECK_TRANSPARENT (0x01000000)
#define FL_HAS_TOKENS (0x02000000)
#define FL_LOCKED_STATE (0x04000000)
#define FL_BARRIER (0x08000000)
#define FL_SHOOTMODE (0x10000000)
#define FL_SLIDE_INIT (0x20000000)
#define FL_STORED_OBJPTR (0x40000000)
#define FL_FAKE_STATIC (0x80000000)

#define FL_BARRIER_DAMAGE (FL_HAS_TOKENS)


// object flags2 values

#define FL2_BFGSHOT_SOLID (0x0001)
#define FL2_BFG_SHOOTABLE (0x0002)

#define FL2_NOTGUNSHOOTABLE (0x0004)
#define FL2_SCARED (0x0008)
#define FL2_DROP_RKEY (0x0010)
#define FL2_DROP_YKEY (0x0020)
#define FL2_DROP_BKEY (0x0040)

#define FL2_DROP_BFG (0x0080)
#define FL2_DROP_ION (0x0100)
#define FL2_DROP_DETONATOR (0x0200)
#define FL2_CLOAKED (0x0400)
#define FL2_LINC (0x0800)
#define FL2_DAMAGE_CLOAK (0x1000)


// Run Reason Flags -- Why am I running..Duh..

#define RR_AMMO (0x0001)
#define RR_HEALTH (0x0002)
#define RR_INTERROGATED (0x0004)
#define RR_CASUAL_PICKUP (0x0008)
#define RR_SCARED (0x0010)


//
// Concession Machine Types
#define CT_FOOD 0x1
#define CT_BEVS 0x2

//
// Door Flags
//
#define DR_BLASTABLE (0x01)

//
// Smart Trigger Possiblities
//
#define ST_TURN_OFF (0x00)
#define ST_TURN_ON (0x01)
#define ST_TOGGLE (0x02)


//
//
//

#define CANT_SAVE_GAME_TXT " Can't save this game! \n" \
    "    Hard Disk FULL!"

enum ss_type {
    ss_normal,
    ss_quick,
    ss_justcalc
}; // ss_type

enum cds_io_type {
    cds_dos_print,
    cds_id_print,
    cds_menu_print
}; // cds_io_type


enum sp_type {
    sp_normal,
    sp_loading,
    sp_saving,
    sp_changeview,
    sp_teleporting
}; // sp_type

//
// sprite constants
//

extern int16_t SPR_DEMO;

//
// Static sprites
//
extern int16_t SPR_STAT_0;
extern int16_t SPR_STAT_1;
extern int16_t SPR_STAT_2;
extern int16_t SPR_STAT_3;
extern int16_t SPR_STAT_4;
extern int16_t SPR_STAT_5;
extern int16_t SPR_STAT_6;
extern int16_t SPR_STAT_7;
extern int16_t SPR_STAT_8;
extern int16_t SPR_STAT_9;
extern int16_t SPR_STAT_10;
extern int16_t SPR_STAT_11;
extern int16_t SPR_STAT_12;
extern int16_t SPR_STAT_13;
extern int16_t SPR_STAT_14;
extern int16_t SPR_STAT_15;
extern int16_t SPR_STAT_16;
extern int16_t SPR_STAT_17;
extern int16_t SPR_STAT_18;
extern int16_t SPR_STAT_19;
extern int16_t SPR_STAT_20;
extern int16_t SPR_STAT_21;
extern int16_t SPR_STAT_22;
extern int16_t SPR_STAT_23;
extern int16_t SPR_STAT_24;
extern int16_t SPR_STAT_25;
extern int16_t SPR_STAT_26;
extern int16_t SPR_STAT_27;
extern int16_t SPR_STAT_28;
extern int16_t SPR_STAT_29;
extern int16_t SPR_STAT_30;
extern int16_t SPR_STAT_31;
extern int16_t SPR_STAT_32;
extern int16_t SPR_STAT_33;
extern int16_t SPR_STAT_34; // Green Key (AOG) / Anti-Plasma Cannon (PS)
extern int16_t SPR_STAT_35;
extern int16_t SPR_STAT_36; // Gold Key (AOG) / Yellow Package? (PS)
extern int16_t SPR_STAT_37;
extern int16_t SPR_STAT_38;
extern int16_t SPR_STAT_39;
extern int16_t SPR_STAT_40;
extern int16_t SPR_STAT_41;
extern int16_t SPR_STAT_42;
extern int16_t SPR_STAT_43;
extern int16_t SPR_STAT_44;
extern int16_t SPR_STAT_45;
extern int16_t SPR_STAT_46;
extern int16_t SPR_STAT_47;
extern int16_t SPR_STAT_48;
extern int16_t SPR_STAT_49;
extern int16_t SPR_STAT_50;
extern int16_t SPR_STAT_51;
extern int16_t SPR_STAT_52;
extern int16_t SPR_STAT_53;
extern int16_t SPR_STAT_54;
extern int16_t SPR_STAT_55;
extern int16_t SPR_STAT_56;

extern int16_t SPR_CRATE_1;
extern int16_t SPR_CRATE_2;
extern int16_t SPR_CRATE_3;

extern int16_t SPR_STAT_57;
extern int16_t SPR_STAT_58;
extern int16_t SPR_STAT_59;
extern int16_t SPR_STAT_60;
extern int16_t SPR_STAT_61;
extern int16_t SPR_STAT_62;
extern int16_t SPR_STAT_63;
extern int16_t SPR_STAT_64;
extern int16_t SPR_STAT_65;
extern int16_t SPR_STAT_66;
extern int16_t SPR_STAT_67;
extern int16_t SPR_STAT_68;
extern int16_t SPR_STAT_69;
extern int16_t SPR_STAT_70;
extern int16_t SPR_STAT_71;
extern int16_t SPR_STAT_72;
extern int16_t SPR_STAT_73;
extern int16_t SPR_STAT_74;
extern int16_t SPR_STAT_75;
extern int16_t SPR_STAT_76;
extern int16_t SPR_STAT_77;
extern int16_t SPR_STAT_78;
extern int16_t SPR_STAT_79;

extern int16_t SPR_DOORBOMB;
extern int16_t SPR_ALT_DOORBOMB;
extern int16_t SPR_RUBBLE;
extern int16_t SPR_BONZI_TREE;
extern int16_t SPR_AUTOMAPPER;
extern int16_t SPR_POT_PLANT;
extern int16_t SPR_TUBE_PLANT;
extern int16_t SPR_HITECH_CHAIR;

//
// Aesthetics
//
extern int16_t SPR_AIR_VENT;
extern int16_t SPR_BLOOD_DRIP1;
extern int16_t SPR_BLOOD_DRIP2;
extern int16_t SPR_BLOOD_DRIP3;
extern int16_t SPR_BLOOD_DRIP4;
extern int16_t SPR_WATER_DRIP1;
extern int16_t SPR_WATER_DRIP2;
extern int16_t SPR_WATER_DRIP3;
extern int16_t SPR_WATER_DRIP4;

extern int16_t SPR_DECO_ARC_1;
extern int16_t SPR_DECO_ARC_2;
extern int16_t SPR_DECO_ARC_3;

extern int16_t SPR_GRATE;
extern int16_t SPR_STEAM_1;
extern int16_t SPR_STEAM_2;
extern int16_t SPR_STEAM_3;
extern int16_t SPR_STEAM_4;

extern int16_t SPR_STEAM_PIPE;
extern int16_t SPR_PIPE_STEAM_1;
extern int16_t SPR_PIPE_STEAM_2;
extern int16_t SPR_PIPE_STEAM_3;
extern int16_t SPR_PIPE_STEAM_4;

//
// Dead Actors (from Blake Stone: AOG)
//
extern int16_t SPR_DEAD_RENT;
extern int16_t SPR_DEAD_PRO;
extern int16_t SPR_DEAD_SWAT;

//
// Sector Patrol (AOG) / Rent-A-Cop (PS)
//
extern int16_t SPR_RENT_S_1;
extern int16_t SPR_RENT_S_2;
extern int16_t SPR_RENT_S_3;
extern int16_t SPR_RENT_S_4;
extern int16_t SPR_RENT_S_5;
extern int16_t SPR_RENT_S_6;
extern int16_t SPR_RENT_S_7;
extern int16_t SPR_RENT_S_8;

extern int16_t SPR_RENT_W1_1;
extern int16_t SPR_RENT_W1_2;
extern int16_t SPR_RENT_W1_3;
extern int16_t SPR_RENT_W1_4;
extern int16_t SPR_RENT_W1_5;
extern int16_t SPR_RENT_W1_6;
extern int16_t SPR_RENT_W1_7;
extern int16_t SPR_RENT_W1_8;

extern int16_t SPR_RENT_W2_1;
extern int16_t SPR_RENT_W2_2;
extern int16_t SPR_RENT_W2_3;
extern int16_t SPR_RENT_W2_4;
extern int16_t SPR_RENT_W2_5;
extern int16_t SPR_RENT_W2_6;
extern int16_t SPR_RENT_W2_7;
extern int16_t SPR_RENT_W2_8;

extern int16_t SPR_RENT_W3_1;
extern int16_t SPR_RENT_W3_2;
extern int16_t SPR_RENT_W3_3;
extern int16_t SPR_RENT_W3_4;
extern int16_t SPR_RENT_W3_5;
extern int16_t SPR_RENT_W3_6;
extern int16_t SPR_RENT_W3_7;
extern int16_t SPR_RENT_W3_8;

extern int16_t SPR_RENT_W4_1;
extern int16_t SPR_RENT_W4_2;
extern int16_t SPR_RENT_W4_3;
extern int16_t SPR_RENT_W4_4;
extern int16_t SPR_RENT_W4_5;
extern int16_t SPR_RENT_W4_6;
extern int16_t SPR_RENT_W4_7;
extern int16_t SPR_RENT_W4_8;

extern int16_t SPR_RENT_DIE_1;
extern int16_t SPR_RENT_DIE_2;
extern int16_t SPR_RENT_DIE_3;
extern int16_t SPR_RENT_DIE_4;
extern int16_t SPR_RENT_PAIN_1;
extern int16_t SPR_RENT_DEAD;

extern int16_t SPR_RENT_SHOOT1;
extern int16_t SPR_RENT_SHOOT2;
extern int16_t SPR_RENT_SHOOT3;

//
// Star Sentinel (AOG) / Pro-Guard (PS)
//
extern int16_t SPR_PRO_S_1;
extern int16_t SPR_PRO_S_2;
extern int16_t SPR_PRO_S_3;
extern int16_t SPR_PRO_S_4;
extern int16_t SPR_PRO_S_5;
extern int16_t SPR_PRO_S_6;
extern int16_t SPR_PRO_S_7;
extern int16_t SPR_PRO_S_8;

extern int16_t SPR_PRO_W1_1;
extern int16_t SPR_PRO_W1_2;
extern int16_t SPR_PRO_W1_3;
extern int16_t SPR_PRO_W1_4;
extern int16_t SPR_PRO_W1_5;
extern int16_t SPR_PRO_W1_6;
extern int16_t SPR_PRO_W1_7;
extern int16_t SPR_PRO_W1_8;

extern int16_t SPR_PRO_W2_1;
extern int16_t SPR_PRO_W2_2;
extern int16_t SPR_PRO_W2_3;
extern int16_t SPR_PRO_W2_4;
extern int16_t SPR_PRO_W2_5;
extern int16_t SPR_PRO_W2_6;
extern int16_t SPR_PRO_W2_7;
extern int16_t SPR_PRO_W2_8;

extern int16_t SPR_PRO_W3_1;
extern int16_t SPR_PRO_W3_2;
extern int16_t SPR_PRO_W3_3;
extern int16_t SPR_PRO_W3_4;
extern int16_t SPR_PRO_W3_5;
extern int16_t SPR_PRO_W3_6;
extern int16_t SPR_PRO_W3_7;
extern int16_t SPR_PRO_W3_8;

extern int16_t SPR_PRO_W4_1;
extern int16_t SPR_PRO_W4_2;
extern int16_t SPR_PRO_W4_3;
extern int16_t SPR_PRO_W4_4;
extern int16_t SPR_PRO_W4_5;
extern int16_t SPR_PRO_W4_6;
extern int16_t SPR_PRO_W4_7;
extern int16_t SPR_PRO_W4_8;

extern int16_t SPR_PRO_PAIN_1;
extern int16_t SPR_PRO_DIE_1;
extern int16_t SPR_PRO_DIE_2;
extern int16_t SPR_PRO_DIE_3;
extern int16_t SPR_PRO_PAIN_2;
extern int16_t SPR_PRO_DIE_4;
extern int16_t SPR_PRO_DEAD;

extern int16_t SPR_PRO_SHOOT1;
extern int16_t SPR_PRO_SHOOT2;
extern int16_t SPR_PRO_SHOOT3;

//
// Star Trooper (AOG) / SWAT (PS)
//
extern int16_t SPR_SWAT_S_1;
extern int16_t SPR_SWAT_S_2;
extern int16_t SPR_SWAT_S_3;
extern int16_t SPR_SWAT_S_4;
extern int16_t SPR_SWAT_S_5;
extern int16_t SPR_SWAT_S_6;
extern int16_t SPR_SWAT_S_7;
extern int16_t SPR_SWAT_S_8;

extern int16_t SPR_SWAT_W1_1;
extern int16_t SPR_SWAT_W1_2;
extern int16_t SPR_SWAT_W1_3;
extern int16_t SPR_SWAT_W1_4;
extern int16_t SPR_SWAT_W1_5;
extern int16_t SPR_SWAT_W1_6;
extern int16_t SPR_SWAT_W1_7;
extern int16_t SPR_SWAT_W1_8;

extern int16_t SPR_SWAT_W2_1;
extern int16_t SPR_SWAT_W2_2;
extern int16_t SPR_SWAT_W2_3;
extern int16_t SPR_SWAT_W2_4;
extern int16_t SPR_SWAT_W2_5;
extern int16_t SPR_SWAT_W2_6;
extern int16_t SPR_SWAT_W2_7;
extern int16_t SPR_SWAT_W2_8;

extern int16_t SPR_SWAT_W3_1;
extern int16_t SPR_SWAT_W3_2;
extern int16_t SPR_SWAT_W3_3;
extern int16_t SPR_SWAT_W3_4;
extern int16_t SPR_SWAT_W3_5;
extern int16_t SPR_SWAT_W3_6;
extern int16_t SPR_SWAT_W3_7;
extern int16_t SPR_SWAT_W3_8;

extern int16_t SPR_SWAT_W4_1;
extern int16_t SPR_SWAT_W4_2;
extern int16_t SPR_SWAT_W4_3;
extern int16_t SPR_SWAT_W4_4;
extern int16_t SPR_SWAT_W4_5;
extern int16_t SPR_SWAT_W4_6;
extern int16_t SPR_SWAT_W4_7;
extern int16_t SPR_SWAT_W4_8;

extern int16_t SPR_SWAT_PAIN_1;
extern int16_t SPR_SWAT_DIE_1;
extern int16_t SPR_SWAT_DIE_2;
extern int16_t SPR_SWAT_DIE_3;
extern int16_t SPR_SWAT_PAIN_2;
extern int16_t SPR_SWAT_DIE_4;
extern int16_t SPR_SWAT_DEAD;

extern int16_t SPR_SWAT_SHOOT1;
extern int16_t SPR_SWAT_SHOOT2;
extern int16_t SPR_SWAT_SHOOT3;

extern int16_t SPR_SWAT_WOUNDED1;
extern int16_t SPR_SWAT_WOUNDED2;
extern int16_t SPR_SWAT_WOUNDED3;
extern int16_t SPR_SWAT_WOUNDED4;

//
// General Scientist
//
extern int16_t SPR_OFC_S_1;
extern int16_t SPR_OFC_S_2;
extern int16_t SPR_OFC_S_3;
extern int16_t SPR_OFC_S_4;
extern int16_t SPR_OFC_S_5;
extern int16_t SPR_OFC_S_6;
extern int16_t SPR_OFC_S_7;
extern int16_t SPR_OFC_S_8;

extern int16_t SPR_OFC_W1_1;
extern int16_t SPR_OFC_W1_2;
extern int16_t SPR_OFC_W1_3;
extern int16_t SPR_OFC_W1_4;
extern int16_t SPR_OFC_W1_5;
extern int16_t SPR_OFC_W1_6;
extern int16_t SPR_OFC_W1_7;
extern int16_t SPR_OFC_W1_8;

extern int16_t SPR_OFC_W2_1;
extern int16_t SPR_OFC_W2_2;
extern int16_t SPR_OFC_W2_3;
extern int16_t SPR_OFC_W2_4;
extern int16_t SPR_OFC_W2_5;
extern int16_t SPR_OFC_W2_6;
extern int16_t SPR_OFC_W2_7;
extern int16_t SPR_OFC_W2_8;

extern int16_t SPR_OFC_W3_1;
extern int16_t SPR_OFC_W3_2;
extern int16_t SPR_OFC_W3_3;
extern int16_t SPR_OFC_W3_4;
extern int16_t SPR_OFC_W3_5;
extern int16_t SPR_OFC_W3_6;
extern int16_t SPR_OFC_W3_7;
extern int16_t SPR_OFC_W3_8;

extern int16_t SPR_OFC_W4_1;
extern int16_t SPR_OFC_W4_2;
extern int16_t SPR_OFC_W4_3;
extern int16_t SPR_OFC_W4_4;
extern int16_t SPR_OFC_W4_5;
extern int16_t SPR_OFC_W4_6;
extern int16_t SPR_OFC_W4_7;
extern int16_t SPR_OFC_W4_8;

extern int16_t SPR_OFC_PAIN_1;
extern int16_t SPR_OFC_DIE_1;
extern int16_t SPR_OFC_DIE_2;
extern int16_t SPR_OFC_DIE_3;
extern int16_t SPR_OFC_PAIN_2;
extern int16_t SPR_OFC_DIE_4;
extern int16_t SPR_OFC_DEAD;

extern int16_t SPR_OFC_SHOOT1;
extern int16_t SPR_OFC_SHOOT2;
extern int16_t SPR_OFC_SHOOT3;

//
// Bad Boy Dr. Goldstern
//
extern int16_t SPR_GOLD_S_1;
extern int16_t SPR_GOLD_S_2;
extern int16_t SPR_GOLD_S_3;
extern int16_t SPR_GOLD_S_4;
extern int16_t SPR_GOLD_S_5;
extern int16_t SPR_GOLD_S_6;
extern int16_t SPR_GOLD_S_7;
extern int16_t SPR_GOLD_S_8;

extern int16_t SPR_GOLD_W1_1;
extern int16_t SPR_GOLD_W1_2;
extern int16_t SPR_GOLD_W1_3;
extern int16_t SPR_GOLD_W1_4;
extern int16_t SPR_GOLD_W1_5;
extern int16_t SPR_GOLD_W1_6;
extern int16_t SPR_GOLD_W1_7;
extern int16_t SPR_GOLD_W1_8;

extern int16_t SPR_GOLD_W2_1;
extern int16_t SPR_GOLD_W2_2;
extern int16_t SPR_GOLD_W2_3;
extern int16_t SPR_GOLD_W2_4;
extern int16_t SPR_GOLD_W2_5;
extern int16_t SPR_GOLD_W2_6;
extern int16_t SPR_GOLD_W2_7;
extern int16_t SPR_GOLD_W2_8;

extern int16_t SPR_GOLD_W3_1;
extern int16_t SPR_GOLD_W3_2;
extern int16_t SPR_GOLD_W3_3;
extern int16_t SPR_GOLD_W3_4;
extern int16_t SPR_GOLD_W3_5;
extern int16_t SPR_GOLD_W3_6;
extern int16_t SPR_GOLD_W3_7;
extern int16_t SPR_GOLD_W3_8;

extern int16_t SPR_GOLD_W4_1;
extern int16_t SPR_GOLD_W4_2;
extern int16_t SPR_GOLD_W4_3;
extern int16_t SPR_GOLD_W4_4;
extern int16_t SPR_GOLD_W4_5;
extern int16_t SPR_GOLD_W4_6;
extern int16_t SPR_GOLD_W4_7;
extern int16_t SPR_GOLD_W4_8;

extern int16_t SPR_GOLD_PAIN_1;

extern int16_t SPR_GOLD_WRIST_1;
extern int16_t SPR_GOLD_WRIST_2;

extern int16_t SPR_GOLD_SHOOT1;
extern int16_t SPR_GOLD_SHOOT2;
extern int16_t SPR_GOLD_SHOOT3;

extern int16_t SPR_GOLD_WARP1;
extern int16_t SPR_GOLD_WARP2;
extern int16_t SPR_GOLD_WARP3;
extern int16_t SPR_GOLD_WARP4;
extern int16_t SPR_GOLD_WARP5;

extern int16_t SPR_GOLD_DEATH1;
extern int16_t SPR_GOLD_DEATH2;
extern int16_t SPR_GOLD_DEATH3;
extern int16_t SPR_GOLD_DEATH4;
extern int16_t SPR_GOLD_DEATH5;
extern int16_t SPR_MGOLD_OUCH;

extern int16_t SPR_GOLD_MORPH1;
extern int16_t SPR_GOLD_MORPH2;
extern int16_t SPR_GOLD_MORPH3;
extern int16_t SPR_GOLD_MORPH4;
extern int16_t SPR_GOLD_MORPH5;
extern int16_t SPR_GOLD_MORPH6;
extern int16_t SPR_GOLD_MORPH7;
extern int16_t SPR_GOLD_MORPH8;

extern int16_t SPR_MGOLD_WALK1;
extern int16_t SPR_MGOLD_WALK2;
extern int16_t SPR_MGOLD_WALK3;
extern int16_t SPR_MGOLD_WALK4;
extern int16_t SPR_MGOLD_ATTACK1;
extern int16_t SPR_MGOLD_ATTACK2;
extern int16_t SPR_MGOLD_ATTACK3;
extern int16_t SPR_MGOLD_ATTACK4;

extern int16_t SPR_MGOLD_SHOT1;
extern int16_t SPR_MGOLD_SHOT2;
extern int16_t SPR_MGOLD_SHOT3;
extern int16_t SPR_MGOLD_SHOT_EXP1;
extern int16_t SPR_MGOLD_SHOT_EXP2;
extern int16_t SPR_MGOLD_SHOT_EXP3;

//
// Volatile Material Transporter
//
extern int16_t SPR_GSCOUT_W1_1;
extern int16_t SPR_GSCOUT_W1_2;
extern int16_t SPR_GSCOUT_W1_3;
extern int16_t SPR_GSCOUT_W1_4;
extern int16_t SPR_GSCOUT_W1_5;
extern int16_t SPR_GSCOUT_W1_6;
extern int16_t SPR_GSCOUT_W1_7;
extern int16_t SPR_GSCOUT_W1_8;

extern int16_t SPR_GSCOUT_W2_1;
extern int16_t SPR_GSCOUT_W2_2;
extern int16_t SPR_GSCOUT_W2_3;
extern int16_t SPR_GSCOUT_W2_4;
extern int16_t SPR_GSCOUT_W2_5;
extern int16_t SPR_GSCOUT_W2_6;
extern int16_t SPR_GSCOUT_W2_7;
extern int16_t SPR_GSCOUT_W2_8;

extern int16_t SPR_GSCOUT_W3_1;
extern int16_t SPR_GSCOUT_W3_2;
extern int16_t SPR_GSCOUT_W3_3;
extern int16_t SPR_GSCOUT_W3_4;
extern int16_t SPR_GSCOUT_W3_5;
extern int16_t SPR_GSCOUT_W3_6;
extern int16_t SPR_GSCOUT_W3_7;
extern int16_t SPR_GSCOUT_W3_8;

extern int16_t SPR_GSCOUT_W4_1;
extern int16_t SPR_GSCOUT_W4_2;
extern int16_t SPR_GSCOUT_W4_3;
extern int16_t SPR_GSCOUT_W4_4;
extern int16_t SPR_GSCOUT_W4_5;
extern int16_t SPR_GSCOUT_W4_6;
extern int16_t SPR_GSCOUT_W4_7;
extern int16_t SPR_GSCOUT_W4_8;

extern int16_t SPR_GSCOUT_DIE1;
extern int16_t SPR_GSCOUT_DIE2;
extern int16_t SPR_GSCOUT_DIE3;
extern int16_t SPR_GSCOUT_DIE4;
extern int16_t SPR_GSCOUT_DIE5;
extern int16_t SPR_GSCOUT_DIE6;
extern int16_t SPR_GSCOUT_DIE7;
extern int16_t SPR_GSCOUT_DIE8;

extern int16_t SPR_GSCOUT_DEAD;

//
// Perscan Drone
//
extern int16_t SPR_FSCOUT_W1_1;
extern int16_t SPR_FSCOUT_W1_2;
extern int16_t SPR_FSCOUT_W1_3;
extern int16_t SPR_FSCOUT_W1_4;
extern int16_t SPR_FSCOUT_W1_5;
extern int16_t SPR_FSCOUT_W1_6;
extern int16_t SPR_FSCOUT_W1_7;
extern int16_t SPR_FSCOUT_W1_8;

extern int16_t SPR_FSCOUT_W2_1;
extern int16_t SPR_FSCOUT_W2_2;
extern int16_t SPR_FSCOUT_W2_3;
extern int16_t SPR_FSCOUT_W2_4;
extern int16_t SPR_FSCOUT_W2_5;
extern int16_t SPR_FSCOUT_W2_6;
extern int16_t SPR_FSCOUT_W2_7;
extern int16_t SPR_FSCOUT_W2_8;

extern int16_t SPR_FSCOUT_W3_1;
extern int16_t SPR_FSCOUT_W3_2;
extern int16_t SPR_FSCOUT_W3_3;
extern int16_t SPR_FSCOUT_W3_4;
extern int16_t SPR_FSCOUT_W3_5;
extern int16_t SPR_FSCOUT_W3_6;
extern int16_t SPR_FSCOUT_W3_7;
extern int16_t SPR_FSCOUT_W3_8;

extern int16_t SPR_FSCOUT_W4_1;
extern int16_t SPR_FSCOUT_W4_2;
extern int16_t SPR_FSCOUT_W4_3;
extern int16_t SPR_FSCOUT_W4_4;
extern int16_t SPR_FSCOUT_W4_5;
extern int16_t SPR_FSCOUT_W4_6;
extern int16_t SPR_FSCOUT_W4_7;
extern int16_t SPR_FSCOUT_W4_8;

extern int16_t SPR_FSCOUT_DIE1;
extern int16_t SPR_FSCOUT_DIE2;
extern int16_t SPR_FSCOUT_DIE3;
extern int16_t SPR_FSCOUT_DIE4;
extern int16_t SPR_FSCOUT_DIE5;
extern int16_t SPR_FSCOUT_DIE6;
extern int16_t SPR_FSCOUT_DIE7;
extern int16_t SPR_FSCOUT_DEAD;

//
// General Explosion Anim
//
extern int16_t SPR_EXPLOSION_1;
extern int16_t SPR_EXPLOSION_2;
extern int16_t SPR_EXPLOSION_3;
extern int16_t SPR_EXPLOSION_4;
extern int16_t SPR_EXPLOSION_5;

//
// Projection Generator
//
extern int16_t SPR_VITAL_STAND;
extern int16_t SPR_VITAL_DIE_1;
extern int16_t SPR_VITAL_DIE_2;
extern int16_t SPR_VITAL_DIE_3;
extern int16_t SPR_VITAL_DIE_4;
extern int16_t SPR_VITAL_DIE_5;
extern int16_t SPR_VITAL_DIE_6;
extern int16_t SPR_VITAL_DIE_7;
extern int16_t SPR_VITAL_DIE_8;
extern int16_t SPR_VITAL_DEAD_1;
extern int16_t SPR_VITAL_DEAD_2;
extern int16_t SPR_VITAL_DEAD_3;
extern int16_t SPR_VITAL_OUCH;

//
// Security Cube
//
extern int16_t SPR_CUBE1;
extern int16_t SPR_CUBE2;
extern int16_t SPR_CUBE3;
extern int16_t SPR_CUBE4;
extern int16_t SPR_CUBE5;
extern int16_t SPR_CUBE6;
extern int16_t SPR_CUBE7;
extern int16_t SPR_CUBE8;
extern int16_t SPR_CUBE9;
extern int16_t SPR_CUBE10;
extern int16_t SPR_CUBE_EXP1;
extern int16_t SPR_CUBE_EXP2;
extern int16_t SPR_CUBE_EXP3;
extern int16_t SPR_CUBE_EXP4;
extern int16_t SPR_CUBE_EXP5;
extern int16_t SPR_CUBE_EXP6;
extern int16_t SPR_CUBE_EXP7;
extern int16_t SPR_CUBE_EXP8;
extern int16_t SPR_DEAD_CUBE;

//
// Red Security Lights
//
extern int16_t SPR_SECURITY_NORMAL;
extern int16_t SPR_SECURITY_ALERT;

//
// P.O.D. Alien
//
extern int16_t SPR_POD_EGG;
extern int16_t SPR_POD_HATCH1;
extern int16_t SPR_POD_HATCH2;
extern int16_t SPR_POD_HATCH3;
extern int16_t SPR_POD_WALK1;
extern int16_t SPR_POD_WALK2;
extern int16_t SPR_POD_WALK3;
extern int16_t SPR_POD_WALK4;
extern int16_t SPR_POD_ATTACK1;
extern int16_t SPR_POD_ATTACK2;
extern int16_t SPR_POD_ATTACK3;
extern int16_t SPR_POD_OUCH;
extern int16_t SPR_POD_DIE1;
extern int16_t SPR_POD_DIE2;
extern int16_t SPR_POD_DIE3;
extern int16_t SPR_POD_SPIT1;
extern int16_t SPR_POD_SPIT2;
extern int16_t SPR_POD_SPIT3;

//
// Electro-Alien
//
extern int16_t SPR_ELEC_APPEAR1;
extern int16_t SPR_ELEC_APPEAR2;
extern int16_t SPR_ELEC_APPEAR3;
extern int16_t SPR_ELEC_WALK1;
extern int16_t SPR_ELEC_WALK2;
extern int16_t SPR_ELEC_WALK3;
extern int16_t SPR_ELEC_WALK4;
extern int16_t SPR_ELEC_OUCH;
extern int16_t SPR_ELEC_SHOOT1;
extern int16_t SPR_ELEC_SHOOT2;
extern int16_t SPR_ELEC_SHOOT3;
extern int16_t SPR_ELEC_DIE1;
extern int16_t SPR_ELEC_DIE2;
extern int16_t SPR_ELEC_DIE3;
extern int16_t SPR_ELEC_SHOT1;
extern int16_t SPR_ELEC_SHOT2;
extern int16_t SPR_ELEC_SHOT_EXP1;
extern int16_t SPR_ELEC_SHOT_EXP2;

//
// ElectroSphere
//
extern int16_t SPR_ELECTRO_SPHERE_ROAM1;
extern int16_t SPR_ELECTRO_SPHERE_ROAM2;
extern int16_t SPR_ELECTRO_SPHERE_ROAM3;
extern int16_t SPR_ELECTRO_SPHERE_OUCH;
extern int16_t SPR_ELECTRO_SPHERE_DIE1;
extern int16_t SPR_ELECTRO_SPHERE_DIE2;
extern int16_t SPR_ELECTRO_SPHERE_DIE3;
extern int16_t SPR_ELECTRO_SPHERE_DIE4;

//
// Genetic Guard
//
extern int16_t SPR_GENETIC_W1;
extern int16_t SPR_GENETIC_W2;
extern int16_t SPR_GENETIC_W3;
extern int16_t SPR_GENETIC_W4;
extern int16_t SPR_GENETIC_SWING1;
extern int16_t SPR_GENETIC_SWING2;
extern int16_t SPR_GENETIC_SWING3;
extern int16_t SPR_GENETIC_DEAD;
extern int16_t SPR_GENETIC_DIE1;
extern int16_t SPR_GENETIC_DIE2;
extern int16_t SPR_GENETIC_DIE3;
extern int16_t SPR_GENETIC_DIE4;
extern int16_t SPR_GENETIC_OUCH;
extern int16_t SPR_GENETIC_SHOOT1;
extern int16_t SPR_GENETIC_SHOOT2;
extern int16_t SPR_GENETIC_SHOOT3;

//
// Experimental Mech-Sentinel
//
extern int16_t SPR_MUTHUM1_W1;
extern int16_t SPR_MUTHUM1_W2;
extern int16_t SPR_MUTHUM1_W3;
extern int16_t SPR_MUTHUM1_W4;
extern int16_t SPR_MUTHUM1_SWING1;
extern int16_t SPR_MUTHUM1_SWING2;
extern int16_t SPR_MUTHUM1_SWING3;
extern int16_t SPR_MUTHUM1_DEAD;
extern int16_t SPR_MUTHUM1_DIE1;
extern int16_t SPR_MUTHUM1_DIE2;
extern int16_t SPR_MUTHUM1_DIE3;
extern int16_t SPR_MUTHUM1_DIE4;
extern int16_t SPR_MUTHUM1_OUCH;
extern int16_t SPR_MUTHUM1_SPIT1;
extern int16_t SPR_MUTHUM1_SPIT2;
extern int16_t SPR_MUTHUM1_SPIT3;

//
// Muntant human type 2
//
extern int16_t SPR_MUTHUM2_W1;
extern int16_t SPR_MUTHUM2_W2;
extern int16_t SPR_MUTHUM2_W3;
extern int16_t SPR_MUTHUM2_W4;
extern int16_t SPR_MUTHUM2_SWING1;
extern int16_t SPR_MUTHUM2_SWING2;
extern int16_t SPR_MUTHUM2_SWING3;
extern int16_t SPR_MUTHUM2_DEAD;
extern int16_t SPR_MUTHUM2_DIE1;
extern int16_t SPR_MUTHUM2_DIE2;
extern int16_t SPR_MUTHUM2_DIE3;
extern int16_t SPR_MUTHUM2_DIE4;
extern int16_t SPR_MUTHUM2_OUCH;
extern int16_t SPR_MUTHUM2_SPIT1;
extern int16_t SPR_MUTHUM2_SPIT2;
extern int16_t SPR_MUTHUM2_SPIT3;

extern int16_t SPR_MUTHUM2_MORPH1;
extern int16_t SPR_MUTHUM2_MORPH2;
extern int16_t SPR_MUTHUM2_MORPH3;
extern int16_t SPR_MUTHUM2_MORPH4;
extern int16_t SPR_MUTHUM2_MORPH5;
extern int16_t SPR_MUTHUM2_MORPH6;
extern int16_t SPR_MUTHUM2_MORPH7;
extern int16_t SPR_MUTHUM2_MORPH8;
extern int16_t SPR_MUTHUM2_MORPH9;

//
// Large Cantained Alien
//
extern int16_t SPR_LCAN_ALIEN_READY;
extern int16_t SPR_LCAN_ALIEN_B1;
extern int16_t SPR_LCAN_ALIEN_B2;
extern int16_t SPR_LCAN_ALIEN_B3;
extern int16_t SPR_LCAN_ALIEN_EMPTY;

extern int16_t SPR_LCAN_ALIEN_W1;
extern int16_t SPR_LCAN_ALIEN_W2;
extern int16_t SPR_LCAN_ALIEN_W3;
extern int16_t SPR_LCAN_ALIEN_W4;
extern int16_t SPR_LCAN_ALIEN_SWING1;
extern int16_t SPR_LCAN_ALIEN_SWING2;
extern int16_t SPR_LCAN_ALIEN_SWING3;
extern int16_t SPR_LCAN_ALIEN_DEAD;
extern int16_t SPR_LCAN_ALIEN_DIE1;
extern int16_t SPR_LCAN_ALIEN_DIE2;
extern int16_t SPR_LCAN_ALIEN_DIE3;
extern int16_t SPR_LCAN_ALIEN_DIE4;
extern int16_t SPR_LCAN_ALIEN_OUCH;
extern int16_t SPR_LCAN_ALIEN_SPIT1;
extern int16_t SPR_LCAN_ALIEN_SPIT2;
extern int16_t SPR_LCAN_ALIEN_SPIT3;

//
// Small Canister Alien
//
extern int16_t SPR_SCAN_ALIEN_READY;
extern int16_t SPR_SCAN_ALIEN_B1;
extern int16_t SPR_SCAN_ALIEN_B2;
extern int16_t SPR_SCAN_ALIEN_B3;
extern int16_t SPR_SCAN_ALIEN_EMPTY;

extern int16_t SPR_SCAN_ALIEN_W1;
extern int16_t SPR_SCAN_ALIEN_W2;
extern int16_t SPR_SCAN_ALIEN_W3;
extern int16_t SPR_SCAN_ALIEN_W4;
extern int16_t SPR_SCAN_ALIEN_SWING1;
extern int16_t SPR_SCAN_ALIEN_SWING2;
extern int16_t SPR_SCAN_ALIEN_SWING3;
extern int16_t SPR_SCAN_ALIEN_DEAD;
extern int16_t SPR_SCAN_ALIEN_DIE1;
extern int16_t SPR_SCAN_ALIEN_DIE2;
extern int16_t SPR_SCAN_ALIEN_DIE3;
extern int16_t SPR_SCAN_ALIEN_DIE4;
extern int16_t SPR_SCAN_ALIEN_OUCH;

extern int16_t SPR_SCAN_ALIEN_SPIT1;
extern int16_t SPR_SCAN_ALIEN_SPIT2;
extern int16_t SPR_SCAN_ALIEN_SPIT3;

//
// Gurney Mutant
//
extern int16_t SPR_GURNEY_MUT_READY;
extern int16_t SPR_GURNEY_MUT_B1;
extern int16_t SPR_GURNEY_MUT_B2;
extern int16_t SPR_GURNEY_MUT_B3;
extern int16_t SPR_GURNEY_MUT_EMPTY;

extern int16_t SPR_GURNEY_MUT_W1;
extern int16_t SPR_GURNEY_MUT_W2;
extern int16_t SPR_GURNEY_MUT_W3;
extern int16_t SPR_GURNEY_MUT_W4;
extern int16_t SPR_GURNEY_MUT_SWING1;
extern int16_t SPR_GURNEY_MUT_SWING2;
extern int16_t SPR_GURNEY_MUT_SWING3;
extern int16_t SPR_GURNEY_MUT_DEAD;
extern int16_t SPR_GURNEY_MUT_DIE1;
extern int16_t SPR_GURNEY_MUT_DIE2;
extern int16_t SPR_GURNEY_MUT_DIE3;
extern int16_t SPR_GURNEY_MUT_DIE4;
extern int16_t SPR_GURNEY_MUT_OUCH;

//
// Liquid Alien
//
extern int16_t SPR_LIQUID_M1;
extern int16_t SPR_LIQUID_M2;
extern int16_t SPR_LIQUID_M3;
extern int16_t SPR_LIQUID_R1;
extern int16_t SPR_LIQUID_R2;
extern int16_t SPR_LIQUID_R3;
extern int16_t SPR_LIQUID_R4;
extern int16_t SPR_LIQUID_S1;
extern int16_t SPR_LIQUID_S2;
extern int16_t SPR_LIQUID_S3;
extern int16_t SPR_LIQUID_OUCH;
extern int16_t SPR_LIQUID_DIE_1;
extern int16_t SPR_LIQUID_DIE_2;
extern int16_t SPR_LIQUID_DIE_3;
extern int16_t SPR_LIQUID_DIE_4;
extern int16_t SPR_LIQUID_DEAD;
extern int16_t SPR_LIQUID_SHOT_FLY_1;
extern int16_t SPR_LIQUID_SHOT_FLY_2;
extern int16_t SPR_LIQUID_SHOT_FLY_3;
extern int16_t SPR_LIQUID_SHOT_BURST_1;
extern int16_t SPR_LIQUID_SHOT_BURST_2;
extern int16_t SPR_LIQUID_SHOT_BURST_3;

//
// Spit Shots
//
extern int16_t SPR_SPIT1_1;
extern int16_t SPR_SPIT1_2;
extern int16_t SPR_SPIT1_3;
extern int16_t SPR_SPIT_EXP1_1;
extern int16_t SPR_SPIT_EXP1_2;
extern int16_t SPR_SPIT_EXP1_3;

extern int16_t SPR_SPIT2_1;
extern int16_t SPR_SPIT2_2;
extern int16_t SPR_SPIT2_3;
extern int16_t SPR_SPIT_EXP2_1;
extern int16_t SPR_SPIT_EXP2_2;
extern int16_t SPR_SPIT_EXP2_3;

extern int16_t SPR_SPIT3_1;
extern int16_t SPR_SPIT3_2;
extern int16_t SPR_SPIT3_3;
extern int16_t SPR_SPIT_EXP3_1;
extern int16_t SPR_SPIT_EXP3_2;
extern int16_t SPR_SPIT_EXP3_3;

//
// Hanging Turret
//
extern int16_t SPR_TERROT_1;
extern int16_t SPR_TERROT_2;
extern int16_t SPR_TERROT_3;
extern int16_t SPR_TERROT_4;
extern int16_t SPR_TERROT_5;
extern int16_t SPR_TERROT_6;
extern int16_t SPR_TERROT_7;
extern int16_t SPR_TERROT_8;

extern int16_t SPR_TERROT_FIRE_1;
extern int16_t SPR_TERROT_FIRE_2;
extern int16_t SPR_TERROT_DIE_1;
extern int16_t SPR_TERROT_DIE_2;
extern int16_t SPR_TERROT_DIE_3;
extern int16_t SPR_TERROT_DIE_4;
extern int16_t SPR_TERROT_DEAD;

//
// Player attack frames
//
extern int16_t SPR_KNIFEREADY;
extern int16_t SPR_KNIFEATK1;
extern int16_t SPR_KNIFEATK2;
extern int16_t SPR_KNIFEATK3;
extern int16_t SPR_KNIFEATK4;

extern int16_t SPR_PISTOLREADY;
extern int16_t SPR_PISTOLATK1;
extern int16_t SPR_PISTOLATK2;
extern int16_t SPR_PISTOLATK3;
extern int16_t SPR_PISTOLATK4;

extern int16_t SPR_MACHINEGUNREADY;
extern int16_t SPR_MACHINEGUNATK1;
extern int16_t SPR_MACHINEGUNATK2;
extern int16_t SPR_MACHINEGUNATK3;
extern int16_t SPR_MACHINEGUNATK4;

extern int16_t SPR_CHAINREADY;
extern int16_t SPR_CHAINATK1;
extern int16_t SPR_CHAINATK2;
extern int16_t SPR_CHAINATK3;
extern int16_t SPR_CHAINATK4;

extern int16_t SPR_GRENADEREADY;
extern int16_t SPR_GRENADEATK1;
extern int16_t SPR_GRENADEATK2;
extern int16_t SPR_GRENADEATK3;
extern int16_t SPR_GRENADEATK4;

extern int16_t SPR_GRENADE_FLY1;
extern int16_t SPR_GRENADE_FLY2;
extern int16_t SPR_GRENADE_FLY3;
extern int16_t SPR_GRENADE_FLY4;
extern int16_t SPR_GRENADE_EXPLODE1;
extern int16_t SPR_GRENADE_EXPLODE2;
extern int16_t SPR_GRENADE_EXPLODE3;
extern int16_t SPR_GRENADE_EXPLODE4;
extern int16_t SPR_GRENADE_EXPLODE5;

extern int16_t SPR_ELEC_ARC1;
extern int16_t SPR_ELEC_ARC2;
extern int16_t SPR_ELEC_ARC3;
extern int16_t SPR_ELEC_ARC4;
extern int16_t SPR_ELEC_POST1;
extern int16_t SPR_ELEC_POST2;
extern int16_t SPR_ELEC_POST3;
extern int16_t SPR_ELEC_POST4;

extern int16_t SPR_VPOST1;
extern int16_t SPR_VPOST2;
extern int16_t SPR_VPOST3;
extern int16_t SPR_VPOST4;
extern int16_t SPR_VPOST5;
extern int16_t SPR_VPOST6;
extern int16_t SPR_VPOST7;
extern int16_t SPR_VPOST8;

extern int16_t SPR_VSPIKE1;
extern int16_t SPR_VSPIKE2;
extern int16_t SPR_VSPIKE3;
extern int16_t SPR_VSPIKE4;
extern int16_t SPR_VSPIKE5;
extern int16_t SPR_VSPIKE6;
extern int16_t SPR_VSPIKE7;
extern int16_t SPR_VSPIKE8;

extern int16_t SPR_GREEN_OOZE1;
extern int16_t SPR_GREEN_OOZE2;
extern int16_t SPR_GREEN_OOZE3;
extern int16_t SPR_BLACK_OOZE1;
extern int16_t SPR_BLACK_OOZE2;
extern int16_t SPR_BLACK_OOZE3;

extern int16_t SPR_GREEN2_OOZE1;
extern int16_t SPR_GREEN2_OOZE2;
extern int16_t SPR_GREEN2_OOZE3;
extern int16_t SPR_BLACK2_OOZE1;
extern int16_t SPR_BLACK2_OOZE2;
extern int16_t SPR_BLACK2_OOZE3;

extern int16_t SPR_CANDY_BAR;
extern int16_t SPR_CANDY_WRAPER;
extern int16_t SPR_SANDWICH;
extern int16_t SPR_SANDWICH_WRAPER;

extern int16_t SPR_BLAKE_W1;
extern int16_t SPR_BLAKE_W2;
extern int16_t SPR_BLAKE_W3;
extern int16_t SPR_BLAKE_W4;

//
// Boss 1 (Spider Mutant)
//
extern int16_t SPR_BOSS1_W1;
extern int16_t SPR_BOSS1_W2;
extern int16_t SPR_BOSS1_W3;
extern int16_t SPR_BOSS1_W4;
extern int16_t SPR_BOSS1_SWING1;
extern int16_t SPR_BOSS1_SWING2;
extern int16_t SPR_BOSS1_SWING3;
extern int16_t SPR_BOSS1_DEAD;
extern int16_t SPR_BOSS1_DIE1;
extern int16_t SPR_BOSS1_DIE2;
extern int16_t SPR_BOSS1_DIE3;
extern int16_t SPR_BOSS1_DIE4;
extern int16_t SPR_BOSS1_OUCH;
extern int16_t SPR_BOSS1_PROJ1;
extern int16_t SPR_BOSS1_PROJ2;
extern int16_t SPR_BOSS1_PROJ3;
extern int16_t SPR_BOSS1_EXP1;
extern int16_t SPR_BOSS1_EXP2;
extern int16_t SPR_BOSS1_EXP3;
extern int16_t SPR_BOSS1_MORPH1;
extern int16_t SPR_BOSS1_MORPH2;
extern int16_t SPR_BOSS1_MORPH3;
extern int16_t SPR_BOSS1_MORPH4;
extern int16_t SPR_BOSS1_MORPH5;
extern int16_t SPR_BOSS1_MORPH6;
extern int16_t SPR_BOSS1_MORPH7;
extern int16_t SPR_BOSS1_MORPH8;
extern int16_t SPR_BOSS1_MORPH9;

//
// Boss 2 (Breather Beast)
//
extern int16_t SPR_BOSS2_W1;
extern int16_t SPR_BOSS2_W2;
extern int16_t SPR_BOSS2_W3;
extern int16_t SPR_BOSS2_W4;
extern int16_t SPR_BOSS2_SWING1;
extern int16_t SPR_BOSS2_SWING2;
extern int16_t SPR_BOSS2_SWING3;
extern int16_t SPR_BOSS2_DEAD;
extern int16_t SPR_BOSS2_DIE1;
extern int16_t SPR_BOSS2_DIE2;
extern int16_t SPR_BOSS2_DIE3;
extern int16_t SPR_BOSS2_DIE4;
extern int16_t SPR_BOSS2_OUCH;

//
// Boss 3 (Cyborg Warrior)
//
extern int16_t SPR_BOSS3_W1;
extern int16_t SPR_BOSS3_W2;
extern int16_t SPR_BOSS3_W3;
extern int16_t SPR_BOSS3_W4;
extern int16_t SPR_BOSS3_SWING1;
extern int16_t SPR_BOSS3_SWING2;
extern int16_t SPR_BOSS3_SWING3;
extern int16_t SPR_BOSS3_DEAD;
extern int16_t SPR_BOSS3_DIE1;
extern int16_t SPR_BOSS3_DIE2;
extern int16_t SPR_BOSS3_DIE3;
extern int16_t SPR_BOSS3_DIE4;
extern int16_t SPR_BOSS3_OUCH;

//
// Boss 4 (Reptilian Warrior)
//
extern int16_t SPR_BOSS4_W1;
extern int16_t SPR_BOSS4_W2;
extern int16_t SPR_BOSS4_W3;
extern int16_t SPR_BOSS4_W4;
extern int16_t SPR_BOSS4_SWING1;
extern int16_t SPR_BOSS4_SWING2;
extern int16_t SPR_BOSS4_SWING3;
extern int16_t SPR_BOSS4_DEAD;
extern int16_t SPR_BOSS4_DIE1;
extern int16_t SPR_BOSS4_DIE2;
extern int16_t SPR_BOSS4_DIE3;
extern int16_t SPR_BOSS4_DIE4;
extern int16_t SPR_BOSS4_OUCH;
extern int16_t SPR_BOSS4_MORPH1;
extern int16_t SPR_BOSS4_MORPH2;
extern int16_t SPR_BOSS4_MORPH3;
extern int16_t SPR_BOSS4_MORPH4;
extern int16_t SPR_BOSS4_MORPH5;
extern int16_t SPR_BOSS4_MORPH6;
extern int16_t SPR_BOSS4_MORPH7;
extern int16_t SPR_BOSS4_MORPH8;
extern int16_t SPR_BOSS4_MORPH9;

//
// Boss 5 (Acid Dragon)
//
extern int16_t SPR_BOSS5_W1;
extern int16_t SPR_BOSS5_W2;
extern int16_t SPR_BOSS5_W3;
extern int16_t SPR_BOSS5_W4;
extern int16_t SPR_BOSS5_SWING1;
extern int16_t SPR_BOSS5_SWING2;
extern int16_t SPR_BOSS5_SWING3;
extern int16_t SPR_BOSS5_DEAD;
extern int16_t SPR_BOSS5_DIE1;
extern int16_t SPR_BOSS5_DIE2;
extern int16_t SPR_BOSS5_DIE3;
extern int16_t SPR_BOSS5_DIE4;
extern int16_t SPR_BOSS5_OUCH;
extern int16_t SPR_BOSS5_PROJ1;
extern int16_t SPR_BOSS5_PROJ2;
extern int16_t SPR_BOSS5_PROJ3;
extern int16_t SPR_BOSS5_EXP1;
extern int16_t SPR_BOSS5_EXP2;
extern int16_t SPR_BOSS5_EXP3;

//
// Boss 6 (Bio-Mech Guardian)
//
extern int16_t SPR_BOSS6_W1;
extern int16_t SPR_BOSS6_W2;
extern int16_t SPR_BOSS6_W3;
extern int16_t SPR_BOSS6_W4;
extern int16_t SPR_BOSS6_SWING1;
extern int16_t SPR_BOSS6_SWING2;
extern int16_t SPR_BOSS6_SWING3;
extern int16_t SPR_BOSS6_DEAD;
extern int16_t SPR_BOSS6_DIE1;
extern int16_t SPR_BOSS6_DIE2;
extern int16_t SPR_BOSS6_DIE3;
extern int16_t SPR_BOSS6_DIE4;
extern int16_t SPR_BOSS6_OUCH;


//
// Boss 7
//
extern int16_t SPR_BOSS7_W1;
extern int16_t SPR_BOSS7_W2;
extern int16_t SPR_BOSS7_W3;
extern int16_t SPR_BOSS7_W4;
extern int16_t SPR_BOSS7_SHOOT1;
extern int16_t SPR_BOSS7_SHOOT2;
extern int16_t SPR_BOSS7_SHOOT3;
extern int16_t SPR_BOSS7_DEAD;
extern int16_t SPR_BOSS7_DIE1;
extern int16_t SPR_BOSS7_DIE2;
extern int16_t SPR_BOSS7_DIE3;
extern int16_t SPR_BOSS7_DIE4;
extern int16_t SPR_BOSS7_OUCH;

//
// Boss 8
//
extern int16_t SPR_BOSS8_W1;
extern int16_t SPR_BOSS8_W2;
extern int16_t SPR_BOSS8_W3;
extern int16_t SPR_BOSS8_W4;
extern int16_t SPR_BOSS8_SHOOT1;
extern int16_t SPR_BOSS8_SHOOT2;
extern int16_t SPR_BOSS8_SHOOT3;
extern int16_t SPR_BOSS8_DIE1;
extern int16_t SPR_BOSS8_DIE2;
extern int16_t SPR_BOSS8_DIE3;
extern int16_t SPR_BOSS8_DIE4;
extern int16_t SPR_BOSS8_DEAD;
extern int16_t SPR_BOSS8_OUCH;

//
// Boss 9
//
extern int16_t SPR_BOSS9_W1;
extern int16_t SPR_BOSS9_W2;
extern int16_t SPR_BOSS9_W3;
extern int16_t SPR_BOSS9_W4;
extern int16_t SPR_BOSS9_SHOOT1;
extern int16_t SPR_BOSS9_SHOOT2;
extern int16_t SPR_BOSS9_SHOOT3;
extern int16_t SPR_BOSS9_DIE1;
extern int16_t SPR_BOSS9_DIE2;
extern int16_t SPR_BOSS9_DIE3;
extern int16_t SPR_BOSS9_DIE4;
extern int16_t SPR_BOSS9_DEAD;
extern int16_t SPR_BOSS9_OUCH;

//
// Boss 10
//
extern int16_t SPR_BOSS10_W1;
extern int16_t SPR_BOSS10_W2;
extern int16_t SPR_BOSS10_W3;
extern int16_t SPR_BOSS10_W4;
extern int16_t SPR_BOSS10_SHOOT1;
extern int16_t SPR_BOSS10_SHOOT2;
extern int16_t SPR_BOSS10_SHOOT3;
extern int16_t SPR_BOSS10_DEAD;
extern int16_t SPR_BOSS10_DIE1;
extern int16_t SPR_BOSS10_DIE2;
extern int16_t SPR_BOSS10_DIE3;
extern int16_t SPR_BOSS10_DIE4;
extern int16_t SPR_BOSS10_OUCH;

extern int16_t SPR_BOSS10_SPIT1;
extern int16_t SPR_BOSS10_SPIT2;
extern int16_t SPR_BOSS10_SPIT3;
extern int16_t SPR_BOSS10_SPIT_EXP1;
extern int16_t SPR_BOSS10_SPIT_EXP2;
extern int16_t SPR_BOSS10_SPIT_EXP3;

//
// Plasma detonator explosion
//
extern int16_t SPR_DETONATOR_EXP1;
extern int16_t SPR_DETONATOR_EXP2;
extern int16_t SPR_DETONATOR_EXP3;
extern int16_t SPR_DETONATOR_EXP4;
extern int16_t SPR_DETONATOR_EXP5;
extern int16_t SPR_DETONATOR_EXP6;
extern int16_t SPR_DETONATOR_EXP7;
extern int16_t SPR_DETONATOR_EXP8;

//
// Clip Explosion
//
extern int16_t SPR_CLIP_EXP1;
extern int16_t SPR_CLIP_EXP2;
extern int16_t SPR_CLIP_EXP3;
extern int16_t SPR_CLIP_EXP4;
extern int16_t SPR_CLIP_EXP5;
extern int16_t SPR_CLIP_EXP6;
extern int16_t SPR_CLIP_EXP7;
extern int16_t SPR_CLIP_EXP8;

//
// BFG Cannon Weapon
//
extern int16_t SPR_BFG_WEAPON1;
extern int16_t SPR_BFG_WEAPON2;
extern int16_t SPR_BFG_WEAPON3;
extern int16_t SPR_BFG_WEAPON4;
extern int16_t SPR_BFG_WEAPON5;

//
// BFG Cannon Weapon shots
//
extern int16_t SPR_BFG_WEAPON_SHOT1;
extern int16_t SPR_BFG_WEAPON_SHOT2;
extern int16_t SPR_BFG_WEAPON_SHOT3;

//
// BFG Shot explosion
//
extern int16_t SPR_BFG_EXP1;
extern int16_t SPR_BFG_EXP2;
extern int16_t SPR_BFG_EXP3;
extern int16_t SPR_BFG_EXP4;
extern int16_t SPR_BFG_EXP5;
extern int16_t SPR_BFG_EXP6;
extern int16_t SPR_BFG_EXP7;
extern int16_t SPR_BFG_EXP8;


//
// Door Objects
//
enum doortype {
    // LOCKED DOORS

    L_METAL,
    L_METAL_SHADE,

    L_BIO,
    L_BIO_SHADE,

    L_ELEVATOR,
    L_ELEVATOR_SHADE,

    L_SPACE,
    L_SPACE_SHADE,

    L_PRISON,
    L_PRISON_SHADE,

    L_HIGH_SECURITY,
    L_HIGH_SECURITY_SHADE,

    L_ENTER_ONLY,
    L_ENTER_ONLY_SHADE,

    L_HIGH_TECH,
    L_HIGH_TECH_SHADE,


    // UNLOCKED DOORS

    UL_METAL,
    UL_METAL_SHADE,

    UL_BIO,
    UL_BIO_SHADE,

    UL_ELEVATOR,
    UL_ELEVATOR_SHADE,

    UL_SPACE,
    UL_SPACE_SHADE,

    UL_PRISON,
    UL_PRISON_SHADE,

    UL_HIGH_SECURITY,
    UL_HIGH_SECURITY_SHADE,

    UL_ENTER_ONLY,
    UL_ENTER_ONLY_SHADE,

    UL_HIGH_TECH,
    UL_HIGH_TECH_SHADE,


    // MISC DOORS

    NOEXIT,
    NOEXIT_SHADE,

    STEEL_JAM,
    STEEL_JAM_SHADE,

    SPACE_JAM,
    SPACE_JAM_SHADE,

    OFFICE_JAM,
    OFFICE_JAM_SHADE,

    BIO_JAM,
    BIO_JAM_SHADE,

    SPACE_JAM_2,
    SPACE_JAM_2_SHADE,


    // END OF DOOR LIST
    NUMDOORTYPES,
}; // doortype


//
// Breifing types - Note these are ordered to an char array in Breifing().
//
enum breifing_type {
    BT_LOSE,
    BT_WIN,
    BT_INTRO
}; // breifing_type


//
// Barrier State Transistions
//
enum barrier_state_type {
    bt_OFF,
    bt_ON,
    bt_DISABLING,
    bt_DISABLED,
    bt_OPENING, // For physical barriers
    bt_CLOSING, //            " " "
}; // barrier_state_type


/*
=============================================================================

 GLOBAL TYPES

=============================================================================
*/

using fixed = int32_t;

// Display priority is determined by the order of these bits!
// And, this order must match the PinballBonus table in AGENT.C!
//
#define B_GALIEN_DESTROYED 0x0001
#define B_SCORE_ROLLED 0x0002
#define B_ONE_MILLION 0x0004
#define B_EXTRA_MAN 0x0008
#define B_ENEMY_DESTROYED 0x0010
#define B_TOTAL_POINTS 0x0020
#define B_INFORMANTS_ALIVE 0x0040

#define BONUS_QUEUE gamestuff.level[gamestate.mapon].bonus_queue
#define BONUS_SHOWN gamestuff.level[gamestate.mapon].bonus_shown

#define PinballBonusShown(bonus) (BONUS_SHOWN & bonus)
#define ActivatePinballBonus(bonus) if (!PinballBonusShown(bonus))BONUS_QUEUE |= bonus

struct PinballBonusInfo {
    char* BonusText; // REBA text pointer
    int32_t Points; // Score for this bonus
    bool Recurring; // Appear multiple times in a single level?
    void (* func)(); // Code to execute when you get this bonus.
}; // PinballBonusInfo

struct atkinf_t {
    int8_t tics;
    int8_t attack;
    int8_t frame; // attack is 1 for gun, 2 for knife
}; // atkinf_t


// NOTE - This enum list is ORDERED!
enum movie_t {
    mv_intro,
    mv_final,
    mv_final2, // AOG Episode 3/5
    mv_final3, // AOG Episode 6
    mv_NUM_MOVIES,
}; // movie_t

enum controldir_t {
    di_north,
    di_east,
    di_south,
    di_west
}; // controldir_t

// NOTE - This enum list is ORDERED!
enum door_t {
    dr_bio,
    dr_normal,
    dr_prison,
    dr_elevator,
    dr_high_security,
    dr_office,
    dr_oneway_left,
    dr_oneway_up,
    dr_oneway_right,
    dr_oneway_down,
    dr_space
}; // door_t

enum keytype {
    kt_none = -1,
    kt_red,
    kt_yellow,
    kt_blue,
    kt_green,
    kt_gold,
    NUMKEYS,
}; // keytype

enum activetype {
    ac_badobject = -1,
    ac_no,
    ac_yes,
    ac_allways
}; // activetype

enum classtype {
    nothing,
    playerobj,
    inertobj,
    fixup_inertobj,
    deadobj,


    // BEGIN - Start of ordered list for ActorInfoMsg[] for attacking
    // actor REBA messages

    rentacopobj,
    hang_terrotobj,
    gen_scientistobj,
    podobj,
    electroobj,
    electrosphereobj,
    proguardobj,
    genetic_guardobj,
    mutant_human1obj,
    mutant_human2obj,
    lcan_wait_alienobj,
    lcan_alienobj,
    scan_wait_alienobj,
    scan_alienobj,
    gurney_waitobj,
    gurneyobj,
    liquidobj,
    swatobj,
    goldsternobj,
    gold_morphobj,
    volatiletransportobj,
    floatingbombobj,
    rotating_cubeobj,

    spider_mutantobj,
    breather_beastobj,
    cyborg_warriorobj,
    reptilian_warriorobj,
    acid_dragonobj,
    mech_guardianobj,

    final_boss1obj,
    final_boss2obj,
    final_boss3obj,
    final_boss4obj,

    blakeobj,

    crate1obj,
    crate2obj,
    crate3obj,

    green_oozeobj,
    black_oozeobj,
    green2_oozeobj,
    black2_oozeobj,
    podeggobj,

    morphing_spider_mutantobj,
    morphing_reptilian_warriorobj,
    morphing_mutanthuman2obj,

    SPACER1_OBJ,
    electroshotobj, // NON-HITPOINT objects...
    post_barrierobj,
    arc_barrierobj,
    vpost_barrierobj,
    vspike_barrierobj,
    goldmorphshotobj,

    security_lightobj,
    explosionobj,
    steamgrateobj,
    steampipeobj,

    liquidshotobj,

    lcanshotobj,
    podshotobj,
    scanshotobj,
    dogshotobj,
    mut_hum1shotobj,

    ventdripobj,
    playerspshotobj,
    flickerlightobj,

    plasma_detonatorobj,
    plasma_detonator_reserveobj,

    grenadeobj,
    bfg_shotobj,
    bfg_explosionobj,
    pd_explosionobj,

    spider_mutantshotobj,
    breather_beastshotobj,
    cyborg_warriorshotobj,
    reptilian_warriorshotobj,
    acid_dragonshotobj,
    mech_guardianshotobj,
    final_boss2shotobj,
    final_boss4shotobj,

    doorexplodeobj, // Door explosion_anim acto
    gr_explosionobj,
    gold_morphingobj

}; // classtype

// BBi
inline classtype operator++(
    classtype& a,
    int)
{
    auto result = a;
    a = static_cast<classtype>(a + 1);
    return result;
}
// BBi

//
// NOTE: When adding bonus objects - Make sure that they are added
// at the bottom of the list or that BonusMsg[] is correctly
// updated.
//

enum stat_t {
    dressing,

    bo_red_key,
    bo_yellow_key,
    bo_blue_key,

    bo_clip,
    bo_clip2,
    bo_pistol,
    bo_burst_rifle,
    bo_ion_cannon,
    bo_grenade,
    bo_bfg_cannon,

    // START of Bonus Health Ordered list

    bo_fullheal,
    bo_firstaid,
    bo_ham,
    bo_chicken,
    bo_sandwich,
    bo_candybar,
    bo_water,
    bo_water_puddle,

    // END of ordered ...

    bo_money_bag,
    bo_loot,

    bo_gold1,
    bo_gold2,
    bo_gold3,

    bo_gold,
    bo_bonus,

    bo_plainvent,
    bo_bloodvent,
    bo_watervent,

    bo_coin,
    bo_coin5,

    bo_plasma_detonator,
    bo_automapper1,

    bo_nothing,

    block,

    bo_green_key,
    bo_gold_key,
}; // stat_t

struct stattype {
    int16_t picnum;
    stat_t type;
}; // stattype

enum dirtype {
    east,
    northeast,
    north,
    northwest,
    west,
    southwest,
    south,
    southeast,
    nodir
}; // dirtype

// BBi
inline dirtype operator+=(
    dirtype& a,
    int b)
{
    a = static_cast<dirtype>(a + b);
    return a;
}

inline dirtype operator-=(
    dirtype& a,
    int b)
{
    a = static_cast<dirtype>(a - b);
    return a;
}

inline dirtype operator|=(
    dirtype& a,
    int b)
{
    a = static_cast<dirtype>(a | b);
    return a;
}

inline dirtype operator--(
    dirtype& a,
    int)
{
    auto result = a;
    a -= 1;
    return result;
}

inline dirtype operator++(
    dirtype& a,
    int)
{
    auto result = a;
    a += 1;
    return result;
}

enum enemy_t {
    en_rentacop, // Actors with hitpoints (normal actors)
    en_hang_terrot,
    en_gen_scientist,
    en_pod,
    en_electro_alien,
    en_electrosphere,
    en_proguard,
    en_genetic_guard,
    en_mutant_human1,
    en_mutant_human2,
    en_lcan_wait_alien,
    en_lcan_alien,
    en_scan_wait_alien,
    en_scan_alien,
    en_gurney_wait,
    en_gurney,
    en_liquid,
    en_swat,
    en_goldstern,
    en_gold_morph,
    en_volatiletransport,
    en_floatingbomb,
    en_rotating_cube,

    en_spider_mutant,
    en_breather_beast,
    en_cyborg_warrior,
    en_reptilian_warrior,
    en_acid_dragon,
    en_mech_guardian,

    en_final_boss1,
    en_final_boss2,
    en_final_boss3,
    en_final_boss4,

    en_blake,

    en_crate1,
    en_crate2,
    en_crate3,

    en_green_ooze,
    en_black_ooze,
    en_green2_ooze,
    en_black2_ooze,
    en_podegg,

    en_morphing_spider_mutant,
    en_morphing_reptilian_warrior,
    en_morphing_mutanthuman2,

    NUMHITENEMIES,

    en_electro_shot, // Actors WITHOUT hitpoints (abnormal actors?)
    en_post_barrier,
    en_arc_barrier,
    en_vpost_barrier,
    en_vspike_barrier,
    en_goldmorphshot,

    en_security_light,
    en_explosion,
    en_steamgrate,
    en_steampipe,

    en_liquidshot,

    en_lcanshot,
    en_podshot,
    en_scanshot,
    en_dogshot,
    en_mut_hum1shot,

    en_ventdrip,
    en_playerspshotobj,
    en_flickerlight,

    en_plasma_detonator,
    en_plasma_detonator_reserve,

    en_vertsphere, // Actor types only used for spawning.
    en_horzsphere,
    en_diagsphere,
    en_bloodvent,
    en_watervent,
    NUMENEMIES,
}; // enemy_t


#define SF_ROTATE (0x01)
#define SF_PAINFRAME (0x02)


struct objtype;


struct statetype {
    int flags;
    int shapenum; // a shapenum of -1 means get from ob->temp1
    int tictime;

    void (* think)(
        objtype* actor);

    void (* action)(
        objtype* actor);

    statetype* next;
}; // statetype


// ---------------------
//
// trivial actor structure
//
// ---------------------

struct statobj_t {
    uint8_t tilex;
    uint8_t tiley;
    uint8_t areanumber;

    uint8_t* visspot;
    int16_t shapenum; // if shapenum == -1 the obj has been removed
    uint16_t flags;
    uint8_t itemnumber;
    int8_t lighting;

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // statobj_t


// ---------------------
//
// door actor structure
//
// ---------------------

// BBi
enum DoorAction {
    dr_open,
    dr_closed,
    dr_opening,
    dr_closing,
    dr_jammed,
}; // DoorAction

struct doorobj_t {
    uint8_t tilex;
    uint8_t tiley;
    bool vertical;
    int8_t flags;
    keytype lock;
    door_t type;
    DoorAction action;
    int16_t ticcount;
    uint8_t areanumber[2];

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // doorobj_t


// --------------------
//
// thinking actor structure
//
// --------------------

struct objtype {
    uint8_t tilex;
    uint8_t tiley;
    uint8_t areanumber;

    activetype active;
    int16_t ticcount;
    classtype obclass;
    statetype* state;

    uint32_t flags;
    uint16_t flags2; // Aux flags

    int32_t distance; // if negative, wait for that door to open
    dirtype dir;
    dirtype trydir; // "bit 7" == "direction to turn" flag

    fixed x;
    fixed y;
    uint8_t s_tilex;
    uint8_t s_tiley; // s_tilex==0, running for corner

    int16_t viewx;
    uint16_t viewheight;
    fixed transx;
    fixed transy; // in global coord

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    int16_t hitpoints;

    uint8_t ammo;
    int8_t lighting;
    uint16_t linc;
    int16_t angle;
    int32_t speed;

    int16_t temp1;

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    int16_t temp2;

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    uint16_t temp3; // holds 'last door used' by 'smart' actors

    objtype* next;
    objtype* prev;


    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // objtype


enum ButtonState {
    bt_nobutton = -1,
    bt_attack = 0,
    bt_strafe,
    bt_run,
    bt_use,
    bt_ready_autocharge,
    bt_ready_pistol,
    bt_ready_burst_rifle,
    bt_ready_ion_cannon,
    bt_ready_grenade,
    bt_ready_bfg_cannon,
    bt_ready_plasma_detonators,

    bt_SPACER,

    NUMBUTTONS,
}; // ButtonState


enum weapontype {
    wp_autocharge,
    wp_pistol,
    wp_burst_rifle,
    wp_ion_cannon,
    wp_grenade,
    wp_bfg_cannon,

    wp_SPACER,
}; // weapontype

extern int NUMWEAPONS;


enum Difficulty {
    gd_baby,
    gd_easy,
    gd_medium,
    gd_hard,
}; // Difficulty


enum backgroundtype {
    ELEVATOR_BACK,
    TRANSPORTER_BACK
}; // backgroundtype


//
// General Coord (tile) structure
//
struct tilecoord_t {
    uint8_t tilex;
    uint8_t tiley;

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // tilecoord_t


// -----------------------------------
//
// barrier coord/table structure
//
// -----------------------------------
struct barrier_type {
    uint8_t level;
    tilecoord_t coord;
    uint8_t on;

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // barrier_type;

struct statsInfoType {
    int32_t total_points;
    int32_t accum_points;
    uint8_t total_enemy;
    uint8_t accum_enemy;
    uint8_t total_inf;
    uint8_t accum_inf;
    int16_t overall_floor;

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // statsInfoType

struct levelinfo {
    uint16_t bonus_queue; // bonuses that need to be shown
    uint16_t bonus_shown; // bonuses that have been shown
    bool locked;
    statsInfoType stats;
    uint8_t ptilex;
    uint8_t ptiley;
    int16_t pangle;

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // levelinfo


struct fargametype {
    using LevelInfos = std::vector<levelinfo>;

    LevelInfos old_levelinfo;
    LevelInfos level;

    fargametype();

    fargametype(
        const fargametype& that) = delete;

    fargametype& operator=(
        const fargametype& that) = delete;

    void initialize();

    void clear();

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // fargametype

struct gametype {
    int16_t turn_around;
    int16_t turn_angle;
    uint16_t flags;
    int16_t lastmapon;
    int16_t difficulty;
    int16_t mapon;
    int32_t oldscore;
    int32_t tic_score;
    int32_t score;
    int32_t nextextra;
    int16_t score_roll_wait;
    int16_t lives;
    int16_t health;
    char health_str[4];

    int16_t rpower;
    int16_t old_rpower;
    int8_t rzoom;
    int8_t radar_leds;
    int8_t lastradar_leds;

    int8_t lastammo_leds;
    int8_t ammo_leds;
    int16_t ammo;
    int16_t old_ammo;

    int16_t plasma_detonators;
    int16_t old_plasma_detonators;

    int8_t useable_weapons;
    int8_t weapons;
    int8_t weapon;
    int8_t chosenweapon;
    int8_t old_weapons[4];

    int8_t weapon_wait;
    int16_t attackframe;
    int16_t attackcount;
    int16_t weaponframe;
    int16_t episode;
    uint32_t TimeCount;
    const char* msg; // InfoArea msg...
    int8_t numkeys[NUMKEYS];
    int8_t old_numkeys[NUMKEYS];

    // BBi
    barrier_type cross_barriers[MAX_BARRIER_SWITCHES];
    // BBi

    barrier_type barrier_table[MAX_BARRIER_SWITCHES];
    barrier_type old_barrier_table[MAX_BARRIER_SWITCHES];
    uint16_t tokens;
    uint16_t old_tokens;
    bool boss_key_dropped;
    bool old_boss_key_dropped;
    int16_t wintilex;
    int16_t wintiley;


    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);

    void initialize_cross_barriers();
    void initialize_local_barriers();
    void store_local_barriers();
    void restore_local_barriers();
}; // gametype

enum exit_t {
    ex_stillplaying,
    ex_completed,
    ex_transported,
    ex_died,
    ex_warped,
    ex_resetgame,
    ex_loadedgame,
    ex_victorious,
    ex_abort,
    ex_demodone,
    ex_secretlevel,
    ex_title
}; // exit_t


struct CycleInfo {
    uint8_t init_delay;
    uint8_t delay_count;
    uint8_t firstreg;
    uint8_t lastreg;
}; // CycleInfo


struct visobj_t {
    int16_t viewx;
    int16_t viewheight;
    int16_t shapenum;
    int8_t lighting;
    bool cloaked;
}; // visobj_t


enum animtype_t {
    at_NONE = 0,
    at_CYCLE,
    at_REBOUND,
    at_ONCE,
}; // animtype_t

enum animdir_t {
    ad_FWD = 0,
    ad_REV,
}; // animdir_t


struct ofs_anim_t {
    static const int animtype_offset = 0;
    static const int animtype_size = 2;

    static const int curframe_offset = animtype_offset + animtype_size;
    static const int curframe_size = 5;

    static const int maxframe_offset = curframe_offset + curframe_size;
    static const int maxframe_size = 5;

    static const int animdir_offset = maxframe_offset + maxframe_size;
    static const int animdir_size = 1;


    template<int TOffset, int TSize>
    static uint16_t get(
        const objtype* o)
    {
        return (o->temp3 >> TOffset) & ((1 << TSize) - 1);
    }

    template<int TOffset, int TSize>
    static void set(
        const uint16_t value,
        objtype* o)
    {
        o->temp3 &= ~(((1 << TSize) - 1) << TOffset);
        o->temp3 |= ((value & ((1 << TSize) - 1)) << TOffset);
    }

    static uint16_t get_animtype(
        const objtype* o)
    {
        return get<animtype_offset, animtype_size>(o);
    }

    static void set_animtype(
        const uint16_t value,
        objtype* o)
    {
        set<animtype_offset, animtype_size>(value, o);
    }


    static uint16_t get_curframe(
        const objtype* o)
    {
        return get<curframe_offset, curframe_size>(o);
    }

    static void set_curframe(
        const uint16_t value,
        objtype* o)
    {
        set<curframe_offset, curframe_size>(value, o);
    }


    static uint16_t get_maxframe(
        const objtype* o)
    {
        return get<maxframe_offset, maxframe_size>(o);
    }

    static void set_maxframe(
        const uint16_t value,
        objtype* o)
    {
        set<maxframe_offset, maxframe_size>(value, o);
    }


    static uint16_t get_animdir(
        const objtype* o)
    {
        return get<animdir_offset, animdir_size>(o);
    }

    static void set_animdir(
        const uint16_t value,
        objtype* o)
    {
        set<animdir_offset, animdir_size>(value, o);
    }
}; // ofs_anim_t


//
// DisplayInfoMsg Priorities - Duplicate Values are Allowed.
//
// ("enum" list used simply for debuging use like Object Classes)
//

//
// Msg_Priorities - Hell.. Lets just make them all the same...

enum msg_priorities {
    MP_min_val = 0,

    MP_HEARTB_SND = 0x0200,
    MP_WALLSWITCH_OPERATE = 0x0200,

    MP_DOOR_OPERATE = 0x0200,
    MP_CONCESSION_OPERATE = 0x0200,
    MP_WEAPON_AVAIL = 0x0200,

    MP_ATTACK_INFO = 0x0200,
    MP_NO_MORE_AMMO = 0x0200,
    MP_WEAPON_MALFUNCTION = 0x0200,

    MP_INTERROGATE = 0x0200,
    MP_CONCESSION_HINT = 0x0200,
    MP_NO_MORE_TOKENS = 0x0200,
    MP_CONCESSION_OUT_ORDER = 0x0200,

    MP_BONUS = 0x0200,

    MP_TAKE_DAMAGE = 0x0200,
    MP_DETONATOR = 0x0200,

    MP_PINBALL_BONUS = 0x3000,
    MP_FLOOR_UNLOCKED = 0x3000,

    MP_POWERUP = 0x7000, // Power-Up/Game-Start Value
    MP_max_val = 0x7FFF, // DO NOT USE/EXCEED - MAX Val
}; // msg_priorities

enum infomsg_type {
    MT_NOTHING,
    MT_CLEAR,
    MT_ATTACK,
    MT_GENERAL,
    MT_OUT_OF_AMMO,
    MT_MALFUNCTION,
    MT_NO_MO_FOOD_TOKENS,
    MT_BONUS,
}; // infomsg_type


//
// Menu Instruction Text types...
//
enum inst_type {
    IT_STANDARD,
    IT_HIGHSCORES,
    IT_ENTER_HIGHSCORE,
    IT_MOUSE_SEN,

    // BBi
    IT_SOUND_VOLUME,
    IT_CONTROLS,
    IT_CONTROLS_ASSIGNING_KEY,

    MAX_INSTRUCTIONS,
}; // inst_type



#define MAX_CACHE_MSGS (30)
#define MAX_CACHE_MSG_LEN (190)

// ------------------------- BASIC STRUCTURES -----------------------------

// Basic 'message info' structure
//
struct mCacheInfo {
    uint8_t local_val; // where msg is in 'local' list
    uint8_t global_val; // where msg was in 'global' list
    char* mSeg; // pointer to message

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // mCacheInfo

// Basic 'message list' structure
//
struct mCacheList {
    int16_t NumMsgs; // number of messages
    mCacheInfo mInfo[MAX_CACHE_MSGS]; // table of message 'info'
}; // mCacheList

// ----------------------- CONCESSION STRUCTURES --------------------------

// Concession 'message info' structure
//
struct con_mCacheInfo {
    mCacheInfo mInfo;
    uint8_t type; // type of concession
    uint8_t operate_cnt; // # of times req'd to operate

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // con_mCacheInfo

// Concession 'message list' structure
//
struct concession_t {
    int16_t NumMsgs; // also, num concessions
    con_mCacheInfo cmInfo[MAX_CACHE_MSGS];

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // concession_t

// ------------------------ INFORMANT STRUCTURES --------------------------

// Informant 'message info' structure
//
struct sci_mCacheInfo {
    mCacheInfo mInfo;
    uint8_t areanumber; // 'where' msg can be used
}; // sci_mCacheInfo

// Informant 'message list' structure
//
struct scientist_t {
    int16_t NumMsgs;
    sci_mCacheInfo smInfo[MAX_CACHE_MSGS];
}; // scientist_t

// ------------------------------------------------------------------------

// Electro-Alien controller structer
//
struct eaWallInfo {
    int8_t tilex;
    int8_t tiley; // where this controller is in the map.
    int8_t aliens_out; // aliens spawned by this controller.
    int16_t delay; // delay before spawning another alien.

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // eaWallInfo


// General Structure to hold goldstern specific stuff...
//

struct GoldsternInfo_t {
    uint8_t LastIndex; // Last Spawn Coord Index
    uint8_t SpawnCnt; // Num of Spawnpoints for Goldstern
    uint16_t flags; // What type of command/operation is needed...
    uint16_t WaitTime; // Wait time for Goldstern Spawn (current & Next)
    bool GoldSpawned; // Has Goldstern been spawned?

    void serialize(
        bstone::BinaryWriter& writer,
        bstone::Crc32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        bstone::Crc32& checksum);
}; // GoldsternInfo_t


struct star_t {
    int32_t x;
    int32_t y;
    int32_t z;
    uint8_t color;
}; // star_t


/*
=============================================================================

3D_MAIN DEFINITIONS

=============================================================================
*/

extern int16_t TITLE_LOOP_MUSIC;

#define CANT_PLAY_TXT "\n" \
    "ERROR: Insufficient disk space.\n" \
    "Try deleting some files from your hard disk.\n\n"


extern std::string data_dir;

extern const float radtoint; // = (float)FINEANGLES/2/PI;

extern int16_t view_xl;
extern int16_t view_xh;
extern int16_t view_yl;
extern int16_t view_yh;
extern int16_t starting_level;
extern int16_t debug_value;
extern int16_t starting_episode;
extern int16_t starting_difficulty;

extern bool MS_CheckParm(
    const std::string& value);

extern int8_t lastmap_tilex;
extern int8_t lastmap_tiley;
extern uint16_t TopColor;
extern uint16_t BottomColor;
extern char str[80];
extern char str2[20];
extern bool nospr;
extern bool IsA386;

extern int focallength;
extern int viewangles;
extern int screenofs;
extern int viewwidth;
extern int viewheight;
extern int centerx;
extern int shootdelta;

extern int16_t dirangle[9];

extern bool startgame;
extern bool loadedgame;
extern int16_t mouseadjustment;

// BBi
const int min_mouse_sensitivity = 0;
const int max_mouse_sensitivity = 54;
const int default_mouse_sensitivity = 5;
// BBi

//
// math tables
//
extern int* pixelangle;
extern int finetangent[FINEANGLES / 4];
extern int sintable[];
extern int* costable;

//
// derived constants
//
extern int scale;
extern int maxslope;
extern int heightnumerator;
extern int minheightdiv;
extern int mindist;

extern char configname[13];

extern bool ShowQuickMsg;

int DeleteChunk(
    bstone::MemoryStream& stream,
    const std::string& chunk_name);

void LoadFonts();
void ClearNClose();
void CycleColors();
void LoadAccessCodes();
void AlignPlayerInElevator();
void HelpScreens();
void OrderingInfo();
void TEDDeath();

void CalcProjection(
    int32_t focal);

void SetViewSize(
    int width,
    int height);

void NewGame(
    int16_t difficulty,
    int16_t episode);

void NewViewSize();

uint16_t scan_atoi(
    const char* s);

void AlignPlayerOnTransporter();

uint16_t UseFunc(
    const char* first,
    const char* next);

bool DoMovie(
    movie_t movie,
    void* palette);

bool CheckDiskSpace(
    int32_t needed,
    const char* text,
    cds_io_type io_type);

bool SaveTheGame(
    const std::string& file_name,
    const std::string& description);

int32_t ChecksumFile(
    const std::string& file,
    int32_t checksum);

void BadChecksum();

void CheckValidity(
    char* file,
    int32_t valid_checksum);

void UnauthorizedLevels();
void ShowChecksums();

void SetupWalls();
void InitDigiMap();

void CleanUpDoors_N_Actors();


void InitDestPath();

// On success current stream position is at chunk's data.
int FindChunk(
    bstone::IStream* stream,
    const std::string& dst_chunk_name);

int NextChunk(
    bstone::IStream* stream);

/*
=============================================================================

 3D_GAME DEFINITIONS

=============================================================================
*/

extern int16_t db_count;
extern classtype debug_bonus[2][800];
extern fargametype gamestuff;
extern tilecoord_t GoldieList[GOLDIE_MAX_SPAWNS];
extern GoldsternInfo_t GoldsternInfo;

extern uint8_t VitalsRemain;
extern uint8_t VitalsOnFloor;

extern eaWallInfo eaList[];
extern int8_t NumEAWalls;
extern int8_t NumEASpawned;
extern bool ingame;
extern bool fizzlein;
extern int latchpics[NUMLATCHPICS];
extern gametype gamestate;
extern int16_t doornum;

void DrawPlayBorder();
void ScanInfoPlane();
void SetupGameLevel();
void NormalScreen();
void DrawPlayScreen(bool);
void FizzleOut();
void GameLoop();


void Warped();

void RotateView(
    int16_t DestAngle,
    uint8_t RotSpeed);

void DrawWarpIn();

void BMAmsg(
    const char* msg);

void CacheBMAmsg(
    uint16_t MsgNum);

void BevelBox(
    int16_t xl,
    int16_t yl,
    int16_t w,
    int16_t h,
    uint8_t hi,
    uint8_t med,
    uint8_t lo);

void AddTotalPoints(
    uint16_t points);

void AddTotalInformants(
    int8_t informants);

void AddTotalEnemy(
    uint16_t enemies);

void ShadowPrintLocationText(
    sp_type type);

void LoseScreen();

void LoadLocationText(
    int16_t textNum);

/*
=============================================================================

 3D_PLAY DEFINITIONS

=============================================================================
*/

extern int objcount;

extern objtype* DeadGuys[];
extern objtype dummyobj;
extern uint8_t NumDeadGuys;

extern exit_t playstate;

extern int16_t bordertime;

extern bool madenoise;
extern bool usedummy;
extern bool nevermark;
extern uint8_t alerted;
extern uint8_t alerted_areanum;

extern objtype objlist[MAXACTORS];
extern objtype* new_actor;
extern objtype* player;
extern objtype* lastobj;
extern objtype* objfreelist;
extern objtype* killerobj;
extern statobj_t statobjlist[MAXSTATS];
extern statobj_t* laststatobj;
extern doorobj_t doorobjlist[MAXDOORS];
extern doorobj_t* lastdoorobj;

extern uint16_t farmapylookup[MAPSIZE];
extern uint8_t* nearmapylookup[MAPSIZE];

extern uint8_t tilemap[MAPSIZE][MAPSIZE]; // wall values only
extern uint8_t spotvis[MAPSIZE][MAPSIZE];
extern objtype* actorat[MAPSIZE][MAPSIZE];

extern bool singlestep;
extern bool godmode;
extern bool noclip;
extern bool DebugOk;
extern int16_t InstantWin;
extern int16_t InstantQuit;
extern int16_t TestQuickSave;
extern int16_t TestAutoMapper;
extern uint16_t ExtraRadarFlags;

//
// control info
//
using ScanCodes = std::vector<ScanCode>;
using Buttons = std::vector<int16_t>;


extern bool mouseenabled;
extern bool joystickenabled;
extern bool joypadenabled;
extern bool joystickprogressive;
extern int16_t joystickport;
extern ScanCodes dirscan;
extern ScanCodes buttonscan;
extern Buttons buttonmouse;
extern Buttons buttonjoy;

extern bool buttonheld[NUMBUTTONS];

extern const int viewsize;

//
// curent user input
//
extern int controlx;
extern int controly; // range from -100 to 100
extern bool buttonstate[NUMBUTTONS];
extern int strafe_value;

extern char Computing[];


void CenterWindow(
    uint16_t w,
    uint16_t h);

void InitActorList();
void GetNewActor();

void RemoveObj(
    objtype* gone);

void PollControls();
void StopMusic();

void StartMusic(
    bool startit);

void PlayLoop();

void ChangeSwapFiles(
    bool display);

void OpenPageFile();
void CheckMusicToggle();

/*
=============================================================================

 3D_DEBUG

=============================================================================
*/

int16_t DebugKeys();
void CalcMemFree();


/*
=============================================================================

 3D_DRAW DEFINITIONS

=============================================================================
*/

extern uint8_t TravelTable[MAPSIZE][MAPSIZE];

extern int16_t weaponchangetics;
extern int16_t itemchangetics;
extern int16_t bodychangetics;
extern int16_t plaqueon;
extern int16_t plaquetime;
extern int16_t plaquetimefrac;
extern int16_t getpic;

extern statobj_t* firststarobj;

extern int screenloc[3];
extern int freelatch;

extern int32_t space_xmove;
extern int32_t space_ymove;

extern int32_t lasttimecount;
extern int32_t framecount;
extern int32_t frameon;
extern bool fizzlein;

extern int* wallheight;

extern fixed focallength;
extern fixed mindist;

//
// refresh variables
//
extern int viewx;
extern int viewy; // the focal point
extern int viewangle;
extern int viewsin;
extern int viewcos;

extern const uint8_t* postsource;
extern int postx;

// BBi
extern int posty;


extern int16_t horizwall[];
extern int16_t vertwall[];

extern uint16_t pwallpos;

extern bool cloaked_shape;

fixed FixedByFrac(
    fixed a,
    fixed b);

void TransformActor(
    objtype* ob);

void BuildTables();

void ClearScreen();

int16_t CalcRotate(
    objtype* ob);

void DrawScaleds();
void CalcTics();
void ThreeDRefresh();
void FarScalePost();
void DrawStars();

bool TransformTile(
    int16_t tx,
    int16_t ty,
    int16_t* dispx,
    int16_t* dispheight);

void WrapTransformActor(
    objtype* ob);

void ComputeActorPosition(
    objtype* ob,
    int8_t adjust_x,
    int8_t adjust_y);

void WrapDrawScaleds();

bool WrapActorPosition(
    objtype* obj);

void WrapStaticPosition(
    statobj_t* statptr,
    visobj_t* visptr);

void ShowOverhead(
    int bx,
    int by,
    int radius,
    int zoom,
    int flags);

void UpdateTravelTable();


/*
=============================================================================

 3D_DRAW2 DEFINITIONS

=============================================================================
*/
extern uint16_t CeilingTile;
extern uint16_t FloorTile;
extern void (* MapRowPtr)();

void DrawPlanes();


void MapRow();
void C_MapRow();
void F_MapRow();


/*
=============================================================================

 3D_STATE DEFINITIONS

=============================================================================
*/

extern uint16_t actor_points[];
extern dirtype opposite[9];
extern dirtype diagonal[9][9];


void SeekPlayerOrStatic(
    objtype* ob,
    int16_t* deltax,
    int16_t* deltay);

uint16_t CheckRunChase(
    objtype* ob);

void GetCornerSeek(
    objtype* ob);

bool LookForGoodies(
    objtype* ob,
    uint16_t RunReason);

void InitHitRect(
    objtype* ob,
    uint16_t radius);

void SpawnNewObj(
    uint16_t tilex,
    uint16_t tiley,
    statetype* state);

void NewState(
    objtype* ob,
    statetype* state);

bool TryWalk(
    objtype* ob,
    bool moveit);

void SelectChaseDir(
    objtype* ob);

void SelectDodgeDir(
    objtype* ob);

void MoveObj(
    objtype* ob,
    int32_t move);

void KillActor(
    objtype* ob);

void DamageActor(
    objtype* ob,
    uint16_t damage,
    objtype* attacker);

bool CheckLine(
    objtype* from_obj,
    objtype* to_obj);

bool CheckSight(
    objtype* from_obj,
    objtype* to_obj);

bool PosVisable(
    fixed from_x,
    fixed from_y,
    fixed to_x,
    fixed to_y,
    int16_t from_angle);

bool PlayerIsBlocking(
    objtype* ob);

void MakeAlertNoise(
    objtype* obj);

objtype* CheckAndReserve();


/*
=============================================================================

 3D_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART (65 * 4) // offset to start of code in comp scaler

struct t_compshape {
    uint16_t leftpix, rightpix;
    uint16_t dataofs[64];

    // table data after dataofs[rightpix-leftpix+1]
}; // t_compshape

extern int maxscale;
extern int maxscaleshl2;
extern bool scaledir_avail;

extern int normalshade;
extern int normalshade_div;
extern int shade_max;


void FreeScaleDirectory();

void SetupScaling(
    int maxscaleheight);

void ScaleShape(
    int xcenter,
    int shapenum,
    int height);

void SimpleScaleShape(
    int xcenter,
    int shapenum,
    int height);

void MegaSimpleScaleShape(
    int xcenter,
    int ycenter,
    int shapenum,
    int height,
    int shade);


/*
=============================================================================

 3D_AGENT DEFINITIONS

=============================================================================
*/

extern scientist_t InfHintList;
extern scientist_t NiceSciList;
extern scientist_t MeanSciList;

extern uint16_t static_points[];
extern bool GAN_HiddenArea;
extern char* InfAreaMsgs[];
extern uint8_t NumAreaMsgs;
extern uint8_t LastInfArea;
extern int16_t FirstGenInfMsg;
extern int16_t TotalGenInfMsgs;
extern classtype LastInfoAttacker;

extern int16_t LastInfoAttacker_Cloaked;

extern char term_com_name[];
extern char term_msg_name[];

extern atkinf_t attackinfo[7][14];

//
// player state info
//
extern bool commandmode;
extern int32_t thrustspeed;
extern uint16_t plux;
extern uint16_t pluy; // player coordinates scaled to unsigned
extern bool PlayerInvisable;
extern int8_t DrawInfoArea_COUNT;
extern int8_t InitInfoArea_COUNT;

extern uint16_t player_oldtilex;
extern uint16_t player_oldtiley;

// Terminal variables

extern uint16_t RadarSw;

// Location Bar message string...

extern char LocationText[MAX_LOCATION_DESC_LEN];


//
// search / move info
//
extern uint16_t searchon; // held object number looking at
extern int16_t searchtics; // when it reaches SEARCHTICS, get an obj
extern objtype* searchobj; // current object being searched
extern uint16_t foundflag; // only show NOTHING if nothing was found
extern objtype* moveobj; // current object being draged

extern int16_t anglefrac;
extern int16_t facecount;

extern uint16_t LastMsgPri;
extern int16_t MsgTicsRemain;

void GivePoints(
    int32_t score,
    bool add_to_stats);

void SpawnPlayer(
    int16_t tilex,
    int16_t tiley,
    int16_t dir);

void DrawCash();
void UpdateHealth();

void DrawAmmoGuage();
void DrawAmmoMsg();

void DrawAmmo(
    bool ForceRefresh);

bool DisplayInfoMsg(
    const char* Msg,
    msg_priorities Priority,
    int16_t DisplayTime,
    int16_t MessageType);

bool DisplayInfoMsg(
    const std::string& Msg,
    msg_priorities Priority,
    int16_t DisplayTime,
    int16_t MessageType);

void UpdateInfoAreaClock();
void UpdateInfoArea();
void DrawHealthMonitor();
void CalcHealthDisplay();
void UpdateScore();

uint8_t ValidAreaTile(
    const uint16_t* ptr);

int8_t GetAreaNumber(
    int tilex,
    int tiley);

int16_t InputFloor();
void RestoreInfoArea();
void DrawHeartPic();
void DrawInfoArea();
void AnimatePage();

void ActivateTerminal(bool);

void TerminalPrint(
    char* msg,
    bool FastPrint);

void FloorCheat(
    uint16_t RadarFlags);

bool Interrogate(
    objtype* ob);

void GiveKey(
    int16_t key);

void TakeKey(
    int16_t key);

void GiveToken(
    int16_t tokens);

void TakePlasmaDetonator(
    int16_t count);

void GivePlasmaDetonator(
    int16_t count);

void CacheDrawPic(
    int x,
    int y,
    int pic);

void LoadTerminalCommands();

// BBi
void store_cross_barrier(
    uint8_t level,
    uint8_t x,
    uint8_t y,
    bool state);

void apply_cross_barriers();
// BBi

void ActivateWallSwitch(
    uint16_t iconnum,
    int16_t x,
    int16_t y);

uint16_t UpdateBarrierTable(
    uint8_t level,
    uint8_t x,
    uint8_t y,
    bool OnOff);

uint16_t ScanBarrierTable(
    uint8_t x,
    uint8_t y);

void DisplaySwitchOperateMsg(
    int coords);

void DisplayNoMoMsgs();

void PrintStatPercent(
    int16_t nx,
    int16_t ny,
    int8_t percentage);

int16_t ShowStats(
    int16_t bx,
    int16_t by,
    ss_type type,
    statsInfoType* stats);

bool PerfectStats();
bool CheckPerfectStats();

bool OperateSmartSwitch(
    uint16_t tilex,
    uint16_t tiley,
    int8_t Operation,
    bool Force);

/*
=============================================================================

 3D_ACT1 DEFINITIONS

=============================================================================
*/
using StatInfos = std::vector<stattype>;


extern int8_t xy_offset[8][2];
extern StatInfos statinfo;
extern concession_t ConHintList;

extern doorobj_t doorobjlist[MAXDOORS];
extern doorobj_t* lastdoorobj;
extern int16_t doornum;

extern uint16_t doorposition[MAXDOORS];
extern uint16_t pwallstate;

extern uint8_t areaconnect[NUMAREAS][NUMAREAS];

extern bool areabyplayer[NUMAREAS];

extern uint16_t pwallstate;
extern uint16_t pwallpos; // amount a pushable wall has been moved (0-63)
extern uint16_t pwallx;
extern uint16_t pwally;
extern int16_t pwalldir;
extern int16_t pwalldist;


statobj_t* ReserveStatic();

void SpawnStatic(
    int16_t tilex,
    int16_t tiley,
    int16_t type);

void SpawnDoor(
    int16_t tilex,
    int16_t tiley,
    bool vertical,
    keytype lock,
    door_t type);

void OperateConcession(
    uint16_t concession);

void SpawnConcession(
    int16_t tilex,
    int16_t tiley,
    uint16_t credits,
    uint16_t machinetype);

uint16_t LoadConcessionHint(
    uint16_t MsgNum);

void CacheInfoAreaMsg(
    uint16_t block,
    uint16_t MsgNum,
    char* hint_buffer,
    uint16_t MaxBufferLen);

void CheckSpawnEA();

int16_t TransformAreas(
    int8_t tilex,
    int8_t tiley,
    int8_t xform);


void CheckSpawnGoldstern();
void FindNewGoldieSpawnSite();

void InitMsgCache(
    mCacheList* mList,
    uint16_t listSize,
    uint16_t infoSize);

void FreeMsgCache(
    mCacheList* mList,
    uint16_t listSize);

void CacheMsg(
    mCacheInfo* ci,
    uint16_t SegNum,
    uint16_t MsgNum);

int16_t LoadMsg(
    char* hint_buffer,
    uint16_t SegNum,
    uint16_t MsgNum,
    uint16_t MaxMsgLen);

void CacheConcessionMsg();

bool ReuseMsg(
    mCacheInfo* ci,
    int16_t count,
    int16_t struct_size);

void DropPlasmaDetonator();

void BlockDoorOpen(
    int16_t door);

void BlastNearDoors(
    int16_t tilex,
    int16_t tiley);

void TryBlastDoor(
    int8_t door);

statobj_t* FindStatic(
    uint16_t tilex,
    uint16_t tiley);

statobj_t* UseReservedStatic(
    int16_t itemtype,
    int16_t tilex,
    int16_t tiley);

void PlaceReservedItemNearTile(
    int16_t itemtype,
    int16_t tilex,
    int16_t tiley);

void ExplodeStatics(
    int16_t tilex,
    int16_t tiley);


/*
=============================================================================

 3D_ACT2 DEFINITIONS

=============================================================================
*/

#define s_nakedbody s_static10


#define BARRIER_STATE(obj) ((obj)->ammo)

void MakeFakeStatic(
    objtype* ob);

void UnmakeFakeStatic(
    objtype* ob);

extern int8_t detonators_spawned;

extern int16_t starthitpoints[][NUMHITENEMIES];

extern uint16_t MorphClass[];

extern statetype s_ofs_bounce;

extern statetype s_ofs_esphere_death1;
extern statetype s_ofs_ouch;

extern statetype s_ofs_static;

extern statetype s_rent_die1;
extern statetype s_ofcdie1;
extern statetype s_swatdie1;
extern statetype s_prodie1;
extern statetype s_proshoot1;


extern statetype s_rent_chase1;
extern statetype s_ofcchase1;
extern statetype s_prochase1;
extern statetype s_swatchase1;


extern statetype s_rent_pain;
extern statetype s_ofcpain;
extern statetype s_propain;
extern statetype s_swatpain;

extern statetype s_hold;


extern statetype s_swatwounded1;

extern statetype s_deathcam;


extern statetype s_terrot_wait;
extern statetype s_terrot_seek1;


extern statetype s_ofs_stand;
extern statetype s_ofs_chase1;
extern statetype s_ofs_pain;
extern statetype s_ofs_die1;
extern statetype s_ofs_pod_death1;
extern statetype s_ofs_pod_ouch;
extern statetype s_ofs_attack1;

extern statetype s_electro_appear1;
extern statetype s_electro_chase1;
extern statetype s_electro_ouch;
extern statetype s_electro_shoot1;
extern statetype s_electro_die1;


extern statetype s_liquid_wait;
extern statetype s_liquid_move;
extern statetype s_liquid_rise1;
extern statetype s_liquid_shoot1;
extern statetype s_liquid_ouch;
extern statetype s_liquid_shot;
extern statetype s_liquid_die1;
extern statetype s_liquid_shot_exp1;



extern statetype s_goldstand;
extern statetype s_goldpath1;
extern statetype s_goldpain;
extern statetype s_goldshoot1;
extern statetype s_goldchase1;
extern statetype s_goldwarp_it; // Warp In w/button pressing
extern statetype s_goldwarp_out1;
extern statetype s_goldwarp_in1;

extern statetype s_goldmorph1;
extern statetype s_goldmorph2;
extern statetype s_goldmorph3;
extern statetype s_goldmorph4;
extern statetype s_goldmorph5;
extern statetype s_goldmorph6;
extern statetype s_goldmorph7;
extern statetype s_goldmorph8;

extern statetype s_mgold_chase1;
extern statetype s_mgold_chase2;
extern statetype s_mgold_chase3;
extern statetype s_mgold_chase4;

extern statetype s_mgold_shoot1;
extern statetype s_mgold_shoot2;
extern statetype s_mgold_shoot3;
extern statetype s_mgold_shoot4;

extern statetype s_mgold_pain;

extern statetype s_security_light;


extern statetype s_scout_path1;
extern statetype s_scout_path2;
extern statetype s_scout_path3;
extern statetype s_scout_path4;
extern statetype s_scout_pain;
extern statetype s_scout_run;
extern statetype s_scout_run2;
extern statetype s_scout_run3;
extern statetype s_scout_run4;
extern statetype s_scout_die1;
extern statetype s_scout_stand;
extern statetype s_scout_dead;

extern statetype s_explosion1;

extern statetype s_steamgrate;
extern statetype s_vital;

extern statetype s_vital_ouch;
extern statetype s_vital_die;
extern statetype s_vital_die1;
extern statetype s_vital_die2;
extern statetype s_vital_die3;
extern statetype s_vital_die4;
extern statetype s_vital_die5;
extern statetype s_vital_die6;
extern statetype s_vital_die7;
extern statetype s_vital_die8;

extern statetype s_ooze_chase;
extern statetype s_vpost_barrier;
extern statetype s_spike_barrier;

void T_PainThink(
    objtype* obj);

void T_ExplodeScout(
    objtype* obj);

void T_Security(
    objtype* obj);

void T_ChangeShape(
    objtype* obj);

void T_MakeOffset(
    objtype* obj);

void T_LiquidStand(
    objtype* obj);

void PlaceTowardPlayer(
    objtype* obj);

void T_Seek(
    objtype* ob);

void SpawnProjectile(
    objtype* shooter,
    classtype class_type);

void SpawnStand(
    enemy_t which,
    int16_t tilex,
    int16_t tiley,
    int16_t dir);

void SpawnPatrol(
    enemy_t which,
    int16_t tilex,
    int16_t tiley,
    int16_t dir);

void KillActor(
    objtype* ob);

void US_ControlPanel(
    ScanCode scan_code);

int16_t IntSqrt(
    int32_t va);

uint16_t CalcDistance(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2);

void T_Hit(
    objtype* ob);

void SpawnOffsetObj(
    enemy_t which,
    int16_t tilex,
    int16_t tiley);


void InitSmartAnimStruct(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir);

bool AnimateOfsObj(
    objtype* obj);

void AdvanceAnimREV(
    objtype* obj);

void AdvanceAnimFWD(
    objtype* obj);

void SpawnCusExplosion(
    fixed x,
    fixed y,
    uint16_t StartFrame,
    uint16_t NumFrames,
    uint16_t Delay,
    uint16_t Class);

void T_SpawnExplosion(
    objtype* obj);

void T_ExplodeDamage(
    objtype* obj);

void ExplodeRadius(
    objtype* obj,
    int16_t damage,
    bool damageplayer);

extern statetype s_barrier_transition;
extern statetype s_barrier_shutdown;

void SpawnBarrier(
    enemy_t which,
    int16_t tilex,
    int16_t tiley,
    bool OnOff);

void ToggleBarrier(
    objtype* obj);

void InitAnim(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir,
    uint16_t Delay,
    uint16_t WaitDelay);

objtype* FindObj(
    classtype which,
    int16_t tilex,
    int16_t tiley);

objtype* FindHiddenOfs(
    classtype which);

void SpawnHiddenOfs(
    enemy_t which,
    int16_t tilex,
    int16_t tiley);

objtype* MoveHiddenOfs(
    classtype which_class,
    classtype new1,
    fixed x,
    fixed y);

void CheckForSpecialTile(
    objtype* obj,
    uint16_t tilex,
    uint16_t tiley);

void DropCargo(
    objtype* obj);


/*
=============================================================================

 3D_TEXT DEFINITIONS

=============================================================================
*/

extern char helpfilename[];
extern char endfilename[];

extern void HelpScreens();
extern void EndText();


/*
=============================================================================

 3D_MSGS TEXT DEFINITIONS

=============================================================================
*/


using BonusMessages = std::vector<std::reference_wrapper<std::string>>;
using ActorMessages = std::vector<std::reference_wrapper<std::string>>;


extern std::string noeat_msg1;

extern std::string bevs_msg1;
extern std::string food_msg1;

extern std::string bonus_msg7;

extern std::string bonus_msg26;

extern BonusMessages BonusMsg;
extern ActorMessages ActorInfoMsg;
extern std::string ekg_heartbeat_enabled;
extern std::string ekg_heartbeat_disabled;
extern std::string attacker_info_enabled;
extern std::string attacker_info_disabled;
extern std::string WeaponNotAvailMsg;
extern std::string WeaponAvailMsg;
extern std::string RadarAvailMsg;
extern std::string RadarEnergyGoneMsg;
extern std::string WeaponAutoSelectMsg;
extern std::string EnergyPackDepleted;
extern std::string NotEnoughEnergyForWeapon;

extern std::string WeaponMalfunction;

extern std::string SwitchNotActivateMsg;
extern std::string NoFoodTokens;
extern std::string ExtraMan;
extern std::string OneMillion;
extern std::string TenMillion;

extern std::string NoAdLibCard;
extern std::string MusicOn;
extern std::string MusicOff;
extern std::string SoundOn;
extern std::string SoundOff;

extern std::string pd_dropped;
extern std::string pd_nomore;
extern std::string pd_switching;
extern std::string pd_notnear;
extern std::string pd_getcloser;
extern std::string pd_floorunlocked;
extern std::string pd_donthaveany;
extern std::string pd_no_computer;
extern std::string pd_floornotlocked;


/*
=============================================================================

 3D_INTER DEFINITIONS

=============================================================================
*/

extern char BreifingText[];

void UpdateScreenPic();

void DisplayPrepingMsg(
    const char* text);

bool Breifing(
    breifing_type BreifingType,
    uint16_t episode);

void ShPrint(
    const char* text,
    int8_t shadow_color,
    bool single_char);

uint16_t Random(
    uint16_t Max);


// ===========================================================================
//
// 3D_MENU DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================

extern bool EscPressed;

void DrawInstructions(
    inst_type Type);

void CacheMessage(
    uint16_t MessageNum);

void TerminateStr(
    char* pos);

uint32_t CacheCompData(
    uint16_t ItemNum,
    void** dest_loc);


// ===========================================================================
//
// 3D_FREE DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================


extern char JM_FREE_DATA_END[];
extern char JM_FREE_DATA_START[];


// BBi
using Buffer = std::vector<unsigned char>;


objtype* ui16_to_actor(
    uint16_t value);

uint16_t actor_to_ui16(
    const objtype* actor);

statobj_t* ui16_to_static_object(
    uint16_t value);

uint16_t static_object_to_ui16(
    const statobj_t* static_object);

doorobj_t* ui16_to_door_object(
    uint16_t value);

uint16_t door_object_to_ui16(
    const doorobj_t* door_object);

extern bool g_no_wall_hit_sound;
extern bool g_always_run;

// BBi AOG only options
extern bool g_heart_beat_sound;
extern bool g_rotated_automap;


class ArchiveException : public std::exception {
public:
    explicit ArchiveException(
        const char* message) throw ();

    ArchiveException(
        const ArchiveException& that) throw ();

    virtual ~ArchiveException() throw ();

    ArchiveException& operator=(
        const ArchiveException& that) throw ();

    virtual const char* what() const throw ();

private:
    const char* message_;
}; // ArchiveException


template<typename T>
class ArchiveIsContainter
{
public:
    static const bool value = false;
}; // ArchiveIsContainter

template<typename T, typename... TArgs>
class ArchiveIsContainter<std::vector<T,TArgs...>>
{
public:
    static const bool value = true;
}; // ArchiveIsContainter


class ArchiveRemapU8Tag
{
public:
}; // ArchiveRemapU8Tag

class ArchiveRemapContainerTag
{
public:
}; // ArchiveRemapContainerTag

class ArchiveRemapNoneTag
{
public:
}; // ArchiveRemapNoneTag


template<typename T>
class ArchiveRemapTag
{
public:
    using Type = typename std::conditional<
        std::is_same<T,char>::value ||
            std::is_same<T,bool>::value ||
            std::is_same<T,keytype>::value ||
            std::is_same<T,door_t>::value ||
            std::is_same<T,DoorAction>::value ||
            std::is_same<T,activetype>::value ||
            std::is_same<T,classtype>::value ||
            std::is_same<T,dirtype>::value ||
            std::is_same<T,ScanCode>::value,
        ArchiveRemapU8Tag,
        typename std::conditional<
            ArchiveIsContainter<T>::value,
            ArchiveRemapContainerTag,
            ArchiveRemapNoneTag>::type
    >::type;
}; // RemapTag


// Contains an id for enumeration or integral type.
template<typename T>
class ArchiveIEId {
public:
    static const int value = (
        std::is_integral<T>::value ?
            1 : (std::is_enum<T>::value ? 2 : 0));
};

// Remaps enumeration or integral type to integral type.
template<typename T, int>
class ArchiveIETag;

template<typename T>
class ArchiveIETag<T,1>
{
public:
    using Type = T;
};

template<typename T>
class ArchiveIETag<T,2>
{
public:
    using Type = typename std::underlying_type<T>::type;
};

template<typename T>
using ArchiveIEType =
    typename ArchiveIETag<T, ArchiveIEId<T>::value>::Type;


// Contains an id of signed or unsigned integral type.
template<typename T>
class ArchiveSignX8Id {
public:
    static const int value = (
        std::is_integral<T>::value ?
            (std::is_signed<T>::value ? 1 : 2) :
            0);
};

// Remaps integral type to appropriate 8-bit signed/unsigned type.
template<typename T, int>
class ArchiveSignX8Tag;

template<typename T>
class ArchiveSignX8Tag<T,1>
{
public:
    using Type = int8_t;
};

template<typename T>
class ArchiveSignX8Tag<T,2>
{
public:
    using Type = uint8_t;
};

template<typename T>
using ArchiveX8Type =
    typename ArchiveSignX8Tag<T, ArchiveSignX8Id<T>::value>::Type;


template<typename T>
inline void serialize_field_internal(
    const T& value,
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum,
    ArchiveRemapNoneTag)
{
    checksum.update(value);

    if (!writer.write(bstone::Endian::le(value))) {
        throw ArchiveException("serialize_field");
    }
}

template<typename T>
inline void serialize_field_internal(
    const T& value,
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum,
    ArchiveRemapU8Tag)
{
    auto target_value = static_cast<uint8_t>(value);

    checksum.update(target_value);

    if (!writer.write(target_value)) {
        throw ArchiveException("serialize_field");
    }
}

template<typename T>
inline void serialize_field(
    const T& value,
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum);

template<typename T>
inline void serialize_field_internal(
    const T& container,
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum,
    ArchiveRemapContainerTag)
{
    for (auto& item : container) {
        ::serialize_field(item, writer, checksum);
    }
}

template<typename T>
inline void serialize_field(
    const T& value,
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum)
{
    ::serialize_field_internal(
        value,
        writer,
        checksum,
        typename ArchiveRemapTag<T>::Type());
}

template<typename T, size_t N>
inline void serialize_field(
    const T(&value)[N],
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum)
{
    for (size_t i = 0; i < N; ++i) {
        ::serialize_field(value[i], writer, checksum);
    }
}

template<typename T, size_t M, size_t N>
inline void serialize_field(
    const T(&value)[M][N],
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum)
{
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            ::serialize_field(value[i][j], writer, checksum);
        }
    }
}

template<typename T>
inline void deserialize_field(
    T& value,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum);

template<typename T>
inline void deserialize_field_internal(
    T& value,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum,
    ArchiveRemapNoneTag)
{
    if (!reader.read(value)) {
        throw ArchiveException("deserialize_field");
    }

    bstone::Endian::lei(value);

    checksum.update(value);
}

template<typename T>
inline void deserialize_field_internal(
    T& value,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum,
    ArchiveRemapU8Tag)
{
    // Get proper signed/unsigned 8-bit type based on value's one.
    using ValueType = ArchiveIEType<T>;
    using ValueX8Type = ArchiveX8Type<ValueType>;

    ValueX8Type source_value = 0;

    if (!reader.read(source_value)) {
        throw ArchiveException("deserialize_field");
    }

    checksum.update(source_value);

    value = static_cast<T>(source_value);
}

template<>
inline void deserialize_field_internal<bool>(
    bool& value,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum,
    ArchiveRemapU8Tag)
{
    uint8_t source_value = 0;

    if (!reader.read(source_value)) {
        throw ArchiveException("deserialize_field");
    }

    checksum.update(source_value);

    value = (source_value != 0 ? true : false);
}

template<typename T>
inline void deserialize_field_internal(
    T& container,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum,
    ArchiveRemapContainerTag)
{
    for (auto& item : container) {
        ::deserialize_field(item, reader, checksum);
    }
}

template<typename T>
inline void deserialize_field(
    T& value,
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field_internal(
        value,
        reader,
        checksum,
        typename ArchiveRemapTag<T>::Type());
}

template<typename T, size_t N>
inline void deserialize_field(
    T(&value)[N],
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    for (size_t i = 0; i < N; ++i) {
        ::deserialize_field(value[i], reader, checksum);
    }
}

template<typename T, size_t M, size_t N>
inline void deserialize_field(
    T(&value)[M][N],
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            ::deserialize_field(value[i][j], reader, checksum);
        }
    }
}


enum class GameType {
    none,
    aog_sw, // Aliens Of Gold (shareware, v3.0)
    aog_full_v1_0, // Aliens Of Gold (full, v1.0)
    aog_full_v2_x, // Aliens Of Gold (full, v2.0/v2.1)
    aog_full_v3_0, // Aliens Of Gold (full, v3.0)
    ps, // Planet Strike (full, v1.0/v1.1)
}; // GameType

extern GameType g_game_type;

bool is_aog_full_v1_0();

bool is_aog_full_v2_x();

bool is_aog_full_v3_0();

bool is_aog_full();

bool is_aog_sw();

bool is_aog();

bool is_ps();


void InitSmartSpeedAnim(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir,
    uint16_t Delay);

void InitSmartAnim(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir);


inline double m_pi()
{
    return 3.14159265358979323846;
}


void sys_sleep_for(
    int milliseconds);

void sys_default_sleep_for();

const std::string& get_version_string();

const std::string& get_profile_dir();

const std::string& get_default_data_dir();
// BBi


#endif // BSTONE_3D_DEF_INCLUDED
