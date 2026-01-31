/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Utilities for Windows

namespace bstone::win32 {

int get_string_size(const char* string);

// =======================================

int utf8_to_utf16(const char* u8_chars, int u8_count, wchar_t* u16_chars, int u16_count);
int utf8_to_utf16(const char* u8_chars, int u8_count);

int utf16_to_utf8(const wchar_t* u16_chars, int u16_count, char* u8_chars, int u8_count);
int utf16_to_utf8(const wchar_t* u16_chars, int u16_count);

} // namespace bstone::win32
