/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


#include <cstdarg>
#include <cmath>

#include "id_heads.h"
#include "jm_io.h"
#include "jm_cio.h"

// #define DEMOS_EXTERN
// #define MYPROFILE
// #define TRACK_ENEMY_COUNT
#define OBJ_RESERV

#define __BLAKE_VERSION__ "V1.01"

// BBi
#define BS_SAVE_VERSION "2"

#ifdef _DEBUG
#define SKIP_TITLE_AND_CREDITS (1)
#else
#define SKIP_TITLE_AND_CREDITS (0)
#endif // _DEBUG

#ifdef BSTONE_PS
#define GOLD_MORPH_LEVEL (19) // Level which Dr. GoldFire Morphs.
#endif

#define VERSION_TEXT_COLOR (0x82)
#define VERSION_TEXT_BKCOLOR (0x80)

#define NO_SHADING (0x80)
#define LAMP_ON_SHADING (-12)
#define EXPLOSION_SHADING (-52)

#define PAGEFLIP

#define BETA_TEST (0)
#define LOOK_FOR_DEAD_GUYS (0)

#define BETA_CODE "NEWGAME"

#define MIN_MEM_NEEDED (231568l) // 560K
#define LIMITED_AMMO

#define SHADE_MAX 51
#define SHADE_DIV 6

#define BETA_MONTH 8
#define BETA_DAY 1
#define BETA_YEAR 1994

#define DISK_SPACE_NEEDED (1024l * 1024l)

#define BORDER_HI_COLOR 0x85
#define BORDER_MED_COLOR 0x82
#define BORDER_LO_COLOR 0x80
#define BORDER_TEXT_COLOR 0xaf


// FONT DEFINES
//
#define  ID_FONT (STARTFONT)
#define  BIG_FONT (STARTFONT + 1)
#define SIXPOINT_FONT (STARTFONT + 2)
#define         TALL_FONT (STARTFONT + 3)
#define         COAL_FONT (STARTFONT + 4)
#define  RADAR_FONT (STARTFONT + 5)

// #define LOCKED_FLOORS

#define ID_CACHE_BRIEFS
#define ID_CACHE_HELP
#define ID_CACHE_LOSE
#define ID_CACHE_CREDITS


// #define DEBUG_STATICS
// #define DEBUG_ACTORS

// ==========================================================================
//
// GAME VERSION DEPENDANT DEFINATIONS
//
// ==========================================================================

void jsprintf(
    char* msg,
    ...);

/*
=============================================================================

 MACROS

=============================================================================
*/


#define BASE_GRAY 0x8
#define HIGH_GRAY 0xb
#define LOW_GRAY 0x5
#define BASE_GRN 0x76
#define HIGH_GRN 0x78
#define LOW_GRN 0x74


#define MAX_GUN_DELAY 12

#define MAX_RADAR_ENERGY 14400
#define RADAR_PAK_VALUE 60 * 15

#define MAXKEYS 1

#define COLORBORDER(color) asm { mov dx, STATUS_REGISTER_1; in al, dx; \
                                 mov dx, ATR_INDEX; mov al, ATR_OVERSCAN; out dx, al; mov al, color; out dx, al; \
                                 mov al, 32; out dx, al };

#define MAPSPOT(x, y, plane) (*(mapsegs[plane] + farmapylookup[y] + x))

#define SIGN(x) ((x) > 0 ? 1 : -1)
#define ABS(x) ((Sint16)(x) > 0 ? (x) : -(x))
#define LABS(x) ((Sint32)(x) > 0 ? (x) : -(x))

// #define STATUSDRAWPIC(x, y, picnum) LatchDrawPic((x),(y+(200-STATUSLINES)),(picnum))

#define FMAPWIDTH ((fixed)mapwidth << TILESHIFT)
#define FMAPHEIGHT ((fixed)mapheight << TILESHIFT)
#define ObjVisable(from_obj, to_obj) PosVisable(from_obj->x, from_obj->y, to_obj->x, to_obj->y, from_obj->angle)

// SmartAnim macro
#define ANIM_INFO(o) (reinterpret_cast<ofs_anim_t*>(&(o)->temp3))


#define DISPLAY_MSG_STD_TIME (5 * 60) // Tics display len
#define DISPLAY_TIMED_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), DISPLAY_MSG_STD_TIME, (type))
#define DISPLAY_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), 0, (type))


// SMART_ACTORS is the "case" used for certain switch statements.
//

#define SMART_ACTORS proguardobj : \
case rentacopobj : \
case gen_scientistobj


#define SECURITY_LAMPS_ALERTED (madenoise)

// Barrier Code Stuff

#define MAX_BARRIER_SWITCHES 40 // max number level wall switches


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
#define RATIO(M_BASE1, M_BASE2, F_BASE2, SCALE) ((Uint16)(M_BASE1 * ((F_BASE2 << SCALE) / M_BASE2)) >> SCALE)
#define LRATIO(M_BASE1, M_BASE2, F_BASE2, SCALE) (((Sint32)M_BASE1 * (((Sint32)F_BASE2 << SCALE) / M_BASE2)) >> SCALE)


#define MAX_INF_AREA_MSGS 6
#define MAX_LOCATION_DESC_LEN 45

#define DOOR_RUBBLE_STATNUM 112 // Door Rubble sprite

#define SpawnExplosion(a, b) SpawnCusExplosion((a), (b), SPR_EXPLOSION_1, 4, 5, explosionobj)
#define SpawnFlash(a, b) SpawnCusExplosion((a), (b), SPR_EXPLOSION_1, 4, 5, deadobj)
#define InitSmartSpeedAnim(a, b, c, d, e, f, g) InitAnim((a), (b), (c), (d), (e), (f), (g), (g))

/*
=============================================================================

                                                 GLOBAL CONSTANTS

=============================================================================
*/

#define OV_ACTORS 0x0001
#define OV_SHOWALL 0x0002
#define OV_KEYS 0x0004
#define OV_PUSHWALLS 0x0008

// BBi
#define OV_WHOLE_MAP (0x0010)


#define TT_TRAVELED 0x01
#define TT_KEYS 0x02

// Max number of concession reply messages

// #define CON_HINTS // Un/Comment to support concession hints

#define MAXACTORS 150 // max number of nazis, etc / map
#define MAXSTATS 400 // max number of lamps, bonus, etc
#define MAXDOORS 64 // max number of sliding doors
#define MAXCONCESSIONS 15 // max number of concession machines
#define MAXWALLTILES 64 // max number of wall tiles
#define MAXEAWALLS 12 // max electro-alien wall sockets




#define GS_NEEDCOORD 0
#define GS_FIRSTTIME 1
#define GS_COORDFOUND 2
#define GS_NO_MORE 3

#define GOLDIE_MAX_SPAWNS 10 // Max Number of spawn points for Goldstern
#define MIN_GOLDIE_FIRST_WAIT (5 * 60) // Min wait time for FIRST goldstern (5 Secs)
#define MAX_GOLDIE_FIRST_WAIT (15 * 60) // Max wait time for FIRST goldstern (15 Secs)
#define MIN_GOLDIE_WAIT (30 * 60) // Min wait time for next goldstern (30 Secs)
#define MAX_GOLDIE_WAIT (4 * 60 * 60) // Max wait time for next goldstern (4 Mins)




//
// tile constants
//

#define ICONARROWS 90
#define PUSHABLETILE 98
#define EXITTILE 99 // at end of castle
#define AREATILE 108 // first of NUMAREAS floor tiles
#define HIDDENAREATILE 162 // first of hidden floor tiles
#define NUMAREAS 45
#define DOORTRIGGERTILE 158

#ifdef BSTONE_PS
#define SMART_OFF_TRIGGER 159
#define SMART_ON_TRIGGER 160
#endif

#define ELEVATORTILE2 27 // Space Will Switch
#define TRANSPORTERTILE 21
#define DIRECTTRANSPORTTILE 32 // Wall to use on direct level transporters
#define SODATILE 15
#define TERMINALTILE 17
#define FOODTILE 18
#define AMBUSHTILE 106

#ifdef BSTONE_PS
#define RKEY_TILE 72
#define YKEY_TILE 73
#define BKEY_TILE 74
#define BFG_TILE 75
#define ION_TILE 76
#define DETONATOR_TILE 77
#define CLOAK_TILE 78
#define LINC_TILE 79
#define CLOAK_AMBUSH_TILE 80
#endif

#define EATILE 24
#define ON_SWITCH 45 // ON Wall Switch -
#define OFF_SWITCH 57 // OFF Wall Switch -
#define WINTIGGERTILE 157 // Win Tile
#define NUMBERCHARS 9

#define START_TEXTURES 125 // Start of Textures - (Also LAST_WALL_TILE NUM)
#define NUM_TILES (PMSpriteStart)


// ----------------

#define EXTRAPOINTS 400000

#define MAX_EXTRA_LIVES 4

#define PLAYERSPEED 3000
#define RUNSPEED 6000

#define SCREENSEG 0xa000

#define SCREENBWIDE 80

#define HEIGHTRATIO 0.41
#define TOP_STRIP_HEIGHT 16 // Pix height of top strip.

// #define BORDERCOLOR     116
#define FLASHCOLOR 5
#define FLASHTICS 4


#define PLAYERSIZE MINDIST // player radius
#define MINACTORDIST 0x10000l // minimum dist from player center
// to any actor center

#define NUMLATCHPICS 100


#define PI 3.141592657

#define GLOBAL1 (1l << 16)
#define TILEGLOBAL GLOBAL1
#define PIXGLOBAL (GLOBAL1 / 64)
#define TILESHIFT 16l
#define UNSIGNEDSHIFT 8

#define ANGLES 360 // must be divisable by 4
#define ANGLEQUAD (ANGLES / 4)
#define FINEANGLES 3600
#define ANG90 (FINEANGLES / 4)
#define ANG180 (ANG90 * 2)
#define ANG270 (ANG90 * 3)
#define ANG360 (ANG90 * 4)
#define VANG90 (ANGLES / 4)
#define VANG180 (VANG90 * 2)
#define VANG270 (VANG90 * 3)
#define VANG360 (VANG90 * 4)

#define MINDIST (0x5800l)

#define MAX_WVIEW_DIST (44) // Max wrap_view dist in TILES



#define MAXSCALEHEIGHT 256 // largest scale on largest view
#define MAXVIEWWIDTH 320

#define MAPSIZE 64 // maps are 64*64 max
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3


#define STATUSLINES 48

#define SCREENSIZE (SCREENBWIDE * 208)
#define PAGE1START 0
#define PAGE2START (SCREENSIZE)
#define PAGE3START (SCREENSIZE * 2u)
#define FREESTART (SCREENSIZE * 3u)


#define PIXRADIUS 512

#define STARTAMMO 8

// Token Definations

#define MAX_TOKENS 25



// Ammo/Weapon Definations

// #define NUM_AMMO_SEGS 42 // 42 Color segments (OR 21 2-Color segs)
#define MAX_AMMO 100 // Max ammount of ammo for any weapon
#define AUTOCHARGE_WAIT 50 // Tics wait time for a full charge

#ifdef BSTONE_PS
#define MAX_PLASMA_DETONATORS 100 // Max number of Plasma Detonators
#define PLASMA_DETONATORS_DELAY 60 * 4 // Number of tics before plasma detonator explodes
#endif



// gamestate.flags flag values

#define GS_HEARTB_SOUND 0x0001

#ifdef CEILING_FLOOR_COLORS
#define GS_DRAW_CEILING 0x0002
#endif

#define GS_CLIP_WALLS 0x0004

#ifdef CEILING_FLOOR_COLORS
#define GS_DRAW_FLOOR 0x0008
#endif

#define GS_VIRGIN_LEVEL 0x0010
#define GS_CHECK_STATS_BONUS 0x0020
#define GS_ATTACK_INFOAREA 0x0040
#define GS_KILL_INF_WARN 0x0080
#define GS_SHOW_OVERHEAD 0x0100
#define GS_BAD_DIZ_FILE 0x0200
#define GS_MUSIC_TEST 0x0400
#define GS_LIGHTING 0x0800
#define GS_TICS_FOR_SCORE 0x1000
#define GS_NOWAIT 0x2000
#define GS_STARTLEVEL 0x4000
#define GS_QUICKRUN 0x8000

// object flag values - Oh Shit Longs!

#define FL_SHOOTABLE 0x00000001
#define FL_BONUS 0x00000002
#define FL_NEVERMARK 0x00000004
#define FL_VISABLE 0x00000008
#define FL_ATTACKMODE 0x00000010
#define FL_FIRSTATTACK 0x00000020
#define FL_AMBUSH 0x00000040
#define FL_NONMARK 0x00000080
#define FL_SOLID 0x00000100
#define FL_STATIONARY 0x00000200
#define FL_FRIENDLY 0x00000400
#define FL_DEADGUY 0x00000800
#define FL_RUNAWAY 0x00001000
#define FL_RUNTOSTATIC 0x00002000
#define FL_OFFSET_STATES 0x00004000
#define FL_INFORMANT 0x00008000
#define FL_INTERROGATED 0x00010000
#define FL_RANDOM_TURN 0x00020000
#define FL_NO_SLIDE 0x00040000
#define FL_MUST_ATTACK 0x00080000
#define FL_ALERTED 0x00100000
#define FL_FREEZE 0x00200000
#define FL_HAS_AMMO 0x00400000
#define FL_PROJ_TRANSPARENT 0x00800000
#define FL_PROJ_CHECK_TRANSPARENT 0x01000000
#define FL_HAS_TOKENS 0x02000000
#define FL_LOCKED_STATE 0x04000000
#define FL_BARRIER 0x08000000
#define FL_SHOOTMODE 0x10000000
#define FL_SLIDE_INIT 0x20000000
#define FL_STORED_OBJPTR 0x40000000
#define FL_FAKE_STATIC 0x80000000

#define FL_BARRIER_DAMAGE FL_HAS_TOKENS


// object flags2 values

#ifdef BSTONE_PS
#define FL2_BFGSHOT_SOLID 0x0001
#define FL2_BFG_SHOOTABLE 0x0002
#endif

#define FL2_NOTGUNSHOOTABLE 0x0004
#define FL2_SCARED 0x0008
#define FL2_DROP_RKEY 0x0010
#define FL2_DROP_YKEY 0x0020
#define FL2_DROP_BKEY 0x0040

#ifdef BSTONE_PS
#define FL2_DROP_BFG 0x0080
#define FL2_DROP_ION 0x0100
#define FL2_DROP_DETONATOR 0x0200
#define FL2_CLOAKED 0x0400
#define FL2_LINC 0x0800
#define FL2_DAMAGE_CLOAK 0x1000
#endif


// Run Reason Flags -- Why am I running..Duh..

#define RR_AMMO 0x0001
#define RR_HEALTH 0x0002
#define RR_INTERROGATED 0x0004
#define RR_CASUAL_PICKUP 0x0008
#define RR_SCARED 0x0010

// 0xFFxx Special Tile Flags (Flags in MASKED plane of Ted Maps)

#define TF_STARFIELD 0x01
#define TF_WRAP_VIEW 0x02

//
// Concession Machine Types

#define CT_HINT 0x0
#define CT_FOOD 0x1
#define CT_BEVS 0x2


//
// Radar switches for terminal Floor Cheat

#define RS_GOLDSTERN_TRACKER 0x0001
#define RS_PERSONNEL_TRACKER 0x0002
#define RS_SECURITY_STATUS 0x0004

//
// Door Flags
//
#define DR_BLASTABLE 0x01

//
// Smart Trigger Possiblities
//
#define ST_TURN_OFF 0x00
#define ST_TURN_ON 0x01
#define ST_TOGGLE 0x02


//
//
//

#define LT_GUN_DELAY gamestate.lastammo_leds
#define RT_GUN_DELAY gamestate.ammo_leds
#define GUN_TOGGLE gamestate.ammo


#define CANT_SAVE_GAME_TXT " Can't save this game! \n" \
    "    Hard Disk FULL!"

enum ss_type {
    ss_normal,
    ss_quick,
    ss_justcalc
}; // enum ss_type

enum cds_io_type {
    cds_dos_print,
    cds_id_print,
    cds_menu_print
}; // enum cds_io_type


enum sp_type {
    sp_normal,
    sp_loading,
    sp_saving,
    sp_changeview,
    sp_teleporting
}; // enum sp_type

//
// sprite constants
//

enum sprite_t {
    SPR_DEMO = 0,

    //
    // Static sprites
    //
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_STAT_0 = 1,
    SPR_STAT_1 = 2,
    SPR_STAT_2 = 3,
    SPR_STAT_3 = 4,
    SPR_STAT_4 = 5,
    SPR_STAT_5 = 6,
    SPR_STAT_6 = 7,
    SPR_STAT_7 = 8,
    SPR_STAT_8 = 9,
    SPR_STAT_9 = 10,
    SPR_STAT_10 = 11,
    SPR_STAT_11 = 12,
    SPR_STAT_12 = 13,
    SPR_STAT_13 = 14,
    SPR_STAT_14 = 15,
    SPR_STAT_15 = 16,
    SPR_STAT_16 = 17,
    SPR_STAT_17 = 18,
    SPR_STAT_18 = 19,
    SPR_STAT_19 = 20,
    SPR_STAT_20 = 21,
    SPR_STAT_21 = 22,
    SPR_STAT_22 = 23,
    SPR_STAT_23 = 24,
    SPR_STAT_24 = 25,
    SPR_STAT_25 = 26,
    SPR_STAT_26 = 27,
    SPR_STAT_27 = 28,
    SPR_STAT_28 = 29,
    SPR_STAT_29 = 30,
    SPR_STAT_30 = 31,
    SPR_STAT_31 = 32,
    SPR_STAT_32 = 33,
    SPR_STAT_33 = 34,
    SPR_STAT_34 = 35, // Green Key (AOG) / Anti-Plasma Cannon (PS)
    SPR_STAT_35 = 36,
    SPR_STAT_36 = 37, // Gold Key (AOG) / Yellow Package? (PS)
    SPR_STAT_37 = 38,
    SPR_STAT_38 = 39,
    SPR_STAT_39 = 40,
    SPR_STAT_40 = 41,
    SPR_STAT_41 = 42,
    SPR_STAT_42 = 43,
    SPR_STAT_43 = 44,
    SPR_STAT_44 = 45,
    SPR_STAT_45 = 46,
    SPR_STAT_46 = 47,
    SPR_STAT_47 = 48,
    SPR_STAT_48 = 49,
    SPR_STAT_49 = 50,
    SPR_STAT_50 = 51,
    SPR_STAT_51 = 52,
    SPR_STAT_52 = 53,
    SPR_STAT_53 = 54,
    SPR_STAT_54 = 55,
    SPR_STAT_55 = 56,
    SPR_STAT_56 = 57,

    SPR_CRATE_1 = 58,
    SPR_CRATE_2 = 59,
    SPR_CRATE_3 = 60,

    SPR_STAT_57 = 61,
    SPR_STAT_58 = 62,
    SPR_STAT_59 = 63,
    SPR_STAT_60 = 64,
    SPR_STAT_61 = 65,
    SPR_STAT_62 = 66,
    SPR_STAT_63 = 67,
    SPR_STAT_64 = 68,
    SPR_STAT_65 = 69,
    SPR_STAT_66 = 70,
    SPR_STAT_67 = 71,
    SPR_STAT_68 = 72,
    SPR_STAT_69 = 73,
    SPR_STAT_70 = 74,
    SPR_STAT_71 = 75,
    SPR_STAT_72 = 76,
    SPR_STAT_73 = 77,
    SPR_STAT_74 = 78,
    SPR_STAT_75 = 79,
    SPR_STAT_76 = 80,
    SPR_STAT_77 = 81,
    SPR_STAT_78 = 82,
    SPR_STAT_79 = 83,
#else
    SPR_STAT_0 = 1,
    SPR_STAT_2 = 2,
    SPR_STAT_8 = 3,
    SPR_STAT_9 = 4,
    SPR_STAT_10 = 5,
    SPR_STAT_11 = 6,
    SPR_STAT_12 = 7,
    SPR_STAT_13 = 8,
    SPR_STAT_14 = 9,
    SPR_STAT_15 = 10,
    SPR_STAT_18 = 11,
    SPR_STAT_24 = 12,
    SPR_STAT_26 = 13,
    SPR_STAT_27 = 14,
    SPR_STAT_28 = 15,
    SPR_STAT_29 = 16,
    SPR_STAT_31 = 17,
    SPR_STAT_32 = 18,
    SPR_STAT_33 = 19,
    SPR_STAT_34 = 20, // Green Key (AOG) / Anti-Plasma Cannon (PS)
    SPR_STAT_35 = 21,
    SPR_STAT_36 = 22, // Gold Key (AOG) / Yellow Package? (PS)
    SPR_STAT_38 = 23,
    SPR_STAT_40 = 24,
    SPR_STAT_41 = 25,
    SPR_STAT_42 = 26,
    SPR_STAT_43 = 27,
    SPR_STAT_44 = 28,
    SPR_STAT_45 = 29,
    SPR_STAT_46 = 30,
    SPR_STAT_47 = 31,
    SPR_STAT_48 = 32,
    SPR_STAT_49 = 33,
    SPR_STAT_50 = 34,
    SPR_STAT_51 = 35,
    SPR_STAT_52 = 36,
    SPR_STAT_53 = 37,
    SPR_STAT_54 = 38,
    SPR_STAT_55 = 39,
    SPR_STAT_56 = 40,
    SPR_CRATE_1 = 41,
    SPR_STAT_57 = 42,
    SPR_STAT_60 = 43,
    SPR_STAT_61 = 44,
    SPR_STAT_62 = 45,
    SPR_STAT_63 = 46,
    SPR_STAT_65 = 47,
    SPR_STAT_66 = 48,
    SPR_STAT_67 = 49,
    SPR_STAT_68 = 50,
    SPR_STAT_69 = 51,
    SPR_STAT_77 = 52,
    SPR_STAT_78 = 53,
    SPR_STAT_79 = 54,
#endif

#ifdef BSTONE_PS
    SPR_DOORBOMB = 84,
    SPR_ALT_DOORBOMB = 85,
    SPR_RUBBLE = 86,
    SPR_BONZI_TREE = 87,
    SPR_AUTOMAPPER = 88,
    SPR_POT_PLANT = 89,
    SPR_TUBE_PLANT = 90,
    SPR_HITECH_CHAIR = 91,
#endif

    //
    // Aesthetics
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_AIR_VENT = 84,
    SPR_BLOOD_DRIP1 = 85,
    SPR_BLOOD_DRIP2 = 86,
    SPR_BLOOD_DRIP3 = 87,
    SPR_BLOOD_DRIP4 = 88,
    SPR_WATER_DRIP1 = 89,
    SPR_WATER_DRIP2 = 90,
    SPR_WATER_DRIP3 = 91,
    SPR_WATER_DRIP4 = 92,

    SPR_DECO_ARC_1 = 93,
    SPR_DECO_ARC_2 = 94,
    SPR_DECO_ARC_3 = 95,

