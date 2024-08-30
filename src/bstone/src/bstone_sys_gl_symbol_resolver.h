/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver.

#ifndef BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED
#define BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED

namespace bstone {
namespace sys {

class GlSymbolResolver
{
public:
	GlSymbolResolver();
	virtual ~GlSymbolResolver();

	void* find_symbol(const char* symbol_name) const noexcept;

	template<typename T>
	T find_symbol(const char* symbol_name) const noexcept;

private:
	virtual void* do_find_symbol(const char* symbol_name) const noexcept = 0;
};

// --------------------------------------------------------------------------

template<typename T>
T GlSymbolResolver::find_symbol(const char* symbol_name) const noexcept
{
	return reinterpret_cast<T>(find_symbol(symbol_name));
}

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED
