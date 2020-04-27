/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "bstone_audio_mixer.h"

#include <atomic>
#include <list>
#include <mutex>
#include <utility>

#include "SDL_audio.h"

#include "3d_def.h"
#include "audio.h"
#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


const int ATABLEMAX = 15;

extern bool sd_sq_played_once_;
extern std::uint8_t lefttable[ATABLEMAX][ATABLEMAX * 2];
extern std::uint8_t righttable[ATABLEMAX][ATABLEMAX * 2];


namespace bstone
{


class AudioMixer::Impl final
{
public:
	Impl();

	~Impl();


	// Note: Mix size in milliseconds.
	bool initialize(
		const int dst_rate,
		const int mix_size_ms);

	void uninitialize();

	bool is_initialized() const;

	int get_rate() const;

	int get_channel_count() const;

	int get_mix_size_ms() const;

	float get_sfx_volume() const;

	float get_music_volume() const;

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
		int x;
		int y;
	}; // Location

	using Locations = std::vector<Location>;

	struct PlayerLocation final
	{
		int view_x;
		int view_y;
		int view_cos;
		int view_sin;
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
		stop_all_sfx
	}; // CommandType

	struct Command final
	{
		CommandType command;
		Sound sound;
		const void* data;
		int data_size;
	}; // Command

	using Commands = std::vector<Command>;


