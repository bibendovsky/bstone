/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Audio mixer with system backend

#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_scope_exit.h"
#include "bstone_system_audio_mixer.h"
#include "bstone_sys_audio_mgr.h"
#include <algorithm>
#include <atomic>
#include <deque>
#include <mutex>
#include <vector>

namespace bstone {

namespace {

class SystemAudioMixer final : public AudioMixer
{
public:
	SystemAudioMixer(const AudioMixerInitParam& param);
	~SystemAudioMixer() override = default;

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
	using Sample = float;
	using Samples = std::vector<Sample>;

	using MixSample = float;
	using MixSamples = std::vector<MixSample>;

	using MtLock = std::mutex;
	using MtLockGuard = std::lock_guard<MtLock>;

	struct SysCallback final : public sys::PollingAudioDeviceCallback
	{
	public:
		void set_mixer(SystemAudioMixer* mixer);

		void invoke(float* samples, int sample_count) override;

	private:
		SystemAudioMixer* mixer_{};
	};

	struct CacheItem
	{
		bool is_active{};
		bool is_invalid{};
		SoundType sound_type{};
		int samples_count{};
		int decoded_count{};
		int buffer_size{};
		Samples samples{};
		AudioDecoderUPtr decoder{};

		bool is_decoded() const;
	};

	using Cache = std::deque<CacheItem>;

	struct Voice
	{
		SoundType type{};
		bool is_active{};
		bool is_r3{};
		bool is_looping{};
		bool is_paused{};
		bool is_r3_position_changed{};
		bool is_custom_output_gains{};
		CacheItem* cache{};
		int decode_offset{};
		double gain{};
		AudioMixerOutputGains output_gains{};
		AudioMixerOutputGains custom_output_gains{};
		AudioMixerVoiceHandle handle{};
		AudioMixerVoiceR3Position r3_position{};
		AudioMixerVoiceR3Position r3_position_cache{};
	};

	using Voices = std::vector<Voice>;

	enum class CommandType
	{
		play_sound,

		set_mute,
		set_gain,

		set_listener_r3_position,
		set_listener_r3_orientation,

		pause_voice,
		resume_voice,
		stop_voice,

		set_voice_gain,
		set_voice_r3_position,

		enable_set_voice_output_gains,
		set_voice_output_gains,
	};

	struct PlaySoundCommandParam
	{
		SoundType sound_type{};
		bool is_r3{};
		bool is_looping{};
		CacheItem* cache{};
		AudioMixerVoiceHandle handle{};
		const void* data{};
		int data_size{};
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

	struct EnableSetVoiceOutputGainsCommandParam
	{
		AudioMixerVoiceHandle handle{};
		bool is_enable{};
	};

	struct SetVoiceOutputGainsCommandParam
	{
		AudioMixerVoiceHandle handle{};
		AudioMixerOutputGains output_gains{};
	};

	union CommandParam
	{
		PlaySoundCommandParam play_sound;

		SetMuteCommandParam set_mute;
		SetGainCommandParam set_gain;

		SetListenerR3PositionCommandParam set_listener_r3_position;
		SetListenerR3OrientationCommandParam set_listener_r3_orientation;

		PauseVoiceCommandParam pause_voice;
		ResumeVoiceCommandParam resume_voice;
		StopVoiceCommandParam stop_voice;

		SetVoiceGainCommandParam set_voice_gain;
		SetVoiceR3PositionCommandParam set_voice_r3_position;

		EnableSetVoiceOutputGainsCommandParam enable_set_voice_output_gains;
		SetVoiceOutputGainsCommandParam set_voice_output_gains;
	};

	struct Command
	{
		CommandType type{};
		CommandParam param{};
	};

	using Commands = std::vector<Command>;

	using VoiceHandleMgr = AudioMixerVoiceHandleMgr<Voice>;

