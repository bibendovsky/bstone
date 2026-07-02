/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
 * Glossary:
 *   - r3s - 3D stereo
 */

#include "bstone_oal_audio_mixer.h"
#include "3d_def.h"
#include "audio.h"
#include "id_sd.h"
#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"
#include "bstone_scope_exit.h"
#include <cfloat>
#include <array>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace bstone {

namespace {

class OalAudioMixer final : public AudioMixer
{
public:
	OalAudioMixer(const AudioMixerInitParam& param);
	~OalAudioMixer() override;

	OplEmulatorType get_opl_emulator_type() const override;
	int get_rate() const override;
	int get_channel_count() const override;
	int get_mix_size_ms() const override;

	void suspend_state() override;
	void resume_state() override;

	void set_mute(bool value) override;
	void set_gain(double gain) override;

	void set_listener_r3_position(const AudioMixerListenerR3Position& r3_position) override;
	void set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation) override;

	AudioMixerVoiceHandle play_sound(const AudioMixerPlaySoundParam& param) override;

	bool is_voice_playing(AudioMixerVoiceHandle voice_handle) const override;

	void pause_voice(AudioMixerVoiceHandle voice_handle) override;
	void resume_voice(AudioMixerVoiceHandle voice_handle) override;
	void stop_voice(AudioMixerVoiceHandle voice_handle) override;

	void set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain) override;
	void set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position) override;

	bool can_set_voice_output_gains() const override;
	void enable_set_voice_output_gains(AudioMixerVoiceHandle voice_handle, bool is_enable) override;
	void set_voice_output_gains(AudioMixerVoiceHandle voice_handle, AudioMixerOutputGains& output_gains) override;

private:
	using Mutex = std::mutex;
	using MutexUniqueLock = std::unique_lock<Mutex>;

	using Strings = std::vector<std::string>;

	using SfxOplSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcSpeakerSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcmSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;

	struct Voice
	{
		int index{};
		bool is_active{};
		bool is_r3{};
		bool is_looping{};
		bool is_music{};
		bool is_r3s{};
		bool r3s_is_paused{};
		int r3s_frame_offset{};
		float r3s_gain{};
		OalSourceCachingSound* r3s_sound{};

		AudioMixerVoiceR3Position r3_position{};

		OalSource oal_source{};
		AudioMixerVoiceHandle handle{};
	};

	using Voices = std::vector<Voice>;
	using VoiceMutex = std::mutex;

	enum class CommandType
	{
		none,

		play_music,
		play_sfx,

		set_mute,
		set_gain,

		set_listener_r3_position,
		set_listener_r3_orientation,

		pause_voice,
		resume_voice,
		stop_voice,

		set_voice_gain,
		set_voice_r3_position,
	};

	struct PlayMusicCommandParam
	{
		const void* data{};
		int data_size{};
		bool is_looping{};
		AudioMixerVoiceHandle voice_handle{};
	};

	struct PlaySfxCommandParam
	{
		SoundType sound_type{};
		bool is_r3{};
		int sound_index{};
		const void* data{};
		int data_size{};
		AudioMixerVoiceHandle voice_handle{};
	};

	struct SetMuteCommandParam
	{
		bool is_mute{};
	};

	struct SetGainCommandParam
	{
		double gain{};
	};

	struct SetListenerR3PositionCommandParam
	{
		AudioMixerListenerR3Position r3_position{};
	};

	struct SetListenerR3OrientationCommandParam
	{
		AudioMixerListenerR3Orientation r3_orientation{};
	};

	struct PauseVoiceCommandParam
	{
		AudioMixerVoiceHandle handle{};
	};

	struct ResumeVoiceCommandParam
	{
		AudioMixerVoiceHandle handle{};
	};

	struct StopVoiceCommandParam
	{
		AudioMixerVoiceHandle handle{};
	};

	struct SetVoiceGainCommandParam
	{
		AudioMixerVoiceHandle handle{};
		double gain{};
	};

	struct SetVoiceR3PositionCommandParam
	{
		AudioMixerVoiceHandle handle{};
		AudioMixerVoiceR3Position position{};
	};

	union CommandParam
	{
		PlayMusicCommandParam play_music;
		PlaySfxCommandParam play_sfx;

		SetMuteCommandParam set_mute;
		SetGainCommandParam set_gain;

		SetListenerR3PositionCommandParam set_listener_r3_position;
		SetListenerR3OrientationCommandParam set_listener_r3_orientation;

		PauseVoiceCommandParam pause_voice;
		ResumeVoiceCommandParam resume_voice;
		StopVoiceCommandParam stop_voice;

		SetVoiceGainCommandParam set_voice_gain;
		SetVoiceR3PositionCommandParam set_voice_r3_position;
	};

	struct Command
	{
		CommandType type{};
		CommandParam param{};
	};

	using Commands = std::vector<Command>;
	using CommandQueueMutex = Mutex;

	using SamplesF32 = std::vector<float>;
	using SamplesS16 = std::vector<std::int16_t>;

	inline static constexpr int commands_min_capacity = 1024;

	inline static constexpr int min_mix_size_ms = 20;
	inline static constexpr int max_mix_size_ms = 40;
	static_assert(max_mix_size_ms > min_mix_size_ms, "Mix size out of range.");

	inline static constexpr int sfx_voices_limit = 255;
	inline static constexpr int music_voices_limit = 1;
	inline static constexpr int voices_limit = sfx_voices_limit + music_voices_limit;

	inline static constexpr float opl_sfx_gain_scale = 7.0F;
	inline static constexpr float opl_music_gain_scale = 6.0F;

	inline static constexpr const char* alc_enumeration_ext_str = "ALC_ENUMERATION_EXT";
	inline static constexpr const char* alc_enumerate_all_ext_str = "ALC_ENUMERATE_ALL_EXT";
	inline static constexpr const ALchar* al_ext_float32_name = "AL_EXT_float32";

	using Thread = std::thread;
	using VoiceHandleMgr = AudioMixerVoiceHandleMgr<Voice>;

	OplEmulatorType opl_emulator_type_{};
	int dst_rate_{};
	int mix_sample_count_{};
	int mix_size_ms_{};
	bool is_mute_{};
	double gain_{};
	AudioMixerListenerR3Position listener_r3_position_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_{};

	bool has_alc_enumeration_ext_{};
	bool has_alc_enumerate_all_ext_{};
	bool has_al_ext_float32_{};
	int sample_size_{};
	SamplesF32 samples_f32_{};
	SamplesS16 samples_s16_{};

	OalLoaderUPtr oal_loader_{};
	OalAlSymbols al_symbols_{};
	OalDeviceResource oal_device_resource_{};
	OalContextResource oal_context_resource_{};

	VoiceHandleMgr voice_handle_mgr_{};

	CommandQueueMutex commands_mutex_{};
	Commands commands_{};
	Commands mt_commands_{};

	OalSourceUncachingSound r2s_sound_{};
	OalSource r2s_oal_source_{};
	SamplesF32 r2s_samples_f32_mix_{};

	SfxOplSounds sfx_opl_sounds_{};
	SfxPcSpeakerSounds sfx_pc_speaker_sounds_{};
	SfxPcmSounds sfx_pcm_sounds_{};

	Voices voices_{};

	bool is_quit_thread_{};
	Mutex thread_mutex_{};
	Thread thread_{};

	std::atomic_bool is_state_suspended_{};

	[[noreturn]] static void fail_unsupported();

	int get_min_rate() const;
	int get_min_mix_size_ms() const;
	int get_default_mix_size_ms() const;
	int get_max_channels() const;

	void make_al_context_current();

	Strings get_alc_device_names();
	std::string get_default_alc_device_name();
	std::string get_alc_device_name();

	Strings parse_al_token_string(const char* al_token_string);
	Strings get_alc_extensions();
	Strings get_al_extensions();

	int get_al_mixing_frequency();
	int get_max_voice_count();

	void detect_alc_extensions();
	void detect_al_extension_al_ext_float32();
	void detect_al_extensions();
	
	void log(const std::string& string);
	void log_oal_library_file_name();
	void log_oal_custom_device();
	void log_oal_devices();
	void log_oal_default_device();
	void log_oal_current_device_name();
	void log_oal_alc_extensions();
	void log_oal_al_info();
	void log_oal_al_extensions();

	static const char* get_oal_default_library_file_name();

	void initialize_oal(const AudioMixerInitParam& param);
	void initialize_distance_model();
	void initialize_is_mute();
	void initialize_gain();
	void initialize_listener_r3_position();
	void initialize_listener_r3_orientation();
	void initialize_voice_handles();
	void initialize_command_queue();
	void initialize_voices();
	void initialize_r2s_sound();
	void initialize_r2s_oal_source();
	void initialize_r2s();
	void uninitialize_r2s();
	void initialize_sfx_opl_sounds();
	void initialize_sfx_pc_speaker_sounds();
	void initialize_sfx_pcm_sounds();
	void initialize_sfx();
	void uninitialize_sfx();
	void initialize_misc();

	void on_music_stop(Voice& voice);
	void on_sfx_stop(const Voice& voice);

	AudioMixerVoiceHandle play_opl_music_internal(const void* data, int data_size, bool is_looping);
	AudioMixerVoiceHandle play_sfx_sound_internal(SoundType sound_type, int sound_index, const void* data, int data_size, bool is_positional);

	void update_al_gain();

	void handle_play_music_command(const PlayMusicCommandParam& param);
	void handle_play_sfx_command(const PlaySfxCommandParam& param);
	void handle_set_mute_command(const SetMuteCommandParam& param);
	void handle_set_gain_command(const SetGainCommandParam& param);
	void handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param);
	void handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param);
	void handle_set_voice_pause_command(AudioMixerVoiceHandle voice_handle, bool is_pause);
	void handle_pause_voice_command(const PauseVoiceCommandParam& param);
	void handle_resume_voice_command(const ResumeVoiceCommandParam& param);
	void handle_stop_voice_command(const StopVoiceCommandParam& param);
	void handle_set_voice_gain_command(const SetVoiceGainCommandParam& param);
	void handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param);
	void handle_commands();

	void decode_opl_sound(OalSourceCachingSound& opl_sound, float gain_scale);
	void decode_pc_speaker_sound(OalSourceCachingSound& pc_speaker_sound);
	void decode_pcm_sound(OalSourceCachingSound& pcm_sound);

	void mix_mono_f32_to_stereo_f32(float gain, const float* src_samples, int frame_count, float* dst_samples);
	void mix_mono_f32_to_stereo_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples);
	void mix_stereo_f32_to_stereo_f32(float gain, const float* src_samples, int frame_count, float* dst_samples);
	void mix_stereo_f32_to_stereo_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples);
	void mix_stereo_f32_to_mono_f32(float gain, const float* src_samples, int frame_count, float* dst_samples);
	void mix_stereo_f32_to_mono_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples);

	void mix_sfx_voice(Voice& voice);
	void mix_r2s_music(Voice& voice);
	void mix_r2s_sfx(Voice& voice);
	void r2s_update_oal_source();
	void mix_r2s();

	void initialize_thread();
	void thread_func();

	Voice* find_free_voice();
	Voice* find_music_voice();

	void set_al_listener_r3_position(double x, double y, double z);
	void set_listener_r3_position();

	void set_al_listener_orientation(double at_x, double at_y, double at_z, double up_x, double up_y, double up_z);
	void set_listener_r3_orientation();

	static float scale_sample(float sample, float scalar);
};

