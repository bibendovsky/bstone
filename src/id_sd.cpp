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

#include <vector>
#include "id_sd.h"
#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "bstone_audio_content_mgr.h"
#include "bstone_audio_mixer.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_scope_guard.h"
#include "bstone_string_helper.h"
#include "bstone_text_writer.h"
#include "bstone_voice_group.h"

constexpr auto max_voices =
	MAXACTORS +
	MAXDOORS +
	32;

// Global variables

bool sd_has_audio_ = false;
bool sd_is_sound_enabled_ = false;
bool sd_is_music_enabled_ = false;

// Internal variables

static bool sd_started_;


// AdLib variables

bool sd_sq_active_;

// Internal routines

// BBi

namespace
{

class SdException : public bstone::Exception
{
public:
	explicit SdException(const char* message) noexcept
		:
		Exception{"SD", message}
	{
	}
}; // SdException

struct SdR3PositionCacheItem
{
	double x;
	double y;
}; // SdR3PositionCacheItem

using SdR3PositionCache = std::vector<SdR3PositionCacheItem>;
SdR3PositionCache sd_r3_position_cache{};

[[noreturn]] void sd_fail_nested(const char* message)
{
	std::throw_with_nested(SdException{message});
}

bstone::AudioContentMgrUPtr audio_content_mgr{};

auto sd_sfx_type = AudioSfxType::adlib;
auto sd_is_sfx_digitized = true;

auto sd_master_voice_group = static_cast<bstone::VoiceGroup*>(nullptr);
auto sd_ui_sfx_voice_group = static_cast<bstone::VoiceGroup*>(nullptr);
auto sd_scene_sfx_voice_group = static_cast<bstone::VoiceGroup*>(nullptr);
auto sd_music_voice_group = static_cast<bstone::VoiceGroup*>(nullptr);
auto sd_player_voice_group = static_cast<bstone::VoiceGroup*>(nullptr);
auto sd_voice_group_mgr = bstone::VoiceGroupUPtr{};

auto sd_pwall_voice_handle = bstone::AudioMixerVoiceHandle{};

auto sd_player_item_voice_handle = bstone::AudioMixerVoiceHandle{};
auto sd_player_hit_wall_voice_handle = bstone::AudioMixerVoiceHandle{};
auto sd_player_no_way_voice_handle = bstone::AudioMixerVoiceHandle{};
auto sd_player_interrogation_voice_handle = bstone::AudioMixerVoiceHandle{};

} // namespace


static int sd_music_index_ = -1;
static bstone::AudioMixerUPtr sd_mixer_;

int sd_sfx_volume_ = sd_default_sfx_volume;
int sd_music_volume_ = sd_default_music_volume;

AudioDriverType sd_audio_driver_type = AudioDriverType::auto_detect;
std::string sd_oal_library = std::string{};
std::string sd_oal_device_name = std::string{};
bstone::Opl3Type sd_opl3_type_ = bstone::Opl3Type::dbopl;

void sd_cfg_parse_cl();

const std::string& sd_get_snd_string()
{
	static const auto result = std::string{"[SND]"};

	return result;
}

void sd_log()
{
	bstone::logger_->write();
}

void sd_log(const bstone::LoggerMessageKind message_kind, const std::string& message)
{
	bstone::logger_->write(message_kind, sd_get_snd_string() + ' ' + message);
}

void sd_log(const std::string& message)
{
	bstone::logger_->write(bstone::LoggerMessageKind::information, sd_get_snd_string() + ' ' + message);
}

void sd_log_error(const std::string& message)
{
	sd_log(bstone::LoggerMessageKind::error, message);
}


// Determines if there's an AdLib (or SoundBlaster emulating an AdLib) present
static bool sd_detect_ad_lib()
{
	const auto& snd_is_disabled_string = g_args.get_option_value("snd_is_disabled");

	if (snd_is_disabled_string.empty())
	{
		return true;
	}

	auto snd_is_disabled_int = 0;

	if (!bstone::StringHelper::string_to_int(snd_is_disabled_string, snd_is_disabled_int))
	{
		return true;
	}

	return snd_is_disabled_int == 0;
}