	OplEmulatorType opl_emulator_type_{};
	int dst_rate_{};
	SysCallback sys_callback_{};
	sys::AudioMgr* sys_audio_mgr_{};
	int mix_samples_count_{};
	Samples buffer_{};
	MixSamples mix_buffer_{};
	VoiceHandleMgr voice_handle_mgr_{};
	Voices voices_{};
	Commands commands_{};
	Commands mt_commands_{};
	MtLock mt_commands_lock_{};
	Cache opl_music_cache_{};
	Cache opl_sfx_cache_{};
	Cache pc_speaker_sfx_cache_{};
	Cache pcm_cache_{};
	int mix_size_ms_{};
	bool is_mute_{};
	double gain_{};
	AudioMixerListenerR3Position listener_r3_position_{};
	AudioMixerListenerR3Position listener_r3_position_cache_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_cache_{};
	bool is_listener_r3_position_changed_{};
	bool is_listener_r3_orientation_changed_{};
	std::atomic_bool is_state_suspended_{};
	sys::PollingAudioDeviceUPtr sys_audio_device_{};

	int get_min_rate() const;
	int get_default_rate() const;
	int get_min_mix_size_ms() const;
	int get_default_mix_size_ms() const;
	int get_max_channels() const;

	void initialize_is_mute();
	void initialize_gain();
	void initialize_listener_r3_position();
	void initialize_listener_r3_orientation();
	void initialize_voice_handles();
	void initialize_voices(int max_voices);

	void callback(float* samples, int sample_count);

	void mix();
	void mix_samples_mono_to_stereo(
		double gain,
		const AudioMixerOutputGains& output_gains,
		int frame_count,
		const Sample* src_samples,
		Sample* dst_samples);
	void mix_samples_stereo_to_mono(
		double gain,
		const AudioMixerOutputGains& output_gains,
		int frame_count,
		const Sample* src_samples,
		Sample* dst_samples);
	void mix_samples_stereo_to_stereo(
		double gain,
		const AudioMixerOutputGains& output_gains,
		int frame_count,
		const Sample* src_samples,
		Sample* dst_samples);
	void mix_samples();

	void handle_set_mute_command(const SetMuteCommandParam& param);
	void handle_set_gain_command(const SetGainCommandParam& param);

	void handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param);
	void handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param);

	void handle_pause_voice_command(const PauseVoiceCommandParam& param);
	void handle_resume_voice_command(const ResumeVoiceCommandParam& param);
	void handle_stop_voice_command(const StopVoiceCommandParam& param);
	void handle_set_voice_gain_command(const SetVoiceGainCommandParam& param);
	void handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param);
	void handle_enable_set_voice_output_gains_command(const EnableSetVoiceOutputGainsCommandParam& param);
	void handle_set_voice_output_gains_command(const SetVoiceOutputGainsCommandParam& param);
	void handle_commands();

	void handle_play_sound_command(const Command& command);
	bool initialize_cache_item(const Command& command, CacheItem& cache_item);

	bool decode_voice(const Voice& voice);

	void spatialize_voice(Voice& voice);
	void spatialize_voices();

	CacheItem* get_cache_item(SoundType sound_type, int sound_index);

	static int calculate_mix_samples_count(int dst_rate, int mix_size_ms);
	AudioDecoderUPtr create_decoder_by_sound_type(SoundType sound_type) const;
	static bool is_sound_type_valid(SoundType sound_type);
	static bool is_sound_index_valid(int sound_index, SoundType sound_type);
};

// -------------------------------------

