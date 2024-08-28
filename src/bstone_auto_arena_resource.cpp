/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Arena memory resource with auto-reseting size.

#include "bstone_assert.h"
#include "bstone_auto_arena_resource.h"
#include "bstone_exception.h"

namespace bstone {

AutoArenaResource::AutoArenaResource() noexcept
	:
	storage_{nullptr, StorageDeleter{get_null_memory_resource()}}
{}

AutoArenaResource::~AutoArenaResource() = default;

AutoArenaResource::AutoArenaResource(std::intptr_t capacity, MemoryResource& memory_resource)
	:
	storage_{
		memory_resource.allocate<char>(capacity),
		StorageDeleter{memory_resource}},
	capacity_{capacity}
{}

std::intptr_t AutoArenaResource::get_capacity() const noexcept
{
	return capacity_;
}

std::intptr_t AutoArenaResource::get_size() const noexcept
{
	return size_;
}

void AutoArenaResource::reserve(std::intptr_t capacity, MemoryResource& memory_resource)
{
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
		memory_resource.allocate<char>(capacity),
		StorageDeleter{memory_resource}
	};

	capacity_ = capacity;
}

BSTONE_CXX_NODISCARD void* AutoArenaResource::do_allocate(std::intptr_t size)
{
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

void AutoArenaResource::do_deallocate(void* ptr) noexcept
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