// -------------------------------------

OalAudioMixer::OalAudioMixer(const AudioMixerInitParam& param)
try
{
	switch (param.opl_emulator_type)
	{
		case OplEmulatorType::dbopl:
		case OplEmulatorType::nuked_opl3:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown OPL emulator.");
	}
	initialize_oal(param);
	opl_emulator_type_ = param.opl_emulator_type;
	if (param.mix_size_ms < get_min_mix_size_ms())
		mix_size_ms_ = get_min_mix_size_ms();
	else if (param.mix_size_ms < get_default_mix_size_ms())
		mix_size_ms_ = get_default_mix_size_ms();
	else
		mix_size_ms_ = param.mix_size_ms;
	mix_sample_count_ = static_cast<int>((static_cast<long long>(dst_rate_) * mix_size_ms_) / 1000);
	initialize_distance_model();
	initialize_is_mute();
	initialize_gain();
	update_al_gain();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices();
	initialize_command_queue();
	initialize_r2s();
	initialize_sfx();
	initialize_misc();
	initialize_thread();
	is_mute_ = false;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

OalAudioMixer::~OalAudioMixer()
{
	{
		const MutexUniqueLock thread_mutex_guard{thread_mutex_};
		is_quit_thread_ = true;
	}
	if (thread_.joinable())
		thread_.join();
	uninitialize_r2s();
	uninitialize_sfx();
	oal_context_resource_ = nullptr;
	oal_device_resource_ = nullptr;
	oal_loader_ = nullptr;
}

OplEmulatorType OalAudioMixer::get_opl_emulator_type() const
{
	return opl_emulator_type_;
}

int OalAudioMixer::get_rate() const
{
	return dst_rate_;
}

int OalAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int OalAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

void OalAudioMixer::set_mute(bool is_mute)
try
{
	Command command{};
	command.type = CommandType::set_mute;
	auto& command_param = command.param.set_mute;
	command_param.is_mute = is_mute;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int OalAudioMixer::get_min_rate() const
{
	return 11025;
}

int OalAudioMixer::get_min_mix_size_ms() const
{
	return min_mix_size_ms;
}

int OalAudioMixer::get_default_mix_size_ms() const
{
	return max_mix_size_ms;
}

int OalAudioMixer::get_max_channels() const
{
	return 2;
}

void OalAudioMixer::suspend_state()
{
	is_state_suspended_.store(true, std::memory_order_release);
}

void OalAudioMixer::resume_state()
{
	is_state_suspended_.store(false, std::memory_order_release);
}

void OalAudioMixer::set_gain(double gain)
try
{
	AudioMixerValidator::validate_gain(gain);
	Command command{};
	command.type = CommandType::set_gain;
	auto& command_param = command.param.set_gain;
	command_param.gain = gain;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try
{
	Command command{};
	command.type = CommandType::set_listener_r3_position;
	auto& command_param = command.param.set_listener_r3_position;
	command_param.r3_position = r3_position;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try
{
	Command command{};
	command.type = CommandType::set_listener_r3_orientation;
	auto& command_param = command.param.set_listener_r3_orientation;
	command_param.r3_orientation = r3_orientation;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

AudioMixerVoiceHandle OalAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try
{
	bool is_music = false;
	switch (param.sound_type)
	{
		case SoundType::opl_music:
			is_music = true;
			break;
		case SoundType::opl_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown sound type.");
	}
	if (is_music)
	{
		AudioMixerVoiceHandle voice_handle{};
		if (param.data == nullptr || param.data_size <= 0)
			return voice_handle;
		return play_opl_music_internal(param.data, param.data_size, param.is_looping);
	}
	else
	{
		AudioMixerVoiceHandle voice_handle{};
		if (param.sound_index < 0 || param.sound_index >= NUMSOUNDS)
			return voice_handle;
		if (param.data == nullptr || param.data_size <= 0)
			return voice_handle;
		return play_sfx_sound_internal(param.sound_type, param.sound_index, param.data, param.data_size, param.is_r3);
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool OalAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try
{
	return voice_handle_mgr_.is_valid_handle(voice_handle);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::pause_voice;
	auto& command_param = command.param.pause_voice;
	command_param.handle = voice_handle;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::resume_voice;
	auto& command_param = command.param.resume_voice;
	command_param.handle = voice_handle;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::stop_voice;
	auto& command_param = command.param.stop_voice;
	command_param.handle = voice_handle;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try
{
	AudioMixerValidator::validate_gain(gain);
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::set_voice_gain;
	auto& command_param = command.param.set_voice_gain;
	command_param.handle = voice_handle;
	command_param.gain = gain;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::set_voice_r3_position;
	auto& command_param = command.param.set_voice_r3_position;
	command_param.handle = voice_handle;
	command_param.position = r3_position;
	const MutexUniqueLock commands_lock{commands_mutex_};
	commands_.emplace_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool OalAudioMixer::can_set_voice_output_gains() const
{
	return false;
}

void OalAudioMixer::enable_set_voice_output_gains([[maybe_unused]] AudioMixerVoiceHandle voice_handle, [[maybe_unused]] bool is_enable)
try
{
	fail_unsupported();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_output_gains(
	[[maybe_unused]] AudioMixerVoiceHandle voice_handle,
	[[maybe_unused]] AudioMixerOutputGains& output_gains)
try
{
	fail_unsupported();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

[[noreturn]] void OalAudioMixer::fail_unsupported()
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void OalAudioMixer::make_al_context_current()
{
	if (const ALCboolean al_result = al_symbols_.alcMakeContextCurrent(oal_context_resource_.get());
		al_result == ALC_FALSE)
		BSTONE_THROW_STATIC_SOURCE("Failed to make context current.");
}

std::string OalAudioMixer::get_default_alc_device_name()
{
	std::string default_device_name{};
	if (!has_alc_enumeration_ext_ && !has_alc_enumerate_all_ext_)
		return default_device_name;
	const ALCenum alc_enum = (has_alc_enumerate_all_ext_ ? ALC_DEFAULT_ALL_DEVICES_SPECIFIER : ALC_DEFAULT_DEVICE_SPECIFIER);
	const ALCchar* const default_alc_device_name = al_symbols_.alcGetString(nullptr, alc_enum);
	if (default_alc_device_name != nullptr)
		default_device_name = default_alc_device_name;
	return default_device_name;
}

std::string OalAudioMixer::get_alc_device_name()
{
	std::string device_name{};
	const ALCchar* const alc_device_name = al_symbols_.alcGetString(oal_device_resource_.get(), ALC_DEVICE_SPECIFIER);
	if (alc_device_name != nullptr)
		device_name = alc_device_name;
	return device_name;
}

OalAudioMixer::Strings OalAudioMixer::get_alc_device_names()
{
	Strings device_names{};
	if (!has_alc_enumeration_ext_ && !has_alc_enumerate_all_ext_)
		return device_names;
	device_names.reserve(4);
	const ALCenum alc_enum = (has_alc_enumerate_all_ext_ ? ALC_ALL_DEVICES_SPECIFIER : ALC_DEVICE_SPECIFIER);
	if (const ALCchar* alc_device_names = al_symbols_.alcGetString(nullptr, alc_enum);
		alc_device_names != nullptr)
	{
		while (*alc_device_names != '\0')
		{
			device_names.emplace_back(alc_device_names);
			alc_device_names += device_names.back().size() + 1;
		}
	}
	return device_names;
}

OalAudioMixer::Strings OalAudioMixer::parse_al_token_string(const char* al_token_string)
{
	Strings tokens{};
	if (al_token_string != nullptr)
	{
		{
			int token_count = 0;
			for (const char* al_token = al_token_string; *al_token != '\0'; ++al_token)
			{
				if (*al_token == ' ')
					++token_count;
			}
			tokens.reserve(token_count + 1);
		}
		bool is_parsed = false;
		const char* al_token = al_token_string;
		const char* al_token_begin = al_token_string;
		while (!is_parsed)
		{
			switch (*al_token)
			{
				case '\0':
					is_parsed = true;
					break;
				case ' ':
					if (al_token != al_token_begin)
					{
						tokens.emplace_back();
						tokens.back().assign(al_token_begin, al_token - al_token_begin);
					}
					al_token_begin = al_token + 1;
					break;
			}
			++al_token;
		}
	}
	return tokens;
}

OalAudioMixer::Strings OalAudioMixer::get_alc_extensions()
{
	const ALCchar* const alc_extensions = al_symbols_.alcGetString(oal_device_resource_.get(), ALC_EXTENSIONS);
	const Strings extensions = parse_al_token_string(alc_extensions);
	Strings present_extensions{};
	present_extensions.reserve(extensions.size());
	for (const std::string& extension : extensions)
	{
		if (const bool is_present = (al_symbols_.alcIsExtensionPresent(oal_device_resource_.get(), extension.c_str()) != ALC_FALSE);
			is_present)
			present_extensions.emplace_back(extension);
	}
	return present_extensions;
}

OalAudioMixer::Strings OalAudioMixer::get_al_extensions()
{
	const ALCchar* const al_extensions = al_symbols_.alGetString(AL_EXTENSIONS);
	const Strings extensions = parse_al_token_string(al_extensions);
	Strings present_extensions{};
	present_extensions.reserve(extensions.size());
	for (const std::string& extension : extensions)
	{
		if (const bool is_present = (al_symbols_.alIsExtensionPresent(extension.c_str()) != AL_FALSE);
			is_present)
			present_extensions.emplace_back(extension);
	}
	return present_extensions;
}

int OalAudioMixer::get_al_mixing_frequency()
{
	ALCint al_attribute_size = 0;
	al_symbols_.alcGetIntegerv(oal_device_resource_.get(), ALC_ATTRIBUTES_SIZE, 1, &al_attribute_size);
	constexpr int max_attributes = 64;
	constexpr int max_al_attributes_size = (2 * (max_attributes - 1)) + 1;
	if (al_attribute_size <= 0 || al_attribute_size > max_al_attributes_size)
		BSTONE_THROW_STATIC_SOURCE("Attributes size out of range.");
	struct OalAttribute
	{
		ALint name{};
		ALint value{};
	};
	using OalAttributes = std::array<OalAttribute, max_attributes>;
	OalAttributes al_attributes{};
	al_symbols_.alcGetIntegerv(
		oal_device_resource_.get(),
		ALC_ALL_ATTRIBUTES,
		max_al_attributes_size,
		reinterpret_cast<ALCint*>(al_attributes.data()));
	for (const OalAttribute& al_attribute : al_attributes)
	{
		if (al_attribute.name == 0)
			break;
		if (al_attribute.name == ALC_FREQUENCY)
			return al_attribute.value;
	}
	BSTONE_THROW_STATIC_SOURCE("No ALC_FREQUENCY attribute.");
}

int OalAudioMixer::get_max_voice_count()
{
	using OalSourceResources = std::array<OalSourceResource, voices_limit>;
	OalSourceResources oal_source_resources{};
	int voice_count = 0;
	try
	{
		for (auto& oal_source_resource : oal_source_resources)
		{
			oal_source_resource = make_oal_source(al_symbols_);
			++voice_count;
		}
	}
	catch (...)
	{}
	return voice_count;
}

void OalAudioMixer::detect_alc_extensions()
{
	BSTONE_ASSERT(al_symbols_.alcIsExtensionPresent);
	has_alc_enumeration_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumeration_ext_str) != ALC_FALSE);
	has_alc_enumerate_all_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumerate_all_ext_str) != ALC_FALSE);
}

void OalAudioMixer::detect_al_extension_al_ext_float32()
{
	BSTONE_ASSERT(al_symbols_.alIsExtensionPresent != nullptr);
	BSTONE_ASSERT(al_symbols_.alGetEnumValue != nullptr);
	has_al_ext_float32_ = al_symbols_.alIsExtensionPresent(al_ext_float32_name) == AL_TRUE;
}

void OalAudioMixer::detect_al_extensions()
{
	detect_al_extension_al_ext_float32();
}

void OalAudioMixer::log(const std::string& string)
{
	static const auto prefix = std::string{"[SND_OAL] "};
	globals::logger->log_information((prefix + string).c_str());
}

void OalAudioMixer::log_oal_library_file_name()
{
	log(std::string{"Default library: \""} + get_oal_default_library_file_name() + '\"');
	const std::string_view oal_library = sd_get_oal_library();
	const std::string oal_library_string{oal_library.data(), oal_library.size()};
	log("Custom library: \"" + oal_library_string + '\"');
}

void OalAudioMixer::log_oal_custom_device()
{
	std::string message{};
	message += "Custom device: \"";
	const std::string_view device_name = sd_get_oal_device_name();
	message.append(device_name.data(), device_name.size());
	message += '"';
	log(message);
}

void OalAudioMixer::log_oal_devices()
{
	log("Available devices:");
	const Strings device_names = get_alc_device_names();
	for (const std::string& device_name : device_names)
		log("\t\"" + device_name + '\"');
}

void OalAudioMixer::log_oal_default_device()
{
	const std::string default_device_name = get_default_alc_device_name();
	log("Default device: \"" + default_device_name + '\"');
}

void OalAudioMixer::log_oal_current_device_name()
{
	const std::string current_device_name = get_alc_device_name();
	log("Current device: \"" + current_device_name + '\"');
}

void OalAudioMixer::log_oal_alc_extensions()
{
	log("Device extensions:");
	const Strings extensions = get_alc_extensions();
	for (const std::string& extension : extensions)
		log('\t' + extension);
}

void OalAudioMixer::log_oal_al_info()
{
	const ALCchar* const al_version = al_symbols_.alGetString(AL_VERSION);
	const std::string version{al_version ? al_version : ""};
	log("Version: " + version);
	const ALCchar* const al_renderer = al_symbols_.alGetString(AL_RENDERER);
	const std::string renderer{al_renderer ? al_renderer : ""};
	log("Renderer: " + renderer);
	const ALCchar* const al_vendor = al_symbols_.alGetString(AL_VENDOR);
	const std::string vendor{al_vendor ? al_vendor : ""};
	log("Vendor: " + vendor);
}

void OalAudioMixer::log_oal_al_extensions()
{
	log("Context extensions:");
	const Strings extensions = get_al_extensions();
	for (const std::string& extension : extensions)
		log('\t' + extension);
}

const char* OalAudioMixer::get_oal_default_library_file_name()
{
	return
#if _WIN32
		"OpenAL32.dll"
#else
		"libopenal.so"
#endif // _WIN32
	;
}

void OalAudioMixer::initialize_oal(const AudioMixerInitParam& param)
{
	ALCint al_context_attributes[] = {0, 0, 0};
	if (param.dst_rate > 0)
	{
		al_context_attributes[0] = ALC_FREQUENCY;
		al_context_attributes[1] = param.dst_rate;
	}
	log_oal_library_file_name();
	log_oal_custom_device();
	std::string oal_library_string{};
	const std::string_view oal_library = sd_get_oal_library();
	if (oal_library.empty())
		oal_library_string = get_oal_default_library_file_name();
	else
		oal_library_string.append(oal_library.data(), oal_library.size());
	oal_loader_ = make_oal_loader(oal_library_string.c_str());
	oal_loader_->load_alc_symbols(al_symbols_);
	detect_alc_extensions();
	log_oal_devices();
	log_oal_default_device();
	const char* device_name_c_string = nullptr;
	std::string device_name_string{};
	const std::string_view device_name_sv = sd_get_oal_device_name();
	if (!device_name_sv.empty())
	{
		device_name_string.append(device_name_sv.data(), device_name_sv.size());
		device_name_c_string = device_name_string.c_str();
	}
	oal_device_resource_ = make_oal_device(al_symbols_, device_name_c_string);
	log_oal_current_device_name();
	log_oal_alc_extensions();
	oal_context_resource_ = make_oal_context(al_symbols_, *oal_device_resource_, al_context_attributes);
	make_al_context_current();
	oal_loader_->load_al_symbols(al_symbols_);
	detect_al_extensions();
	log_oal_al_info();
	log_oal_al_extensions();
	dst_rate_ = get_al_mixing_frequency();
	sample_size_ = has_al_ext_float32_ ? 4 : 2;
}

void OalAudioMixer::initialize_distance_model()
{
	BSTONE_ASSERT(al_symbols_.alDistanceModel != nullptr);
	al_symbols_.alDistanceModel(AL_NONE);
}

void OalAudioMixer::initialize_is_mute()
{
	is_mute_ = false;
}

void OalAudioMixer::initialize_gain()
{
	gain_ = audio_mixer_max_gain;
}

void OalAudioMixer::initialize_listener_r3_position()
{
	listener_r3_position_ = audio_mixer_make_default_listener_r3_position();
	set_listener_r3_position();
}

void OalAudioMixer::initialize_listener_r3_orientation()
{
	listener_r3_orientation_ = audio_mixer_make_default_listener_r3_orientation();
	set_listener_r3_orientation();
}

void OalAudioMixer::initialize_voice_handles()
{
	voice_handle_mgr_.set_cache_capacity(commands_min_capacity);
	voice_handle_mgr_.set_map_capacity(voices_limit);
}

void OalAudioMixer::initialize_command_queue()
{
	commands_.clear();
	commands_.reserve(commands_min_capacity);
	mt_commands_.clear();
	mt_commands_.reserve(commands_min_capacity);
}

void OalAudioMixer::initialize_voices()
{
	const int max_voice_count = get_max_voice_count();
	if (max_voice_count < 2)
		return;
	const int max_mono_voices = max_voice_count - 1;
	voices_.resize(max_mono_voices);
	const OalSourceInitParam param{
		.mix_sample_rate = dst_rate_,
		.mix_sample_count = mix_sample_count_,
		.oal_al_symbols = &al_symbols_,
		.sample_size = sample_size_};
	for (Voice& voice : voices_)
	{
		voice.index = 0;
		voice.is_active = false;
		voice.is_looping = false;
		voice.is_music = false;
		voice.is_r3s = false;
		voice.oal_source.initialize(param);
	}
}

void OalAudioMixer::initialize_r2s_sound()
{
	r2s_sound_.is_initialized = true;
	r2s_sound_.is_stereo = true;
	r2s_sound_.queue_size = 0;
	r2s_sound_.read_sample_offset = 0;
	r2s_sound_.write_sample_offset = 0;
	r2s_sound_.samples.resize(mix_sample_count_ * oal_source_max_streaming_buffers * sample_size_ * audio_mixer_max_channels);
	r2s_sound_.audio_decoder = make_audio_decoder(AudioDecoderType::opl_music, opl_emulator_type_);
}

void OalAudioMixer::initialize_r2s_oal_source()
{
	const OalSourceInitParam init_param{
		.mix_sample_rate = dst_rate_,
		.mix_sample_count = mix_sample_count_,
		.oal_al_symbols = &al_symbols_,
		.sample_size = sample_size_};
	r2s_oal_source_.initialize(init_param);
	if (!r2s_oal_source_.is_initialized())
		return;
	const OalSourceOpenStreamingParam open_param{
		.is_3d = false,
		.is_looping = false,
		.is_stereo = true,
		.sample_rate = dst_rate_,
		.sample_size = sample_size_,
		.al_format = has_al_ext_float32_ ? AL_FORMAT_STEREO_FLOAT32 : AL_FORMAT_STEREO16,
		.caching_sound = nullptr,
		.uncaching_sound = &r2s_sound_,
	};
	r2s_oal_source_.open(open_param);
	if (r2s_oal_source_.is_open())
		r2s_oal_source_.play();
	r2s_samples_f32_mix_.resize(mix_sample_count_ * 2);
}

void OalAudioMixer::initialize_r2s()
{
	initialize_r2s_sound();
	initialize_r2s_oal_source();
}

void OalAudioMixer::uninitialize_r2s()
{
	r2s_sound_.is_initialized = false;
	r2s_sound_.audio_decoder = nullptr;
	r2s_oal_source_.uninitialize();
}

void OalAudioMixer::initialize_sfx_opl_sounds()
{
	for (OalSourceCachingSound& sfx_opl_sound : sfx_opl_sounds_)
	{
		sfx_opl_sound.is_initialized = false;
		sfx_opl_sound.audio_decoder = make_audio_decoder(AudioDecoderType::opl_sfx, opl_emulator_type_);
		if (sfx_opl_sound.audio_decoder == nullptr)
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX OPL audio decoder.");
	}
}

void OalAudioMixer::initialize_sfx_pc_speaker_sounds()
{
	for (OalSourceCachingSound& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
	{
		sfx_pc_speaker_sound.is_initialized = false;
		sfx_pc_speaker_sound.audio_decoder = make_audio_decoder(AudioDecoderType::pc_speaker, opl_emulator_type_);
		if (sfx_pc_speaker_sound.audio_decoder == nullptr)
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX PC Speaker audio decoder.");
	}
}

void OalAudioMixer::initialize_sfx_pcm_sounds()
{
	for (OalSourceCachingSound& sfx_pcm_sound : sfx_pcm_sounds_)
	{
		sfx_pcm_sound.is_initialized = false;
		sfx_pcm_sound.audio_decoder = make_audio_decoder(AudioDecoderType::pcm, opl_emulator_type_);
		if (sfx_pcm_sound.audio_decoder == nullptr)
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX PCM audio decoder.");
	}
}

void OalAudioMixer::initialize_sfx()
{
	initialize_sfx_opl_sounds();
	initialize_sfx_pc_speaker_sounds();
	initialize_sfx_pcm_sounds();
}

void OalAudioMixer::uninitialize_sfx()
{
	for (OalSourceCachingSound& sfx_opl_sound : sfx_opl_sounds_)
		sfx_opl_sound.audio_decoder = nullptr;
	for (OalSourceCachingSound& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
		sfx_pc_speaker_sound.audio_decoder = nullptr;
	voices_.clear();
}

void OalAudioMixer::initialize_misc()
{
	samples_f32_.resize(dst_rate_ * oal_source_max_streaming_buffers * audio_mixer_max_channels);
	samples_s16_.resize(dst_rate_ * oal_source_max_streaming_buffers * audio_mixer_max_channels);
}

void OalAudioMixer::on_music_stop(Voice& voice)
{
	voice_handle_mgr_.unmap(voice.handle);
}

void OalAudioMixer::on_sfx_stop(const Voice& voice)
{
	voice_handle_mgr_.unmap(voice.handle);
}

AudioMixerVoiceHandle OalAudioMixer::play_opl_music_internal(const void* data, int data_size, bool is_looping)
{
	const AudioMixerVoiceHandle voice_handle = voice_handle_mgr_.generate();
	Command command{};
	command.type = CommandType::play_music;
	auto& command_param = command.param.play_music;
	command_param.data = data;
	command_param.data_size = data_size;
	command_param.is_looping = is_looping;
	command_param.voice_handle = voice_handle;
	const MutexUniqueLock command_mutex_guard{commands_mutex_};
	commands_.emplace_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}

AudioMixerVoiceHandle OalAudioMixer::play_sfx_sound_internal(SoundType sound_type, int sound_index, const void* data, int data_size, bool is_r3)
{
	const AudioMixerVoiceHandle voice_handle = voice_handle_mgr_.generate();
	Command command{};
	command.type = CommandType::play_sfx;
	auto& command_param = command.param.play_sfx;
	command_param.sound_type = sound_type;
	command_param.is_r3 = is_r3;
	command_param.sound_index = sound_index;
	command_param.data = data;
	command_param.data_size = data_size;
	command_param.voice_handle = voice_handle;
	const MutexUniqueLock command_mutex_guard{commands_mutex_};
	commands_.emplace_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}

void OalAudioMixer::update_al_gain()
{
	const auto al_gain = (is_mute_ ? 0.0F : static_cast<ALfloat>(gain_));
	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerf(AL_GAIN, al_gain);
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::handle_play_music_command(const PlayMusicCommandParam& param)
{
	bool is_started = false;
	Voice* voice = nullptr;
	const auto voice_handle_guard = make_scope_exit(
		[this, &param, &is_started, &voice]()
		{
			if (is_started && voice != nullptr)
			{
				voice->handle = param.voice_handle;
				voice_handle_mgr_.uncache_and_map(param.voice_handle, voice);
			}
			else
				voice_handle_mgr_.uncache(param.voice_handle);
		});
	voice = find_music_voice();
	if (voice != nullptr)
	{
		voice->is_active = false;
		voice->oal_source.close();
	}
	else
		voice = find_free_voice();
	if (voice == nullptr)
		return;
	const AudioDecoderInitParam audio_decoder_param{
		.src_raw_data = param.data,
		.src_raw_size = param.data_size,
		.dst_rate = dst_rate_};
	if (!r2s_sound_.audio_decoder->initialize(audio_decoder_param))
		return;
	OalSourceOpenStreamingParam source_param{};
	source_param.is_3d = false;
	source_param.is_looping = param.is_looping;
	source_param.is_stereo = true;
	source_param.sample_rate = dst_rate_;
	source_param.sample_size = sample_size_;
	source_param.al_format = has_al_ext_float32_ ? AL_FORMAT_STEREO_FLOAT32 : AL_FORMAT_STEREO16;
	source_param.uncaching_sound = &r2s_sound_;
	voice->is_active = true;
	voice->is_r3 = false;
	voice->is_looping = param.is_looping;
	voice->is_music = true;
	voice->is_r3s = true;
	voice->r3s_is_paused = false;
	voice->r3s_frame_offset = 0;
	voice->r3s_gain = 1.0F;
	voice->r3s_sound = nullptr;
	is_started = true;
}

void OalAudioMixer::handle_play_sfx_command(const PlaySfxCommandParam& param)
{
	bool is_started = false;
	Voice* voice = nullptr;
	const auto voice_handle_guard = make_scope_exit(
		[this, &param, &is_started, &voice]()
		{
			if (is_started && voice != nullptr)
				voice_handle_mgr_.uncache_and_map(param.voice_handle, voice);
			else
				voice_handle_mgr_.uncache(param.voice_handle);
		});
	OalSourceCachingSound& sfx_sound = (
		param.sound_type == SoundType::opl_sfx ?
			sfx_opl_sounds_[param.sound_index] :
			(
				param.sound_type == SoundType::pc_speaker_sfx ?
					sfx_pc_speaker_sounds_[param.sound_index] :
					sfx_pcm_sounds_[param.sound_index])
			);
	if (!sfx_sound.is_initialized)
	{
		const AudioDecoderInitParam audio_decoder_param{
			.src_raw_data = param.data,
			.src_raw_size = param.data_size,
			.dst_rate = dst_rate_};
		AudioDecoder* const audio_decoder = sfx_sound.audio_decoder.get();
		if (!audio_decoder->initialize(audio_decoder_param))
			return;
		const int sample_count = audio_decoder->get_total_frames();
		if (sample_count <= 0)
			return;
		sfx_sound.is_initialized = true;
		sfx_sound.is_decoded = false;
		sfx_sound.sample_offset = 0;
		sfx_sound.sample_count = sample_count;
		sfx_sound.samples.resize(sample_count * sample_size_);
		sfx_sound.stereo_samples.resize(sample_count * sample_size_ * audio_mixer_max_channels);
	}
	voice = find_free_voice();
	if (voice == nullptr)
		return;
	const bool is_3d = param.is_r3;
	const bool is_stereo = sfx_sound.audio_decoder->get_channel_count() == 2;
	const bool is_r3s = !is_3d && is_stereo;
	if (!is_r3s)
	{
		if (sfx_sound.is_decoded)
		{
			const int decoded_data_size = sfx_sound.sample_count * sample_size_;
			const OalSourceOpenStaticParam source_param{
				.is_3d = is_3d,
				.sample_rate = dst_rate_,
				.sample_size = sample_size_,
				.al_format = has_al_ext_float32_ ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_MONO16,
				.data = sfx_sound.samples.data(),
				.data_size = decoded_data_size};
			voice->oal_source.open(source_param);
		}
		else
		{
			const OalSourceOpenStreamingParam source_param{
				.is_3d = is_3d,
				.is_looping = false,
				.is_stereo = false,
				.sample_rate = dst_rate_,
				.sample_size = sample_size_,
				.al_format = has_al_ext_float32_ ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_MONO16,
				.caching_sound = &sfx_sound};
			voice->oal_source.open(source_param);
		}
	}
	voice->is_r3 = is_3d;
	voice->is_looping = false;
	voice->is_music = false;
	if (is_r3s)
	{
		voice->is_r3s = true;
		voice->r3s_is_paused = false;
		voice->r3s_frame_offset = 0;
		voice->r3s_gain = 1.0F;
		voice->r3s_sound = &sfx_sound;
	}
	else
		voice->is_r3s = false;
	if (voice->is_r3)
	{
		voice->r3_position = audio_mixer_make_default_voice_r3_position();
		voice->oal_source.set_position(voice->r3_position.x, voice->r3_position.y, voice->r3_position.z);
	}
	if (!is_r3s)
		voice->oal_source.play();
	voice->handle = param.voice_handle;
	voice->is_active = true;
	is_started = true;
}

void OalAudioMixer::handle_set_mute_command(const SetMuteCommandParam& param)
{
	if (is_mute_ == param.is_mute)
		return;
	is_mute_ = param.is_mute;
	update_al_gain();
}

void OalAudioMixer::handle_set_gain_command(const SetGainCommandParam& param)
{
	gain_ = param.gain;
	update_al_gain();
}

void OalAudioMixer::handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param)
{
	if (listener_r3_position_ == param.r3_position)
		return;
	listener_r3_position_ = param.r3_position;
	set_listener_r3_position();
}

void OalAudioMixer::handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param)
{
	if (listener_r3_orientation_ == param.r3_orientation)
		return;
	listener_r3_orientation_ = param.r3_orientation;
	set_listener_r3_orientation();
}

void OalAudioMixer::handle_set_voice_pause_command(AudioMixerVoiceHandle voice_handle, bool is_pause)
{
	Voice* const voice = voice_handle_mgr_.get_voice(voice_handle);
	if (voice == nullptr || !voice->is_active)
		return;
	if (voice->is_r3s)
	{
		voice->r3s_is_paused = is_pause;
		return;
	}
	if (is_pause)
		voice->oal_source.pause();
	else
		voice->oal_source.resume();
}

void OalAudioMixer::handle_pause_voice_command(const PauseVoiceCommandParam& param)
{
	handle_set_voice_pause_command(param.handle, true);
}

void OalAudioMixer::handle_resume_voice_command(const ResumeVoiceCommandParam& param)
{
	handle_set_voice_pause_command(param.handle, false);
}

void OalAudioMixer::handle_stop_voice_command(const StopVoiceCommandParam& param)
{
	Voice* const voice = voice_handle_mgr_.get_voice_and_invalidate(param.handle);
	if (voice == nullptr || !voice->is_active)
		return;
	voice->is_active = false;
	if (!voice->is_r3s)
		voice->oal_source.stop();
}

void OalAudioMixer::handle_set_voice_gain_command(const SetVoiceGainCommandParam& param)
{
	Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
	if (voice == nullptr || !voice->is_active)
		return;
	if (voice->is_r3s)
		voice->r3s_gain = static_cast<float>(param.gain);
	else
		voice->oal_source.set_gain(param.gain);
}

void OalAudioMixer::handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param)
{
	Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
	if (voice == nullptr)
		return;
	if (voice->is_r3s)
		return;
	if (voice->r3_position == param.position)
		return;
	voice->r3_position = param.position;
	voice->oal_source.set_position(param.position.x, param.position.y, param.position.z);
}

void OalAudioMixer::handle_commands()
{
	if (is_state_suspended_.load(std::memory_order_acquire))
		return;
	{
		const MutexUniqueLock commands_lock{commands_mutex_};
		if (!commands_.empty())
		{
			mt_commands_.insert(mt_commands_.end(), commands_.cbegin(), commands_.cend());
			commands_.clear();
		}
	}

	for (const Command& command : mt_commands_)
	{
		switch (command.type)
		{
			case CommandType::play_music:
				handle_play_music_command(command.param.play_music);
				break;
			case CommandType::play_sfx:
				handle_play_sfx_command(command.param.play_sfx);
				break;
			case CommandType::set_mute:
				handle_set_mute_command(command.param.set_mute);
				break;
			case CommandType::set_listener_r3_position:
				handle_set_listener_r3_position_command(command.param.set_listener_r3_position);
				break;
			case CommandType::set_listener_r3_orientation:
				handle_set_listener_r3_orientation_command(command.param.set_listener_r3_orientation);
				break;
			case CommandType::pause_voice:
				handle_pause_voice_command(command.param.pause_voice);
				break;
			case CommandType::resume_voice:
				handle_resume_voice_command(command.param.resume_voice);
				break;
			case CommandType::stop_voice:
				handle_stop_voice_command(command.param.stop_voice);
				break;
			case CommandType::set_voice_gain:
				handle_set_voice_gain_command(command.param.set_voice_gain);
				break;
			case CommandType::set_voice_r3_position:
				handle_set_voice_r3_position_command(command.param.set_voice_r3_position);
				break;
			default:
				BSTONE_ASSERT(false && "Unknown command.");
				break;
		}
	}
	mt_commands_.clear();
}

void OalAudioMixer::decode_opl_sound(OalSourceCachingSound& opl_sound, float gain_scale)
{
	if (!opl_sound.is_initialized || opl_sound.is_decoded)
		return;
	const int remain_count = opl_sound.sample_count - opl_sound.sample_offset;
	if (remain_count == 0)
	{
		opl_sound.is_decoded = true;
		return;
	}
	const int sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	const int src_channel_count = opl_sound.audio_decoder->get_channel_count();
	int decoded_count = 0;
	if (has_al_ext_float32_)
	{
		if (src_channel_count == 1)
		{
			const auto dst_samples = reinterpret_cast<float*>(&opl_sound.samples[opl_sound.sample_offset * 4]);
			decoded_count = opl_sound.audio_decoder->decode_frames(dst_samples, sample_count);
			for (int i = 0; i < decoded_count; ++i)
				dst_samples[i] *= gain_scale;
		}
		else if (src_channel_count == 2)
		{
			const auto dst_samples = reinterpret_cast<float*>(&opl_sound.samples[opl_sound.sample_offset * 4]);
			const auto dst_stereo_samples = reinterpret_cast<float*>(&opl_sound.stereo_samples[opl_sound.sample_offset * 4 * 2]);
			decoded_count = opl_sound.audio_decoder->decode_frames(dst_stereo_samples, sample_count);
			for (int i = 0; i < decoded_count; ++i)
			{
				float& left_sample = dst_stereo_samples[i * 2 + 0];
				float& right_sample = dst_stereo_samples[i * 2 + 1];
				left_sample *= gain_scale;
				right_sample *= gain_scale;
				dst_samples[i] = (left_sample + right_sample) * 0.5F;
			}
		}
	}
	else
	{
		if (src_channel_count == 1)
		{
			const auto src_samples = samples_f32_.data();
			decoded_count = opl_sound.audio_decoder->decode_frames(src_samples, sample_count);
			const auto dst_samples = reinterpret_cast<std::int16_t*>(&opl_sound.samples[opl_sound.sample_offset * 2]);
			for (int i = 0; i < decoded_count; ++i)
				dst_samples[i] = AudioSampleConverter::f32_to_s16(src_samples[i] * gain_scale);
		}
		else if (src_channel_count == 2)
		{
			const auto src_samples = samples_f32_.data();
			decoded_count = opl_sound.audio_decoder->decode_frames(src_samples, sample_count);
			const auto dst_samples = reinterpret_cast<std::int16_t*>(&opl_sound.samples[opl_sound.sample_offset * 2]);
			const auto dst_stereo_samples = reinterpret_cast<std::int16_t*>(&opl_sound.stereo_samples[opl_sound.sample_offset * 2 * 2]);
			for (int i = 0; i < decoded_count; ++i)
			{
				const float left_sample = src_samples[i * 2 + 0] * gain_scale;
				const float right_sample = src_samples[i * 2 + 1] * gain_scale;
				const float mono_sample = (left_sample + right_sample) * 0.5F;
				dst_samples[i] = AudioSampleConverter::f32_to_s16(mono_sample);
				dst_stereo_samples[i * 2 + 0] = AudioSampleConverter::f32_to_s16(left_sample);
				dst_stereo_samples[i * 2 + 1] = AudioSampleConverter::f32_to_s16(right_sample);
			}
		}
	}
	opl_sound.sample_offset += decoded_count;
	if (decoded_count <= sample_count)
	{
		opl_sound.is_decoded = true;
		opl_sound.sample_count = opl_sound.sample_offset;
	}
}

void OalAudioMixer::decode_pc_speaker_sound(OalSourceCachingSound& pc_speaker_sound)
{
	if (!pc_speaker_sound.is_initialized || pc_speaker_sound.is_decoded)
		return;
	const int remain_count = pc_speaker_sound.sample_count - pc_speaker_sound.sample_offset;
	if (remain_count == 0)
	{
		pc_speaker_sound.is_decoded = true;
		return;
	}
	const int sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	int decoded_count;
	if (has_al_ext_float32_)
	{
		const auto dst_samples = reinterpret_cast<float*>(&pc_speaker_sound.samples[pc_speaker_sound.sample_offset * 4]);
		decoded_count = pc_speaker_sound.audio_decoder->decode_frames(dst_samples, sample_count);
	}
	else
	{
		float* const src_samples = samples_f32_.data();
		const auto dst_samples = reinterpret_cast<std::int16_t*>(&pc_speaker_sound.samples[pc_speaker_sound.sample_offset * 2]);
		decoded_count = pc_speaker_sound.audio_decoder->decode_frames(src_samples, sample_count);
		for (int i = 0; i < decoded_count; ++i)
			dst_samples[i] = AudioSampleConverter::f32_to_s16(src_samples[i]);
	}
	pc_speaker_sound.sample_offset += decoded_count;
	if (decoded_count <= sample_count)
	{
		pc_speaker_sound.is_decoded = true;
		pc_speaker_sound.sample_count = pc_speaker_sound.sample_offset;
	}
}

void OalAudioMixer::decode_pcm_sound(OalSourceCachingSound& pcm_sound)
{
	if (!pcm_sound.is_initialized || pcm_sound.is_decoded)
		return;
	const int remain_count = pcm_sound.sample_count - pcm_sound.sample_offset;
	if (remain_count == 0)
	{
		pcm_sound.is_decoded = true;
		return;
	}
	const int sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	int decoded_count;
	if (has_al_ext_float32_)
	{
		const auto dst_samples = reinterpret_cast<float*>(&pcm_sound.samples[pcm_sound.sample_offset * 4]);
		decoded_count = pcm_sound.audio_decoder->decode_frames(dst_samples, sample_count);
	}
	else
	{
		float* const src_samples = samples_f32_.data();
		const auto dst_samples = reinterpret_cast<std::int16_t*>(&pcm_sound.samples[pcm_sound.sample_offset * 2]);
		decoded_count = pcm_sound.audio_decoder->decode_frames(src_samples, sample_count);
		for (int i = 0; i < decoded_count; ++i)
			dst_samples[i] = AudioSampleConverter::f32_to_s16(src_samples[i]);
	}
	pcm_sound.sample_offset += decoded_count;
	if (decoded_count <= sample_count)
	{
		pcm_sound.is_decoded = true;
		pcm_sound.sample_count = pcm_sound.sample_offset;
	}
}

void OalAudioMixer::mix_mono_f32_to_stereo_f32(float gain, const float* src_samples, int frame_count, float* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const float dst_sample = scale_sample(src_samples[i], gain);
		dst_samples[i * 2 + 0] = dst_sample;
		dst_samples[i * 2 + 1] = dst_sample;
	}
}

void OalAudioMixer::mix_mono_f32_to_stereo_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const std::int16_t dst_sample = AudioSampleConverter::f32_to_s16(scale_sample(src_samples[i], gain));
		dst_samples[i * 2 + 0] = dst_sample;
		dst_samples[i * 2 + 1] = dst_sample;
	}
}

void OalAudioMixer::mix_stereo_f32_to_stereo_f32(float gain, const float* src_samples, int frame_count, float* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		dst_samples[i * 2 + 0] = scale_sample(src_samples[i * 2 + 0], gain);
		dst_samples[i * 2 + 1] = scale_sample(src_samples[i * 2 + 1], gain);
	}
}

void OalAudioMixer::mix_stereo_f32_to_stereo_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		dst_samples[i * 2 + 0] = AudioSampleConverter::f32_to_s16(scale_sample(src_samples[i * 2 + 0], gain));
		dst_samples[i * 2 + 1] = AudioSampleConverter::f32_to_s16(scale_sample(src_samples[i * 2 + 1], gain));
	}
}

void OalAudioMixer::mix_stereo_f32_to_mono_f32(float gain, const float* src_samples, int frame_count, float* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
		dst_samples[i] = scale_sample((src_samples[i * 2 + 0] + src_samples[i * 2 + 1]) * 0.5F, gain);
}

void OalAudioMixer::mix_stereo_f32_to_mono_s16(float gain, const float* src_samples, int frame_count, std::int16_t* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const float average_sample = (src_samples[i * 2 + 0] + src_samples[i * 2 + 1]) * 0.5F;
		dst_samples[i] = AudioSampleConverter::f32_to_s16(scale_sample(average_sample, gain));
	}
}