    SPR_GRATE = 96,
    SPR_STEAM_1 = 97,
    SPR_STEAM_2 = 98,
    SPR_STEAM_3 = 99,
    SPR_STEAM_4 = 100,

    SPR_STEAM_PIPE = 101,
    SPR_PIPE_STEAM_1 = 102,
    SPR_PIPE_STEAM_2 = 103,
    SPR_PIPE_STEAM_3 = 104,
    SPR_PIPE_STEAM_4 = 105,
#else
    SPR_AIR_VENT = 55,
    SPR_BLOOD_DRIP1 = 56,
    SPR_BLOOD_DRIP2 = 57,
    SPR_BLOOD_DRIP3 = 58,
    SPR_BLOOD_DRIP4 = 59,
    SPR_WATER_DRIP1 = 60,
    SPR_WATER_DRIP2 = 61,
    SPR_WATER_DRIP3 = 62,
    SPR_WATER_DRIP4 = 63,

    SPR_DECO_ARC_1 = 64,
    SPR_DECO_ARC_2 = 65,
    SPR_DECO_ARC_3 = 66,

    SPR_GRATE = 67,
    SPR_STEAM_1 = 68,
    SPR_STEAM_2 = 69,
    SPR_STEAM_3 = 70,
    SPR_STEAM_4 = 71,

    SPR_STEAM_PIPE = 72,
    SPR_PIPE_STEAM_1 = 73,
    SPR_PIPE_STEAM_2 = 74,
    SPR_PIPE_STEAM_3 = 75,
    SPR_PIPE_STEAM_4 = 76,
#endif
#else
    SPR_AIR_VENT = 92,
    SPR_BLOOD_DRIP1 = 93,
    SPR_BLOOD_DRIP2 = 94,
    SPR_BLOOD_DRIP3 = 95,
    SPR_BLOOD_DRIP4 = 96,
    SPR_WATER_DRIP1 = 97,
    SPR_WATER_DRIP2 = 98,
    SPR_WATER_DRIP3 = 99,
    SPR_WATER_DRIP4 = 100,

    SPR_DECO_ARC_1 = 101,
    SPR_DECO_ARC_2 = 102,
    SPR_DECO_ARC_3 = 103,

    SPR_GRATE = 104,
    SPR_STEAM_1 = 105,
    SPR_STEAM_2 = 106,
    SPR_STEAM_3 = 107,
    SPR_STEAM_4 = 108,

    SPR_STEAM_PIPE = 109,
    SPR_PIPE_STEAM_1 = 110,
    SPR_PIPE_STEAM_2 = 111,
    SPR_PIPE_STEAM_3 = 112,
    SPR_PIPE_STEAM_4 = 113,
#endif

#ifdef BSTONE_PS
    //
    // Dead Actors (from Blake Stone: AOG)
    //
    SPR_DEAD_RENT = 114,
    SPR_DEAD_PRO = 115,
    SPR_DEAD_SWAT = 116,
#endif

    //
    // Sector Patrol (AOG) / Rent-A-Cop (PS)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_RENT_S_1 = 106,
    SPR_RENT_S_2 = 107,
    SPR_RENT_S_3 = 108,
    SPR_RENT_S_4 = 109,
    SPR_RENT_S_5 = 110,
    SPR_RENT_S_6 = 111,
    SPR_RENT_S_7 = 112,
    SPR_RENT_S_8 = 113,

    SPR_RENT_W1_1 = 114,
    SPR_RENT_W1_2 = 115,
    SPR_RENT_W1_3 = 116,
    SPR_RENT_W1_4 = 117,
    SPR_RENT_W1_5 = 118,
    SPR_RENT_W1_6 = 119,
    SPR_RENT_W1_7 = 120,
    SPR_RENT_W1_8 = 121,

    SPR_RENT_W2_1 = 122,
    SPR_RENT_W2_2 = 123,
    SPR_RENT_W2_3 = 124,
    SPR_RENT_W2_4 = 125,
    SPR_RENT_W2_5 = 126,
    SPR_RENT_W2_6 = 127,
    SPR_RENT_W2_7 = 128,
    SPR_RENT_W2_8 = 129,

    SPR_RENT_W3_1 = 130,
    SPR_RENT_W3_2 = 131,
    SPR_RENT_W3_3 = 132,
    SPR_RENT_W3_4 = 133,
    SPR_RENT_W3_5 = 134,
    SPR_RENT_W3_6 = 135,
    SPR_RENT_W3_7 = 136,
    SPR_RENT_W3_8 = 137,

    SPR_RENT_W4_1 = 138,
    SPR_RENT_W4_2 = 139,
    SPR_RENT_W4_3 = 140,
    SPR_RENT_W4_4 = 141,
    SPR_RENT_W4_5 = 142,
    SPR_RENT_W4_6 = 143,
    SPR_RENT_W4_7 = 144,
    SPR_RENT_W4_8 = 145,

    SPR_RENT_DIE_1 = 146,
    SPR_RENT_DIE_2 = 147,
    SPR_RENT_DIE_3 = 148,
    SPR_RENT_DIE_4 = 149,
    SPR_RENT_PAIN_1 = 150,
    SPR_RENT_DEAD = 151,

    SPR_RENT_SHOOT1 = 152,
    SPR_RENT_SHOOT2 = 153,
    SPR_RENT_SHOOT3 = 154,
#else
#endif
#else
    SPR_RENT_S_1 = 77,
    SPR_RENT_S_2 = 78,
    SPR_RENT_S_3 = 79,
    SPR_RENT_S_4 = 80,
    SPR_RENT_S_5 = 81,
    SPR_RENT_S_6 = 82,
    SPR_RENT_S_7 = 83,
    SPR_RENT_S_8 = 84,

    SPR_RENT_W1_1 = 85,
    SPR_RENT_W1_2 = 86,
    SPR_RENT_W1_3 = 87,
    SPR_RENT_W1_4 = 88,
    SPR_RENT_W1_5 = 89,
    SPR_RENT_W1_6 = 90,
    SPR_RENT_W1_7 = 91,
    SPR_RENT_W1_8 = 92,

    SPR_RENT_W2_1 = 93,
    SPR_RENT_W2_2 = 94,
    SPR_RENT_W2_3 = 95,
    SPR_RENT_W2_4 = 96,
    SPR_RENT_W2_5 = 97,
    SPR_RENT_W2_6 = 98,
    SPR_RENT_W2_7 = 99,
    SPR_RENT_W2_8 = 100,

    SPR_RENT_W3_1 = 101,
    SPR_RENT_W3_2 = 102,
    SPR_RENT_W3_3 = 103,
    SPR_RENT_W3_4 = 104,
    SPR_RENT_W3_5 = 105,
    SPR_RENT_W3_6 = 106,
    SPR_RENT_W3_7 = 107,
    SPR_RENT_W3_8 = 108,

    SPR_RENT_W4_1 = 109,
    SPR_RENT_W4_2 = 110,
    SPR_RENT_W4_3 = 111,
    SPR_RENT_W4_4 = 112,
    SPR_RENT_W4_5 = 113,
    SPR_RENT_W4_6 = 114,
    SPR_RENT_W4_7 = 115,
    SPR_RENT_W4_8 = 116,

    SPR_RENT_DIE_1 = 117,
    SPR_RENT_DIE_2 = 118,
    SPR_RENT_DIE_3 = 119,
    SPR_RENT_DIE_4 = 120,
    SPR_RENT_PAIN_1 = 121,
    SPR_RENT_DEAD = 122,

    SPR_RENT_SHOOT1 = 123,
    SPR_RENT_SHOOT2 = 124,
    SPR_RENT_SHOOT3 = 125,
#endif

    //
    // Star Sentinel (AOG) / Pro-Guard (PS)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_PRO_S_1 = 155,
    SPR_PRO_S_2 = 156,
    SPR_PRO_S_3 = 157,
    SPR_PRO_S_4 = 158,
    SPR_PRO_S_5 = 159,
    SPR_PRO_S_6 = 160,
    SPR_PRO_S_7 = 161,
    SPR_PRO_S_8 = 162,

    SPR_PRO_W1_1 = 163,
    SPR_PRO_W1_2 = 164,
    SPR_PRO_W1_3 = 165,
    SPR_PRO_W1_4 = 166,
    SPR_PRO_W1_5 = 167,
    SPR_PRO_W1_6 = 168,
    SPR_PRO_W1_7 = 169,
    SPR_PRO_W1_8 = 170,

    SPR_PRO_W2_1 = 171,
    SPR_PRO_W2_2 = 172,
    SPR_PRO_W2_3 = 173,
    SPR_PRO_W2_4 = 174,
    SPR_PRO_W2_5 = 175,
    SPR_PRO_W2_6 = 176,
    SPR_PRO_W2_7 = 177,
    SPR_PRO_W2_8 = 178,

    SPR_PRO_W3_1 = 179,
    SPR_PRO_W3_2 = 180,
    SPR_PRO_W3_3 = 181,
    SPR_PRO_W3_4 = 182,
    SPR_PRO_W3_5 = 183,
    SPR_PRO_W3_6 = 184,
    SPR_PRO_W3_7 = 185,
    SPR_PRO_W3_8 = 186,

    SPR_PRO_W4_1 = 187,
    SPR_PRO_W4_2 = 188,
    SPR_PRO_W4_3 = 189,
    SPR_PRO_W4_4 = 190,
    SPR_PRO_W4_5 = 191,
    SPR_PRO_W4_6 = 192,
    SPR_PRO_W4_7 = 193,
    SPR_PRO_W4_8 = 194,

    SPR_PRO_PAIN_1 = 195,
    SPR_PRO_DIE_1 = 196,
    SPR_PRO_DIE_2 = 197,
    SPR_PRO_DIE_3 = 198,
    SPR_PRO_PAIN_2 = 199,
    SPR_PRO_DIE_4 = 200,
    SPR_PRO_DEAD = 201,

    SPR_PRO_SHOOT1 = 202,
    SPR_PRO_SHOOT2 = 203,
    SPR_PRO_SHOOT3 = 204,
#else
    SPR_PRO_S_1 = 126,
    SPR_PRO_S_2 = 127,
    SPR_PRO_S_3 = 128,
    SPR_PRO_S_4 = 129,
    SPR_PRO_S_5 = 130,
    SPR_PRO_S_6 = 131,
    SPR_PRO_S_7 = 132,
    SPR_PRO_S_8 = 133,

    SPR_PRO_W1_1 = 134,
    SPR_PRO_W1_2 = 135,
    SPR_PRO_W1_3 = 136,
    SPR_PRO_W1_4 = 137,
    SPR_PRO_W1_5 = 138,
    SPR_PRO_W1_6 = 139,
    SPR_PRO_W1_7 = 140,
    SPR_PRO_W1_8 = 141,

    SPR_PRO_W2_1 = 142,
    SPR_PRO_W2_2 = 143,
    SPR_PRO_W2_3 = 144,
    SPR_PRO_W2_4 = 145,
    SPR_PRO_W2_5 = 146,
    SPR_PRO_W2_6 = 147,
    SPR_PRO_W2_7 = 148,
    SPR_PRO_W2_8 = 149,

    SPR_PRO_W3_1 = 150,
    SPR_PRO_W3_2 = 151,
    SPR_PRO_W3_3 = 152,
    SPR_PRO_W3_4 = 153,
    SPR_PRO_W3_5 = 154,
    SPR_PRO_W3_6 = 155,
    SPR_PRO_W3_7 = 156,
    SPR_PRO_W3_8 = 157,

    SPR_PRO_W4_1 = 158,
    SPR_PRO_W4_2 = 159,
    SPR_PRO_W4_3 = 160,
    SPR_PRO_W4_4 = 161,
    SPR_PRO_W4_5 = 162,
    SPR_PRO_W4_6 = 163,
    SPR_PRO_W4_7 = 164,
    SPR_PRO_W4_8 = 165,

    SPR_PRO_PAIN_1 = 166,
    SPR_PRO_DIE_1 = 167,
    SPR_PRO_DIE_2 = 168,
    SPR_PRO_DIE_3 = 169,
    SPR_PRO_PAIN_2 = 170,
    SPR_PRO_DIE_4 = 171,
    SPR_PRO_DEAD = 172,

    SPR_PRO_SHOOT1 = 173,
    SPR_PRO_SHOOT2 = 174,
    SPR_PRO_SHOOT3 = 175,
#endif
#else
    SPR_PRO_S_1 = 166,
    SPR_PRO_S_2 = 167,
    SPR_PRO_S_3 = 168,
    SPR_PRO_S_4 = 169,
    SPR_PRO_S_5 = 170,
    SPR_PRO_S_6 = 171,
    SPR_PRO_S_7 = 172,
    SPR_PRO_S_8 = 173,

    SPR_PRO_W1_1 = 174,
    SPR_PRO_W1_2 = 175,
    SPR_PRO_W1_3 = 176,
    SPR_PRO_W1_4 = 177,
    SPR_PRO_W1_5 = 178,
    SPR_PRO_W1_6 = 179,
    SPR_PRO_W1_7 = 180,
    SPR_PRO_W1_8 = 181,

    SPR_PRO_W2_1 = 182,
    SPR_PRO_W2_2 = 183,
    SPR_PRO_W2_3 = 184,
    SPR_PRO_W2_4 = 185,
    SPR_PRO_W2_5 = 186,
    SPR_PRO_W2_6 = 187,
    SPR_PRO_W2_7 = 188,
    SPR_PRO_W2_8 = 189,

    SPR_PRO_W3_1 = 190,
    SPR_PRO_W3_2 = 191,
    SPR_PRO_W3_3 = 192,
    SPR_PRO_W3_4 = 193,
    SPR_PRO_W3_5 = 194,
    SPR_PRO_W3_6 = 195,
    SPR_PRO_W3_7 = 196,
    SPR_PRO_W3_8 = 197,

    SPR_PRO_W4_1 = 198,
    SPR_PRO_W4_2 = 199,
    SPR_PRO_W4_3 = 200,
    SPR_PRO_W4_4 = 201,
    SPR_PRO_W4_5 = 202,
    SPR_PRO_W4_6 = 203,
    SPR_PRO_W4_7 = 204,
    SPR_PRO_W4_8 = 205,

    SPR_PRO_PAIN_1 = 206,
    SPR_PRO_DIE_1 = 207,
    SPR_PRO_DIE_2 = 208,
    SPR_PRO_DIE_3 = 209,
    SPR_PRO_PAIN_2 = 210,
    SPR_PRO_DIE_4 = 211,
    SPR_PRO_DEAD = 212,

    SPR_PRO_SHOOT1 = 213,
    SPR_PRO_SHOOT2 = 214,
    SPR_PRO_SHOOT3 = 215,
#endif

    //
    // Star Trooper (AOG) / SWAT (PS)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_SWAT_S_1 = 205,
    SPR_SWAT_S_2 = 206,
    SPR_SWAT_S_3 = 207,
    SPR_SWAT_S_4 = 208,
    SPR_SWAT_S_5 = 209,
    SPR_SWAT_S_6 = 210,
    SPR_SWAT_S_7 = 211,
    SPR_SWAT_S_8 = 212,

    SPR_SWAT_W1_1 = 213,
    SPR_SWAT_W1_2 = 214,
    SPR_SWAT_W1_3 = 215,
    SPR_SWAT_W1_4 = 216,
    SPR_SWAT_W1_5 = 217,
    SPR_SWAT_W1_6 = 218,
    SPR_SWAT_W1_7 = 219,
    SPR_SWAT_W1_8 = 220,

    SPR_SWAT_W2_1 = 221,
    SPR_SWAT_W2_2 = 222,
    SPR_SWAT_W2_3 = 223,
    SPR_SWAT_W2_4 = 224,
    SPR_SWAT_W2_5 = 225,
    SPR_SWAT_W2_6 = 226,
    SPR_SWAT_W2_7 = 227,
    SPR_SWAT_W2_8 = 228,

    SPR_SWAT_W3_1 = 229,
    SPR_SWAT_W3_2 = 230,
    SPR_SWAT_W3_3 = 231,
    SPR_SWAT_W3_4 = 232,
    SPR_SWAT_W3_5 = 233,
    SPR_SWAT_W3_6 = 234,
    SPR_SWAT_W3_7 = 235,
    SPR_SWAT_W3_8 = 236,

    SPR_SWAT_W4_1 = 237,
    SPR_SWAT_W4_2 = 238,
    SPR_SWAT_W4_3 = 239,
    SPR_SWAT_W4_4 = 240,
    SPR_SWAT_W4_5 = 241,
    SPR_SWAT_W4_6 = 242,
    SPR_SWAT_W4_7 = 243,
    SPR_SWAT_W4_8 = 244,

    SPR_SWAT_PAIN_1 = 245,
    SPR_SWAT_DIE_1 = 246,
    SPR_SWAT_DIE_2 = 247,
    SPR_SWAT_DIE_3 = 248,
    SPR_SWAT_PAIN_2 = 249,
    SPR_SWAT_DIE_4 = 250,
    SPR_SWAT_DEAD = 251,

    SPR_SWAT_SHOOT1 = 252,
    SPR_SWAT_SHOOT2 = 253,
    SPR_SWAT_SHOOT3 = 254,

    SPR_SWAT_WOUNDED1 = 255,
    SPR_SWAT_WOUNDED2 = 256,
    SPR_SWAT_WOUNDED3 = 257,
    SPR_SWAT_WOUNDED4 = 258,
#else
    SPR_SWAT_S_1 = 176,
    SPR_SWAT_S_2 = 177,
    SPR_SWAT_S_3 = 178,
    SPR_SWAT_S_4 = 179,
    SPR_SWAT_S_5 = 180,
    SPR_SWAT_S_6 = 181,
    SPR_SWAT_S_7 = 182,
    SPR_SWAT_S_8 = 183,

    SPR_SWAT_W1_1 = 184,
    SPR_SWAT_W1_2 = 185,
    SPR_SWAT_W1_3 = 186,
    SPR_SWAT_W1_4 = 187,
    SPR_SWAT_W1_5 = 188,
    SPR_SWAT_W1_6 = 189,
    SPR_SWAT_W1_7 = 190,
    SPR_SWAT_W1_8 = 191,

    SPR_SWAT_W2_1 = 192,
    SPR_SWAT_W2_2 = 193,
    SPR_SWAT_W2_3 = 194,
    SPR_SWAT_W2_4 = 195,
    SPR_SWAT_W2_5 = 196,
    SPR_SWAT_W2_6 = 197,
    SPR_SWAT_W2_7 = 198,
    SPR_SWAT_W2_8 = 199,

    SPR_SWAT_W3_1 = 200,
    SPR_SWAT_W3_2 = 201,
    SPR_SWAT_W3_3 = 202,
    SPR_SWAT_W3_4 = 203,
    SPR_SWAT_W3_5 = 204,
    SPR_SWAT_W3_6 = 205,
    SPR_SWAT_W3_7 = 206,
    SPR_SWAT_W3_8 = 207,

    SPR_SWAT_W4_1 = 208,
    SPR_SWAT_W4_2 = 209,
    SPR_SWAT_W4_3 = 210,
    SPR_SWAT_W4_4 = 211,
    SPR_SWAT_W4_5 = 212,
    SPR_SWAT_W4_6 = 213,
    SPR_SWAT_W4_7 = 214,
    SPR_SWAT_W4_8 = 215,

    SPR_SWAT_PAIN_1 = 216,
    SPR_SWAT_DIE_1 = 217,
    SPR_SWAT_DIE_2 = 218,
    SPR_SWAT_DIE_3 = 219,
    SPR_SWAT_PAIN_2 = 220,
    SPR_SWAT_DIE_4 = 221,
    SPR_SWAT_DEAD = 222,

    SPR_SWAT_SHOOT1 = 223,
    SPR_SWAT_SHOOT2 = 224,
    SPR_SWAT_SHOOT3 = 225,

    SPR_SWAT_WOUNDED1 = 226,
    SPR_SWAT_WOUNDED2 = 227,
    SPR_SWAT_WOUNDED3 = 228,
    SPR_SWAT_WOUNDED4 = 229,
#endif
#else
    SPR_SWAT_S_1 = 216,
    SPR_SWAT_S_2 = 217,
    SPR_SWAT_S_3 = 218,
    SPR_SWAT_S_4 = 219,
    SPR_SWAT_S_5 = 220,
    SPR_SWAT_S_6 = 221,
    SPR_SWAT_S_7 = 222,
    SPR_SWAT_S_8 = 223,

    SPR_SWAT_W1_1 = 224,
    SPR_SWAT_W1_2 = 225,
    SPR_SWAT_W1_3 = 226,
    SPR_SWAT_W1_4 = 227,
    SPR_SWAT_W1_5 = 228,
    SPR_SWAT_W1_6 = 229,
    SPR_SWAT_W1_7 = 230,
    SPR_SWAT_W1_8 = 231,

    SPR_SWAT_W2_1 = 232,
    SPR_SWAT_W2_2 = 233,
    SPR_SWAT_W2_3 = 234,
    SPR_SWAT_W2_4 = 235,
    SPR_SWAT_W2_5 = 236,
    SPR_SWAT_W2_6 = 237,
    SPR_SWAT_W2_7 = 238,
    SPR_SWAT_W2_8 = 239,

    SPR_SWAT_W3_1 = 240,
    SPR_SWAT_W3_2 = 241,
    SPR_SWAT_W3_3 = 242,
    SPR_SWAT_W3_4 = 243,
    SPR_SWAT_W3_5 = 244,
    SPR_SWAT_W3_6 = 245,
    SPR_SWAT_W3_7 = 246,
    SPR_SWAT_W3_8 = 247,

    SPR_SWAT_W4_1 = 248,
    SPR_SWAT_W4_2 = 249,
    SPR_SWAT_W4_3 = 250,
    SPR_SWAT_W4_4 = 251,
    SPR_SWAT_W4_5 = 252,
    SPR_SWAT_W4_6 = 253,
    SPR_SWAT_W4_7 = 254,
    SPR_SWAT_W4_8 = 255,

    SPR_SWAT_PAIN_1 = 256,
    SPR_SWAT_DIE_1 = 257,
    SPR_SWAT_DIE_2 = 258,
    SPR_SWAT_DIE_3 = 259,
    SPR_SWAT_PAIN_2 = 260,
    SPR_SWAT_DIE_4 = 261,
    SPR_SWAT_DEAD = 262,

    SPR_SWAT_SHOOT1 = 263,
    SPR_SWAT_SHOOT2 = 264,
    SPR_SWAT_SHOOT3 = 265,

    SPR_SWAT_WOUNDED1 = 266,
    SPR_SWAT_WOUNDED2 = 267,
    SPR_SWAT_WOUNDED3 = 268,
    SPR_SWAT_WOUNDED4 = 269,
#endif

    //
    // General Scientist
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_OFC_S_1 = 259,
    SPR_OFC_S_2 = 260,
    SPR_OFC_S_3 = 261,
    SPR_OFC_S_4 = 262,
    SPR_OFC_S_5 = 263,
    SPR_OFC_S_6 = 264,
    SPR_OFC_S_7 = 265,
    SPR_OFC_S_8 = 266,

