/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Mouse manager (SDL)

#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sys_mouse_mgr_sdl.h"
#include <format>
#include <string>
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

namespace {

class MouseMgrSdl final : public MouseMgr
{
public:
	MouseMgrSdl() = default;
	MouseMgrSdl(const MouseMgrSdl&) = delete;
	MouseMgrSdl& operator=(const MouseMgrSdl&) = delete;
	~MouseMgrSdl() override = default;

private:
	void do_set_relative_mode(bool is_enable) override;
};

// --------------------------------------

void MouseMgrSdl::do_set_relative_mode(bool is_enable)
{
	// TODO Move it somewhere else.
	int window_count;
	SDL_Window** const sdl_windows = SDL_GetWindows(&window_count);
	if (sdl_windows != nullptr)
	{
		const auto scope_exit = make_scope_exit(
			[sdl_windows]()
			{
				SDL_free(sdl_windows);
			});
		if (window_count > 0)
		{
			if (window_count != 1)
			{
				BSTONE_THROW_STATIC_SOURCE("Too many windows.");
			}
			SDL_Window* const sdl_window = sdl_windows[0];
			if (!SDL_SetWindowRelativeMouseMode(sdl_window, is_enable))
			{
				const std::string message = std::format(
					"[{}] {}",
					"SDL_SetWindowRelativeMouseMode",
					SDL_GetError());
				BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
			}
		}
	}
}

} // namespace

// ======================================

MouseMgrUPtr make_mouse_mgr_sdl([[maybe_unused]] Logger& logger)
{
	return std::make_unique<MouseMgrSdl>();
}

} // namespace bstone::sys