void OalAudioMixer::mix_sfx_voice(Voice& voice)
{
	if (!voice.is_active)
		return;
	if (voice.is_r3s)
		return;
	OalSource& oal_source = voice.oal_source;
	oal_source.mix();
	if (oal_source.is_finished())
	{
		voice.is_active = false;
		on_sfx_stop(voice);
	}
	if (!voice.is_active)
	{
		return;
	}
}

void OalAudioMixer::mix_r2s_music(Voice& voice)
{
	if (!voice.is_active)
		return;
	BSTONE_ASSERT(voice.is_music);
	BSTONE_ASSERT(voice.is_r3s);
	BSTONE_ASSERT(voice.r3s_gain >= 0.0F && voice.r3s_gain <= 1.0F);
	if (voice.r3s_is_paused)
		return;
	AudioDecoder* const audio_decoder = r2s_sound_.audio_decoder.get();
	const int channel_count = audio_decoder->get_channel_count();
	const float gain = opl_music_gain_scale * voice.r3s_gain;
	float* const src_samples = samples_f32_.data();
	int frame_offset = 0;
	while (frame_offset < mix_sample_count_)
	{
		const int decoded_count = audio_decoder->decode_frames(src_samples, mix_sample_count_ - frame_offset);
		if (decoded_count == 0)
		{
			if (!voice.is_looping || !audio_decoder->rewind())
			{
				voice.is_active = false;
				break;
			}
			continue;
		}
		float* const dst_samples = &r2s_samples_f32_mix_[frame_offset * 2];
		if (channel_count == 1)
		{
			for (int i = 0; i < decoded_count; ++i)
			{
				const float dst_sample = scale_sample(src_samples[i], gain);
				dst_samples[i * 2 + 0] += dst_sample;
				dst_samples[i * 2 + 1] += dst_sample;
			}
		}
		else if (channel_count == 2)
		{
			for (int i = 0; i < decoded_count; ++i)
			{
				dst_samples[i * 2 + 0] += scale_sample(src_samples[i * 2 + 0], gain);
				dst_samples[i * 2 + 1] += scale_sample(src_samples[i * 2 + 1], gain);
			}
		}
		frame_offset += decoded_count;
	}
	if (!voice.is_active)
		on_music_stop(voice);
}