    SPR_OFC_W1_1 = 267,
    SPR_OFC_W1_2 = 268,
    SPR_OFC_W1_3 = 269,
    SPR_OFC_W1_4 = 270,
    SPR_OFC_W1_5 = 271,
    SPR_OFC_W1_6 = 272,
    SPR_OFC_W1_7 = 273,
    SPR_OFC_W1_8 = 274,

    SPR_OFC_W2_1 = 275,
    SPR_OFC_W2_2 = 276,
    SPR_OFC_W2_3 = 277,
    SPR_OFC_W2_4 = 278,
    SPR_OFC_W2_5 = 279,
    SPR_OFC_W2_6 = 280,
    SPR_OFC_W2_7 = 281,
    SPR_OFC_W2_8 = 282,

    SPR_OFC_W3_1 = 283,
    SPR_OFC_W3_2 = 284,
    SPR_OFC_W3_3 = 285,
    SPR_OFC_W3_4 = 286,
    SPR_OFC_W3_5 = 287,
    SPR_OFC_W3_6 = 288,
    SPR_OFC_W3_7 = 289,
    SPR_OFC_W3_8 = 290,

    SPR_OFC_W4_1 = 291,
    SPR_OFC_W4_2 = 292,
    SPR_OFC_W4_3 = 293,
    SPR_OFC_W4_4 = 294,
    SPR_OFC_W4_5 = 295,
    SPR_OFC_W4_6 = 296,
    SPR_OFC_W4_7 = 297,
    SPR_OFC_W4_8 = 298,

    SPR_OFC_PAIN_1 = 299,
    SPR_OFC_DIE_1 = 300,
    SPR_OFC_DIE_2 = 301,
    SPR_OFC_DIE_3 = 302,
    SPR_OFC_PAIN_2 = 303,
    SPR_OFC_DIE_4 = 304,
    SPR_OFC_DEAD = 305,

    SPR_OFC_SHOOT1 = 306,
    SPR_OFC_SHOOT2 = 307,
    SPR_OFC_SHOOT3 = 308,
#else
    SPR_OFC_S_1 = 230,
    SPR_OFC_S_2 = 231,
    SPR_OFC_S_3 = 232,
    SPR_OFC_S_4 = 233,
    SPR_OFC_S_5 = 234,
    SPR_OFC_S_6 = 235,
    SPR_OFC_S_7 = 236,
    SPR_OFC_S_8 = 237,

    SPR_OFC_W1_1 = 238,
    SPR_OFC_W1_2 = 239,
    SPR_OFC_W1_3 = 240,
    SPR_OFC_W1_4 = 241,
    SPR_OFC_W1_5 = 242,
    SPR_OFC_W1_6 = 243,
    SPR_OFC_W1_7 = 244,
    SPR_OFC_W1_8 = 245,

    SPR_OFC_W2_1 = 246,
    SPR_OFC_W2_2 = 247,
    SPR_OFC_W2_3 = 248,
    SPR_OFC_W2_4 = 249,
    SPR_OFC_W2_5 = 250,
    SPR_OFC_W2_6 = 251,
    SPR_OFC_W2_7 = 252,
    SPR_OFC_W2_8 = 253,

    SPR_OFC_W3_1 = 254,
    SPR_OFC_W3_2 = 255,
    SPR_OFC_W3_3 = 256,
    SPR_OFC_W3_4 = 257,
    SPR_OFC_W3_5 = 258,
    SPR_OFC_W3_6 = 259,
    SPR_OFC_W3_7 = 260,
    SPR_OFC_W3_8 = 261,

    SPR_OFC_W4_1 = 262,
    SPR_OFC_W4_2 = 263,
    SPR_OFC_W4_3 = 264,
    SPR_OFC_W4_4 = 265,
    SPR_OFC_W4_5 = 266,
    SPR_OFC_W4_6 = 267,
    SPR_OFC_W4_7 = 268,
    SPR_OFC_W4_8 = 269,

    SPR_OFC_PAIN_1 = 270,
    SPR_OFC_DIE_1 = 271,
    SPR_OFC_DIE_2 = 272,
    SPR_OFC_DIE_3 = 273,
    SPR_OFC_PAIN_2 = 274,
    SPR_OFC_DIE_4 = 275,
    SPR_OFC_DEAD = 276,

    SPR_OFC_SHOOT1 = 277,
    SPR_OFC_SHOOT2 = 278,
    SPR_OFC_SHOOT3 = 279,
#endif
#else
    SPR_OFC_S_1 = 270,
    SPR_OFC_S_2 = 271,
    SPR_OFC_S_3 = 272,
    SPR_OFC_S_4 = 273,
    SPR_OFC_S_5 = 274,
    SPR_OFC_S_6 = 275,
    SPR_OFC_S_7 = 276,
    SPR_OFC_S_8 = 277,

    SPR_OFC_W1_1 = 278,
    SPR_OFC_W1_2 = 279,
    SPR_OFC_W1_3 = 280,
    SPR_OFC_W1_4 = 281,
    SPR_OFC_W1_5 = 282,
    SPR_OFC_W1_6 = 283,
    SPR_OFC_W1_7 = 284,
    SPR_OFC_W1_8 = 285,

    SPR_OFC_W2_1 = 286,
    SPR_OFC_W2_2 = 287,
    SPR_OFC_W2_3 = 288,
    SPR_OFC_W2_4 = 289,
    SPR_OFC_W2_5 = 290,
    SPR_OFC_W2_6 = 291,
    SPR_OFC_W2_7 = 292,
    SPR_OFC_W2_8 = 293,

    SPR_OFC_W3_1 = 294,
    SPR_OFC_W3_2 = 295,
    SPR_OFC_W3_3 = 296,
    SPR_OFC_W3_4 = 297,
    SPR_OFC_W3_5 = 298,
    SPR_OFC_W3_6 = 299,
    SPR_OFC_W3_7 = 300,
    SPR_OFC_W3_8 = 301,

    SPR_OFC_W4_1 = 302,
    SPR_OFC_W4_2 = 303,
    SPR_OFC_W4_3 = 304,
    SPR_OFC_W4_4 = 305,
    SPR_OFC_W4_5 = 306,
    SPR_OFC_W4_6 = 307,
    SPR_OFC_W4_7 = 308,
    SPR_OFC_W4_8 = 309,

    SPR_OFC_PAIN_1 = 310,
    SPR_OFC_DIE_1 = 311,
    SPR_OFC_DIE_2 = 312,
    SPR_OFC_DIE_3 = 313,
    SPR_OFC_PAIN_2 = 314,
    SPR_OFC_DIE_4 = 315,
    SPR_OFC_DEAD = 316,

    SPR_OFC_SHOOT1 = 317,
    SPR_OFC_SHOOT2 = 318,
    SPR_OFC_SHOOT3 = 319,
#endif

    //
    // Bad Boy Dr. Goldstern
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_GOLD_S_1 = 309,
    SPR_GOLD_S_2 = 310,
    SPR_GOLD_S_3 = 311,
    SPR_GOLD_S_4 = 312,
    SPR_GOLD_S_5 = 313,
    SPR_GOLD_S_6 = 314,
    SPR_GOLD_S_7 = 315,
    SPR_GOLD_S_8 = 316,

    SPR_GOLD_W1_1 = 317,
    SPR_GOLD_W1_2 = 318,
    SPR_GOLD_W1_3 = 319,
    SPR_GOLD_W1_4 = 320,
    SPR_GOLD_W1_5 = 321,
    SPR_GOLD_W1_6 = 322,
    SPR_GOLD_W1_7 = 323,
    SPR_GOLD_W1_8 = 324,

    SPR_GOLD_W2_1 = 325,
    SPR_GOLD_W2_2 = 326,
    SPR_GOLD_W2_3 = 327,
    SPR_GOLD_W2_4 = 328,
    SPR_GOLD_W2_5 = 329,
    SPR_GOLD_W2_6 = 330,
    SPR_GOLD_W2_7 = 331,
    SPR_GOLD_W2_8 = 332,

    SPR_GOLD_W3_1 = 333,
    SPR_GOLD_W3_2 = 334,
    SPR_GOLD_W3_3 = 335,
    SPR_GOLD_W3_4 = 336,
    SPR_GOLD_W3_5 = 337,
    SPR_GOLD_W3_6 = 338,
    SPR_GOLD_W3_7 = 339,
    SPR_GOLD_W3_8 = 340,

    SPR_GOLD_W4_1 = 341,
    SPR_GOLD_W4_2 = 342,
    SPR_GOLD_W4_3 = 343,
    SPR_GOLD_W4_4 = 344,
    SPR_GOLD_W4_5 = 345,
    SPR_GOLD_W4_6 = 346,
    SPR_GOLD_W4_7 = 347,
    SPR_GOLD_W4_8 = 348,

    SPR_GOLD_PAIN_1 = 349,

    SPR_GOLD_WRIST_1 = 350,
    SPR_GOLD_WRIST_2 = 351,

    SPR_GOLD_SHOOT1 = 352,
    SPR_GOLD_SHOOT2 = 353,
    SPR_GOLD_SHOOT3 = 354,

    SPR_GOLD_WARP1 = 355,
    SPR_GOLD_WARP2 = 356,
    SPR_GOLD_WARP3 = 357,
    SPR_GOLD_WARP4 = 358,
    SPR_GOLD_WARP5 = 359,
#else
    SPR_GOLD_S_1 = 280,
    SPR_GOLD_S_2 = 281,
    SPR_GOLD_S_3 = 282,
    SPR_GOLD_S_4 = 283,
    SPR_GOLD_S_5 = 284,
    SPR_GOLD_S_6 = 285,
    SPR_GOLD_S_7 = 286,
    SPR_GOLD_S_8 = 287,

    SPR_GOLD_W1_1 = 288,
    SPR_GOLD_W1_2 = 289,
    SPR_GOLD_W1_3 = 290,
    SPR_GOLD_W1_4 = 291,
    SPR_GOLD_W1_5 = 292,
    SPR_GOLD_W1_6 = 293,
    SPR_GOLD_W1_7 = 294,
    SPR_GOLD_W1_8 = 295,

    SPR_GOLD_W2_1 = 296,
    SPR_GOLD_W2_2 = 297,
    SPR_GOLD_W2_3 = 298,
    SPR_GOLD_W2_4 = 299,
    SPR_GOLD_W2_5 = 300,
    SPR_GOLD_W2_6 = 301,
    SPR_GOLD_W2_7 = 302,
    SPR_GOLD_W2_8 = 303,

    SPR_GOLD_W3_1 = 304,
    SPR_GOLD_W3_2 = 305,
    SPR_GOLD_W3_3 = 306,
    SPR_GOLD_W3_4 = 307,
    SPR_GOLD_W3_5 = 308,
    SPR_GOLD_W3_6 = 309,
    SPR_GOLD_W3_7 = 310,
    SPR_GOLD_W3_8 = 311,

    SPR_GOLD_W4_1 = 312,
    SPR_GOLD_W4_2 = 313,
    SPR_GOLD_W4_3 = 314,
    SPR_GOLD_W4_4 = 315,
    SPR_GOLD_W4_5 = 316,
    SPR_GOLD_W4_6 = 317,
    SPR_GOLD_W4_7 = 318,
    SPR_GOLD_W4_8 = 319,

    SPR_GOLD_PAIN_1 = 320,

    SPR_GOLD_WRIST_1 = 321,
    SPR_GOLD_WRIST_2 = 322,

    SPR_GOLD_SHOOT1 = 323,
    SPR_GOLD_SHOOT2 = 324,
    SPR_GOLD_SHOOT3 = 325,

    SPR_GOLD_WARP1 = 326,
    SPR_GOLD_WARP2 = 327,
    SPR_GOLD_WARP3 = 328,
    SPR_GOLD_WARP4 = 329,
    SPR_GOLD_WARP5 = 330,
#endif
#else
    SPR_GOLD_S_1 = 320,
    SPR_GOLD_S_2 = 321,
    SPR_GOLD_S_3 = 322,
    SPR_GOLD_S_4 = 323,
    SPR_GOLD_S_5 = 324,
    SPR_GOLD_S_6 = 325,
    SPR_GOLD_S_7 = 326,
    SPR_GOLD_S_8 = 327,

    SPR_GOLD_W1_1 = 328,
    SPR_GOLD_W1_2 = 329,
    SPR_GOLD_W1_3 = 330,
    SPR_GOLD_W1_4 = 331,
    SPR_GOLD_W1_5 = 332,
    SPR_GOLD_W1_6 = 333,
    SPR_GOLD_W1_7 = 334,
    SPR_GOLD_W1_8 = 335,

    SPR_GOLD_W2_1 = 336,
    SPR_GOLD_W2_2 = 337,
    SPR_GOLD_W2_3 = 338,
    SPR_GOLD_W2_4 = 339,
    SPR_GOLD_W2_5 = 340,
    SPR_GOLD_W2_6 = 341,
    SPR_GOLD_W2_7 = 342,
    SPR_GOLD_W2_8 = 343,

    SPR_GOLD_W3_1 = 344,
    SPR_GOLD_W3_2 = 345,
    SPR_GOLD_W3_3 = 346,
    SPR_GOLD_W3_4 = 347,
    SPR_GOLD_W3_5 = 348,
    SPR_GOLD_W3_6 = 349,
    SPR_GOLD_W3_7 = 350,
    SPR_GOLD_W3_8 = 351,

    SPR_GOLD_W4_1 = 352,
    SPR_GOLD_W4_2 = 353,
    SPR_GOLD_W4_3 = 354,
    SPR_GOLD_W4_4 = 355,
    SPR_GOLD_W4_5 = 356,
    SPR_GOLD_W4_6 = 357,
    SPR_GOLD_W4_7 = 358,
    SPR_GOLD_W4_8 = 359,

    SPR_GOLD_PAIN_1 = 360,

    SPR_GOLD_WRIST_1 = 361,
    SPR_GOLD_WRIST_2 = 362,

    SPR_GOLD_SHOOT1 = 363,
    SPR_GOLD_SHOOT2 = 364,
    SPR_GOLD_SHOOT3 = 365,

    SPR_GOLD_WARP1 = 366,
    SPR_GOLD_WARP2 = 367,
    SPR_GOLD_WARP3 = 368,
    SPR_GOLD_WARP4 = 369,
    SPR_GOLD_WARP5 = 370,
#endif

#ifdef BSTONE_PS
    SPR_GOLD_DEATH1 = 371,
    SPR_GOLD_DEATH2 = 372,
    SPR_GOLD_DEATH3 = 373,
    SPR_GOLD_DEATH4 = 374,
    SPR_GOLD_DEATH5 = 375,
    SPR_MGOLD_OUCH = 376,

    SPR_GOLD_MORPH1 = 377,
    SPR_GOLD_MORPH2 = 378,
    SPR_GOLD_MORPH3 = 379,
    SPR_GOLD_MORPH4 = 380,
    SPR_GOLD_MORPH5 = 381,
    SPR_GOLD_MORPH6 = 382,
    SPR_GOLD_MORPH7 = 383,
    SPR_GOLD_MORPH8 = 384,

    SPR_MGOLD_WALK1 = 385,
    SPR_MGOLD_WALK2 = 386,
    SPR_MGOLD_WALK3 = 387,
    SPR_MGOLD_WALK4 = 388,
    SPR_MGOLD_ATTACK1 = 389,
    SPR_MGOLD_ATTACK2 = 390,
    SPR_MGOLD_ATTACK3 = 391,
    SPR_MGOLD_ATTACK4 = 392,

    SPR_MGOLD_SHOT1 = 393,
    SPR_MGOLD_SHOT2 = 394,
    SPR_MGOLD_SHOT3 = 395,
    SPR_MGOLD_SHOT_EXP1 = 396,
    SPR_MGOLD_SHOT_EXP2 = 397,
    SPR_MGOLD_SHOT_EXP3 = 398,
#endif

    //
    // Volatile Material Transporter
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_GSCOUT_W1_1 = 360,
    SPR_GSCOUT_W1_2 = 361,
    SPR_GSCOUT_W1_3 = 362,
    SPR_GSCOUT_W1_4 = 363,
    SPR_GSCOUT_W1_5 = 364,
    SPR_GSCOUT_W1_6 = 365,
    SPR_GSCOUT_W1_7 = 366,
    SPR_GSCOUT_W1_8 = 367,

    SPR_GSCOUT_W2_1 = 368,
    SPR_GSCOUT_W2_2 = 369,
    SPR_GSCOUT_W2_3 = 370,
    SPR_GSCOUT_W2_4 = 371,
    SPR_GSCOUT_W2_5 = 372,
    SPR_GSCOUT_W2_6 = 373,
    SPR_GSCOUT_W2_7 = 374,
    SPR_GSCOUT_W2_8 = 375,

    SPR_GSCOUT_W3_1 = 376,
    SPR_GSCOUT_W3_2 = 377,
    SPR_GSCOUT_W3_3 = 378,
    SPR_GSCOUT_W3_4 = 379,
    SPR_GSCOUT_W3_5 = 380,
    SPR_GSCOUT_W3_6 = 381,
    SPR_GSCOUT_W3_7 = 382,
    SPR_GSCOUT_W3_8 = 383,

    SPR_GSCOUT_W4_1 = 384,
    SPR_GSCOUT_W4_2 = 385,
    SPR_GSCOUT_W4_3 = 386,
    SPR_GSCOUT_W4_4 = 387,
    SPR_GSCOUT_W4_5 = 388,
    SPR_GSCOUT_W4_6 = 389,
    SPR_GSCOUT_W4_7 = 390,
    SPR_GSCOUT_W4_8 = 391,

    SPR_GSCOUT_DIE1 = 392,
    SPR_GSCOUT_DIE2 = 393,
    SPR_GSCOUT_DIE3 = 394,
    SPR_GSCOUT_DIE4 = 395,
    SPR_GSCOUT_DIE5 = 396,
    SPR_GSCOUT_DIE6 = 397,
    SPR_GSCOUT_DIE7 = 398,
    SPR_GSCOUT_DIE8 = 399,

    SPR_GSCOUT_DEAD = 400,
#endif
#else
    SPR_GSCOUT_W1_1 = 399,
    SPR_GSCOUT_W1_2 = 400,
    SPR_GSCOUT_W1_3 = 401,
    SPR_GSCOUT_W1_4 = 402,
    SPR_GSCOUT_W1_5 = 403,
    SPR_GSCOUT_W1_6 = 404,
    SPR_GSCOUT_W1_7 = 405,
    SPR_GSCOUT_W1_8 = 406,

    SPR_GSCOUT_W2_1 = 407,
    SPR_GSCOUT_W2_2 = 408,
    SPR_GSCOUT_W2_3 = 409,
    SPR_GSCOUT_W2_4 = 410,
    SPR_GSCOUT_W2_5 = 411,
    SPR_GSCOUT_W2_6 = 412,
    SPR_GSCOUT_W2_7 = 413,
    SPR_GSCOUT_W2_8 = 414,

    SPR_GSCOUT_W3_1 = 415,
    SPR_GSCOUT_W3_2 = 416,
    SPR_GSCOUT_W3_3 = 417,
    SPR_GSCOUT_W3_4 = 418,
    SPR_GSCOUT_W3_5 = 419,
    SPR_GSCOUT_W3_6 = 420,
    SPR_GSCOUT_W3_7 = 421,
    SPR_GSCOUT_W3_8 = 422,

    SPR_GSCOUT_W4_1 = 423,
    SPR_GSCOUT_W4_2 = 424,
    SPR_GSCOUT_W4_3 = 425,
    SPR_GSCOUT_W4_4 = 426,
    SPR_GSCOUT_W4_5 = 427,
    SPR_GSCOUT_W4_6 = 428,
    SPR_GSCOUT_W4_7 = 429,
    SPR_GSCOUT_W4_8 = 430,

    SPR_GSCOUT_DIE1 = 431,
    SPR_GSCOUT_DIE2 = 432,
    SPR_GSCOUT_DIE3 = 433,
    SPR_GSCOUT_DIE4 = 434,
    SPR_GSCOUT_DIE5 = 435,
    SPR_GSCOUT_DIE6 = 436,
    SPR_GSCOUT_DIE7 = 437,
    SPR_GSCOUT_DIE8 = 438,

    SPR_GSCOUT_DEAD = 439,
#endif

    //
    // Perscan Drone
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_FSCOUT_W1_1 = 401,
    SPR_FSCOUT_W1_2 = 402,
    SPR_FSCOUT_W1_3 = 403,
    SPR_FSCOUT_W1_4 = 404,
    SPR_FSCOUT_W1_5 = 405,
    SPR_FSCOUT_W1_6 = 406,
    SPR_FSCOUT_W1_7 = 407,
    SPR_FSCOUT_W1_8 = 408,

    SPR_FSCOUT_W2_1 = 409,
    SPR_FSCOUT_W2_2 = 410,
    SPR_FSCOUT_W2_3 = 411,
    SPR_FSCOUT_W2_4 = 412,
    SPR_FSCOUT_W2_5 = 413,
    SPR_FSCOUT_W2_6 = 414,
    SPR_FSCOUT_W2_7 = 415,
    SPR_FSCOUT_W2_8 = 416,

    SPR_FSCOUT_W3_1 = 417,
    SPR_FSCOUT_W3_2 = 418,
    SPR_FSCOUT_W3_3 = 419,
    SPR_FSCOUT_W3_4 = 420,
    SPR_FSCOUT_W3_5 = 421,
    SPR_FSCOUT_W3_6 = 422,
    SPR_FSCOUT_W3_7 = 423,
    SPR_FSCOUT_W3_8 = 424,

    SPR_FSCOUT_W4_1 = 425,
    SPR_FSCOUT_W4_2 = 426,
    SPR_FSCOUT_W4_3 = 427,
    SPR_FSCOUT_W4_4 = 428,
    SPR_FSCOUT_W4_5 = 429,
    SPR_FSCOUT_W4_6 = 430,
    SPR_FSCOUT_W4_7 = 431,
    SPR_FSCOUT_W4_8 = 432,

    SPR_FSCOUT_DIE1 = 433,
    SPR_FSCOUT_DIE2 = 434,
    SPR_FSCOUT_DIE3 = 435,
    SPR_FSCOUT_DIE4 = 436,
    SPR_FSCOUT_DIE5 = 437,
    SPR_FSCOUT_DIE6 = 438,
    SPR_FSCOUT_DIE7 = 439,
    SPR_FSCOUT_DEAD = 440,
#else
    SPR_FSCOUT_W1_1 = 331,
    SPR_FSCOUT_W1_2 = 332,
    SPR_FSCOUT_W1_3 = 333,
    SPR_FSCOUT_W1_4 = 334,
    SPR_FSCOUT_W1_5 = 335,
    SPR_FSCOUT_W1_6 = 336,
    SPR_FSCOUT_W1_7 = 337,
    SPR_FSCOUT_W1_8 = 338,

