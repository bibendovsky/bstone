/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Matrix 4x4.
//


#ifndef BSTONE_MAT4_INCLUDED
#define BSTONE_MAT4_INCLUDED


#include <array>


namespace bstone
{


template<typename T>
class Mat4T
{
public:
	class DiagonalTag
	{
	public:
	}; // DiagonalTag


	static constexpr auto rank = 4;
	static constexpr auto item_count = rank * rank;


	constexpr Mat4T()
		:
		items_{}
	{
	}

	explicit constexpr Mat4T(
		const T filler)
		:
		items_{
			filler, filler, filler, filler,
			filler, filler, filler, filler,
			filler, filler, filler, filler,
			filler, filler, filler, filler}
	{
	}

	constexpr Mat4T(
		const T filler,
		const DiagonalTag)
		:
		items_{
			filler, T{}, T{}, T{},
			T{}, filler, T{}, T{},
			T{}, T{}, filler, T{},
			T{}, T{}, T{}, filler}
	{
	}

	constexpr Mat4T(
		const T a11,
		const T a12,
		const T a13,
		const T a14,
		const T a21,
		const T a22,
		const T a23,
		const T a24,
		const T a31,
		const T a32,
		const T a33,
		const T a34,
		const T a41,
		const T a42,
		const T a43,
		const T a44)
		:
		items_{a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41, a42, a43, a44}
	{
	}

	T& get_item(
		const int index)
	{
		return items_[index];
	}

	constexpr T get_item(
		const int index) const
	{
		return items_[index];
	}

	// Notes:
	//    - Indices are one-based.
	T& get_item(
		const int row_index,
		const int column_index)
	{
		return items_[(rank * (row_index - 1)) + column_index - 1];
	}

	// Notes:
	//    - Indices are one-based.
	constexpr T get_item(
		const int row_index,
		const int column_index) const
	{
		return items_[(rank * (row_index - 1)) + column_index - 1];
	}

	constexpr Mat4T transpose() const
	{
		return
		{
			get_item(1, 1), get_item(2, 1), get_item(3, 1), get_item(4, 1),
			get_item(1, 2), get_item(2, 2), get_item(3, 2), get_item(4, 2),
			get_item(1, 3), get_item(2, 3), get_item(3, 3), get_item(4, 3),
			get_item(1, 4), get_item(2, 4), get_item(3, 4), get_item(4, 4)
		};
	}

	T& operator[](
		const int index)
	{
		return get_item(index);
	}

	constexpr T operator[](
		const int index) const
	{
		return get_item(index);
	}

	// Notes:
	//    - Indices are one-based.
	T& operator()(
		const int row_index,
		const int column_index)
	{
		return get_item(row_index, column_index);
	}

	// Notes:
	//    - Indices are one-based.
	constexpr T operator()(
		const int row_index,
		const int column_index) const
	{
		return get_item(row_index, column_index);
	}

	static constexpr Mat4T get_identity()
	{
		return Mat4T{T{1}, DiagonalTag{}};
	}


private:
	using Items = std::array<T, item_count>;

