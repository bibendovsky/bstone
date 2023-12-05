/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SHARED_LIBRARY_INCLUDED)
#define BSTONE_SHARED_LIBRARY_INCLUDED

#include <memory>

namespace bstone {

struct SharedLibraryHandleDeleter
{
	void operator()(void* handle) const;
};

using SharedLibraryHandleUPtr = std::unique_ptr<void, SharedLibraryHandleDeleter>;

// ==========================================================================

class SharedLibrary
{
public:
	SharedLibrary() = default;
	explicit SharedLibrary(const char* file_path);

	bool is_open() const noexcept;
	void open(const char* file_path);
	void close();

	void* find_symbol(const char* symbol_name) const noexcept;

	template<typename T>
	T find_symbol(const char* symbol_name) const noexcept;

private:
	SharedLibraryHandleUPtr handle_{};

private:
	void ensure_is_open() const;
};

// --------------------------------------------------------------------------

template<typename T>
T SharedLibrary::find_symbol(const char* symbol_name) const noexcept
{
	return reinterpret_cast<T>(find_symbol(symbol_name));
}

} // namespace bstone

#endif // BSTONE_SHARED_LIBRARY_INCLUDED
