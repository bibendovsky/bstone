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


//
//      ID Engine
//      ID_SD.c - Sound Manager for Wolfenstein 3D
//      v1.3 (revised for **********, screwed with for Blake Stone)
//      By Jason Blochowiak
//

//
//      This module handles dealing with generating sound on the appropriate
//              hardware
//
//      Depends on: User Mgr (for parm checking)
//
//      Globals:
//              For User Mgr:
//                      SoundSourcePresent - Sound Source thingie present?
//                      SoundBlasterPresent - SoundBlaster card present?
//                      AdLibPresent - AdLib card present?
//                      SoundMode - What device is used for sound effects
//                              (Use SM_SetSoundMode() to set)
//                      MusicMode - What device is used for music
//                              (Use SM_SetMusicMode() to set)
//                      DigiMode - What device is used for digitized sound effects
//                              (Use SM_SetDigiDevice() to set)
//
//              For Cache Mgr:
//                      NeedsDigitized - load digitized sounds?
//                      NeedsMusic - load music?
//


#ifdef  _MUSE_ // Will be defined in ID_Types.h
#include "id_sd.h"
#else
#include "id_heads.h"
#endif


// BBi
#include "3d_def.h"
#include "bstone_audio_mixer.h"
#include "bstone_memory_binary_reader.h"
// BBi


#define SDL_SoundFinished() { SoundNumber = HITWALLSND; SoundPriority = 0; }

#if 0
//      Imports from ID_SDD.C
#undef  NUMSOUNDS
#undef  NUMSNDCHUNKS
#undef  STARTPCSOUNDS
#undef  STARTADLIBSOUNDS
#undef  STARTDIGISOUNDS
#undef  STARTMUSIC
#endif // 0

extern uint16_t sdStartPCSounds;
extern uint16_t sdStartALSounds;
extern int16_t sdLastSound;
extern int16_t DigiMap[];


//      Global variables
boolean SoundSourcePresent,
        AdLibPresent,
        SoundBlasterPresent, SBProPresent,
        NeedsDigitized, NeedsMusic,
        SoundPositioned;
SDMode SoundMode;
SMMode MusicMode;
SDSMode DigiMode;
std::atomic_uint32_t TimeCount;
uint16_t HackCount;

uint8_t** SoundTable;

//      Internal variables
static boolean SD_Started;
boolean nextsoundpos;
uint32_t TimerDivisor, TimerCount;
static const char* ParmStrings[] = {
    "noal",
    "nosb",
    "nopro",
    "noss",
    "sst",
    "ss1",
    "ss2",
    "ss3",
    nullptr
};
int SoundNumber;
int DigiNumber;
uint16_t SoundPriority, DigiPriority;
int16_t LeftPosition, RightPosition;
int32_t LocalTime;
uint16_t TimerRate;

uint16_t* DigiList;

//      SoundBlaster variables
static boolean sbNoCheck, sbNoProCheck;

//      SoundSource variables
boolean ssNoCheck;

//      PC Sound variables

//      AdLib variables
boolean alNoCheck;
boolean sqActive;
uint16_t* sqHack;
uint16_t sqHackLen;
boolean sqPlayedOnce;

//      Internal routines
void SDL_DigitizedDone();

// BBi
static int music_index = -1;
static bstone::AudioMixer mixer;

int g_sfx_volume = MAX_VOLUME;
int g_music_volume = MAX_VOLUME;

//
//      Stuff for digitized sounds
//

void SD_SetDigiDevice(
    SDSMode mode)
{
    if (mode == DigiMode) {
        return;
    }

    switch (mode) {
    case sds_SoundBlaster:
        DigiMode = sds_SoundBlaster;
        break;

    default:
        DigiMode = mode;
    }
}

void SDL_SetupDigi()
{
    const uint16_t* p;
    int pg;
    int i;

    p = static_cast<const uint16_t*>(PM_GetPage(ChunksInFile - 1));
    pg = PMSoundStart;
    for (i = 0; i < static_cast<int>(PMPageSize / (2 * 2)); ++i) {
        if (pg >= ChunksInFile - 1) {
            break;
        }
        pg += (bstone::Endian::le(p[1]) + (PMPageSize - 1)) / PMPageSize;
        p += 2;
    }
    DigiList = new uint16_t[i * 2];

    const uint16_t* src_list = static_cast<const uint16_t*>(
        ::PM_GetPage(ChunksInFile - 1));

    bstone::Endian::le(src_list, i * 2, DigiList);

    for (i = 0; i < sdLastSound; i++) {
        DigiMap[i] = -1;
    }
}

