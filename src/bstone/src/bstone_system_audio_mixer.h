/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED
#define BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED

#include <cstdint>
#include <array>
#include <atomic>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer.h"
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_sys_audio_mgr.h"

namespace bstone
{

class SystemAudioMixer final : public AudioMixer
{
public:
	SystemAudioMixer(const AudioMixerInitParam& param);
	~SystemAudioMixer() override;

	Opl3Type get_opl3_type() const override;
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
	void set_voice_r3_position(
		AudioMixerVoiceHandle voice_handle,
		const AudioMixerVoiceR3Position& r3_position) override;

	bool can_set_voice_output_gains() const override;
	void enable_set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		bool is_enable) override;
	void set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		AudioMixerOutputGains& output_gains) override;

private:
	using Sample = float;
	using Samples = std::vector<Sample>;

	using S16Sample = std::int16_t;
	using S16Samples = std::vector<S16Sample>;

	using MixSample = float;
	using MixSamples = std::vector<MixSample>;

	using MtLock = std::mutex;
	using MtLockGuard = std::lock_guard<MtLock>;

	struct SysCallback final : public sys::PollingAudioDeviceCallback
	{
	public:
		void set_mixer(SystemAudioMixer* mixer);

	private:
		SystemAudioMixer* mixer_{};

	private:
		void do_invoke(float* samples, int sample_count) override;
	};

	class CacheItem
	{
	public:
		bool is_active;
		bool is_invalid;
		SoundType sound_type;
		int samples_count;
		int decoded_count;
		int digitized_resampler_counter{};
		int digitized_data_offset{};
		int digitized_data_size{};
		Sample digitized_last_sample{};
		const std::uint8_t* digitized_data{};
		int buffer_size_;
		Samples samples;
		AudioDecoderUPtr decoder;

		CacheItem();

		bool is_decoded() const noexcept;
	}; // CacheItem

	using Cache = std::deque<CacheItem>;

	struct Voice
	{
		SoundType type{SoundType::none};
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
		AudioMixerVoiceHandle handle;
		AudioMixerVoiceR3Position r3_position;
		AudioMixerVoiceR3Position r3_position_cache;
	}; // Voice

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
	}; // CommandType

	struct PlaySoundCommandParam
	{
		SoundType sound_type;
		bool is_r3;
		bool is_looping;
		CacheItem* cache;
		AudioMixerVoiceHandle handle;
		const void* data;
		int data_size;
	}; // PlaySoundCommandParam

	struct SetMuteCommandParam
	{
		bool is_mute;
	}; // SetMuteCommandParam

	struct SetGainCommandParam
	{
		double gain;
	};

	struct SetListenerR3PositionCommandParam
	{
		AudioMixerListenerR3Position r3_position;
	}; // SetListenerR3PositionCommandParam

	struct SetListenerR3OrientationCommandParam
	{
		AudioMixerListenerR3Orientation r3_orientation;
	}; // SetListenerR3OrientationCommandParam

	struct PauseVoiceCommandParam
	{
		AudioMixerVoiceHandle handle;
	}; // PauseVoiceCommandParam

	struct ResumeVoiceCommandParam
	{
		AudioMixerVoiceHandle handle;
	}; // ResumeVoiceCommandParam

	struct StopVoiceCommandParam
	{
		AudioMixerVoiceHandle handle;
	}; // StopVoiceCommandParam

	struct SetVoiceGainCommandParam
	{
		AudioMixerVoiceHandle handle;
		double gain;
	}; // SetVoiceGainCommandParam

	struct SetVoiceR3PositionCommandParam
	{
		AudioMixerVoiceHandle handle;
		AudioMixerVoiceR3Position position;
	}; // SetVoiceR3PositionCommandParam

	struct EnableSetVoiceOutputGainsCommandParam
	{
		AudioMixerVoiceHandle handle;
		bool is_enable;
	};

	struct SetVoiceOutputGainsCommandParam
	{
		AudioMixerVoiceHandle handle;
		AudioMixerOutputGains output_gains;
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
	}; // CommandParam

	struct Command
	{
		CommandType type{};
		CommandParam param{};
	}; // Command

	using Commands = std::vector<Command>;

	using VoiceHandleMgr = AudioMixerVoiceHandleMgr<Voice>;

	Opl3Type opl3_type_{};
	int dst_rate_{};
	SysCallback sys_callback_{};
	sys::AudioMgr* sys_audio_mgr_{};
	int mix_samples_count_{};
	Samples buffer_{};
	S16Samples s16_samples_{};
	MixSamples mix_buffer_{};
	VoiceHandleMgr voice_handle_mgr_{};
	Voices voices_{};
	Commands commands_{};
	Commands mt_commands_{};
	MtLock mt_commands_lock_{};
	Cache adlib_music_cache_{};
	Cache adlib_sfx_cache_{};
	Cache pc_speaker_sfx_cache_{};
	Cache pcm_cache_{};
	int mix_size_ms_{};
	bool is_mute_{};
	double gain_{};
	AudioMixerListenerR3Position listener_r3_position_{};
	AudioMixerListenerR3Position listener_r3_position_cache_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_cache_{};
	bool is_listener_r3_position_changed_;
	bool is_listener_r3_orientation_changed_;
	std::atomic_bool is_state_suspended_{};
	sys::PollingAudioDeviceUPtr sys_audio_device_{};

	int get_min_rate() const noexcept;
	int get_default_rate() const noexcept;
	int get_min_mix_size_ms() const noexcept;
	int get_default_mix_size_ms() const noexcept;
	int get_max_channels() const noexcept;

	void initialize_is_mute();
	void initialize_gain();
	void initialize_listener_r3_position();
	void initialize_listener_r3_orientation();
	void initialize_voice_handles();
	void initialize_voices(int max_voices);

	void callback(float* samples, int sample_count);

	void mix();
	void mix_samples();

	void handle_set_mute_command(const SetMuteCommandParam& param) noexcept;
	void handle_set_gain_command(const SetGainCommandParam& param) noexcept;

	void handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param) noexcept;
	void handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param) noexcept;

	void handle_pause_voice_command(const PauseVoiceCommandParam& param);
	void handle_resume_voice_command(const ResumeVoiceCommandParam& param);
	void handle_stop_voice_command(const StopVoiceCommandParam& param);
	void handle_set_voice_gain_command(const SetVoiceGainCommandParam& param);
	void handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param);
	void handle_enable_set_voice_output_gains_command(const EnableSetVoiceOutputGainsCommandParam& param);
	void handle_set_voice_output_gains_command(const SetVoiceOutputGainsCommandParam& param);
	void handle_commands();

	void handle_play_sound_command(const Command& command);
	bool initialize_digitized_cache_item(const Command& command, CacheItem& cache_item);
	bool initialize_cache_item(const Command& command, CacheItem& cache_item);

	bool decode_digitized_voice(const Voice& voice);
	bool decode_voice(const Voice& voice);

	void spatialize_voice(Voice& voice);
	void spatialize_voices();

	CacheItem* get_cache_item(SoundType sound_type, int sound_index);

	static int calculate_mix_samples_count(int dst_rate, int mix_size_ms);
	AudioDecoderUPtr create_decoder_by_sound_type(SoundType sound_type) const;
	static bool is_sound_type_valid(SoundType sound_type);
	static bool is_sound_index_valid(int sound_index, SoundType sound_type);
	static int calculate_digitized_sample_count(int dst_sample_rate, int digitized_byte_count) noexcept;
};

} // bstone

#endif // BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED
