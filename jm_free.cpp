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


// JM_FREE.C

//
// Warning!
//
// All functions in this source file are designated usable by the memory
// manager after program initialization.
//

#include "id_heads.h"
#ifdef MSVC
#pragma hdrstop
#endif

#include "3d_def.h"


extern int* spanstart;
extern int* stepscale;
extern int* basedist;
extern int* planeylookup;
extern int* mirrorofs;


void MML_ClearBlock();
void CA_CannotOpen(
    const std::string& string);
void CAL_GetGrChunkLength(
    Sint16 chunk);
void CA_CacheScreen(
    Sint16 chunk);
void VH_UpdateScreen();
void IN_StartAck();
boolean IN_CheckAck();
void OpenMapFile();
void CloseMapFile();
void ClearMemory();
void PM_SetMainMemPurge(
    Sint16 level);
void ShutdownId();
void InitRedShifts();
void CAL_OptimizeNodes(
    huffnode* table);
void OpenAudioFile();
void ReadConfig();

void initialize_sprites();
void initialize_states();
void initialize_audio_constants();
void initialize_songs();
void initialize_gfxv_contants();
void initialize_shape_table();
void initialize_static_info_constants();
void initialize_weapon_constants();
void initialize_grenade_shape_constants();
void initialize_static_health_table();
void initialize_boss_constants();
void initialize_messages();
void initialize_ca_constants();


#define SKIP_CHECKSUMS (1)
#define SHOW_CHECKSUM (0)

#if GAME_VERSION == SHAREWARE_VERSION

#define AUDIOT_CHECKSUM 0xFFF87142
#define MAPTEMP_CHECKSUM 0x000370C9
#define VGAGRAPH_CHECKSUM 0xFFFFDE44
#define DIZFILE_CHECKSUM 0x00000879l

#elif GAME_VERSION == MISSIONS_1_THR_3

#define AUDIOT_CHECKSUM 0xFFF87142
#define MAPTEMP_CHECKSUM 0x00084F1F
#define VGAGRAPH_CHECKSUM 0xFFFFDE44
#define DIZFILE_CHECKSUM 0x00000879l

#else


#define AUDIOT_CHECKSUM 0xfff912C9
#define MAPTEMP_CHECKSUM 0x00107739
#define VGAGRAPH_CHECKSUM 0xffff6C9A
#define DIZFILE_CHECKSUM 0x00000879l


#endif

extern SDL_TimerID sys_timer_id;

void SDL_SBSetDMA(
    Uint8 channel);
void SDL_SetupDigi();

// =========================================================================
//
// FAR FREE DATA
//
// =========================================================================

#if FREE_DATA

char JM_FREE_DATA_START[1] = { 0 };

#endif

#if TECH_SUPPORT_VERSION
char EnterBetaCode[] = "\n  TECH SUPPORT VERSION!\n\n  NO DISTRIBUTING!";
#elif BETA_TEST
char EnterBetaCode[] = "      !BETA VERSION!\n    DO NOT DISTRIBUTE\n UNDER PENALTY OF DEATH\n\n   ENTER BETA CODE";
#endif


const char* JHParmStrings[] = { "no386", "is386", nil };

char show_text1[] = "\n     SYSTEM INFO\n";
char show_text2[] = "=======================\n\n";
char show_text3[] = "-- Memory avail after game is loaded --\n\n";
char show_text4[] = "            ** Insufficient memory to run the game **";
char show_text5[] = "---- Extra Devices ----\n\n";

static const char* ParmStrings[] = { "HIDDENCARD", "" };

// BBi FIXME
static Uint8 wolfdigimap[] = {
    // These first sounds are in the upload version

    ATKIONCANNONSND, 0,
    ATKCHARGEDSND, 1,
    ATKBURSTRIFLESND, 2,
    ATKGRENADESND, 46,

    OPENDOORSND, 3,
    CLOSEDOORSND, 4,
    HTECHDOOROPENSND, 5,
    HTECHDOORCLOSESND, 6,

    INFORMANTDEATHSND, 7,
    SCIENTISTHALTSND, 19,
    SCIENTISTDEATHSND, 20,

    GOLDSTERNHALTSND, 8,
    GOLDSTERNLAUGHSND, 24,

    HALTSND, 9, // Rent-A-Cop 1st sighting
    RENTDEATH1SND, 10, // Rent-A-Cop Death

    EXPLODE1SND, 11,

    GGUARDHALTSND, 12,
    GGUARDDEATHSND, 17,

    PROHALTSND, 16,
    PROGUARDDEATHSND, 13,

    BLUEBOYDEATHSND, 18,
    BLUEBOYHALTSND, 51,

    SWATHALTSND, 22,
    SWATDIESND, 47,

    SCANHALTSND, 15,
    SCANDEATHSND, 23,

    PODHATCHSND, 26,
    PODHALTSND, 50,
    PODDEATHSND, 49,

    ELECTSHOTSND, 27,

    DOGBOYHALTSND, 14,
    DOGBOYDEATHSND, 21,
    ELECARCDAMAGESND, 25,
    ELECAPPEARSND, 28,
    ELECDIESND, 29,

    INFORMDEATH2SND, 39, // Informant Death #2
    RENTDEATH2SND, 34, // Rent-A-Cop Death #2
    PRODEATH2SND, 42, // PRO Death #2
    SWATDEATH2SND, 48, // SWAT Death #2
    SCIDEATH2SND, 53, // Gen. Sci Death #2

    LIQUIDDIESND, 30,

    GURNEYSND, 31,
    GURNEYDEATHSND, 41,

    WARPINSND, 32,
    WARPOUTSND, 33,

    EXPLODE2SND, 35,

    LCANHALTSND, 36,
    LCANDEATHSND, 37,

// RENTDEATH3SND, 38, // Rent-A-Cop Death #3
    INFORMDEATH3SND, 40, // Informant Death #3
// PRODEATH3SND, 43, // PRO Death #3
// SWATDEATH3SND, 52, // Swat Guard #3
    SCIDEATH3SND, 54, // Gen. Sci Death #3

    LCANBREAKSND, 44,
    SCANBREAKSND, 45,
    CLAWATTACKSND, 56,
    SPITATTACKSND, 55,
    PUNCHATTACKSND, 57,

    LASTSOUND
};

