/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non-thread-safe memory pool for only one object.

#if !defined(BSTONE_MEMORY_POOL_1X_INCLUDED)
#define BSTONE_MEMORY_POOL_1X_INCLUDED

#include <cassert>
#include "bstone_exception.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<typename T>
class MemoryPool1XT final : public MemoryResource
{
public:
	static constexpr auto object_size = sizeof(T);

public:
	MemoryPool1XT() = default;
	MemoryPool1XT(const MemoryPool1XT&) = delete;
	MemoryPool1XT(MemoryPool1XT&&) noexcept = delete;
	~MemoryPool1XT();

	void* allocate(std::size_t size) override;
	void deallocate(void* resource) noexcept override;

private:
	using Storage = unsigned char[object_size];

	Storage storage_{};
	bool is_allocated_{};
};

// --------------------------------------------------------------------------

template<typename T>
MemoryPool1XT<T>::~MemoryPool1XT()
{
	assert(!is_allocated_);
}

template<typename T>
void* MemoryPool1XT<T>::allocate(std::size_t size)
try
{
	if (size != object_size)
	{
		BSTONE_STATIC_THROW("Size mismatch.");
	}

	if (is_allocated_)
	{
		BSTONE_STATIC_THROW("Out of memory.");
	}

	is_allocated_ = true;
	return storage_;
}
BSTONE_STATIC_THROW_NESTED_FUNC

template<typename T>
void MemoryPool1XT<T>::deallocate(void* resource) noexcept
{
	if (resource == nullptr)
	{
		return;
	}

	assert(resource == storage_);
	is_allocated_ = false;
}

} // namespace bstone

#endif // BSTONE_MEMORY_POOL_1X_INCLUDED
