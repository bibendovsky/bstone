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


#ifndef BSTONE_OAL_SOURCE_INCLUDED
#define BSTONE_OAL_SOURCE_INCLUDED


#include <array>
#include <vector>

#include "bstone_audio_decoder.h"
#include "bstone_oal_resource.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

static constexpr auto oal_source_max_streaming_buffers = 2;

static_assert(oal_source_max_streaming_buffers >= 2, "Streaming buffer count out of range.");


using OalSourceSample = std::int16_t;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using OalSourceSoundSamples = std::vector<OalSourceSample>;

struct OalSourceCachingSound
{
	bool is_initialized{};
	bool is_decoded{};
	int sample_offset{};
	int sample_count{};
	OalSourceSoundSamples samples{};
	AudioDecoderUPtr audio_decoder{};
}; // OalSourceCachingSound

struct OalSourceUncachingSound
{
	bool is_initialized{};
	int queue_size{};
	int read_sample_offset{};
	int write_sample_offset{};
	OalSourceSoundSamples samples{};
	AudioDecoderUPtr audio_decoder{};
}; // OalSourceUncachingSound

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct OalSourceInitParam
{
	int mix_sample_rate{};
	int mix_sample_count{};
	const OalSymbols* oal_symbols{};
}; // OalSourceInitParam

// --------------------------------------------------------------------------

struct OalSourceOpenStaticParam
{
	bool is_2d{};
	bool is_8_bit{};
	int sample_rate{};
	const void* data{};
	int data_size{};
}; // OalSourceOpenStaticParam

struct OalSourceOpenStreamingParam
{
	bool is_2d{};
	bool is_looping{};
	int sample_rate{};
	OalSourceCachingSound* caching_sound{};
	OalSourceUncachingSound* uncaching_sound{};
}; // OalSourceOpenStreamingParam

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalSource
{
public:
	void initialize(
		const OalSourceInitParam& param);

	bool is_initialized() const noexcept;

	void uninitialize();


	void open(
		const OalSourceOpenStaticParam& param);

	void open(
		const OalSourceOpenStreamingParam& param);

	bool is_open() const;

	bool is_paused() const;

	bool is_playing() const;

	bool is_finished() const;

	void set_volume(
		float volume);

	void set_position(
		float x,
		float y,
		float z);

	void set_reference_distance(
		float reference_distance);

	void play();

	void pause();

	void resume();

	void stop();

	void mix();

	void close();


private:
	using StreamingMixBuffer = std::vector<OalSourceSample>;
	using StreamingOalBufferResources = std::array<OalBufferResource, oal_source_max_streaming_buffers>;
	using StreamingOalQueue = std::array<ALuint, oal_source_max_streaming_buffers>;

	using StreamingMixOalBufferFunc = bool (OalSource::*)(
		ALuint al_buffer);


	static constexpr auto sample_size = static_cast<int>(sizeof(OalSourceSample));


	const OalSymbols* oal_symbols_{};

	bool is_initialized_{};
	bool is_open_{};
	bool is_streaming_{};
	bool is_2d_{};
	bool is_looping_{};
	bool is_started_{};
	bool is_paused_{};
	mutable bool is_finished_{};

	OalBufferResource static_al_buffer_resource_{};

	int streaming_sample_rate_{};
	int streaming_mix_sample_count_{};
	int streaming_caching_sample_offset_{};
	int streaming_caching_sample_count_{};
	StreamingMixOalBufferFunc streaming_mix_oal_buffer_func_{};

	StreamingOalBufferResources streaming_al_buffer_resources_{};
	StreamingOalQueue streaming_al_queue_{};
	StreamingMixBuffer streaming_mix_buffer_{};
	OalSourceCachingSound* streaming_caching_sound_{};
	OalSourceUncachingSound* streaming_uncaching_sound_{};

	OalSourceResource al_source_resource_{};


	void initialize_al_resources();


	void ensure_is_initialized() const;

	void ensure_is_open() const;

	void ensure_is_started() const;


	int get_al_state() const;

	void al_play();

	void al_stop();

	int get_al_queued_buffer_count() const;

	void enqueue_al_buffer(
		ALuint al_buffer);

	void unqueue_al_buffers(
		int buffer_count,
		ALuint* al_buffer_names);

	void set_al_relative();

	void set_al_position(
		ALfloat x,
		ALfloat y,
		ALfloat z);

	void set_default_al_position();

	void set_al_velocity(
		ALfloat x,
		ALfloat y,
		ALfloat z);

	void set_default_al_velocity();


	void set_al_reference_distance(
		ALfloat reference_distance);


	void attach_static_al_buffer();

	void detach_static_al_buffer();


	void set_static_al_buffer_data(
		const OalSourceOpenStaticParam& param);


	void set_streaming_al_buffer_data(
		ALint al_buffer,
		int sample_count,
		OalSourceSample* samples);

	void set_streaming_al_buffer_data(
		ALint al_buffer);

	void set_streaming_al_buffer_data(
		ALint al_buffer,
		int sample_count);

	void set_streaming_al_buffer_defaults();


	void play_static();

	void play_streaming();


	void static_mix();

	bool streaming_mix_uncaching_sound(
		ALuint al_buffer);

	bool streaming_mix_caching_sound(
		ALuint al_buffer);

	void streaming_mix();
}; // OalSource

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_OAL_SOURCE_INCLUDED