	bool is_initialized_;
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
	bool mute_;
	Cache adlib_music_cache_;
	Cache adlib_sfx_cache_;
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


AudioMixer::Impl::CacheItem::CacheItem()
	:
	is_active{},
	is_invalid{},
	sound_type{},
	samples_count{},
	decoded_count{},
	buffer_size_{},
	decoder{}
{
}

AudioMixer::Impl::CacheItem::CacheItem(
	const CacheItem& that)
	:
	is_active{that.is_active},
	is_invalid{that.is_invalid},
	sound_type{that.sound_type},
	samples_count{that.samples_count},
	decoded_count{that.decoded_count},
	samples{that.samples}
{
	if (that.decoder)
	{
		decoder.reset(that.decoder->clone());
	}
	else
	{
		decoder = nullptr;
	}
}

AudioMixer::Impl::CacheItem::~CacheItem() = default;

AudioMixer::Impl::CacheItem& AudioMixer::Impl::CacheItem::operator=(
	const CacheItem& that)
{
	if (std::addressof(that) != this)
	{
		is_active = that.is_active;
		is_invalid = that.is_invalid;
		sound_type = that.sound_type;
		samples_count = that.samples_count;
		decoded_count = that.decoded_count;
		samples = that.samples;

		if (that.decoder)
		{
			decoder.reset(that.decoder->clone());
		}
		else
		{
			decoder = nullptr;
		}
	}

	return *this;
}

bool AudioMixer::Impl::CacheItem::is_decoded() const
{
	return decoded_count == samples_count;
}

void AudioMixer::Impl::Positions::initialize()
{
	player = {};

	actors.clear();
	actors.resize(MAXACTORS);

	doors.clear();
	doors.resize(MAXDOORS);

	wall = {};
}

void AudioMixer::Impl::Positions::fixed_copy_to(
	Positions& target)
{
	player = target.player;

	std::copy(actors.cbegin(), actors.cend(), target.actors.begin());
	std::copy(doors.cbegin(), doors.cend(), target.doors.begin());

	wall = target.wall;
}

bool AudioMixer::Impl::Sound::is_audible() const
{
	return left_volume > 0.0F || right_volume > 0.0F;
}

AudioMixer::Impl::Impl()
	:
	is_initialized_{},
	dst_rate_{},
	device_id_{},
	mix_samples_count_{},
	buffer_{},
	mix_buffer_{},
	is_data_available_{},
	sounds_{},
	commands_{},
	mt_commands_{},
	mt_commands_lock_{},
	mute_{},
	adlib_music_cache_{},
	adlib_sfx_cache_{},
	pcm_cache_{},
	mt_positions_{},
	positions_{},
	modified_actors_indices_{},
	modified_doors_indices_{},
	mt_positions_lock_{},
	player_channels_state_{},
	is_music_playing_{},
	is_any_sfx_playing_{},
	sfx_volume_{},
	music_volume_{},
	mix_size_ms_{}
{
	// Initialize atomic fields
	//
	is_data_available_ = false;
	player_channels_state_ = 0;
	is_music_playing_ = false;
	is_any_sfx_playing_ = false;
	sfx_volume_ = 1.0F;
	music_volume_ = 1.0F;
}

AudioMixer::Impl::~Impl()
{
	uninitialize();
}

bool AudioMixer::Impl::initialize(
	const int dst_rate,
	const int mix_size_ms)
{
	uninitialize();

	if (dst_rate == 0)
	{
		dst_rate_ = get_default_rate();
	}
	else
	{
		dst_rate_ = std::max(dst_rate, get_min_rate());
	}

	if (mix_size_ms == 0)
	{
		mix_size_ms_ = get_default_mix_size_ms();
	}
	else
	{
		mix_size_ms_ = std::max(mix_size_ms, get_min_mix_size_ms());
	}

	mix_samples_count_ = calculate_mix_samples_count(dst_rate_, mix_size_ms_);

	auto src_spec = SDL_AudioSpec{};
	src_spec.freq = dst_rate_;
	src_spec.format = AUDIO_S16SYS;
	src_spec.channels = static_cast<std::uint8_t>(get_max_channels());
	src_spec.samples = static_cast<std::uint16_t>(mix_samples_count_);
	src_spec.callback = callback_proxy;
	src_spec.userdata = this;

	auto dst_spec = SDL_AudioSpec{};

	device_id_ = SDL_OpenAudioDevice(
		nullptr,
		0,
		&src_spec,
		&dst_spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	if (device_id_ == 0)
	{
		return false;
	}

	auto is_succeed = true;

	if (is_succeed)
	{
		is_initialized_ = true;
		mix_samples_count_ = dst_spec.samples;

		const auto total_samples = get_max_channels() * mix_samples_count_;

		buffer_.resize(total_samples);
		mix_buffer_.resize(total_samples);

		adlib_music_cache_.resize(LASTMUSIC);
		adlib_sfx_cache_.resize(NUMSOUNDS);
		pcm_cache_.resize(NUMSOUNDS);

		const auto max_commands = get_max_commands();

		commands_.reserve(max_commands);
		mt_commands_.reserve(max_commands);

		mt_positions_.initialize();
		positions_.initialize();
		modified_actors_indices_.reserve(MAXACTORS);
		modified_doors_indices_.reserve(MAXDOORS);

		SDL_PauseAudioDevice(device_id_, 0);
	}
	else
	{
		uninitialize();
	}

	return is_succeed;
}

void AudioMixer::Impl::uninitialize()
{
	is_initialized_ = false;

	if (device_id_ != 0)
	{
		SDL_PauseAudioDevice(device_id_, 1);
		SDL_CloseAudioDevice(device_id_);
		device_id_ = 0;
	}

	dst_rate_ = 0;
	mix_samples_count_ = 0;
	buffer_ = {};
	mix_buffer_ = {};
	is_data_available_ = false;

	sounds_.clear();
	commands_.clear();
	mt_commands_.clear();
	mute_ = false;
	adlib_music_cache_ = {};
	adlib_sfx_cache_ = {};
	pcm_cache_ = {};
	player_channels_state_ = 0;
	is_music_playing_ = false;
	is_any_sfx_playing_ = false;
	mix_size_ms_ = 0;
}

bool AudioMixer::Impl::is_initialized() const
{
	return is_initialized_;
}

int AudioMixer::Impl::get_rate() const
{
	return dst_rate_;
}

int AudioMixer::Impl::get_channel_count() const
{
	return get_max_channels();
}

int AudioMixer::Impl::get_mix_size_ms() const
{
	return mix_size_ms_;
}

float AudioMixer::Impl::get_sfx_volume() const
{
	return sfx_volume_.load(std::memory_order_acquire);
}

float AudioMixer::Impl::get_music_volume() const
{
	return music_volume_.load(std::memory_order_acquire);
}

bool AudioMixer::Impl::play_adlib_music(
	const int music_index,
	const void* const data,
	const int data_size)
{
	return play_sound(SoundType::adlib_music, 0, music_index, data, data_size);
}

bool AudioMixer::Impl::play_adlib_sound(
	const int sound_index,
	const int priority,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	return play_sound(SoundType::adlib_sfx, priority, sound_index, data, data_size, actor_index, actor_type, actor_channel);
}

bool AudioMixer::Impl::play_pcm_sound(
	const int sound_index,
	const int priority,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	return play_sound(SoundType::pcm, priority, sound_index, data, data_size, actor_index, actor_type, actor_channel);
}

bool AudioMixer::Impl::update_positions()
{
	if (!is_initialized())
	{
		return false;
	}

	auto has_modifications = false;

	// Player.
	//
	auto is_player_modified = false;

	{
		auto& the_player = positions_.player;

		is_player_modified =
			the_player.view_x != viewx ||
			the_player.view_y != viewy ||
			the_player.view_cos != viewcos ||
			the_player.view_sin != viewsin;

		if (is_player_modified)
		{
			the_player.view_x = viewx;
			the_player.view_y = viewy;
			the_player.view_cos = viewcos;
			the_player.view_sin = viewsin;

			has_modifications = true;
		}
	}



	// Actors.
	//
	modified_actors_indices_.clear();

	for (int i = 0; i < MAXACTORS; ++i)
	{
		auto& actor = positions_.actors[i];

		const auto is_actor_modified =
			actor.x != objlist[i].x ||
			actor.y != objlist[i].y;

		if (is_actor_modified)
		{
			actor.x = objlist[i].x;
			actor.y = objlist[i].y;

			has_modifications = true;
			modified_actors_indices_.emplace_back(i);
		}
	}

	// Doors.
	//
	modified_doors_indices_.clear();

	for (int i = 0; i < MAXDOORS; ++i)
	{
		auto& door = positions_.doors[i];

		const auto x = (doorobjlist[i].tilex << TILESHIFT) + (1 << (TILESHIFT - 1));
		const auto y = (doorobjlist[i].tiley << TILESHIFT) + (1 << (TILESHIFT - 1));

		const auto is_door_modified =
			door.x != x ||
			door.y != y;

		if (is_door_modified)
		{
			door.x = x;
			door.y = y;

			has_modifications = true;
			modified_doors_indices_.emplace_back(i);
		}
	}

	// Push-wall.
	//
	auto is_push_wall_modified = false;

	{
		auto x = (pwallx << TILESHIFT) + (1 << (TILESHIFT - 1));
		auto y = (pwally << TILESHIFT) + (1 << (TILESHIFT - 1));

		const auto wall_offset = (65535 * pwallpos) / 63;

		switch (pwalldir)
		{
		case di_east:
			x += wall_offset;
			break;

		case di_north:
			y -= wall_offset;
			break;

		case di_south:
			y += wall_offset;
			break;

		case di_west:
			x -= wall_offset;
			break;

		default:
			break;
		}

		is_push_wall_modified =
			positions_.wall.x != x ||
			positions_.wall.y != y;

		if (is_push_wall_modified)
		{
			positions_.wall.x = x;
			positions_.wall.y = y;

			has_modifications = true;
		}
	}

	// Commit changes.
	//
	if (!has_modifications)
	{
		return true;
	}

	MtLockGuard guard_lock{mt_positions_lock_};

	if (is_player_modified)
	{
		mt_positions_.player = positions_.player;
	}

	if (!modified_actors_indices_.empty())
	{
		for (const auto& actor_index : modified_actors_indices_)
		{
			mt_positions_.actors[actor_index] = positions_.actors[actor_index];
		}
	}

	if (!modified_doors_indices_.empty())
	{
		for (const auto& door_index : modified_doors_indices_)
		{
			mt_positions_.doors[door_index] = positions_.doors[door_index];
		}
	}

	if (is_push_wall_modified)
	{
		mt_positions_.wall = positions_.wall;
	}

	return true;
}

bool AudioMixer::Impl::stop_music()
{
	if (!is_initialized())
	{
		return false;
	}

	auto command = Command{};
	command.command = CommandType::stop_music;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

bool AudioMixer::Impl::stop_all_sfx()
{
	if (!is_initialized())
	{
		return false;
	}

	auto command = Command{};
	command.command = CommandType::stop_all_sfx;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

bool AudioMixer::Impl::set_mute(
	const bool value)
{
	if (!is_initialized())
	{
		return false;
	}

	mute_ = value;

	return true;
}

bool AudioMixer::Impl::set_sfx_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	sfx_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool AudioMixer::Impl::set_music_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	music_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool AudioMixer::Impl::is_music_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_music_playing_;
}

bool AudioMixer::Impl::is_any_sfx_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_any_sfx_playing_;
}

bool AudioMixer::Impl::is_player_channel_playing(
	const ActorChannel channel) const
{
	return (player_channels_state_ & (1 << static_cast<int>(channel))) != 0;
}

int AudioMixer::Impl::get_min_rate()
{
	return 11025;
}

int AudioMixer::Impl::get_default_rate()
{
	return 44100;
}

int AudioMixer::Impl::get_min_mix_size_ms()
{
	return 20;
}

int AudioMixer::Impl::get_default_mix_size_ms()
{
	return 40;
}

int AudioMixer::Impl::get_max_channels()
{
	return 2;
}

int AudioMixer::Impl::get_max_commands()
{
	return 128;
}

void AudioMixer::Impl::callback(
	std::uint8_t* dst_data,
	const int dst_length)
{
	if (!mute_ && is_data_available_)
	{
		std::uninitialized_copy_n(reinterpret_cast<const std::uint8_t*>(buffer_.data()), dst_length, dst_data);
	}
	else
	{
		std::uninitialized_fill_n(dst_data, dst_length, std::uint8_t{});
	}

	is_data_available_ = false;
}

void AudioMixer::Impl::mix()
{
	handle_commands();

	if (!is_data_available_ && !sounds_.empty())
	{
		mix_samples();
		is_data_available_ = true;
	}
}

void AudioMixer::Impl::mix_samples()
{
	if (sounds_.empty())
	{
		is_any_sfx_playing_ = false;
		std::uninitialized_fill(buffer_.begin(), buffer_.end(), std::int16_t{});
		return;
	}

	spatialize_sounds();

	std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), 0);

