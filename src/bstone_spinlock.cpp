/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_spinlock.h"

#include <cassert>
#include <tuple>

namespace bstone
{

Spinlock::Spinlock() noexcept = default;

Spinlock::Spinlock(Spinlock&& rhs) noexcept
{
	std::ignore = rhs;
}

void Spinlock::operator=(Spinlock&& rhs) noexcept
{
	std::ignore = rhs;
}

Spinlock::~Spinlock() = default;

bool Spinlock::try_lock() noexcept
{
	return !flag_.test_and_set(std::memory_order_acquire);
}

void Spinlock::lock() noexcept
{
	while (flag_.test_and_set(std::memory_order_acquire))
	{
	}
}

void Spinlock::unlock() noexcept
{
	flag_.clear(std::memory_order_release);
}

} // bstone
