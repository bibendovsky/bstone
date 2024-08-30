/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <vector>
#include "id_sd.h"
#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "bstone_algorithm.h"
#include "bstone_audio_content_mgr.h"
#include "bstone_audio_mixer.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_math.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_scope_exit.h"
#include "bstone_string_helper.h"
#include "bstone_text_writer.h"
#include "bstone_voice_group.h"

constexpr auto max_voices =
	MAXACTORS +
	MAXDOORS +
	32;

// Global variables

// Internal variables

static bool sd_started_;


// AdLib variables

bool sd_sq_active_;

// Internal routines


namespace {

auto sd_music_is_looping_ = true;

bstone::AudioContentMgrUPtr audio_content_mgr{};

auto sd_use_voice_output_gains_ = false;

auto sd_music_voice_group_ = bstone::VoiceGroupUPtr{};
auto sd_ui_sfx_voice_group_ = bstone::VoiceGroupUPtr{};
auto sd_scene_sfx_voice_group_ = bstone::VoiceGroupUPtr{};

auto sd_music_voice_ = bstone::Voice{};
auto sd_ui_sfx_voice_ = bstone::Voice{};
auto sd_pwall_voice_ = bstone::Voice{};

auto sd_player_item_voice_ = bstone::Voice{};
auto sd_player_hit_wall_voice_ = bstone::Voice{};
auto sd_player_no_way_voice_ = bstone::Voice{};
auto sd_player_interrogation_voice_ = bstone::Voice{};

auto sd_player_x_ = 0.0;
auto sd_player_y_ = 0.0;

auto sd_viewsin_ = 0.0;
auto sd_viewcos_ = 0.0;

auto sd_listener_r3_position_ = bstone::AudioMixerR3Vector{};

} // namespace


static int sd_music_index_ = -1;
static bstone::AudioMixerUPtr sd_mixer_;

namespace {

constexpr auto snd_auto_detect_string = bstone::StringView{"auto-detect"};

// snd_is_disabled

constexpr auto snd_is_disabled_cvar_name = bstone::StringView{"snd_is_disabled"};
constexpr auto snd_is_disabled_cvar_default = false;

auto snd_is_disabled_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_disabled_cvar_name,
	bstone::CVarFlags::archive,
	snd_is_disabled_cvar_default};

// snd_rate

constexpr auto snd_rate_cvar_name = bstone::StringView{"snd_rate"};
constexpr auto snd_rate_cvar_default = 0;

auto snd_rate_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_rate_cvar_name,
	bstone::CVarFlags::none,
	snd_rate_cvar_default};

// snd_mix_size

constexpr auto snd_mix_size_cvar_name = bstone::StringView{"snd_mix_size"};
constexpr auto snd_mix_size_cvar_default = 20;

auto snd_mix_size_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_mix_size_cvar_name,
	bstone::CVarFlags::none,
	snd_mix_size_cvar_default};

// snd_driver

constexpr auto snd_driver_cvar_name = bstone::StringView{"snd_driver"};
constexpr auto snd_driver_cvar_system = bstone::StringView{"system"};
constexpr auto snd_driver_cvar_openal = bstone::StringView{"openal"};

constexpr bstone::StringView snd_driver_cvar_values[] =
{
	snd_auto_detect_string,
	snd_driver_cvar_system,
	snd_driver_cvar_openal,
};

auto snd_driver_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_driver_cvar_name,
	bstone::CVarFlags::archive,
	snd_auto_detect_string,
	bstone::make_span(snd_driver_cvar_values)};

// snd_oal_library

constexpr auto snd_oal_library_cvar_name = bstone::StringView{"snd_oal_library"};

auto snd_oal_library_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_oal_library_cvar_name,
	bstone::CVarFlags::archive,
	bstone::StringView{}};

// snd_oal_device_name

constexpr auto snd_oal_device_name_cvar_name = bstone::StringView{"snd_oal_device_name"};

auto snd_oal_device_name_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_oal_device_name_cvar_name,
	bstone::CVarFlags::archive,
	bstone::StringView{}};

// snd_opl3_type

constexpr auto snd_opl3_type_cvar_name = bstone::StringView{"snd_opl3_type"};
constexpr auto snd_opl3_type_cvar_dbopl = bstone::StringView{"dbopl"};
constexpr auto snd_opl3_type_cvar_nuked = bstone::StringView{"nuked"};

constexpr bstone::StringView snd_opl3_type_cvar_values[] =
{
	snd_opl3_type_cvar_dbopl,
	snd_opl3_type_cvar_nuked,
};

