/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_shared_library.h"
#include <bit>
#include <string_view>
#include <utility>
#include "SDL3/SDL_loadso.h"

namespace bstone {

SharedLibrary::~SharedLibrary()
{
	close_handle();
}

bool SharedLibrary::is_open() const
{
	return handle_ != nullptr;
}

bool SharedLibrary::open(const char* pathname)
{
	close_handle();
	if (handle_ = SDL_LoadObject(pathname);
		handle_ == nullptr)
		return false;
	return true;
}

bool SharedLibrary::open(const char* pathname, std::string& error_message)
{
	error_message.clear();
	close_handle();
	if (handle_ = SDL_LoadObject(pathname);
		handle_ == nullptr)
	{
		constinit static const std::string_view prefix = "[SDL_LoadObject] ";
		const std::string_view sdl_error_message = SDL_GetError();
		error_message.reserve(prefix.size() + sdl_error_message.size());
		error_message += prefix;
		error_message += sdl_error_message;
		return false;
	}
	return true;
}

void SharedLibrary::close()
{
	close_handle();
	handle_ = nullptr;
}

SharedLibrarySymbol SharedLibrary::find_symbol(const char* symbol_name)
{
	return std::bit_cast<SharedLibrarySymbol>(SDL_LoadFunction(static_cast<SDL_SharedObject*>(handle_), symbol_name));
}

void SharedLibrary::swap(SharedLibrary& rhs) noexcept
{
	std::swap(handle_, rhs.handle_);
}

void SharedLibrary::close_handle()
{
	SDL_UnloadObject(static_cast<SDL_SharedObject*>(handle_));
}

} // namespace bstone
