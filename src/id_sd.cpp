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


#include "id_sd.h"

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_pm.h"

#include "bstone_audio_mixer.h"
#include "bstone_endian.h"
#include "bstone_logger.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_string_helper.h"
#include "bstone_text_writer.h"


void InitDigiMap();


std::uint16_t sd_start_pc_sounds_ = STARTPCSOUNDS;
std::uint16_t sd_start_al_sounds_ = STARTADLIBSOUNDS;

auto sd_base_index_ = 0;

std::int16_t sd_last_sound_ = LASTSOUND;
std::int16_t sd_digi_map_[LASTSOUND];


// Global variables

bool sd_has_audio_ = false;
bool sd_is_sound_enabled_ = false;
bool sd_is_music_enabled_ = false;

// Internal variables

static bool sd_started_;

using SdDigiList = std::vector<std::uint16_t>;
SdDigiList sd_digi_list_;

// AdLib variables

bool sd_sq_active_;
std::uint16_t* sd_sq_hack_;
std::uint16_t sd_sq_hack_len_;
bool sd_sq_played_once_;

// Internal routines

// BBi

static int sd_music_index_ = -1;
static bstone::AudioMixerUPtr sd_mixer_;

static bstone::AudioDecoderInterpolationType sd_interpolation_ = bstone::AudioDecoderInterpolationType::linear;
static bool sd_lpf_ = true;

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


//
// Stuff for digitized sounds
//

void sd_setup_digi()
{
	const std::uint16_t* p;
	int pg;
	int i;

	p = static_cast<const std::uint16_t*>(PM_GetPage(ChunksInFile - 1));
	pg = PMSoundStart;
	for (i = 0; i < static_cast<int>(PMPageSize / (2 * 2)); ++i)
	{
		if (pg >= ChunksInFile - 1)
		{
			break;
		}
		pg += (bstone::Endian::little(p[1]) + (PMPageSize - 1)) / PMPageSize;
		p += 2;
	}
	sd_digi_list_.resize(i * 2);

	const std::uint16_t* src_list = static_cast<const std::uint16_t*>(
		PM_GetPage(ChunksInFile - 1));

	for (auto j = 0; j < (i * 2); ++j)
	{
		sd_digi_list_[j] = bstone::Endian::little(src_list[j]);
	}

	for (i = 0; i < sd_last_sound_; i++)
	{
		sd_digi_map_[i] = -1;
	}

	InitDigiMap();
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

	sd_base_index_ = sd_start_al_sounds_;
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
	static const auto unknown = std::string{"???"};
	static const auto zoh = std::string{"Zero-Order Hold"};
	static const auto linear = std::string{"Linear"};

	switch (interpolation_type)
	{
		case bstone::AudioDecoderInterpolationType::zoh:
			return zoh;

		case bstone::AudioDecoderInterpolationType::linear:
			return linear;

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

		try
		{
			sd_mixer_ = bstone::make_audio_mixer(sd_audio_driver_type, mt_task_manager_);

			auto param = bstone::AudioMixerInitParam{};
			param.opl3_type_ = bstone::Opl3Type::dbopl;
			param.dst_rate_ = snd_rate;
			param.mix_size_ms_ = snd_mix_size;
			param.resampling_interpolation_ = sd_interpolation_;
			param.resampling_lpf_ = sd_lpf_;

			if (sd_mixer_->initialize(param))
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
			}
			else
			{
				sd_log_error("Failed to initialize mixer.");
			}
		}
		catch (const std::exception& ex)
		{
			sd_log_error(std::string{"Failed to initialize mixer: "} + ex.what());
		}
	}
	else
	{
		sd_log("Audio subsystem disabled.");

		if (sd_mixer_)
		{
			sd_mixer_->uninitialize();
		}
	}

	sd_setup_digi();

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

	// Free music data
	for (int i = 0; i < LASTMUSIC; ++i)
	{
		audiosegs[STARTMUSIC + i] = std::move(AudioSegment{});
	}

	sd_started_ = false;
}

