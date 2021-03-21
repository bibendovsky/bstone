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


#ifndef BSTONE_SDL2_AUDIO_MIXER_INCLUDED
#define BSTONE_SDL2_AUDIO_MIXER_INCLUDED


#include "bstone_audio_mixer.h"

#include <atomic>
#include <deque>
#include <list>
#include <mutex>
#include <utility>
#include <vector>

#include "SDL_audio.h"

#include "bstone_atomic_flag.h"
#include "bstone_audio_decoder.h"
#include "bstone_mt_task_mgr.h"


namespace bstone
{


class Sdl2AudioMixer final :
	public AudioMixer
{
public:
	Sdl2AudioMixer(
		MtTaskMgr* const mt_task_manager);

	~Sdl2AudioMixer() override;


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
		const int music_index,
		const void* const data,
		const int data_size) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_adlib_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pc_speaker_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pcm_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) override;

	bool set_resampling(
		const bstone::AudioDecoderInterpolationType interpolation,
		const bool low_pass_filter_) override;

	bool update_positions() override;

	bool stop_music() override;

	bool stop_pausable_sfx() override;

	bool pause_all_sfx(
		const bool is_pause) override;

	bool pause_music(
		const bool is_pause) override;

	bool set_mute(
		const bool value) override;

	bool set_sfx_volume(
		const float volume) override;

	bool set_music_volume(
		const float volume) override;

	bool is_music_playing() const override;

	bool is_any_unpausable_sfx_playing() const override;

	bool is_player_channel_playing(
		const ActorChannel channel) const override;

	int get_min_rate() const override;

	int get_default_rate() const override;

	int get_min_mix_size_ms() const override;

	int get_default_mix_size_ms() const override;

	int get_max_channels() const override;

	int get_max_commands() const override;


private:
	using Sample = float;
	using Samples = std::vector<Sample>;

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
		int buffer_size_;
		Samples samples;
		AudioDecoderUPtr decoder;


		CacheItem();

		bool is_decoded() const;
	}; // CacheItem

	using Cache = std::deque<CacheItem>;

	struct Location
	{
		double x;
		double y;
	}; // Location

	using Locations = std::vector<Location>;

	struct PlayerLocation
	{
		double view_x;
		double view_y;
		double view_cos;
		double view_sin;
	}; // PlayerLocation

	struct Positions
	{
		PlayerLocation player;
		Locations actors;
		Locations doors;
		Location wall;

		void initialize();

		void fixed_copy_to(
			Positions& positions);
	}; // Positions

	using Indices = std::vector<int>;

	struct Sound
	{
		SoundType type;
		int priority;
		CacheItem* cache;
		int decode_offset;
		int actor_index;
		ActorType actor_type;
		ActorChannel actor_channel;
		float left_volume;
		float right_volume;

		bool is_audible() const;
	}; // Sound

	using Sounds = std::list<Sound>;

	enum class CommandType
	{
		play,
		stop_music,
		stop_pausable_sfx,
		resampling,
	}; // CommandType

	struct CommandPlay
	{
		Sound sound;
		const void* data;
		int data_size;
	}; // CommandPlay

	struct CommandResampling
	{
		AudioDecoderInterpolationType interpolation_;
		bool low_pass_filter_;
	}; // CommandResampling

	struct Command
	{
		CommandType command_;

		union
		{
			CommandPlay play_;
			CommandResampling resampling_;
		};
	}; // Command

	using Commands = std::vector<Command>;


	class SetResamplingMtTask final :
		public MtTask
	{
	public:
		void execute() override;


		bool is_completed() const noexcept override;

		void set_completed() override;


		bool is_failed() const noexcept override;

		std::exception_ptr get_exception_ptr() const noexcept override;

		void set_failed(
			std::exception_ptr exception_ptr) override;


		void initialize(
			CacheItem* cache_item,
			const AudioDecoderInterpolationType interpolation_,
			const bool is_lpf_) noexcept;


	private:
		CacheItem* cache_item_{};
		AudioDecoderInterpolationType interpolation_{};
		bool is_lpf_{};

		AtomicFlag is_completed_{};
		AtomicFlag is_failed_{};

		std::exception_ptr exception_ptr_{};
	}; // SetResamplingMtTask

	using SetResamplingMtTasks = std::vector<SetResamplingMtTask>;
	using SetResamplingMtTasksPtrs = std::vector<MtTask*>;


	MtTaskMgr* const mt_task_manager_;

	bool is_initialized_;
	Opl3Type opl3_type_;
	int dst_rate_;
	SDL_AudioDeviceID device_id_;
	int mix_samples_count_;
	Samples buffer_;
	MixSamples mix_buffer_;
	std::atomic_bool is_data_available_;
	Sounds sounds_;
	Commands commands_;
	Commands mt_commands_;
	MtLock mt_commands_lock_;
	std::atomic_bool mt_is_muted_;
	std::atomic_bool mt_is_sfx_paused_;
	std::atomic_bool mt_is_music_paused_;
	Cache adlib_music_cache_;
	Cache adlib_sfx_cache_;
	Cache pc_speaker_sfx_cache_;
	Cache pcm_cache_;
	Positions mt_positions_;
	Positions positions_;
	Indices modified_actors_indices_;
	Indices modified_doors_indices_;
	MtLock mt_positions_lock_;
	std::atomic_int player_channels_state_;
	std::atomic_bool is_music_playing_;
	std::atomic_bool is_any_sfx_playing_;
	std::atomic<float> sfx_volume_;
	std::atomic<float> music_volume_;
	int mix_size_ms_;
	AudioDecoderInterpolationType interpolation_;
	bool is_lpf_;
	SetResamplingMtTasks set_resampling_mt_tasks_;
	SetResamplingMtTasksPtrs set_resampling_mt_tasks_ptrs_;


	void callback(
		std::uint8_t* dst_data,
		const int dst_length);

	void mix();

	void mix_samples();

	void handle_resampling();

	void handle_commands();

	void handle_play_command(
		const Command& command);

	void handle_stop_music_command();

	void handle_stop_pausable_sfx_command();

	bool initialize_cache_item(
		const Command& command,
		CacheItem& cache_item);

	bool decode_sound(
		const Sound& sound);

	void spatialize_sound(
		Sound& sound);

	void spatialize_sounds();

	bool play_sound(
		const SoundType sound_type,
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice);

	CacheItem* get_cache_item(
		const SoundType sound_type,
		const int sound_index);

	void set_player_channel_state(
		const Sound& sound,
		const bool state);

	void lock();

	void unlock();

	static void callback_proxy(
		void* user_data,
		std::uint8_t* dst_data,
		const int dst_length);

	static int mix_proxy(
		void* user_data);

	static int calculate_mix_samples_count(
		const int dst_rate,
		const int mix_size_ms);

	AudioDecoderUPtr create_decoder_by_sound_type(
		const SoundType sound_type) const;

	static bool is_sound_type_valid(
		const SoundType sound_type);

	static bool is_sound_index_valid(
		const int sound_index,
		const SoundType sound_type);
}; // Sdl2AudioMixer


} // bstone


#endif // !BSTONE_SDL2_AUDIO_MIXER_INCLUDED
