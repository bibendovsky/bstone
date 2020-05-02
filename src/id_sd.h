/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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

struct SfxInfo
{
	bool is_digitized_;
	int digi_index_;
	const void* data_;
	int size_;
}; // SfxInfo


// Global variables

extern bool sd_has_audio_;
extern bool sd_is_sound_enabled_;
extern bool sd_is_music_enabled_;
extern std::int16_t sd_digi_map_[];

extern bool sd_sq_active_;
extern bool sd_sq_played_once_;


// Function prototypes

void sd_startup();

void sd_shutdown();

void sd_stop_sound();

void sd_wait_sound_done();

void sd_start_music(
	const int index);

void sd_music_on();

void sd_music_off();

bool sd_enable_sound(
	const bool enable);

bool sd_enable_music(
	const bool enable);

bool sd_sound_playing();


// BBi
const int sd_min_volume = 0;
const int sd_max_volume = 20;
const int sd_default_sfx_volume = 10;
const int sd_default_music_volume = 5;

extern int sd_sfx_volume_;
extern int sd_music_volume_;


void sd_play_sound(
	const int sound_index,
	const void* actor,
	const bstone::ActorType actor_type,
	const bstone::ActorChannel actor_channel);

void sd_play_actor_sound(
	const int sound_index,
	const objtype* actor,
	const bstone::ActorChannel actor_channel);

void sd_play_player_sound(
	const int sound_index,
	const bstone::ActorChannel actor_channel);

void sd_play_door_sound(
	const int sound_index,
	const doorobj_t* door);

void sd_play_wall_sound(
	const int sound_index);

void sd_update_positions();

bool sd_is_player_channel_playing(
	const bstone::ActorChannel channel);

void sd_set_sfx_volume(
	const int volume);

void sd_set_music_volume(
	const int volume);

void sd_mute(
	const bool mute);

int sd_get_adlib_music_data_size(
	const void* const raw_music_data);

SfxInfo sd_get_sfx_info(
	const int sfx_number);

void sd_setup_extracting();
// BBi


#endif // BSTONE_ID_SD_INCLUDED