	auto sfx_volume = static_cast<float>(sfx_volume_);
	auto music_volume = static_cast<float>(music_volume_);

	auto min_left_sample = 32767;
	auto max_left_sample = -32768;

	auto min_right_sample = 32767;
	auto max_right_sample = -32768;

	auto sound_it = sounds_.begin();
	auto sound_end_it = sounds_.end();

	while (sound_it != sound_end_it)
	{
		if (!decode_sound(*sound_it))
		{
			set_player_channel_state(*sound_it, false);
			sound_it = sounds_.erase(sound_it);
			continue;
		}

		const auto is_adlib_music = (sound_it->type == SoundType::adlib_music);

		auto cache_item = sound_it->cache;

		if (!is_adlib_music && sound_it->decode_offset == cache_item->decoded_count)
		{
			++sound_it;
			continue;
		}

		if (!sound_it->is_audible())
		{
			++sound_it;
			continue;
		}

		auto volume_scale = 1.0F;

		switch (sound_it->type)
		{
		case SoundType::adlib_music:
			volume_scale = 8.0F * music_volume;
			break;

		case SoundType::adlib_sfx:
			volume_scale = 8.0F * sfx_volume;
			break;

		default:
			volume_scale = sfx_volume;
			break;
		}

		auto decode_count = 0;

		if (is_adlib_music)
		{
			decode_count = cache_item->buffer_size_;
		}
		else
		{
			const auto remain_count = cache_item->decoded_count - sound_it->decode_offset;
			decode_count = std::min(remain_count, mix_samples_count_);
		}

		const auto base_offset = (is_adlib_music ? 0 : sound_it->decode_offset);

		for (int i = 0; i < decode_count; ++i)
		{
			const auto sample = volume_scale * cache_item->samples[base_offset + i];

			// Left channel.
			//
			const auto left_sample = static_cast<int>(sound_it->left_volume * sample);

			mix_buffer_[(2 * i) + 0] += left_sample;

			min_left_sample = std::min(left_sample, min_left_sample);
			max_left_sample = std::max(left_sample, max_left_sample);

			// Right channel.
			//
			const auto right_sample = static_cast<int>(sound_it->right_volume * sample);

			mix_buffer_[(2 * i) + 1] += right_sample;

			min_right_sample = std::min(right_sample, min_right_sample);
			max_right_sample = std::max(right_sample, max_right_sample);
		}

		if (!is_adlib_music)
		{
			sound_it->decode_offset += decode_count;
		}

		if ((is_adlib_music && cache_item->is_decoded()) ||
			(!is_adlib_music && sound_it->decode_offset == cache_item->decoded_count))
		{
			if (cache_item->is_decoded())
			{
				auto is_erase = false;

				if (sound_it->type == SoundType::adlib_music)
				{
					sd_sq_played_once_ = true;
#if 0
					sound_it->decode_offset = 0;
#endif
					if (cache_item->decoder->reset())
					{
						cache_item->decoded_count = 0;
						cache_item->buffer_size_ = 0;
					}
					else
					{
						is_erase = true;
					}
				}
				else
				{
					is_erase = true;
				}

				if (is_erase)
				{
					set_player_channel_state(*sound_it, false);
					sound_it = sounds_.erase(sound_it);
					continue;
				}
			}
		}

		++sound_it;
	}


