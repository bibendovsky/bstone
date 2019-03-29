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
// Row-major square matrix.
//


#ifndef BSTONE_MATN_INCLUDED
#define BSTONE_MATN_INCLUDED


#include <array>
#include <initializer_list>


namespace bstone
{


struct MatNDiagonalTag{};

template<int N, typename T>
class MatNT
{
public:
	static_assert(N >= 2 && N <= 4, "Unsupported rank.");


	using Items = std::array<T, N * N>;
	using Iterator = typename Items::iterator;
	using CIterator = typename Items::const_iterator;


	MatNT()
		:
		items_{}
	{
	}

	MatNT(
		const T& filler,
		const MatNDiagonalTag)
		:
		MatNT{}
	{
		for (int i = 1; i <= N; ++i)
		{
			get_item(i, i) = filler;
		}
	}

	explicit MatNT(
		const std::initializer_list<T>& list)
	{
		const auto list_size = list.size();

		if (list_size == 1)
		{
			items_.fill(*list.begin());
		}
		else if (list_size == (N * N))
		{
			std::uninitialized_copy(list.begin(), list.end(), begin());
		}
		else
		{
			throw "Invalid argument count.";
		}
	}

	Iterator begin()
	{
		return items_.begin();
	}

	CIterator begin() const
	{
		return items_.cbegin();
	}

	CIterator cbegin() const
	{
		return begin();
	}

	Iterator end()
	{
		return items_.end();
	}

	CIterator end() const
	{
		return items_.cend();
	}

	CIterator cend() const
	{
		return end();
	}

	// Notes:
	//    - Indices are one-based.
	static int get_item_index(
		const int row_index,
		const int column_index)
	{
		return (N * (row_index - 1)) + column_index - 1;
	}

	T& get_item(
		const int index)
	{
		return items_[index];
	}

	const T& get_item(
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
		return items_[get_item_index(row_index, column_index)];
	}

	// Notes:
	//    - Indices are one-based.
	const T& get_item(
		const int row_index,
		const int column_index) const
	{
		return items_[get_item_index(row_index, column_index)];
	}

	T* get_data()
	{
		return items_.data();
	}

	const T* get_data() const
	{
		return items_.data();
	}

	MatNT transpose() const
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

	static bool are_equal(
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

	const T& operator[](
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
	const T& operator()(
		const int row_index,
		const int column_index) const
	{
		return get_item(row_index, column_index);
	}

	MatNT scale(
		const T& scalar) const
	{
		MatNT result;

		std::transform(
			cbegin(),
			cend()
			begin(),
			[&](const T& item)
			{
				return scalar * item;
			}
		);

		return result;
	}

	template<typename U>
	MatNT<N, U> cast() const
	{
		MatNT<N, U> result;

		std::transform(
			cbegin(),
			cend(),
			result.begin(),
			[](const auto& item)
			{
				return static_cast<U>(item);
			}
		);

		return result;
	}

	static const MatNT& get_identity()
	{
		static auto result = MatNT{T{1}, MatNDiagonalTag{}};

		return result;
	}


private:
	Items items_;
}; // MatNT

using Mat4I = MatNT<4, int>;
using Mat4F = MatNT<4, float>;
using Mat4D = MatNT<4, double>;


template<int N, typename T>
inline bool operator==(
	const MatNT<N, T>& lhs,
	const MatNT<N, T>& rhs)
{
	return MatNT<N, T>::are_equal(lhs, rhs);
}

template<int N, typename T>
inline bool operator!=(
	const MatNT<N, T>& lhs,
	const MatNT<N, T>& rhs)
{
	return !(lhs == rhs);
}

template<int N, typename T>
inline MatNT<N, T> operator*(
	const T& lhs,
	const MatNT<N, T>& rhs)
{
	return rhs.scale(lhs);
}

template<int N, typename T>
inline MatNT<N, T> operator*(
	const MatNT<N, T>& lhs,
	const T& rhs)
{
	return rhs * lhs;
}

template<int N, typename T>
inline MatNT<N, T> operator*(
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
