/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File's unique resource (WIN32).

#if defined(_WIN32)

#include <cassert>

#include <windows.h>

#include "bstone_file_uresource.h"

namespace bstone {

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return INVALID_HANDLE_VALUE;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const
{
#if !defined(NDEBUG)
	const auto win32_result =
#endif
		CloseHandle(static_cast<HANDLE>(handle));
#if !defined(NDEBUG)
	assert(win32_result != 0);
#endif // NDEBUG
}

} // namespace bstone

#endif // _WIN32
