/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
#include "bstone_math.h"
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


namespace {

class SdException : public bstone::Exception
{
public:
	explicit SdException(const char* message) noexcept
		:
		Exception{"SD", message}
	{}
}; // SdException

[[noreturn]] void sd_fail_nested(const char* message)
{
	std::throw_with_nested(SdException{message});
}

auto sd_music_is_looping_ = true;

bstone::AudioContentMgrUPtr audio_content_mgr{};

auto sd_use_voice_output_gains_ = false;

auto sd_sfx_type = AudioSfxType::adlib;
auto sd_is_sfx_digitized = true;

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

int sd_sfx_volume_ = sd_default_sfx_volume;
int sd_music_volume_ = sd_default_music_volume;

auto sd_oal_library_ = std::string{};
auto sd_oal_device_name_ = std::string{};
bstone::Opl3Type sd_opl3_type_ = bstone::Opl3Type::dbopl;

namespace {

constexpr auto snd_auto_detect_string = bstone::StringView{"auto-detect"};

// snd_driver

constexpr auto snd_driver_cvar_name = bstone::StringView{"snd_driver"};
constexpr auto snd_driver_cvar_2d_sdl = bstone::StringView{"2d_sdl"};
constexpr auto snd_driver_cvar_3d_openal = bstone::StringView{"3d_openal"};

auto snd_driver_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_driver_cvar_name,
	snd_auto_detect_string,
	{
		snd_auto_detect_string,
		snd_driver_cvar_2d_sdl,
		snd_driver_cvar_3d_openal,
	}};

// snd_oal_library

constexpr auto snd_oal_library_cvar_name = bstone::StringView{"snd_oal_library"};

auto snd_oal_library_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_oal_library_cvar_name,
	bstone::StringView{}};

// snd_oal_device_name

constexpr auto snd_oal_device_name_cvar_name = bstone::StringView{"snd_oal_device_name"};

auto snd_oal_device_name_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_oal_device_name_cvar_name,
	bstone::StringView{}};

// snd_opl3_type

constexpr auto snd_opl3_type_cvar_name = bstone::StringView{"snd_opl3_type"};
constexpr auto snd_opl3_type_cvar_dbopl = bstone::StringView{"dbopl"};
constexpr auto snd_opl3_type_cvar_nuked = bstone::StringView{"nuked"};

auto snd_opl3_type_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_opl3_type_cvar_name,
	snd_opl3_type_cvar_dbopl,
	{
		snd_opl3_type_cvar_dbopl,
		snd_opl3_type_cvar_nuked,
	}};

// snd_is_sfx_enabled

constexpr auto snd_is_sfx_enabled_cvar_name = bstone::StringView{"snd_is_sfx_enabled"};
constexpr auto snd_is_sfx_enabled_cvar_default = true;

auto snd_is_sfx_enabled_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_sfx_enabled_cvar_name,
	snd_is_sfx_enabled_cvar_default};

// snd_sfx_type

constexpr auto snd_sfx_type_cvar_name = bstone::StringView{"snd_sfx_type"};
constexpr auto snd_sfx_type_cvar_pc_speaker = bstone::StringView{"pc_speaker"};
constexpr auto snd_sfx_type_cvar_adlib = bstone::StringView{"adlib"};

auto snd_sfx_type_cvar = bstone::CVar{
	bstone::CVarStringTag{},
	snd_sfx_type_cvar_name,
	snd_sfx_type_cvar_adlib,
	{
		snd_sfx_type_cvar_adlib,
		snd_sfx_type_cvar_pc_speaker,
	}};

// snd_is_sfx_digitized

constexpr auto snd_is_sfx_digitized_cvar_name = bstone::StringView{"snd_is_sfx_digitized"};
constexpr auto snd_is_sfx_digitized_cvar_default = true;

auto snd_sfx_digitized_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_sfx_digitized_cvar_name,
	snd_is_sfx_digitized_cvar_default};

// snd_sfx_volume

constexpr auto snd_sfx_volume_cvar_name = bstone::StringView{"snd_sfx_volume"};

auto snd_sfx_volume_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_sfx_volume_cvar_name,
	sd_default_sfx_volume,
	sd_min_volume,
	sd_max_volume};

