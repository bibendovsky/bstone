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

#ifndef BSTONE_SDL_AUDIO_MIXER_INCLUDED
#define BSTONE_SDL_AUDIO_MIXER_INCLUDED

#include "bstone_audio_mixer.h"
#include <cstdint>
#include <array>
#include <atomic>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>
#include "SDL_audio.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_sdl_types.h"
#include "bstone_spinlock.h"

namespace bstone
{

class SdlAudioMixer final : public AudioMixer
{
public:
	SdlAudioMixer(const AudioMixerInitParam& param);
	~SdlAudioMixer();

	Opl3Type get_opl3_type() const override;
	int get_rate() const override;
	int get_channel_count() const override;
	int get_mix_size_ms() const override;

	void suspend_state() override;
	void resume_state() override;

	void set_mute(bool value) override;
	void set_distance_model(AudioMixerDistanceModel distance_model) override;

	void set_listener_r3_position(const AudioMixerListenerR3Position& r3_position) override;
	void set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation) override;

	AudioMixerVoiceHandle play_sound(const AudioMixerPlaySoundParam& param) override;

	bool is_voice_playing(AudioMixerVoiceHandle voice_handle) const override;

	void pause_voice(AudioMixerVoiceHandle voice_handle) override;
	void resume_voice(AudioMixerVoiceHandle voice_handle) override;
	void stop_voice(AudioMixerVoiceHandle voice_handle) override;

	void set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain) override;
	void set_voice_r3_attenuation(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Attenuation& r3_attenuation) override;
	void set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position) override;

private:
	using Sample = float;
	using Samples = std::vector<Sample>;

	using S16Sample = std::int16_t;
	using S16Samples = std::vector<S16Sample>;

	using MixSample = float;
	using MixSamples = std::vector<MixSample>;

	using MtLock = std::mutex;
	using MtLockGuard = std::lock_guard<MtLock>;

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
		SoundType type;
		bool is_active;
		bool is_r3;
		bool is_looping;
		bool is_paused;
		bool is_r3_attenuation_changed;
		bool is_r3_position_changed;
		CacheItem* cache;
		int decode_offset;
		double gain;
		double left_gain;
		double right_gain;
		AudioMixerVoiceHandle handle;
		AudioMixerVoiceR3Attenuation r3_attenuation;
		AudioMixerVoiceR3Attenuation r3_attenuation_cache;
		AudioMixerVoiceR3Position r3_position;
		AudioMixerVoiceR3Position r3_position_cache;

		bool is_audible() const noexcept;
	}; // Voice

	using Voices = std::vector<Voice>;

	enum class CommandType
	{
		play_sound,

		set_mute,
		set_distance_model,

		set_listener_r3_position,
		set_listener_r3_orientation,

		pause_voice,
		resume_voice,
		stop_voice,

		set_voice_gain,
		set_voice_r3_attenuation,
		set_voice_r3_position,
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

	struct SetDistanceModelCommandParam
	{
		AudioMixerDistanceModel distance_model;
	}; // SetDistanceModelCommandParam

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

	struct SetVoiceR3AttenuationCommandParam
	{
		AudioMixerVoiceHandle handle;
		AudioMixerVoiceR3Attenuation attributes;
	}; // SetVoiceR3AttenuationCommandParam

	struct SetVoiceR3PositionCommandParam
	{
		AudioMixerVoiceHandle handle;
		AudioMixerVoiceR3Position position;
	}; // SetVoiceR3PositionCommandParam

	union CommandParam
	{
		PlaySoundCommandParam play_sound;

		SetMuteCommandParam set_mute;
		SetDistanceModelCommandParam set_distance_model;

		SetListenerR3PositionCommandParam set_listener_r3_position;
		SetListenerR3OrientationCommandParam set_listener_r3_orientation;

		PauseVoiceCommandParam pause_voice;
		ResumeVoiceCommandParam resume_voice;
		StopVoiceCommandParam stop_voice;

		SetVoiceGainCommandParam set_voice_gain;
		SetVoiceR3AttenuationCommandParam set_voice_r3_attenuation;
		SetVoiceR3PositionCommandParam set_voice_r3_position;
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
	SdlAudioDevice sdl_audio_device_{};
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
	AudioMixerDistanceModel distance_model_{};
	AudioMixerListenerR3Position listener_r3_position_{};
	AudioMixerListenerR3Position listener_r3_position_cache_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_cache_{};
	bool is_distance_model_changed_;
	bool is_listener_r3_position_changed_;
	bool is_listener_r3_orientation_changed_;
	std::atomic_bool is_state_suspended_{};

	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);

	int get_min_rate() const noexcept;
	int get_default_rate() const noexcept;
	int get_min_mix_size_ms() const noexcept;
	int get_default_mix_size_ms() const noexcept;
	int get_max_channels() const noexcept;

	void initialize_mute();
	void initialize_distance_model();
	void initialize_listener_r3_position();
	void initialize_listener_r3_orientation();
	void initialize_voice_handles();
	void initialize_voices(int max_voices);

	void callback(std::uint8_t* dst_data, int dst_length);

	void mix();
	void mix_samples();

	void handle_set_mute_command(const SetMuteCommandParam& param) noexcept;
	void handle_set_distance_model_command(const SetDistanceModelCommandParam& param) noexcept;

	void handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param) noexcept;
	void handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param) noexcept;

	void handle_pause_voice_command(const PauseVoiceCommandParam& param);
	void handle_resume_voice_command(const ResumeVoiceCommandParam& param);
	void handle_stop_voice_command(const StopVoiceCommandParam& param);
	void handle_set_voice_gain_command(const SetVoiceGainCommandParam& param);
	void handle_set_voice_r3_attenuation_command(const SetVoiceR3AttenuationCommandParam& param);
	void handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param);
	void handle_commands();

	void handle_play_sound_command(const Command& command);
	bool initialize_digitized_cache_item(const Command& command, CacheItem& cache_item);
	bool initialize_cache_item(const Command& command, CacheItem& cache_item);

	bool decode_digitized_voice(const Voice& voice);
	bool decode_voice(const Voice& voice);

	void spatialize_voice(Voice& voice);
	void spatialize_voices();

	CacheItem* get_cache_item(SoundType sound_type, int sound_index);

	void lock();
	void unlock();

	static void callback_proxy(void* user_data, std::uint8_t* dst_data, int dst_length);
	static int calculate_mix_samples_count(int dst_rate, int mix_size_ms);
	AudioDecoderUPtr create_decoder_by_sound_type(SoundType sound_type) const;
	static bool is_sound_type_valid(SoundType sound_type);
	static bool is_sound_index_valid(int sound_index, SoundType sound_type);
	static int calculate_digitized_sample_count(int dst_sample_rate, int digitized_byte_count) noexcept;
}; // SdlAudioMixer

} // bstone

#endif // !BSTONE_SDL_AUDIO_MIXER_INCLUDED
