/////////////////////////////////////////////////
//
// MUSE Header for .VSI
// Created Tue Aug 23 16:07:48 1994
//
/////////////////////////////////////////////////

#define NUMSOUNDS		100
#define NUMSNDCHUNKS		321

//
// Sound names & indexes
//
typedef enum {
		HITWALLSND,              // 0
		TERM_TYPESND,            // 1
		GETPISTOLSND,            // 2
		LIQUIDDIESND,            // 3
		MOVEGUN2SND,             // 4
		MOVEGUN1SND,             // 5
		NOWAYSND,                // 6
		SCOUT_ALERTSND,          // 7
		GURNEYSND,               // 8
		PLAYERDEATHSND,          // 9
		CONCESSIONSSND,          // 10
		ATKIONCANNONSND,         // 11
		GETKEYSND,               // 12
		WARPOUTSND,              // 13
		WARPINSND,               // 14
		ROBOT_SERVOSND,          // 15
		INFORMANTDEATHSND,       // 16
		GOLDSTERNHALTSND,        // 17
		OPENDOORSND,             // 18
		CLOSEDOORSND,            // 19
		GETDETONATORSND,         // 20
		HALTSND,                 // 21
		RENTDEATH2SND,           // 22
		ATKAUTOCHARGESND,        // 23
		ATKCHARGEDSND,           // 24
		RADAR_POWERUPSND,        // 25
		ATKBURSTRIFLESND,        // 26
		VITAL_GONESND,           // 27
		SHOOTDOORSND,            // 28
		RENTDEATH1SND,           // 29
		GETBURSTRIFLESND,        // 30
		GETAMMOSND,              // 31
		SHOOTSND,                // 32
		HEALTH1SND,              // 33
		HEALTH2SND,              // 34
		BONUS1SND,               // 35
		BONUS2SND,               // 36
		BONUS3SND,               // 37
		GETIONCANNONSND,         // 38
		ESCPRESSEDSND,           // 39
		ELECAPPEARSND,           // 40
		EXTRA_MANSND,            // 41
		ELEV_BUTTONSND,          // 42
		INTERROGATESND,          // 43
		BONUS5SND,               // 44
		BONUS4SND,               // 45
		PUSHWALLSND,             // 46
		TERM_BEEPSND,            // 47
		ROLL_SCORESND,           // 48
		TURRETSND,               // 49
		EXPLODE1SND,             // 50
		__UNUSED_51__SND,        // 51
		SWATDIESND,              // 52
		GGUARDHALTSND,           // 53
		EXPLODE2SND,             // 54
		BLUEBOYHALTSND,          // 55
		PROGUARDDEATHSND,        // 56
		DOGBOYHALTSND,           // 57
		ENGINE_THRUSTSND,        // 58
		SCANHALTSND,             // 59
		GETCANNONSND,            // 60
		LCANHALTSND,             // 61
		PROHALTSND,              // 62
		GGUARDDEATHSND,          // 63
		BLUEBOYDEATHSND,         // 64
		GOLDSTERNLAUGHSND,       // 65
		SCIENTISTHALTSND,        // 66
		SCIENTISTDEATHSND,       // 67
		DOGBOYDEATHSND,          // 68
		H_BEATSND,               // 69
		SWATHALTSND,             // 70
		SCANDEATHSND,            // 71
		LCANDEATHSND,            // 72
		INFORMDEATH2SND,         // 73
		INFORMDEATH3SND,         // 74
		GURNEYDEATHSND,          // 75
		PRODEATH2SND,            // 76
		__UNUSED_77__SND,        // 77
		SWATDEATH2SND,           // 78
		LCANBREAKSND,            // 79
		SCANBREAKSND,            // 80
		HTECHDOOROPENSND,        // 81
		HTECHDOORCLOSESND,       // 82
		ELECARCDAMAGESND,        // 83
		PODHATCHSND,             // 84
		ELECTSHOTSND,            // 85
		ELECDIESND,              // 86
		ATKGRENADESND,           // 87
		CLAWATTACKSND,           // 88
		PUNCHATTACKSND,          // 89
		SPITATTACKSND,           // 90
		PODDEATHSND,             // 91
		PODHALTSND,              // 92
		__UNUSED_93__SND,        // 93
		SCIDEATH2SND,            // 94
		SCIDEATH3SND,            // 95
		GOTTOKENSND,             // 96
		SWITCHSND,               // 97
		STATS1SND,               // 98
		STATS2SND,               // 99
		LASTSOUND
	     } soundnames;

//
// Base offsets
//
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	100
#define STARTDIGISOUNDS		200
#define STARTMUSIC		300

//
// Music names & indexes
//
typedef enum {
		CATACOMB_MUS,            // 0
		STICKS_MUS,              // 1
		APOGFNFM_MUS,            // 2
		PLOT_MUS,                // 3
		CIRCLES_MUS,             // 4
		LASTLAFF_MUS,            // 5
		TOHELL_MUS,              // 6
		FORTRESS_MUS,            // 7
		GIVING_MUS,              // 8
		HARTBEAT_MUS,            // 9
		LURKING_MUS,             // 10
		MAJMIN_MUS,              // 11
		VACCINAP_MUS,            // 12
		DARKNESS_MUS,            // 13
		MONASTRY_MUS,            // 14
		TOMBP_MUS,               // 15
		TIME_MUS,                // 16
		MOURNING_MUS,            // 17
		SERPENT_MUS,             // 18
		HISCORE_MUS,             // 19
		THEME_MUS,               // 20
		LASTMUSIC
	     } musicnames;

/////////////////////////////////////////////////
//
// Thanks for playing with MUSE!
//
/////////////////////////////////////////////////
