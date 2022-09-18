/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_memory_pool.h"

#include <cassert>
#include <limits>
#include <new>
#include "bstone_exception.h"
#include "bstone_memory.h"

namespace bstone
{

namespace detail
{

class MemoryPoolException : public Exception
{
public:
	explicit MemoryPoolException(const char* message) noexcept
		:
		Exception{"MEMORY_POOL", message}
	{
	}
}; // MemoryPoolException

} // detail

MemoryPool::MemoryPool(const MemoryPoolInitParam& param)
{
	const MutexLock mutex_lock{mutex_};
	resize(param);
}

MemoryPool::MemoryPool(MemoryPool&& rhs) noexcept
{
	assert(this != std::addressof(rhs));
	const MutexLock mutex_lock{mutex_};
	const MutexLock rhs_mutex_lock{rhs.mutex_};
	blocks_.swap(rhs.blocks_);
}

void MemoryPool::clear() noexcept
{
	const MutexLock mutex_lock{mutex_};
	blocks_.clear();
}

void MemoryPool::resize(const MemoryPoolInitParam& param)
try
{
	if (param.object_size <= 0)
	{
		fail("Object size out of range.");
	}

	constexpr auto max_int = std::numeric_limits<int>::max();

	if (param.capacity <= 0 || param.capacity > (max_int / param.object_size))
	{
		fail("Capacity out of range.");
	}

	const MutexLock mutex_lock{mutex_};

	if (!blocks_.empty())
	{
		fail("Non-empty pool.");
	}

	const auto block_data_size = static_cast<std::size_t>(param.object_size) * objects_per_block;
	const auto block_count = (param.capacity + objects_per_block - 1) / objects_per_block;
	auto blocks = Blocks{};
	blocks.resize(block_count);

	for (auto& block : blocks)
	{
		block = make_block(block_data_size);
	}

	blocks_.swap(blocks);
}
catch (...)
{
	fail_nested(__func__);
}

void* MemoryPool::allocate(std::size_t size)
try
{
	const MutexLock mutex_lock{mutex_};

	if (size != static_cast<std::size_t>(object_size_))
	{
		fail("Size mismatch.");
	}

	for (auto& block : blocks_)
	{
		if (block->free_mask == BlockFreeMask{})
		{
			continue;
		}

		for (auto i = std::size_t{}; i < objects_per_block; ++i)
		{
			if (!bitwise::test(block->free_mask, i))
			{
				continue;
			}

			const auto object = block->get_data() + (i * object_size_);
			mark_created_object(*block, i);
			return object;
		}
	}

	fail("Out of memory.");
}
catch (...)
{
	fail_nested(__func__);
}

void MemoryPool::deallocate(void* object) noexcept
{
	if (!object)
	{
		return;
	}

	const MutexLock mutex_lock{mutex_};

	for (auto& block : blocks_)
	{
		const auto block_objects = block->get_data();
		const auto offset = static_cast<unsigned char*>(object) - block_objects;

		if (offset < 0)
		{
			continue;
		}

		const auto index_u = static_cast<std::size_t>(offset) / object_size_;

		if (index_u >= objects_per_block)
		{
			continue;
		}

		assert(!bitwise::test(block->free_mask, index_u));
		mark_destroyed_object(*block, index_u);
		return;
	}

	assert(false && "Unknown object's origin.");
}

unsigned char* MemoryPool::Block::get_data() noexcept
{
	return reinterpret_cast<unsigned char*>(this);
}

void MemoryPool::BlockDeleter::operator()(Block* block) const noexcept
{
	get_default_memory_resource().deallocate(block);
}

[[noreturn]] void MemoryPool::fail(const char* message)
{
	throw detail::MemoryPoolException{message};
}

[[noreturn]] void MemoryPool::fail_nested(const char* message)
{
	std::throw_with_nested(detail::MemoryPoolException{message});
}

MemoryPool::BlockUPtr MemoryPool::make_block(std::size_t block_data_size)
try
{
	auto block = static_cast<Block*>(get_default_memory_resource().allocate(sizeof(Block) + block_data_size));

	if (!block)
	{
		fail("Out of memory.");
	}

	block->free_mask = bitwise::flip(BlockFreeMask{});

	return BlockUPtr{block};
}
catch (...)
{
	fail_nested(__func__);
}

void MemoryPool::mark_destroyed_object(Block& block, std::size_t index) noexcept
{
	block.free_mask = bitwise::set(block.free_mask, index);
}

void MemoryPool::mark_created_object(Block& block, std::size_t index) noexcept
{
	block.free_mask = bitwise::reset(block.free_mask, index);
}

} // bstone
