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
// The N-dimensional vector.
//



#ifndef BSTONE_VECN_INCLUDED
#define BSTONE_VECN_INCLUDED


#include <algorithm>
#include <array>
#include <numeric>


namespace bstone
{


template<int N, typename T>
class VecNT
{
public:
	static_assert(N >= 2 && N <= 4, "Unsupported dimension.");


	constexpr VecNT()
		:
		items_{}
	{
	}

	template<typename... TArgs>
	constexpr VecNT(
		const TArgs&... args)
		:
		items_{args...}
	{
		static_assert(sizeof...(TArgs) == N, "Argument count mismatch.");
	}


	T* get_data()
	{
		return items_.data();
	}

	const T* get_data() const
	{
		return items_.data();
	}


	T& operator[](
		const int index)
	{
		return items_[index];
	}

	constexpr const T& operator[](
		const int index) const
	{
		return items_[index];
	}

	T get_square_magnitude() const
	{
		return std::accumulate(
			items_.cbegin(),
			items_.cend(),
			T{},
			[](const auto& sum, const auto& value)
			{
				return sum + (value * value);
			}
		);
	}

	T get_magnitude() const
	{
		const auto square_magnitude = get_square_magnitude();

		return std::sqrt(square_magnitude);
	}

	static constexpr VecNT scale(
		const T& scalar)
	{
		VecNT result;

		std::transform(
			items_.cbegin(),
			items_.cend()
			items_.begin(),
			[=](const T& item)
			{
				return scalar * item;
			}
		);

		return result;
	}

	static constexpr bool are_equal(
		const VecNT& lhs,
		const VecNT& rhs)
	{
		return lhs.items_ == rhs.items_;
	}

	static constexpr VecNT add(
		const VecNT& lhs,
		const VecNT& rhs)
	{
		VecNT result;

		std::transform(
			lhs.items_.cbegin(),
			lhs.items_.cend(),
			rhs.items_.cbegin(),
			result.items_.begin(),
			std::plus<T>{}
		);

		return result;
	}

	static constexpr VecNT sub(
		const VecNT& lhs,
		const VecNT& rhs)
	{
		VecNT result;

		std::transform(
			lhs.items_.cbegin(),
			lhs.items_.cend(),
			rhs.items_.cbegin(),
			result.items_.begin(),
			std::minus<T>{}
		);

		return result;
	}

	static constexpr T dot_product(
		const VecNT& lhs,
		const VecNT& rhs)
	{
		auto result = T{};

		for (int i = 0; i < N; ++i)
		{
			result += lhs.items_[i] * rhs.items_[i];
		}

		return result;
	}


private:
	using Items = std::array<T, N>;


	Items items_;
}; // VecNT


using Vec2I = VecNT<2, int>;
using Vec2F = VecNT<2, float>;
using Vec2D = VecNT<2, double>;

using Vec3I = VecNT<3, int>;
using Vec3F = VecNT<3, float>;
using Vec3D = VecNT<3, double>;

using Vec4I = VecNT<4, int>;
using Vec4F = VecNT<4, float>;
using Vec4D = VecNT<4, double>;


} // bstone


template<int N, typename T>
constexpr bool operator==(
	const bstone::VecNT<N, T>& lhs,
	const bstone::VecNT<N, T>& rhs)
{
	return lhs.are_equal(lhs, rhs);
}

template<int N, typename T>
constexpr bool operator!=(
	const bstone::VecNT<N, T>& lhs,
	const bstone::VecNT<N, T>& rhs)
{
	return !(lhs == rhs);
}


template<int N, typename T>
constexpr bstone::VecNT<N, T> operator+(
	const bstone::VecNT<N, T>& lhs,
	const bstone::VecNT<N, T>& rhs)
{
	return lhs.add(lhs, rhs);
}

template<int N, typename T>
constexpr bstone::VecNT<N, T> operator-(
	const bstone::VecNT<N, T>& lhs,
	const bstone::VecNT<N, T>& rhs)
{
	return lhs.sub(lhs, rhs);
}


template<int N, typename T>
constexpr bstone::VecNT<N, T> operator*(
	const T& scalar,
	const bstone::VecNT<N, T>& rhs)
{
	return rhs.scale(scalar);
}

template<int N, typename T>
constexpr bstone::VecNT<N, T> operator*(
	const bstone::VecNT<N, T>& lhs,
	const T& scalar)
{
	return lhs.scale(scalar);
}

template<int N, typename T>
constexpr bstone::VecNT<N, T> operator/(
	const bstone::VecNT<N, T>& lhs,
	const T& scalar)
{
	return lhs.scale(T{1} / scalar);
}


#endif // !BSTONE_VECN_INCLUDED
