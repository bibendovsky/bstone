/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_INCLUDED
#define BSTONE_AUDIO_INCLUDED

inline constexpr int NUMSOUNDS = 100;

// Sound names & indexes

inline constexpr int HITWALLSND = 0;
inline constexpr int TERM_TYPESND = 1;
inline constexpr int GETPISTOLSND = 2;
inline constexpr int LIQUIDDIESND = 3;
inline constexpr int MOVEGUN2SND = 4;
inline constexpr int MOVEGUN1SND = 5;
inline constexpr int NOWAYSND = 6;
inline constexpr int SCOUT_ALERTSND = 7;
inline constexpr int GURNEYSND = 8;
inline constexpr int PLAYERDEATHSND = 9;
inline constexpr int CONCESSIONSSND = 10;
inline constexpr int ATKIONCANNONSND = 11;
inline constexpr int GETKEYSND = 12;
inline constexpr int WARPOUTSND = 13;
inline constexpr int WARPINSND = 14;
inline constexpr int ROBOT_SERVOSND = 15;
inline constexpr int INFORMANTDEATHSND = 16;
inline constexpr int GOLDSTERNHALTSND = 17;
inline constexpr int OPENDOORSND = 18;
inline constexpr int CLOSEDOORSND = 19;
inline constexpr int GETDETONATORSND = 20;
inline constexpr int HALTSND = 21;
inline constexpr int RENTDEATH2SND = 22;
inline constexpr int ATKAUTOCHARGESND = 23;
inline constexpr int ATKCHARGEDSND = 24;
inline constexpr int RADAR_POWERUPSND = 25;
inline constexpr int ATKBURSTRIFLESND = 26;
inline constexpr int VITAL_GONESND = 27;
inline constexpr int SHOOTDOORSND = 28;
inline constexpr int RENTDEATH1SND = 29;
inline constexpr int GETBURSTRIFLESND = 30;
inline constexpr int GETAMMOSND = 31;
inline constexpr int SHOOTSND = 32;
inline constexpr int HEALTH1SND = 33;
inline constexpr int HEALTH2SND = 34;
inline constexpr int BONUS1SND = 35;
inline constexpr int BONUS2SND = 36;
inline constexpr int BONUS3SND = 37;
inline constexpr int GETIONCANNONSND = 38;
inline constexpr int ESCPRESSEDSND = 39;
inline constexpr int ELECAPPEARSND = 40;
inline constexpr int EXTRA_MANSND = 41;
inline constexpr int ELEV_BUTTONSND = 42;
inline constexpr int INTERROGATESND = 43;
inline constexpr int BONUS5SND = 44;
inline constexpr int BONUS4SND = 45;
inline constexpr int PUSHWALLSND = 46;
inline constexpr int TERM_BEEPSND = 47;
inline constexpr int ROLL_SCORESND = 48;
inline constexpr int TURRETSND = 49;
inline constexpr int EXPLODE1SND = 50;
inline constexpr int __UNUSED_51__SND = 51;
inline constexpr int SWATDIESND = 52;
inline constexpr int GGUARDHALTSND = 53;
inline constexpr int EXPLODE2SND = 54;
inline constexpr int BLUEBOYHALTSND = 55;
inline constexpr int PROGUARDDEATHSND = 56;
inline constexpr int DOGBOYHALTSND = 57;
inline constexpr int ENGINE_THRUSTSND = 58;
inline constexpr int SCANHALTSND = 59;
inline constexpr int GETCANNONSND = 60;
inline constexpr int LCANHALTSND = 61;
inline constexpr int PROHALTSND = 62;
inline constexpr int GGUARDDEATHSND = 63;
inline constexpr int BLUEBOYDEATHSND = 64;
inline constexpr int GOLDSTERNLAUGHSND = 65;
inline constexpr int SCIENTISTHALTSND = 66;
inline constexpr int SCIENTISTDEATHSND = 67;
inline constexpr int DOGBOYDEATHSND = 68;
inline constexpr int H_BEATSND = 69;
inline constexpr int SWATHALTSND = 70;
inline constexpr int SCANDEATHSND = 71;
inline constexpr int LCANDEATHSND = 72;
inline constexpr int INFORMDEATH2SND = 73;
inline constexpr int INFORMDEATH3SND = 74;
inline constexpr int GURNEYDEATHSND = 75;
inline constexpr int PRODEATH2SND = 76;
inline constexpr int PRODEATH3SND = 77; // AOG
inline constexpr int SWATDEATH2SND = 78;
inline constexpr int LCANBREAKSND = 79;
inline constexpr int SCANBREAKSND = 80;
inline constexpr int HTECHDOOROPENSND = 81;
inline constexpr int HTECHDOORCLOSESND = 82;
inline constexpr int ELECARCDAMAGESND = 83;
inline constexpr int PODHATCHSND = 84;
inline constexpr int ELECTSHOTSND = 85;
inline constexpr int ELECDIESND = 86;
inline constexpr int ATKGRENADESND = 87;
inline constexpr int CLAWATTACKSND = 88;
inline constexpr int PUNCHATTACKSND = 89;
inline constexpr int SPITATTACKSND = 90;
inline constexpr int PODDEATHSND = 91;
inline constexpr int PODHALTSND = 92;
inline constexpr int SWATDEATH3SND = 93; // AOG
inline constexpr int SCIDEATH2SND = 94;
inline constexpr int SCIDEATH3SND = 95;
inline constexpr int GOTTOKENSND = 96;
inline constexpr int SWITCHSND = 97;
inline constexpr int STATS1SND = 98;
inline constexpr int STATS2SND = 99;
inline constexpr int LASTSOUND = 100;

// Music names & indexes (AOG)

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

// Music names & indexes (PS)

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

// Music names & indexes (shared)

extern int APOGFNFM_MUS;
extern int THEME_MUS;
extern int LASTMUSIC;

enum class AudioDriverType
{
	auto_detect = 0,
	system,
	openal,
};

enum class AudioSfxType
{
	adlib = 1,
	pc_speaker = 2,
	pcm = 3,
};

enum class AudioChunkType
{
	adlib_music = 1,
	adlib_sfx = 2,
	pc_speaker = 3,
	digitized = 4,
};

struct AudioChunk
{
	const unsigned char* data;
	int data_size;
	AudioChunkType type;
	int index;
	int audio_index;
};

#endif // BSTONE_AUDIO_INCLUDED