// snd_is_music_enabled

constexpr auto snd_is_music_enabled_cvar_name = bstone::StringView{"snd_is_music_enabled"};
constexpr auto snd_is_music_enabled_cvar_default = true;

auto snd_music_cvar = bstone::CVar{
	bstone::CVarBoolTag{},
	snd_is_music_enabled_cvar_name,
	snd_is_music_enabled_cvar_default};

// snd_music_volume

constexpr auto snd_music_volume_cvar_name = bstone::StringView{"snd_music_volume"};

auto snd_music_volume_cvar = bstone::CVar{
	bstone::CVarInt32Tag{},
	snd_music_volume_cvar_name,
	sd_default_music_volume,
	sd_min_volume,
	sd_max_volume};

} // namespace

void sd_initialize_cvars(bstone::CVarMgr& cvar_mgr)
{
	cvar_mgr.add(snd_driver_cvar);
	cvar_mgr.add(snd_oal_library_cvar);
	cvar_mgr.add(snd_oal_device_name_cvar);
	cvar_mgr.add(snd_opl3_type_cvar);
	cvar_mgr.add(snd_is_sfx_enabled_cvar);
	cvar_mgr.add(snd_sfx_type_cvar);
	cvar_mgr.add(snd_sfx_digitized_cvar);
	cvar_mgr.add(snd_sfx_volume_cvar);
	cvar_mgr.add(snd_music_cvar);
	cvar_mgr.add(snd_music_volume_cvar);
}

#if FIXMENOW
void sd_cfg_parse_cl();
#endif

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

bool sd_is_sound_enabled() noexcept
{
	return sd_is_sound_enabled_;
}

void sd_set_is_sound_enabled(bool is_enabled) noexcept
{
	sd_is_sound_enabled_ = is_enabled;
}

bool sd_enable_sound(bool enable)
{
	sd_stop_sfx_sound();

	if (enable && !sd_has_audio_)
	{
		enable = false;
	}

	sd_set_is_sound_enabled(enable);
	sd_set_sfx_volume();
	return enable;
}

bool sd_is_music_enabled() noexcept
{
	return sd_is_music_enabled_;
}

void sd_set_is_music_enabled(bool is_enabled) noexcept
{
	sd_is_music_enabled_ = is_enabled;
}