bool sd_enable_sound(bool enable)
{
	auto is_enabled = enable;

	sd_stop_sound();

	if (is_enabled && !sd_has_audio_)
	{
		is_enabled = false;
	}

	sd_is_sound_enabled_ = is_enabled;

	if (is_enabled)
	{
		sd_set_sfx_volume(sd_sfx_volume_);
	}

	return is_enabled;
}

bool sd_enable_music(bool enable)
{
	sd_music_off();
	sd_is_music_enabled_ = enable;
	sd_set_music_volume(sd_music_volume_);

	return enable;
}

const std::string& sd_get_opl3_long_name(const bstone::Opl3Type opl3_type)
{
	static const auto unknown = std::string{"???"};
	static const auto dosbox_dbopl = std::string{"DBOPL"};
	static const auto nuked_opl3 = std::string{"Nuked"};

	switch (opl3_type)
	{
		case bstone::Opl3Type::dbopl:
			return dosbox_dbopl;

		case bstone::Opl3Type::nuked:
			return nuked_opl3;

		default:
			return unknown;
	}
}

const std::string& sd_get_resampling_lpf_long_name(bool is_enabled)
{
	static const auto on = std::string{"On"};
	static const auto off = std::string{"Off"};
	return is_enabled ? on : off;
}

namespace
{

void sd_make_mixer(AudioDriverType audio_driver_type, int sample_rate, int mix_size_ms)
try
{
	sd_mixer_ = nullptr;

	auto param = bstone::AudioMixerInitParam{};
	param.audio_driver_type = audio_driver_type;
	param.opl3_type = sd_opl3_type_;
	param.dst_rate = sample_rate;
	param.mix_size_ms = mix_size_ms;
	param.max_voices = max_voices;
	auto sd_mixer = bstone::make_audio_mixer(param);

	auto voice_group_mgr = bstone::make_voice_group(*sd_mixer);

	sd_master_voice_group = voice_group_mgr.get();
	sd_master_voice_group->set_gain(1.0);

	sd_ui_sfx_voice_group = &sd_master_voice_group->add_group();
	sd_scene_sfx_voice_group = &sd_master_voice_group->add_group();
	sd_music_voice_group = &sd_master_voice_group->add_group();
	sd_player_voice_group = &sd_master_voice_group->add_group();

	sd_mixer_.swap(sd_mixer);
	sd_voice_group_mgr.swap(voice_group_mgr);
}
catch (...)
{
	sd_fail_nested(__func__);
}

} // namespace

