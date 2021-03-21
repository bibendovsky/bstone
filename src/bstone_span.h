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


#ifndef BSTONE_SPAN_INCLUDED
#define BSTONE_SPAN_INCLUDED


#include <cassert>
#include <cstdint>


namespace bstone
{


template<
	typename T
>
class Span
{
public:
	using Size = std::intptr_t;


	constexpr Span() noexcept
	{
	}

	template<
		Size USize
	>
	constexpr Span(
		T (&array)[USize]) noexcept
		:
		pointer_{array},
		size_{USize}
	{
		assert(pointer_);
		assert(size_ >= 0);
	}

	constexpr Span(
		T* pointer,
		Size size) noexcept
		:
		pointer_{pointer},
		size_{size}
	{
		assert(pointer_);
		assert(size_ >= 0);
	}

	constexpr const T* begin() const noexcept
	{
		return pointer_;
	}

	constexpr const T* end() const noexcept
	{
		return pointer_ + size_;
	}

	constexpr T* begin() noexcept
	{
		return pointer_;
	}

	constexpr T* end() noexcept
	{
		return pointer_ + size_;
	}

	constexpr const T& operator[](
		Size index) const noexcept
	{
		assert(index >= 0 && index < size_);

		return pointer_[index];
	}

	constexpr T& operator[](
		Size index) noexcept
	{
		assert(index >= 0 && index < size_);

		return pointer_[index];
	}


private:
	T* pointer_{};
	Size size_{};
}; // Span


} // bstone


#endif // !BSTONE_SPAN_INCLUDED