char cinfo_text[] = "\n"
                    "Planet Strike\n"
                    "Copyright (c) 1993 - JAM Productions, Inc.\n"
                    "All rights reserved.\n";

#if BETA_TEST
char dver_text[] = "Download the latest version pal!";
#endif


#if FREE_DATA

char JM_FREE_DATA_END[1] = { 0 };

#endif

extern const uint8_t colormap[16896];
const Uint8* lightsource;

// =========================================================================
//
// FREE FUNCTIONS
//
// =========================================================================



#if FREE_FUNCTIONS

// This function is used as a label for the start of the
// functions used by the memory manager.
//
void JM_FREE_START()
{
}

#endif

// ------------------ ID Software 'startup' functions ---------------------

///////////////////////////////////////////////////////////////////////////
//
//      US_Startup() - Starts the User Mgr
//
///////////////////////////////////////////////////////////////////////////

extern boolean US_Started;
extern const char* US_ParmStrings[];
extern const char* US_ParmStrings2[];

// BBi
Uint32 sys_timer_callback(
    Uint32 interval,
    void*)
{
    ++TimeCount;
    return interval;
}
// BBi

void US_Startup()
{
    Sint16 n;

    // BBi
    int sdl_result;
    // BBi

    if (US_Started) {
        return;
    }

    // BBi
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "SDL: %s", "Initializing timer...");

    sdl_result = SDL_InitSubSystem(SDL_INIT_TIMER);

    if (sdl_result != 0) {
        Quit("%s", SDL_GetError());
    }

    sys_timer_id = SDL_AddTimer(1000 / 70, sys_timer_callback, NULL);

    if (sys_timer_id == 0) {
        Quit("%s", SDL_GetError());
    }
    // BBi

    US_InitRndT(true); // Initialize the random number generator

    switch (::g_args.check_argument(US_ParmStrings2)) {
    case 0:
        compatability = true;
        break;
    case 1:
        compatability = false;
        break;
    }

    // Check for TED launching here
    n = static_cast<int16_t>(g_args.check_argument(US_ParmStrings));
    switch (n) {
    case 0:
#if 0
        tedlevelnum = atoi(g_argv[i + 1]);
//                 if (tedlevelnum >= 0)
        tedlevel = true;
#endif
        break;

//               case 1:
//                      NoWait = true;
//                      break;
    }

    US_Started = true;
}


/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const float radtoint = static_cast<float>(FINEANGLES / 2 / PI);

void BuildTables()
{
    Sint16 i;
    float angle, anglestep;
    double tang;
    fixed value;


//
// calculate fine tangents
//

    for (i = 0; i < FINEANGLES / 8; i++) {
        tang = tan((i + 0.5) / radtoint);
        finetangent[i] = static_cast<Sint32>(tang * TILEGLOBAL);
        finetangent[FINEANGLES / 4 - 1 - i] = static_cast<Sint32>(1 / tang * TILEGLOBAL);
    }

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//
// The low word of the value is the fraction, the high bit is the sign bit,
// bits 16-30 should be 0
//

    angle = 0.0F;
    anglestep = static_cast<float>(PI / 2 / ANGLEQUAD);
    for (i = 0; i <= ANGLEQUAD; i++) {
        value = static_cast<fixed>(GLOBAL1 * sin(static_cast<double>(angle)));
        sintable[i] =
            sintable[i + ANGLES] =
                sintable[ANGLES / 2 - i] = value;
        sintable[ANGLES - i] =
            sintable[ANGLES / 2 + i] = value | 0x80000000l;
        angle += anglestep;
    }

    lightsource = colormap;
}

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls()
{
    Sint16 i;

    //
    // Hey! Yea You! This is where you can VERY easly setup to use a
    // specific 'BANK' of wall graphics.... JTR
    //

    for (i = 1; i < MAXWALLTILES; i++) {
        horizwall[i] = (i - 1) * 2;
        vertwall[i] = (i - 1) * 2 + 1;
    }

    delete [] wallheight;
    wallheight = new int[vga_width];

    const int k_half_height = vga_height / 2;

    delete [] spanstart;
    spanstart = new int[k_half_height];
    std::uninitialized_fill_n(spanstart, k_half_height, 0);

    delete [] stepscale;
    stepscale = new int[k_half_height];
    std::uninitialized_fill_n(stepscale, k_half_height, 0);

    delete [] basedist;
    basedist = new int[k_half_height];
    std::uninitialized_fill_n(basedist, k_half_height, 0);

    delete [] planeylookup;
    planeylookup = new int[k_half_height];
    std::uninitialized_fill_n(planeylookup, k_half_height, 0);

    delete [] mirrorofs;
    mirrorofs = new int[k_half_height];
    std::uninitialized_fill_n(mirrorofs, k_half_height, 0);
}