void sd_startup()
{
	if (sd_started_)
	{
		return;
	}

	sd_log();
	sd_log("Initializing audio");
	sd_log("------------------");

	sd_has_audio_ = sd_detect_ad_lib();

	if (sd_has_audio_)
	{
		auto snd_rate = 0;
		auto snd_mix_size = 0;

		{
			const auto& snd_rate_string = g_args.get_option_value("snd_rate");

			static_cast<void>(bstone::StringHelper::string_to_int(snd_rate_string, snd_rate));
		}

		{
			const auto& snd_mix_size_string = g_args.get_option_value("snd_mix_size");

			static_cast<void>(bstone::StringHelper::string_to_int(snd_mix_size_string, snd_mix_size));
		}

		sd_cfg_parse_cl();

		auto is_driver_initialized = false;

		switch (sd_audio_driver_type)
		{
			case AudioDriverType::r2_sdl:
			case AudioDriverType::r3_openal:
				try
				{
					sd_make_mixer(sd_audio_driver_type, snd_rate, snd_mix_size);
					is_driver_initialized = true;
				}
				catch (...)
				{
					bstone::log_exception();
				}

				break;

			case AudioDriverType::auto_detect:
			default:
				try
				{
					sd_make_mixer(AudioDriverType::r3_openal, snd_rate, snd_mix_size);
					is_driver_initialized = true;
				}
				catch (...)
				{
					bstone::log_exception();

					try
					{
						sd_make_mixer(AudioDriverType::r2_sdl, snd_rate, snd_mix_size);
						is_driver_initialized = true;
					}
					catch (...)
					{
						bstone::log_exception();
					}
				}

				break;
		}

		if (is_driver_initialized)
		{
			sd_log("Channel count: " + std::to_string(sd_mixer_->get_channel_count()));
			sd_log("Sample rate: " + std::to_string(sd_mixer_->get_rate()) + " Hz");
			sd_log("Mix size: " + std::to_string(sd_mixer_->get_mix_size_ms()) + " ms");
			sd_log("Effects volume: " + std::to_string(sd_sfx_volume_) + " / " + std::to_string(sd_max_volume));
			sd_log("Music volume: " + std::to_string(sd_music_volume_) + " / " + std::to_string(sd_max_volume));
			sd_log("OPL3 type: " + sd_get_opl3_long_name(sd_mixer_->get_opl3_type()));

			audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
			audio_content_mgr->set_sfx_type(sd_sfx_type);
			audio_content_mgr->set_is_sfx_digitized(sd_is_sfx_digitized);
		}
		else
		{
			sd_log_error("Failed to initialize mixer.");
		}
	}
	else
	{
		sd_log("Audio subsystem disabled.");

		sd_mixer_ = nullptr;
		audio_content_mgr = nullptr;
	}

	sd_started_ = true;

	if (sd_has_audio_)
	{
		if (sd_is_sound_enabled_)
		{
			sd_enable_sound(true);
		}

		if (sd_is_music_enabled_)
		{
			sd_enable_music(true);
			sd_music_on(true);
		}
	}
}

void sd_shutdown()
{
	if (!sd_started_)
	{
		return;
	}

	sd_mixer_ = nullptr;
	audio_content_mgr = nullptr;
	sd_started_ = false;
}

bool sd_is_playing_any_ui_sound()
{
	if (!sd_is_sound_enabled_ || !sd_mixer_ || !sd_scene_sfx_voice_group)
	{
		return false;
	}

	return sd_scene_sfx_voice_group->is_any_playing();
}

bool sd_is_music_playing()
{
	if (!sd_mixer_ || !sd_is_music_enabled_)
	{
		return false;
	}

	return sd_music_voice_group->is_any_playing();
}

// If a sound is playing, stops it.
void sd_stop_sound()
{
	if (!sd_mixer_ || !sd_is_music_enabled_)
	{
		return;
	}

	sd_scene_sfx_voice_group->stop();
}

// Waits until the current sound is done playing.
void sd_wait_sound_done()
{
	if (!sd_is_sound_enabled_ || !sd_mixer_ || !sd_ui_sfx_voice_group || !sd_scene_sfx_voice_group)
	{
		return;
	}

	while (sd_ui_sfx_voice_group->is_any_playing() || sd_scene_sfx_voice_group->is_any_playing())
	{
		sys_default_sleep_for();
	}
}

// Turns on the sequencer.
void sd_music_on(bool is_looping)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_ || sd_music_index_ < 0)
	{
		return;
	}

	sd_sq_active_ = true;

	const auto& audio_chunk = audio_content_mgr->get_adlib_music_chunk(sd_music_index_);

	sd_music_voice_group->stop();
	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = bstone::SoundType::adlib_music;
	play_sound_param.sound_index = 0;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = is_looping;
	play_sound_param.is_r3 = false;
	const auto voice_handle = sd_mixer_->play_sound(play_sound_param);
	sd_music_voice_group->add_voice(voice_handle);
}

// Turns off the sequencer and any playing notes.
void sd_music_off()
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_sq_active_ = false;
	sd_music_voice_group->stop();
}

