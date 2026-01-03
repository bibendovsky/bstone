/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window manager (SDL)

#include "bstone_sys_window_mgr_sdl.h"
#include "bstone_sys_window_sdl.h"
#include "bstone_sys_window_decoration_mgr.h"

namespace bstone::sys {

namespace {

class WindowMgrSdl final : public WindowMgr
{
public:
	explicit WindowMgrSdl(Logger& logger);
	~WindowMgrSdl() override;

private:
	Logger& logger_;
	WindowDecorationMgrUPtr decoration_mgr_{};

	WindowUPtr do_make_window(const WindowInitParam& param) override;
};

// --------------------------------------

WindowMgrSdl::WindowMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL window manager.");
	decoration_mgr_ = make_window_decoration_mgr();
	logger_.log_information("SDL window manager has started.");
}

WindowMgrSdl::~WindowMgrSdl()
{
	logger_.log_information("Shut down SDL window manager.");
}

WindowUPtr WindowMgrSdl::do_make_window(const WindowInitParam& param)
{
	return make_window_sdl(logger_, *decoration_mgr_, param);
}

} // namespace

// ======================================

WindowMgrUPtr make_window_mgr_sdl(Logger& logger)
{
	return std::make_unique<WindowMgrSdl>(logger);
}

} // namespace bstone::sys
