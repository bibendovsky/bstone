/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_shared_library.h"
#include "bstone_exception.h"
#include "bstone_sdl.h"
#include <utility>
#include "SDL3/SDL_loadso.h"

namespace bstone {

class SharedLibrary::Impl
{
public:
	static SDL_SharedObject* cast_to_native(void* handle)
	{
		return static_cast<SDL_SharedObject*>(handle);
	}
};

// ======================================

SharedLibrary::SharedLibrary(const char* file_path)
{
	open(file_path);
}

SharedLibrary::~SharedLibrary()
{
	internal_close();
}

bool SharedLibrary::is_open() const
{
	return native_handle_ != nullptr;
}

bool SharedLibrary::try_open(const char* file_path)
{
	internal_close();
	native_handle_ = SDL_LoadObject(file_path);
	return is_open();
}

void SharedLibrary::open(const char* file_path)
{
	if (!try_open(file_path))
	{
		sdl::fail("SDL_LoadObject");
	}
}

void SharedLibrary::close()
{
	internal_close();
	native_handle_ = nullptr;
}

SharedLibrarySymbol SharedLibrary::find_symbol(const char* symbol_name)
{
	return std::bit_cast<SharedLibrarySymbol>(SDL_LoadFunction(Impl::cast_to_native(native_handle_), symbol_name));
}

void SharedLibrary::swap(SharedLibrary& rhs) noexcept
{
	std::swap(native_handle_, rhs.native_handle_);
}

void SharedLibrary::internal_close()
{
	SDL_UnloadObject(Impl::cast_to_native(native_handle_));
}

} // namespace bstone