// Starts playing the music pointed to.
void sd_start_music(int index, bool is_looping)
{
	sd_music_off();

	if (sd_is_music_enabled_)
	{
		sd_music_index_ = index;
		sd_set_music_volume(sd_music_volume_);
		sd_music_on(is_looping);
	}
}

// BBi

void sd_stop_voice(bstone::AudioMixerVoiceHandle& voice_handle, bstone::VoiceGroup& voice_group)
{
	voice_group.remove_voice(voice_handle);
	sd_mixer_->stop_voice(voice_handle);
	voice_handle.reset();
}

bstone::AudioMixerVoiceHandle sd_play_non_positional_sfx_sound(int sound_index, bstone::VoiceGroup& voice_group)
{
	auto result = bstone::AudioMixerVoiceHandle{};

	const auto& audio_chunk = audio_content_mgr->get_sfx_chunk(sound_index);

	if (!audio_chunk.data || audio_chunk.data_size == 0)
	{
		return result;
	}

	auto sound_type = bstone::SoundType::none;

	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_sfx:
			sound_type = bstone::SoundType::adlib_sfx;
			break;

		case AudioChunkType::pc_speaker:
			sound_type = bstone::SoundType::pc_speaker_sfx;
			break;

		case AudioChunkType::digitized:
			sound_type = bstone::SoundType::pcm;
			break;

		default:
			fail("Unknown audio chunk type.");
	}

	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = sound_type;
	play_sound_param.sound_index = sound_index;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = false;
	play_sound_param.is_r3 = false;
	const auto voice_handle = sd_mixer_->play_sound(play_sound_param);

	if (voice_handle.is_valid())
	{
		voice_group.add_voice(voice_handle);
	}

	return voice_handle;
}

bstone::AudioMixerVoiceHandle sd_play_positional_sfx_sound(int sound_index, bstone::VoiceGroup& voice_group, const bstone::AudioMixerVoiceR3Position& r3_position)
{
	auto result = bstone::AudioMixerVoiceHandle{};

	const auto& audio_chunk = audio_content_mgr->get_sfx_chunk(sound_index);

	if (!audio_chunk.data || audio_chunk.data_size == 0)
	{
		return result;
	}

	auto sound_type = bstone::SoundType::none;

	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_sfx:
			sound_type = bstone::SoundType::adlib_sfx;
			break;

		case AudioChunkType::pc_speaker:
			sound_type = bstone::SoundType::pc_speaker_sfx;
			break;

		case AudioChunkType::digitized:
			sound_type = bstone::SoundType::pcm;
			break;

		default:
			fail("Unknown audio chunk type.");
	}

	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = sound_type;
	play_sound_param.sound_index = sound_index;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = false;
	play_sound_param.is_r3 = true;
	const auto voice_handle = sd_mixer_->play_sound(play_sound_param);
	
	if (voice_handle.is_valid())
	{
		sd_mixer_->set_voice_r3_position(voice_handle, r3_position);
		voice_group.add_voice(voice_handle);
	}

	return voice_handle;
}

void sd_update_listener_r3_position()
{
	const auto listener_r3_position = bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(player->x, player->y, 0.5);
	sd_mixer_->set_listener_r3_position(bstone::AudioMixerListenerR3Position{listener_r3_position});
}

void sd_update_listener_r3_orientation()
{
	const auto listener_r3_orientation = bstone::AudioMixerUtils::make_listener_r3_orientation_from_w3d_view(viewcos, viewsin);
	sd_mixer_->set_listener_r3_orientation(listener_r3_orientation);
}

void sd_play_ui_sound(int sound_index)
{
	sd_play_non_positional_sfx_sound(sound_index, *sd_ui_sfx_voice_group);
}

namespace
{

bstone::AudioMixerVoiceR3Position make_r3_position_from_actor(const objtype& actor)
{
	const auto x = actor.x;
	const auto y = actor.y;
	const auto z = 0.5;
	return bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(x, y, z);
}

void sd_play_actor_sound(int sound_index, objtype& actor, bstone::VoiceGroup& voice_group, bstone::AudioMixerVoiceHandle& voice_handle)
{
	const auto r3_position = make_r3_position_from_actor(actor);
	voice_handle = sd_play_positional_sfx_sound(sound_index, voice_group, r3_position);
}

} // namespace

