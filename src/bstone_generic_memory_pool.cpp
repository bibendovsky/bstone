/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Generic memory pool.

#include <cassert>

#include <limits>
#include <memory>

#include "bstone_exception.h"
#include "bstone_generic_memory_pool.h"

namespace bstone {

GenericMemoryPool::GenericMemoryPool()
	:
	bitmap_{nullptr, BitmapDeleter{get_default_memory_resource()}},
	storage_{nullptr, StorageDeleter{get_default_memory_resource()}}
{}

GenericMemoryPool::~GenericMemoryPool()
{
	assert(object_count_ == 0);
}

void GenericMemoryPool::reserve(
	GenericMemoryPoolInt object_size,
	GenericMemoryPoolInt max_objects,
	MemoryResource& memory_resource)
{
	if (object_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Object size out of range.");
	}

	if (max_objects < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Max object count out of range.");
	}

	constexpr auto max_objects_value = std::numeric_limits<GenericMemoryPoolInt>::max();

	if (max_objects > max_objects_value / object_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Max object count out of range.");
	}

	if (object_count_ != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("In use.");
	}

	bitmap_ = nullptr;
	storage_ = nullptr;
	object_size_ = 0;
	max_objects_ = 0;
	object_count_ = 0;
	bitmap_pivot_index_ = 0;

	if (max_objects == 0)
	{
		return;
	}

	auto bitmap = Bitmap
	{
		static_cast<bool*>(memory_resource.allocate(max_objects)),
		BitmapDeleter{memory_resource}
	};

	std::uninitialized_fill_n(bitmap.get(), max_objects, false);

	const auto storage_size = static_cast<std::size_t>(object_size * max_objects);

	auto storage = Storage
	{
		static_cast<unsigned char*>(memory_resource.allocate(storage_size)),
		StorageDeleter{memory_resource}
	};

	bitmap_.swap(bitmap);
	storage_.swap(storage);
	object_size_ = object_size;
	max_objects_ = max_objects;
}

void GenericMemoryPool::reserve(
	GenericMemoryPoolInt object_size,
	GenericMemoryPoolInt max_size)
{
	reserve(object_size, max_size, get_default_memory_resource());
}

void* GenericMemoryPool::do_allocate(std::size_t size)
{
	if (size != static_cast<std::size_t>(object_size_))
	{
		BSTONE_THROW_STATIC_SOURCE("Object size mismatch.");
	}

	constexpr auto out_of_memory_message = "Out of memory.";

	if (object_count_ == max_objects_)
	{
		BSTONE_THROW_STATIC_SOURCE(out_of_memory_message);
	}

	struct IndexPair
	{
		GenericMemoryPoolInt from;
		GenericMemoryPoolInt to;
	};

	const IndexPair index_pairs[2] = {{bitmap_pivot_index_, max_objects_}, {0, bitmap_pivot_index_}};

	for (const auto& index_pair : index_pairs)
	{
		for (auto i = index_pair.from; i < index_pair.to; ++i)
		{
			if (!bitmap_[i])
			{
				bitmap_[i] = true;
				++object_count_;
				bitmap_pivot_index_ = i;
				return storage_.get() + i * object_size_;
			}
		}
	}

	BSTONE_THROW_STATIC_SOURCE(out_of_memory_message);
}

void GenericMemoryPool::do_deallocate(void* ptr)
{
	if (ptr == nullptr)
	{
		return;
	}

	const auto index = (static_cast<unsigned char*>(ptr) - storage_.get()) / object_size_;
	assert(index >= 0 && index < max_objects_);

	bitmap_[index] = false;
	--object_count_;
	bitmap_pivot_index_ = index;
}

} // namespace bstone