	Items items_;
}; // Mat4T

using Mat4I = Mat4T<int>;
using Mat4F = Mat4T<float>;
using Mat4D = Mat4T<double>;


template<typename T>
constexpr Mat4T<T> operator*(
	const T rhs,
	const Mat4T<T>& lhs)
{
	return
	{
		lhs(1, 1) * rhs,
		lhs(1, 2) * rhs,
		lhs(1, 3) * rhs,
		lhs(1, 4) * rhs,

		lhs(2, 1) * rhs,
		lhs(2, 2) * rhs,
		lhs(2, 3) * rhs,
		lhs(2, 4) * rhs,

		lhs(3, 1) * rhs,
		lhs(3, 2) * rhs,
		lhs(3, 3) * rhs,
		lhs(3, 4) * rhs,

		lhs(4, 1) * rhs,
		lhs(4, 2) * rhs,
		lhs(4, 3) * rhs,
		lhs(4, 4) * rhs,
	};
}

template<typename T>
constexpr Mat4T<T> operator*(
	const Mat4T<T>& lhs,
	const T rhs)
{
	return rhs * lhs;
}

template<typename T>
constexpr Mat4T<T> operator*(
	const Mat4T<T>& lhs,
	const Mat4T<T>& rhs)
{
	return
	{
		(lhs(1, 1) * rhs(1, 1)) + (lhs(1, 2) * rhs(2, 1)) + (lhs(1, 3) * rhs(3, 1)) + (lhs(1, 4) * rhs(4, 1)),
		(lhs(1, 1) * rhs(1, 2)) + (lhs(1, 2) * rhs(2, 2)) + (lhs(1, 3) * rhs(3, 2)) + (lhs(1, 4) * rhs(4, 2)),
		(lhs(1, 1) * rhs(1, 3)) + (lhs(1, 2) * rhs(2, 3)) + (lhs(1, 3) * rhs(3, 3)) + (lhs(1, 4) * rhs(4, 3)),
		(lhs(1, 1) * rhs(1, 4)) + (lhs(1, 2) * rhs(2, 4)) + (lhs(1, 3) * rhs(3, 4)) + (lhs(1, 4) * rhs(4, 4)),

		(lhs(2, 1) * rhs(1, 1)) + (lhs(2, 2) * rhs(2, 1)) + (lhs(2, 3) * rhs(3, 1)) + (lhs(2, 4) * rhs(4, 1)),
		(lhs(2, 1) * rhs(1, 2)) + (lhs(2, 2) * rhs(2, 2)) + (lhs(2, 3) * rhs(3, 2)) + (lhs(2, 4) * rhs(4, 2)),
		(lhs(2, 1) * rhs(1, 3)) + (lhs(2, 2) * rhs(2, 3)) + (lhs(2, 3) * rhs(3, 3)) + (lhs(2, 4) * rhs(4, 3)),
		(lhs(2, 1) * rhs(1, 4)) + (lhs(2, 2) * rhs(2, 4)) + (lhs(2, 3) * rhs(3, 4)) + (lhs(2, 4) * rhs(4, 4)),

		(lhs(3, 1) * rhs(1, 1)) + (lhs(3, 2) * rhs(2, 1)) + (lhs(3, 3) * rhs(3, 1)) + (lhs(3, 4) * rhs(4, 1)),
		(lhs(3, 1) * rhs(1, 2)) + (lhs(3, 2) * rhs(2, 2)) + (lhs(3, 3) * rhs(3, 2)) + (lhs(3, 4) * rhs(4, 2)),
		(lhs(3, 1) * rhs(1, 3)) + (lhs(3, 2) * rhs(2, 3)) + (lhs(3, 3) * rhs(3, 3)) + (lhs(3, 4) * rhs(4, 3)),
		(lhs(3, 1) * rhs(1, 4)) + (lhs(3, 2) * rhs(2, 4)) + (lhs(3, 3) * rhs(3, 4)) + (lhs(3, 4) * rhs(4, 4)),

		(lhs(4, 1) * rhs(1, 1)) + (lhs(4, 2) * rhs(2, 1)) + (lhs(4, 3) * rhs(3, 1)) + (lhs(4, 4) * rhs(4, 1)),
		(lhs(4, 1) * rhs(1, 2)) + (lhs(4, 2) * rhs(2, 2)) + (lhs(4, 3) * rhs(3, 2)) + (lhs(4, 4) * rhs(4, 2)),
		(lhs(4, 1) * rhs(1, 3)) + (lhs(4, 2) * rhs(2, 3)) + (lhs(4, 3) * rhs(3, 3)) + (lhs(4, 4) * rhs(4, 3)),
		(lhs(4, 1) * rhs(1, 4)) + (lhs(4, 2) * rhs(2, 4)) + (lhs(4, 3) * rhs(3, 4)) + (lhs(4, 4) * rhs(4, 4)),
	};
}


} // bstone


#endif // !BSTONE_MAT4_INCLUDED
