/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_3D_DEF_INCLUDED
#define BSTONE_3D_DEF_INCLUDED


#include <array>
#include <atomic>
#include <bitset>
#include <functional>
#include <string>
#include <vector>

#include "movie.h"

#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_cvar_mgr.h"
#include "bstone_game_timer.h"
#include "bstone_math.h"
#include "bstone_mt_task_mgr.h"
#include "bstone_string_view.h"
#include "bstone_voice.h"


constexpr auto min_fixed = 1;
constexpr auto min_fixed_floating = bstone::math::fixed_to_floating(min_fixed);


namespace bstone
{


class Archiver;
class MemoryStream;
class Stream;


} // bstone


#define LOOK_FOR_DEAD_GUYS (0)

#define LIMITED_AMMO


enum class ScanCode;


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


/*
=============================================================================

 MACROS

=============================================================================
*/


#define MAX_RADAR_ENERGY (14400)
#define RADAR_PAK_VALUE (60 * 15)

#define MAXKEYS (1)

#define ABS(x) (static_cast<std::int16_t>(x) > 0 ? (x) : -(x))
#define LABS(x) (static_cast<std::int32_t>(x) > 0 ? (x) : -(x))

#define DISPLAY_MSG_STD_TIME (5 * 60) // Tics display len
#define DISPLAY_TIMED_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), DISPLAY_MSG_STD_TIME, (type))
#define DISPLAY_MSG(msg, pri, type) DisplayInfoMsg((msg), (pri), 0, (type))


// SMART_ACTORS is the "case" used for certain switch statements.
//

#define SMART_ACTORS proguardobj : \
case rentacopobj : \
case gen_scientistobj


// Barrier Code Stuff
#if 0
#define MAX_BARRIER_SWITCHES (40) // max number level wall switches
#endif


#define MAX_INF_AREA_MSGS (6)
#define MAX_LOCATION_DESC_LEN (45)


/*
=============================================================================

 GLOBAL CONSTANTS

=============================================================================
*/

constexpr auto MINDIST = bstone::math::fixed_to_floating(0x5800);


#define OV_ACTORS (0x0001)
#define OV_SHOWALL (0x0002)
#define OV_KEYS (0x0004)
#define OV_PUSHWALLS (0x0008)

// BBi
#define OV_WHOLE_MAP (0x0010)


#define TT_TRAVELED (0x01)
#define TT_KEYS (0x02)

constexpr auto MAXACTORS = 200; // max number of nazis, etc / map

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


// ----------------

#define EXTRAPOINTS (400000)

#define MAX_EXTRA_LIVES (4)

constexpr auto RUNSPEED = bstone::math::fixed_to_floating(6000);

#define HEIGHTRATIO (0.41)
#define TOP_STRIP_HEIGHT (16) // Pix height of top strip.

constexpr auto PLAYERSIZE = MINDIST; // player radius
constexpr auto MINACTORDIST = bstone::math::fixed_to_floating(0x10000); // minimum dist from player center
// to any actor center

#define NUMLATCHPICS (100)


#define GLOBAL1 (1L << 16)
#define TILEGLOBAL (GLOBAL1)
#define TILESHIFT (16L)
#define UNSIGNEDSHIFT (8)

#define ANGLES (360) // must be divisable by 4
#define ANGLEQUAD (ANGLES / 4)
#define FINEANGLES (3600)

#define MAPSIZE (64) // maps are 64*64 max
#define NORTH (0)
#define EAST (1)
#define SOUTH (2)
#define WEST (3)

#define STATUSLINES (48)

#define STARTAMMO (8)

// Token Definations

#define MAX_TOKENS (25)



// Ammo/Weapon Definations

#define MAX_AMMO (100) // Max amount of ammo for any weapon
#define AUTOCHARGE_WAIT (50) // Tics wait time for a full charge

#define MAX_PLASMA_DETONATORS (100) // Max number of Plasma Detonators
#define PLASMA_DETONATORS_DELAY (60 * 4) // Number of tics before plasma detonator explodes



// gamestate.flags flag values

#define GS_KILL_INF_WARN (0x0080)

// object flag values - Oh Shit Longs!

#define FL_SHOOTABLE (0x00000001)
#define FL_BONUS (0x00000002)
#define FL_NEVERMARK (0x00000004)
#define FL_VISIBLE (0x00000008)
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
// Smart Trigger Possibilities
//
#define ST_TURN_OFF (0x00)
#define ST_TURN_ON (0x01)
#define ST_TOGGLE (0x02)


//
//
//

#define CANT_SAVE_GAME_TXT " Can't save this game! \n" \
    "    Hard Disk FULL!"

enum ss_type
{
	ss_normal,
	ss_quick,
	ss_justcalc
}; // ss_type

enum cds_io_type
{
	cds_dos_print,
	cds_id_print,
	cds_menu_print
}; // cds_io_type

enum sp_type
{
	sp_normal,
	sp_loading,
	sp_saving,
	sp_changeview,
	sp_teleporting
}; // sp_type

//
// sprite constants
//

extern std::int16_t SPR_DEMO;

//
// Static sprites
//
extern std::int16_t SPR_STAT_0;
extern std::int16_t SPR_STAT_1;
extern std::int16_t SPR_STAT_2;
extern std::int16_t SPR_STAT_3;
extern std::int16_t SPR_STAT_4;
extern std::int16_t SPR_STAT_5;
extern std::int16_t SPR_STAT_6;
extern std::int16_t SPR_STAT_7;
extern std::int16_t SPR_STAT_8;
extern std::int16_t SPR_STAT_9;
extern std::int16_t SPR_STAT_10;
extern std::int16_t SPR_STAT_11;
extern std::int16_t SPR_STAT_12;
extern std::int16_t SPR_STAT_13;
extern std::int16_t SPR_STAT_14;
extern std::int16_t SPR_STAT_15;
extern std::int16_t SPR_STAT_16;
extern std::int16_t SPR_STAT_17;
extern std::int16_t SPR_STAT_18;
extern std::int16_t SPR_STAT_19;
extern std::int16_t SPR_STAT_20;
extern std::int16_t SPR_STAT_21;
extern std::int16_t SPR_STAT_22;
extern std::int16_t SPR_STAT_23;
extern std::int16_t SPR_STAT_24;
extern std::int16_t SPR_STAT_25;
extern std::int16_t SPR_STAT_26;
extern std::int16_t SPR_STAT_27;
extern std::int16_t SPR_STAT_28;
extern std::int16_t SPR_STAT_29;
extern std::int16_t SPR_STAT_30;
extern std::int16_t SPR_STAT_31;
extern std::int16_t SPR_STAT_32;
extern std::int16_t SPR_STAT_33;
extern std::int16_t SPR_STAT_34; // Green Key (AOG) / Anti-Plasma Cannon (PS)
extern std::int16_t SPR_STAT_35;
extern std::int16_t SPR_STAT_36; // Gold Key (AOG) / Yellow Package? (PS)
extern std::int16_t SPR_STAT_37;
extern std::int16_t SPR_STAT_38;
extern std::int16_t SPR_STAT_39;
extern std::int16_t SPR_STAT_40;
extern std::int16_t SPR_STAT_41;
extern std::int16_t SPR_STAT_42;
extern std::int16_t SPR_STAT_43;
extern std::int16_t SPR_STAT_44;
extern std::int16_t SPR_STAT_45;
extern std::int16_t SPR_STAT_46;
extern std::int16_t SPR_STAT_47;
extern std::int16_t SPR_STAT_48;
extern std::int16_t SPR_STAT_49;
extern std::int16_t SPR_STAT_50;
extern std::int16_t SPR_STAT_51;
extern std::int16_t SPR_STAT_52;
extern std::int16_t SPR_STAT_53;
extern std::int16_t SPR_STAT_54;
extern std::int16_t SPR_STAT_55;
extern std::int16_t SPR_STAT_56;

extern std::int16_t SPR_CRATE_1;
extern std::int16_t SPR_CRATE_2;
extern std::int16_t SPR_CRATE_3;

extern std::int16_t SPR_STAT_57;
extern std::int16_t SPR_STAT_58;
extern std::int16_t SPR_STAT_59;
extern std::int16_t SPR_STAT_60;
extern std::int16_t SPR_STAT_61;
extern std::int16_t SPR_STAT_62;
extern std::int16_t SPR_STAT_63;
extern std::int16_t SPR_STAT_64;
extern std::int16_t SPR_STAT_65;
extern std::int16_t SPR_STAT_66;
extern std::int16_t SPR_STAT_67;
extern std::int16_t SPR_STAT_68;
extern std::int16_t SPR_STAT_69;
extern std::int16_t SPR_STAT_70;
extern std::int16_t SPR_STAT_71;
extern std::int16_t SPR_STAT_72;
extern std::int16_t SPR_STAT_73;
extern std::int16_t SPR_STAT_74;
extern std::int16_t SPR_STAT_75;
extern std::int16_t SPR_STAT_76;
extern std::int16_t SPR_STAT_77;
extern std::int16_t SPR_STAT_78;
extern std::int16_t SPR_STAT_79;

extern std::int16_t SPR_DOORBOMB;
extern std::int16_t SPR_ALT_DOORBOMB;
extern std::int16_t SPR_RUBBLE;
extern std::int16_t SPR_BONZI_TREE;
extern std::int16_t SPR_AUTOMAPPER;
extern std::int16_t SPR_POT_PLANT;
extern std::int16_t SPR_TUBE_PLANT;
extern std::int16_t SPR_HITECH_CHAIR;

//
// Aesthetics
//
extern std::int16_t SPR_AIR_VENT;
extern std::int16_t SPR_BLOOD_DRIP1;
extern std::int16_t SPR_BLOOD_DRIP2;
extern std::int16_t SPR_BLOOD_DRIP3;
extern std::int16_t SPR_BLOOD_DRIP4;
extern std::int16_t SPR_WATER_DRIP1;
extern std::int16_t SPR_WATER_DRIP2;
extern std::int16_t SPR_WATER_DRIP3;
extern std::int16_t SPR_WATER_DRIP4;

extern std::int16_t SPR_DECO_ARC_1;
extern std::int16_t SPR_DECO_ARC_2;
extern std::int16_t SPR_DECO_ARC_3;

extern std::int16_t SPR_GRATE;
extern std::int16_t SPR_STEAM_1;
extern std::int16_t SPR_STEAM_2;
extern std::int16_t SPR_STEAM_3;
extern std::int16_t SPR_STEAM_4;

extern std::int16_t SPR_STEAM_PIPE;
extern std::int16_t SPR_PIPE_STEAM_1;
extern std::int16_t SPR_PIPE_STEAM_2;
extern std::int16_t SPR_PIPE_STEAM_3;
extern std::int16_t SPR_PIPE_STEAM_4;

//
// Dead Actors (from Blake Stone: AOG)
//
extern std::int16_t SPR_DEAD_RENT;
extern std::int16_t SPR_DEAD_PRO;
extern std::int16_t SPR_DEAD_SWAT;

//
// Sector Patrol (AOG) / Rent-A-Cop (PS)
//
extern std::int16_t SPR_RENT_S_1;
extern std::int16_t SPR_RENT_S_2;
extern std::int16_t SPR_RENT_S_3;
extern std::int16_t SPR_RENT_S_4;
extern std::int16_t SPR_RENT_S_5;
extern std::int16_t SPR_RENT_S_6;
extern std::int16_t SPR_RENT_S_7;
extern std::int16_t SPR_RENT_S_8;

extern std::int16_t SPR_RENT_W1_1;
extern std::int16_t SPR_RENT_W1_2;
extern std::int16_t SPR_RENT_W1_3;
extern std::int16_t SPR_RENT_W1_4;
extern std::int16_t SPR_RENT_W1_5;
extern std::int16_t SPR_RENT_W1_6;
extern std::int16_t SPR_RENT_W1_7;
extern std::int16_t SPR_RENT_W1_8;

extern std::int16_t SPR_RENT_W2_1;
extern std::int16_t SPR_RENT_W2_2;
extern std::int16_t SPR_RENT_W2_3;
extern std::int16_t SPR_RENT_W2_4;
extern std::int16_t SPR_RENT_W2_5;
extern std::int16_t SPR_RENT_W2_6;
extern std::int16_t SPR_RENT_W2_7;
extern std::int16_t SPR_RENT_W2_8;

extern std::int16_t SPR_RENT_W3_1;
extern std::int16_t SPR_RENT_W3_2;
extern std::int16_t SPR_RENT_W3_3;
extern std::int16_t SPR_RENT_W3_4;
extern std::int16_t SPR_RENT_W3_5;
extern std::int16_t SPR_RENT_W3_6;
extern std::int16_t SPR_RENT_W3_7;
extern std::int16_t SPR_RENT_W3_8;

