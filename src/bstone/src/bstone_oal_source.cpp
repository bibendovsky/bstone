/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_oal_source.h"
#include "bstone_assert.h"
#include "bstone_oal_symbols.h"
#include <algorithm>
#include <iterator>

namespace bstone {

bool OalSource::initialize(const OalSourceInitParam& param)
{
	BSTONE_ASSERT(param.mix_sample_rate > 0);
	BSTONE_ASSERT(param.mix_frame_count > 0);
	BSTONE_ASSERT(param.sample_size == 2 || param.sample_size == 4);
	terminate();
	streaming_mix_frame_count_ = param.mix_frame_count;
	sample_size_ = param.sample_size;
	streaming_mix_buffer_.resize(streaming_mix_frame_count_ * sample_size_ * 2);
	if (!initialize_al_resources())
	{
		terminate();
		return false;
	}
	is_initialized_ = true;
	return true;
}

bool OalSource::is_initialized() const
{
	return is_initialized_;
}

void OalSource::terminate()
{
	close();
	is_initialized_ = false;
	is_stereo_ = false;
	al_source_resource_.reset();
	static_al_buffer_resource_.reset();
	for (OalBufferResource& streaming_al_buffer_resource : streaming_al_buffer_resources_)
		streaming_al_buffer_resource.reset();
	al_format_ = 0;
}

void OalSource::open(const OalSourceOpenStaticParam& param)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(param.sample_rate > 0);
	BSTONE_ASSERT(param.data != nullptr);
	BSTONE_ASSERT(param.data_size >= 0);
	BSTONE_ASSERT(param.al_format > 0);
	close();
	is_3d_ = param.is_3d;
	is_stereo_ = false;
	al_format_ = param.al_format;
	set_al_relative();
	set_al_default_gain();
	set_al_default_position();
	set_al_default_reference_distance();
	set_al_default_max_distance();
	set_al_default_rolloff_factor();
	detach_static_al_buffer();
	set_static_al_buffer_data(param);
	attach_static_al_buffer();
	is_open_ = true;
	is_streaming_ = false;
	is_started_ = false;
	is_paused_ = false;
	is_stopped_ = false;
}

void OalSource::open(const OalSourceOpenStreamingParam& param)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(param.sample_rate > 0);
	BSTONE_ASSERT((param.caching_sound != nullptr && param.uncaching_sound == nullptr) ||
		(param.caching_sound == nullptr && param.uncaching_sound != nullptr));
	BSTONE_ASSERT(!(param.is_looping && param.caching_sound != nullptr));
	BSTONE_ASSERT(!(param.is_3d && param.is_stereo));
	BSTONE_ASSERT(!(param.is_stereo && param.caching_sound != nullptr));
	BSTONE_ASSERT(param.al_format > 0);
	close();
	const int al_processed_buffer_count = get_al_processed_buffer_count();
	if (al_processed_buffer_count != 0)
		unqueue_al_buffers(al_processed_buffer_count, streaming_al_queue_.data());
	is_3d_ = param.is_3d;
	is_looping_ = param.is_looping;
	is_stereo_ = param.is_stereo;
	streaming_sample_rate_ = param.sample_rate;
	streaming_caching_sound_ = param.caching_sound;
	streaming_uncaching_sound_ = param.uncaching_sound;
	streaming_caching_frame_offset_ = 0;
	streaming_mix_oal_buffer_func_ = (streaming_uncaching_sound_ ?
		&OalSource::streaming_mix_uncaching_sound :
		&OalSource::streaming_mix_caching_sound);
	al_format_ = param.al_format;
	set_al_relative();
	set_al_default_position();
	detach_static_al_buffer();
	set_streaming_al_buffer_defaults();
	is_open_ = true;
	is_streaming_ = true;
	is_started_ = false;
	is_paused_ = false;
	is_stopped_ = false;
}

bool OalSource::is_open() const
{
	return is_open_;
}

bool OalSource::is_paused() const
{
	return is_paused_;
}

bool OalSource::is_playing() const
{
	if (!is_open_ || is_paused_ || is_stopped_)
		return false;
	switch (get_al_state())
	{
		case AL_PLAYING:
			return true;
		default:
			is_stopped_ = true;
			return false;
	}
}

bool OalSource::is_stopped() const
{
	return is_stopped_;
}

void OalSource::set_gain(double gain)
{
	BSTONE_ASSERT(is_open());
	set_al_gain(static_cast<ALfloat>(gain));
}

void OalSource::set_position(double x, double y, double z)
{
	BSTONE_ASSERT(is_open());
	set_al_position(static_cast<ALfloat>(x), static_cast<ALfloat>(y), static_cast<ALfloat>(z));
}

void OalSource::set_reference_distance(double reference_distance)
{
	BSTONE_ASSERT(is_open());
	set_al_reference_distance(static_cast<ALfloat>(reference_distance));
}

