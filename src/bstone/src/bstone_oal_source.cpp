/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_oal_source.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include "bstone_assert.h"
#include "bstone_exception.h"

namespace bstone
{

void OalSource::initialize(const OalSourceInitParam& param)
try {
	uninitialize();

	if (param.mix_sample_rate <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Mix sample rate out of range.");
	}

	if (param.mix_sample_count <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Mix sample count out of range.");
	}

	if (!param.oal_al_symbols)
	{
		BSTONE_THROW_STATIC_SOURCE("Null AL symbols.");
	}

	oal_al_symbols_ = param.oal_al_symbols;
	streaming_mix_sample_count_ = param.mix_sample_count;
	streaming_mix_buffer_.resize(streaming_mix_sample_count_ * sample_size);
	initialize_al_resources();
	is_initialized_ = true;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

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

void OalSource::open(const OalSourceOpenStaticParam& param)
try {
	ensure_is_initialized();
	close();

	if (param.sample_rate <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Sample rate out of range.");
	}

	if (!param.data)
	{
		BSTONE_THROW_STATIC_SOURCE("Null data.");
	}

	if (param.data_size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Data size out of range.");
	}

	is_3d_ = param.is_3d;

	set_al_relative();
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
	is_finished_ = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalSource::open(const OalSourceOpenStreamingParam& param)
{
	ensure_is_initialized();
	close();

	if (param.sample_rate <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Sample rate out of range.");
	}

	if (!((param.caching_sound && !param.uncaching_sound) || (!param.caching_sound && param.uncaching_sound)))
	{
		BSTONE_THROW_STATIC_SOURCE("Caching and uncaching sounds are mutual exclusive.");
	}

	if (param.is_looping && param.caching_sound != nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Looping the caching sound not supported.");
	}

	const auto al_processed_buffer_count = get_al_processed_buffer_count();

	if (al_processed_buffer_count != 0)
	{
		unqueue_al_buffers(al_processed_buffer_count, streaming_al_queue_.data());
	}

	is_3d_ = param.is_3d;
	is_looping_ = param.is_looping;
	streaming_sample_rate_ = param.sample_rate;
	streaming_caching_sound_ = param.caching_sound;
	streaming_uncaching_sound_ = param.uncaching_sound;
	streaming_caching_sample_offset_ = 0;
	streaming_caching_sample_count_ = (streaming_caching_sound_ ? streaming_caching_sound_->sample_count : 0);

	streaming_mix_oal_buffer_func_ = (streaming_uncaching_sound_ ? &OalSource::streaming_mix_uncaching_sound : &OalSource::streaming_mix_caching_sound);

	set_al_relative();
	set_al_default_position();

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

void OalSource::set_gain(double gain)
{
	ensure_is_open();

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcef);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcef(al_source_resource_.get(), AL_GAIN, static_cast<ALfloat>(gain));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_position(double x, double y, double z)
{
	ensure_is_open();
	set_al_position(x, y, z);
}

void OalSource::set_reference_distance(double reference_distance)
{
	ensure_is_open();
	set_al_reference_distance(reference_distance);
}

void OalSource::set_max_distance(double max_distance)
{
	ensure_is_open();
	set_al_max_distance(max_distance);
}

void OalSource::set_rolloff_factor(double rolloff_factor)
{
	ensure_is_open();
	set_al_rolloff_factor(rolloff_factor);
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

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcePause);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcePause(al_source_resource_.get());
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
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
	static_al_buffer_resource_ = make_oal_buffer(*oal_al_symbols_);

	for (auto& streaming_al_buffer_resource : streaming_al_buffer_resources_)
	{
		streaming_al_buffer_resource = make_oal_buffer(*oal_al_symbols_);
	}

	al_source_resource_ = make_oal_source(*oal_al_symbols_);
}

void OalSource::ensure_is_initialized() const
{
	if (!is_initialized_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not initialized.");
	}
}

void OalSource::ensure_is_open() const
{
	if (!is_open_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not open.");
	}
}

void OalSource::ensure_is_started() const
{
	if (!is_started_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not started.");
	}
}

int OalSource::get_al_state() const
{
	auto al_state = ALint{};

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alGetSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alGetSourcei(al_source_resource_.get(), AL_SOURCE_STATE, &al_state);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);

	return static_cast<int>(al_state);
}

void OalSource::al_play()
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcePlay);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcePlay(al_source_resource_.get());
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::al_stop()
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourceStop);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourceStop(al_source_resource_.get());
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

int OalSource::get_al_processed_buffer_count() const
{
	auto al_buffer_count = ALint{};

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alGetSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_PROCESSED, &al_buffer_count);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);

	return al_buffer_count;
}

