/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
