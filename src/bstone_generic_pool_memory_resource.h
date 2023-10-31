/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Generic memory pool.

#if !defined(BSTONE_GENERIC_POOL_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_GENERIC_POOL_MEMORY_RESOURCE_INCLUDED

#include <cstdint>

#include <memory>

#include "bstone_memory_pool_bitmap.h"
#include "bstone_memory_resource.h"

namespace bstone {

class GenericPoolMemoryResource final : public MemoryResource
{
public:
	using MemoryResource::allocate;
	using MemoryResource::deallocate;

public:
	GenericPoolMemoryResource();
	~GenericPoolMemoryResource() override;

	void reserve(std::intptr_t object_size, std::intptr_t max_objects, MemoryResource& memory_resource);

private:
	class StorageDeleter
	{
	public:
		StorageDeleter();
		StorageDeleter(MemoryResource& memory_resource);
		void operator()(unsigned char* ptr) const;

	private:
		MemoryResource* memory_resource_{};
	};

private:
	using Bitmap = DynamicMemoryPoolBitmap;
	using Storage = std::unique_ptr<unsigned char[], StorageDeleter>;

private:
	Bitmap bitmap_{};
	Storage storage_;
	std::intptr_t object_size_{};
	std::intptr_t max_objects_{};
	std::intptr_t object_count_{};

private:
	void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) override;
};

} // namespace bstone

#endif // BSTONE_GENERIC_POOL_MEMORY_RESOURCE_INCLUDED
