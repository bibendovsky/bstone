/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL symbol resolver

#ifndef BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED
#define BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED

namespace bstone::sys {

using GlSymbolResolverSymbolFunc = void (*)();

class GlSymbolResolver
{
public:
	GlSymbolResolver() = default;
	virtual ~GlSymbolResolver() = default;

	GlSymbolResolverSymbolFunc find_symbol(const char* symbol_name) const;

	template<typename T>
	T find_symbol(const char* symbol_name) const;

private:
	virtual GlSymbolResolverSymbolFunc do_find_symbol(const char* symbol_name) const = 0;
};

// --------------------------------------

template<typename T>
T GlSymbolResolver::find_symbol(const char* symbol_name) const
{
	return reinterpret_cast<T>(find_symbol(symbol_name));
}

} // namespace bstone::sys

#endif // BSTONE_SYS_GL_SYMBOL_RESOLVER_INCLUDED