void sd_play_actor_voice_sound(int sound_index, objtype& actor)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_actor_sound(sound_index, actor, *sd_scene_sfx_voice_group, actor.voice_voice_handle);
}

void sd_play_actor_weapon_sound(int sound_index, objtype& actor)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_actor_sound(sound_index, actor, *sd_scene_sfx_voice_group, actor.weapon_voice_handle);
}

namespace
{

void sd_play_player_sound(int sound_index, bstone::AudioMixerVoiceHandle& voice_handle)
{
	sd_stop_voice(voice_handle, *sd_scene_sfx_voice_group);
	voice_handle = sd_play_non_positional_sfx_sound(sound_index, *sd_scene_sfx_voice_group);
}

} // namespace

void sd_play_player_voice_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_ || !player)
	{
		return;
	}

	sd_play_player_sound(sound_index, player->voice_voice_handle);
}

void sd_play_player_weapon_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_ || !player)
	{
		return;
	}

	sd_play_player_sound(sound_index, player->weapon_voice_handle);
}

void sd_play_player_item_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_item_voice_handle);
}

void sd_play_player_hit_wall_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_hit_wall_voice_handle);
}

void sd_play_player_no_way_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_no_way_voice_handle);
}

void sd_play_player_interrogation_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_interrogation_voice_handle);
}

void sd_play_door_sound(int sound_index, doorobj_t& door)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_stop_voice(door.voice_handle, *sd_scene_sfx_voice_group);
	const auto r3_position = bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(door.tilex + 0.5, door.tiley + 0.5, 0.5);
	door.voice_handle = sd_play_positional_sfx_sound(sound_index, *sd_scene_sfx_voice_group, r3_position);
}

namespace
{

bstone::AudioMixerVoiceR3Position sd_make_pwall_r3_position() noexcept
{
	auto x = pwallx + 0.5;
	auto y = pwally + 0.5;
	const auto z = 0.5;

	switch (pwalldir)
	{
		case di_east:
			x += pwallpos;
			break;

		case di_north:
			y -= pwallpos;
			break;

		case di_south:
			y += pwallpos;
			break;

		case di_west:
			x -= pwallpos;
			break;

		default:
			break;
	}

	return bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(x, y, z);
}

void sd_update_pwall()
{
	if (!sd_mixer_ || !sd_is_sound_enabled_ || !sd_pwall_voice_handle.is_valid())
	{
		return;
	}

	const auto r3_position = sd_make_pwall_r3_position();
	sd_mixer_->set_voice_r3_position(sd_pwall_voice_handle, r3_position);
}

} // namespace

void sd_play_pwall_sound(int sound_index)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_stop_voice(sd_pwall_voice_handle, *sd_scene_sfx_voice_group);
	const auto r3_position = sd_make_pwall_r3_position();
	sd_pwall_voice_handle = sd_play_positional_sfx_sound(sound_index, *sd_scene_sfx_voice_group, r3_position);
}

namespace
{

void sd_update_actor_r3_positions()
{
	if (sd_r3_position_cache.empty())
	{
		return;
	}

	constexpr auto z = 0.5;

	for (auto actor = player->next; actor; actor = actor->next)
	{
		const auto actor_index = actor - objlist;
		auto& cache_item = sd_r3_position_cache[actor_index];

		if (cache_item.x == actor->x && cache_item.y == actor->y)
		{
			continue;
		}

		cache_item.x = actor->x;
		cache_item.y = actor->y;
		const auto r3_position = bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(actor->x, actor->y, z);
		sd_mixer_->set_voice_r3_position(actor->voice_voice_handle, r3_position);
		sd_mixer_->set_voice_r3_position(actor->weapon_voice_handle, r3_position);
	}
}

} // namespace

