/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_loadso.h"
#include "bstone_exception.h"
#include "bstone_shared_library.h"
#include "bstone_utility.h"
#include "bstone_sys_sdl_exception.h"

namespace bstone {

namespace {

class SharedLibraryImpl
{
public:
	static void* open(const char* path);
	static void close(void* handle) noexcept;
	static bool is_open(void* handle) noexcept;
	static void* find_symbol(void* handle, const char* symbol_name) noexcept;
};

// --------------------------------------------------------------------------

void* SharedLibraryImpl::open(const char* path)
BSTONE_BEGIN_FUNC_TRY
	return sys::sdl_ensure_result(SDL_LoadObject(path));
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SharedLibraryImpl::close(void* handle) noexcept
{
	SDL_UnloadObject(handle);
}

bool SharedLibraryImpl::is_open(void* handle) noexcept
{
	return handle != nullptr;
}

void* SharedLibraryImpl::find_symbol(void* handle, const char* symbol_name) noexcept
{
	return SDL_LoadFunction(handle, symbol_name);
}

} // namespace

// ==========================================================================

SharedLibrary::SharedLibrary(const char* path)
	:
	handle_{SharedLibraryImpl::open(path)}
{}

SharedLibrary::SharedLibrary(SharedLibrary&& rhs) noexcept
{
	bstone::swop(handle_, rhs.handle_);
}

SharedLibrary::~SharedLibrary()
{
	SharedLibraryImpl::close(handle_);
}

void SharedLibrary::open(const char* path)
{
	SharedLibraryImpl::close(handle_);
	handle_ = nullptr;
	handle_ = SharedLibraryImpl::open(path);
}

void* SharedLibrary::find_symbol(const char* symbol_name) noexcept
{
	return SharedLibraryImpl::find_symbol(handle_, symbol_name);
}

} // bstone
