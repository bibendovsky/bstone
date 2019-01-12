/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
//      ID Engine
//      ID_SD.h - Sound Manager Header
//      Version for Wolfenstein
//      By Jason Blochowiak
//


#ifndef BSTONE_ID_SD_INCLUDED
#define BSTONE_ID_SD_INCLUDED


#include <cstdint>
#include <atomic>
#include "bstone_audio_mixer.h"


struct objtype;
struct doorobj_t;


struct SoundCommon
{
	std::uint32_t length;
	std::uint16_t priority;
}; // SoundCommon

struct Instrument
{
	std::uint8_t mChar;
	std::uint8_t cChar;
	std::uint8_t mScale;
	std::uint8_t cScale;
	std::uint8_t mAttack;
	std::uint8_t cAttack;
	std::uint8_t mSus;
	std::uint8_t cSus;
	std::uint8_t mWave;
	std::uint8_t cWave;
	std::uint8_t nConn;

	// These are only for Muse - these bytes are really unused
	std::uint8_t voice;
	std::uint8_t mode;
	std::uint8_t unused[3];
}; // Instrument

struct MusicGroup
{
	std::uint16_t length;
	std::uint16_t values[1];
}; // MusicGroup


// Global variables
extern bool sd_has_audio;
extern bool sd_is_sound_enabled;
extern bool sd_is_music_enabled;
extern std::int16_t DigiMap[];
extern std::atomic<std::uint32_t> TimeCount; // Global time in ticks

extern bool sqActive;
extern bool sqPlayedOnce;

// Function prototypes
void SD_Startup();
void SD_Shutdown();
void SD_StopSound();
void SD_WaitSoundDone();
void SD_StartMusic(
	int index);
void SD_MusicOn();
void SD_MusicOff();

bool SD_EnableSound(
	bool enable);

bool SD_EnableMusic(
	bool enable);

bool SD_SoundPlaying();


// BBi
const int sd_min_volume = 0;
const int sd_max_volume = 20;
const int sd_default_sfx_volume = 10;
const int sd_default_music_volume = 5;

extern int sd_sfx_volume;
extern int sd_music_volume;

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

void sd_mute(
	bool mute);
// BBi


#endif // BSTONE_ID_SD_INCLUDED
