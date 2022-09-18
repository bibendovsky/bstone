/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_MEMORY_INCLUDED
#define BSTONE_MEMORY_INCLUDED

#include <cassert>
#include <new>
#include <utility>

namespace bstone
{

template<typename T, typename ...TArgs>
T* construct_at(T* ptr, TArgs&& ...args)
{
	assert(ptr);
	return ::new (ptr) T(std::forward<TArgs>(args)...);
}

// --------------------------------------------------------------------------

template<typename T>
void destroy_at(T* ptr) noexcept
{
	assert(ptr);
	ptr->~T();
}

} // bstone

#endif // !BSTONE_MEMORY_INCLUDED