auto snd_opl3_type_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_opl3_type_cvar_name,
	bstone::CVarFlags::archive,
	snd_opl3_type_cvar_dbopl,
	bstone::make_span(snd_opl3_type_cvar_values)};

// snd_is_sfx_enabled

constexpr auto snd_is_sfx_enabled_cvar_name = bstone::StringView{"snd_is_sfx_enabled"};
constexpr auto snd_is_sfx_enabled_cvar_default = true;

auto snd_is_sfx_enabled_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_sfx_enabled_cvar_name,
	bstone::CVarFlags::archive,
	snd_is_sfx_enabled_cvar_default};

// snd_sfx_type

constexpr auto snd_sfx_type_cvar_name = bstone::StringView{"snd_sfx_type"};
constexpr auto snd_sfx_type_cvar_pc_speaker = bstone::StringView{"pc_speaker"};
constexpr auto snd_sfx_type_cvar_adlib = bstone::StringView{"adlib"};

constexpr bstone::StringView snd_sfx_type_cvar_values[] =
{
	snd_sfx_type_cvar_adlib,
	snd_sfx_type_cvar_pc_speaker,
};

auto snd_sfx_type_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_sfx_type_cvar_name,
	bstone::CVarFlags::archive,
	snd_sfx_type_cvar_adlib,
	bstone::make_span(snd_sfx_type_cvar_values)};

// snd_is_sfx_digitized

constexpr auto snd_is_sfx_digitized_cvar_name = bstone::StringView{"snd_is_sfx_digitized"};
constexpr auto snd_is_sfx_digitized_cvar_default = true;

auto snd_is_sfx_digitized_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_sfx_digitized_cvar_name,
	bstone::CVarFlags::archive,
	snd_is_sfx_digitized_cvar_default};

// snd_sfx_volume

constexpr auto snd_sfx_volume_cvar_name = bstone::StringView{"snd_sfx_volume"};

auto snd_sfx_volume_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_sfx_volume_cvar_name,
	bstone::CVarFlags::archive,
	sd_default_sfx_volume,
	sd_min_volume,
	sd_max_volume};

// snd_is_music_enabled

constexpr auto snd_is_music_enabled_cvar_name = bstone::StringView{"snd_is_music_enabled"};
constexpr auto snd_is_music_enabled_cvar_default = true;

auto snd_is_music_enabled_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_music_enabled_cvar_name,
	bstone::CVarFlags::archive,
	snd_is_music_enabled_cvar_default};

// snd_music_volume

constexpr auto snd_music_volume_cvar_name = bstone::StringView{"snd_music_volume"};

auto snd_music_volume_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_music_volume_cvar_name,
	bstone::CVarFlags::archive,
	sd_default_music_volume,
	sd_min_volume,
	sd_max_volume};

} // namespace

void sd_initialize_cvars(bstone::CVarMgr& cvar_mgr)
{
	cvar_mgr.add(snd_is_disabled_cvar);
	cvar_mgr.add(snd_rate_cvar);
	cvar_mgr.add(snd_mix_size_cvar);
	cvar_mgr.add(snd_driver_cvar);
	cvar_mgr.add(snd_oal_library_cvar);
	cvar_mgr.add(snd_oal_device_name_cvar);
	cvar_mgr.add(snd_opl3_type_cvar);
	cvar_mgr.add(snd_is_sfx_enabled_cvar);
	cvar_mgr.add(snd_sfx_type_cvar);
	cvar_mgr.add(snd_is_sfx_digitized_cvar);
	cvar_mgr.add(snd_sfx_volume_cvar);
	cvar_mgr.add(snd_is_music_enabled_cvar);
	cvar_mgr.add(snd_music_volume_cvar);
}

const std::string& sd_get_snd_string()
{
	static const auto result = std::string{"[SND]"};

	return result;
}

void sd_log()
{
	bstone::globals::logger->log_information();
}

void sd_log(const bstone::LoggerMessageType message_type, const std::string& message)
{
	bstone::globals::logger->log(message_type, (sd_get_snd_string() + ' ' + message).c_str());
}

void sd_log(const std::string& message)
{
	bstone::globals::logger->log(
		bstone::LoggerMessageType::information,
		(sd_get_snd_string() + ' ' + message).c_str());
}

void sd_log_error(const std::string& message)
{
	sd_log(bstone::LoggerMessageType::error, message);
}


bool sd_is_sound_enabled() noexcept
{
	return snd_is_sfx_enabled_cvar.get_bool();
}

void sd_set_is_sound_enabled(bool is_enabled) noexcept
{
	snd_is_sfx_enabled_cvar.set_bool(is_enabled);
}