extern std::int16_t SPR_RENT_W4_1;
extern std::int16_t SPR_RENT_W4_2;
extern std::int16_t SPR_RENT_W4_3;
extern std::int16_t SPR_RENT_W4_4;
extern std::int16_t SPR_RENT_W4_5;
extern std::int16_t SPR_RENT_W4_6;
extern std::int16_t SPR_RENT_W4_7;
extern std::int16_t SPR_RENT_W4_8;

extern std::int16_t SPR_RENT_DIE_1;
extern std::int16_t SPR_RENT_DIE_2;
extern std::int16_t SPR_RENT_DIE_3;
extern std::int16_t SPR_RENT_DIE_4;
extern std::int16_t SPR_RENT_PAIN_1;
extern std::int16_t SPR_RENT_DEAD;

extern std::int16_t SPR_RENT_SHOOT1;
extern std::int16_t SPR_RENT_SHOOT2;
extern std::int16_t SPR_RENT_SHOOT3;

//
// Star Sentinel (AOG) / Pro-Guard (PS)
//
extern std::int16_t SPR_PRO_S_1;
extern std::int16_t SPR_PRO_S_2;
extern std::int16_t SPR_PRO_S_3;
extern std::int16_t SPR_PRO_S_4;
extern std::int16_t SPR_PRO_S_5;
extern std::int16_t SPR_PRO_S_6;
extern std::int16_t SPR_PRO_S_7;
extern std::int16_t SPR_PRO_S_8;

extern std::int16_t SPR_PRO_W1_1;
extern std::int16_t SPR_PRO_W1_2;
extern std::int16_t SPR_PRO_W1_3;
extern std::int16_t SPR_PRO_W1_4;
extern std::int16_t SPR_PRO_W1_5;
extern std::int16_t SPR_PRO_W1_6;
extern std::int16_t SPR_PRO_W1_7;
extern std::int16_t SPR_PRO_W1_8;

extern std::int16_t SPR_PRO_W2_1;
extern std::int16_t SPR_PRO_W2_2;
extern std::int16_t SPR_PRO_W2_3;
extern std::int16_t SPR_PRO_W2_4;
extern std::int16_t SPR_PRO_W2_5;
extern std::int16_t SPR_PRO_W2_6;
extern std::int16_t SPR_PRO_W2_7;
extern std::int16_t SPR_PRO_W2_8;

extern std::int16_t SPR_PRO_W3_1;
extern std::int16_t SPR_PRO_W3_2;
extern std::int16_t SPR_PRO_W3_3;
extern std::int16_t SPR_PRO_W3_4;
extern std::int16_t SPR_PRO_W3_5;
extern std::int16_t SPR_PRO_W3_6;
extern std::int16_t SPR_PRO_W3_7;
extern std::int16_t SPR_PRO_W3_8;

extern std::int16_t SPR_PRO_W4_1;
extern std::int16_t SPR_PRO_W4_2;
extern std::int16_t SPR_PRO_W4_3;
extern std::int16_t SPR_PRO_W4_4;
extern std::int16_t SPR_PRO_W4_5;
extern std::int16_t SPR_PRO_W4_6;
extern std::int16_t SPR_PRO_W4_7;
extern std::int16_t SPR_PRO_W4_8;

extern std::int16_t SPR_PRO_PAIN_1;
extern std::int16_t SPR_PRO_DIE_1;
extern std::int16_t SPR_PRO_DIE_2;
extern std::int16_t SPR_PRO_DIE_3;
extern std::int16_t SPR_PRO_PAIN_2;
extern std::int16_t SPR_PRO_DIE_4;
extern std::int16_t SPR_PRO_DEAD;

extern std::int16_t SPR_PRO_SHOOT1;
extern std::int16_t SPR_PRO_SHOOT2;
extern std::int16_t SPR_PRO_SHOOT3;

//
// Star Trooper (AOG) / SWAT (PS)
//
extern std::int16_t SPR_SWAT_S_1;
extern std::int16_t SPR_SWAT_S_2;
extern std::int16_t SPR_SWAT_S_3;
extern std::int16_t SPR_SWAT_S_4;
extern std::int16_t SPR_SWAT_S_5;
extern std::int16_t SPR_SWAT_S_6;
extern std::int16_t SPR_SWAT_S_7;
extern std::int16_t SPR_SWAT_S_8;

extern std::int16_t SPR_SWAT_W1_1;
extern std::int16_t SPR_SWAT_W1_2;
extern std::int16_t SPR_SWAT_W1_3;
extern std::int16_t SPR_SWAT_W1_4;
extern std::int16_t SPR_SWAT_W1_5;
extern std::int16_t SPR_SWAT_W1_6;
extern std::int16_t SPR_SWAT_W1_7;
extern std::int16_t SPR_SWAT_W1_8;

extern std::int16_t SPR_SWAT_W2_1;
extern std::int16_t SPR_SWAT_W2_2;
extern std::int16_t SPR_SWAT_W2_3;
extern std::int16_t SPR_SWAT_W2_4;
extern std::int16_t SPR_SWAT_W2_5;
extern std::int16_t SPR_SWAT_W2_6;
extern std::int16_t SPR_SWAT_W2_7;
extern std::int16_t SPR_SWAT_W2_8;

extern std::int16_t SPR_SWAT_W3_1;
extern std::int16_t SPR_SWAT_W3_2;
extern std::int16_t SPR_SWAT_W3_3;
extern std::int16_t SPR_SWAT_W3_4;
extern std::int16_t SPR_SWAT_W3_5;
extern std::int16_t SPR_SWAT_W3_6;
extern std::int16_t SPR_SWAT_W3_7;
extern std::int16_t SPR_SWAT_W3_8;

extern std::int16_t SPR_SWAT_W4_1;
extern std::int16_t SPR_SWAT_W4_2;
extern std::int16_t SPR_SWAT_W4_3;
extern std::int16_t SPR_SWAT_W4_4;
extern std::int16_t SPR_SWAT_W4_5;
extern std::int16_t SPR_SWAT_W4_6;
extern std::int16_t SPR_SWAT_W4_7;
extern std::int16_t SPR_SWAT_W4_8;

extern std::int16_t SPR_SWAT_PAIN_1;
extern std::int16_t SPR_SWAT_DIE_1;
extern std::int16_t SPR_SWAT_DIE_2;
extern std::int16_t SPR_SWAT_DIE_3;
extern std::int16_t SPR_SWAT_PAIN_2;
extern std::int16_t SPR_SWAT_DIE_4;
extern std::int16_t SPR_SWAT_DEAD;

extern std::int16_t SPR_SWAT_SHOOT1;
extern std::int16_t SPR_SWAT_SHOOT2;
extern std::int16_t SPR_SWAT_SHOOT3;

extern std::int16_t SPR_SWAT_WOUNDED1;
extern std::int16_t SPR_SWAT_WOUNDED2;
extern std::int16_t SPR_SWAT_WOUNDED3;
extern std::int16_t SPR_SWAT_WOUNDED4;

//
// General Scientist
//
extern std::int16_t SPR_OFC_S_1;
extern std::int16_t SPR_OFC_S_2;
extern std::int16_t SPR_OFC_S_3;
extern std::int16_t SPR_OFC_S_4;
extern std::int16_t SPR_OFC_S_5;
extern std::int16_t SPR_OFC_S_6;
extern std::int16_t SPR_OFC_S_7;
extern std::int16_t SPR_OFC_S_8;

extern std::int16_t SPR_OFC_W1_1;
extern std::int16_t SPR_OFC_W1_2;
extern std::int16_t SPR_OFC_W1_3;
extern std::int16_t SPR_OFC_W1_4;
extern std::int16_t SPR_OFC_W1_5;
extern std::int16_t SPR_OFC_W1_6;
extern std::int16_t SPR_OFC_W1_7;
extern std::int16_t SPR_OFC_W1_8;

extern std::int16_t SPR_OFC_W2_1;
extern std::int16_t SPR_OFC_W2_2;
extern std::int16_t SPR_OFC_W2_3;
extern std::int16_t SPR_OFC_W2_4;
extern std::int16_t SPR_OFC_W2_5;
extern std::int16_t SPR_OFC_W2_6;
extern std::int16_t SPR_OFC_W2_7;
extern std::int16_t SPR_OFC_W2_8;

extern std::int16_t SPR_OFC_W3_1;
extern std::int16_t SPR_OFC_W3_2;
extern std::int16_t SPR_OFC_W3_3;
extern std::int16_t SPR_OFC_W3_4;
extern std::int16_t SPR_OFC_W3_5;
extern std::int16_t SPR_OFC_W3_6;
extern std::int16_t SPR_OFC_W3_7;
extern std::int16_t SPR_OFC_W3_8;

extern std::int16_t SPR_OFC_W4_1;
extern std::int16_t SPR_OFC_W4_2;
extern std::int16_t SPR_OFC_W4_3;
extern std::int16_t SPR_OFC_W4_4;
extern std::int16_t SPR_OFC_W4_5;
extern std::int16_t SPR_OFC_W4_6;
extern std::int16_t SPR_OFC_W4_7;
extern std::int16_t SPR_OFC_W4_8;

extern std::int16_t SPR_OFC_PAIN_1;
extern std::int16_t SPR_OFC_DIE_1;
extern std::int16_t SPR_OFC_DIE_2;
extern std::int16_t SPR_OFC_DIE_3;
extern std::int16_t SPR_OFC_PAIN_2;
extern std::int16_t SPR_OFC_DIE_4;
extern std::int16_t SPR_OFC_DEAD;

extern std::int16_t SPR_OFC_SHOOT1;
extern std::int16_t SPR_OFC_SHOOT2;
extern std::int16_t SPR_OFC_SHOOT3;

//
// Bad Boy Dr. Goldstern
//
extern std::int16_t SPR_GOLD_S_1;
extern std::int16_t SPR_GOLD_S_2;
extern std::int16_t SPR_GOLD_S_3;
extern std::int16_t SPR_GOLD_S_4;
extern std::int16_t SPR_GOLD_S_5;
extern std::int16_t SPR_GOLD_S_6;
extern std::int16_t SPR_GOLD_S_7;
extern std::int16_t SPR_GOLD_S_8;

extern std::int16_t SPR_GOLD_W1_1;
extern std::int16_t SPR_GOLD_W1_2;
extern std::int16_t SPR_GOLD_W1_3;
extern std::int16_t SPR_GOLD_W1_4;
extern std::int16_t SPR_GOLD_W1_5;
extern std::int16_t SPR_GOLD_W1_6;
extern std::int16_t SPR_GOLD_W1_7;
extern std::int16_t SPR_GOLD_W1_8;

extern std::int16_t SPR_GOLD_W2_1;
extern std::int16_t SPR_GOLD_W2_2;
extern std::int16_t SPR_GOLD_W2_3;
extern std::int16_t SPR_GOLD_W2_4;
extern std::int16_t SPR_GOLD_W2_5;
extern std::int16_t SPR_GOLD_W2_6;
extern std::int16_t SPR_GOLD_W2_7;
extern std::int16_t SPR_GOLD_W2_8;

extern std::int16_t SPR_GOLD_W3_1;
extern std::int16_t SPR_GOLD_W3_2;
extern std::int16_t SPR_GOLD_W3_3;
extern std::int16_t SPR_GOLD_W3_4;
extern std::int16_t SPR_GOLD_W3_5;
extern std::int16_t SPR_GOLD_W3_6;
extern std::int16_t SPR_GOLD_W3_7;
extern std::int16_t SPR_GOLD_W3_8;

extern std::int16_t SPR_GOLD_W4_1;
extern std::int16_t SPR_GOLD_W4_2;
extern std::int16_t SPR_GOLD_W4_3;
extern std::int16_t SPR_GOLD_W4_4;
extern std::int16_t SPR_GOLD_W4_5;
extern std::int16_t SPR_GOLD_W4_6;
extern std::int16_t SPR_GOLD_W4_7;
extern std::int16_t SPR_GOLD_W4_8;

extern std::int16_t SPR_GOLD_PAIN_1;

extern std::int16_t SPR_GOLD_WRIST_1;
extern std::int16_t SPR_GOLD_WRIST_2;

extern std::int16_t SPR_GOLD_SHOOT1;
extern std::int16_t SPR_GOLD_SHOOT2;
extern std::int16_t SPR_GOLD_SHOOT3;

extern std::int16_t SPR_GOLD_WARP1;
extern std::int16_t SPR_GOLD_WARP2;
extern std::int16_t SPR_GOLD_WARP3;
extern std::int16_t SPR_GOLD_WARP4;
extern std::int16_t SPR_GOLD_WARP5;