void sd_reset_r3_position_cache()
{
	sd_r3_position_cache.clear();
	sd_r3_position_cache.resize(MAXACTORS);
}

void sd_update_positions()
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}


	const auto mixer_state = bstone::ScopeGuard{
		[]()
		{
			sd_mixer_->resume_state();
		}};

	sd_mixer_->suspend_state();
	sd_update_pwall();
	sd_update_actor_r3_positions();
	sd_update_listener_r3_position();
	sd_update_listener_r3_orientation();
}

namespace
{

bool sd_is_player_sound_playing(bstone::AudioMixerVoiceHandle voice_handle)
{
	return sd_mixer_->is_voice_playing(voice_handle);
}

} // namespace

bool sd_is_player_hit_wall_sound_playing()
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return false;
	}

	return sd_is_player_sound_playing(sd_player_hit_wall_voice_handle);
}

bool sd_is_player_no_way_sound_playing()
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return false;
	}

	return sd_is_player_sound_playing(sd_player_no_way_voice_handle);
}

void sd_set_sfx_volume(int volume)
{
	if (!sd_mixer_)
	{
		return;
	}

	auto new_volume = volume;

	if (new_volume < sd_min_volume)
	{
		new_volume = sd_min_volume;
	}

	if (new_volume > sd_max_volume)
	{
		new_volume = sd_max_volume;
	}

	const auto gain = static_cast<double>(new_volume) / static_cast<double>(sd_max_volume);
	sd_ui_sfx_voice_group->set_gain(gain);
	sd_scene_sfx_voice_group->set_gain(gain);
}

void sd_set_music_volume(int volume)
{
	if (!sd_mixer_)
	{
		return;
	}

	auto new_volume = volume;

	if (new_volume < sd_min_volume)
	{
		new_volume = sd_min_volume;
	}

	if (new_volume > sd_max_volume)
	{
		new_volume = sd_max_volume;
	}

	const auto gain = static_cast<double>(new_volume) / static_cast<double>(sd_max_volume);
	sd_music_voice_group->set_gain(gain);
}

void sd_mute(bool mute)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	sd_mixer_->set_mute(mute);
}

void sd_pause_sfx(bool is_pause)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	if (is_pause)
	{
		sd_scene_sfx_voice_group->pause();
	}
	else
	{
		sd_scene_sfx_voice_group->resume();
	}
}

void sd_pause_music(bool is_pause)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	if (is_pause)
	{
		sd_music_voice_group->pause();
	}
	else
	{
		sd_music_voice_group->resume();
	}
}

void apply_digitized_sfx()
{
	if (!audio_content_mgr)
	{
		return;
	}

	audio_content_mgr->set_is_sfx_digitized(sd_is_sfx_digitized);
}

AudioSfxType sd_cfg_get_sfx_type() noexcept
{
	return sd_sfx_type;
}

void sd_cfg_set_sfx_type(AudioSfxType sfx_type)
{
	sd_sfx_type = sfx_type;
}

void sd_apply_sfx_type()
{
	if (!audio_content_mgr)
	{
		return;
	}

	audio_content_mgr->set_sfx_type(sd_sfx_type);
}

bool sd_cfg_get_is_sfx_digitized() noexcept
{
	return sd_is_sfx_digitized;
}

void sd_cfg_set_is_sfx_digitized(bool is_sfx_digitized)
{
	sd_is_sfx_digitized = is_sfx_digitized;
}

bstone::Opl3Type sd_get_opl3_type() noexcept
{
	return sd_opl3_type_;
}

void sd_set_opl3_type(bstone::Opl3Type opl3_type)
{
	switch (opl3_type)
	{
		case bstone::Opl3Type::dbopl:
		case bstone::Opl3Type::nuked:
			if (opl3_type != sd_opl3_type_)
			{
				sd_opl3_type_ = opl3_type;
			}
			break;

		default:
			sd_log_error("Invalid OPL3 type.");
			break;
	}
}