/*
=====================
=
= InitDigiMap
=
=====================
*/
// 3D_GAME.C

void InitDigiMap()
{
    char* map;

    for (map = reinterpret_cast<char*>(wolfdigimap); *map != LASTSOUND; map += 2) {
        DigiMap[static_cast<int>(map[0])] = map[1];
    }
}



/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile()
{
    bstone::FileStream handle;
    int32_t length;
    std::string fname;

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
    fname = ::aheadname + ::extension;

    handle.open(fname);
    if (!handle.is_open()) {
        ::CA_CannotOpen(fname);
    }

    length = static_cast<int32_t>(handle.get_size());
    ::audiostarts = new int32_t[length / 4];
    handle.read(::audiostarts, length);
    handle.close();
#else
    audiohuffman = (huffnode*)&audiodict;
    CAL_OptimizeNodes(audiohuffman);
    audiostarts = (Sint32*)FP_SEG(&audiohead);
#endif

//
// open the data file
//
    ::OpenAudioFile();
}



/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile()
{
    std::string fname;
    bstone::FileStream handle;
    Uint8* compseg;

    //
    // load ???dict.ext (huffman dictionary for graphics files)
    //

    fname = ::gdictname + ::extension;

    handle.open(fname);

    if (!handle.is_open()) {
        ::CA_CannotOpen(fname);
    }

    handle.read(&::grhuffman, sizeof(::grhuffman));

    //
    // load the data offsets from ???head.ext
    //
    int grstarts_size = (NUMCHUNKS + 1) * FILEPOSSIZE;

    ::grstarts = new int32_t[(grstarts_size + 3) / 4];

    fname = ::gheadname + ::extension;

    handle.open(fname);

    if (!handle.is_open()) {
        ::CA_CannotOpen(fname);
    }

    handle.read(::grstarts, grstarts_size);

    //
    // Open the graphics file, leaving it open until the game is finished
    //
    ::OpenGrFile();

    //
    // load the pic and sprite headers into the arrays in the data segment
    //
    ::pictable = new pictabletype[NUMPICS];
    ::CAL_GetGrChunkLength(STRUCTPIC); // position file pointer
    compseg = new Uint8[::chunkcomplen];
    ::grhandle.read(compseg, ::chunkcomplen);

    ::CAL_HuffExpand(
        compseg,
        reinterpret_cast<uint8_t*>(pictable),
        NUMPICS * sizeof(pictabletype),
        ::grhuffman);

    delete [] compseg;
}



/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile()
{
    Sint16 i;
    bstone::FileStream handle;
    Sint32 pos;
    std::string fname;
    mapfiletype header;
    maptype* map_header;

    //
    // load maphead.ext (offsets and tileinfo for map file)
    //

    fname = mheadname + extension;

    handle.open(fname);

    if (!handle.is_open()) {
        CA_CannotOpen(fname);
    }

    handle.read(&header.RLEWtag, sizeof(header.RLEWtag));
    handle.read(&header.headeroffsets, sizeof(header.headeroffsets));

    rlew_tag = header.RLEWtag;

    //
    // open the data file
    //
    OpenMapFile();

    //
    // load all map header
    //
    for (i = 0; i < NUMMAPS; ++i) {
        pos = header.headeroffsets[i];

        if (pos < 0) {
            continue;
        }

        mapheaderseg[i] = new maptype();
        map_header = mapheaderseg[i];

        maphandle.set_position(pos);

        maphandle.read(
            &map_header->planestart,
            sizeof(map_header->planestart));

        maphandle.read(
            &map_header->planelength,
            sizeof(map_header->planelength));

        maphandle.read(
            &map_header->width,
            sizeof(map_header->width));

        maphandle.read(
            &map_header->height,
            sizeof(map_header->height));

        maphandle.read(
            &map_header->name,
            sizeof(map_header->name));
    }

    //
    // allocate space for 3 64*64 planes
    //
    for (i = 0; i < MAPPLANES; ++i) {
        mapsegs[i] = new Uint16[64 * 64];
    }

#if FORCE_FILE_CLOSE
    CloseMapFile();
#endif

}


// --------------------- Other general functions ------------------------

#define CHECK_FOR_EPISODES

extern CP_itemtype NewEmenu[];
extern Sint16 EpisodeSelect[];


static bool are_files_exist(
    const bstone::StringList& file_names)
{
    for (const auto& file_name : file_names) {
        if (!bstone::FileStream::is_exists(file_name)) {
            return false;
        }
    }

    return true;
}

