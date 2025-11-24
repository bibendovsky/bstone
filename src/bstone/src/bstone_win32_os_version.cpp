/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows OS version.

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "bstone_win32_os_version.h"
#include "bstone_shared_library.h"
#include <windows.h>

namespace bstone {
namespace win32 {

namespace {

class OsVersionImpl
{
public:
	OsVersionImpl();
	OsVersion get_os_version() const;

private:
	OsVersion os_version_{};
};

// --------------------------------------------------------------------------

OsVersionImpl::OsVersionImpl()
{
	// Set default version to Windows XP (Whistler).
	os_version_.major = 5;
	os_version_.minor = 1;
	os_version_.build = 2600;

	using IMPL_NTSTATUS = long;
	using IMPL_RtlGetVersion = IMPL_NTSTATUS (WINAPI *)(const OSVERSIONINFOW* lpVersionInformation);
	SharedLibrary impl_ntdll_{};
	if (impl_ntdll_.try_open("ntdll.dll"))
	{
		const IMPL_RtlGetVersion impl_RtlGetVersion = impl_ntdll_.find_symbol<IMPL_RtlGetVersion>("RtlGetVersion");
		if (impl_RtlGetVersion != nullptr)
		{
			OSVERSIONINFOW osversioninfow{};
			osversioninfow.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
			if (impl_RtlGetVersion(&osversioninfow) == 0)
			{
				os_version_.major = osversioninfow.dwMajorVersion;
				os_version_.minor = osversioninfow.dwMinorVersion;
				os_version_.build = osversioninfow.dwBuildNumber;
			}
		}
	}
}

OsVersion OsVersionImpl::get_os_version() const
{
	return os_version_;
}

// ==========================================================================

const OsVersionImpl os_version_impl{};

} // namespace

// ==========================================================================

bool operator>=(const OsVersion& lhs, const OsVersion& rhs)
{
	return
		lhs.major > rhs.major ||
		(lhs.major == rhs.major && lhs.minor > rhs.minor) ||
		(lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.build >= rhs.build);
}

bool operator<(const OsVersion& lhs, const OsVersion& rhs)
{
	return !(lhs >= rhs);
}

// ==========================================================================

OsVersion get_os_version()
{
	return os_version_impl.get_os_version();
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
