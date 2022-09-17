/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_memory_resource.h"

#include <new>

namespace bstone
{

namespace
{

class DefaultMemoryResource final : public MemoryResource
{
public:
	DefaultMemoryResource() noexcept = default;

	void* allocate(std::size_t size) override
	{
		return ::operator new[](size);
	}

	void deallocate(void* resource) noexcept override
	{
		::operator delete[](resource);
	}
}; // DefaultMemoryResource

DefaultMemoryResource default_memory_resource{};

} // namespace

// ==========================================================================

MemoryResource& get_default_memory_resource() noexcept
{
	return default_memory_resource;
}

} // bstone