//      AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//      SDL_DetectAdLib() - Determines if there's an AdLib (or SoundBlaster
//              emulating an AdLib) present
//
///////////////////////////////////////////////////////////////////////////

static boolean SDL_DetectAdLib()
{
    int sdl_result = 0;

    sdl_result = ::SDL_Init(SDL_INIT_AUDIO);

    return sdl_result == 0;
}

////////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////

static void SDL_ShutDevice()
{
    SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////

static void SDL_StartDevice()
{
}

//      Public routines

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetSoundMode(
    SDMode mode)
{
    boolean result = false;
    uint16_t tableoffset = 0;

    SD_StopSound();

#ifndef _MUSE_
    if ((mode == sdm_AdLib) && !AdLibPresent) {
        mode = sdm_Off;
    }

    switch (mode) {
    case sdm_Off:
        tableoffset = sdStartPCSounds;
        NeedsDigitized = false;
        result = true;
        break;

    case sdm_AdLib:
        if (AdLibPresent) {
            tableoffset = sdStartALSounds;
            NeedsDigitized = false;
            result = true;
        }
        break;
    }
#else
    result = true;
#endif

    if (result && (mode != SoundMode)) {
        SDL_ShutDevice();
        SoundMode = mode;
#ifndef _MUSE_
        SoundTable = &audiosegs[tableoffset];
#endif
        SDL_StartDevice();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetMusicMode(
    SMMode mode)
{
    boolean result = false;

    SD_MusicOff();

    switch (mode) {
    case smm_Off:
        result = true;

    case smm_AdLib:
        result = AdLibPresent;
    }

    if (result) {
        MusicMode = mode;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Startup() - starts up the Sound Mgr
//              Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void SD_Startup()
{
    if (SD_Started) {
        return;
    }

    ssNoCheck = false;
    alNoCheck = false;
    sbNoCheck = false;
    sbNoProCheck = false;

#ifndef _MUSE_
    switch (::g_args.check_argument(ParmStrings)) {
    case 0: // No AdLib detection
        alNoCheck = true;
        break;

    case 1: // No SoundBlaster detection
        sbNoCheck = true;
        break;

    case 2: // No SoundBlaster Pro detection
        sbNoProCheck = true;
        break;
    }
#endif

    LocalTime = 0;
    TimeCount = 0;

    SD_SetSoundMode(sdm_Off);
    SD_SetMusicMode(smm_Off);

    SoundSourcePresent = false;

    AdLibPresent = false;
    SoundBlasterPresent = false;

    if (!alNoCheck) {
        AdLibPresent = SDL_DetectAdLib();

        if (AdLibPresent && !sbNoCheck) {
            SoundBlasterPresent = true;
        }
    }

    if (AdLibPresent) {
        mixer.initialize(44100);
    } else {
        mixer.uninitialize();
    }

    SDL_SetupDigi();

    SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//              the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void SD_Default(
    boolean gotit,
    SDMode sd,
    SMMode sm)
{
    boolean gotsd, gotsm;

    gotsd = gotsm = gotit;

    if (gotsd) { // Make sure requested sound hardware is available
        switch (sd) {
        case sdm_AdLib:
            gotsd = AdLibPresent;
            break;
        default:
            break;
        }
    }
    if (!gotsd) {
        if (AdLibPresent) {
            sd = sdm_AdLib;
        } else {
            sd = sdm_Off;
        }
    }
    if (sd != SoundMode) {
        SD_SetSoundMode(sd);
    }


    if (gotsm) { // Make sure requested music hardware is available
        switch (static_cast<SDMode>(sm)) {
        case sdm_AdLib:
            gotsm = AdLibPresent;
            break;
        default:
            break;
        }
    }
    if (!gotsm) {
        if (AdLibPresent) {
            sm = smm_AdLib;
        }
    }
    if (sm != MusicMode) {
        SD_SetMusicMode(sm);
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Shutdown() - shuts down the Sound Mgr
//              Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////

void SD_Shutdown()
{
    if (!SD_Started) {
        return;
    }

    mixer.uninitialize();

    // Free music data
    for (int i = 0; i < LASTMUSIC; ++i) {
        delete [] static_cast<uint8_t*>(::audiosegs[STARTMUSIC + i]);
    }

    SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//              no sound is playing
//
///////////////////////////////////////////////////////////////////////////

bool SD_SoundPlaying()
{
    switch (SoundMode) {
    case sdm_AdLib:
        return mixer.is_any_sfx_playing();

    default:
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////

void SD_StopSound()
{
    mixer.stop_all_sfx();
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////

void SD_WaitSoundDone()
{
    while (::SD_SoundPlaying()) {
        ::SDL_Delay(1);
    }
}


///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////

void SD_MusicOn()
{
    ::sqActive = true;
    mixer.play_adlib_music(music_index, sqHack, sqHackLen);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////

void SD_MusicOff()
{
    ::sqActive = false;
    mixer.stop_music();
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////

void SD_StartMusic(
    int index)
{
    SD_MusicOff();

    sqPlayedOnce = false;

    if (MusicMode == smm_AdLib) {
        music_index = index;

        uint16_t* music_data = reinterpret_cast<uint16_t*>(
            audiosegs[STARTMUSIC + index]);

        int length = bstone::Endian::le(music_data[0]) + 2;

        sqHack = music_data;
        sqHackLen = static_cast<uint16_t>(length);

        SD_MusicOn();
    } else {
        sqPlayedOnce = true;
    }
}

// BBi
void sd_play_sound(
    int sound_index,
    const void* actor,
    bstone::ActorType actor_type,
    bstone::ActorChannel actor_channel)
{
    if (sound_index < 0) {
        return;
    }

    if (!SoundTable) {
        return;
    }

    int actor_index = -1;

    if (actor) {
        switch (actor_type) {
        case bstone::AT_ACTOR:
            actor_index = static_cast<int>(
                static_cast<const objtype*>(actor) - objlist);
            break;

        case bstone::AT_DOOR:
            actor_index = static_cast<int>(
                static_cast<const doorobj_t*>(actor) - doorobjlist);
            break;

        default:
            return;
        }
    }

    const SoundCommon* sound = reinterpret_cast<SoundCommon*>(
        SoundTable[sound_index]);

    if (!sound) {
        return;
    }

    if (SoundMode != sdm_Off && !sound) {
        SD_ERROR(SD_PLAYSOUND_UNCACHED);
    }

    int priority = bstone::Endian::le(sound->priority);

    int digi_index = DigiMap[sound_index];

    if (DigiMode != sds_Off && digi_index != -1) {
        int digi_page = DigiList[(2 * digi_index) + 0];
        int digi_length = DigiList[(2 * digi_index) + 1];
        const void* digi_data = ::PM_GetSoundPage(digi_page);

        mixer.play_pcm_sound(digi_index, priority, digi_data, digi_length,
                             actor_index, actor_type, actor_channel);

        return;
    }

    if (SoundMode == sdm_Off) {
        return;
    }

    switch (SoundMode) {
    case sdm_AdLib:
        break;

    default:
        return;
    }

    int data_size = audiostarts[sdStartALSounds + sound_index + 1] -
                    audiostarts[sdStartALSounds + sound_index];

    mixer.play_adlib_sound(sound_index, priority, sound, data_size,
                           actor_index, actor_type, actor_channel);
}

void sd_play_actor_sound(
    int sound_index,
    const objtype* actor,
    bstone::ActorChannel actor_channel)
{
    sd_play_sound(
        sound_index,
        actor,
        bstone::AT_ACTOR,
        actor_channel);
}

void sd_play_player_sound(
    int sound_index,
    bstone::ActorChannel actor_channel)
{
    sd_play_sound(
        sound_index,
        player,
        bstone::AT_ACTOR,
        actor_channel);
}

void sd_play_door_sound(
    int sound_index,
    const doorobj_t* door)
{
    sd_play_sound(
        sound_index,
        door,
        bstone::AT_DOOR,
        bstone::AC_VOICE);
}

void sd_play_wall_sound(
    int sound_index)
{
    sd_play_sound(
        sound_index,
        nullptr,
        bstone::AT_WALL,
        bstone::AC_VOICE);
}

void sd_update_positions()
{
    mixer.update_positions();
}

bool sd_is_player_channel_playing(
    bstone::ActorChannel channel)
{
    return mixer.is_player_channel_playing(channel);
}

void sd_set_sfx_volume(
    int volume)
{
    if (volume < MIN_VOLUME) {
        volume = MIN_VOLUME;
    }

    if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }

    mixer.set_sfx_volume(static_cast<float>(volume) / MAX_VOLUME);
}

void sd_set_music_volume(
    int volume)
{
    if (volume < MIN_VOLUME) {
        volume = MIN_VOLUME;
    }

    if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }

    mixer.set_music_volume(static_cast<float>(volume) / MAX_VOLUME);
}
// BBi