// -------------------------------------------------------------------------
// CheckForEpisodes() - CHECK FOR VERSION/EXTESION
// -------------------------------------------------------------------------
void CheckForEpisodes()
{
    const bstone::StringList aog_file_names = {
        "AUDIOHED.BS6",
        "AUDIOT.BS6",
        "EANIM.BS6",
        "GANIM.BS6",
        "IANIM.BS6",
        "MAPHEAD.BS6",
        "MAPTEMP.BS6",
        "SANIM.BS6",
        "VGADICT.BS6",
        "VGAGRAPH.BS6",
        "VGAHEAD.BS6",
        "VSWAP.BS6",
    }; // aog_file_names

    const bstone::StringList aog_sw_file_names = {
        "AUDIOHED.BS1",
        "AUDIOT.BS1",
        "IANIM.BS1",
        "MAPHEAD.BS1",
        "MAPTEMP.BS1",
        "SANIM.BS1",
        "VGADICT.BS1",
        "VGAGRAPH.BS1",
        "VGAHEAD.BS1",
        "VSWAP.BS1",
    }; // aog_sw_file_names

    const bstone::StringList ps_file_names = {
        "AUDIOHED.VSI",
        "AUDIOT.VSI",
        "EANIM.VSI",
        "IANIM.VSI",
        "MAPHEAD.VSI",
        "MAPTEMP.VSI",
        "VGADICT.VSI",
        "VGAGRAPH.VSI",
        "VGAHEAD.VSI",
        "VSWAP.VSI",
    }; // ps_file_names


    if (::g_args.has_option("aog")) {
        ::g_game_type = GameType::aog;

        ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "%s\n", "Forcing Aliens Of Gold (full).");
    } else if (::g_args.has_option("aog_sw")) {
        ::g_game_type = GameType::aog_sw;

        ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "%s\n", "Forcing Aliens Of Gold (shareware).");
    } else if (::g_args.has_option("ps")) {
        ::g_game_type = GameType::ps;

        ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "%s\n", "Forcing Planet Strike.");
    }


    auto is_succeed = true;
    std::string error_message = "All expected files not found.";

    switch (::g_game_type) {
    case GameType::aog:
        if (!::are_files_exist(aog_file_names)) {
            is_succeed = false;
        }
        break;

    case GameType::aog_sw:
        if (!::are_files_exist(aog_sw_file_names)) {
            is_succeed = false;
        }
        break;

    case GameType::ps:
        if (!::are_files_exist(ps_file_names)) {
            is_succeed = false;
        }
        break;

    default:
        ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "%s\n", "Determining the game type.");

        if (::are_files_exist(aog_file_names)) {
            ::g_game_type = GameType::aog;

            ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "%s\n", "Found Aliens Of Gold (full).");
        } else if (::are_files_exist(ps_file_names)) {
            ::g_game_type = GameType::ps;

            ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "%s\n", "Found Planet Strike.");
        } else if (::are_files_exist(aog_sw_file_names)) {
            ::g_game_type = GameType::aog_sw;

            ::SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "%s\n", "Found Aliens Of Gold (shareware).");
        } else {
            is_succeed = false;
            error_message = "Unable to find all expected files of any game.";
        }
        break;
    }

    if (!is_succeed) {
        ::Quit("%s\n", error_message.c_str());
    }


    switch (::g_game_type) {
    case GameType::aog:
        ::extension = "BS6";
        break;

    case GameType::aog_sw:
        ::extension = "BS1";
        break;

    case GameType::ps:
        ::extension = "PS";
        break;

    default:
        throw std::runtime_error("Invalid game type.");
    }

    for (auto i = 0; i < mv_NUM_MOVIES; ++i) {
        ::strcat(::Movies[i].FName, ::extension.c_str());
    }

    if (::is_aog()) {
        for (auto i = 1; i < 6; ++i) {
            ::NewEmenu[i].active = AT_ENABLED;
        }
    }

#ifdef ACTIVATE_TERMINAL
    strcat(term_com_name, extension);
    strcat(term_msg_name, extension);
#endif

    ::PageFileName += ::extension;
    ::audioname += ::extension;
    ::demoname += ::extension;

#if DUAL_SWAP_FILES
    strcat(AltPageFileName, extension);
    ShadowsAvail = (!findfirst(AltPageFileName, &f, FA_ARCH));
#endif
}


