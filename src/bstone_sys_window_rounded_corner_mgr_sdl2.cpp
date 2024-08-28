/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager (SDL2).

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "bstone_sys_window_rounded_corner_mgr.h"

#include <cstddef>

#include <windows.h>

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_shared_library.h"
#include "bstone_sys_window_sdl2.h"
#include "bstone_win32_os_version.h"

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

// ==========================================================================

namespace bstone {
namespace sys {

namespace {

class Sdl2WindowRoundedCornerMgr final : public WindowRoundedCornerMgr
{
public:
	Sdl2WindowRoundedCornerMgr();
	~Sdl2WindowRoundedCornerMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	using PFNDWMSETWINDOWATTRIBUTEPROC = HRESULT (WINAPI *)(
		HWND hwnd,
		DWORD dwAttribute,
		LPCVOID pvAttribute,
		DWORD cbAttribute);

	using Pool = SinglePoolResource<Sdl2WindowRoundedCornerMgr>;

private:
	void do_set_round_corner_type(Window& window, WindowRoundedCornerType rounded_corner_type) override;

private:
	static Pool pool_;

private:
	SharedLibrary wdmapi_shared_library_{};
	PFNDWMSETWINDOWATTRIBUTEPROC DwmSetWindowAttribute_{};
};

// --------------------------------------------------------------------------

Sdl2WindowRoundedCornerMgr::Pool Sdl2WindowRoundedCornerMgr::pool_{};

// --------------------------------------------------------------------------

Sdl2WindowRoundedCornerMgr::Sdl2WindowRoundedCornerMgr()
{
	constexpr auto min_os_version = win32::OsVersion{10, 0, 22000};
	const auto os_version = win32::get_os_version();

	if (os_version < min_os_version)
	{
		return;
	}

	auto wdmapi_shared_library = SharedLibrary{};

	if (!wdmapi_shared_library.try_open("dwmapi.dll"))
	{
		return;
	}

	DwmSetWindowAttribute_ = wdmapi_shared_library.find_symbol<PFNDWMSETWINDOWATTRIBUTEPROC>(
		"DwmSetWindowAttribute");

	if (DwmSetWindowAttribute_ == nullptr)
	{
		return;
	}

	wdmapi_shared_library_.swap(wdmapi_shared_library);
}

Sdl2WindowRoundedCornerMgr::~Sdl2WindowRoundedCornerMgr() = default;

void* Sdl2WindowRoundedCornerMgr::operator new(std::size_t size)
{
	return pool_.allocate(size);
}

void Sdl2WindowRoundedCornerMgr::operator delete(void* ptr) noexcept
{
	pool_.deallocate(ptr);
}

void Sdl2WindowRoundedCornerMgr::do_set_round_corner_type(
	Window& window,
	WindowRoundedCornerType rounded_corner_type)
{
	if (DwmSetWindowAttribute_ == nullptr)
	{
		return;
	}

	auto win32_rounded_corner_type = DWORD{};

	switch (rounded_corner_type)
	{
		case WindowRoundedCornerType::none: win32_rounded_corner_type = DWMWCP_DONOTROUND; break;
		case WindowRoundedCornerType::system: win32_rounded_corner_type = DWMWCP_DEFAULT; break;
		case WindowRoundedCornerType::round: win32_rounded_corner_type = DWMWCP_ROUND; break;
		case WindowRoundedCornerType::round_small: win32_rounded_corner_type = DWMWCP_ROUNDSMALL; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown window rounded corner type.");
	}

	const auto native_handle = static_cast<Sdl2WindowInternal&>(window).get_native_handle();
	const auto win32_native_handle = static_cast<HWND>(native_handle);

	const auto hresult = DwmSetWindowAttribute_(
		win32_native_handle,
		DWMWA_WINDOW_CORNER_PREFERENCE,
		&win32_rounded_corner_type,
		static_cast<DWORD>(sizeof(win32_rounded_corner_type)));

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set window rounded corner type.");
	}
}

} // namespace

// ==========================================================================

WindowRoundedCornerMgrUPtr make_window_rounded_corner_mgr()
{
	return std::make_unique<Sdl2WindowRoundedCornerMgr>();
}

} // namespace sys
} // namespace bstone

#endif // _WIN32
