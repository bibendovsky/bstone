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

#include <cassert>
#include <cmath>

#include <algorithm>
#include <atomic>
#include <deque>
#include <list>
#include <mutex>
#include <utility>
#include <vector>

#include "SDL_audio.h"

#include "3d_def.h"
#include "audio.h"

#include "bstone_audio_decoder.h"


const int ATABLEMAX = 15;

extern bool sd_sq_played_once_;
extern std::uint8_t lefttable[ATABLEMAX][ATABLEMAX * 2];
extern std::uint8_t righttable[ATABLEMAX][ATABLEMAX * 2];


namespace bstone
{


class AudioMixerImpl final :
	public AudioMixer
{
public:
	AudioMixerImpl();

	~AudioMixerImpl() override;


	// Note: Mix size in milliseconds.
	bool initialize(
		const Opl3Type opl3_type,
		const int dst_rate,
		const int mix_size_ms) override;

	void uninitialize() override;

	bool is_initialized() const override;

	Opl3Type get_opl3_type() const override;

	int get_rate() const override;

	int get_channel_count() const override;

	int get_mix_size_ms() const override;

	float get_sfx_volume() const override;

	float get_music_volume() const override;

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
	bool play_pcm_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) override;

	bool update_positions() override;

	bool stop_music() override;

	bool stop_all_sfx() override;

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

	bool is_any_sfx_playing() const override;

	bool is_player_channel_playing(
		const ActorChannel channel) const override;

	int get_min_rate() const override;

	int get_default_rate() const override;

	int get_min_mix_size_ms() const override;

	int get_default_mix_size_ms() const override;

	int get_max_channels() const override;

	int get_max_commands() const override;


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
		AudioDecoderUPtr decoder;


		CacheItem();

		bool is_decoded() const;
	}; // CacheItem

	using Cache = std::deque<CacheItem>;

	struct Location
	{
		int x;
		int y;
	}; // Location

	using Locations = std::vector<Location>;

	struct PlayerLocation
	{
		int view_x;
		int view_y;
		int view_cos;
		int view_sin;
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
		stop_all_sfx
	}; // CommandType

	struct Command
	{
		CommandType command;
		Sound sound;
		const void* data;
		int data_size;
	}; // Command

	using Commands = std::vector<Command>;


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

	AudioDecoderUPtr create_decoder_by_sound_type(
		const SoundType sound_type) const;

	static bool is_sound_type_valid(
		const SoundType sound_type);

	static bool is_sound_index_valid(
		const int sound_index,
		const SoundType sound_type);
}; // AudioMixer


AudioMixerImpl::CacheItem::CacheItem()
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

bool AudioMixerImpl::CacheItem::is_decoded() const
{
	return decoded_count == samples_count;
}

void AudioMixerImpl::Positions::initialize()
{
	player = {};

	actors.clear();
	actors.resize(MAXACTORS);

	doors.clear();
	doors.resize(MAXDOORS);

	wall = {};
}

void AudioMixerImpl::Positions::fixed_copy_to(
	Positions& target)
{
	player = target.player;

	std::copy(actors.cbegin(), actors.cend(), target.actors.begin());
	std::copy(doors.cbegin(), doors.cend(), target.doors.begin());

	wall = target.wall;
}

bool AudioMixerImpl::Sound::is_audible() const
{
	return left_volume > 0.0F || right_volume > 0.0F;
}

AudioMixerImpl::AudioMixerImpl()
	:
	is_initialized_{},
	opl3_type_{},
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
	mt_is_muted_{},
	mt_is_sfx_paused_{},
	mt_is_music_paused_{},
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

AudioMixerImpl::~AudioMixerImpl()
{
	uninitialize();
}