void OalSource::set_max_distance(double max_distance)
{
	BSTONE_ASSERT(is_open());
	set_al_max_distance(static_cast<ALfloat>(max_distance));
}

void OalSource::set_rolloff_factor(double rolloff_factor)
{
	BSTONE_ASSERT(is_open());
	set_al_rolloff_factor(static_cast<ALfloat>(rolloff_factor));
}

void OalSource::play()
{
	BSTONE_ASSERT(is_open());
	if (is_started_)
		return;
	is_started_ = true;
	if (is_streaming_)
		play_streaming();
	else
		play_static();
}

void OalSource::pause()
{
	BSTONE_ASSERT(is_started_);
	if (is_paused_ || is_stopped_)
		return;
	is_paused_ = true;
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcePause != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcePause(al_source_resource_.get());
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::resume()
{
	BSTONE_ASSERT(is_started_);
	if (!is_paused_ || is_stopped_)
		return;
	is_paused_ = false;
	al_play();
}

void OalSource::stop()
{
	BSTONE_ASSERT(is_started_);
	if (is_stopped_)
		return;
	is_paused_ = false;
	is_stopped_ = true;
	al_stop();
}

void OalSource::mix()
{
	BSTONE_ASSERT(is_started_);
	if (is_paused_ || is_stopped_)
		return;
	if (is_streaming_)
		streaming_mix();
	else
		static_mix();
}

void OalSource::close()
{
	is_open_ = false;
	if (!al_source_resource_.is_empty())
		al_stop();
}

bool OalSource::initialize_al_resources()
{
	static_al_buffer_resource_ = make_oal_buffer();
	if (static_al_buffer_resource_.is_empty())
		return false;
	for (OalBufferResource& streaming_al_buffer_resource : streaming_al_buffer_resources_)
	{
		streaming_al_buffer_resource = make_oal_buffer();
		if (streaming_al_buffer_resource.is_empty())
			return false;
	}
	al_source_resource_ = make_oal_source();
	if (al_source_resource_.is_empty())
		return false;
	return true;
}

int OalSource::get_al_state() const
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alGetSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	ALint al_state = 0;
	alGetSourcei(al_source_resource_.get(), AL_SOURCE_STATE, &al_state);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
	return al_state;
}

void OalSource::al_play()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcePlay != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcePlay(al_source_resource_.get());
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::al_stop()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourceStop != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourceStop(al_source_resource_.get());
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

int OalSource::get_al_processed_buffer_count() const
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alGetSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	ALint al_buffer_count = 0;
	alGetSourcei(al_source_resource_.get(), AL_BUFFERS_PROCESSED, &al_buffer_count);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
	return al_buffer_count;
}