void OalAudioMixer::mix_r2s_sfx(Voice& voice)
{
	if (!voice.is_active)
		return;
	BSTONE_ASSERT(!voice.is_music);
	BSTONE_ASSERT(voice.is_r3s);
	BSTONE_ASSERT(voice.r3s_gain >= 0.0F && voice.r3s_gain <= 1.0F);
	BSTONE_ASSERT(voice.r3s_sound != nullptr);
	if (voice.r3s_is_paused)
		return;
	const OalSourceCachingSound& sound = *voice.r3s_sound;
	if (!sound.is_initialized)
		return;
	const int frame_count = std::min(sound.sample_offset - voice.r3s_frame_offset, mix_sample_count_);
	if (frame_count <= 0)
	{
		if (sound.is_decoded)
		{
			voice.is_active = false;
			on_sfx_stop(voice);
		}
		return;
	}
	const int sample_count = frame_count * 2;
	if (has_al_ext_float32_)
	{
		const auto src_samples = reinterpret_cast<const float*>(&sound.stereo_samples[voice.r3s_frame_offset * 4 * 2]);
		float* const dst_samples = r2s_samples_f32_mix_.data();
		for (int i = 0; i < sample_count; ++i)
			dst_samples[i] += src_samples[i] * voice.r3s_gain;
	}
	else
	{
		const auto src_samples = reinterpret_cast<const std::int16_t*>(&sound.stereo_samples[voice.r3s_frame_offset * 2 * 2]);
		float* const dst_samples = r2s_samples_f32_mix_.data();
		for (int i = 0; i < sample_count; ++i)
			dst_samples[i] += AudioSampleConverter::s16_to_f32(src_samples[i]) * voice.r3s_gain;
	}
	voice.r3s_frame_offset += frame_count;
}

