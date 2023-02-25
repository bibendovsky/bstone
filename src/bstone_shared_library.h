/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SHARED_LIBRARY_INCLUDED)
#define BSTONE_SHARED_LIBRARY_INCLUDED

namespace bstone {

class SharedLibrary
{
public:
	SharedLibrary() = default;
	SharedLibrary(const char* path);
	SharedLibrary(const SharedLibrary&) = delete;
	SharedLibrary(SharedLibrary&& rhs) noexcept;
	SharedLibrary& operator=(const SharedLibrary&) = delete;
	SharedLibrary& operator=(SharedLibrary&& rhs) = delete;
	~SharedLibrary();

	void open(const char* path);
	void* find_symbol(const char* symbol_name) noexcept;

	template<typename T>
	T find_symbol(const char* symbol_name) noexcept
	{
		return reinterpret_cast<T>(find_symbol(symbol_name));
	}

private:
	void* handle_{};
};

} // namespace bstone

#endif // BSTONE_SHARED_LIBRARY_INCLUDED
