/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if defined(_WIN32)

#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "bstone_exception.h"
#include "bstone_shared_library.h"
#include "bstone_win32_wstring.h"

namespace bstone {

void SharedLibraryHandleDeleter::operator()(void* handle) const
{
	FreeLibrary(reinterpret_cast<HMODULE>(handle));
}

// ==========================================================================

void SharedLibrary::open(const char* file_path)
{
	close();

	const auto u16_file_name = Win32WString{file_path};
	auto handle = SharedLibraryHandleUPtr{LoadLibraryW(u16_file_name.get_data())};

	if (handle == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to load a shared library.");
	}

	handle_.swap(handle);
}

void* SharedLibrary::find_symbol(const char* symbol_name) const noexcept
{
	ensure_is_open();

	return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle_.get()), symbol_name));
}

} // namespace bstone

#endif // _WIN32