bool AudioMixerImpl::initialize(
	const Opl3Type opl3_type,
	const int dst_rate,
	const int mix_size_ms)
{
	uninitialize();

	switch (opl3_type)
	{
		case Opl3Type::dbopl:
			break;

		default:
			return false;
	}

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

		opl3_type_ = opl3_type;

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

void AudioMixerImpl::uninitialize()
{
	is_initialized_ = false;

	opl3_type_ = {};

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
	mt_is_muted_.store(false, std::memory_order_release);
	mt_is_sfx_paused_.store(false, std::memory_order_release);
	mt_is_music_paused_.store(false, std::memory_order_release);
	adlib_music_cache_.clear();
	adlib_sfx_cache_.clear();
	pcm_cache_.clear();
	player_channels_state_ = 0;
	is_music_playing_ = false;
	is_any_sfx_playing_ = false;
	mix_size_ms_ = 0;
}

bool AudioMixerImpl::is_initialized() const
{
	return is_initialized_;
}

Opl3Type AudioMixerImpl::get_opl3_type() const
{
	return opl3_type_;
}

int AudioMixerImpl::get_rate() const
{
	return dst_rate_;
}

int AudioMixerImpl::get_channel_count() const
{
	return get_max_channels();
}

int AudioMixerImpl::get_mix_size_ms() const
{
	return mix_size_ms_;
}

float AudioMixerImpl::get_sfx_volume() const
{
	return sfx_volume_.load(std::memory_order_acquire);
}

float AudioMixerImpl::get_music_volume() const
{
	return music_volume_.load(std::memory_order_acquire);
}

bool AudioMixerImpl::play_adlib_music(
	const int music_index,
	const void* const data,
	const int data_size)
{
	return play_sound(SoundType::adlib_music, 0, music_index, data, data_size);
}

bool AudioMixerImpl::play_adlib_sound(
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

bool AudioMixerImpl::play_pcm_sound(
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

bool AudioMixerImpl::update_positions()
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

bool AudioMixerImpl::stop_music()
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

bool AudioMixerImpl::stop_all_sfx()
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

bool AudioMixerImpl::pause_all_sfx(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_sfx_paused_.store(is_paused, std::memory_order_release);

	return true;
}

bool AudioMixerImpl::pause_music(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_music_paused_.store(is_paused, std::memory_order_release);

	return true;
}

bool AudioMixerImpl::set_mute(
	const bool is_mute)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_muted_.store(is_mute, std::memory_order_release);

	return true;
}

bool AudioMixerImpl::set_sfx_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	sfx_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool AudioMixerImpl::set_music_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	music_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool AudioMixerImpl::is_music_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_music_playing_;
}

bool AudioMixerImpl::is_any_sfx_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_any_sfx_playing_;
}

bool AudioMixerImpl::is_player_channel_playing(
	const ActorChannel channel) const
{
	return (player_channels_state_ & (1 << static_cast<int>(channel))) != 0;
}

int AudioMixerImpl::get_min_rate() const
{
	return 11'025;
}

int AudioMixerImpl::get_default_rate() const
{
	return 44'100;
}

int AudioMixerImpl::get_min_mix_size_ms() const
{
	return 20;
}

int AudioMixerImpl::get_default_mix_size_ms() const
{
	return 40;
}

int AudioMixerImpl::get_max_channels() const
{
	return 2;
}

int AudioMixerImpl::get_max_commands() const
{
	return 128;
}

void AudioMixerImpl::callback(
	std::uint8_t* dst_data,
	const int dst_length)
{
	if (!mt_is_muted_.load(std::memory_order_acquire) && is_data_available_)
	{
		std::uninitialized_copy_n(reinterpret_cast<const std::uint8_t*>(buffer_.data()), dst_length, dst_data);
	}
	else
	{
		std::uninitialized_fill_n(dst_data, dst_length, std::uint8_t{});
	}

	is_data_available_ = false;
}

void AudioMixerImpl::mix()
{
	handle_commands();

	if (!is_data_available_ && !sounds_.empty())
	{
		mix_samples();
		is_data_available_ = true;
	}
}