void OalSource::enqueue_al_buffer(ALuint al_buffer)
{
	BSTONE_ASSERT(al_buffer != 0);
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourceQueueBuffers != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourceQueueBuffers(al_source_resource_.get(), 1, &al_buffer);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::unqueue_al_buffers(int buffer_count, ALuint* al_buffer_names)
{
	BSTONE_ASSERT(buffer_count >= 0 && buffer_count <= oal_source_max_streaming_buffers);
	BSTONE_ASSERT(al_buffer_names != nullptr);
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourceUnqueueBuffers != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourceUnqueueBuffers(al_source_resource_.get(), buffer_count, al_buffer_names);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_relative()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcei(al_source_resource_.get(), AL_SOURCE_RELATIVE, is_3d_ ? AL_FALSE : AL_TRUE);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_gain(ALfloat gain)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcef != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcef(al_source_resource_.get(), AL_GAIN, gain);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_gain()
{
	set_al_gain(1.0F);
}

void OalSource::set_al_position(ALfloat x, ALfloat y, ALfloat z)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSource3f != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSource3f(al_source_resource_.get(), AL_POSITION, x, y, z);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_position()
{
	set_al_position(0.0F, 0.0F, 0.0F);
}

void OalSource::set_al_reference_distance(ALfloat reference_distance)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcef != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcef(al_source_resource_.get(), AL_REFERENCE_DISTANCE, reference_distance);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_reference_distance()
{
	set_al_reference_distance(1.0F);
}

void OalSource::set_al_max_distance(ALfloat max_distance)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcef != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcef(al_source_resource_.get(), AL_MAX_DISTANCE, max_distance);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_max_distance()
{
	set_al_max_distance(FLT_MAX);
}

void OalSource::set_al_rolloff_factor(ALfloat rolloff_factor)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcef != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcef(al_source_resource_.get(), AL_ROLLOFF_FACTOR, rolloff_factor);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_rolloff_factor()
{
	set_al_rolloff_factor(1.0F);
}

void OalSource::attach_static_al_buffer()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcei(al_source_resource_.get(), AL_BUFFER, static_cast<ALint>(static_al_buffer_resource_.get()));
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::detach_static_al_buffer()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	alSourcei(al_source_resource_.get(), AL_BUFFER, 0);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_static_al_buffer_data(const OalSourceOpenStaticParam& param)
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alBufferData != nullptr);
	BSTONE_ASSERT(al_format_ != 0);
	BSTONE_ASSERT((alGetError(), true));
	alBufferData(static_al_buffer_resource_.get(), al_format_, param.data, param.data_size, param.sample_rate);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(ALint al_buffer, int frame_count, std::byte* sample_bytes)
{
	BSTONE_ASSERT(al_buffer != 0);
	BSTONE_ASSERT(frame_count > 0);
	BSTONE_ASSERT(sample_bytes != nullptr);
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alBufferData != nullptr);
	BSTONE_ASSERT(al_format_ != 0);
	const int buffer_size = frame_count * sample_size_ * (1 + is_stereo_);
	BSTONE_ASSERT((alGetError(), true));
	alBufferData(al_buffer, al_format_, sample_bytes, buffer_size, streaming_sample_rate_);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(ALint al_buffer)
{
	set_streaming_al_buffer_data(al_buffer, streaming_mix_frame_count_, streaming_mix_buffer_.data());
}

void OalSource::set_streaming_al_buffer_defaults()
{
	std::fill(streaming_mix_buffer_.begin(), streaming_mix_buffer_.end(), std::byte{});
	for (int i = 0; i < oal_source_max_streaming_buffers; ++i)
	{
		const ALuint al_buffer = streaming_al_buffer_resources_[i].get();
		streaming_al_queue_[i] = al_buffer;
		set_streaming_al_buffer_data(al_buffer);
	}
}

void OalSource::play_static()
{
	al_play();
}

void OalSource::play_streaming()
{}

void OalSource::static_mix()
{
	switch (get_al_state())
	{
		case AL_PLAYING:
			break;
		default:
			is_stopped_ = true;
			break;
	}
}

bool OalSource::streaming_mix_uncaching_sound(ALuint al_buffer)
{
	if (streaming_uncaching_sound_->queue_size <= 0)
		return false;
	--streaming_uncaching_sound_->queue_size;
	const int streaming_max_mix_frame_count = oal_source_max_streaming_buffers * streaming_mix_frame_count_;
	if (streaming_uncaching_sound_->read_frame_offset >= streaming_max_mix_frame_count)
		streaming_uncaching_sound_->read_frame_offset = 0;
	const int channel_count = 1 + streaming_uncaching_sound_->is_stereo;
	set_streaming_al_buffer_data(
		al_buffer,
		streaming_mix_frame_count_,
		&streaming_uncaching_sound_->samples[streaming_uncaching_sound_->read_frame_offset * sample_size_ * channel_count]);
	streaming_uncaching_sound_->read_frame_offset += streaming_mix_frame_count_;
	enqueue_al_buffer(al_buffer);
	return true;
}

bool OalSource::streaming_mix_caching_sound(ALuint al_buffer)
{
	const int frames_left = streaming_caching_sound_->frame_count - streaming_caching_frame_offset_;
	if (frames_left <= 0)
		return false;
	const int frame_count = std::min(frames_left, streaming_mix_frame_count_);
	set_streaming_al_buffer_data(al_buffer, frame_count, &streaming_caching_sound_->samples[streaming_caching_frame_offset_ * sample_size_]);
	enqueue_al_buffer(al_buffer);
	streaming_caching_frame_offset_ += frame_count;
	return true;
}

void OalSource::streaming_mix()
{
	BSTONE_ASSERT(alGetError != nullptr);
	BSTONE_ASSERT(alGetSourcei != nullptr);
	BSTONE_ASSERT((alGetError(), true));
	ALint al_queue_size = 0;
	ALint al_mixed_size = 0;
	alGetSourcei(al_source_resource_.get(), AL_BUFFERS_QUEUED, &al_queue_size);
	alGetSourcei(al_source_resource_.get(), AL_BUFFERS_PROCESSED, &al_mixed_size);
	BSTONE_ASSERT(alGetError() == AL_NO_ERROR);
	if (al_mixed_size > 0)
	{
		unqueue_al_buffers(al_mixed_size, streaming_al_queue_.data());
		if (al_mixed_size != oal_source_max_streaming_buffers)
			std::rotate(streaming_al_queue_.begin(), std::next(streaming_al_queue_.begin(), al_mixed_size), streaming_al_queue_.end());
		al_queue_size -= al_mixed_size;
	}
	ALint buffer_index = al_queue_size;
	while (al_queue_size < oal_source_max_streaming_buffers)
	{
		const ALuint al_buffer = streaming_al_queue_[buffer_index];
		const bool mix_result = (this->*streaming_mix_oal_buffer_func_)(al_buffer);
		if (!mix_result)
			break;
		++buffer_index;
		++al_queue_size;
	}
	if (al_queue_size == 0)
	{
		is_stopped_ = true;
		return;
	}
	switch (get_al_state())
	{
		case AL_PLAYING:
			break;
		default:
			al_play();
			break;
	}
}

} // namespace bstone