void sd_cfg_set_defaults()
{
	sd_sfx_type = AudioSfxType::adlib;
	sd_is_sfx_digitized = true;

	sd_opl3_type_ = bstone::Opl3Type::dbopl;
}

const std::string& sd_cfg_get_2d_sdl_name()
{
	static const auto result = std::string{"2d_sdl"};
	return result;
}

const std::string& sd_cfg_get_3d_openal_name()
{
	static const auto result = std::string{"3d_openal"};
	return result;
}

const std::string& sd_cfg_get_auto_detect_name()
{
	static const auto result = std::string{"auto-detect"};
	return result;
}

const std::string& sd_cfg_get_driver_name()
{
	static const auto result = std::string{"snd_driver"};
	return result;
}

const std::string& sd_cfg_get_oal_library_name()
{
	static const auto result = std::string{"snd_oal_library"};
	return result;
}

const std::string& sd_cfg_get_oal_device_name_name()
{
	static const auto result = std::string{"snd_oal_device_name"};
	return result;
}

const std::string& sd_cfg_get_adlib_name()
{
	static const auto result = std::string{"adlib"};
	return result;
}

const std::string& sd_cfg_get_pc_speaker_name()
{
	static const auto result = std::string{"pc_speaker"};
	return result;
}

const std::string& sd_cfg_get_sfx_type_name()
{
	static const auto result = std::string{"snd_sfx_type"};
	return result;
}

const std::string& sd_cfg_get_is_sfx_digitized_name()
{
	static const auto result = std::string{"snd_is_sfx_digitized"};
	return result;
}

const std::string& sd_cfg_get_snd_opl3_type_str()
{
	static const auto result = std::string{"snd_opl3_type"};
	return result;
}

const std::string& sd_cfg_get_dbopl_str()
{
	static const auto result = std::string{"dbopl"};
	return result;
}

const std::string& sd_cfg_get_nuked_str()
{
	static const auto result = std::string{"nuked"};
	return result;
}

bool sd_cfg_parse_key_value(const std::string& key_string, const std::string& value_string)
{
	if (false)
	{
	}
	else if (key_string == sd_cfg_get_driver_name())
	{
		if (value_string == sd_cfg_get_2d_sdl_name())
		{
			sd_audio_driver_type = AudioDriverType::r2_sdl;
		}
		else if (value_string == sd_cfg_get_3d_openal_name())
		{
			sd_audio_driver_type = AudioDriverType::r3_openal;
		}
		else
		{
			sd_audio_driver_type = AudioDriverType::auto_detect;
		}

		return true;
	}
	else if (key_string == sd_cfg_get_oal_library_name())
	{
		sd_oal_library = value_string;
		return true;
	}
	else if (key_string == sd_cfg_get_oal_device_name_name())
	{
		sd_oal_device_name = value_string;
		return true;
	}
	else if (key_string == sd_cfg_get_sfx_type_name())
	{
		auto sfx_type = AudioSfxType{};

		if (value_string == sd_cfg_get_adlib_name())
		{
			sfx_type = AudioSfxType::adlib;
		}
		else if (value_string == sd_cfg_get_pc_speaker_name())
		{
			sfx_type = AudioSfxType::pc_speaker;
		}
		else
		{
			sfx_type = AudioSfxType::adlib;
		}

		sd_cfg_set_sfx_type(sfx_type);

		return true;
	}
	else if (key_string == sd_cfg_get_is_sfx_digitized_name())
	{
		int value = 0;

		if (bstone::StringHelper::string_to_int(value_string, value))
		{
			sd_cfg_set_is_sfx_digitized(value != 0);
		}

		return true;
	}
	else if (key_string == sd_cfg_get_snd_opl3_type_str())
	{
		if (value_string == sd_cfg_get_nuked_str())
		{
			sd_opl3_type_ = bstone::Opl3Type::nuked;
		}
		else
		{
			sd_opl3_type_ = bstone::Opl3Type::dbopl;
		}
	}

	return false;
}

