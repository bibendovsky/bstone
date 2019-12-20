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


#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED


#include <memory>


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
		const AudioMixer& rhs) = delete;

	AudioMixer(
		AudioMixer&& rhs);

	AudioMixer& operator=(
		const AudioMixer& rhs) = delete;

	~AudioMixer();


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
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr impl_;
}; // AudioMixer


} // bstone


#endif // !BSTONE_AUDIO_MIXER_INCLUDED
