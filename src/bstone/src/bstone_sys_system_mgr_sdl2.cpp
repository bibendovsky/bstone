/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include <iterator>
#include <string>
#include "SDL.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_string_view.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_audio_mgr_sdl2.h"
#include "bstone_sys_event_mgr_sdl2.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_video_mgr_sdl2.h"
#include "bstone_sys_system_mgr.h"

#define BSTONE_SDL_2_0_2 SDL_VERSION_ATLEAST(2,0,2)
#define BSTONE_SDL_2_0_4 SDL_VERSION_ATLEAST(2,0,4)
#define BSTONE_SDL_2_0_5 SDL_VERSION_ATLEAST(2,0,5)
#define BSTONE_SDL_2_0_9 SDL_VERSION_ATLEAST(2,0,9)
#define BSTONE_SDL_2_0_14 SDL_VERSION_ATLEAST(2,0,14)
#define BSTONE_SDL_2_0_22 SDL_VERSION_ATLEAST(2,0,22)
#define BSTONE_SDL_2_24_0 SDL_VERSION_ATLEAST(2,24,0)

#if !BSTONE_SDL_2_0_2
#define SDL_RENDER_TARGETS_RESET (0x2000)
#endif

#if !BSTONE_SDL_2_0_4
#define SDL_KEYMAPCHANGED (0x304)
#define SDL_AUDIODEVICEADDED (0x1100)
#define SDL_AUDIODEVICEREMOVED (0x1101)
#define SDL_RENDER_DEVICE_RESET (0x2001)
#endif

#if !BSTONE_SDL_2_0_5
#define SDL_DROPTEXT (0x1001)
#define SDL_DROPBEGIN (0x1002)
#define SDL_DROPCOMPLETE (0x1003)
#endif

#if !BSTONE_SDL_2_0_9
#define SDL_DISPLAYEVENT (0x150)
#define SDL_SENSORUPDATE (0x1200)
#endif

#if !BSTONE_SDL_2_0_14
#define SDL_LOCALECHANGED (0x107)
#define SDL_CONTROLLERTOUCHPADDOWN (0x656)
#define SDL_CONTROLLERTOUCHPADMOTION (0x657)
#define SDL_CONTROLLERTOUCHPADUP (0x658)
#define SDL_CONTROLLERSENSORUPDATE (0x659)
#endif

#if !BSTONE_SDL_2_0_22
#define SDL_TEXTEDITING_EXT (0x305)
#endif

#if !BSTONE_SDL_2_24_0
#define SDL_JOYBATTERYUPDATED (0x607)
#endif

namespace bstone {
namespace sys {

namespace {

class Sdl2SystemMgr final : public SystemMgr
{
public:
	Sdl2SystemMgr(Logger& logger);
	Sdl2SystemMgr(const Sdl2SystemMgr&) = delete;
	Sdl2SystemMgr& operator=(const Sdl2SystemMgr&) = delete;
	~Sdl2SystemMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	AudioMgrUPtr audio_mgr_{};
	EventMgrUPtr event_mgr_{};
	VideoMgrUPtr video_mgr_{};

private:
	AudioMgr& do_get_audio_mgr() override;
	EventMgr& do_get_event_mgr() override;
	VideoMgr& do_get_video_mgr() override;

private:
	static MemoryResource& get_memory_resource();

	static void configure_event_types() noexcept;

	void log_version(const SDL_version& sdl_version, StringView version_name);
	void log_compiled_version();
	void log_linked_version();
	void log_versions();
	void log_info() noexcept;
};

// ==========================================================================

Sdl2SystemMgr::Sdl2SystemMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL system manager.");

	log_versions();
	configure_event_types();
	sdl2_ensure_result(SDL_Init(0));

	audio_mgr_ = make_sdl2_audio_mgr(logger_);
	event_mgr_ = make_sdl2_event_mgr(logger_);
	video_mgr_ = make_sdl2_video_mgr(logger_);

	logger_.log_information(">>> SDL system manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2SystemMgr::~Sdl2SystemMgr()
{
	logger_.log_information("Shut down SDL system manager.");

	video_mgr_ = nullptr;
	event_mgr_ = nullptr;
	audio_mgr_ = nullptr;

	SDL_Quit();
}

void* Sdl2SystemMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2SystemMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

AudioMgr& Sdl2SystemMgr::do_get_audio_mgr()
{
	return *audio_mgr_;
}

EventMgr& Sdl2SystemMgr::do_get_event_mgr()
{
	return *event_mgr_;
}

VideoMgr& Sdl2SystemMgr::do_get_video_mgr()
{
	return *video_mgr_;
}

MemoryResource& Sdl2SystemMgr::get_memory_resource()
{
	static SinglePoolResource<Sdl2SystemMgr> memory_pool{};

	return memory_pool;
}

void Sdl2SystemMgr::configure_event_types() noexcept
{
	SDL_EventState(SDL_QUIT, SDL_ENABLE);
	SDL_EventState(SDL_APP_TERMINATING, SDL_DISABLE);
	SDL_EventState(SDL_APP_LOWMEMORY, SDL_DISABLE);
	SDL_EventState(SDL_APP_WILLENTERBACKGROUND, SDL_DISABLE);
	SDL_EventState(SDL_APP_DIDENTERBACKGROUND, SDL_DISABLE);
	SDL_EventState(SDL_APP_WILLENTERFOREGROUND, SDL_DISABLE);
	SDL_EventState(SDL_APP_DIDENTERFOREGROUND, SDL_DISABLE);

	SDL_EventState(SDL_LOCALECHANGED, SDL_DISABLE); // v2.0.14+ (0x107)

	SDL_EventState(SDL_DISPLAYEVENT, SDL_DISABLE); // v2.0.9+ (0x150)

	SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);

	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);
	SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
	SDL_EventState(SDL_KEYMAPCHANGED, SDL_DISABLE); // v2.0.4+ (0x304)
	SDL_EventState(SDL_TEXTEDITING_EXT, SDL_DISABLE); // v2.0.22+ (0x305)

	SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEWHEEL, SDL_ENABLE);

	SDL_EventState(SDL_JOYAXISMOTION, SDL_DISABLE);
	SDL_EventState(SDL_JOYBALLMOTION, SDL_DISABLE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_DISABLE);
	SDL_EventState(SDL_JOYBUTTONDOWN, SDL_DISABLE);
	SDL_EventState(SDL_JOYBUTTONUP, SDL_DISABLE);
	SDL_EventState(SDL_JOYDEVICEADDED, SDL_DISABLE);
	SDL_EventState(SDL_JOYDEVICEREMOVED, SDL_DISABLE);
	SDL_EventState(SDL_JOYBATTERYUPDATED, SDL_DISABLE); // v2.24.0+ (0x607)

	SDL_EventState(SDL_CONTROLLERAXISMOTION, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERBUTTONDOWN, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERBUTTONUP, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERDEVICEADDED, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERDEVICEREMOVED, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERDEVICEREMAPPED, SDL_DISABLE);
	SDL_EventState(SDL_CONTROLLERTOUCHPADDOWN, SDL_DISABLE); // v2.0.14+ (0x656)
	SDL_EventState(SDL_CONTROLLERTOUCHPADMOTION, SDL_DISABLE); // v2.0.14+ (0x657)
	SDL_EventState(SDL_CONTROLLERTOUCHPADUP, SDL_DISABLE); // v2.0.14+ (0x658)
	SDL_EventState(SDL_CONTROLLERSENSORUPDATE, SDL_DISABLE); // v2.0.14+ (0x659)

	SDL_EventState(SDL_FINGERDOWN, SDL_DISABLE);
	SDL_EventState(SDL_FINGERUP, SDL_DISABLE);
	SDL_EventState(SDL_FINGERMOTION, SDL_DISABLE);

	SDL_EventState(SDL_DOLLARGESTURE, SDL_DISABLE);
	SDL_EventState(SDL_DOLLARRECORD, SDL_DISABLE);
	SDL_EventState(SDL_MULTIGESTURE, SDL_DISABLE);

	SDL_EventState(SDL_CLIPBOARDUPDATE, SDL_DISABLE);

	SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
	SDL_EventState(SDL_DROPTEXT, SDL_DISABLE); // v2.0.5+ (0x1001)
	SDL_EventState(SDL_DROPBEGIN, SDL_DISABLE); // v2.0.5+ (0x1002)
	SDL_EventState(SDL_DROPCOMPLETE, SDL_DISABLE); // v2.0.5+ (0x1003)

	SDL_EventState(SDL_AUDIODEVICEADDED, SDL_ENABLE); // v2.0.4+ (0x1100)
	SDL_EventState(SDL_AUDIODEVICEREMOVED, SDL_ENABLE); // v2.0.4+ (0x1101)

	SDL_EventState(SDL_SENSORUPDATE, SDL_DISABLE); // v2.0.9+ (0x1200)

	SDL_EventState(SDL_RENDER_TARGETS_RESET, SDL_DISABLE); // v2.0.2+ (0x2000)
	SDL_EventState(SDL_RENDER_DEVICE_RESET, SDL_DISABLE); // v2.0.4+ (0x2001)
}

void Sdl2SystemMgr::log_version(const SDL_version& sdl_version, StringView version_name)
{
	char major_chars[3];
	const auto major_size =
		to_chars(sdl_version.major, std::begin(major_chars), std::end(major_chars)) - major_chars;

	char minor_chars[3];
	const auto minor_size =
		to_chars(sdl_version.minor, std::begin(minor_chars), std::end(minor_chars)) - minor_chars;

	char patch_chars[3];
	const auto patch_size =
		to_chars(sdl_version.patch, std::begin(patch_chars), std::end(patch_chars)) - patch_chars;

	auto version_string = std::string{};
	version_string.reserve(32);
	version_string.append(version_name.get_data(), static_cast<std::size_t>(version_name.get_size()));
	version_string += " version: ";
	version_string.append(major_chars, static_cast<std::size_t>(major_size));
	version_string += '.';
	version_string.append(minor_chars, static_cast<std::size_t>(minor_size));
	version_string += '.';
	version_string.append(patch_chars, static_cast<std::size_t>(patch_size));

	logger_.log_information(version_string.c_str());
}

void Sdl2SystemMgr::log_compiled_version()
{
	auto sdl_version = SDL_version{};
	SDL_VERSION(&sdl_version);
	log_version(sdl_version, "Compiled");
}

void Sdl2SystemMgr::log_linked_version()
{
	auto sdl_version = SDL_version{};
	SDL_GetVersion(&sdl_version);
	log_version(sdl_version, "Linked");
}

void Sdl2SystemMgr::log_versions()
{
	log_compiled_version();
	log_linked_version();
}

void Sdl2SystemMgr::log_info() noexcept
try
{
	log_versions();
}
catch (...) {}

} // namespace

// ==========================================================================

SystemMgrUPtr make_system_mgr(Logger& logger)
try {
	return std::make_unique<Sdl2SystemMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
