/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SHARED_LIBRARY_INCLUDED
#define BSTONE_SHARED_LIBRARY_INCLUDED

#include <string>

namespace bstone {

using SharedLibrarySymbol = void (*)();

class SharedLibrary
{
public:
	SharedLibrary() = default;
	SharedLibrary(const SharedLibrary&) = delete;
	SharedLibrary& operator=(const SharedLibrary&) = delete;
	~SharedLibrary();

	bool is_open() const;
	bool open(const char* pathname);
	bool open(const char* pathname, std::string& error_message);
	void close();
	SharedLibrarySymbol find_symbol(const char* symbol_name);
	void swap(SharedLibrary& rhs) noexcept;

private:
	void* handle_{};

	void close_handle();
};

} // namespace bstone

#endif // BSTONE_SHARED_LIBRARY_INCLUDED
