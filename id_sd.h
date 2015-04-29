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
//      ID_SD.h - Sound Manager Header
//      Version for Wolfenstein
//      By Jason Blochowiak
//


#ifndef BSTONE_ID_SD_INCLUDED
#define BSTONE_ID_SD_INCLUDED


// BBi
#include <cstdint>
#include "bstone_audio_mixer.h"


struct objtype;
struct doorobj_t;

void alOut(
    uint8_t n,
    uint8_t b);

#ifdef  __DEBUG__
#define __DEBUG_SoundMgr__
#endif

#define TickBase 70 // 70Hz per tick - used as a base for timer 0

enum SDMode {
    sdm_Off,
    sdm_PC,
    sdm_AdLib
}; // SDMode

enum SMMode {
    smm_Off,
    smm_AdLib
}; // SMMode

enum SDSMode {
    sds_Off,
    sds_PC,
    sds_SoundSource,
    sds_SoundBlaster
}; // SDSMode

struct SoundCommon {
    uint32_t length;
    uint16_t priority;
}; // SoundCommon

//      PC Sound stuff
#define pcTimer 0x42
#define pcTAccess 0x43
#define pcSpeaker 0x61

#define pcSpkBits 3

struct PCSound {
    SoundCommon common;
    uint8_t data[1];
}; // PCSound

//      Registers for the Sound Blaster card - needs to be offset by n0 (0x10,0x20,0x30,0x40,0x50,0x60)
#define sbReset 0x206 // W
#define sbFMStatus 0x208 // R
#define sbFMAddr 0x208 // W
#define sbFMData 0x209 // W
#define sbReadData 0x20a // R
#define sbWriteCmd 0x20c // W
#define sbWriteData 0x20c // W
#define sbWriteStat 0x20c // R
#define sbDataAvail 0x20e // R

//      Registers for the Sound Blaster Pro card - needs to be offset by n0 (0x20 or 0x40)
#define sbpLFMStatus 0x200 // R
#define sbpLFMAddr 0x200 // W
#define sbpLFMData 0x201 // W
#define sbpRFMStatus 0x202 // R
#define sbpRFMAddr 0x202 // W
#define sbpRFMData 0x203 // W
#define sbpMixerAddr 0x204 // W
#define sbpMixerData 0x205 // RW
#define sbpCDData 0x210 // R
#define sbpCDCommand 0x210 // W
#define sbpCDStatus 0x211 // R
#define sbpCDReset 0x212 // W

//      SBPro Mixer addresses
#define sbpmReset 0x00
#define sbpmVoiceVol 0x04
#define sbpmMicMix 0x0a
#define sbpmFilterADC 0x0c
#define sbpmControl 0x0e
#define sbpmMasterVol 0x22
#define sbpmFMVol 0x26
#define sbpmCDVol 0x28
#define sbpmLineVol 0x2e

struct SampledSound {
    SoundCommon common;
    uint16_t hertz;
    uint8_t bits;
    uint8_t reference;
    uint8_t data[1];
}; // SampledSound

//      Registers for the AdLib card
#define alFMStatus 0x388 // R
#define alFMAddr 0x388 // W
#define alFMData 0x389 // W

//      Register addresses
// Operator stuff
#define alChar 0x20
#define alScale 0x40
#define alAttack 0x60
#define alSus 0x80
#define alWave 0xe0
// Channel stuff
#define alFreqL 0xa0
#define alFreqH 0xb0
#define alFeedCon 0xc0
// Global stuff
#define alEffects 0xbd

struct Instrument {
    uint8_t mChar;
    uint8_t cChar;
    uint8_t mScale;
    uint8_t cScale;
    uint8_t mAttack;
    uint8_t cAttack;
    uint8_t mSus;
    uint8_t cSus;
    uint8_t mWave;
    uint8_t cWave;
    uint8_t nConn;

    // These are only for Muse - these bytes are really unused
    uint8_t voice;
    uint8_t mode;
    uint8_t unused[3];
}; // Instrument

struct AdLibSound {
    SoundCommon common;
    Instrument inst;
    uint8_t block;
    uint8_t data[1];
}; // AdLibSound

