/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Thread-safe, automatically clearing arena memory resource.

#include "bstone_ts_auto_arena_memory_resource.h"
#
namespace bstone {

void TsAutoArenaMemoryResource::reserve(std::intptr_t capacity, MemoryResource& memory_resource)
{
	MutexLock mutex_lock{mutex_};
	arena_.reserve(capacity, memory_resource);
}

BSTONE_CXX_NODISCARD void* TsAutoArenaMemoryResource::do_allocate(std::intptr_t size)
{
	MutexLock mutex_lock{mutex_};
	return arena_.allocate(size);
}

void TsAutoArenaMemoryResource::do_deallocate(void* ptr) noexcept
{
	MutexLock mutex_lock{mutex_};
	arena_.deallocate(ptr);
}

} // namespace bstone
