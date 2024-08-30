/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_OAL_AUDIO_MIXER_INCLUDED
#define BSTONE_OAL_AUDIO_MIXER_INCLUDED

#include <array>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include "3d_def.h"
#include "audio.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer.h"
#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"
#include "bstone_oal_source.h"

namespace bstone
{

class OalAudioMixer final : public AudioMixer
{
public:
	OalAudioMixer(const AudioMixerInitParam& param);
	~OalAudioMixer() override;

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
	void set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position) override;

	bool can_set_voice_output_gains() const override;
	void enable_set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		bool is_enable) override;
	void set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		AudioMixerOutputGains& output_gains) override;

private:
	using Mutex = std::mutex;
	using MutexUniqueLock = std::unique_lock<Mutex>;

	using OalString = std::string;
	using OalStrings = std::vector<OalString>;

	using SfxAdLibSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcSpeakerSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcmSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;

	struct Voice
	{
		int index{};
		bool is_active{};
		bool is_r3{};
		bool is_looping{};
		bool is_music{};

		AudioMixerVoiceR3Position r3_position;

		OalSource oal_source;
		AudioMixerVoiceHandle handle;
	}; // Voice

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
	}; // CommandType

	struct PlayMusicCommandParam
	{
		const void* data;
		int data_size;
		bool is_looping;
		AudioMixerVoiceHandle voice_handle;
	}; // PlayMusicCommandParam

	struct PlaySfxCommandParam
	{
		SoundType sound_type;
		bool is_r3;
		int sound_index;
		const void* data;
		int data_size;
		AudioMixerVoiceHandle voice_handle;
	}; // PlaySfxCommandParam

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
	}; // CommandParam

	struct Command
	{
		CommandType type;
		CommandParam param;
	}; // Command

	using Commands = std::vector<Command>;
	using CommandQueueMutex = Mutex;

	static constexpr auto commands_min_capacity = 1'024;

	static constexpr auto min_mix_size_ms = 20;
	static constexpr auto max_mix_size_ms = 40;
	static_assert(max_mix_size_ms > min_mix_size_ms, "Mix size out of range.");

	static constexpr auto sfx_voices_limit = 255;
	static constexpr auto music_voices_limit = 1;
	static constexpr auto voices_limit = sfx_voices_limit + music_voices_limit;

	static constexpr auto adlib_sfx_gain_scale = 7;
	static constexpr auto adlib_music_gain_scale = 6;

	static constexpr auto alc_enumeration_ext_str = "ALC_ENUMERATION_EXT";
	static constexpr auto alc_enumerate_all_ext_str = "ALC_ENUMERATE_ALL_EXT";

	using Thread = std::thread;
	using VoiceHandleMgr = AudioMixerVoiceHandleMgr<Voice>;

	Opl3Type opl3_type_;
	int dst_rate_;
	int mix_sample_count_;
	int mix_size_ms_;
	bool is_mute_{};
	double gain_{};
	AudioMixerListenerR3Position listener_r3_position_{};
	AudioMixerListenerR3Orientation listener_r3_orientation_{};

	bool has_alc_enumeration_ext_{};
	bool has_alc_enumerate_all_ext_{};

	OalLoaderUPtr oal_loader_{};
	OalAlSymbols al_symbols_{};
	OalDeviceResource oal_device_resource_{};
	OalContextResource oal_context_resource_{};

	VoiceHandleMgr voice_handle_mgr_{};

	CommandQueueMutex commands_mutex_{};
	Commands commands_{};
	Commands mt_commands_{};

	OalSourceUncachingSound music_adlib_sound_{};

	SfxAdLibSounds sfx_adlib_sounds_{};
	SfxPcSpeakerSounds sfx_pc_speaker_sounds_{};
	SfxPcmSounds sfx_pcm_sounds_{};

	Voices voices_{};

	bool is_quit_thread_{};
	Mutex thread_mutex_{};
	Thread thread_{};

	std::atomic_bool is_state_suspended_{};

	[[noreturn]] static void fail_unsupported();

	int get_min_rate() const noexcept;
	int get_min_mix_size_ms() const noexcept;
	int get_default_mix_size_ms() const noexcept;
	int get_max_channels() const noexcept;

	void make_al_context_current();

	OalStrings get_alc_device_names();
	OalString get_default_alc_device_name();
	OalString get_alc_device_name();

	OalStrings parse_al_token_string(const char* al_token_string);
	OalStrings get_alc_extensions();
	OalStrings get_al_extensions();

	int get_al_mixing_frequency();
	int get_max_voice_count();

	void detect_alc_extensions();
	
	void log(const OalString& string);
	void log_oal_library_file_name();
	void log_oal_custom_device();
	void log_oal_devices();
	void log_oal_default_device();
	void log_oal_current_device_name();
	void log_oal_alc_extensions();
	void log_oal_al_info();
	void log_oal_al_extensions();

	static const char* get_oal_default_library_file_name() noexcept;

	void initialize_oal(const AudioMixerInitParam& param);
	void initialize_distance_model();
	void initialize_is_mute() noexcept;
	void initialize_gain() noexcept;
	void initialize_listener_r3_position();
	void initialize_listener_r3_orientation();
	void initialize_voice_handles();
	void initialize_command_queue();
	void initialize_voices();
	void initialize_music_adlib_sound();
	void initialize_music();
	void uninitialize_music();
	void initialize_sfx_adlib_sounds();
	void initialize_sfx_pc_speaker_sounds();
	void initialize_sfx_pcm_sounds();
	void initialize_sfx();
	void uninitialize_sfx();

	void on_music_stop(Voice& voice);
	void on_sfx_stop(const Voice& voice);

	AudioMixerVoiceHandle play_adlib_music_internal(const void* data, int data_size, bool is_looping);
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

	void decode_adlib_sound(OalSourceCachingSound& adlib_sound, int gain_scale);
	void decode_pc_speaker_sound(OalSourceCachingSound& pc_speaker_sound);
	void decode_pcm_sound(OalSourceCachingSound& pcm_sound);

	void mix_sfx_voice(Voice& voice);
	bool mix_music_mix_buffer(Voice& voice);
	bool mix_music_mix_buffers(Voice& voice);
	void mix_music(Voice& voice);

	void initialize_thread();
	void thread_func();

	Voice* find_free_voice() noexcept;
	Voice* find_music_voice() noexcept;

	void set_al_listener_r3_position(double x, double y, double z);
	void set_listener_r3_position();

	void set_al_listener_orientation(double at_x, double at_y, double at_z, double up_x, double up_y, double up_z);
	void set_listener_r3_orientation();

	static OalSourceSample scale_sample(OalSourceSample sample, int scalar) noexcept;
}; // OalAudioMixer

} // bstone

#endif // !BSTONE_OAL_AUDIO_MIXER_INCLUDED
