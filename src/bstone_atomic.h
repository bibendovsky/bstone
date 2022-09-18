/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// A wrapper for std::atomic with copy ability
//


#ifndef BSTONE_ATOMIC_INCLUDED
#define BSTONE_ATOMIC_INCLUDED


#include <atomic>
#include <memory>
#include <type_traits>


namespace bstone
{


template<typename T>
class Atomic
{
public:
	Atomic()
		:
		value_{T{}}
	{
	}

	Atomic(
		const T& value)
		:
		value_{value}
	{
	}

	Atomic(
		const Atomic& that)
		:
		value_{that.value_.load()}
	{
	}

	Atomic& operator=(
		const T& value)
	{
		value_ = value;
		return *this;
	}

	Atomic& operator=(
		const Atomic& that)
	{
		if (std::addressof(that) != this)
		{
			value_ = that.value_.load();
		}

		return *this;
	}

	~Atomic()
	{
	}

	operator T() const
	{
		return value_.load(std::memory_order_acquire);
	}

	Atomic& operator-=(
		typename std::enable_if<std::is_integral<T>::value, const T&>::type value)
	{
		value_ -= value;
		return *this;
	}

	Atomic& operator+=(
		typename std::enable_if<std::is_integral<T>::value, const T&>::type value)
	{
		value_ += value;
		return *this;
	}


private:
	std::atomic<T> value_;
}; // Atomic


} // bstone


#endif // !BSTONE_ATOMIC_INCLUDED
