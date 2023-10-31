/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cstddef>

#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

void* MemoryResource::allocate(std::intptr_t size)
try {
	return do_allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void MemoryResource::deallocate(void* resource)
try {
	do_deallocate(resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

MemoryResourceUPtrDeleterBase::MemoryResourceUPtrDeleterBase(MemoryResource& memory_resource)
	:
	memory_resource_{&memory_resource}
{}

MemoryResource& MemoryResourceUPtrDeleterBase::get_memory_resource() const noexcept
{
	return *memory_resource_;
}

// ==========================================================================

void* NullMemoryResource::do_allocate(std::intptr_t)
{
	BSTONE_THROW_STATIC_SOURCE("Out of memory.");
}

void NullMemoryResource::do_deallocate(void*) {}

// ==========================================================================

void* NewDeleteMemoryResource::do_allocate(std::intptr_t size)
{
	return ::operator new(static_cast<std::size_t>(size));
}

void NewDeleteMemoryResource::do_deallocate(void* ptr)
{
	::operator delete(ptr);
}

// ==========================================================================

MemoryResource& get_null_memory_resource()
{
	static auto memory_resource = NullMemoryResource{};
	return memory_resource;
}

MemoryResource& get_new_delete_memory_resource()
{
	static auto memory_resource = NewDeleteMemoryResource{};
	return memory_resource;
}

MemoryResource& get_default_memory_resource()
{
	static auto memory_resource = NewDeleteMemoryResource{};
	return memory_resource;
}

} // namespace bstone
