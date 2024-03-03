/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// advapi32.dll dynamic symbols.

#ifndef BSTONE_BSTONE_WIN32_ADVAPI32_SYMBOLS_INCLUDED
#define BSTONE_BSTONE_WIN32_ADVAPI32_SYMBOLS_INCLUDED

#ifdef _WIN32

#include <windows.h>

namespace bstone {
namespace win32 {

using RegDeleteKeyExWFunc = LSTATUS (APIENTRY *)(
	HKEY hKey,
	LPCWSTR lpSubKey,
	REGSAM samDesired,
	DWORD Reserved);

struct AdvApi32Symbols
{
	static RegDeleteKeyExWFunc get_reg_delete_key_ex_w() noexcept;
};

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_BSTONE_WIN32_ADVAPI32_SYMBOLS_INCLUDED