void SystemAudioMixer::SysCallback::set_mixer(SystemAudioMixer* mixer)
{
	if (mixer == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null mixer.");
	mixer_ = mixer;
}

void SystemAudioMixer::SysCallback::invoke(float* samples, int sample_count)
{
	if (mixer_ == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null mixer.");
	mixer_->mix();
	mixer_->callback(samples, sample_count);
}

bool SystemAudioMixer::CacheItem::is_decoded() const
{
	return decoded_count == samples_count;
}

SystemAudioMixer::SystemAudioMixer(const AudioMixerInitParam& param)
try {
	if (param.max_voices < 0)
		BSTONE_THROW_STATIC_SOURCE("Max voice count out of range.");
	switch (param.opl_emulator_type)
	{
		case OplEmulatorType::dbopl:
		case OplEmulatorType::nuked_opl3:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown OPL emulator.");
	}
	if (param.dst_rate == 0)
		dst_rate_ = get_default_rate();
	else
		dst_rate_ = std::max(param.dst_rate, get_min_rate());
	if (param.mix_size_ms == 0)
		mix_size_ms_ = get_default_mix_size_ms();
	else
		mix_size_ms_ = std::max(param.mix_size_ms, get_min_mix_size_ms());
	if (bstone::globals::sys_system_mgr == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null system manager.");
	mix_samples_count_ = calculate_mix_samples_count(dst_rate_, mix_size_ms_);
	sys_audio_mgr_ = &bstone::globals::sys_system_mgr->get_audio_mgr();
	if (!sys_audio_mgr_->is_initialized())
		BSTONE_THROW_STATIC_SOURCE("Audio system not available.");
	sys_callback_.set_mixer(this);
	const bstone::sys::PollingAudioDeviceOpenParam audio_device_param{
		.desired_rate = dst_rate_,
		.channel_count = get_max_channels(),
		.desired_frame_count = mix_samples_count_,
		.callback = &sys_callback_};
	sys::PollingAudioDeviceUPtr audio_device = sys_audio_mgr_->make_polling_audio_device(audio_device_param);
	dst_rate_ = audio_device->get_rate();
	mix_samples_count_ = audio_device->get_frame_count();
	opl_emulator_type_ = param.opl_emulator_type;
	const int total_samples = get_max_channels() * mix_samples_count_;
	buffer_.resize(total_samples);
	mix_buffer_.resize(total_samples);
	opl_music_cache_.resize(LASTMUSIC);
	opl_sfx_cache_.resize(NUMSOUNDS);
	pc_speaker_sfx_cache_.resize(NUMSOUNDS);
	pcm_cache_.resize(NUMSOUNDS);
	const int commands_reserve = param.max_voices * 4;
	commands_.reserve(commands_reserve);
	mt_commands_.reserve(commands_reserve);
	initialize_is_mute();
	initialize_gain();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices(param.max_voices);
	audio_device->pause(false);
	sys_audio_device_.swap(audio_device);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

OplEmulatorType SystemAudioMixer::get_opl_emulator_type() const
{
	return opl_emulator_type_;
}

int SystemAudioMixer::get_rate() const
{
	return dst_rate_;
}

int SystemAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int SystemAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

void SystemAudioMixer::set_mute(bool is_mute)
try
{
	Command command{};
	command.type = CommandType::set_mute;
	command.param.set_mute.is_mute = is_mute;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_gain(double gain)
try
{
	AudioMixerValidator::validate_gain(gain);
	Command command{};
	command.type = CommandType::set_gain;
	command.param.set_gain.gain = gain;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int SystemAudioMixer::get_min_rate() const
{
	return 11'025;
}

int SystemAudioMixer::get_default_rate() const
{
	return 44'100;
}

int SystemAudioMixer::get_min_mix_size_ms() const
{
	return 20;
}

int SystemAudioMixer::get_default_mix_size_ms() const
{
	return 40;
}

int SystemAudioMixer::get_max_channels() const
{
	return 2;
}

void SystemAudioMixer::suspend_state()
try
{
	is_state_suspended_.store(true, std::memory_order_release);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::resume_state()
try
{
	is_state_suspended_.store(false, std::memory_order_release);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try
{
	Command command{};
	command.type = CommandType::set_listener_r3_position;
	command.param.set_listener_r3_position.r3_position = r3_position;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try
{
	Command command{};
	command.type = CommandType::set_listener_r3_orientation;
	command.param.set_listener_r3_orientation.r3_orientation = r3_orientation;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool SystemAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try
{
	return voice_handle_mgr_.is_valid_handle(voice_handle);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::pause_voice;
	command.param.pause_voice.handle = voice_handle;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}

void SystemAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::resume_voice;
	command.param.resume_voice.handle = voice_handle;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}

void SystemAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::stop_voice;
	command.param.stop_voice.handle = voice_handle;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try
{
	AudioMixerValidator::validate_gain(gain);
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::set_voice_gain;
	command.param.set_voice_gain.handle = voice_handle;
	command.param.set_voice_gain.gain = gain;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::set_voice_r3_position;
	command.param.set_voice_r3_position.handle = voice_handle;
	command.param.set_voice_r3_position.position = r3_position;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool SystemAudioMixer::can_set_voice_output_gains() const
{
	return true;
}

void SystemAudioMixer::enable_set_voice_output_gains(AudioMixerVoiceHandle voice_handle, bool is_enable)
try
{
	if (!voice_handle.is_valid())
		return;
	Command command{};
	command.type = CommandType::enable_set_voice_output_gains;
	command.param.enable_set_voice_output_gains.handle = voice_handle;
	command.param.enable_set_voice_output_gains.is_enable = is_enable;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_output_gains(AudioMixerVoiceHandle voice_handle, AudioMixerOutputGains& output_gains)
try
{
	if (!voice_handle.is_valid())
		return;
	AudioMixerValidator::validate_output_gains(output_gains);
	Command command{};
	command.type = CommandType::set_voice_output_gains;
	command.param.set_voice_output_gains.handle = voice_handle;
	command.param.set_voice_output_gains.output_gains = output_gains;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::initialize_is_mute()
{
	is_mute_ = false;
}

void SystemAudioMixer::initialize_gain()
{
	gain_ = audio_mixer_max_gain;
}

void SystemAudioMixer::initialize_listener_r3_position()
{
	is_listener_r3_position_changed_ = true;
	listener_r3_position_ = audio_mixer_make_default_listener_r3_position();
}

void SystemAudioMixer::initialize_listener_r3_orientation()
{
	is_listener_r3_orientation_changed_ = true;
	listener_r3_orientation_ = audio_mixer_make_default_listener_r3_orientation();
}

void SystemAudioMixer::initialize_voice_handles()
{
	const int capacity = static_cast<int>(commands_.size());
	voice_handle_mgr_.set_cache_capacity(capacity);
	voice_handle_mgr_.set_map_capacity(capacity);
}

void SystemAudioMixer::initialize_voices(int max_voices)
{
	voices_.clear();
	voices_.resize(max_voices);
}

void SystemAudioMixer::callback(float* samples, [[maybe_unused]] int sample_count)
{
	std::copy(buffer_.cbegin(), buffer_.cend(), samples);
}

void SystemAudioMixer::mix()
{
	handle_commands();
	mix_samples();
}

void SystemAudioMixer::mix_samples_mono_to_stereo(
	double gain,
	const AudioMixerOutputGains& output_gains,
	int frame_count,
	const Sample* src_samples,
	Sample* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const double sample = gain * src_samples[i];
		const Sample left_sample = static_cast<Sample>(output_gains[0] * sample);
		const Sample right_sample = static_cast<Sample>(output_gains[1] * sample);
		dst_samples[(2 * i) + 0] += left_sample;
		dst_samples[(2 * i) + 1] += right_sample;
	}
}

void SystemAudioMixer::mix_samples_stereo_to_mono(
	double gain,
	const AudioMixerOutputGains& output_gains,
	int frame_count,
	const Sample* src_samples,
	Sample* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const double left_sample = src_samples[2 * i + 0];
		const double right_sample = src_samples[2 * i + 1];
		const double sample = 0.5 * gain * (left_sample + right_sample);
		dst_samples[(2 * i) + 0] += static_cast<Sample>(sample * output_gains[0]);
		dst_samples[(2 * i) + 1] += static_cast<Sample>(sample * output_gains[1]);
	}
}

void SystemAudioMixer::mix_samples_stereo_to_stereo(
	double gain,
	const AudioMixerOutputGains& output_gains,
	int frame_count,
	const Sample* src_samples,
	Sample* dst_samples)
{
	for (int i = 0; i < frame_count; ++i)
	{
		const double left_sample = gain * output_gains[0] * src_samples[2 * i + 0];
		const double right_sample = gain * output_gains[1] * src_samples[2 * i + 1];
		dst_samples[(2 * i) + 0] += static_cast<Sample>(left_sample);
		dst_samples[(2 * i) + 1] += static_cast<Sample>(right_sample);
	}
}

void SystemAudioMixer::mix_samples()
{
	spatialize_voices();
	std::fill(buffer_.begin(), buffer_.end(), Sample{});
	std::fill(mix_buffer_.begin(), mix_buffer_.end(), MixSample{});
	constexpr float sfx_gain_scale = 7.0F;
	constexpr float music_gain_scale = 6.0F;
	for (Voice& voice : voices_)
	{
		if (!voice.is_active || voice.is_paused)
			continue;
		if (!decode_voice(voice))
		{
			voice_handle_mgr_.unmap(voice.handle);
			voice.is_active = false;
			continue;
		}
		const bool is_opl_music = (voice.type == SoundType::opl_music);
		CacheItem* const cache_item = voice.cache;
		if (!is_opl_music && voice.decode_offset == cache_item->decoded_count)
		{
			voice_handle_mgr_.unmap(voice.handle);
			voice.is_active = false;
			continue;
		}
		double gain_scale;
		switch (voice.type)
		{
			case SoundType::opl_music:
				gain_scale = music_gain_scale;
				break;
			case SoundType::opl_sfx:
				gain_scale = sfx_gain_scale;
				break;
			default:
				gain_scale = 1.0;
				break;
		}
		if (!voice.is_custom_output_gains)
			gain_scale *= voice.gain;
		int decode_count = 0;
		if (is_opl_music)
			decode_count = cache_item->buffer_size;
		else
		{
			const auto remain_count = cache_item->decoded_count - voice.decode_offset;
			BSTONE_ASSERT(remain_count >= 0);
			decode_count = std::min(remain_count, mix_samples_count_);
		}
		if (!is_mute_)
		{
			MixSamples& mix_buffer = mix_buffer_;
			const int channel_count = cache_item->decoder->get_channel_count();
			const int base_offset = (is_opl_music ? 0 : voice.decode_offset) * channel_count;
			const AudioMixerOutputGains& gains = (voice.is_custom_output_gains ? voice.custom_output_gains : voice.output_gains);
			const bool is_src_mono = channel_count == 1;
			if (voice.is_r3)
			{
				if (is_src_mono)
					mix_samples_mono_to_stereo(gain_scale, gains, decode_count, &cache_item->samples[base_offset], &mix_buffer[0]);
				else
					mix_samples_stereo_to_mono(gain_scale, gains, decode_count, &cache_item->samples[base_offset], &mix_buffer[0]);
			}
			else
			{
				if (is_src_mono)
					mix_samples_mono_to_stereo(gain_scale, gains, decode_count, &cache_item->samples[base_offset], &mix_buffer[0]);
				else
					mix_samples_stereo_to_stereo(gain_scale, gains, decode_count, &cache_item->samples[base_offset], &mix_buffer[0]);
			}
		}
		if (!is_opl_music)
			voice.decode_offset += decode_count;
		if ((is_opl_music && cache_item->is_decoded()) ||
			(!is_opl_music && voice.decode_offset == cache_item->decoded_count))
		{
			if (cache_item->is_decoded())
			{
				bool is_erase = false;
				if (voice.type == SoundType::opl_music)
				{
					if (voice.is_looping && cache_item->decoder->rewind())
					{
						cache_item->decoded_count = 0;
						cache_item->buffer_size = 0;
					}
					else
						is_erase = true;
				}
				else
					is_erase = true;
				if (is_erase)
				{
					voice_handle_mgr_.unmap(voice.handle);
					voice.is_active = false;
					continue;
				}
			}
		}
	}
	if (!is_mute_)
	{
		const auto max_mix_sample_it = std::max_element(
			mix_buffer_.cbegin(),
			mix_buffer_.cend(),
			[](MixSample lhs, MixSample rhs)
			{
				return std::abs(lhs) < std::abs(rhs);
			});
		if (max_mix_sample_it != mix_buffer_.cend())
		{
			const double max_mix_sample_value = gain_;
			const float max_mix_sample = std::abs(*max_mix_sample_it);
			if (max_mix_sample <= max_mix_sample_value)
				std::copy(mix_buffer_.cbegin(), mix_buffer_.cend(), buffer_.begin());
			else
			{
				const double scalar = max_mix_sample_value / max_mix_sample;
				std::transform(
					mix_buffer_.cbegin(),
					mix_buffer_.cend(),
					buffer_.begin(),
					[scalar](MixSample item)
					{
						return static_cast<Sample>(item * scalar);
					});
			}
		}
	}
}

void SystemAudioMixer::handle_set_mute_command(const SetMuteCommandParam& param)
{
	is_mute_ = param.is_mute;
}

void SystemAudioMixer::handle_set_gain_command(const SetGainCommandParam& param)
{
	gain_ = param.gain;
}

void SystemAudioMixer::handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param)
{
	if (listener_r3_position_ != param.r3_position)
	{
		is_listener_r3_position_changed_ = true;
		listener_r3_position_ = param.r3_position;
	}
}

void SystemAudioMixer::handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param)
{
	if (listener_r3_orientation_ != param.r3_orientation)
	{
		is_listener_r3_orientation_changed_ = true;
		listener_r3_orientation_ = param.r3_orientation;
	}
}

void SystemAudioMixer::handle_pause_voice_command(const PauseVoiceCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
		voice != nullptr)
		voice->is_paused = true;
}

void SystemAudioMixer::handle_resume_voice_command(const ResumeVoiceCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
		voice != nullptr)
		voice->is_paused = false;
}

void SystemAudioMixer::handle_stop_voice_command(const StopVoiceCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice_and_invalidate(param.handle);
		voice != nullptr)
		voice->is_active = false;
}

void SystemAudioMixer::handle_set_voice_gain_command(const SetVoiceGainCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
		voice != nullptr)
		voice->gain = param.gain;
}

void SystemAudioMixer::handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param)
{
	Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
	if (voice == nullptr ||
		!voice->is_r3 ||
		voice->r3_position == param.position)
		return;
	voice->r3_position = param.position;
	voice->is_r3_position_changed = true;
}

void SystemAudioMixer::handle_enable_set_voice_output_gains_command(const EnableSetVoiceOutputGainsCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
		voice != nullptr)
		voice->is_custom_output_gains = param.is_enable;
}

void SystemAudioMixer::handle_set_voice_output_gains_command(const SetVoiceOutputGainsCommandParam& param)
{
	if (Voice* const voice = voice_handle_mgr_.get_voice(param.handle);
		voice != nullptr)
		voice->custom_output_gains = param.output_gains;
}

void SystemAudioMixer::handle_commands()
{
	if (is_state_suspended_.load(std::memory_order_acquire))
		return;
	{
		MtLockGuard guard_lock{mt_commands_lock_};
		if (!mt_commands_.empty())
		{
			if (commands_.empty())
				commands_ = mt_commands_;
			else
				commands_.insert(commands_.cend(), mt_commands_.cbegin(), mt_commands_.cend());
			mt_commands_.clear();
		}
	}
	if (commands_.empty())
		return;
	for (const Command& command : commands_)
	{
		switch (command.type)
		{
			case CommandType::play_sound:
				handle_play_sound_command(command);
				break;
			case CommandType::set_mute:
				handle_set_mute_command(command.param.set_mute);
				break;
			case CommandType::set_gain:
				handle_set_gain_command(command.param.set_gain);
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
			case CommandType::enable_set_voice_output_gains:
				handle_enable_set_voice_output_gains_command(command.param.enable_set_voice_output_gains);
				break;
			case CommandType::set_voice_output_gains:
				handle_set_voice_output_gains_command(command.param.set_voice_output_gains);
				break;
			default:
				BSTONE_ASSERT(false && "Unknown command.");
				break;
		}
	}
	commands_.clear();
}

void SystemAudioMixer::handle_play_sound_command(const Command& command)
{
	bool is_started = false;
	Voice* voice = nullptr;

	const auto voice_handle_guard = make_scope_exit(
		[this, &is_started, &voice, &command]()
		{
			if (is_started && voice != nullptr)
				voice_handle_mgr_.uncache_and_map(voice->handle, voice);
			else
				voice_handle_mgr_.uncache(command.param.play_sound.handle);
		});

	CacheItem* const cache_item = command.param.play_sound.cache;
	if (cache_item == nullptr)
		return;
	if (!initialize_cache_item(command, *cache_item))
		return;
	for (Voice& i_voice : voices_)
	{
		if (!i_voice.is_active)
		{
			voice = &i_voice;
			break;
		}
	}
	if (voice == nullptr)
		return;
	const PlaySoundCommandParam& play_sound_param = command.param.play_sound;
	voice->type = play_sound_param.sound_type;
	voice->is_r3 = play_sound_param.is_r3;
	voice->is_looping = play_sound_param.is_looping;
	voice->is_paused = false;
	voice->is_r3_position_changed = voice->is_r3;
	voice->is_custom_output_gains = false;
	voice->cache = play_sound_param.cache;
	voice->decode_offset = 0;
	voice->gain = audio_mixer_default_gain;
	voice->output_gains.fill(audio_mixer_max_gain * 0.5 * voice->gain);
	voice->custom_output_gains.fill(audio_mixer_max_gain * 0.5);
	voice->handle = play_sound_param.handle;
	voice->r3_position = audio_mixer_make_default_voice_r3_position();
	voice->r3_position_cache = voice->r3_position;
	voice->is_active = true;
	is_started = true;
}

bool SystemAudioMixer::initialize_cache_item(const Command& command, CacheItem& cache_item)
{
	const bool is_opl_music = (command.param.play_sound.sound_type == SoundType::opl_music);
	if (cache_item.is_active)
	{
		if (!is_opl_music)
			return !cache_item.is_invalid;
	}
	cache_item = CacheItem{};
	cache_item.is_invalid = true;
	AudioDecoderUPtr decoder = create_decoder_by_sound_type(command.param.play_sound.sound_type);
	if (decoder == nullptr)
		return false;
	const AudioDecoderInitParam param{
		.src_raw_data = command.param.play_sound.data,
		.src_raw_size = command.param.play_sound.data_size,
		.dst_rate = dst_rate_};
	if (!decoder->initialize(param))
		return false;
	const int samples_count = decoder->get_dst_length_in_samples();
	if (samples_count <= 0)
		return false;
	cache_item.is_active = true;
	cache_item.is_invalid = false;
	cache_item.sound_type = command.param.play_sound.sound_type;
	cache_item.samples_count = samples_count;
	cache_item.samples.resize((is_opl_music ? mix_samples_count_ : samples_count) * decoder->get_channel_count());
	cache_item.buffer_size = 0;
	cache_item.decoder.swap(decoder);
	return true;
}

bool SystemAudioMixer::decode_voice(const Voice& voice)
{
	CacheItem* const cache_item = voice.cache;
	if (cache_item == nullptr)
		return false;
	if (!cache_item->is_active)
		return false;
	if (cache_item->is_invalid)
		return false;
	if (cache_item->is_decoded())
		return true;
	if (voice.type == SoundType::opl_music)
	{
		const int total_remain_count = cache_item->samples_count - cache_item->decoded_count;
		if (total_remain_count == 0)
			return true;
		int remain_count = std::min(total_remain_count, cache_item->buffer_size);
		if (remain_count == 0)
			remain_count = std::min(total_remain_count, mix_samples_count_);
		cache_item->buffer_size = cache_item->decoder->decode(remain_count, cache_item->samples.data());
		cache_item->decoded_count += cache_item->buffer_size;
		return true;
	}
	const int ahead_count = std::min(voice.decode_offset + mix_samples_count_, cache_item->samples_count);
	if (ahead_count <= cache_item->decoded_count)
		return true;
	const int planned_count = std::min(cache_item->samples_count - cache_item->decoded_count, mix_samples_count_);
	const int channel_count = cache_item->decoder->get_channel_count();
	const int actual_count = cache_item->decoder->decode(planned_count, cache_item->samples.data() + cache_item->decoded_count * channel_count);
	cache_item->decoded_count += actual_count;
	return true;
}

void SystemAudioMixer::spatialize_voice(Voice& voice)
{
	if (!voice.is_active || !voice.is_r3 || voice.is_custom_output_gains)
		return;
	if (!is_listener_r3_position_changed_ &&
		!is_listener_r3_orientation_changed_ &&
		!voice.is_r3_position_changed)
		return;
	if (voice.is_r3_position_changed)
		voice.r3_position_cache = voice.r3_position;
	voice.is_r3_position_changed = false;
	AudioMixerUtils::spatialize_voice_2_0(
		listener_r3_position_cache_,
		listener_r3_orientation_cache_,
		voice.r3_position_cache,
		voice.output_gains[0],
		voice.output_gains[1]);
}

void SystemAudioMixer::spatialize_voices()
{
	if (is_listener_r3_position_changed_)
		listener_r3_position_cache_ = listener_r3_position_;
	if (is_listener_r3_orientation_changed_)
	{
		listener_r3_orientation_cache_.at = AudioMixerUtils::normalize(listener_r3_orientation_.at);
		listener_r3_orientation_cache_.up = AudioMixerUtils::normalize(listener_r3_orientation_.up);
	}
	for (Voice& voice : voices_)
		spatialize_voice(voice);
	is_listener_r3_position_changed_ = false;
	is_listener_r3_orientation_changed_ = false;
}

AudioMixerVoiceHandle SystemAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try
{
	if (!is_sound_type_valid(param.sound_type))
		BSTONE_THROW_STATIC_SOURCE("Invalid sound type.");
	if (!param.data)
		BSTONE_THROW_STATIC_SOURCE("Null data.");
	if (param.data_size <= 0)
		BSTONE_THROW_STATIC_SOURCE("Data size out of range.");
	const AudioMixerVoiceHandle voice_handle = voice_handle_mgr_.generate();
	Command command{};
	command.type = CommandType::play_sound;
	command.param.play_sound.sound_type = param.sound_type;
	command.param.play_sound.is_r3 = param.is_r3;
	command.param.play_sound.is_looping = param.is_looping;
	command.param.play_sound.cache = get_cache_item(param.sound_type, param.sound_index);
	command.param.play_sound.handle = voice_handle;
	command.param.play_sound.data = param.data;
	command.param.play_sound.data_size = param.data_size;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int SystemAudioMixer::calculate_mix_samples_count(int dst_rate, int mix_size_ms)
{
	const int exact_count = (dst_rate * mix_size_ms) / 1000;
	int actual_count = 1;
	while (actual_count < exact_count)
		actual_count *= 2;
	// Maximum power-of-two value for 16-bit unsigned type is 2^15 (32'768).
	if (actual_count > 32'768)
		actual_count = 32'768;
	return actual_count;
}

SystemAudioMixer::CacheItem* SystemAudioMixer::get_cache_item(SoundType sound_type, int sound_index)
{
	if (!is_sound_index_valid(sound_index, sound_type))
		return nullptr;
	switch (sound_type)
	{
		case SoundType::opl_music: return &opl_music_cache_[sound_index];
		case SoundType::opl_sfx: return &opl_sfx_cache_[sound_index];
		case SoundType::pc_speaker_sfx: return &pc_speaker_sfx_cache_[sound_index];
		case SoundType::pcm: return &pcm_cache_[sound_index];
		default: return nullptr;
	}
}

AudioDecoderUPtr SystemAudioMixer::create_decoder_by_sound_type(SoundType sound_type) const
{
	switch (sound_type)
	{
		case SoundType::opl_music:
			return make_audio_decoder(AudioDecoderType::opl_music, opl_emulator_type_);
		case SoundType::opl_sfx:
			return make_audio_decoder(AudioDecoderType::opl_sfx, opl_emulator_type_);
		case SoundType::pc_speaker_sfx:
			return make_audio_decoder(AudioDecoderType::pc_speaker, opl_emulator_type_);
		case SoundType::pcm:
			return make_audio_decoder(AudioDecoderType::pcm, opl_emulator_type_);
		default:
			return nullptr;
	}
}

bool SystemAudioMixer::is_sound_type_valid(SoundType sound_type)
{
	switch (sound_type)
	{
		case SoundType::opl_music:
		case SoundType::opl_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			return true;
		default:
			return false;
	}
}

bool SystemAudioMixer::is_sound_index_valid(int sound_index, SoundType sound_type)
{
	switch (sound_type)
	{
		case SoundType::opl_music:
			return sound_index >= 0 && sound_index < LASTMUSIC;
		case SoundType::opl_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			return sound_index >= 0 && sound_index < NUMSOUNDS;
		default:
			return false;
	}
}

} // namespace

// =====================================

AudioMixerUPtr make_system_audio_mixer(const AudioMixerInitParam& param)
{
	return std::make_unique<SystemAudioMixer>(param);
}

} // namespace bstone