	//
	// Calculate normalizations factors.
	//

	// Left channel.
	//
	auto normalize_left = false;
	auto normalize_left_scale = 1.0F;

	if (min_left_sample < -32768 && -min_left_sample > max_left_sample)
	{
		normalize_left = true;
		normalize_left_scale = -32768.0F / min_left_sample;
	}
	else if (max_left_sample > 32767 && max_left_sample >= -min_left_sample)
	{
		normalize_left = true;
		normalize_left_scale = 32767.0F / max_left_sample;
	}

	// Right channel.
	//
	auto normalize_right = false;
	auto normalize_right_scale = 1.0F;

	if (min_right_sample < -32768 && -min_right_sample > max_right_sample)
	{
		normalize_right = true;
		normalize_right_scale = -32768.0F / min_right_sample;
	}
	else if (max_right_sample > 32767 && max_right_sample >= -min_right_sample)
	{
		normalize_right = true;
		normalize_right_scale = 32767.0F / max_right_sample;
	}


	//
	// Normalize and output.
	//
	for (int i = 0; i < mix_samples_count_; ++i)
	{
		// Left channel.
		//
		auto left_sample = mix_buffer_[(2 * i) + 0];

		if (normalize_left)
		{
			left_sample = static_cast<int>(normalize_left_scale * left_sample);
		}

		left_sample = std::min(left_sample, 32767);
		left_sample = std::max(left_sample, -32768);

		buffer_[(2 * i) + 0] = static_cast<std::int16_t>(left_sample);


		// Right channel.
		//
		auto right_sample = mix_buffer_[(2 * i) + 1];

		if (normalize_right)
		{
			right_sample = static_cast<int>(normalize_right_scale * right_sample);
		}

		right_sample = std::min(right_sample, 32767);
		right_sample = std::max(right_sample, -32768);

		buffer_[(2 * i) + 1] = static_cast<std::int16_t>(right_sample);
	}

