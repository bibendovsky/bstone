/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_audio_mgr_sdl3.h"

#include <charconv>
#include <string>

#include "SDL3/SDL_audio.h"

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_sys_audio_mgr_null.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_exception_sdl3.h"
#include "bstone_sys_polling_audio_device_sdl3.h"
#include "bstone_sys_sdl3_subsystem.h"

namespace bstone {
namespace sys {

namespace {

class Sdl3AudioMgr final : public AudioMgr
{
public:
	Sdl3AudioMgr(Logger& logger);
	Sdl3AudioMgr(const Sdl3AudioMgr&) = delete;
	Sdl3AudioMgr& operator=(const Sdl3AudioMgr&) = delete;
	~Sdl3AudioMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	Sdl3Subsystem sdl3_subsystem_{};
	bool is_initialized_{};

private:
	bool do_is_initialized() const noexcept override;

	PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) override;

private:
	void log_int(int value, std::string& message);
	void log_drivers();
	void log_devices();
	void log_info() noexcept;
};

// ==========================================================================

using Sdl3AudioMgrPool = SinglePoolResource<Sdl3AudioMgr>;
Sdl3AudioMgrPool sdl3_audio_mgr_pool{};

// ==========================================================================

Sdl3AudioMgr::Sdl3AudioMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL audio manager.");

	auto sdl3_subsystem = Sdl3Subsystem{SDL_INIT_AUDIO};
	log_info();
	sdl3_subsystem_.swap(sdl3_subsystem);
	is_initialized_ = true;

	logger_.log_information(">>> SDL audio manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl3AudioMgr::~Sdl3AudioMgr()
{
	logger_.log_information("Shut down SDL audio manager.");
}

void* Sdl3AudioMgr::operator new(std::size_t size)
try {
	return sdl3_audio_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl3AudioMgr::operator delete(void* ptr)
{
	sdl3_audio_mgr_pool.deallocate(ptr);
}

bool Sdl3AudioMgr::do_is_initialized() const noexcept
{
	return is_initialized_;
}

PollingAudioDeviceUPtr Sdl3AudioMgr::do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param)
{
	BSTONE_ASSERT(is_initialized_);

	return make_sdl3_polling_audio_device(logger_, param);
}

void Sdl3AudioMgr::log_int(int value, std::string& message)
{
	char chars[11];
	if (const auto [ptr, ec] = std::to_chars(std::begin(chars), std::end(chars), value);
		ec == std::errc{})
	{
		message.append(chars, static_cast<std::size_t>(ptr - chars));
	}
	else
	{
		message.append("???");
	}
}

void Sdl3AudioMgr::log_drivers()
{
	std::string message{};
	message.reserve(1024);
	message += "Built-in drivers:\n";
	const int driver_count = SDL_GetNumAudioDrivers();
	for (int i_driver = 0; i_driver < driver_count; ++i_driver)
	{
		const char* const sdl_driver_name = SDL_GetAudioDriver(i_driver);
		const char* const driver_name = sdl_driver_name != nullptr ? sdl_driver_name : "<NULL>";
		message += "  ";
		message += driver_name;
		message += '\n';
	}
	logger_.log_information(message.c_str());
}

void Sdl3AudioMgr::log_devices()
{
	std::string message{};
	message.reserve(2048);
	message += "Playback devices:\n";
	int device_count;
	SDL_AudioDeviceID* const sdl_audio_device_ids = SDL_GetAudioPlaybackDevices(&device_count);
	if (sdl_audio_device_ids != nullptr)
	{
		for (int i_device = 0; i_device < device_count; ++i_device)
		{
			const SDL_AudioDeviceID sdl_audio_device_id = sdl_audio_device_ids[i_device];
			{
				message += "  ";
				log_int(i_device + 1, message);
				message += ".\n";
			}
			{
				message += "    Name: ";
				const char* const sdl_device_name = SDL_GetAudioDeviceName(sdl_audio_device_id);
				if (sdl_device_name != nullptr)
				{
					message += sdl_device_name;
				}
				else
				{
					message += "[ERROR] ";
					message += SDL_GetError();
				}
				message += '\n';
			}
			message += "    Specification:\n";
			SDL_AudioSpec sdl_audio_spec;
			int sdl_sample_frames;
			if (SDL_GetAudioDeviceFormat(sdl_audio_device_id, &sdl_audio_spec, &sdl_sample_frames))
			{
				{
					message += "      Format: ";
					const char* const sdl_format_name = SDL_GetAudioFormatName(sdl_audio_spec.format);
					message += sdl_format_name;
					message += '\n';
				}
				{
					message += "      Channel count: ";
					log_int(sdl_audio_spec.channels, message);
					message += '\n';
				}
				{
					message += "      Sample rate: ";
					log_int(sdl_audio_spec.freq, message);
					message += " Hz\n";
				}
				{
					message += "      Buffer size: ";
					log_int(sdl_sample_frames, message);
					message += " sample frames\n";
				}
			}
			else
			{
				message += "      [ERROR] ";
				message += SDL_GetError();
				message += '\n';
			}
		}
	}
	else
	{
		message += "  [ERROR] ";
		message += SDL_GetError();
	}
	logger_.log_information(message.c_str());
}

void Sdl3AudioMgr::log_info() noexcept
try {
	log_drivers();
	log_devices();
}
catch (...) {}

} // namespace

// ==========================================================================

AudioMgrUPtr make_sdl3_audio_mgr(Logger& logger)
try
{
	return std::make_unique<Sdl3AudioMgr>(logger);
}
catch (...)
{
	return make_null_audio_mgr(logger);
}

} // namespace sys
} // namespace bstone
