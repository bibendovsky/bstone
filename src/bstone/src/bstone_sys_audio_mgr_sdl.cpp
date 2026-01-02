/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager (SDL)

#include "bstone_sys_audio_mgr_sdl.h"
#include "bstone_scope_exit.h"
#include "bstone_string_builder.h"
#include "bstone_sys_audio_mgr_null.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_polling_audio_device_sdl.h"
#include "bstone_sys_sdl_subsystem.h"
#include <exception>
#include "SDL3/SDL_audio.h"

namespace bstone::sys {

namespace {

class AudioMgrSdl final : public AudioMgr
{
public:
	explicit AudioMgrSdl(Logger& logger);
	AudioMgrSdl(const AudioMgrSdl&) = delete;
	AudioMgrSdl& operator=(const AudioMgrSdl&) = delete;
	~AudioMgrSdl() override;

private:
	Logger& logger_;
	SdlSubsystem sdl_subsystem_{};

	bool do_is_initialized() const override;
	PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) override;

	static void log_drivers(StringBuilder& formatter);
	static void log_devices(StringBuilder& formatter);
	void log_info();
};

// --------------------------------------

AudioMgrSdl::AudioMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL audio manager.");
	SdlSubsystem sdl_subsystem{SDL_INIT_AUDIO};
	log_info();
	sdl_subsystem_.swap(sdl_subsystem);
	logger_.log_information("SDL audio manager has started.");
}

AudioMgrSdl::~AudioMgrSdl()
{
	logger_.log_information("Shut down SDL audio manager.");
}

bool AudioMgrSdl::do_is_initialized() const
{
	return true;
}

PollingAudioDeviceUPtr AudioMgrSdl::do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param)
{
	return make_polling_audio_device_sdl(logger_, param);
}

void AudioMgrSdl::log_drivers(StringBuilder& formatter)
{
	formatter.reset_indent();
	formatter.add_line("Built-in drivers:");
	formatter.increase_indent();
	const int driver_count = SDL_GetNumAudioDrivers();
	for (int i_driver = 0; i_driver < driver_count; ++i_driver)
	{
		const char* const sdl_driver_name = SDL_GetAudioDriver(i_driver);
		formatter.add_indented_line(sdl_driver_name != nullptr ? sdl_driver_name : "<NULL>");
	}
}

void AudioMgrSdl::log_devices(StringBuilder& formatter)
{
	const auto add_error = [&formatter]()
	{
		formatter.add("ERROR: {}", SDL_GetError());
	};
	formatter.reset_indent();
	formatter.add_line("Playback devices:");
	formatter.increase_indent();
	int device_count;
	SDL_AudioDeviceID* const sdl_audio_device_ids = SDL_GetAudioPlaybackDevices(&device_count);
	if (sdl_audio_device_ids != nullptr)
	{
		const auto scope_exit = make_scope_exit(
			[sdl_audio_device_ids]()
			{
				SDL_free(sdl_audio_device_ids);
			});
		for (int i_device = 0; i_device < device_count; ++i_device)
		{
			const SDL_AudioDeviceID sdl_audio_device_id = sdl_audio_device_ids[i_device];
			{
				formatter.add_indented("{}. ", i_device + 1);
				const char* const sdl_device_name = SDL_GetAudioDeviceName(sdl_audio_device_id);
				if (sdl_device_name != nullptr)
				{
					formatter.add(sdl_device_name);
				}
				else
				{
					add_error();
				}
				formatter.add_line();
			}
			formatter.increase_indent();
			SDL_AudioSpec sdl_audio_spec;
			int sdl_sample_frames;
			if (SDL_GetAudioDeviceFormat(sdl_audio_device_id, &sdl_audio_spec, &sdl_sample_frames))
			{
				const char* const sdl_format_name = SDL_GetAudioFormatName(sdl_audio_spec.format);
				formatter.add_indented_line("Format: {}", sdl_format_name);
				formatter.add_indented_line("Channel count: {}", sdl_audio_spec.channels);
				formatter.add_indented_line("Sample rate: {} Hz", sdl_audio_spec.freq);
				formatter.add_indented_line("Buffer size: {} sample frames", sdl_sample_frames);
			}
			else
			{
				formatter.add_indent();
				add_error();
				formatter.add_line();
			}
		}
	}
	else
	{
		formatter.add_indent();
		add_error();
		formatter.add_line();
	}
}

void AudioMgrSdl::log_info()
{
	StringBuilder formatter{};
	formatter.reserve(2048);
	log_drivers(formatter);
	log_devices(formatter);
	logger_.log_information(formatter.get_string().c_str());
}

} // namespace

// ======================================

AudioMgrUPtr make_audio_mgr_sdl(Logger& logger)
try
{
	return std::make_unique<AudioMgrSdl>(logger);
}
catch (const std::exception& exception)
{
	logger.log_error(exception.what());
	return make_audio_mgr_null(logger);
}

} // namespace bstone::sys
