/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Automatically clearing arena memory resource.

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
	~AutoArenaMemoryResource() override;

	void reserve(std::intptr_t capacity, MemoryResource& memory_resource);

private:
	class StorageDeleter
	{
	public:
		StorageDeleter(MemoryResource& memory_resource) noexcept;

		void operator()(unsigned char* ptr) const noexcept;

	private:
		MemoryResource* memory_resource_{};
	};

	using Storage = std::unique_ptr<unsigned char[], StorageDeleter>;

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) noexcept override;

private:
	Storage storage_;
	std::intptr_t capacity_{};
	std::intptr_t size_{};
	std::intptr_t counter_{};
};

} // namespace bstone

#endif // BSTONE_AUTO_ARENA_MEMORY_RESOURCE_INCLUDED
