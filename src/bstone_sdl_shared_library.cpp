/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <utility>
#include "SDL_loadso.h"
#include "bstone_exception.h"
#include "bstone_sdl_exception.h"
#include "bstone_shared_library.h"

namespace bstone {

namespace {

class SdlSharedLibraryException : public Exception
{
public:
	explicit SdlSharedLibraryException(const char* message) noexcept
		:
		Exception{"SDL_SHARED_LIBRARY", message}
	{}

	~SdlSharedLibraryException() override = default;
};

// ==========================================================================

class SharedLibraryImpl
{
public:
	static void* open(const char* path);
	static void close(void* handle) noexcept;
	static bool is_open(void* handle) noexcept;
	static void* find_symbol(void* handle, const char* symbol_name) noexcept;

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
};

// --------------------------------------------------------------------------

void* SharedLibraryImpl::open(const char* path)
try
{
	return sdl_ensure_result(SDL_LoadObject(path));
}
catch (...)
{
	fail_nested(__func__);
}

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

[[noreturn]] void SharedLibraryImpl::fail(const char* message)
{
	throw SdlSharedLibraryException{message};
}

[[noreturn]] void SharedLibraryImpl::fail_nested(const char* message)
{
	std::throw_with_nested(SdlSharedLibraryException{message});
}

} // namespace

// ==========================================================================

SharedLibrary::SharedLibrary(const char* path)
	:
	handle_{SharedLibraryImpl::open(path)}
{}

SharedLibrary::SharedLibrary(SharedLibrary&& rhs) noexcept
{
	std::swap(handle_, rhs.handle_);
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
