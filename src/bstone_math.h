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
// Various math constants and functions.
//


#ifndef BSTONE_MATH_INCLUDED
#define BSTONE_MATH_INCLUDED


#include <type_traits>


namespace bstone
{
namespace math
{


constexpr double pi()
{
	return 3.14159265358979323846;
}


constexpr double deg_to_rad(
	const double angle_deg)
{
	return angle_deg * pi() / 180.0;
}

constexpr double rad_to_deg(
	const double angle_rad)
{
	return angle_rad * 180.0 / pi();
}


constexpr double fixed_to_floating(
	const int value)
{
	return static_cast<double>(value) / static_cast<double>(1 << 16);
}

constexpr int floating_to_fixed(
	const double value)
{
	return static_cast<int>(value * (1 << 16));
}


template<typename T>
inline T gcd(
	const T a,
	const T b) noexcept
{
	static_assert(std::is_integral<T>::value, "Expected integral type.");


	if (b == 0)
	{
		return a;
	}
	else if (a < b)
	{
		return gcd(b, a);
	}
	else
	{
		return gcd(b, a % b);
	}
}

template<typename T>
constexpr T clamp(
	const T& value,
	const T& min_value,
	const T& max_value)
{
	return value < min_value ? min_value : (value > max_value ? max_value : value);
}


} // math
} // bstone


#endif // BSTONE_MATH_INCLUDED
