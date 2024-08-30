/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Memory pool with known size of an object and maximum objects.

#ifndef BSTONE_FIXED_POOL_RESOURCE_INCLUDED
#define BSTONE_FIXED_POOL_RESOURCE_INCLUDED

#include "bstone_assert.h"
#include "bstone_cxx.h"
#include "bstone_exception.h"
#include "bstone_memory_pool_bitmap.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<typename TObject, std::intptr_t TMaxObjects>
class FixedPoolResource final : public MemoryResource
{
	static_assert(TMaxObjects > 0, "Max objects out of range.");

public:
	using MemoryResource::allocate;
	using MemoryResource::deallocate;

	using Object = TObject;

public:
	static constexpr auto max_objects = TMaxObjects;
	static constexpr auto object_size = static_cast<std::intptr_t>(sizeof(Object));

public:
	FixedPoolResource() noexcept;
	~FixedPoolResource() override;

private:
	static constexpr auto storage_size = object_size * max_objects;

private:
	using Bitmap = MemoryPoolBitmap<MemoryPoolBitmapStaticStorage<max_objects>>;
	using Storage = unsigned char[storage_size];

private:
	Bitmap bitmap_{};
	Storage storage_{};

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) noexcept override;
};

// --------------------------------------------------------------------------

template<typename TObject, std::intptr_t TMaxObjects>
FixedPoolResource<TObject, TMaxObjects>::FixedPoolResource() noexcept = default;

template<typename TObject, std::intptr_t TMaxObjects>
FixedPoolResource<TObject, TMaxObjects>::~FixedPoolResource()
{
	BSTONE_ASSERT(bitmap_.is_empty());
}

template<typename TObject, std::intptr_t TMaxObjects>
BSTONE_CXX_NODISCARD void* FixedPoolResource<TObject, TMaxObjects>::do_allocate(std::intptr_t size)
{
	if (size != object_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Allocation size mismatch.");
	}

	const auto index = bitmap_.set_first_free();
	const auto values = reinterpret_cast<Object*>(storage_);
	return &values[index];
}

template<typename TObject, std::intptr_t TMaxObjects>
void FixedPoolResource<TObject, TMaxObjects>::do_deallocate(void* ptr) noexcept
{
	if (ptr == nullptr)
	{
		return;
	}

	const auto value = static_cast<Object*>(ptr);
	const auto values = reinterpret_cast<Object*>(storage_);
	const auto index = value - values;
	bitmap_.reset(index);
}

} // namespace bstone

#endif // BSTONE_FIXED_POOL_RESOURCE_INCLUDED
