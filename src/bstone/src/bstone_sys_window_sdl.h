/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window (SDL)

#ifndef BSTONE_SYS_WINDOW_SDL_INCLUDED
#define BSTONE_SYS_WINDOW_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_window.h"
#include "bstone_sys_window_decoration_mgr.h"

namespace bstone::sys {

class WindowSdlInternal : public Window
{
public:
	WindowSdlInternal() {}
	~WindowSdlInternal() override {}

	void* get_native_handle() const;
	void* get_sdl_window() const;

private:
	virtual void* do_get_native_handle() const = 0;
	virtual void* do_get_sdl_window() const = 0;
};

// ==========================================================================

WindowUPtr make_window_sdl(
	Logger& logger,
	WindowDecorationMgr& decoration_mgr,
	const WindowInitParam& param);

} // namespace bstone::sys

#endif // BSTONE_SYS_WINDOW_SDL_INCLUDED