void OalAudioMixer::r2s_update_oal_source()
{
	if (!r2s_sound_.is_initialized || !r2s_oal_source_.is_open())
		return;
	r2s_oal_source_.mix();
}

void OalAudioMixer::mix_r2s()
{
	const int max_frames = mix_sample_count_ * oal_source_max_streaming_buffers;
	while (r2s_sound_.queue_size < oal_source_max_streaming_buffers)
	{
		// Fill the mixing buffer with silence.
		std::fill(r2s_samples_f32_mix_.begin(), r2s_samples_f32_mix_.end(), 0.0F);
		// Mix in voices.
		for (Voice& voice : voices_)
		{
			if (voice.is_r3s)
			{
				if (voice.is_music)
					mix_r2s_music(voice);
				else
					mix_r2s_sfx(voice);
			}
		}
		// Convert samples.
		const int dst_byte_offset = r2s_sound_.write_sample_offset * sample_size_ * 2;
		if (has_al_ext_float32_)
		{
			const auto dst_samples = reinterpret_cast<float*>(&r2s_sound_.samples[dst_byte_offset]);
			std::copy_n(r2s_samples_f32_mix_.cbegin(), mix_sample_count_ * 2, dst_samples);
		}
		else
		{
			// Normalize the peak.
			float abs_max_amplitude = 1.0F;
			for (const float sample : r2s_samples_f32_mix_)
			{
				const float abs_amplitude = std::abs(sample);
				if (abs_amplitude > abs_max_amplitude)
					abs_max_amplitude = abs_amplitude;
			}
			if (abs_max_amplitude > 1.0F)
			{
				const float scalar = 1.0F / abs_max_amplitude;
				for (float& sample : r2s_samples_f32_mix_)
					sample *= scalar;
			}
			// Convert the samples.
			const float* const src_samples = r2s_samples_f32_mix_.data();
			const auto dst_samples = reinterpret_cast<std::int16_t*>(&r2s_sound_.samples[dst_byte_offset]);
			const int convert_count = mix_sample_count_ * 2;
			for (int i = 0; i < convert_count; ++i)
				dst_samples[i] = AudioSampleConverter::f32_to_s16(src_samples[i]);
		}
		// Advance.
		r2s_sound_.write_sample_offset += mix_sample_count_;
		if (r2s_sound_.write_sample_offset >= max_frames)
			r2s_sound_.write_sample_offset = 0;
		r2s_sound_.queue_size += 1;
	}
	r2s_update_oal_source();
}

