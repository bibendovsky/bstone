/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_SHARED_LIBRARY_INCLUDED
#define BSTONE_SHARED_LIBRARY_INCLUDED


#include <memory>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SharedLibrary
{
public:
	SharedLibrary() noexcept = default;

	virtual ~SharedLibrary() = default;


	virtual void open(
		const char* path) = 0;

	virtual void close() noexcept = 0;

	virtual bool is_open() const noexcept = 0;

	virtual void* find_symbol(
		const char* symbol_name) noexcept = 0;
}; // SharedLibrary

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SharedLibraryUPtr = std::unique_ptr<SharedLibrary>;


SharedLibraryUPtr make_shared_library();

SharedLibraryUPtr make_shared_library(
	const char* path);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_SHARED_LIBRARY_INCLUDED
