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


//
// Warning!
//
// All functions in this source file are designated usable by the memory
// manager after program initialization.
//


#include "3d_def.h"


extern int* spanstart;
extern int* stepscale;
extern int* basedist;
extern int* planeylookup;
extern int* mirrorofs;


void CA_CannotOpen(
    const std::string& string);

void CAL_GetGrChunkLength(
    int16_t chunk);

void CA_CacheScreen(
    int16_t chunk);

void VH_UpdateScreen();
void IN_StartAck();
bool IN_CheckAck();
void OpenMapFile();
void CloseMapFile();
void ClearMemory();
void ShutdownId();
void InitRedShifts();

void CAL_OptimizeNodes(
    huffnode* table);

void OpenAudioFile();
void ReadConfig();
void read_high_scores();

void initialize_sprites();
void initialize_states();
void initialize_audio_constants();
void initialize_songs();
void initialize_gfxv_contants();
void initialize_tp_shape_table();
void initialize_tp_animation_table();
void initialize_static_info_constants();
void initialize_weapon_constants();
void initialize_grenade_shape_constants();
void initialize_static_health_table();
void initialize_boss_constants();
void initialize_messages();
void initialize_ca_constants();

void SDL_SetupDigi();


static uint8_t wolfdigimap[] = {
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
    PRODEATH3SND, 43, // PRO Death #3
    SWATDEATH3SND, 52, // Swat Guard #3
    SCIDEATH3SND, 54, // Gen. Sci Death #3

    LCANBREAKSND, 44,
    SCANBREAKSND, 45,
    CLAWATTACKSND, 56,
    SPITATTACKSND, 55,
    PUNCHATTACKSND, 57,

    LASTSOUND,
};


extern const uint8_t colormap[16896];
const uint8_t* lightsource;


// ------------------ ID Software 'startup' functions ---------------------


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
    int16_t i;
    float angle, anglestep;
    double tang;
    fixed value;


