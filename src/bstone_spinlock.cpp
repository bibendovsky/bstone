/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(_MSC_VER)
#include <thread>
#endif
#include "bstone_spinlock.h"

namespace bstone {

Spinlock::Spinlock() noexcept = default;

Spinlock::Spinlock(Spinlock&&) noexcept
{
}

bool Spinlock::try_lock() noexcept
{
	// First do a relaxed load to check if lock is free in order to prevent
    // unnecessary cache misses if someone does `while (!try_lock())`.
	return
		!lock_.load(std::memory_order_relaxed) &&
		!lock_.exchange(true, std::memory_order_acquire);
}

void Spinlock::lock() noexcept
{
	while (true)
	{
		// Optimistically assume the lock is free on the first try.
		if (!lock_.exchange(true, std::memory_order_acquire))
		{
			return;
		}

		// Wait for lock to be released without generating cache misses.
		while (lock_.load(std::memory_order_relaxed))
		{
#if defined(_MSC_VER)
			_mm_pause();
#else
			std::this_thread::yield();
#endif
		}
	}
}

void Spinlock::unlock() noexcept
{
	lock_.store(false, std::memory_order_release);
}

} // namespace bstone
