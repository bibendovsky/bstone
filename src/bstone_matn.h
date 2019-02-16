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
// Square matrix.
//


#ifndef BSTONE_MATN_INCLUDED
#define BSTONE_MATN_INCLUDED


#include <array>


namespace bstone
{


struct MatNDiagonalTag
{
}; // MatNDiagonalTag

template<int N, typename T>
class MatNT
{
public:
	static_assert(N >= 2 && N <= 4, "Unsupported rank.");


	constexpr MatNT()
		:
		items_{}
	{
	}

	explicit constexpr MatNT(
		const T& filler)
	{
		items_.fill(filler);
	}

	constexpr MatNT(
		const T& filler,
		const MatNDiagonalTag)
		:
		items_{}
	{
		for (int i = 1; i <= N; ++i)
		{
			get_item(i, i) = filler;
		}
	}

	template<typename... TArgs>
	constexpr MatNT(
		const TArgs&... args)
		:
		items_{args...}
	{
		static_assert(sizeof...(TArgs) == (N * N), "Argument count mismatch.");
	}

	// Notes:
	//    - Indices are one-based.
	static constexpr int get_item_index(
		const int row_index,
		const int column_index)
	{
		return (N * (row_index - 1)) + column_index - 1;
	}

	constexpr T& get_item(
		const int index)
	{
		return items_[index];
	}

	constexpr const T& get_item(
		const int index) const
	{
		return items_[index];
	}

	// Notes:
	//    - Indices are one-based.
	constexpr T& get_item(
		const int row_index,
		const int column_index)
	{
		return items_[get_item_index(row_index, column_index)];
	}

	// Notes:
	//    - Indices are one-based.
	constexpr const T& get_item(
		const int row_index,
		const int column_index) const
	{
		return items_[get_item_index(row_index, column_index)];
	}

	constexpr T* get_data()
	{
		return items_.data();
	}

	constexpr const T* get_data() const
	{
		return items_.data();
	}

	constexpr MatNT transpose() const
	{
		MatNT result;

		for (int i_row = 1; i_row <= N; ++i_row)
		{
			for (int i_column = 1; i_column <= N; ++i_column)
			{
				result(i_column, i_row) = get_item(i_row, i_column);
			}
		}

		return result;
	}

	static constexpr bool are_equal(
		const MatNT& lhs,
		const MatNT& rhs)
	{
		return lhs.items_ == rhs.items_;
	}

	T& operator[](
		const int index)
	{
		return get_item(index);
	}

	constexpr const T& operator[](
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
	constexpr const T& operator()(
		const int row_index,
		const int column_index) const
	{
		return get_item(row_index, column_index);
	}

	static constexpr MatNT get_identity()
	{
		return MatNT{T{1}, MatNDiagonalTag{}};
	}


private:
	using Items = std::array<T, N * N>;


	Items items_;
}; // MatNT

using Mat4I = MatNT<4, int>;
using Mat4F = MatNT<4, float>;
using Mat4D = MatNT<4, double>;


template<int N, typename T>
constexpr bool operator==(
	const MatNT<N, T>& lhs,
	const MatNT<N, T>& rhs)
{
	return MatNT<N, T>::are_equal(lhs, rhs);
}

template<int N, typename T>
constexpr bool operator!=(
	const MatNT<N, T>& lhs,
	const MatNT<N, T>& rhs)
{
	return !(lhs == rhs);
}

template<int N, typename T>
constexpr MatNT<N, T> operator*(
	const T lhs,
	const MatNT<N, T>& rhs)
{
	MatNT<N, T> result;

	for (int i = 0; i < (N * N); ++i)
	{
		result[i] = lhs * rhs[i];
	}

	return result;
}

template<int N, typename T>
constexpr MatNT<N, T> operator*(
	const MatNT<N, T>& lhs,
	const T rhs)
{
	return rhs * lhs;
}

template<int N, typename T>
constexpr MatNT<N, T> operator*(
	const MatNT<N, T>& lhs,
	const MatNT<N, T>& rhs)
{
	MatNT<N, T> result;

	for (int i = 1; i <= N; ++i)
	{
		for (int j = 1; j <= N; ++j)
		{
			auto sum = T{};

			for (int k = 1; k <= N; ++k)
			{
				sum += lhs(i, k) * rhs(k, j);
			}

			result(i, j) = sum;
		}
	}

	return result;
}


} // bstone


#endif // !BSTONE_MATN_INCLUDED
