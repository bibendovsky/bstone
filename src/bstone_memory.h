/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Dynamic memory management.

#if !defined(BSTONE_MEMORY_INCLUDED)
#define BSTONE_MEMORY_INCLUDED

#include <cassert>
#include <new>
#include <utility>

namespace bstone {

template<typename T, typename ...TArgs>
inline T* construct_at(T* ptr, TArgs&& ...args)
{
	assert(ptr != nullptr);
	return ::new (static_cast<void*>(ptr)) T(std::forward<TArgs>(args)...);
}

template<typename T>
inline void destroy_at(T* ptr) noexcept
{
	assert(ptr != nullptr);
	ptr->~T();
}

} // namespace bstone

#endif // BSTONE_MEMORY_INCLUDED
