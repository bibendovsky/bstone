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


#include "id_sd.h"

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_pm.h"

#include "bstone_audio_content_mgr.h"
#include "bstone_audio_mixer.h"
#include "bstone_endian.h"
#include "bstone_logger.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_string_helper.h"
#include "bstone_text_writer.h"


// Global variables

bool sd_has_audio_ = false;
bool sd_is_sound_enabled_ = false;
bool sd_is_music_enabled_ = false;

// Internal variables

static bool sd_started_;


// AdLib variables

bool sd_sq_active_;
bool sd_sq_played_once_;

// Internal routines

// BBi

namespace {


bstone::AudioContentMgrUPtr audio_content_mgr{};

auto sd_sfx_type = AudioSfxType::adlib;
auto sd_is_sfx_digitized = true;


} // namespace

static int sd_music_index_ = -1;
static bstone::AudioMixerUPtr sd_mixer_;

static bstone::AudioDecoderInterpolationType sd_interpolation_ = bstone::AudioDecoderInterpolationType::linear;
static bool sd_lpf_ = false;

int sd_sfx_volume_ = sd_default_sfx_volume;
int sd_music_volume_ = sd_default_music_volume;

AudioDriverType sd_audio_driver_type = AudioDriverType::auto_detect;
std::string sd_oal_library = std::string{};
std::string sd_oal_device_name = std::string{};


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

void sd_log(
	const bstone::LoggerMessageKind message_kind,
	const std::string& message)
{
	bstone::logger_->write(message_kind, sd_get_snd_string() + ' ' + message);
}

void sd_log(
	const std::string& message)
{
	bstone::logger_->write(bstone::LoggerMessageKind::information, sd_get_snd_string() + ' ' + message);
}

void sd_log_error(
	const std::string& message)
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

bool sd_enable_sound(
	const bool enable)
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

bool sd_enable_music(
	const bool enable)
{
	sd_music_off();

	sd_is_music_enabled_ = enable;

	return enable;
}

const std::string& sd_get_opl3_long_name(
	const bstone::Opl3Type opl3_type)
{
	static const auto unknown = std::string{"???"};
	static const auto dosbox_dbopl = std::string{"DosBox DBOPL"};

	switch (opl3_type)
	{
		case bstone::Opl3Type::dbopl:
			return dosbox_dbopl;

		default:
			return unknown;
	}
}

const std::string& sd_get_resampling_interpolation_long_name(
	const bstone::AudioDecoderInterpolationType interpolation_type)
{
	static const auto unknown = std::string{"none"};
	static const auto zoh = std::string{"Zero-Order Hold"};
	static const auto linear = std::string{"Linear"};

	switch (interpolation_type)
	{
		case bstone::AudioDecoderInterpolationType::zoh:
			return zoh;

		case bstone::AudioDecoderInterpolationType::linear:
			return linear;

		case bstone::AudioDecoderInterpolationType::none:
		default:
			return unknown;
	}
}

const std::string& sd_get_resampling_lpf_long_name(
	const bool is_enabled)
{
	static const auto on = std::string{"On"};
	static const auto off = std::string{"Off"};

	return is_enabled ? on : off;
}

bool sd_initialize_driver(
	AudioDriverType audio_driver_type,
	int sample_rate,
	int mix_size_ms)
