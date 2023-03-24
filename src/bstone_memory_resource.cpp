/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <new>
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

void* MemoryResource::allocate(std::size_t size)
try
{
	return do_allocate(size);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void MemoryResource::deallocate(void* resource) noexcept
{
	do_deallocate(resource);
}

// ==========================================================================

namespace {

class DefaultMemoryResource final : public MemoryResource
{
public:
	DefaultMemoryResource() noexcept = default;

private:
	void* do_allocate(std::size_t size) override;
	void do_deallocate(void* resource) noexcept override;
};

// --------------------------------------------------------------------------

void* DefaultMemoryResource::do_allocate(std::size_t size)
{
	return ::operator new[](size);
}

void DefaultMemoryResource::do_deallocate(void* resource) noexcept
{
	::operator delete[](resource);
}

// ==========================================================================

DefaultMemoryResource default_memory_resource{};

} // namespace

// ==========================================================================

MemoryResource& get_default_memory_resource() noexcept
{
	return default_memory_resource;
}

} // namespace bstone
