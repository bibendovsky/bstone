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
