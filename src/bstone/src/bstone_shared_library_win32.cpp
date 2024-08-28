/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include "bstone_shared_library.h"

#include <windows.h>

#include "bstone_exception.h"
#include "bstone_win32_wstring.h"

namespace bstone {

void SharedLibraryHandleDeleter::operator()(void* handle) const noexcept
{
	FreeLibrary(reinterpret_cast<HMODULE>(handle));
}

// ==========================================================================

bool SharedLibrary::try_open(const char* file_path)
{
	close();

	const auto u16_file_name = Win32WString{file_path};
	auto handle = SharedLibraryHandleUPtr{LoadLibraryW(u16_file_name.get_data())};

	if (handle == nullptr)
	{
		return false;
	}

	handle_.swap(handle);
	return true;
}

void* SharedLibrary::find_symbol(const char* symbol_name) const noexcept
{
	return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle_.get()), symbol_name));
}

} // namespace bstone

#endif // _WIN32
