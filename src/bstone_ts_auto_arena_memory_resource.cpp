/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Thread-safe, automatically clearing arena memory resource.

#include <cassert>

#include "bstone_ts_auto_arena_memory_resource.h"
#include "bstone_exception.h"

namespace bstone {

void TsAutoArenaMemoryResource::reserve(IntP capacity, MemoryResource& memory_resource)
try {
	MutexLock mutex_lock{mutex_};
	arena_.reserve(capacity, memory_resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* TsAutoArenaMemoryResource::do_allocate(IntP size)
try {
	MutexLock mutex_lock{mutex_};
	return arena_.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void TsAutoArenaMemoryResource::do_deallocate(void* ptr)
try {
	MutexLock mutex_lock{mutex_};
	arena_.deallocate(ptr);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
