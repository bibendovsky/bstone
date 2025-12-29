/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include <iterator>
#include <string>
#include <string_view>
#include "SDL3/SDL.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_audio_mgr_sdl.h"
#include "bstone_sys_event_mgr_sdl.h"
#include "bstone_sys_exception_sdl.h"
#include "bstone_sys_video_mgr_sdl.h"
#include "bstone_sys_system_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlSystemMgr final : public SystemMgr
{
public:
	SdlSystemMgr(Logger& logger);
	SdlSystemMgr(const SdlSystemMgr&) = delete;
	SdlSystemMgr& operator=(const SdlSystemMgr&) = delete;
	~SdlSystemMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	AudioMgrUPtr audio_mgr_{};
	EventMgrUPtr event_mgr_{};
	VideoMgrUPtr video_mgr_{};

private:
	Logger& do_get_logger() override;
	AudioMgr& do_get_audio_mgr() override;
	EventMgr& do_get_event_mgr() override;
	VideoMgr& do_get_video_mgr() override;

private:
	static MemoryResource& get_memory_resource();

	void log_version(int sdl_version, std::string_view version_name);
	void log_compiled_version();
	void log_linked_version();
	void log_versions();
	void log_info() noexcept;
};

// ==========================================================================

SdlSystemMgr::SdlSystemMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("<<< Start up SDL system manager.");

	log_versions();
	sdl_ensure_result(SDL_Init(0));
	sdl_ensure_result(SDL_SetHint(SDL_HINT_VIDEO_SYNC_WINDOW_OPERATIONS, "1"));

	audio_mgr_ = make_sdl_audio_mgr(logger_);
	event_mgr_ = make_sdl_event_mgr(logger_);
	video_mgr_ = make_sdl_video_mgr(logger_);

	logger_.log_information(">>> SDL system manager started up.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlSystemMgr::~SdlSystemMgr()
{
	logger_.log_information("Shut down SDL system manager.");

	video_mgr_ = nullptr;
	event_mgr_ = nullptr;
	audio_mgr_ = nullptr;

	SDL_Quit();
}

void* SdlSystemMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlSystemMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

Logger& SdlSystemMgr::do_get_logger()
{
	return logger_;
}

AudioMgr& SdlSystemMgr::do_get_audio_mgr()
{
	return *audio_mgr_;
}

EventMgr& SdlSystemMgr::do_get_event_mgr()
{
	return *event_mgr_;
}

VideoMgr& SdlSystemMgr::do_get_video_mgr()
{
	return *video_mgr_;
}

MemoryResource& SdlSystemMgr::get_memory_resource()
{
	static SinglePoolResource<SdlSystemMgr> memory_pool{};

	return memory_pool;
}

void SdlSystemMgr::log_version(int sdl_version, std::string_view version_name)
{
	char major_chars[3];
	const auto major_size =
		to_chars(SDL_VERSIONNUM_MAJOR(sdl_version), std::begin(major_chars), std::end(major_chars)) - major_chars;

	char minor_chars[3];
	const auto minor_size =
		to_chars(SDL_VERSIONNUM_MINOR(sdl_version), std::begin(minor_chars), std::end(minor_chars)) - minor_chars;

	char patch_chars[3];
	const auto patch_size =
		to_chars(SDL_VERSIONNUM_MICRO(sdl_version), std::begin(patch_chars), std::end(patch_chars)) - patch_chars;

	auto version_string = std::string{};
	version_string.reserve(32);
	version_string.append(version_name.data(), version_name.size());
	version_string += " version: ";
	version_string.append(major_chars, static_cast<std::size_t>(major_size));
	version_string += '.';
	version_string.append(minor_chars, static_cast<std::size_t>(minor_size));
	version_string += '.';
	version_string.append(patch_chars, static_cast<std::size_t>(patch_size));

	logger_.log_information(version_string.c_str());
}

void SdlSystemMgr::log_compiled_version()
{
	constexpr int sdl_version = SDL_VERSION;
	log_version(sdl_version, "Compiled");
}

void SdlSystemMgr::log_linked_version()
{
	const int sdl_version = SDL_GetVersion();
	log_version(sdl_version, "Linked");
}

void SdlSystemMgr::log_versions()
{
	log_compiled_version();
	log_linked_version();
}

void SdlSystemMgr::log_info() noexcept
try
{
	log_versions();
}
catch (...) {}

} // namespace

// ==========================================================================

SystemMgrUPtr make_system_mgr(Logger& logger)
try {
	return std::make_unique<SdlSystemMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
