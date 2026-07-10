/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_OAL_SOURCE_INCLUDED
#define BSTONE_OAL_SOURCE_INCLUDED

#include "bstone_audio_decoder.h"
#include "bstone_oal_resource.h"
#include <cstddef>
#include <array>
#include <vector>

namespace bstone {

inline static constexpr int oal_source_max_streaming_buffers = 2;
static_assert(oal_source_max_streaming_buffers >= 2, "Streaming buffer count out of range.");

// =====================================

using OalSourceSoundSamples = std::vector<std::byte>;

struct OalSourceCachingSound
{
	bool is_initialized{};
	bool is_decoded{};
	int frame_count{};
	float pre_gain{};
	OalSourceSoundSamples samples{};
	OalSourceSoundSamples stereo_samples{};
	AudioDecoderUPtr audio_decoder{};
};

struct OalSourceUncachingSound
{
	bool is_initialized{};
	bool is_stereo{};
	int queue_size{};
	int read_frame_offset{};
	int write_frame_offset{};
	float pre_gain{};
	OalSourceSoundSamples samples{};
	AudioDecoderUPtr audio_decoder{};
};

// =====================================

struct OalSourceInitParam
{
	int mix_sample_rate{};
	int mix_frame_count{};
	int sample_size{};
};

// -------------------------------------

struct OalSourceOpenStaticParam
{
	bool is_3d{};
	int sample_rate{};
	int sample_size{};
	ALenum al_format{};
	const void* data{};
	int data_size{};
};

struct OalSourceOpenStreamingParam
{
	bool is_3d{};
	bool is_looping{};
	bool is_stereo{};
	int sample_rate{};
	int sample_size{};
	ALenum al_format{};
	OalSourceCachingSound* caching_sound{};
	OalSourceUncachingSound* uncaching_sound{};
};

// =====================================

class OalSource
{
public:
	bool initialize(const OalSourceInitParam& param);
	bool is_initialized() const;
	void terminate();

	void open(const OalSourceOpenStaticParam& param);
	void open(const OalSourceOpenStreamingParam& param);

	bool is_open() const;
	bool is_paused() const;
	bool is_playing() const;
	bool is_stopped() const;
	void set_gain(double gain);
	void set_position(double x, double y, double z);
	void set_reference_distance(double reference_distance);
	void set_max_distance(double max_distance);
	void set_rolloff_factor(double rolloff_factor);

	void play();
	void pause();
	void resume();
	void stop();

	void mix();
	void close();

private:
	using StreamingMixBuffer = std::vector<std::byte>;
	using StreamingOalBufferResources = std::array<OalBufferResource, oal_source_max_streaming_buffers>;
	using StreamingOalQueue = std::array<ALuint, oal_source_max_streaming_buffers>;

	using StreamingMixOalBufferFunc = bool (OalSource::*)(ALuint al_buffer);

	bool is_initialized_{};
	bool is_open_{};
	bool is_streaming_{};
	bool is_3d_{};
	bool is_looping_{};
	bool is_started_{};
	bool is_paused_{};
	bool is_stereo_{};
	mutable bool is_stopped_{};
	OalBufferResource static_al_buffer_resource_{};
	int streaming_sample_rate_{};
	int streaming_mix_frame_count_{};
	int streaming_caching_frame_offset_{};
	int sample_size_{};
	ALenum al_format_{};
	StreamingMixOalBufferFunc streaming_mix_oal_buffer_func_{};
	StreamingOalBufferResources streaming_al_buffer_resources_{};
	StreamingOalQueue streaming_al_queue_{};
	StreamingMixBuffer streaming_mix_buffer_{};
	OalSourceCachingSound* streaming_caching_sound_{};
	OalSourceUncachingSound* streaming_uncaching_sound_{};
	OalSourceResource al_source_resource_{};

	bool initialize_al_resources();

	int get_al_state() const;
	void al_play();
	void al_stop();
	int get_al_processed_buffer_count() const;
	void enqueue_al_buffer(ALuint al_buffer);
	void unqueue_al_buffers(int buffer_count, ALuint* al_buffer_names);

	void set_al_relative();

	void set_al_gain(ALfloat gain);
	void set_al_default_gain();

	void set_al_position(ALfloat x, ALfloat y, ALfloat z);
	void set_al_default_position();

	void set_al_reference_distance(ALfloat reference_distance);
	void set_al_default_reference_distance();

	void set_al_max_distance(ALfloat max_distance);
	void set_al_default_max_distance();

	void set_al_rolloff_factor(ALfloat rolloff_factor);
	void set_al_default_rolloff_factor();

	void attach_static_al_buffer();
	void detach_static_al_buffer();

	void set_static_al_buffer_data(const OalSourceOpenStaticParam& param);
	void set_streaming_al_buffer_data(ALint al_buffer, int frame_count, std::byte* samples_data);
	void set_streaming_al_buffer_data(ALint al_buffer);
	void set_streaming_al_buffer_defaults();

	void play_static();
	void play_streaming();

	void static_mix();

	bool streaming_mix_uncaching_sound(ALuint al_buffer);
	bool streaming_mix_caching_sound(ALuint al_buffer);
	void streaming_mix();
};

} // namespace bstone

#endif // BSTONE_OAL_SOURCE_INCLUDED