void sd_cfg_parse_cl()
{
	if (g_args.has_option(sd_cfg_get_driver_name()))
	{
		const auto value_string = g_args.get_option_value(sd_cfg_get_driver_name());

		if (value_string == sd_cfg_get_2d_sdl_name())
		{
			sd_audio_driver_type = AudioDriverType::r2_sdl;
		}
		else if (value_string == sd_cfg_get_3d_openal_name())
		{
			sd_audio_driver_type = AudioDriverType::r3_openal;
		}
		else
		{
			sd_audio_driver_type = AudioDriverType::auto_detect;
		}
	}

	if (g_args.has_option(sd_cfg_get_oal_library_name()))
	{
		sd_oal_library = g_args.get_option_value(sd_cfg_get_oal_library_name());
	}

	if (g_args.has_option(sd_cfg_get_oal_device_name_name()))
	{
		sd_oal_device_name = g_args.get_option_value(sd_cfg_get_oal_device_name_name());
	}

	if (g_args.has_option(sd_cfg_get_sfx_type_name()))
	{
		const auto value_string = g_args.get_option_value(sd_cfg_get_sfx_type_name());

		if (!value_string.empty())
		{
			sd_cfg_parse_key_value(sd_cfg_get_sfx_type_name(), value_string);
		}
	}

	if (g_args.has_option(sd_cfg_get_is_sfx_digitized_name()))
	{
		const auto value_string = g_args.get_option_value(sd_cfg_get_is_sfx_digitized_name());

		if (!value_string.empty())
		{
			sd_cfg_parse_key_value(sd_cfg_get_is_sfx_digitized_name(), value_string);
		}
	}

	if (g_args.has_option(sd_cfg_get_snd_opl3_type_str()))
	{
		const auto value_string = g_args.get_option_value(sd_cfg_get_snd_opl3_type_str());

		if (!value_string.empty())
		{
			sd_cfg_parse_key_value(sd_cfg_get_snd_opl3_type_str(), value_string);
		}
	}
}

void sd_cfg_write(bstone::TextWriter& text_writer)
{
	{
		auto value_string = std::string{};

		switch (sd_audio_driver_type)
		{
			case AudioDriverType::r2_sdl:
				value_string = sd_cfg_get_2d_sdl_name();
				break;

			case AudioDriverType::r3_openal:
				value_string = sd_cfg_get_3d_openal_name();
				break;

			default:
				value_string = sd_cfg_get_auto_detect_name();
				break;
		}

		cfg_file_write_entry(
			text_writer,
			sd_cfg_get_driver_name(),
			value_string
		);
	}

	cfg_file_write_entry(
		text_writer,
		sd_cfg_get_oal_library_name(),
		sd_oal_library
	);

	cfg_file_write_entry(
		text_writer,
		sd_cfg_get_oal_device_name_name(),
		sd_oal_device_name
	);

	{
		auto value_string = std::string{};

		switch (sd_sfx_type)
		{
			case AudioSfxType::pc_speaker:
				value_string = sd_cfg_get_pc_speaker_name();
				break;

			case AudioSfxType::adlib:
			default:
				value_string = sd_cfg_get_adlib_name();
				break;
		}

		cfg_file_write_entry(
			text_writer,
			sd_cfg_get_sfx_type_name(),
			value_string
		);
	}

	cfg_file_write_entry(
		text_writer,
		sd_cfg_get_is_sfx_digitized_name(),
		sd_is_sfx_digitized ? "1" : "0"
	);

	{
		auto value_string = std::string{};

		switch (sd_opl3_type_)
		{
			case bstone::Opl3Type::nuked:
				value_string = sd_cfg_get_nuked_str();
				break;

			default:
				value_string = sd_cfg_get_dbopl_str();
				break;
		}

		cfg_file_write_entry(
			text_writer,
			sd_cfg_get_snd_opl3_type_str(),
			value_string
		);
	}
}
// BBi
