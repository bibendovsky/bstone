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


#include "bstone_oal_source.h"

#include <cassert>

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>

#include "bstone_exception.h"
#include "bstone_not_null.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalSourceException :
	public Exception
{
public:
	explicit OalSourceException(
		const char* message)
		:
		Exception{std::string{"[OAL_SOURCE] "} + message}
	{
	}
}; // OalSourceException

class OalSourceNullException :
	public OalSourceException
{
public:
	explicit OalSourceNullException()
		:
		OalSourceException{"Null value."}
	{
	}
}; // OalSourceNullException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void OalSource::initialize(
	const OalSourceInitParam& param)
{
	uninitialize();

	if (param.mix_sample_rate <= 0)
	{
		throw OalSourceException{"Mix sample rate out of range."};
	}

	if (param.mix_sample_count <= 0)
	{
		throw OalSourceException{"Mix sample count out of range."};
	}

	oal_symbols_ = not_null<OalSourceNullException>(param.oal_symbols);

	streaming_mix_sample_count_ = param.mix_sample_count;
	streaming_mix_buffer_.resize(streaming_mix_sample_count_ * sample_size);

	initialize_al_resources();

	is_initialized_ = true;
}

bool OalSource::is_initialized() const noexcept
{
	return is_initialized_;
}

void OalSource::uninitialize()
{
	close();

	is_initialized_ = false;

	al_source_resource_.reset();

	static_al_buffer_resource_.reset();

	for (auto& streaming_al_buffer_resource : streaming_al_buffer_resources_)
	{
		streaming_al_buffer_resource.reset();
	}
}

void OalSource::open(
	const OalSourceOpenStaticParam& param)
{
	ensure_is_initialized();

	close();

	if (param.sample_rate <= 0)
	{
		throw OalSourceException{"Sample rate out of range."};
	}

	not_null<OalSourceNullException>(param.data);

	if (param.data_size < 0)
	{
		throw OalSourceException{"Data size out of range."};
	}

	is_2d_ = param.is_2d;

	set_al_relative();
	set_default_al_position();
	set_default_al_velocity();

	detach_static_al_buffer();
	set_static_al_buffer_data(param);
	attach_static_al_buffer();

	is_open_ = true;
	is_streaming_ = false;
	is_started_ = false;
	is_paused_ = false;
	is_finished_ = false;
}

void OalSource::open(
	const OalSourceOpenStreamingParam& param)
{
	ensure_is_initialized();

	close();

	if (param.sample_rate <= 0)
	{
		throw OalSourceException{"Sample rate out of range."};
	}

	if (!((param.caching_sound && !param.uncaching_sound) ||
		(!param.caching_sound && param.uncaching_sound)))
	{
		throw OalSourceException{"Caching and uncaching sounds are mutual exclusive."};
	}

	if (param.is_looping && streaming_caching_sound_)
	{
		throw OalSourceException{"Looping caching sound not supported."};
	}

	const auto al_queued_buffer_count = get_al_queued_buffer_count();

	if (al_queued_buffer_count != 0)
	{
		unqueue_al_buffers(al_queued_buffer_count, streaming_al_queue_.data());
	}

	is_2d_ = param.is_2d;
	is_looping_ = param.is_looping;
	streaming_sample_rate_ = param.sample_rate;
	streaming_caching_sound_ = param.caching_sound;
	streaming_uncaching_sound_ = param.uncaching_sound;
	streaming_caching_sample_offset_ = 0;
	streaming_caching_sample_count_ = (streaming_caching_sound_ ? streaming_caching_sound_->sample_count : 0);

	streaming_mix_oal_buffer_func_ = (
		streaming_uncaching_sound_ ?
		&OalSource::streaming_mix_uncaching_sound :
		&OalSource::streaming_mix_caching_sound
	);

	set_al_relative();
	set_default_al_position();
	set_default_al_velocity();

	detach_static_al_buffer();
	set_streaming_al_buffer_defaults();

	is_open_ = true;
	is_streaming_ = true;
	is_started_ = false;
	is_paused_ = false;
	is_finished_ = false;
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
	if (!is_open_ || is_paused_ || is_finished_)
	{
		return false;
	}

	switch (get_al_state())
	{
		case AL_PLAYING:
			return true;

		default:
			is_finished_ = true;
			return false;
	}
}

bool OalSource::is_finished() const
{
	return is_finished_;
}

void OalSource::set_volume(
	float volume)
{
	ensure_is_open();

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcef);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcef(al_source_resource_.get(), AL_GAIN, volume);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_position(
	float x,
	float y,
	float z)
{
	ensure_is_open();

	set_al_position(x, y, z);
}