bool sd_enable_music(bool enable)
{
	sd_music_off();
	sd_set_is_music_enabled(enable);
	sd_set_music_volume();
	return enable;
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
try
{
	sd_mixer_ = nullptr;
	sd_music_voice_group_ = nullptr;
	sd_ui_sfx_voice_group_ = nullptr;
	sd_scene_sfx_voice_group_ = nullptr;

	auto param = bstone::AudioMixerInitParam{};
	param.audio_driver_type = audio_driver_type;
	param.opl3_type = sd_opl3_type_;
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
}
catch (...)
{
	sd_fail_nested(__func__);
}

AudioDriverType sd_get_driver_type_from_cvar() noexcept
{
	const auto driver_sv = snd_driver_cvar.get_string();

	if (driver_sv == snd_driver_cvar_2d_sdl)
	{
		return AudioDriverType::r2_sdl;
		return AudioDriverType::r3_openal;
	}

	if (driver_sv == snd_driver_cvar_3d_openal)
	{
		return AudioDriverType::r3_openal;
	}

	return AudioDriverType::auto_detect;
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

#if FIXMENOW
		sd_cfg_parse_cl();
#endif

		auto is_driver_initialized = false;

		const auto driver_type = sd_get_driver_type_from_cvar();

		switch (driver_type)
		{
			case AudioDriverType::r2_sdl:
			case AudioDriverType::r3_openal:
				try
				{
					sd_make_mixer(driver_type, snd_rate, snd_mix_size);
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
			sd_log("Effects volume: " + std::to_string(sd_get_sfx_volume()) + " / " + std::to_string(sd_max_volume));
			sd_log("Music volume: " + std::to_string(sd_get_music_volume()) + " / " + std::to_string(sd_max_volume));
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
		sd_initialize_voices();

		sd_enable_sound(sd_is_sound_enabled());

		sd_enable_music(sd_is_music_enabled_);
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
	const auto x = bstone::math::clamp(std::abs(static_cast<int>(xt_c - yt_s)), 0, ATABLEMAX - 1);

	const auto xt_s = gx * sd_viewsin_;
	const auto yt_c = gy * sd_viewcos_;
	const auto y = bstone::math::clamp(static_cast<int>(yt_c + xt_s), -ATABLEMAX, ATABLEMAX - 1);

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
			fail("Unknown audio chunk type.");
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
			fail("Unknown audio chunk type.");
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

const std::string& sd_get_oal_library() noexcept
{
	return sd_oal_library_;
}

const std::string& sd_get_oal_device_name() noexcept
{
	return sd_oal_device_name_;
}

void sd_play_ui_sound(int sound_index)
{
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

		if (sd_use_voice_output_gains_)
		{
			sd_calculate_voice_output_gains(r3_position, actor->voice_voice);
			actor->weapon_voice.output_gains = actor->voice_voice.output_gains;
			sd_scene_sfx_voice_group_->set_voice_output_gains(actor->voice_voice);
			sd_scene_sfx_voice_group_->set_voice_output_gains(actor->weapon_voice);
		}
		else
		{
			sd_calculate_voice_gain(r3_position, actor->voice_voice);
			actor->weapon_voice.output_gains = actor->voice_voice.output_gains;
			sd_scene_sfx_voice_group_->set_voice_gain(actor->voice_voice);
			sd_scene_sfx_voice_group_->set_voice_gain(actor->weapon_voice);
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


	const auto mixer_state = bstone::ScopeGuard{
		[]()
		{
			sd_mixer_->resume_state();
		}
	};

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
	return sd_sfx_volume_;
}

void sd_set_sfx_volume(int volume) noexcept
{
	sd_sfx_volume_ = volume;
}

void sd_set_sfx_volume()
{
	if (sd_mixer_ == nullptr)
	{
		return;
	}

	const auto volume = (sd_is_sound_enabled() ? sd_get_sfx_volume() : sd_min_volume);
	const auto clamped_volume = bstone::math::clamp(volume, sd_min_volume, sd_max_volume);
	const auto gain = static_cast<double>(clamped_volume) / static_cast<double>(sd_max_volume);
	sd_ui_sfx_voice_group_->set_gain(gain);
	sd_scene_sfx_voice_group_->set_gain(gain);
}

int sd_get_music_volume() noexcept
{
	return sd_music_volume_;
}

void sd_set_music_volume(int volume) noexcept
{
	sd_music_volume_ = volume;
}

void sd_set_music_volume()
{
	if (sd_mixer_ == nullptr)
	{
		return;
	}

	const auto volume = (sd_is_music_enabled() ? sd_get_music_volume() : sd_min_volume);
	const auto clamped_volume = bstone::math::clamp(volume, sd_min_volume, sd_max_volume);
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

	audio_content_mgr->set_is_sfx_digitized(sd_is_sfx_digitized);
}

AudioDriverType sd_get_audio_driver_type() noexcept
{
	return sd_get_driver_type_from_cvar();
}

void sd_set_audio_driver_type(AudioDriverType audio_driver_type)
{
	switch (audio_driver_type)
	{
		case AudioDriverType::r2_sdl:
			snd_driver_cvar.set_string(snd_driver_cvar_2d_sdl);
			break;

		case AudioDriverType::r3_openal:
			snd_driver_cvar.set_string(snd_driver_cvar_3d_openal);
			break;

		default:
			snd_driver_cvar.set_string(snd_auto_detect_string);
			break;
	}
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

#if FIXMENOW
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
		sd_oal_library_ = value_string;
		return true;
	}
	else if (key_string == sd_cfg_get_oal_device_name_name())
	{
		sd_oal_device_name_ = value_string;
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
		sd_oal_library_ = g_args.get_option_value(sd_cfg_get_oal_library_name());
	}

	if (g_args.has_option(sd_cfg_get_oal_device_name_name()))
	{
		sd_oal_device_name_ = g_args.get_option_value(sd_cfg_get_oal_device_name_name());
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
		sd_get_oal_library()
	);

	cfg_file_write_entry(
		text_writer,
		sd_cfg_get_oal_device_name_name(),
		sd_oal_device_name_
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
#endif
