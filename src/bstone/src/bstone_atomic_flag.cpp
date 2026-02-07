/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_atomic_flag.h"

namespace bstone {

AtomicFlag::AtomicFlag(bool value)
	:
	flag_{value}
{}

AtomicFlag::AtomicFlag(const AtomicFlag& rhs)
	:
	AtomicFlag{rhs.get()}
{}

AtomicFlag& AtomicFlag::operator=(bool value)
{
	set(value);
	return *this;
}

AtomicFlag& AtomicFlag::operator=(const AtomicFlag& rhs)
{
	return *this = rhs.get();
}

bool AtomicFlag::get() const
{
	return flag_.load(std::memory_order_acquire);
}

void AtomicFlag::set(bool value)
{
	flag_.store(value, std::memory_order_release);
}

AtomicFlag::operator bool() const
{
	return get();
}

} // namespace bstone