	const auto music_count = (is_music_playing() ? 1 : 0);

	is_any_sfx_playing_ = ((sounds_.size() - music_count) > 0);
}

void AudioMixer::Impl::handle_commands()
{
	{
		MtLockGuard guard_lock{mt_commands_lock_};

		if (!mt_commands_.empty())
		{
			if (commands_.empty())
			{
				commands_ = mt_commands_;
			}
			else
			{
				commands_.insert(commands_.cend(), mt_commands_.cbegin(), mt_commands_.cend());
			}

			mt_commands_.clear();
		}
	}

	if (commands_.empty())
	{
		return;
	}

	for (const auto& command : commands_)
	{
		switch (command.command)
		{
		case CommandType::play:
			handle_play_command(command);
			break;

		case CommandType::stop_music:
			handle_stop_music_command();
			break;

		default:
			break;
		}
	}

	commands_.clear();
}

void AudioMixer::Impl::handle_play_command(
	const Command& command)
{
	auto cache_item = command.sound.cache;

	if (!cache_item)
	{
		return;
	}

	if (!initialize_cache_item(command, *cache_item))
	{
		return;
	}

	const auto is_adlib_music = (command.sound.type == SoundType::adlib_music);

	if (!is_adlib_music && command.sound.actor_index >= 0)
	{
		// Search existing sound which can override a
		// new one because of priority.

		for (const auto& sound : sounds_)
		{
			if (sound.priority > command.sound.priority &&
				sound.actor_index == command.sound.actor_index &&
				sound.actor_type == command.sound.actor_type &&
				sound.actor_channel == command.sound.actor_channel)
			{
				return;
			}
		}

		// Remove sounds which will be overwritten.

		for (auto i = sounds_.begin(); i != sounds_.end(); )
		{
			if (i->actor_index == command.sound.actor_index &&
				i->actor_type == command.sound.actor_type &&
				i->actor_channel == command.sound.actor_channel)
			{
				set_player_channel_state(*i, false);
				i = sounds_.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	if (is_adlib_music)
	{
		is_music_playing_ = true;
	}
	else
	{
		is_any_sfx_playing_ = true;
	}

	auto sound = command.sound;
	sound.decode_offset = 0;
	sounds_.push_back(sound);

	set_player_channel_state(sound, true);
}

void AudioMixer::Impl::handle_stop_music_command()
{
	is_music_playing_ = false;

	for (auto i = sounds_.begin(); i != sounds_.end(); )
	{
		if (i->type != SoundType::adlib_music)
		{
			++i;
		}
		else
		{
			*(i->cache) = CacheItem{};
			i = sounds_.erase(i);
		}
	}
}

void AudioMixer::Impl::handle_stop_all_sfx_command()
{
	is_any_sfx_playing_ = false;

	sounds_.remove_if(
		[](const Sound& sound)
		{
			return sound.type != SoundType::adlib_music;
		}
	);
}

bool AudioMixer::Impl::initialize_cache_item(
	const Command& command,
	CacheItem& cache_item)
{
	if (cache_item.is_active)
	{
		return !cache_item.is_invalid;
	}

	cache_item = CacheItem{};

	bool is_succeed = true;
	auto decoder = std::unique_ptr<AudioDecoder>{};

	if (is_succeed)
	{
		decoder.reset(create_decoder_by_sound_type(command.sound.type));

		is_succeed = (decoder != nullptr);
	}

	if (is_succeed)
	{
		is_succeed = decoder->initialize(command.data, command.data_size, dst_rate_);
	}

	auto samples_count = 0;

	if (is_succeed)
	{
		samples_count = decoder->get_dst_length_in_samples();
		is_succeed = (samples_count > 0);
	}

	cache_item.is_active = true;

	if (is_succeed)
	{
		const auto is_adlib_music = (command.sound.type == SoundType::adlib_music);

		cache_item.sound_type = command.sound.type;
		cache_item.samples_count = samples_count;
		cache_item.samples.resize(is_adlib_music ? mix_samples_count_ : samples_count);
		cache_item.buffer_size_ = 0;
		cache_item.decoder.swap(decoder);
	}
	else
	{
		cache_item.is_invalid = true;
	}

	return is_succeed;
}

bool AudioMixer::Impl::decode_sound(
	const Sound& sound)
{
	auto cache_item = sound.cache;

	if (!cache_item)
	{
		return false;
	}

	if (!cache_item->is_active)
	{
		return false;
	}

	if (cache_item->is_invalid)
	{
		return false;
	}

	if (cache_item->is_decoded())
	{
		return true;
	}

	if (sound.type == SoundType::adlib_music)
	{
		const auto total_remain_count = cache_item->samples_count - cache_item->decoded_count;

		if (total_remain_count == 0)
		{
			return true;
		}

		auto remain_count = std::min(total_remain_count, cache_item->buffer_size_);

		if (remain_count == 0)
		{
			remain_count = std::min(total_remain_count, mix_samples_count_);
		}

		cache_item->buffer_size_ = cache_item->decoder->decode(
			remain_count,
			cache_item->samples.data());

		cache_item->decoded_count += cache_item->buffer_size_;

		return true;
	}

	const auto ahead_count = std::min(
		sound.decode_offset + mix_samples_count_,
		cache_item->samples_count
	);

	if (ahead_count <= cache_item->decoded_count)
	{
		return true;
	}

	const auto planned_count = std::min(
		cache_item->samples_count - cache_item->decoded_count,
		mix_samples_count_);

	const auto actual_count = cache_item->decoder->decode(
		planned_count,
		&cache_item->samples[cache_item->decoded_count]);

	cache_item->decoded_count += actual_count;

	return true;
}

void AudioMixer::Impl::spatialize_sound(
	Sound& sound)
{
	sound.left_volume = 1.0F;
	sound.right_volume = 1.0F;

	if (sound.type == SoundType::adlib_music)
	{
		return;
	}

	if (sound.actor_index <= 0)
	{
		return;
	}

	Location* location = nullptr;

	switch (sound.actor_type)
	{
	case ActorType::actor:
		location = &mt_positions_.actors[sound.actor_index];
		break;

	case ActorType::door:
		location = &mt_positions_.doors[sound.actor_index];
		break;

	case ActorType::wall:
		location = &mt_positions_.wall;
		break;

	default:
		return;
	}

	auto gx = location->x;
	auto gy = location->y;

	//
	// translate point to view centered coordinates
	//
	gx -= mt_positions_.player.view_x;
	gy -= mt_positions_.player.view_y;

	//
	// calculate newx
	//
	auto xt = FixedByFrac(gx, mt_positions_.player.view_cos);
	auto yt = FixedByFrac(gy, mt_positions_.player.view_sin);
	auto x = (xt - yt) >> TILESHIFT;

	//
	// calculate newy
	//
	xt = FixedByFrac(gx, mt_positions_.player.view_sin);
	yt = FixedByFrac(gy, mt_positions_.player.view_cos);

	auto y = (yt + xt) >> TILESHIFT;

	if (y <= -ATABLEMAX)
	{
		y = -ATABLEMAX;
	}

	if (y >= ATABLEMAX)
	{
		y = ATABLEMAX - 1;
	}

	if (x < 0)
	{
		x = -x;
	}

	if (x >= ATABLEMAX)
	{
		x = ATABLEMAX - 1;
	}

	const auto left = 9 - lefttable[x][y + ATABLEMAX];
	const auto right = 9 - righttable[x][y + ATABLEMAX];

	sound.left_volume = left / 9.0F;
	sound.right_volume = right / 9.0F;
}

void AudioMixer::Impl::spatialize_sounds()
{
	if (sounds_.empty())
	{
		return;
	}

	MtLockGuard guard_lock{mt_positions_lock_};

	for (auto& sound : sounds_)
	{
		spatialize_sound(sound);
	}
}

bool AudioMixer::Impl::play_sound(
	const SoundType sound_type,
	const int priority,
	const int sound_index,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	if (!is_initialized())
	{
		return false;
	}

	if (!is_sound_type_valid(sound_type))
	{
		return false;
	}

	if (priority < 0)
	{
		return false;
	}

	if (!data)
	{
		return false;
	}

	if (data_size <= 0)
	{
		return false;
	}

	if (actor_index >= MAXACTORS)
	{
		return false;
	}

	switch (actor_channel)
	{
	case ActorChannel::voice:
	case ActorChannel::weapon:
	case ActorChannel::item:
	case ActorChannel::hit_wall:
	case ActorChannel::no_way:
	case ActorChannel::interrogation:
		break;

	default:
		throw std::runtime_error("Invalid actor channel.");
	}

	auto command = Command{};
	command.command = CommandType::play;
	command.sound.type = sound_type;
	command.sound.priority = priority;
	command.sound.cache = get_cache_item(sound_type, sound_index);
	command.sound.actor_index = actor_index;
	command.sound.actor_type = actor_type;
	command.sound.actor_channel = actor_channel;
	command.data = data;
	command.data_size = data_size;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

void AudioMixer::Impl::lock()
{
	SDL_LockAudioDevice(device_id_);
}

void AudioMixer::Impl::unlock()
{
	SDL_UnlockAudioDevice(device_id_);
}

void AudioMixer::Impl::callback_proxy(
	void* user_data,
	std::uint8_t* dst_data,
	const int dst_length)
{
	assert(user_data);

	auto mixer = static_cast<AudioMixer::Impl*>(user_data);

	mixer->mix();

	mixer->callback(dst_data, dst_length);
}

int AudioMixer::Impl::mix_proxy(
	void* user_data)
{
	assert(user_data);

	auto mixer = static_cast<AudioMixer::Impl*>(user_data);
	mixer->mix();

	return 0;
}

int AudioMixer::Impl::calculate_mix_samples_count(
	const int dst_rate,
	const int mix_size_ms)
{
	const auto exact_count = (dst_rate * mix_size_ms) / 1000;

	auto actual_count = 1;

	while (actual_count < exact_count)
	{
		actual_count *= 2;
	}

	if (actual_count > 65536)
	{
		actual_count = 65536;
	}

	return actual_count;
}

AudioMixer::Impl::CacheItem* AudioMixer::Impl::get_cache_item(
	const SoundType sound_type,
	const int sound_index)
{
	if (!is_sound_index_valid(sound_index, sound_type))
	{
		return nullptr;
	}

	switch (sound_type)
	{
	case SoundType::adlib_music:
		return &adlib_music_cache_[sound_index];

	case SoundType::adlib_sfx:
		return &adlib_sfx_cache_[sound_index];

	case SoundType::pcm:
		return &pcm_cache_[sound_index];

	default:
		return nullptr;
	}
}

void AudioMixer::Impl::set_player_channel_state(
	const Sound& sound,
	const bool state)
{
	if (sound.type == SoundType::adlib_music)
	{
		return;
	}

	if (sound.actor_type != ActorType::actor)
	{
		return;
	}

	if (sound.actor_index > 0)
	{
		return;
	}

	const auto mask = 1 << static_cast<int>(sound.actor_channel);

	if (state)
	{
		player_channels_state_ |= mask;
	}
	else
	{
		player_channels_state_ &= ~mask;
	}
}

AudioDecoder* AudioMixer::Impl::create_decoder_by_sound_type(
	const SoundType sound_type)
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
		return new AdlibMusicDecoder();

	case SoundType::adlib_sfx:
		return new AdlibSfxDecoder();

	case SoundType::pcm:
		return new PcmDecoder();

	default:
		return nullptr;
	}
}

bool AudioMixer::Impl::is_sound_type_valid(
	const SoundType sound_type)
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
	case SoundType::adlib_sfx:
	case SoundType::pcm:
		return true;

	default:
		return false;
	}
}

