/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_atomic_flag.h"

namespace bstone {

AtomicFlag::AtomicFlag(bool value) noexcept
	:
	flag_{value}
{}

AtomicFlag::AtomicFlag(const AtomicFlag& rhs) noexcept
	:
	AtomicFlag{rhs.get()}
{}

AtomicFlag& AtomicFlag::operator=(bool value) noexcept
{
	set(value);
	return *this;
}

AtomicFlag& AtomicFlag::operator=(const AtomicFlag& rhs) noexcept
{
	return *this = rhs.get();
}

bool AtomicFlag::get() const noexcept
{
	return flag_.load(std::memory_order_acquire);
}

void AtomicFlag::set(bool value) noexcept
{
	flag_.store(value, std::memory_order_release);
}

AtomicFlag::operator bool() const noexcept
{
	return get();
}

} // namespace bstone