bool sd_enable_sound(bool enable)
{
	sd_stop_sfx_sound();

	if (enable && !sd_has_audio())
	{
		enable = false;
	}

	sd_set_is_sound_enabled(enable);
	sd_set_sfx_volume();
	return enable;
}

bool sd_is_music_enabled() noexcept
{
	return snd_is_music_enabled_cvar.get_bool();
}

void sd_set_is_music_enabled(bool is_enabled) noexcept
{
	snd_is_music_enabled_cvar.set_bool(is_enabled);
}

bool sd_enable_music(bool enable)
{
	sd_music_off();
	sd_set_is_music_enabled(enable);
	sd_set_music_volume();
	return enable;
}

bstone::Opl3Type sd_get_opl3_type_from_cvar() noexcept
{
	const auto opl3_type_sv = snd_opl3_type_cvar.get_string();

	if (opl3_type_sv == snd_opl3_type_cvar_dbopl)
	{
		return bstone::Opl3Type::dbopl;
	}

	if (opl3_type_sv == snd_opl3_type_cvar_nuked)
	{
		return bstone::Opl3Type::nuked;
	}

	return bstone::Opl3Type::none;
}

const std::string& sd_get_opl3_long_name(const bstone::Opl3Type opl3_type)
{
	static const auto unknown = std::string{"???"};
	static const auto dosbox_dbopl = std::string{"DBOPL"};
	static const auto nuked_opl3 = std::string{"Nuked"};

	switch (opl3_type)
	{
		case bstone::Opl3Type::dbopl: return dosbox_dbopl;
		case bstone::Opl3Type::nuked: return nuked_opl3;
		default: return unknown;
	}
}

