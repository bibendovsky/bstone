/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Arena memory resource with auto-reseting size.

#if !defined(BSTONE_AUTO_ARENA_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_AUTO_ARENA_MEMORY_RESOURCE_INCLUDED

#include <cstdint>

#include <memory>

#include "bstone_cxx.h"
#include "bstone_memory_resource.h"

namespace bstone {

class AutoArenaMemoryResource : public MemoryResource
{
public:
	AutoArenaMemoryResource() noexcept;
	AutoArenaMemoryResource(std::intptr_t capacity, MemoryResource& memory_resource);
	~AutoArenaMemoryResource() override = default;

	std::intptr_t get_capacity() const noexcept;
	std::intptr_t get_size() const noexcept;

	void reserve(std::intptr_t capacity, MemoryResource& memory_resource);

private:
	using Storage = MemoryResourceUPtr<char[]>;
	using StorageDeleter = typename Storage::deleter_type;

private:
	Storage storage_;
	std::intptr_t capacity_{};
	std::intptr_t size_{};
	std::intptr_t counter_{};

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) noexcept override;
};

} // namespace bstone

#endif // BSTONE_AUTO_ARENA_MEMORY_RESOURCE_INCLUDED