bool AudioMixer::Impl::is_sound_index_valid(
	const int sound_index,
	const SoundType sound_type)
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
		return sound_index >= 0 && sound_index < LASTMUSIC;

	case SoundType::adlib_sfx:
	case SoundType::pcm:
		return sound_index >= 0 && sound_index < NUMSOUNDS;

	default:
		return false;
	}
}


///


AudioMixer::AudioMixer()
	:
	impl_{new Impl{}}
{
}

AudioMixer::AudioMixer(
	AudioMixer&& rhs)
	:
	impl_{std::move(rhs.impl_)}
{
}

AudioMixer::~AudioMixer()
{
}

bool AudioMixer::initialize(
	const int dst_rate,
	const int mix_size_ms)
{
	return impl_->initialize(dst_rate, mix_size_ms);
}

void AudioMixer::uninitialize()
{
	impl_->uninitialize();
}

bool AudioMixer::is_initialized() const
{
	return impl_->is_initialized();
}

int AudioMixer::get_rate() const
{
	return impl_->get_rate();
}

int AudioMixer::get_channel_count() const
{
	return impl_->get_channel_count();
}

int AudioMixer::get_mix_size_ms() const
{
	return impl_->get_mix_size_ms();
}

float AudioMixer::get_sfx_volume() const
{
	return impl_->get_sfx_volume();
}

