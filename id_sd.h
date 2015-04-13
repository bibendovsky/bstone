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

#ifndef __ID_SD__
#define __ID_SD__

// BBi
#include "bstone_audio_mixer.h"

struct objtype;
struct doorobj_t;

void alOut(
    Uint8 n,
    Uint8 b);

#ifdef  __DEBUG__
#define __DEBUG_SoundMgr__
#endif

#define TickBase 70 // 70Hz per tick - used as a base for timer 0

enum SDMode {
    sdm_Off,
    sdm_PC,
    sdm_AdLib
}; // enum SDMode

enum SMMode {
    smm_Off,
    smm_AdLib
}; // enum SMMode

enum SDSMode {
    sds_Off,
    sds_PC,
    sds_SoundSource,
    sds_SoundBlaster
}; // enum SDSMode

struct SoundCommon {
    Uint32 length;
    Uint16 priority;
}; // struct SoundCommon

//      PC Sound stuff
#define pcTimer 0x42
#define pcTAccess 0x43
#define pcSpeaker 0x61

#define pcSpkBits 3

struct PCSound {
    SoundCommon common;
    Uint8 data[1];
}; // struct PCSound

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
    Uint16 hertz;
    Uint8 bits;
    Uint8 reference;
    Uint8 data[1];
}; // struct SampledSound

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
    Uint8 mChar;
    Uint8 cChar;
    Uint8 mScale;
    Uint8 cScale;
    Uint8 mAttack;
    Uint8 cAttack;
    Uint8 mSus;
    Uint8 cSus;
    Uint8 mWave;
    Uint8 cWave;
    Uint8 nConn;

    // These are only for Muse - these bytes are really unused
    Uint8 voice;
    Uint8 mode;
    Uint8 unused[3];
}; // struct Instrument

struct AdLibSound {
    SoundCommon common;
    Instrument inst;
    Uint8 block;
    Uint8 data[1];
}; // struct AdLibSound

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
    Uint16 length;
    Uint16 values[1];
}; // struct MusicGroup
#else
typedef struct  {
    Uint16 flags,
           count,
           offsets[1];
} MusicGroup;
#endif

struct ActiveTrack {
    // This part needs to be set up by the user
    Uint16 mood;
    Uint16* moods[sqMaxMoods];

    // The rest is set up by the code
    Instrument inst;
    boolean percussive;
    Uint16* seq;
    Uint32 nextevent;
}; // struct ActiveTrack

#define sqmode_Normal 0
#define sqmode_FadeIn 1
#define sqmode_FadeOut 2

#define sqMaxFade 64 // DEBUG


// Global variables
extern boolean AdLibPresent,
               SoundSourcePresent,
               SoundBlasterPresent,
               NeedsMusic,                                                              // For Caching Mgr
               SoundPositioned;
extern SDMode SoundMode;
extern SDSMode DigiMode;
extern SMMode MusicMode;
extern boolean DigiPlaying;
extern Sint16 DigiMap[];
extern volatile Uint32 TimeCount; // Global time in ticks

extern boolean sqActive;
extern boolean sqPlayedOnce;

// Function prototypes
extern void SD_Startup(),
SD_Shutdown(),
SD_Default(boolean gotit, SDMode sd, SMMode sm),

SD_PositionSound(Sint16 leftvol, Sint16 rightvol);
extern boolean SD_PlaySound(
    soundnames sound);
extern void SD_SetPosition(
    Sint16 leftvol,
    Sint16 rightvol),
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
SD_PlayDigitized(Uint16 which, Sint16 leftpos, Sint16 rightpos),
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


#endif
