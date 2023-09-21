/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Memory pool with known size of an object and maximum size.

#if !defined(BSTONE_FIXED_MEMORY_POOL_INCLUDED)
#define BSTONE_FIXED_MEMORY_POOL_INCLUDED

#include <cassert>

#include "bstone_exception.h"
#include "bstone_memory_pool_bitmap.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<typename T, MemoryResourceInt TMaxSize>
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
	using Bitmap = MemoryPoolBitmap<MemoryPoolBitmapStaticStorage<max_size>>;
	using Storage = unsigned char[storage_size];

private:
	Bitmap bitmap_{};
	Storage storage_{};

private:
	void* do_allocate(MemoryResourceInt size) override;
	void do_deallocate(void* ptr) override;
};

// --------------------------------------------------------------------------

template<typename T, IntP TMaxSize>
FixedMemoryPool<T, TMaxSize>::~FixedMemoryPool()
{
	assert(bitmap_.is_empty());
}

template<typename T, IntP TMaxSize>
auto FixedMemoryPool<T, TMaxSize>::allocate() -> Value*
{
	return static_cast<Value*>(do_allocate(value_size));
}

template<typename T, IntP TMaxSize>
void* FixedMemoryPool<T, TMaxSize>::do_allocate(MemoryResourceInt size)
{
	if (size != value_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Allocation size mismatch.");
	}

	const auto index = bitmap_.set_first_free();
	const auto values = reinterpret_cast<Value*>(storage_);
	return &values[index];
}

template<typename T, IntP TMaxSize>
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
