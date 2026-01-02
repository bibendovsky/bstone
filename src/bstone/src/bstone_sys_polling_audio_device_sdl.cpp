/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Polling audio device (SDL)

#include "bstone_sys_polling_audio_device_sdl.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include <cstddef>
#include <format>
#include <string>
#include <vector>
#include "SDL3/SDL_audio.h"

namespace bstone::sys {

namespace {

class PollingAudioDeviceSdl final : public PollingAudioDevice
{
public:
	PollingAudioDeviceSdl(Logger& logger, const PollingAudioDeviceOpenParam& param);
	PollingAudioDeviceSdl(const PollingAudioDeviceSdl&) = delete;
	PollingAudioDeviceSdl& operator=(const PollingAudioDeviceSdl&) = delete;
	~PollingAudioDeviceSdl() override;

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

	int do_get_rate() const override;
	int do_get_channel_count() const override;
	int do_get_frame_count() const override;
	void do_pause(bool is_pause) override;

	static void SDLCALL callback_proxy(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
	void callback(int sdl_size);
};

// --------------------------------------

PollingAudioDeviceSdl::PollingAudioDeviceSdl(Logger& logger, const PollingAudioDeviceOpenParam& param)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL polling audio device.");
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
	SDL_AudioStream* sdl_audio_stream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
		&sdl_audio_spec,
		callback_proxy,
		this);
	if (sdl_audio_stream == nullptr)
	{
		const std::string message = std::format("[{}] {}", "SDL_OpenAudioDeviceStream", SDL_GetError());
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_audio_stream]()
		{
			if (sdl_audio_stream != nullptr)
			{
				SDL_DestroyAudioStream(sdl_audio_stream);
			}
		});
	rate_ = param.desired_rate;
	channel_count_ = param.channel_count;
	frame_count_ = param.desired_frame_count;
	audio_cache_capacity_ = sample_size * channel_count_ * frame_count_;
	audio_cache_.resize(static_cast<std::size_t>(audio_cache_capacity_));
	logger_.log_information("SDL polling audio device has started.");
	sdl_audio_stream_ = sdl_audio_stream;
	sdl_audio_stream = nullptr;
}

PollingAudioDeviceSdl::~PollingAudioDeviceSdl()
{
	logger_.log_information("Shut down SDL polling audio device.");
	SDL_DestroyAudioStream(sdl_audio_stream_);
}

int PollingAudioDeviceSdl::do_get_rate() const
{
	return rate_;
}

int PollingAudioDeviceSdl::do_get_channel_count() const
{
	return channel_count_;
}

int PollingAudioDeviceSdl::do_get_frame_count() const
{
	return frame_count_;
}

void PollingAudioDeviceSdl::do_pause(bool is_pause)
{
	if (is_pause)
	{
		if (!SDL_PauseAudioStreamDevice(sdl_audio_stream_))
		{
			const std::string message = std::format("[{}] {}", "SDL_PauseAudioStreamDevice", SDL_GetError());
			BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
		}
	}
	else
	{
		if (!SDL_ResumeAudioStreamDevice(sdl_audio_stream_))
		{
			const std::string message = std::format("[{}] {}", "SDL_ResumeAudioStreamDevice", SDL_GetError());
			BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
		}
	}
}

void SDLCALL PollingAudioDeviceSdl::callback_proxy(
	void* userdata,
	[[maybe_unused]] SDL_AudioStream* stream,
	[[maybe_unused]] int additional_amount,
	int total_amount)
{
	static_cast<PollingAudioDeviceSdl*>(userdata)->callback(total_amount);
}

void PollingAudioDeviceSdl::callback(int sdl_size)
{
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
}

} // namespace

// ======================================

PollingAudioDeviceUPtr make_polling_audio_device_sdl(Logger& logger, const PollingAudioDeviceOpenParam& param)
{
	return std::make_unique<PollingAudioDeviceSdl>(logger, param);
}

} // namespace bstone::sys
