/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_atomic_flag.h"


namespace bstone
{


AtomicFlag::AtomicFlag() noexcept = default;

AtomicFlag::AtomicFlag(
	bool value) noexcept
	:
	flag_{value}
{
}

AtomicFlag::AtomicFlag(
	const AtomicFlag& rhs)
	:
	flag_{rhs}
{
}

void AtomicFlag::operator=(
	bool value) noexcept
{
	set(value);
}

bool AtomicFlag::is_set() const noexcept
{
	return flag_.load(std::memory_order_acquire);
}

void AtomicFlag::set(
	bool value) noexcept
{
	flag_.store(value, std::memory_order_release);
}

AtomicFlag::operator bool() const noexcept
{
	return is_set();
}


} // bstone