#if 0
//
// ORGINAL used in Aliens of Gold
//
// -------------------------------------------------------------------------
// CheckForEpisodes() - CHECK FOR EPISODES
// -------------------------------------------------------------------------
void CheckForEpisodes()
{
    struct ffblk f;
    Sint16 i;

#if (GAME_VERSION != SHAREWARE_VERSION)
#ifdef CHECK_FOR_EPISODES
    if (!findfirst("*.BS6", &f, FA_ARCH))
#else
    if (!findfirst("*.BS1", &f, FA_ARCH))
#endif
    {
#ifdef CHECK_FOR_EPISODES
        strcpy(extension, "BS6");
#else
        strcpy(extension, "BS1");
#endif
        NewEmenu[1].active =
            NewEmenu[2].active =
                NewEmenu[3].active =
                    NewEmenu[4].active =
                        NewEmenu[5].active =
                            EpisodeSelect[1] =
                                EpisodeSelect[2] =
                                    EpisodeSelect[3] =
                                        EpisodeSelect[4] =
                                            EpisodeSelect[5] = AT_ENABLED;
    } else
#ifdef CHECK_FOR_EPISODES
    if (!findfirst("*.BS3", &f, FA_ARCH))
#else
    if (!findfirst("*.BS1", &f, FA_ARCH))
#endif
    {
#ifdef CHECK_FOR_EPISODES
        strcpy(extension, "BS3");
#else
        strcpy(extension, "BS1");
#endif
        NewEmenu[1].active =
            NewEmenu[2].active =
                EpisodeSelect[1] =
                    EpisodeSelect[2] = AT_ENABLED;
    } else
#endif
    if (!findfirst("*.BS1", &f, FA_ARCH)) {
        strcpy(extension, "BS1");
    } else {
        printf("No Blake Stone data files found!");
        exit(0);
    }

#if BETA_TEST
// This enables specific missions for beta testers.
//
    NewEmenu[1].active =
        EpisodeSelect[1] =
            NewEmenu[2].active =
                EpisodeSelect[2] =
                    NewEmenu[3].active =
                        EpisodeSelect[3] =
                            NewEmenu[4].active =
                                EpisodeSelect[4] =
                                    NewEmenu[5].active =
                                        EpisodeSelect[5] = AT_ENABLED;
#endif

    for (i = 0; i < mv_NUM_MOVIES; i++) {
        strcat(Movies[i].FName, extension);
    }

#ifdef ACTIVATE_TERMINAL
    strcat(term_com_name, extension);
    strcat(term_msg_name, extension);
#endif

    strcat(configname, extension);
    _fstrcat(SaveName, extension);
    strcat(PageFileName, extension);
    strcat(audioname, extension);
    strcat(demoname, extension);

#if DUAL_SWAP_FILES
    strcat(AltPageFileName, extension);
    ShadowsAvail = (!findfirst(AltPageFileName, &f, FA_ARCH));
#endif
}
#endif


extern const char* MainStrs[];
extern char bc_buffer[];

