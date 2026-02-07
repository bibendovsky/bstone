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
	explicit AtomicFlag(bool value = false);
	AtomicFlag(const AtomicFlag& rhs);
	AtomicFlag& operator=(bool value);
	AtomicFlag& operator=(const AtomicFlag& rhs);

	bool get() const;
	void set(bool value);

	explicit operator bool() const;

private:
	using Flag = std::atomic_bool;

private:
	Flag flag_{};
};

} // namespace bstone

#endif // BSTONE_ATOMIC_FLAG_INCLUDED
