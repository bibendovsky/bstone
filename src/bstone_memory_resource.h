/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include <cstddef>

namespace bstone {

class MemoryResource
{
public:
	MemoryResource() noexcept = default;
	virtual ~MemoryResource() = default;

	void* allocate(std::size_t size);
	void deallocate(void* resource) noexcept;

private:
	virtual void* do_allocate(std::size_t size) = 0;
	virtual void do_deallocate(void* resource) noexcept = 0;
};

// ==========================================================================

MemoryResource& get_default_memory_resource() noexcept;

} // namespace bstone

#endif // BSTONE_MEMORY_RESOURCE_INCLUDED