//
// calculate fine tangents
//

    for (i = 0; i < FINEANGLES / 8; i++) {
        tang = tan((i + 0.5) / radtoint);
        finetangent[i] = static_cast<int32_t>(tang * TILEGLOBAL);
        finetangent[FINEANGLES / 4 - 1 - i] = static_cast<int32_t>(1 / tang * TILEGLOBAL);
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

// Map tile values to scaled pics
void SetupWalls()
{
    int16_t i;

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

void InitDigiMap()
{
    char* map;

    for (map = reinterpret_cast<char*>(wolfdigimap); *map != LASTSOUND; map += 2) {
        DigiMap[static_cast<int>(map[0])] = map[1];
    }
}

void CAL_SetupAudioFile()
{
    bstone::FileStream handle;
    int32_t length;
    std::string fname;

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
    fname = ::data_dir + ::aheadname + ::extension;

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
    audiostarts = (int32_t*)FP_SEG(&audiohead);
#endif

//
// open the data file
//
    ::OpenAudioFile();
}

void CAL_SetupGrFile()
{
    std::string fname;
    bstone::FileStream handle;
    uint8_t* compseg;

    //
    // load ???dict.ext (huffman dictionary for graphics files)
    //

    fname = ::data_dir + ::gdictname + ::extension;

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

    fname = ::data_dir + ::gheadname + ::extension;

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
    compseg = new uint8_t[::chunkcomplen];
    ::grhandle.read(compseg, ::chunkcomplen);

    ::CAL_HuffExpand(
        compseg,
        reinterpret_cast<uint8_t*>(pictable),
        NUMPICS * sizeof(pictabletype),
        ::grhuffman);

    delete [] compseg;
}

void CAL_SetupMapFile()
{
    int16_t i;
    bstone::FileStream handle;
    int32_t pos;
    std::string fname;
    mapfiletype header;
    maptype* map_header;

    //
    // load maphead.ext (offsets and tileinfo for map file)
    //

    fname = ::data_dir + ::mheadname + ::extension;

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
        mapsegs[i] = new uint16_t[64 * 64];
    }

    CloseMapFile();
}


// --------------------- Other general functions ------------------------

extern CP_itemtype NewEmenu[];
extern int16_t EpisodeSelect[];


namespace {


int get_ref_vgahead_offset_count(
    GameType game_type)
{
    switch (game_type) {
    case GameType::aog_sw:
        return 213;

    case GameType::aog_full_v1_0:
        return 213;

    case GameType::aog_full_v2_x:
        return 224;

    case GameType::aog_full_v3_0:
        return 226;

    case GameType::ps:
        return 249;

    default:
        throw std::invalid_argument("Invalid game type.");
    }
}

const bstone::StringList& get_file_names(
    GameType game_type)
{
    static const bstone::StringList aog_sw_file_names = {
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

    static const bstone::StringList aog_full_file_names = {
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

    static const bstone::StringList ps_file_names = {
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


    switch (game_type) {
    case GameType::aog_sw:
        return aog_sw_file_names;

    case GameType::aog_full_v1_0:
    case GameType::aog_full_v2_x:
    case GameType::aog_full_v3_0:
        return aog_full_file_names;

    case GameType::ps:
        return ps_file_names;

    default:
        throw std::invalid_argument("Invalid game type.");
    }
}

const std::string& get_file_extension(
    GameType game_type)
{
    static const std::string aog_sw_extension = "BS1";
    static const std::string aog_full_extension = "BS6";
    static const std::string ps_extension = "VSI";

    switch (game_type) {
    case GameType::aog_sw:
        return aog_sw_extension;

    case GameType::aog_full_v1_0:
    case GameType::aog_full_v2_x:
    case GameType::aog_full_v3_0:
        return aog_full_extension;

    case GameType::ps:
        return ps_extension;

    default:
        throw std::invalid_argument("Invalid game type.");
    }
}

bool are_files_exist(
    GameType game_type)
{
    auto&& file_names = get_file_names(game_type);

    for (const auto& file_name : file_names) {
        auto file_path = ::data_dir + file_name;

        if (!bstone::FileStream::is_exists(file_path)) {
            return false;
        }
    }

    return true;
}

int get_vgahead_offset_count(
    GameType game_type)
{
    auto file_name = "VGAHEAD." + get_file_extension(game_type);

    bstone::FileStream stream(::data_dir + file_name);

    if (!stream.is_open()) {
        return 0;
    }

    auto file_size = stream.get_size();

    if ((file_size % FILEPOSSIZE) != 0) {
        return 0;
    }

    return static_cast<int>(file_size / FILEPOSSIZE);
}

bool check_vgahead_offset_count(
    GameType game_type)
{
    auto offset_count = get_vgahead_offset_count(game_type);
    return offset_count == get_ref_vgahead_offset_count(game_type);
}

static bool set_game_type(
    GameType game_type)
{
    if (!are_files_exist(game_type)) {
        return false;
    }

    if (!check_vgahead_offset_count(game_type)) {
        return false;
    }

    ::g_game_type = game_type;

    return true;
}


} // namespace


void CheckForEpisodes()
{
    bool is_succeed = true;

    ::g_game_type = GameType::none;

    if (::g_args.has_option("aog_sw")) {
        bstone::Log::write("Forcing Aliens Of Gold (shareware, v3.0).\n");

        if (!::set_game_type(GameType::aog_sw)) {
            is_succeed = false;
        }
    } else if (::g_args.has_option("aog_10")) {
        bstone::Log::write("Forcing Aliens Of Gold (full, v1.0).\n");

        if (!::set_game_type(GameType::aog_full_v1_0)) {
            is_succeed = false;
        }
    } else if (::g_args.has_option("aog_2x")) {
        bstone::Log::write("Forcing Aliens Of Gold (full, v2.x).\n");

        if (!::set_game_type(GameType::aog_full_v2_x)) {
            is_succeed = false;
        }
    } else if (::g_args.has_option("aog_30")) {
        bstone::Log::write("Forcing Aliens Of Gold (full, v3.0).\n");

        if (!::set_game_type(GameType::aog_full_v3_0)) {
            is_succeed = false;
        }
    } else if (::g_args.has_option("ps")) {
        bstone::Log::write("Forcing Planet Strike (full, v1.0/v1.1).\n");

        if (!::set_game_type(GameType::ps)) {
            is_succeed = false;
        }
    } else {
        bool is_found = false;

        bstone::Log::write("Searching for data files...");

        if (!is_found) {
            is_found = ::set_game_type(GameType::aog_full_v3_0);
        }

        if (!is_found) {
            is_found = ::set_game_type(GameType::aog_full_v2_x);
        }

        if (!is_found) {
            is_found = ::set_game_type(GameType::aog_full_v1_0);
        }

        if (!is_found) {
            is_found = ::set_game_type(GameType::ps);
        }

        if (!is_found) {
            is_found = ::set_game_type(GameType::aog_sw);
        }

        if (is_found) {
            switch (::g_game_type) {
            case GameType::aog_sw:
                bstone::Log::write("Found Aliens Of Gold (shareware, v3.0).\n");
                break;

            case GameType::aog_full_v1_0:
                bstone::Log::write("Found Aliens Of Gold (full, v1.0).\n");
                break;

            case GameType::aog_full_v2_x:
                bstone::Log::write("Found Aliens Of Gold (full, v2.x).\n");
                break;

            case GameType::aog_full_v3_0:
                bstone::Log::write("Found Aliens Of Gold (full, v3.0).\n");
                break;

            case GameType::ps:
                bstone::Log::write("Found Planet Strike (v1.x).\n");
                break;

            default:
                is_succeed = false;
                break;
            }
        } else {
            is_succeed = false;
        }
    }

    if (!is_succeed) {
        ::Quit("Data files not found.");
    }


    ::extension = ::get_file_extension(::g_game_type);

    for (int i = 0; i < mv_NUM_MOVIES; ++i) {
        ::movies[i].file_name += ::extension;
    }

    if (::is_aog_full()) {
        for (int i = 1; i < 6; ++i) {
            ::NewEmenu[i].active = AT_ENABLED;
            ::EpisodeSelect[i] = 1;
        }
    }

    ::PageFileName += ::extension;
    ::audioname += ::extension;
}


extern const char* MainStrs[];
extern char bc_buffer[];


void PreDemo()
{
    if (::no_screens) {
        return;
    }

    VL_SetPaletteIntensity(0, 255, vgapal, 0);

    if (!(gamestate.flags & GS_NOWAIT)) {
        if (::is_aog_full()) {
            // ---------------------
            // Anti-piracy screen
            // ---------------------

            // Cache pic
            //
            CA_CacheScreen(PIRACYPIC);

            // Cache and set palette.  AND  Fade it in!
            //
            CA_CacheGrChunk(PIRACYPALETTE);
            VL_SetPalette(0, 256, static_cast<const uint8_t*>(grsegs[PIRACYPALETTE]));
            VL_SetPaletteIntensity(0, 255, static_cast<const uint8_t*>(grsegs[PIRACYPALETTE]), 0);
            VW_UpdateScreen();

            VL_FadeOut(0, 255, 0, 0, 25, 20);
            VL_FadeIn(0, 255, static_cast<const uint8_t*>(grsegs[PIRACYPALETTE]), 30);

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
            ::VL_Bar(0, 0, 320, 200, 0);
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
        VL_SetPalette(0, 256, static_cast<const uint8_t*>(grsegs[APOGEEPALETTE]));
        VL_SetPaletteIntensity(0, 255, static_cast<const uint8_t*>(grsegs[APOGEEPALETTE]), 0);
        VW_UpdateScreen();
        if (::is_aog()) {
            VL_FadeOut(0, 255, 0, 0, 0, 20);
        } else {
            VL_FadeOut(0, 255, 25, 29, 53, 20);
        }
        VL_FadeIn(0, 255, static_cast<const uint8_t*>(grsegs[APOGEEPALETTE]), 30);

        // Wait for end of fanfare
        //
        if (::sd_is_music_enabled) {
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
        audiosegs[STARTMUSIC + APOGFNFM_MUS] = nullptr;

        if (::is_ps()) {
            // Do A Blue Flash!
            ::VL_FadeOut(0, 255, 25, 29, 53, 20);
        } else {
            ::VL_FadeOut(0, 255, 0, 0, 0, 30);
        }

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
            ::Quit("JAM animation (IANIM.xxx) does not exist.");
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

        if (::is_aog()) {
            ::VL_FadeOut(0, 255, 39, 0, 0, 20);
        } else {
            ::VL_FadeOut(0, 255, 0, 0, 0, 20);
        }

        VW_FadeOut();
    }
}

void InitGame()
{
    int16_t i, x, y;
    uint16_t* blockstart;

    CA_Startup();

    VL_SetVGAPlaneMode();
    VL_SetPalette(0, 256, vgapal);

    VW_Startup();
    IN_Startup();
    PM_Startup();
    SD_Startup();
    US_Startup();


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

    ::ReadConfig();
    ::read_high_scores();


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

uint16_t scan_atoi(
    const char* s)
{
    while (*s && (!isdigit(*s))) { // First scans for a digit...
        s++;
    }

    return static_cast<uint16_t>(atoi(s)); // Then converts to integer...
}


extern const char* MainStrs[];
extern int16_t starting_episode, starting_level, starting_difficulty;


void freed_main()
{
    if (::g_args.has_option("version")) {
        bstone::Log::write_version();
        ::Quit();
    }

    // Setup for APOGEECD thingie.
    //
    ::InitDestPath();

    // Make sure there's room to play the game
    //
    ::CheckDiskSpace(DISK_SPACE_NEEDED, CANT_PLAY_TXT, cds_dos_print);

    // Which version is this? (SHAREWARE? 1-3? 1-6?)
    //
    ::CheckForEpisodes();

    // BBi
    ::initialize_sprites();
    ::initialize_gfxv_contants();
    ::initialize_states();
    ::initialize_tp_shape_table();
    ::initialize_tp_animation_table();
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

    if (::g_args.has_option("no_screens")) {
        ::no_screens = true;
    }

    if (::g_args.has_option("cheats")) {
        ::DebugOk = true;
    }

    ::InitGame();

    ::PreDemo();
}
