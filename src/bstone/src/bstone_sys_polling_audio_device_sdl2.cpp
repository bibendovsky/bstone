/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include "SDL_audio.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_polling_audio_device_sdl2.h"
#include "bstone_exception.h"

namespace bstone {
namespace sys {

namespace {

class Sdl2PollingAudioDevice final : public PollingAudioDevice
{
public:
	Sdl2PollingAudioDevice(Logger& logger, const PollingAudioDeviceOpenParam& param);
	Sdl2PollingAudioDevice(const Sdl2PollingAudioDevice&) = delete;
	Sdl2PollingAudioDevice& operator=(const Sdl2PollingAudioDevice&) = delete;
	~Sdl2PollingAudioDevice() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	int rate_{};
	int channel_count_{};
	int frame_count_{};
	PollingAudioDeviceCallback* callback_{};
	SDL_AudioDeviceID sdl_audio_device_id_{};

private:
	int do_get_rate() const noexcept override;
	int do_get_channel_count() const noexcept override;
	int do_get_frame_count() const noexcept override;

	void do_pause(bool is_pause) override;

private:
	static MemoryResource& get_memory_resource();

	static void SDLCALL sdl_callback(void* userdata, Uint8* stream, int len);
	void callback(float* samples, int sample_count);
};

// ==========================================================================

Sdl2PollingAudioDevice::Sdl2PollingAudioDevice(Logger& logger, const PollingAudioDeviceOpenParam& param)
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

	auto desired_spec = SDL_AudioSpec{};
	desired_spec.freq = param.desired_rate;
	desired_spec.format = AUDIO_F32SYS;
	desired_spec.channels = static_cast<decltype(SDL_AudioSpec::channels)>(param.channel_count);
	desired_spec.samples = static_cast<decltype(SDL_AudioSpec::samples)>(param.desired_frame_count);
	desired_spec.callback = sdl_callback;
	desired_spec.userdata = this;

	auto effective_spec = SDL_AudioSpec{};

	sdl_audio_device_id_ = SDL_OpenAudioDevice(
		nullptr,
		SDL_FALSE,
		&desired_spec,
		&effective_spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

	if (sdl_audio_device_id_ <= 1)
	{
		sdl2_fail();
	}

	rate_ = effective_spec.freq;
	channel_count_ = effective_spec.channels;
	frame_count_ = effective_spec.samples;

	logger_.log_information(">>> SDL polling audio device started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2PollingAudioDevice::~Sdl2PollingAudioDevice()
{
	logger_.log_information("Shut down SDL polling audio device.");

	SDL_CloseAudioDevice(sdl_audio_device_id_);
}

void* Sdl2PollingAudioDevice::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2PollingAudioDevice::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

int Sdl2PollingAudioDevice::do_get_rate() const noexcept
{
	return rate_;
}

int Sdl2PollingAudioDevice::do_get_channel_count() const noexcept
{
	return channel_count_;
}

int Sdl2PollingAudioDevice::do_get_frame_count() const noexcept
{
	return frame_count_;
}

void Sdl2PollingAudioDevice::do_pause(bool is_pause)
{
	SDL_PauseAudioDevice(sdl_audio_device_id_, is_pause ? SDL_TRUE : SDL_FALSE);
}

MemoryResource& Sdl2PollingAudioDevice::get_memory_resource()
{
	static SinglePoolResource<Sdl2PollingAudioDevice> memory_pool{};

	return memory_pool;
}

void SDLCALL Sdl2PollingAudioDevice::sdl_callback(void* userdata, Uint8* stream, int len)
{
	static_cast<Sdl2PollingAudioDevice*>(userdata)->callback(
		reinterpret_cast<float*>(stream),
		len / static_cast<int>(sizeof(float)));
}

void Sdl2PollingAudioDevice::callback(float* samples, int sample_count)
try {
	callback_->invoke(samples, sample_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

PollingAudioDeviceUPtr make_sdl2_polling_audio_device(Logger& logger, const PollingAudioDeviceOpenParam& param)
try {
	return std::make_unique<Sdl2PollingAudioDevice>(logger, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