// ------------------------------------------------------------------------
// PreDemo()
// ------------------------------------------------------------------------
void PreDemo()
{
#if !SKIP_TITLE_AND_CREDITS
#if TECH_SUPPORT_VERSION

    fontnumber = 4;
    SETFONTCOLOR(0, 15 * 3);
    CenterWindow(26, 7);
    US_Print(EnterBetaCode);
    VW_UpdateScreen();
    CA_LoadAllSounds();
    PM_CheckMainMem();
    SD_PlaySound(INFORMDEATH2SND); // Nooooo!
    IN_UserInput(TickBase * 20);
    ClearMemory();

#elif BETA_TEST

    boolean param = false;

    for (i = 1; i < g_argc; i++) {
        switch (US_CheckParm(g_argv[i], MainStrs)) {
        case 13:
            param = true;
            break;
        }
    }

    if (!param) {
        char buffer[15] = { 0 };

        fontnumber = 4;
        CenterWindow(26, 7);
        US_Print(EnterBetaCode);
        VW_UpdateScreen();
        SETFONTCOLOR(0, 15 * 3);
        US_LineInput(24 * 8, 92, buffer, buffer, true, 14, 100);
        if (_fstricmp(buffer, bc_buffer)) {
            Quit("Bad beta code!");
        }
    }
#endif



#if GAME_VERSION == SHAREWARE_VERSION
#if IN_DEVELOPMENT || GEORGE_CHEAT
    if (!MS_CheckParm("nochex"))
#endif
    {
#if  (!SKIP_CHECKSUMS)
//      CheckValidity("MAPTEMP.",MAPTEMP_CHECKSUM,"LEVELS");
        CheckValidity("MAPTEMP.", MAPTEMP_CHECKSUM);
#endif
    }
#else
#if  (!SKIP_CHECKSUMS)
    if (ChecksumFile("FILE_ID.DIZ", 0) != DIZFILE_CHECKSUM) {
        gamestate.flags |= GS_BAD_DIZ_FILE;
    }
#endif
#endif

    VL_SetPaletteIntensity(0, 255, vgapal, 0);

    if (!(gamestate.flags & GS_NOWAIT)) {
        if (::is_aog_sw()) {
            // ---------------------
            // Anti-piracy screen
            // ---------------------

            // Cache pic
            //
            CA_CacheScreen(PIRACYPIC);

            // Cache and set palette.  AND  Fade it in!
            //
            CA_CacheGrChunk(PIRACYPALETTE);
            VL_SetPalette(0, 256, static_cast<const Uint8*>(grsegs[PIRACYPALETTE]));
            VL_SetPaletteIntensity(0, 255, static_cast<const Uint8*>(grsegs[PIRACYPALETTE]), 0);
            VW_UpdateScreen();

            VL_FadeOut(0, 255, 0, 0, 25, 20);
            VL_FadeIn(0, 255, static_cast<const Uint8*>(grsegs[PIRACYPALETTE]), 30);

            // Wait a little
            //
            IN_UserInput(TickBase * 20);

            // Free palette
            //
            UNCACHEGRCHUNK(PIRACYPALETTE);

            VL_FadeOut(0, 255, 0, 0, 25, 20);
            VW_FadeOut();

            // Cleanup screen for upcoming SetPalette call
            //
            Uint16 old_bufferofs = bufferofs;

            bufferofs = displayofs;
            VL_Bar(0, 0, 320, 200, 0);
            bufferofs = old_bufferofs;
        }

// ---------------------
// Apogee presents
// ---------------------
        // Cache pic
        //
        CA_CacheScreen(APOGEEPIC);

        // Load and start music
        //
        CA_CacheAudioChunk(STARTMUSIC + APOGFNFM_MUS);

        ::SD_StartMusic(APOGFNFM_MUS);

        // Cache and set palette.  AND  Fade it in!
        //
        CA_CacheGrChunk(APOGEEPALETTE);
        VL_SetPalette(0, 256, static_cast<const Uint8*>(grsegs[APOGEEPALETTE]));
        VL_SetPaletteIntensity(0, 255, static_cast<const Uint8*>(grsegs[APOGEEPALETTE]), 0);
        VW_UpdateScreen();
        if (!::is_ps) {
            VL_FadeOut(0, 255, 0, 0, 0, 20);
        } else {
            VL_FadeOut(0, 255, 25, 29, 53, 20);
        }
        VL_FadeIn(0, 255, static_cast<const Uint8*>(grsegs[APOGEEPALETTE]), 30);

        // Wait for end of fanfare
        //
        if (MusicMode == smm_AdLib) {
            IN_StartAck();
            while ((!sqPlayedOnce) && (!IN_CheckAck())) {
            }
        } else {
            IN_UserInput(TickBase * 6);
        }

        SD_MusicOff();

        // Free palette and music.  AND  Restore palette
        //
        UNCACHEGRCHUNK(APOGEEPALETTE);

        delete [] audiosegs[STARTMUSIC + APOGFNFM_MUS];
        audiosegs[STARTMUSIC + APOGFNFM_MUS] = NULL;

        if (::is_ps()) {
            // Do A Blue Flash!
            VL_FadeOut(0, 255, 25, 29, 53, 20);
        }

        VL_FadeOut(0, 255, 0, 0, 0, 30);

// ---------------------
// JAM logo intro
// ---------------------
        // Load and start music
        //
        CA_CacheAudioChunk(STARTMUSIC + TITLE_LOOP_MUSIC);
        ::SD_StartMusic(TITLE_LOOP_MUSIC);

        // Show JAM logo
        //
        if (!DoMovie(mv_intro, 0)) {
            MAIN_ERROR(PREDEMO_NOJAM);
        }

        if (PowerBall) {
            Sint16 i;

            for (i = 0; i < 60 && (!DebugOk); i++) {
                VL_WaitVBL(1);

                // BBi
                ::in_handle_events();

                if (Keyboard[sc_left_shift] && Keyboard[sc_right_shift]) {
                    CA_LoadAllSounds();

                    SD_MusicOff();

                    ::sd_play_player_sound(SHOOTDOORSND, bstone::AC_ITEM);

                    SD_WaitSoundDone();

                    ClearMemory();
                    DebugOk = 1;

                    CA_CacheAudioChunk(STARTMUSIC + TITLE_LOOP_MUSIC);
                    ::SD_StartMusic(TITLE_LOOP_MUSIC);
                }
            }
        }

// ---------------------
// PC-13
// ---------------------
        VL_Bar(0, 0, 320, 200, 0x14);
        CacheDrawPic(0, 64, PC13PIC);
        VW_UpdateScreen();
        VW_FadeIn();
        IN_UserInput(TickBase * 2);

        // Do A Red Flash!

        VL_FadeOut(0, 255, 39, 0, 0, 20);
        VW_FadeOut();
    }
#endif // SKIP_TITLE_AND_CREDITS
}

// ------------------------------------------------------------------------
// InitGame()
// ------------------------------------------------------------------------
void InitGame()
{
    Sint16 i, x, y;
    Uint16* blockstart;
// long mmsize;

    MM_Startup(); // so the signon screen can be freed
#if IN_DEVELOPMENT || GEORGE_CHEAT || SHOW_CHECKSUM
    if (MS_CheckParm("checksum")) {
        ShowChecksums();
        MM_Shutdown();
        exit(0);
    }
#endif
    CA_Startup();

// Any problems with this version of the game?
//
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
    if (!MS_CheckParm("nochex"))
#endif

#if !SKIP_CHECKSUMS
    CheckValidity("MAPTEMP.", MAPTEMP_CHECKSUM);

#if GAME_VERSION != SHAREWARE_VERSION
    if (ChecksumFile("FILE_ID.DIZ", 0) != DIZFILE_CHECKSUM) {
        gamestate.flags |= GS_BAD_DIZ_FILE;
    }
#endif
#endif

    VL_SetVGAPlaneMode();
    VL_SetPalette(0, 256, vgapal);

    VW_Startup();
    IN_Startup();
    PM_Startup();
    SD_Startup();
    US_Startup();

    if (CheckForSpecialCode(POWERBALLTEXT))
#if IN_DEVELOPMENT
    { DebugOk = true;
    }
#else
    { PowerBall = true;
    }
#endif

    if (CheckForSpecialCode(TICSTEXT)) {
        gamestate.flags |= GS_TICS_FOR_SCORE;
    }

    if (CheckForSpecialCode(MUSICTEXT)) {
        gamestate.flags |= GS_MUSIC_TEST;
    }

    if (CheckForSpecialCode(RADARTEXT)) {
        gamestate.flags |= GS_SHOW_OVERHEAD;
    }

#if IN_DEVELOPMENT
    //
    // Clear Monocrome
    //
    _fmemset(MK_FP(0xb000, 0x0000), 0, 4000);
#endif

//
// build some tables
//
    InitDigiMap();

    for (i = 0; i < MAPSIZE; i++) {
        nearmapylookup[i] = &tilemap[0][0] + MAPSIZE * i;
        farmapylookup[i] = i * 64;
    }

    for (i = 0; i < PORTTILESHIGH; i++) {
        uwidthtable[i] = UPDATEWIDE * i;
    }

    blockstart = &blockstarts[0];
    for (y = 0; y < UPDATEHIGH; y++) {
        for (x = 0; x < UPDATEWIDE; x++) {
            *blockstart++ = SCREENWIDTH * 16 * y + x * TILEWIDTH;
        }
    }

    updateptr = &update[0];

    bufferofs = 0;
    displayofs = 0;
    ReadConfig();

//
// draw intro screen stuff
//
//      if (!(gamestate.flags & GS_QUICKRUN))
//              IntroScreen ();

//
// load in and lock down some basic chunks
//

    LoadFonts();

    LoadLatchMem();
    BuildTables(); // trig tables
    SetupWalls();
    NewViewSize();

//
// initialize variables
//
    InitRedShifts();
}

