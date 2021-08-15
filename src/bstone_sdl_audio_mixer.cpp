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


#include "bstone_sdl_audio_mixer.h"

#include <cassert>
#include <cmath>

#include <algorithm>
#include <mutex>

#include "SDL_audio.h"

#include "3d_def.h"
#include "audio.h"

#include "bstone_audio_decoder.h"
#include "bstone_audio_sample_converter.h"


constexpr auto ATABLEMAX = 15;

extern bool sd_sq_played_once_;

const std::uint8_t righttable[ATABLEMAX][ATABLEMAX * 2] =
{
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

const std::uint8_t lefttable[ATABLEMAX][ATABLEMAX * 2] =
{
	{8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};


namespace bstone
{


SdlAudioMixer::CacheItem::CacheItem()
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

bool SdlAudioMixer::CacheItem::is_decoded() const
{
	return decoded_count == samples_count;
}

void SdlAudioMixer::Positions::initialize()
{
	player = {};

	actors.clear();
	actors.resize(MAXACTORS);

	doors.clear();
	doors.resize(MAXDOORS);

	wall = {};
}

void SdlAudioMixer::Positions::fixed_copy_to(
	Positions& target)
{
	player = target.player;

	std::copy(actors.cbegin(), actors.cend(), target.actors.begin());
	std::copy(doors.cbegin(), doors.cend(), target.doors.begin());

	wall = target.wall;
}

bool SdlAudioMixer::Sound::is_audible() const
{
	return left_volume > 0.0F || right_volume > 0.0F;
}

SdlAudioMixer::SdlAudioMixer() = default;

bool SdlAudioMixer::initialize(
	const AudioMixerInitParam& param)
{
	uninitialize();

	switch (param.opl3_type_)
	{
		case Opl3Type::dbopl:
		case Opl3Type::nuked:
			break;

		default:
			return false;
	}

	if (param.dst_rate_ == 0)
	{
		dst_rate_ = get_default_rate();
	}
	else
	{
		dst_rate_ = std::max(param.dst_rate_, get_min_rate());
	}

	if (param.mix_size_ms_ == 0)
	{
		mix_size_ms_ = get_default_mix_size_ms();
	}
	else
	{
		mix_size_ms_ = std::max(param.mix_size_ms_, get_min_mix_size_ms());
	}

	mix_samples_count_ = calculate_mix_samples_count(dst_rate_, mix_size_ms_);

	constexpr auto lpf_order =
#ifdef NDEBUG
		80
#else
		40
#endif // NDEBUG
	;

	digitized_left_lpf_.initialize(
		lpf_order,
		audio_decoder_pcm_fixed_frequency / 2,
		dst_rate_
	);

	digitized_right_lpf_.initialize(
		lpf_order,
		audio_decoder_pcm_fixed_frequency / 2,
		dst_rate_
	);

	auto src_spec = SDL_AudioSpec{};
	src_spec.freq = dst_rate_;
	src_spec.format = AUDIO_F32SYS;
	src_spec.channels = static_cast<std::uint8_t>(get_max_channels());
	src_spec.samples = static_cast<std::uint16_t>(mix_samples_count_);
	src_spec.callback = callback_proxy;
	src_spec.userdata = this;

	auto dst_spec = SDL_AudioSpec{};

	sdl_audio_device_.reset(SDL_OpenAudioDevice(
		nullptr,
		0,
		&src_spec,
		&dst_spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE
	));

	if (!sdl_audio_device_)
	{
		return false;
	}

	auto is_succeed = true;

	if (is_succeed)
	{
		is_initialized_ = true;

		opl3_type_ = param.opl3_type_;

		mix_samples_count_ = dst_spec.samples;

		const auto total_samples = get_max_channels() * mix_samples_count_;

		buffer_.resize(total_samples);
		digitized_mix_samples_.resize(total_samples);
		mix_buffer_.resize(total_samples);

		adlib_music_cache_.resize(LASTMUSIC);
		adlib_sfx_cache_.resize(NUMSOUNDS);
		pc_speaker_sfx_cache_.resize(NUMSOUNDS);
		pcm_cache_.resize(NUMSOUNDS);

		const auto max_commands = get_max_commands();

		commands_.reserve(max_commands);
		mt_commands_.reserve(max_commands);

		mt_positions_.initialize();
		positions_.initialize();
		modified_actors_indices_.reserve(MAXACTORS);
		modified_doors_indices_.reserve(MAXDOORS);

		is_lpf_ = param.resampling_lpf_;

		SDL_PauseAudioDevice(sdl_audio_device_.get(), 0);
	}
	else
	{
		uninitialize();
	}

	return is_succeed;
}

void SdlAudioMixer::uninitialize()
{
	is_initialized_ = false;

	opl3_type_ = {};

	sdl_audio_device_.reset();

	dst_rate_ = 0;
	mix_samples_count_ = 0;
	buffer_ = {};
	mix_buffer_ = {};
	is_data_available_ = false;

	sounds_.clear();
	commands_.clear();
	mt_commands_.clear();
	mt_is_muted_ = false;
	mt_is_sfx_paused_ = false;
	mt_is_music_paused_ = false;
	adlib_music_cache_.clear();
	adlib_sfx_cache_.clear();
	pc_speaker_sfx_cache_.clear();
	pcm_cache_.clear();
	player_channels_state_ = 0;
	is_music_playing_ = false;
	is_any_sfx_playing_ = false;
	mix_size_ms_ = 0;
}

bool SdlAudioMixer::is_initialized() const
{
	return is_initialized_;
}

Opl3Type SdlAudioMixer::get_opl3_type() const
{
	return opl3_type_;
}

int SdlAudioMixer::get_rate() const
{
	return dst_rate_;
}

int SdlAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int SdlAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

float SdlAudioMixer::get_sfx_volume() const
{
	return sfx_volume_.load(std::memory_order_acquire);
}

float SdlAudioMixer::get_music_volume() const
{
	return music_volume_.load(std::memory_order_acquire);
}

bool SdlAudioMixer::get_resampling_lpf() const noexcept
{
	return is_lpf_;
}

bool SdlAudioMixer::play_adlib_music(
	const int music_index,
	const void* const data,
	const int data_size)
{
	return play_sound(SoundType::adlib_music, 0, music_index, data, data_size);
}

bool SdlAudioMixer::play_adlib_sound(
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

bool SdlAudioMixer::play_pc_speaker_sound(
	const int sound_index,
	const int priority,
	const void* const data,
	const int data_size,
	const int actor_index,
	const ActorType actor_type,
	const ActorChannel actor_channel)
{
	return play_sound(SoundType::pc_speaker_sfx, priority, sound_index, data, data_size, actor_index, actor_type, actor_channel);
}

bool SdlAudioMixer::play_pcm_sound(
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

bool SdlAudioMixer::set_resampling_low_pass_filter(
	const bool low_pass_filter)
{
	if (!is_initialized())
	{
		return false;
	}

	if (low_pass_filter == is_lpf_)
	{
		return true;
	}


	is_lpf_ = low_pass_filter;

	return true;
}

bool SdlAudioMixer::update_positions()
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

		const auto x = doorobjlist[i].tilex + 0.5;
		const auto y = doorobjlist[i].tiley + 0.5;

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

	// Pushwall.
	//
	auto is_push_wall_modified = false;

	{
		auto x = pwallx + 0.5;
		auto y = pwally + 0.5;

		const auto wall_offset = pwallpos;

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

bool SdlAudioMixer::stop_music()
{
	if (!is_initialized())
	{
		return false;
	}

	auto command = Command{};
	command.command_ = CommandType::stop_music;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

bool SdlAudioMixer::stop_pausable_sfx()
{
	if (!is_initialized())
	{
		return false;
	}

	auto command = Command{};
	command.command_ = CommandType::stop_pausable_sfx;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

bool SdlAudioMixer::pause_all_sfx(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_sfx_paused_ = is_paused;

	return true;
}

bool SdlAudioMixer::pause_music(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_music_paused_ = is_paused;

	return true;
}

bool SdlAudioMixer::set_mute(
	const bool is_mute)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_muted_ = is_mute;

	return true;
}

bool SdlAudioMixer::set_sfx_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	sfx_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool SdlAudioMixer::set_music_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	music_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool SdlAudioMixer::is_music_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_music_playing_;
}

bool SdlAudioMixer::is_any_unpausable_sfx_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_any_sfx_playing_;
}

bool SdlAudioMixer::is_player_channel_playing(
	const ActorChannel channel) const
{
	return (player_channels_state_ & (1 << static_cast<int>(channel))) != 0;
}

int SdlAudioMixer::get_min_rate() const
{
	return 11'025;
}

int SdlAudioMixer::get_default_rate() const
{
	return 44'100;
}

int SdlAudioMixer::get_min_mix_size_ms() const
{
	return 20;
}

int SdlAudioMixer::get_default_mix_size_ms() const
{
	return 40;
}

int SdlAudioMixer::get_max_channels() const
{
	return 2;
}

int SdlAudioMixer::get_max_commands() const
{
	return 192;
}

void SdlAudioMixer::callback(
	std::uint8_t* dst_data,
	const int dst_length)
{
	if (!mt_is_muted_ && is_data_available_)
	{
		std::uninitialized_copy_n(reinterpret_cast<const std::uint8_t*>(buffer_.data()), dst_length, dst_data);
	}
	else
	{
		std::uninitialized_fill_n(dst_data, dst_length, std::uint8_t{});
	}

	is_data_available_ = false;
}

void SdlAudioMixer::mix()
{
	handle_commands();

	if (!is_data_available_ && !sounds_.empty())
	{
		mix_samples();
		is_data_available_ = true;
	}
}

void SdlAudioMixer::mix_samples()
{
	if (sounds_.empty())
	{
		is_any_sfx_playing_ = false;
		std::uninitialized_fill(buffer_.begin(), buffer_.end(), Sample{});
		return;
	}

	spatialize_sounds();

	std::uninitialized_fill(digitized_mix_samples_.begin(), digitized_mix_samples_.end(), MixSample{});
	std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), MixSample{});

	const auto sfx_volume = sfx_volume_.load(std::memory_order_acquire);
	const auto music_volume = music_volume_.load(std::memory_order_acquire);

	auto sound_it = sounds_.begin();
	auto sound_end_it = sounds_.end();

	constexpr auto sfx_volume_scale = 7.0F;
	constexpr auto music_volume_scale = 6.0F;

	const auto is_sfx_paused = mt_is_sfx_paused_;
	const auto is_music_paused = mt_is_music_paused_;
	const auto is_lpf = is_lpf_;

	while (sound_it != sound_end_it)
	{
		if (is_sfx_paused &&
			(sound_it->type == SoundType::pc_speaker_sfx ||
				sound_it->type == SoundType::adlib_sfx ||
				sound_it->type == SoundType::pcm) &&
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
		const auto is_digitized = (sound_it->type == SoundType::pcm);

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

		auto& mix_buffer = ((!is_digitized || (is_digitized && !is_lpf)) ? mix_buffer_ : digitized_mix_samples_);

		const auto base_offset = (is_adlib_music ? 0 : sound_it->decode_offset);

		for (int i = 0; i < decode_count; ++i)
		{
			const auto sample = volume_scale * cache_item->samples[base_offset + i];
			const auto left_sample = static_cast<Sample>(sound_it->left_volume * sample);
			const auto right_sample = static_cast<Sample>(sound_it->right_volume * sample);

			mix_buffer[(2 * i) + 0] += left_sample;
			mix_buffer[(2 * i) + 1] += right_sample;
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

	if (is_lpf)
	{
		for (auto i = 0; i < mix_samples_count_; ++i)
		{
			const auto left_index = (2 * i) + 0;
			const auto right_index = left_index + 1;

			const auto left_sample = digitized_mix_samples_[left_index];
			const auto right_sample = digitized_mix_samples_[right_index];

			const auto lpf_left_sample = digitized_left_lpf_.process_sample(left_sample);
			const auto lpf_right_sample = digitized_right_lpf_.process_sample(right_sample);

			mix_buffer_[left_index] += static_cast<Sample>(lpf_left_sample);
			mix_buffer_[right_index] += static_cast<Sample>(lpf_right_sample);
		}
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
		constexpr auto max_mix_sample_value = 1.0F;

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
			const auto scalar = 1.0F / max_mix_sample;

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

	if (!is_sfx_paused)
	{
		is_any_sfx_playing_ = ((static_cast<int>(sounds_.size()) - music_count) > 0);
	}
	else
	{
		const auto unpausable_count = static_cast<int>(std::count_if(
			sounds_.cbegin(),
			sounds_.cend(),
			[](const Sound& item)
			{
				return item.actor_channel == ActorChannel::unpausable;
			}
		));

		is_any_sfx_playing_ = (unpausable_count - music_count) > 0;
	}
}

void SdlAudioMixer::handle_commands()
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
		switch (command.command_)
		{
		case CommandType::play:
			handle_play_command(command);
			break;

		case CommandType::stop_music:
			handle_stop_music_command();
			break;

		case CommandType::stop_pausable_sfx:
			handle_stop_pausable_sfx_command();
			break;

		default:
			break;
		}
	}

	commands_.clear();
}

void SdlAudioMixer::handle_play_command(
	const Command& command)
{
	auto cache_item = command.play_.sound.cache;

	if (!cache_item)
	{
		return;
	}

	if (!initialize_cache_item(command, *cache_item))
	{
		return;
	}

	const auto is_adlib_music = (command.play_.sound.type == SoundType::adlib_music);

	if (!is_adlib_music && command.play_.sound.actor_index >= 0)
	{
		// Search existing sound which can override a
		// new one because of priority.

		for (const auto& sound : sounds_)
		{
			if (sound.priority > command.play_.sound.priority &&
				sound.actor_index == command.play_.sound.actor_index &&
				sound.actor_type == command.play_.sound.actor_type &&
				sound.actor_channel == command.play_.sound.actor_channel)
			{
				return;
			}
		}

		// Remove sounds which will be overwritten.

		for (auto i = sounds_.begin(); i != sounds_.end(); )
		{
			if (i->actor_index == command.play_.sound.actor_index &&
				i->actor_type == command.play_.sound.actor_type &&
				i->actor_channel == command.play_.sound.actor_channel)
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

	auto sound = command.play_.sound;
	sound.decode_offset = 0;
	sounds_.push_back(sound);

	set_player_channel_state(sound, true);
}

void SdlAudioMixer::handle_stop_music_command()
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

void SdlAudioMixer::handle_stop_pausable_sfx_command()
{
	is_any_sfx_playing_ = false;

	sounds_.remove_if(
		[](const Sound& sound)
		{
			return
				sound.type != SoundType::adlib_music &&
				sound.actor_channel != ActorChannel::unpausable;
		}
	);
}

bool SdlAudioMixer::initialize_digitized_cache_item(
	const Command& command,
	CacheItem& cache_item)
{
	assert(!cache_item.is_active);
	assert(command.play_.sound.type == SoundType::pcm);

	const auto sample_count = calculate_digitized_sample_count(dst_rate_, command.play_.data_size);

	cache_item.is_active = true;
	cache_item.sound_type = command.play_.sound.type;
	cache_item.samples_count = sample_count;
	cache_item.samples.resize(sample_count);
	cache_item.digitized_resampler_counter = dst_rate_;
	cache_item.digitized_data = static_cast<const std::uint8_t*>(command.play_.data);
	cache_item.digitized_data_size = command.play_.data_size;

	return true;
}

bool SdlAudioMixer::initialize_cache_item(
	const Command& command,
	CacheItem& cache_item)
{
	if (cache_item.is_active)
	{
		return !cache_item.is_invalid;
	}

	if (command.play_.sound.type == SoundType::pcm)
	{
		return initialize_digitized_cache_item(command, cache_item);
	}

	cache_item = CacheItem{};
	cache_item.is_invalid = true;

	auto decoder = create_decoder_by_sound_type(command.play_.sound.type);

	if (!decoder)
	{
		return false;
	}

	auto param = AudioDecoderInitParam{};
	param.src_raw_data_ = command.play_.data;
	param.src_raw_size_ = command.play_.data_size;
	param.dst_rate_ = dst_rate_;

	if (!decoder->initialize(param))
	{
		return false;
	}

	const auto samples_count = decoder->get_dst_length_in_samples();

	if (samples_count <= 0)
	{
		return false;
	}

	const auto is_adlib_music = (command.play_.sound.type == SoundType::adlib_music);

	cache_item.is_active = true;
	cache_item.is_invalid = false;
	cache_item.sound_type = command.play_.sound.type;
	cache_item.samples_count = samples_count;
	cache_item.samples.resize(is_adlib_music ? mix_samples_count_ : samples_count);
	cache_item.buffer_size_ = 0;
	cache_item.decoder.swap(decoder);

	return true;
}

bool SdlAudioMixer::decode_digitized_sound(
	const Sound& sound)
{
	auto cache_item = sound.cache;

	assert(cache_item);
	assert(cache_item->is_active);
	assert(!cache_item->is_invalid);
	assert(!cache_item->is_decoded());
	assert(sound.type == SoundType::pcm);

	auto to_decode_count = std::min(cache_item->samples_count - cache_item->decoded_count, mix_samples_count_);

	for (auto i = 0; i < to_decode_count; ++i)
	{
		if (cache_item->digitized_resampler_counter >= dst_rate_)
		{
			cache_item->digitized_resampler_counter -= dst_rate_;

			if (cache_item->digitized_data_offset < cache_item->digitized_data_size)
			{
				const auto u8_sample = cache_item->digitized_data[cache_item->digitized_data_offset];
				const auto f32_sample = AudioSampleConverter::u8_to_f32(u8_sample);
				cache_item->digitized_last_sample = f32_sample;

				cache_item->digitized_data_offset += 1;
			}
		}

		cache_item->samples[cache_item->decoded_count] = cache_item->digitized_last_sample;
		cache_item->decoded_count += 1;

		cache_item->digitized_resampler_counter += audio_decoder_pcm_fixed_frequency;
	}

	return true;
}

bool SdlAudioMixer::decode_sound(
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

	if (sound.type == SoundType::pcm)
	{
		return decode_digitized_sound(sound);
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

void SdlAudioMixer::spatialize_sound(
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
		mt_is_sfx_paused_)
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

	//
	// translate point to view centered coordinates
	//
	const auto gx = location->x - mt_positions_.player.view_x;
	const auto gy = location->y - mt_positions_.player.view_y;

	//
	// calculate newx
	//
	auto xt = gx * mt_positions_.player.view_cos;
	auto yt = gy * mt_positions_.player.view_sin;
	auto x = static_cast<int>(xt - yt);

	if (x < 0)
	{
		x = -x;
	}

	if (x >= ATABLEMAX)
	{
		x = ATABLEMAX - 1;
	}

	//
	// calculate newy
	//
	xt = gx * mt_positions_.player.view_sin;
	yt = gy * mt_positions_.player.view_cos;
	auto y = static_cast<int>(yt + xt);

	if (y <= -ATABLEMAX)
	{
		y = -ATABLEMAX;
	}

	if (y >= ATABLEMAX)
	{
		y = ATABLEMAX - 1;
	}

	const auto left = 9 - lefttable[x][y + ATABLEMAX];
	const auto right = 9 - righttable[x][y + ATABLEMAX];

	sound.left_volume = left / 9.0F;
	sound.right_volume = right / 9.0F;
}

void SdlAudioMixer::spatialize_sounds()
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

bool SdlAudioMixer::play_sound(
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
	command.command_ = CommandType::play;
	command.play_.sound.type = sound_type;
	command.play_.sound.priority = priority;
	command.play_.sound.cache = get_cache_item(sound_type, sound_index);
	command.play_.sound.actor_index = actor_index;
	command.play_.sound.actor_type = actor_type;
	command.play_.sound.actor_channel = actor_channel;
	command.play_.data = data;
	command.play_.data_size = data_size;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

void SdlAudioMixer::lock()
{
	SDL_LockAudioDevice(sdl_audio_device_.get());
}

void SdlAudioMixer::unlock()
{
	SDL_UnlockAudioDevice(sdl_audio_device_.get());
}

void SdlAudioMixer::callback_proxy(
	void* user_data,
	std::uint8_t* dst_data,
	const int dst_length)
{
	assert(user_data);

	auto mixer = static_cast<SdlAudioMixer*>(user_data);

	mixer->mix();
	mixer->callback(dst_data, dst_length);
}

int SdlAudioMixer::mix_proxy(
	void* user_data)
{
	assert(user_data);

	auto mixer = static_cast<SdlAudioMixer*>(user_data);
	mixer->mix();

	return 0;
}

int SdlAudioMixer::calculate_mix_samples_count(
	const int dst_rate,
	const int mix_size_ms)
{
	const auto exact_count = (dst_rate * mix_size_ms) / 1000;

	auto actual_count = 1;

	while (actual_count < exact_count)
	{
		actual_count *= 2;
	}

	// Maximum power-of-two value for 16-bit unsigned type is 2^15 (32'768).
	if (actual_count > 32'768)
	{
		actual_count = 32'768;
	}

	return actual_count;
}

SdlAudioMixer::CacheItem* SdlAudioMixer::get_cache_item(
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

	case SoundType::pc_speaker_sfx:
		return &pc_speaker_sfx_cache_[sound_index];

	case SoundType::pcm:
		return &pcm_cache_[sound_index];

	default:
		return nullptr;
	}
}

void SdlAudioMixer::set_player_channel_state(
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

AudioDecoderUPtr SdlAudioMixer::create_decoder_by_sound_type(
	const SoundType sound_type) const
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
		return make_audio_decoder(AudioDecoderType::adlib_music, opl3_type_);

	case SoundType::adlib_sfx:
		return make_audio_decoder(AudioDecoderType::adlib_sfx, opl3_type_);

	case SoundType::pc_speaker_sfx:
		return make_audio_decoder(AudioDecoderType::pc_speaker, opl3_type_);

	default:
		return nullptr;
	}
}

bool SdlAudioMixer::is_sound_type_valid(
	const SoundType sound_type)
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
	case SoundType::adlib_sfx:
	case SoundType::pc_speaker_sfx:
	case SoundType::pcm:
		return true;

	default:
		return false;
	}
}

bool SdlAudioMixer::is_sound_index_valid(
	const int sound_index,
	const SoundType sound_type)
{
	switch (sound_type)
	{
	case SoundType::adlib_music:
		return sound_index >= 0 && sound_index < LASTMUSIC;

	case SoundType::adlib_sfx:
	case SoundType::pc_speaker_sfx:
	case SoundType::pcm:
		return sound_index >= 0 && sound_index < NUMSOUNDS;

	default:
		return false;
	}
}

int SdlAudioMixer::calculate_digitized_sample_count(
	int dst_sample_rate,
	int digitized_byte_count) noexcept
{
	assert(dst_sample_rate >= 0);
	assert(digitized_byte_count >= 0);
	assert(audio_decoder_pcm_fixed_frequency <= dst_sample_rate);

	const auto src_sample_rate = audio_decoder_pcm_fixed_frequency;
	const auto sample_count = ((digitized_byte_count * dst_sample_rate) + src_sample_rate - 1) / src_sample_rate;

	return sample_count;
}


} // bstone
