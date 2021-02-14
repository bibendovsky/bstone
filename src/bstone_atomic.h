/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