//
//      Sequencing stuff
//
#define sqMaxTracks 10
#define sqMaxMoods 1 // DEBUG

#define sev_Null 0 // Does nothing
#define sev_NoteOff 1 // Turns a note off
#define sev_NoteOn 2 // Turns a note on
#define sev_NotePitch 3 // Sets the pitch of a currently playing note
#define sev_NewInst 4 // Installs a new instrument
#define sev_NewPerc 5 // Installs a new percussive instrument
#define sev_PercOn 6 // Turns a percussive note on
#define sev_PercOff 7 // Turns a percussive note off
#define sev_SeqEnd -1 // Terminates a sequence

//      Flags for MusicGroup.flags
#define sf_Melodic 0
#define sf_Percussive 1

#if 1
struct MusicGroup {
    uint16_t length;
    uint16_t values[1];
}; // MusicGroup
#else
typedef struct  {
    uint16_t flags,
           count,
           offsets[1];
} MusicGroup;
#endif

struct ActiveTrack {
    // This part needs to be set up by the user
    uint16_t mood;
    uint16_t* moods[sqMaxMoods];

    // The rest is set up by the code
    Instrument inst;
    boolean percussive;
    uint16_t* seq;
    uint32_t nextevent;
}; // ActiveTrack

#define sqmode_Normal 0
#define sqmode_FadeIn 1
#define sqmode_FadeOut 2

#define sqMaxFade 64 // DEBUG


// Global variables
extern boolean AdLibPresent,
               SoundSourcePresent,
               SoundBlasterPresent,
               NeedsMusic, // For Caching Mgr
               SoundPositioned;
extern SDMode SoundMode;
extern SDSMode DigiMode;
extern SMMode MusicMode;
extern boolean DigiPlaying;
extern int16_t DigiMap[];
extern volatile uint32_t TimeCount; // Global time in ticks

extern boolean sqActive;
extern boolean sqPlayedOnce;

// Function prototypes
extern void SD_Startup(),
SD_Shutdown(),
SD_Default(boolean gotit, SDMode sd, SMMode sm),

SD_PositionSound(int16_t leftvol, int16_t rightvol);
extern boolean SD_PlaySound(
    int sound);
extern void SD_SetPosition(
    int16_t leftvol,
    int16_t rightvol),
SD_StopSound(),
SD_WaitSoundDone(),
SD_StartMusic(int index),
SD_MusicOn(),
SD_MusicOff(),
SD_FadeOutMusic(),

SD_SetUserHook(void (* hook)());
extern boolean SD_MusicPlaying(),
SD_SetSoundMode(SDMode mode),
SD_SetMusicMode(SMMode mode);
bool SD_SoundPlaying();

extern void SD_SetDigiDevice(SDSMode),
SD_PlayDigitized(uint16_t which, int16_t leftpos, int16_t rightpos),
SD_StopDigitized(),
SD_Poll();

#ifdef  _MUSE_ // MUSE Goes directly to the lower level routines
extern void SDL_PCPlaySound(
    PCSound* sound),
SDL_PCStopSound(),
SDL_ALPlaySound(AdLibSound * sound),
SDL_ALStopSound();
#endif

// BBi
const int MIN_VOLUME = 0;
const int MAX_VOLUME = 9;

extern int g_sfx_volume;
extern int g_music_volume;

void sd_play_sound(
    int sound_index,
    const void* actor,
    bstone::ActorType actor_type,
    bstone::ActorChannel actor_channel);

void sd_play_actor_sound(
    int sound_index,
    const objtype* actor,
    bstone::ActorChannel actor_channel);

void sd_play_player_sound(
    int sound_index,
    bstone::ActorChannel actor_channel);

void sd_play_door_sound(
    int sound_index,
    const doorobj_t* door);

void sd_play_wall_sound(
    int sound_index);

void sd_update_positions();

bool sd_is_player_channel_playing(
    bstone::ActorChannel channel);

void sd_set_sfx_volume(
    int volume);

void sd_set_music_volume(
    int volume);
// BBi


#endif // BSTONE_ID_SD_INCLUDED