try
{
	auto sd_mixer = bstone::make_audio_mixer(audio_driver_type, mt_task_manager_);

	auto param = bstone::AudioMixerInitParam{};
	param.opl3_type_ = bstone::Opl3Type::dbopl;
	param.dst_rate_ = sample_rate;
	param.mix_size_ms_ = mix_size_ms;
	param.resampling_interpolation_ = sd_interpolation_;
	param.resampling_lpf_ = sd_lpf_;

	if (sd_mixer->initialize(param))
	{
		sd_mixer_.swap(sd_mixer);
		return true;
	}
	else
	{
		return false;
	}
}
catch (const std::exception& ex)
{
	sd_log_error(std::string{"Failed to initialize mixer: "} + ex.what());
	return false;
}

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
				is_driver_initialized = sd_initialize_driver(sd_audio_driver_type, snd_rate, snd_mix_size);
				break;

			case AudioDriverType::r3_openal:
				is_driver_initialized = sd_initialize_driver(sd_audio_driver_type, snd_rate, snd_mix_size);
				break;

			case AudioDriverType::auto_detect:
			default:
				if (sd_initialize_driver(AudioDriverType::r3_openal, snd_rate, snd_mix_size))
				{
					is_driver_initialized = true;
				}
				else if (sd_initialize_driver(AudioDriverType::r2_sdl, snd_rate, snd_mix_size))
				{
					is_driver_initialized = true;
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

			sd_log("Resampling interpolation: " +
				sd_get_resampling_interpolation_long_name(sd_mixer_->get_resampling_interpolation()));

			sd_log("Resampling low-pass filter: " +
				sd_get_resampling_lpf_long_name(sd_mixer_->get_resampling_lpf()));

			audio_content_mgr = bstone::make_audio_content_mgr();
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

		if (sd_mixer_)
		{
			sd_mixer_->uninitialize();
		}

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
		}
	}
}

void sd_shutdown()
{
	if (!sd_started_)
	{
		return;
	}

	if (sd_mixer_)
	{
		sd_mixer_->uninitialize();
	}

	audio_content_mgr = nullptr;

	sd_started_ = false;
}

// Returns the sound number that's playing, or 0 if no sound is playing
bool sd_is_any_unpausable_sound_playing()
{
	if (sd_is_sound_enabled_ && sd_mixer_)
	{
		return sd_mixer_->is_any_unpausable_sfx_playing();
	}
	else
	{
		return false;
	}
}

// If a sound is playing, stops it.
void sd_stop_sound()
{
	if (sd_mixer_)
	{
		sd_mixer_->stop_pausable_sfx();
	}
}

// Waits until the current sound is done playing.
void sd_wait_sound_done()
{
	while (sd_is_any_unpausable_sound_playing())
	{
		sys_default_sleep_for();
	}
}

// Turns on the sequencer.
void sd_music_on()
{
	if (sd_mixer_)
	{
		sd_sq_active_ = true;

		const auto& audio_chunk = audio_content_mgr->get_adlib_music_chunk(sd_music_index_);

		sd_mixer_->play_adlib_music(sd_music_index_, audio_chunk.data, audio_chunk.data_size);
	}
}

// Turns off the sequencer and any playing notes.
void sd_music_off()
{
	if (sd_mixer_)
	{
		sd_sq_active_ = false;
		sd_mixer_->stop_music();
	}
}

// Starts playing the music pointed to.
void sd_start_music(
	const int index)
{
	sd_music_off();

	sd_sq_played_once_ = false;

	if (sd_is_music_enabled_)
	{
		sd_music_index_ = index;

		sd_set_music_volume(sd_music_volume_);

		sd_music_on();
	}
	else
	{
		sd_sq_played_once_ = true;
	}
}

// BBi