namespace {

void sd_initialize_voice(bstone::Voice& voice)
{
	voice.use_output_gains = false;
	voice.handle.reset();
	voice.gain = bstone::audio_mixer_max_gain;
	voice.output_gains.fill(1.0);
}

void sd_initialize_voices()
{
	sd_initialize_voice(sd_music_voice_);

	sd_initialize_voice(sd_ui_sfx_voice_);
	sd_initialize_voice(sd_pwall_voice_);

	sd_initialize_voice(sd_player_item_voice_);
	sd_initialize_voice(sd_player_hit_wall_voice_);
	sd_initialize_voice(sd_player_no_way_voice_);
	sd_initialize_voice(sd_player_interrogation_voice_);
}

void sd_make_mixer(AudioDriverType audio_driver_type, int sample_rate, int mix_size_ms)
try {
	sd_mixer_ = nullptr;
	sd_music_voice_group_ = nullptr;
	sd_ui_sfx_voice_group_ = nullptr;
	sd_scene_sfx_voice_group_ = nullptr;

	auto param = bstone::AudioMixerInitParam{};
	param.audio_driver_type = audio_driver_type;
	param.opl3_type = sd_get_opl3_type_from_cvar();
	param.dst_rate = sample_rate;
	param.mix_size_ms = mix_size_ms;
	param.max_voices = max_voices;
	auto sd_mixer = bstone::make_audio_mixer(param);
	const auto use_voice_output_gains = sd_mixer->can_set_voice_output_gains();

	auto music_voice_group = bstone::make_voice_group(*sd_mixer);
	auto ui_sfx_voice_group = bstone::make_voice_group(*sd_mixer);
	auto scene_sfx_voice_group = bstone::make_voice_group(*sd_mixer);

	sd_use_voice_output_gains_ = use_voice_output_gains;
	sd_mixer_.swap(sd_mixer);
	sd_music_voice_group_.swap(music_voice_group);
	sd_ui_sfx_voice_group_.swap(ui_sfx_voice_group);
	sd_scene_sfx_voice_group_.swap(scene_sfx_voice_group);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

AudioDriverType sd_get_driver_type_from_cvar() noexcept
{
	const auto driver_sv = snd_driver_cvar.get_string();

	if (driver_sv == snd_driver_cvar_system)
	{
		return AudioDriverType::system;
	}

	if (driver_sv == snd_driver_cvar_openal)
	{
		return AudioDriverType::openal;
	}

	return AudioDriverType::auto_detect;
}

AudioSfxType sd_get_sfx_type_from_cvar() noexcept
{
	const auto sfx_type_sv = snd_sfx_type_cvar.get_string();

	if (sfx_type_sv == snd_sfx_type_cvar_pc_speaker)
	{
		return AudioSfxType::pc_speaker;
	}

	return AudioSfxType::adlib;
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

	if (sd_has_audio())
	{
		const auto rate = snd_rate_cvar.get_int32();
		const auto mix_size = snd_mix_size_cvar.get_int32();
		const auto user_driver_type = sd_get_driver_type_from_cvar();

		using DriverTypes = std::vector<AudioDriverType>;
		auto driver_types = DriverTypes{};
		driver_types.reserve(8);

		if (user_driver_type == AudioDriverType::auto_detect)
		{
			driver_types.emplace_back(AudioDriverType::openal);
			driver_types.emplace_back(AudioDriverType::system);
		}
		else
		{
			driver_types.emplace_back(user_driver_type);
		}

		auto is_driver_initialized = false;

		for (const auto& driver_type : driver_types)
		{
			try
			{
				sd_make_mixer(driver_type, rate, mix_size);
				is_driver_initialized = true;
			}
			catch (...)
			{
				bstone::globals::logger->log_current_exception();
			}

			if (is_driver_initialized)
			{
				break;
			}
		}

		if (is_driver_initialized)
		{
			sd_log("Channel count: " + std::to_string(sd_mixer_->get_channel_count()));
			sd_log("Sample rate: " + std::to_string(sd_mixer_->get_rate()) + " Hz");
			sd_log("Mix size: " + std::to_string(sd_mixer_->get_mix_size_ms()) + " ms");
			sd_log("Effects volume: " + std::to_string(sd_get_sfx_volume()) + " / " + std::to_string(sd_max_volume));
			sd_log("Music volume: " + std::to_string(sd_get_music_volume()) + " / " + std::to_string(sd_max_volume));
			sd_log("OPL3 type: " + sd_get_opl3_long_name(sd_mixer_->get_opl3_type()));

			audio_content_mgr = bstone::make_audio_content_mgr(*bstone::globals::page_mgr);
			audio_content_mgr->set_sfx_type(sd_get_sfx_type_from_cvar());
			audio_content_mgr->set_is_sfx_digitized(snd_is_sfx_digitized_cvar.get_bool());
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

	if (sd_has_audio())
	{
		sd_initialize_voices();

		sd_enable_sound(sd_is_sound_enabled());

		sd_enable_music(snd_is_music_enabled_cvar.get_bool());
		sd_music_on(sd_music_is_looping_);
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

	sd_music_voice_group_ = nullptr;
	sd_ui_sfx_voice_group_ = nullptr;
	sd_scene_sfx_voice_group_ = nullptr;

	sd_use_voice_output_gains_ = false;
	sd_started_ = false;
}

bool sd_is_playing_any_ui_sound()
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled() || sd_ui_sfx_voice_group_ == nullptr)
	{
		return false;
	}

	return sd_ui_sfx_voice_group_->is_any_playing();
}

bool sd_is_music_playing()
{
	if (sd_mixer_ == nullptr || !sd_is_music_enabled())
	{
		return false;
	}

	return sd_music_voice_group_->is_any_playing();
}

bool sd_has_audio() noexcept
{
	return !snd_is_disabled_cvar.get_bool();
}

// If a sound is playing, stops it.
void sd_stop_sfx_sound()
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_ui_sfx_voice_group_->stop();
	sd_scene_sfx_voice_group_->stop();
}

// Waits until the current sound is done playing.
void sd_wait_sound_done()
{
	if (sd_mixer_ == nullptr ||
		!sd_is_sound_enabled() ||
		sd_ui_sfx_voice_group_ == nullptr ||
		sd_scene_sfx_voice_group_ == nullptr)
	{
		return;
	}

	while (sd_ui_sfx_voice_group_->is_any_playing() || sd_scene_sfx_voice_group_->is_any_playing())
	{
		sys_default_sleep_for();
	}
}

// Turns on the sequencer.
void sd_music_on(bool is_looping)
{
	if (sd_mixer_ == nullptr || !sd_is_music_enabled() || sd_music_index_ < 0)
	{
		return;
	}

	sd_music_is_looping_ = is_looping;

	sd_sq_active_ = true;
	sd_music_voice_group_->stop();

	const auto& audio_chunk = audio_content_mgr->get_adlib_music_chunk(sd_music_index_);

	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = bstone::SoundType::adlib_music;
	play_sound_param.sound_index = 0;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = is_looping;
	play_sound_param.is_r3 = false;
	sd_music_voice_.handle = sd_mixer_->play_sound(play_sound_param);
	sd_music_voice_group_->add_voice(sd_music_voice_);
}

// Turns off the sequencer and any playing notes.
void sd_music_off()
{
	if (sd_mixer_ == nullptr || !sd_is_music_enabled())
	{
		return;
	}

	sd_sq_active_ = false;
	sd_music_voice_group_->stop();
}

// Starts playing the music pointed to.
void sd_start_music(int index, bool is_looping)
{
	sd_music_off();
	sd_set_music_volume();
	sd_music_index_ = index;

	if (sd_is_music_enabled())
	{
		sd_music_on(is_looping);
	}
}

namespace {

constexpr auto ATABLEMAX = 15;

int righttable[ATABLEMAX][ATABLEMAX * 2] =
{
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

int lefttable[ATABLEMAX][ATABLEMAX * 2] =
{
	{8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

void sd_calculate_w3d_indices(
	double emitter_x,
	double emitter_y,
	int& w3d_left_index,
	int& w3d_right_index)
{
	const auto gx = emitter_x - sd_player_x_;
	const auto gy = emitter_y - sd_player_y_;

	const auto xt_c = gx * sd_viewcos_;
	const auto yt_s = gy * sd_viewsin_;
	const auto x = bstone::clamp(std::abs(static_cast<int>(xt_c - yt_s)), 0, ATABLEMAX - 1);

	const auto xt_s = gx * sd_viewsin_;
	const auto yt_c = gy * sd_viewcos_;
	const auto y = bstone::clamp(static_cast<int>(yt_c + xt_s), -ATABLEMAX, ATABLEMAX - 1);

	w3d_left_index = lefttable[x][y + ATABLEMAX];
	w3d_right_index = righttable[x][y + ATABLEMAX];
}

void sd_calculate_voice_gain(
	const bstone::AudioMixerVoiceR3Position& voice_r3_position,
	bstone::Voice& voice)
{
	int w3d_left_index;
	int w3d_right_index;
	sd_calculate_w3d_indices(voice_r3_position.x, voice_r3_position.z, w3d_left_index, w3d_right_index);

	const auto index = (w3d_left_index + w3d_right_index) / 2.0;
	const auto gain = 1.0 - (index / 9.0);
	voice.gain = gain;
}

void sd_calculate_voice_output_gains(
	const bstone::AudioMixerVoiceR3Position& voice_r3_position,
	bstone::Voice& voice)
{
	int w3d_left_index;
	int w3d_right_index;
	sd_calculate_w3d_indices(voice_r3_position.x, voice_r3_position.z, w3d_left_index, w3d_right_index);

	voice.output_gains[0] = 1.0 - (w3d_left_index / 9.0);
	voice.output_gains[1] = 1.0 - (w3d_right_index / 9.0);
}

}

void sd_play_non_positional_sfx_sound(
	int sound_index,
	bstone::Voice& voice,
	bstone::VoiceGroup& voice_group)
{
	voice_group.stop_and_remove_voice(voice);

	const auto& audio_chunk = audio_content_mgr->get_sfx_chunk(sound_index);

	if (audio_chunk.data == nullptr || audio_chunk.data_size == 0)
	{
		return;
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
			BSTONE_THROW_STATIC_SOURCE("Unknown audio chunk type.");
	}

	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = sound_type;
	play_sound_param.sound_index = sound_index;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = false;
	play_sound_param.is_r3 = false;
	voice.use_output_gains = sd_use_voice_output_gains_;
	voice.handle = sd_mixer_->play_sound(play_sound_param);

	if (!voice.handle.is_valid())
	{
		return;
	}

	voice_group.add_voice(voice);

	if (sd_use_voice_output_gains_)
	{
		voice.output_gains.fill(bstone::audio_mixer_max_gain);
		sd_mixer_->enable_set_voice_output_gains(voice.handle, true);
		voice_group.set_voice_output_gains(voice);
	}
	else
	{
		voice.gain = bstone::audio_mixer_max_gain;
		voice_group.set_voice_gain(voice);
	}
}

void sd_play_positional_sfx_sound(
	int sound_index,
	bstone::Voice& voice,
	bstone::VoiceGroup& voice_group,
	const bstone::AudioMixerVoiceR3Position& r3_position)
{
	voice_group.stop_voice(voice);
	const auto& audio_chunk = audio_content_mgr->get_sfx_chunk(sound_index);

	if (!audio_chunk.data || audio_chunk.data_size == 0)
	{
		return;
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
			BSTONE_THROW_STATIC_SOURCE("Unknown audio chunk type.");
	}

	auto play_sound_param = bstone::AudioMixerPlaySoundParam{};
	play_sound_param.sound_type = sound_type;
	play_sound_param.sound_index = sound_index;
	play_sound_param.data = audio_chunk.data;
	play_sound_param.data_size = audio_chunk.data_size;
	play_sound_param.is_looping = false;
	play_sound_param.is_r3 = true;
	voice.use_output_gains = sd_use_voice_output_gains_;
	voice.handle = sd_mixer_->play_sound(play_sound_param);
	
	if (!voice.handle.is_valid())
	{
		return;
	}

	voice_group.add_voice(voice);

	if (sd_use_voice_output_gains_)
	{
		sd_mixer_->enable_set_voice_output_gains(voice.handle, true);
		sd_calculate_voice_output_gains(r3_position, voice);
		voice_group.set_voice_output_gains(voice);
	}
	else
	{
		sd_calculate_voice_gain(r3_position, voice);
		voice_group.set_voice_gain(voice);
		sd_mixer_->set_voice_r3_position(voice.handle, r3_position);
	}
}

void sd_update_listener_r3_position()
{
	sd_player_x_ = player->x;
	sd_player_y_ = player->y;

	sd_listener_r3_position_ = bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(
		sd_player_x_,
		sd_player_y_,
		0.5);

	if (!sd_use_voice_output_gains_)
	{
		sd_mixer_->set_listener_r3_position(bstone::AudioMixerListenerR3Position{sd_listener_r3_position_});
	}
}

void sd_update_listener_r3_orientation()
{
	sd_viewcos_ = viewcos;
	sd_viewsin_ = viewsin;

	const auto listener_r3_orientation = bstone::AudioMixerUtils::make_listener_r3_orientation_from_w3d_view(
		sd_viewcos_,
		sd_viewsin_);

	if (!sd_use_voice_output_gains_)
	{
		sd_mixer_->set_listener_r3_orientation(listener_r3_orientation);
	}
}

bstone::StringView sd_get_oal_library() noexcept
{
	return snd_oal_library_cvar.get_string();
}

bstone::StringView sd_get_oal_device_name() noexcept
{
	return snd_oal_device_name_cvar.get_string();
}

void sd_play_ui_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_non_positional_sfx_sound(sound_index, sd_ui_sfx_voice_, *sd_ui_sfx_voice_group_);
}

namespace {

bstone::AudioMixerVoiceR3Position make_r3_position_from_actor(const objtype& actor)
{
	const auto x = actor.x;
	const auto y = actor.y;
	constexpr auto z = 0.5;
	return bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(x, y, z);
}

void sd_play_actor_sound(
	int sound_index,
	objtype& actor,
	bstone::Voice& voice,
	bstone::VoiceGroup& voice_group)
{
	const auto r3_position = make_r3_position_from_actor(actor);
	sd_play_positional_sfx_sound(sound_index, voice, voice_group, r3_position);
}

} // namespace

void sd_play_actor_voice_sound(int sound_index, objtype& actor)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_actor_sound(sound_index, actor, actor.voice_voice, *sd_scene_sfx_voice_group_);
}

void sd_play_actor_weapon_sound(int sound_index, objtype& actor)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_actor_sound(sound_index, actor, actor.weapon_voice, *sd_scene_sfx_voice_group_);
}

void sd_play_actor_walking_sound(int sound_index, objtype& actor)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_actor_sound(sound_index, actor, actor.walking_voice, *sd_scene_sfx_voice_group_);
}

namespace {

void sd_play_player_sound(int sound_index, bstone::Voice& voice)
{
	sd_scene_sfx_voice_group_->stop_voice(voice);
	sd_play_non_positional_sfx_sound(sound_index, voice, *sd_scene_sfx_voice_group_);
}

} // namespace

void sd_play_player_voice_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled() || player == nullptr)
	{
		return;
	}

	sd_play_player_sound(sound_index, player->voice_voice);
}

