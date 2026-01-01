/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager (SDL).

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "bstone_sys_window_rounded_corner_mgr.h"
#include "bstone_exception.h"
#include "bstone_shared_library.h"
#include "bstone_sys_window_sdl.h"
#include "bstone_win32_os_version.h"
#include <windows.h>

enum DWMWINDOWATTRIBUTE
{
	DWMWA_WINDOW_CORNER_PREFERENCE = 33,
};

enum DWM_WINDOW_CORNER_PREFERENCE
{
	DWMWCP_DEFAULT = 0,
	DWMWCP_DONOTROUND = 1,
	DWMWCP_ROUND = 2,
	DWMWCP_ROUNDSMALL = 3
};

// ======================================

namespace bstone::sys {

namespace {

class WindowRoundedCornerMgrSdl final : public WindowRoundedCornerMgr
{
public:
	WindowRoundedCornerMgrSdl();
	~WindowRoundedCornerMgrSdl() override = default;

private:
	using PFNDWMSETWINDOWATTRIBUTEPROC = HRESULT (WINAPI *)(
		HWND hwnd,
		DWORD dwAttribute,
		LPCVOID pvAttribute,
		DWORD cbAttribute);

	void do_set_round_corner_type(Window& window, WindowRoundedCornerType rounded_corner_type) override;

	SharedLibrary wdmapi_shared_library_{};
	PFNDWMSETWINDOWATTRIBUTEPROC DwmSetWindowAttribute_{};
};

// --------------------------------------

WindowRoundedCornerMgrSdl::WindowRoundedCornerMgrSdl()
{
	constexpr win32::OsVersion min_os_version{10, 0, 22000};
	const win32::OsVersion os_version = win32::get_os_version();
	if (os_version < min_os_version)
	{
		return;
	}
	SharedLibrary wdmapi_shared_library{};
	if (!wdmapi_shared_library.try_open("dwmapi.dll"))
	{
		return;
	}
	DwmSetWindowAttribute_ = wdmapi_shared_library.find_symbol<PFNDWMSETWINDOWATTRIBUTEPROC>("DwmSetWindowAttribute");
	if (DwmSetWindowAttribute_ == nullptr)
	{
		return;
	}
	wdmapi_shared_library_.swap(wdmapi_shared_library);
}

void WindowRoundedCornerMgrSdl::do_set_round_corner_type(
	Window& window,
	WindowRoundedCornerType rounded_corner_type)
{
	if (DwmSetWindowAttribute_ == nullptr)
	{
		return;
	}
	DWORD win32_rounded_corner_type = 0;
	switch (rounded_corner_type)
	{
		case WindowRoundedCornerType::none:
			win32_rounded_corner_type = DWMWCP_DONOTROUND;
			break;
		case WindowRoundedCornerType::system:
			win32_rounded_corner_type = DWMWCP_DEFAULT;
			break;
		case WindowRoundedCornerType::round:
			win32_rounded_corner_type = DWMWCP_ROUND;
			break;
		case WindowRoundedCornerType::round_small:
			win32_rounded_corner_type = DWMWCP_ROUNDSMALL;
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown window rounded corner type.");
	}
	void* const native_handle = static_cast<WindowSdlInternal&>(window).get_native_handle();
	const HWND win32_native_handle = static_cast<HWND>(native_handle);
	const HRESULT hresult = DwmSetWindowAttribute_(
		win32_native_handle,
		DWMWA_WINDOW_CORNER_PREFERENCE,
		&win32_rounded_corner_type,
		sizeof(win32_rounded_corner_type));
	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set window rounded corner type.");
	}
}

} // namespace

// ======================================

WindowRoundedCornerMgrUPtr make_window_rounded_corner_mgr()
{
	return std::make_unique<WindowRoundedCornerMgrSdl>();
}

} // namespace bstone::sys

#endif // _WIN32
