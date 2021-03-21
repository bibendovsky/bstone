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


#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED


#include <memory>

#include "audio.h"

#include "bstone_audio_decoder.h"
#include "bstone_opl3.h"


namespace bstone
{


class MtTaskMgr;


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
	unpausable,
}; // ActorChannel

enum class SoundType
{
	none,
	adlib_music,
	adlib_sfx,
	pc_speaker_sfx,
	pcm,
}; // SoundType


struct AudioMixerInitParam
{
	Opl3Type opl3_type_;
	int dst_rate_;

	int mix_size_ms_; // (milliseconds)

	AudioDecoderInterpolationType resampling_interpolation_;
	bool resampling_lpf_;
}; // AudioMixerInitParam


class AudioMixer
{
public:
	AudioMixer() = default;

	virtual ~AudioMixer() = default;


	virtual bool initialize(
		const AudioMixerInitParam& param) = 0;

	virtual void uninitialize() = 0;

	virtual bool is_initialized() const = 0;

	virtual Opl3Type get_opl3_type() const = 0;

	virtual int get_rate() const = 0;

	virtual int get_channel_count() const = 0;

	virtual int get_mix_size_ms() const = 0;

	virtual float get_sfx_volume() const = 0;

	virtual float get_music_volume() const = 0;

	virtual AudioDecoderInterpolationType get_resampling_interpolation() const noexcept = 0;

	virtual bool get_resampling_lpf() const noexcept = 0;

	virtual bool play_adlib_music(
		const int music_index,
		const void* const data,
		const int data_size) = 0;

	// Negative index of an actor defines a non-positional sound.
	virtual bool play_adlib_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) = 0;

	// Negative index of an actor defines a non-positional sound.
	virtual bool play_pc_speaker_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) = 0;

	// Negative index of an actor defines a non-positional sound.
	virtual bool play_pcm_sound(
		const int sound_index,
		const int priority,
		const void* const data,
		const int data_size,
		const int actor_index = -1,
		const ActorType actor_type = ActorType::none,
		const ActorChannel actor_channel = ActorChannel::voice) = 0;

	virtual bool set_resampling(
		const bstone::AudioDecoderInterpolationType interpolation,
		const bool low_pass_filter_) = 0;

	virtual bool update_positions() = 0;

	virtual bool stop_music() = 0;

	virtual bool stop_pausable_sfx() = 0;

	virtual bool pause_all_sfx(
		const bool is_pause) = 0;

	virtual bool pause_music(
		const bool is_pause) = 0;

	virtual bool set_mute(
		const bool value) = 0;

	virtual bool set_sfx_volume(
		const float volume) = 0;

	virtual bool set_music_volume(
		const float volume) = 0;

	virtual bool is_music_playing() const = 0;

	virtual bool is_any_unpausable_sfx_playing() const = 0;

	virtual bool is_player_channel_playing(
		const ActorChannel channel) const = 0;

	virtual int get_min_rate() const = 0;

	virtual int get_default_rate() const = 0;

	virtual int get_min_mix_size_ms() const = 0;

	virtual int get_default_mix_size_ms() const = 0;

	virtual int get_max_channels() const = 0;

	virtual int get_max_commands() const = 0;
}; // AudioMixer


using AudioMixerUPtr = std::unique_ptr<AudioMixer>;


AudioMixerUPtr make_audio_mixer(
	AudioDriverType audio_driver_type,
	MtTaskMgr* const mt_task_manager);


} // bstone


#endif // !BSTONE_AUDIO_MIXER_INCLUDED