extern std::int16_t SPR_GOLD_DEATH1;
extern std::int16_t SPR_GOLD_DEATH2;
extern std::int16_t SPR_GOLD_DEATH3;
extern std::int16_t SPR_GOLD_DEATH4;
extern std::int16_t SPR_GOLD_DEATH5;
extern std::int16_t SPR_MGOLD_OUCH;

extern std::int16_t SPR_GOLD_MORPH1;
extern std::int16_t SPR_GOLD_MORPH2;
extern std::int16_t SPR_GOLD_MORPH3;
extern std::int16_t SPR_GOLD_MORPH4;
extern std::int16_t SPR_GOLD_MORPH5;
extern std::int16_t SPR_GOLD_MORPH6;
extern std::int16_t SPR_GOLD_MORPH7;
extern std::int16_t SPR_GOLD_MORPH8;

extern std::int16_t SPR_MGOLD_WALK1;
extern std::int16_t SPR_MGOLD_WALK2;
extern std::int16_t SPR_MGOLD_WALK3;
extern std::int16_t SPR_MGOLD_WALK4;
extern std::int16_t SPR_MGOLD_ATTACK1;
extern std::int16_t SPR_MGOLD_ATTACK2;
extern std::int16_t SPR_MGOLD_ATTACK3;
extern std::int16_t SPR_MGOLD_ATTACK4;

extern std::int16_t SPR_MGOLD_SHOT1;
extern std::int16_t SPR_MGOLD_SHOT2;
extern std::int16_t SPR_MGOLD_SHOT3;
extern std::int16_t SPR_MGOLD_SHOT_EXP1;
extern std::int16_t SPR_MGOLD_SHOT_EXP2;
extern std::int16_t SPR_MGOLD_SHOT_EXP3;

//
// Volatile Material Transporter
//
extern std::int16_t SPR_GSCOUT_W1_1;
extern std::int16_t SPR_GSCOUT_W1_2;
extern std::int16_t SPR_GSCOUT_W1_3;
extern std::int16_t SPR_GSCOUT_W1_4;
extern std::int16_t SPR_GSCOUT_W1_5;
extern std::int16_t SPR_GSCOUT_W1_6;
extern std::int16_t SPR_GSCOUT_W1_7;
extern std::int16_t SPR_GSCOUT_W1_8;

extern std::int16_t SPR_GSCOUT_W2_1;
extern std::int16_t SPR_GSCOUT_W2_2;
extern std::int16_t SPR_GSCOUT_W2_3;
extern std::int16_t SPR_GSCOUT_W2_4;
extern std::int16_t SPR_GSCOUT_W2_5;
extern std::int16_t SPR_GSCOUT_W2_6;
extern std::int16_t SPR_GSCOUT_W2_7;
extern std::int16_t SPR_GSCOUT_W2_8;

extern std::int16_t SPR_GSCOUT_W3_1;
extern std::int16_t SPR_GSCOUT_W3_2;
extern std::int16_t SPR_GSCOUT_W3_3;
extern std::int16_t SPR_GSCOUT_W3_4;
extern std::int16_t SPR_GSCOUT_W3_5;
extern std::int16_t SPR_GSCOUT_W3_6;
extern std::int16_t SPR_GSCOUT_W3_7;
extern std::int16_t SPR_GSCOUT_W3_8;

extern std::int16_t SPR_GSCOUT_W4_1;
extern std::int16_t SPR_GSCOUT_W4_2;
extern std::int16_t SPR_GSCOUT_W4_3;
extern std::int16_t SPR_GSCOUT_W4_4;
extern std::int16_t SPR_GSCOUT_W4_5;
extern std::int16_t SPR_GSCOUT_W4_6;
extern std::int16_t SPR_GSCOUT_W4_7;
extern std::int16_t SPR_GSCOUT_W4_8;

extern std::int16_t SPR_GSCOUT_DIE1;
extern std::int16_t SPR_GSCOUT_DIE2;
extern std::int16_t SPR_GSCOUT_DIE3;
extern std::int16_t SPR_GSCOUT_DIE4;
extern std::int16_t SPR_GSCOUT_DIE5;
extern std::int16_t SPR_GSCOUT_DIE6;
extern std::int16_t SPR_GSCOUT_DIE7;
extern std::int16_t SPR_GSCOUT_DIE8;

extern std::int16_t SPR_GSCOUT_DEAD;

//
// Perscan Drone
//
extern std::int16_t SPR_FSCOUT_W1_1;
extern std::int16_t SPR_FSCOUT_W1_2;
extern std::int16_t SPR_FSCOUT_W1_3;
extern std::int16_t SPR_FSCOUT_W1_4;
extern std::int16_t SPR_FSCOUT_W1_5;
extern std::int16_t SPR_FSCOUT_W1_6;
extern std::int16_t SPR_FSCOUT_W1_7;
extern std::int16_t SPR_FSCOUT_W1_8;

extern std::int16_t SPR_FSCOUT_W2_1;
extern std::int16_t SPR_FSCOUT_W2_2;
extern std::int16_t SPR_FSCOUT_W2_3;
extern std::int16_t SPR_FSCOUT_W2_4;
extern std::int16_t SPR_FSCOUT_W2_5;
extern std::int16_t SPR_FSCOUT_W2_6;
extern std::int16_t SPR_FSCOUT_W2_7;
extern std::int16_t SPR_FSCOUT_W2_8;

extern std::int16_t SPR_FSCOUT_W3_1;
extern std::int16_t SPR_FSCOUT_W3_2;
extern std::int16_t SPR_FSCOUT_W3_3;
extern std::int16_t SPR_FSCOUT_W3_4;
extern std::int16_t SPR_FSCOUT_W3_5;
extern std::int16_t SPR_FSCOUT_W3_6;
extern std::int16_t SPR_FSCOUT_W3_7;
extern std::int16_t SPR_FSCOUT_W3_8;

extern std::int16_t SPR_FSCOUT_W4_1;
extern std::int16_t SPR_FSCOUT_W4_2;
extern std::int16_t SPR_FSCOUT_W4_3;
extern std::int16_t SPR_FSCOUT_W4_4;
extern std::int16_t SPR_FSCOUT_W4_5;
extern std::int16_t SPR_FSCOUT_W4_6;
extern std::int16_t SPR_FSCOUT_W4_7;
extern std::int16_t SPR_FSCOUT_W4_8;

extern std::int16_t SPR_FSCOUT_DIE1;
extern std::int16_t SPR_FSCOUT_DIE2;
extern std::int16_t SPR_FSCOUT_DIE3;
extern std::int16_t SPR_FSCOUT_DIE4;
extern std::int16_t SPR_FSCOUT_DIE5;
extern std::int16_t SPR_FSCOUT_DIE6;
extern std::int16_t SPR_FSCOUT_DIE7;
extern std::int16_t SPR_FSCOUT_DEAD;

//
// General Explosion Anim
//
extern std::int16_t SPR_EXPLOSION_1;
extern std::int16_t SPR_EXPLOSION_2;
extern std::int16_t SPR_EXPLOSION_3;
extern std::int16_t SPR_EXPLOSION_4;
extern std::int16_t SPR_EXPLOSION_5;

//
// Projection Generator
//
extern std::int16_t SPR_VITAL_STAND;
extern std::int16_t SPR_VITAL_DIE_1;
extern std::int16_t SPR_VITAL_DIE_2;
extern std::int16_t SPR_VITAL_DIE_3;
extern std::int16_t SPR_VITAL_DIE_4;
extern std::int16_t SPR_VITAL_DIE_5;
extern std::int16_t SPR_VITAL_DIE_6;
extern std::int16_t SPR_VITAL_DIE_7;
extern std::int16_t SPR_VITAL_DIE_8;
extern std::int16_t SPR_VITAL_DEAD_1;
extern std::int16_t SPR_VITAL_DEAD_2;
extern std::int16_t SPR_VITAL_DEAD_3;
extern std::int16_t SPR_VITAL_OUCH;

//
// Security Cube
//
extern std::int16_t SPR_CUBE1;
extern std::int16_t SPR_CUBE2;
extern std::int16_t SPR_CUBE3;
extern std::int16_t SPR_CUBE4;
extern std::int16_t SPR_CUBE5;
extern std::int16_t SPR_CUBE6;
extern std::int16_t SPR_CUBE7;
extern std::int16_t SPR_CUBE8;
extern std::int16_t SPR_CUBE9;
extern std::int16_t SPR_CUBE10;
extern std::int16_t SPR_CUBE_EXP1;
extern std::int16_t SPR_CUBE_EXP2;
extern std::int16_t SPR_CUBE_EXP3;
extern std::int16_t SPR_CUBE_EXP4;
extern std::int16_t SPR_CUBE_EXP5;
extern std::int16_t SPR_CUBE_EXP6;
extern std::int16_t SPR_CUBE_EXP7;
extern std::int16_t SPR_CUBE_EXP8;
extern std::int16_t SPR_DEAD_CUBE;

//
// Red Security Lights
//
extern std::int16_t SPR_SECURITY_NORMAL;
extern std::int16_t SPR_SECURITY_ALERT;

//
// P.O.D. Alien
//
extern std::int16_t SPR_POD_EGG;
extern std::int16_t SPR_POD_HATCH1;
extern std::int16_t SPR_POD_HATCH2;
extern std::int16_t SPR_POD_HATCH3;
extern std::int16_t SPR_POD_WALK1;
extern std::int16_t SPR_POD_WALK2;
extern std::int16_t SPR_POD_WALK3;
extern std::int16_t SPR_POD_WALK4;
extern std::int16_t SPR_POD_ATTACK1;
extern std::int16_t SPR_POD_ATTACK2;
extern std::int16_t SPR_POD_ATTACK3;
extern std::int16_t SPR_POD_OUCH;
extern std::int16_t SPR_POD_DIE1;
extern std::int16_t SPR_POD_DIE2;
extern std::int16_t SPR_POD_DIE3;
extern std::int16_t SPR_POD_SPIT1;
extern std::int16_t SPR_POD_SPIT2;
extern std::int16_t SPR_POD_SPIT3;

//
// Electro-Alien
//
extern std::int16_t SPR_ELEC_APPEAR1;
extern std::int16_t SPR_ELEC_APPEAR2;
extern std::int16_t SPR_ELEC_APPEAR3;
extern std::int16_t SPR_ELEC_WALK1;
extern std::int16_t SPR_ELEC_WALK2;
extern std::int16_t SPR_ELEC_WALK3;
extern std::int16_t SPR_ELEC_WALK4;
extern std::int16_t SPR_ELEC_OUCH;
extern std::int16_t SPR_ELEC_SHOOT1;
extern std::int16_t SPR_ELEC_SHOOT2;
extern std::int16_t SPR_ELEC_SHOOT3;
extern std::int16_t SPR_ELEC_DIE1;
extern std::int16_t SPR_ELEC_DIE2;
extern std::int16_t SPR_ELEC_DIE3;
extern std::int16_t SPR_ELEC_SHOT1;
extern std::int16_t SPR_ELEC_SHOT2;
extern std::int16_t SPR_ELEC_SHOT_EXP1;
extern std::int16_t SPR_ELEC_SHOT_EXP2;

//
// ElectroSphere
//
extern std::int16_t SPR_ELECTRO_SPHERE_ROAM1;
extern std::int16_t SPR_ELECTRO_SPHERE_ROAM2;
extern std::int16_t SPR_ELECTRO_SPHERE_ROAM3;
extern std::int16_t SPR_ELECTRO_SPHERE_OUCH;
extern std::int16_t SPR_ELECTRO_SPHERE_DIE1;
extern std::int16_t SPR_ELECTRO_SPHERE_DIE2;
extern std::int16_t SPR_ELECTRO_SPHERE_DIE3;
extern std::int16_t SPR_ELECTRO_SPHERE_DIE4;

//
// Genetic Guard
//
extern std::int16_t SPR_GENETIC_W1;
extern std::int16_t SPR_GENETIC_W2;
extern std::int16_t SPR_GENETIC_W3;
extern std::int16_t SPR_GENETIC_W4;
extern std::int16_t SPR_GENETIC_SWING1;
extern std::int16_t SPR_GENETIC_SWING2;
extern std::int16_t SPR_GENETIC_SWING3;
extern std::int16_t SPR_GENETIC_DEAD;
extern std::int16_t SPR_GENETIC_DIE1;
extern std::int16_t SPR_GENETIC_DIE2;
extern std::int16_t SPR_GENETIC_DIE3;
extern std::int16_t SPR_GENETIC_DIE4;
extern std::int16_t SPR_GENETIC_OUCH;
extern std::int16_t SPR_GENETIC_SHOOT1;
extern std::int16_t SPR_GENETIC_SHOOT2;
extern std::int16_t SPR_GENETIC_SHOOT3;

