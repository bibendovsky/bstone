/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include <cstring>
#include <vector>
#include "SDL3/SDL_audio.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl3.h"
#include "bstone_sys_polling_audio_device_sdl3.h"
#include "bstone_exception.h"

namespace bstone {
namespace sys {

namespace {

class Sdl3PollingAudioDevice final : public PollingAudioDevice
{
public:
	Sdl3PollingAudioDevice(Logger& logger, const PollingAudioDeviceOpenParam& param);
	Sdl3PollingAudioDevice(const Sdl3PollingAudioDevice&) = delete;
	Sdl3PollingAudioDevice& operator=(const Sdl3PollingAudioDevice&) = delete;
	~Sdl3PollingAudioDevice() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	using Sample = float;
	static constexpr int sample_size = sizeof(Sample);
	using AudioCache = std::vector<std::byte>;

	Logger& logger_;
	int rate_{};
	int channel_count_{};
	int frame_count_{};
	PollingAudioDeviceCallback* callback_{};
	SDL_AudioDeviceID sdl_audio_device_id_{};
	SDL_AudioStream* sdl_audio_stream_{};
	AudioCache audio_cache_{};
	int audio_cache_capacity_{};
	int audio_cache_size_{};
	int audio_cache_offset_{};

private:
	int do_get_rate() const noexcept override;
	int do_get_channel_count() const noexcept override;
	int do_get_frame_count() const noexcept override;

	void do_pause(bool is_pause) override;

private:
	static MemoryResource& get_memory_resource();

	static void SDLCALL sdl_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
	void callback(int sdl_size);
};

// ==========================================================================

Sdl3PollingAudioDevice::Sdl3PollingAudioDevice(Logger& logger, const PollingAudioDeviceOpenParam& param)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL polling audio device.");

	if (param.channel_count <= 0 || param.channel_count > 255)
	{
		BSTONE_THROW_STATIC_SOURCE("Channel count out of range.");
	}

	if (param.desired_frame_count <= 0 || param.desired_frame_count > 65535)
	{
		BSTONE_THROW_STATIC_SOURCE("Frame count out of range.");
	}

	if (param.callback == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null callback.");
	}

	callback_ = param.callback;

	const SDL_AudioSpec sdl_audio_spec{
		/* format   */ SDL_AUDIO_F32,
		/* channels */ param.channel_count,
		/* freq     */ param.desired_rate,
	};
	sdl_audio_stream_ = sdl3_ensure_result(SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
		&sdl_audio_spec,
		sdl_callback,
		this));
	rate_ = param.desired_rate;
	channel_count_ = param.channel_count;
	frame_count_ = param.desired_frame_count;
	audio_cache_capacity_ = sample_size * channel_count_ * frame_count_;
	audio_cache_.resize(static_cast<std::size_t>(audio_cache_capacity_));
	logger_.log_information(">>> SDL polling audio device started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3PollingAudioDevice::~Sdl3PollingAudioDevice()
{
	logger_.log_information("Shut down SDL polling audio device.");
	if (sdl_audio_stream_ != nullptr)
	{
		SDL_DestroyAudioStream(sdl_audio_stream_);
	}
}

void* Sdl3PollingAudioDevice::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3PollingAudioDevice::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

int Sdl3PollingAudioDevice::do_get_rate() const noexcept
{
	return rate_;
}

int Sdl3PollingAudioDevice::do_get_channel_count() const noexcept
{
	return channel_count_;
}

int Sdl3PollingAudioDevice::do_get_frame_count() const noexcept
{
	return frame_count_;
}

void Sdl3PollingAudioDevice::do_pause(bool is_pause)
{
	if (is_pause)
	{
		SDL_PauseAudioStreamDevice(sdl_audio_stream_);
	}
	else
	{
		SDL_ResumeAudioStreamDevice(sdl_audio_stream_);
	}
}

MemoryResource& Sdl3PollingAudioDevice::get_memory_resource()
{
	static SinglePoolResource<Sdl3PollingAudioDevice> memory_pool{};

	return memory_pool;
}

void SDLCALL Sdl3PollingAudioDevice::sdl_callback(
	void* userdata,
	[[maybe_unused]] SDL_AudioStream* stream,
	[[maybe_unused]] int additional_amount,
	int total_amount)
{
	static_cast<Sdl3PollingAudioDevice*>(userdata)->callback(total_amount);
}

void Sdl3PollingAudioDevice::callback(int sdl_size)
try {
	if (sdl_size <= 0)
	{
		return;
	}
	for (int sdl_offset = 0; sdl_offset < sdl_size; )
	{
		if (audio_cache_offset_ >= audio_cache_size_)
		{
			const int sample_count = audio_cache_capacity_ / (channel_count_ * sample_size);
			callback_->invoke(reinterpret_cast<float*>(audio_cache_.data()), sample_count);
			audio_cache_offset_ = 0;
			audio_cache_size_ = audio_cache_capacity_;
		}
		const int put_count = std::min(sdl_size - sdl_offset, audio_cache_size_ - audio_cache_offset_);
		SDL_PutAudioStreamData(
			sdl_audio_stream_,
			audio_cache_.data() + audio_cache_offset_,
			put_count);
		sdl_offset += put_count;
		audio_cache_offset_ += put_count;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

PollingAudioDeviceUPtr make_sdl3_polling_audio_device(Logger& logger, const PollingAudioDeviceOpenParam& param)
try {
	return std::make_unique<Sdl3PollingAudioDevice>(logger, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