// Returns the sound number that's playing, or 0 if no sound is playing
bool sd_sound_playing()
{
	if (sd_is_sound_enabled_ && sd_mixer_)
	{
		return sd_mixer_->is_any_sfx_playing();
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
	while (sd_sound_playing())
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
		sd_mixer_->play_adlib_music(sd_music_index_, sd_sq_hack_, sd_sq_hack_len_);
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

		const auto music_data = reinterpret_cast<std::uint16_t*>(
			audiosegs[STARTMUSIC + index].data());

		const auto length = sd_get_adlib_music_data_size(music_data);

		sd_sq_hack_ = music_data;
		sd_sq_hack_len_ = static_cast<std::uint16_t>(length);

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
	if (sound_index < 0)
	{
		return;
	}

	if (!sd_is_sound_enabled_)
	{
		return;
	}

	int actor_index = -1;

	if (actor)
	{
		switch (actor_type)
		{
		case bstone::ActorType::actor:
			actor_index = static_cast<int>(
				static_cast<const objtype*>(actor) - objlist);
			break;

		case bstone::ActorType::door:
			actor_index = static_cast<int>(
				static_cast<const doorobj_t*>(actor) - doorobjlist);
			break;

		default:
			return;
		}
	}

	const auto sound = reinterpret_cast<SoundCommon*>(audiosegs[sd_base_index_ + sound_index].data());

	if (!sound)
	{
		return;
	}

	if (sd_is_sound_enabled_ && !sound)
	{
		Quit("Uncached sound.");
	}

	if (!sd_mixer_)
	{
		return;
	}

	int priority = bstone::Endian::little(sound->priority);

	const auto sfx_info = sd_get_sfx_info(sound_index);

	if (!sfx_info.data_ || sfx_info.size_ <= 0)
	{
		return;
	}

	if (sfx_info.is_digitized_)
	{
		sd_mixer_->play_pcm_sound(
			sfx_info.digi_index_,
			priority,
			sfx_info.data_,
			sfx_info.size_,
			actor_index,
			actor_type,
			actor_channel
		);
	}
	else
	{
		sd_mixer_->play_adlib_sound(
			sound_index,
			priority,
			sfx_info.data_,
			sfx_info.size_,
			actor_index,
			actor_type,
			actor_channel
		);
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
		actor_channel);
}

void sd_play_player_sound(
	const int sound_index,
	const bstone::ActorChannel actor_channel)
{
	sd_play_sound(
		sound_index,
		player,
		bstone::ActorType::actor,
		actor_channel);
}

void sd_play_door_sound(
	const int sound_index,
	const doorobj_t* door)
{
	sd_play_sound(
		sound_index,
		door,
		bstone::ActorType::door,
		bstone::ActorChannel::voice);
}

void sd_play_wall_sound(
	const int sound_index)
{
	sd_play_sound(
		sound_index,
		nullptr,
		bstone::ActorType::wall,
		bstone::ActorChannel::voice);
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

int sd_get_adlib_music_data_size(
	const void* const raw_music_data)
{
	return bstone::Endian::little(reinterpret_cast<const std::uint16_t*>(raw_music_data)[0]) + 2;
}

SfxInfo sd_get_sfx_info(
	const int sfx_number)
{
	auto result = SfxInfo{};

	if (sfx_number >= 0 && sfx_number < NUMSOUNDS)
	{
		const auto digi_index = sd_digi_map_[sfx_number];

		if (digi_index >= 0)
		{
			const auto digi_page = sd_digi_list_[(2 * digi_index) + 0];

			result.is_digitized_ = true;
			result.digi_index_ = digi_index;
			result.data_ = PM_GetSoundPage(digi_page);
			result.size_ = sd_digi_list_[(2 * digi_index) + 1];
		}
		else
		{
			const auto start_index = sd_start_al_sounds_ + sfx_number;

			result.is_digitized_ = false;
			result.digi_index_ = -1;
			result.data_ = audiosegs[sd_base_index_ + sfx_number].data();
			result.size_ = audiostarts[start_index + 1] - audiostarts[start_index];
		}
	}

	return result;
}

void sd_setup_extracting()
{
	sd_setup_digi();
	sd_base_index_ = sd_start_al_sounds_;
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

void sd_cfg_set_defaults()
{
	sd_interpolation_ = bstone::AudioDecoderInterpolationType::linear;
	sd_lpf_ = true;
}

const std::string sd_cfg_get_2d_sdl_name()
{
	static const auto result = std::string{"2d_sdl"};
	return result;
}

const std::string sd_cfg_get_3d_openal_name()
{
	static const auto result = std::string{"3d_openal"};
	return result;
}

const std::string sd_cfg_get_auto_detect_name()
{
	static const auto result = std::string{"auto-detect"};
	return result;
}

const std::string sd_cfg_get_driver_name()
{
	static const auto result = std::string{"snd_driver"};
	return result;
}

const std::string sd_cfg_get_oal_library_name()
{
	static const auto result = std::string{"snd_oal_library"};
	return result;
}

const std::string sd_cfg_get_oal_device_name_name()
{
	static const auto result = std::string{"snd_oal_device_name"};
	return result;
}

const std::string sd_cfg_get_zoh_name()
{
	static const auto result = std::string{"zoh"};
	return result;
}

const std::string sd_cfg_get_linear_name()
{
	static const auto result = std::string{"linear"};
	return result;
}

const std::string sd_cfg_get_resampling_interpolation_name()
{
	static const auto result = std::string{"snd_resampling_interpolation"};
	return result;
}

const std::string sd_cfg_get_resampling_lpf_name()
{
	static const auto result = std::string{"snd_resampling_lpf"};
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
}
// BBi
