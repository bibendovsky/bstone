/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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

constexpr double pi() noexcept
{
	return 3.14159265358979323846;
}

constexpr double deg_to_rad(double angle_deg) noexcept
{
	return angle_deg * pi() / 180.0;
}

constexpr double rad_to_deg(double angle_rad) noexcept
{
	return angle_rad * 180.0 / pi();
}

constexpr double fixed_to_floating(int value) noexcept
{
	return static_cast<double>(value) / static_cast<double>(1 << 16);
}

constexpr int floating_to_fixed(double value) noexcept
{
	return static_cast<int>(value * (1 << 16));
}

template<typename T>
inline T gcd(T a, T b) noexcept
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
constexpr const T& clamp(const T& value, const T& min_value, const T& max_value) noexcept
{
	return (value < min_value) ? min_value : ((value > max_value) ? max_value : value);
}

} // math
} // bstone

#endif // BSTONE_MATH_INCLUDED
