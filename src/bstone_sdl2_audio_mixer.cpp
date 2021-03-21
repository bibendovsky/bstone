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


#include "bstone_sdl2_audio_mixer.h"

#include <cassert>
#include <cmath>

#include <algorithm>
#include <mutex>

#include "SDL_audio.h"

#include "3d_def.h"
#include "audio.h"

#include "bstone_audio_decoder.h"


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


Sdl2AudioMixer::CacheItem::CacheItem()
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

bool Sdl2AudioMixer::CacheItem::is_decoded() const
{
	return decoded_count == samples_count;
}

void Sdl2AudioMixer::Positions::initialize()
{
	player = {};

	actors.clear();
	actors.resize(MAXACTORS);

	doors.clear();
	doors.resize(MAXDOORS);

	wall = {};
}

void Sdl2AudioMixer::Positions::fixed_copy_to(
	Positions& target)
{
	player = target.player;

	std::copy(actors.cbegin(), actors.cend(), target.actors.begin());
	std::copy(doors.cbegin(), doors.cend(), target.doors.begin());

	wall = target.wall;
}

bool Sdl2AudioMixer::Sound::is_audible() const
{
	return left_volume > 0.0F || right_volume > 0.0F;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// SetResamplingMtTask
//

void Sdl2AudioMixer::SetResamplingMtTask::execute()
{
	auto is_invalid = false;

	if (!is_invalid)
	{
		const auto set_result = cache_item_->decoder->set_resampling(
			interpolation_,
			is_lpf_,
			true
		);

		if (!set_result)
		{
			is_invalid = true;
		}
	}

	if (!is_invalid)
	{
		const auto rewind_result = cache_item_->decoder->rewind();

		if (!rewind_result)
		{
			is_invalid = true;
		}
	}

	if (!is_invalid && cache_item_->decoded_count > 0)
	{
		const auto decoded_count = cache_item_->decoder->decode(
			cache_item_->decoded_count,
			cache_item_->samples.data()
		);

		if (decoded_count != cache_item_->decoded_count)
		{
			is_invalid = true;
		}
	}

	if (is_invalid)
	{
		cache_item_->is_invalid = true;
	}
}

bool Sdl2AudioMixer::SetResamplingMtTask::is_completed() const noexcept
{
	return is_completed_;
}

void Sdl2AudioMixer::SetResamplingMtTask::set_completed()
{
	is_completed_ = true;
}

bool Sdl2AudioMixer::SetResamplingMtTask::is_failed() const noexcept
{
	return is_failed_;
}

std::exception_ptr Sdl2AudioMixer::SetResamplingMtTask::get_exception_ptr() const noexcept
{
	return exception_ptr_;
}

void Sdl2AudioMixer::SetResamplingMtTask::set_failed(
	std::exception_ptr exception_ptr)
{
	is_completed_ = true;
	is_failed_ = true;
	exception_ptr_ = exception_ptr;
}

void Sdl2AudioMixer::SetResamplingMtTask::initialize(
	CacheItem* cache_item,
	const AudioDecoderInterpolationType interpolation,
	const bool is_lpf) noexcept
{
	cache_item_ = cache_item;
	interpolation_ = interpolation;
	is_lpf_ = is_lpf;
}

//
// SetResamplingMtTask
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

Sdl2AudioMixer::Sdl2AudioMixer(
	MtTaskMgr* const mt_task_manager)
	:
	mt_task_manager_{mt_task_manager},
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
	mix_size_ms_{},
	interpolation_{},
	is_lpf_{},
	set_resampling_mt_tasks_{},
	set_resampling_mt_tasks_ptrs_{}
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

Sdl2AudioMixer::~Sdl2AudioMixer()
{
	uninitialize();
}

bool Sdl2AudioMixer::initialize(
	const AudioMixerInitParam& param)
{
	uninitialize();

	switch (param.opl3_type_)
	{
		case Opl3Type::dbopl:
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

	auto src_spec = SDL_AudioSpec{};
	src_spec.freq = dst_rate_;
	src_spec.format = AUDIO_F32SYS;
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

		opl3_type_ = param.opl3_type_;

		mix_samples_count_ = dst_spec.samples;

		const auto total_samples = get_max_channels() * mix_samples_count_;

		buffer_.resize(total_samples);
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

		interpolation_ = param.resampling_interpolation_;
		is_lpf_ = param.resampling_lpf_;

		set_resampling_mt_tasks_.resize(NUMSOUNDS);
		set_resampling_mt_tasks_ptrs_.resize(NUMSOUNDS);

		for (int i = 0; i < NUMSOUNDS; ++i)
		{
			set_resampling_mt_tasks_ptrs_[i] = &set_resampling_mt_tasks_[i];
		}

		SDL_PauseAudioDevice(device_id_, 0);
	}
	else
	{
		uninitialize();
	}

	return is_succeed;
}

void Sdl2AudioMixer::uninitialize()
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
	pc_speaker_sfx_cache_.clear();
	pcm_cache_.clear();
	player_channels_state_ = 0;
	is_music_playing_ = false;
	is_any_sfx_playing_ = false;
	mix_size_ms_ = 0;
}

