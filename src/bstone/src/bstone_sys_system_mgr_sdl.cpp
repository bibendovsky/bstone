/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// System manager (SDL)

#include "bstone_exception.h"
#include "bstone_string_builder.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_audio_mgr_sdl.h"
#include "bstone_sys_event_mgr_sdl.h"
#include "bstone_sys_video_mgr_sdl.h"
#include "bstone_sys_system_mgr.h"
#include <string>
#include <string_view>
#include "SDL3/SDL.h"

namespace bstone::sys {

namespace {

class SystemMgrSdl final : public SystemMgr
{
public:
	explicit SystemMgrSdl(Logger& logger);
	SystemMgrSdl(const SystemMgrSdl&) = delete;
	SystemMgrSdl& operator=(const SystemMgrSdl&) = delete;
	~SystemMgrSdl() override;

private:
	Logger& logger_;
	AudioMgrUPtr audio_mgr_{};
	EventMgrUPtr event_mgr_{};
	VideoMgrUPtr video_mgr_{};

	Logger& do_get_logger() override;
	AudioMgr& do_get_audio_mgr() override;
	EventMgr& do_get_event_mgr() override;
	VideoMgr& do_get_video_mgr() override;

	void log_version(int sdl_version, std::string_view version_name, StringBuilder& formatter);
	void log_compiled_version(StringBuilder& formatter);
	void log_linked_version(StringBuilder& formatter);
	void log_versions(StringBuilder& formatter);
	void log_info();
};

// --------------------------------------

SystemMgrSdl::SystemMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL system manager.");
	log_info();
	if (!SDL_SetHint(SDL_HINT_VIDEO_SYNC_WINDOW_OPERATIONS, "1"))
	{
		BSTONE_THROW_STATIC_SOURCE("SDL_SetHint(SDL_HINT_VIDEO_SYNC_WINDOW_OPERATIONS)");
	}
	audio_mgr_ = make_audio_mgr_sdl(logger_);
	event_mgr_ = make_event_mgr_sdl(logger_);
	video_mgr_ = make_sdl_video_mgr(logger_);
	logger_.log_information("SDL system manager has started.");
}

SystemMgrSdl::~SystemMgrSdl()
{
	logger_.log_information("Shut down SDL system manager.");
	video_mgr_ = nullptr;
	event_mgr_ = nullptr;
	audio_mgr_ = nullptr;
	SDL_Quit();
}

Logger& SystemMgrSdl::do_get_logger()
{
	return logger_;
}

AudioMgr& SystemMgrSdl::do_get_audio_mgr()
{
	return *audio_mgr_;
}

EventMgr& SystemMgrSdl::do_get_event_mgr()
{
	return *event_mgr_;
}

VideoMgr& SystemMgrSdl::do_get_video_mgr()
{
	return *video_mgr_;
}

void SystemMgrSdl::log_version(int sdl_version, std::string_view version_name, StringBuilder& formatter)
{
	formatter.reset_indent();
	formatter.add_line(
		"{} version: {}.{}.{}",
		version_name,
		SDL_VERSIONNUM_MAJOR(sdl_version),
		SDL_VERSIONNUM_MINOR(sdl_version),
		SDL_VERSIONNUM_MICRO(sdl_version));
}

void SystemMgrSdl::log_compiled_version(StringBuilder& formatter)
{
	constexpr int sdl_version = SDL_VERSION;
	log_version(sdl_version, "Compiled", formatter);
}

void SystemMgrSdl::log_linked_version(StringBuilder& formatter)
{
	const int sdl_version = SDL_GetVersion();
	log_version(sdl_version, "Linked", formatter);
}

void SystemMgrSdl::log_versions(StringBuilder& formatter)
{
	log_compiled_version(formatter);
	log_linked_version(formatter);
}

void SystemMgrSdl::log_info()
{
	StringBuilder formatter{};
	formatter.reserve(256);
	log_versions(formatter);
	logger_.log_information(formatter.get_string().c_str());
}

} // namespace

// ======================================

SystemMgrUPtr make_system_mgr(Logger& logger)
{
	return std::make_unique<SystemMgrSdl>(logger);
}

} // namespace bstone::sys