    SPR_FSCOUT_DIE1 = 339,
    SPR_FSCOUT_DIE2 = 340,
    SPR_FSCOUT_DIE3 = 341,
    SPR_FSCOUT_DIE4 = 342,
    SPR_FSCOUT_DIE5 = 343,
    SPR_FSCOUT_DIE6 = 344,
    SPR_FSCOUT_DIE7 = 345,
    SPR_FSCOUT_DEAD = 346,
#endif
#else
    SPR_FSCOUT_W1_1 = 440,
    SPR_FSCOUT_W1_2 = 441,
    SPR_FSCOUT_W1_3 = 442,
    SPR_FSCOUT_W1_4 = 443,
    SPR_FSCOUT_W1_5 = 444,
    SPR_FSCOUT_W1_6 = 445,
    SPR_FSCOUT_W1_7 = 446,
    SPR_FSCOUT_W1_8 = 447,

    SPR_FSCOUT_W2_1 = 448,
    SPR_FSCOUT_W2_2 = 449,
    SPR_FSCOUT_W2_3 = 450,
    SPR_FSCOUT_W2_4 = 451,
    SPR_FSCOUT_W2_5 = 452,
    SPR_FSCOUT_W2_6 = 453,
    SPR_FSCOUT_W2_7 = 454,
    SPR_FSCOUT_W2_8 = 455,

    SPR_FSCOUT_W3_1 = 456,
    SPR_FSCOUT_W3_2 = 457,
    SPR_FSCOUT_W3_3 = 458,
    SPR_FSCOUT_W3_4 = 459,
    SPR_FSCOUT_W3_5 = 460,
    SPR_FSCOUT_W3_6 = 461,
    SPR_FSCOUT_W3_7 = 462,
    SPR_FSCOUT_W3_8 = 463,

    SPR_FSCOUT_W4_1 = 464,
    SPR_FSCOUT_W4_2 = 465,
    SPR_FSCOUT_W4_3 = 466,
    SPR_FSCOUT_W4_4 = 467,
    SPR_FSCOUT_W4_5 = 468,
    SPR_FSCOUT_W4_6 = 469,
    SPR_FSCOUT_W4_7 = 470,
    SPR_FSCOUT_W4_8 = 471,

    SPR_FSCOUT_DIE1 = 472,
    SPR_FSCOUT_DIE2 = 473,
    SPR_FSCOUT_DIE3 = 474,
    SPR_FSCOUT_DIE4 = 475,
    SPR_FSCOUT_DIE5 = 476,
    SPR_FSCOUT_DIE6 = 477,
    SPR_FSCOUT_DIE7 = 478,
    SPR_FSCOUT_DEAD = 479,
#endif

    //
    // General Explosion Anim
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_EXPLOSION_1 = 441,
    SPR_EXPLOSION_2 = 442,
    SPR_EXPLOSION_3 = 443,
    SPR_EXPLOSION_4 = 444,
    SPR_EXPLOSION_5 = 445,
#else
    SPR_EXPLOSION_1 = 347,
    SPR_EXPLOSION_2 = 348,
    SPR_EXPLOSION_3 = 349,
    SPR_EXPLOSION_4 = 350,
    SPR_EXPLOSION_5 = 351,
#endif
#else
    SPR_EXPLOSION_1 = 480,
    SPR_EXPLOSION_2 = 481,
    SPR_EXPLOSION_3 = 482,
    SPR_EXPLOSION_4 = 483,
    SPR_EXPLOSION_5 = 484,
#endif

    //
    // Projection Generator
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_VITAL_STAND = 446,
    SPR_VITAL_DIE_1 = 447,
    SPR_VITAL_DIE_2 = 448,
    SPR_VITAL_DIE_3 = 449,
    SPR_VITAL_DIE_4 = 450,
    SPR_VITAL_DIE_5 = 451,
    SPR_VITAL_DIE_6 = 452,
    SPR_VITAL_DIE_7 = 453,
    SPR_VITAL_DIE_8 = 454,
    SPR_VITAL_DEAD_1 = 455,
    SPR_VITAL_DEAD_2 = 456,
    SPR_VITAL_DEAD_3 = 457,
    SPR_VITAL_OUCH = 458,
#endif
#endif

    //
    // Security Cube
    //
#ifdef BSTONE_PS
    SPR_CUBE1 = 485,
    SPR_CUBE2 = 486,
    SPR_CUBE3 = 487,
    SPR_CUBE4 = 488,
    SPR_CUBE5 = 489,
    SPR_CUBE6 = 490,
    SPR_CUBE7 = 491,
    SPR_CUBE8 = 492,
    SPR_CUBE9 = 493,
    SPR_CUBE10 = 494,
    SPR_CUBE_EXP1 = 495,
    SPR_CUBE_EXP2 = 496,
    SPR_CUBE_EXP3 = 497,
    SPR_CUBE_EXP4 = 498,
    SPR_CUBE_EXP5 = 499,
    SPR_CUBE_EXP6 = 500,
    SPR_CUBE_EXP7 = 501,
    SPR_CUBE_EXP8 = 502,
    SPR_DEAD_CUBE = 503,
#endif

    //
    // Red Security Lights
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_SECURITY_NORMAL = 459,
    SPR_SECURITY_ALERT = 460,
#else
    SPR_SECURITY_NORMAL = 352,
    SPR_SECURITY_ALERT = 353,
#endif
#else
    SPR_SECURITY_NORMAL = 504,
    SPR_SECURITY_ALERT = 505,
#endif

    //
    // P.O.D. Alien
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_POD_EGG = 461,
    SPR_POD_HATCH1 = 462,
    SPR_POD_HATCH2 = 463,
    SPR_POD_HATCH3 = 464,
    SPR_POD_WALK1 = 465,
    SPR_POD_WALK2 = 466,
    SPR_POD_WALK3 = 467,
    SPR_POD_WALK4 = 468,
    SPR_POD_ATTACK1 = 469,
    SPR_POD_ATTACK2 = 470,
    SPR_POD_ATTACK3 = 471,
    SPR_POD_OUCH = 472,
    SPR_POD_DIE1 = 473,
    SPR_POD_DIE2 = 474,
    SPR_POD_DIE3 = 475,
    SPR_POD_SPIT1 = 476,
    SPR_POD_SPIT2 = 477,
    SPR_POD_SPIT3 = 478,
#else
    SPR_POD_EGG = 354,
    SPR_POD_HATCH1 = 355,
    SPR_POD_HATCH2 = 356,
    SPR_POD_HATCH3 = 357,
    SPR_POD_WALK1 = 358,
    SPR_POD_WALK2 = 359,
    SPR_POD_WALK3 = 360,
    SPR_POD_WALK4 = 361,
    SPR_POD_ATTACK1 = 362,
    SPR_POD_ATTACK2 = 363,
    SPR_POD_ATTACK3 = 364,
    SPR_POD_OUCH = 365,
    SPR_POD_DIE1 = 366,
    SPR_POD_DIE2 = 367,
    SPR_POD_DIE3 = 368,
    SPR_POD_SPIT1 = 369,
    SPR_POD_SPIT2 = 370,
    SPR_POD_SPIT3 = 371,
#endif
#else
    SPR_POD_EGG = 506,
    SPR_POD_HATCH1 = 507,
    SPR_POD_HATCH2 = 508,
    SPR_POD_HATCH3 = 509,
    SPR_POD_WALK1 = 510,
    SPR_POD_WALK2 = 511,
    SPR_POD_WALK3 = 512,
    SPR_POD_WALK4 = 513,
    SPR_POD_ATTACK1 = 514,
    SPR_POD_ATTACK2 = 515,
    SPR_POD_ATTACK3 = 516,
    SPR_POD_OUCH = 517,
    SPR_POD_DIE1 = 518,
    SPR_POD_DIE2 = 519,
    SPR_POD_DIE3 = 520,
    SPR_POD_SPIT1 = 521,
    SPR_POD_SPIT2 = 522,
    SPR_POD_SPIT3 = 523,
#endif

    //
    // Electro-Alien
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_ELEC_APPEAR1 = 479,
    SPR_ELEC_APPEAR2 = 480,
    SPR_ELEC_APPEAR3 = 481,
    SPR_ELEC_WALK1 = 482,
    SPR_ELEC_WALK2 = 483,
    SPR_ELEC_WALK3 = 484,
    SPR_ELEC_WALK4 = 485,
    SPR_ELEC_OUCH = 486,
    SPR_ELEC_SHOOT1 = 487,
    SPR_ELEC_SHOOT2 = 488,
    SPR_ELEC_SHOOT3 = 489,
    SPR_ELEC_DIE1 = 490,
    SPR_ELEC_DIE2 = 491,
    SPR_ELEC_DIE3 = 492,
    SPR_ELEC_SHOT1 = 493,
    SPR_ELEC_SHOT2 = 494,
    SPR_ELEC_SHOT_EXP1 = 495,
    SPR_ELEC_SHOT_EXP2 = 496,
#else
    SPR_ELEC_APPEAR1 = 372,
    SPR_ELEC_APPEAR2 = 373,
    SPR_ELEC_APPEAR3 = 374,
    SPR_ELEC_WALK1 = 375,
    SPR_ELEC_WALK2 = 376,
    SPR_ELEC_WALK3 = 377,
    SPR_ELEC_WALK4 = 378,
    SPR_ELEC_OUCH = 379,
    SPR_ELEC_SHOOT1 = 380,
    SPR_ELEC_SHOOT2 = 381,
    SPR_ELEC_SHOOT3 = 382,
    SPR_ELEC_DIE1 = 383,
    SPR_ELEC_DIE2 = 384,
    SPR_ELEC_DIE3 = 385,
    SPR_ELEC_SHOT1 = 386,
    SPR_ELEC_SHOT2 = 387,
    SPR_ELEC_SHOT_EXP1 = 388,
    SPR_ELEC_SHOT_EXP2 = 389,
#endif
#else
    SPR_ELEC_APPEAR1 = 524,
    SPR_ELEC_APPEAR2 = 525,
    SPR_ELEC_APPEAR3 = 526,
    SPR_ELEC_WALK1 = 527,
    SPR_ELEC_WALK2 = 528,
    SPR_ELEC_WALK3 = 529,
    SPR_ELEC_WALK4 = 530,
    SPR_ELEC_OUCH = 531,
    SPR_ELEC_SHOOT1 = 532,
    SPR_ELEC_SHOOT2 = 533,
    SPR_ELEC_SHOOT3 = 534,
    SPR_ELEC_DIE1 = 535,
    SPR_ELEC_DIE2 = 536,
    SPR_ELEC_DIE3 = 537,
    SPR_ELEC_SHOT1 = 538,
    SPR_ELEC_SHOT2 = 539,
    SPR_ELEC_SHOT_EXP1 = 540,
    SPR_ELEC_SHOT_EXP2 = 541,
#endif

    //
    // ElectroSphere
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_ELECTRO_SPHERE_ROAM1 = 497,
    SPR_ELECTRO_SPHERE_ROAM2 = 498,
    SPR_ELECTRO_SPHERE_ROAM3 = 499,
    SPR_ELECTRO_SPHERE_OUCH = 500,
    SPR_ELECTRO_SPHERE_DIE1 = 501,
    SPR_ELECTRO_SPHERE_DIE2 = 502,
    SPR_ELECTRO_SPHERE_DIE3 = 503,
    SPR_ELECTRO_SPHERE_DIE4 = 504,
#else
    SPR_ELECTRO_SPHERE_ROAM1 = 390,
    SPR_ELECTRO_SPHERE_ROAM2 = 391,
    SPR_ELECTRO_SPHERE_ROAM3 = 392,
    SPR_ELECTRO_SPHERE_OUCH = 393,
    SPR_ELECTRO_SPHERE_DIE1 = 394,
    SPR_ELECTRO_SPHERE_DIE2 = 395,
    SPR_ELECTRO_SPHERE_DIE3 = 396,
    SPR_ELECTRO_SPHERE_DIE4 = 397,
#endif
#else
    SPR_ELECTRO_SPHERE_ROAM1 = 542,
    SPR_ELECTRO_SPHERE_ROAM2 = 543,
    SPR_ELECTRO_SPHERE_ROAM3 = 544,
    SPR_ELECTRO_SPHERE_OUCH = 545,
    SPR_ELECTRO_SPHERE_DIE1 = 546,
    SPR_ELECTRO_SPHERE_DIE2 = 547,
    SPR_ELECTRO_SPHERE_DIE3 = 548,
    SPR_ELECTRO_SPHERE_DIE4 = 549,
#endif

    //
    // Genetic Guard
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_GENETIC_W1 = 505,
    SPR_GENETIC_W2 = 506,
    SPR_GENETIC_W3 = 507,
    SPR_GENETIC_W4 = 508,
    SPR_GENETIC_SWING1 = 509,
    SPR_GENETIC_SWING2 = 510,
    SPR_GENETIC_SWING3 = 511,
    SPR_GENETIC_DEAD = 512,
    SPR_GENETIC_DIE1 = 513,
    SPR_GENETIC_DIE2 = 514,
    SPR_GENETIC_DIE3 = 515,
    SPR_GENETIC_DIE4 = 516,
    SPR_GENETIC_OUCH = 517,
    SPR_GENETIC_SHOOT1 = 518,
    SPR_GENETIC_SHOOT2 = 519,
    SPR_GENETIC_SHOOT3 = 520,
#else
    SPR_GENETIC_W1 = 398,
    SPR_GENETIC_W2 = 399,
    SPR_GENETIC_W3 = 400,
    SPR_GENETIC_W4 = 401,
    SPR_GENETIC_SWING1 = 402,
    SPR_GENETIC_SWING2 = 403,
    SPR_GENETIC_SWING3 = 404,
    SPR_GENETIC_DEAD = 405,
    SPR_GENETIC_DIE1 = 406,
    SPR_GENETIC_DIE2 = 407,
    SPR_GENETIC_DIE3 = 408,
    SPR_GENETIC_DIE4 = 409,
    SPR_GENETIC_OUCH = 410,
    SPR_GENETIC_SHOOT1 = 411,
    SPR_GENETIC_SHOOT2 = 412,
    SPR_GENETIC_SHOOT3 = 413,
#endif
#else
    SPR_GENETIC_W1 = 550,
    SPR_GENETIC_W2 = 551,
    SPR_GENETIC_W3 = 552,
    SPR_GENETIC_W4 = 553,
    SPR_GENETIC_SWING1 = 554,
    SPR_GENETIC_SWING2 = 555,
    SPR_GENETIC_SWING3 = 556,
    SPR_GENETIC_DEAD = 557,
    SPR_GENETIC_DIE1 = 558,
    SPR_GENETIC_DIE2 = 559,
    SPR_GENETIC_DIE3 = 560,
    SPR_GENETIC_DIE4 = 561,
    SPR_GENETIC_OUCH = 562,
    SPR_GENETIC_SHOOT1 = 563,
    SPR_GENETIC_SHOOT2 = 564,
    SPR_GENETIC_SHOOT3 = 565,
#endif

    //
    // Experimental Mech-Sentinel
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_MUTHUM1_W1 = 521,
    SPR_MUTHUM1_W2 = 522,
    SPR_MUTHUM1_W3 = 523,
    SPR_MUTHUM1_W4 = 524,
    SPR_MUTHUM1_SWING1 = 525,
    SPR_MUTHUM1_SWING2 = 526,
    SPR_MUTHUM1_SWING3 = 527,
    SPR_MUTHUM1_DEAD = 528,
    SPR_MUTHUM1_DIE1 = 529,
    SPR_MUTHUM1_DIE2 = 530,
    SPR_MUTHUM1_DIE3 = 531,
    SPR_MUTHUM1_DIE4 = 532,
    SPR_MUTHUM1_OUCH = 533,
    SPR_MUTHUM1_SPIT1 = 534,
    SPR_MUTHUM1_SPIT2 = 535,
    SPR_MUTHUM1_SPIT3 = 536,
#else
    SPR_MUTHUM1_W1 = 414,
    SPR_MUTHUM1_W2 = 415,
    SPR_MUTHUM1_W3 = 416,
    SPR_MUTHUM1_W4 = 417,
    SPR_MUTHUM1_SWING1 = 418,
    SPR_MUTHUM1_SWING2 = 419,
    SPR_MUTHUM1_SWING3 = 420,
    SPR_MUTHUM1_DEAD = 421,
    SPR_MUTHUM1_DIE1 = 422,
    SPR_MUTHUM1_DIE2 = 423,
    SPR_MUTHUM1_DIE3 = 424,
    SPR_MUTHUM1_DIE4 = 425,
    SPR_MUTHUM1_OUCH = 426,
    SPR_MUTHUM1_SPIT1 = 427,
    SPR_MUTHUM1_SPIT2 = 428,
    SPR_MUTHUM1_SPIT3 = 429,
#endif
#else
    SPR_MUTHUM1_W1 = 566,
    SPR_MUTHUM1_W2 = 567,
    SPR_MUTHUM1_W3 = 568,
    SPR_MUTHUM1_W4 = 569,
    SPR_MUTHUM1_SWING1 = 570,
    SPR_MUTHUM1_SWING2 = 571,
    SPR_MUTHUM1_SWING3 = 572,
    SPR_MUTHUM1_DEAD = 573,
    SPR_MUTHUM1_DIE1 = 574,
    SPR_MUTHUM1_DIE2 = 575,
    SPR_MUTHUM1_DIE3 = 576,
    SPR_MUTHUM1_DIE4 = 577,
    SPR_MUTHUM1_OUCH = 578,
    SPR_MUTHUM1_SPIT1 = 579,
    SPR_MUTHUM1_SPIT2 = 580,
    SPR_MUTHUM1_SPIT3 = 581,
#endif

    //
    // Muntant human type 2
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_MUTHUM2_W1 = 537,
    SPR_MUTHUM2_W2 = 538,
    SPR_MUTHUM2_W3 = 539,
    SPR_MUTHUM2_W4 = 540,
    SPR_MUTHUM2_SWING1 = 541,
    SPR_MUTHUM2_SWING2 = 542,
    SPR_MUTHUM2_SWING3 = 543,
    SPR_MUTHUM2_DEAD = 544,
    SPR_MUTHUM2_DIE1 = 545,
    SPR_MUTHUM2_DIE2 = 546,
    SPR_MUTHUM2_DIE3 = 547,
    SPR_MUTHUM2_DIE4 = 548,
    SPR_MUTHUM2_OUCH = 549,
    SPR_MUTHUM2_SPIT1 = 550,
    SPR_MUTHUM2_SPIT2 = 551,
    SPR_MUTHUM2_SPIT3 = 552,
#endif
#else
    SPR_MUTHUM2_W1 = 582,
    SPR_MUTHUM2_W2 = 583,
    SPR_MUTHUM2_W3 = 584,
    SPR_MUTHUM2_W4 = 585,
    SPR_MUTHUM2_SWING1 = 586,
    SPR_MUTHUM2_SWING2 = 587,
    SPR_MUTHUM2_SWING3 = 588,
    SPR_MUTHUM2_DEAD = 589,
    SPR_MUTHUM2_DIE1 = 590,
    SPR_MUTHUM2_DIE2 = 591,
    SPR_MUTHUM2_DIE3 = 592,
    SPR_MUTHUM2_DIE4 = 593,
    SPR_MUTHUM2_OUCH = 594,
    SPR_MUTHUM2_SPIT1 = 595,
    SPR_MUTHUM2_SPIT2 = 596,
    SPR_MUTHUM2_SPIT3 = 597,

    SPR_MUTHUM2_MORPH1 = 598,
    SPR_MUTHUM2_MORPH2 = 599,
    SPR_MUTHUM2_MORPH3 = 600,
    SPR_MUTHUM2_MORPH4 = 601,
    SPR_MUTHUM2_MORPH5 = 602,
    SPR_MUTHUM2_MORPH6 = 603,
    SPR_MUTHUM2_MORPH7 = 604,
    SPR_MUTHUM2_MORPH8 = 605,
    SPR_MUTHUM2_MORPH9 = 606,
#endif

    //
    // Large Cantained Alien
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_LCAN_ALIEN_READY = 553,
    SPR_LCAN_ALIEN_B1 = 554,
    SPR_LCAN_ALIEN_B2 = 555,
    SPR_LCAN_ALIEN_B3 = 556,
    SPR_LCAN_ALIEN_EMPTY = 557,

    SPR_LCAN_ALIEN_W1 = 558,
    SPR_LCAN_ALIEN_W2 = 559,
    SPR_LCAN_ALIEN_W3 = 560,
    SPR_LCAN_ALIEN_W4 = 561,
    SPR_LCAN_ALIEN_SWING1 = 562,
    SPR_LCAN_ALIEN_SWING2 = 563,
    SPR_LCAN_ALIEN_SWING3 = 564,
    SPR_LCAN_ALIEN_DEAD = 565,
    SPR_LCAN_ALIEN_DIE1 = 566,
    SPR_LCAN_ALIEN_DIE2 = 567,
    SPR_LCAN_ALIEN_DIE3 = 568,
    SPR_LCAN_ALIEN_DIE4 = 569,
    SPR_LCAN_ALIEN_OUCH = 570,
    SPR_LCAN_ALIEN_SPIT1 = 571,
    SPR_LCAN_ALIEN_SPIT2 = 572,
    SPR_LCAN_ALIEN_SPIT3 = 573,
#endif
#else
    SPR_LCAN_ALIEN_READY = 607,
    SPR_LCAN_ALIEN_B1 = 608,
    SPR_LCAN_ALIEN_B2 = 609,
    SPR_LCAN_ALIEN_B3 = 610,
    SPR_LCAN_ALIEN_EMPTY = 611,

    SPR_LCAN_ALIEN_W1 = 612,
    SPR_LCAN_ALIEN_W2 = 613,
    SPR_LCAN_ALIEN_W3 = 614,
    SPR_LCAN_ALIEN_W4 = 615,
    SPR_LCAN_ALIEN_SWING1 = 616,
    SPR_LCAN_ALIEN_SWING2 = 617,
    SPR_LCAN_ALIEN_SWING3 = 618,
    SPR_LCAN_ALIEN_DEAD = 619,
    SPR_LCAN_ALIEN_DIE1 = 620,
    SPR_LCAN_ALIEN_DIE2 = 621,
    SPR_LCAN_ALIEN_DIE3 = 622,
    SPR_LCAN_ALIEN_DIE4 = 623,
    SPR_LCAN_ALIEN_OUCH = 624,
    SPR_LCAN_ALIEN_SPIT1 = 625,
    SPR_LCAN_ALIEN_SPIT2 = 626,
    SPR_LCAN_ALIEN_SPIT3 = 627,
#endif

    //
    // Small Canister Alien
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_SCAN_ALIEN_READY = 574,
    SPR_SCAN_ALIEN_B1 = 575,
    SPR_SCAN_ALIEN_B2 = 576,
    SPR_SCAN_ALIEN_B3 = 577,
    SPR_SCAN_ALIEN_EMPTY = 578,

