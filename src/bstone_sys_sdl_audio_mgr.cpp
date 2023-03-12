/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include "SDL.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_sdl_exception.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_sdl_audio_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlAudioMgr final : public AudioMgr
{
public:
	SdlAudioMgr(Logger& logger);
	SdlAudioMgr(const SdlAudioMgr&) = delete;
	SdlAudioMgr& operator=(const SdlAudioMgr&) = delete;
	~SdlAudioMgr() override;

private:
	Logger& logger_;

private:
	void log_int(int value, std::string& message);
	void log_drivers();
	void log_devices();
	void log_info() noexcept;
};

// ==========================================================================

SdlAudioMgr::SdlAudioMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information();
	logger_.log_information("<<< Start up SDL audio manager.");

	sdl_ensure_result(SDL_InitSubSystem(SDL_INIT_AUDIO));
	log_info();

	logger_.log_information(">>> SDL audio manager started up.");
}
BSTONE_FUNC_STATIC_THROW_NESTED

SdlAudioMgr::~SdlAudioMgr()
{
	logger_.log_information();
	logger_.log_information("Shut down SDL audio manager.");

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SdlAudioMgr::log_int(int value, std::string& message)
{
	char chars[11];
	const auto digit_count = char_conv::to_chars(value, make_span(chars), 10);
	message.append(chars, static_cast<std::size_t>(digit_count));
}

void SdlAudioMgr::log_drivers()
{
	logger_.log_information();

	const auto driver_count = SDL_GetNumAudioDrivers();

	if (driver_count == 0)
	{
		logger_.log_information("No built-in drivers.");
		return;
	}

	auto message = std::string{};
	message.reserve(256);

	logger_.log_information("Built-in drivers:");

	for (auto i = decltype(driver_count){}; i < driver_count; ++i)
	{
		const auto sdl_driver_name = SDL_GetAudioDriver(i);

		message.clear();
		message += "  \"";
		message += (sdl_driver_name != nullptr ? sdl_driver_name : "???");
		message += '"';
		logger_.log_information(message);
	}
}

void SdlAudioMgr::log_devices()
{
	logger_.log_information();

	const auto device_count = SDL_GetNumAudioDevices(SDL_FALSE);

	if (device_count == 0)
	{
		logger_.log_information("No devices.");
		return;
	}

	auto message = std::string{};
	message.reserve(256);

	logger_.log_information("Devices:");

	for (auto i = decltype(device_count){}; i < device_count; ++i)
	{
		auto sdl_spec = SDL_AudioSpec{};
		const auto sdl_device_name = SDL_GetAudioDeviceName(i, SDL_FALSE);

		if (i != 0)
		{
			logger_.log_information("");
		}

		message.clear();
		message += "  ";
		log_int(i + 1, message);
		message += ". \"";
		message += (sdl_device_name != nullptr ? sdl_device_name : "???");
		message += '"';
		logger_.log_information(message);

#if SDL_VERSION_ATLEAST(2, 0, 16)
		if (SDL_GetAudioDeviceSpec(i, SDL_FALSE, &sdl_spec) == 0)
		{
			//
			message.clear();
			message += "  Frequency: ";
			log_int(sdl_spec.freq, message);
			message += " Hz";
			logger_.log_information(message);

			//
			message.clear();
			message += "  Format: ";
			const auto bit_depth = SDL_AUDIO_BITSIZE(sdl_spec.format);
			log_int(bit_depth, message);
			message += "-bit ";

			if (SDL_AUDIO_ISFLOAT(sdl_spec.format))
			{
				message += "floating-point";
			}
			else
			{
				message += SDL_AUDIO_ISSIGNED(sdl_spec.format) ? "signed" : "unsigned";
			}

			logger_.log_information();

			//
			message.clear();
			message += "  Channels: ";
			log_int(sdl_spec.channels, message);
			logger_.log_information(message);
		}
#endif
	}

	logger_.log_information();
}

void SdlAudioMgr::log_info() noexcept
try
{
	log_drivers();
	log_devices();
}
catch (...) {}

} // namespace

// ==========================================================================

AudioMgrUPtr make_sdl_audio_mgr(Logger& logger)
try
{
	return std::make_unique<SdlAudioMgr>(logger);
}
BSTONE_FUNC_STATIC_THROW_NESTED

} // namespace sys
} // namespace bstone
