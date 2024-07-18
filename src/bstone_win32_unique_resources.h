/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows API unique resources.

#ifndef BSTONE_WIN32_UNIQUE_RESOURCES_INCLUDED
#define BSTONE_WIN32_UNIQUE_RESOURCES_INCLUDED

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "bstone_unique_resource.h"

namespace bstone {

struct Win32HandleDeleter
{
	void operator()(HANDLE handle) const noexcept;
};

struct Win32HandleEmptyValue
{
	// TODO constexpr method fails to compile in mingw-w64 v8.1.0
	HANDLE operator()() const noexcept;
};

using Win32HandleUResource = UniqueResource<HANDLE, Win32HandleDeleter, Win32HandleEmptyValue>;

} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_UNIQUE_RESOURCES_INCLUDED
