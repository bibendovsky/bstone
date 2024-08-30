/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_AUDIO_INCLUDED
#define BSTONE_AUDIO_INCLUDED


#include <cstdint>


constexpr auto NUMSOUNDS = 100;


//
// Sound names & indexes
//
const std::int16_t HITWALLSND = 0;
const std::int16_t TERM_TYPESND = 1;
const std::int16_t GETPISTOLSND = 2;
const std::int16_t LIQUIDDIESND = 3;
const std::int16_t MOVEGUN2SND = 4;
const std::int16_t MOVEGUN1SND = 5;
const std::int16_t NOWAYSND = 6;
const std::int16_t SCOUT_ALERTSND = 7;
const std::int16_t GURNEYSND = 8;
const std::int16_t PLAYERDEATHSND = 9;
const std::int16_t CONCESSIONSSND = 10;
const std::int16_t ATKIONCANNONSND = 11;
const std::int16_t GETKEYSND = 12;
const std::int16_t WARPOUTSND = 13;
const std::int16_t WARPINSND = 14;
const std::int16_t ROBOT_SERVOSND = 15;
const std::int16_t INFORMANTDEATHSND = 16;
const std::int16_t GOLDSTERNHALTSND = 17;
const std::int16_t OPENDOORSND = 18;
const std::int16_t CLOSEDOORSND = 19;
const std::int16_t GETDETONATORSND = 20;
const std::int16_t HALTSND = 21;
const std::int16_t RENTDEATH2SND = 22;
const std::int16_t ATKAUTOCHARGESND = 23;
const std::int16_t ATKCHARGEDSND = 24;
const std::int16_t RADAR_POWERUPSND = 25;
const std::int16_t ATKBURSTRIFLESND = 26;
const std::int16_t VITAL_GONESND = 27;
const std::int16_t SHOOTDOORSND = 28;
const std::int16_t RENTDEATH1SND = 29;
const std::int16_t GETBURSTRIFLESND = 30;
const std::int16_t GETAMMOSND = 31;
const std::int16_t SHOOTSND = 32;
const std::int16_t HEALTH1SND = 33;
const std::int16_t HEALTH2SND = 34;
const std::int16_t BONUS1SND = 35;
const std::int16_t BONUS2SND = 36;
const std::int16_t BONUS3SND = 37;
const std::int16_t GETIONCANNONSND = 38;
const std::int16_t ESCPRESSEDSND = 39;
const std::int16_t ELECAPPEARSND = 40;
const std::int16_t EXTRA_MANSND = 41;
const std::int16_t ELEV_BUTTONSND = 42;
const std::int16_t INTERROGATESND = 43;
const std::int16_t BONUS5SND = 44;
const std::int16_t BONUS4SND = 45;
const std::int16_t PUSHWALLSND = 46;
const std::int16_t TERM_BEEPSND = 47;
const std::int16_t ROLL_SCORESND = 48;
const std::int16_t TURRETSND = 49;
const std::int16_t EXPLODE1SND = 50;
const std::int16_t __UNUSED_51__SND = 51;
const std::int16_t SWATDIESND = 52;
const std::int16_t GGUARDHALTSND = 53;
const std::int16_t EXPLODE2SND = 54;
const std::int16_t BLUEBOYHALTSND = 55;
const std::int16_t PROGUARDDEATHSND = 56;
const std::int16_t DOGBOYHALTSND = 57;
const std::int16_t ENGINE_THRUSTSND = 58;
const std::int16_t SCANHALTSND = 59;
const std::int16_t GETCANNONSND = 60;
const std::int16_t LCANHALTSND = 61;
const std::int16_t PROHALTSND = 62;
const std::int16_t GGUARDDEATHSND = 63;
const std::int16_t BLUEBOYDEATHSND = 64;
const std::int16_t GOLDSTERNLAUGHSND = 65;
const std::int16_t SCIENTISTHALTSND = 66;
const std::int16_t SCIENTISTDEATHSND = 67;
const std::int16_t DOGBOYDEATHSND = 68;
const std::int16_t H_BEATSND = 69;
const std::int16_t SWATHALTSND = 70;
const std::int16_t SCANDEATHSND = 71;
const std::int16_t LCANDEATHSND = 72;
const std::int16_t INFORMDEATH2SND = 73;
const std::int16_t INFORMDEATH3SND = 74;
const std::int16_t GURNEYDEATHSND = 75;
const std::int16_t PRODEATH2SND = 76;
const std::int16_t PRODEATH3SND = 77; // AOG
const std::int16_t SWATDEATH2SND = 78;
const std::int16_t LCANBREAKSND = 79;
const std::int16_t SCANBREAKSND = 80;
const std::int16_t HTECHDOOROPENSND = 81;
const std::int16_t HTECHDOORCLOSESND = 82;
const std::int16_t ELECARCDAMAGESND = 83;
const std::int16_t PODHATCHSND = 84;
const std::int16_t ELECTSHOTSND = 85;
const std::int16_t ELECDIESND = 86;
const std::int16_t ATKGRENADESND = 87;
const std::int16_t CLAWATTACKSND = 88;
const std::int16_t PUNCHATTACKSND = 89;
const std::int16_t SPITATTACKSND = 90;
const std::int16_t PODDEATHSND = 91;
const std::int16_t PODHALTSND = 92;
const std::int16_t SWATDEATH3SND = 93; // AOG
const std::int16_t SCIDEATH2SND = 94;
const std::int16_t SCIDEATH3SND = 95;
const std::int16_t GOTTOKENSND = 96;
const std::int16_t SWITCHSND = 97;
const std::int16_t STATS1SND = 98;
const std::int16_t STATS2SND = 99;
const std::int16_t LASTSOUND = 100;


