/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non thread-safe memory pool for only one object.

#if !defined(BSTONE_SINGLE_MEMORY_POOL_INCLUDED)
#define BSTONE_SINGLE_MEMORY_POOL_INCLUDED

#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<typename T>
class SingleMemoryPool final : public MemoryResource
{
public:
	static constexpr auto object_size = sizeof(T);

public:
	SingleMemoryPool() = default;
	SingleMemoryPool(const SingleMemoryPool&) = delete;
	SingleMemoryPool(SingleMemoryPool&&) noexcept = delete;
	~SingleMemoryPool() override;

private:
	void* do_allocate(std::size_t size) override;
	void do_deallocate(void* ptr) override;

private:
	using Storage = unsigned char[object_size];

private:
	Storage storage_{};
	bool is_allocated_{};
};

// --------------------------------------------------------------------------

template<typename T>
SingleMemoryPool<T>::~SingleMemoryPool()
{
	if (is_allocated_)
	{
		BSTONE_THROW_STATIC_SOURCE("Unallocated pool.");
	}
}

template<typename T>
void* SingleMemoryPool<T>::do_allocate(std::size_t size)
try {
	if (size != object_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Size mismatch.");
	}

	if (is_allocated_)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of memory.");
	}

	is_allocated_ = true;
	return storage_;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename T>
void SingleMemoryPool<T>::do_deallocate(void* ptr)
{
	if (ptr == nullptr)
	{
		return;
	}

	if (ptr != storage_)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid pointer.");
	}

	is_allocated_ = false;
}

} // namespace bstone

#endif // BSTONE_SINGLE_MEMORY_POOL_INCLUDED
