/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows OS version.

#ifndef BSTONE_WIN32_OS_VERSION_INCLUDED
#define BSTONE_WIN32_OS_VERSION_INCLUDED

#ifdef _WIN32

namespace bstone {
namespace win32 {

struct OsVersion
{
	unsigned int major;
	unsigned int minor;
	unsigned int build;
};

// ==========================================================================

bool operator>=(const OsVersion& lhs, const OsVersion& rhs);
bool operator<(const OsVersion& lhs, const OsVersion& rhs);

// ==========================================================================

OsVersion get_os_version();

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_OS_VERSION_INCLUDED
