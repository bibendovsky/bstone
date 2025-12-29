/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl.h"
#include "bstone_sys_mouse_mgr_sdl.h"

namespace bstone {
namespace sys {

namespace {

class SdlMouseMgr final : public MouseMgr
{
public:
	SdlMouseMgr(Logger& logger);
	~SdlMouseMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;

private:
	void do_set_relative_mode(bool is_enable) override;

private:
	static MemoryResource& get_memory_resource();
};

// ==========================================================================

SdlMouseMgr::SdlMouseMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL mouse manager.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlMouseMgr::~SdlMouseMgr()
{
	logger_.log_information("Shut down SDL mouse manager.");
}

void* SdlMouseMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlMouseMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void SdlMouseMgr::do_set_relative_mode(bool is_enable)
{
	// TODO Move it somewhere else.
	int window_count;
	SDL_Window** const sdl_windows = SDL_GetWindows(&window_count);
	if (sdl_windows != nullptr)
	{
		if (window_count > 0)
		{
			if (window_count != 1)
			{
				BSTONE_THROW_STATIC_SOURCE("Too many windows.");
			}
			SDL_Window* const sdl_window = sdl_windows[0];
			SDL_SetWindowRelativeMouseMode(sdl_window, is_enable);
		}
		SDL_free(sdl_windows);
	}
}

MemoryResource& SdlMouseMgr::get_memory_resource()
{
	static SinglePoolResource<SdlMouseMgr> memory_pool{};

	return memory_pool;
}

} // namespace

// ==========================================================================

MouseMgrUPtr make_sdl_mouse_mgr(Logger& logger)
{
	return std::make_unique<SdlMouseMgr>(logger);
}

} // namespace sys
} // namespace bstone
