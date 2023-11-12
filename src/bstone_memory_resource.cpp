/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cstddef>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

BSTONE_CXX_NODISCARD void* MemoryResource::allocate(std::intptr_t size)
{
	return do_allocate(size);
}

void MemoryResource::deallocate(void* resource) noexcept
{
	do_deallocate(resource);
}

// ==========================================================================

MemoryResourceUPtrDeleterBase::MemoryResourceUPtrDeleterBase(MemoryResource& memory_resource) noexcept
	:
	memory_resource_{&memory_resource}
{}

MemoryResource& MemoryResourceUPtrDeleterBase::get_memory_resource() const noexcept
{
	return *memory_resource_;
}

// ==========================================================================

BSTONE_CXX_NODISCARD void* NullMemoryResource::do_allocate(std::intptr_t)
{
	BSTONE_THROW_STATIC_SOURCE("Out of memory.");
}

void NullMemoryResource::do_deallocate(void*) noexcept {}

// ==========================================================================

BSTONE_CXX_NODISCARD void* NewDeleteMemoryResource::do_allocate(std::intptr_t size)
{
	BSTONE_ASSERT(size >= 0);

	return ::operator new(static_cast<std::size_t>(size));
}

void NewDeleteMemoryResource::do_deallocate(void* ptr) noexcept
{
	::operator delete(ptr);
}

// ==========================================================================

MemoryResource& get_null_memory_resource() noexcept
{
	static auto memory_resource = NullMemoryResource{};
	return memory_resource;
}

MemoryResource& get_new_delete_memory_resource() noexcept
{
	static auto memory_resource = NewDeleteMemoryResource{};
	return memory_resource;
}

MemoryResource& get_default_memory_resource() noexcept
{
	static auto memory_resource = NewDeleteMemoryResource{};
	return memory_resource;
}

} // namespace bstone