void OalSource::set_reference_distance(
	float reference_distance)
{
	ensure_is_open();

	set_al_reference_distance(reference_distance);
}

void OalSource::play()
{
	ensure_is_open();

	if (is_started_)
	{
		return;
	}

	is_started_ = true;

	if (is_streaming_)
	{
		play_streaming();
	}
	else
	{
		play_static();
	}
}

void OalSource::pause()
{
	ensure_is_started();

	if (is_paused_ || is_finished_)
	{
		return;
	}

	is_paused_ = true;

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcePause);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcePause(al_source_resource_.get());
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::resume()
{
	ensure_is_started();

	if (!is_paused_ || is_finished_)
	{
		return;
	}

	is_paused_ = false;

	al_play();
}

void OalSource::stop()
{
	ensure_is_started();

	if (is_finished_)
	{
		return;
	}

	is_paused_ = false;
	is_finished_ = true;

	al_stop();
}

void OalSource::mix()
{
	ensure_is_started();

	if (is_paused_ || is_finished_)
	{
		return;
	}

	if (is_streaming_)
	{
		streaming_mix();
	}
	else
	{
		static_mix();
	}
}

void OalSource::close()
{
	is_open_ = false;

	if (!al_source_resource_.is_empty())
	{
		al_stop();
	}
}

void OalSource::initialize_al_resources()
{
	static_al_buffer_resource_ = make_oal_buffer(oal_symbols_);

	for (auto& streaming_al_buffer_resource : streaming_al_buffer_resources_)
	{
		streaming_al_buffer_resource = make_oal_buffer(oal_symbols_);
	}

	al_source_resource_ = make_oal_source(oal_symbols_);
}

void OalSource::ensure_is_initialized() const
{
	if (!is_initialized_)
	{
		throw OalSourceException{"Not initialized."};
	}
}

void OalSource::ensure_is_open() const
{
	if (!is_open_)
	{
		throw OalSourceException{"Not open."};
	}
}

void OalSource::ensure_is_started() const
{
	if (!is_started_)
	{
		throw OalSourceException{"Not started."};
	}
}

int OalSource::get_al_state() const
{
	auto al_state = ALint{};

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alGetSourcei);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alGetSourcei(al_source_resource_.get(), AL_SOURCE_STATE, &al_state);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);

	return static_cast<int>(al_state);
}

void OalSource::al_play()
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcePlay);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcePlay(al_source_resource_.get());
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::al_stop()
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourceStop);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourceStop(al_source_resource_.get());
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

int OalSource::get_al_queued_buffer_count() const
{
	auto al_queued_buffer_count = ALint{};

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alGetSourcei);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_QUEUED, &al_queued_buffer_count);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);

	return al_queued_buffer_count;
}

void OalSource::enqueue_al_buffer(
	ALuint al_buffer)
{
	assert(al_buffer != 0);

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourceQueueBuffers);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourceQueueBuffers(al_source_resource_.get(), 1, &al_buffer);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::unqueue_al_buffers(
	int buffer_count,
	ALuint* al_buffer_names)
{
	assert(buffer_count >= 0 && buffer_count <= oal_source_max_streaming_buffers);
	assert(al_buffer_names);

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourceUnqueueBuffers);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourceUnqueueBuffers(al_source_resource_.get(), buffer_count, al_buffer_names);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_relative()
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcei);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcei(al_source_resource_.get(), AL_SOURCE_RELATIVE, is_2d_);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_position(
	ALfloat x,
	ALfloat y,
	ALfloat z)
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSource3f);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSource3f(al_source_resource_.get(), AL_POSITION, x, y, z);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_default_al_position()
{
	set_al_position(0.0F, 0.0F, 0.0F);
}

void OalSource::set_al_velocity(
	ALfloat x,
	ALfloat y,
	ALfloat z)
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSource3f);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSource3f(al_source_resource_.get(), AL_VELOCITY, x, y, z);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_default_al_velocity()
{
	set_al_velocity(0.0F, 0.0F, 0.0F);
}