float AudioMixer::get_music_volume() const
{
	return impl_->get_music_volume();
}

bool AudioMixer::play_adlib_music(
	const int music_index,
	const void* const data,
	const int data_size)
{
	return impl_->play_adlib_music(music_index, data, data_size);
}

bool AudioMixer::play_adlib_sound(
	const int sound_index,
	const int priority,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	return impl_->play_adlib_sound(
		sound_index,
		priority,
		data,
		data_size,
		actor_index,
		actor_type,
		actor_channel);
}

bool AudioMixer::play_pcm_sound(
	const int sound_index,
	const int priority,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	return impl_->play_pcm_sound(
		sound_index,
		priority,
		data,
		data_size,
		actor_index,
		actor_type,
		actor_channel);
}

bool AudioMixer::update_positions()
{
	return impl_->update_positions();
}

bool AudioMixer::stop_music()
{
	return impl_->stop_music();
}

bool AudioMixer::stop_all_sfx()
{
	return impl_->stop_all_sfx();
}

bool AudioMixer::set_mute(
	const bool value)
{
	return impl_->set_mute(value);
}

bool AudioMixer::set_sfx_volume(
	const float volume)
{
	return impl_->set_sfx_volume(volume);
}

bool AudioMixer::set_music_volume(
	const float volume)
{
	return impl_->set_music_volume(volume);
}

bool AudioMixer::is_music_playing() const
{
	return impl_->is_music_playing();
}

bool AudioMixer::is_any_sfx_playing() const
{
	return impl_->is_any_sfx_playing();
}

bool AudioMixer::is_player_channel_playing(
	const ActorChannel channel) const
{
	return impl_->is_player_channel_playing(channel);
}

int AudioMixer::get_min_rate()
{
	return Impl::get_min_rate();
}

int AudioMixer::get_default_rate()
{
	return Impl::get_default_rate();
}

int AudioMixer::get_min_mix_size_ms()
{
	return Impl::get_min_mix_size_ms();
}

int AudioMixer::get_default_mix_size_ms()
{
	return Impl::get_default_mix_size_ms();
}

int AudioMixer::get_max_channels()
{
	return Impl::get_max_channels();
}

int AudioMixer::get_max_commands()
{
	return Impl::get_max_commands();
}


} // bstone