    SPR_SCAN_ALIEN_W1 = 579,
    SPR_SCAN_ALIEN_W2 = 580,
    SPR_SCAN_ALIEN_W3 = 581,
    SPR_SCAN_ALIEN_W4 = 582,
    SPR_SCAN_ALIEN_SWING1 = 583,
    SPR_SCAN_ALIEN_SWING2 = 584,
    SPR_SCAN_ALIEN_SWING3 = 585,
    SPR_SCAN_ALIEN_DEAD = 586,
    SPR_SCAN_ALIEN_DIE1 = 587,
    SPR_SCAN_ALIEN_DIE2 = 588,
    SPR_SCAN_ALIEN_DIE3 = 589,
    SPR_SCAN_ALIEN_DIE4 = 590,
    SPR_SCAN_ALIEN_OUCH = 591,

    SPR_SCAN_ALIEN_SPIT1 = 592,
    SPR_SCAN_ALIEN_SPIT2 = 593,
    SPR_SCAN_ALIEN_SPIT3 = 594,
#else
    SPR_SCAN_ALIEN_READY = 430,
    SPR_SCAN_ALIEN_B1 = 431,
    SPR_SCAN_ALIEN_B2 = 432,
    SPR_SCAN_ALIEN_B3 = 433,
    SPR_SCAN_ALIEN_EMPTY = 434,

    SPR_SCAN_ALIEN_W1 = 435,
    SPR_SCAN_ALIEN_W2 = 436,
    SPR_SCAN_ALIEN_W3 = 437,
    SPR_SCAN_ALIEN_W4 = 438,
    SPR_SCAN_ALIEN_SWING1 = 439,
    SPR_SCAN_ALIEN_SWING2 = 440,
    SPR_SCAN_ALIEN_SWING3 = 441,
    SPR_SCAN_ALIEN_DEAD = 442,
    SPR_SCAN_ALIEN_DIE1 = 443,
    SPR_SCAN_ALIEN_DIE2 = 444,
    SPR_SCAN_ALIEN_DIE3 = 445,
    SPR_SCAN_ALIEN_DIE4 = 446,
    SPR_SCAN_ALIEN_OUCH = 447,

    SPR_SCAN_ALIEN_SPIT1 = 448,
    SPR_SCAN_ALIEN_SPIT2 = 449,
    SPR_SCAN_ALIEN_SPIT3 = 450,
#endif
#else
    SPR_SCAN_ALIEN_READY = 628,
    SPR_SCAN_ALIEN_B1 = 629,
    SPR_SCAN_ALIEN_B2 = 630,
    SPR_SCAN_ALIEN_B3 = 631,
    SPR_SCAN_ALIEN_EMPTY = 632,

    SPR_SCAN_ALIEN_W1 = 633,
    SPR_SCAN_ALIEN_W2 = 634,
    SPR_SCAN_ALIEN_W3 = 635,
    SPR_SCAN_ALIEN_W4 = 636,
    SPR_SCAN_ALIEN_SWING1 = 637,
    SPR_SCAN_ALIEN_SWING2 = 638,
    SPR_SCAN_ALIEN_SWING3 = 639,
    SPR_SCAN_ALIEN_DEAD = 640,
    SPR_SCAN_ALIEN_DIE1 = 641,
    SPR_SCAN_ALIEN_DIE2 = 642,
    SPR_SCAN_ALIEN_DIE3 = 643,
    SPR_SCAN_ALIEN_DIE4 = 644,
    SPR_SCAN_ALIEN_OUCH = 645,

    SPR_SCAN_ALIEN_SPIT1 = 646,
    SPR_SCAN_ALIEN_SPIT2 = 647,
    SPR_SCAN_ALIEN_SPIT3 = 648,
#endif

    //
    // Gurney Mutant
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_GURNEY_MUT_READY = 595,
    SPR_GURNEY_MUT_B1 = 596,
    SPR_GURNEY_MUT_B2 = 597,
    SPR_GURNEY_MUT_B3 = 598,
    SPR_GURNEY_MUT_EMPTY = 599,

    SPR_GURNEY_MUT_W1 = 600,
    SPR_GURNEY_MUT_W2 = 601,
    SPR_GURNEY_MUT_W3 = 602,
    SPR_GURNEY_MUT_W4 = 603,
    SPR_GURNEY_MUT_SWING1 = 604,
    SPR_GURNEY_MUT_SWING2 = 605,
    SPR_GURNEY_MUT_SWING3 = 606,
    SPR_GURNEY_MUT_DEAD = 607,
    SPR_GURNEY_MUT_DIE1 = 608,
    SPR_GURNEY_MUT_DIE2 = 609,
    SPR_GURNEY_MUT_DIE3 = 610,
    SPR_GURNEY_MUT_DIE4 = 611,
    SPR_GURNEY_MUT_OUCH = 612,
#endif
#else
    SPR_GURNEY_MUT_READY = 649,
    SPR_GURNEY_MUT_B1 = 650,
    SPR_GURNEY_MUT_B2 = 651,
    SPR_GURNEY_MUT_B3 = 652,
    SPR_GURNEY_MUT_EMPTY = 653,

    SPR_GURNEY_MUT_W1 = 654,
    SPR_GURNEY_MUT_W2 = 655,
    SPR_GURNEY_MUT_W3 = 656,
    SPR_GURNEY_MUT_W4 = 657,
    SPR_GURNEY_MUT_SWING1 = 658,
    SPR_GURNEY_MUT_SWING2 = 659,
    SPR_GURNEY_MUT_SWING3 = 660,
    SPR_GURNEY_MUT_DEAD = 661,
    SPR_GURNEY_MUT_DIE1 = 662,
    SPR_GURNEY_MUT_DIE2 = 663,
    SPR_GURNEY_MUT_DIE3 = 664,
    SPR_GURNEY_MUT_DIE4 = 665,
    SPR_GURNEY_MUT_OUCH = 666,
#endif

    //
    // Liquid Alien
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_LIQUID_M1 = 613,
    SPR_LIQUID_M2 = 614,
    SPR_LIQUID_M3 = 615,
    SPR_LIQUID_R1 = 616,
    SPR_LIQUID_R2 = 617,
    SPR_LIQUID_R3 = 618,
    SPR_LIQUID_R4 = 619,
    SPR_LIQUID_S1 = 620,
    SPR_LIQUID_S2 = 621,
    SPR_LIQUID_S3 = 622,
    SPR_LIQUID_OUCH = 623,
    SPR_LIQUID_DIE_1 = 624,
    SPR_LIQUID_DIE_2 = 625,
    SPR_LIQUID_DIE_3 = 626,
    SPR_LIQUID_DIE_4 = 627,
    SPR_LIQUID_DEAD = 628,
    SPR_LIQUID_SHOT_FLY_1 = 629,
    SPR_LIQUID_SHOT_FLY_2 = 630,
    SPR_LIQUID_SHOT_FLY_3 = 631,
    SPR_LIQUID_SHOT_BURST_1 = 632,
    SPR_LIQUID_SHOT_BURST_2 = 633,
    SPR_LIQUID_SHOT_BURST_3 = 634,
#endif
#else
    SPR_LIQUID_M1 = 667,
    SPR_LIQUID_M2 = 668,
    SPR_LIQUID_M3 = 669,
    SPR_LIQUID_R1 = 670,
    SPR_LIQUID_R2 = 671,
    SPR_LIQUID_R3 = 672,
    SPR_LIQUID_R4 = 673,
    SPR_LIQUID_S1 = 674,
    SPR_LIQUID_S2 = 675,
    SPR_LIQUID_S3 = 676,
    SPR_LIQUID_OUCH = 677,
    SPR_LIQUID_DIE_1 = 678,
    SPR_LIQUID_DIE_2 = 679,
    SPR_LIQUID_DIE_3 = 680,
    SPR_LIQUID_DIE_4 = 681,
    SPR_LIQUID_DEAD = 682,
    SPR_LIQUID_SHOT_FLY_1 = 683,
    SPR_LIQUID_SHOT_FLY_2 = 684,
    SPR_LIQUID_SHOT_FLY_3 = 685,
    SPR_LIQUID_SHOT_BURST_1 = 686,
    SPR_LIQUID_SHOT_BURST_2 = 687,
    SPR_LIQUID_SHOT_BURST_3 = 688,
#endif

    //
    // Spit Shots
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_SPIT1_1 = 635,
    SPR_SPIT1_2 = 636,
    SPR_SPIT1_3 = 637,
    SPR_SPIT_EXP1_1 = 638,
    SPR_SPIT_EXP1_2 = 639,
    SPR_SPIT_EXP1_3 = 640,

    SPR_SPIT2_1 = 641,
    SPR_SPIT2_2 = 642,
    SPR_SPIT2_3 = 643,
    SPR_SPIT_EXP2_1 = 644,
    SPR_SPIT_EXP2_2 = 645,
    SPR_SPIT_EXP2_3 = 646,

    SPR_SPIT3_1 = 647,
    SPR_SPIT3_2 = 648,
    SPR_SPIT3_3 = 649,
    SPR_SPIT_EXP3_1 = 650,
    SPR_SPIT_EXP3_2 = 651,
    SPR_SPIT_EXP3_3 = 652,
#else
    SPR_SPIT1_1 = 451,
    SPR_SPIT1_2 = 452,
    SPR_SPIT1_3 = 453,
    SPR_SPIT_EXP1_1 = 454,
    SPR_SPIT_EXP1_2 = 455,
    SPR_SPIT_EXP1_3 = 456,

    SPR_SPIT2_1 = 457,
    SPR_SPIT2_2 = 458,
    SPR_SPIT2_3 = 459,
    SPR_SPIT_EXP2_1 = 460,
    SPR_SPIT_EXP2_2 = 461,
    SPR_SPIT_EXP2_3 = 462,

    SPR_SPIT3_1 = 463,
    SPR_SPIT3_2 = 464,
    SPR_SPIT3_3 = 465,
    SPR_SPIT_EXP3_1 = 466,
    SPR_SPIT_EXP3_2 = 467,
    SPR_SPIT_EXP3_3 = 468,
#endif
#else
    SPR_SPIT1_1 = 689,
    SPR_SPIT1_2 = 690,
    SPR_SPIT1_3 = 691,
    SPR_SPIT_EXP1_1 = 692,
    SPR_SPIT_EXP1_2 = 693,
    SPR_SPIT_EXP1_3 = 694,

    SPR_SPIT2_1 = 695,
    SPR_SPIT2_2 = 696,
    SPR_SPIT2_3 = 697,
    SPR_SPIT_EXP2_1 = 698,
    SPR_SPIT_EXP2_2 = 699,
    SPR_SPIT_EXP2_3 = 700,

    SPR_SPIT3_1 = 701,
    SPR_SPIT3_2 = 702,
    SPR_SPIT3_3 = 703,
    SPR_SPIT_EXP3_1 = 704,
    SPR_SPIT_EXP3_2 = 705,
    SPR_SPIT_EXP3_3 = 706,
#endif

    //
    // Hanging Turret
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_TERROT_1 = 653,
    SPR_TERROT_2 = 654,
    SPR_TERROT_3 = 655,
    SPR_TERROT_4 = 656,
    SPR_TERROT_5 = 657,
    SPR_TERROT_6 = 658,
    SPR_TERROT_7 = 659,
    SPR_TERROT_8 = 660,

    SPR_TERROT_FIRE_1 = 661,
    SPR_TERROT_FIRE_2 = 662,
    SPR_TERROT_DIE_1 = 663,
    SPR_TERROT_DIE_2 = 664,
    SPR_TERROT_DIE_3 = 665,
    SPR_TERROT_DIE_4 = 666,
    SPR_TERROT_DEAD = 667,
#else
    SPR_TERROT_1 = 469,
    SPR_TERROT_2 = 470,
    SPR_TERROT_3 = 471,
    SPR_TERROT_4 = 472,
    SPR_TERROT_5 = 473,
    SPR_TERROT_6 = 474,
    SPR_TERROT_7 = 475,
    SPR_TERROT_8 = 476,

    SPR_TERROT_FIRE_1 = 477,
    SPR_TERROT_FIRE_2 = 478,
    SPR_TERROT_DIE_1 = 479,
    SPR_TERROT_DIE_2 = 480,
    SPR_TERROT_DIE_3 = 481,
    SPR_TERROT_DIE_4 = 482,
    SPR_TERROT_DEAD = 483,
#endif
#else
    SPR_TERROT_1 = 707,
    SPR_TERROT_2 = 708,
    SPR_TERROT_3 = 709,
    SPR_TERROT_4 = 710,
    SPR_TERROT_5 = 711,
    SPR_TERROT_6 = 712,
    SPR_TERROT_7 = 713,
    SPR_TERROT_8 = 714,

    SPR_TERROT_FIRE_1 = 715,
    SPR_TERROT_FIRE_2 = 716,
    SPR_TERROT_DIE_1 = 717,
    SPR_TERROT_DIE_2 = 718,
    SPR_TERROT_DIE_3 = 719,
    SPR_TERROT_DIE_4 = 720,
    SPR_TERROT_DEAD = 721,
#endif

    //
    // Player attack frames
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_KNIFEREADY = 668,
    SPR_KNIFEATK1 = 669,
    SPR_KNIFEATK2 = 670,
    SPR_KNIFEATK3 = 671,
    SPR_KNIFEATK4 = 672,

    SPR_PISTOLREADY = 673,
    SPR_PISTOLATK1 = 674,
    SPR_PISTOLATK2 = 675,
    SPR_PISTOLATK3 = 676,
    SPR_PISTOLATK4 = 677,

    SPR_MACHINEGUNREADY = 678,
    SPR_MACHINEGUNATK1 = 679,
    SPR_MACHINEGUNATK2 = 680,
    MACHINEGUNATK3 = 681,
    SPR_MACHINEGUNATK4 = 682,

    SPR_CHAINREADY = 683,
    SPR_CHAINATK1 = 684,
    SPR_CHAINATK2 = 685,
    SPR_CHAINATK3 = 686,
    SPR_CHAINATK4 = 687,

    SPR_GRENADEREADY = 688,
    SPR_GRENADEATK1 = 689,
    SPR_GRENADEATK2 = 690,
    SPR_GRENADEATK3 = 691,
    SPR_GRENADEATK4 = 692,

    SPR_GRENADE_FLY1 = 693,
    SPR_GRENADE_FLY2 = 694,
    SPR_GRENADE_FLY3 = 695,
    SPR_GRENADE_FLY4 = 696,
    SPR_GRENADE_EXPLODE1 = 697,
    SPR_GRENADE_EXPLODE2 = 698,
    SPR_GRENADE_EXPLODE3 = 699,
    SPR_GRENADE_EXPLODE4 = 700,
    SPR_GRENADE_EXPLODE5 = 701,
#else
    SPR_KNIFEREADY = 484,
    SPR_KNIFEATK1 = 485,
    SPR_KNIFEATK2 = 486,
    SPR_KNIFEATK3 = 487,
    SPR_KNIFEATK4 = 488,

    SPR_PISTOLREADY = 489,
    SPR_PISTOLATK1 = 490,
    SPR_PISTOLATK2 = 491,
    SPR_PISTOLATK3 = 492,
    SPR_PISTOLATK4 = 493,

    SPR_MACHINEGUNREADY = 494,
    SPR_MACHINEGUNATK1 = 495,
    SPR_MACHINEGUNATK2 = 496,
    MACHINEGUNATK3 = 497,
    SPR_MACHINEGUNATK4 = 498,

    SPR_CHAINREADY = 499,
    SPR_CHAINATK1 = 500,
    SPR_CHAINATK2 = 501,
    SPR_CHAINATK3 = 502,
    SPR_CHAINATK4 = 503,

    SPR_GRENADEREADY = 504,
    SPR_GRENADEATK1 = 505,
    SPR_GRENADEATK2 = 506,
    SPR_GRENADEATK3 = 507,
    SPR_GRENADEATK4 = 508,

    SPR_GRENADE_FLY1 = 509,
    SPR_GRENADE_FLY2 = 510,
    SPR_GRENADE_FLY3 = 511,
    SPR_GRENADE_FLY4 = 512,
    SPR_GRENADE_EXPLODE1 = 513,
    SPR_GRENADE_EXPLODE2 = 514,
    SPR_GRENADE_EXPLODE3 = 515,
    SPR_GRENADE_EXPLODE4 = 516,
    SPR_GRENADE_EXPLODE5 = 517,
#endif
#else
    SPR_KNIFEREADY = 722,
    SPR_KNIFEATK1 = 723,
    SPR_KNIFEATK2 = 724,
    SPR_KNIFEATK3 = 725,
    SPR_KNIFEATK4 = 726,

    SPR_PISTOLREADY = 727,
    SPR_PISTOLATK1 = 728,
    SPR_PISTOLATK2 = 729,
    SPR_PISTOLATK3 = 730,
    SPR_PISTOLATK4 = 731,

    SPR_MACHINEGUNREADY = 732,
    SPR_MACHINEGUNATK1 = 733,
    SPR_MACHINEGUNATK2 = 734,
    MACHINEGUNATK3 = 735,
    SPR_MACHINEGUNATK4 = 736,

    SPR_CHAINREADY = 737,
    SPR_CHAINATK1 = 738,
    SPR_CHAINATK2 = 739,
    SPR_CHAINATK3 = 740,
    SPR_CHAINATK4 = 741,

    SPR_GRENADEREADY = 742,
    SPR_GRENADEATK1 = 743,
    SPR_GRENADEATK2 = 744,
    SPR_GRENADEATK3 = 745,
    SPR_GRENADEATK4 = 746,

    SPR_GRENADE_FLY1 = 747,
    SPR_GRENADE_FLY2 = 748,
    SPR_GRENADE_FLY3 = 749,
    SPR_GRENADE_FLY4 = 750,
    SPR_GRENADE_EXPLODE1 = 751,
    SPR_GRENADE_EXPLODE2 = 752,
    SPR_GRENADE_EXPLODE3 = 753,
    SPR_GRENADE_EXPLODE4 = 754,
    SPR_GRENADE_EXPLODE5 = 755,
#endif

#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_ELEC_ARC1 = 702,
    SPR_ELEC_ARC2 = 703,
    SPR_ELEC_ARC3 = 704,
    SPR_ELEC_ARC4 = 705,
    SPR_ELEC_POST1 = 706,
    SPR_ELEC_POST2 = 707,
    SPR_ELEC_POST3 = 708,
    SPR_ELEC_POST4 = 709,
#else
    SPR_ELEC_ARC1 = 518,
    SPR_ELEC_ARC2 = 519,
    SPR_ELEC_ARC3 = 520,
    SPR_ELEC_ARC4 = 521,
    SPR_ELEC_POST1 = 522,
    SPR_ELEC_POST2 = 523,
    SPR_ELEC_POST3 = 524,
    SPR_ELEC_POST4 = 525,
#endif
#else
    SPR_ELEC_ARC1 = 756,
    SPR_ELEC_ARC2 = 757,
    SPR_ELEC_ARC3 = 758,
    SPR_ELEC_ARC4 = 759,
    SPR_ELEC_POST1 = 760,
    SPR_ELEC_POST2 = 761,
    SPR_ELEC_POST3 = 762,
    SPR_ELEC_POST4 = 763,
#endif

#ifdef BSTONE_PS
    SPR_VPOST1 = 764,
    SPR_VPOST2 = 765,
    SPR_VPOST3 = 766,
    SPR_VPOST4 = 767,
    SPR_VPOST5 = 768,
    SPR_VPOST6 = 769,
    SPR_VPOST7 = 770,
    SPR_VPOST8 = 771,

    SPR_VSPIKE1 = 772,
    SPR_VSPIKE2 = 773,
    SPR_VSPIKE3 = 774,
    SPR_VSPIKE4 = 775,
    SPR_VSPIKE5 = 776,
    SPR_VSPIKE6 = 777,
    SPR_VSPIKE7 = 778,
    SPR_VSPIKE8 = 779,
#endif

#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_GREEN_OOZE1 = 710,
    SPR_GREEN_OOZE2 = 711,
    SPR_GREEN_OOZE3 = 712,
    SPR_BLACK_OOZE1 = 713,
    SPR_BLACK_OOZE2 = 714,
    SPR_BLACK_OOZE3 = 715,
#endif
#else
    SPR_GREEN_OOZE1 = 780,
    SPR_GREEN_OOZE2 = 781,
    SPR_GREEN_OOZE3 = 782,
    SPR_BLACK_OOZE1 = 783,
    SPR_BLACK_OOZE2 = 784,
    SPR_BLACK_OOZE3 = 785,
#endif

#ifdef BSTONE_PS
    SPR_GREEN2_OOZE1 = 786,
    SPR_GREEN2_OOZE2 = 787,
    SPR_GREEN2_OOZE3 = 788,
    SPR_BLACK2_OOZE1 = 789,
    SPR_BLACK2_OOZE2 = 790,
    SPR_BLACK2_OOZE3 = 791,
#endif

#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_CANDY_BAR = 716,
    SPR_CANDY_WRAPER = 717,
    SPR_SANDWICH = 718,
    SPR_SANDWICH_WRAPER = 719,

    SPR_BLAKE_W1 = 720,
    SPR_BLAKE_W2 = 721,
    SPR_BLAKE_W3 = 722,
    SPR_BLAKE_W4 = 723,
#else
    SPR_CANDY_BAR = 526,
    SPR_CANDY_WRAPER = 527,
    SPR_SANDWICH = 528,
    SPR_SANDWICH_WRAPER = 529,

    SPR_BLAKE_W1 = 530,
    SPR_BLAKE_W2 = 531,
    SPR_BLAKE_W3 = 532,
    SPR_BLAKE_W4 = 533,
#endif
#else
    SPR_CANDY_BAR = 792,
    SPR_CANDY_WRAPER = 793,
    SPR_SANDWICH = 794,
    SPR_SANDWICH_WRAPER = 795,

    SPR_BLAKE_W1 = 796,
    SPR_BLAKE_W2 = 797,
    SPR_BLAKE_W3 = 798,
    SPR_BLAKE_W4 = 799,
#endif