// -------------------------------------------------------------------------
// ShowSystem()
// -------------------------------------------------------------------------
void ShowSystem()
{
    char avail[2][8] = { "None", "Present" };

    MM_Startup();
    CA_Startup();
    IN_Startup();
    PM_Startup();
    SD_Startup();

    fprint(show_text1);
    fprint(show_text2);
    fprint(show_text3);

    fprint(show_text5);

    printf("        Mouse: %s\n", avail[MousePresent]);
    printf("     Joystick: %s\n", avail[JoysPresent[0] || JoysPresent[1]]);
    printf("        AdLib: %s\n", avail[AdLibPresent && !SoundBlasterPresent]);
    printf("Sound Blaster: %s\n", avail[SoundBlasterPresent]);
    printf(" Sound Source: %s\n\n", avail[SoundSourcePresent]);
    fprint(show_text2);

    SD_Shutdown();
    PM_Shutdown();
    IN_Shutdown();
    CA_Shutdown();
    MM_Shutdown();
}

// -------------------------------------------------------------------------
// scan_atoi()
// -------------------------------------------------------------------------
Uint16 scan_atoi(
    const char* s)
{
    while (*s && (!isdigit(*s))) { // First scans for a digit...
        s++;
    }

    return static_cast<Uint16>(atoi(s)); // Then converts to integer...
}

extern const char* MainStrs[];
extern Sint16 starting_episode, starting_level, starting_difficulty;


// -------------------------------------------------------------------------
// freed_main()
// -------------------------------------------------------------------------
void freed_main()
{
// Setup for APOGEECD thingie.
//
    InitDestPath();

// Make sure there's room to play the game
//
    CheckDiskSpace(DISK_SPACE_NEEDED, CANT_PLAY_TXT, cds_dos_print);

// Which version is this? (SHAREWARE? 1-3? 1-6?)
//
    CheckForEpisodes();

    // BBi
    ::initialize_sprites();
    ::initialize_gfxv_contants();
    ::initialize_states();
    ::initialize_shape_table();
    ::initialize_audio_constants();
    ::initialize_songs();
    ::initialize_static_info_constants();
    ::initialize_weapon_constants();
    ::initialize_grenade_shape_constants();
    ::initialize_static_health_table();
    ::initialize_boss_constants();
    ::initialize_messages();
    ::initialize_ca_constants();
    ::gamestuff.initialize();

    std::string arg;

    switch (::g_args.check_argument(MainStrs, arg)) {
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
    case 0: // quick run
        gamestate.flags |= GS_QUICKRUN;

    case 1: // no wait
        gamestate.flags |= GS_NOWAIT;
        break;
#endif

    case 2: // starting level
        gamestate.flags |= GS_STARTLEVEL;
        starting_level = scan_atoi(arg.c_str());
        break;

    case 3:
        gamestate.flags |= GS_STARTLEVEL;
        starting_episode = scan_atoi(arg.c_str()) - 1;
        break;

    case 4:
        fprint(cinfo_text);

        printf("\n"
               "     Version: %s\n"
               "COMPILE DATE: %s\n\n",
               __BLAKE_VERSION__, __DATE__);
        exit(0);
        break;

    case 5:
        ShowSystem();
        exit(0);
        break;

#if IN_DEVELOPMENT
#ifdef DEBUG_VALUE
    case 6:
        debug_value = scan_atoi(g_argv[i]);
        break;
#endif
#endif

    case 7:
        gamestate.flags |= GS_TICS_FOR_SCORE;
        break;

    case 8:
// gamestate.flags |= GS_MEM_FOR_SCORE;
        break;

    case 9:
        PowerBall = 1;
        break;

    case 11:
        gamestate.flags |= GS_STARTLEVEL;
        starting_difficulty = scan_atoi(arg.c_str()) - 1;
        break;

    case 10:
        gamestate.flags |= GS_MUSIC_TEST;
        break;

    case 12:
        gamestate.flags |= GS_SHOW_OVERHEAD;
        break;
    }


#if BETA_TEST
    //
    // THIS IS FOR BETA ONLY!
    //

    _dos_getdate(&d);
    if ((d.year > BETA_YEAR) ||
        ((d.year == BETA_YEAR) && (d.month > BETA_MONTH)) ||
        ((d.year == BETA_YEAR) && (d.month == BETA_MONTH) && (d.day >= BETA_DAY)))
    {
        FILE* out;
        char name[20] = "VSWAP.";

        strcat(name, extension);
        out = fopen(name, "w");
        fprintf(out, "\n\n SELF DESTRUCTED \n");
        fclose(out);
        remove("vswap.bs1");
        fprint(dver_text);
        exit(0);
    }
#endif

    // BBi
    if (::g_args.find_option("cheats") >= 0) {
        DebugOk = true;
    }

    InitGame();

    bufferofs = SCREENSIZE;

    PreDemo();
}


