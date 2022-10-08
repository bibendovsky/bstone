/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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

// Function prototypes

void sd_startup();
void sd_shutdown();
void sd_stop_sfx_sound();
void sd_wait_sound_done();
void sd_start_music(int index, bool is_looping = true);
void sd_music_on(bool is_looping);
void sd_music_off();
bool sd_enable_sound(const bool enable);
bool sd_enable_music(const bool enable);
bool sd_is_playing_any_ui_sound();
bool sd_is_music_playing();

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

void sd_play_ui_sound(int sound_index);
void sd_play_actor_voice_sound(int sound_index, objtype& actor);
void sd_play_actor_weapon_sound(int sound_index, objtype& actor);

void sd_play_player_voice_sound(int sound_index);
void sd_play_player_weapon_sound(int sound_index);
void sd_play_player_item_sound(int sound_index);
void sd_play_player_hit_wall_sound(int sound_index);
void sd_play_player_no_way_sound(int sound_index);
void sd_play_player_interrogation_sound(int sound_index);

void sd_play_door_sound(int sound_index, doorobj_t& door);
void sd_play_pwall_sound(int sound_index);

void sd_update();

bool sd_is_player_hit_wall_sound_playing();
bool sd_is_player_no_way_sound_playing();

void sd_set_sfx_volume();
void sd_set_music_volume();

void sd_mute(bool mute);
void sd_pause_scene_sfx(bool is_pause);
void sd_pause_music(bool is_pause);

AudioSfxType sd_cfg_get_sfx_type() noexcept;
void sd_cfg_set_sfx_type(AudioSfxType sfx_type);
void sd_apply_sfx_type();
bool sd_cfg_get_is_sfx_digitized() noexcept;
void sd_cfg_set_is_sfx_digitized(bool is_sfx_digitized);
void apply_digitized_sfx();
bstone::Opl3Type sd_get_opl3_type() noexcept;
void sd_set_opl3_type(bstone::Opl3Type opl3_type);
void sd_cfg_set_defaults();
bool sd_cfg_parse_key_value(const std::string& key_string, const std::string& value_string);
void sd_cfg_write(bstone::TextWriter& text_writer);

// BBi


#endif // BSTONE_ID_SD_INCLUDED