void sd_play_player_weapon_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled() || player == nullptr)
	{
		return;
	}

	sd_play_player_sound(sound_index, player->weapon_voice);
}

void sd_play_player_item_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_item_voice_);
}

void sd_play_player_hit_wall_sound(int sound_index)
{
	if (sd_mixer_ == nullptr ||
		!sd_is_sound_enabled() ||
		sd_mixer_->is_voice_playing(sd_player_hit_wall_voice_.handle))
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_hit_wall_voice_);
}

void sd_play_player_no_way_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_no_way_voice_);
}

void sd_play_player_interrogation_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_play_player_sound(sound_index, sd_player_interrogation_voice_);
}

namespace {

bstone::AudioMixerVoiceR3Position sd_make_door_r3_position(const doorobj_t& bs_door) noexcept
{
	auto x = bs_door.tilex + 0.5;
	auto y = bs_door.tiley + 0.5;
	constexpr auto z = 0.5;
	return bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(x, y, z);
}

void sd_update_door(doorobj_t& bs_door)
{
	const auto r3_position = sd_make_door_r3_position(bs_door);

	if (sd_use_voice_output_gains_)
	{
		sd_calculate_voice_output_gains(r3_position, bs_door.voice);
		sd_scene_sfx_voice_group_->set_voice_output_gains(bs_door.voice);
	}
	else
	{
		sd_calculate_voice_gain(r3_position, bs_door.voice);
		sd_scene_sfx_voice_group_->set_voice_gain(bs_door.voice);
	}
}

void sd_update_doors()
{
	for (auto& bs_door : doorobjlist)
	{
		sd_update_door(bs_door);
	}
}

}