// -------------------------------------------------------------------------
// CheckValidity()
// -------------------------------------------------------------------------
void CheckValidity(
    char* file,
    Sint32 valid_checksum)
{
    std::string filename;
    Sint32 checksum;

    if (::strlen(file) > 9) {
        MAIN_ERROR(CHECK_FILENAME_TOO_LONG);
    }

    filename = file + ::extension;

    checksum = ::ChecksumFile(filename, 0);
    if (checksum != valid_checksum)
#if GAME_VERSION != SHAREWARE_VERSION
    { if (::strstr(file, "MAP")) {
          InvalidLevels();
      }
    }
#else
    { BadChecksum();
    }
#endif
}

#define CFC_BUFFERSIZE 65535

char* cfc_buffer;

// -------------------------------------------------------------------------
// ChecksumFile()
// -------------------------------------------------------------------------
Sint32 ChecksumFile(
    const std::string& file,
    Sint32 checksum)
{
#define JUMPSIZE 8

    bstone::FileStream handle;
    Sint32 size, readlen, i;
    char* p;

    cfc_buffer = new char[CFC_BUFFERSIZE];
    p = cfc_buffer;

    handle.open(file);

    if (!handle.is_open()) {
        checksum = 0;
        goto exit_func;
    }

    size = static_cast<Sint32>(handle.get_size());
    while (size) {
        if (size >= CFC_BUFFERSIZE) {
            readlen = CFC_BUFFERSIZE;
        } else {
            readlen = size;
        }

        handle.read(cfc_buffer, readlen);

        for (i = 0; i < readlen - JUMPSIZE; i += JUMPSIZE) {
            checksum += p[i] ^ p[i + 1];
        }

        size -= readlen;
    }

exit_func:;

    delete [] cfc_buffer;
    cfc_buffer = NULL;

    return checksum;
}

#if IN_DEVELOPMENT || GEORGE_CHEAT || SHOW_CHECKSUM


// -------------------------------------------------------------------------
// ShowChecksums()
// -------------------------------------------------------------------------
void ShowChecksums()
{
#if GAME_VERSION == SHAREWARE_VERSION
    char mapname[23] = "SWMAPS\\MAPTEMP.";
#elif GAME_VERSION == MISSIONS_1_THR_3
    char mapname[23] = "1_3MAPS\\MAPTEMP.";
#else
    char mapname[13] = "MAPTEMP.";
#endif
    char gfxname[13] = "VGAGRAPH.";
    char audioname[13] = "AUDIOT.";
    char dizname[] = "FILE_ID.DIZ";

    strcat(gfxname, extension);
    strcat(mapname, extension);
    strcat(audioname, extension);

    printf("\n");
    printf("%s: %lx\n", audioname, ChecksumFile(audioname, 0));
    printf("%s: %lx\n", mapname, ChecksumFile(mapname, 0));
    printf("%s: %lx\n", gfxname, ChecksumFile(gfxname, 0));
    printf("%s: %lx\n", dizname, ChecksumFile(dizname, 0));
    printf("\n");
}


#endif

#if GAME_VERSION != SHAREWARE_VERSION

// -------------------------------------------------------------------------
// InvalidLevels()
// -------------------------------------------------------------------------
void InvalidLevels()
{
    char* chunkptr;

    CA_CacheGrChunk(BADLEVELSTEXT);
    chunkptr = static_cast<char*>(grsegs[BADLEVELSTEXT]);
    *(strstr(chunkptr, "^XX")) = 0;

    fprint(chunkptr);

    UNCACHEGRCHUNK(BADLEVELSTEXT);
}

#endif

#if GAME_VERSION == SHAREWARE_VERSION

// -------------------------------------------------------------------------
// BadChecksum()
// -------------------------------------------------------------------------
void BadChecksum()
{
    char* chunkptr;

    CA_CacheGrChunk(BADCHECKSUMTEXT);
    chunkptr = grsegs[BADCHECKSUMTEXT];
    *(_fstrstr(chunkptr, "^XX")) = 0;

    CA_Shutdown();
    MM_Shutdown();

    fprint(chunkptr);

    exit(0);
}

#endif


#if FREE_FUNCTIONS

// This function is used as a label for the end of the
// functions used by the memory manager.
//
void JM_FREE_END()
{
}

#endif
