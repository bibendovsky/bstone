/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED


#define BSTONE_AUDIO_MIXER_USE_THREAD (0)


#include <atomic>
#include <list>
#include <memory>

#if BSTONE_AUDIO_MIXER_USE_THREAD
#include <mutex>
#include <thread>
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

#include <vector>
#include "SDL.h"
#include "bstone_atomic.h"
#include "bstone_audio_decoder.h"
#include "bstone_mt_queue_1r1w.h"


namespace bstone
{


enum class ActorType
{
	none,
	actor,
	door,
	wall,
}; // ActorType

enum class ActorChannel
{
	voice,
	weapon,
	item,
	hit_wall,
	no_way,
	interrogation,
}; // ActorChannel

enum class SoundType
{
	none,
	adlib_music,
	adlib_sfx,
	pcm,
}; // SoundType


class AudioMixer final
{
public:
	AudioMixer();

	AudioMixer(
		const AudioMixer& that) = delete;

	AudioMixer& operator=(
		const AudioMixer& that) = delete;

	~AudioMixer();


	// Note: Mix size in milliseconds.
	bool initialize(
		const int dst_rate,
		const int mix_size_ms);

	void uninitialize();

	bool is_initialized() const;

	bool play_adlib_music(
		const int music_index,
		const void* const data,
		const int data_size);

	// Negative index of an actor defines a non-positional sound.
	bool play_adlib_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice);

	// Negative index of an actor defines a non-positional sound.
	bool play_pcm_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice);

	bool update_positions();

	bool stop_music();

	bool stop_all_sfx();

	bool set_mute(
		const bool value);

	bool set_sfx_volume(
		const float volume);

	bool set_music_volume(
		const float volume);

	bool is_music_playing() const;

	bool is_any_sfx_playing() const;

	bool is_player_channel_playing(
		const ActorChannel channel) const;

	static int get_min_rate();

	static int get_default_rate();

	static int get_min_mix_size_ms();

	static int get_default_mix_size_ms();

	static int get_max_channels();

	static int get_max_commands();


private:
	using Sample = std::int16_t;
	using Samples = std::vector<Sample>;

	using MixSample = int;
	using MixSamples = std::vector<MixSample>;

	class CacheItem
	{
	public:
		bool is_active;
		bool is_invalid;
		SoundType sound_type;
		int samples_count;
		int decoded_count;
		Samples samples;
		std::unique_ptr<AudioDecoder> decoder;

		CacheItem();

		CacheItem(
			const CacheItem& that);

		~CacheItem();

		CacheItem& operator=(
			const CacheItem& that);

		bool is_decoded() const;
	}; // CacheItem

	using Cache = std::vector<CacheItem>;

	struct Location final
	{
		Atomic<int> x;
		Atomic<int> y;
	}; // Location

	using Locations = std::vector<Location>;

	struct PlayerLocation final
	{
		Atomic<int> view_x;
		Atomic<int> view_y;
		Atomic<int> view_cos;
		Atomic<int> view_sin;
	}; // PlayerLocation

	struct Positions final
	{
		PlayerLocation player;
		Locations actors;
		Locations doors;
		Location wall;

		void initialize();

		void fixed_copy_to(
			Positions& positions);
	}; // Positions

	struct Sound final
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

	enum CommandType
	{
		CMD_PLAY,
		CMD_STOP_MUSIC,
		CMD_STOP_ALL_SFX
	}; // CommandType

	struct Command final
	{
		CommandType command;
		Sound sound;
		const void* data;
		int data_size;
	}; // Command

	using Commands = bstone::MtQueue1R1W<Command>;

#if BSTONE_AUDIO_MIXER_USE_THREAD
	using Mutex = std::mutex;
	using MutexGuard = std::lock_guard<Mutex>;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

	bool is_initialized_;
	int dst_rate_;
	SDL_AudioDeviceID device_id_;

#if BSTONE_AUDIO_MIXER_USE_THREAD
	Mutex mutex_;
	std::thread thread_;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

	int mix_samples_count_;
	Samples buffer_;
	MixSamples mix_buffer_;
	std::atomic_bool is_data_available_;

#if BSTONE_AUDIO_MIXER_USE_THREAD
	std::atomic_bool quit_thread_;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

	Sounds sounds_;
	Commands commands_;
	bool mute_;
	Cache adlib_music_cache_;
	Cache adlib_sfx_cache_;
	Cache pcm_cache_;
	Positions positions_;
	std::atomic_int player_channels_state_;
	std::atomic_bool is_music_playing_;
	std::atomic_bool is_any_sfx_playing_;
	std::atomic<float> sfx_volume_;
	std::atomic<float> music_volume_;
	int mix_size_ms_;

	void callback(
		std::uint8_t* dst_data,
		const int dst_length);

	void mix();

	void mix_samples();

	void handle_commands();

	void handle_play_command(
		const Command& command);

	void handle_stop_music_command();

	void handle_stop_all_sfx_command();

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

	static AudioDecoder* create_decoder_by_sound_type(
		const SoundType sound_type);

	static bool is_sound_type_valid(
		const SoundType sound_type);

	static bool is_sound_index_valid(
		const int sound_index,
		const SoundType sound_type);
}; // AudioMixer


} // bstone


#endif // !BSTONE_AUDIO_MIXER_INCLUDED
