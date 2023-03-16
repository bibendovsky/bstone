/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_NHSZ_MEMORY_POOL_INCLUDED)
#define BSTONE_NHSZ_MEMORY_POOL_INCLUDED

#include <cassert>
#include "bstone_allocator_bitmap.h"
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

// Non-thread-safe, non-heap memory pool with known maximum objects and it's size at compile time.
template<typename T, std::size_t TCount>
class NhszMemoryPool final : public MemoryResource
{
public:
	using Type = T;
	static constexpr auto object_size = sizeof(Type);
	static constexpr auto max_objects = TCount;

public:
	NhszMemoryPool() = default;
	NhszMemoryPool(const NhszMemoryPool&) = delete;
	NhszMemoryPool& operator=(const NhszMemoryPool&) = delete;

	void* allocate(std::size_t size) override;
	void deallocate(void* resource) noexcept override;

private:
	static constexpr auto storage_size = object_size * max_objects;
	using Storage = unsigned char[storage_size];

	using Bitmap = AllocatorBitmap<max_objects>;

	Storage storage_{};
	Bitmap bitmap_{};
};

// ==========================================================================

template<typename T, std::size_t TCount>
void* NhszMemoryPool<T, TCount>::allocate(std::size_t size)
try
{
	if (size != object_size)
	{
		BSTONE_STATIC_THROW("Object size mismatch.");
	}

	const auto object_index = bitmap_.set_first_free();
	const auto object_offset = object_index * object_size;
	return storage_ + object_offset;
}
BSTONE_STATIC_THROW_NESTED_FUNC

template<typename T, std::size_t TCount>
void NhszMemoryPool<T, TCount>::deallocate(void* resource) noexcept
{
	if (resource == nullptr)
	{
		return;
	}

	assert(resource >= storage_ && resource < (storage_ + storage_size));
	const auto object_index = static_cast<const Type*>(resource) - reinterpret_cast<const Type*>(storage_);
	bitmap_.reset(object_index);
}

} // namespace bstone

#endif // BSTONE_NHSZ_MEMORY_POOL_INCLUDED