void AudioMixerImpl::mix_samples()
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

	auto sound_it = sounds_.begin();
	auto sound_end_it = sounds_.end();

	constexpr auto sfx_volume_scale = 7.0F;
	constexpr auto music_volume_scale = 6.0F;

	const auto is_sfx_paused = mt_is_sfx_paused_.load(std::memory_order_acquire);
	const auto is_music_paused = mt_is_music_paused_.load(std::memory_order_acquire);

	while (sound_it != sound_end_it)
	{
		if (is_sfx_paused &&
			(sound_it->type == SoundType::adlib_sfx || sound_it->type == SoundType::pcm) &&
			sound_it->actor_channel != ActorChannel::unpausable)
		{
			++sound_it;
			continue;
		}

		if (is_music_paused &&
			sound_it->type == SoundType::adlib_music)
		{
			++sound_it;
			continue;
		}

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
			volume_scale = music_volume_scale * music_volume;
			break;

		case SoundType::adlib_sfx:
			volume_scale = sfx_volume_scale * sfx_volume;
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

			// Right channel.
			//
			const auto right_sample = static_cast<int>(sound_it->right_volume * sample);

			mix_buffer_[(2 * i) + 1] += right_sample;
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

					if (cache_item->decoder->rewind())
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


	const auto max_mix_sample_it = std::max_element(
		mix_buffer_.cbegin(),
		mix_buffer_.cend(),
		[](const auto lhs, const auto rhs)
		{
			return std::abs(lhs) < std::abs(rhs);
		}
	);

	if (max_mix_sample_it != mix_buffer_.cend())
	{
		constexpr auto max_mix_sample_value = 32'760;

		const auto max_mix_sample = std::abs(*max_mix_sample_it);

		if (max_mix_sample <= max_mix_sample_value)
		{
			std::uninitialized_copy(
				mix_buffer_.cbegin(),
				mix_buffer_.cend(),
				buffer_.begin()
			);
		}
		else
		{
			const auto scalar = 32'768.0F / max_mix_sample;

			std::transform(
				mix_buffer_.cbegin(),
				mix_buffer_.cend(),
				buffer_.begin(),
				[scalar](const auto item)
				{
					return static_cast<Sample>(item * scalar);
				}
			);
		}
	}

	const auto music_count = (is_music_playing() ? 1 : 0);

	is_any_sfx_playing_ = ((sounds_.size() - music_count) > 0);
}

void AudioMixerImpl::handle_commands()
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

void AudioMixerImpl::handle_play_command(
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

void AudioMixerImpl::handle_stop_music_command()
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

void AudioMixerImpl::handle_stop_all_sfx_command()
{
	is_any_sfx_playing_ = false;

	sounds_.remove_if(
		[](const Sound& sound)
		{
			return sound.type != SoundType::adlib_music;
		}
	);
}

bool AudioMixerImpl::initialize_cache_item(
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
		decoder = create_decoder_by_sound_type(command.sound.type);

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

bool AudioMixerImpl::decode_sound(
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

void AudioMixerImpl::spatialize_sound(
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

	if (sound.actor_channel != ActorChannel::unpausable &&
		mt_is_sfx_paused_.load(std::memory_order_acquire))
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

void AudioMixerImpl::spatialize_sounds()
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

bool AudioMixerImpl::play_sound(
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
	case ActorChannel::unpausable:
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

void AudioMixerImpl::lock()
{
	SDL_LockAudioDevice(device_id_);
}

void AudioMixerImpl::unlock()
{
	SDL_UnlockAudioDevice(device_id_);
}

void AudioMixerImpl::callback_proxy(
	void* user_data,
	std::uint8_t* dst_data,
	const int dst_length)
{
	assert(user_data);

	auto mixer = static_cast<AudioMixerImpl*>(user_data);

	mixer->mix();
	mixer->callback(dst_data, dst_length);
}

int AudioMixerImpl::mix_proxy(
	void* user_data)
{
	assert(user_data);

	auto mixer = static_cast<AudioMixerImpl*>(user_data);
	mixer->mix();

	return 0;
}

int AudioMixerImpl::calculate_mix_samples_count(
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

AudioMixerImpl::CacheItem* AudioMixerImpl::get_cache_item(
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

void AudioMixerImpl::set_player_channel_state(
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

AudioDecoderUPtr AudioMixerImpl::create_decoder_by_sound_type(
	const SoundType sound_type) const
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
		return make_audio_decoder(AudioDecoderType::adlib_music, opl3_type_);

	case SoundType::adlib_sfx:
		return make_audio_decoder(AudioDecoderType::adlib_sfx, opl3_type_);

	case SoundType::pcm:
		return make_audio_decoder(AudioDecoderType::pcm, opl3_type_);

	default:
		return nullptr;
	}
}

bool AudioMixerImpl::is_sound_type_valid(
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

bool AudioMixerImpl::is_sound_index_valid(
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


AudioMixerUPtr make_audio_mixer()
{
	return std::make_unique<AudioMixerImpl>();
}


} // bstone
