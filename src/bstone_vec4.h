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
// The fourth dimensional vector.
//



#ifndef BSTONE_VEC4_INCLUDED
#define BSTONE_VEC4_INCLUDED


#include <array>


namespace bstone
{


template<typename T>
class Vec4T
{
public:
	static constexpr auto dimension = 4;


	constexpr Vec4T()
		:
		items_{}
	{
	}

	constexpr Vec4T(
		const T a1,
		const T a2,
		const T a3,
		const T a4)
		:
		items_{a1, a2, a3, a4}
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


	T& operator(
		const int index)
	{
		return get_item(index);
	}

	constexpr T operator(
		const int index) const
	{
		return get_item(index);
	}


private:
	using Items = std::array<T, dimension>;


	Items items_;
}; // Vec4T

using Vec4I = Vec4T<int>;
using Vec4F = Vec4T<float>;
using Vec4D = Vec4T<double>;


} // bstone


#endif // !BSTONE_VEC4_INCLUDED