void sd_play_door_sound(int sound_index, doorobj_t& door)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	const auto r3_position = sd_make_door_r3_position(door);
	sd_play_positional_sfx_sound(sound_index, door.voice, *sd_scene_sfx_voice_group_, r3_position);
}

namespace {

bstone::AudioMixerVoiceR3Position sd_make_pwall_r3_position() noexcept
{
	auto x = pwallx + 0.5;
	auto y = pwally + 0.5;
	constexpr auto z = 0.5;

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
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	const auto r3_position = sd_make_pwall_r3_position();
	sd_mixer_->set_voice_r3_position(sd_pwall_voice_.handle, r3_position);

	if (sd_use_voice_output_gains_)
	{
		sd_calculate_voice_output_gains(r3_position, sd_pwall_voice_);
		sd_scene_sfx_voice_group_->set_voice_output_gains(sd_pwall_voice_);
	}
	else
	{
		sd_calculate_voice_gain(r3_position, sd_pwall_voice_);
		sd_scene_sfx_voice_group_->set_voice_gain(sd_pwall_voice_);
	}
}

} // namespace

void sd_play_pwall_sound(int sound_index)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	const auto r3_position = sd_make_pwall_r3_position();
	sd_play_positional_sfx_sound(sound_index, sd_pwall_voice_, *sd_scene_sfx_voice_group_, r3_position);
}