//
// Experimental Mech-Sentinel
//
extern std::int16_t SPR_MUTHUM1_W1;
extern std::int16_t SPR_MUTHUM1_W2;
extern std::int16_t SPR_MUTHUM1_W3;
extern std::int16_t SPR_MUTHUM1_W4;
extern std::int16_t SPR_MUTHUM1_SWING1;
extern std::int16_t SPR_MUTHUM1_SWING2;
extern std::int16_t SPR_MUTHUM1_SWING3;
extern std::int16_t SPR_MUTHUM1_DEAD;
extern std::int16_t SPR_MUTHUM1_DIE1;
extern std::int16_t SPR_MUTHUM1_DIE2;
extern std::int16_t SPR_MUTHUM1_DIE3;
extern std::int16_t SPR_MUTHUM1_DIE4;
extern std::int16_t SPR_MUTHUM1_OUCH;
extern std::int16_t SPR_MUTHUM1_SPIT1;
extern std::int16_t SPR_MUTHUM1_SPIT2;
extern std::int16_t SPR_MUTHUM1_SPIT3;

//
// Muntant human type 2
//
extern std::int16_t SPR_MUTHUM2_W1;
extern std::int16_t SPR_MUTHUM2_W2;
extern std::int16_t SPR_MUTHUM2_W3;
extern std::int16_t SPR_MUTHUM2_W4;
extern std::int16_t SPR_MUTHUM2_SWING1;
extern std::int16_t SPR_MUTHUM2_SWING2;
extern std::int16_t SPR_MUTHUM2_SWING3;
extern std::int16_t SPR_MUTHUM2_DEAD;
extern std::int16_t SPR_MUTHUM2_DIE1;
extern std::int16_t SPR_MUTHUM2_DIE2;
extern std::int16_t SPR_MUTHUM2_DIE3;
extern std::int16_t SPR_MUTHUM2_DIE4;
extern std::int16_t SPR_MUTHUM2_OUCH;
extern std::int16_t SPR_MUTHUM2_SPIT1;
extern std::int16_t SPR_MUTHUM2_SPIT2;
extern std::int16_t SPR_MUTHUM2_SPIT3;

extern std::int16_t SPR_MUTHUM2_MORPH1;
extern std::int16_t SPR_MUTHUM2_MORPH2;
extern std::int16_t SPR_MUTHUM2_MORPH3;
extern std::int16_t SPR_MUTHUM2_MORPH4;
extern std::int16_t SPR_MUTHUM2_MORPH5;
extern std::int16_t SPR_MUTHUM2_MORPH6;
extern std::int16_t SPR_MUTHUM2_MORPH7;
extern std::int16_t SPR_MUTHUM2_MORPH8;
extern std::int16_t SPR_MUTHUM2_MORPH9;

//
// Large Cantained Alien
//
extern std::int16_t SPR_LCAN_ALIEN_READY;
extern std::int16_t SPR_LCAN_ALIEN_B1;
extern std::int16_t SPR_LCAN_ALIEN_B2;
extern std::int16_t SPR_LCAN_ALIEN_B3;
extern std::int16_t SPR_LCAN_ALIEN_EMPTY;

extern std::int16_t SPR_LCAN_ALIEN_W1;
extern std::int16_t SPR_LCAN_ALIEN_W2;
extern std::int16_t SPR_LCAN_ALIEN_W3;
extern std::int16_t SPR_LCAN_ALIEN_W4;
extern std::int16_t SPR_LCAN_ALIEN_SWING1;
extern std::int16_t SPR_LCAN_ALIEN_SWING2;
extern std::int16_t SPR_LCAN_ALIEN_SWING3;
extern std::int16_t SPR_LCAN_ALIEN_DEAD;
extern std::int16_t SPR_LCAN_ALIEN_DIE1;
extern std::int16_t SPR_LCAN_ALIEN_DIE2;
extern std::int16_t SPR_LCAN_ALIEN_DIE3;
extern std::int16_t SPR_LCAN_ALIEN_DIE4;
extern std::int16_t SPR_LCAN_ALIEN_OUCH;
extern std::int16_t SPR_LCAN_ALIEN_SPIT1;
extern std::int16_t SPR_LCAN_ALIEN_SPIT2;
extern std::int16_t SPR_LCAN_ALIEN_SPIT3;

//
// Small Canister Alien
//
extern std::int16_t SPR_SCAN_ALIEN_READY;
extern std::int16_t SPR_SCAN_ALIEN_B1;
extern std::int16_t SPR_SCAN_ALIEN_B2;
extern std::int16_t SPR_SCAN_ALIEN_B3;
extern std::int16_t SPR_SCAN_ALIEN_EMPTY;

extern std::int16_t SPR_SCAN_ALIEN_W1;
extern std::int16_t SPR_SCAN_ALIEN_W2;
extern std::int16_t SPR_SCAN_ALIEN_W3;
extern std::int16_t SPR_SCAN_ALIEN_W4;
extern std::int16_t SPR_SCAN_ALIEN_SWING1;
extern std::int16_t SPR_SCAN_ALIEN_SWING2;
extern std::int16_t SPR_SCAN_ALIEN_SWING3;
extern std::int16_t SPR_SCAN_ALIEN_DEAD;
extern std::int16_t SPR_SCAN_ALIEN_DIE1;
extern std::int16_t SPR_SCAN_ALIEN_DIE2;
extern std::int16_t SPR_SCAN_ALIEN_DIE3;
extern std::int16_t SPR_SCAN_ALIEN_DIE4;
extern std::int16_t SPR_SCAN_ALIEN_OUCH;

extern std::int16_t SPR_SCAN_ALIEN_SPIT1;
extern std::int16_t SPR_SCAN_ALIEN_SPIT2;
extern std::int16_t SPR_SCAN_ALIEN_SPIT3;

//
// Gurney Mutant
//
extern std::int16_t SPR_GURNEY_MUT_READY;
extern std::int16_t SPR_GURNEY_MUT_B1;
extern std::int16_t SPR_GURNEY_MUT_B2;
extern std::int16_t SPR_GURNEY_MUT_B3;
extern std::int16_t SPR_GURNEY_MUT_EMPTY;

extern std::int16_t SPR_GURNEY_MUT_W1;
extern std::int16_t SPR_GURNEY_MUT_W2;
extern std::int16_t SPR_GURNEY_MUT_W3;
extern std::int16_t SPR_GURNEY_MUT_W4;
extern std::int16_t SPR_GURNEY_MUT_SWING1;
extern std::int16_t SPR_GURNEY_MUT_SWING2;
extern std::int16_t SPR_GURNEY_MUT_SWING3;
extern std::int16_t SPR_GURNEY_MUT_DEAD;
extern std::int16_t SPR_GURNEY_MUT_DIE1;
extern std::int16_t SPR_GURNEY_MUT_DIE2;
extern std::int16_t SPR_GURNEY_MUT_DIE3;
extern std::int16_t SPR_GURNEY_MUT_DIE4;
extern std::int16_t SPR_GURNEY_MUT_OUCH;

//
// Liquid Alien
//
extern std::int16_t SPR_LIQUID_M1;
extern std::int16_t SPR_LIQUID_M2;
extern std::int16_t SPR_LIQUID_M3;
extern std::int16_t SPR_LIQUID_R1;
extern std::int16_t SPR_LIQUID_R2;
extern std::int16_t SPR_LIQUID_R3;
extern std::int16_t SPR_LIQUID_R4;
extern std::int16_t SPR_LIQUID_S1;
extern std::int16_t SPR_LIQUID_S2;
extern std::int16_t SPR_LIQUID_S3;
extern std::int16_t SPR_LIQUID_OUCH;
extern std::int16_t SPR_LIQUID_DIE_1;
extern std::int16_t SPR_LIQUID_DIE_2;
extern std::int16_t SPR_LIQUID_DIE_3;
extern std::int16_t SPR_LIQUID_DIE_4;
extern std::int16_t SPR_LIQUID_DEAD;
extern std::int16_t SPR_LIQUID_SHOT_FLY_1;
extern std::int16_t SPR_LIQUID_SHOT_FLY_2;
extern std::int16_t SPR_LIQUID_SHOT_FLY_3;
extern std::int16_t SPR_LIQUID_SHOT_BURST_1;
extern std::int16_t SPR_LIQUID_SHOT_BURST_2;
extern std::int16_t SPR_LIQUID_SHOT_BURST_3;

//
// Spit Shots
//
extern std::int16_t SPR_SPIT1_1;
extern std::int16_t SPR_SPIT1_2;
extern std::int16_t SPR_SPIT1_3;
extern std::int16_t SPR_SPIT_EXP1_1;
extern std::int16_t SPR_SPIT_EXP1_2;
extern std::int16_t SPR_SPIT_EXP1_3;

extern std::int16_t SPR_SPIT2_1;
extern std::int16_t SPR_SPIT2_2;
extern std::int16_t SPR_SPIT2_3;
extern std::int16_t SPR_SPIT_EXP2_1;
extern std::int16_t SPR_SPIT_EXP2_2;
extern std::int16_t SPR_SPIT_EXP2_3;

extern std::int16_t SPR_SPIT3_1;
extern std::int16_t SPR_SPIT3_2;
extern std::int16_t SPR_SPIT3_3;
extern std::int16_t SPR_SPIT_EXP3_1;
extern std::int16_t SPR_SPIT_EXP3_2;
extern std::int16_t SPR_SPIT_EXP3_3;

//
// Hanging Turret
//
extern std::int16_t SPR_TERROT_1;
extern std::int16_t SPR_TERROT_2;
extern std::int16_t SPR_TERROT_3;
extern std::int16_t SPR_TERROT_4;
extern std::int16_t SPR_TERROT_5;
extern std::int16_t SPR_TERROT_6;
extern std::int16_t SPR_TERROT_7;
extern std::int16_t SPR_TERROT_8;

extern std::int16_t SPR_TERROT_FIRE_1;
extern std::int16_t SPR_TERROT_FIRE_2;
extern std::int16_t SPR_TERROT_DIE_1;
extern std::int16_t SPR_TERROT_DIE_2;
extern std::int16_t SPR_TERROT_DIE_3;
extern std::int16_t SPR_TERROT_DIE_4;
extern std::int16_t SPR_TERROT_DEAD;

//
// Player attack frames
//
extern std::int16_t SPR_KNIFEREADY;
extern std::int16_t SPR_KNIFEATK1;
extern std::int16_t SPR_KNIFEATK2;
extern std::int16_t SPR_KNIFEATK3;
extern std::int16_t SPR_KNIFEATK4;

extern std::int16_t SPR_PISTOLREADY;
extern std::int16_t SPR_PISTOLATK1;
extern std::int16_t SPR_PISTOLATK2;
extern std::int16_t SPR_PISTOLATK3;
extern std::int16_t SPR_PISTOLATK4;

extern std::int16_t SPR_MACHINEGUNREADY;
extern std::int16_t SPR_MACHINEGUNATK1;
extern std::int16_t SPR_MACHINEGUNATK2;
extern std::int16_t SPR_MACHINEGUNATK3;
extern std::int16_t SPR_MACHINEGUNATK4;

extern std::int16_t SPR_CHAINREADY;
extern std::int16_t SPR_CHAINATK1;
extern std::int16_t SPR_CHAINATK2;
extern std::int16_t SPR_CHAINATK3;
extern std::int16_t SPR_CHAINATK4;

extern std::int16_t SPR_GRENADEREADY;
extern std::int16_t SPR_GRENADEATK1;
extern std::int16_t SPR_GRENADEATK2;
extern std::int16_t SPR_GRENADEATK3;
extern std::int16_t SPR_GRENADEATK4;

extern std::int16_t SPR_GRENADE_FLY1;
extern std::int16_t SPR_GRENADE_FLY2;
extern std::int16_t SPR_GRENADE_FLY3;
extern std::int16_t SPR_GRENADE_FLY4;
extern std::int16_t SPR_GRENADE_EXPLODE1;
extern std::int16_t SPR_GRENADE_EXPLODE2;
extern std::int16_t SPR_GRENADE_EXPLODE3;
extern std::int16_t SPR_GRENADE_EXPLODE4;
extern std::int16_t SPR_GRENADE_EXPLODE5;

