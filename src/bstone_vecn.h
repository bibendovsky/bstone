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


#include <array>


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


#endif // !BSTONE_VECN_INCLUDED
