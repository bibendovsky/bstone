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


#ifndef AUDIO_INCLUDED
#define AUDIO_INCLUDED


//
// Base offsets
//
const int STARTPCSOUNDS = 0;
const int STARTADLIBSOUNDS = 100;
const int STARTDIGISOUNDS = 200;
const int STARTMUSIC = 300;

const int NUMSOUNDS = 100;

extern int NUMSNDCHUNKS;


//
// Sound names & indexes
//
const int HITWALLSND = 0;
const int TERM_TYPESND = 1;
const int GETPISTOLSND = 2;
const int LIQUIDDIESND = 3;
const int MOVEGUN2SND = 4;
const int MOVEGUN1SND = 5;
const int NOWAYSND = 6;
const int SCOUT_ALERTSND = 7;
const int GURNEYSND = 8;
const int PLAYERDEATHSND = 9;
const int CONCESSIONSSND = 10;
const int ATKIONCANNONSND = 11;
const int GETKEYSND = 12;
const int WARPOUTSND = 13;
const int WARPINSND = 14;
const int ROBOT_SERVOSND = 15;
const int INFORMANTDEATHSND = 16;
const int GOLDSTERNHALTSND = 17;
const int OPENDOORSND = 18;
const int CLOSEDOORSND = 19;
const int GETDETONATORSND = 20;
const int HALTSND = 21;
const int RENTDEATH2SND = 22;
const int ATKAUTOCHARGESND = 23;
const int ATKCHARGEDSND = 24;
const int RADAR_POWERUPSND = 25;
const int ATKBURSTRIFLESND = 26;
const int VITAL_GONESND = 27;
const int SHOOTDOORSND = 28;
const int RENTDEATH1SND = 29;
const int GETBURSTRIFLESND = 30;
const int GETAMMOSND = 31;
const int SHOOTSND = 32;
const int HEALTH1SND = 33;
const int HEALTH2SND = 34;
const int BONUS1SND = 35;
const int BONUS2SND = 36;
const int BONUS3SND = 37;
const int GETIONCANNONSND = 38;
const int ESCPRESSEDSND = 39;
const int ELECAPPEARSND = 40;
const int EXTRA_MANSND = 41;
const int ELEV_BUTTONSND = 42;
const int INTERROGATESND = 43;
const int BONUS5SND = 44;
const int BONUS4SND = 45;
const int PUSHWALLSND = 46;
const int TERM_BEEPSND = 47;
const int ROLL_SCORESND = 48;
const int TURRETSND = 49;
const int EXPLODE1SND = 50;
const int __UNUSED_51__SND = 51;
const int SWATDIESND = 52;
const int GGUARDHALTSND = 53;
const int EXPLODE2SND = 54;
const int BLUEBOYHALTSND = 55;
const int PROGUARDDEATHSND = 56;
const int DOGBOYHALTSND = 57;
const int ENGINE_THRUSTSND = 58;
const int SCANHALTSND = 59;
const int GETCANNONSND = 60;
const int LCANHALTSND = 61;
const int PROHALTSND = 62;
const int GGUARDDEATHSND = 63;
const int BLUEBOYDEATHSND = 64;
const int GOLDSTERNLAUGHSND = 65;
const int SCIENTISTHALTSND = 66;
const int SCIENTISTDEATHSND = 67;
const int DOGBOYDEATHSND = 68;
const int H_BEATSND = 69;
const int SWATHALTSND = 70;
const int SCANDEATHSND = 71;
const int LCANDEATHSND = 72;
const int INFORMDEATH2SND = 73;
const int INFORMDEATH3SND = 74;
const int GURNEYDEATHSND = 75;
const int PRODEATH2SND = 76;
const int PRODEATH3SND = 77; // AOG
const int SWATDEATH2SND = 78;
const int LCANBREAKSND = 79;
const int SCANBREAKSND = 80;
const int HTECHDOOROPENSND = 81;
const int HTECHDOORCLOSESND = 82;
const int ELECARCDAMAGESND = 83;
const int PODHATCHSND = 84;
const int ELECTSHOTSND = 85;
const int ELECDIESND = 86;
const int ATKGRENADESND = 87;
const int CLAWATTACKSND = 88;
const int PUNCHATTACKSND = 89;
const int SPITATTACKSND = 90;
const int PODDEATHSND = 91;
const int PODHALTSND = 92;
const int SWATDEATH3SND = 93; // AOG
const int SCIDEATH2SND = 94;
const int SCIDEATH3SND = 95;
const int GOTTOKENSND = 96;
const int SWITCHSND = 97;
const int STATS1SND = 98;
const int STATS2SND = 99;
const int LASTSOUND = 100;


//
// Music names & indexes (AOG)
//
extern int S2100A_MUS;
extern int GOLDA_MUS;
extern int APOGFNFM_MUS;
extern int DRKHALLA_MUS;
extern int FREEDOMA_MUS;
extern int GENEFUNK_MUS;
extern int TIMEA_MUS;
extern int HIDINGA_MUS;
extern int INCNRATN_MUS;
extern int JUNGLEA_MUS;
extern int LEVELA_MUS;
extern int MEETINGA_MUS;
extern int STRUTA_MUS;
extern int RACSHUFL_MUS;
extern int RUMBAA_MUS;
extern int SEARCHNA_MUS;
extern int THEWAYA_MUS;
extern int INTRIGEA_MUS;

//
// Music names & indexes (PS)
//
extern int CATACOMB_MUS;
extern int STICKS_MUS;
extern int PLOT_MUS;
extern int CIRCLES_MUS;
extern int LASTLAFF_MUS;
extern int TOHELL_MUS;
extern int FORTRESS_MUS;
extern int GIVING_MUS;
extern int HARTBEAT_MUS;
extern int LURKING_MUS;
extern int MAJMIN_MUS;
extern int VACCINAP_MUS;
extern int DARKNESS_MUS;
extern int MONASTRY_MUS;
extern int TOMBP_MUS;
extern int TIME_MUS;
extern int MOURNING_MUS;
extern int SERPENT_MUS;
extern int HISCORE_MUS;


//
// Music names & indexes (shared)
//
extern int APOGFNFM_MUS;
extern int THEME_MUS;
extern int LASTMUSIC;


#endif // AUDIO_INCLUDED