extern std::int16_t SPR_ELEC_ARC1;
extern std::int16_t SPR_ELEC_ARC2;
extern std::int16_t SPR_ELEC_ARC3;
extern std::int16_t SPR_ELEC_ARC4;
extern std::int16_t SPR_ELEC_POST1;
extern std::int16_t SPR_ELEC_POST2;
extern std::int16_t SPR_ELEC_POST3;
extern std::int16_t SPR_ELEC_POST4;

extern std::int16_t SPR_VPOST1;
extern std::int16_t SPR_VPOST2;
extern std::int16_t SPR_VPOST3;
extern std::int16_t SPR_VPOST4;
extern std::int16_t SPR_VPOST5;
extern std::int16_t SPR_VPOST6;
extern std::int16_t SPR_VPOST7;
extern std::int16_t SPR_VPOST8;

extern std::int16_t SPR_VSPIKE1;
extern std::int16_t SPR_VSPIKE2;
extern std::int16_t SPR_VSPIKE3;
extern std::int16_t SPR_VSPIKE4;
extern std::int16_t SPR_VSPIKE5;
extern std::int16_t SPR_VSPIKE6;
extern std::int16_t SPR_VSPIKE7;
extern std::int16_t SPR_VSPIKE8;

extern std::int16_t SPR_GREEN_OOZE1;
extern std::int16_t SPR_GREEN_OOZE2;
extern std::int16_t SPR_GREEN_OOZE3;
extern std::int16_t SPR_BLACK_OOZE1;
extern std::int16_t SPR_BLACK_OOZE2;
extern std::int16_t SPR_BLACK_OOZE3;

extern std::int16_t SPR_GREEN2_OOZE1;
extern std::int16_t SPR_GREEN2_OOZE2;
extern std::int16_t SPR_GREEN2_OOZE3;
extern std::int16_t SPR_BLACK2_OOZE1;
extern std::int16_t SPR_BLACK2_OOZE2;
extern std::int16_t SPR_BLACK2_OOZE3;

extern std::int16_t SPR_CANDY_BAR;
extern std::int16_t SPR_CANDY_WRAPER;
extern std::int16_t SPR_SANDWICH;
extern std::int16_t SPR_SANDWICH_WRAPER;

extern std::int16_t SPR_BLAKE_W1;
extern std::int16_t SPR_BLAKE_W2;
extern std::int16_t SPR_BLAKE_W3;
extern std::int16_t SPR_BLAKE_W4;

//
// Boss 1 (Spider Mutant)
//
extern std::int16_t SPR_BOSS1_W1;
extern std::int16_t SPR_BOSS1_W2;
extern std::int16_t SPR_BOSS1_W3;
extern std::int16_t SPR_BOSS1_W4;
extern std::int16_t SPR_BOSS1_SWING1;
extern std::int16_t SPR_BOSS1_SWING2;
extern std::int16_t SPR_BOSS1_SWING3;
extern std::int16_t SPR_BOSS1_DEAD;
extern std::int16_t SPR_BOSS1_DIE1;
extern std::int16_t SPR_BOSS1_DIE2;
extern std::int16_t SPR_BOSS1_DIE3;
extern std::int16_t SPR_BOSS1_DIE4;
extern std::int16_t SPR_BOSS1_OUCH;
extern std::int16_t SPR_BOSS1_PROJ1;
extern std::int16_t SPR_BOSS1_PROJ2;
extern std::int16_t SPR_BOSS1_PROJ3;
extern std::int16_t SPR_BOSS1_EXP1;
extern std::int16_t SPR_BOSS1_EXP2;
extern std::int16_t SPR_BOSS1_EXP3;
extern std::int16_t SPR_BOSS1_MORPH1;
extern std::int16_t SPR_BOSS1_MORPH2;
extern std::int16_t SPR_BOSS1_MORPH3;
extern std::int16_t SPR_BOSS1_MORPH4;
extern std::int16_t SPR_BOSS1_MORPH5;
extern std::int16_t SPR_BOSS1_MORPH6;
extern std::int16_t SPR_BOSS1_MORPH7;
extern std::int16_t SPR_BOSS1_MORPH8;
extern std::int16_t SPR_BOSS1_MORPH9;

//
// Boss 2 (Breather Beast)
//
extern std::int16_t SPR_BOSS2_W1;
extern std::int16_t SPR_BOSS2_W2;
extern std::int16_t SPR_BOSS2_W3;
extern std::int16_t SPR_BOSS2_W4;
extern std::int16_t SPR_BOSS2_SWING1;
extern std::int16_t SPR_BOSS2_SWING2;
extern std::int16_t SPR_BOSS2_SWING3;
extern std::int16_t SPR_BOSS2_DEAD;
extern std::int16_t SPR_BOSS2_DIE1;
extern std::int16_t SPR_BOSS2_DIE2;
extern std::int16_t SPR_BOSS2_DIE3;
extern std::int16_t SPR_BOSS2_DIE4;
extern std::int16_t SPR_BOSS2_OUCH;

//
// Boss 3 (Cyborg Warrior)
//
extern std::int16_t SPR_BOSS3_W1;
extern std::int16_t SPR_BOSS3_W2;
extern std::int16_t SPR_BOSS3_W3;
extern std::int16_t SPR_BOSS3_W4;
extern std::int16_t SPR_BOSS3_SWING1;
extern std::int16_t SPR_BOSS3_SWING2;
extern std::int16_t SPR_BOSS3_SWING3;
extern std::int16_t SPR_BOSS3_DEAD;
extern std::int16_t SPR_BOSS3_DIE1;
extern std::int16_t SPR_BOSS3_DIE2;
extern std::int16_t SPR_BOSS3_DIE3;
extern std::int16_t SPR_BOSS3_DIE4;
extern std::int16_t SPR_BOSS3_OUCH;

//
// Boss 4 (Reptilian Warrior)
//
extern std::int16_t SPR_BOSS4_W1;
extern std::int16_t SPR_BOSS4_W2;
extern std::int16_t SPR_BOSS4_W3;
extern std::int16_t SPR_BOSS4_W4;
extern std::int16_t SPR_BOSS4_SWING1;
extern std::int16_t SPR_BOSS4_SWING2;
extern std::int16_t SPR_BOSS4_SWING3;
extern std::int16_t SPR_BOSS4_DEAD;
extern std::int16_t SPR_BOSS4_DIE1;
extern std::int16_t SPR_BOSS4_DIE2;
extern std::int16_t SPR_BOSS4_DIE3;
extern std::int16_t SPR_BOSS4_DIE4;
extern std::int16_t SPR_BOSS4_OUCH;
extern std::int16_t SPR_BOSS4_MORPH1;
extern std::int16_t SPR_BOSS4_MORPH2;
extern std::int16_t SPR_BOSS4_MORPH3;
extern std::int16_t SPR_BOSS4_MORPH4;
extern std::int16_t SPR_BOSS4_MORPH5;
extern std::int16_t SPR_BOSS4_MORPH6;
extern std::int16_t SPR_BOSS4_MORPH7;
extern std::int16_t SPR_BOSS4_MORPH8;
extern std::int16_t SPR_BOSS4_MORPH9;

//
// Boss 5 (Acid Dragon)
//
extern std::int16_t SPR_BOSS5_W1;
extern std::int16_t SPR_BOSS5_W2;
extern std::int16_t SPR_BOSS5_W3;
extern std::int16_t SPR_BOSS5_W4;
extern std::int16_t SPR_BOSS5_SWING1;
extern std::int16_t SPR_BOSS5_SWING2;
extern std::int16_t SPR_BOSS5_SWING3;
extern std::int16_t SPR_BOSS5_DEAD;
extern std::int16_t SPR_BOSS5_DIE1;
extern std::int16_t SPR_BOSS5_DIE2;
extern std::int16_t SPR_BOSS5_DIE3;
extern std::int16_t SPR_BOSS5_DIE4;
extern std::int16_t SPR_BOSS5_OUCH;
extern std::int16_t SPR_BOSS5_PROJ1;
extern std::int16_t SPR_BOSS5_PROJ2;
extern std::int16_t SPR_BOSS5_PROJ3;
extern std::int16_t SPR_BOSS5_EXP1;
extern std::int16_t SPR_BOSS5_EXP2;
extern std::int16_t SPR_BOSS5_EXP3;

//
// Boss 6 (Bio-Mech Guardian)
//
extern std::int16_t SPR_BOSS6_W1;
extern std::int16_t SPR_BOSS6_W2;
extern std::int16_t SPR_BOSS6_W3;
extern std::int16_t SPR_BOSS6_W4;
extern std::int16_t SPR_BOSS6_SWING1;
extern std::int16_t SPR_BOSS6_SWING2;
extern std::int16_t SPR_BOSS6_SWING3;
extern std::int16_t SPR_BOSS6_DEAD;
extern std::int16_t SPR_BOSS6_DIE1;
extern std::int16_t SPR_BOSS6_DIE2;
extern std::int16_t SPR_BOSS6_DIE3;
extern std::int16_t SPR_BOSS6_DIE4;
extern std::int16_t SPR_BOSS6_OUCH;


//
// Boss 7
//
extern std::int16_t SPR_BOSS7_W1;
extern std::int16_t SPR_BOSS7_W2;
extern std::int16_t SPR_BOSS7_W3;
extern std::int16_t SPR_BOSS7_W4;
extern std::int16_t SPR_BOSS7_SHOOT1;
extern std::int16_t SPR_BOSS7_SHOOT2;
extern std::int16_t SPR_BOSS7_SHOOT3;
extern std::int16_t SPR_BOSS7_DEAD;
extern std::int16_t SPR_BOSS7_DIE1;
extern std::int16_t SPR_BOSS7_DIE2;
extern std::int16_t SPR_BOSS7_DIE3;
extern std::int16_t SPR_BOSS7_DIE4;
extern std::int16_t SPR_BOSS7_OUCH;

//
// Boss 8
//
extern std::int16_t SPR_BOSS8_W1;
extern std::int16_t SPR_BOSS8_W2;
extern std::int16_t SPR_BOSS8_W3;
extern std::int16_t SPR_BOSS8_W4;
extern std::int16_t SPR_BOSS8_SHOOT1;
extern std::int16_t SPR_BOSS8_SHOOT2;
extern std::int16_t SPR_BOSS8_SHOOT3;
extern std::int16_t SPR_BOSS8_DIE1;
extern std::int16_t SPR_BOSS8_DIE2;
extern std::int16_t SPR_BOSS8_DIE3;
extern std::int16_t SPR_BOSS8_DIE4;
extern std::int16_t SPR_BOSS8_DEAD;
extern std::int16_t SPR_BOSS8_OUCH;

//
// Boss 9
//
extern std::int16_t SPR_BOSS9_W1;
extern std::int16_t SPR_BOSS9_W2;
extern std::int16_t SPR_BOSS9_W3;
extern std::int16_t SPR_BOSS9_W4;
extern std::int16_t SPR_BOSS9_SHOOT1;
extern std::int16_t SPR_BOSS9_SHOOT2;
extern std::int16_t SPR_BOSS9_SHOOT3;
extern std::int16_t SPR_BOSS9_DIE1;
extern std::int16_t SPR_BOSS9_DIE2;
extern std::int16_t SPR_BOSS9_DIE3;
extern std::int16_t SPR_BOSS9_DIE4;
extern std::int16_t SPR_BOSS9_DEAD;
extern std::int16_t SPR_BOSS9_OUCH;

//
// Boss 10
//
extern std::int16_t SPR_BOSS10_W1;
extern std::int16_t SPR_BOSS10_W2;
extern std::int16_t SPR_BOSS10_W3;
extern std::int16_t SPR_BOSS10_W4;
extern std::int16_t SPR_BOSS10_SHOOT1;
extern std::int16_t SPR_BOSS10_SHOOT2;
extern std::int16_t SPR_BOSS10_SHOOT3;
extern std::int16_t SPR_BOSS10_DEAD;
extern std::int16_t SPR_BOSS10_DIE1;
extern std::int16_t SPR_BOSS10_DIE2;
extern std::int16_t SPR_BOSS10_DIE3;
extern std::int16_t SPR_BOSS10_DIE4;
extern std::int16_t SPR_BOSS10_OUCH;

extern std::int16_t SPR_BOSS10_SPIT1;
extern std::int16_t SPR_BOSS10_SPIT2;
extern std::int16_t SPR_BOSS10_SPIT3;
extern std::int16_t SPR_BOSS10_SPIT_EXP1;
extern std::int16_t SPR_BOSS10_SPIT_EXP2;
extern std::int16_t SPR_BOSS10_SPIT_EXP3;

