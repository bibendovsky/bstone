/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