void OalAudioMixer::initialize_thread()
{
	is_quit_thread_ = false;
	thread_ = Thread{&OalAudioMixer::thread_func, this};
}

void OalAudioMixer::thread_func()
{
	constexpr std::chrono::milliseconds sleep_delay{1};
	while (true)
	{
		{
			const MutexUniqueLock thread_mutex_guard{thread_mutex_};
			if (is_quit_thread_)
				return;
		}
		for (OalSourceCachingSound& sfx_opl_sound : sfx_opl_sounds_)
			decode_opl_sound(sfx_opl_sound, opl_sfx_gain_scale);
		for (OalSourceCachingSound& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
			decode_pc_speaker_sound(sfx_pc_speaker_sound);
		for (OalSourceCachingSound& sfx_pcm_sound : sfx_pcm_sounds_)
			decode_pcm_sound(sfx_pcm_sound);
		al_symbols_.alcSuspendContext(oal_context_resource_.get());
		handle_commands();
		for (Voice& voice : voices_)
			mix_sfx_voice(voice);
		mix_r2s();
		al_symbols_.alcProcessContext(oal_context_resource_.get());
		std::this_thread::sleep_for(sleep_delay);
	}
}

OalAudioMixer::Voice* OalAudioMixer::find_free_voice()
{
	for (Voice& voice : voices_)
	{
		if (!voice.is_active)
			return &voice;
	}
	return nullptr;
}

OalAudioMixer::Voice* OalAudioMixer::find_music_voice()
{
	for (Voice& voice : voices_)
	{
		if (voice.is_active && voice.is_music)
			return &voice;
	}
	return nullptr;
}

void OalAudioMixer::set_al_listener_r3_position(double x, double y, double z)
{
	BSTONE_ASSERT(al_symbols_.alGetError != nullptr);
	BSTONE_ASSERT(al_symbols_.alListener3f != nullptr);
	al_symbols_.alGetError();
	al_symbols_.alListener3f(AL_POSITION, static_cast<ALfloat>(x), static_cast<ALfloat>(y), static_cast<ALfloat>(z));
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_listener_r3_position()
{
	set_al_listener_r3_position(listener_r3_position_.x, listener_r3_position_.y, listener_r3_position_.z);
}

void OalAudioMixer::set_al_listener_orientation(double at_x, double at_y, double at_z, double up_x, double up_y, double up_z)
{
	const ALfloat al_orientation[] =
	{
		static_cast<ALfloat>(at_x),
		static_cast<ALfloat>(at_y),
		static_cast<ALfloat>(at_z),

		static_cast<ALfloat>(up_x),
		static_cast<ALfloat>(up_y),
		static_cast<ALfloat>(up_z),
	};
	BSTONE_ASSERT(al_symbols_.alGetError != nullptr);
	BSTONE_ASSERT(al_symbols_.alListenerfv != nullptr);
	al_symbols_.alGetError();
	al_symbols_.alListenerfv(AL_ORIENTATION, al_orientation);
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_listener_r3_orientation()
{
	set_al_listener_orientation(
		listener_r3_orientation_.at.x,
		listener_r3_orientation_.at.y,
		listener_r3_orientation_.at.z,
		listener_r3_orientation_.up.x,
		listener_r3_orientation_.up.y,
		listener_r3_orientation_.up.z);
}

float OalAudioMixer::scale_sample(float sample, float scalar)
{
	return std::clamp(sample * scalar, -1.0F, +1.0F);
}

} // namespace

// =====================================

AudioMixerUPtr make_oal_audio_mixer(const AudioMixerInitParam& param)
{
	return std::make_unique<OalAudioMixer>(param);
}

} // namespace bstone