bool Sdl2AudioMixer::is_initialized() const
{
	return is_initialized_;
}

Opl3Type Sdl2AudioMixer::get_opl3_type() const
{
	return opl3_type_;
}

int Sdl2AudioMixer::get_rate() const
{
	return dst_rate_;
}

int Sdl2AudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int Sdl2AudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

float Sdl2AudioMixer::get_sfx_volume() const
{
	return sfx_volume_.load(std::memory_order_acquire);
}

float Sdl2AudioMixer::get_music_volume() const
{
	return music_volume_.load(std::memory_order_acquire);
}

AudioDecoderInterpolationType Sdl2AudioMixer::get_resampling_interpolation() const noexcept
{
	return interpolation_;
}

bool Sdl2AudioMixer::get_resampling_lpf() const noexcept
{
	return is_lpf_;
}

bool Sdl2AudioMixer::play_adlib_music(
	const int music_index,
	const void* const data,
	const int data_size)
{
	return play_sound(SoundType::adlib_music, 0, music_index, data, data_size);
}

bool Sdl2AudioMixer::play_adlib_sound(
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

bool Sdl2AudioMixer::play_pc_speaker_sound(
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

bool Sdl2AudioMixer::play_pcm_sound(
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

bool Sdl2AudioMixer::set_resampling(
	const bstone::AudioDecoderInterpolationType interpolation,
	const bool low_pass_filter_)
{
	if (!is_initialized())
	{
		return false;
	}

	auto new_interpolation = interpolation;

	switch (interpolation)
	{
		case bstone::AudioDecoderInterpolationType::zoh:
		case bstone::AudioDecoderInterpolationType::linear:
			break;

		default:
			new_interpolation = bstone::AudioDecoderInterpolationType::linear;
			break;
	}

	if (new_interpolation == interpolation_ &&
		low_pass_filter_ == is_lpf_)
	{
		return true;
	}

	interpolation_ = new_interpolation;
	is_lpf_ = low_pass_filter_;

	auto command = Command{};
	command.command_ = CommandType::resampling;
	command.resampling_.interpolation_ = interpolation_;
	command.resampling_.low_pass_filter_ = is_lpf_;

	MtLockGuard guard_lock{mt_commands_lock_};

	mt_commands_.push_back(command);

	return true;
}

bool Sdl2AudioMixer::update_positions()
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

bool Sdl2AudioMixer::stop_music()
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

bool Sdl2AudioMixer::stop_pausable_sfx()
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

bool Sdl2AudioMixer::pause_all_sfx(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_sfx_paused_.store(is_paused, std::memory_order_release);

	return true;
}

bool Sdl2AudioMixer::pause_music(
	const bool is_paused)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_music_paused_.store(is_paused, std::memory_order_release);

	return true;
}

bool Sdl2AudioMixer::set_mute(
	const bool is_mute)
{
	if (!is_initialized())
	{
		return false;
	}

	mt_is_muted_.store(is_mute, std::memory_order_release);

	return true;
}

bool Sdl2AudioMixer::set_sfx_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	sfx_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool Sdl2AudioMixer::set_music_volume(
	const float volume)
{
	if (!is_initialized())
	{
		return false;
	}

	music_volume_ = std::min(std::max(volume, 0.0F), 1.0F);

	return true;
}

bool Sdl2AudioMixer::is_music_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_music_playing_;
}

bool Sdl2AudioMixer::is_any_unpausable_sfx_playing() const
{
	if (!is_initialized())
	{
		return false;
	}

	return is_any_sfx_playing_;
}

bool Sdl2AudioMixer::is_player_channel_playing(
	const ActorChannel channel) const
{
	return (player_channels_state_ & (1 << static_cast<int>(channel))) != 0;
}

int Sdl2AudioMixer::get_min_rate() const
{
	return 11'025;
}

int Sdl2AudioMixer::get_default_rate() const
{
	return 44'100;
}

int Sdl2AudioMixer::get_min_mix_size_ms() const
{
	return 20;
}

int Sdl2AudioMixer::get_default_mix_size_ms() const
{
	return 40;
}

int Sdl2AudioMixer::get_max_channels() const
{
	return 2;
}

int Sdl2AudioMixer::get_max_commands() const
{
	return 192;
}

void Sdl2AudioMixer::callback(
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

void Sdl2AudioMixer::mix()
{
	handle_commands();

	if (!is_data_available_ && !sounds_.empty())
	{
		mix_samples();
		is_data_available_ = true;
	}
}

void Sdl2AudioMixer::mix_samples()
{
	if (sounds_.empty())
	{
		is_any_sfx_playing_ = false;
		std::uninitialized_fill(buffer_.begin(), buffer_.end(), Sample{});
		return;
	}

	spatialize_sounds();

	std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), MixSample{});

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
			const auto left_sample = static_cast<Sample>(sound_it->left_volume * sample);

			mix_buffer_[(2 * i) + 0] += left_sample;

			// Right channel.
			//
			const auto right_sample = static_cast<Sample>(sound_it->right_volume * sample);

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

void Sdl2AudioMixer::handle_resampling()
{
	auto task_index = 0;

	for (auto& cache : pcm_cache_)
	{
		if (cache.is_invalid || !cache.is_active)
		{
			continue;
		}

#if 0
		auto is_invalid = false;

		if (!is_invalid)
		{
			const auto set_result = cache.decoder->set_resampling(
				interpolation_,
				is_lpf_,
				true
			);

			if (!set_result)
			{
				is_invalid = true;
			}
		}

		if (!is_invalid)
		{
			const auto rewind_result = cache.decoder->rewind();

			if (!rewind_result)
			{
				is_invalid = true;
			}
		}

		if (!is_invalid && cache.decoded_count > 0)
		{
			const auto decoded_count = cache.decoder->decode(
				cache.decoded_count,
				cache.samples.data()
			);

			if (decoded_count != cache.decoded_count)
			{
				is_invalid = true;
			}
		}

		if (is_invalid)
		{
			cache.is_invalid = true;
		}
#else
		set_resampling_mt_tasks_[task_index++].initialize(
			&cache,
			interpolation_,
			is_lpf_
		);
#endif
	}

	if (task_index > 0)
	{
		try
		{
			mt_task_manager_->add_tasks_and_wait_for_added(
				set_resampling_mt_tasks_ptrs_.data(),
				task_index
			);
		}
		catch (...)
		{
		}
	}
}

void Sdl2AudioMixer::handle_commands()
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

	auto has_resampling = false;
	CommandResampling command_resampling;

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

		case CommandType::resampling:
			has_resampling = true;
			command_resampling = command.resampling_;
			break;

		default:
			break;
		}
	}

	commands_.clear();

	if (has_resampling)
	{
		handle_resampling();
	}
}

