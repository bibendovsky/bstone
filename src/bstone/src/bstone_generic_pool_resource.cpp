/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Generic memory pool.

#include <limits>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_generic_pool_resource.h"

namespace bstone {

GenericPoolResource::StorageDeleter::StorageDeleter() noexcept
	:
	memory_resource_{&get_null_memory_resource()}
{}

GenericPoolResource::StorageDeleter::StorageDeleter(MemoryResource& memory_resource) noexcept
	:
	memory_resource_{&memory_resource}
{}

void GenericPoolResource::StorageDeleter::operator()(unsigned char* ptr) const noexcept
{
	memory_resource_->deallocate(ptr);
}

// ==========================================================================

GenericPoolResource::GenericPoolResource() noexcept
	:
	storage_{nullptr, StorageDeleter{get_null_memory_resource()}}
{}

GenericPoolResource::~GenericPoolResource()
{
	BSTONE_ASSERT(object_count_ == 0);
}

void GenericPoolResource::reserve(
	std::intptr_t object_size,
	std::intptr_t max_objects,
	MemoryResource& memory_resource)
{
	BSTONE_ASSERT(object_size > 0);
	BSTONE_ASSERT(max_objects >= 0);

	constexpr auto max_objects_value = std::numeric_limits<std::intptr_t>::max();

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

BSTONE_CXX_NODISCARD void* GenericPoolResource::do_allocate(std::intptr_t size)
{
	if (size != object_size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Object size mismatch.");
	}

	if (object_count_ == max_objects_)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of memory.");
	}

	const auto index = bitmap_.set_first_free();
	++object_count_;
	return storage_.get() + index * object_size_;
}

void GenericPoolResource::do_deallocate(void* ptr) noexcept
{
	if (ptr == nullptr)
	{
		return;
	}

	const auto index = (static_cast<unsigned char*>(ptr) - storage_.get()) / object_size_;
	BSTONE_ASSERT(index >= 0 && index < max_objects_);

	bitmap_.reset(index);
	--object_count_;
}

} // namespace bstone