void OalSource::enqueue_al_buffer(ALuint al_buffer)
{
	BSTONE_ASSERT(al_buffer != 0);

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourceQueueBuffers);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourceQueueBuffers(al_source_resource_.get(), 1, &al_buffer);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::unqueue_al_buffers(int buffer_count, ALuint* al_buffer_names)
{
	BSTONE_ASSERT(buffer_count >= 0 && buffer_count <= oal_source_max_streaming_buffers);
	BSTONE_ASSERT(al_buffer_names);

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourceUnqueueBuffers);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourceUnqueueBuffers(al_source_resource_.get(), buffer_count, al_buffer_names);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_relative()
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcei(al_source_resource_.get(), AL_SOURCE_RELATIVE, is_3d_ ? AL_FALSE : AL_TRUE);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_position(double x, double y, double z)
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSource3f);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSource3f(al_source_resource_.get(), AL_POSITION, static_cast<ALfloat>(x), static_cast<ALfloat>(y), static_cast<ALfloat>(z));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_position()
{
	set_al_position(0.0, 0.0, 0.0);
}

void OalSource::set_al_reference_distance(double reference_distance)
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcef);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcef(al_source_resource_.get(), AL_REFERENCE_DISTANCE, static_cast<ALfloat>(reference_distance));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_reference_distance()
{
	set_al_reference_distance(1.0);
}

void OalSource::set_al_max_distance(double max_distance)
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcef);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcef(al_source_resource_.get(), AL_MAX_DISTANCE, static_cast<ALfloat>(max_distance));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_max_distance()
{
	set_al_max_distance(FLT_MAX);
}

void OalSource::set_al_rolloff_factor(double rolloff_factor)
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcef);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcef(al_source_resource_.get(), AL_ROLLOFF_FACTOR, static_cast<ALfloat>(rolloff_factor));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_al_default_rolloff_factor()
{
	set_al_rolloff_factor(1.0);
}

void OalSource::attach_static_al_buffer()
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcei(al_source_resource_.get(), AL_BUFFER, static_cast<ALint>(static_al_buffer_resource_.get()));
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::detach_static_al_buffer()
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alSourcei(al_source_resource_.get(), AL_BUFFER, 0);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_static_al_buffer_data(const OalSourceOpenStaticParam& param)
{
	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alBufferData);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alBufferData(static_al_buffer_resource_.get(), AL_FORMAT_MONO16, param.data, param.data_size, param.sample_rate);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(ALint al_buffer, int sample_count, OalSourceSample* samples)
{
	BSTONE_ASSERT(al_buffer != 0);
	BSTONE_ASSERT(sample_count > 0);
	BSTONE_ASSERT(samples);

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alBufferData);

	const auto buffer_size = sample_count * sample_size;
	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alBufferData(al_buffer, AL_FORMAT_MONO16, samples, buffer_size, streaming_sample_rate_);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);
}

void OalSource::set_streaming_al_buffer_data(ALint al_buffer)
{
	set_streaming_al_buffer_data(al_buffer, streaming_mix_sample_count_, streaming_mix_buffer_.data());
}

void OalSource::set_streaming_al_buffer_data(ALint al_buffer, int sample_count)
{
	set_streaming_al_buffer_data(al_buffer, sample_count, streaming_mix_buffer_.data());
}

void OalSource::set_streaming_al_buffer_defaults()
{
	std::fill(streaming_mix_buffer_.begin(), streaming_mix_buffer_.end(), OalSourceSample{});

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

bool OalSource::streaming_mix_uncaching_sound(ALuint al_buffer)
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

	set_streaming_al_buffer_data(al_buffer, streaming_mix_sample_count_, &streaming_uncaching_sound_->samples[streaming_uncaching_sound_->read_sample_offset]);
	streaming_uncaching_sound_->read_sample_offset += streaming_mix_sample_count_;
	enqueue_al_buffer(al_buffer);
	return true;
}

bool OalSource::streaming_mix_caching_sound(ALuint al_buffer)
{
	const auto remain_sample_count = streaming_caching_sample_count_ - streaming_caching_sample_offset_;

	if (remain_sample_count == 0)
	{
		return false;
	}

	const auto sample_count = std::min(remain_sample_count, streaming_mix_sample_count_);

	set_streaming_al_buffer_data(al_buffer, sample_count, &streaming_caching_sound_->samples[streaming_caching_sample_offset_]);
	enqueue_al_buffer(al_buffer);
	streaming_caching_sample_offset_ += sample_count;
	return true;
}

void OalSource::streaming_mix()
{
	auto al_queue_size = ALint{};
	auto al_mixed_size = ALint{};

	BSTONE_ASSERT(oal_al_symbols_->alGetError);
	BSTONE_ASSERT(oal_al_symbols_->alGetSourcei);

	static_cast<void>(oal_al_symbols_->alGetError());
	oal_al_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_QUEUED, &al_queue_size);
	oal_al_symbols_->alGetSourcei(al_source_resource_.get(), AL_BUFFERS_PROCESSED, &al_mixed_size);
	BSTONE_ASSERT(oal_al_symbols_->alGetError() == AL_NO_ERROR);

	if (al_mixed_size > 0)
	{
		unqueue_al_buffers(al_mixed_size, streaming_al_queue_.data());

		if (al_mixed_size != oal_source_max_streaming_buffers)
		{
			std::rotate(streaming_al_queue_.begin(), std::next(streaming_al_queue_.begin(), al_mixed_size), streaming_al_queue_.end());
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

} // bstone
