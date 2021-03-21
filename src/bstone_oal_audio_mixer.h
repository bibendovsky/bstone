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


#ifndef BSTONE_OAL_AUDIO_MIXER_INCLUDED
#define BSTONE_OAL_AUDIO_MIXER_INCLUDED


#include "bstone_audio_mixer.h"

#include <array>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "3d_def.h"
#include "audio.h"

#include "bstone_audio_decoder.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalAudioMixer final :
	public AudioMixer
{
public:
	OalAudioMixer(
		MtTaskMgr* mt_task_manager);

	~OalAudioMixer() override;


	// Note: Mix size in milliseconds.
	bool initialize(
		const AudioMixerInitParam& param) override;

	void uninitialize() override;

	bool is_initialized() const override;

	Opl3Type get_opl3_type() const override;

	int get_rate() const override;

	int get_channel_count() const override;

	int get_mix_size_ms() const override;

	float get_sfx_volume() const override;

	float get_music_volume() const override;

	AudioDecoderInterpolationType get_resampling_interpolation() const noexcept override;

	bool get_resampling_lpf() const noexcept override;

	bool play_adlib_music(
		int music_index,
		const void* data,
		int data_size) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_adlib_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pc_speaker_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pcm_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	bool set_resampling(
		bstone::AudioDecoderInterpolationType interpolation,
		bool low_pass_filter) override;

	bool update_positions() override;

	bool stop_music() override;

	bool stop_pausable_sfx() override;

	bool pause_all_sfx(
		bool is_pause) override;

	bool pause_music(
		bool is_pause) override;

	bool set_mute(
		bool value) override;

	bool set_sfx_volume(
		float volume) override;

	bool set_music_volume(
		float volume) override;

	bool is_music_playing() const override;

	bool is_any_unpausable_sfx_playing() const override;

	bool is_player_channel_playing(
		ActorChannel channel) const override;

	int get_min_rate() const override;

	int get_default_rate() const override;

	int get_min_mix_size_ms() const override;

	int get_default_mix_size_ms() const override;

	int get_max_channels() const override;

	int get_max_commands() const override;


private:
	using OalString = std::string;
	using OalStrings = std::vector<OalString>;

	using SfxAdLibSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcSpeakerSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;

	struct SfxPosition
	{
		double x{};
		double y{};
	}; // SfxPosition

	struct SfxPlayer
	{
		double view_x{};
		double view_y{};

		double view_cos{};
		double view_sin{};
	}; // SfxPlayer

	struct SfxPushwall
	{
		int direction{};
		int x{};
		int y{};
		double offset{};
	}; // SfxPushwall

	using SfxActorPositions = std::array<SfxPosition, MAXACTORS>;
	using SfxDoorPositions = std::array<SfxPosition, MAXDOORS>;
	using SfxModifiedPositionIndices = std::unordered_set<int>;

	struct SfxVoice
	{
		int index{};
		bool is_active{};
		OalSource oal_source{};

		int priority{};
		ActorType actor_type{};
		int actor_index{};
		ActorChannel actor_channel{};
	}; // SfxVoice

	using SfxVoices = std::vector<SfxVoice>;


	static constexpr auto min_mix_size_ms = 20;
	static constexpr auto max_mix_size_ms = 40;
	static_assert(max_mix_size_ms > min_mix_size_ms, "Mix size out of range.");

	static constexpr auto min_thread_delay_ms = min_mix_size_ms / 2;
	static_assert(min_thread_delay_ms > 1, "Mix size out of range.");

	static constexpr auto sfx_voices_limit = 255;
	static constexpr auto music_voices_limit = 1;
	static constexpr auto voices_limit = sfx_voices_limit + music_voices_limit;

	static constexpr auto meters_per_unit = 1.0;
	static constexpr auto default_reference_distance = 1.0;
	static constexpr auto default_position_y = 0.5;

	static constexpr auto adlib_sfx_volume_scale = 7;
	static constexpr auto adlib_music_volume_scale = 6;


	using Mutex = std::mutex;
	using MutexUniqueLock = std::unique_lock<Mutex>;
	using MusicMutex = Mutex;
	using SfxMutex = Mutex;

	using Thread = std::thread;


	MtTaskMgr* const mt_task_manager_;

	bool is_initialized_{};
	Opl3Type opl3_type_;
	int dst_rate_;
	int mix_sample_count_;
	int mix_size_ms_;
	AudioDecoderInterpolationType interpolation_;
	bool is_lpf_;
	bool is_mute_{};

	OalLoaderUPtr oal_loader_{};
	const OalSymbols* oal_symbols_{};
	OalDeviceResource oal_device_resource_{};
	OalContextResource oal_context_resource_{};

	mutable MusicMutex music_mutex_{};
	OalSourceUncachingSound music_adlib_sound_{};
	OalSource music_source_{};
	float music_volume_{};

	mutable SfxMutex sfx_mutex_{};
	SfxVoices sfx_voices_{};
	SfxAdLibSounds sfx_adlib_sounds_{};
	SfxPcSpeakerSounds sfx_pc_speaker_sounds_{};
	float sfx_volume_{};

	SfxActorPositions sfx_actor_positions_{};
	SfxDoorPositions sfx_door_positions_{};

	bool is_sfx_player_position_modified_{};
	SfxPlayer sfx_player_{};

	SfxModifiedPositionIndices sfx_actor_modified_position_indices_{};

	SfxModifiedPositionIndices sfx_door_modified_position_indices_{};

	bool is_sfx_pushwall_position_modified_{};
	SfxPushwall sfx_pushwall_{};


	bool is_quit_thread_{};
	Mutex thread_mutex_{};
	Thread thread_{};


	bool play_sfx_sound(
		AudioSfxType sfx_type,
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index,
		ActorType actor_type,
		ActorChannel actor_channel);


	void make_al_context_current();

	OalStrings get_alc_device_names();

	OalString get_default_alc_device_name();

	OalString get_alc_device_name();

	OalStrings parse_al_token_string(
		const char* al_token_string);

	OalStrings get_alc_extensions();

	OalStrings get_al_extensions();

	int get_al_mixing_frequency();

	int get_max_voice_count();


	void log(
		const OalString& string);

	void log_oal_library_path_name();

	void log_oal_device_name();

	void log_oal_device_names();

	void log_oal_default_device_name();

	void log_oal_current_device_name();

	void log_oal_alc_extensions();

	void log_oal_al_info();

	void log_oal_al_extensions();

	static const char* get_oal_default_library_path_name() noexcept;

	void initialize_oal(
		const AudioMixerInitParam& param);


	void initialize_music_adlib_sound();

	void initialize_music_source();

	void initialize_music();

	void uninitialize_music();


	void initialize_sfx_adlib_sounds();

	void initialize_sfx_pc_speaker_sounds();

	void initialize_sfx_voices();

	void initialize_sfx_positions();

	void initialize_sfx();

	void uninitialize_sfx();


	void decode_adlib_sound(
		OalSourceCachingSound& adlib_sound,
		int volume_scale);

	void decode_pc_speaker_sound(
		OalSourceCachingSound& pc_speaker_sound);

	void mix_sfx_voice(
		SfxVoice& sfx_voice);

	bool mix_music_mix_buffer();

	bool mix_music_mix_buffers();

	void mix_music();

	void initialize_thread();

	void thread_func();


	static bool is_2d_sfx(
		ActorType actor_type,
		int actor_index);

	SfxVoice* find_free_sfx_voice(
		int priority,
		ActorType actor_type,
		int actor_index,
		ActorChannel actor_channel);

	void set_al_listener_position(
		float x,
		float y,
		float z);

	void set_al_listener_orientation(
		float at_x,
		float at_y,
		float at_z,
		float up_x,
		float up_y,
		float up_z);


	void update_positions_player();

	void update_positions_actors();

	void update_positions_doors();

	void update_positions_pushwall();


	void apply_positions_player();

	void apply_actor_position(
		SfxVoice& sfx_voice);

	void apply_door_position(
		SfxVoice& sfx_voice);

	void apply_pushwall_position(
		SfxVoice& sfx_voice);

	void apply_positions_actors();

	void apply_positions_doors();

	void apply_positions_pushwall();

	void apply_positions();

	void set_sfx_actor_position(
		OalSource& oal_source,
		int actor_index);

	void set_sfx_door_position(
		OalSource& oal_source,
		int actor_index);

	void set_sfx_pushwall_position(
		OalSource& oal_source,
		int actor_index);

	void set_sfx_position(
		OalSource& oal_source,
		ActorType actor_type,
		int actor_index);

	void set_sfx_reference_distance(
		OalSource& oal_source);


	static OalSourceSample scale_sample(
		OalSourceSample sample,
		int scalar) noexcept;
}; // OalAudioMixer

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_OAL_AUDIO_MIXER_INCLUDED
