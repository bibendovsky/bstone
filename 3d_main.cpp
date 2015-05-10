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


#include "3d_def.h"
#include "jm_lzh.h"
#include "bstone_binary_reader.h"
#include "bstone_binary_writer.h"


void VL_LatchToScreen(
    int source,
    int width,
    int height,
    int x,
    int y);

/*
=============================================================================

 BLAKE STONE
 (C)opyright 1993, JAM Productions, Inc.

 3D engine licensed by ID Software, Inc.
 Shareware distribution by Apogee Software, Inc.

=============================================================================
*/


void ConnectBarriers();
void FreeMusic();
void ClearMemory();

void CA_CacheScreen(
    int16_t chunk);

void VH_UpdateScreen();
void DrawHighScores();
void freed_main();

void PreloadUpdate(
    uint16_t current,
    uint16_t total);

void OpenAudioFile();


bstone::ClArgs g_args;


#define FOCALLENGTH (0x5700L) // in global coordinates
#define VIEWGLOBAL 0x10000 // globals visable flush to wall

#define VIEWWIDTH (256) // size of view window
#define VIEWHEIGHT (144)


#define MAX_DEST_PATH_LEN (30)


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

extern int16_t pickquick;


void DrawCreditsPage();
void unfreed_main();
void ShowPromo();

int16_t starting_episode;
int16_t starting_level;
int16_t starting_difficulty;

std::string data_dir;

void InitPlaytemp();


uint16_t TopColor;
uint16_t BottomColor;

bool nospr;

int16_t dirangle[9] = {
    0 * ANGLES / 8,
    1 * ANGLES / 8,
    2 * ANGLES / 8,
    3 * ANGLES / 8,
    4 * ANGLES / 8,
    5 * ANGLES / 8,
    6 * ANGLES / 8,
    7 * ANGLES / 8,
    8 * ANGLES / 8,
}; // dirangle

//
// proejection variables
//
int focallength;
int screenofs;
int viewwidth;
int viewheight;
int centerx;
int shootdelta; // pixels away from centerx a target can be
int scale;
int maxslope;
int heightnumerator;
int minheightdiv;


bool startgame;
bool loadedgame;
int16_t mouseadjustment;

const std::string config_file_name = "bstone_config";

int16_t view_xl;
int16_t view_xh;
int16_t view_yl;
int16_t view_yh;

#if IN_DEVELOPMENT
uint16_t democount = 0, jim = 0;
#endif

static const bool k_no_wall_hit_sound_default = false;
bool g_no_wall_hit_sound = k_no_wall_hit_sound_default;

static const bool k_always_run_default = false;
bool g_always_run = k_always_run_default;

// BBi AOG only options
static const bool default_heart_beat_sound = false;
bool g_heart_beat_sound = default_heart_beat_sound;

static const bool default_rotated_automap = false;
bool g_rotated_automap = default_rotated_automap;

GameType g_game_type;


bool no_screens = false;


// ==========================================================================
// Sprites

int16_t SPR_DEMO = 0;

int16_t SPR_STAT_0 = 0;
int16_t SPR_STAT_1 = 0;
int16_t SPR_STAT_2 = 0;
int16_t SPR_STAT_3 = 0;
int16_t SPR_STAT_4 = 0;
int16_t SPR_STAT_5 = 0;
int16_t SPR_STAT_6 = 0;
int16_t SPR_STAT_7 = 0;
int16_t SPR_STAT_8 = 0;
int16_t SPR_STAT_9 = 0;
int16_t SPR_STAT_10 = 0;
int16_t SPR_STAT_11 = 0;
int16_t SPR_STAT_12 = 0;
int16_t SPR_STAT_13 = 0;
int16_t SPR_STAT_14 = 0;
int16_t SPR_STAT_15 = 0;
int16_t SPR_STAT_16 = 0;
int16_t SPR_STAT_17 = 0;
int16_t SPR_STAT_18 = 0;
int16_t SPR_STAT_19 = 0;
int16_t SPR_STAT_20 = 0;
int16_t SPR_STAT_21 = 0;
int16_t SPR_STAT_22 = 0;
int16_t SPR_STAT_23 = 0;
int16_t SPR_STAT_24 = 0;
int16_t SPR_STAT_25 = 0;
int16_t SPR_STAT_26 = 0;
int16_t SPR_STAT_27 = 0;
int16_t SPR_STAT_28 = 0;
int16_t SPR_STAT_29 = 0;
int16_t SPR_STAT_30 = 0;
int16_t SPR_STAT_31 = 0;
int16_t SPR_STAT_32 = 0;
int16_t SPR_STAT_33 = 0;
int16_t SPR_STAT_34 = 0;
int16_t SPR_STAT_35 = 0;
int16_t SPR_STAT_36 = 0;
int16_t SPR_STAT_37 = 0;
int16_t SPR_STAT_38 = 0;
int16_t SPR_STAT_39 = 0;
int16_t SPR_STAT_40 = 0;
int16_t SPR_STAT_41 = 0;
int16_t SPR_STAT_42 = 0;
int16_t SPR_STAT_43 = 0;
int16_t SPR_STAT_44 = 0;
int16_t SPR_STAT_45 = 0;
int16_t SPR_STAT_46 = 0;
int16_t SPR_STAT_47 = 0;
int16_t SPR_STAT_48 = 0;
int16_t SPR_STAT_49 = 0;
int16_t SPR_STAT_50 = 0;
int16_t SPR_STAT_51 = 0;
int16_t SPR_STAT_52 = 0;
int16_t SPR_STAT_53 = 0;
int16_t SPR_STAT_54 = 0;
int16_t SPR_STAT_55 = 0;
int16_t SPR_STAT_56 = 0;

int16_t SPR_CRATE_1 = 0;
int16_t SPR_CRATE_2 = 0;
int16_t SPR_CRATE_3 = 0;

int16_t SPR_STAT_57 = 0;
int16_t SPR_STAT_58 = 0;
int16_t SPR_STAT_59 = 0;
int16_t SPR_STAT_60 = 0;
int16_t SPR_STAT_61 = 0;
int16_t SPR_STAT_62 = 0;
int16_t SPR_STAT_63 = 0;
int16_t SPR_STAT_64 = 0;
int16_t SPR_STAT_65 = 0;
int16_t SPR_STAT_66 = 0;
int16_t SPR_STAT_67 = 0;
int16_t SPR_STAT_68 = 0;
int16_t SPR_STAT_69 = 0;
int16_t SPR_STAT_70 = 0;
int16_t SPR_STAT_71 = 0;
int16_t SPR_STAT_72 = 0;
int16_t SPR_STAT_73 = 0;
int16_t SPR_STAT_74 = 0;
int16_t SPR_STAT_75 = 0;
int16_t SPR_STAT_76 = 0;
int16_t SPR_STAT_77 = 0;
int16_t SPR_STAT_78 = 0;
int16_t SPR_STAT_79 = 0;

int16_t SPR_DOORBOMB = 0;
int16_t SPR_ALT_DOORBOMB = 0;
int16_t SPR_RUBBLE = 0;
int16_t SPR_BONZI_TREE = 0;
int16_t SPR_AUTOMAPPER = 0;
int16_t SPR_POT_PLANT = 0;
int16_t SPR_TUBE_PLANT = 0;
int16_t SPR_HITECH_CHAIR = 0;

int16_t SPR_AIR_VENT = 0;
int16_t SPR_BLOOD_DRIP1 = 0;
int16_t SPR_BLOOD_DRIP2 = 0;
int16_t SPR_BLOOD_DRIP3 = 0;
int16_t SPR_BLOOD_DRIP4 = 0;
int16_t SPR_WATER_DRIP1 = 0;
int16_t SPR_WATER_DRIP2 = 0;
int16_t SPR_WATER_DRIP3 = 0;
int16_t SPR_WATER_DRIP4 = 0;

int16_t SPR_DECO_ARC_1 = 0;
int16_t SPR_DECO_ARC_2 = 0;
int16_t SPR_DECO_ARC_3 = 0;

int16_t SPR_GRATE = 0;
int16_t SPR_STEAM_1 = 0;
int16_t SPR_STEAM_2 = 0;
int16_t SPR_STEAM_3 = 0;
int16_t SPR_STEAM_4 = 0;

int16_t SPR_STEAM_PIPE = 0;
int16_t SPR_PIPE_STEAM_1 = 0;
int16_t SPR_PIPE_STEAM_2 = 0;
int16_t SPR_PIPE_STEAM_3 = 0;
int16_t SPR_PIPE_STEAM_4 = 0;

int16_t SPR_DEAD_RENT = 0;
int16_t SPR_DEAD_PRO = 0;
int16_t SPR_DEAD_SWAT = 0;

int16_t SPR_RENT_S_1 = 0;
int16_t SPR_RENT_S_2 = 0;
int16_t SPR_RENT_S_3 = 0;
int16_t SPR_RENT_S_4 = 0;
int16_t SPR_RENT_S_5 = 0;
int16_t SPR_RENT_S_6 = 0;
int16_t SPR_RENT_S_7 = 0;
int16_t SPR_RENT_S_8 = 0;

int16_t SPR_RENT_W1_1 = 0;
int16_t SPR_RENT_W1_2 = 0;
int16_t SPR_RENT_W1_3 = 0;
int16_t SPR_RENT_W1_4 = 0;
int16_t SPR_RENT_W1_5 = 0;
int16_t SPR_RENT_W1_6 = 0;
int16_t SPR_RENT_W1_7 = 0;
int16_t SPR_RENT_W1_8 = 0;

int16_t SPR_RENT_W2_1 = 0;
int16_t SPR_RENT_W2_2 = 0;
int16_t SPR_RENT_W2_3 = 0;
int16_t SPR_RENT_W2_4 = 0;
int16_t SPR_RENT_W2_5 = 0;
int16_t SPR_RENT_W2_6 = 0;
int16_t SPR_RENT_W2_7 = 0;
int16_t SPR_RENT_W2_8 = 0;

int16_t SPR_RENT_W3_1 = 0;
int16_t SPR_RENT_W3_2 = 0;
int16_t SPR_RENT_W3_3 = 0;
int16_t SPR_RENT_W3_4 = 0;
int16_t SPR_RENT_W3_5 = 0;
int16_t SPR_RENT_W3_6 = 0;
int16_t SPR_RENT_W3_7 = 0;
int16_t SPR_RENT_W3_8 = 0;

int16_t SPR_RENT_W4_1 = 0;
int16_t SPR_RENT_W4_2 = 0;
int16_t SPR_RENT_W4_3 = 0;
int16_t SPR_RENT_W4_4 = 0;
int16_t SPR_RENT_W4_5 = 0;
int16_t SPR_RENT_W4_6 = 0;
int16_t SPR_RENT_W4_7 = 0;
int16_t SPR_RENT_W4_8 = 0;

int16_t SPR_RENT_DIE_1 = 0;
int16_t SPR_RENT_DIE_2 = 0;
int16_t SPR_RENT_DIE_3 = 0;
int16_t SPR_RENT_DIE_4 = 0;
int16_t SPR_RENT_PAIN_1 = 0;
int16_t SPR_RENT_DEAD = 0;

int16_t SPR_RENT_SHOOT1 = 0;
int16_t SPR_RENT_SHOOT2 = 0;
int16_t SPR_RENT_SHOOT3 = 0;

int16_t SPR_PRO_S_1 = 0;
int16_t SPR_PRO_S_2 = 0;
int16_t SPR_PRO_S_3 = 0;
int16_t SPR_PRO_S_4 = 0;
int16_t SPR_PRO_S_5 = 0;
int16_t SPR_PRO_S_6 = 0;
int16_t SPR_PRO_S_7 = 0;
int16_t SPR_PRO_S_8 = 0;

int16_t SPR_PRO_W1_1 = 0;
int16_t SPR_PRO_W1_2 = 0;
int16_t SPR_PRO_W1_3 = 0;
int16_t SPR_PRO_W1_4 = 0;
int16_t SPR_PRO_W1_5 = 0;
int16_t SPR_PRO_W1_6 = 0;
int16_t SPR_PRO_W1_7 = 0;
int16_t SPR_PRO_W1_8 = 0;

int16_t SPR_PRO_W2_1 = 0;
int16_t SPR_PRO_W2_2 = 0;
int16_t SPR_PRO_W2_3 = 0;
int16_t SPR_PRO_W2_4 = 0;
int16_t SPR_PRO_W2_5 = 0;
int16_t SPR_PRO_W2_6 = 0;
int16_t SPR_PRO_W2_7 = 0;
int16_t SPR_PRO_W2_8 = 0;

int16_t SPR_PRO_W3_1 = 0;
int16_t SPR_PRO_W3_2 = 0;
int16_t SPR_PRO_W3_3 = 0;
int16_t SPR_PRO_W3_4 = 0;
int16_t SPR_PRO_W3_5 = 0;
int16_t SPR_PRO_W3_6 = 0;
int16_t SPR_PRO_W3_7 = 0;
int16_t SPR_PRO_W3_8 = 0;

int16_t SPR_PRO_W4_1 = 0;
int16_t SPR_PRO_W4_2 = 0;
int16_t SPR_PRO_W4_3 = 0;
int16_t SPR_PRO_W4_4 = 0;
int16_t SPR_PRO_W4_5 = 0;
int16_t SPR_PRO_W4_6 = 0;
int16_t SPR_PRO_W4_7 = 0;
int16_t SPR_PRO_W4_8 = 0;

int16_t SPR_PRO_PAIN_1 = 0;
int16_t SPR_PRO_DIE_1 = 0;
int16_t SPR_PRO_DIE_2 = 0;
int16_t SPR_PRO_DIE_3 = 0;
int16_t SPR_PRO_PAIN_2 = 0;
int16_t SPR_PRO_DIE_4 = 0;
int16_t SPR_PRO_DEAD = 0;

int16_t SPR_PRO_SHOOT1 = 0;
int16_t SPR_PRO_SHOOT2 = 0;
int16_t SPR_PRO_SHOOT3 = 0;

int16_t SPR_SWAT_S_1 = 0;
int16_t SPR_SWAT_S_2 = 0;
int16_t SPR_SWAT_S_3 = 0;
int16_t SPR_SWAT_S_4 = 0;
int16_t SPR_SWAT_S_5 = 0;
int16_t SPR_SWAT_S_6 = 0;
int16_t SPR_SWAT_S_7 = 0;
int16_t SPR_SWAT_S_8 = 0;

int16_t SPR_SWAT_W1_1 = 0;
int16_t SPR_SWAT_W1_2 = 0;
int16_t SPR_SWAT_W1_3 = 0;
int16_t SPR_SWAT_W1_4 = 0;
int16_t SPR_SWAT_W1_5 = 0;
int16_t SPR_SWAT_W1_6 = 0;
int16_t SPR_SWAT_W1_7 = 0;
int16_t SPR_SWAT_W1_8 = 0;

int16_t SPR_SWAT_W2_1 = 0;
int16_t SPR_SWAT_W2_2 = 0;
int16_t SPR_SWAT_W2_3 = 0;
int16_t SPR_SWAT_W2_4 = 0;
int16_t SPR_SWAT_W2_5 = 0;
int16_t SPR_SWAT_W2_6 = 0;
int16_t SPR_SWAT_W2_7 = 0;
int16_t SPR_SWAT_W2_8 = 0;

int16_t SPR_SWAT_W3_1 = 0;
int16_t SPR_SWAT_W3_2 = 0;
int16_t SPR_SWAT_W3_3 = 0;
int16_t SPR_SWAT_W3_4 = 0;
int16_t SPR_SWAT_W3_5 = 0;
int16_t SPR_SWAT_W3_6 = 0;
int16_t SPR_SWAT_W3_7 = 0;
int16_t SPR_SWAT_W3_8 = 0;

int16_t SPR_SWAT_W4_1 = 0;
int16_t SPR_SWAT_W4_2 = 0;
int16_t SPR_SWAT_W4_3 = 0;
int16_t SPR_SWAT_W4_4 = 0;
int16_t SPR_SWAT_W4_5 = 0;
int16_t SPR_SWAT_W4_6 = 0;
int16_t SPR_SWAT_W4_7 = 0;
int16_t SPR_SWAT_W4_8 = 0;

int16_t SPR_SWAT_PAIN_1 = 0;
int16_t SPR_SWAT_DIE_1 = 0;
int16_t SPR_SWAT_DIE_2 = 0;
int16_t SPR_SWAT_DIE_3 = 0;
int16_t SPR_SWAT_PAIN_2 = 0;
int16_t SPR_SWAT_DIE_4 = 0;
int16_t SPR_SWAT_DEAD = 0;

int16_t SPR_SWAT_SHOOT1 = 0;
int16_t SPR_SWAT_SHOOT2 = 0;
int16_t SPR_SWAT_SHOOT3 = 0;

int16_t SPR_SWAT_WOUNDED1 = 0;
int16_t SPR_SWAT_WOUNDED2 = 0;
int16_t SPR_SWAT_WOUNDED3 = 0;
int16_t SPR_SWAT_WOUNDED4 = 0;

int16_t SPR_OFC_S_1 = 0;
int16_t SPR_OFC_S_2 = 0;
int16_t SPR_OFC_S_3 = 0;
int16_t SPR_OFC_S_4 = 0;
int16_t SPR_OFC_S_5 = 0;
int16_t SPR_OFC_S_6 = 0;
int16_t SPR_OFC_S_7 = 0;
int16_t SPR_OFC_S_8 = 0;

int16_t SPR_OFC_W1_1 = 0;
int16_t SPR_OFC_W1_2 = 0;
int16_t SPR_OFC_W1_3 = 0;
int16_t SPR_OFC_W1_4 = 0;
int16_t SPR_OFC_W1_5 = 0;
int16_t SPR_OFC_W1_6 = 0;
int16_t SPR_OFC_W1_7 = 0;
int16_t SPR_OFC_W1_8 = 0;

int16_t SPR_OFC_W2_1 = 0;
int16_t SPR_OFC_W2_2 = 0;
int16_t SPR_OFC_W2_3 = 0;
int16_t SPR_OFC_W2_4 = 0;
int16_t SPR_OFC_W2_5 = 0;
int16_t SPR_OFC_W2_6 = 0;
int16_t SPR_OFC_W2_7 = 0;
int16_t SPR_OFC_W2_8 = 0;

int16_t SPR_OFC_W3_1 = 0;
int16_t SPR_OFC_W3_2 = 0;
int16_t SPR_OFC_W3_3 = 0;
int16_t SPR_OFC_W3_4 = 0;
int16_t SPR_OFC_W3_5 = 0;
int16_t SPR_OFC_W3_6 = 0;
int16_t SPR_OFC_W3_7 = 0;
int16_t SPR_OFC_W3_8 = 0;

int16_t SPR_OFC_W4_1 = 0;
int16_t SPR_OFC_W4_2 = 0;
int16_t SPR_OFC_W4_3 = 0;
int16_t SPR_OFC_W4_4 = 0;
int16_t SPR_OFC_W4_5 = 0;
int16_t SPR_OFC_W4_6 = 0;
int16_t SPR_OFC_W4_7 = 0;
int16_t SPR_OFC_W4_8 = 0;

int16_t SPR_OFC_PAIN_1 = 0;
int16_t SPR_OFC_DIE_1 = 0;
int16_t SPR_OFC_DIE_2 = 0;
int16_t SPR_OFC_DIE_3 = 0;
int16_t SPR_OFC_PAIN_2 = 0;
int16_t SPR_OFC_DIE_4 = 0;
int16_t SPR_OFC_DEAD = 0;

int16_t SPR_OFC_SHOOT1 = 0;
int16_t SPR_OFC_SHOOT2 = 0;
int16_t SPR_OFC_SHOOT3 = 0;

int16_t SPR_GOLD_S_1 = 0;
int16_t SPR_GOLD_S_2 = 0;
int16_t SPR_GOLD_S_3 = 0;
int16_t SPR_GOLD_S_4 = 0;
int16_t SPR_GOLD_S_5 = 0;
int16_t SPR_GOLD_S_6 = 0;
int16_t SPR_GOLD_S_7 = 0;
int16_t SPR_GOLD_S_8 = 0;

int16_t SPR_GOLD_W1_1 = 0;
int16_t SPR_GOLD_W1_2 = 0;
int16_t SPR_GOLD_W1_3 = 0;
int16_t SPR_GOLD_W1_4 = 0;
int16_t SPR_GOLD_W1_5 = 0;
int16_t SPR_GOLD_W1_6 = 0;
int16_t SPR_GOLD_W1_7 = 0;
int16_t SPR_GOLD_W1_8 = 0;

int16_t SPR_GOLD_W2_1 = 0;
int16_t SPR_GOLD_W2_2 = 0;
int16_t SPR_GOLD_W2_3 = 0;
int16_t SPR_GOLD_W2_4 = 0;
int16_t SPR_GOLD_W2_5 = 0;
int16_t SPR_GOLD_W2_6 = 0;
int16_t SPR_GOLD_W2_7 = 0;
int16_t SPR_GOLD_W2_8 = 0;

int16_t SPR_GOLD_W3_1 = 0;
int16_t SPR_GOLD_W3_2 = 0;
int16_t SPR_GOLD_W3_3 = 0;
int16_t SPR_GOLD_W3_4 = 0;
int16_t SPR_GOLD_W3_5 = 0;
int16_t SPR_GOLD_W3_6 = 0;
int16_t SPR_GOLD_W3_7 = 0;
int16_t SPR_GOLD_W3_8 = 0;

int16_t SPR_GOLD_W4_1 = 0;
int16_t SPR_GOLD_W4_2 = 0;
int16_t SPR_GOLD_W4_3 = 0;
int16_t SPR_GOLD_W4_4 = 0;
int16_t SPR_GOLD_W4_5 = 0;
int16_t SPR_GOLD_W4_6 = 0;
int16_t SPR_GOLD_W4_7 = 0;
int16_t SPR_GOLD_W4_8 = 0;

int16_t SPR_GOLD_PAIN_1 = 0;

int16_t SPR_GOLD_WRIST_1 = 0;
int16_t SPR_GOLD_WRIST_2 = 0;

int16_t SPR_GOLD_SHOOT1 = 0;
int16_t SPR_GOLD_SHOOT2 = 0;
int16_t SPR_GOLD_SHOOT3 = 0;

int16_t SPR_GOLD_WARP1 = 0;
int16_t SPR_GOLD_WARP2 = 0;
int16_t SPR_GOLD_WARP3 = 0;
int16_t SPR_GOLD_WARP4 = 0;
int16_t SPR_GOLD_WARP5 = 0;

int16_t SPR_GOLD_DEATH1 = 0;
int16_t SPR_GOLD_DEATH2 = 0;
int16_t SPR_GOLD_DEATH3 = 0;
int16_t SPR_GOLD_DEATH4 = 0;
int16_t SPR_GOLD_DEATH5 = 0;
int16_t SPR_MGOLD_OUCH = 0;

int16_t SPR_GOLD_MORPH1 = 0;
int16_t SPR_GOLD_MORPH2 = 0;
int16_t SPR_GOLD_MORPH3 = 0;
int16_t SPR_GOLD_MORPH4 = 0;
int16_t SPR_GOLD_MORPH5 = 0;
int16_t SPR_GOLD_MORPH6 = 0;
int16_t SPR_GOLD_MORPH7 = 0;
int16_t SPR_GOLD_MORPH8 = 0;

int16_t SPR_MGOLD_WALK1 = 0;
int16_t SPR_MGOLD_WALK2 = 0;
int16_t SPR_MGOLD_WALK3 = 0;
int16_t SPR_MGOLD_WALK4 = 0;
int16_t SPR_MGOLD_ATTACK1 = 0;
int16_t SPR_MGOLD_ATTACK2 = 0;
int16_t SPR_MGOLD_ATTACK3 = 0;
int16_t SPR_MGOLD_ATTACK4 = 0;

int16_t SPR_MGOLD_SHOT1 = 0;
int16_t SPR_MGOLD_SHOT2 = 0;
int16_t SPR_MGOLD_SHOT3 = 0;
int16_t SPR_MGOLD_SHOT_EXP1 = 0;
int16_t SPR_MGOLD_SHOT_EXP2 = 0;
int16_t SPR_MGOLD_SHOT_EXP3 = 0;

int16_t SPR_GSCOUT_W1_1 = 0;
int16_t SPR_GSCOUT_W1_2 = 0;
int16_t SPR_GSCOUT_W1_3 = 0;
int16_t SPR_GSCOUT_W1_4 = 0;
int16_t SPR_GSCOUT_W1_5 = 0;
int16_t SPR_GSCOUT_W1_6 = 0;
int16_t SPR_GSCOUT_W1_7 = 0;
int16_t SPR_GSCOUT_W1_8 = 0;

int16_t SPR_GSCOUT_W2_1 = 0;
int16_t SPR_GSCOUT_W2_2 = 0;
int16_t SPR_GSCOUT_W2_3 = 0;
int16_t SPR_GSCOUT_W2_4 = 0;
int16_t SPR_GSCOUT_W2_5 = 0;
int16_t SPR_GSCOUT_W2_6 = 0;
int16_t SPR_GSCOUT_W2_7 = 0;
int16_t SPR_GSCOUT_W2_8 = 0;

int16_t SPR_GSCOUT_W3_1 = 0;
int16_t SPR_GSCOUT_W3_2 = 0;
int16_t SPR_GSCOUT_W3_3 = 0;
int16_t SPR_GSCOUT_W3_4 = 0;
int16_t SPR_GSCOUT_W3_5 = 0;
int16_t SPR_GSCOUT_W3_6 = 0;
int16_t SPR_GSCOUT_W3_7 = 0;
int16_t SPR_GSCOUT_W3_8 = 0;

int16_t SPR_GSCOUT_W4_1 = 0;
int16_t SPR_GSCOUT_W4_2 = 0;
int16_t SPR_GSCOUT_W4_3 = 0;
int16_t SPR_GSCOUT_W4_4 = 0;
int16_t SPR_GSCOUT_W4_5 = 0;
int16_t SPR_GSCOUT_W4_6 = 0;
int16_t SPR_GSCOUT_W4_7 = 0;
int16_t SPR_GSCOUT_W4_8 = 0;

int16_t SPR_GSCOUT_DIE1 = 0;
int16_t SPR_GSCOUT_DIE2 = 0;
int16_t SPR_GSCOUT_DIE3 = 0;
int16_t SPR_GSCOUT_DIE4 = 0;
int16_t SPR_GSCOUT_DIE5 = 0;
int16_t SPR_GSCOUT_DIE6 = 0;
int16_t SPR_GSCOUT_DIE7 = 0;
int16_t SPR_GSCOUT_DIE8 = 0;

int16_t SPR_GSCOUT_DEAD = 0;

int16_t SPR_FSCOUT_W1_1 = 0;
int16_t SPR_FSCOUT_W1_2 = 0;
int16_t SPR_FSCOUT_W1_3 = 0;
int16_t SPR_FSCOUT_W1_4 = 0;
int16_t SPR_FSCOUT_W1_5 = 0;
int16_t SPR_FSCOUT_W1_6 = 0;
int16_t SPR_FSCOUT_W1_7 = 0;
int16_t SPR_FSCOUT_W1_8 = 0;

int16_t SPR_FSCOUT_W2_1 = 0;
int16_t SPR_FSCOUT_W2_2 = 0;
int16_t SPR_FSCOUT_W2_3 = 0;
int16_t SPR_FSCOUT_W2_4 = 0;
int16_t SPR_FSCOUT_W2_5 = 0;
int16_t SPR_FSCOUT_W2_6 = 0;
int16_t SPR_FSCOUT_W2_7 = 0;
int16_t SPR_FSCOUT_W2_8 = 0;

int16_t SPR_FSCOUT_W3_1 = 0;
int16_t SPR_FSCOUT_W3_2 = 0;
int16_t SPR_FSCOUT_W3_3 = 0;
int16_t SPR_FSCOUT_W3_4 = 0;
int16_t SPR_FSCOUT_W3_5 = 0;
int16_t SPR_FSCOUT_W3_6 = 0;
int16_t SPR_FSCOUT_W3_7 = 0;
int16_t SPR_FSCOUT_W3_8 = 0;

int16_t SPR_FSCOUT_W4_1 = 0;
int16_t SPR_FSCOUT_W4_2 = 0;
int16_t SPR_FSCOUT_W4_3 = 0;
int16_t SPR_FSCOUT_W4_4 = 0;
int16_t SPR_FSCOUT_W4_5 = 0;
int16_t SPR_FSCOUT_W4_6 = 0;
int16_t SPR_FSCOUT_W4_7 = 0;
int16_t SPR_FSCOUT_W4_8 = 0;

int16_t SPR_FSCOUT_DIE1 = 0;
int16_t SPR_FSCOUT_DIE2 = 0;
int16_t SPR_FSCOUT_DIE3 = 0;
int16_t SPR_FSCOUT_DIE4 = 0;
int16_t SPR_FSCOUT_DIE5 = 0;
int16_t SPR_FSCOUT_DIE6 = 0;
int16_t SPR_FSCOUT_DIE7 = 0;
int16_t SPR_FSCOUT_DEAD = 0;

int16_t SPR_EXPLOSION_1 = 0;
int16_t SPR_EXPLOSION_2 = 0;
int16_t SPR_EXPLOSION_3 = 0;
int16_t SPR_EXPLOSION_4 = 0;
int16_t SPR_EXPLOSION_5 = 0;

int16_t SPR_VITAL_STAND = 0;
int16_t SPR_VITAL_DIE_1 = 0;
int16_t SPR_VITAL_DIE_2 = 0;
int16_t SPR_VITAL_DIE_3 = 0;
int16_t SPR_VITAL_DIE_4 = 0;
int16_t SPR_VITAL_DIE_5 = 0;
int16_t SPR_VITAL_DIE_6 = 0;
int16_t SPR_VITAL_DIE_7 = 0;
int16_t SPR_VITAL_DIE_8 = 0;
int16_t SPR_VITAL_DEAD_1 = 0;
int16_t SPR_VITAL_DEAD_2 = 0;
int16_t SPR_VITAL_DEAD_3 = 0;
int16_t SPR_VITAL_OUCH = 0;

int16_t SPR_CUBE1 = 0;
int16_t SPR_CUBE2 = 0;
int16_t SPR_CUBE3 = 0;
int16_t SPR_CUBE4 = 0;
int16_t SPR_CUBE5 = 0;
int16_t SPR_CUBE6 = 0;
int16_t SPR_CUBE7 = 0;
int16_t SPR_CUBE8 = 0;
int16_t SPR_CUBE9 = 0;
int16_t SPR_CUBE10 = 0;
int16_t SPR_CUBE_EXP1 = 0;
int16_t SPR_CUBE_EXP2 = 0;
int16_t SPR_CUBE_EXP3 = 0;
int16_t SPR_CUBE_EXP4 = 0;
int16_t SPR_CUBE_EXP5 = 0;
int16_t SPR_CUBE_EXP6 = 0;
int16_t SPR_CUBE_EXP7 = 0;
int16_t SPR_CUBE_EXP8 = 0;
int16_t SPR_DEAD_CUBE = 0;

int16_t SPR_SECURITY_NORMAL = 0;
int16_t SPR_SECURITY_ALERT = 0;

int16_t SPR_POD_EGG = 0;
int16_t SPR_POD_HATCH1 = 0;
int16_t SPR_POD_HATCH2 = 0;
int16_t SPR_POD_HATCH3 = 0;
int16_t SPR_POD_WALK1 = 0;
int16_t SPR_POD_WALK2 = 0;
int16_t SPR_POD_WALK3 = 0;
int16_t SPR_POD_WALK4 = 0;
int16_t SPR_POD_ATTACK1 = 0;
int16_t SPR_POD_ATTACK2 = 0;
int16_t SPR_POD_ATTACK3 = 0;
int16_t SPR_POD_OUCH = 0;
int16_t SPR_POD_DIE1 = 0;
int16_t SPR_POD_DIE2 = 0;
int16_t SPR_POD_DIE3 = 0;
int16_t SPR_POD_SPIT1 = 0;
int16_t SPR_POD_SPIT2 = 0;
int16_t SPR_POD_SPIT3 = 0;

int16_t SPR_ELEC_APPEAR1 = 0;
int16_t SPR_ELEC_APPEAR2 = 0;
int16_t SPR_ELEC_APPEAR3 = 0;
int16_t SPR_ELEC_WALK1 = 0;
int16_t SPR_ELEC_WALK2 = 0;
int16_t SPR_ELEC_WALK3 = 0;
int16_t SPR_ELEC_WALK4 = 0;
int16_t SPR_ELEC_OUCH = 0;
int16_t SPR_ELEC_SHOOT1 = 0;
int16_t SPR_ELEC_SHOOT2 = 0;
int16_t SPR_ELEC_SHOOT3 = 0;
int16_t SPR_ELEC_DIE1 = 0;
int16_t SPR_ELEC_DIE2 = 0;
int16_t SPR_ELEC_DIE3 = 0;
int16_t SPR_ELEC_SHOT1 = 0;
int16_t SPR_ELEC_SHOT2 = 0;
int16_t SPR_ELEC_SHOT_EXP1 = 0;
int16_t SPR_ELEC_SHOT_EXP2 = 0;

int16_t SPR_ELECTRO_SPHERE_ROAM1 = 0;
int16_t SPR_ELECTRO_SPHERE_ROAM2 = 0;
int16_t SPR_ELECTRO_SPHERE_ROAM3 = 0;
int16_t SPR_ELECTRO_SPHERE_OUCH = 0;
int16_t SPR_ELECTRO_SPHERE_DIE1 = 0;
int16_t SPR_ELECTRO_SPHERE_DIE2 = 0;
int16_t SPR_ELECTRO_SPHERE_DIE3 = 0;
int16_t SPR_ELECTRO_SPHERE_DIE4 = 0;

int16_t SPR_GENETIC_W1 = 0;
int16_t SPR_GENETIC_W2 = 0;
int16_t SPR_GENETIC_W3 = 0;
int16_t SPR_GENETIC_W4 = 0;
int16_t SPR_GENETIC_SWING1 = 0;
int16_t SPR_GENETIC_SWING2 = 0;
int16_t SPR_GENETIC_SWING3 = 0;
int16_t SPR_GENETIC_DEAD = 0;
int16_t SPR_GENETIC_DIE1 = 0;
int16_t SPR_GENETIC_DIE2 = 0;
int16_t SPR_GENETIC_DIE3 = 0;
int16_t SPR_GENETIC_DIE4 = 0;
int16_t SPR_GENETIC_OUCH = 0;
int16_t SPR_GENETIC_SHOOT1 = 0;
int16_t SPR_GENETIC_SHOOT2 = 0;
int16_t SPR_GENETIC_SHOOT3 = 0;

int16_t SPR_MUTHUM1_W1 = 0;
int16_t SPR_MUTHUM1_W2 = 0;
int16_t SPR_MUTHUM1_W3 = 0;
int16_t SPR_MUTHUM1_W4 = 0;
int16_t SPR_MUTHUM1_SWING1 = 0;
int16_t SPR_MUTHUM1_SWING2 = 0;
int16_t SPR_MUTHUM1_SWING3 = 0;
int16_t SPR_MUTHUM1_DEAD = 0;
int16_t SPR_MUTHUM1_DIE1 = 0;
int16_t SPR_MUTHUM1_DIE2 = 0;
int16_t SPR_MUTHUM1_DIE3 = 0;
int16_t SPR_MUTHUM1_DIE4 = 0;
int16_t SPR_MUTHUM1_OUCH = 0;
int16_t SPR_MUTHUM1_SPIT1 = 0;
int16_t SPR_MUTHUM1_SPIT2 = 0;
int16_t SPR_MUTHUM1_SPIT3 = 0;

int16_t SPR_MUTHUM2_W1 = 0;
int16_t SPR_MUTHUM2_W2 = 0;
int16_t SPR_MUTHUM2_W3 = 0;
int16_t SPR_MUTHUM2_W4 = 0;
int16_t SPR_MUTHUM2_SWING1 = 0;
int16_t SPR_MUTHUM2_SWING2 = 0;
int16_t SPR_MUTHUM2_SWING3 = 0;
int16_t SPR_MUTHUM2_DEAD = 0;
int16_t SPR_MUTHUM2_DIE1 = 0;
int16_t SPR_MUTHUM2_DIE2 = 0;
int16_t SPR_MUTHUM2_DIE3 = 0;
int16_t SPR_MUTHUM2_DIE4 = 0;
int16_t SPR_MUTHUM2_OUCH = 0;
int16_t SPR_MUTHUM2_SPIT1 = 0;
int16_t SPR_MUTHUM2_SPIT2 = 0;
int16_t SPR_MUTHUM2_SPIT3 = 0;

int16_t SPR_MUTHUM2_MORPH1 = 0;
int16_t SPR_MUTHUM2_MORPH2 = 0;
int16_t SPR_MUTHUM2_MORPH3 = 0;
int16_t SPR_MUTHUM2_MORPH4 = 0;
int16_t SPR_MUTHUM2_MORPH5 = 0;
int16_t SPR_MUTHUM2_MORPH6 = 0;
int16_t SPR_MUTHUM2_MORPH7 = 0;
int16_t SPR_MUTHUM2_MORPH8 = 0;
int16_t SPR_MUTHUM2_MORPH9 = 0;

int16_t SPR_LCAN_ALIEN_READY = 0;
int16_t SPR_LCAN_ALIEN_B1 = 0;
int16_t SPR_LCAN_ALIEN_B2 = 0;
int16_t SPR_LCAN_ALIEN_B3 = 0;
int16_t SPR_LCAN_ALIEN_EMPTY = 0;

int16_t SPR_LCAN_ALIEN_W1 = 0;
int16_t SPR_LCAN_ALIEN_W2 = 0;
int16_t SPR_LCAN_ALIEN_W3 = 0;
int16_t SPR_LCAN_ALIEN_W4 = 0;
int16_t SPR_LCAN_ALIEN_SWING1 = 0;
int16_t SPR_LCAN_ALIEN_SWING2 = 0;
int16_t SPR_LCAN_ALIEN_SWING3 = 0;
int16_t SPR_LCAN_ALIEN_DEAD = 0;
int16_t SPR_LCAN_ALIEN_DIE1 = 0;
int16_t SPR_LCAN_ALIEN_DIE2 = 0;
int16_t SPR_LCAN_ALIEN_DIE3 = 0;
int16_t SPR_LCAN_ALIEN_DIE4 = 0;
int16_t SPR_LCAN_ALIEN_OUCH = 0;
int16_t SPR_LCAN_ALIEN_SPIT1 = 0;
int16_t SPR_LCAN_ALIEN_SPIT2 = 0;
int16_t SPR_LCAN_ALIEN_SPIT3 = 0;

int16_t SPR_SCAN_ALIEN_READY = 0;
int16_t SPR_SCAN_ALIEN_B1 = 0;
int16_t SPR_SCAN_ALIEN_B2 = 0;
int16_t SPR_SCAN_ALIEN_B3 = 0;
int16_t SPR_SCAN_ALIEN_EMPTY = 0;

int16_t SPR_SCAN_ALIEN_W1 = 0;
int16_t SPR_SCAN_ALIEN_W2 = 0;
int16_t SPR_SCAN_ALIEN_W3 = 0;
int16_t SPR_SCAN_ALIEN_W4 = 0;
int16_t SPR_SCAN_ALIEN_SWING1 = 0;
int16_t SPR_SCAN_ALIEN_SWING2 = 0;
int16_t SPR_SCAN_ALIEN_SWING3 = 0;
int16_t SPR_SCAN_ALIEN_DEAD = 0;
int16_t SPR_SCAN_ALIEN_DIE1 = 0;
int16_t SPR_SCAN_ALIEN_DIE2 = 0;
int16_t SPR_SCAN_ALIEN_DIE3 = 0;
int16_t SPR_SCAN_ALIEN_DIE4 = 0;
int16_t SPR_SCAN_ALIEN_OUCH = 0;

int16_t SPR_SCAN_ALIEN_SPIT1 = 0;
int16_t SPR_SCAN_ALIEN_SPIT2 = 0;
int16_t SPR_SCAN_ALIEN_SPIT3 = 0;

int16_t SPR_GURNEY_MUT_READY = 0;
int16_t SPR_GURNEY_MUT_B1 = 0;
int16_t SPR_GURNEY_MUT_B2 = 0;
int16_t SPR_GURNEY_MUT_B3 = 0;
int16_t SPR_GURNEY_MUT_EMPTY = 0;

int16_t SPR_GURNEY_MUT_W1 = 0;
int16_t SPR_GURNEY_MUT_W2 = 0;
int16_t SPR_GURNEY_MUT_W3 = 0;
int16_t SPR_GURNEY_MUT_W4 = 0;
int16_t SPR_GURNEY_MUT_SWING1 = 0;
int16_t SPR_GURNEY_MUT_SWING2 = 0;
int16_t SPR_GURNEY_MUT_SWING3 = 0;
int16_t SPR_GURNEY_MUT_DEAD = 0;
int16_t SPR_GURNEY_MUT_DIE1 = 0;
int16_t SPR_GURNEY_MUT_DIE2 = 0;
int16_t SPR_GURNEY_MUT_DIE3 = 0;
int16_t SPR_GURNEY_MUT_DIE4 = 0;
int16_t SPR_GURNEY_MUT_OUCH = 0;

int16_t SPR_LIQUID_M1 = 0;
int16_t SPR_LIQUID_M2 = 0;
int16_t SPR_LIQUID_M3 = 0;
int16_t SPR_LIQUID_R1 = 0;
int16_t SPR_LIQUID_R2 = 0;
int16_t SPR_LIQUID_R3 = 0;
int16_t SPR_LIQUID_R4 = 0;
int16_t SPR_LIQUID_S1 = 0;
int16_t SPR_LIQUID_S2 = 0;
int16_t SPR_LIQUID_S3 = 0;
int16_t SPR_LIQUID_OUCH = 0;
int16_t SPR_LIQUID_DIE_1 = 0;
int16_t SPR_LIQUID_DIE_2 = 0;
int16_t SPR_LIQUID_DIE_3 = 0;
int16_t SPR_LIQUID_DIE_4 = 0;
int16_t SPR_LIQUID_DEAD = 0;
int16_t SPR_LIQUID_SHOT_FLY_1 = 0;
int16_t SPR_LIQUID_SHOT_FLY_2 = 0;
int16_t SPR_LIQUID_SHOT_FLY_3 = 0;
int16_t SPR_LIQUID_SHOT_BURST_1 = 0;
int16_t SPR_LIQUID_SHOT_BURST_2 = 0;
int16_t SPR_LIQUID_SHOT_BURST_3 = 0;

int16_t SPR_SPIT1_1 = 0;
int16_t SPR_SPIT1_2 = 0;
int16_t SPR_SPIT1_3 = 0;
int16_t SPR_SPIT_EXP1_1 = 0;
int16_t SPR_SPIT_EXP1_2 = 0;
int16_t SPR_SPIT_EXP1_3 = 0;

int16_t SPR_SPIT2_1 = 0;
int16_t SPR_SPIT2_2 = 0;
int16_t SPR_SPIT2_3 = 0;
int16_t SPR_SPIT_EXP2_1 = 0;
int16_t SPR_SPIT_EXP2_2 = 0;
int16_t SPR_SPIT_EXP2_3 = 0;

int16_t SPR_SPIT3_1 = 0;
int16_t SPR_SPIT3_2 = 0;
int16_t SPR_SPIT3_3 = 0;
int16_t SPR_SPIT_EXP3_1 = 0;
int16_t SPR_SPIT_EXP3_2 = 0;
int16_t SPR_SPIT_EXP3_3 = 0;

int16_t SPR_TERROT_1 = 0;
int16_t SPR_TERROT_2 = 0;
int16_t SPR_TERROT_3 = 0;
int16_t SPR_TERROT_4 = 0;
int16_t SPR_TERROT_5 = 0;
int16_t SPR_TERROT_6 = 0;
int16_t SPR_TERROT_7 = 0;
int16_t SPR_TERROT_8 = 0;

int16_t SPR_TERROT_FIRE_1 = 0;
int16_t SPR_TERROT_FIRE_2 = 0;
int16_t SPR_TERROT_DIE_1 = 0;
int16_t SPR_TERROT_DIE_2 = 0;
int16_t SPR_TERROT_DIE_3 = 0;
int16_t SPR_TERROT_DIE_4 = 0;
int16_t SPR_TERROT_DEAD = 0;

int16_t SPR_KNIFEREADY = 0;
int16_t SPR_KNIFEATK1 = 0;
int16_t SPR_KNIFEATK2 = 0;
int16_t SPR_KNIFEATK3 = 0;
int16_t SPR_KNIFEATK4 = 0;

int16_t SPR_PISTOLREADY = 0;
int16_t SPR_PISTOLATK1 = 0;
int16_t SPR_PISTOLATK2 = 0;
int16_t SPR_PISTOLATK3 = 0;
int16_t SPR_PISTOLATK4 = 0;

int16_t SPR_MACHINEGUNREADY = 0;
int16_t SPR_MACHINEGUNATK1 = 0;
int16_t SPR_MACHINEGUNATK2 = 0;
int16_t SPR_MACHINEGUNATK3 = 0;
int16_t SPR_MACHINEGUNATK4 = 0;

int16_t SPR_CHAINREADY = 0;
int16_t SPR_CHAINATK1 = 0;
int16_t SPR_CHAINATK2 = 0;
int16_t SPR_CHAINATK3 = 0;
int16_t SPR_CHAINATK4 = 0;

int16_t SPR_GRENADEREADY = 0;
int16_t SPR_GRENADEATK1 = 0;
int16_t SPR_GRENADEATK2 = 0;
int16_t SPR_GRENADEATK3 = 0;
int16_t SPR_GRENADEATK4 = 0;

int16_t SPR_GRENADE_FLY1 = 0;
int16_t SPR_GRENADE_FLY2 = 0;
int16_t SPR_GRENADE_FLY3 = 0;
int16_t SPR_GRENADE_FLY4 = 0;
int16_t SPR_GRENADE_EXPLODE1 = 0;
int16_t SPR_GRENADE_EXPLODE2 = 0;
int16_t SPR_GRENADE_EXPLODE3 = 0;
int16_t SPR_GRENADE_EXPLODE4 = 0;
int16_t SPR_GRENADE_EXPLODE5 = 0;

int16_t SPR_ELEC_ARC1 = 0;
int16_t SPR_ELEC_ARC2 = 0;
int16_t SPR_ELEC_ARC3 = 0;
int16_t SPR_ELEC_ARC4 = 0;
int16_t SPR_ELEC_POST1 = 0;
int16_t SPR_ELEC_POST2 = 0;
int16_t SPR_ELEC_POST3 = 0;
int16_t SPR_ELEC_POST4 = 0;

int16_t SPR_VPOST1 = 0;
int16_t SPR_VPOST2 = 0;
int16_t SPR_VPOST3 = 0;
int16_t SPR_VPOST4 = 0;
int16_t SPR_VPOST5 = 0;
int16_t SPR_VPOST6 = 0;
int16_t SPR_VPOST7 = 0;
int16_t SPR_VPOST8 = 0;

int16_t SPR_VSPIKE1 = 0;
int16_t SPR_VSPIKE2 = 0;
int16_t SPR_VSPIKE3 = 0;
int16_t SPR_VSPIKE4 = 0;
int16_t SPR_VSPIKE5 = 0;
int16_t SPR_VSPIKE6 = 0;
int16_t SPR_VSPIKE7 = 0;
int16_t SPR_VSPIKE8 = 0;

int16_t SPR_GREEN_OOZE1 = 0;
int16_t SPR_GREEN_OOZE2 = 0;
int16_t SPR_GREEN_OOZE3 = 0;
int16_t SPR_BLACK_OOZE1 = 0;
int16_t SPR_BLACK_OOZE2 = 0;
int16_t SPR_BLACK_OOZE3 = 0;

int16_t SPR_GREEN2_OOZE1 = 0;
int16_t SPR_GREEN2_OOZE2 = 0;
int16_t SPR_GREEN2_OOZE3 = 0;
int16_t SPR_BLACK2_OOZE1 = 0;
int16_t SPR_BLACK2_OOZE2 = 0;
int16_t SPR_BLACK2_OOZE3 = 0;

int16_t SPR_CANDY_BAR = 0;
int16_t SPR_CANDY_WRAPER = 0;
int16_t SPR_SANDWICH = 0;
int16_t SPR_SANDWICH_WRAPER = 0;

int16_t SPR_BLAKE_W1 = 0;
int16_t SPR_BLAKE_W2 = 0;
int16_t SPR_BLAKE_W3 = 0;
int16_t SPR_BLAKE_W4 = 0;

int16_t SPR_BOSS1_W1 = 0;
int16_t SPR_BOSS1_W2 = 0;
int16_t SPR_BOSS1_W3 = 0;
int16_t SPR_BOSS1_W4 = 0;
int16_t SPR_BOSS1_SWING1 = 0;
int16_t SPR_BOSS1_SWING2 = 0;
int16_t SPR_BOSS1_SWING3 = 0;
int16_t SPR_BOSS1_DEAD = 0;
int16_t SPR_BOSS1_DIE1 = 0;
int16_t SPR_BOSS1_DIE2 = 0;
int16_t SPR_BOSS1_DIE3 = 0;
int16_t SPR_BOSS1_DIE4 = 0;
int16_t SPR_BOSS1_OUCH = 0;
int16_t SPR_BOSS1_PROJ1 = 0;
int16_t SPR_BOSS1_PROJ2 = 0;
int16_t SPR_BOSS1_PROJ3 = 0;
int16_t SPR_BOSS1_EXP1 = 0;
int16_t SPR_BOSS1_EXP2 = 0;
int16_t SPR_BOSS1_EXP3 = 0;
int16_t SPR_BOSS1_MORPH1 = 0;
int16_t SPR_BOSS1_MORPH2 = 0;
int16_t SPR_BOSS1_MORPH3 = 0;
int16_t SPR_BOSS1_MORPH4 = 0;
int16_t SPR_BOSS1_MORPH5 = 0;
int16_t SPR_BOSS1_MORPH6 = 0;
int16_t SPR_BOSS1_MORPH7 = 0;
int16_t SPR_BOSS1_MORPH8 = 0;
int16_t SPR_BOSS1_MORPH9 = 0;

int16_t SPR_BOSS2_W1 = 0;
int16_t SPR_BOSS2_W2 = 0;
int16_t SPR_BOSS2_W3 = 0;
int16_t SPR_BOSS2_W4 = 0;
int16_t SPR_BOSS2_SWING1 = 0;
int16_t SPR_BOSS2_SWING2 = 0;
int16_t SPR_BOSS2_SWING3 = 0;
int16_t SPR_BOSS2_DEAD = 0;
int16_t SPR_BOSS2_DIE1 = 0;
int16_t SPR_BOSS2_DIE2 = 0;
int16_t SPR_BOSS2_DIE3 = 0;
int16_t SPR_BOSS2_DIE4 = 0;
int16_t SPR_BOSS2_OUCH = 0;

int16_t SPR_BOSS3_W1 = 0;
int16_t SPR_BOSS3_W2 = 0;
int16_t SPR_BOSS3_W3 = 0;
int16_t SPR_BOSS3_W4 = 0;
int16_t SPR_BOSS3_SWING1 = 0;
int16_t SPR_BOSS3_SWING2 = 0;
int16_t SPR_BOSS3_SWING3 = 0;
int16_t SPR_BOSS3_DEAD = 0;
int16_t SPR_BOSS3_DIE1 = 0;
int16_t SPR_BOSS3_DIE2 = 0;
int16_t SPR_BOSS3_DIE3 = 0;
int16_t SPR_BOSS3_DIE4 = 0;
int16_t SPR_BOSS3_OUCH = 0;

int16_t SPR_BOSS4_W1 = 0;
int16_t SPR_BOSS4_W2 = 0;
int16_t SPR_BOSS4_W3 = 0;
int16_t SPR_BOSS4_W4 = 0;
int16_t SPR_BOSS4_SWING1 = 0;
int16_t SPR_BOSS4_SWING2 = 0;
int16_t SPR_BOSS4_SWING3 = 0;
int16_t SPR_BOSS4_DEAD = 0;
int16_t SPR_BOSS4_DIE1 = 0;
int16_t SPR_BOSS4_DIE2 = 0;
int16_t SPR_BOSS4_DIE3 = 0;
int16_t SPR_BOSS4_DIE4 = 0;
int16_t SPR_BOSS4_OUCH = 0;
int16_t SPR_BOSS4_MORPH1 = 0;
int16_t SPR_BOSS4_MORPH2 = 0;
int16_t SPR_BOSS4_MORPH3 = 0;
int16_t SPR_BOSS4_MORPH4 = 0;
int16_t SPR_BOSS4_MORPH5 = 0;
int16_t SPR_BOSS4_MORPH6 = 0;
int16_t SPR_BOSS4_MORPH7 = 0;
int16_t SPR_BOSS4_MORPH8 = 0;
int16_t SPR_BOSS4_MORPH9 = 0;

int16_t SPR_BOSS5_W1 = 0;
int16_t SPR_BOSS5_W2 = 0;
int16_t SPR_BOSS5_W3 = 0;
int16_t SPR_BOSS5_W4 = 0;
int16_t SPR_BOSS5_SWING1 = 0;
int16_t SPR_BOSS5_SWING2 = 0;
int16_t SPR_BOSS5_SWING3 = 0;
int16_t SPR_BOSS5_DEAD = 0;
int16_t SPR_BOSS5_DIE1 = 0;
int16_t SPR_BOSS5_DIE2 = 0;
int16_t SPR_BOSS5_DIE3 = 0;
int16_t SPR_BOSS5_DIE4 = 0;
int16_t SPR_BOSS5_OUCH = 0;
int16_t SPR_BOSS5_PROJ1 = 0;
int16_t SPR_BOSS5_PROJ2 = 0;
int16_t SPR_BOSS5_PROJ3 = 0;
int16_t SPR_BOSS5_EXP1 = 0;
int16_t SPR_BOSS5_EXP2 = 0;
int16_t SPR_BOSS5_EXP3 = 0;

int16_t SPR_BOSS6_W1 = 0;
int16_t SPR_BOSS6_W2 = 0;
int16_t SPR_BOSS6_W3 = 0;
int16_t SPR_BOSS6_W4 = 0;
int16_t SPR_BOSS6_SWING1 = 0;
int16_t SPR_BOSS6_SWING2 = 0;
int16_t SPR_BOSS6_SWING3 = 0;
int16_t SPR_BOSS6_DEAD = 0;
int16_t SPR_BOSS6_DIE1 = 0;
int16_t SPR_BOSS6_DIE2 = 0;
int16_t SPR_BOSS6_DIE3 = 0;
int16_t SPR_BOSS6_DIE4 = 0;
int16_t SPR_BOSS6_OUCH = 0;

int16_t SPR_BOSS7_W1 = 0;
int16_t SPR_BOSS7_W2 = 0;
int16_t SPR_BOSS7_W3 = 0;
int16_t SPR_BOSS7_W4 = 0;
int16_t SPR_BOSS7_SHOOT1 = 0;
int16_t SPR_BOSS7_SHOOT2 = 0;
int16_t SPR_BOSS7_SHOOT3 = 0;
int16_t SPR_BOSS7_DEAD = 0;
int16_t SPR_BOSS7_DIE1 = 0;
int16_t SPR_BOSS7_DIE2 = 0;
int16_t SPR_BOSS7_DIE3 = 0;
int16_t SPR_BOSS7_DIE4 = 0;
int16_t SPR_BOSS7_OUCH = 0;

int16_t SPR_BOSS8_W1 = 0;
int16_t SPR_BOSS8_W2 = 0;
int16_t SPR_BOSS8_W3 = 0;
int16_t SPR_BOSS8_W4 = 0;
int16_t SPR_BOSS8_SHOOT1 = 0;
int16_t SPR_BOSS8_SHOOT2 = 0;
int16_t SPR_BOSS8_SHOOT3 = 0;
int16_t SPR_BOSS8_DIE1 = 0;
int16_t SPR_BOSS8_DIE2 = 0;
int16_t SPR_BOSS8_DIE3 = 0;
int16_t SPR_BOSS8_DIE4 = 0;
int16_t SPR_BOSS8_DEAD = 0;
int16_t SPR_BOSS8_OUCH = 0;

int16_t SPR_BOSS9_W1 = 0;
int16_t SPR_BOSS9_W2 = 0;
int16_t SPR_BOSS9_W3 = 0;
int16_t SPR_BOSS9_W4 = 0;
int16_t SPR_BOSS9_SHOOT1 = 0;
int16_t SPR_BOSS9_SHOOT2 = 0;
int16_t SPR_BOSS9_SHOOT3 = 0;
int16_t SPR_BOSS9_DIE1 = 0;
int16_t SPR_BOSS9_DIE2 = 0;
int16_t SPR_BOSS9_DIE3 = 0;
int16_t SPR_BOSS9_DIE4 = 0;
int16_t SPR_BOSS9_DEAD = 0;
int16_t SPR_BOSS9_OUCH = 0;

int16_t SPR_BOSS10_W1 = 0;
int16_t SPR_BOSS10_W2 = 0;
int16_t SPR_BOSS10_W3 = 0;
int16_t SPR_BOSS10_W4 = 0;
int16_t SPR_BOSS10_SHOOT1 = 0;
int16_t SPR_BOSS10_SHOOT2 = 0;
int16_t SPR_BOSS10_SHOOT3 = 0;
int16_t SPR_BOSS10_DEAD = 0;
int16_t SPR_BOSS10_DIE1 = 0;
int16_t SPR_BOSS10_DIE2 = 0;
int16_t SPR_BOSS10_DIE3 = 0;
int16_t SPR_BOSS10_DIE4 = 0;
int16_t SPR_BOSS10_OUCH = 0;

int16_t SPR_BOSS10_SPIT1 = 0;
int16_t SPR_BOSS10_SPIT2 = 0;
int16_t SPR_BOSS10_SPIT3 = 0;
int16_t SPR_BOSS10_SPIT_EXP1 = 0;
int16_t SPR_BOSS10_SPIT_EXP2 = 0;
int16_t SPR_BOSS10_SPIT_EXP3 = 0;

int16_t SPR_DETONATOR_EXP1 = 0;
int16_t SPR_DETONATOR_EXP2 = 0;
int16_t SPR_DETONATOR_EXP3 = 0;
int16_t SPR_DETONATOR_EXP4 = 0;
int16_t SPR_DETONATOR_EXP5 = 0;
int16_t SPR_DETONATOR_EXP6 = 0;
int16_t SPR_DETONATOR_EXP7 = 0;
int16_t SPR_DETONATOR_EXP8 = 0;

int16_t SPR_CLIP_EXP1 = 0;
int16_t SPR_CLIP_EXP2 = 0;
int16_t SPR_CLIP_EXP3 = 0;
int16_t SPR_CLIP_EXP4 = 0;
int16_t SPR_CLIP_EXP5 = 0;
int16_t SPR_CLIP_EXP6 = 0;
int16_t SPR_CLIP_EXP7 = 0;
int16_t SPR_CLIP_EXP8 = 0;

int16_t SPR_BFG_WEAPON1 = 0;
int16_t SPR_BFG_WEAPON2 = 0;
int16_t SPR_BFG_WEAPON3 = 0;
int16_t SPR_BFG_WEAPON4 = 0;
int16_t SPR_BFG_WEAPON5 = 0;

int16_t SPR_BFG_WEAPON_SHOT1 = 0;
int16_t SPR_BFG_WEAPON_SHOT2 = 0;
int16_t SPR_BFG_WEAPON_SHOT3 = 0;

int16_t SPR_BFG_EXP1 = 0;
int16_t SPR_BFG_EXP2 = 0;
int16_t SPR_BFG_EXP3 = 0;
int16_t SPR_BFG_EXP4 = 0;
int16_t SPR_BFG_EXP5 = 0;
int16_t SPR_BFG_EXP6 = 0;
int16_t SPR_BFG_EXP7 = 0;
int16_t SPR_BFG_EXP8 = 0;


void initialize_sprites()
{
    if (::is_aog_sw()) {
        SPR_STAT_0 = 1;
        SPR_STAT_2 = 2;
        SPR_STAT_8 = 3;
        SPR_STAT_9 = 4;
        SPR_STAT_10 = 5;
        SPR_STAT_11 = 6;
        SPR_STAT_12 = 7;
        SPR_STAT_13 = 8;
        SPR_STAT_14 = 9;
        SPR_STAT_15 = 10;
        SPR_STAT_18 = 11;
        SPR_STAT_24 = 12;
        SPR_STAT_26 = 13;
        SPR_STAT_27 = 14;
        SPR_STAT_28 = 15;
        SPR_STAT_29 = 16;
        SPR_STAT_31 = 17;
        SPR_STAT_32 = 18;
        SPR_STAT_33 = 19;
        SPR_STAT_34 = 20;
        SPR_STAT_35 = 21;
        SPR_STAT_36 = 22;
        SPR_STAT_38 = 23;
        SPR_STAT_40 = 24;
        SPR_STAT_41 = 25;
        SPR_STAT_42 = 26;
        SPR_STAT_43 = 27;
        SPR_STAT_44 = 28;
        SPR_STAT_45 = 29;
        SPR_STAT_46 = 30;
        SPR_STAT_47 = 31;
        SPR_STAT_48 = 32;
        SPR_STAT_49 = 33;
        SPR_STAT_50 = 34;
        SPR_STAT_51 = 35;
        SPR_STAT_52 = 36;
        SPR_STAT_53 = 37;
        SPR_STAT_54 = 38;
        SPR_STAT_55 = 39;
        SPR_STAT_56 = 40;
        SPR_CRATE_1 = 41;
        SPR_STAT_57 = 42;
        SPR_STAT_60 = 43;
        SPR_STAT_61 = 44;
        SPR_STAT_62 = 45;
        SPR_STAT_63 = 46;
        SPR_STAT_65 = 47;
        SPR_STAT_66 = 48;
        SPR_STAT_67 = 49;
        SPR_STAT_68 = 50;
        SPR_STAT_69 = 51;
        SPR_STAT_77 = 52;
        SPR_STAT_78 = 53;
        SPR_STAT_79 = 54;

        SPR_AIR_VENT = 55;

        SPR_BLOOD_DRIP1 = 56;
        SPR_BLOOD_DRIP2 = 57;
        SPR_BLOOD_DRIP3 = 58;
        SPR_BLOOD_DRIP4 = 59;

        SPR_WATER_DRIP1 = 60;
        SPR_WATER_DRIP2 = 61;
        SPR_WATER_DRIP3 = 62;
        SPR_WATER_DRIP4 = 63;

        SPR_DECO_ARC_1 = 64;
        SPR_DECO_ARC_2 = 65;
        SPR_DECO_ARC_3 = 66;

        SPR_GRATE = 67;

        SPR_STEAM_1 = 68;
        SPR_STEAM_2 = 69;
        SPR_STEAM_3 = 70;
        SPR_STEAM_4 = 71;
        SPR_STEAM_PIPE = 72;
        SPR_PIPE_STEAM_1 = 73;
        SPR_PIPE_STEAM_2 = 74;
        SPR_PIPE_STEAM_3 = 75;
        SPR_PIPE_STEAM_4 = 76;

        SPR_RENT_S_1 = 77;
        SPR_RENT_S_2 = 78;
        SPR_RENT_S_3 = 79;
        SPR_RENT_S_4 = 80;
        SPR_RENT_S_5 = 81;
        SPR_RENT_S_6 = 82;
        SPR_RENT_S_7 = 83;
        SPR_RENT_S_8 = 84;
        SPR_RENT_W1_1 = 85;
        SPR_RENT_W1_2 = 86;
        SPR_RENT_W1_3 = 87;
        SPR_RENT_W1_4 = 88;
        SPR_RENT_W1_5 = 89;
        SPR_RENT_W1_6 = 90;
        SPR_RENT_W1_7 = 91;
        SPR_RENT_W1_8 = 92;
        SPR_RENT_W2_1 = 93;
        SPR_RENT_W2_2 = 94;
        SPR_RENT_W2_3 = 95;
        SPR_RENT_W2_4 = 96;
        SPR_RENT_W2_5 = 97;
        SPR_RENT_W2_6 = 98;
        SPR_RENT_W2_7 = 99;
        SPR_RENT_W2_8 = 100;
        SPR_RENT_W3_1 = 101;
        SPR_RENT_W3_2 = 102;
        SPR_RENT_W3_3 = 103;
        SPR_RENT_W3_4 = 104;
        SPR_RENT_W3_5 = 105;
        SPR_RENT_W3_6 = 106;
        SPR_RENT_W3_7 = 107;
        SPR_RENT_W3_8 = 108;
        SPR_RENT_W4_1 = 109;
        SPR_RENT_W4_2 = 110;
        SPR_RENT_W4_3 = 111;
        SPR_RENT_W4_4 = 112;
        SPR_RENT_W4_5 = 113;
        SPR_RENT_W4_6 = 114;
        SPR_RENT_W4_7 = 115;
        SPR_RENT_W4_8 = 116;
        SPR_RENT_DIE_1 = 117;
        SPR_RENT_DIE_2 = 118;
        SPR_RENT_DIE_3 = 119;
        SPR_RENT_DIE_4 = 120;
        SPR_RENT_PAIN_1 = 121;
        SPR_RENT_DEAD = 122;
        SPR_RENT_SHOOT1 = 123;
        SPR_RENT_SHOOT2 = 124;
        SPR_RENT_SHOOT3 = 125;

        SPR_PRO_S_1 = 126;
        SPR_PRO_S_2 = 127;
        SPR_PRO_S_3 = 128;
        SPR_PRO_S_4 = 129;
        SPR_PRO_S_5 = 130;
        SPR_PRO_S_6 = 131;
        SPR_PRO_S_7 = 132;
        SPR_PRO_S_8 = 133;
        SPR_PRO_W1_1 = 134;
        SPR_PRO_W1_2 = 135;
        SPR_PRO_W1_3 = 136;
        SPR_PRO_W1_4 = 137;
        SPR_PRO_W1_5 = 138;
        SPR_PRO_W1_6 = 139;
        SPR_PRO_W1_7 = 140;
        SPR_PRO_W1_8 = 141;
        SPR_PRO_W2_1 = 142;
        SPR_PRO_W2_2 = 143;
        SPR_PRO_W2_3 = 144;
        SPR_PRO_W2_4 = 145;
        SPR_PRO_W2_5 = 146;
        SPR_PRO_W2_6 = 147;
        SPR_PRO_W2_7 = 148;
        SPR_PRO_W2_8 = 149;
        SPR_PRO_W3_1 = 150;
        SPR_PRO_W3_2 = 151;
        SPR_PRO_W3_3 = 152;
        SPR_PRO_W3_4 = 153;
        SPR_PRO_W3_5 = 154;
        SPR_PRO_W3_6 = 155;
        SPR_PRO_W3_7 = 156;
        SPR_PRO_W3_8 = 157;
        SPR_PRO_W4_1 = 158;
        SPR_PRO_W4_2 = 159;
        SPR_PRO_W4_3 = 160;
        SPR_PRO_W4_4 = 161;
        SPR_PRO_W4_5 = 162;
        SPR_PRO_W4_6 = 163;
        SPR_PRO_W4_7 = 164;
        SPR_PRO_W4_8 = 165;
        SPR_PRO_PAIN_1 = 166;
        SPR_PRO_DIE_1 = 167;
        SPR_PRO_DIE_2 = 168;
        SPR_PRO_DIE_3 = 169;
        SPR_PRO_PAIN_2 = 170;
        SPR_PRO_DIE_4 = 171;
        SPR_PRO_DEAD = 172;
        SPR_PRO_SHOOT1 = 173;
        SPR_PRO_SHOOT2 = 174;
        SPR_PRO_SHOOT3 = 175;

        SPR_SWAT_S_1 = 176;
        SPR_SWAT_S_2 = 177;
        SPR_SWAT_S_3 = 178;
        SPR_SWAT_S_4 = 179;
        SPR_SWAT_S_5 = 180;
        SPR_SWAT_S_6 = 181;
        SPR_SWAT_S_7 = 182;
        SPR_SWAT_S_8 = 183;
        SPR_SWAT_W1_1 = 184;
        SPR_SWAT_W1_2 = 185;
        SPR_SWAT_W1_3 = 186;
        SPR_SWAT_W1_4 = 187;
        SPR_SWAT_W1_5 = 188;
        SPR_SWAT_W1_6 = 189;
        SPR_SWAT_W1_7 = 190;
        SPR_SWAT_W1_8 = 191;
        SPR_SWAT_W2_1 = 192;
        SPR_SWAT_W2_2 = 193;
        SPR_SWAT_W2_3 = 194;
        SPR_SWAT_W2_4 = 195;
        SPR_SWAT_W2_5 = 196;
        SPR_SWAT_W2_6 = 197;
        SPR_SWAT_W2_7 = 198;
        SPR_SWAT_W2_8 = 199;
        SPR_SWAT_W3_1 = 200;
        SPR_SWAT_W3_2 = 201;
        SPR_SWAT_W3_3 = 202;
        SPR_SWAT_W3_4 = 203;
        SPR_SWAT_W3_5 = 204;
        SPR_SWAT_W3_6 = 205;
        SPR_SWAT_W3_7 = 206;
        SPR_SWAT_W3_8 = 207;
        SPR_SWAT_W4_1 = 208;
        SPR_SWAT_W4_2 = 209;
        SPR_SWAT_W4_3 = 210;
        SPR_SWAT_W4_4 = 211;
        SPR_SWAT_W4_5 = 212;
        SPR_SWAT_W4_6 = 213;
        SPR_SWAT_W4_7 = 214;
        SPR_SWAT_W4_8 = 215;
        SPR_SWAT_PAIN_1 = 216;
        SPR_SWAT_DIE_1 = 217;
        SPR_SWAT_DIE_2 = 218;
        SPR_SWAT_DIE_3 = 219;
        SPR_SWAT_PAIN_2 = 220;
        SPR_SWAT_DIE_4 = 221;
        SPR_SWAT_DEAD = 222;
        SPR_SWAT_SHOOT1 = 223;
        SPR_SWAT_SHOOT2 = 224;
        SPR_SWAT_SHOOT3 = 225;
        SPR_SWAT_WOUNDED1 = 226;
        SPR_SWAT_WOUNDED2 = 227;
        SPR_SWAT_WOUNDED3 = 228;
        SPR_SWAT_WOUNDED4 = 229;

        SPR_OFC_S_1 = 230;
        SPR_OFC_S_2 = 231;
        SPR_OFC_S_3 = 232;
        SPR_OFC_S_4 = 233;
        SPR_OFC_S_5 = 234;
        SPR_OFC_S_6 = 235;
        SPR_OFC_S_7 = 236;
        SPR_OFC_S_8 = 237;
        SPR_OFC_W1_1 = 238;
        SPR_OFC_W1_2 = 239;
        SPR_OFC_W1_3 = 240;
        SPR_OFC_W1_4 = 241;
        SPR_OFC_W1_5 = 242;
        SPR_OFC_W1_6 = 243;
        SPR_OFC_W1_7 = 244;
        SPR_OFC_W1_8 = 245;
        SPR_OFC_W2_1 = 246;
        SPR_OFC_W2_2 = 247;
        SPR_OFC_W2_3 = 248;
        SPR_OFC_W2_4 = 249;
        SPR_OFC_W2_5 = 250;
        SPR_OFC_W2_6 = 251;
        SPR_OFC_W2_7 = 252;
        SPR_OFC_W2_8 = 253;
        SPR_OFC_W3_1 = 254;
        SPR_OFC_W3_2 = 255;
        SPR_OFC_W3_3 = 256;
        SPR_OFC_W3_4 = 257;
        SPR_OFC_W3_5 = 258;
        SPR_OFC_W3_6 = 259;
        SPR_OFC_W3_7 = 260;
        SPR_OFC_W3_8 = 261;
        SPR_OFC_W4_1 = 262;
        SPR_OFC_W4_2 = 263;
        SPR_OFC_W4_3 = 264;
        SPR_OFC_W4_4 = 265;
        SPR_OFC_W4_5 = 266;
        SPR_OFC_W4_6 = 267;
        SPR_OFC_W4_7 = 268;
        SPR_OFC_W4_8 = 269;
        SPR_OFC_PAIN_1 = 270;
        SPR_OFC_DIE_1 = 271;
        SPR_OFC_DIE_2 = 272;
        SPR_OFC_DIE_3 = 273;
        SPR_OFC_PAIN_2 = 274;
        SPR_OFC_DIE_4 = 275;
        SPR_OFC_DEAD = 276;
        SPR_OFC_SHOOT1 = 277;
        SPR_OFC_SHOOT2 = 278;
        SPR_OFC_SHOOT3 = 279;

        SPR_GOLD_S_1 = 280;
        SPR_GOLD_S_2 = 281;
        SPR_GOLD_S_3 = 282;
        SPR_GOLD_S_4 = 283;
        SPR_GOLD_S_5 = 284;
        SPR_GOLD_S_6 = 285;
        SPR_GOLD_S_7 = 286;
        SPR_GOLD_S_8 = 287;
        SPR_GOLD_W1_1 = 288;
        SPR_GOLD_W1_2 = 289;
        SPR_GOLD_W1_3 = 290;
        SPR_GOLD_W1_4 = 291;
        SPR_GOLD_W1_5 = 292;
        SPR_GOLD_W1_6 = 293;
        SPR_GOLD_W1_7 = 294;
        SPR_GOLD_W1_8 = 295;
        SPR_GOLD_W2_1 = 296;
        SPR_GOLD_W2_2 = 297;
        SPR_GOLD_W2_3 = 298;
        SPR_GOLD_W2_4 = 299;
        SPR_GOLD_W2_5 = 300;
        SPR_GOLD_W2_6 = 301;
        SPR_GOLD_W2_7 = 302;
        SPR_GOLD_W2_8 = 303;
        SPR_GOLD_W3_1 = 304;
        SPR_GOLD_W3_2 = 305;
        SPR_GOLD_W3_3 = 306;
        SPR_GOLD_W3_4 = 307;
        SPR_GOLD_W3_5 = 308;
        SPR_GOLD_W3_6 = 309;
        SPR_GOLD_W3_7 = 310;
        SPR_GOLD_W3_8 = 311;
        SPR_GOLD_W4_1 = 312;
        SPR_GOLD_W4_2 = 313;
        SPR_GOLD_W4_3 = 314;
        SPR_GOLD_W4_4 = 315;
        SPR_GOLD_W4_5 = 316;
        SPR_GOLD_W4_6 = 317;
        SPR_GOLD_W4_7 = 318;
        SPR_GOLD_W4_8 = 319;
        SPR_GOLD_PAIN_1 = 320;
        SPR_GOLD_WRIST_1 = 321;
        SPR_GOLD_WRIST_2 = 322;
        SPR_GOLD_SHOOT1 = 323;
        SPR_GOLD_SHOOT2 = 324;
        SPR_GOLD_SHOOT3 = 325;
        SPR_GOLD_WARP1 = 326;
        SPR_GOLD_WARP2 = 327;
        SPR_GOLD_WARP3 = 328;
        SPR_GOLD_WARP4 = 329;
        SPR_GOLD_WARP5 = 330;

        SPR_FSCOUT_W1_1 = 331;
        SPR_FSCOUT_W1_2 = 332;
        SPR_FSCOUT_W1_3 = 333;
        SPR_FSCOUT_W1_4 = 334;
        SPR_FSCOUT_W1_5 = 335;
        SPR_FSCOUT_W1_6 = 336;
        SPR_FSCOUT_W1_7 = 337;
        SPR_FSCOUT_W1_8 = 338;
        SPR_FSCOUT_DIE1 = 339;
        SPR_FSCOUT_DIE2 = 340;
        SPR_FSCOUT_DIE3 = 341;
        SPR_FSCOUT_DIE4 = 342;
        SPR_FSCOUT_DIE5 = 343;
        SPR_FSCOUT_DIE6 = 344;
        SPR_FSCOUT_DIE7 = 345;
        SPR_FSCOUT_DEAD = 346;

        SPR_EXPLOSION_1 = 347;
        SPR_EXPLOSION_2 = 348;
        SPR_EXPLOSION_3 = 349;
        SPR_EXPLOSION_4 = 350;
        SPR_EXPLOSION_5 = 351;

        SPR_SECURITY_NORMAL = 352;
        SPR_SECURITY_ALERT = 353;

        SPR_POD_EGG = 354;
        SPR_POD_HATCH1 = 355;
        SPR_POD_HATCH2 = 356;
        SPR_POD_HATCH3 = 357;
        SPR_POD_WALK1 = 358;
        SPR_POD_WALK2 = 359;
        SPR_POD_WALK3 = 360;
        SPR_POD_WALK4 = 361;
        SPR_POD_ATTACK1 = 362;
        SPR_POD_ATTACK2 = 363;
        SPR_POD_ATTACK3 = 364;
        SPR_POD_OUCH = 365;
        SPR_POD_DIE1 = 366;
        SPR_POD_DIE2 = 367;
        SPR_POD_DIE3 = 368;
        SPR_POD_SPIT1 = 369;
        SPR_POD_SPIT2 = 370;
        SPR_POD_SPIT3 = 371;

        SPR_ELEC_APPEAR1 = 372;
        SPR_ELEC_APPEAR2 = 373;
        SPR_ELEC_APPEAR3 = 374;
        SPR_ELEC_WALK1 = 375;
        SPR_ELEC_WALK2 = 376;
        SPR_ELEC_WALK3 = 377;
        SPR_ELEC_WALK4 = 378;
        SPR_ELEC_OUCH = 379;
        SPR_ELEC_SHOOT1 = 380;
        SPR_ELEC_SHOOT2 = 381;
        SPR_ELEC_SHOOT3 = 382;
        SPR_ELEC_DIE1 = 383;
        SPR_ELEC_DIE2 = 384;
        SPR_ELEC_DIE3 = 385;
        SPR_ELEC_SHOT1 = 386;
        SPR_ELEC_SHOT2 = 387;
        SPR_ELEC_SHOT_EXP1 = 388;
        SPR_ELEC_SHOT_EXP2 = 389;

        SPR_ELECTRO_SPHERE_ROAM1 = 390;
        SPR_ELECTRO_SPHERE_ROAM2 = 391;
        SPR_ELECTRO_SPHERE_ROAM3 = 392;
        SPR_ELECTRO_SPHERE_OUCH = 393;
        SPR_ELECTRO_SPHERE_DIE1 = 394;
        SPR_ELECTRO_SPHERE_DIE2 = 395;
        SPR_ELECTRO_SPHERE_DIE3 = 396;
        SPR_ELECTRO_SPHERE_DIE4 = 397;

        SPR_GENETIC_W1 = 398;
        SPR_GENETIC_W2 = 399;
        SPR_GENETIC_W3 = 400;
        SPR_GENETIC_W4 = 401;
        SPR_GENETIC_SWING1 = 402;
        SPR_GENETIC_SWING2 = 403;
        SPR_GENETIC_SWING3 = 404;
        SPR_GENETIC_DEAD = 405;
        SPR_GENETIC_DIE1 = 406;
        SPR_GENETIC_DIE2 = 407;
        SPR_GENETIC_DIE3 = 408;
        SPR_GENETIC_DIE4 = 409;
        SPR_GENETIC_OUCH = 410;
        SPR_GENETIC_SHOOT1 = 411;
        SPR_GENETIC_SHOOT2 = 412;
        SPR_GENETIC_SHOOT3 = 413;

        SPR_MUTHUM1_W1 = 414;
        SPR_MUTHUM1_W2 = 415;
        SPR_MUTHUM1_W3 = 416;
        SPR_MUTHUM1_W4 = 417;
        SPR_MUTHUM1_SWING1 = 418;
        SPR_MUTHUM1_SWING2 = 419;
        SPR_MUTHUM1_SWING3 = 420;
        SPR_MUTHUM1_DEAD = 421;
        SPR_MUTHUM1_DIE1 = 422;
        SPR_MUTHUM1_DIE2 = 423;
        SPR_MUTHUM1_DIE3 = 424;
        SPR_MUTHUM1_DIE4 = 425;
        SPR_MUTHUM1_OUCH = 426;
        SPR_MUTHUM1_SPIT1 = 427;
        SPR_MUTHUM1_SPIT2 = 428;
        SPR_MUTHUM1_SPIT3 = 429;

        SPR_SCAN_ALIEN_READY = 430;
        SPR_SCAN_ALIEN_B1 = 431;
        SPR_SCAN_ALIEN_B2 = 432;
        SPR_SCAN_ALIEN_B3 = 433;
        SPR_SCAN_ALIEN_EMPTY = 434;
        SPR_SCAN_ALIEN_W1 = 435;
        SPR_SCAN_ALIEN_W2 = 436;
        SPR_SCAN_ALIEN_W3 = 437;
        SPR_SCAN_ALIEN_W4 = 438;
        SPR_SCAN_ALIEN_SWING1 = 439;
        SPR_SCAN_ALIEN_SWING2 = 440;
        SPR_SCAN_ALIEN_SWING3 = 441;
        SPR_SCAN_ALIEN_DEAD = 442;
        SPR_SCAN_ALIEN_DIE1 = 443;
        SPR_SCAN_ALIEN_DIE2 = 444;
        SPR_SCAN_ALIEN_DIE3 = 445;
        SPR_SCAN_ALIEN_DIE4 = 446;
        SPR_SCAN_ALIEN_OUCH = 447;
        SPR_SCAN_ALIEN_SPIT1 = 448;
        SPR_SCAN_ALIEN_SPIT2 = 449;
        SPR_SCAN_ALIEN_SPIT3 = 450;

        SPR_SPIT1_1 = 451;
        SPR_SPIT1_2 = 452;
        SPR_SPIT1_3 = 453;
        SPR_SPIT_EXP1_1 = 454;
        SPR_SPIT_EXP1_2 = 455;
        SPR_SPIT_EXP1_3 = 456;

        SPR_SPIT2_1 = 457;
        SPR_SPIT2_2 = 458;
        SPR_SPIT2_3 = 459;
        SPR_SPIT_EXP2_1 = 460;
        SPR_SPIT_EXP2_2 = 461;
        SPR_SPIT_EXP2_3 = 462;

        SPR_SPIT3_1 = 463;
        SPR_SPIT3_2 = 464;
        SPR_SPIT3_3 = 465;
        SPR_SPIT_EXP3_1 = 466;
        SPR_SPIT_EXP3_2 = 467;
        SPR_SPIT_EXP3_3 = 468;

        SPR_TERROT_1 = 469;
        SPR_TERROT_2 = 470;
        SPR_TERROT_3 = 471;
        SPR_TERROT_4 = 472;
        SPR_TERROT_5 = 473;
        SPR_TERROT_6 = 474;
        SPR_TERROT_7 = 475;
        SPR_TERROT_8 = 476;
        SPR_TERROT_FIRE_1 = 477;
        SPR_TERROT_FIRE_2 = 478;
        SPR_TERROT_DIE_1 = 479;
        SPR_TERROT_DIE_2 = 480;
        SPR_TERROT_DIE_3 = 481;
        SPR_TERROT_DIE_4 = 482;
        SPR_TERROT_DEAD = 483;

        SPR_KNIFEREADY = 484;
        SPR_KNIFEATK1 = 485;
        SPR_KNIFEATK2 = 486;
        SPR_KNIFEATK3 = 487;
        SPR_KNIFEATK4 = 488;

        SPR_PISTOLREADY = 489;
        SPR_PISTOLATK1 = 490;
        SPR_PISTOLATK2 = 491;
        SPR_PISTOLATK3 = 492;
        SPR_PISTOLATK4 = 493;

        SPR_MACHINEGUNREADY = 494;
        SPR_MACHINEGUNATK1 = 495;
        SPR_MACHINEGUNATK2 = 496;
        SPR_MACHINEGUNATK3 = 497;
        SPR_MACHINEGUNATK4 = 498;

        SPR_CHAINREADY = 499;
        SPR_CHAINATK1 = 500;
        SPR_CHAINATK2 = 501;
        SPR_CHAINATK3 = 502;
        SPR_CHAINATK4 = 503;

        SPR_GRENADEREADY = 504;
        SPR_GRENADEATK1 = 505;
        SPR_GRENADEATK2 = 506;
        SPR_GRENADEATK3 = 507;
        SPR_GRENADEATK4 = 508;
        SPR_GRENADE_FLY1 = 509;
        SPR_GRENADE_FLY2 = 510;
        SPR_GRENADE_FLY3 = 511;
        SPR_GRENADE_FLY4 = 512;
        SPR_GRENADE_EXPLODE1 = 513;
        SPR_GRENADE_EXPLODE2 = 514;
        SPR_GRENADE_EXPLODE3 = 515;
        SPR_GRENADE_EXPLODE4 = 516;
        SPR_GRENADE_EXPLODE5 = 517;

        SPR_ELEC_ARC1 = 518;
        SPR_ELEC_ARC2 = 519;
        SPR_ELEC_ARC3 = 520;
        SPR_ELEC_ARC4 = 521;
        SPR_ELEC_POST1 = 522;
        SPR_ELEC_POST2 = 523;
        SPR_ELEC_POST3 = 524;
        SPR_ELEC_POST4 = 525;

        SPR_CANDY_BAR = 526;
        SPR_CANDY_WRAPER = 527;

        SPR_SANDWICH = 528;
        SPR_SANDWICH_WRAPER = 529;

        SPR_BLAKE_W1 = 530;
        SPR_BLAKE_W2 = 531;
        SPR_BLAKE_W3 = 532;
        SPR_BLAKE_W4 = 533;

        SPR_BOSS1_W1 = 534;
        SPR_BOSS1_W2 = 535;
        SPR_BOSS1_W3 = 536;
        SPR_BOSS1_W4 = 537;
        SPR_BOSS1_SWING1 = 538;
        SPR_BOSS1_SWING2 = 539;
        SPR_BOSS1_SWING3 = 540;
        SPR_BOSS1_DEAD = 541;
        SPR_BOSS1_DIE1 = 542;
        SPR_BOSS1_DIE2 = 543;
        SPR_BOSS1_DIE3 = 544;
        SPR_BOSS1_DIE4 = 545;
        SPR_BOSS1_OUCH = 546;
        SPR_BOSS1_PROJ1 = 547;
        SPR_BOSS1_PROJ2 = 548;
        SPR_BOSS1_PROJ3 = 549;
        SPR_BOSS1_EXP1 = 550;
        SPR_BOSS1_EXP2 = 551;
        SPR_BOSS1_EXP3 = 552;
    } else if (::is_aog_full()) {
        SPR_STAT_0 = 1;
        SPR_STAT_1 = 2;
        SPR_STAT_2 = 3;
        SPR_STAT_3 = 4;
        SPR_STAT_4 = 5;
        SPR_STAT_5 = 6;
        SPR_STAT_6 = 7;
        SPR_STAT_7 = 8;
        SPR_STAT_8 = 9;
        SPR_STAT_9 = 10;
        SPR_STAT_10 = 11;
        SPR_STAT_11 = 12;
        SPR_STAT_12 = 13;
        SPR_STAT_13 = 14;
        SPR_STAT_14 = 15;
        SPR_STAT_15 = 16;
        SPR_STAT_16 = 17;
        SPR_STAT_17 = 18;
        SPR_STAT_18 = 19;
        SPR_STAT_19 = 20;
        SPR_STAT_20 = 21;
        SPR_STAT_21 = 22;
        SPR_STAT_22 = 23;
        SPR_STAT_23 = 24;
        SPR_STAT_24 = 25;
        SPR_STAT_25 = 26;
        SPR_STAT_26 = 27;
        SPR_STAT_27 = 28;
        SPR_STAT_28 = 29;
        SPR_STAT_29 = 30;
        SPR_STAT_30 = 31;
        SPR_STAT_31 = 32;
        SPR_STAT_32 = 33;
        SPR_STAT_33 = 34;
        SPR_STAT_34 = 35;
        SPR_STAT_35 = 36;
        SPR_STAT_36 = 37;
        SPR_STAT_37 = 38;
        SPR_STAT_38 = 39;
        SPR_STAT_39 = 40;
        SPR_STAT_40 = 41;
        SPR_STAT_41 = 42;
        SPR_STAT_42 = 43;
        SPR_STAT_43 = 44;
        SPR_STAT_44 = 45;
        SPR_STAT_45 = 46;
        SPR_STAT_46 = 47;
        SPR_STAT_47 = 48;
        SPR_STAT_48 = 49;
        SPR_STAT_49 = 50;
        SPR_STAT_50 = 51;
        SPR_STAT_51 = 52;
        SPR_STAT_52 = 53;
        SPR_STAT_53 = 54;
        SPR_STAT_54 = 55;
        SPR_STAT_55 = 56;
        SPR_STAT_56 = 57;

        SPR_CRATE_1 = 58;
        SPR_CRATE_2 = 59;
        SPR_CRATE_3 = 60;

        SPR_STAT_57 = 61;
        SPR_STAT_58 = 62;
        SPR_STAT_59 = 63;
        SPR_STAT_60 = 64;
        SPR_STAT_61 = 65;
        SPR_STAT_62 = 66;
        SPR_STAT_63 = 67;
        SPR_STAT_64 = 68;
        SPR_STAT_65 = 69;
        SPR_STAT_66 = 70;
        SPR_STAT_67 = 71;
        SPR_STAT_68 = 72;
        SPR_STAT_69 = 73;
        SPR_STAT_70 = 74;
        SPR_STAT_71 = 75;
        SPR_STAT_72 = 76;
        SPR_STAT_73 = 77;
        SPR_STAT_74 = 78;
        SPR_STAT_75 = 79;
        SPR_STAT_76 = 80;
        SPR_STAT_77 = 81;
        SPR_STAT_78 = 82;
        SPR_STAT_79 = 83;

        SPR_AIR_VENT = 84;

        SPR_BLOOD_DRIP1 = 85;
        SPR_BLOOD_DRIP2 = 86;
        SPR_BLOOD_DRIP3 = 87;
        SPR_BLOOD_DRIP4 = 88;

        SPR_WATER_DRIP1 = 89;
        SPR_WATER_DRIP2 = 90;
        SPR_WATER_DRIP3 = 91;
        SPR_WATER_DRIP4 = 92;

        SPR_DECO_ARC_1 = 93;
        SPR_DECO_ARC_2 = 94;
        SPR_DECO_ARC_3 = 95;

        SPR_GRATE = 96;

        SPR_STEAM_1 = 97;
        SPR_STEAM_2 = 98;
        SPR_STEAM_3 = 99;
        SPR_STEAM_4 = 100;
        SPR_STEAM_PIPE = 101;
        SPR_PIPE_STEAM_1 = 102;
        SPR_PIPE_STEAM_2 = 103;
        SPR_PIPE_STEAM_3 = 104;
        SPR_PIPE_STEAM_4 = 105;

        SPR_RENT_S_1 = 106;
        SPR_RENT_S_2 = 107;
        SPR_RENT_S_3 = 108;
        SPR_RENT_S_4 = 109;
        SPR_RENT_S_5 = 110;
        SPR_RENT_S_6 = 111;
        SPR_RENT_S_7 = 112;
        SPR_RENT_S_8 = 113;
        SPR_RENT_W1_1 = 114;
        SPR_RENT_W1_2 = 115;
        SPR_RENT_W1_3 = 116;
        SPR_RENT_W1_4 = 117;
        SPR_RENT_W1_5 = 118;
        SPR_RENT_W1_6 = 119;
        SPR_RENT_W1_7 = 120;
        SPR_RENT_W1_8 = 121;
        SPR_RENT_W2_1 = 122;
        SPR_RENT_W2_2 = 123;
        SPR_RENT_W2_3 = 124;
        SPR_RENT_W2_4 = 125;
        SPR_RENT_W2_5 = 126;
        SPR_RENT_W2_6 = 127;
        SPR_RENT_W2_7 = 128;
        SPR_RENT_W2_8 = 129;
        SPR_RENT_W3_1 = 130;
        SPR_RENT_W3_2 = 131;
        SPR_RENT_W3_3 = 132;
        SPR_RENT_W3_4 = 133;
        SPR_RENT_W3_5 = 134;
        SPR_RENT_W3_6 = 135;
        SPR_RENT_W3_7 = 136;
        SPR_RENT_W3_8 = 137;
        SPR_RENT_W4_1 = 138;
        SPR_RENT_W4_2 = 139;
        SPR_RENT_W4_3 = 140;
        SPR_RENT_W4_4 = 141;
        SPR_RENT_W4_5 = 142;
        SPR_RENT_W4_6 = 143;
        SPR_RENT_W4_7 = 144;
        SPR_RENT_W4_8 = 145;
        SPR_RENT_DIE_1 = 146;
        SPR_RENT_DIE_2 = 147;
        SPR_RENT_DIE_3 = 148;
        SPR_RENT_DIE_4 = 149;
        SPR_RENT_PAIN_1 = 150;
        SPR_RENT_DEAD = 151;
        SPR_RENT_SHOOT1 = 152;
        SPR_RENT_SHOOT2 = 153;
        SPR_RENT_SHOOT3 = 154;

        SPR_PRO_S_1 = 155;
        SPR_PRO_S_2 = 156;
        SPR_PRO_S_3 = 157;
        SPR_PRO_S_4 = 158;
        SPR_PRO_S_5 = 159;
        SPR_PRO_S_6 = 160;
        SPR_PRO_S_7 = 161;
        SPR_PRO_S_8 = 162;
        SPR_PRO_W1_1 = 163;
        SPR_PRO_W1_2 = 164;
        SPR_PRO_W1_3 = 165;
        SPR_PRO_W1_4 = 166;
        SPR_PRO_W1_5 = 167;
        SPR_PRO_W1_6 = 168;
        SPR_PRO_W1_7 = 169;
        SPR_PRO_W1_8 = 170;
        SPR_PRO_W2_1 = 171;
        SPR_PRO_W2_2 = 172;
        SPR_PRO_W2_3 = 173;
        SPR_PRO_W2_4 = 174;
        SPR_PRO_W2_5 = 175;
        SPR_PRO_W2_6 = 176;
        SPR_PRO_W2_7 = 177;
        SPR_PRO_W2_8 = 178;
        SPR_PRO_W3_1 = 179;
        SPR_PRO_W3_2 = 180;
        SPR_PRO_W3_3 = 181;
        SPR_PRO_W3_4 = 182;
        SPR_PRO_W3_5 = 183;
        SPR_PRO_W3_6 = 184;
        SPR_PRO_W3_7 = 185;
        SPR_PRO_W3_8 = 186;
        SPR_PRO_W4_1 = 187;
        SPR_PRO_W4_2 = 188;
        SPR_PRO_W4_3 = 189;
        SPR_PRO_W4_4 = 190;
        SPR_PRO_W4_5 = 191;
        SPR_PRO_W4_6 = 192;
        SPR_PRO_W4_7 = 193;
        SPR_PRO_W4_8 = 194;
        SPR_PRO_PAIN_1 = 195;
        SPR_PRO_DIE_1 = 196;
        SPR_PRO_DIE_2 = 197;
        SPR_PRO_DIE_3 = 198;
        SPR_PRO_PAIN_2 = 199;
        SPR_PRO_DIE_4 = 200;
        SPR_PRO_DEAD = 201;
        SPR_PRO_SHOOT1 = 202;
        SPR_PRO_SHOOT2 = 203;
        SPR_PRO_SHOOT3 = 204;

        SPR_SWAT_S_1 = 205;
        SPR_SWAT_S_2 = 206;
        SPR_SWAT_S_3 = 207;
        SPR_SWAT_S_4 = 208;
        SPR_SWAT_S_5 = 209;
        SPR_SWAT_S_6 = 210;
        SPR_SWAT_S_7 = 211;
        SPR_SWAT_S_8 = 212;
        SPR_SWAT_W1_1 = 213;
        SPR_SWAT_W1_2 = 214;
        SPR_SWAT_W1_3 = 215;
        SPR_SWAT_W1_4 = 216;
        SPR_SWAT_W1_5 = 217;
        SPR_SWAT_W1_6 = 218;
        SPR_SWAT_W1_7 = 219;
        SPR_SWAT_W1_8 = 220;
        SPR_SWAT_W2_1 = 221;
        SPR_SWAT_W2_2 = 222;
        SPR_SWAT_W2_3 = 223;
        SPR_SWAT_W2_4 = 224;
        SPR_SWAT_W2_5 = 225;
        SPR_SWAT_W2_6 = 226;
        SPR_SWAT_W2_7 = 227;
        SPR_SWAT_W2_8 = 228;
        SPR_SWAT_W3_1 = 229;
        SPR_SWAT_W3_2 = 230;
        SPR_SWAT_W3_3 = 231;
        SPR_SWAT_W3_4 = 232;
        SPR_SWAT_W3_5 = 233;
        SPR_SWAT_W3_6 = 234;
        SPR_SWAT_W3_7 = 235;
        SPR_SWAT_W3_8 = 236;
        SPR_SWAT_W4_1 = 237;
        SPR_SWAT_W4_2 = 238;
        SPR_SWAT_W4_3 = 239;
        SPR_SWAT_W4_4 = 240;
        SPR_SWAT_W4_5 = 241;
        SPR_SWAT_W4_6 = 242;
        SPR_SWAT_W4_7 = 243;
        SPR_SWAT_W4_8 = 244;
        SPR_SWAT_PAIN_1 = 245;
        SPR_SWAT_DIE_1 = 246;
        SPR_SWAT_DIE_2 = 247;
        SPR_SWAT_DIE_3 = 248;
        SPR_SWAT_PAIN_2 = 249;
        SPR_SWAT_DIE_4 = 250;
        SPR_SWAT_DEAD = 251;
        SPR_SWAT_SHOOT1 = 252;
        SPR_SWAT_SHOOT2 = 253;
        SPR_SWAT_SHOOT3 = 254;
        SPR_SWAT_WOUNDED1 = 255;
        SPR_SWAT_WOUNDED2 = 256;
        SPR_SWAT_WOUNDED3 = 257;
        SPR_SWAT_WOUNDED4 = 258;

        SPR_OFC_S_1 = 259;
        SPR_OFC_S_2 = 260;
        SPR_OFC_S_3 = 261;
        SPR_OFC_S_4 = 262;
        SPR_OFC_S_5 = 263;
        SPR_OFC_S_6 = 264;
        SPR_OFC_S_7 = 265;
        SPR_OFC_S_8 = 266;
        SPR_OFC_W1_1 = 267;
        SPR_OFC_W1_2 = 268;
        SPR_OFC_W1_3 = 269;
        SPR_OFC_W1_4 = 270;
        SPR_OFC_W1_5 = 271;
        SPR_OFC_W1_6 = 272;
        SPR_OFC_W1_7 = 273;
        SPR_OFC_W1_8 = 274;
        SPR_OFC_W2_1 = 275;
        SPR_OFC_W2_2 = 276;
        SPR_OFC_W2_3 = 277;
        SPR_OFC_W2_4 = 278;
        SPR_OFC_W2_5 = 279;
        SPR_OFC_W2_6 = 280;
        SPR_OFC_W2_7 = 281;
        SPR_OFC_W2_8 = 282;
        SPR_OFC_W3_1 = 283;
        SPR_OFC_W3_2 = 284;
        SPR_OFC_W3_3 = 285;
        SPR_OFC_W3_4 = 286;
        SPR_OFC_W3_5 = 287;
        SPR_OFC_W3_6 = 288;
        SPR_OFC_W3_7 = 289;
        SPR_OFC_W3_8 = 290;
        SPR_OFC_W4_1 = 291;
        SPR_OFC_W4_2 = 292;
        SPR_OFC_W4_3 = 293;
        SPR_OFC_W4_4 = 294;
        SPR_OFC_W4_5 = 295;
        SPR_OFC_W4_6 = 296;
        SPR_OFC_W4_7 = 297;
        SPR_OFC_W4_8 = 298;
        SPR_OFC_PAIN_1 = 299;
        SPR_OFC_DIE_1 = 300;
        SPR_OFC_DIE_2 = 301;
        SPR_OFC_DIE_3 = 302;
        SPR_OFC_PAIN_2 = 303;
        SPR_OFC_DIE_4 = 304;
        SPR_OFC_DEAD = 305;
        SPR_OFC_SHOOT1 = 306;
        SPR_OFC_SHOOT2 = 307;
        SPR_OFC_SHOOT3 = 308;

        SPR_GOLD_S_1 = 309;
        SPR_GOLD_S_2 = 310;
        SPR_GOLD_S_3 = 311;
        SPR_GOLD_S_4 = 312;
        SPR_GOLD_S_5 = 313;
        SPR_GOLD_S_6 = 314;
        SPR_GOLD_S_7 = 315;
        SPR_GOLD_S_8 = 316;
        SPR_GOLD_W1_1 = 317;
        SPR_GOLD_W1_2 = 318;
        SPR_GOLD_W1_3 = 319;
        SPR_GOLD_W1_4 = 320;
        SPR_GOLD_W1_5 = 321;
        SPR_GOLD_W1_6 = 322;
        SPR_GOLD_W1_7 = 323;
        SPR_GOLD_W1_8 = 324;
        SPR_GOLD_W2_1 = 325;
        SPR_GOLD_W2_2 = 326;
        SPR_GOLD_W2_3 = 327;
        SPR_GOLD_W2_4 = 328;
        SPR_GOLD_W2_5 = 329;
        SPR_GOLD_W2_6 = 330;
        SPR_GOLD_W2_7 = 331;
        SPR_GOLD_W2_8 = 332;
        SPR_GOLD_W3_1 = 333;
        SPR_GOLD_W3_2 = 334;
        SPR_GOLD_W3_3 = 335;
        SPR_GOLD_W3_4 = 336;
        SPR_GOLD_W3_5 = 337;
        SPR_GOLD_W3_6 = 338;
        SPR_GOLD_W3_7 = 339;
        SPR_GOLD_W3_8 = 340;
        SPR_GOLD_W4_1 = 341;
        SPR_GOLD_W4_2 = 342;
        SPR_GOLD_W4_3 = 343;
        SPR_GOLD_W4_4 = 344;
        SPR_GOLD_W4_5 = 345;
        SPR_GOLD_W4_6 = 346;
        SPR_GOLD_W4_7 = 347;
        SPR_GOLD_W4_8 = 348;
        SPR_GOLD_PAIN_1 = 349;
        SPR_GOLD_WRIST_1 = 350;
        SPR_GOLD_WRIST_2 = 351;
        SPR_GOLD_SHOOT1 = 352;
        SPR_GOLD_SHOOT2 = 353;
        SPR_GOLD_SHOOT3 = 354;
        SPR_GOLD_WARP1 = 355;
        SPR_GOLD_WARP2 = 356;
        SPR_GOLD_WARP3 = 357;
        SPR_GOLD_WARP4 = 358;
        SPR_GOLD_WARP5 = 359;

        SPR_GSCOUT_W1_1 = 360;
        SPR_GSCOUT_W1_2 = 361;
        SPR_GSCOUT_W1_3 = 362;
        SPR_GSCOUT_W1_4 = 363;
        SPR_GSCOUT_W1_5 = 364;
        SPR_GSCOUT_W1_6 = 365;
        SPR_GSCOUT_W1_7 = 366;
        SPR_GSCOUT_W1_8 = 367;
        SPR_GSCOUT_W2_1 = 368;
        SPR_GSCOUT_W2_2 = 369;
        SPR_GSCOUT_W2_3 = 370;
        SPR_GSCOUT_W2_4 = 371;
        SPR_GSCOUT_W2_5 = 372;
        SPR_GSCOUT_W2_6 = 373;
        SPR_GSCOUT_W2_7 = 374;
        SPR_GSCOUT_W2_8 = 375;
        SPR_GSCOUT_W3_1 = 376;
        SPR_GSCOUT_W3_2 = 377;
        SPR_GSCOUT_W3_3 = 378;
        SPR_GSCOUT_W3_4 = 379;
        SPR_GSCOUT_W3_5 = 380;
        SPR_GSCOUT_W3_6 = 381;
        SPR_GSCOUT_W3_7 = 382;
        SPR_GSCOUT_W3_8 = 383;
        SPR_GSCOUT_W4_1 = 384;
        SPR_GSCOUT_W4_2 = 385;
        SPR_GSCOUT_W4_3 = 386;
        SPR_GSCOUT_W4_4 = 387;
        SPR_GSCOUT_W4_5 = 388;
        SPR_GSCOUT_W4_6 = 389;
        SPR_GSCOUT_W4_7 = 390;
        SPR_GSCOUT_W4_8 = 391;
        SPR_GSCOUT_DIE1 = 392;
        SPR_GSCOUT_DIE2 = 393;
        SPR_GSCOUT_DIE3 = 394;
        SPR_GSCOUT_DIE4 = 395;
        SPR_GSCOUT_DIE5 = 396;
        SPR_GSCOUT_DIE6 = 397;
        SPR_GSCOUT_DIE7 = 398;
        SPR_GSCOUT_DIE8 = 399;
        SPR_GSCOUT_DEAD = 400;

        SPR_FSCOUT_W1_1 = 401;
        SPR_FSCOUT_W1_2 = 402;
        SPR_FSCOUT_W1_3 = 403;
        SPR_FSCOUT_W1_4 = 404;
        SPR_FSCOUT_W1_5 = 405;
        SPR_FSCOUT_W1_6 = 406;
        SPR_FSCOUT_W1_7 = 407;
        SPR_FSCOUT_W1_8 = 408;
        SPR_FSCOUT_W2_1 = 409;
        SPR_FSCOUT_W2_2 = 410;
        SPR_FSCOUT_W2_3 = 411;
        SPR_FSCOUT_W2_4 = 412;
        SPR_FSCOUT_W2_5 = 413;
        SPR_FSCOUT_W2_6 = 414;
        SPR_FSCOUT_W2_7 = 415;
        SPR_FSCOUT_W2_8 = 416;
        SPR_FSCOUT_W3_1 = 417;
        SPR_FSCOUT_W3_2 = 418;
        SPR_FSCOUT_W3_3 = 419;
        SPR_FSCOUT_W3_4 = 420;
        SPR_FSCOUT_W3_5 = 421;
        SPR_FSCOUT_W3_6 = 422;
        SPR_FSCOUT_W3_7 = 423;
        SPR_FSCOUT_W3_8 = 424;
        SPR_FSCOUT_W4_1 = 425;
        SPR_FSCOUT_W4_2 = 426;
        SPR_FSCOUT_W4_3 = 427;
        SPR_FSCOUT_W4_4 = 428;
        SPR_FSCOUT_W4_5 = 429;
        SPR_FSCOUT_W4_6 = 430;
        SPR_FSCOUT_W4_7 = 431;
        SPR_FSCOUT_W4_8 = 432;
        SPR_FSCOUT_DIE1 = 433;
        SPR_FSCOUT_DIE2 = 434;
        SPR_FSCOUT_DIE3 = 435;
        SPR_FSCOUT_DIE4 = 436;
        SPR_FSCOUT_DIE5 = 437;
        SPR_FSCOUT_DIE6 = 438;
        SPR_FSCOUT_DIE7 = 439;
        SPR_FSCOUT_DEAD = 440;

        SPR_EXPLOSION_1 = 441;
        SPR_EXPLOSION_2 = 442;
        SPR_EXPLOSION_3 = 443;
        SPR_EXPLOSION_4 = 444;
        SPR_EXPLOSION_5 = 445;

        SPR_VITAL_STAND = 446;
        SPR_VITAL_DIE_1 = 447;
        SPR_VITAL_DIE_2 = 448;
        SPR_VITAL_DIE_3 = 449;
        SPR_VITAL_DIE_4 = 450;
        SPR_VITAL_DIE_5 = 451;
        SPR_VITAL_DIE_6 = 452;
        SPR_VITAL_DIE_7 = 453;
        SPR_VITAL_DIE_8 = 454;
        SPR_VITAL_DEAD_1 = 455;
        SPR_VITAL_DEAD_2 = 456;
        SPR_VITAL_DEAD_3 = 457;
        SPR_VITAL_OUCH = 458;

        SPR_SECURITY_NORMAL = 459;
        SPR_SECURITY_ALERT = 460;

        SPR_POD_EGG = 461;
        SPR_POD_HATCH1 = 462;
        SPR_POD_HATCH2 = 463;
        SPR_POD_HATCH3 = 464;
        SPR_POD_WALK1 = 465;
        SPR_POD_WALK2 = 466;
        SPR_POD_WALK3 = 467;
        SPR_POD_WALK4 = 468;
        SPR_POD_ATTACK1 = 469;
        SPR_POD_ATTACK2 = 470;
        SPR_POD_ATTACK3 = 471;
        SPR_POD_OUCH = 472;
        SPR_POD_DIE1 = 473;
        SPR_POD_DIE2 = 474;
        SPR_POD_DIE3 = 475;
        SPR_POD_SPIT1 = 476;
        SPR_POD_SPIT2 = 477;
        SPR_POD_SPIT3 = 478;

        SPR_ELEC_APPEAR1 = 479;
        SPR_ELEC_APPEAR2 = 480;
        SPR_ELEC_APPEAR3 = 481;
        SPR_ELEC_WALK1 = 482;
        SPR_ELEC_WALK2 = 483;
        SPR_ELEC_WALK3 = 484;
        SPR_ELEC_WALK4 = 485;
        SPR_ELEC_OUCH = 486;
        SPR_ELEC_SHOOT1 = 487;
        SPR_ELEC_SHOOT2 = 488;
        SPR_ELEC_SHOOT3 = 489;
        SPR_ELEC_DIE1 = 490;
        SPR_ELEC_DIE2 = 491;
        SPR_ELEC_DIE3 = 492;
        SPR_ELEC_SHOT1 = 493;
        SPR_ELEC_SHOT2 = 494;
        SPR_ELEC_SHOT_EXP1 = 495;
        SPR_ELEC_SHOT_EXP2 = 496;

        SPR_ELECTRO_SPHERE_ROAM1 = 497;
        SPR_ELECTRO_SPHERE_ROAM2 = 498;
        SPR_ELECTRO_SPHERE_ROAM3 = 499;
        SPR_ELECTRO_SPHERE_OUCH = 500;
        SPR_ELECTRO_SPHERE_DIE1 = 501;
        SPR_ELECTRO_SPHERE_DIE2 = 502;
        SPR_ELECTRO_SPHERE_DIE3 = 503;
        SPR_ELECTRO_SPHERE_DIE4 = 504;

        SPR_GENETIC_W1 = 505;
        SPR_GENETIC_W2 = 506;
        SPR_GENETIC_W3 = 507;
        SPR_GENETIC_W4 = 508;
        SPR_GENETIC_SWING1 = 509;
        SPR_GENETIC_SWING2 = 510;
        SPR_GENETIC_SWING3 = 511;
        SPR_GENETIC_DEAD = 512;
        SPR_GENETIC_DIE1 = 513;
        SPR_GENETIC_DIE2 = 514;
        SPR_GENETIC_DIE3 = 515;
        SPR_GENETIC_DIE4 = 516;
        SPR_GENETIC_OUCH = 517;
        SPR_GENETIC_SHOOT1 = 518;
        SPR_GENETIC_SHOOT2 = 519;
        SPR_GENETIC_SHOOT3 = 520;

        SPR_MUTHUM1_W1 = 521;
        SPR_MUTHUM1_W2 = 522;
        SPR_MUTHUM1_W3 = 523;
        SPR_MUTHUM1_W4 = 524;
        SPR_MUTHUM1_SWING1 = 525;
        SPR_MUTHUM1_SWING2 = 526;
        SPR_MUTHUM1_SWING3 = 527;
        SPR_MUTHUM1_DEAD = 528;
        SPR_MUTHUM1_DIE1 = 529;
        SPR_MUTHUM1_DIE2 = 530;
        SPR_MUTHUM1_DIE3 = 531;
        SPR_MUTHUM1_DIE4 = 532;
        SPR_MUTHUM1_OUCH = 533;
        SPR_MUTHUM1_SPIT1 = 534;
        SPR_MUTHUM1_SPIT2 = 535;
        SPR_MUTHUM1_SPIT3 = 536;

        SPR_MUTHUM2_W1 = 537;
        SPR_MUTHUM2_W2 = 538;
        SPR_MUTHUM2_W3 = 539;
        SPR_MUTHUM2_W4 = 540;
        SPR_MUTHUM2_SWING1 = 541;
        SPR_MUTHUM2_SWING2 = 542;
        SPR_MUTHUM2_SWING3 = 543;
        SPR_MUTHUM2_DEAD = 544;
        SPR_MUTHUM2_DIE1 = 545;
        SPR_MUTHUM2_DIE2 = 546;
        SPR_MUTHUM2_DIE3 = 547;
        SPR_MUTHUM2_DIE4 = 548;
        SPR_MUTHUM2_OUCH = 549;
        SPR_MUTHUM2_SPIT1 = 550;
        SPR_MUTHUM2_SPIT2 = 551;
        SPR_MUTHUM2_SPIT3 = 552;

        SPR_LCAN_ALIEN_READY = 553;
        SPR_LCAN_ALIEN_B1 = 554;
        SPR_LCAN_ALIEN_B2 = 555;
        SPR_LCAN_ALIEN_B3 = 556;
        SPR_LCAN_ALIEN_EMPTY = 557;
        SPR_LCAN_ALIEN_W1 = 558;
        SPR_LCAN_ALIEN_W2 = 559;
        SPR_LCAN_ALIEN_W3 = 560;
        SPR_LCAN_ALIEN_W4 = 561;
        SPR_LCAN_ALIEN_SWING1 = 562;
        SPR_LCAN_ALIEN_SWING2 = 563;
        SPR_LCAN_ALIEN_SWING3 = 564;
        SPR_LCAN_ALIEN_DEAD = 565;
        SPR_LCAN_ALIEN_DIE1 = 566;
        SPR_LCAN_ALIEN_DIE2 = 567;
        SPR_LCAN_ALIEN_DIE3 = 568;
        SPR_LCAN_ALIEN_DIE4 = 569;
        SPR_LCAN_ALIEN_OUCH = 570;
        SPR_LCAN_ALIEN_SPIT1 = 571;
        SPR_LCAN_ALIEN_SPIT2 = 572;
        SPR_LCAN_ALIEN_SPIT3 = 573;

        SPR_SCAN_ALIEN_READY = 574;
        SPR_SCAN_ALIEN_B1 = 575;
        SPR_SCAN_ALIEN_B2 = 576;
        SPR_SCAN_ALIEN_B3 = 577;
        SPR_SCAN_ALIEN_EMPTY = 578;
        SPR_SCAN_ALIEN_W1 = 579;
        SPR_SCAN_ALIEN_W2 = 580;
        SPR_SCAN_ALIEN_W3 = 581;
        SPR_SCAN_ALIEN_W4 = 582;
        SPR_SCAN_ALIEN_SWING1 = 583;
        SPR_SCAN_ALIEN_SWING2 = 584;
        SPR_SCAN_ALIEN_SWING3 = 585;
        SPR_SCAN_ALIEN_DEAD = 586;
        SPR_SCAN_ALIEN_DIE1 = 587;
        SPR_SCAN_ALIEN_DIE2 = 588;
        SPR_SCAN_ALIEN_DIE3 = 589;
        SPR_SCAN_ALIEN_DIE4 = 590;
        SPR_SCAN_ALIEN_OUCH = 591;
        SPR_SCAN_ALIEN_SPIT1 = 592;
        SPR_SCAN_ALIEN_SPIT2 = 593;
        SPR_SCAN_ALIEN_SPIT3 = 594;

        SPR_GURNEY_MUT_READY = 595;
        SPR_GURNEY_MUT_B1 = 596;
        SPR_GURNEY_MUT_B2 = 597;
        SPR_GURNEY_MUT_B3 = 598;
        SPR_GURNEY_MUT_EMPTY = 599;
        SPR_GURNEY_MUT_W1 = 600;
        SPR_GURNEY_MUT_W2 = 601;
        SPR_GURNEY_MUT_W3 = 602;
        SPR_GURNEY_MUT_W4 = 603;
        SPR_GURNEY_MUT_SWING1 = 604;
        SPR_GURNEY_MUT_SWING2 = 605;
        SPR_GURNEY_MUT_SWING3 = 606;
        SPR_GURNEY_MUT_DEAD = 607;
        SPR_GURNEY_MUT_DIE1 = 608;
        SPR_GURNEY_MUT_DIE2 = 609;
        SPR_GURNEY_MUT_DIE3 = 610;
        SPR_GURNEY_MUT_DIE4 = 611;
        SPR_GURNEY_MUT_OUCH = 612;

        SPR_LIQUID_M1 = 613;
        SPR_LIQUID_M2 = 614;
        SPR_LIQUID_M3 = 615;
        SPR_LIQUID_R1 = 616;
        SPR_LIQUID_R2 = 617;
        SPR_LIQUID_R3 = 618;
        SPR_LIQUID_R4 = 619;
        SPR_LIQUID_S1 = 620;
        SPR_LIQUID_S2 = 621;
        SPR_LIQUID_S3 = 622;
        SPR_LIQUID_OUCH = 623;
        SPR_LIQUID_DIE_1 = 624;
        SPR_LIQUID_DIE_2 = 625;
        SPR_LIQUID_DIE_3 = 626;
        SPR_LIQUID_DIE_4 = 627;
        SPR_LIQUID_DEAD = 628;
        SPR_LIQUID_SHOT_FLY_1 = 629;
        SPR_LIQUID_SHOT_FLY_2 = 630;
        SPR_LIQUID_SHOT_FLY_3 = 631;
        SPR_LIQUID_SHOT_BURST_1 = 632;
        SPR_LIQUID_SHOT_BURST_2 = 633;
        SPR_LIQUID_SHOT_BURST_3 = 634;

        SPR_SPIT1_1 = 635;
        SPR_SPIT1_2 = 636;
        SPR_SPIT1_3 = 637;
        SPR_SPIT_EXP1_1 = 638;
        SPR_SPIT_EXP1_2 = 639;
        SPR_SPIT_EXP1_3 = 640;

        SPR_SPIT2_1 = 641;
        SPR_SPIT2_2 = 642;
        SPR_SPIT2_3 = 643;
        SPR_SPIT_EXP2_1 = 644;
        SPR_SPIT_EXP2_2 = 645;
        SPR_SPIT_EXP2_3 = 646;

        SPR_SPIT3_1 = 647;
        SPR_SPIT3_2 = 648;
        SPR_SPIT3_3 = 649;
        SPR_SPIT_EXP3_1 = 650;
        SPR_SPIT_EXP3_2 = 651;
        SPR_SPIT_EXP3_3 = 652;

        SPR_TERROT_1 = 653;
        SPR_TERROT_2 = 654;
        SPR_TERROT_3 = 655;
        SPR_TERROT_4 = 656;
        SPR_TERROT_5 = 657;
        SPR_TERROT_6 = 658;
        SPR_TERROT_7 = 659;
        SPR_TERROT_8 = 660;
        SPR_TERROT_FIRE_1 = 661;
        SPR_TERROT_FIRE_2 = 662;
        SPR_TERROT_DIE_1 = 663;
        SPR_TERROT_DIE_2 = 664;
        SPR_TERROT_DIE_3 = 665;
        SPR_TERROT_DIE_4 = 666;
        SPR_TERROT_DEAD = 667;

        SPR_KNIFEREADY = 668;
        SPR_KNIFEATK1 = 669;
        SPR_KNIFEATK2 = 670;
        SPR_KNIFEATK3 = 671;
        SPR_KNIFEATK4 = 672;

        SPR_PISTOLREADY = 673;
        SPR_PISTOLATK1 = 674;
        SPR_PISTOLATK2 = 675;
        SPR_PISTOLATK3 = 676;
        SPR_PISTOLATK4 = 677;

        SPR_MACHINEGUNREADY = 678;
        SPR_MACHINEGUNATK1 = 679;
        SPR_MACHINEGUNATK2 = 680;
        SPR_MACHINEGUNATK3 = 681;
        SPR_MACHINEGUNATK4 = 682;

        SPR_CHAINREADY = 683;
        SPR_CHAINATK1 = 684;
        SPR_CHAINATK2 = 685;
        SPR_CHAINATK3 = 686;
        SPR_CHAINATK4 = 687;

        SPR_GRENADEREADY = 688;
        SPR_GRENADEATK1 = 689;
        SPR_GRENADEATK2 = 690;
        SPR_GRENADEATK3 = 691;
        SPR_GRENADEATK4 = 692;
        SPR_GRENADE_FLY1 = 693;
        SPR_GRENADE_FLY2 = 694;
        SPR_GRENADE_FLY3 = 695;
        SPR_GRENADE_FLY4 = 696;
        SPR_GRENADE_EXPLODE1 = 697;
        SPR_GRENADE_EXPLODE2 = 698;
        SPR_GRENADE_EXPLODE3 = 699;
        SPR_GRENADE_EXPLODE4 = 700;
        SPR_GRENADE_EXPLODE5 = 701;

        SPR_ELEC_ARC1 = 702;
        SPR_ELEC_ARC2 = 703;
        SPR_ELEC_ARC3 = 704;
        SPR_ELEC_ARC4 = 705;
        SPR_ELEC_POST1 = 706;
        SPR_ELEC_POST2 = 707;
        SPR_ELEC_POST3 = 708;
        SPR_ELEC_POST4 = 709;

        SPR_GREEN_OOZE1 = 710;
        SPR_GREEN_OOZE2 = 711;
        SPR_GREEN_OOZE3 = 712;

        SPR_BLACK_OOZE1 = 713;
        SPR_BLACK_OOZE2 = 714;
        SPR_BLACK_OOZE3 = 715;

        SPR_CANDY_BAR = 716;
        SPR_CANDY_WRAPER = 717;

        SPR_SANDWICH = 718;
        SPR_SANDWICH_WRAPER = 719;

        SPR_BLAKE_W1 = 720;
        SPR_BLAKE_W2 = 721;
        SPR_BLAKE_W3 = 722;
        SPR_BLAKE_W4 = 723;

        SPR_BOSS1_W1 = 724;
        SPR_BOSS1_W2 = 725;
        SPR_BOSS1_W3 = 726;
        SPR_BOSS1_W4 = 727;
        SPR_BOSS1_SWING1 = 728;
        SPR_BOSS1_SWING2 = 729;
        SPR_BOSS1_SWING3 = 730;
        SPR_BOSS1_DEAD = 731;
        SPR_BOSS1_DIE1 = 732;
        SPR_BOSS1_DIE2 = 733;
        SPR_BOSS1_DIE3 = 734;
        SPR_BOSS1_DIE4 = 735;
        SPR_BOSS1_OUCH = 736;
        SPR_BOSS1_PROJ1 = 737;
        SPR_BOSS1_PROJ2 = 738;
        SPR_BOSS1_PROJ3 = 739;
        SPR_BOSS1_EXP1 = 740;
        SPR_BOSS1_EXP2 = 741;
        SPR_BOSS1_EXP3 = 742;

        SPR_BOSS2_W1 = 743;
        SPR_BOSS2_W2 = 744;
        SPR_BOSS2_W3 = 745;
        SPR_BOSS2_W4 = 746;
        SPR_BOSS2_SWING1 = 747;
        SPR_BOSS2_SWING2 = 748;
        SPR_BOSS2_SWING3 = 749;
        SPR_BOSS2_DEAD = 750;
        SPR_BOSS2_DIE1 = 751;
        SPR_BOSS2_DIE2 = 752;
        SPR_BOSS2_DIE3 = 753;
        SPR_BOSS2_DIE4 = 754;
        SPR_BOSS2_OUCH = 755;

        SPR_BOSS3_W1 = 756;
        SPR_BOSS3_W2 = 757;
        SPR_BOSS3_W3 = 758;
        SPR_BOSS3_W4 = 759;
        SPR_BOSS3_SWING1 = 760;
        SPR_BOSS3_SWING2 = 761;
        SPR_BOSS3_SWING3 = 762;
        SPR_BOSS3_DEAD = 763;
        SPR_BOSS3_DIE1 = 764;
        SPR_BOSS3_DIE2 = 765;
        SPR_BOSS3_DIE3 = 766;
        SPR_BOSS3_DIE4 = 767;
        SPR_BOSS3_OUCH = 768;

        SPR_BOSS4_W1 = 769;
        SPR_BOSS4_W2 = 770;
        SPR_BOSS4_W3 = 771;
        SPR_BOSS4_W4 = 772;
        SPR_BOSS4_SWING1 = 773;
        SPR_BOSS4_SWING2 = 774;
        SPR_BOSS4_SWING3 = 775;
        SPR_BOSS4_DEAD = 776;
        SPR_BOSS4_DIE1 = 777;
        SPR_BOSS4_DIE2 = 778;
        SPR_BOSS4_DIE3 = 779;
        SPR_BOSS4_DIE4 = 780;
        SPR_BOSS4_OUCH = 781;

        SPR_BOSS5_W1 = 782;
        SPR_BOSS5_W2 = 783;
        SPR_BOSS5_W3 = 784;
        SPR_BOSS5_W4 = 785;
        SPR_BOSS5_SWING1 = 786;
        SPR_BOSS5_SWING2 = 787;
        SPR_BOSS5_SWING3 = 788;
        SPR_BOSS5_DEAD = 789;
        SPR_BOSS5_DIE1 = 790;
        SPR_BOSS5_DIE2 = 791;
        SPR_BOSS5_DIE3 = 792;
        SPR_BOSS5_DIE4 = 793;
        SPR_BOSS5_OUCH = 794;
        SPR_BOSS5_PROJ1 = 795;
        SPR_BOSS5_PROJ2 = 796;
        SPR_BOSS5_PROJ3 = 797;
        SPR_BOSS5_EXP1 = 798;
        SPR_BOSS5_EXP2 = 799;
        SPR_BOSS5_EXP3 = 800;

        SPR_BOSS6_W1 = 801;
        SPR_BOSS6_W2 = 802;
        SPR_BOSS6_W3 = 803;
        SPR_BOSS6_W4 = 804;
        SPR_BOSS6_SWING1 = 805;
        SPR_BOSS6_SWING2 = 806;
        SPR_BOSS6_SWING3 = 807;
        SPR_BOSS6_DEAD = 808;
        SPR_BOSS6_DIE1 = 809;
        SPR_BOSS6_DIE2 = 810;
        SPR_BOSS6_DIE3 = 811;
        SPR_BOSS6_DIE4 = 812;
        SPR_BOSS6_OUCH = 813;
    } else if (::is_ps()) {
        SPR_STAT_0 = 1;
        SPR_STAT_1 = 2;
        SPR_STAT_2 = 3;
        SPR_STAT_3 = 4;
        SPR_STAT_4 = 5;
        SPR_STAT_5 = 6;
        SPR_STAT_6 = 7;
        SPR_STAT_7 = 8;
        SPR_STAT_8 = 9;
        SPR_STAT_9 = 10;
        SPR_STAT_10 = 11;
        SPR_STAT_11 = 12;
        SPR_STAT_12 = 13;
        SPR_STAT_13 = 14;
        SPR_STAT_14 = 15;
        SPR_STAT_15 = 16;
        SPR_STAT_16 = 17;
        SPR_STAT_17 = 18;
        SPR_STAT_18 = 19;
        SPR_STAT_19 = 20;
        SPR_STAT_20 = 21;
        SPR_STAT_21 = 22;
        SPR_STAT_22 = 23;
        SPR_STAT_23 = 24;
        SPR_STAT_24 = 25;
        SPR_STAT_25 = 26;
        SPR_STAT_26 = 27;
        SPR_STAT_27 = 28;
        SPR_STAT_28 = 29;
        SPR_STAT_29 = 30;
        SPR_STAT_30 = 31;
        SPR_STAT_31 = 32;
        SPR_STAT_32 = 33;
        SPR_STAT_33 = 34;
        SPR_STAT_34 = 35;
        SPR_STAT_35 = 36;
        SPR_STAT_36 = 37;
        SPR_STAT_37 = 38;
        SPR_STAT_38 = 39;
        SPR_STAT_39 = 40;
        SPR_STAT_40 = 41;
        SPR_STAT_41 = 42;
        SPR_STAT_42 = 43;
        SPR_STAT_43 = 44;
        SPR_STAT_44 = 45;
        SPR_STAT_45 = 46;
        SPR_STAT_46 = 47;
        SPR_STAT_47 = 48;
        SPR_STAT_48 = 49;
        SPR_STAT_49 = 50;
        SPR_STAT_50 = 51;
        SPR_STAT_51 = 52;
        SPR_STAT_52 = 53;
        SPR_STAT_53 = 54;
        SPR_STAT_54 = 55;
        SPR_STAT_55 = 56;
        SPR_STAT_56 = 57;

        SPR_CRATE_1 = 58;
        SPR_CRATE_2 = 59;
        SPR_CRATE_3 = 60;

        SPR_STAT_57 = 61;
        SPR_STAT_58 = 62;
        SPR_STAT_59 = 63;
        SPR_STAT_60 = 64;
        SPR_STAT_61 = 65;
        SPR_STAT_62 = 66;
        SPR_STAT_63 = 67;
        SPR_STAT_64 = 68;
        SPR_STAT_65 = 69;
        SPR_STAT_66 = 70;
        SPR_STAT_67 = 71;
        SPR_STAT_68 = 72;
        SPR_STAT_69 = 73;
        SPR_STAT_70 = 74;
        SPR_STAT_71 = 75;
        SPR_STAT_72 = 76;
        SPR_STAT_73 = 77;
        SPR_STAT_74 = 78;
        SPR_STAT_75 = 79;
        SPR_STAT_76 = 80;
        SPR_STAT_77 = 81;
        SPR_STAT_78 = 82;
        SPR_STAT_79 = 83;

        SPR_DOORBOMB = 84;
        SPR_ALT_DOORBOMB = 85;

        SPR_RUBBLE = 86;
        SPR_BONZI_TREE = 87;
        SPR_AUTOMAPPER = 88;
        SPR_POT_PLANT = 89;
        SPR_TUBE_PLANT = 90;
        SPR_HITECH_CHAIR = 91;

        SPR_AIR_VENT = 92;

        SPR_BLOOD_DRIP1 = 93;
        SPR_BLOOD_DRIP2 = 94;
        SPR_BLOOD_DRIP3 = 95;
        SPR_BLOOD_DRIP4 = 96;
        SPR_WATER_DRIP1 = 97;
        SPR_WATER_DRIP2 = 98;
        SPR_WATER_DRIP3 = 99;
        SPR_WATER_DRIP4 = 100;

        SPR_DECO_ARC_1 = 101;
        SPR_DECO_ARC_2 = 102;
        SPR_DECO_ARC_3 = 103;

        SPR_GRATE = 104;
        SPR_STEAM_1 = 105;
        SPR_STEAM_2 = 106;
        SPR_STEAM_3 = 107;
        SPR_STEAM_4 = 108;
        SPR_STEAM_PIPE = 109;
        SPR_PIPE_STEAM_1 = 110;
        SPR_PIPE_STEAM_2 = 111;
        SPR_PIPE_STEAM_3 = 112;
        SPR_PIPE_STEAM_4 = 113;

        SPR_DEAD_RENT = 114;
        SPR_DEAD_PRO = 115;
        SPR_DEAD_SWAT = 116;

        SPR_RENT_S_1 = 117;
        SPR_RENT_S_2 = 118;
        SPR_RENT_S_3 = 119;
        SPR_RENT_S_4 = 120;
        SPR_RENT_S_5 = 121;
        SPR_RENT_S_6 = 122;
        SPR_RENT_S_7 = 123;
        SPR_RENT_S_8 = 124;
        SPR_RENT_W1_1 = 125;
        SPR_RENT_W1_2 = 126;
        SPR_RENT_W1_3 = 127;
        SPR_RENT_W1_4 = 128;
        SPR_RENT_W1_5 = 129;
        SPR_RENT_W1_6 = 130;
        SPR_RENT_W1_7 = 131;
        SPR_RENT_W1_8 = 132;
        SPR_RENT_W2_1 = 133;
        SPR_RENT_W2_2 = 134;
        SPR_RENT_W2_3 = 135;
        SPR_RENT_W2_4 = 136;
        SPR_RENT_W2_5 = 137;
        SPR_RENT_W2_6 = 138;
        SPR_RENT_W2_7 = 139;
        SPR_RENT_W2_8 = 140;
        SPR_RENT_W3_1 = 141;
        SPR_RENT_W3_2 = 142;
        SPR_RENT_W3_3 = 143;
        SPR_RENT_W3_4 = 144;
        SPR_RENT_W3_5 = 145;
        SPR_RENT_W3_6 = 146;
        SPR_RENT_W3_7 = 147;
        SPR_RENT_W3_8 = 148;
        SPR_RENT_W4_1 = 149;
        SPR_RENT_W4_2 = 150;
        SPR_RENT_W4_3 = 151;
        SPR_RENT_W4_4 = 152;
        SPR_RENT_W4_5 = 153;
        SPR_RENT_W4_6 = 154;
        SPR_RENT_W4_7 = 155;
        SPR_RENT_W4_8 = 156;
        SPR_RENT_DIE_1 = 157;
        SPR_RENT_DIE_2 = 158;
        SPR_RENT_DIE_3 = 159;
        SPR_RENT_DIE_4 = 160;
        SPR_RENT_PAIN_1 = 161;
        SPR_RENT_DEAD = 162;
        SPR_RENT_SHOOT1 = 163;
        SPR_RENT_SHOOT2 = 164;
        SPR_RENT_SHOOT3 = 165;

        SPR_PRO_S_1 = 166;
        SPR_PRO_S_2 = 167;
        SPR_PRO_S_3 = 168;
        SPR_PRO_S_4 = 169;
        SPR_PRO_S_5 = 170;
        SPR_PRO_S_6 = 171;
        SPR_PRO_S_7 = 172;
        SPR_PRO_S_8 = 173;
        SPR_PRO_W1_1 = 174;
        SPR_PRO_W1_2 = 175;
        SPR_PRO_W1_3 = 176;
        SPR_PRO_W1_4 = 177;
        SPR_PRO_W1_5 = 178;
        SPR_PRO_W1_6 = 179;
        SPR_PRO_W1_7 = 180;
        SPR_PRO_W1_8 = 181;
        SPR_PRO_W2_1 = 182;
        SPR_PRO_W2_2 = 183;
        SPR_PRO_W2_3 = 184;
        SPR_PRO_W2_4 = 185;
        SPR_PRO_W2_5 = 186;
        SPR_PRO_W2_6 = 187;
        SPR_PRO_W2_7 = 188;
        SPR_PRO_W2_8 = 189;
        SPR_PRO_W3_1 = 190;
        SPR_PRO_W3_2 = 191;
        SPR_PRO_W3_3 = 192;
        SPR_PRO_W3_4 = 193;
        SPR_PRO_W3_5 = 194;
        SPR_PRO_W3_6 = 195;
        SPR_PRO_W3_7 = 196;
        SPR_PRO_W3_8 = 197;
        SPR_PRO_W4_1 = 198;
        SPR_PRO_W4_2 = 199;
        SPR_PRO_W4_3 = 200;
        SPR_PRO_W4_4 = 201;
        SPR_PRO_W4_5 = 202;
        SPR_PRO_W4_6 = 203;
        SPR_PRO_W4_7 = 204;
        SPR_PRO_W4_8 = 205;
        SPR_PRO_PAIN_1 = 206;
        SPR_PRO_DIE_1 = 207;
        SPR_PRO_DIE_2 = 208;
        SPR_PRO_DIE_3 = 209;
        SPR_PRO_PAIN_2 = 210;
        SPR_PRO_DIE_4 = 211;
        SPR_PRO_DEAD = 212;
        SPR_PRO_SHOOT1 = 213;
        SPR_PRO_SHOOT2 = 214;
        SPR_PRO_SHOOT3 = 215;

        SPR_SWAT_S_1 = 216;
        SPR_SWAT_S_2 = 217;
        SPR_SWAT_S_3 = 218;
        SPR_SWAT_S_4 = 219;
        SPR_SWAT_S_5 = 220;
        SPR_SWAT_S_6 = 221;
        SPR_SWAT_S_7 = 222;
        SPR_SWAT_S_8 = 223;
        SPR_SWAT_W1_1 = 224;
        SPR_SWAT_W1_2 = 225;
        SPR_SWAT_W1_3 = 226;
        SPR_SWAT_W1_4 = 227;
        SPR_SWAT_W1_5 = 228;
        SPR_SWAT_W1_6 = 229;
        SPR_SWAT_W1_7 = 230;
        SPR_SWAT_W1_8 = 231;
        SPR_SWAT_W2_1 = 232;
        SPR_SWAT_W2_2 = 233;
        SPR_SWAT_W2_3 = 234;
        SPR_SWAT_W2_4 = 235;
        SPR_SWAT_W2_5 = 236;
        SPR_SWAT_W2_6 = 237;
        SPR_SWAT_W2_7 = 238;
        SPR_SWAT_W2_8 = 239;
        SPR_SWAT_W3_1 = 240;
        SPR_SWAT_W3_2 = 241;
        SPR_SWAT_W3_3 = 242;
        SPR_SWAT_W3_4 = 243;
        SPR_SWAT_W3_5 = 244;
        SPR_SWAT_W3_6 = 245;
        SPR_SWAT_W3_7 = 246;
        SPR_SWAT_W3_8 = 247;
        SPR_SWAT_W4_1 = 248;
        SPR_SWAT_W4_2 = 249;
        SPR_SWAT_W4_3 = 250;
        SPR_SWAT_W4_4 = 251;
        SPR_SWAT_W4_5 = 252;
        SPR_SWAT_W4_6 = 253;
        SPR_SWAT_W4_7 = 254;
        SPR_SWAT_W4_8 = 255;
        SPR_SWAT_PAIN_1 = 256;
        SPR_SWAT_DIE_1 = 257;
        SPR_SWAT_DIE_2 = 258;
        SPR_SWAT_DIE_3 = 259;
        SPR_SWAT_PAIN_2 = 260;
        SPR_SWAT_DIE_4 = 261;
        SPR_SWAT_DEAD = 262;
        SPR_SWAT_SHOOT1 = 263;
        SPR_SWAT_SHOOT2 = 264;
        SPR_SWAT_SHOOT3 = 265;
        SPR_SWAT_WOUNDED1 = 266;
        SPR_SWAT_WOUNDED2 = 267;
        SPR_SWAT_WOUNDED3 = 268;
        SPR_SWAT_WOUNDED4 = 269;

        SPR_OFC_S_1 = 270;
        SPR_OFC_S_2 = 271;
        SPR_OFC_S_3 = 272;
        SPR_OFC_S_4 = 273;
        SPR_OFC_S_5 = 274;
        SPR_OFC_S_6 = 275;
        SPR_OFC_S_7 = 276;
        SPR_OFC_S_8 = 277;
        SPR_OFC_W1_1 = 278;
        SPR_OFC_W1_2 = 279;
        SPR_OFC_W1_3 = 280;
        SPR_OFC_W1_4 = 281;
        SPR_OFC_W1_5 = 282;
        SPR_OFC_W1_6 = 283;
        SPR_OFC_W1_7 = 284;
        SPR_OFC_W1_8 = 285;
        SPR_OFC_W2_1 = 286;
        SPR_OFC_W2_2 = 287;
        SPR_OFC_W2_3 = 288;
        SPR_OFC_W2_4 = 289;
        SPR_OFC_W2_5 = 290;
        SPR_OFC_W2_6 = 291;
        SPR_OFC_W2_7 = 292;
        SPR_OFC_W2_8 = 293;
        SPR_OFC_W3_1 = 294;
        SPR_OFC_W3_2 = 295;
        SPR_OFC_W3_3 = 296;
        SPR_OFC_W3_4 = 297;
        SPR_OFC_W3_5 = 298;
        SPR_OFC_W3_6 = 299;
        SPR_OFC_W3_7 = 300;
        SPR_OFC_W3_8 = 301;
        SPR_OFC_W4_1 = 302;
        SPR_OFC_W4_2 = 303;
        SPR_OFC_W4_3 = 304;
        SPR_OFC_W4_4 = 305;
        SPR_OFC_W4_5 = 306;
        SPR_OFC_W4_6 = 307;
        SPR_OFC_W4_7 = 308;
        SPR_OFC_W4_8 = 309;
        SPR_OFC_PAIN_1 = 310;
        SPR_OFC_DIE_1 = 311;
        SPR_OFC_DIE_2 = 312;
        SPR_OFC_DIE_3 = 313;
        SPR_OFC_PAIN_2 = 314;
        SPR_OFC_DIE_4 = 315;
        SPR_OFC_DEAD = 316;
        SPR_OFC_SHOOT1 = 317;
        SPR_OFC_SHOOT2 = 318;
        SPR_OFC_SHOOT3 = 319;

        SPR_GOLD_S_1 = 320;
        SPR_GOLD_S_2 = 321;
        SPR_GOLD_S_3 = 322;
        SPR_GOLD_S_4 = 323;
        SPR_GOLD_S_5 = 324;
        SPR_GOLD_S_6 = 325;
        SPR_GOLD_S_7 = 326;
        SPR_GOLD_S_8 = 327;
        SPR_GOLD_W1_1 = 328;
        SPR_GOLD_W1_2 = 329;
        SPR_GOLD_W1_3 = 330;
        SPR_GOLD_W1_4 = 331;
        SPR_GOLD_W1_5 = 332;
        SPR_GOLD_W1_6 = 333;
        SPR_GOLD_W1_7 = 334;
        SPR_GOLD_W1_8 = 335;
        SPR_GOLD_W2_1 = 336;
        SPR_GOLD_W2_2 = 337;
        SPR_GOLD_W2_3 = 338;
        SPR_GOLD_W2_4 = 339;
        SPR_GOLD_W2_5 = 340;
        SPR_GOLD_W2_6 = 341;
        SPR_GOLD_W2_7 = 342;
        SPR_GOLD_W2_8 = 343;
        SPR_GOLD_W3_1 = 344;
        SPR_GOLD_W3_2 = 345;
        SPR_GOLD_W3_3 = 346;
        SPR_GOLD_W3_4 = 347;
        SPR_GOLD_W3_5 = 348;
        SPR_GOLD_W3_6 = 349;
        SPR_GOLD_W3_7 = 350;
        SPR_GOLD_W3_8 = 351;
        SPR_GOLD_W4_1 = 352;
        SPR_GOLD_W4_2 = 353;
        SPR_GOLD_W4_3 = 354;
        SPR_GOLD_W4_4 = 355;
        SPR_GOLD_W4_5 = 356;
        SPR_GOLD_W4_6 = 357;
        SPR_GOLD_W4_7 = 358;
        SPR_GOLD_W4_8 = 359;
        SPR_GOLD_PAIN_1 = 360;
        SPR_GOLD_WRIST_1 = 361;
        SPR_GOLD_WRIST_2 = 362;
        SPR_GOLD_SHOOT1 = 363;
        SPR_GOLD_SHOOT2 = 364;
        SPR_GOLD_SHOOT3 = 365;
        SPR_GOLD_WARP1 = 366;
        SPR_GOLD_WARP2 = 367;
        SPR_GOLD_WARP3 = 368;
        SPR_GOLD_WARP4 = 369;
        SPR_GOLD_WARP5 = 370;
        SPR_GOLD_DEATH1 = 371;
        SPR_GOLD_DEATH2 = 372;
        SPR_GOLD_DEATH3 = 373;
        SPR_GOLD_DEATH4 = 374;
        SPR_GOLD_DEATH5 = 375;
        SPR_MGOLD_OUCH = 376;
        SPR_GOLD_MORPH1 = 377;
        SPR_GOLD_MORPH2 = 378;
        SPR_GOLD_MORPH3 = 379;
        SPR_GOLD_MORPH4 = 380;
        SPR_GOLD_MORPH5 = 381;
        SPR_GOLD_MORPH6 = 382;
        SPR_GOLD_MORPH7 = 383;
        SPR_GOLD_MORPH8 = 384;

        SPR_MGOLD_WALK1 = 385;
        SPR_MGOLD_WALK2 = 386;
        SPR_MGOLD_WALK3 = 387;
        SPR_MGOLD_WALK4 = 388;
        SPR_MGOLD_ATTACK1 = 389;
        SPR_MGOLD_ATTACK2 = 390;
        SPR_MGOLD_ATTACK3 = 391;
        SPR_MGOLD_ATTACK4 = 392;
        SPR_MGOLD_SHOT1 = 393;
        SPR_MGOLD_SHOT2 = 394;
        SPR_MGOLD_SHOT3 = 395;
        SPR_MGOLD_SHOT_EXP1 = 396;
        SPR_MGOLD_SHOT_EXP2 = 397;
        SPR_MGOLD_SHOT_EXP3 = 398;

        SPR_GSCOUT_W1_1 = 399;
        SPR_GSCOUT_W1_2 = 400;
        SPR_GSCOUT_W1_3 = 401;
        SPR_GSCOUT_W1_4 = 402;
        SPR_GSCOUT_W1_5 = 403;
        SPR_GSCOUT_W1_6 = 404;
        SPR_GSCOUT_W1_7 = 405;
        SPR_GSCOUT_W1_8 = 406;
        SPR_GSCOUT_W2_1 = 407;
        SPR_GSCOUT_W2_2 = 408;
        SPR_GSCOUT_W2_3 = 409;
        SPR_GSCOUT_W2_4 = 410;
        SPR_GSCOUT_W2_5 = 411;
        SPR_GSCOUT_W2_6 = 412;
        SPR_GSCOUT_W2_7 = 413;
        SPR_GSCOUT_W2_8 = 414;
        SPR_GSCOUT_W3_1 = 415;
        SPR_GSCOUT_W3_2 = 416;
        SPR_GSCOUT_W3_3 = 417;
        SPR_GSCOUT_W3_4 = 418;
        SPR_GSCOUT_W3_5 = 419;
        SPR_GSCOUT_W3_6 = 420;
        SPR_GSCOUT_W3_7 = 421;
        SPR_GSCOUT_W3_8 = 422;
        SPR_GSCOUT_W4_1 = 423;
        SPR_GSCOUT_W4_2 = 424;
        SPR_GSCOUT_W4_3 = 425;
        SPR_GSCOUT_W4_4 = 426;
        SPR_GSCOUT_W4_5 = 427;
        SPR_GSCOUT_W4_6 = 428;
        SPR_GSCOUT_W4_7 = 429;
        SPR_GSCOUT_W4_8 = 430;
        SPR_GSCOUT_DIE1 = 431;
        SPR_GSCOUT_DIE2 = 432;
        SPR_GSCOUT_DIE3 = 433;
        SPR_GSCOUT_DIE4 = 434;
        SPR_GSCOUT_DIE5 = 435;
        SPR_GSCOUT_DIE6 = 436;
        SPR_GSCOUT_DIE7 = 437;
        SPR_GSCOUT_DIE8 = 438;
        SPR_GSCOUT_DEAD = 439;

        SPR_FSCOUT_W1_1 = 440;
        SPR_FSCOUT_W1_2 = 441;
        SPR_FSCOUT_W1_3 = 442;
        SPR_FSCOUT_W1_4 = 443;
        SPR_FSCOUT_W1_5 = 444;
        SPR_FSCOUT_W1_6 = 445;
        SPR_FSCOUT_W1_7 = 446;
        SPR_FSCOUT_W1_8 = 447;
        SPR_FSCOUT_W2_1 = 448;
        SPR_FSCOUT_W2_2 = 449;
        SPR_FSCOUT_W2_3 = 450;
        SPR_FSCOUT_W2_4 = 451;
        SPR_FSCOUT_W2_5 = 452;
        SPR_FSCOUT_W2_6 = 453;
        SPR_FSCOUT_W2_7 = 454;
        SPR_FSCOUT_W2_8 = 455;
        SPR_FSCOUT_W3_1 = 456;
        SPR_FSCOUT_W3_2 = 457;
        SPR_FSCOUT_W3_3 = 458;
        SPR_FSCOUT_W3_4 = 459;
        SPR_FSCOUT_W3_5 = 460;
        SPR_FSCOUT_W3_6 = 461;
        SPR_FSCOUT_W3_7 = 462;
        SPR_FSCOUT_W3_8 = 463;
        SPR_FSCOUT_W4_1 = 464;
        SPR_FSCOUT_W4_2 = 465;
        SPR_FSCOUT_W4_3 = 466;
        SPR_FSCOUT_W4_4 = 467;
        SPR_FSCOUT_W4_5 = 468;
        SPR_FSCOUT_W4_6 = 469;
        SPR_FSCOUT_W4_7 = 470;
        SPR_FSCOUT_W4_8 = 471;
        SPR_FSCOUT_DIE1 = 472;
        SPR_FSCOUT_DIE2 = 473;
        SPR_FSCOUT_DIE3 = 474;
        SPR_FSCOUT_DIE4 = 475;
        SPR_FSCOUT_DIE5 = 476;
        SPR_FSCOUT_DIE6 = 477;
        SPR_FSCOUT_DIE7 = 478;
        SPR_FSCOUT_DEAD = 479;

        SPR_EXPLOSION_1 = 480;
        SPR_EXPLOSION_2 = 481;
        SPR_EXPLOSION_3 = 482;
        SPR_EXPLOSION_4 = 483;
        SPR_EXPLOSION_5 = 484;

        SPR_CUBE1 = 485;
        SPR_CUBE2 = 486;
        SPR_CUBE3 = 487;
        SPR_CUBE4 = 488;
        SPR_CUBE5 = 489;
        SPR_CUBE6 = 490;
        SPR_CUBE7 = 491;
        SPR_CUBE8 = 492;
        SPR_CUBE9 = 493;
        SPR_CUBE10 = 494;
        SPR_CUBE_EXP1 = 495;
        SPR_CUBE_EXP2 = 496;
        SPR_CUBE_EXP3 = 497;
        SPR_CUBE_EXP4 = 498;
        SPR_CUBE_EXP5 = 499;
        SPR_CUBE_EXP6 = 500;
        SPR_CUBE_EXP7 = 501;
        SPR_CUBE_EXP8 = 502;
        SPR_DEAD_CUBE = 503;

        SPR_SECURITY_NORMAL = 504;
        SPR_SECURITY_ALERT = 505;

        SPR_POD_EGG = 506;
        SPR_POD_HATCH1 = 507;
        SPR_POD_HATCH2 = 508;
        SPR_POD_HATCH3 = 509;
        SPR_POD_WALK1 = 510;
        SPR_POD_WALK2 = 511;
        SPR_POD_WALK3 = 512;
        SPR_POD_WALK4 = 513;
        SPR_POD_ATTACK1 = 514;
        SPR_POD_ATTACK2 = 515;
        SPR_POD_ATTACK3 = 516;
        SPR_POD_OUCH = 517;
        SPR_POD_DIE1 = 518;
        SPR_POD_DIE2 = 519;
        SPR_POD_DIE3 = 520;
        SPR_POD_SPIT1 = 521;
        SPR_POD_SPIT2 = 522;
        SPR_POD_SPIT3 = 523;

        SPR_ELEC_APPEAR1 = 524;
        SPR_ELEC_APPEAR2 = 525;
        SPR_ELEC_APPEAR3 = 526;
        SPR_ELEC_WALK1 = 527;
        SPR_ELEC_WALK2 = 528;
        SPR_ELEC_WALK3 = 529;
        SPR_ELEC_WALK4 = 530;
        SPR_ELEC_OUCH = 531;
        SPR_ELEC_SHOOT1 = 532;
        SPR_ELEC_SHOOT2 = 533;
        SPR_ELEC_SHOOT3 = 534;
        SPR_ELEC_DIE1 = 535;
        SPR_ELEC_DIE2 = 536;
        SPR_ELEC_DIE3 = 537;
        SPR_ELEC_SHOT1 = 538;
        SPR_ELEC_SHOT2 = 539;
        SPR_ELEC_SHOT_EXP1 = 540;
        SPR_ELEC_SHOT_EXP2 = 541;

        SPR_ELECTRO_SPHERE_ROAM1 = 542;
        SPR_ELECTRO_SPHERE_ROAM2 = 543;
        SPR_ELECTRO_SPHERE_ROAM3 = 544;
        SPR_ELECTRO_SPHERE_OUCH = 545;
        SPR_ELECTRO_SPHERE_DIE1 = 546;
        SPR_ELECTRO_SPHERE_DIE2 = 547;
        SPR_ELECTRO_SPHERE_DIE3 = 548;
        SPR_ELECTRO_SPHERE_DIE4 = 549;

        SPR_GENETIC_W1 = 550;
        SPR_GENETIC_W2 = 551;
        SPR_GENETIC_W3 = 552;
        SPR_GENETIC_W4 = 553;
        SPR_GENETIC_SWING1 = 554;
        SPR_GENETIC_SWING2 = 555;
        SPR_GENETIC_SWING3 = 556;
        SPR_GENETIC_DEAD = 557;
        SPR_GENETIC_DIE1 = 558;
        SPR_GENETIC_DIE2 = 559;
        SPR_GENETIC_DIE3 = 560;
        SPR_GENETIC_DIE4 = 561;
        SPR_GENETIC_OUCH = 562;
        SPR_GENETIC_SHOOT1 = 563;
        SPR_GENETIC_SHOOT2 = 564;
        SPR_GENETIC_SHOOT3 = 565;

        SPR_MUTHUM1_W1 = 566;
        SPR_MUTHUM1_W2 = 567;
        SPR_MUTHUM1_W3 = 568;
        SPR_MUTHUM1_W4 = 569;
        SPR_MUTHUM1_SWING1 = 570;
        SPR_MUTHUM1_SWING2 = 571;
        SPR_MUTHUM1_SWING3 = 572;
        SPR_MUTHUM1_DEAD = 573;
        SPR_MUTHUM1_DIE1 = 574;
        SPR_MUTHUM1_DIE2 = 575;
        SPR_MUTHUM1_DIE3 = 576;
        SPR_MUTHUM1_DIE4 = 577;
        SPR_MUTHUM1_OUCH = 578;
        SPR_MUTHUM1_SPIT1 = 579;
        SPR_MUTHUM1_SPIT2 = 580;
        SPR_MUTHUM1_SPIT3 = 581;

        SPR_MUTHUM2_W1 = 582;
        SPR_MUTHUM2_W2 = 583;
        SPR_MUTHUM2_W3 = 584;
        SPR_MUTHUM2_W4 = 585;
        SPR_MUTHUM2_SWING1 = 586;
        SPR_MUTHUM2_SWING2 = 587;
        SPR_MUTHUM2_SWING3 = 588;
        SPR_MUTHUM2_DEAD = 589;
        SPR_MUTHUM2_DIE1 = 590;
        SPR_MUTHUM2_DIE2 = 591;
        SPR_MUTHUM2_DIE3 = 592;
        SPR_MUTHUM2_DIE4 = 593;
        SPR_MUTHUM2_OUCH = 594;
        SPR_MUTHUM2_SPIT1 = 595;
        SPR_MUTHUM2_SPIT2 = 596;
        SPR_MUTHUM2_SPIT3 = 597;
        SPR_MUTHUM2_MORPH1 = 598;
        SPR_MUTHUM2_MORPH2 = 599;
        SPR_MUTHUM2_MORPH3 = 600;
        SPR_MUTHUM2_MORPH4 = 601;
        SPR_MUTHUM2_MORPH5 = 602;
        SPR_MUTHUM2_MORPH6 = 603;
        SPR_MUTHUM2_MORPH7 = 604;
        SPR_MUTHUM2_MORPH8 = 605;
        SPR_MUTHUM2_MORPH9 = 606;

        SPR_LCAN_ALIEN_READY = 607;
        SPR_LCAN_ALIEN_B1 = 608;
        SPR_LCAN_ALIEN_B2 = 609;
        SPR_LCAN_ALIEN_B3 = 610;
        SPR_LCAN_ALIEN_EMPTY = 611;
        SPR_LCAN_ALIEN_W1 = 612;
        SPR_LCAN_ALIEN_W2 = 613;
        SPR_LCAN_ALIEN_W3 = 614;
        SPR_LCAN_ALIEN_W4 = 615;
        SPR_LCAN_ALIEN_SWING1 = 616;
        SPR_LCAN_ALIEN_SWING2 = 617;
        SPR_LCAN_ALIEN_SWING3 = 618;
        SPR_LCAN_ALIEN_DEAD = 619;
        SPR_LCAN_ALIEN_DIE1 = 620;
        SPR_LCAN_ALIEN_DIE2 = 621;
        SPR_LCAN_ALIEN_DIE3 = 622;
        SPR_LCAN_ALIEN_DIE4 = 623;
        SPR_LCAN_ALIEN_OUCH = 624;
        SPR_LCAN_ALIEN_SPIT1 = 625;
        SPR_LCAN_ALIEN_SPIT2 = 626;
        SPR_LCAN_ALIEN_SPIT3 = 627;

        SPR_SCAN_ALIEN_READY = 628;
        SPR_SCAN_ALIEN_B1 = 629;
        SPR_SCAN_ALIEN_B2 = 630;
        SPR_SCAN_ALIEN_B3 = 631;
        SPR_SCAN_ALIEN_EMPTY = 632;
        SPR_SCAN_ALIEN_W1 = 633;
        SPR_SCAN_ALIEN_W2 = 634;
        SPR_SCAN_ALIEN_W3 = 635;
        SPR_SCAN_ALIEN_W4 = 636;
        SPR_SCAN_ALIEN_SWING1 = 637;
        SPR_SCAN_ALIEN_SWING2 = 638;
        SPR_SCAN_ALIEN_SWING3 = 639;
        SPR_SCAN_ALIEN_DEAD = 640;
        SPR_SCAN_ALIEN_DIE1 = 641;
        SPR_SCAN_ALIEN_DIE2 = 642;
        SPR_SCAN_ALIEN_DIE3 = 643;
        SPR_SCAN_ALIEN_DIE4 = 644;
        SPR_SCAN_ALIEN_OUCH = 645;
        SPR_SCAN_ALIEN_SPIT1 = 646;
        SPR_SCAN_ALIEN_SPIT2 = 647;
        SPR_SCAN_ALIEN_SPIT3 = 648;

        SPR_GURNEY_MUT_READY = 649;
        SPR_GURNEY_MUT_B1 = 650;
        SPR_GURNEY_MUT_B2 = 651;
        SPR_GURNEY_MUT_B3 = 652;
        SPR_GURNEY_MUT_EMPTY = 653;
        SPR_GURNEY_MUT_W1 = 654;
        SPR_GURNEY_MUT_W2 = 655;
        SPR_GURNEY_MUT_W3 = 656;
        SPR_GURNEY_MUT_W4 = 657;
        SPR_GURNEY_MUT_SWING1 = 658;
        SPR_GURNEY_MUT_SWING2 = 659;
        SPR_GURNEY_MUT_SWING3 = 660;
        SPR_GURNEY_MUT_DEAD = 661;
        SPR_GURNEY_MUT_DIE1 = 662;
        SPR_GURNEY_MUT_DIE2 = 663;
        SPR_GURNEY_MUT_DIE3 = 664;
        SPR_GURNEY_MUT_DIE4 = 665;
        SPR_GURNEY_MUT_OUCH = 666;

        SPR_LIQUID_M1 = 667;
        SPR_LIQUID_M2 = 668;
        SPR_LIQUID_M3 = 669;
        SPR_LIQUID_R1 = 670;
        SPR_LIQUID_R2 = 671;
        SPR_LIQUID_R3 = 672;
        SPR_LIQUID_R4 = 673;
        SPR_LIQUID_S1 = 674;
        SPR_LIQUID_S2 = 675;
        SPR_LIQUID_S3 = 676;
        SPR_LIQUID_OUCH = 677;
        SPR_LIQUID_DIE_1 = 678;
        SPR_LIQUID_DIE_2 = 679;
        SPR_LIQUID_DIE_3 = 680;
        SPR_LIQUID_DIE_4 = 681;
        SPR_LIQUID_DEAD = 682;
        SPR_LIQUID_SHOT_FLY_1 = 683;
        SPR_LIQUID_SHOT_FLY_2 = 684;
        SPR_LIQUID_SHOT_FLY_3 = 685;
        SPR_LIQUID_SHOT_BURST_1 = 686;
        SPR_LIQUID_SHOT_BURST_2 = 687;
        SPR_LIQUID_SHOT_BURST_3 = 688;

        SPR_SPIT1_1 = 689;
        SPR_SPIT1_2 = 690;
        SPR_SPIT1_3 = 691;
        SPR_SPIT_EXP1_1 = 692;
        SPR_SPIT_EXP1_2 = 693;
        SPR_SPIT_EXP1_3 = 694;

        SPR_SPIT2_1 = 695;
        SPR_SPIT2_2 = 696;
        SPR_SPIT2_3 = 697;
        SPR_SPIT_EXP2_1 = 698;
        SPR_SPIT_EXP2_2 = 699;
        SPR_SPIT_EXP2_3 = 700;

        SPR_SPIT3_1 = 701;
        SPR_SPIT3_2 = 702;
        SPR_SPIT3_3 = 703;
        SPR_SPIT_EXP3_1 = 704;
        SPR_SPIT_EXP3_2 = 705;
        SPR_SPIT_EXP3_3 = 706;

        SPR_TERROT_1 = 707;
        SPR_TERROT_2 = 708;
        SPR_TERROT_3 = 709;
        SPR_TERROT_4 = 710;
        SPR_TERROT_5 = 711;
        SPR_TERROT_6 = 712;
        SPR_TERROT_7 = 713;
        SPR_TERROT_8 = 714;
        SPR_TERROT_FIRE_1 = 715;
        SPR_TERROT_FIRE_2 = 716;
        SPR_TERROT_DIE_1 = 717;
        SPR_TERROT_DIE_2 = 718;
        SPR_TERROT_DIE_3 = 719;
        SPR_TERROT_DIE_4 = 720;
        SPR_TERROT_DEAD = 721;

        SPR_KNIFEREADY = 722;
        SPR_KNIFEATK1 = 723;
        SPR_KNIFEATK2 = 724;
        SPR_KNIFEATK3 = 725;
        SPR_KNIFEATK4 = 726;

        SPR_PISTOLREADY = 727;
        SPR_PISTOLATK1 = 728;
        SPR_PISTOLATK2 = 729;
        SPR_PISTOLATK3 = 730;
        SPR_PISTOLATK4 = 731;

        SPR_MACHINEGUNREADY = 732;
        SPR_MACHINEGUNATK1 = 733;
        SPR_MACHINEGUNATK2 = 734;
        SPR_MACHINEGUNATK3 = 735;
        SPR_MACHINEGUNATK4 = 736;

        SPR_CHAINREADY = 737;
        SPR_CHAINATK1 = 738;
        SPR_CHAINATK2 = 739;
        SPR_CHAINATK3 = 740;
        SPR_CHAINATK4 = 741;

        SPR_GRENADEREADY = 742;
        SPR_GRENADEATK1 = 743;
        SPR_GRENADEATK2 = 744;
        SPR_GRENADEATK3 = 745;
        SPR_GRENADEATK4 = 746;
        SPR_GRENADE_FLY1 = 747;
        SPR_GRENADE_FLY2 = 748;
        SPR_GRENADE_FLY3 = 749;
        SPR_GRENADE_FLY4 = 750;
        SPR_GRENADE_EXPLODE1 = 751;
        SPR_GRENADE_EXPLODE2 = 752;
        SPR_GRENADE_EXPLODE3 = 753;
        SPR_GRENADE_EXPLODE4 = 754;
        SPR_GRENADE_EXPLODE5 = 755;

        SPR_ELEC_ARC1 = 756;
        SPR_ELEC_ARC2 = 757;
        SPR_ELEC_ARC3 = 758;
        SPR_ELEC_ARC4 = 759;
        SPR_ELEC_POST1 = 760;
        SPR_ELEC_POST2 = 761;
        SPR_ELEC_POST3 = 762;
        SPR_ELEC_POST4 = 763;

        SPR_VPOST1 = 764;
        SPR_VPOST2 = 765;
        SPR_VPOST3 = 766;
        SPR_VPOST4 = 767;
        SPR_VPOST5 = 768;
        SPR_VPOST6 = 769;
        SPR_VPOST7 = 770;
        SPR_VPOST8 = 771;

        SPR_VSPIKE1 = 772;
        SPR_VSPIKE2 = 773;
        SPR_VSPIKE3 = 774;
        SPR_VSPIKE4 = 775;
        SPR_VSPIKE5 = 776;
        SPR_VSPIKE6 = 777;
        SPR_VSPIKE7 = 778;
        SPR_VSPIKE8 = 779;

        SPR_GREEN_OOZE1 = 780;
        SPR_GREEN_OOZE2 = 781;
        SPR_GREEN_OOZE3 = 782;

        SPR_BLACK_OOZE1 = 783;
        SPR_BLACK_OOZE2 = 784;
        SPR_BLACK_OOZE3 = 785;

        SPR_GREEN2_OOZE1 = 786;
        SPR_GREEN2_OOZE2 = 787;
        SPR_GREEN2_OOZE3 = 788;

        SPR_BLACK2_OOZE1 = 789;
        SPR_BLACK2_OOZE2 = 790;
        SPR_BLACK2_OOZE3 = 791;

        SPR_CANDY_BAR = 792;
        SPR_CANDY_WRAPER = 793;

        SPR_SANDWICH = 794;
        SPR_SANDWICH_WRAPER = 795;

        SPR_BLAKE_W1 = 796;
        SPR_BLAKE_W2 = 797;
        SPR_BLAKE_W3 = 798;
        SPR_BLAKE_W4 = 799;

        SPR_BOSS1_W1 = 800;
        SPR_BOSS1_W2 = 801;
        SPR_BOSS1_W3 = 802;
        SPR_BOSS1_W4 = 803;
        SPR_BOSS1_SWING1 = 804;
        SPR_BOSS1_SWING2 = 805;
        SPR_BOSS1_SWING3 = 806;
        SPR_BOSS1_DEAD = 807;
        SPR_BOSS1_DIE1 = 808;
        SPR_BOSS1_DIE2 = 809;
        SPR_BOSS1_DIE3 = 810;
        SPR_BOSS1_DIE4 = 811;
        SPR_BOSS1_OUCH = 812;
        SPR_BOSS1_PROJ1 = 813;
        SPR_BOSS1_PROJ2 = 814;
        SPR_BOSS1_PROJ3 = 815;
        SPR_BOSS1_EXP1 = 816;
        SPR_BOSS1_EXP2 = 817;
        SPR_BOSS1_EXP3 = 818;
        SPR_BOSS1_MORPH1 = 819;
        SPR_BOSS1_MORPH2 = 820;
        SPR_BOSS1_MORPH3 = 821;
        SPR_BOSS1_MORPH4 = 822;
        SPR_BOSS1_MORPH5 = 823;
        SPR_BOSS1_MORPH6 = 824;
        SPR_BOSS1_MORPH7 = 825;
        SPR_BOSS1_MORPH8 = 826;
        SPR_BOSS1_MORPH9 = 827;

        SPR_BOSS2_W1 = 828;
        SPR_BOSS2_W2 = 829;
        SPR_BOSS2_W3 = 830;
        SPR_BOSS2_W4 = 831;
        SPR_BOSS2_SWING1 = 832;
        SPR_BOSS2_SWING2 = 833;
        SPR_BOSS2_SWING3 = 834;
        SPR_BOSS2_DEAD = 835;
        SPR_BOSS2_DIE1 = 836;
        SPR_BOSS2_DIE2 = 837;
        SPR_BOSS2_DIE3 = 838;
        SPR_BOSS2_DIE4 = 839;
        SPR_BOSS2_OUCH = 840;

        SPR_BOSS3_W1 = 841;
        SPR_BOSS3_W2 = 842;
        SPR_BOSS3_W3 = 843;
        SPR_BOSS3_W4 = 844;
        SPR_BOSS3_SWING1 = 845;
        SPR_BOSS3_SWING2 = 846;
        SPR_BOSS3_SWING3 = 847;
        SPR_BOSS3_DEAD = 848;
        SPR_BOSS3_DIE1 = 849;
        SPR_BOSS3_DIE2 = 850;
        SPR_BOSS3_DIE3 = 851;
        SPR_BOSS3_DIE4 = 852;
        SPR_BOSS3_OUCH = 853;

        SPR_BOSS4_W1 = 854;
        SPR_BOSS4_W2 = 855;
        SPR_BOSS4_W3 = 856;
        SPR_BOSS4_W4 = 857;
        SPR_BOSS4_SWING1 = 858;
        SPR_BOSS4_SWING2 = 859;
        SPR_BOSS4_SWING3 = 860;
        SPR_BOSS4_DEAD = 861;
        SPR_BOSS4_DIE1 = 862;
        SPR_BOSS4_DIE2 = 863;
        SPR_BOSS4_DIE3 = 864;
        SPR_BOSS4_DIE4 = 865;
        SPR_BOSS4_OUCH = 866;
        SPR_BOSS4_MORPH1 = 867;
        SPR_BOSS4_MORPH2 = 868;
        SPR_BOSS4_MORPH3 = 869;
        SPR_BOSS4_MORPH4 = 870;
        SPR_BOSS4_MORPH5 = 871;
        SPR_BOSS4_MORPH6 = 872;
        SPR_BOSS4_MORPH7 = 873;
        SPR_BOSS4_MORPH8 = 874;
        SPR_BOSS4_MORPH9 = 875;

        SPR_BOSS5_W1 = 876;
        SPR_BOSS5_W2 = 877;
        SPR_BOSS5_W3 = 878;
        SPR_BOSS5_W4 = 879;
        SPR_BOSS5_SWING1 = 880;
        SPR_BOSS5_SWING2 = 881;
        SPR_BOSS5_SWING3 = 882;
        SPR_BOSS5_DEAD = 883;
        SPR_BOSS5_DIE1 = 884;
        SPR_BOSS5_DIE2 = 885;
        SPR_BOSS5_DIE3 = 886;
        SPR_BOSS5_DIE4 = 887;
        SPR_BOSS5_OUCH = 888;
        SPR_BOSS5_PROJ1 = 889;
        SPR_BOSS5_PROJ2 = 890;
        SPR_BOSS5_PROJ3 = 891;
        SPR_BOSS5_EXP1 = 892;
        SPR_BOSS5_EXP2 = 893;
        SPR_BOSS5_EXP3 = 894;

        SPR_BOSS6_W1 = 895;
        SPR_BOSS6_W2 = 896;
        SPR_BOSS6_W3 = 897;
        SPR_BOSS6_W4 = 898;
        SPR_BOSS6_SWING1 = 899;
        SPR_BOSS6_SWING2 = 900;
        SPR_BOSS6_SWING3 = 901;
        SPR_BOSS6_DEAD = 902;
        SPR_BOSS6_DIE1 = 903;
        SPR_BOSS6_DIE2 = 904;
        SPR_BOSS6_DIE3 = 905;
        SPR_BOSS6_DIE4 = 906;
        SPR_BOSS6_OUCH = 907;

        SPR_BOSS7_W1 = 908;
        SPR_BOSS7_W2 = 909;
        SPR_BOSS7_W3 = 910;
        SPR_BOSS7_W4 = 911;
        SPR_BOSS7_SHOOT1 = 912;
        SPR_BOSS7_SHOOT2 = 913;
        SPR_BOSS7_SHOOT3 = 914;
        SPR_BOSS7_DEAD = 915;
        SPR_BOSS7_DIE1 = 916;
        SPR_BOSS7_DIE2 = 917;
        SPR_BOSS7_DIE3 = 918;
        SPR_BOSS7_DIE4 = 919;
        SPR_BOSS7_OUCH = 920;

        SPR_BOSS8_W1 = 921;
        SPR_BOSS8_W2 = 922;
        SPR_BOSS8_W3 = 923;
        SPR_BOSS8_W4 = 924;
        SPR_BOSS8_SHOOT1 = 925;
        SPR_BOSS8_SHOOT2 = 926;
        SPR_BOSS8_SHOOT3 = 927;
        SPR_BOSS8_DIE1 = 928;
        SPR_BOSS8_DIE2 = 929;
        SPR_BOSS8_DIE3 = 930;
        SPR_BOSS8_DIE4 = 931;
        SPR_BOSS8_DEAD = 932;
        SPR_BOSS8_OUCH = 933;

        SPR_BOSS9_W1 = 934;
        SPR_BOSS9_W2 = 935;
        SPR_BOSS9_W3 = 936;
        SPR_BOSS9_W4 = 937;
        SPR_BOSS9_SHOOT1 = 938;
        SPR_BOSS9_SHOOT2 = 939;
        SPR_BOSS9_SHOOT3 = 940;
        SPR_BOSS9_DIE1 = 941;
        SPR_BOSS9_DIE2 = 942;
        SPR_BOSS9_DIE3 = 943;
        SPR_BOSS9_DIE4 = 944;
        SPR_BOSS9_DEAD = 945;
        SPR_BOSS9_OUCH = 946;

        SPR_BOSS10_W1 = 947;
        SPR_BOSS10_W2 = 948;
        SPR_BOSS10_W3 = 949;
        SPR_BOSS10_W4 = 950;
        SPR_BOSS10_SHOOT1 = 951;
        SPR_BOSS10_SHOOT2 = 952;
        SPR_BOSS10_SHOOT3 = 953;
        SPR_BOSS10_DEAD = 954;
        SPR_BOSS10_DIE1 = 955;
        SPR_BOSS10_DIE2 = 956;
        SPR_BOSS10_DIE3 = 957;
        SPR_BOSS10_DIE4 = 958;
        SPR_BOSS10_OUCH = 959;
        SPR_BOSS10_SPIT1 = 960;
        SPR_BOSS10_SPIT2 = 961;
        SPR_BOSS10_SPIT3 = 962;
        SPR_BOSS10_SPIT_EXP1 = 963;
        SPR_BOSS10_SPIT_EXP2 = 964;
        SPR_BOSS10_SPIT_EXP3 = 965;

        SPR_DETONATOR_EXP1 = 966;
        SPR_DETONATOR_EXP2 = 967;
        SPR_DETONATOR_EXP3 = 968;
        SPR_DETONATOR_EXP4 = 969;
        SPR_DETONATOR_EXP5 = 970;
        SPR_DETONATOR_EXP6 = 971;
        SPR_DETONATOR_EXP7 = 972;
        SPR_DETONATOR_EXP8 = 973;

        SPR_CLIP_EXP1 = 974;
        SPR_CLIP_EXP2 = 975;
        SPR_CLIP_EXP3 = 976;
        SPR_CLIP_EXP4 = 977;
        SPR_CLIP_EXP5 = 978;
        SPR_CLIP_EXP6 = 979;
        SPR_CLIP_EXP7 = 980;
        SPR_CLIP_EXP8 = 981;

        SPR_BFG_WEAPON1 = 982;
        SPR_BFG_WEAPON2 = 983;
        SPR_BFG_WEAPON3 = 984;
        SPR_BFG_WEAPON4 = 985;
        SPR_BFG_WEAPON5 = 986;
        SPR_BFG_WEAPON_SHOT1 = 987;
        SPR_BFG_WEAPON_SHOT2 = 988;
        SPR_BFG_WEAPON_SHOT3 = 989;

        SPR_BFG_EXP1 = 990;
        SPR_BFG_EXP2 = 991;
        SPR_BFG_EXP3 = 992;
        SPR_BFG_EXP4 = 993;
        SPR_BFG_EXP5 = 994;
        SPR_BFG_EXP6 = 995;
        SPR_BFG_EXP7 = 996;
        SPR_BFG_EXP8 = 997;
    }
}


// Sprites
// ==========================================================================


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

#if 0

unsigned mspeed;

void CalcSpeedRating()
{
    int16_t loop;

    for (loop = 0; loop < 10; loop++) {
        ThreeDRefresh();
        mspeed += tics;
    }
}

#endif


// BBi
extern statetype s_ofs_stand;
extern statetype s_ofs_chase1;
extern statetype s_ofs_chase1s;
extern statetype s_ofs_chase2;
extern statetype s_ofs_chase3;
extern statetype s_ofs_chase3s;
extern statetype s_ofs_chase4;
extern statetype s_ofs_pain;
extern statetype s_ofs_die1;
extern statetype s_ofs_die1s;
extern statetype s_ofs_die2;
extern statetype s_ofs_die3;
extern statetype s_ofs_die4;
extern statetype s_ofs_die5;
extern statetype s_ofs_attack1;
extern statetype s_ofs_attack2;
extern statetype s_ofs_attack3;
extern statetype s_ofs_spit1;
extern statetype s_ofs_spit2;
extern statetype s_ofs_spit3;
extern statetype s_ofs_shoot1;
extern statetype s_ofs_shoot2;
extern statetype s_ofs_shoot3;
extern statetype s_ofs_pod_attack1;
extern statetype s_ofs_pod_attack1a;
extern statetype s_ofs_pod_attack2;
extern statetype s_ofs_pod_spit1;
extern statetype s_ofs_pod_spit2;
extern statetype s_ofs_pod_spit3;
extern statetype s_ofs_pod_death1;
extern statetype s_ofs_pod_death2;
extern statetype s_ofs_pod_death3;
extern statetype s_ofs_pod_ouch;
extern statetype s_ofs_bounce;
extern statetype s_ofs_ouch;
extern statetype s_ofs_esphere_death1;
extern statetype s_ofs_esphere_death2;
extern statetype s_ofs_esphere_death3;
extern statetype s_ofs_random;
extern statetype s_ofs_static;
extern statetype s_hold;
extern statetype s_ofs_smart_anim;
extern statetype s_ofs_smart_anim2;
extern statetype s_barrier_transition;
extern statetype s_vpost_barrier;
extern statetype s_spike_barrier;
extern statetype s_barrier_shutdown;
extern statetype s_rent_stand;
extern statetype s_rent_path1;
extern statetype s_rent_path1s;
extern statetype s_rent_path2;
extern statetype s_rent_path3;
extern statetype s_rent_path3s;
extern statetype s_rent_path4;
extern statetype s_rent_pain;
extern statetype s_rent_shoot1;
extern statetype s_rent_shoot2;
extern statetype s_rent_shoot3;
extern statetype s_rent_chase1;
extern statetype s_rent_chase1s;
extern statetype s_rent_chase2;
extern statetype s_rent_chase3;
extern statetype s_rent_chase3s;
extern statetype s_rent_chase4;
extern statetype s_rent_die1;
extern statetype s_rent_die2;
extern statetype s_rent_die3;
extern statetype s_rent_die3s;
extern statetype s_rent_die4;
extern statetype s_ofcstand;
extern statetype s_ofcpath1;
extern statetype s_ofcpath1s;
extern statetype s_ofcpath2;
extern statetype s_ofcpath3;
extern statetype s_ofcpath3s;
extern statetype s_ofcpath4;
extern statetype s_ofcpain;
extern statetype s_ofcshoot1;
extern statetype s_ofcshoot2;
extern statetype s_ofcshoot3;
extern statetype s_ofcchase1;
extern statetype s_ofcchase1s;
extern statetype s_ofcchase2;
extern statetype s_ofcchase3;
extern statetype s_ofcchase3s;
extern statetype s_ofcchase4;
extern statetype s_ofcdie1;
extern statetype s_ofcdie2;
extern statetype s_ofcdie3;
extern statetype s_ofcdie4;
extern statetype s_ofcdie5;
extern statetype s_swatstand;
extern statetype s_swatpath1;
extern statetype s_swatpath1s;
extern statetype s_swatpath2;
extern statetype s_swatpath3;
extern statetype s_swatpath3s;
extern statetype s_swatpath4;
extern statetype s_swatpain;
extern statetype s_swatshoot1;
extern statetype s_swatshoot2;
extern statetype s_swatshoot3;
extern statetype s_swatshoot4;
extern statetype s_swatshoot5;
extern statetype s_swatshoot6;
extern statetype s_swatshoot7;
extern statetype s_swatchase1;
extern statetype s_swatchase1s;
extern statetype s_swatchase2;
extern statetype s_swatchase3;
extern statetype s_swatchase3s;
extern statetype s_swatchase4;
extern statetype s_swatwounded1;
extern statetype s_swatwounded2;
extern statetype s_swatwounded3;
extern statetype s_swatwounded4;
extern statetype s_swatunwounded1;
extern statetype s_swatunwounded2;
extern statetype s_swatunwounded3;
extern statetype s_swatunwounded4;
extern statetype s_swatdie1;
extern statetype s_swatdie2;
extern statetype s_swatdie3;
extern statetype s_swatdie4;
extern statetype s_swatdie5;
extern statetype s_prostand;
extern statetype s_propath1;
extern statetype s_propath1s;
extern statetype s_propath2;
extern statetype s_propath3;
extern statetype s_propath3s;
extern statetype s_propath4;
extern statetype s_propain;
extern statetype s_proshoot1;
extern statetype s_proshoot2;
extern statetype s_proshoot3;
extern statetype s_proshoot4;
extern statetype s_proshoot5;
extern statetype s_proshoot6;
extern statetype s_proshoot6a;
extern statetype s_prochase1;
extern statetype s_prochase1s;
extern statetype s_prochase2;
extern statetype s_prochase3;
extern statetype s_prochase3s;
extern statetype s_prochase4;
extern statetype s_prodie1;
extern statetype s_prodie2;
extern statetype s_prodie3;
extern statetype s_prodie3a;
extern statetype s_prodie4;
extern statetype s_electro_appear1;
extern statetype s_electro_appear2;
extern statetype s_electro_appear3;
extern statetype s_electro_chase1;
extern statetype s_electro_chase2;
extern statetype s_electro_chase3;
extern statetype s_electro_chase4;
extern statetype s_electro_ouch;
extern statetype s_electro_shoot1;
extern statetype s_electro_shoot2;
extern statetype s_electro_shoot3;
extern statetype s_electro_shot1;
extern statetype s_electro_shot2;
extern statetype s_ofs_shot1;
extern statetype s_ofs_shot2;
extern statetype s_electro_die1;
extern statetype s_electro_die2;
extern statetype s_electro_die3;
extern statetype s_liquid_wait;
extern statetype s_liquid_move;
extern statetype s_liquid_rise1;
extern statetype s_liquid_rise2;
extern statetype s_liquid_rise3;
extern statetype s_liquid_stand;
extern statetype s_liquid_fall1;
extern statetype s_liquid_fall2;
extern statetype s_liquid_fall3;
extern statetype s_liquid_shoot1;
extern statetype s_liquid_shoot2;
extern statetype s_liquid_shoot3;
extern statetype s_liquid_ouch;
extern statetype s_liquid_die1;
extern statetype s_liquid_die2;
extern statetype s_liquid_die3;
extern statetype s_liquid_die4;
extern statetype s_liquid_dead;
extern statetype s_liquid_shot;
extern statetype s_blake1;
extern statetype s_blake2;
extern statetype s_blake3;
extern statetype s_blake4;
extern statetype s_goldstand;
extern statetype s_goldpath1;
extern statetype s_goldpath1s;
extern statetype s_goldpath2;
extern statetype s_goldpath3;
extern statetype s_goldpath3s;
extern statetype s_goldpath4;
extern statetype s_goldpain;
extern statetype s_goldshoot1;
extern statetype s_goldshoot2;
extern statetype s_goldshoot3;
extern statetype s_goldshoot4;
extern statetype s_goldshoot5;
extern statetype s_goldshoot6;
extern statetype s_goldshoot7;
extern statetype s_goldchase1;
extern statetype s_goldchase1s;
extern statetype s_goldchase2;
extern statetype s_goldchase3;
extern statetype s_goldchase3s;
extern statetype s_goldchase4;
extern statetype s_goldwarp_it;
extern statetype s_goldwarp_it1;
extern statetype s_goldwarp_it2;
extern statetype s_goldwarp_it3;
extern statetype s_goldwarp_it4;
extern statetype s_goldwarp_it5;
extern statetype s_goldwarp_out1;
extern statetype s_goldwarp_out2;
extern statetype s_goldwarp_out3;
extern statetype s_goldwarp_out4;
extern statetype s_goldwarp_out5;
extern statetype s_goldwarp_in1;
extern statetype s_goldwarp_in2;
extern statetype s_goldwarp_in3;
extern statetype s_goldwarp_in4;
extern statetype s_goldwarp_in5;
extern statetype s_goldmorphwait1;
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
extern statetype s_scout_stand;
extern statetype s_scout_path1;
extern statetype s_scout_path2;
extern statetype s_scout_path3;
extern statetype s_scout_path4;
extern statetype s_scout_run;
extern statetype s_scout_run2;
extern statetype s_scout_run3;
extern statetype s_scout_run4;
extern statetype s_scout_dead;
extern statetype s_steamgrate;
extern statetype s_steamrelease1;
extern statetype s_steamrelease2;
extern statetype s_steamrelease3;
extern statetype s_steamrelease4;
extern statetype s_steamrelease5;
extern statetype s_steamrelease6;
extern statetype s_terrot_wait;
extern statetype s_terrot_found;
extern statetype s_terrot_shoot1;
extern statetype s_terrot_shoot2;
extern statetype s_terrot_shoot3;
extern statetype s_terrot_shoot4;
extern statetype s_terrot_seek1;
extern statetype s_terrot_seek1s;
extern statetype s_terrot_die1;
extern statetype s_terrot_die2;
extern statetype s_terrot_die3;
extern statetype s_terrot_die4;
extern statetype s_terrot_die5;
extern statetype s_player;
extern statetype s_attack;


statetype* states_list[] = {
    nullptr,
    &s_ofs_stand,
    &s_ofs_chase1,
    &s_ofs_chase1s,
    &s_ofs_chase2,
    &s_ofs_chase3,
    &s_ofs_chase3s,
    &s_ofs_chase4,
    &s_ofs_pain,
    &s_ofs_die1,
    &s_ofs_die1s,
    &s_ofs_die2,
    &s_ofs_die3,
    &s_ofs_die4,
    &s_ofs_die5,
    &s_ofs_attack1,
    &s_ofs_attack2,
    &s_ofs_attack3,
    &s_ofs_spit1,
    &s_ofs_spit2,
    &s_ofs_spit3,
    &s_ofs_shoot1,
    &s_ofs_shoot2,
    &s_ofs_shoot3,
    &s_ofs_pod_attack1,
    &s_ofs_pod_attack1a,
    &s_ofs_pod_attack2,
    &s_ofs_pod_spit1,
    &s_ofs_pod_spit2,
    &s_ofs_pod_spit3,
    &s_ofs_pod_death1,
    &s_ofs_pod_death2,
    &s_ofs_pod_death3,
    &s_ofs_pod_ouch,
    &s_ofs_bounce,
    &s_ofs_ouch,
    &s_ofs_esphere_death1,
    &s_ofs_esphere_death2,
    &s_ofs_esphere_death3,
    &s_ofs_random,
    &s_ofs_static,
    &s_hold,
    &s_ofs_smart_anim,
    &s_ofs_smart_anim2,
    &s_barrier_transition,
    &s_vpost_barrier,
    &s_spike_barrier,
    &s_barrier_shutdown,
    &s_rent_stand,
    &s_rent_path1,
    &s_rent_path1s,
    &s_rent_path2,
    &s_rent_path3,
    &s_rent_path3s,
    &s_rent_path4,
    &s_rent_pain,
    &s_rent_shoot1,
    &s_rent_shoot2,
    &s_rent_shoot3,
    &s_rent_chase1,
    &s_rent_chase1s,
    &s_rent_chase2,
    &s_rent_chase3,
    &s_rent_chase3s,
    &s_rent_chase4,
    &s_rent_die1,
    &s_rent_die2,
    &s_rent_die3,
    &s_rent_die3s,
    &s_rent_die4,
    &s_ofcstand,
    &s_ofcpath1,
    &s_ofcpath1s,
    &s_ofcpath2,
    &s_ofcpath3,
    &s_ofcpath3s,
    &s_ofcpath4,
    &s_ofcpain,
    &s_ofcshoot1,
    &s_ofcshoot2,
    &s_ofcshoot3,
    &s_ofcchase1,
    &s_ofcchase1s,
    &s_ofcchase2,
    &s_ofcchase3,
    &s_ofcchase3s,
    &s_ofcchase4,
    &s_ofcdie1,
    &s_ofcdie2,
    &s_ofcdie3,
    &s_ofcdie4,
    &s_ofcdie5,
    &s_swatstand,
    &s_swatpath1,
    &s_swatpath1s,
    &s_swatpath2,
    &s_swatpath3,
    &s_swatpath3s,
    &s_swatpath4,
    &s_swatpain,
    &s_swatshoot1,
    &s_swatshoot2,
    &s_swatshoot3,
    &s_swatshoot4,
    &s_swatshoot5,
    &s_swatshoot6,
    &s_swatshoot7,
    &s_swatchase1,
    &s_swatchase1s,
    &s_swatchase2,
    &s_swatchase3,
    &s_swatchase3s,
    &s_swatchase4,
    &s_swatwounded1,
    &s_swatwounded2,
    &s_swatwounded3,
    &s_swatwounded4,
    &s_swatunwounded1,
    &s_swatunwounded2,
    &s_swatunwounded3,
    &s_swatunwounded4,
    &s_swatdie1,
    &s_swatdie2,
    &s_swatdie3,
    &s_swatdie4,
    &s_swatdie5,
    &s_prostand,
    &s_propath1,
    &s_propath1s,
    &s_propath2,
    &s_propath3,
    &s_propath3s,
    &s_propath4,
    &s_propain,
    &s_proshoot1,
    &s_proshoot2,
    &s_proshoot3,
    &s_proshoot4,
    &s_proshoot5,
    &s_proshoot6,
    &s_proshoot6a,
    &s_prochase1,
    &s_prochase1s,
    &s_prochase2,
    &s_prochase3,
    &s_prochase3s,
    &s_prochase4,
    &s_prodie1,
    &s_prodie2,
    &s_prodie3,
    &s_prodie3a,
    &s_prodie4,
    &s_electro_appear1,
    &s_electro_appear2,
    &s_electro_appear3,
    &s_electro_chase1,
    &s_electro_chase2,
    &s_electro_chase3,
    &s_electro_chase4,
    &s_electro_ouch,
    &s_electro_shoot1,
    &s_electro_shoot2,
    &s_electro_shoot3,
    &s_electro_shot1,
    &s_electro_shot2,
    &s_ofs_shot1,
    &s_ofs_shot2,
    &s_electro_die1,
    &s_electro_die2,
    &s_electro_die3,
    &s_liquid_wait,
    &s_liquid_move,
    &s_liquid_rise1,
    &s_liquid_rise2,
    &s_liquid_rise3,
    &s_liquid_stand,
    &s_liquid_fall1,
    &s_liquid_fall2,
    &s_liquid_fall3,
    &s_liquid_shoot1,
    &s_liquid_shoot2,
    &s_liquid_shoot3,
    &s_liquid_ouch,
    &s_liquid_die1,
    &s_liquid_die2,
    &s_liquid_die3,
    &s_liquid_die4,
    &s_liquid_dead,
    &s_liquid_shot,
    &s_blake1,
    &s_blake2,
    &s_blake3,
    &s_blake4,
    &s_goldstand,
    &s_goldpath1,
    &s_goldpath1s,
    &s_goldpath2,
    &s_goldpath3,
    &s_goldpath3s,
    &s_goldpath4,
    &s_goldpain,
    &s_goldshoot1,
    &s_goldshoot2,
    &s_goldshoot3,
    &s_goldshoot4,
    &s_goldshoot5,
    &s_goldshoot6,
    &s_goldshoot7,
    &s_goldchase1,
    &s_goldchase1s,
    &s_goldchase2,
    &s_goldchase3,
    &s_goldchase3s,
    &s_goldchase4,
    &s_goldwarp_it,
    &s_goldwarp_it1,
    &s_goldwarp_it2,
    &s_goldwarp_it3,
    &s_goldwarp_it4,
    &s_goldwarp_it5,
    &s_goldwarp_out1,
    &s_goldwarp_out2,
    &s_goldwarp_out3,
    &s_goldwarp_out4,
    &s_goldwarp_out5,
    &s_goldwarp_in1,
    &s_goldwarp_in2,
    &s_goldwarp_in3,
    &s_goldwarp_in4,
    &s_goldwarp_in5,
    &s_goldmorphwait1,
    &s_goldmorph1,
    &s_goldmorph2,
    &s_goldmorph3,
    &s_goldmorph4,
    &s_goldmorph5,
    &s_goldmorph6,
    &s_goldmorph7,
    &s_goldmorph8,
    &s_mgold_chase1,
    &s_mgold_chase2,
    &s_mgold_chase3,
    &s_mgold_chase4,
    &s_mgold_shoot1,
    &s_mgold_shoot2,
    &s_mgold_shoot3,
    &s_mgold_shoot4,
    &s_mgold_pain,
    &s_security_light,
    &s_scout_stand,
    &s_scout_path1,
    &s_scout_path2,
    &s_scout_path3,
    &s_scout_path4,
    &s_scout_run,
    &s_scout_run2,
    &s_scout_run3,
    &s_scout_run4,
    &s_scout_dead,
    &s_steamgrate,
    &s_steamrelease1,
    &s_steamrelease2,
    &s_steamrelease3,
    &s_steamrelease4,
    &s_steamrelease5,
    &s_steamrelease6,
    &s_terrot_wait,
    &s_terrot_found,
    &s_terrot_shoot1,
    &s_terrot_shoot2,
    &s_terrot_shoot3,
    &s_terrot_shoot4,
    &s_terrot_seek1,
    &s_terrot_seek1s,
    &s_terrot_die1,
    &s_terrot_die2,
    &s_terrot_die3,
    &s_terrot_die4,
    &s_terrot_die5,
    &s_player,
    &s_attack,
    nullptr,
};

static int get_state_index(
    statetype* state)
{
    if (!state) {
        return 0;
    }

    for (int i = 1; states_list[i]; ++i) {
        if (states_list[i] == state) {
            return i;
        }
    }

    return -1;
}

void initialize_states()
{
    s_ofs_stand.shapenum = SPR_GENETIC_W1 - SPR_GENETIC_W1;
    s_ofs_chase1.shapenum = SPR_GENETIC_W1 - SPR_GENETIC_W1;
    s_ofs_chase1s.shapenum = SPR_GENETIC_W1 - SPR_GENETIC_W1;
    s_ofs_chase2.shapenum = SPR_GENETIC_W2 - SPR_GENETIC_W1;
    s_ofs_chase3.shapenum = SPR_GENETIC_W3 - SPR_GENETIC_W1;
    s_ofs_chase3s.shapenum = SPR_GENETIC_W3 - SPR_GENETIC_W1;
    s_ofs_chase4.shapenum = SPR_GENETIC_W4 - SPR_GENETIC_W1;
    s_ofs_pain.shapenum = SPR_GENETIC_OUCH - SPR_GENETIC_W1;
    s_ofs_die1.shapenum = SPR_GENETIC_OUCH - SPR_GENETIC_W1;
    s_ofs_die1s.shapenum = SPR_GENETIC_DIE1 - SPR_GENETIC_W1;
    s_ofs_die2.shapenum = SPR_GENETIC_DIE2 - SPR_GENETIC_W1;
    s_ofs_die3.shapenum = SPR_GENETIC_DIE3 - SPR_GENETIC_W1;
    s_ofs_die4.shapenum = SPR_GENETIC_DIE4 - SPR_GENETIC_W1;
    s_ofs_die5.shapenum = SPR_GENETIC_DEAD - SPR_GENETIC_W1;
    s_ofs_attack1.shapenum = SPR_GENETIC_SWING1 - SPR_GENETIC_W1;
    s_ofs_attack2.shapenum = SPR_GENETIC_SWING2 - SPR_GENETIC_W1;
    s_ofs_attack3.shapenum = SPR_GENETIC_SWING3 - SPR_GENETIC_W1;
    s_ofs_spit1.shapenum = SPR_GENETIC_SHOOT1 - SPR_GENETIC_W1;
    s_ofs_spit2.shapenum = SPR_GENETIC_SHOOT2 - SPR_GENETIC_W1;
    s_ofs_spit3.shapenum = SPR_GENETIC_SHOOT3 - SPR_GENETIC_W1;
    s_ofs_shoot1.shapenum = SPR_GENETIC_SWING1 - SPR_GENETIC_W1;
    s_ofs_shoot2.shapenum = SPR_GENETIC_SWING2 - SPR_GENETIC_W1;
    s_ofs_shoot3.shapenum = SPR_GENETIC_SWING3 - SPR_GENETIC_W1;
    s_ofs_pod_attack1.shapenum = SPR_POD_ATTACK1 - SPR_POD_WALK1;
    s_ofs_pod_attack1a.shapenum = SPR_POD_ATTACK2 - SPR_POD_WALK1;
    s_ofs_pod_attack2.shapenum = SPR_POD_ATTACK3 - SPR_POD_WALK1;
    s_ofs_pod_spit1.shapenum = SPR_POD_SPIT1 - SPR_POD_WALK1;
    s_ofs_pod_spit2.shapenum = SPR_POD_SPIT2 - SPR_POD_WALK1;
    s_ofs_pod_spit3.shapenum = SPR_POD_SPIT3 - SPR_POD_WALK1;
    s_ofs_pod_ouch.shapenum = SPR_POD_OUCH - SPR_POD_WALK1;
    s_vpost_barrier.shapenum = SPR_VPOST1;
    s_spike_barrier.shapenum = SPR_VSPIKE1;
    s_rent_stand.shapenum = SPR_RENT_S_1;
    s_rent_path1.shapenum = SPR_RENT_W1_1;
    s_rent_path1s.shapenum = SPR_RENT_W2_1;
    s_rent_path2.shapenum = SPR_RENT_W2_1;
    s_rent_path3.shapenum = SPR_RENT_W3_1;
    s_rent_path3s.shapenum = SPR_RENT_W3_1;
    s_rent_path4.shapenum = SPR_RENT_W4_1;
    s_rent_pain.shapenum = SPR_RENT_PAIN_1;
    s_rent_shoot1.shapenum = SPR_RENT_SHOOT1;
    s_rent_shoot2.shapenum = SPR_RENT_SHOOT2;
    s_rent_shoot3.shapenum = SPR_RENT_SHOOT3;
    s_rent_chase1.shapenum = SPR_RENT_W1_1;
    s_rent_chase1s.shapenum = SPR_RENT_W1_1;
    s_rent_chase2.shapenum = SPR_RENT_W2_1;
    s_rent_chase3.shapenum = SPR_RENT_W3_1;
    s_rent_chase3s.shapenum = SPR_RENT_W3_1;
    s_rent_chase4.shapenum = SPR_RENT_W4_1;
    s_rent_die1.shapenum = SPR_RENT_DIE_1;
    s_rent_die2.shapenum = SPR_RENT_DIE_2;
    s_rent_die3.shapenum = SPR_RENT_DIE_3;
    s_rent_die3s.shapenum = SPR_RENT_DIE_4;
    s_rent_die4.shapenum = SPR_RENT_DEAD;
    s_ofcstand.shapenum = SPR_OFC_S_1;
    s_ofcpath1.shapenum = SPR_OFC_W1_1;
    s_ofcpath1s.shapenum = SPR_OFC_W1_1;
    s_ofcpath2.shapenum = SPR_OFC_W2_1;
    s_ofcpath3.shapenum = SPR_OFC_W3_1;
    s_ofcpath3s.shapenum = SPR_OFC_W3_1;
    s_ofcpath4.shapenum = SPR_OFC_W4_1;
    s_ofcpain.shapenum = SPR_OFC_PAIN_1;
    s_ofcshoot1.shapenum = SPR_OFC_SHOOT1;
    s_ofcshoot2.shapenum = SPR_OFC_SHOOT2;
    s_ofcshoot3.shapenum = SPR_OFC_SHOOT3;
    s_ofcchase1.shapenum = SPR_OFC_W1_1;
    s_ofcchase1s.shapenum = SPR_OFC_W1_1;
    s_ofcchase2.shapenum = SPR_OFC_W2_1;
    s_ofcchase3.shapenum = SPR_OFC_W3_1;
    s_ofcchase3s.shapenum = SPR_OFC_W3_1;
    s_ofcchase4.shapenum = SPR_OFC_W4_1;
    s_ofcdie1.shapenum = SPR_OFC_DIE_1;
    s_ofcdie2.shapenum = SPR_OFC_DIE_2;
    s_ofcdie3.shapenum = SPR_OFC_DIE_3;
    s_ofcdie4.shapenum = SPR_OFC_DIE_4;
    s_ofcdie5.shapenum = SPR_OFC_DEAD;
    s_swatstand.shapenum = SPR_SWAT_S_1;
    s_swatpath1.shapenum = SPR_SWAT_W1_1;
    s_swatpath1s.shapenum = SPR_SWAT_W1_1;
    s_swatpath2.shapenum = SPR_SWAT_W2_1;
    s_swatpath3.shapenum = SPR_SWAT_W3_1;
    s_swatpath3s.shapenum = SPR_SWAT_W3_1;
    s_swatpath4.shapenum = SPR_SWAT_W4_1;
    s_swatpain.shapenum = SPR_SWAT_PAIN_1;
    s_swatshoot1.shapenum = SPR_SWAT_SHOOT1;
    s_swatshoot2.shapenum = SPR_SWAT_SHOOT2;
    s_swatshoot3.shapenum = SPR_SWAT_SHOOT3;
    s_swatshoot4.shapenum = SPR_SWAT_SHOOT2;
    s_swatshoot5.shapenum = SPR_SWAT_SHOOT3;
    s_swatshoot6.shapenum = SPR_SWAT_SHOOT2;
    s_swatshoot7.shapenum = SPR_SWAT_SHOOT3;
    s_swatchase1.shapenum = SPR_SWAT_W1_1;
    s_swatchase1s.shapenum = SPR_SWAT_W1_1;
    s_swatchase2.shapenum = SPR_SWAT_W2_1;
    s_swatchase3.shapenum = SPR_SWAT_W3_1;
    s_swatchase3s.shapenum = SPR_SWAT_W3_1;
    s_swatchase4.shapenum = SPR_SWAT_W4_1;
    s_swatwounded1.shapenum = SPR_SWAT_WOUNDED1;
    s_swatwounded2.shapenum = SPR_SWAT_WOUNDED2;
    s_swatwounded3.shapenum = SPR_SWAT_WOUNDED3;
    s_swatwounded4.shapenum = SPR_SWAT_WOUNDED4;
    s_swatunwounded1.shapenum = SPR_SWAT_WOUNDED4;
    s_swatunwounded2.shapenum = SPR_SWAT_WOUNDED3;
    s_swatunwounded3.shapenum = SPR_SWAT_WOUNDED2;
    s_swatunwounded4.shapenum = SPR_SWAT_WOUNDED1;
    s_swatdie1.shapenum = SPR_SWAT_DIE_1;
    s_swatdie2.shapenum = SPR_SWAT_DIE_2;
    s_swatdie3.shapenum = SPR_SWAT_DIE_3;
    s_swatdie4.shapenum = SPR_SWAT_DIE_4;
    s_swatdie5.shapenum = SPR_SWAT_DEAD;
    s_prostand.shapenum = SPR_PRO_S_1;
    s_propath1.shapenum = SPR_PRO_W1_1;
    s_propath1s.shapenum = SPR_PRO_W1_1;
    s_propath2.shapenum = SPR_PRO_W2_1;
    s_propath3.shapenum = SPR_PRO_W3_1;
    s_propath3s.shapenum = SPR_PRO_W3_1;
    s_propath4.shapenum = SPR_PRO_W4_1;
    s_propain.shapenum = SPR_PRO_PAIN_1;
    s_proshoot1.shapenum = SPR_PRO_SHOOT1;
    s_proshoot2.shapenum = SPR_PRO_SHOOT2;
    s_proshoot3.shapenum = SPR_PRO_SHOOT3;
    s_proshoot4.shapenum = SPR_PRO_SHOOT2;
    s_proshoot5.shapenum = SPR_PRO_SHOOT3;
    s_proshoot6.shapenum = SPR_PRO_SHOOT2;
    s_proshoot6a.shapenum = SPR_PRO_SHOOT3;
    s_prochase1.shapenum = SPR_PRO_W1_1;
    s_prochase1s.shapenum = SPR_PRO_W1_1;
    s_prochase2.shapenum = SPR_PRO_W2_1;
    s_prochase3.shapenum = SPR_PRO_W3_1;
    s_prochase3s.shapenum = SPR_PRO_W3_1;
    s_prochase4.shapenum = SPR_PRO_W4_1;
    s_prodie1.shapenum = SPR_PRO_DIE_1;
    s_prodie2.shapenum = SPR_PRO_DIE_2;
    s_prodie3.shapenum = SPR_PRO_DIE_3;
    s_prodie3a.shapenum = SPR_PRO_DIE_4;
    s_prodie4.shapenum = SPR_PRO_DEAD;
    s_electro_appear1.shapenum = SPR_ELEC_APPEAR1;
    s_electro_appear2.shapenum = SPR_ELEC_APPEAR2;
    s_electro_appear3.shapenum = SPR_ELEC_APPEAR3;
    s_electro_chase1.shapenum = SPR_ELEC_WALK1;
    s_electro_chase2.shapenum = SPR_ELEC_WALK2;
    s_electro_chase3.shapenum = SPR_ELEC_WALK3;
    s_electro_chase4.shapenum = SPR_ELEC_WALK4;
    s_electro_ouch.shapenum = SPR_ELEC_OUCH;
    s_electro_shoot1.shapenum = SPR_ELEC_SHOOT1;
    s_electro_shoot2.shapenum = SPR_ELEC_SHOOT2;
    s_electro_shoot3.shapenum = SPR_ELEC_SHOOT3;
    s_electro_shot1.shapenum = SPR_ELEC_SHOT1;
    s_electro_shot2.shapenum = SPR_ELEC_SHOT2;
    s_electro_die1.shapenum = SPR_ELEC_DIE1;
    s_electro_die2.shapenum = SPR_ELEC_DIE2;
    s_electro_die3.shapenum = SPR_ELEC_DIE3;
    s_liquid_wait.shapenum = SPR_LIQUID_M1;
    s_liquid_move.shapenum = SPR_LIQUID_M1;
    s_liquid_rise1.shapenum = SPR_LIQUID_R1;
    s_liquid_rise2.shapenum = SPR_LIQUID_R2;
    s_liquid_rise3.shapenum = SPR_LIQUID_R3;
    s_liquid_stand.shapenum = SPR_LIQUID_R4;
    s_liquid_fall1.shapenum = SPR_LIQUID_R3;
    s_liquid_fall2.shapenum = SPR_LIQUID_R2;
    s_liquid_fall3.shapenum = SPR_LIQUID_R1;
    s_liquid_shoot1.shapenum = SPR_LIQUID_S1;
    s_liquid_shoot2.shapenum = SPR_LIQUID_S2;
    s_liquid_shoot3.shapenum = SPR_LIQUID_S3;
    s_liquid_ouch.shapenum = SPR_LIQUID_OUCH;
    s_liquid_die1.shapenum = SPR_LIQUID_DIE_1;
    s_liquid_die2.shapenum = SPR_LIQUID_DIE_2;
    s_liquid_die3.shapenum = SPR_LIQUID_DIE_3;
    s_liquid_die4.shapenum = SPR_LIQUID_DIE_4;
    s_liquid_dead.shapenum = SPR_LIQUID_DEAD;
    s_blake1.shapenum = SPR_BLAKE_W1;
    s_blake2.shapenum = SPR_BLAKE_W2;
    s_blake3.shapenum = SPR_BLAKE_W3;
    s_blake4.shapenum = SPR_BLAKE_W4;
    s_goldstand.shapenum = SPR_GOLD_S_1;
    s_goldpath1.shapenum = SPR_GOLD_W1_1;
    s_goldpath1s.shapenum = SPR_GOLD_W1_1;
    s_goldpath2.shapenum = SPR_GOLD_W2_1;
    s_goldpath3.shapenum = SPR_GOLD_W3_1;
    s_goldpath3s.shapenum = SPR_GOLD_W3_1;
    s_goldpath4.shapenum = SPR_GOLD_W4_1;
    s_goldpain.shapenum = SPR_GOLD_PAIN_1;
    s_goldshoot1.shapenum = SPR_GOLD_SHOOT1;
    s_goldshoot2.shapenum = SPR_GOLD_SHOOT2;
    s_goldshoot3.shapenum = SPR_GOLD_SHOOT3;
    s_goldshoot4.shapenum = SPR_GOLD_SHOOT2;
    s_goldshoot5.shapenum = SPR_GOLD_SHOOT3;
    s_goldshoot6.shapenum = SPR_GOLD_SHOOT2;
    s_goldshoot7.shapenum = SPR_GOLD_SHOOT3;
    s_goldchase1.shapenum = SPR_GOLD_W1_1;
    s_goldchase1s.shapenum = SPR_GOLD_W1_1;
    s_goldchase2.shapenum = SPR_GOLD_W2_1;
    s_goldchase3.shapenum = SPR_GOLD_W3_1;
    s_goldchase3s.shapenum = SPR_GOLD_W3_1;
    s_goldchase4.shapenum = SPR_GOLD_W4_1;
    s_goldwarp_it.shapenum = SPR_GOLD_S_1;
    s_goldwarp_it1.shapenum = SPR_GOLD_WRIST_1;
    s_goldwarp_it2.shapenum = SPR_GOLD_WRIST_2;
    s_goldwarp_it3.shapenum = SPR_GOLD_WRIST_1;
    s_goldwarp_it4.shapenum = SPR_GOLD_WRIST_2;
    s_goldwarp_it5.shapenum = SPR_GOLD_S_1;
    s_goldwarp_out1.shapenum = SPR_GOLD_WARP1;
    s_goldwarp_out2.shapenum = SPR_GOLD_WARP2;
    s_goldwarp_out3.shapenum = SPR_GOLD_WARP3;
    s_goldwarp_out4.shapenum = SPR_GOLD_WARP4;
    s_goldwarp_out5.shapenum = SPR_GOLD_WARP5;
    s_goldwarp_in1.shapenum = SPR_GOLD_WARP5;
    s_goldwarp_in2.shapenum = SPR_GOLD_WARP4;
    s_goldwarp_in3.shapenum = SPR_GOLD_WARP3;
    s_goldwarp_in4.shapenum = SPR_GOLD_WARP2;
    s_goldwarp_in5.shapenum = SPR_GOLD_WARP1;
    s_goldmorphwait1.shapenum = SPR_GOLD_WRIST_1;
    s_goldmorph1.shapenum = SPR_GOLD_MORPH1;
    s_goldmorph2.shapenum = SPR_GOLD_MORPH2;
    s_goldmorph3.shapenum = SPR_GOLD_MORPH3;
    s_goldmorph4.shapenum = SPR_GOLD_MORPH4;
    s_goldmorph5.shapenum = SPR_GOLD_MORPH5;
    s_goldmorph6.shapenum = SPR_GOLD_MORPH6;
    s_goldmorph7.shapenum = SPR_GOLD_MORPH7;
    s_goldmorph8.shapenum = SPR_GOLD_MORPH8;
    s_mgold_chase1.shapenum = SPR_MGOLD_WALK1;
    s_mgold_chase2.shapenum = SPR_MGOLD_WALK2;
    s_mgold_chase3.shapenum = SPR_MGOLD_WALK3;
    s_mgold_chase4.shapenum = SPR_MGOLD_WALK4;
    s_mgold_shoot1.shapenum = SPR_MGOLD_ATTACK1;
    s_mgold_shoot2.shapenum = SPR_MGOLD_ATTACK2;
    s_mgold_shoot3.shapenum = SPR_MGOLD_ATTACK3;
    s_mgold_shoot4.shapenum = SPR_MGOLD_ATTACK4;
    s_mgold_pain.shapenum = SPR_MGOLD_OUCH;
    s_security_light.shapenum = SPR_SECURITY_NORMAL;
    s_scout_stand.shapenum = SPR_GSCOUT_W1_1;
    s_scout_path1.shapenum = SPR_GSCOUT_W1_1;
    s_scout_path2.shapenum = SPR_GSCOUT_W2_1;
    s_scout_path3.shapenum = SPR_GSCOUT_W3_1;
    s_scout_path4.shapenum = SPR_GSCOUT_W4_1;
    s_scout_run.shapenum = SPR_GSCOUT_W1_1;
    s_scout_run2.shapenum = SPR_GSCOUT_W2_1;
    s_scout_run3.shapenum = SPR_GSCOUT_W3_1;
    s_scout_run4.shapenum = SPR_GSCOUT_W4_1;
    s_scout_dead.shapenum = SPR_GSCOUT_W1_1;
    s_terrot_wait.shapenum = SPR_TERROT_1;
    s_terrot_found.shapenum = SPR_TERROT_1;
    s_terrot_shoot1.shapenum = SPR_TERROT_FIRE_1;
    s_terrot_shoot2.shapenum = SPR_TERROT_FIRE_2;
    s_terrot_shoot3.shapenum = SPR_TERROT_FIRE_1;
    s_terrot_shoot4.shapenum = SPR_TERROT_FIRE_2;
    s_terrot_seek1.shapenum = SPR_TERROT_1;
    s_terrot_seek1s.shapenum = SPR_TERROT_1;
    s_terrot_die1.shapenum = SPR_TERROT_DIE_1;
    s_terrot_die2.shapenum = SPR_TERROT_DIE_2;
    s_terrot_die3.shapenum = SPR_TERROT_DIE_3;
    s_terrot_die4.shapenum = SPR_TERROT_DIE_4;
    s_terrot_die5.shapenum = SPR_TERROT_DEAD;
}


int16_t NUMSNDCHUNKS = 0;

int16_t S2100A_MUS = 0;
int16_t GOLDA_MUS = 0;
int16_t DRKHALLA_MUS = 0;
int16_t FREEDOMA_MUS = 0;
int16_t GENEFUNK_MUS = 0;
int16_t TIMEA_MUS = 0;
int16_t HIDINGA_MUS = 0;
int16_t INCNRATN_MUS = 0;
int16_t JUNGLEA_MUS = 0;
int16_t LEVELA_MUS = 0;
int16_t MEETINGA_MUS = 0;
int16_t STRUTA_MUS = 0;
int16_t RACSHUFL_MUS = 0;
int16_t RUMBAA_MUS = 0;
int16_t SEARCHNA_MUS = 0;
int16_t THEWAYA_MUS = 0;
int16_t INTRIGEA_MUS = 0;

int16_t CATACOMB_MUS = 0;
int16_t STICKS_MUS = 0;
int16_t PLOT_MUS = 0;
int16_t CIRCLES_MUS = 0;
int16_t LASTLAFF_MUS = 0;
int16_t TOHELL_MUS = 0;
int16_t FORTRESS_MUS = 0;
int16_t GIVING_MUS = 0;
int16_t HARTBEAT_MUS = 0;
int16_t LURKING_MUS = 0;
int16_t MAJMIN_MUS = 0;
int16_t VACCINAP_MUS = 0;
int16_t DARKNESS_MUS = 0;
int16_t MONASTRY_MUS = 0;
int16_t TOMBP_MUS = 0;
int16_t TIME_MUS = 0;
int16_t MOURNING_MUS = 0;
int16_t SERPENT_MUS = 0;
int16_t HISCORE_MUS = 0;

int16_t APOGFNFM_MUS = 0;
int16_t THEME_MUS = 0;
int16_t LASTMUSIC = 0;
int16_t TITLE_LOOP_MUSIC = 0;


void initialize_audio_constants()
{
    if (::is_aog()) {
        NUMSNDCHUNKS = 319;

        THEME_MUS = 16;
        LASTMUSIC = 19;
    } else {
        NUMSNDCHUNKS = 321;

        THEME_MUS = 20;
        LASTMUSIC = 21;
    }

    APOGFNFM_MUS = 2;

    S2100A_MUS = 0;
    GOLDA_MUS = 1;
    APOGFNFM_MUS = 2;
    DRKHALLA_MUS = 3;
    FREEDOMA_MUS = 4;
    GENEFUNK_MUS = 5;
    TIMEA_MUS = 6;
    HIDINGA_MUS = 7;
    INCNRATN_MUS = 8;
    JUNGLEA_MUS = 9;
    LEVELA_MUS = 10;
    MEETINGA_MUS = 11;
    STRUTA_MUS = 12;
    RACSHUFL_MUS = 13;
    RUMBAA_MUS = 14;
    SEARCHNA_MUS = 15;
    THEWAYA_MUS = 17;
    INTRIGEA_MUS = 18;

    CATACOMB_MUS = 0;
    STICKS_MUS = 1;
    PLOT_MUS = 3;
    CIRCLES_MUS = 4;
    LASTLAFF_MUS = 5;
    TOHELL_MUS = 6;
    FORTRESS_MUS = 7;
    GIVING_MUS = 8;
    HARTBEAT_MUS = 9;
    LURKING_MUS = 10;
    MAJMIN_MUS = 11;
    VACCINAP_MUS = 12;
    DARKNESS_MUS = 13;
    MONASTRY_MUS = 14;
    TOMBP_MUS = 15;
    TIME_MUS = 16;
    MOURNING_MUS = 17;
    SERPENT_MUS = 18;
    HISCORE_MUS = 19;

    audiosegs.resize(NUMSNDCHUNKS);

    if (::is_ps()) {
        MENUSONG = LASTLAFF_MUS;
        ROSTER_MUS = HISCORE_MUS;
        TEXTSONG = TOHELL_MUS;
        TITLE_LOOP_MUSIC = PLOT_MUS;
    } else {
        MENUSONG = MEETINGA_MUS;
        ROSTER_MUS = LEVELA_MUS;
        TEXTSONG = RUMBAA_MUS;
        TITLE_LOOP_MUSIC = GOLDA_MUS;
    }
}


int16_t TELEPORT_LUMP_START = 0;
int16_t TELEPORT_LUMP_END = 0;

int16_t README_LUMP_START = 0;
int16_t README_LUMP_END = 0;

int16_t CONTROLS_LUMP_START = 0;
int16_t CONTROLS_LUMP_END = 0;

int16_t LATCHPICS_LUMP_START = 0;
int16_t LATCHPICS_LUMP_END = 0;


//
// Amount of each data item
//
int16_t NUMCHUNKS = 0;
int16_t NUMFONT = 0;
int16_t NUMFONTM = 0;
int16_t NUMPICS = 0;
int16_t NUMPICM = 0;
int16_t NUMSPRITES = 0;
int16_t NUMTILE8 = 0;
int16_t NUMTILE8M = 0;
int16_t NUMTILE16 = 0;
int16_t NUMTILE16M = 0;
int16_t NUMTILE32 = 0;
int16_t NUMTILE32M = 0;
int16_t NUMEXTERNS = 0;

//
// File offsets for data items
//
int16_t STRUCTPIC = 0;

int16_t STARTFONT = 0;
int16_t STARTFONTM = 0;
int16_t STARTPICS = 0;
int16_t STARTPICM = 0;
int16_t STARTSPRITES = 0;
int16_t STARTTILE8 = 0;
int16_t STARTTILE8M = 0;
int16_t STARTTILE16 = 0;
int16_t STARTTILE16M = 0;
int16_t STARTTILE32 = 0;
int16_t STARTTILE32M = 0;
int16_t STARTEXTERNS = 0;


int16_t TELEPORTBACKPIC = 0;
int16_t TELEPORT1OFFPIC = 0;
int16_t TELEPORT2OFFPIC = 0;
int16_t TELEPORT3OFFPIC = 0;
int16_t TELEPORT4OFFPIC = 0;
int16_t TELEPORT5OFFPIC = 0;
int16_t TELEPORT6OFFPIC = 0;
int16_t TELEPORT7OFFPIC = 0;
int16_t TELEPORT8OFFPIC = 0;
int16_t TELEPORT9OFFPIC = 0;
int16_t TELEPORT10OFFPIC = 0;
int16_t TELEPORT1ONPIC = 0;
int16_t TELEPORT2ONPIC = 0;
int16_t TELEPORT3ONPIC = 0;
int16_t TELEPORT4ONPIC = 0;
int16_t TELEPORT5ONPIC = 0;
int16_t TELEPORT6ONPIC = 0;
int16_t TELEPORT7ONPIC = 0;
int16_t TELEPORT8ONPIC = 0;
int16_t TELEPORT9ONPIC = 0;
int16_t TELEPORT10ONPIC = 0;
int16_t TELEPORT_TEXT_BG = 0;
int16_t BACKGROUND_SCREENPIC = 0;
int16_t APOGEEPIC = 0;
int16_t PIRACYPIC = 0;
int16_t PC13PIC = 0;
int16_t LOSEPIC = 0;
int16_t AUTOMAPPIC = 0;
int16_t PSPROMO1PIC = 0;
int16_t PSPROMO2PIC = 0;
int16_t PSPROMO3PIC = 0;
int16_t H_ALTPIC = 0;
int16_t H_CTRLPIC = 0;
int16_t H_SPACEPIC = 0;
int16_t H_PAUSEPIC = 0;
int16_t H_ESCPIC = 0;
int16_t H_LTARROWPIC = 0;
int16_t H_UPARROWPIC = 0;
int16_t H_DNARROWPIC = 0;
int16_t H_RTARROWPIC = 0;
int16_t H_ENTERPIC = 0;
int16_t H_QPIC = 0;
int16_t H_WPIC = 0;
int16_t H_EPIC = 0;
int16_t H_IPIC = 0;
int16_t H_HPIC = 0;
int16_t H_1PIC = 0;
int16_t H_2PIC = 0;
int16_t H_3PIC = 0;
int16_t H_4PIC = 0;
int16_t H_5PIC = 0;
int16_t H_F1PIC = 0;
int16_t H_F2PIC = 0;
int16_t H_F3PIC = 0;
int16_t H_F4PIC = 0;
int16_t H_F5PIC = 0;
int16_t H_F6PIC = 0;
int16_t H_F7PIC = 0;
int16_t H_F8PIC = 0;
int16_t H_F9PIC = 0;
int16_t H_F10PIC = 0;
int16_t H_TABPIC = 0;
int16_t H_CPIC = 0;
int16_t H_FPIC = 0;
int16_t H_PPIC = 0;
int16_t H_MPIC = 0;
int16_t H_LPIC = 0;
int16_t H_SHIFTPIC = 0;
int16_t APOGEE_LOGOPIC = 0;
int16_t VISAPIC = 0;
int16_t MCPIC = 0;
int16_t FAXPIC = 0;
int16_t H_TOPWINDOWPIC = 0;
int16_t H_LEFTWINDOWPIC = 0;
int16_t H_RIGHTWINDOWPIC = 0;
int16_t H_BOTTOMINFOPIC = 0;
int16_t C_NOTSELECTEDPIC = 0;
int16_t C_SELECTEDPIC = 0;
int16_t C_NOTSELECTED_HIPIC = 0;
int16_t C_SELECTED_HIPIC = 0;
int16_t C_BABYMODEPIC = 0;
int16_t C_EASYPIC = 0;
int16_t C_NORMALPIC = 0;
int16_t C_HARDPIC = 0;
int16_t C_EPISODE1PIC = 0;
int16_t C_EPISODE2PIC = 0;
int16_t C_EPISODE3PIC = 0;
int16_t C_EPISODE4PIC = 0;
int16_t C_EPISODE5PIC = 0;
int16_t C_EPISODE6PIC = 0;
int16_t BIGGOLDSTERNPIC = 0;
int16_t STARLOGOPIC = 0;
int16_t BLAKEWITHGUNPIC = 0;
int16_t STARINSTITUTEPIC = 0;
int16_t MEDALOFHONORPIC = 0;
int16_t SMALLGOLDSTERNPIC = 0;
int16_t BLAKEWINPIC = 0;
int16_t SHUTTLEEXPPIC = 0;
int16_t PLANETSPIC = 0;
int16_t MOUSEPIC = 0;
int16_t JOYSTICKPIC = 0;
int16_t GRAVISPADPIC = 0;
int16_t TITLEPIC = 0;
int16_t PROMO1PIC = 0;
int16_t PROMO2PIC = 0;
int16_t WEAPON1PIC = 0;
int16_t WEAPON2PIC = 0;
int16_t WEAPON3PIC = 0;
int16_t WEAPON4PIC = 0;
int16_t WEAPON5PIC = 0;
int16_t WAITPIC = 0;
int16_t READYPIC = 0;
int16_t N_BLANKPIC = 0;
int16_t N_0PIC = 0;
int16_t N_1PIC = 0;
int16_t N_2PIC = 0;
int16_t N_3PIC = 0;
int16_t N_4PIC = 0;
int16_t N_5PIC = 0;
int16_t N_6PIC = 0;
int16_t N_7PIC = 0;
int16_t N_8PIC = 0;
int16_t N_9PIC = 0;
int16_t N_RPIC = 0;
int16_t N_OPIC = 0;
int16_t N_LPIC = 0;
int16_t N_DASHPIC = 0;
int16_t DIM_LIGHTPIC = 0;
int16_t BRI_LIGHTPIC = 0;
int16_t ECG_HEART_GOOD = 0;
int16_t ECG_HEART_BAD = 0;
int16_t ECG_GRID_PIECE = 0;
int16_t ECG_HEARTBEAT_00 = 0;
int16_t ECG_HEARTBEAT_01 = 0;
int16_t ECG_HEARTBEAT_02 = 0;
int16_t ECG_HEARTBEAT_03 = 0;
int16_t ECG_HEARTBEAT_04 = 0;
int16_t ECG_HEARTBEAT_05 = 0;
int16_t ECG_HEARTBEAT_06 = 0;
int16_t ECG_HEARTBEAT_07 = 0;
int16_t ECG_HEARTBEAT_08 = 0;
int16_t ECG_HEARTBEAT_09 = 0;
int16_t ECG_HEARTBEAT_10 = 0;
int16_t ECG_HEARTBEAT_11 = 0;
int16_t ECG_HEARTBEAT_12 = 0;
int16_t ECG_HEARTBEAT_13 = 0;
int16_t ECG_HEARTBEAT_14 = 0;
int16_t ECG_HEARTBEAT_15 = 0;
int16_t ECG_HEARTBEAT_16 = 0;
int16_t ECG_HEARTBEAT_17 = 0;
int16_t ECG_HEARTBEAT_18 = 0;
int16_t ECG_HEARTBEAT_19 = 0;
int16_t ECG_HEARTBEAT_20 = 0;
int16_t ECG_HEARTBEAT_21 = 0;
int16_t ECG_HEARTBEAT_22 = 0;
int16_t ECG_HEARTBEAT_23 = 0;
int16_t ECG_HEARTBEAT_24 = 0;
int16_t ECG_HEARTBEAT_25 = 0;
int16_t ECG_HEARTBEAT_26 = 0;
int16_t ECG_HEARTBEAT_27 = 0;
int16_t INFOAREAPIC = 0;
int16_t TOP_STATUSBARPIC = 0;
int16_t STATUSBARPIC = 0;
int16_t PIRACYPALETTE = 0;
int16_t APOGEEPALETTE = 0;
int16_t TITLEPALETTE = 0;
int16_t ORDERSCREEN = 0;
int16_t ERRORSCREEN = 0;
int16_t INFORMANT_HINTS = 0;
int16_t NICE_SCIE_HINTS = 0;
int16_t MEAN_SCIE_HINTS = 0;
int16_t BRIEF_W1 = 0;
int16_t BRIEF_I1 = 0;
int16_t BRIEF_W2 = 0;
int16_t BRIEF_I2 = 0;
int16_t BRIEF_W3 = 0;
int16_t BRIEF_I3 = 0;
int16_t BRIEF_W4 = 0;
int16_t BRIEF_I4 = 0;
int16_t BRIEF_W5 = 0;
int16_t BRIEF_I5 = 0;
int16_t BRIEF_W6 = 0;
int16_t BRIEF_I6 = 0;
int16_t LEVEL_DESCS = 0;
int16_t POWERBALLTEXT = 0;
int16_t TICSTEXT = 0;
int16_t MUSICTEXT = 0;
int16_t RADARTEXT = 0;
int16_t HELPTEXT = 0;
int16_t SAGATEXT = 0;
int16_t LOSETEXT = 0;
int16_t ORDERTEXT = 0;
int16_t CREDITSTEXT = 0;
int16_t MUSTBE386TEXT = 0;
int16_t QUICK_INFO1_TEXT = 0;
int16_t QUICK_INFO2_TEXT = 0;
int16_t BADINFO_TEXT = 0;
int16_t CALJOY1_TEXT = 0;
int16_t CALJOY2_TEXT = 0;
int16_t READTHIS_TEXT = 0;
int16_t ELEVMSG0_TEXT = 0;
int16_t ELEVMSG1_TEXT = 0;
int16_t ELEVMSG4_TEXT = 0;
int16_t ELEVMSG5_TEXT = 0;
int16_t FLOORMSG_TEXT = 0;
int16_t YOUWIN_TEXT = 0;
int16_t CHANGEVIEW_TEXT = 0;
int16_t DIZ_ERR_TEXT = 0;
int16_t BADLEVELSTEXT = 0;
int16_t BADSAVEGAME_TEXT = 0;

int16_t TELEPORTBACKTOPPIC = 0;
int16_t TELEPORTBACKBOTPIC = 0;
int16_t TELEPORT11ONPIC = 0;
int16_t TELEPORT12ONPIC = 0;
int16_t TELEPORT13ONPIC = 0;
int16_t TELEPORT14ONPIC = 0;
int16_t TELEPORT15ONPIC = 0;
int16_t TELEPORT16ONPIC = 0;
int16_t TELEPORT17ONPIC = 0;
int16_t TELEPORT18ONPIC = 0;
int16_t TELEPORT19ONPIC = 0;
int16_t TELEPORT20ONPIC = 0;
int16_t TELEUPONPIC = 0;
int16_t TELEDNONPIC = 0;
int16_t TELEUPOFFPIC = 0;
int16_t TELEDNOFFPIC = 0;
int16_t TELEPORT11OFFPIC = 0;
int16_t TELEPORT12OFFPIC = 0;
int16_t TELEPORT13OFFPIC = 0;
int16_t TELEPORT14OFFPIC = 0;
int16_t TELEPORT15OFFPIC = 0;
int16_t TELEPORT16OFFPIC = 0;
int16_t TELEPORT17OFFPIC = 0;
int16_t TELEPORT18OFFPIC = 0;
int16_t TELEPORT19OFFPIC = 0;
int16_t TELEPORT20OFFPIC = 0;
int16_t AUTOMAP_MAG1PIC = 0;
int16_t AUTOMAP_MAG2PIC = 0;
int16_t AUTOMAP_MAG4PIC = 0;
int16_t H_6PIC = 0;
int16_t H_TILDEPIC = 0;
int16_t H_PLUSPIC = 0;
int16_t H_MINUSPIC = 0;
int16_t STARPORTPIC = 0;
int16_t BOSSPIC = 0;
int16_t THREEPLANETSPIC = 0;
int16_t SOLARSYSTEMPIC = 0;
int16_t AOGENDINGPIC = 0;
int16_t GFLOGOSPIC = 0;
int16_t BLAKEHEADPIC = 0;
int16_t PROJECTFOLDERPIC = 0;
int16_t TITLE1PIC = 0;
int16_t TITLE2PIC = 0;
int16_t WEAPON6PIC = 0;
int16_t WEAPON7PIC = 0;
int16_t W1_CORNERPIC = 0;
int16_t W2_CORNERPIC = 0;
int16_t W3_CORNERPIC = 0;
int16_t W4_CORNERPIC = 0;
int16_t W5_CORNERPIC = 0;
int16_t W6_CORNERPIC = 0;
int16_t NG_BLANKPIC = 0;
int16_t NG_0PIC = 0;
int16_t NG_1PIC = 0;
int16_t NG_2PIC = 0;
int16_t NG_3PIC = 0;
int16_t NG_4PIC = 0;
int16_t NG_5PIC = 0;
int16_t NG_6PIC = 0;
int16_t NG_7PIC = 0;
int16_t NG_8PIC = 0;
int16_t NG_9PIC = 0;
int16_t ONEXZOOMPIC = 0;
int16_t TWOXZOOMPIC = 0;
int16_t FOURXZOOMPIC = 0;
int16_t NO_KEYPIC = 0;
int16_t RED_KEYPIC = 0;
int16_t YEL_KEYPIC = 0;
int16_t BLU_KEYPIC = 0;
int16_t ENDINGPALETTE = 0;
int16_t NO386SCREEN = 0;
int16_t T_DEMO0 = 0;
int16_t T_DEMO1 = 0;
int16_t T_DEMO2 = 0;
int16_t T_DEMO3 = 0;
int16_t T_DEMO4 = 0;
int16_t T_DEMO5 = 0;
int16_t DECOY = 0;
int16_t DECOY2 = 0;
int16_t DECOY3 = 0;
int16_t DECOY4 = 0;


void initialize_gfxv_contants()
{
    if (::is_aog_sw()) {
        TELEPORTBACKPIC = 6;
        TELEPORT1OFFPIC = 7;
        TELEPORT2OFFPIC = 8;
        TELEPORT3OFFPIC = 9;
        TELEPORT4OFFPIC = 10;
        TELEPORT5OFFPIC = 11;
        TELEPORT6OFFPIC = 12;
        TELEPORT7OFFPIC = 13;
        TELEPORT8OFFPIC = 14;
        TELEPORT9OFFPIC = 15;
        TELEPORT10OFFPIC = 16;
        TELEPORT1ONPIC = 17;
        TELEPORT2ONPIC = 18;
        TELEPORT3ONPIC = 19;
        TELEPORT4ONPIC = 20;
        TELEPORT5ONPIC = 21;
        TELEPORT6ONPIC = 22;
        TELEPORT7ONPIC = 23;
        TELEPORT8ONPIC = 24;
        TELEPORT9ONPIC = 25;
        TELEPORT10ONPIC = 26;
        TELEPORT_TEXT_BG = 27;
        BACKGROUND_SCREENPIC = 28;
        APOGEEPIC = 29;
        PC13PIC = 30;
        LOSEPIC = 31;
        PSPROMO1PIC = 32;
        PSPROMO2PIC = 33;
        PSPROMO3PIC = 34;
        AUTOMAPPIC = 35;
        H_ALTPIC = 36;
        H_CTRLPIC = 37;
        H_SPACEPIC = 38;
        H_PAUSEPIC = 39;
        H_ESCPIC = 40;
        H_LTARROWPIC = 41;
        H_UPARROWPIC = 42;
        H_DNARROWPIC = 43;
        H_RTARROWPIC = 44;
        H_ENTERPIC = 45;
        H_QPIC = 46;
        H_WPIC = 47;
        H_EPIC = 48;
        H_IPIC = 49;
        H_HPIC = 50;
        H_1PIC = 51;
        H_2PIC = 52;
        H_3PIC = 53;
        H_4PIC = 54;
        H_5PIC = 55;
        H_F1PIC = 56;
        H_F2PIC = 57;
        H_F3PIC = 58;
        H_F4PIC = 59;
        H_F5PIC = 60;
        H_F6PIC = 61;
        H_F7PIC = 62;
        H_F8PIC = 63;
        H_F9PIC = 64;
        H_F10PIC = 65;
        H_TABPIC = 66;
        H_CPIC = 67;
        H_FPIC = 68;
        H_PPIC = 69;
        H_MPIC = 70;
        H_LPIC = 71;
        H_SHIFTPIC = 72;
        APOGEE_LOGOPIC = 73;
        VISAPIC = 74;
        MCPIC = 75;
        FAXPIC = 76;
        H_TOPWINDOWPIC = 77;
        H_LEFTWINDOWPIC = 78;
        H_RIGHTWINDOWPIC = 79;
        H_BOTTOMINFOPIC = 80;
        C_NOTSELECTEDPIC = 81;
        C_SELECTEDPIC = 82;
        C_NOTSELECTED_HIPIC = 83;
        C_SELECTED_HIPIC = 84;
        C_BABYMODEPIC = 85;
        C_EASYPIC = 86;
        C_NORMALPIC = 87;
        C_HARDPIC = 88;
        C_EPISODE1PIC = 89;
        C_EPISODE2PIC = 90;
        C_EPISODE3PIC = 91;
        C_EPISODE4PIC = 92;
        C_EPISODE5PIC = 93;
        C_EPISODE6PIC = 94;
        BIGGOLDSTERNPIC = 95;
        STARLOGOPIC = 96;
        BLAKEWITHGUNPIC = 97;
        STARINSTITUTEPIC = 98;
        MEDALOFHONORPIC = 99;
        SMALLGOLDSTERNPIC = 100;
        BLAKEWINPIC = 101;
        SHUTTLEEXPPIC = 102;
        PLANETSPIC = 103;
        MOUSEPIC = 104;
        JOYSTICKPIC = 105;
        GRAVISPADPIC = 106;
        TITLEPIC = 107;
        PROMO1PIC = 108;
        PROMO2PIC = 109;
        WEAPON1PIC = 110;
        WEAPON2PIC = 111;
        WEAPON3PIC = 112;
        WEAPON4PIC = 113;
        WEAPON5PIC = 114;
        WAITPIC = 115;
        READYPIC = 116;
        N_BLANKPIC = 117;
        N_0PIC = 118;
        N_1PIC = 119;
        N_2PIC = 120;
        N_3PIC = 121;
        N_4PIC = 122;
        N_5PIC = 123;
        N_6PIC = 124;
        N_7PIC = 125;
        N_8PIC = 126;
        N_9PIC = 127;
        N_RPIC = 128;
        N_OPIC = 129;
        N_LPIC = 130;
        N_DASHPIC = 131;
        DIM_LIGHTPIC = 132;
        BRI_LIGHTPIC = 133;
        ECG_HEART_GOOD = 134;
        ECG_HEART_BAD = 135;
        ECG_GRID_PIECE = 136;
        // ??? = 137
        ECG_HEARTBEAT_00 = 138;
        ECG_HEARTBEAT_01 = 139;
        ECG_HEARTBEAT_02 = 140;
        ECG_HEARTBEAT_03 = 141;
        ECG_HEARTBEAT_04 = 142;
        ECG_HEARTBEAT_05 = 143;
        ECG_HEARTBEAT_06 = 144;
        ECG_HEARTBEAT_07 = 145;
        ECG_HEARTBEAT_08 = 146;
        ECG_HEARTBEAT_09 = 147;
        ECG_HEARTBEAT_10 = 148;
        ECG_HEARTBEAT_11 = 149;
        ECG_HEARTBEAT_12 = 150;
        ECG_HEARTBEAT_13 = 151;
        ECG_HEARTBEAT_14 = 152;
        ECG_HEARTBEAT_15 = 153;
        ECG_HEARTBEAT_16 = 154;
        ECG_HEARTBEAT_17 = 155;
        ECG_HEARTBEAT_18 = 156;
        ECG_HEARTBEAT_19 = 157;
        ECG_HEARTBEAT_20 = 158;
        ECG_HEARTBEAT_21 = 159;
        ECG_HEARTBEAT_22 = 160;
        ECG_HEARTBEAT_23 = 161;
        ECG_HEARTBEAT_24 = 162;
        ECG_HEARTBEAT_25 = 163;
        ECG_HEARTBEAT_26 = 164;
        ECG_HEARTBEAT_27 = 165;
        INFOAREAPIC = 166;
        TOP_STATUSBARPIC = 167;
        STATUSBARPIC = 168;
        APOGEEPALETTE = 171;
        TITLEPALETTE = 175;
        ORDERSCREEN = 176;
        ERRORSCREEN = 177;
        INFORMANT_HINTS = 181;
        NICE_SCIE_HINTS = 182;
        MEAN_SCIE_HINTS = 183;
        BRIEF_W1 = 184;
        BRIEF_I1 = 185;
        LEVEL_DESCS = 186;
        POWERBALLTEXT = 187;
        TICSTEXT = 188;
        MUSICTEXT = 189;
        RADARTEXT = 190;
        HELPTEXT = 191;
        SAGATEXT = 192;
        LOSETEXT = 193;
        ORDERTEXT = 194;
        CREDITSTEXT = 195;
        MUSTBE386TEXT = 196;
        QUICK_INFO1_TEXT = 197;
        QUICK_INFO2_TEXT = 198;
        BADINFO_TEXT = 199;
        CALJOY1_TEXT = 200;
        CALJOY2_TEXT = 201;
        READTHIS_TEXT = 202;
        ELEVMSG0_TEXT = 203;
        ELEVMSG1_TEXT = 204;
        ELEVMSG4_TEXT = 205;
        ELEVMSG5_TEXT = 206;
        FLOORMSG_TEXT = 207;
        YOUWIN_TEXT = 208;
        CHANGEVIEW_TEXT = 209;
        BADLEVELSTEXT = 210;
        BADSAVEGAME_TEXT = 211;


        STRUCTPIC = 0;

        STARTFONT = 1;
        STARTFONTM = 6;
        STARTPICS = 6;
        STARTPICM = STATUSBARPIC + 1;
        STARTSPRITES = STATUSBARPIC + 1;
        STARTTILE8 = STATUSBARPIC + 1;
        STARTTILE8M = STATUSBARPIC + 2;
        STARTTILE16 = STATUSBARPIC + 2;
        STARTTILE16M = STATUSBARPIC + 2;
        STARTTILE32 = STATUSBARPIC + 2;
        STARTTILE32M = STATUSBARPIC + 2;
        STARTEXTERNS = STATUSBARPIC + 2;

        TELEPORT_LUMP_START = TELEPORTBACKPIC;
        TELEPORT_LUMP_END = TELEPORT_TEXT_BG;

        README_LUMP_START = H_ALTPIC;
        README_LUMP_END = H_BOTTOMINFOPIC;

        CONTROLS_LUMP_START = C_NOTSELECTEDPIC;
        CONTROLS_LUMP_END = C_EPISODE6PIC;

        LATCHPICS_LUMP_START = WEAPON1PIC;
        LATCHPICS_LUMP_END = STATUSBARPIC;

        NUMCHUNKS = 212;
        NUMFONT = 5;
        NUMFONTM = 0;
        NUMPICS = 163;
        NUMPICM = 0;
        NUMSPRITES = 0;
        NUMTILE8 = 72;
        NUMTILE8M = 0;
        NUMTILE16 = 0;
        NUMTILE16M = 0;
        NUMTILE32 = 0;
        NUMTILE32M = 0;
        NUMEXTERNS = NUMCHUNKS - (STATUSBARPIC + 2);
    } else if (::is_aog_full()) {
        TELEPORTBACKPIC = 6;
        TELEPORT1OFFPIC = 7;
        TELEPORT2OFFPIC = 8;
        TELEPORT3OFFPIC = 9;
        TELEPORT4OFFPIC = 10;
        TELEPORT5OFFPIC = 11;
        TELEPORT6OFFPIC = 12;
        TELEPORT7OFFPIC = 13;
        TELEPORT8OFFPIC = 14;
        TELEPORT9OFFPIC = 15;
        TELEPORT10OFFPIC = 16;
        TELEPORT1ONPIC = 17;
        TELEPORT2ONPIC = 18;
        TELEPORT3ONPIC = 19;
        TELEPORT4ONPIC = 20;
        TELEPORT5ONPIC = 21;
        TELEPORT6ONPIC = 22;
        TELEPORT7ONPIC = 23;
        TELEPORT8ONPIC = 24;
        TELEPORT9ONPIC = 25;
        TELEPORT10ONPIC = 26;
        TELEPORT_TEXT_BG = 27;
        BACKGROUND_SCREENPIC = 28;
        APOGEEPIC = 29;
        PIRACYPIC = 30;
        PC13PIC = 31;
        LOSEPIC = 32;
        AUTOMAPPIC = 33;
        H_ALTPIC = 34;
        H_CTRLPIC = 35;
        H_SPACEPIC = 36;
        H_PAUSEPIC = 37;
        H_ESCPIC = 38;
        H_LTARROWPIC = 39;
        H_UPARROWPIC = 40;
        H_DNARROWPIC = 41;
        H_RTARROWPIC = 42;
        H_ENTERPIC = 43;
        H_QPIC = 44;
        H_WPIC = 45;
        H_EPIC = 46;
        H_IPIC = 47;
        H_HPIC = 48;
        H_1PIC = 49;
        H_2PIC = 50;
        H_3PIC = 51;
        H_4PIC = 52;
        H_5PIC = 53;
        H_F1PIC = 54;
        H_F2PIC = 55;
        H_F3PIC = 56;
        H_F4PIC = 57;
        H_F5PIC = 58;
        H_F6PIC = 59;
        H_F7PIC = 60;
        H_F8PIC = 61;
        H_F9PIC = 62;
        H_F10PIC = 63;
        H_TABPIC = 64;
        H_CPIC = 65;
        H_FPIC = 66;
        H_PPIC = 67;
        H_MPIC = 68;
        H_LPIC = 69;
        H_SHIFTPIC = 70;
        APOGEE_LOGOPIC = 71;
        VISAPIC = 72;
        MCPIC = 73;
        FAXPIC = 74;
        H_TOPWINDOWPIC = 75;
        H_LEFTWINDOWPIC = 76;
        H_RIGHTWINDOWPIC = 77;
        H_BOTTOMINFOPIC = 78;
        C_NOTSELECTEDPIC = 79;
        C_SELECTEDPIC = 80;
        C_NOTSELECTED_HIPIC = 81;
        C_SELECTED_HIPIC = 82;
        C_BABYMODEPIC = 83;
        C_EASYPIC = 84;
        C_NORMALPIC = 85;
        C_HARDPIC = 86;
        C_EPISODE1PIC = 87;
        C_EPISODE2PIC = 88;
        C_EPISODE3PIC = 89;
        C_EPISODE4PIC = 90;
        C_EPISODE5PIC = 91;
        C_EPISODE6PIC = 92;
        BIGGOLDSTERNPIC = 93;
        STARLOGOPIC = 94;
        BLAKEWITHGUNPIC = 95;
        STARINSTITUTEPIC = 96;
        MEDALOFHONORPIC = 97;
        SMALLGOLDSTERNPIC = 98;
        BLAKEWINPIC = 99;
        SHUTTLEEXPPIC = 100;
        PLANETSPIC = 101;
        MOUSEPIC = 102;
        JOYSTICKPIC = 103;
        GRAVISPADPIC = 104;
        TITLEPIC = 105;
        WEAPON1PIC = 106;
        WEAPON2PIC = 107;
        WEAPON3PIC = 108;
        WEAPON4PIC = 109;
        WEAPON5PIC = 110;
        WAITPIC = 111;
        READYPIC = 112;
        N_BLANKPIC = 113;
        N_0PIC = 114;
        N_1PIC = 115;
        N_2PIC = 116;
        N_3PIC = 117;
        N_4PIC = 118;
        N_5PIC = 119;
        N_6PIC = 120;
        N_7PIC = 121;
        N_8PIC = 122;
        N_9PIC = 123;
        N_RPIC = 124;
        N_OPIC = 125;
        N_LPIC = 126;
        N_DASHPIC = 127;
        DIM_LIGHTPIC = 128;
        BRI_LIGHTPIC = 129;
        ECG_HEART_GOOD = 130;
        ECG_HEART_BAD = 131;
        ECG_GRID_PIECE = 132;
        ECG_HEARTBEAT_00 = 134;
        ECG_HEARTBEAT_01 = 135;
        ECG_HEARTBEAT_02 = 136;
        ECG_HEARTBEAT_03 = 137;
        ECG_HEARTBEAT_04 = 138;
        ECG_HEARTBEAT_05 = 139;
        ECG_HEARTBEAT_06 = 140;
        ECG_HEARTBEAT_07 = 141;
        ECG_HEARTBEAT_08 = 142;
        ECG_HEARTBEAT_09 = 143;
        ECG_HEARTBEAT_10 = 144;
        ECG_HEARTBEAT_11 = 145;
        ECG_HEARTBEAT_12 = 146;
        ECG_HEARTBEAT_13 = 147;
        ECG_HEARTBEAT_14 = 148;
        ECG_HEARTBEAT_15 = 149;
        ECG_HEARTBEAT_16 = 150;
        ECG_HEARTBEAT_17 = 151;
        ECG_HEARTBEAT_18 = 152;
        ECG_HEARTBEAT_19 = 153;
        ECG_HEARTBEAT_20 = 154;
        ECG_HEARTBEAT_21 = 155;
        ECG_HEARTBEAT_22 = 156;
        ECG_HEARTBEAT_23 = 157;
        ECG_HEARTBEAT_24 = 158;
        ECG_HEARTBEAT_25 = 159;
        ECG_HEARTBEAT_26 = 160;
        ECG_HEARTBEAT_27 = 161;
        INFOAREAPIC = 162;
        TOP_STATUSBARPIC = 163;
        STATUSBARPIC = 164;
        PIRACYPALETTE = 167;
        APOGEEPALETTE = 168;
        TITLEPALETTE = 169;
        ORDERSCREEN = 170;
        ERRORSCREEN = 171;
        INFORMANT_HINTS = 181;
        NICE_SCIE_HINTS = 182;
        MEAN_SCIE_HINTS = 183;
        BRIEF_W1 = 184;
        BRIEF_I1 = 185;
        BRIEF_W2 = 186;
        BRIEF_I2 = 187;
        BRIEF_W3 = 188;
        BRIEF_I3 = 189;
        BRIEF_W4 = 190;
        BRIEF_I4 = 191;
        BRIEF_W5 = 192;
        BRIEF_I5 = 193;
        BRIEF_W6 = 194;
        BRIEF_I6 = 195;
        LEVEL_DESCS = 196;
        POWERBALLTEXT = 197;
        TICSTEXT = 198;
        MUSICTEXT = 199;
        RADARTEXT = 200;
        HELPTEXT = 201;
        SAGATEXT = 202;
        LOSETEXT = 203;
        ORDERTEXT = 204;
        CREDITSTEXT = 205;
        MUSTBE386TEXT = 206;
        QUICK_INFO1_TEXT = 207;
        QUICK_INFO2_TEXT = 208;
        BADINFO_TEXT = 209;
        CALJOY1_TEXT = 210;
        CALJOY2_TEXT = 211;
        READTHIS_TEXT = 212;
        ELEVMSG0_TEXT = 213;
        ELEVMSG1_TEXT = 214;
        ELEVMSG4_TEXT = 215;
        ELEVMSG5_TEXT = 216;
        FLOORMSG_TEXT = 217;
        YOUWIN_TEXT = 218;
        CHANGEVIEW_TEXT = 219;
        DIZ_ERR_TEXT = 220;
        BADLEVELSTEXT = 221;
        BADSAVEGAME_TEXT = 222;


        STRUCTPIC = 0;

        STARTFONT = 1;
        STARTFONTM = 6;
        STARTPICS = 6;
        STARTPICM = STATUSBARPIC + 1;
        STARTSPRITES = STATUSBARPIC + 1;
        STARTTILE8 = STATUSBARPIC + 1;
        STARTTILE8M = STATUSBARPIC + 2;
        STARTTILE16 = STATUSBARPIC + 2;
        STARTTILE16M = STATUSBARPIC + 2;
        STARTTILE32 = STATUSBARPIC + 2;
        STARTTILE32M = STATUSBARPIC + 2;
        STARTEXTERNS = STATUSBARPIC + 2;

        TELEPORT_LUMP_START = TELEPORTBACKPIC;
        TELEPORT_LUMP_END = TELEPORT_TEXT_BG;

        README_LUMP_START = H_ALTPIC;
        README_LUMP_END = H_BOTTOMINFOPIC;

        CONTROLS_LUMP_START = C_NOTSELECTEDPIC;
        CONTROLS_LUMP_END = C_EPISODE6PIC;

        LATCHPICS_LUMP_START = WEAPON1PIC;
        LATCHPICS_LUMP_END = STATUSBARPIC;

        NUMCHUNKS = 223;
        NUMFONT = 5;
        NUMFONTM = 0;
        NUMPICS = 159;
        NUMPICM = 0;
        NUMSPRITES = 0;
        NUMTILE8 = 72;
        NUMTILE8M = 0;
        NUMTILE16 = 0;
        NUMTILE16M = 0;
        NUMTILE32 = 0;
        NUMTILE32M = 0;
        NUMEXTERNS = NUMCHUNKS - (STARTEXTERNS + 2);
    } else if (::is_ps()) {
        TELEPORTBACKTOPPIC = 6;
        TELEPORTBACKBOTPIC = 7;
        TELEPORT1ONPIC = 8;
        TELEPORT2ONPIC = 9;
        TELEPORT3ONPIC = 10;
        TELEPORT4ONPIC = 11;
        TELEPORT5ONPIC = 12;
        TELEPORT6ONPIC = 13;
        TELEPORT7ONPIC = 14;
        TELEPORT8ONPIC = 15;
        TELEPORT9ONPIC = 16;
        TELEPORT10ONPIC = 17;
        TELEPORT11ONPIC = 18;
        TELEPORT12ONPIC = 19;
        TELEPORT13ONPIC = 20;
        TELEPORT14ONPIC = 21;
        TELEPORT15ONPIC = 22;
        TELEPORT16ONPIC = 23;
        TELEPORT17ONPIC = 24;
        TELEPORT18ONPIC = 25;
        TELEPORT19ONPIC = 26;
        TELEPORT20ONPIC = 27;
        TELEUPONPIC = 28;
        TELEDNONPIC = 29;
        TELEUPOFFPIC = 30;
        TELEDNOFFPIC = 31;
        TELEPORT1OFFPIC = 32;
        TELEPORT2OFFPIC = 33;
        TELEPORT3OFFPIC = 34;
        TELEPORT4OFFPIC = 35;
        TELEPORT5OFFPIC = 36;
        TELEPORT6OFFPIC = 37;
        TELEPORT7OFFPIC = 38;
        TELEPORT8OFFPIC = 39;
        TELEPORT9OFFPIC = 40;
        TELEPORT10OFFPIC = 41;
        TELEPORT11OFFPIC = 42;
        TELEPORT12OFFPIC = 43;
        TELEPORT13OFFPIC = 44;
        TELEPORT14OFFPIC = 45;
        TELEPORT15OFFPIC = 46;
        TELEPORT16OFFPIC = 47;
        TELEPORT17OFFPIC = 48;
        TELEPORT18OFFPIC = 49;
        TELEPORT19OFFPIC = 50;
        TELEPORT20OFFPIC = 51;
        BACKGROUND_SCREENPIC = 52;
        APOGEEPIC = 53;
        PC13PIC = 54;
        LOSEPIC = 55;
        AUTOMAPPIC = 56;
        AUTOMAP_MAG1PIC = 57;
        AUTOMAP_MAG2PIC = 58;
        AUTOMAP_MAG4PIC = 59;
        H_ALTPIC = 60;
        H_CTRLPIC = 61;
        H_SPACEPIC = 62;
        H_PAUSEPIC = 63;
        H_ESCPIC = 64;
        H_LTARROWPIC = 65;
        H_UPARROWPIC = 66;
        H_DNARROWPIC = 67;
        H_RTARROWPIC = 68;
        H_ENTERPIC = 69;
        H_QPIC = 70;
        H_WPIC = 71;
        H_EPIC = 72;
        H_IPIC = 73;
        H_HPIC = 74;
        H_1PIC = 75;
        H_2PIC = 76;
        H_3PIC = 77;
        H_4PIC = 78;
        H_5PIC = 79;
        H_F1PIC = 80;
        H_F2PIC = 81;
        H_F3PIC = 82;
        H_F4PIC = 83;
        H_F5PIC = 84;
        H_F6PIC = 85;
        H_F7PIC = 86;
        H_F8PIC = 87;
        H_F9PIC = 88;
        H_F10PIC = 89;
        H_TABPIC = 90;
        H_CPIC = 91;
        H_FPIC = 92;
        H_PPIC = 93;
        H_MPIC = 94;
        H_LPIC = 95;
        H_SHIFTPIC = 96;
        H_6PIC = 97;
        H_TILDEPIC = 98;
        H_PLUSPIC = 99;
        H_MINUSPIC = 100;
        APOGEE_LOGOPIC = 101;
        VISAPIC = 102;
        MCPIC = 103;
        FAXPIC = 104;
        H_TOPWINDOWPIC = 105;
        H_LEFTWINDOWPIC = 106;
        H_RIGHTWINDOWPIC = 107;
        H_BOTTOMINFOPIC = 108;
        C_NOTSELECTEDPIC = 109;
        C_SELECTEDPIC = 110;
        C_NOTSELECTED_HIPIC = 111;
        C_SELECTED_HIPIC = 112;
        C_BABYMODEPIC = 113;
        C_EASYPIC = 114;
        C_NORMALPIC = 115;
        C_HARDPIC = 116;
        C_EPISODE1PIC = 117;
        C_EPISODE2PIC = 118;
        C_EPISODE3PIC = 119;
        C_EPISODE4PIC = 120;
        C_EPISODE5PIC = 121;
        C_EPISODE6PIC = 122;
        BIGGOLDSTERNPIC = 123;
        STARLOGOPIC = 124;
        BLAKEWITHGUNPIC = 125;
        STARINSTITUTEPIC = 126;
        MEDALOFHONORPIC = 127;
        SMALLGOLDSTERNPIC = 128;
        BLAKEWINPIC = 129;
        SHUTTLEEXPPIC = 130;
        PLANETSPIC = 131;
        MOUSEPIC = 132;
        JOYSTICKPIC = 133;
        GRAVISPADPIC = 134;
        STARPORTPIC = 135;
        BOSSPIC = 136;
        THREEPLANETSPIC = 137;
        SOLARSYSTEMPIC = 138;
        AOGENDINGPIC = 139;
        GFLOGOSPIC = 140;
        BLAKEHEADPIC = 141;
        PROJECTFOLDERPIC = 142;
        TITLE1PIC = 143;
        TITLE2PIC = 144;
        WEAPON1PIC = 145;
        WEAPON2PIC = 146;
        WEAPON3PIC = 147;
        WEAPON4PIC = 148;
        WEAPON5PIC = 149;
        WEAPON6PIC = 150;
        WEAPON7PIC = 151;
        W1_CORNERPIC = 152;
        W2_CORNERPIC = 153;
        W3_CORNERPIC = 154;
        W4_CORNERPIC = 155;
        W5_CORNERPIC = 156;
        W6_CORNERPIC = 157;
        WAITPIC = 158;
        READYPIC = 159;
        N_BLANKPIC = 160;
        N_0PIC = 161;
        N_1PIC = 162;
        N_2PIC = 163;
        N_3PIC = 164;
        N_4PIC = 165;
        N_5PIC = 166;
        N_6PIC = 167;
        N_7PIC = 168;
        N_8PIC = 169;
        N_9PIC = 170;
        N_RPIC = 171;
        N_OPIC = 172;
        N_LPIC = 173;
        N_DASHPIC = 174;
        NG_BLANKPIC = 175;
        NG_0PIC = 176;
        NG_1PIC = 177;
        NG_2PIC = 178;
        NG_3PIC = 179;
        NG_4PIC = 180;
        NG_5PIC = 181;
        NG_6PIC = 182;
        NG_7PIC = 183;
        NG_8PIC = 184;
        NG_9PIC = 185;
        DIM_LIGHTPIC = 186;
        BRI_LIGHTPIC = 187;
        INFOAREAPIC = 188;
        TOP_STATUSBARPIC = 189;
        STATUSBARPIC = 190;
        ONEXZOOMPIC = 191;
        TWOXZOOMPIC = 192;
        FOURXZOOMPIC = 193;
        NO_KEYPIC = 194;
        RED_KEYPIC = 195;
        YEL_KEYPIC = 196;
        BLU_KEYPIC = 197;
        POWERBALLTEXT = 199;
        PIRACYPALETTE = 200;
        APOGEEPALETTE = 201;
        ENDINGPALETTE = 202;
        TITLEPALETTE = 203;
        ORDERSCREEN = 204;
        ERRORSCREEN = 205;
        NO386SCREEN = 206;
        MUSICTEXT = 207;
        RADARTEXT = 208;
        TICSTEXT = 209;
        T_DEMO0 = 210;
        T_DEMO1 = 211;
        T_DEMO2 = 212;
        T_DEMO3 = 213;
        T_DEMO4 = 214;
        T_DEMO5 = 215;
        INFORMANT_HINTS = 216;
        NICE_SCIE_HINTS = 217;
        MEAN_SCIE_HINTS = 218;
        BRIEF_W1 = 219;
        BRIEF_I1 = 220;
        LEVEL_DESCS = 221;
        DECOY = 222;
        DECOY2 = 223;
        DECOY3 = 224;
        DECOY4 = 225;
        HELPTEXT = 226;
        SAGATEXT = 227;
        LOSETEXT = 228;
        ORDERTEXT = 229;
        CREDITSTEXT = 230;
        MUSTBE386TEXT = 231;
        QUICK_INFO1_TEXT = 232;
        QUICK_INFO2_TEXT = 233;
        BADINFO_TEXT = 234;
        CALJOY1_TEXT = 235;
        CALJOY2_TEXT = 236;
        READTHIS_TEXT = 237;
        ELEVMSG0_TEXT = 238;
        ELEVMSG1_TEXT = 239;
        ELEVMSG4_TEXT = 240;
        ELEVMSG5_TEXT = 241;
        FLOORMSG_TEXT = 242;
        YOUWIN_TEXT = 243;
        CHANGEVIEW_TEXT = 244;
        DIZ_ERR_TEXT = 245;
        BADLEVELSTEXT = 246;
        BADSAVEGAME_TEXT = 247;


        STARTFONT = 1;
        STARTFONTM = 6;
        STARTPICS = 6;
        STARTPICM = POWERBALLTEXT - 1;
        STARTSPRITES = POWERBALLTEXT - 1;
        STARTTILE8 = POWERBALLTEXT - 1;
        STARTTILE8M = POWERBALLTEXT;
        STARTTILE16 = POWERBALLTEXT;
        STARTTILE16M = POWERBALLTEXT;
        STARTTILE32 = POWERBALLTEXT;
        STARTTILE32M = POWERBALLTEXT;
        STARTEXTERNS = POWERBALLTEXT;

        TELEPORT_LUMP_START = TELEPORT1ONPIC;
        TELEPORT_LUMP_END = TELEPORT20OFFPIC;

        README_LUMP_START = H_ALTPIC;
        README_LUMP_END = H_BOTTOMINFOPIC;

        CONTROLS_LUMP_START = C_NOTSELECTEDPIC;
        CONTROLS_LUMP_END = C_EPISODE6PIC;

        LATCHPICS_LUMP_START = WEAPON1PIC;
        LATCHPICS_LUMP_END = BLU_KEYPIC;


        NUMCHUNKS = 248;
        NUMFONT = 5;
        NUMFONTM = 0;
        NUMPICS = 192;
        NUMPICM = 0;
        NUMSPRITES = 0;
        NUMTILE8 = 72;
        NUMTILE8M = 0;
        NUMTILE16 = 0;
        NUMTILE16M = 0;
        NUMTILE32 = 0;
        NUMTILE32M = 0;
        NUMEXTERNS = NUMCHUNKS - STARTEXTERNS;

        STRUCTPIC = 0;
    }

    grsegs.resize(NUMCHUNKS);
    grneeded.resize(NUMCHUNKS);
}


void InitSmartSpeedAnim(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir,
    uint16_t Delay)
{
    ::InitAnim(
        obj,
        ShapeNum,
        StartOfs,
        MaxOfs,
        AnimType,
        AnimDir,
        Delay,
        Delay);
}

void InitSmartAnim(
    objtype* obj,
    uint16_t ShapeNum,
    uint8_t StartOfs,
    uint8_t MaxOfs,
    animtype_t AnimType,
    animdir_t AnimDir)
{
    ::InitSmartSpeedAnim(
        obj,
        ShapeNum,
        StartOfs,
        MaxOfs,
        AnimType,
        AnimDir,
        ::is_ps() ? 7 : 21);
}

// ========================================================================
// ArchiveException

ArchiveException::ArchiveException(
    const char* what) throw () :
    what_(what)
{
}

ArchiveException::ArchiveException(
    const ArchiveException& that) throw () :
    what_(that.what_)
{
}

// (virtual)
ArchiveException::~ArchiveException() throw ()
{
}

ArchiveException& ArchiveException::operator=(
    const ArchiveException& that) throw ()
{
    what_ = that.what_;
    return *this;
}

// (virtual)
const char* ArchiveException::what() const throw ()
{
    return what_;
}

// ArchiveException
// ========================================================================

bstone::MemoryStream g_playtemp;

static bool is_config_loaded = false;

static const std::string& get_score_file_name()
{
    static std::string file_name;
    static bool is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        std::string game_type_string;

        switch (::g_game_type) {
        case GameType::aog_sw:
            game_type_string = "aog_sw";
            break;

        case GameType::aog_full:
            game_type_string = "aog_full";
            break;

        case GameType::ps:
            game_type_string = "ps";
            break;

        default:
            throw std::runtime_error("Invalid game type.");
        }

        file_name = "bstone_" + game_type_string + "_high_scores";
    }

    return file_name;
}

static void set_default_high_scores()
{
    Scores = {
        HighScore { "JAM PRODUCTIONS INC.", 10000, 1, 0, },
        HighScore { "", 10000, 1, 0, },
        HighScore { "JERRY JONES", 10000, 1, 0, },
        HighScore { "MICHAEL MAYNARD", 10000, 1, 0, },
        HighScore { "JAMES T. ROW", 10000, 1, 0, },
        HighScore { "", 10000, 1, 0, },
        HighScore { "", 10000, 1, 0, },
        HighScore { "TO REGISTER CALL", 10000, 1, 0, },
        HighScore { " 1-800-GAME123", 10000, 1, 0, },
        HighScore { "", 10000, 1, 0, },
    }; // Scores
}

void read_high_scores()
{
    auto is_succeed = true;

    auto scores_path = ::get_profile_dir() + ::get_score_file_name();

    HighScores scores(MaxScores);
    bstone::FileStream stream(scores_path);

    if (stream.is_open()) {
        bstone::Crc32 check_sum;
        bstone::BinaryReader reader(&stream);

        try {
            for (auto& score : scores) {
                ::deserialize_field(score.name, reader, check_sum);
                ::deserialize_field(score.score, reader, check_sum);
                ::deserialize_field(score.completed, reader, check_sum);
                ::deserialize_field(score.episode, reader, check_sum);
                ::deserialize_field(score.ratio, reader, check_sum);
            }
        } catch (const ArchiveException&) {
            is_succeed = false;
        }

        if (is_succeed) {
            uint32_t saved_checksum = 0;
            reader.read(saved_checksum);
            bstone::Endian::lei(saved_checksum);

            is_succeed = (saved_checksum == check_sum.get_value());
        }
    } else {
        is_succeed = false;
    }

    if (is_succeed) {
        ::Scores = scores;
    } else {
        ::set_default_high_scores();
    }
}

static void write_high_scores()
{
    auto scores_path = ::get_profile_dir() + ::get_score_file_name();

    bstone::FileStream stream(scores_path, bstone::StreamOpenMode::write);

    if (!stream.is_open()) {
        bstone::Log::write_error(
            "Failed to open a high scores file for writing: {}.",
            scores_path);

        return;
    }

    bstone::Crc32 checksum;
    bstone::BinaryWriter writer(&stream);

    for (const auto& score : Scores) {
        ::serialize_field(score.name, writer, checksum);
        ::serialize_field(score.score, writer, checksum);
        ::serialize_field(score.completed, writer, checksum);
        ::serialize_field(score.episode, writer, checksum);
        ::serialize_field(score.ratio, writer, checksum);
    }

    writer.write(bstone::Endian::le(checksum.get_value()));
}

static void set_vanilla_controls()
{
    dirscan = {
        ScanCode::sc_up_arrow,
        ScanCode::sc_right_arrow,
        ScanCode::sc_down_arrow,
        ScanCode::sc_left_arrow,
    }; // dirscan

    buttonscan = {
        ScanCode::sc_control,
        ScanCode::sc_alt,
        ScanCode::sc_right_shift,
        ScanCode::sc_space,
        ScanCode::sc_1,
        ScanCode::sc_2,
        ScanCode::sc_3,
        ScanCode::sc_4,
        ScanCode::sc_5,
        ScanCode::sc_6,
        ScanCode::sc_7,
        ScanCode::sc_none,
    }; // buttonscan

    buttonmouse = {
        bt_attack,
        bt_strafe,
        bt_use,
        bt_nobutton,
    }; // buttonmouse

    buttonjoy = {
        bt_attack,
        bt_strafe,
        bt_use,
        bt_run,
    }; // buttonjoy

    ::in_set_default_bindings();
}
// BBi

void ReadConfig()
{
    auto is_sound_enabled = false;
    auto is_music_enabled = false;

    bool is_succeed = true;
    uint16_t flags = gamestate.flags;

    auto config_path = ::get_profile_dir() + ::config_file_name;

    bstone::FileStream stream(config_path);

    ::set_vanilla_controls();

    if (stream.is_open()) {
        bstone::Crc32 checksum;
        bstone::BinaryReader reader(&stream);

        try {
            ::deserialize_field(is_sound_enabled, reader, checksum);
            ::deserialize_field(is_music_enabled, reader, checksum);

            ::deserialize_field(mouseenabled, reader, checksum);
            ::deserialize_field(joystickenabled, reader, checksum);
            ::deserialize_field(joypadenabled, reader, checksum);
            ::deserialize_field(joystickprogressive, reader, checksum);
            ::deserialize_field(joystickport, reader, checksum);

            ::deserialize_field(dirscan, reader, checksum);
            ::deserialize_field(buttonscan, reader, checksum);
            ::deserialize_field(buttonmouse, reader, checksum);
            ::deserialize_field(buttonjoy, reader, checksum);

            ::deserialize_field(in_bindings, reader, checksum);

            int16_t dummy_viewsize;
            ::deserialize_field(dummy_viewsize, reader, checksum);

            ::deserialize_field(mouseadjustment, reader, checksum);

            // Use temp so we don't destroy pre-sets.
            ::deserialize_field(flags, reader, checksum);

            uint8_t temp_u8;

            ::deserialize_field(temp_u8, reader, checksum);
            ::sd_sfx_volume = temp_u8;

            ::deserialize_field(temp_u8, reader, checksum);
            ::sd_music_volume = temp_u8;

            ::deserialize_field(g_no_wall_hit_sound, reader, checksum);
            ::deserialize_field(in_use_modern_bindings, reader, checksum);
            ::deserialize_field(g_always_run, reader, checksum);

            // AOG options
            ::deserialize_field(g_heart_beat_sound, reader, checksum);
            ::deserialize_field(g_rotated_automap, reader, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }

        if (is_succeed) {
            uint32_t saved_checksum = 0;
            reader.read(saved_checksum);
            bstone::Endian::lei(saved_checksum);

            is_succeed = (saved_checksum == checksum.get_value());
        }
    } else {
        is_succeed = false;
    }

    if (is_succeed) {
        flags &=
            GS_HEARTB_SOUND |
            GS_ATTACK_INFOAREA |
            GS_LIGHTING |
            GS_DRAW_CEILING |
            GS_DRAW_FLOOR; // Mask out the useful flags!

        gamestate.flags |= flags; // Must "OR", some flags are already set.

        if (is_sound_enabled && !::sd_has_audio) {
            is_sound_enabled = false;
        }

        if (is_music_enabled && !::sd_has_audio) {
            is_music_enabled = false;
        }

        if (!MousePresent) {
            mouseenabled = false;
        }

        if (!JoysPresent[joystickport]) {
            joystickenabled = false;
        }

        MainMenu[6].active = AT_ENABLED;
        MainItems.curpos = 0;

        if (sd_sfx_volume < ::sd_min_volume) {
            sd_sfx_volume = ::sd_min_volume;
        }

        if (sd_sfx_volume > ::sd_max_volume) {
            sd_sfx_volume = ::sd_max_volume;
        }

        if (sd_music_volume < ::sd_min_volume) {
            sd_music_volume = ::sd_min_volume;
        }

        if (sd_music_volume > ::sd_max_volume) {
            sd_music_volume = ::sd_max_volume;
        }
    }

    if (!is_succeed || viewsize == 0) {
        //
        // no config file, so select by hardware
        //

        if (::sd_has_audio) {
            is_sound_enabled = true;
            is_music_enabled = true;
        } else {
            is_sound_enabled = false;
            is_music_enabled = false;
        }

        if (MousePresent) {
            mouseenabled = true;
        }

        joystickenabled = false;
        joypadenabled = false;
        joystickport = 0;
        joystickprogressive = false;

        ::set_vanilla_controls();

        mouseadjustment = 5;
        gamestate.flags |= GS_HEARTB_SOUND | GS_ATTACK_INFOAREA;
        gamestate.flags |= GS_DRAW_CEILING | GS_DRAW_FLOOR | GS_LIGHTING;

        sd_sfx_volume = ::sd_default_sfx_volume;
        sd_music_volume = ::sd_default_music_volume;

        g_no_wall_hit_sound = k_no_wall_hit_sound_default;
        in_use_modern_bindings = k_in_use_modern_bindings_default;
        g_always_run = k_always_run_default;

        g_heart_beat_sound = false;
        g_rotated_automap = false;
    }

    ::SD_EnableSound(is_sound_enabled);
    ::SD_EnableMusic(is_music_enabled);

    sd_set_sfx_volume(sd_sfx_volume);
    sd_set_music_volume(sd_music_volume);

    ::is_config_loaded |= is_succeed;
}

void WriteConfig()
{
    auto config_path = ::get_profile_dir() + ::config_file_name;

    bstone::FileStream stream(config_path, bstone::StreamOpenMode::write);

    if (!stream.is_open()) {
        bstone::Log::write_error(
            "Failed to open a config file for writing: {}.",
            config_path);

        return;
    }

    bstone::Crc32 checksum;
    bstone::BinaryWriter writer(&stream);

    ::serialize_field(::sd_is_sound_enabled, writer, checksum);
    ::serialize_field(::sd_is_music_enabled, writer, checksum);

    ::serialize_field(mouseenabled, writer, checksum);
    ::serialize_field(joystickenabled, writer, checksum);
    ::serialize_field(joypadenabled, writer, checksum);
    ::serialize_field(joystickprogressive, writer, checksum);
    ::serialize_field(joystickport, writer, checksum);

    ::serialize_field(dirscan, writer, checksum);
    ::serialize_field(buttonscan, writer, checksum);
    ::serialize_field(buttonmouse, writer, checksum);
    ::serialize_field(buttonjoy, writer, checksum);

    ::serialize_field(in_bindings, writer, checksum);

    int16_t dummy_viewsize = static_cast<int16_t>(viewsize);
    ::serialize_field(dummy_viewsize, writer, checksum);

    ::serialize_field(mouseadjustment, writer, checksum);
    ::serialize_field(gamestate.flags, writer, checksum);

    uint8_t temp_u8;

    temp_u8 = static_cast<uint8_t>(::sd_sfx_volume);
    ::serialize_field(temp_u8, writer, checksum);

    temp_u8 = static_cast<uint8_t>(::sd_music_volume);
    ::serialize_field(temp_u8, writer, checksum);

    ::serialize_field(g_no_wall_hit_sound, writer, checksum);
    ::serialize_field(in_use_modern_bindings, writer, checksum);
    ::serialize_field(g_always_run, writer, checksum);

    // AOG options
    ::serialize_field(g_heart_beat_sound, writer, checksum);
    ::serialize_field(g_rotated_automap, writer, checksum);

    writer.write(bstone::Endian::le(checksum.get_value()));
}


bool ShowQuickMsg;

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/
void NewGame(
    int16_t difficulty,
    int16_t episode)
{
    uint16_t oldf = gamestate.flags, loop;

    InitPlaytemp();
    playstate = ex_stillplaying;

    ShowQuickMsg = true;
    ::gamestuff.clear();
    memset(&gamestate, 0, sizeof(gamestate));

    memset(&gamestate.barrier_table, 0xff, sizeof(gamestate.barrier_table));
    memset(&gamestate.old_barrier_table, 0xff, sizeof(gamestate.old_barrier_table));
    gamestate.flags = oldf & ~(GS_KILL_INF_WARN);

    gamestate.difficulty = difficulty;

    gamestate.weapons = 1 << wp_autocharge; // |1<<wp_plasma_detonators;
    gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
    gamestate.old_weapons[0] = gamestate.weapons;
    gamestate.old_weapons[1] = gamestate.weapon;
    gamestate.old_weapons[2] = gamestate.chosenweapon;

    gamestate.health = 100;
    gamestate.old_ammo = gamestate.ammo = STARTAMMO;
    gamestate.lives = 3;
    gamestate.nextextra = EXTRAPOINTS;
    gamestate.episode = episode;
    gamestate.flags |= (GS_CLIP_WALLS | GS_ATTACK_INFOAREA); // |GS_DRAW_CEILING|GS_DRAW_FLOOR);

#if IN_DEVELOPMENT
    if (gamestate.flags & GS_STARTLEVEL) {
        gamestate.mapon = starting_level;
        gamestate.difficulty = starting_difficulty;
        gamestate.episode = starting_episode;
    } else
#endif
    gamestate.mapon = (::is_ps() ? 0 : 1);

    gamestate.key_floor = static_cast<int8_t>(gamestate.mapon + 1);
    startgame = true;

    for (loop = 0; loop < MAPS_WITH_STATS; loop++) {
        gamestuff.old_levelinfo[loop].stats.overall_floor = 100;
        if (loop) {
            gamestuff.old_levelinfo[loop].locked = true;
        }
    }

    ExtraRadarFlags = InstantWin = InstantQuit = 0;

    pickquick = 0;

    // BBi
    g_playtemp.set_size(0);
    g_playtemp.set_position(0);
    // BBi
}

// ===========================================================================


bool LevelInPlaytemp(
    int level_index);

#define LZH_WORK_BUFFER_SIZE (8192)

void* lzh_work_buffer;

void InitPlaytemp()
{
    g_playtemp.open(1 * 1024 * 1024);
    g_playtemp.set_size(0);
    g_playtemp.set_position(0);
}

int FindChunk(
    bstone::IStream* stream,
    const std::string& chunk_name)
{
    char name_buffer[5];
    name_buffer[4] = '\0';
    std::string name;

    while (true) {
        if (stream->read(name_buffer, 4) != 4) {
            break;
        }

        int32_t chunk_size = 0;

        if (stream->read(&chunk_size, 4) != 4) {
            break;
        }

        chunk_size = SDL_SwapLE32(chunk_size);

        name = name_buffer;

        if (name.find(chunk_name) != std::string::npos) {
            return chunk_size;
        }

        stream->skip(chunk_size);
    }

    stream->seek(0, bstone::StreamSeekOrigin::end);
    return 0;
}

int NextChunk(
    bstone::IStream* stream)
{
    bool is_succeed = true;

    if (is_succeed) {
        char name_buffer[4];
        is_succeed = (stream->read(name_buffer, 4) == 4);
    }

    int32_t chunk_size = 0;

    if (is_succeed) {
        is_succeed = (stream->read(&chunk_size, 4) == 4);
    }

    if (is_succeed) {
        return chunk_size;
    }

    stream->seek(0, bstone::StreamSeekOrigin::end);
    return 0;
}

int8_t LS_current = -1;
int8_t LS_total = -1;

bool LoadLevel(
    int level_index)
{
    extern bool ForceLoadDefault;

    bool oldloaded = loadedgame;

    extern int16_t nsd_table[];
    extern int16_t sm_table[];

    WindowY = 181;

    int real_level_index =
        level_index != 0xFF ? level_index : gamestate.mapon;

    gamestuff.level[real_level_index].locked = false;
    int mod = real_level_index % 6;
    normalshade_div = nsd_table[mod];
    shade_max = sm_table[mod];
    normalshade = (3 * (maxscale >> 2)) / normalshade_div;

    std::string chunk_name = "LV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << level_index).to_string();

    g_playtemp.set_position(0);

    if ((::FindChunk(&g_playtemp, chunk_name) == 0) || ForceLoadDefault) {
        ::SetupGameLevel();

        gamestate.flags |= GS_VIRGIN_LEVEL;
        gamestate.turn_around = 0;

        ::PreloadUpdate(1, 1);
        ForceLoadDefault = false;
        return true;
    }

    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Read all sorts of stuff...
    //

    bool is_succeed = true;
    bstone::Crc32 checksum;

    loadedgame = true;
    ::SetupGameLevel();
    loadedgame = oldloaded;

    bstone::BinaryReader reader(&g_playtemp);

    try {
        ::deserialize_field(tilemap, reader, checksum);

        for (int i = 0; i < MAPSIZE; ++i) {
            for (int j = 0; j < MAPSIZE; ++j) {
                int32_t value = 0;
                ::deserialize_field(value, reader, checksum);

                if (value < 0) {
                    actorat[i][j] = &objlist[-value];
                } else {
                    actorat[i][j] = reinterpret_cast<objtype*>(value);
                }
            }
        }

        ::deserialize_field(areaconnect, reader, checksum);
        ::deserialize_field(areabyplayer, reader, checksum);

        // Restore 'save game' actors
        //

        int32_t actor_count = 0;
        ::deserialize_field(actor_count, reader, checksum);

        if (actor_count < 1 || actor_count >= MAXACTORS) {
            throw ArchiveException("actor_count");
        }

        ::InitActorList();

        // First actor is always player
        new_actor->deserialize(reader, checksum);

        for (int32_t i = 1; i < actor_count; ++i) {
            ::GetNewActor();
            new_actor->deserialize(reader, checksum);
            actorat[new_actor->tilex][new_actor->tiley] = new_actor;

#if LOOK_FOR_DEAD_GUYS
            if ((new_actor->flags & FL_DEADGUY) != 0) {
                DeadGuys[NumDeadGuys++] = new_actor;
            }
#endif
        }

        //
        //  Re-Establish links to barrier switches
        //

        for (objtype* actor = objlist; actor; actor = actor->next) {
            switch (actor->obclass) {
            case arc_barrierobj:
            case post_barrierobj:
            case vspike_barrierobj:
            case vpost_barrierobj:
                actor->temp2 = ::ScanBarrierTable(
                    actor->tilex, actor->tiley);
                break;

            default:
                break;
            }
        }

        ::ConnectBarriers();

        // Read all sorts of stuff...
        //

        int32_t laststatobj_index = 0;
        ::deserialize_field(laststatobj_index, reader, checksum);

        if (laststatobj_index < 0) {
            laststatobj = nullptr;
        } else {
            laststatobj = &statobjlist[laststatobj_index];
        }

        for (int i = 0; i < MAXSTATS; ++i) {
            statobjlist[i].deserialize(reader, checksum);
        }

        ::deserialize_field(doorposition, reader, checksum);

        for (int i = 0; i < MAXDOORS; ++i) {
            doorobjlist[i].deserialize(reader, checksum);
        }

        ::deserialize_field(pwallstate, reader, checksum);
        ::deserialize_field(pwallx, reader, checksum);
        ::deserialize_field(pwally, reader, checksum);
        ::deserialize_field(pwalldir, reader, checksum);
        ::deserialize_field(pwallpos, reader, checksum);
        ::deserialize_field(pwalldist, reader, checksum);
        ::deserialize_field(TravelTable, reader, checksum);
        ConHintList.deserialize(reader, checksum);

        for (int i = 0; i < MAXEAWALLS; ++i) {
            eaList[i].deserialize(reader, checksum);
        }

        GoldsternInfo.deserialize(reader, checksum);

        for (int i = 0; i < GOLDIE_MAX_SPAWNS; ++i) {
            GoldieList[i].deserialize(reader, checksum);
        }

        for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
            gamestate.barrier_table[i].deserialize(reader, checksum);
        }

        ::deserialize_field(gamestate.plasma_detonators, reader, checksum);
    } catch (const ArchiveException&) {
        is_succeed = false;
    }

    // Read and evaluate checksum
    //
    if (is_succeed) {
        uint32_t saved_checksum = 0;
        reader.read(saved_checksum);
        bstone::Endian::lei(saved_checksum);

        is_succeed = (saved_checksum == checksum.get_value());
    }

    if (!is_succeed) {
        int16_t old_wx = WindowX;
        int16_t old_wy = WindowY;
        int16_t old_ww = WindowW;
        int16_t old_wh = WindowH;
        int16_t old_px = px;
        int16_t old_py = py;

        WindowX = 0;
        WindowY = 16;
        WindowW = 320;
        WindowH = 168;

        ::CacheMessage(BADINFO_TEXT);

        WindowX = old_wx;
        WindowY = old_wy;
        WindowW = old_ww;
        WindowH = old_wh;

        px = old_px;
        py = old_py;

        ::IN_ClearKeysDown();
        ::IN_Ack();

        gamestate.score = 0;
        gamestate.nextextra = EXTRAPOINTS;
        gamestate.lives = 1;

        gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
        gamestate.weapons = 1 << wp_autocharge;

        gamestate.ammo = 8;
    }

    ::NewViewSize();

    // Check for Strange Door and Actor combos
    //
    if (is_succeed) {
        ::CleanUpDoors_N_Actors();
    }

    return is_succeed;
}

bool SaveLevel(
    int level_index)
{
    WindowY = 181;

    // Make sure floor stats are saved!
    //
    int16_t oldmapon = gamestate.mapon;
    gamestate.mapon = gamestate.lastmapon;
    ::ShowStats(0, 0, ss_justcalc,
                &gamestuff.level[gamestate.mapon].stats);
    gamestate.mapon = oldmapon;

    // Yeah! We're no longer a virgin!
    //
    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Remove level chunk from file
    //
    std::string chunk_name = "LV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << level_index).to_string();

    ::DeleteChunk(g_playtemp, chunk_name);

    g_playtemp.seek(0, bstone::StreamSeekOrigin::end);

    // Write level chunk id
    //
    g_playtemp.write(chunk_name.c_str(), 4);

    // leave four bytes for chunk size
    g_playtemp.skip(4);

    bstone::Crc32 checksum;
    int64_t beg_offset = g_playtemp.get_position();

    bstone::BinaryWriter writer(&g_playtemp);

    ::serialize_field(tilemap, writer, checksum);

    //
    // actorat
    //

    for (int i = 0; i < MAPSIZE; ++i) {
        for (int j = 0; j < MAPSIZE; ++j) {
            int32_t s_value;

            if (actorat[i][j] >= objlist) {
                s_value = -static_cast<int32_t>(
                    actorat[i][j] - objlist);
            } else {
                s_value = static_cast<int32_t>(
                    reinterpret_cast<size_t>(actorat[i][j]));
            }

            ::serialize_field(s_value, writer, checksum);
        }
    }

    ::serialize_field(areaconnect, writer, checksum);
    ::serialize_field(areabyplayer, writer, checksum);

    //
    // objlist
    //

    int32_t actor_count = 0;
    const objtype* actor = nullptr;

    for (actor = player; actor; actor = actor->next) {
        ++actor_count;
    }

    ::serialize_field(actor_count, writer, checksum);

    for (actor = player; actor; actor = actor->next) {
        actor->serialize(writer, checksum);
    }

    //
    // laststatobj
    //

    int32_t laststatobj_index =
        static_cast<int32_t>(laststatobj - statobjlist);

    ::serialize_field(laststatobj_index, writer, checksum);


    //
    // statobjlist
    //
    for (int i = 0; i < MAXSTATS; ++i) {
        statobjlist[i].serialize(writer, checksum);
    }

    //

    ::serialize_field(doorposition, writer, checksum);

    for (int i = 0; i < MAXDOORS; ++i) {
        doorobjlist[i].serialize(writer, checksum);
    }

    ::serialize_field(pwallstate, writer, checksum);
    ::serialize_field(pwallx, writer, checksum);
    ::serialize_field(pwally, writer, checksum);
    ::serialize_field(pwalldir, writer, checksum);
    ::serialize_field(pwallpos, writer, checksum);
    ::serialize_field(pwalldist, writer, checksum);
    ::serialize_field(TravelTable, writer, checksum);
    ConHintList.serialize(writer, checksum);

    for (int i = 0; i < MAXEAWALLS; ++i) {
        eaList[i].serialize(writer, checksum);
    }

    GoldsternInfo.serialize(writer, checksum);

    for (int i = 0; i < GOLDIE_MAX_SPAWNS; ++i) {
        GoldieList[i].serialize(writer, checksum);
    }

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
        gamestate.barrier_table[i].serialize(writer, checksum);
    }

    ::serialize_field(gamestate.plasma_detonators, writer, checksum);

    // Write checksum and determine size of file
    //
    writer.write(bstone::Endian::le(checksum.get_value()));

    int64_t end_offset = g_playtemp.get_position();
    int32_t chunk_size = static_cast<int32_t>(end_offset - beg_offset);

    // Write chunk size, set file size, and close file
    //
    g_playtemp.seek(-(chunk_size + 4), bstone::StreamSeekOrigin::current);
    writer.write(bstone::Endian::le(chunk_size));
    g_playtemp.set_size(end_offset);

    ::NewViewSize();

    return true;
}

int DeleteChunk(
    bstone::MemoryStream& stream,
    const std::string& chunk_name)
{
    stream.set_position(0);

    int chunk_size = ::FindChunk(&stream, chunk_name);

    if (chunk_size > 0) {
        int64_t offset = stream.get_position() - 8;
        int count = chunk_size + 8;

        stream.remove_block(offset, count);
    }

    return chunk_size;
}

static const std::string& get_saved_game_version_string()
{
    static auto version_string = std::string();
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        version_string = "bstone: ";

        switch (::g_game_type) {
        case GameType::aog_sw:
            version_string += "aliens of gold (shareware)";
            break;

        case GameType::aog_full:
            version_string += "aliens of gold (full)";
            break;

        case GameType::ps:
            version_string += "planet strike";
            break;

        default:
            throw std::runtime_error("Invalid game type.");
        }

        version_string +=
            " saved game (v" +
            std::to_string(BS_SAVE_VERSION) +
            ")"
        ;
    }

    return version_string;
}

bool LoadTheGame(
    bstone::IStream* stream)
{
    assert(stream);

    bool is_succeed = true;

    if (is_succeed) {
        is_succeed &= g_playtemp.set_size(0);
        is_succeed &= g_playtemp.set_position(0);
    }

    // Read in VERSion chunk
    //
    if (is_succeed) {
        is_succeed = (::FindChunk(stream, "VERS") != 0);
    }

    if (is_succeed) {
        const auto& version_string = ::get_saved_game_version_string();

        auto version_size = static_cast<int>(version_string.length());
        std::vector<char> version_buffer;
        version_buffer.resize(version_size);
        stream->read(&version_buffer[0], version_size);
        std::string version(&version_buffer[0], version_size);
        is_succeed = (version_string.compare(version) == 0);
    }

    // Read in HEAD chunk
    //
    if (is_succeed) {
        is_succeed = (::FindChunk(stream, "HEAD") != 0);
    }

    bstone::Crc32 checksum;
    bstone::BinaryReader reader(stream);

    if (is_succeed) {
        try {
            gamestate.deserialize(reader, checksum);
            gamestuff.deserialize(reader, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }
    }

    if (is_succeed) {
        uint32_t saved_checksum = 0;
        reader.read(saved_checksum);
        bstone::Endian::lei(saved_checksum);
        is_succeed = (saved_checksum == checksum.get_value());
    }

    if (is_succeed) {
        is_succeed = stream->copy_to(&g_playtemp);
    }

    ::NewViewSize();

    bool show_error_message = true;

    if (is_succeed) {
        // Start music for the starting level in this loaded game.
        //
        ::FreeMusic();
        ::StartMusic(false);

        is_succeed = ::LoadLevel(0xFF);

        // Already shown in LoadLevel
        show_error_message = false;
    }

    if (is_succeed) {
        ShowQuickMsg = false;
    } else {
        g_playtemp.set_size(0);
        g_playtemp.set_position(0);

        if (show_error_message) {
            int16_t old_wx = WindowX;
            int16_t old_wy = WindowY;
            int16_t old_ww = WindowW;
            int16_t old_wh = WindowH;
            int16_t old_px = px;
            int16_t old_py = py;

            WindowX = 0;
            WindowY = 16;
            WindowW = 320;
            WindowH = 168;

            ::CacheMessage(BADSAVEGAME_TEXT);

            ::sd_play_player_sound(NOWAYSND, bstone::AC_NO_WAY);

            WindowX = old_wx;
            WindowY = old_wy;
            WindowW = old_ww;
            WindowH = old_wh;

            px = old_px;
            py = old_py;

            ::IN_ClearKeysDown();
            ::IN_Ack();

            ::VW_FadeOut();
            screenfaded = true;
        }
    }

    return is_succeed;
}

bool SaveTheGame(
    bstone::IStream* stream,
    const std::string& description)
{
    bool is_succeed = true;

    int32_t cksize;

    // Save current level -- saves it into PLAYTEMP.
    //
    ::SaveLevel(0xFF);

    // Write VERSion chunk
    //
    const auto& version_string = ::get_saved_game_version_string();
    cksize = static_cast<int32_t>(version_string.length());
    is_succeed &= stream->write("VERS", 4);
    is_succeed &= stream->write(&cksize, 4);
    is_succeed &= stream->write(version_string.c_str(), cksize);

    // Write DESC chunk
    //
    std::vector<char> desc_buffer(description.begin(), description.end());
    desc_buffer.resize(GAME_DESCRIPTION_LEN + 1);
    cksize = static_cast<int32_t>(desc_buffer.size());
    is_succeed &= stream->write("DESC", 4);
    is_succeed &= stream->write(&cksize, 4);
    is_succeed &= stream->write(&desc_buffer[0], cksize);

    // Write HEAD chunk
    //
    cksize = 0;
    is_succeed &= stream->write("HEAD", 4);

    // leave four bytes for chunk size
    is_succeed &= (stream->skip(4) >= 0);

    bstone::Crc32 checksum;
    bstone::BinaryWriter writer(stream);

    int64_t beg_position = stream->get_position();

    is_succeed &= (beg_position >= 0);

    if (is_succeed) {
        try {
            gamestate.serialize(writer, checksum);
            gamestuff.serialize(writer, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }
    }

    is_succeed &= writer.write(bstone::Endian::le(checksum.get_value()));

    int64_t end_position = stream->get_position();
    is_succeed &= (end_position >= 0);

    if (is_succeed) {
        cksize = static_cast<int32_t>(end_position - beg_position);

        stream->seek(-(cksize + 4), bstone::StreamSeekOrigin::current);
        stream->write(&cksize, 4);
        stream->seek(cksize, bstone::StreamSeekOrigin::current);

        g_playtemp.set_position(0);
        is_succeed = g_playtemp.copy_to(stream);
    }

    ::NewViewSize();

    return is_succeed;
}

bool LevelInPlaytemp(
    int level_index)
{
    char chunk[] = "LVxx";
    ::sprintf(&chunk[2], "%02x", level_index);
    return ::FindChunk(&g_playtemp, chunk) != 0;
}

bool CheckDiskSpace(
    int32_t needed,
    const char* text,
    cds_io_type io_type)
{
    static_cast<void>(needed);
    static_cast<void>(text);
    static_cast<void>(io_type);
    return true;
}

void CleanUpDoors_N_Actors()
{
    int x;
    int y;
    objtype* actor;
    uint8_t tile;
    uint16_t door;

    for (y = 0; y < mapheight; ++y) {
        for (x = 0; x < mapwidth; ++x) {
            tile = tilemap[y][x];

            if ((tile & 0x80) != 0) {
                // Found a door

                actor = actorat[y][x];
                uint16_t actor_u16 = static_cast<uint16_t>(
                    reinterpret_cast<size_t>(actor));

                if (ui16_to_actor(actor_u16)) {
                    // Found an actor

                    door = tile & 0x3F;

                    if ((actor->flags & (FL_SOLID | FL_DEADGUY)) == (FL_SOLID | FL_DEADGUY)) {
                        actor->flags &= ~(FL_SHOOTABLE | FL_SOLID | FL_FAKE_STATIC);
                    }

                    // Make sure door is open

                    doorobjlist[door].ticcount = 0;
                    doorobjlist[door].action = dr_open;
                    doorposition[door] = 0xFFFF;
                }
            }
        }
    }
}

// --------------------------------------------------------------------------
// ClearNClose() - Use when changing levels via standard elevator.
//
// - This code doesn't CLEAR the elevator door as originally
// planned because, actors were coded to stay out of the
// elevator doorway.
//
// --------------------------------------------------------------------------
void ClearNClose()
{
    int8_t x;
    int8_t y;
    int8_t tx = 0;
    int8_t ty = 0;
    auto px = static_cast<int8_t>(player->x >> TILESHIFT);
    auto py = static_cast<int8_t>(player->y >> TILESHIFT);

    // Locate the door.
    //
    for (x = -1; x < 2 && !tx; x += 2) {
        for (y = -1; y < 2; y += 2) {
            if (tilemap[px + x][py + y] & 0x80) {
                tx = px + x;
                ty = py + y;
                break;
            }
        }
    }

    // Close the door!
    //
    if (tx) {
        auto doornum = static_cast<int8_t>(tilemap[static_cast<int>(tx)][static_cast<int>(ty)] & 63);

        doorobjlist[static_cast<int>(doornum)].action = dr_closed; // this door is closed!
        doorposition[static_cast<int>(doornum)] = 0; // draw it closed!

        // make it solid!
        actorat[static_cast<int>(tx)][static_cast<int>(ty)] = reinterpret_cast<objtype*>(doornum | 0x80);
    }
}

void CycleColors()
{
    const int NUM_RANGES = 5;
    const uint8_t CRNG_LOW = 0xF0;
    const uint8_t CRNG_HIGH = 0xFE;
    const int CRNG_SIZE = CRNG_HIGH - CRNG_LOW + 1;

    static CycleInfo crng[NUM_RANGES] = {
        { 7, 0, 0xF0, 0xF1 },
        { 15, 0, 0xF2, 0xF3 },
        { 30, 0, 0xF4, 0xF5 },
        { 10, 0, 0xF6, 0xF9 },
        { 12, 0, 0xFA, 0xFE },
    };

    uint8_t loop;
    uint8_t cbuffer[CRNG_SIZE][3];
    bool changes = false;

    for (loop = 0; loop < NUM_RANGES; loop++) {
        CycleInfo* c = &crng[loop];

        if (tics >= c->delay_count) {
            uint8_t temp[3], first, last, numregs;

            if (!changes) {
                VL_GetPalette(CRNG_LOW, CRNG_SIZE, (uint8_t*)cbuffer);
                changes = true;
            }

            first = c->firstreg - CRNG_LOW;
            numregs = c->lastreg - c->firstreg;                 // is one less than in range
            last = first + numregs;

            memcpy(temp, cbuffer[last], 3);
            memmove(cbuffer[first + 1], cbuffer[first], numregs * 3);
            memcpy(cbuffer[first], temp, 3);

            c->delay_count = c->init_delay;
        } else {
            c->delay_count -= static_cast<uint8_t>(tics);
        }
    }

    if (changes) {
        VL_SetPalette(CRNG_LOW, CRNG_SIZE, (uint8_t*)cbuffer);
    } else {
        VW_WaitVBL(1);
    }
}

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/
void ShutdownId()
{
    US_Shutdown();
    SD_Shutdown();
    PM_Shutdown();
    IN_Shutdown();
    VW_Shutdown();
    CA_Shutdown();
}

/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/
void CalcProjection(
    int32_t focal)
{
    focallength = focal;
    double facedist = focal + MINDIST;
    int halfview = (viewwidth * vga_scale) / 2; // half view in pixels

    //
    // calculate scale value for vertical height calculations
    // and sprite x calculations
    //
    scale = static_cast<fixed>(halfview * facedist / (VIEWGLOBAL / 2));

    //
    // divide heightnumerator by a posts distance to get the posts height for
    // the heightbuffer.  The pixel height is height>>2
    //
    heightnumerator = (TILEGLOBAL * scale) >> 6;
    minheightdiv = static_cast<int16_t>((heightnumerator / 0x7FFF) + 1);

    //
    // calculate the angle offset from view angle of each pixel's ray
    //

    delete [] pixelangle;
    pixelangle = new int[vga_width];

    for (int i = 0; i < halfview; i++) {
        // start 1/2 pixel over, so viewangle bisects two middle pixels
        double tang = i * VIEWGLOBAL / (viewwidth * vga_scale) / facedist;
        float angle = static_cast<float>(atan(tang));
        int intang = static_cast<int>(angle * radtoint);
        pixelangle[halfview - 1 - i] = intang;
        pixelangle[halfview + i] = -intang;
    }

    //
    // if a point's abs(y/x) is greater than maxslope, the point is outside
    // the view area
    //
    maxslope = finetangent[pixelangle[0]];
    maxslope >>= 8;
}

bool DoMovie(
    movie_t movie,
    void* palette)
{
    bool ReturnVal;
    SD_StopSound();

    ClearMemory();
    UnCacheLump(STARTFONT, STARTFONT + NUMFONT);
    CA_LoadAllSounds();

    if (palette) {
        Movies[movie].palette = palette;
    } else {
        Movies[movie].palette = vgapal;
    }

    ReturnVal = MOVIE_Play(&Movies[movie]);

    SD_StopSound();
    ClearMemory();
    LoadFonts();

    return ReturnVal;
}

void LoadFonts()
{
    CA_CacheGrChunk(STARTFONT + 4);
    CA_CacheGrChunk(STARTFONT + 2);
}

void SetViewSize(
    int width,
    int height)
{
    viewwidth = width & ~15; // must be divisable by 16
    viewheight = height & ~1; // must be even
    centerx = (viewwidth / 2) - 1;
    shootdelta = viewwidth / 10;

    screenofs = ((200 - STATUSLINES - viewheight + TOP_STRIP_HEIGHT) /
                 2 * SCREENWIDTH) + ((320 - viewwidth) / 8);

    //
    // calculate trace angles and projection constants
    //
    CalcProjection(FOCALLENGTH);

    //
    // build all needed compiled scalers
    //
    SetupScaling((3 * viewwidth) / 2);

    view_xl = 0;
    view_xh = static_cast<int16_t>(view_xl + viewwidth - 1);
    view_yl = 0;
    view_yh = static_cast<int16_t>(view_yl + viewheight - 1);
}

void NewViewSize()
{
    CA_UpLevel();
    SetViewSize(viewsize * 16, static_cast<int>(viewsize * 16 * HEIGHTRATIO));
    CA_DownLevel();
}

void pre_quit()
{
    if (::is_config_loaded) {
        ::WriteConfig();
    }

    ::write_high_scores();

    ::ShutdownId();
}

void Quit()
{
    ::pre_quit();

    ::exit(1);
}

void DemoLoop()
{
    bool breakit;
    uint16_t old_bufferofs;

    while (true) {
        playstate = ex_title;
        if (!screenfaded) {
            VW_FadeOut();
        }
        VL_SetPaletteIntensity(0, 255, vgapal, 0);

        if (!::no_screens) {
            while (!(gamestate.flags & GS_NOWAIT)) {
                extern bool sqActive;

                // Start music when coming from menu...
                //
                if (!sqActive) {
                    // Load and start music
                    //
                    if (::is_aog()) {
                        CA_CacheAudioChunk(STARTMUSIC + MEETINGA_MUS);
                        ::SD_StartMusic(MEETINGA_MUS);
                    } else {
                        CA_CacheAudioChunk(STARTMUSIC + TITLE_LOOP_MUSIC);
                        ::SD_StartMusic(TITLE_LOOP_MUSIC);
                    }
                }


                //
                // title page
                //
                breakit = false;

                if (::is_aog()) {
                    ::CA_CacheScreen(TITLEPIC);
                } else {
                    ::CA_CacheScreen(TITLE1PIC);
                }

                ::CA_CacheGrChunk(TITLEPALETTE);
                old_bufferofs = static_cast<uint16_t>(::bufferofs);
                ::bufferofs = displayofs;
                VW_Bar(0, 0, ::vga_ref_width, ::vga_ref_height, 0);
                ::bufferofs = old_bufferofs;

                ::VL_SetPalette(
                    0,
                    256,
                    reinterpret_cast<const uint8_t*>(::grsegs[TITLEPALETTE]));

                ::VL_SetPaletteIntensity(
                    0,
                    255,
                    reinterpret_cast<const uint8_t*>(::grsegs[TITLEPALETTE]),
                    0);

                auto version_text_width = 0;
                auto version_text_height = 0;
                const auto version_padding = 1;
                const auto version_margin = 4;
                const auto ps_fizzle_height = 15;
                auto&& version_string = ::get_version_string();

                ::fontnumber = 2;

                ::USL_MeasureString(
                    version_string.c_str(),
                    &version_text_width,
                    &version_text_height);

                const auto version_bar_width =
                    version_text_width + (2 * version_padding);

                const auto version_bar_height =
                    version_text_height + (2 * version_padding);

                const auto version_bar_x =
                    ::vga_ref_width - (version_margin + version_bar_width);

                const auto version_bar_y = (
                    ::is_aog() ?
                        version_margin :
                        ::vga_ref_height -
                            (version_bar_height + ps_fizzle_height));

                ::WindowX = static_cast<uint16_t>(version_bar_x);
                ::WindowY = static_cast<uint16_t>(version_bar_y);
                ::PrintX = ::WindowX + version_padding;
                ::PrintY = ::WindowY + version_padding;
                ::WindowW = static_cast<uint16_t>(version_bar_width);
                ::WindowH = static_cast<uint16_t>(version_bar_height);

                ::VWB_Bar(
                    ::WindowX,
                    ::WindowY,
                    ::WindowW,
                    ::WindowH,
                    VERSION_TEXT_BKCOLOR);

                SETFONTCOLOR(VERSION_TEXT_COLOR, VERSION_TEXT_BKCOLOR);
                ::US_Print(::get_version_string().c_str());

                VW_UpdateScreen();
                ::VL_FadeIn(0, 255, reinterpret_cast<uint8_t*>(::grsegs[TITLEPALETTE]), 30);
                ::UNCACHEGRCHUNK(TITLEPALETTE);

                if (::is_ps()) {
                    // Cache screen 2 with Warnings and Copyrights

                    ::CA_CacheScreen(TITLE2PIC);
                    ::fontnumber = 2;
                    ::PrintX = ::WindowX + version_padding;
                    ::PrintY = ::WindowY + version_padding;

                    ::VWB_Bar(
                        ::WindowX,
                        ::WindowY,
                        ::WindowW,
                        ::WindowH,
                        VERSION_TEXT_BKCOLOR);

                    SETFONTCOLOR(VERSION_TEXT_COLOR, VERSION_TEXT_BKCOLOR);
                    ::US_Print(::get_version_string().c_str());

                    // Fizzle whole screen incase of any last minute changes needed
                    // on title intro.

                    // BBi Made abortable.
                    breakit |= ::FizzleFade(
                        ::bufferofs,
                        ::displayofs,
                        ::vga_ref_width,
                        ::vga_ref_height,
                        70,
                        true);
                }

                if (breakit || ::IN_UserInput(TickBase * 6)) {
                    break;
                }

                VW_FadeOut();

                //
                // credits page
                //
                DrawCreditsPage();
                VW_UpdateScreen();
                VW_FadeIn();
                if (IN_UserInput(TickBase * 6)) {
                    break;
                }
                VW_FadeOut();


            //
            // high scores
            //
            CA_CacheScreen(BACKGROUND_SCREENPIC);
            DrawHighScores();
            VW_UpdateScreen();
            VW_FadeIn();

            if (IN_UserInput(TickBase * 9)) {
                break;
            }
            VW_FadeOut();
        }
    } else {
        // Start music when coming from menu...
        if (!sqActive) {
            // Load and start music
            //
            if (!::is_aog()) {
                CA_CacheAudioChunk(STARTMUSIC + MENUSONG);
                ::SD_StartMusic(MENUSONG);
            } else {
                CA_CacheAudioChunk(STARTMUSIC + TITLE_LOOP_MUSIC);
                ::SD_StartMusic(TITLE_LOOP_MUSIC);
            }
        }
    }

        if (!screenfaded) {
            VW_FadeOut();
        }

        while (true) {
#if IN_DEVELOPMENT
            if (gamestate.flags & GS_QUICKRUN) {
                ReadGameNames();
                CA_LoadAllSounds();
                NewGame(2, gamestate.episode);
                startgame = true;
            } else
#endif
            US_ControlPanel(ScanCode::sc_none);

            if (startgame || loadedgame) {
                GameLoop();
            }
        }
    }
}

void DrawCreditsPage()
{
    PresenterInfo pi;

    CA_CacheScreen(BACKGROUND_SCREENPIC);

    memset(&pi, 0, sizeof(pi));
    pi.flags = TPF_CACHE_NO_GFX;
    pi.xl = 38;
    pi.yl = 28;
    pi.xh = 281;
    pi.yh = 170;
    pi.bgcolor = 2;
    pi.ltcolor = BORDER_HI_COLOR;
    fontcolor = BORDER_TEXT_COLOR;
    pi.shcolor = pi.dkcolor = 0;
    pi.fontnumber = static_cast<int8_t>(fontnumber);

#ifdef ID_CACHE_CREDITS
    TP_LoadScript(nullptr, &pi, CREDITSTEXT);
#else
    TP_LoadScript("CREDITS.TXT", &pi, 0);
#endif

    TP_Presenter(&pi);
}


#if IN_DEVELOPMENT
int16_t starting_episode = 0;
int16_t starting_level = 0;
int16_t starting_difficulty = 2;
#endif
int16_t debug_value = 0;

int main(
    int argc,
    char* argv[])
{
    int sdl_result = 0;

    sdl_result = ::SDL_Init(0);

    if (sdl_result != 0) {
        ::Quit(::SDL_GetError());
    }

    ::g_args.initialize(argc, argv);

#if IN_DEVELOPMENT
    MakeDestPath(ERROR_LOG);
    remove(tempPath);
#endif

    freed_main();

    DemoLoop();

    Quit();

    return 0;
}

void fprint(
    char* text)
{
    while (*text) {
        printf("%c", *text++);
    }
}

void InitDestPath()
{
    auto default_data_dir = ::get_default_data_dir();
    auto requested_data_dir = ::g_args.get_option_value("datadir");

    if (requested_data_dir.empty()) {
        data_dir = ::get_default_data_dir();
    } else {
        const auto separator =
#ifdef _WIN32
            '\\'
#else
            '/'
#endif
        ;

        data_dir = requested_data_dir;

        if (data_dir.back() != separator) {
            data_dir += separator;
        }
    }
}

#if IN_DEVELOPMENT
// -------------------------------------------------------------------------
// ShowMemory()
// -------------------------------------------------------------------------
void ShowMemory()
{
    int32_t psize, size;

    size = MM_TotalFree();
    psize = MM_LargestAvail();
    mprintf("Mem free: %ld   %ld\n", size, psize);
}
#endif

// BBi
void objtype::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);
    ::serialize_field(active, writer, checksum);
    ::serialize_field(ticcount, writer, checksum);
    ::serialize_field(obclass, writer, checksum);

    int32_t state_index = static_cast<int32_t>(::get_state_index(state));
    ::serialize_field(state_index, writer, checksum);

    ::serialize_field(flags, writer, checksum);
    ::serialize_field(flags2, writer, checksum);
    ::serialize_field(distance, writer, checksum);
    ::serialize_field(dir, writer, checksum);
    ::serialize_field(trydir, writer, checksum);
    ::serialize_field(x, writer, checksum);
    ::serialize_field(y, writer, checksum);
    ::serialize_field(s_tilex, writer, checksum);
    ::serialize_field(s_tiley, writer, checksum);
    ::serialize_field(viewx, writer, checksum);
    ::serialize_field(viewheight, writer, checksum);
    ::serialize_field(transx, writer, checksum);
    ::serialize_field(transy, writer, checksum);
    ::serialize_field(hitpoints, writer, checksum);
    ::serialize_field(ammo, writer, checksum);
    ::serialize_field(lighting, writer, checksum);
    ::serialize_field(linc, writer, checksum);
    ::serialize_field(angle, writer, checksum);
    ::serialize_field(speed, writer, checksum);
    ::serialize_field(temp1, writer, checksum);
    ::serialize_field(temp2, writer, checksum);
    ::serialize_field(temp3, writer, checksum);
}

void objtype::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);
    ::deserialize_field(active, reader, checksum);
    ::deserialize_field(ticcount, reader, checksum);
    ::deserialize_field(obclass, reader, checksum);

    int32_t state_index = 0;
    ::deserialize_field(state_index, reader, checksum);
    state = states_list[state_index];

    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(flags2, reader, checksum);
    ::deserialize_field(distance, reader, checksum);
    ::deserialize_field(dir, reader, checksum);
    ::deserialize_field(trydir, reader, checksum);
    ::deserialize_field(x, reader, checksum);
    ::deserialize_field(y, reader, checksum);
    ::deserialize_field(s_tilex, reader, checksum);
    ::deserialize_field(s_tiley, reader, checksum);
    ::deserialize_field(viewx, reader, checksum);
    ::deserialize_field(viewheight, reader, checksum);
    ::deserialize_field(transx, reader, checksum);
    ::deserialize_field(transy, reader, checksum);
    ::deserialize_field(hitpoints, reader, checksum);
    ::deserialize_field(ammo, reader, checksum);
    ::deserialize_field(lighting, reader, checksum);
    ::deserialize_field(linc, reader, checksum);
    ::deserialize_field(angle, reader, checksum);
    ::deserialize_field(speed, reader, checksum);
    ::deserialize_field(temp1, reader, checksum);
    ::deserialize_field(temp2, reader, checksum);
    ::deserialize_field(temp3, reader, checksum);
}

void statobj_t::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);

    int32_t vis_index = static_cast<int32_t>(visspot - &spotvis[0][0]);
    ::serialize_field(vis_index, writer, checksum);

    ::serialize_field(shapenum, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(itemnumber, writer, checksum);
    ::serialize_field(lighting, writer, checksum);
}

void statobj_t::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);

    int32_t vis_index = 0;
    ::deserialize_field(vis_index, reader, checksum);

    if (vis_index < 0) {
        visspot = nullptr;
    } else {
        visspot = &(&spotvis[0][0])[vis_index];
    }

    ::deserialize_field(shapenum, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(itemnumber, reader, checksum);
    ::deserialize_field(lighting, reader, checksum);
}

void doorobj_t::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(vertical, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(lock, writer, checksum);
    ::serialize_field(type, writer, checksum);
    ::serialize_field(action, writer, checksum);
    ::serialize_field(ticcount, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);
}

void doorobj_t::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(vertical, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(lock, reader, checksum);
    ::deserialize_field(type, reader, checksum);
    ::deserialize_field(action, reader, checksum);
    ::deserialize_field(ticcount, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);
}

void mCacheInfo::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(local_val, writer, checksum);
    ::serialize_field(global_val, writer, checksum);
}

void mCacheInfo::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(local_val, reader, checksum);
    ::deserialize_field(global_val, reader, checksum);
    mSeg = nullptr;
}

void con_mCacheInfo::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    mInfo.serialize(writer, checksum);
    ::serialize_field(type, writer, checksum);
    ::serialize_field(operate_cnt, writer, checksum);
}

void con_mCacheInfo::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    mInfo.deserialize(reader, checksum);
    ::deserialize_field(type, reader, checksum);
    ::deserialize_field(operate_cnt, reader, checksum);
}

void concession_t::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(NumMsgs, writer, checksum);

    for (int i = 0; i < NumMsgs; ++i) {
        cmInfo[i].serialize(writer, checksum);
    }
}

void concession_t::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(NumMsgs, reader, checksum);

    for (int i = 0; i < NumMsgs; ++i) {
        cmInfo[i].deserialize(reader, checksum);
    }
}

void eaWallInfo::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(aliens_out, writer, checksum);
    ::serialize_field(delay, writer, checksum);
}

void eaWallInfo::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(aliens_out, reader, checksum);
    ::deserialize_field(delay, reader, checksum);
}

void GoldsternInfo_t::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(LastIndex, writer, checksum);
    ::serialize_field(SpawnCnt, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(WaitTime, writer, checksum);
    ::serialize_field(GoldSpawned, writer, checksum);
}

void GoldsternInfo_t::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(LastIndex, reader, checksum);
    ::deserialize_field(SpawnCnt, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(WaitTime, reader, checksum);
    ::deserialize_field(GoldSpawned, reader, checksum);
}

void tilecoord_t::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
}

void tilecoord_t::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
}

void barrier_type::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(level, writer, checksum);
    coord.serialize(writer, checksum);
    ::serialize_field(on, writer, checksum);
}

void barrier_type::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(level, reader, checksum);
    coord.deserialize(reader, checksum);
    ::deserialize_field(on, reader, checksum);
}

void statsInfoType::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(total_points, writer, checksum);
    ::serialize_field(accum_points, writer, checksum);
    ::serialize_field(total_enemy, writer, checksum);
    ::serialize_field(accum_enemy, writer, checksum);
    ::serialize_field(total_inf, writer, checksum);
    ::serialize_field(accum_inf, writer, checksum);
    ::serialize_field(overall_floor, writer, checksum);
}

void statsInfoType::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(total_points, reader, checksum);
    ::deserialize_field(accum_points, reader, checksum);
    ::deserialize_field(total_enemy, reader, checksum);
    ::deserialize_field(accum_enemy, reader, checksum);
    ::deserialize_field(total_inf, reader, checksum);
    ::deserialize_field(accum_inf, reader, checksum);
    ::deserialize_field(overall_floor, reader, checksum);
}

void levelinfo::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(bonus_queue, writer, checksum);
    ::serialize_field(bonus_shown, writer, checksum);
    ::serialize_field(locked, writer, checksum);
    stats.serialize(writer, checksum);
    ::serialize_field(ptilex, writer, checksum);
    ::serialize_field(ptiley, writer, checksum);
    ::serialize_field(pangle, writer, checksum);
}

void levelinfo::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(bonus_queue, reader, checksum);
    ::deserialize_field(bonus_shown, reader, checksum);
    ::deserialize_field(locked, reader, checksum);
    stats.deserialize(reader, checksum);
    ::deserialize_field(ptilex, reader, checksum);
    ::deserialize_field(ptiley, reader, checksum);
    ::deserialize_field(pangle, reader, checksum);
}

fargametype::fargametype() :
    old_levelinfo(),
    level()
{
}

void fargametype::initialize()
{
    old_levelinfo.resize(MAPS_PER_EPISODE);
    level.resize(MAPS_PER_EPISODE);
}

void fargametype::clear()
{
    old_levelinfo.clear();
    level.clear();

    initialize();
}

void fargametype::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    for (int i = 0; i < MAPS_PER_EPISODE; ++i) {
        old_levelinfo[i].serialize(writer, checksum);
    }

    for (int i = 0; i < MAPS_PER_EPISODE; ++i) {
        level[i].serialize(writer, checksum);
    }
}

void fargametype::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    for (int i = 0; i < MAPS_PER_EPISODE; ++i) {
        old_levelinfo[i].deserialize(reader, checksum);
    }

    for (int i = 0; i < MAPS_PER_EPISODE; ++i) {
        level[i].deserialize(reader, checksum);
    }
}

void gametype::serialize(
    bstone::BinaryWriter& writer,
    bstone::Crc32& checksum) const
{
    ::serialize_field(turn_around, writer, checksum);
    ::serialize_field(turn_angle, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(lastmapon, writer, checksum);
    ::serialize_field(difficulty, writer, checksum);
    ::serialize_field(mapon, writer, checksum);
    ::serialize_field(status_refresh, writer, checksum);
    ::serialize_field(oldscore, writer, checksum);
    ::serialize_field(tic_score, writer, checksum);
    ::serialize_field(score, writer, checksum);
    ::serialize_field(nextextra, writer, checksum);
    ::serialize_field(score_roll_wait, writer, checksum);
    ::serialize_field(lives, writer, checksum);
    ::serialize_field(health, writer, checksum);
    ::serialize_field(health_delay, writer, checksum);
    ::serialize_field(health_str, writer, checksum);
    ::serialize_field(rpower, writer, checksum);
    ::serialize_field(old_rpower, writer, checksum);
    ::serialize_field(rzoom, writer, checksum);
    ::serialize_field(radar_leds, writer, checksum);
    ::serialize_field(lastradar_leds, writer, checksum);
    ::serialize_field(lastammo_leds, writer, checksum);
    ::serialize_field(ammo_leds, writer, checksum);
    ::serialize_field(ammo, writer, checksum);
    ::serialize_field(old_ammo, writer, checksum);
    ::serialize_field(plasma_detonators, writer, checksum);
    ::serialize_field(old_plasma_detonators, writer, checksum);
    ::serialize_field(useable_weapons, writer, checksum);
    ::serialize_field(weapons, writer, checksum);
    ::serialize_field(weapon, writer, checksum);
    ::serialize_field(chosenweapon, writer, checksum);
    ::serialize_field(old_weapons, writer, checksum);
    ::serialize_field(key_floor, writer, checksum);
    ::serialize_field(weapon_wait, writer, checksum);
    ::serialize_field(attackframe, writer, checksum);
    ::serialize_field(attackcount, writer, checksum);
    ::serialize_field(weaponframe, writer, checksum);
    ::serialize_field(episode, writer, checksum);

    auto time_count = TimeCount;
    ::serialize_field(time_count, writer, checksum);

    ::serialize_field(killx, writer, checksum);
    ::serialize_field(killy, writer, checksum);
    // Skip "msg"
    ::serialize_field(numkeys, writer, checksum);
    ::serialize_field(old_numkeys, writer, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
        barrier_table[i].serialize(writer, checksum);
    }

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
        old_barrier_table[i].serialize(writer, checksum);
    }

    ::serialize_field(tokens, writer, checksum);
    ::serialize_field(old_tokens, writer, checksum);
    ::serialize_field(boss_key_dropped, writer, checksum);
    ::serialize_field(old_boss_key_dropped, writer, checksum);
    ::serialize_field(wintilex, writer, checksum);
    ::serialize_field(wintiley, writer, checksum);
}

void gametype::deserialize(
    bstone::BinaryReader& reader,
    bstone::Crc32& checksum)
{
    ::deserialize_field(turn_around, reader, checksum);
    ::deserialize_field(turn_angle, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(lastmapon, reader, checksum);
    ::deserialize_field(difficulty, reader, checksum);
    ::deserialize_field(mapon, reader, checksum);
    ::deserialize_field(status_refresh, reader, checksum);
    ::deserialize_field(oldscore, reader, checksum);
    ::deserialize_field(tic_score, reader, checksum);
    ::deserialize_field(score, reader, checksum);
    ::deserialize_field(nextextra, reader, checksum);
    ::deserialize_field(score_roll_wait, reader, checksum);
    ::deserialize_field(lives, reader, checksum);
    ::deserialize_field(health, reader, checksum);
    ::deserialize_field(health_delay, reader, checksum);
    ::deserialize_field(health_str, reader, checksum);
    ::deserialize_field(rpower, reader, checksum);
    ::deserialize_field(old_rpower, reader, checksum);
    ::deserialize_field(rzoom, reader, checksum);
    ::deserialize_field(radar_leds, reader, checksum);
    ::deserialize_field(lastradar_leds, reader, checksum);
    ::deserialize_field(lastammo_leds, reader, checksum);
    ::deserialize_field(ammo_leds, reader, checksum);
    ::deserialize_field(ammo, reader, checksum);
    ::deserialize_field(old_ammo, reader, checksum);
    ::deserialize_field(plasma_detonators, reader, checksum);
    ::deserialize_field(old_plasma_detonators, reader, checksum);
    ::deserialize_field(useable_weapons, reader, checksum);
    ::deserialize_field(weapons, reader, checksum);
    ::deserialize_field(weapon, reader, checksum);
    ::deserialize_field(chosenweapon, reader, checksum);
    ::deserialize_field(old_weapons, reader, checksum);
    ::deserialize_field(key_floor, reader, checksum);
    ::deserialize_field(weapon_wait, reader, checksum);
    ::deserialize_field(attackframe, reader, checksum);
    ::deserialize_field(attackcount, reader, checksum);
    ::deserialize_field(weaponframe, reader, checksum);
    ::deserialize_field(episode, reader, checksum);

    uint32_t time_count = 0;
    ::deserialize_field(time_count, reader, checksum);

    ::deserialize_field(killx, reader, checksum);
    ::deserialize_field(killy, reader, checksum);
    msg = nullptr;
    ::deserialize_field(numkeys, reader, checksum);
    ::deserialize_field(old_numkeys, reader, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
        barrier_table[i].deserialize(reader, checksum);
    }

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i) {
        old_barrier_table[i].deserialize(reader, checksum);
    }

    ::deserialize_field(tokens, reader, checksum);
    ::deserialize_field(old_tokens, reader, checksum);
    ::deserialize_field(boss_key_dropped, reader, checksum);
    ::deserialize_field(old_boss_key_dropped, reader, checksum);
    ::deserialize_field(wintilex, reader, checksum);
    ::deserialize_field(wintiley, reader, checksum);

    TimeCount = time_count;
}

bool is_aog_full()
{
    return ::g_game_type == GameType::aog_full;
}

bool is_aog_sw()
{
    return ::g_game_type == GameType::aog_sw;
}

bool is_aog()
{
    return ::is_aog_sw() || ::is_aog_full();
}

bool is_ps()
{
    return ::g_game_type == GameType::ps;
}

void sys_sleep_for(
    int milliseconds)
{
    std::this_thread::sleep_for(
        std::chrono::milliseconds(milliseconds));
}

void sys_default_sleep_for()
{
    ::sys_sleep_for(10);
}

const std::string& get_version_string()
{
    static const std::string version = "1.0.0";
    return version;
}

const std::string& get_profile_dir()
{
    static std::string profile_dir;
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        auto sdl_dir = ::SDL_GetPrefPath("bibendovsky", "bstone");

        if (sdl_dir) {
            profile_dir = sdl_dir;
            ::SDL_free(sdl_dir);
        }
    }

    return profile_dir;
}

const std::string& get_default_data_dir()
{
    static std::string data_dir;
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        auto sdl_dir = ::SDL_GetBasePath();

        if (sdl_dir) {
            data_dir = sdl_dir;
            ::SDL_free(sdl_dir);
        }
    }

    return data_dir;
}
// BBi
