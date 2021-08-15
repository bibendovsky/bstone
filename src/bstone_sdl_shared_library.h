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


#ifndef BSTONE_SDL_SHARED_LIBRARY_INCLUDED
#define BSTONE_SDL_SHARED_LIBRARY_INCLUDED


#include "bstone_shared_library.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlSharedLibrary final :
	public SharedLibrary
{
public:
	SdlSharedLibrary() noexcept = default;

	SdlSharedLibrary(
		const char* path);

	SdlSharedLibrary(
		SdlSharedLibrary&& rhs) noexcept;

	~SdlSharedLibrary() override;


	// ======================================================================
	// SharedLibrary

	void open(
		const char* path) override;

	void close() noexcept override;

	bool is_open() const noexcept override;

	void* find_symbol(
		const char* symbol_name) noexcept override;

	// SharedLibrary
	// ======================================================================


private:
	void* handle_{};


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);

	void open_internal(
		const char* path);

	void close_internal() noexcept;

	bool is_open_internal() const noexcept;
}; // SdlSharedLibrary

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_SDL_SHARED_LIBRARY_INCLUDED
