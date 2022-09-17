/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_MEMORY_RESOURCE_INCLUDED
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include <cstddef>

namespace bstone
{

class MemoryResource
{
public:
	MemoryResource() noexcept = default;
	virtual ~MemoryResource() = default;

	virtual void* allocate(std::size_t size) = 0;
	virtual void deallocate(void* resource) noexcept = 0;
}; // MemoryResource

// ==========================================================================

MemoryResource& get_default_memory_resource() noexcept;

} // bstone

#endif // !BSTONE_MEMORY_RESOURCE_INCLUDED
