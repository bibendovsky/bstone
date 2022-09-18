/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_ATOMIC_FLAG_INCLUDED
#define BSTONE_ATOMIC_FLAG_INCLUDED


#include <atomic>


namespace bstone
{


class AtomicFlag
{
public:
	AtomicFlag() noexcept;

	AtomicFlag(
		bool value) noexcept;

	AtomicFlag(
		const AtomicFlag& rhs);

	void operator=(
		bool value) noexcept;

	AtomicFlag& operator=(
		const AtomicFlag& rhs) = delete;


	bool is_set() const noexcept;

	void set(
		bool value) noexcept;


	operator bool() const noexcept;


private:
	using Flag = std::atomic_bool;

	Flag flag_{};
}; // AtomicFlag


} // bstone


#endif // !BSTONE_ATOMIC_FLAG_INCLUDED