//
// Plasma detonator explosion
//
extern std::int16_t SPR_DETONATOR_EXP1;
extern std::int16_t SPR_DETONATOR_EXP2;
extern std::int16_t SPR_DETONATOR_EXP3;
extern std::int16_t SPR_DETONATOR_EXP4;
extern std::int16_t SPR_DETONATOR_EXP5;
extern std::int16_t SPR_DETONATOR_EXP6;
extern std::int16_t SPR_DETONATOR_EXP7;
extern std::int16_t SPR_DETONATOR_EXP8;

//
// Clip Explosion
//
extern std::int16_t SPR_CLIP_EXP1;
extern std::int16_t SPR_CLIP_EXP2;
extern std::int16_t SPR_CLIP_EXP3;
extern std::int16_t SPR_CLIP_EXP4;
extern std::int16_t SPR_CLIP_EXP5;
extern std::int16_t SPR_CLIP_EXP6;
extern std::int16_t SPR_CLIP_EXP7;
extern std::int16_t SPR_CLIP_EXP8;

//
// BFG Cannon Weapon
//
extern std::int16_t SPR_BFG_WEAPON1;
extern std::int16_t SPR_BFG_WEAPON2;
extern std::int16_t SPR_BFG_WEAPON3;
extern std::int16_t SPR_BFG_WEAPON4;
extern std::int16_t SPR_BFG_WEAPON5;

//
// BFG Cannon Weapon shots
//
extern std::int16_t SPR_BFG_WEAPON_SHOT1;
extern std::int16_t SPR_BFG_WEAPON_SHOT2;
extern std::int16_t SPR_BFG_WEAPON_SHOT3;

//
// BFG Shot explosion
//
extern std::int16_t SPR_BFG_EXP1;
extern std::int16_t SPR_BFG_EXP2;
extern std::int16_t SPR_BFG_EXP3;
extern std::int16_t SPR_BFG_EXP4;
extern std::int16_t SPR_BFG_EXP5;
extern std::int16_t SPR_BFG_EXP6;
extern std::int16_t SPR_BFG_EXP7;
extern std::int16_t SPR_BFG_EXP8;


//
// Breifing types - Note these are ordered to an char array in Breifing().
//
enum breifing_type
{
	BT_LOSE,
	BT_WIN,
	BT_INTRO
}; // breifing_type


//
// Barrier State Transistions
//
enum barrier_state_type
{
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

using fixed = std::int32_t;

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


enum controldir_t
{
	di_north,
	di_east,
	di_south,
	di_west
}; // controldir_t

// NOTE - This enum list is ORDERED!
enum door_t
{
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

enum keytype
{
	kt_none = -1,
	kt_red,
	kt_yellow,
	kt_blue,
	kt_green,
	kt_gold,
	NUMKEYS,
}; // keytype

enum activetype
{
	ac_badobject = -1,
	ac_no,
	ac_yes,
	ac_allways
}; // activetype

enum classtype
{
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
classtype operator++(
	classtype& value,
	const int);
// BBi

//
// NOTE: When adding bonus objects - Make sure that they are added
// at the bottom of the list or that BonusMsg[] is correctly
// updated.
//

enum stat_t
{
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

struct stattype
{
	std::int16_t picnum;
	stat_t type;
}; // stattype

enum dirtype
{
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
dirtype operator+=(
	dirtype& lhs,
	const int rhs);

dirtype operator-=(
	dirtype& lhs,
	const int rhs);

dirtype operator|=(
	dirtype& lhs,
	const int rhs);

dirtype operator--(
	dirtype& value,
	const int);

dirtype operator++(
	dirtype& value,
	const int);

enum enemy_t
{
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


struct statetype
{
	int flags;
	int shapenum; // a shapenum of -1 means get from ob->temp1
	int tictime;

	void (*think)(
		objtype* actor);

	void (*action)(
		objtype* actor);

	statetype* next;
}; // statetype


// ---------------------
//
// trivial actor structure
//
// ---------------------

struct statobj_t
{
	// !!! Used in saved game.
	std::uint8_t tilex;

	// !!! Used in saved game.
	std::uint8_t tiley;

	// !!! Used in saved game.
	std::uint8_t areanumber;

	// !!! Used in saved game.
	std::uint8_t* visspot;

	// if shapenum == -1 the obj has been removed
	// !!! Used in saved game.
	std::int16_t shapenum;

	// !!! Used in saved game.
	std::uint16_t flags;

	// !!! Used in saved game.
	std::uint8_t itemnumber;

	// !!! Used in saved game.
	std::int8_t lighting;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // statobj_t


// ---------------------
//
// door actor structure
//
// ---------------------

// BBi
enum DoorAction
{
	dr_open,
	dr_closed,
	dr_opening,
	dr_closing,
	dr_jammed,
}; // DoorAction

struct doorobj_t
{
	// !!! Used in saved game.
	std::uint8_t tilex{};

	// !!! Used in saved game.
	std::uint8_t tiley{};

	// !!! Used in saved game.
	bool vertical{};

	// !!! Used in saved game.
	std::int8_t flags{};

	// !!! Used in saved game.
	keytype lock{kt_none};

	// !!! Used in saved game.
	door_t type{dr_normal};

	// !!! Used in saved game.
	DoorAction action{dr_closed};

	// !!! Used in saved game.
	std::int16_t ticcount{};

	// !!! Used in saved game.
	std::uint8_t areanumber[2]{};

	bstone::Voice voice;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // doorobj_t


// --------------------
//
// thinking actor structure
//
// --------------------

struct objtype
{
	// !!! Used in saved game.
	std::uint8_t tilex{};

	// !!! Used in saved game.
	std::uint8_t tiley{};

	// !!! Used in saved game.
	std::uint8_t areanumber{};

	// !!! Used in saved game.
	activetype active{ac_badobject};

	// !!! Used in saved game.
	std::int16_t ticcount{};

	// !!! Used in saved game.
	classtype obclass{nothing};

	// !!! Used in saved game.
	statetype* state{};

	// !!! Used in saved game.
	std::uint32_t flags{};

	// Aux flags
	// !!! Used in saved game.
	std::uint16_t flags2{};

	// if negative, wait for that door to open
	// !!! Used in saved game.
	double distance{};

	// !!! Used in saved game.
	dirtype dir{nodir};

	// "bit 7" == "direction to turn" flag
	// !!! Used in saved game.
	dirtype trydir{nodir};

	// !!! Used in saved game.
	double x{};

	// !!! Used in saved game.
	double y{};

	// !!! Used in saved game.
	std::uint8_t s_tilex{};

	// s_tilex==0, running for corner
	// !!! Used in saved game.
	std::uint8_t s_tiley{};

	std::int16_t viewx{};

	std::uint16_t viewheight{};

	// FIXME
	// In original code it also used to store a 16-bit pointer to object.
	// Since our code is at least 32-bit we are using an index of object.
	// !!! Used in saved game.
	std::int16_t hitpoints{};

	// !!! Used in saved game.
	std::uint8_t ammo{};

	// !!! Used in saved game.
	std::int8_t lighting{};

	// !!! Used in saved game.
	std::uint16_t linc{};

	// !!! Used in saved game.
	std::int16_t angle{};

	// !!! Used in saved game.
	double speed{};

	// !!! Used in saved game.
	std::int16_t temp1{};

	// FIXME
	// In original code it also used to store a 16-bit pointer to object.
	// Since our code is at least 32-bit we are using an index of object.
	// !!! Used in saved game.
	std::int16_t temp2{};

	// FIXME
	// In original code it also used to store a 16-bit pointer to object.
	// Since our code is at least 32-bit we are using an index of object.
	// !!! Used in saved game.
	std::uint16_t temp3{}; // holds 'last door used' by 'smart' actors

	bstone::Voice voice_voice;
	bstone::Voice weapon_voice;
	bstone::Voice walking_voice;

	objtype* next{};
	objtype* prev{};


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // objtype


enum ButtonStateType
{
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
}; // ButtonStateType


enum weapontype
{
	wp_autocharge,
	wp_pistol,
	wp_burst_rifle,
	wp_ion_cannon,
	wp_grenade,
	wp_bfg_cannon,

	wp_SPACER,
}; // weapontype

extern int NUMWEAPONS;


enum Difficulty
{
	gd_baby,
	gd_easy,
	gd_medium,
	gd_hard,
}; // Difficulty


/*
enum backgroundtype
{
	ELEVATOR_BACK,
	TRANSPORTER_BACK
}; // backgroundtype
*/


//
// General Coord (tile) structure
//
struct tilecoord_t
{
	// !!! Used in saved game.
	std::uint8_t tilex;

	// !!! Used in saved game.
	std::uint8_t tiley;

	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // tilecoord_t


// -----------------------------------
//
// barrier coord/table structure
//
// -----------------------------------
struct barrier_type
{
	// !!! Used in saved game.
	tilecoord_t coord;

	// !!! Used in saved game.
	std::uint8_t on;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // barrier_type;

using Barriers = std::vector<barrier_type>;


struct statsInfoType
{
	// !!! Used in saved game.
	std::int32_t total_points;

	// !!! Used in saved game.
	std::int32_t accum_points;

	// !!! Used in saved game.
	std::uint8_t total_enemy;

	// !!! Used in saved game.
	std::uint8_t accum_enemy;

	// !!! Used in saved game.
	std::uint8_t total_inf;

	// !!! Used in saved game.
	std::uint8_t accum_inf;

	// !!! Used in saved game.
	std::int16_t overall_floor;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // statsInfoType

struct levelinfo
{
	// !!! Used in saved game.
	std::uint16_t bonus_queue; // bonuses that need to be shown

	// !!! Used in saved game.
	std::uint16_t bonus_shown; // bonuses that have been shown

	// !!! Used in saved game.
	bool locked;

	// !!! Used in saved game.
	statsInfoType stats;

	// !!! Used in saved game.
	std::uint8_t ptilex;

	// !!! Used in saved game.
	std::uint8_t ptiley;

	// !!! Used in saved game.
	std::int16_t pangle;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // levelinfo


struct fargametype
{
	using LevelInfos = std::vector<levelinfo>;

	LevelInfos level;

	fargametype();

	void initialize();

	void clear();


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // fargametype

struct gametype
{
	// !!! Used in saved game.
	std::int16_t turn_around{};

	// !!! Used in saved game.
	std::int16_t turn_angle{};

	// !!! Used in saved game.
	std::uint16_t flags{};

	// !!! Used in saved game.
	std::int16_t lastmapon{};

	// !!! Used in saved game.
	std::int16_t difficulty{};

	// !!! Used in saved game.
	std::int16_t mapon{};

	// !!! Used in saved game.
	std::int32_t tic_score{};

	// !!! Used in saved game.
	std::int32_t score{};

	// !!! Used in saved game.
	std::int32_t nextextra{};

	// !!! Used in saved game.
	std::int16_t score_roll_wait{};

	// !!! Used in saved game.
	std::int16_t lives{};

	// !!! Used in saved game.
	std::int16_t health{};

	// !!! Used in saved game.
	std::int16_t rpower{};

	// !!! Used in saved game.
	std::int8_t rzoom{};

	// !!! Used in saved game.
	std::int8_t radar_leds{};

	// !!! Used in saved game.
	std::int8_t lastradar_leds{};

	// !!! Used in saved game.
	std::int8_t lastammo_leds{};

	// !!! Used in saved game.
	std::int8_t ammo_leds{};

	// !!! Used in saved game.
	std::int16_t ammo{};

	// !!! Used in saved game.
	std::int16_t plasma_detonators{};

	// !!! Used in saved game.
	std::int8_t useable_weapons{};

	// !!! Used in saved game.
	std::int8_t weapons{};

	// !!! Used in saved game.
	std::int8_t weapon{};

	// !!! Used in saved game.
	std::int8_t chosenweapon{};

	// !!! Used in saved game.
	std::int8_t weapon_wait{};

	// !!! Used in saved game.
	std::int16_t attackframe{};

	// !!! Used in saved game.
	std::int16_t attackcount{};

	// !!! Used in saved game.
	std::int16_t weaponframe{};

	// !!! Used in saved game.
	std::int16_t episode{};

	// !!! Used in saved game.
	std::uint32_t TimeCount{};

	const char* msg{}; // InfoArea msg...

	// !!! Used in saved game.
	std::int8_t numkeys[NUMKEYS]{};

	// !!! Used in saved game.
	Barriers barrier_table;

	// !!! Used in saved game.
	std::uint16_t tokens{};

	// !!! Used in saved game.
	bool boss_key_dropped{};

	// !!! Used in saved game.
	std::int16_t wintilex{};

	// !!! Used in saved game.
	std::int16_t wintiley{};


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);

	void initialize();
	void initialize_barriers();


	int get_barrier_group_offset(
		const int level) const;

