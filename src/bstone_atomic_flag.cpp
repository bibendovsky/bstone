/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
	AtomicFlag{static_cast<bool>(rhs)}
{}

AtomicFlag& AtomicFlag::operator=(bool value) noexcept
{
	flag_.store(value, std::memory_order_release);
	return *this;
}

AtomicFlag& AtomicFlag::operator=(const AtomicFlag& rhs) noexcept
{
	return *this = static_cast<bool>(rhs);
}

AtomicFlag::operator bool() const noexcept
{
	return flag_.load(std::memory_order_acquire);
}

} // namespace bstone