//
// Music names & indexes (AOG)
//
extern std::int16_t S2100A_MUS;
extern std::int16_t GOLDA_MUS;
extern std::int16_t APOGFNFM_MUS;
extern std::int16_t DRKHALLA_MUS;
extern std::int16_t FREEDOMA_MUS;
extern std::int16_t GENEFUNK_MUS;
extern std::int16_t TIMEA_MUS;
extern std::int16_t HIDINGA_MUS;
extern std::int16_t INCNRATN_MUS;
extern std::int16_t JUNGLEA_MUS;
extern std::int16_t LEVELA_MUS;
extern std::int16_t MEETINGA_MUS;
extern std::int16_t STRUTA_MUS;
extern std::int16_t RACSHUFL_MUS;
extern std::int16_t RUMBAA_MUS;
extern std::int16_t SEARCHNA_MUS;
extern std::int16_t THEWAYA_MUS;
extern std::int16_t INTRIGEA_MUS;

//
// Music names & indexes (PS)
//
extern std::int16_t CATACOMB_MUS;
extern std::int16_t STICKS_MUS;
extern std::int16_t PLOT_MUS;
extern std::int16_t CIRCLES_MUS;
extern std::int16_t LASTLAFF_MUS;
extern std::int16_t TOHELL_MUS;
extern std::int16_t FORTRESS_MUS;
extern std::int16_t GIVING_MUS;
extern std::int16_t HARTBEAT_MUS;
extern std::int16_t LURKING_MUS;
extern std::int16_t MAJMIN_MUS;
extern std::int16_t VACCINAP_MUS;
extern std::int16_t DARKNESS_MUS;
extern std::int16_t MONASTRY_MUS;
extern std::int16_t TOMBP_MUS;
extern std::int16_t TIME_MUS;
extern std::int16_t MOURNING_MUS;
extern std::int16_t SERPENT_MUS;
extern std::int16_t HISCORE_MUS;


//
// Music names & indexes (shared)
//
extern std::int16_t APOGFNFM_MUS;
extern std::int16_t THEME_MUS;
extern std::int16_t LASTMUSIC;


enum class AudioDriverType
{
	auto_detect,
	system,
	openal,
}; // AudioDriverType

enum class AudioSfxType
{
	adlib = 1,
	pc_speaker = 2,
	pcm = 3,
}; // AudioResourceType

enum class AudioChunkType
{
	adlib_music = 1,
	adlib_sfx = 2,
	pc_speaker = 3,
	digitized = 4,
}; // AudioResourceType

struct AudioChunk
{
	const std::uint8_t* data;
	int data_size;
	AudioChunkType type;
	int index;
	int audio_index;
}; // AudioChunk


#endif // BSTONE_AUDIO_INCLUDED
