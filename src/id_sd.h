/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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

#include <string>

#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer.h"


namespace bstone
{


class TextWriter;


} // bstone


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


// Global variables

extern bool sd_has_audio_;
extern bool sd_is_sound_enabled_;
extern bool sd_is_music_enabled_;

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

bool sd_is_any_unpausable_sound_playing();


// BBi
const int sd_min_volume = 0;
const int sd_max_volume = 20;
const int sd_default_sfx_volume = 10;
const int sd_default_music_volume = 5;

extern int sd_sfx_volume_;
extern int sd_music_volume_;

extern AudioDriverType sd_audio_driver_type;
extern std::string sd_oal_library;
extern std::string sd_oal_device_name;


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

void sd_pause_sfx(
	const bool is_pause);

void sd_pause_music(
	const bool is_pause);


bstone::AudioDecoderInterpolationType sd_get_resampling_interpolation() noexcept;

void sd_cfg_set_resampling_interpolation(
	const bstone::AudioDecoderInterpolationType interpolation_type);


bool sd_cfg_get_resampling_low_pass_filter() noexcept;

void sd_cfg_set_resampling_low_pass_filter(
	const bool is_enabled);


AudioSfxType sd_cfg_get_sfx_type() noexcept;

void sd_cfg_set_sfx_type(
	AudioSfxType sfx_type);

void sd_apply_sfx_type();


bool sd_cfg_get_is_sfx_digitized() noexcept;

void sd_cfg_set_is_sfx_digitized(
	bool is_sfx_digitized);

void apply_digitized_sfx();


void sd_apply_resampling();

void sd_cfg_set_defaults();

bool sd_cfg_parse_key_value(
	const std::string& key_string,
	const std::string& value_string);

void sd_cfg_write(
	bstone::TextWriter& text_writer);

// BBi


#endif // BSTONE_ID_SD_INCLUDED
