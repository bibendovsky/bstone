/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SHARED_LIBRARY_INCLUDED
#define BSTONE_SHARED_LIBRARY_INCLUDED

#include <bit>

namespace bstone {

using SharedLibrarySymbol = void (*)();

class SharedLibrary
{
public:
	SharedLibrary() = default;
	explicit SharedLibrary(const char* file_path);
	SharedLibrary(const SharedLibrary&) = delete;
	SharedLibrary& operator=(const SharedLibrary&) = delete;
	~SharedLibrary();

	bool is_open() const;
	bool try_open(const char* file_path);
	void open(const char* file_path);
	void close();

	SharedLibrarySymbol find_symbol(const char* symbol_name);

	template<typename T>
	T find_symbol(const char* symbol_name)
	{
		return std::bit_cast<T>(find_symbol(symbol_name));
	}

	void swap(SharedLibrary& rhs) noexcept;

private:
	class Impl;

	void* native_handle_{};

	void internal_close();
};

} // namespace bstone

#endif // BSTONE_SHARED_LIBRARY_INCLUDED
