/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows API unique resources.

#ifdef _WIN32

#include "bstone_win32_unique_resources.h"

#include "bstone_assert.h"

namespace bstone {

void Win32HandleDeleter::operator()(HANDLE handle) const noexcept
{
	const auto win32_result = CloseHandle(handle);
	BSTONE_ASSERT(win32_result == TRUE);
	static_cast<void>(win32_result);
}

// TODO constexpr method fails to compile in mingw-w64 v8.1.0
HANDLE Win32HandleEmptyValue::operator()() const noexcept
{
	return INVALID_HANDLE_VALUE;
}

} // namespace bstone

#endif // _WIN32
