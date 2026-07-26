/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Utilities for Windows

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "bstone_win32_utility.h"
#include "bstone_assert.h"
#include "bstone_win32_os_version.h"
#include <windows.h>

namespace bstone::win32 {

int get_string_size(const char* string)
{
	BSTONE_ASSERT(string != nullptr);
	return lstrlenA(string);
}

// =======================================

namespace {

// TODO Remove if the minimum version is Windows Vista or later.
bool validate_u16_chars(const wchar_t* u16_chars, int u16_count)
{
	constexpr unsigned int high_surrogate_min = 0xD800;
	constexpr unsigned int high_surrogate_max = 0xDBFF;
	constexpr unsigned int low_surrogate_min = 0xDC00;
	constexpr unsigned int low_surrogate_max = 0xDFFF;
	for (int i_char = 0; i_char < u16_count; )
	{
		const unsigned int u16_char = u16_chars[i_char];
		if (u16_char >= high_surrogate_min && u16_char <= high_surrogate_max)
		{
			// The high surrogate followed by the low one.
			if (u16_count - i_char <= 1)
			{
				// Not enough code units.
				return false;
			}
			const unsigned int next_u16_char = u16_chars[i_char + 1];
			if (next_u16_char < low_surrogate_min || next_u16_char > low_surrogate_max)
			{
				// No low surrogate.
				return false;
			}
			i_char += 2;
		}
		else
		{
			if (u16_char >= low_surrogate_min && u16_char <= low_surrogate_max)
			{
				// Low surrogate.
				return false;
			}
			++i_char;
		}
	}
	return true;
}

} // namespace

int utf8_to_utf16(const char* u8_chars, int u8_count, wchar_t* u16_chars, int u16_count)
{
	BSTONE_ASSERT(u8_chars != nullptr);
	BSTONE_ASSERT(u8_count >= 0);
	BSTONE_ASSERT(u16_count >= 0);
	if (u8_count == 0)
	{
		return 0;
	}
	if (const int written_count = MultiByteToWideChar(
		/* CodePage       */ CP_UTF8,
		/* dwFlags        */ MB_ERR_INVALID_CHARS,
		/* lpMultiByteStr */ u8_chars,
		/* cbMultiByte    */ u8_count,
		/* lpWideCharStr  */ u16_chars,
		/* cchWideChar    */ u16_count);
		written_count > 0)
	{
		return written_count;
	}
	return -1;
}

int utf8_to_utf16(const char* u8_chars, int u8_count)
{
	return utf8_to_utf16(u8_chars, u8_count, nullptr, 0);
}

int utf16_to_utf8(const wchar_t* u16_chars, int u16_count, char* u8_chars, int u8_count)
{
	BSTONE_ASSERT(u16_chars != nullptr);
	BSTONE_ASSERT(u16_count >= 0);
	BSTONE_ASSERT(u8_count >= 0);
	if (u16_count == 0)
	{
		return 0;
	}
	DWORD flags;
	if (is_windows_vista_or_later())
	{
		constexpr DWORD IMPL_WC_ERR_INVALID_CHARS = 0x00000080;
		flags = IMPL_WC_ERR_INVALID_CHARS;
	}
	else
	{
		if (!validate_u16_chars(u16_chars, u16_count))
		{
			return -1;
		}
		flags = 0;
	}
	if (const int written_count = WideCharToMultiByte(
		/* CodePage          */ CP_UTF8,
		/* dwFlags           */ flags,
		/* lpWideCharStr     */ u16_chars,
		/* cchWideChar       */ u16_count,
		/* lpMultiByteStr    */ u8_chars,
		/* cbMultiByte       */ u8_count,
		/* lpDefaultChar     */ nullptr,
		/* lpUsedDefaultChar */ nullptr);
		written_count > 0)
	{
		return written_count;
	}
	return -1;
}

int utf16_to_utf8(const wchar_t* u16_chars, int u16_count)
{
	return utf16_to_utf8(u16_chars, u16_count, nullptr, 0);
}

} // namespace bstone::win32

#endif // _WIN32
