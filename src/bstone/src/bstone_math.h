/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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

} // math
} // bstone

#endif // BSTONE_MATH_INCLUDED