namespace {

void sd_update_actors()
{
	constexpr auto z = 0.5;

	for (auto actor = player->next; actor; actor = actor->next)
	{
		const auto r3_position = bstone::AudioMixerUtils::make_r3_position_from_w3d_coords(actor->x, actor->y, z);
		sd_mixer_->set_voice_r3_position(actor->voice_voice.handle, r3_position);
		sd_mixer_->set_voice_r3_position(actor->weapon_voice.handle, r3_position);
		sd_mixer_->set_voice_r3_position(actor->walking_voice.handle, r3_position);

		if (sd_use_voice_output_gains_)
		{
			sd_calculate_voice_output_gains(r3_position, actor->voice_voice);
			actor->weapon_voice.output_gains = actor->voice_voice.output_gains;
			actor->walking_voice.output_gains = actor->voice_voice.output_gains;
			sd_scene_sfx_voice_group_->set_voice_output_gains(actor->voice_voice);
			sd_scene_sfx_voice_group_->set_voice_output_gains(actor->weapon_voice);
			sd_scene_sfx_voice_group_->set_voice_output_gains(actor->walking_voice);
		}
		else
		{
			sd_calculate_voice_gain(r3_position, actor->voice_voice);
			actor->weapon_voice.output_gains = actor->voice_voice.output_gains;
			actor->walking_voice.output_gains = actor->voice_voice.output_gains;
			sd_scene_sfx_voice_group_->set_voice_gain(actor->voice_voice);
			sd_scene_sfx_voice_group_->set_voice_gain(actor->weapon_voice);
			sd_scene_sfx_voice_group_->set_voice_gain(actor->walking_voice);
		}
	}
}

} // namespace

void sd_update()
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	const auto mixer_state = bstone::make_scope_exit(
		[]()
		{
			sd_mixer_->resume_state();
		});

	sd_mixer_->suspend_state();
	sd_update_listener_r3_position();
	sd_update_listener_r3_orientation();
	sd_update_pwall();
	sd_update_doors();
	sd_update_actors();
}

namespace {

bool sd_is_player_sound_playing(bstone::Voice& voice)
{
	return sd_mixer_->is_voice_playing(voice.handle);
}

} // namespace

bool sd_is_player_hit_wall_sound_playing()
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return false;
	}

	return sd_is_player_sound_playing(sd_player_hit_wall_voice_);
}

bool sd_is_player_no_way_sound_playing()
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return false;
	}

	return sd_is_player_sound_playing(sd_player_no_way_voice_);
}

