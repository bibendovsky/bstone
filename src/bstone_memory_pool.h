/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_MEMORY_POOL_INCLUDED
#define BSTONE_MEMORY_POOL_INCLUDED

#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>
#include "bstone_bitwise.h"
#include "bstone_memory_resource.h"
#include "bstone_spinlock.h"

namespace bstone
{

struct MemoryPoolInitParam
{
	int object_size;
	int capacity;
}; // MemoryPoolInitParam

class MemoryPool final : public MemoryResource
{
public:
	MemoryPool() noexcept = default;
	MemoryPool(const MemoryPoolInitParam& param);
	MemoryPool(MemoryPool&& rhs) noexcept;

	void clear() noexcept;

	// Notes:
	//    - Expects the pool is empty.
	void resize(const MemoryPoolInitParam& param);

private:
	void* do_allocate(std::size_t size) override;
	void do_deallocate(void* object) noexcept override;

private:
	using Mutex = Spinlock;
	using MutexLock = std::lock_guard<Mutex>;
	using BlockFreeMask = std::size_t;

	static constexpr auto objects_per_block = bitwise::get_bit_count<BlockFreeMask>();

	struct Block
	{
		BlockFreeMask free_mask;

		unsigned char* get_data() noexcept;
	}; // Block

	static_assert(std::is_trivial<Block>::value, "Expected a trivial type.");

	struct BlockDeleter
	{
		void operator()(Block* block) const noexcept;
	}; // BlockDeleter

	using BlockUPtr = std::unique_ptr<Block, BlockDeleter>;
	using Blocks = std::vector<BlockUPtr>;

	Mutex mutex_{};
	Blocks blocks_{};
	int object_size_{};

	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);

	BlockUPtr make_block(std::size_t block_data_size);

	void mark_destroyed_object(Block& block, std::size_t index) noexcept;
	void mark_created_object(Block& block, std::size_t index) noexcept;
}; // MemoryPool

} // bstone

#endif // !BSTONE_MEMORY_POOL_INCLUDED