void Sdl2AudioMixer::handle_play_command(
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

void Sdl2AudioMixer::handle_stop_music_command()
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

void Sdl2AudioMixer::handle_stop_pausable_sfx_command()
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

bool Sdl2AudioMixer::initialize_cache_item(
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
		decoder = create_decoder_by_sound_type(command.play_.sound.type);

		is_succeed = (decoder != nullptr);
	}

	if (is_succeed)
	{
		auto param = AudioDecoderInitParam{};
		param.src_raw_data_ = command.play_.data;
		param.src_raw_size_ = command.play_.data_size;
		param.dst_rate_ = dst_rate_;
		param.resampler_interpolation_ = interpolation_;
		param.resampler_lpf_ = is_lpf_;

		is_succeed = decoder->initialize(param);
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
		const auto is_adlib_music = (command.play_.sound.type == SoundType::adlib_music);

		cache_item.sound_type = command.play_.sound.type;
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

bool Sdl2AudioMixer::decode_sound(
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

void Sdl2AudioMixer::spatialize_sound(
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

void Sdl2AudioMixer::spatialize_sounds()
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

bool Sdl2AudioMixer::play_sound(
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

void Sdl2AudioMixer::lock()
{
	SDL_LockAudioDevice(device_id_);
}

void Sdl2AudioMixer::unlock()
{
	SDL_UnlockAudioDevice(device_id_);
}

void Sdl2AudioMixer::callback_proxy(
	void* user_data,
	std::uint8_t* dst_data,
	const int dst_length)
{
	assert(user_data);

	auto mixer = static_cast<Sdl2AudioMixer*>(user_data);

	mixer->mix();
	mixer->callback(dst_data, dst_length);
}

int Sdl2AudioMixer::mix_proxy(
	void* user_data)
{
	assert(user_data);

	auto mixer = static_cast<Sdl2AudioMixer*>(user_data);
	mixer->mix();

	return 0;
}

int Sdl2AudioMixer::calculate_mix_samples_count(
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

Sdl2AudioMixer::CacheItem* Sdl2AudioMixer::get_cache_item(
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

void Sdl2AudioMixer::set_player_channel_state(
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

AudioDecoderUPtr Sdl2AudioMixer::create_decoder_by_sound_type(
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

	case SoundType::pcm:
		return make_audio_decoder(AudioDecoderType::pcm, opl3_type_);

	default:
		return nullptr;
	}
}

bool Sdl2AudioMixer::is_sound_type_valid(
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

bool Sdl2AudioMixer::is_sound_index_valid(
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


} // bstone
