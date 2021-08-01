/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_sdl_shared_library.h"

#include <cassert>

#include <exception>
#include <string>
#include <utility>

#include "SDL_loadso.h"

#include "bstone_exception.h"
#include "bstone_sdl2_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlSharedLibraryException :
	public Exception
{
public:
	explicit SdlSharedLibraryException(
		const char* message)
		:
		Exception{"SDL_SHARED_LIBRARY", message}
	{
	}
}; // SdlSharedLibraryException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SdlSharedLibrary::SdlSharedLibrary(
	const char* path)
{
	assert(path && (*path) != '\0');

	open_internal(path);
}

SdlSharedLibrary::SdlSharedLibrary(
	SdlSharedLibrary&& rhs) noexcept
{
	std::swap(handle_, rhs.handle_);
}

SdlSharedLibrary::~SdlSharedLibrary()
{
	close_internal();
}

// ======================================================================
// SharedLibrary

void SdlSharedLibrary::open(
	const char* path)
{
	assert(path && (*path) != '\0');

	open_internal(path);
}

void SdlSharedLibrary::close() noexcept
{
	close_internal();
}

bool SdlSharedLibrary::is_open() const noexcept
{
	return is_open_internal();
}

void* SdlSharedLibrary::find_symbol(
	const char* symbol_name) noexcept
{
	if (!is_open_internal() || !symbol_name || (*symbol_name) == '\0')
	{
		assert(!"Closed or null / empty symbol name.");
		return nullptr;
	}

	return ::SDL_LoadFunction(handle_, symbol_name);
}

[[noreturn]]
void SdlSharedLibrary::fail(
	const char* message)
{
	throw SdlSharedLibraryException{message};
}

[[noreturn]]
void SdlSharedLibrary::fail_nested(
	const char* message)
{
	std::throw_with_nested(SdlSharedLibraryException{message});
}

void SdlSharedLibrary::open_internal(
	const char* path)
{
	if (!path || (*path) == '\0')
	{
		fail("Null or empty path.");
	}

	try
	{
		Sdl2EnsureResult{handle_ = ::SDL_LoadObject(path)};
	}
	catch (...)
	{
		const auto message = std::string{} + "Failed to open shared library \"" + path + "\".";
		fail_nested(message.c_str());
	}
}

void SdlSharedLibrary::close_internal() noexcept
{
	if (is_open_internal())
	{
		const auto handle = handle_;
		handle_ = nullptr;

		::SDL_UnloadObject(handle);
	}
}

bool SdlSharedLibrary::is_open_internal() const noexcept
{
	return handle_ != nullptr;
}

// SharedLibrary
// ======================================================================

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SharedLibraryUPtr make_shared_library()
{
	return std::make_unique<SdlSharedLibrary>();
}

SharedLibraryUPtr make_shared_library(
	const char* path)
{
	return std::make_unique<SdlSharedLibrary>(path);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
