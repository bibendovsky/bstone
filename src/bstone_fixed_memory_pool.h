/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non thread-safe, fixed-size memory pool.

#if !defined(BSTONE_FIXED_MEMORY_POOL_INCLUDED)
#define BSTONE_FIXED_MEMORY_POOL_INCLUDED

#include "bstone_allocator_bitmap.h"
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<typename T, Int TMaxSize>
class FixedMemoryPool final : public MemoryResource
{
	static_assert(TMaxSize > 0, "Invalid max size.");

public:
	using MemoryResource::allocate;
	using MemoryResource::deallocate;

	using Value = T;
	using Size = decltype(TMaxSize);

public:
	static constexpr auto max_size = TMaxSize;
	static constexpr auto value_size = static_cast<Size>(sizeof(Value));

public:
	FixedMemoryPool() = default;
	~FixedMemoryPool() override;

	Value* allocate();

private:
	static constexpr auto storage_size = value_size * max_size;

private:
	using Bitmap = AllocatorBitmap<max_size>;
	using Storage = unsigned char[storage_size];

private:
	Bitmap bitmap_{};
	Storage storage_{};

private:
	void* do_allocate(std::size_t size) override;
	void do_deallocate(void* ptr) override;
};

// --------------------------------------------------------------------------

template<typename T, Int TMaxSize>
FixedMemoryPool<T, TMaxSize>::~FixedMemoryPool()
{
	if (!bitmap_.is_empty())
	{
		BSTONE_STATIC_THROW("Unallocated pool.");
	}
}

template<typename T, Int TMaxSize>
auto FixedMemoryPool<T, TMaxSize>::allocate() -> Value*
{
	return static_cast<Value*>(do_allocate(value_size));
}

template<typename T, Int TMaxSize>
void* FixedMemoryPool<T, TMaxSize>::do_allocate(std::size_t size)
{
	if (size != value_size)
	{
		BSTONE_STATIC_THROW("Allocation size mismatch.");
	}

	const auto index = bitmap_.set_first_free();
	const auto values = reinterpret_cast<Value*>(storage_);
	return &values[index];
}

template<typename T, Int TMaxSize>
void FixedMemoryPool<T, TMaxSize>::do_deallocate(void* ptr)
{
	if (ptr == nullptr)
	{
		return;
	}

	const auto value = static_cast<Value*>(ptr);
	const auto values = reinterpret_cast<Value*>(storage_);
	const auto index = value - values;
	bitmap_.reset(index);
}

} // namespace bstone

#endif // BSTONE_FIXED_MEMORY_POOL_INCLUDED