    //
    // Boss 1 (Spider Mutant)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS1_W1 = 724,
    SPR_BOSS1_W2 = 725,
    SPR_BOSS1_W3 = 726,
    SPR_BOSS1_W4 = 727,
    SPR_BOSS1_SWING1 = 728,
    SPR_BOSS1_SWING2 = 729,
    SPR_BOSS1_SWING3 = 730,
    SPR_BOSS1_DEAD = 731,
    SPR_BOSS1_DIE1 = 732,
    SPR_BOSS1_DIE2 = 733,
    SPR_BOSS1_DIE3 = 734,
    SPR_BOSS1_DIE4 = 735,
    SPR_BOSS1_OUCH = 736,
    SPR_BOSS1_PROJ1 = 737,
    SPR_BOSS1_PROJ2 = 738,
    SPR_BOSS1_PROJ3 = 739,
    SPR_BOSS1_EXP1 = 740,
    SPR_BOSS1_EXP2 = 741,
    SPR_BOSS1_EXP3 = 742,
#else
    SPR_BOSS1_W1 = 534,
    SPR_BOSS1_W2 = 535,
    SPR_BOSS1_W3 = 536,
    SPR_BOSS1_W4 = 537,
    SPR_BOSS1_SWING1 = 538,
    SPR_BOSS1_SWING2 = 539,
    SPR_BOSS1_SWING3 = 540,
    SPR_BOSS1_DEAD = 541,
    SPR_BOSS1_DIE1 = 542,
    SPR_BOSS1_DIE2 = 543,
    SPR_BOSS1_DIE3 = 544,
    SPR_BOSS1_DIE4 = 545,
    SPR_BOSS1_OUCH = 546,
    SPR_BOSS1_PROJ1 = 547,
    SPR_BOSS1_PROJ2 = 548,
    SPR_BOSS1_PROJ3 = 549,
    SPR_BOSS1_EXP1 = 550,
    SPR_BOSS1_EXP2 = 551,
    SPR_BOSS1_EXP3 = 552,
#endif
#else
    SPR_BOSS1_W1 = 800,
    SPR_BOSS1_W2 = 801,
    SPR_BOSS1_W3 = 802,
    SPR_BOSS1_W4 = 803,
    SPR_BOSS1_SWING1 = 804,
    SPR_BOSS1_SWING2 = 805,
    SPR_BOSS1_SWING3 = 806,
    SPR_BOSS1_DEAD = 807,
    SPR_BOSS1_DIE1 = 808,
    SPR_BOSS1_DIE2 = 809,
    SPR_BOSS1_DIE3 = 810,
    SPR_BOSS1_DIE4 = 811,
    SPR_BOSS1_OUCH = 812,
    SPR_BOSS1_PROJ1 = 813,
    SPR_BOSS1_PROJ2 = 814,
    SPR_BOSS1_PROJ3 = 815,
    SPR_BOSS1_EXP1 = 816,
    SPR_BOSS1_EXP2 = 817,
    SPR_BOSS1_EXP3 = 818,
    SPR_BOSS1_MORPH1 = 819,
    SPR_BOSS1_MORPH2 = 820,
    SPR_BOSS1_MORPH3 = 821,
    SPR_BOSS1_MORPH4 = 822,
    SPR_BOSS1_MORPH5 = 823,
    SPR_BOSS1_MORPH6 = 824,
    SPR_BOSS1_MORPH7 = 825,
    SPR_BOSS1_MORPH8 = 826,
    SPR_BOSS1_MORPH9 = 827,
#endif

    //
    // Boss 2 (Breather Beast)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS2_W1 = 743,
    SPR_BOSS2_W2 = 744,
    SPR_BOSS2_W3 = 745,
    SPR_BOSS2_W4 = 746,
    SPR_BOSS2_SWING1 = 747,
    SPR_BOSS2_SWING2 = 748,
    SPR_BOSS2_SWING3 = 749,
    SPR_BOSS2_DEAD = 750,
    SPR_BOSS2_DIE1 = 751,
    SPR_BOSS2_DIE2 = 752,
    SPR_BOSS2_DIE3 = 753,
    SPR_BOSS2_DIE4 = 754,
    SPR_BOSS2_OUCH = 755,
#endif
#else
    SPR_BOSS2_W1 = 828,
    SPR_BOSS2_W2 = 829,
    SPR_BOSS2_W3 = 830,
    SPR_BOSS2_W4 = 831,
    SPR_BOSS2_SWING1 = 832,
    SPR_BOSS2_SWING2 = 833,
    SPR_BOSS2_SWING3 = 834,
    SPR_BOSS2_DEAD = 835,
    SPR_BOSS2_DIE1 = 836,
    SPR_BOSS2_DIE2 = 837,
    SPR_BOSS2_DIE3 = 838,
    SPR_BOSS2_DIE4 = 839,
    SPR_BOSS2_OUCH = 840,
#endif

    //
    // Boss 3 (Cyborg Warrior)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS3_W1 = 756,
    SPR_BOSS3_W2 = 757,
    SPR_BOSS3_W3 = 758,
    SPR_BOSS3_W4 = 759,
    SPR_BOSS3_SWING1 = 760,
    SPR_BOSS3_SWING2 = 761,
    SPR_BOSS3_SWING3 = 762,
    SPR_BOSS3_DEAD = 763,
    SPR_BOSS3_DIE1 = 764,
    SPR_BOSS3_DIE2 = 765,
    SPR_BOSS3_DIE3 = 766,
    SPR_BOSS3_DIE4 = 767,
    SPR_BOSS3_OUCH = 768,
#endif
#else
    SPR_BOSS3_W1 = 841,
    SPR_BOSS3_W2 = 842,
    SPR_BOSS3_W3 = 843,
    SPR_BOSS3_W4 = 844,
    SPR_BOSS3_SWING1 = 845,
    SPR_BOSS3_SWING2 = 846,
    SPR_BOSS3_SWING3 = 847,
    SPR_BOSS3_DEAD = 848,
    SPR_BOSS3_DIE1 = 849,
    SPR_BOSS3_DIE2 = 850,
    SPR_BOSS3_DIE3 = 851,
    SPR_BOSS3_DIE4 = 852,
    SPR_BOSS3_OUCH = 853,
#endif

    //
    // Boss 4 (Reptilian Warrior)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS4_W1 = 769,
    SPR_BOSS4_W2 = 770,
    SPR_BOSS4_W3 = 771,
    SPR_BOSS4_W4 = 772,
    SPR_BOSS4_SWING1 = 773,
    SPR_BOSS4_SWING2 = 774,
    SPR_BOSS4_SWING3 = 775,
    SPR_BOSS4_DEAD = 776,
    SPR_BOSS4_DIE1 = 777,
    SPR_BOSS4_DIE2 = 778,
    SPR_BOSS4_DIE3 = 779,
    SPR_BOSS4_DIE4 = 780,
    SPR_BOSS4_OUCH = 781,
#endif
#else
    SPR_BOSS4_W1 = 854,
    SPR_BOSS4_W2 = 855,
    SPR_BOSS4_W3 = 856,
    SPR_BOSS4_W4 = 857,
    SPR_BOSS4_SWING1 = 858,
    SPR_BOSS4_SWING2 = 859,
    SPR_BOSS4_SWING3 = 860,
    SPR_BOSS4_DEAD = 861,
    SPR_BOSS4_DIE1 = 862,
    SPR_BOSS4_DIE2 = 863,
    SPR_BOSS4_DIE3 = 864,
    SPR_BOSS4_DIE4 = 865,
    SPR_BOSS4_OUCH = 866,
    SPR_BOSS4_MORPH1 = 867,
    SPR_BOSS4_MORPH2 = 868,
    SPR_BOSS4_MORPH3 = 869,
    SPR_BOSS4_MORPH4 = 870,
    SPR_BOSS4_MORPH5 = 871,
    SPR_BOSS4_MORPH6 = 872,
    SPR_BOSS4_MORPH7 = 873,
    SPR_BOSS4_MORPH8 = 874,
    SPR_BOSS4_MORPH9 = 875,
#endif

    //
    // Boss 5 (Acid Dragon)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS5_W1 = 782,
    SPR_BOSS5_W2 = 783,
    SPR_BOSS5_W3 = 784,
    SPR_BOSS5_W4 = 785,
    SPR_BOSS5_SWING1 = 786,
    SPR_BOSS5_SWING2 = 787,
    SPR_BOSS5_SWING3 = 788,
    SPR_BOSS5_DEAD = 789,
    SPR_BOSS5_DIE1 = 790,
    SPR_BOSS5_DIE2 = 791,
    SPR_BOSS5_DIE3 = 792,
    SPR_BOSS5_DIE4 = 793,
    SPR_BOSS5_OUCH = 794,
    SPR_BOSS5_PROJ1 = 795,
    SPR_BOSS5_PROJ2 = 796,
    SPR_BOSS5_PROJ3 = 797,
    SPR_BOSS5_EXP1 = 798,
    SPR_BOSS5_EXP2 = 799,
    SPR_BOSS5_EXP3 = 800,
#endif
#else
    SPR_BOSS5_W1 = 876,
    SPR_BOSS5_W2 = 877,
    SPR_BOSS5_W3 = 878,
    SPR_BOSS5_W4 = 879,
    SPR_BOSS5_SWING1 = 880,
    SPR_BOSS5_SWING2 = 881,
    SPR_BOSS5_SWING3 = 882,
    SPR_BOSS5_DEAD = 883,
    SPR_BOSS5_DIE1 = 884,
    SPR_BOSS5_DIE2 = 885,
    SPR_BOSS5_DIE3 = 886,
    SPR_BOSS5_DIE4 = 887,
    SPR_BOSS5_OUCH = 888,
    SPR_BOSS5_PROJ1 = 889,
    SPR_BOSS5_PROJ2 = 890,
    SPR_BOSS5_PROJ3 = 891,
    SPR_BOSS5_EXP1 = 892,
    SPR_BOSS5_EXP2 = 893,
    SPR_BOSS5_EXP3 = 894,
#endif

    //
    // Boss 6 (Bio-Mech Guardian)
    //
#ifdef BSTONE_AOG
#if GAME_VERSION != SHAREWARE_VERSION
    SPR_BOSS6_W1 = 801,
    SPR_BOSS6_W2 = 802,
    SPR_BOSS6_W3 = 803,
    SPR_BOSS6_W4 = 804,
    SPR_BOSS6_SWING1 = 805,
    SPR_BOSS6_SWING2 = 806,
    SPR_BOSS6_SWING3 = 807,
    SPR_BOSS6_DEAD = 808,
    SPR_BOSS6_DIE1 = 809,
    SPR_BOSS6_DIE2 = 810,
    SPR_BOSS6_DIE3 = 811,
    SPR_BOSS6_DIE4 = 812,
    SPR_BOSS6_OUCH = 813,
#endif
#else
    SPR_BOSS6_W1 = 895,
    SPR_BOSS6_W2 = 896,
    SPR_BOSS6_W3 = 897,
    SPR_BOSS6_W4 = 898,
    SPR_BOSS6_SWING1 = 899,
    SPR_BOSS6_SWING2 = 900,
    SPR_BOSS6_SWING3 = 901,
    SPR_BOSS6_DEAD = 902,
    SPR_BOSS6_DIE1 = 903,
    SPR_BOSS6_DIE2 = 904,
    SPR_BOSS6_DIE3 = 905,
    SPR_BOSS6_DIE4 = 906,
    SPR_BOSS6_OUCH = 907,
#endif


    //
    // Boss 7
    //
#ifdef BSTONE_PS
    SPR_BOSS7_W1 = 908,
    SPR_BOSS7_W2 = 909,
    SPR_BOSS7_W3 = 910,
    SPR_BOSS7_W4 = 911,
    SPR_BOSS7_SHOOT1 = 912,
    SPR_BOSS7_SHOOT2 = 913,
    SPR_BOSS7_SHOOT3 = 914,
    SPR_BOSS7_DEAD = 915,
    SPR_BOSS7_DIE1 = 916,
    SPR_BOSS7_DIE2 = 917,
    SPR_BOSS7_DIE3 = 918,
    SPR_BOSS7_DIE4 = 919,
    SPR_BOSS7_OUCH = 920,
#endif

    //
    // Boss 8
    //
#ifdef BSTONE_PS
    SPR_BOSS8_W1 = 921,
    SPR_BOSS8_W2 = 922,
    SPR_BOSS8_W3 = 923,
    SPR_BOSS8_W4 = 924,
    SPR_BOSS8_SHOOT1 = 925,
    SPR_BOSS8_SHOOT2 = 926,
    SPR_BOSS8_SHOOT3 = 927,
    SPR_BOSS8_DIE1 = 928,
    SPR_BOSS8_DIE2 = 929,
    SPR_BOSS8_DIE3 = 930,
    SPR_BOSS8_DIE4 = 931,
    SPR_BOSS8_DEAD = 932,
    SPR_BOSS8_OUCH = 933,
#endif

    //
    // Boss 9
    //
#ifdef BSTONE_PS
    SPR_BOSS9_W1 = 934,
    SPR_BOSS9_W2 = 935,
    SPR_BOSS9_W3 = 936,
    SPR_BOSS9_W4 = 937,
    SPR_BOSS9_SHOOT1 = 938,
    SPR_BOSS9_SHOOT2 = 939,
    SPR_BOSS9_SHOOT3 = 940,
    SPR_BOSS9_DIE1 = 941,
    SPR_BOSS9_DIE2 = 942,
    SPR_BOSS9_DIE3 = 943,
    SPR_BOSS9_DIE4 = 944,
    SPR_BOSS9_DEAD = 945,
    SPR_BOSS9_OUCH = 946,
#endif

    //
    // Boss 10
    //
#if BSTONE_PS
    SPR_BOSS10_W1 = 947,
    SPR_BOSS10_W2 = 948,
    SPR_BOSS10_W3 = 949,
    SPR_BOSS10_W4 = 950,
    SPR_BOSS10_SHOOT1 = 951,
    SPR_BOSS10_SHOOT2 = 952,
    SPR_BOSS10_SHOOT3 = 953,
    SPR_BOSS10_DEAD = 954,
    SPR_BOSS10_DIE1 = 955,
    SPR_BOSS10_DIE2 = 956,
    SPR_BOSS10_DIE3 = 957,
    SPR_BOSS10_DIE4 = 958,
    SPR_BOSS10_OUCH = 959,

    SPR_BOSS10_SPIT1 = 960,
    SPR_BOSS10_SPIT2 = 961,
    SPR_BOSS10_SPIT3 = 962,
    SPR_BOSS10_SPIT_EXP1 = 963,
    SPR_BOSS10_SPIT_EXP2 = 964,
    SPR_BOSS10_SPIT_EXP3 = 965,
#endif

    //
    // Plasma detonator explosion
    //
#ifdef BSTONE_PS
    SPR_DETONATOR_EXP1 = 966,
    SPR_DETONATOR_EXP2 = 967,
    SPR_DETONATOR_EXP3 = 968,
    SPR_DETONATOR_EXP4 = 969,
    SPR_DETONATOR_EXP5 = 970,
    SPR_DETONATOR_EXP6 = 971,
    SPR_DETONATOR_EXP7 = 972,
    SPR_DETONATOR_EXP8 = 973,
#endif

    //
    // Clip Explosion
    //
#ifdef BSTONE_PS
    SPR_CLIP_EXP1 = 974,
    SPR_CLIP_EXP2 = 975,
    SPR_CLIP_EXP3 = 976,
    SPR_CLIP_EXP4 = 977,
    SPR_CLIP_EXP5 = 978,
    SPR_CLIP_EXP6 = 979,
    SPR_CLIP_EXP7 = 980,
    SPR_CLIP_EXP8 = 981,
#endif

    //
    // BFG Cannon Weapon
    //
#ifdef BSTONE_PS
    SPR_BFG_WEAPON1 = 982,
    SPR_BFG_WEAPON2 = 983,
    SPR_BFG_WEAPON3 = 984,
    SPR_BFG_WEAPON4 = 985,
    SPR_BFG_WEAPON5 = 986,
#endif

    //
    // BFG Cannon Weapon shots
    //
#ifdef BSTONE_PS
    SPR_BFG_WEAPON_SHOT1 = 987,
    SPR_BFG_WEAPON_SHOT2 = 988,
    SPR_BFG_WEAPON_SHOT3 = 989,
#endif

    //
    // BFG Shot explosion
    //
#ifdef BSTONE_PS
    SPR_BFG_EXP1 = 990,
    SPR_BFG_EXP2 = 991,
    SPR_BFG_EXP3 = 992,
    SPR_BFG_EXP4 = 993,
    SPR_BFG_EXP5 = 994,
    SPR_BFG_EXP6 = 995,
    SPR_BFG_EXP7 = 996,
    SPR_BFG_EXP8 = 997
#endif
}; // sprite_t

#if GAME_VERSION == SHAREWARE_VERSION
enum dummy_sprite_t {
    SPR_STAT_1 = 0,
    SPR_STAT_3 = 0,
    SPR_STAT_4 = 0,
    SPR_STAT_5 = 0,
    SPR_STAT_6 = 0,
    SPR_STAT_7 = 0,
    SPR_STAT_16 = 0,
    SPR_STAT_17 = 0,
    SPR_STAT_19 = 0,
    SPR_STAT_20 = 0,
    SPR_STAT_21 = 0,
    SPR_STAT_22 = 0,
    SPR_STAT_23 = 0,
    SPR_STAT_25 = 0,
    SPR_STAT_30 = 0,
    SPR_STAT_37 = 0,
    SPR_STAT_39 = 0,
    SPR_CRATE_2 = 0,
    SPR_CRATE_3 = 0,
    SPR_STAT_58 = 0,
    SPR_STAT_59 = 0,
    SPR_STAT_64 = 0,
    SPR_STAT_70 = 0,
    SPR_STAT_71 = 0,
    SPR_STAT_72 = 0,
    SPR_STAT_73 = 0,
    SPR_STAT_74 = 0,
    SPR_STAT_75 = 0,
    SPR_STAT_76 = 0,

    SPR_GSCOUT_W1_1, SPR_GSCOUT_W1_2, SPR_GSCOUT_W1_3, SPR_GSCOUT_W1_4,
    SPR_GSCOUT_W1_5, SPR_GSCOUT_W1_6, SPR_GSCOUT_W1_7, SPR_GSCOUT_W1_8,

    SPR_GSCOUT_W2_1, SPR_GSCOUT_W2_2, SPR_GSCOUT_W2_3, SPR_GSCOUT_W2_4,
    SPR_GSCOUT_W2_5, SPR_GSCOUT_W2_6, SPR_GSCOUT_W2_7, SPR_GSCOUT_W2_8,

    SPR_GSCOUT_W3_1, SPR_GSCOUT_W3_2, SPR_GSCOUT_W3_3, SPR_GSCOUT_W3_4,
    SPR_GSCOUT_W3_5, SPR_GSCOUT_W3_6, SPR_GSCOUT_W3_7, SPR_GSCOUT_W3_8,

    SPR_GSCOUT_W4_1, SPR_GSCOUT_W4_2, SPR_GSCOUT_W4_3, SPR_GSCOUT_W4_4,
    SPR_GSCOUT_W4_5, SPR_GSCOUT_W4_6, SPR_GSCOUT_W4_7, SPR_GSCOUT_W4_8,

    SPR_GSCOUT_DIE1, SPR_GSCOUT_DIE2, SPR_GSCOUT_DIE3, SPR_GSCOUT_DIE4,
    SPR_GSCOUT_DIE5, SPR_GSCOUT_DIE6, SPR_GSCOUT_DIE7, SPR_GSCOUT_DIE8,

    SPR_GSCOUT_DEAD,

    SPR_FSCOUT_W2_1, SPR_FSCOUT_W2_2, SPR_FSCOUT_W2_3, SPR_FSCOUT_W2_4,
    SPR_FSCOUT_W2_5, SPR_FSCOUT_W2_6, SPR_FSCOUT_W2_7, SPR_FSCOUT_W2_8,

    SPR_FSCOUT_W3_1, SPR_FSCOUT_W3_2, SPR_FSCOUT_W3_3, SPR_FSCOUT_W3_4,
    SPR_FSCOUT_W3_5, SPR_FSCOUT_W3_6, SPR_FSCOUT_W3_7, SPR_FSCOUT_W3_8,

    SPR_FSCOUT_W4_1, SPR_FSCOUT_W4_2, SPR_FSCOUT_W4_3, SPR_FSCOUT_W4_4,
    SPR_FSCOUT_W4_5, SPR_FSCOUT_W4_6, SPR_FSCOUT_W4_7, SPR_FSCOUT_W4_8,

    SPR_VITAL_STAND,
    SPR_VITAL_DIE_1, SPR_VITAL_DIE_2, SPR_VITAL_DIE_3, SPR_VITAL_DIE_4,
    SPR_VITAL_DIE_5, SPR_VITAL_DIE_6, SPR_VITAL_DIE_7, SPR_VITAL_DIE_8,
    SPR_VITAL_DEAD_1, SPR_VITAL_DEAD_2, SPR_VITAL_DEAD_3,
    SPR_VITAL_OUCH,

    SPR_MUTHUM2_W1, SPR_MUTHUM2_W2, SPR_MUTHUM2_W3, SPR_MUTHUM2_W4,
    SPR_MUTHUM2_SWING1, SPR_MUTHUM2_SWING2, SPR_MUTHUM2_SWING3,
    SPR_MUTHUM2_DEAD,
    SPR_MUTHUM2_DIE1, SPR_MUTHUM2_DIE2, SPR_MUTHUM2_DIE3, SPR_MUTHUM2_DIE4,
    SPR_MUTHUM2_OUCH,
    SPR_MUTHUM2_SPIT1, SPR_MUTHUM2_SPIT2, SPR_MUTHUM2_SPIT3,

    SPR_LCAN_ALIEN_READY, SPR_LCAN_ALIEN_B1, SPR_LCAN_ALIEN_B2,
    SPR_LCAN_ALIEN_B3, SPR_LCAN_ALIEN_EMPTY,

    SPR_LCAN_ALIEN_W1, SPR_LCAN_ALIEN_W2, SPR_LCAN_ALIEN_W3, SPR_LCAN_ALIEN_W4,
    SPR_LCAN_ALIEN_SWING1, SPR_LCAN_ALIEN_SWING2, SPR_LCAN_ALIEN_SWING3,
    SPR_LCAN_ALIEN_DEAD,
    SPR_LCAN_ALIEN_DIE1, SPR_LCAN_ALIEN_DIE2, SPR_LCAN_ALIEN_DIE3,
    SPR_LCAN_ALIEN_DIE4, SPR_LCAN_ALIEN_OUCH,
    SPR_LCAN_ALIEN_SPIT1, SPR_LCAN_ALIEN_SPIT2, SPR_LCAN_ALIEN_SPIT3,

    SPR_GURNEY_MUT_READY, SPR_GURNEY_MUT_B1, SPR_GURNEY_MUT_B2,
    SPR_GURNEY_MUT_B3, SPR_GURNEY_MUT_EMPTY,

    SPR_GURNEY_MUT_W1, SPR_GURNEY_MUT_W2, SPR_GURNEY_MUT_W3, SPR_GURNEY_MUT_W4,
    SPR_GURNEY_MUT_SWING1, SPR_GURNEY_MUT_SWING2, SPR_GURNEY_MUT_SWING3,
    SPR_GURNEY_MUT_DEAD,
    SPR_GURNEY_MUT_DIE1, SPR_GURNEY_MUT_DIE2, SPR_GURNEY_MUT_DIE3,
    SPR_GURNEY_MUT_DIE4, SPR_GURNEY_MUT_OUCH,

