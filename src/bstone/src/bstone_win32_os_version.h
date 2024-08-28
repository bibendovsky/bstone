/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows OS version.

#ifndef BSTONE_WIN32_OS_VERSION_INCLUDED
#define BSTONE_WIN32_OS_VERSION_INCLUDED

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <memory>

#include <wtypes.h>
#include <unknwn.h>

namespace bstone {
namespace win32 {

struct OsVersion
{
	int major{};
	int minor{};
	int build{};
};

// ==========================================================================

constexpr bool operator>=(OsVersion lhs, OsVersion rhs) noexcept
{
	return
		lhs.major > rhs.major ||
		(lhs.major == rhs.major && lhs.minor > rhs.minor) ||
		(lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.build >= rhs.build);
}

constexpr bool operator<(OsVersion lhs, OsVersion rhs) noexcept
{
	return !(lhs >= rhs);
}

// ==========================================================================

OsVersion get_os_version() noexcept;

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_OS_VERSION_INCLUDED
