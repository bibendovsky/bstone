/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(_WIN32)

#include <dlfcn.h>

#include "bstone_exception.h"
#include "bstone_shared_library.h"

namespace bstone {

void SharedLibraryHandleDeleter::operator()(void* handle) const
{
	dlclose(handle);
}

// ==========================================================================

void SharedLibrary::open(const char* file_path)
{
	close();

	auto handle = SharedLibraryHandleUPtr{dlopen(file_path, RTLD_NOW | RTLD_LOCAL)};

	if (handle == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to load a shared library.");
	}

	handle_.swap(handle);
}

void* SharedLibrary::find_symbol(const char* symbol_name) const
{
	ensure_is_open();

	return dlsym(handle_.get(), symbol_name);
}

} // namespace bstone

#endif // _WIN32