    SPR_LIQUID_M1, SPR_LIQUID_M2, SPR_LIQUID_M3,
    SPR_LIQUID_R1, SPR_LIQUID_R2, SPR_LIQUID_R3, SPR_LIQUID_R4,
    SPR_LIQUID_S1, SPR_LIQUID_S2, SPR_LIQUID_S3,
    SPR_LIQUID_OUCH,
    SPR_LIQUID_DIE_1, SPR_LIQUID_DIE_2, SPR_LIQUID_DIE_3, SPR_LIQUID_DIE_4,
    SPR_LIQUID_DEAD,
    SPR_LIQUID_SHOT_FLY_1, SPR_LIQUID_SHOT_FLY_2, SPR_LIQUID_SHOT_FLY_3,
    SPR_LIQUID_SHOT_BURST_1, SPR_LIQUID_SHOT_BURST_2, SPR_LIQUID_SHOT_BURST_3,

    SPR_GREEN_OOZE1, SPR_GREEN_OOZE2, SPR_GREEN_OOZE3,
    SPR_BLACK_OOZE1, SPR_BLACK_OOZE2, SPR_BLACK_OOZE3,

    SPR_BOSS2_W1, SPR_BOSS2_W2, SPR_BOSS2_W3, SPR_BOSS2_W4,
    SPR_BOSS2_SWING1, SPR_BOSS2_SWING2, SPR_BOSS2_SWING3,
    SPR_BOSS2_DEAD,
    SPR_BOSS2_DIE1, SPR_BOSS2_DIE2, SPR_BOSS2_DIE3, SPR_BOSS2_DIE4,
    SPR_BOSS2_OUCH,
    SPR_BOSS2_SHOOT1, SPR_BOSS2_SHOOT2, SPR_BOSS2_SHOOT3,
    SPR_BOSS2_PROJ1, SPR_BOSS2_PROJ2, SPR_BOSS2_PROJ3,
    SPR_BOSS2_EXP1, SPR_BOSS2_EXP2, SPR_BOSS2_EXP3,

    SPR_BOSS3_W1, SPR_BOSS3_W2, SPR_BOSS3_W3, SPR_BOSS3_W4,
    SPR_BOSS3_SWING1, SPR_BOSS3_SWING2, SPR_BOSS3_SWING3,
    SPR_BOSS3_DEAD,
    SPR_BOSS3_DIE1, SPR_BOSS3_DIE2, SPR_BOSS3_DIE3, SPR_BOSS3_DIE4,
    SPR_BOSS3_OUCH,
    SPR_BOSS3_SHOOT1, SPR_BOSS3_SHOOT2, SPR_BOSS3_SHOOT3,
    SPR_BOSS3_PROJ1, SPR_BOSS3_PROJ2, SPR_BOSS3_PROJ3,
    SPR_BOSS3_EXP1, SPR_BOSS3_EXP2, SPR_BOSS3_EXP3,

    SPR_BOSS4_W1, SPR_BOSS4_W2, SPR_BOSS4_W3, SPR_BOSS4_W4,
    SPR_BOSS4_SWING1, SPR_BOSS4_SWING2, SPR_BOSS4_SWING3,
    SPR_BOSS4_DEAD,
    SPR_BOSS4_DIE1, SPR_BOSS4_DIE2, SPR_BOSS4_DIE3, SPR_BOSS4_DIE4,
    SPR_BOSS4_OUCH,
    SPR_BOSS4_SHOOT1, SPR_BOSS4_SHOOT2, SPR_BOSS4_SHOOT3,
    SPR_BOSS4_PROJ1, SPR_BOSS4_PROJ2, SPR_BOSS4_PROJ3,
    SPR_BOSS4_EXP1, SPR_BOSS4_EXP2, SPR_BOSS4_EXP3,

    SPR_BOSS5_W1, SPR_BOSS5_W2, SPR_BOSS5_W3, SPR_BOSS5_W4,
    SPR_BOSS5_SWING1, SPR_BOSS5_SWING2, SPR_BOSS5_SWING3,
    SPR_BOSS5_DEAD,
    SPR_BOSS5_DIE1, SPR_BOSS5_DIE2, SPR_BOSS5_DIE3, SPR_BOSS5_DIE4,
    SPR_BOSS5_OUCH,
    SPR_BOSS5_SHOOT1, SPR_BOSS5_SHOOT2, SPR_BOSS5_SHOOT3,
    SPR_BOSS5_PROJ1, SPR_BOSS5_PROJ2, SPR_BOSS5_PROJ3,
    SPR_BOSS5_EXP1, SPR_BOSS5_EXP2, SPR_BOSS5_EXP3,

    SPR_BOSS6_W1, SPR_BOSS6_W2, SPR_BOSS6_W3, SPR_BOSS6_W4,
    SPR_BOSS6_SWING1, SPR_BOSS6_SWING2, SPR_BOSS6_SWING3,
    SPR_BOSS6_DEAD,
    SPR_BOSS6_DIE1, SPR_BOSS6_DIE2, SPR_BOSS6_DIE3, SPR_BOSS6_DIE4,
    SPR_BOSS6_OUCH,
    SPR_BOSS6_SHOOT1, SPR_BOSS6_SHOOT2, SPR_BOSS6_SHOOT3,
    SPR_BOSS6_PROJ1, SPR_BOSS6_PROJ2, SPR_BOSS6_PROJ3,
    SPR_BOSS6_EXP1, SPR_BOSS6_EXP2, SPR_BOSS6_EXP3,
}; // enum dummy_sprite_t
#endif

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

    NUMDOORTYPES


}; // enum doortype


//
// Breifing types - Note these are ordered to an char array in Breifing().
//

enum breifing_type {
    BT_LOSE,
    BT_WIN,
    BT_INTRO

}; // enum breifing_type

// Terminal Messages - These correspond to the order in which they are
// grabbed in VGAGRAPH.BS?.  See BSTONEV.I
//

enum term_msg_type {
    TM_HINT,
    TM_JAM,
    TM_HELP,
    TM_APOGEE,
    TM_ID,
    TM_GOOBERS,
    TM_MIKE,
    TM_JIM,
    TM_JERRY,

    TM_JOSHUA,

    TM_STAR,

    TM_VITALS1,
    TM_VITALS2,

    TM_DEACTIVATE_TURRET,
    TM_TURRETS_ACTIVATED,
    TM_TURRETS_DEACTIVATED,

    TM_LINK,
    TM_LINK_OK,
    TM_LINK_BAD,

    TM_RADAR_OFF,
    TM_RADAR_ON,

    TM_SOUND_OFF,
    TM_SOUND_ON,

    TM_GOLDSTERN_TRACK_OFF,
    TM_GOLDSTERN_TRACK_ON,
    TM_GOLDSTERN_ARRIVED,
    TM_GOLDSTERN_WILL_AR,
    TM_GOLDSTERN_NO_PICK,
    TM_GOLDSTERN_NO_INFO,

    TM_RESET_SECURITY,
    TM_SECURITY_STATUS_OFF,
    TM_SECURITY_STATUS_ON,

    TM_TURRET_DEACTIVATE_BAD,
    TM_TURRET_DEACTIVATE_GOOD,

    TM_UNRECOGNIZED_COMMAND,
    TM_READY,
    TM_RETURN,
    TM_SECONDS,
    TM_CHEATER,
    TM_BLUEPRINTS,

    TM_PROFILE_WHO,
    TM_PROFILE_SARA,
    TM_PROFILE_BLAKE,
    TM_PROFILE_GOLDSTERN,
    TM_PROFILE_UNKNOWN,
    TM_DEACTIVATE_SOCKETS,
    TM_ACTIVATE_SOCKETS,
    TM_UNABLE_TO_PERFORM,
    TM_NO_SOCKETS,
    TM_ALREADY_ACTIVATED,
    TM_ALREADY_DEACTIVATED,
    TM_LAST
}; // enum term_msg_type

// Terminal Commands - The first set of commands TC_HINT - TC_end_of_1to1
// are directly mapped 1 to 1 to the terminal msgs.
//

enum term_cmd_type {
    TC_HINT,
    TC_JAM,
    TC_HELP,
    TC_APOGEE,
    TC_THANKS,
    TC_GOOBERS,
    TC_MIKE,
    TC_JIM,
    TC_JERRY,

    // END OF ONE TO ONE LIST

    TC_JOSHUA,
    TC_STAR,
    TC_BLUEPRINT,
    TC_EXIT,
    TC_QUIT,
    TC_OFF,
    TC_BYE,
    TC_DISPLAY_PERSONNEL,
    TC_SOUND,
    TC_DISPLAY_GOLDSTERN,
    TC_ARRIVAL_GOLDSTERN,
    TC_DEACTIVATE_SECURITY,
    TC_DISPLAY_SECURITY,
    TC_SATALITE_STATUS,
    TC_DEACTIVATE_TURRETS,
    TC_TURRET_STATUS,
    TC_PROFILE,
    TC_SSTONE,
    TC_BSTONE,
    TC_GOLDSTERN,
    TC_DEACTIVATE_SOCKETS,
    TC_ACTIVATE_SOCKETS,

    TC_LAST
}; // enum term_cmd_type

//
// Barrier State Transistions
//

enum barrier_state_type {
    bt_OFF,
    bt_ON,
    bt_DISABLING,
    bt_DISABLED,

#ifdef BSTONE_PS
    bt_OPENING,                         // For physical barriers
    bt_CLOSING                          //            " " "
#endif
}; // enum barrier_state_type



/*
=============================================================================

 GLOBAL TYPES

=============================================================================
*/

typedef Sint32 fixed;

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
    Sint32 Points; // Score for this bonus
    boolean Recurring; // Appear multiple times in a single level?
    void (* func)(); // Code to execute when you get this bonus.
}; // struct PinballBonusInfo

struct atkinf_t {
    char tics;
    char attack;
    char frame;                 // attack is 1 for gun, 2 for knife
}; // struct atkinf_t


// NOTE - This enum list is ORDERED!
enum movie_t {
    mv_intro,
    mv_final,
#ifdef BSTONE_AOG
    mv_final2, // Episode 3/5
    mv_final3, // Episode 6
#endif
    mv_NUM_MOVIES
}; // enum movie_t

enum controldir_t {
    di_north,
    di_east,
    di_south,
    di_west
}; // enum controldir_t

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
}; // enum door_t

enum keytype {
    kt_none = -1,
    kt_red,
    kt_yellow,
    kt_blue,
#ifdef BSTONE_AOG
    kt_green,
    kt_gold,
#endif
    NUMKEYS
}; // enum keytype

enum activetype {
    ac_badobject = -1,
    ac_no,
    ac_yes,
    ac_allways
}; // enum activetype

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

}; // enum classtype

// BBi
inline classtype operator++(
    classtype& a,
    int)
{
    classtype result = a;
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
    bo_gold_key
}; // enum stat_t

struct stattype {
    Sint16 picnum;
    stat_t type;
}; // struct stattype

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
}; // enum dirtype

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
    dirtype result = a;
    a -= 1;
    return result;
}

inline dirtype operator++(
    dirtype& a,
    int)
{
    dirtype result = a;
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
    NUMENEMIES
}; // enum enemy_t


#define SF_ROTATE 0x01
#define SF_PAINFRAME 0x02


struct objtype;


struct statetype {
    Uint8 flags;
    Sint16 shapenum; // a shapenum of -1 means get from ob->temp1
    Sint16 tictime;
    void (* think)(
        objtype* actor);
    void (* action)(
        objtype* actor);
    statetype* next;
}; // struct statetype


// ---------------------
//
// trivial actor structure
//
// ---------------------

struct statobj_t {
    Uint8 tilex;
    Uint8 tiley;
    Uint8 areanumber;

    Uint8* visspot;
    Sint16 shapenum; // if shapenum == -1 the obj has been removed
    Uint16 flags;
    Uint8 itemnumber;
    char lighting;

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct statobj_t

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
    dr_jammed
}; // enum DoorAction

struct doorobj_t {
    Uint8 tilex;
    Uint8 tiley;
    boolean vertical;
    char flags;
    keytype lock;
    door_t type;
    DoorAction action;
    Sint16 ticcount;
    Uint8 areanumber[2];

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct doorobj_t


// --------------------
//
// thinking actor structure
//
// --------------------

struct objtype {
    Uint8 tilex;
    Uint8 tiley;
    Uint8 areanumber;

    activetype active;
    Sint16 ticcount;
    classtype obclass;
    statetype* state;

    Uint32 flags;
    Uint16 flags2; // Aux flags

    Sint32 distance; // if negative, wait for that door to open
    dirtype dir;
    dirtype trydir; // "bit 7" == "direction to turn" flag

    fixed x;
    fixed y;
    Uint8 s_tilex;
    Uint8 s_tiley; // s_tilex==0, running for corner

    Sint16 viewx;
    Uint16 viewheight;
    fixed transx;
    fixed transy; // in global coord

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    Sint16 hitpoints;

    Uint8 ammo;
    char lighting;
    Uint16 linc;
    Sint16 angle;
    Sint32 speed;

    Sint16 temp1;

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    Sint16 temp2;

    // FIXME
    // In original code it also used to store a 16-bit pointer to object.
    // Since our code is at least 32-bit we are using an index of object.
    Uint16 temp3; // holds 'last door used' by 'smart' actors

    objtype* next;
    objtype* prev;


    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct objtype




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

    NUMBUTTONS
}; // enum ButtonState


enum weapontype {
    wp_autocharge,
    wp_pistol,
    wp_burst_rifle,
    wp_ion_cannon,
    wp_grenade,
    wp_bfg_cannon,
//      wp_plasma_detonators,

    wp_SPACER,
    NUMWEAPONS
}; // enum weapontype



enum Difficulty {
    gd_baby,
    gd_easy,
    gd_medium,
    gd_hard
}; // enum Difficulty



enum backgroundtype {
    ELEVATOR_BACK,
    TRANSPORTER_BACK
}; // enum backgroundtype




//
// General Coord (tile) structure
//
struct tilecoord_t {
    Uint8 tilex;
    Uint8 tiley;

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct tilecoord_t

// -----------------------------------
//
// barrier coord/table structure
//
// -----------------------------------

struct barrier_type {
#ifdef BSTONE_AOG
    Uint8 level;
#endif
    tilecoord_t coord;
    Uint8 on;

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct barrier_type;


// ---------------
//
// gamestate structure
//
// ---------------

struct statsInfoType {
    Sint32 total_points;
    Sint32 accum_points;
    Uint8 total_enemy;
    Uint8 accum_enemy;
    Uint8 total_inf;
    Uint8 accum_inf;
    Sint16 overall_floor;

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct statsInfoType

struct levelinfo {
    Uint16 bonus_queue; // bonuses that need to be shown
    Uint16 bonus_shown; // bonuses that have been shown
    boolean locked;
    statsInfoType stats;
    Uint8 ptilex;
    Uint8 ptiley;
    Sint16 pangle;

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct levelinfo


struct fargametype {
    levelinfo old_levelinfo[MAPS_PER_EPISODE];
    levelinfo level[MAPS_PER_EPISODE];

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct fargametype

struct gametype {
    Sint16 turn_around;
    Sint16 turn_angle;
    Uint16 flags;
    Sint16 lastmapon;
    Sint16 difficulty;
    Sint16 mapon;
    Sint16 status_refresh;
    Sint32 oldscore;
    Sint32 tic_score;
    Sint32 score;
    Sint32 nextextra;
    Sint16 score_roll_wait;
    Sint16 lives;
    Sint16 health;
    Sint16 health_delay;
    char health_str[4];

    Sint16 rpower;
    Sint16 old_rpower;
    char rzoom;
    char radar_leds;
    char lastradar_leds;

    char lastammo_leds;
    char ammo_leds;
    Sint16 ammo;
    Sint16 old_ammo;

    Sint16 plasma_detonators;
    Sint16 old_plasma_detonators;

    char useable_weapons;
    char weapons;
    char weapon;
    char chosenweapon;
    char old_weapons[4];
    char key_floor;

    char weapon_wait;
    Sint16 attackframe;
    Sint16 attackcount;
    Sint16 weaponframe;
    Sint16 episode;
    volatile Uint32 TimeCount;
    Sint32 killx;
    Sint32 killy;
    const char* msg; // InfoArea msg...
    char numkeys[NUMKEYS];
    char old_numkeys[NUMKEYS];
    barrier_type barrier_table[MAX_BARRIER_SWITCHES];
    barrier_type old_barrier_table[MAX_BARRIER_SWITCHES];
    Uint16 tokens;
    Uint16 old_tokens;
    boolean boss_key_dropped;
    boolean old_boss_key_dropped;
    Sint16 wintilex;
    Sint16 wintiley;


    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct gametype

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
}; // enum exit_t


struct CycleInfo {
    Uint8 init_delay;
    Uint8 delay_count;
    Uint8 firstreg;
    Uint8 lastreg;
}; // struct CycleInfo


struct visobj_t {
    Sint16 viewx,
           viewheight,
           shapenum;
    char lighting;
    char cloaked;
}; // struct visobj_t


enum animtype_t {
    at_NONE = 0,
    at_CYCLE,
    at_REBOUND,
    at_ONCE
}; // enum animtype_t

enum animdir_t {
    ad_FWD = 0,
    ad_REV
}; // enum animdir_t


struct ofs_anim_t {
    Uint16 animtype: 2; // animtype_t
    Uint16 curframe : 5;
    Uint16 maxframe : 5;
    Uint16 animdir : 1; // animdir_t
    Uint16 extra : 3;
}; // struct ofs_anim_t


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
    MP_max_val = 0x7FFF // DO NOT USE/EXCEED - MAX Val
}; // enum msg_priorities

enum infomsg_type {
    MT_NOTHING,
    MT_CLEAR,
    MT_ATTACK,
    MT_GENERAL,
    MT_OUT_OF_AMMO,
    MT_MALFUNCTION,
    MT_NO_MO_FOOD_TOKENS,
    MT_BONUS
}; // enum infomsg_type


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

    MAX_INSTRUCTIONS
}; // enum inst_type



#define MAX_CACHE_MSGS 30
#define MAX_CACHE_MSG_LEN 190

// ------------------------- BASIC STRUCTURES -----------------------------

// Basic 'message info' structure
//
struct mCacheInfo {
    Uint8 local_val; // where msg is in 'local' list
    Uint8 global_val; // where msg was in 'global' list
    char* mSeg; // pointer to message

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct mCacheInfo

// Basic 'message list' structure
//
struct mCacheList {
    Sint16 NumMsgs; // number of messages
    mCacheInfo mInfo[MAX_CACHE_MSGS]; // table of message 'info'
}; // struct mCacheList

// ----------------------- CONCESSION STRUCTURES --------------------------

// Concession 'message info' structure
//
struct con_mCacheInfo {
    mCacheInfo mInfo;
    Uint8 type; // type of concession
    Uint8 operate_cnt; // # of times req'd to operate

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct con_mCacheInfo

// Concession 'message list' structure
//
struct concession_t {
    Sint16 NumMsgs; // also, num concessions
    con_mCacheInfo cmInfo[MAX_CACHE_MSGS];

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct concession_t

// ------------------------ INFORMANT STRUCTURES --------------------------

// Informant 'message info' structure
//
struct sci_mCacheInfo {
    mCacheInfo mInfo;
    Uint8 areanumber; // 'where' msg can be used
}; // struct sci_mCacheInfo

// Informant 'message list' structure
//
struct scientist_t {
    Sint16 NumMsgs;
    sci_mCacheInfo smInfo[MAX_CACHE_MSGS];
}; // struct scientist_t

// ------------------------------------------------------------------------


// Electro-Alien controller structer
//
struct eaWallInfo {
    char tilex;
    char tiley; // where this controller is in the map.
    char aliens_out; // aliens spawned by this controller.
    Sint16 delay; // delay before spawning another alien.

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct eaWallInfo





// General Structure to hold goldstern specific stuff...
//

struct GoldsternInfo_t {
    Uint8 LastIndex; // Last Spawn Coord Index
    Uint8 SpawnCnt; // Num of Spawnpoints for Goldstern
    Uint16 flags; // What type of command/operation is needed...
    Uint16 WaitTime; // Wait time for Goldstern Spawn (current & Next)
    boolean GoldSpawned; // Has Goldstern been spawned?

    void serialize(
        bstone::BinaryWriter& writer,
        Uint32& checksum) const;

