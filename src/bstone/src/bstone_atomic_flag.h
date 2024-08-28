/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ATOMIC_FLAG_INCLUDED
#define BSTONE_ATOMIC_FLAG_INCLUDED

#include <atomic>

namespace bstone {

class AtomicFlag
{
public:
	explicit AtomicFlag(bool value = false) noexcept;
	AtomicFlag(const AtomicFlag& rhs) noexcept;
	AtomicFlag& operator=(bool value) noexcept;
	AtomicFlag& operator=(const AtomicFlag& rhs) noexcept;

	bool get() const noexcept;
	void set(bool value) noexcept;

	explicit operator bool() const noexcept;

private:
	using Flag = std::atomic_bool;

private:
	Flag flag_{};
};

} // namespace bstone

#endif // BSTONE_ATOMIC_FLAG_INCLUDED
