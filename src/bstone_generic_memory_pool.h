/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Generic memory pool.

#if !defined(BSTONE_GENERIC_MEMORY_POOL_INCLUDED)
#define BSTONE_GENERIC_MEMORY_POOL_INCLUDED

#include <memory>

#include "bstone_int.h"
#include "bstone_memory_resource.h"

namespace bstone {

class GenericMemoryPool final : public MemoryResource
{
public:
	using MemoryResource::allocate;
	using MemoryResource::deallocate;

public:
	GenericMemoryPool();
	~GenericMemoryPool() override;

	void reserve(
		MemoryResourceInt object_size,
		MemoryResourceInt max_objects,
		MemoryResource& memory_resource);

	void reserve(
		MemoryResourceInt object_size,
		MemoryResourceInt max_objects);

private:
	template<typename T>
	class ResourceDeleter
	{
	public:
		ResourceDeleter() = default;

		ResourceDeleter(MemoryResource& memory_resource)
			:
			memory_resource_{&memory_resource}
		{}

		void operator()(T* ptr) const
		{
			memory_resource_->deallocate(ptr);
		}

	private:
		MemoryResource* memory_resource_{};
	};

private:
	using BitmapDeleter = ResourceDeleter<bool>;
	using StorageDeleter = ResourceDeleter<unsigned char>;

	using Bitmap = std::unique_ptr<bool[], BitmapDeleter>;
	using Storage = std::unique_ptr<unsigned char[], StorageDeleter>;

private:
	Bitmap bitmap_;
	Storage storage_;
	MemoryResourceInt object_size_{};
	MemoryResourceInt max_objects_{};
	MemoryResourceInt object_count_{};
	MemoryResourceInt bitmap_pivot_index_{};

private:
	void* do_allocate(MemoryResourceInt size) override;
	void do_deallocate(void* ptr) override;
};

} // namespace bstone

#endif // BSTONE_GENERIC_MEMORY_POOL_INCLUDED
