/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SPINLOCK_INCLUDED
#define BSTONE_SPINLOCK_INCLUDED

#include <atomic>

namespace bstone {

class Spinlock
{
public:
	Spinlock() noexcept;
	Spinlock(Spinlock&& rhs) noexcept;
	void operator=(Spinlock&& rhs) noexcept = delete;

	bool try_lock() noexcept;
	void lock() noexcept;
	void unlock() noexcept;

private:
	using Lock = std::atomic_bool;

	Lock lock_{};
};

} // namespace bstone

#endif // BSTONE_SPINLOCK_INCLUDED