int sd_get_sfx_volume() noexcept
{
	return snd_sfx_volume_cvar.get_int32();
}

void sd_set_sfx_volume(int volume) noexcept
{
	snd_sfx_volume_cvar.set_int32(volume);
}

void sd_set_sfx_volume()
{
	if (sd_mixer_ == nullptr)
	{
		return;
	}

	const auto volume = (sd_is_sound_enabled() ? sd_get_sfx_volume() : sd_min_volume);
	const auto clamped_volume = bstone::clamp(volume, sd_min_volume, sd_max_volume);
	const auto gain = static_cast<double>(clamped_volume) / static_cast<double>(sd_max_volume);
	sd_ui_sfx_voice_group_->set_gain(gain);
	sd_scene_sfx_voice_group_->set_gain(gain);
}

int sd_get_music_volume() noexcept
{
	return snd_music_volume_cvar.get_int32();
}

void sd_set_music_volume(int volume) noexcept
{
	snd_music_volume_cvar.set_int32(volume);
}

void sd_set_music_volume()
{
	if (sd_mixer_ == nullptr)
	{
		return;
	}

	const auto volume = (sd_is_music_enabled() ? sd_get_music_volume() : sd_min_volume);
	const auto clamped_volume = bstone::clamp(volume, sd_min_volume, sd_max_volume);
	const auto gain = static_cast<double>(clamped_volume) / static_cast<double>(sd_max_volume);
	sd_music_voice_group_->set_gain(gain);
}

void sd_mute(bool mute)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	sd_mixer_->set_mute(mute);
}

void sd_pause_scene_sfx(bool is_pause)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	if (is_pause)
	{
		sd_scene_sfx_voice_group_->pause();
	}
	else
	{
		sd_scene_sfx_voice_group_->resume();
	}
}

void sd_pause_music(bool is_pause)
{
	if (sd_mixer_ == nullptr || !sd_is_sound_enabled())
	{
		return;
	}

	if (is_pause)
	{
		sd_music_voice_group_->pause();
	}
	else
	{
		sd_music_voice_group_->resume();
	}
}

void apply_digitized_sfx()
{
	if (audio_content_mgr == nullptr)
	{
		return;
	}

	audio_content_mgr->set_is_sfx_digitized(snd_is_sfx_digitized_cvar.get_bool());
}

AudioDriverType sd_get_audio_driver_type() noexcept
{
	return sd_get_driver_type_from_cvar();
}

void sd_set_audio_driver_type(AudioDriverType audio_driver_type)
{
	switch (audio_driver_type)
	{
		case AudioDriverType::system:
			snd_driver_cvar.set_string(snd_driver_cvar_system);
			break;

		case AudioDriverType::openal:
			snd_driver_cvar.set_string(snd_driver_cvar_openal);
			break;

		default:
			snd_driver_cvar.set_string(snd_auto_detect_string);
			break;
	}
}

AudioSfxType sd_cfg_get_sfx_type() noexcept
{
	return sd_get_sfx_type_from_cvar();
}

void sd_cfg_set_sfx_type(AudioSfxType sfx_type)
{
	switch (sfx_type)
	{
		case AudioSfxType::pc_speaker:
			snd_sfx_type_cvar.set_string(snd_sfx_type_cvar_pc_speaker);
			break;

		default:
			snd_sfx_type_cvar.set_string(snd_sfx_type_cvar_adlib);
			break;
	}
}

void sd_apply_sfx_type()
{
	if (!audio_content_mgr)
	{
		return;
	}

	audio_content_mgr->set_sfx_type(sd_get_sfx_type_from_cvar());
}

bool sd_cfg_get_is_sfx_digitized() noexcept
{
	return snd_is_sfx_digitized_cvar.get_bool();
}

void sd_cfg_set_is_sfx_digitized(bool is_sfx_digitized)
{
	snd_is_sfx_digitized_cvar.set_bool(is_sfx_digitized);
}

bstone::Opl3Type sd_get_opl3_type() noexcept
{
	return sd_get_opl3_type_from_cvar();
}

void sd_set_opl3_type(bstone::Opl3Type opl3_type)
{
	switch (opl3_type)
	{
		case bstone::Opl3Type::dbopl:
			snd_opl3_type_cvar.set_string(snd_opl3_type_cvar_dbopl);
			break;

		case bstone::Opl3Type::nuked:
			snd_opl3_type_cvar.set_string(snd_opl3_type_cvar_nuked);
			break;

		default:
			sd_log_error("Invalid OPL3 type.");
			snd_opl3_type_cvar.set_string(snd_opl3_type_cvar_dbopl);
			break;
	}
}
