/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cstddef>

#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

void* MemoryResource::allocate(MemoryResourceInt size)
try {
	return do_allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void MemoryResource::deallocate(void* resource)
try {
	do_deallocate(resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void* NewDeleteMemoryResource::do_allocate(MemoryResourceInt size)
{
	return ::operator new(static_cast<std::size_t>(size));
}

void NewDeleteMemoryResource::do_deallocate(void* ptr)
{
	::operator delete(ptr);
}

// ==========================================================================

MemoryResource& get_default_memory_resource()
{
	static auto memory_resource = NewDeleteMemoryResource{};
	return memory_resource;
}

} // namespace bstone
