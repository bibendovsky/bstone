/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_WINDOW_SDL2_INCLUDED
#define BSTONE_SYS_WINDOW_SDL2_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_window.h"
#include "bstone_sys_window_rounded_corner_mgr.h"

namespace bstone {
namespace sys {

class Sdl2WindowInternal : public Window
{
public:
	Sdl2WindowInternal();
	~Sdl2WindowInternal() override;

	void* get_native_handle() const noexcept;

private:
	virtual void* do_get_native_handle() const noexcept = 0;
};

// ==========================================================================

WindowUPtr make_sdl2_window(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_WINDOW_SDL2_INCLUDED