void OalSource::set_al_reference_distance(
	ALfloat reference_distance)
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcef);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcef(al_source_resource_.get(), AL_REFERENCE_DISTANCE, reference_distance);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::attach_static_al_buffer()
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcei);

	static_cast<void>(oal_symbols_->alGetError());

	oal_symbols_->alSourcei(
		al_source_resource_.get(),
		AL_BUFFER,
		static_cast<ALint>(static_al_buffer_resource_.get())
	);

	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::detach_static_al_buffer()
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alSourcei);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alSourcei(al_source_resource_.get(), AL_BUFFER, 0);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_static_al_buffer_data(
	const OalSourceOpenStaticParam& param)
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alBufferData);

	static_cast<void>(oal_symbols_->alGetError());

	oal_symbols_->alBufferData(
		static_al_buffer_resource_.get(),
		param.is_8_bit ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16,
		param.data,
		param.data_size,
		param.sample_rate
	);

	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(
	ALint al_buffer,
	int sample_count,
	OalSourceSample* samples)
{
	assert(al_buffer != 0);
	assert(sample_count > 0);
	assert(samples);

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alBufferData);

	const auto buffer_size = sample_count * sample_size;

	static_cast<void>(oal_symbols_->alGetError());

	oal_symbols_->alBufferData(
		al_buffer,
		AL_FORMAT_MONO16,
		samples,
		buffer_size,
		streaming_sample_rate_
	);

	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(
	ALint al_buffer)
{
	set_streaming_al_buffer_data(al_buffer, streaming_mix_sample_count_, streaming_mix_buffer_.data());
}

void OalSource::set_streaming_al_buffer_data(
	ALint al_buffer,
	int sample_count)
{
	set_streaming_al_buffer_data(al_buffer, sample_count, streaming_mix_buffer_.data());
}

void OalSource::set_streaming_al_buffer_defaults()
{
	std::uninitialized_fill(streaming_mix_buffer_.begin(), streaming_mix_buffer_.end(), OalSourceSample{});

	for (auto i = 0; i < oal_source_max_streaming_buffers; ++i)
	{
		const auto al_buffer = streaming_al_buffer_resources_[i].get();
		streaming_al_queue_[i] = al_buffer;

		set_streaming_al_buffer_data(al_buffer);
	}
}

void OalSource::play_static()
{
	al_play();
}

void OalSource::play_streaming()
{
}

void OalSource::static_mix()
{
	switch (get_al_state())
	{
		case AL_PLAYING:
			break;

		default:
			is_finished_ = true;
			break;
	}
}

bool OalSource::streaming_mix_uncaching_sound(
	ALuint al_buffer)
{
	if (streaming_uncaching_sound_->queue_size <= 0)
	{
		return false;
	}

	streaming_uncaching_sound_->queue_size -= 1;

	const auto streaming_max_mix_sample_count = oal_source_max_streaming_buffers * streaming_mix_sample_count_;

	if (streaming_uncaching_sound_->read_sample_offset >= streaming_max_mix_sample_count)
	{
		streaming_uncaching_sound_->read_sample_offset = 0;
	}

	set_streaming_al_buffer_data(
		al_buffer,
		streaming_mix_sample_count_,
		&streaming_uncaching_sound_->samples[streaming_uncaching_sound_->read_sample_offset]
	);

	streaming_uncaching_sound_->read_sample_offset += streaming_mix_sample_count_;

	enqueue_al_buffer(al_buffer);

	return true;
}

bool OalSource::streaming_mix_caching_sound(
	ALuint al_buffer)
{
	const auto remain_sample_count = streaming_caching_sample_count_ - streaming_caching_sample_offset_;

	if (remain_sample_count == 0)
	{
		return false;
	}

	const auto sample_count = std::min(remain_sample_count, streaming_mix_sample_count_);

	set_streaming_al_buffer_data(
		al_buffer,
		sample_count,
		&streaming_caching_sound_->samples[streaming_caching_sample_offset_]
	);

	enqueue_al_buffer(al_buffer);

	streaming_caching_sample_offset_ += sample_count;

	return true;
}

void OalSource::streaming_mix()
{
	auto al_queue_size = ALint{};
	auto al_mixed_size = ALint{};

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alGetSourcei);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_QUEUED, &al_queue_size);
	oal_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_PROCESSED, &al_mixed_size);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);

	if (al_mixed_size > 0)
	{
		unqueue_al_buffers(al_mixed_size, streaming_al_queue_.data());

		if (al_mixed_size != oal_source_max_streaming_buffers)
		{
			std::rotate(
				streaming_al_queue_.begin(),
				std::next(streaming_al_queue_.begin(), al_mixed_size),
				streaming_al_queue_.end()
			);
		}

		al_queue_size -= al_mixed_size;
	}

	auto buffer_index = al_queue_size;

	while (al_queue_size < oal_source_max_streaming_buffers)
	{
		const auto al_buffer = streaming_al_queue_[buffer_index];
		const auto mix_result = (this->*streaming_mix_oal_buffer_func_)(al_buffer);

		if (!mix_result)
		{
			break;
		}

		buffer_index += 1;
		al_queue_size += 1;
	}

	if (al_queue_size == 0)
	{
		is_finished_ = true;
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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