void sd_play_sound(
	const int sound_index,
	const void* actor,
	const bstone::ActorType actor_type,
	const bstone::ActorChannel actor_channel)
{
	if (!sd_mixer_ || !sd_is_sound_enabled_)
	{
		return;
	}

	const auto& audio_chunk = audio_content_mgr->get_sfx_chunk(sound_index);

	if (!audio_chunk.data)
	{
		return;
	}

	auto actor_index = -1;

	if (actor)
	{
		switch (actor_type)
		{
			case bstone::ActorType::actor:
				actor_index = static_cast<int>(static_cast<const objtype*>(actor) - objlist);
				break;

			case bstone::ActorType::door:
				actor_index = static_cast<int>(static_cast<const doorobj_t*>(actor) - doorobjlist);
				break;

			default:
				return;
		}
	}

	const auto priority = audio_content_mgr->get_sfx_priority(sound_index);

	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_sfx:
			sd_mixer_->play_adlib_sound(
				sound_index,
				priority,
				audio_chunk.data,
				audio_chunk.data_size,
				actor_index,
				actor_type,
				actor_channel
			);

			break;

		case AudioChunkType::pc_speaker:
			sd_mixer_->play_pc_speaker_sound(
				sound_index,
				priority,
				audio_chunk.data,
				audio_chunk.data_size,
				actor_index,
				actor_type,
				actor_channel
			);

			break;

		case AudioChunkType::digitized:
			sd_mixer_->play_pcm_sound(
				sound_index,
				priority,
				audio_chunk.data,
				audio_chunk.data_size,
				actor_index,
				actor_type,
				actor_channel
			);

			break;

		case AudioChunkType::adlib_music:
		default:
			break;
	}
}

void sd_play_actor_sound(
	const int sound_index,
	const objtype* actor,
	const bstone::ActorChannel actor_channel)
{
	sd_play_sound(
		sound_index,
		actor,
		bstone::ActorType::actor,
		actor_channel
	);
}

void sd_play_player_sound(
	const int sound_index,
	const bstone::ActorChannel actor_channel)
{
	sd_play_sound(
		sound_index,
		player,
		bstone::ActorType::actor,
		actor_channel
	);
}

void sd_play_door_sound(
	const int sound_index,
	const doorobj_t* door)
{
	sd_play_sound(
		sound_index,
		door,
		bstone::ActorType::door,
		bstone::ActorChannel::voice
	);
}

void sd_play_wall_sound(
	const int sound_index)
{
	sd_play_sound(
		sound_index,
		nullptr,
		bstone::ActorType::wall,
		bstone::ActorChannel::voice
	);
}

void sd_update_positions()
{
	if (sd_mixer_)
	{
		sd_mixer_->update_positions();
	}
}

bool sd_is_player_channel_playing(
	const bstone::ActorChannel channel)
{
	if (sd_mixer_)
	{
		return sd_mixer_->is_player_channel_playing(channel);
	}
	else
	{
		return false;
	}
}

void sd_set_sfx_volume(
	const int volume)
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

	sd_mixer_->set_sfx_volume(static_cast<float>(new_volume) / sd_max_volume);
}

void sd_set_music_volume(
	const int volume)
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

	sd_mixer_->set_music_volume(static_cast<float>(new_volume) / sd_max_volume);
}

void sd_mute(
	const bool mute)
{
	if (sd_mixer_)
	{
		sd_mixer_->set_mute(mute);
	}
}

void sd_pause_sfx(
	const bool is_pause)
{
	if (sd_mixer_)
	{
		sd_mixer_->pause_all_sfx(is_pause);
	}
}

void sd_pause_music(
	const bool is_pause)
{
	if (sd_mixer_)
	{
		sd_mixer_->pause_music(is_pause);
	}
}

bstone::AudioDecoderInterpolationType sd_get_resampling_interpolation() noexcept
{
	return sd_interpolation_;
}

void sd_cfg_set_resampling_interpolation(
	const bstone::AudioDecoderInterpolationType interpolation_type)
{
	sd_interpolation_ = interpolation_type;
}

bool sd_cfg_get_resampling_low_pass_filter() noexcept
{
	return sd_lpf_;
}

void sd_cfg_set_resampling_low_pass_filter(
	const bool is_enabled)
{
	sd_lpf_ = is_enabled;
}

void apply_digitized_sfx()
{
	if (!audio_content_mgr)
	{
		return;
	}

	audio_content_mgr->set_is_sfx_digitized(sd_is_sfx_digitized);
}