	int get_barrier_index(
		const int code) const;

	int encode_barrier_index(
		const int level,
		const int index) const;

	void decode_barrier_index(
		const int code,
		int& level,
		int& index) const;
}; // gametype

enum exit_t
{
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


enum animtype_t
{
	at_NONE = 0,
	at_CYCLE,
	at_REBOUND,
	at_ONCE,
}; // animtype_t

enum animdir_t
{
	ad_FWD = 0,
	ad_REV,
}; // animdir_t


//
// DisplayInfoMsg Priorities - Duplicate Values are Allowed.
//
// ("enum" list used simply for debuging use like Object Classes)
//

//
// Msg_Priorities - Hell.. Lets just make them all the same...

enum msg_priorities
{
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

enum infomsg_type
{
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
enum inst_type
{
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
struct mCacheInfo
{
	using MSeg = std::vector<char>;

	// where msg is in 'local' list
	// !!! Used in saved game.
	std::uint8_t local_val{};

	// where msg was in 'global' list
	// !!! Used in saved game.
	std::uint8_t global_val{};

	// pointer to message
	MSeg mSeg;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // mCacheInfo

// Basic 'message list' structure
//
struct mCacheList
{
	std::int16_t NumMsgs; // number of messages
	mCacheInfo mInfo[MAX_CACHE_MSGS]; // table of message 'info'
}; // mCacheList

// ----------------------- CONCESSION STRUCTURES --------------------------

// Concession 'message info' structure
//
struct con_mCacheInfo
{
	mCacheInfo mInfo;

	// type of concession
	// !!! Used in saved game.
	std::uint8_t type{};

	// # of times req'd to operate
	// !!! Used in saved game.
	std::uint8_t operate_cnt{};


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // con_mCacheInfo

// Concession 'message list' structure
//
struct concession_t
{
	// also, num concessions
	// !!! Used in saved game.
	std::int16_t NumMsgs;

	// !!! Used in saved game.
	con_mCacheInfo cmInfo[MAX_CACHE_MSGS];


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // concession_t

// ------------------------ INFORMANT STRUCTURES --------------------------

// Informant 'message info' structure
//
struct sci_mCacheInfo
{
	mCacheInfo mInfo;
	std::uint8_t areanumber{}; // 'where' msg can be used
}; // sci_mCacheInfo

// Informant 'message list' structure
//
struct scientist_t
{
	std::int16_t NumMsgs{};
	sci_mCacheInfo smInfo[MAX_CACHE_MSGS];
}; // scientist_t

// ------------------------------------------------------------------------

// Electro-Alien controller structer
//
struct eaWallInfo
{
	// !!! Used in saved game.
	std::int8_t tilex;

	// where this controller is in the map.
	// !!! Used in saved game.
	std::int8_t tiley;

	// aliens spawned by this controller.
	// !!! Used in saved game.
	std::int8_t aliens_out;

	// delay before spawning another alien.
	// !!! Used in saved game.
	std::int16_t delay;


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // eaWallInfo


// General Structure to hold goldstern specific stuff...
//

struct GoldsternInfo_t
{
	// Last Spawn Coord Index
	// !!! Used in saved game.
	std::uint8_t LastIndex;

	// Num of Spawnpoints for Goldstern
	// !!! Used in saved game.
	std::uint8_t SpawnCnt;

	// What type of command/operation is needed...
	// !!! Used in saved game.
	std::uint16_t flags;

	// Wait time for Goldstern Spawn (current & Next)
	// !!! Used in saved game.
	std::uint16_t WaitTime;

	// !!! Used in saved game.
	bool GoldSpawned; // Has Goldstern been spawned?


	void archive(
		bstone::Archiver& archiver) const;

	void unarchive(
		bstone::Archiver& archiver);
}; // GoldsternInfo_t

struct PlayerWarp
{
	int tilex;
	int tiley;
	dirtype dir;
};

extern PlayerWarp player_warp;

/*
=============================================================================

3D_MAIN DEFINITIONS

=============================================================================
*/

extern bstone::GameTimer TimeCount; // Global time in ticks

extern std::int16_t TITLE_LOOP_MUSIC;

#define CANT_PLAY_TXT "\n" \
    "ERROR: Insufficient disk space.\n" \
    "Try deleting some files from your hard disk.\n\n"


extern bool is_data_dir_custom_;
extern std::string data_dir_;
extern std::string mod_dir_;

constexpr auto radtoint = static_cast<double>(FINEANGLES) / 2.0 / bstone::math::pi();

extern std::int16_t starting_level;
extern std::int16_t starting_episode;
extern std::int16_t starting_difficulty;

extern std::uint16_t TopColor;
extern std::uint16_t BottomColor;
extern char str[80];

extern int screenofs;
extern int viewwidth;
extern int viewheight;
extern int centerx;

extern std::int16_t dirangle[9];

extern bool startgame;
extern bool loadedgame;

extern int last_map_tile_x;
extern int last_map_tile_y;

// BBi
const int min_mouse_sensitivity = 0;
const int max_mouse_sensitivity = 54;
#ifndef __vita__
const int default_mouse_sensitivity = 5;
#else
const int default_mouse_sensitivity = 27;
#endif
// BBi

//
// math tables
//
extern std::vector<int> pixelangle;

using FineTangent = std::array<double, FINEANGLES / 4>;
extern FineTangent finetangent;

extern double sintable[];
extern double* costable;

//
// derived constants
//
extern double scale_;
extern double heightnumerator;

extern bool ShowQuickMsg;


void LoadFonts();

void ClearNClose();

void CycleColors();

void SetViewSize();

void SetPlaneViewSize();

void NewViewSize();

bool DoMovie(
	const MovieId movie,
	const void* const raw_palette = nullptr);

bool CheckDiskSpace(
	std::int32_t needed,
	const char* text,
	cds_io_type io_type);

bool SaveTheGame(
	const std::string& file_name,
	const std::string& description);

void SetupWalls();

void InitDestPath();


/*
=============================================================================

 3D_GAME DEFINITIONS

=============================================================================
*/

extern fargametype gamestuff;
extern fargametype old_gamestuff;
extern tilecoord_t GoldieList[GOLDIE_MAX_SPAWNS];
extern GoldsternInfo_t GoldsternInfo;

extern eaWallInfo eaList[];
extern std::int8_t NumEAWalls;
extern bool ingame;
extern bool fizzlein;
extern int latchpics[NUMLATCHPICS];
extern gametype gamestate;
extern gametype old_gamestate;
extern std::int16_t doornum;


void DrawPlayBorder();

void ScanInfoPlane();

void SetupGameLevel();

void DrawPlayScreen(bool);

void GameLoop();

void Warped();

void RotateView(
	std::int16_t DestAngle,
	std::uint8_t RotSpeed);

void DrawWarpIn();

void BMAmsg(
	const char* msg);

void CacheBMAmsg(std::uint16_t MsgNum);

void BevelBox(
	std::int16_t xl,
	std::int16_t yl,
	std::int16_t w,
	std::int16_t h,
	std::uint8_t hi,
	std::uint8_t med,
	std::uint8_t lo);

void ShadowPrintLocationText(
	sp_type type);

void LoadLocationText(
	std::int16_t textNum);

void NewGame(
	std::int16_t difficulty,
	std::int16_t episode);

/*
=============================================================================

 3D_PLAY DEFINITIONS

=============================================================================
*/

extern int objcount;

extern objtype* DeadGuys[];
extern objtype dummyobj;
extern std::uint8_t NumDeadGuys;

extern exit_t playstate;

extern bool madenoise;
extern bool usedummy;
extern bool nevermark;
extern std::uint8_t alerted;
extern std::uint8_t alerted_areanum;

extern objtype objlist[MAXACTORS];
extern objtype* new_actor;
extern objtype* player;
extern objtype* lastobj;
extern objtype* objfreelist;
extern objtype* killerobj;

using StatObjList = std::array<statobj_t, MAXSTATS>;
extern StatObjList statobjlist;

extern statobj_t* laststatobj;
extern doorobj_t doorobjlist[MAXDOORS];
extern doorobj_t* lastdoorobj;

extern std::uint16_t farmapylookup[MAPSIZE];
extern std::uint8_t* nearmapylookup[MAPSIZE];

using SubTileMap = std::array<std::uint8_t, MAPSIZE>;
using TileMap = std::array<SubTileMap, MAPSIZE>;
extern TileMap tilemap; // wall values only

using SubSpotVis = std::array<std::uint8_t, MAPSIZE>;
using SpotVis = std::array<SubSpotVis, MAPSIZE>;
extern SpotVis spotvis;

using SubActorAt = std::array<objtype*, MAPSIZE>;
using ActorAt = std::array<SubActorAt, MAPSIZE>;
extern ActorAt actorat;

extern bool singlestep;
extern bool godmode;
extern bool DebugOk;
extern std::int16_t InstantWin;
extern std::int16_t InstantQuit;
extern std::uint16_t ExtraRadarFlags;

//
// control info
//
using ScanCodes = std::vector<ScanCode>;
using Buttons = std::vector<std::int16_t>;

using ButtonHeld = std::bitset<NUMBUTTONS>;
extern ButtonHeld buttonheld;

extern const int viewsize;

//
// curent user input
//
extern int controlx;
extern int controly; // range from -100 to 100
#ifdef __vita__
extern int control2x; //left stick horizontal axis
#endif

using ButtonState = std::bitset<NUMBUTTONS>;
extern ButtonState buttonstate;
extern int strafe_value;

extern char Computing[];


void InitActorList();

void GetNewActor();

void RemoveObj(
	objtype* gone);

void StopMusic();

void StartMusic(
	bool startit);

void PlayLoop();

void CheckMusicToggle();

/*
=============================================================================

 3D_DEBUG

=============================================================================
*/

bool DebugKeys();


/*
=============================================================================

 3D_DRAW DEFINITIONS

=============================================================================
*/

using SubTravelTable = std::array<std::uint8_t, MAPSIZE>;
using TravelTable = std::array<SubTravelTable, MAPSIZE>;
extern TravelTable travel_table_;

extern std::int32_t lasttimecount;
extern std::int32_t framecount;
extern std::int32_t frameon;
extern bool fizzlein;

using SpanStart = std::vector<int>;
using StepScale = std::vector<int>;
using BaseDist = std::vector<double>;
using PlaneYLookup = std::vector<int>;
using MirrorOfs = std::vector<int>;
using WallHeight = std::vector<double>;

extern WallHeight wallheight;

extern double focallength;

//
// refresh variables
//
extern double viewx;
extern double viewy; // the focal point
extern int viewangle;
extern double viewsin;
extern double viewcos;

extern const std::uint8_t* postsource;
extern int postx;

// BBi
extern int posty;


extern std::int16_t horizwall[];
extern std::int16_t vertwall[];

extern double pwallpos;

extern bool cloaked_shape;


void BuildTables();

void CalcTics();

void ThreeDRefresh();

void ShowOverhead(
	int bx,
	int by,
	int radius,
	int zoom,
	int flags);


/*
=============================================================================

 3D_DRAW2 DEFINITIONS

=============================================================================
*/
extern std::uint16_t CeilingTile;
extern std::uint16_t FloorTile;
extern void(*MapRowPtr)();

void DrawPlanes();

void MapRow();

void C_MapRow();

void F_MapRow();


/*
=============================================================================

 3D_STATE DEFINITIONS

=============================================================================
*/

constexpr auto actor_points_size = 45;

using ActorPoints = std::array<std::uint16_t, actor_points_size>;
extern const ActorPoints actor_points;

extern dirtype opposite[9];
extern dirtype diagonal[9][9];


std::uint16_t CheckRunChase(
	objtype* ob);

bool LookForGoodies(
	objtype* ob,
	std::uint16_t RunReason);

void SpawnNewObj(
	std::uint16_t tilex,
	std::uint16_t tiley,
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
	const double move);

void KillActor(
	objtype* ob);

void DamageActor(
	objtype* ob,
	std::uint16_t damage,
	objtype* attacker);

bool CheckLine(
	objtype* from_obj,
	objtype* to_obj);

bool CheckSight(
	objtype* from_obj,
	objtype* to_obj);

void MakeAlertNoise(
	objtype* obj);


/*
=============================================================================

 3D_SCALE DEFINITIONS

=============================================================================
*/


extern int normalshade;
extern int normalshade_div;
extern int shade_max;


void SetupScaling(
	int maxscaleheight) noexcept;

void ScaleShape(
	int xcenter,
	int shapenum,
	int height);

// BBi
void scale_player_weapon(
	const int sprite_id,
	const int height);


/*
=============================================================================

 3D_AGENT DEFINITIONS

=============================================================================
*/

extern scientist_t InfHintList;
extern scientist_t NiceSciList;
extern scientist_t MeanSciList;

extern std::uint16_t static_points[];
extern bool GAN_HiddenArea;
extern std::uint8_t LastInfArea;
extern std::int16_t FirstGenInfMsg;
extern std::int16_t TotalGenInfMsgs;
extern classtype LastInfoAttacker;


//
// player state info
//
extern double thrustspeed;
extern std::uint16_t plux;
extern std::uint16_t pluy; // player coordinates scaled to unsigned
extern bool PlayerInvisable;
extern std::int8_t DrawInfoArea_COUNT;
extern std::int8_t InitInfoArea_COUNT;


// Location Bar message string...

extern char LocationText[MAX_LOCATION_DESC_LEN];


//
// search / move info
//
extern int anglefrac;

extern std::uint16_t LastMsgPri;
extern std::int16_t MsgTicsRemain;

void GivePoints(
	std::int32_t score,
	bool add_to_stats);

void SpawnPlayer(
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir);

void DrawAmmo(
	bool ForceRefresh);

bool DisplayInfoMsg(
	const char* Msg,
	msg_priorities Priority,
	std::int16_t DisplayTime,
	std::int16_t MessageType);

bool DisplayInfoMsg(
	const std::string& Msg,
	msg_priorities Priority,
	std::int16_t DisplayTime,
	std::int16_t MessageType);

void UpdateInfoAreaClock();

std::int8_t GetAreaNumber(
	int tilex,
	int tiley);

void TakeKey(
	std::int16_t key);

void GiveToken(
	std::int16_t tokens);

void TakePlasmaDetonator(
	std::int16_t count);

void CacheDrawPic(
	int x,
	int y,
	int pic);

void ActivateWallSwitch(
	std::uint16_t iconnum,
	std::int16_t x,
	std::int16_t y);

// AOG
std::uint16_t UpdateBarrierTable(
	const int level,
	const int x,
	const int y);

// PS
std::uint16_t UpdateBarrierTable(
	const int x,
	const int y,
	const bool on_off);

std::uint16_t ScanBarrierTable(
	std::uint8_t x,
	std::uint8_t y);

void DisplayNoMoMsgs();

std::int16_t ShowStats(
	std::int16_t bx,
	std::int16_t by,
	ss_type type,
	statsInfoType* stats);

bool OperateSmartSwitch(
	std::uint16_t tilex,
	std::uint16_t tiley,
	std::int8_t Operation,
	bool Force);

/*
=============================================================================

 3D_ACT1 DEFINITIONS

=============================================================================
*/
using StatInfos = std::vector<stattype>;


extern std::int8_t xy_offset[8][2];
extern StatInfos statinfo;
extern concession_t ConHintList;

extern doorobj_t doorobjlist[MAXDOORS];
extern doorobj_t* lastdoorobj;
extern std::int16_t doornum;

using DoorPositions = std::array<double, MAXDOORS>;
extern DoorPositions doorposition;

using SubAreaConnect = std::array<std::uint8_t, NUMAREAS>;
using AreaConnect = std::array<SubAreaConnect, NUMAREAS>;
extern AreaConnect areaconnect;

using AreaByPlayer = std::bitset<NUMAREAS>;
extern AreaByPlayer areabyplayer;

extern double pwallstate;
extern double pwallpos; // amount a pushable wall has been moved (0-63)
extern std::uint16_t pwallx;
extern std::uint16_t pwally;
extern std::int16_t pwalldir;
extern std::int16_t pwalldist;


statobj_t* ReserveStatic();

statobj_t* SpawnStatic(
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t type);

void SpawnDoor(
	std::int16_t tilex,
	std::int16_t tiley,
	bool vertical,
	keytype lock,
	door_t type);

void OperateConcession(
	std::uint16_t concession);

void SpawnConcession(
	std::int16_t tilex,
	std::int16_t tiley,
	std::uint16_t credits,
	std::uint16_t machinetype);

void CheckSpawnEA();

void CheckSpawnGoldstern();

void InitMsgCache(
	mCacheList* mList,
	std::uint16_t listSize,
	std::uint16_t infoSize);

void CacheMsg(
	mCacheInfo* ci,
	std::uint16_t SegNum,
	std::uint16_t MsgNum);

std::int16_t LoadMsg(
	char* hint_buffer,
	std::uint16_t SegNum,
	std::uint16_t MsgNum,
	std::uint16_t MaxMsgLen);

bool ReuseMsg(
	mCacheInfo* ci,
	std::int16_t count,
	std::int16_t struct_size);

void BlastNearDoors(
	std::int16_t tilex,
	std::int16_t tiley);

void TryBlastDoor(
	std::int8_t door);

statobj_t* FindStatic(
	std::uint16_t tilex,
	std::uint16_t tiley);

void PlaceReservedItemNearTile(
	std::int16_t itemtype,
	std::int16_t tilex,
	std::int16_t tiley);

void ExplodeStatics(
	std::int16_t tilex,
	std::int16_t tiley);


/*
=============================================================================

 3D_ACT2 DEFINITIONS

=============================================================================
*/

#define BARRIER_STATE(obj) ((obj)->ammo)

extern std::int8_t detonators_spawned;

constexpr auto difficulty_levels = 4;
using SubStartHitPoints = std::array<std::uint16_t, NUMHITENEMIES>;
using StartHitPoints = std::array<SubStartHitPoints, difficulty_levels>;

extern const StartHitPoints* starthitpoints;

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

void SpawnProjectile(
	objtype* shooter,
	classtype class_type);

void SpawnStand(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir);

void SpawnPatrol(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	std::int16_t dir);

void KillActor(
	objtype* ob);

void US_ControlPanel(
	ScanCode scan_code);

void SpawnOffsetObj(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley);

void SpawnCusExplosion(
	const double x,
	const double y,
	std::uint16_t StartFrame,
	std::uint16_t NumFrames,
	std::uint16_t Delay,
	std::uint16_t Class);

extern statetype s_barrier_transition;
extern statetype s_barrier_shutdown;

void SpawnBarrier(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley,
	bool OnOff);

void InitAnim(
	objtype* obj,
	std::uint16_t ShapeNum,
	std::uint8_t StartOfs,
	std::uint8_t MaxOfs,
	animtype_t AnimType,
	animdir_t AnimDir,
	std::uint16_t Delay,
	std::uint16_t WaitDelay);

objtype* FindObj(
	classtype which,
	std::int16_t tilex,
	std::int16_t tiley);

void SpawnHiddenOfs(
	enemy_t which,
	std::int16_t tilex,
	std::int16_t tiley);

objtype* MoveHiddenOfs(
	classtype which_class,
	classtype new1,
	const double x,
	const double y);

void DropCargo(
	objtype* obj);


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

extern BonusMessages BonusMsg;
extern ActorMessages ActorInfoMsg;

extern std::string ekg_heartbeat_enabled;
extern std::string ekg_heartbeat_disabled;
extern std::string attacker_info_enabled;
extern std::string attacker_info_disabled;
extern std::string WeaponNotAvailMsg;
extern std::string WeaponAvailMsg;
extern std::string RadarEnergyGoneMsg;
extern std::string EnergyPackDepleted;
extern std::string NotEnoughEnergyForWeapon;

extern std::string WeaponMalfunction;

extern std::string SwitchNotActivateMsg;
extern std::string NoFoodTokens;

extern std::string NoAdLibCard;
extern std::string MusicOn;
extern std::string MusicOff;
extern std::string SoundOn;
extern std::string SoundOff;

extern std::string pd_dropped;
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

void DisplayPrepingMsg(
	const char* text);

bool Breifing(
	breifing_type BreifingType,
	std::uint16_t episode);

void ShPrint(
	const char* text,
	std::int8_t shadow_color,
	bool single_char);

std::uint16_t Random(
	std::uint16_t Max);


// ===========================================================================
//
// 3D_MENU DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================

extern bool EscPressed;


void DrawInstructions(
	inst_type Type);

void CacheMessage(
	std::uint16_t MessageNum);


// ===========================================================================
//
// 3D_FREE DEFINATIONS - WHICH NEED TO BE GLOBAL
//
// ===========================================================================


// BBi
namespace bstone
{


class TextWriter;


} // bstone


enum class Game
{
	none,
	aog,
	aog_sw,
	ps,
}; // Game


constexpr int tilemap_wall_mask = 0B0011'1111;
constexpr int tilemap_door_track_flag = 0B0100'0000;
constexpr int tilemap_door_flag = 0B1000'0000;
constexpr int tilemap_door_flags = tilemap_door_track_flag | tilemap_door_flag;


using Buffer = std::vector<unsigned char>;


struct PaletteShiftInfo
{
	bool is_bonus_shifted_;
	std::uint8_t bonus_r_;
	std::uint8_t bonus_g_;
	std::uint8_t bonus_b_;
	std::uint8_t bonus_a_;

