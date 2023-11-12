/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Automatically clearing arena memory resource.

#include "bstone_assert.h"
#include "bstone_auto_arena_memory_resource.h"
#include "bstone_exception.h"

namespace bstone {

AutoArenaMemoryResource::StorageDeleter::StorageDeleter(MemoryResource& memory_resource) noexcept
	:
	memory_resource_{&memory_resource}
{}

void AutoArenaMemoryResource::StorageDeleter::operator()(unsigned char* ptr) const noexcept
{
	memory_resource_->deallocate(ptr);
}

// --------------------------------------------------------------------------

AutoArenaMemoryResource::AutoArenaMemoryResource() noexcept
	:
	storage_{nullptr, StorageDeleter{get_null_memory_resource()}}
{}

AutoArenaMemoryResource::~AutoArenaMemoryResource()
{
	BSTONE_ASSERT(size_ == 0);
}

void AutoArenaMemoryResource::reserve(std::intptr_t capacity, MemoryResource& memory_resource)
{
	BSTONE_ASSERT(capacity >= 0);

	if (size_ != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("In use.");
	}

	storage_ = nullptr;
	capacity_ = 0;
	size_ = 0;
	counter_ = 0;

	storage_ = Storage
	{
		static_cast<unsigned char*>(memory_resource.allocate(capacity)),
		StorageDeleter{memory_resource}
	};

	capacity_ = capacity;
}

BSTONE_CXX_NODISCARD void* AutoArenaMemoryResource::do_allocate(std::intptr_t size)
{
	BSTONE_ASSERT(size >= 0);

	const auto new_size = size > 0 ? size : 1;

	if (new_size > capacity_ - size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of memory.");
	}

	const auto ptr = storage_.get() + size_;
	size_ += new_size;
	++counter_;
	return ptr;
}

void AutoArenaMemoryResource::do_deallocate(void* ptr) noexcept
{
	if (ptr == nullptr)
	{
		return;
	}

	BSTONE_ASSERT(ptr >= storage_.get() && ptr < storage_.get() + size_);
	BSTONE_ASSERT(size_ > 0 && counter_ > 0);

	--counter_;

	if (counter_ == 0)
	{
		size_ = 0;
	}
}

} // namespace bstone