void sd_apply_resampling()
{
	if (!sd_has_audio_ || !sd_mixer_)
	{
		return;
	}

	sd_log();
	sd_log("Applying resample parameters.");

	if (!sd_mixer_->set_resampling(sd_interpolation_, sd_lpf_))
	{
		sd_log_error("Failed to apply resample parameters.");

		return;
	}

	sd_log("Resampling interpolation: " +
		sd_get_resampling_interpolation_long_name(sd_mixer_->get_resampling_interpolation()));

	sd_log("Resampling low-pass filter: " +
		sd_get_resampling_lpf_long_name(sd_mixer_->get_resampling_lpf()));
}

AudioSfxType sd_cfg_get_sfx_type() noexcept
{
	return sd_sfx_type;
}

void sd_cfg_set_sfx_type(
	AudioSfxType sfx_type)
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

void sd_cfg_set_is_sfx_digitized(
	bool is_sfx_digitized)
{
	sd_is_sfx_digitized = is_sfx_digitized;
}

void sd_cfg_set_defaults()
{
	sd_interpolation_ = bstone::AudioDecoderInterpolationType::linear;
	sd_lpf_ = true;

	sd_sfx_type = AudioSfxType::adlib;
	sd_is_sfx_digitized = true;

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

const std::string& sd_cfg_get_zoh_name()
{
	static const auto result = std::string{"zoh"};
	return result;
}

const std::string& sd_cfg_get_linear_name()
{
	static const auto result = std::string{"linear"};
	return result;
}

const std::string& sd_cfg_get_resampling_interpolation_name()
{
	static const auto result = std::string{"snd_resampling_interpolation"};
	return result;
}

const std::string& sd_cfg_get_resampling_lpf_name()
{
	static const auto result = std::string{"snd_resampling_lpf"};
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

bool sd_cfg_parse_key_value(
	const std::string& key_string,
	const std::string& value_string)
{
	if (key_string == sd_cfg_get_resampling_interpolation_name())
	{
		if (value_string == sd_cfg_get_zoh_name())
		{
			sd_cfg_set_resampling_interpolation(bstone::AudioDecoderInterpolationType::zoh);
		}
		else if (value_string == sd_cfg_get_linear_name())
		{
			sd_cfg_set_resampling_interpolation(bstone::AudioDecoderInterpolationType::linear);
		}

		return true;
	}
	else if (key_string == sd_cfg_get_resampling_lpf_name())
	{
		int value = 0;

		if (bstone::StringHelper::string_to_int(value_string, value))
		{
			sd_cfg_set_resampling_low_pass_filter(value != 0);
		}

		return true;
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

	return false;
}

void sd_cfg_parse_cl()
{
	{
		const auto& value_string = g_args.get_option_value(sd_cfg_get_resampling_interpolation_name());

		if (!value_string.empty())
		{
			sd_cfg_parse_key_value(sd_cfg_get_resampling_interpolation_name(), value_string);
		}
	}

	{
		const auto& value_string = g_args.get_option_value(sd_cfg_get_resampling_lpf_name());

		if (!value_string.empty())
		{
			sd_cfg_parse_key_value(sd_cfg_get_resampling_lpf_name(), value_string);
		}
	}

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
}

void sd_cfg_write(
	bstone::TextWriter& text_writer)
{
	// Interpolation.
	//
	{
		auto value_string = std::string{};

		switch (sd_interpolation_)
		{
			case bstone::AudioDecoderInterpolationType::zoh:
				value_string = sd_cfg_get_zoh_name();
				break;

			case bstone::AudioDecoderInterpolationType::linear:
			default:
				value_string = sd_cfg_get_linear_name();
				break;
		}

		cfg_file_write_entry(
			text_writer,
			sd_cfg_get_resampling_interpolation_name(),
			value_string
		);
	}

	// LPF
	//
	cfg_file_write_entry(
		text_writer,
		sd_cfg_get_resampling_lpf_name(),
		std::to_string(sd_lpf_)
	);

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
}
// BBi
