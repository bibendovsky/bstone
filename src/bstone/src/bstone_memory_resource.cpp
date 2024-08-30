/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cstddef>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

MemoryResource::MemoryResource() noexcept = default;

MemoryResource::~MemoryResource() = default;

BSTONE_CXX_NODISCARD void* MemoryResource::allocate(std::intptr_t size)
{
	BSTONE_ASSERT(size >= 0);

	return do_allocate(size);
}

void MemoryResource::deallocate(void* ptr) noexcept
{
	do_deallocate(ptr);
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

NullMemoryResource::NullMemoryResource() noexcept = default;
NullMemoryResource::~NullMemoryResource() = default;

BSTONE_CXX_NODISCARD void* NullMemoryResource::do_allocate(std::intptr_t)
{
	BSTONE_THROW_STATIC_SOURCE("Out of memory.");
}

void NullMemoryResource::do_deallocate(void*) noexcept {}

// ==========================================================================

NewDeleteMemoryResource::NewDeleteMemoryResource() noexcept = default;
NewDeleteMemoryResource::~NewDeleteMemoryResource() = default;

BSTONE_CXX_NODISCARD void* NewDeleteMemoryResource::do_allocate(std::intptr_t size)
{
	return ::operator new(static_cast<std::size_t>(size));
}

void NewDeleteMemoryResource::do_deallocate(void* ptr) noexcept
{
	::operator delete(ptr);
}

// ==========================================================================

namespace {

NullMemoryResource null_memory_resource_impl{};
NewDeleteMemoryResource new_delete_memory_resource_impl{};
NewDeleteMemoryResource default_memory_resource_impl{};

} // namespace

MemoryResource& get_null_memory_resource() noexcept
{
	return null_memory_resource_impl;
}

MemoryResource& get_new_delete_memory_resource() noexcept
{
	return new_delete_memory_resource_impl;
}

MemoryResource& get_default_memory_resource() noexcept
{
	return default_memory_resource_impl;
}

} // namespace bstone