    void deserialize(
        bstone::BinaryReader& reader,
        Uint32& checksum);
}; // struct GoldsternInfo_t


struct star_t {
    Sint32 x, y, z;
    Uint8 color;
}; // struct star_t

#define MAX_SPACE_THRUST ((Sint32)0x4fff)

#define MAX_STARS 128
#define MAX_SPACE_STATS (((MAXSTATS * sizeof(statobj_t)) - ((MAX_STARS + 1) * sizeof(star_t))) / sizeof(statobj_t))





/*
=============================================================================

                                                 3D_MAIN DEFINITIONS

=============================================================================
*/

#define TITLE_LOOP_MUSIC PLOT_MUS

#define CANT_PLAY_TXT "\n" \
    "ERROR: Insufficient disk space.\n" \
    "Try deleting some files from your hard disk.\n\n"


extern char tempPath[];

extern const float radtoint; // = (float)FINEANGLES/2/PI;

extern levelinfo default_level[MAPS_PER_EPISODE];
extern Sint16 view_xl, view_xh, view_yl, view_yh;
extern Sint16 starting_level, debug_value, starting_episode, starting_difficulty;

extern bool MS_CheckParm(
    const std::string& value);

extern signed char lastmap_tilex, lastmap_tiley;
extern Uint16 TopColor, BottomColor;
extern char str[80], str2[20];
// extern  unsigned        tedlevelnum;
// extern  boolean         tedlevel;
extern boolean nospr;
extern boolean IsA386;

extern int focallength;
extern int viewangles;
extern int screenofs;
extern int viewwidth;
extern int viewheight;
extern int centerx;
extern int shootdelta;

extern Sint16 dirangle[9];

extern boolean startgame, loadedgame;
extern Sint16 mouseadjustment;
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

extern boolean ShowQuickMsg;

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
    Sint32 focal);
void SetViewSize(
    int width,
    int height);
void NewGame(
    Sint16 difficulty,
    Sint16 episode);
void NewViewSize();
Uint16 scan_atoi(
    const char* s);
void AlignPlayerOnTransporter();

Uint16 UseFunc(
    const char* first,
    const char* next);
boolean DoMovie(
    movie_t movie,
    void* palette);
boolean CheckDiskSpace(
    Sint32 needed,
    const char* text,
    cds_io_type io_type);

bool SaveTheGame(
    bstone::IStream* stream,
    const std::string& description);

Sint32 ChecksumFile(
    char* file,
    Sint32 checksum);
void BadChecksum();
void InvalidLevels();
void CheckValidity(
    char* file,
    Sint32 valid_checksum);
void UnauthorizedLevels();
void ShowChecksums();
void fprint(
    char* text);

void SetupWalls();
void InitDigiMap();

void CleanUpDoors_N_Actors();


void MakeDestPath(
    const char* file);
void InitDestPath();

int FindChunk(
    bstone::IStream* stream,
    const std::string& chunk_name);

int NextChunk(
    bstone::IStream* stream);

/*
=============================================================================

 3D_GAME DEFINITIONS

=============================================================================
*/

extern Sint16 db_count;
extern classtype debug_bonus[2][800];
extern fargametype gamestuff;
extern tilecoord_t GoldieList[GOLDIE_MAX_SPAWNS];
extern GoldsternInfo_t GoldsternInfo;

extern Uint8 VitalsRemain, VitalsOnFloor;

extern eaWallInfo eaList[];
extern char NumEAWalls, NumEASpawned;
extern boolean ingame, fizzlein, screensplit;
extern int latchpics[NUMLATCHPICS];
extern gametype gamestate;
extern Sint16 doornum;

extern char demoname[13];

void DrawPlayBorder();
void ScanInfoPlane();
void SetupGameLevel();
void NormalScreen();
void DrawPlayScreen(boolean);
void FizzleOut();
void GameLoop();

// JAB
#define PlaySoundLocTile(s, tx, ty) PlaySoundLocGlobal(s, (((Sint32)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))), (((Sint32)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))
#define PlaySoundLocActor(s, ob) PlaySoundLocGlobal(s, (ob)->x, (ob)->y)
void PlaySoundLocGlobal(
    Uint16 s,
    fixed gx,
    fixed gy);


void Warped();
void RotateView(
    Sint16 DestAngle,
    Uint8 RotSpeed);
void DrawWarpIn();
void BMAmsg(
    const char* msg);
void CacheBMAmsg(
    Uint16 MsgNum);
void BevelBox(
    Sint16 xl,
    Sint16 yl,
    Sint16 w,
    Sint16 h,
    Uint8 hi,
    Uint8 med,
    Uint8 lo);

void AddTotalPoints(
    Uint16 points);
void AddTotalInformants(
    char informants);
void AddTotalEnemy(
    Uint16 enemies);

void ShadowPrintLocationText(
    sp_type type);
void LoseScreen();
void LoadLocationText(
    Sint16 textNum);

/*
=============================================================================

 3D_PLAY DEFINITIONS

=============================================================================
*/

extern int objcount;

extern objtype* DeadGuys[], dummyobj;
extern Uint8 NumDeadGuys;

extern exit_t playstate;

extern Sint16 bordertime;

extern boolean madenoise, usedummy, nevermark;
extern Uint8 alerted, alerted_areanum;

extern objtype objlist[MAXACTORS], * new_actor, * player, * lastobj,
               * objfreelist, * killerobj;
extern statobj_t statobjlist[MAXSTATS], * laststatobj;
extern doorobj_t doorobjlist[MAXDOORS], * lastdoorobj;

extern Uint16 farmapylookup[MAPSIZE];
extern Uint8* nearmapylookup[MAPSIZE];

extern Uint8 tilemap[MAPSIZE][MAPSIZE]; // wall values only
extern Uint8 spotvis[MAPSIZE][MAPSIZE];
extern objtype* actorat[MAPSIZE][MAPSIZE];

#define UPDATESIZE (UPDATEWIDE * UPDATEHIGH)
extern Uint8 update[UPDATESIZE];

extern boolean singlestep, godmode, noclip;
extern Sint16 extravbls;
extern Sint16 DebugOk;
extern Sint16 InstantWin, InstantQuit;
extern boolean PowerBall;
extern Sint16 TestQuickSave, TestAutoMapper;
extern Uint16 ExtraRadarFlags;

//
// control info
//
extern boolean mouseenabled, joystickenabled, joypadenabled, joystickprogressive;
extern Sint16 joystickport;
extern Sint16 dirscan[4];
extern Sint16 buttonscan[NUMBUTTONS];
extern Sint16 buttonmouse[4];
extern Sint16 buttonjoy[4];

extern bool buttonheld[NUMBUTTONS];

extern const int viewsize;

//
// curent user input
//
extern int controlx;
extern int controly; // range from -100 to 100
extern bool buttonstate[NUMBUTTONS];
extern int strafe_value;

extern boolean demorecord, demoplayback;
extern char* demoptr, * lastdemoptr;
extern void* demobuffer;

extern char Computing[];


void CenterWindow(
    Uint16 w,
    Uint16 h);
void InitActorList();
void GetNewActor();
void RemoveObj(
    objtype* gone);
void PollControls();
void StopMusic();
void StartMusic(
    boolean startit);
void PlayLoop();

void ChangeSwapFiles(
    boolean display);
void OpenPageFile();
void CheckMusicToggle();

/*
=============================================================================

 3D_DEBUG

=============================================================================
*/

Sint16 DebugKeys();
void CalcMemFree();


/*
=============================================================================

 3D_DRAW DEFINITIONS

=============================================================================
*/

extern Uint8 TravelTable[MAPSIZE][MAPSIZE];

extern Sint16 weaponchangetics, itemchangetics, bodychangetics;
extern Sint16 plaqueon, plaquetime, plaquetimefrac, getpic;

extern statobj_t* firststarobj;

extern int screenloc[3];
extern int freelatch;

extern Sint32 space_xmove, space_ymove;

extern Sint32 lasttimecount;
extern Sint32 framecount;
extern Sint32 frameon;
extern boolean fizzlein;

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

extern const Uint8* postsource;
extern int postx;


extern Sint16 horizwall[], vertwall[];

extern Uint16 pwallpos;

extern boolean cloaked_shape;

fixed FixedByFrac(
    fixed a,
    fixed b);
void TransformActor(
    objtype* ob);
void BuildTables();
void ClearScreen();
Sint16 CalcRotate(
    objtype* ob);
void DrawScaleds();
void CalcTics();
void FixOfs();
void ThreeDRefresh();
void FarScalePost();
void DrawStars();



boolean TransformTile(
    Sint16 tx,
    Sint16 ty,
    Sint16* dispx,
    Sint16* dispheight);
void WrapTransformActor(
    objtype* ob);
void ComputeActorPosition(
    objtype* ob,
    char adjust_x,
    char adjust_y);
void WrapDrawScaleds();
boolean WrapActorPosition(
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
extern Uint16 CeilingTile, FloorTile;
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

extern Uint16 actor_points[];
extern dirtype opposite[9];
extern dirtype diagonal[9][9];


void SeekPlayerOrStatic(
    objtype* ob,
    Sint16* deltax,
    Sint16* deltay);
Uint16 CheckRunChase(
    objtype* ob);
void GetCornerSeek(
    objtype* ob);
boolean LookForGoodies(
    objtype* ob,
    Uint16 RunReason);
void InitHitRect(
    objtype* ob,
    Uint16 radius);
void SpawnNewObj(
    Uint16 tilex,
    Uint16 tiley,
    statetype* state);
void NewState(
    objtype* ob,
    statetype* state);

boolean TryWalk(
    objtype* ob,
    boolean moveit);
void SelectChaseDir(
    objtype* ob);
void SelectDodgeDir(
    objtype* ob);
void MoveObj(
    objtype* ob,
    Sint32 move);

void KillActor(
    objtype* ob);
void DamageActor(
    objtype* ob,
    Uint16 damage,
    objtype* attacker);

boolean CheckLine(
    objtype* from_obj,
    objtype* to_obj);
boolean CheckSight(
    objtype* from_obj,
    objtype* to_obj);

boolean PosVisable(
    fixed from_x,
    fixed from_y,
    fixed to_x,
    fixed to_y,
    Sint16 from_angle);
boolean PlayerIsBlocking(
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
    Uint16 leftpix, rightpix;
    Uint16 dataofs[64];
// table data after dataofs[rightpix-leftpix+1]
}; // struct t_compshape

extern int maxscale;
extern int maxscaleshl2;
extern boolean scaledir_avail;

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

extern Uint16 static_points[];
extern boolean GAN_HiddenArea;
extern char* InfAreaMsgs[];
extern Uint8 NumAreaMsgs, LastInfArea;
extern Sint16 FirstGenInfMsg, TotalGenInfMsgs;
extern classtype LastInfoAttacker;

#ifdef BSTONE_PS
extern Sint16 LastInfoAttacker_Cloaked;
#endif

extern char term_com_name[];
extern char term_msg_name[];

extern atkinf_t attackinfo[7][14];

//
// player state info
//
extern boolean commandmode;
extern Sint32 thrustspeed;
extern Uint16 plux, pluy; // player coordinates scaled to unsigned
extern boolean PlayerInvisable;
extern char DrawInfoArea_COUNT;
extern char InitInfoArea_COUNT;

extern Uint16 player_oldtilex;
extern Uint16 player_oldtiley;

// Terminal variables

extern Uint16 RadarSw;

// Location Bar message string...

extern char LocationText[MAX_LOCATION_DESC_LEN];


//
// search / move info
//
extern Uint16 searchon; // held object number looking at
extern Sint16 searchtics; // when it reaches SEARCHTICS, get an obj
extern objtype* searchobj; // current object being searched
extern Uint16 foundflag; // only show NOTHING if nothing was found
extern objtype* moveobj; // current object being draged

extern Sint16 anglefrac;
extern Sint16 facecount;

extern Uint16 LastMsgPri;
extern Sint16 MsgTicsRemain;

void GivePoints(
    Sint32 score,
    boolean add_to_stats);
void SpawnPlayer(
    Sint16 tilex,
    Sint16 tiley,
    Sint16 dir);
void DrawCash();
void UpdateHealth();

void DrawAmmoGuage();
void DrawAmmoMsg();
void DrawAmmo(
    boolean ForceRefresh);
boolean DisplayInfoMsg(
    const char* Msg,
    msg_priorities Priority,
    Sint16 DisplayTime,
    Sint16 MessageType);
void UpdateInfoAreaClock();
void UpdateInfoArea();
void DrawHealthMonitor();
void CalcHealthDisplay();
void UpdateScore();

Uint8 ValidAreaTile(
    Uint16* ptr);
char GetAreaNumber(
    char tilex,
    char tiley);
Sint16 InputFloor();

void RestoreInfoArea();
void DrawHeartPic();
void DrawInfoArea();
Sint16 DrawShape(
    Sint16 x,
    Sint16 y,
    Sint16 shapenum,
    pisType shapetype);

void AnimatePage();

void ActivateTerminal(boolean);
void TerminalPrint(
    char* msg,
    boolean FastPrint);
void FloorCheat(
    Uint16 RadarFlags);
boolean Interrogate(
    objtype* ob);

void GiveKey(
    Sint16 key);
void TakeKey(
    Sint16 key);
void GiveToken(
    Sint16 tokens);

#ifdef BSTONE_PS
void TakePlasmaDetonator(
    Sint16 count);
void GivePlasmaDetonator(
    Sint16 count);
#endif

void CacheDrawPic(
    int x,
    int y,
    int pic);
void LoadTerminalCommands();

void ActivateWallSwitch(
    Uint16 iconnum,
    Sint16 x,
    Sint16 y);
Uint16 UpdateBarrierTable(
    Uint8 level,
    Uint8 x,
    Uint8 y,
    boolean OnOff);
Uint16 ScanBarrierTable(
    Uint8 x,
    Uint8 y);
void DisplaySwitchOperateMsg(
    Uint16 coords);

void DisplayNoMoMsgs();
void PrintStatPercent(
    Sint16 nx,
    Sint16 ny,
    char percentage);
Sint16 ShowStats(
    Sint16 bx,
    Sint16 by,
    ss_type type,
    statsInfoType* stats);
boolean PerfectStats();
boolean CheckPerfectStats();
boolean OperateSmartSwitch(
    Uint16 tilex,
    Uint16 tiley,
    char Operation,
    boolean Force);

/*
=============================================================================

 3D_ACT1 DEFINITIONS

=============================================================================
*/
extern char xy_offset[8][2];
extern stattype statinfo[];
extern concession_t ConHintList;

extern doorobj_t doorobjlist[MAXDOORS], * lastdoorobj;
extern Sint16 doornum;

extern Uint16 doorposition[MAXDOORS], pwallstate;

extern Uint8 areaconnect[NUMAREAS][NUMAREAS];

extern bool areabyplayer[NUMAREAS];

extern Uint16 pwallstate;
extern Uint16 pwallpos; // amount a pushable wall has been moved (0-63)
extern Uint16 pwallx, pwally;
extern Sint16 pwalldir, pwalldist;


statobj_t* ReserveStatic();
void SpawnStatic(
    Sint16 tilex,
    Sint16 tiley,
    Sint16 type);
void SpawnDoor(
    Sint16 tilex,
    Sint16 tiley,
    boolean vertical,
    keytype lock,
    door_t type);

void OperateConcession(
    Uint16 concession);
void SpawnConcession(
    Sint16 tilex,
    Sint16 tiley,
    Uint16 credits,
    Uint16 machinetype);
Uint16 LoadConcessionHint(
    Uint16 MsgNum);
void CacheInfoAreaMsg(
    Uint16 block,
    Uint16 MsgNum,
    char* hint_buffer,
    Uint16 MaxBufferLen);
void CheckSpawnEA();

Sint16 TransformAreas(
    char tilex,
    char tiley,
    char xform);


void CheckSpawnGoldstern();
void FindNewGoldieSpawnSite();

void InitMsgCache(
    mCacheList* mList,
    Uint16 listSize,
    Uint16 infoSize);
void FreeMsgCache(
    mCacheList* mList,
    Uint16 listSize);
void CacheMsg(
    mCacheInfo* ci,
    Uint16 SegNum,
    Uint16 MsgNum);
Sint16 LoadMsg(
    char* hint_buffer,
    Uint16 SegNum,
    Uint16 MsgNum,
    Uint16 MaxMsgLen);
void CacheConcessionMsg();
boolean ReuseMsg(
    mCacheInfo* ci,
    Sint16 count,
    Sint16 struct_size);

#ifdef BSTONE_PS
void DropPlasmaDetonator();
#endif

void BlockDoorOpen(
    Sint16 door);
void BlastNearDoors(
    Sint16 tilex,
    Sint16 tiley);
void TryBlastDoor(
    char door);

statobj_t* FindStatic(
    Uint16 tilex,
    Uint16 tiley);
statobj_t* UseReservedStatic(
    Sint16 itemtype,
    Sint16 tilex,
    Sint16 tiley);
void PlaceReservedItemNearTile(
    Sint16 itemtype,
    Sint16 tilex,
    Sint16 tiley);
void ExplodeStatics(
    Sint16 tilex,
    Sint16 tiley);


/*
=============================================================================

 3D_ACT2 DEFINITIONS

=============================================================================
*/

#define s_nakedbody s_static10


#define BARRIER_STATE(obj) ((obj)->ammo)

#ifdef BSTONE_AOG
#define InitSmartAnim(obj, ShapeNum, StartOfs, MaxOfs, AnimType, AnimDir) \
    InitSmartSpeedAnim(obj, ShapeNum, StartOfs, MaxOfs, AnimType, AnimDir, 21)
#else
#define InitSmartAnim(obj, ShapeNum, StartOfs, MaxOfs, AnimType, AnimDir) \
    InitSmartSpeedAnim(obj, ShapeNum, StartOfs, MaxOfs, AnimType, AnimDir, 7)
#endif


void MakeFakeStatic(
    objtype* ob);
void UnmakeFakeStatic(
    objtype* ob);

#ifdef BSTONE_PS
extern char detonators_spawned;
#endif

extern Sint16 starthitpoints[][NUMHITENEMIES];

#ifdef BSTONE_PS
extern Uint16 MorphClass[];
#endif

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
    Sint16 tilex,
    Sint16 tiley,
    Sint16 dir);
void SpawnPatrol(
    enemy_t which,
    Sint16 tilex,
    Sint16 tiley,
    Sint16 dir);
void KillActor(
    objtype* ob);

void US_ControlPanel(Uint8);

Sint16 IntSqrt(
    Sint32 va);
Uint16 CalcDistance(
    Uint16 x1,
    Uint16 y1,
    Uint16 x2,
    Uint16 y2);


void T_Hit(
    objtype* ob);
void SpawnOffsetObj(
    enemy_t which,
    Sint16 tilex,
    Sint16 tiley);


void InitSmartAnimStruct(
    objtype* obj,
    Uint16 ShapeNum,
    Uint8 StartOfs,
    Uint8 MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir);
boolean AnimateOfsObj(
    objtype* obj);
void AdvanceAnimREV(
    objtype* obj);
void AdvanceAnimFWD(
    objtype* obj);

void SpawnCusExplosion(
    fixed x,
    fixed y,
    Uint16 StartFrame,
    Uint16 NumFrames,
    Uint16 Delay,
    Uint16 Class);
void T_SpawnExplosion(
    objtype* obj);
void T_ExplodeDamage(
    objtype* obj);

void ExplodeRadius(
    objtype* obj,
    Sint16 damage,
    boolean damageplayer);

extern statetype s_barrier_transition;
extern statetype s_barrier_shutdown;

void SpawnBarrier(
    enemy_t which,
    Sint16 tilex,
    Sint16 tiley,
    boolean OnOff);
void ToggleBarrier(
    objtype* obj);

void InitAnim(
    objtype* obj,
    Uint16 ShapeNum,
    Uint8 StartOfs,
    Uint8 MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir,
    Uint16 Delay,
    Uint16 WaitDelay);

objtype* FindObj(
    classtype which,
    Sint16 tilex,
    Sint16 tiley);
objtype* FindHiddenOfs(
    classtype which);
void SpawnHiddenOfs(
    enemy_t which,
    Sint16 tilex,
    Sint16 tiley);
objtype* MoveHiddenOfs(
    classtype which_class,
    classtype new1,
    fixed x,
    fixed y);

void CheckForSpecialTile(
    objtype* obj,
    Uint16 tilex,
    Uint16 tiley);
void DropCargo(
    objtype* obj);


/*
=============================================================================

 3D_TEXT DEFINITIONS

=============================================================================
*/

extern char helpfilename[], endfilename[];

extern void HelpScreens();
extern void EndText();


/*
=============================================================================

 3D_MSGS TEXT DEFINITIONS

=============================================================================
*/


extern char noeat_msg1[];

#ifdef BSTONE_AOG
extern const char bevs_msg1[];
extern const char food_msg1[];
#else
extern char bevs_msg1[];
extern char food_msg1[];
#endif

extern char bonus_msg7[];

#ifdef BSTONE_PS
extern char bonus_msg26[];
#endif

extern char* BonusMsg[];
extern const char* ActorInfoMsg[];
extern char ekg_heartbeat_enabled[];
extern char ekg_heartbeat_disabled[];
extern char attacker_info_enabled[];
extern char attacker_info_disabled[];
extern char WeaponNotAvailMsg[];
extern char WeaponAvailMsg[];
extern char RadarAvailMsg[];
extern char RadarEnergyGoneMsg[];
extern char WeaponAutoSelectMsg[];
extern char EnergyPackDepleted[];
extern char NotEnoughEnergyForWeapon[];

extern char WeaponMalfunction[];

extern char SwitchNotActivateMsg[];
extern char NoFoodTokens[];
extern char ExtraMan[];
extern char OneMillion[];
extern char TenMillion[];

extern char NoAdLibCard[];
extern char MusicOn[];
extern char MusicOff[];
extern char SoundOn[];
extern char SoundOff[];

#ifdef BSTONE_PS
extern char pd_dropped[];
extern char pd_nomore[];
extern char pd_switching[];
extern char pd_notnear[];
extern char pd_getcloser[];
extern char pd_floorunlocked[];
extern char pd_donthaveany[];
extern char pd_no_computer[];
extern char pd_floornotlocked[];
#endif


/*
=============================================================================

 3D_INTER DEFINITIONS

=============================================================================
*/

extern char BreifingText[];

void UpdateScreenPic();
void DisplayPrepingMsg(
    const char* text);
boolean Breifing(
    breifing_type BreifingType,
    Uint16 episode);
void ShPrint(
    const char* text,
    char shadow_color,
    boolean single_char);
Uint16 Random(
    Uint16 Max);


// ===========================================================================
//
// 3D_MENU DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================

extern boolean EscPressed;

void DrawInstructions(
    inst_type Type);
void CacheMessage(
    Uint16 MessageNum);
void TerminateStr(
    char* pos);
Uint32 CacheCompData(
    Uint16 ItemNum,
    void** dest_loc);
boolean CheckForSpecialCode(
    Uint16 shortItemNum);


// ===========================================================================
//
// 3D_FREE DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================


extern char JM_FREE_DATA_END[];
extern char JM_FREE_DATA_START[];


// BBi
objtype* ui16_to_actor(
    Uint16 value);
Uint16 actor_to_ui16(
    const objtype* actor);

statobj_t* ui16_to_static_object(
    Uint16 value);
Uint16 static_object_to_ui16(
    const statobj_t* static_object);

doorobj_t* ui16_to_door_object(
    Uint16 value);
Uint16 door_object_to_ui16(
    const doorobj_t* door_object);

extern bool g_no_wall_hit_sound;
extern bool g_always_run;

// BBi AOG only options
extern bool g_heart_beat_sound;
extern bool g_rotated_automap;


class ArchiveException : public std::exception {
public:
explicit ArchiveException(
    const char* what) throw ();

ArchiveException(
    const ArchiveException& that) throw ();

virtual ~ArchiveException() throw ();

ArchiveException& operator=(
    const ArchiveException& that) throw ();

virtual const char* what() const throw ();

private:
const char* what_;
}; // class ArchiveException


template<class T>
inline void DoChecksum(
    const T& value,
    Uint32& checksum)
{
    const Uint8* src = reinterpret_cast<const Uint8*>(&value);

    for (size_t i = 0; i < sizeof(T); ++i) {
        checksum += src[i] + 1;
        checksum *= 31;
    }
}

template<class T>
inline void serialize_field(
    const T& value,
    bstone::BinaryWriter& writer,
    Uint32& checksum)
{
    ::DoChecksum(value, checksum);
    if (!writer.write(bstone::Endian::le(value))) {
        throw ArchiveException("serialize_field");
    }
}

template<class T, size_t N>
inline void serialize_field(
    const T(&value)[N],
    bstone::BinaryWriter& writer,
    Uint32& checksum)
{
    for (size_t i = 0; i < N; ++i) {
        ::serialize_field<T>(value[i], writer, checksum);
    }
}

template<class T, size_t M, size_t N>
inline void serialize_field(
    const T(&value)[M][N],
    bstone::BinaryWriter& writer,
    Uint32& checksum)
{
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            ::serialize_field<T>(value[i][j], writer, checksum);
        }
    }
}

template<class T>
inline void deserialize_field(
    T& value,
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    if (!reader.read(value)) {
        throw ArchiveException("deserialize_field");
    }

    bstone::Endian::lei(value);
    ::DoChecksum(value, checksum);
}

template<class T, size_t N>
inline void deserialize_field(
    T(&value)[N],
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    for (size_t i = 0; i < N; ++i) {
        ::deserialize_field<T>(value[i], reader, checksum);
    }
}

template<class T, size_t M, size_t N>
inline void deserialize_field(
    T(&value)[M][N],
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            ::deserialize_field<T>(value[i][j], reader, checksum);
        }
    }
}
// BBi