	bool is_damage_shifted_;
	std::uint8_t damage_r_;
	std::uint8_t damage_g_;
	std::uint8_t damage_b_;
	std::uint8_t damage_a_;
}; // PaletteShiftInfo


objtype* ui16_to_actor(std::uint16_t value);
std::uint16_t actor_to_ui16(const objtype* actor);
statobj_t* ui16_to_static_object(std::uint16_t value);
std::uint16_t static_object_to_ui16(const statobj_t* static_object);
doorobj_t* ui16_to_door_object(std::uint16_t value);
std::uint16_t door_object_to_ui16(const doorobj_t* door_object);

bool gp_is_flooring_solid() noexcept;
void gp_is_flooring_solid(bool is_enable);

bool gp_is_ceiling_solid() noexcept;
void gp_is_ceiling_solid(bool is_enable);

bool gp_no_shading() noexcept;
void gp_no_shading(bool is_enable);

bool gp_hide_attacker_info() noexcept;
void gp_hide_attacker_info(bool is_enable);

bool gp_is_always_run() noexcept;
void gp_is_always_run(bool is_enable);

bool gp_no_wall_hit_sfx() noexcept;
void gp_no_wall_hit_sfx(bool is_enable);

bool gp_use_heart_beat_sfx() noexcept;
void gp_use_heart_beat_sfx(bool is_enable);

bool gp_quit_on_escape() noexcept;
void gp_quit_on_escape(bool is_enable);

bool gp_no_intro_outro() noexcept;
void gp_no_intro_outro(bool is_enable);

bool gp_no_screens() noexcept;
void gp_no_screens(bool is_enable);

bool gp_no_fade_in_or_out() noexcept;
void gp_no_fade_in_or_out(bool is_enable);

bool gp_no_weapon_bobbing() noexcept;
void gp_no_weapon_bobbing(bool is_enable);

bool gp_vanilla_fizzle_fx() noexcept;
void gp_vanilla_fizzle_fx(bool is_enabled);

bool gp_ps_map_in_stats() noexcept;
void gp_ps_map_in_stats(bool is_enabled);

bool am_rotatable() noexcept;
void am_rotatable(bool is_enable);

extern bstone::MtTaskMgr* mt_task_manager_;


void InitSmartSpeedAnim(
	objtype* obj,
	std::uint16_t ShapeNum,
	std::uint8_t StartOfs,
	std::uint8_t MaxOfs,
	animtype_t AnimType,
	animdir_t AnimDir,
	std::uint16_t Delay);

void InitSmartAnim(
	objtype* obj,
	std::uint16_t ShapeNum,
	std::uint8_t StartOfs,
	std::uint8_t MaxOfs,
	animtype_t AnimType,
	animdir_t AnimDir);


void sys_sleep_for(int milliseconds);

void sys_default_sleep_for();

const std::string& get_profile_dir();

const std::string& get_screenshot_dir();

void update_normalshade() noexcept;

int door_get_track_texture_id(
	const doorobj_t& door);

int actor_calculate_rotation(
	const objtype& actor);

int player_get_weapon_sprite_id();

double player_get_weapon_bounce_offset();

PaletteShiftInfo palette_shift_get_info() noexcept;

void cfg_file_write_entry(
	bstone::TextWriter& writer,
	bstone::StringView key,
	bstone::StringView value);

void cfg_file_write_entry(
	bstone::TextWriter& writer,
	const std::string& key_string,
	const std::string& value_string);

std::uint16_t get_start_hit_point(
	const int index);


double get_integral(
	const double value) noexcept;

double get_fractional(
	const double value) noexcept;

void gp_initialize_cvars(bstone::CVarMgr& cvar_mgr);
void am_initialize_cvars(bstone::CVarMgr& cvar_mgr);
// BBi


#endif // BSTONE_3D_DEF_INCLUDED
