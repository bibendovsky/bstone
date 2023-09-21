/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Generic memory pool.

#include <cassert>

#include <limits>

#include "bstone_exception.h"
#include "bstone_generic_memory_pool.h"

namespace bstone {

GenericMemoryPool::StorageDeleter::StorageDeleter()
	:
	memory_resource_{&get_null_memory_resource()}
{}

GenericMemoryPool::StorageDeleter::StorageDeleter(MemoryResource& memory_resource)
	:
	memory_resource_{&memory_resource}
{}

void GenericMemoryPool::StorageDeleter::operator()(unsigned char* ptr) const
{
	memory_resource_->deallocate(ptr);
}

// ==========================================================================

GenericMemoryPool::GenericMemoryPool()
	:
	storage_{nullptr, StorageDeleter{get_null_memory_resource()}}
{}

GenericMemoryPool::~GenericMemoryPool()
{
	assert(object_count_ == 0);
}

void GenericMemoryPool::reserve(
	MemoryResourceInt object_size,
	MemoryResourceInt max_objects,
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

	constexpr auto max_objects_value = std::numeric_limits<MemoryResourceInt>::max();

	if (max_objects > max_objects_value / object_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Max object count out of range.");
	}

	if (object_count_ != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("In use.");
	}

	bitmap_.reset_storage();
	storage_ = nullptr;
	object_size_ = 0;
	max_objects_ = 0;
	object_count_ = 0;

	if (max_objects == 0)
	{
		return;
	}

	const auto storage_size = object_size * max_objects;

	auto storage = Storage
	{
		static_cast<unsigned char*>(memory_resource.allocate(storage_size)),
		StorageDeleter{memory_resource}
	};

	bitmap_.resize(max_objects, memory_resource);
	storage_.swap(storage);
	object_size_ = object_size;
	max_objects_ = max_objects;
}

void* GenericMemoryPool::do_allocate(MemoryResourceInt size)
{
	if (size != object_size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Object size mismatch.");
	}

	constexpr auto out_of_memory_message = "Out of memory.";

	if (object_count_ == max_objects_)
	{
		BSTONE_THROW_STATIC_SOURCE(out_of_memory_message);
	}

	const auto index = bitmap_.set_first_free();
	++object_count_;
	return storage_.get() + index * object_size_;
}

void GenericMemoryPool::do_deallocate(void* ptr)
{
	if (ptr == nullptr)
	{
		return;
	}

	const auto index = (static_cast<unsigned char*>(ptr) - storage_.get()) / object_size_;
	assert(index >= 0 && index < max_objects_);

	bitmap_.reset(index);
	--object_count_;
}

} // namespace bstone
