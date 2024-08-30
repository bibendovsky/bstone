/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_audio_mgr_sdl2.h"

#include <iterator>
#include <string>

#include "SDL.h"

#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_sys_audio_mgr_null.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_polling_audio_device_sdl2.h"
#include "bstone_sys_sdl2_subsystem.h"

namespace bstone {
namespace sys {

namespace {

class Sdl2AudioMgr final : public AudioMgr
{
public:
	Sdl2AudioMgr(Logger& logger);
	Sdl2AudioMgr(const Sdl2AudioMgr&) = delete;
	Sdl2AudioMgr& operator=(const Sdl2AudioMgr&) = delete;
	~Sdl2AudioMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	Sdl2Subsystem sdl2_subsystem_{};
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

using Sdl2AudioMgrPool = SinglePoolResource<Sdl2AudioMgr>;
Sdl2AudioMgrPool sdl2_audio_mgr_pool{};

// ==========================================================================

Sdl2AudioMgr::Sdl2AudioMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL audio manager.");

	auto sdl2_subsystem = Sdl2Subsystem{SDL_INIT_AUDIO};
	log_info();
	sdl2_subsystem_.swap(sdl2_subsystem);
	is_initialized_ = true;

	logger_.log_information(">>> SDL audio manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2AudioMgr::~Sdl2AudioMgr()
{
	logger_.log_information("Shut down SDL audio manager.");
}

void* Sdl2AudioMgr::operator new(std::size_t size)
try {
	return sdl2_audio_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2AudioMgr::operator delete(void* ptr)
{
	sdl2_audio_mgr_pool.deallocate(ptr);
}

bool Sdl2AudioMgr::do_is_initialized() const noexcept
{
	return is_initialized_;
}

PollingAudioDeviceUPtr Sdl2AudioMgr::do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param)
{
	BSTONE_ASSERT(is_initialized_);

	return make_sdl2_polling_audio_device(logger_, param);
}

void Sdl2AudioMgr::log_int(int value, std::string& message)
{
	char chars[11];
	const auto digit_count = to_chars(value, std::begin(chars), std::end(chars)) - chars;
	message.append(chars, static_cast<std::size_t>(digit_count));
}

void Sdl2AudioMgr::log_drivers()
{
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
		logger_.log_information(message.c_str());
	}
}

void Sdl2AudioMgr::log_devices()
{
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

		message.clear();
		message += "  ";
		log_int(i + 1, message);
		message += ". \"";
		message += (sdl_device_name != nullptr ? sdl_device_name : "???");
		message += '"';
		logger_.log_information(message.c_str());

#if SDL_VERSION_ATLEAST(2, 0, 16)
		if (SDL_GetAudioDeviceSpec(i, SDL_FALSE, &sdl_spec) == 0)
		{
			//
			message.clear();
			message += "  Frequency: ";
			log_int(sdl_spec.freq, message);
			message += " Hz";
			logger_.log_information(message.c_str());

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

			//
			message.clear();
			message += "  Channels: ";
			log_int(sdl_spec.channels, message);
			logger_.log_information(message.c_str());
		}
#endif
	}
}

void Sdl2AudioMgr::log_info() noexcept
try {
	log_drivers();
	log_devices();
}
catch (...) {}

} // namespace

// ==========================================================================

AudioMgrUPtr make_sdl2_audio_mgr(Logger& logger)
try
{
	return std::make_unique<Sdl2AudioMgr>(logger);
}
catch (...)
{
	return make_null_audio_mgr(logger);
}

} // namespace sys
} // namespace bstone
