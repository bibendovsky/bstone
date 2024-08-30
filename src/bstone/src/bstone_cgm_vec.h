/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Computer Graphics Math - Vector.

/*
Notes:
  - Only integral and floating-point types are supported.
*/

#ifndef BSTONE_CGM_VEC_INCLUDED
#define BSTONE_CGM_VEC_INCLUDED

#include <cmath>
#include <cstdint>

#include <type_traits>
#include <utility>

#include "bstone_assert.h"
#include "bstone_utility.h"

namespace bstone {
namespace cgm {

template<std::intptr_t N, typename T>
class Vec;

// ==========================================================================

namespace detail {

template<std::intptr_t N, typename T>
class VecBase
{
	static_assert(N >= 2 && N <= 4, "Unsupported dimension.");

	static_assert(
		std::is_integral<T>::value || std::is_floating_point<T>::value,
		"Expected integer or floating-point type.");

public:
	static constexpr auto item_count = N;

public:
	using Item = T;

public:
	template<typename... TArgs>
	constexpr VecBase(TArgs&&... args) noexcept
		:
		v_{std::forward<TArgs>(args)...}
	{}

	constexpr const Item& operator[](std::intptr_t index) const noexcept
	{
		BSTONE_ASSERT(index >= 0 && index < item_count);
		return v_[index];
	}

	constexpr Item& operator[](std::intptr_t index) noexcept
	{
		return bstone::as_mutable(bstone::as_const(*this)[index]);
	}

protected:
	using Items = Item[item_count];

	Items v_{};
};

} // namespace detail

template<typename T>
class Vec<2, T> : public detail::VecBase<2, T>
{
public:
	using Base = detail::VecBase<2, T>;
	using typename Base::Item;

public:
	Vec() = default;

	constexpr Vec(Item v0, Item v1) noexcept
		:
		Base{v0, v1}
	{}
};

// ==========================================================================

template<typename T>
class Vec<3, T> : public detail::VecBase<3, T>
{
public:
	using Base = detail::VecBase<3, T>;
	using typename Base::Item;

public:
	Vec() = default;

	constexpr Vec(Item v0, Item v1, Item v2) noexcept
		:
		Base{v0, v1, v2}
	{}
};

// ==========================================================================

template<typename T>
class Vec<4, T> : public detail::VecBase<4, T>
{
public:
	using Base = detail::VecBase<4, T>;
	using typename Base::Item;

public:
	Vec() = default;

	constexpr Vec(Item v0, Item v1, Item v2, Item v3) noexcept
		:
		Base{v0, v1, v2, v3}
	{}
};

// ==========================================================================

template<typename T>
inline constexpr bool operator==(const Vec<2, T>& a, const Vec<2, T>& b) noexcept
{
	return a[0] == b[0] && a[1] == b[1];
}

template<typename T>
inline constexpr bool operator==(const Vec<3, T>& a, const Vec<3, T>& b) noexcept
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

template<typename T>
inline constexpr bool operator==(const Vec<4, T>& a, const Vec<4, T>& b) noexcept
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

// ==========================================================================

template<std::intptr_t N, typename T>
inline constexpr bool operator!=(const Vec<N, T>& a, const Vec<N, T>& b) noexcept
{
	return !(a == b);
}

// ==========================================================================

template<typename T>
inline constexpr Vec<2, T> operator-(const Vec<2, T>& a) noexcept
{
	return Vec<2, T>{-a[0], -a[1]};
}

template<typename T>
inline constexpr Vec<3, T> operator-(const Vec<3, T>& a) noexcept
{
	return Vec<3, T>{-a[0], -a[1], -a[2]};
}

template<typename T>
inline constexpr Vec<4, T> operator-(const Vec<4, T>& a) noexcept
{
	return Vec<4, T>{-a[0], -a[1], -a[2], -a[3]};
}

// ==========================================================================

template<typename T>
inline constexpr Vec<2, T> operator+(const Vec<2, T>& a, const Vec<2, T>& b) noexcept
{
	return Vec<2, T>{a[0] + b[0], a[1] + b[1]};
}

template<typename T>
inline constexpr Vec<3, T> operator+(const Vec<3, T>& a, const Vec<3, T>& b) noexcept
{
	return Vec<3, T>{a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

template<typename T>
inline constexpr Vec<4, T> operator+(const Vec<4, T>& a, const Vec<4, T>& b) noexcept
{
	return Vec<4, T>{a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
}

// ==========================================================================

template<typename T>
inline constexpr Vec<2, T> operator-(const Vec<2, T>& a, const Vec<2, T>& b) noexcept
{
	return Vec<2, T>{a[0] - b[0], a[1] - b[1]};
}

template<typename T>
inline constexpr Vec<3, T> operator-(const Vec<3, T>& a, const Vec<3, T>& b) noexcept
{
	return Vec<3, T>{a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

template<typename T>
inline constexpr Vec<4, T> operator-(const Vec<4, T>& a, const Vec<4, T>& b) noexcept
{
	return Vec<4, T>{a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]};
}

// ==========================================================================

template<typename T>
inline constexpr Vec<2, T> operator*(const Vec<2, T>& a, T s) noexcept
{
	return Vec<2, T>{a[0] * s, a[1] * s};
}

template<typename T>
inline constexpr Vec<3, T> operator*(const Vec<3, T>& a, T s) noexcept
{
	return Vec<3, T>{a[0] * s, a[1] * s, a[2] * s};
}

template<typename T>
inline constexpr Vec<4, T> operator*(const Vec<4, T>& a, T s) noexcept
{
	return Vec<4, T>{a[0] * s, a[1] * s, a[2] * s, a[3] * s};
}

// ==========================================================================

template<std::intptr_t N, typename T>
inline constexpr Vec<N, T> operator*(T s, const Vec<N, T>& a) noexcept
{
	return a * s;
}

// ==========================================================================

template<typename T>
inline constexpr T dot(const Vec<2, T>& a, const Vec<2, T>& b) noexcept
{
	return a[0] * b[0] + a[1] * b[1];
}

template<typename T>
inline constexpr T dot(const Vec<3, T>& a, const Vec<3, T>& b) noexcept
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template<typename T>
inline constexpr T dot(const Vec<4, T>& a, const Vec<4, T>& b) noexcept
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

// ==========================================================================

template<std::intptr_t N, typename T>
inline constexpr T get_magnitude(const Vec<N, T>& x) noexcept
{
	return std::sqrt(bstone::cgm::dot(x, x));
}

// ==========================================================================

template<std::intptr_t N, typename T>
inline constexpr Vec<N, T> normalize(const Vec<N, T>& x) noexcept
{
	const auto r_magnitude = 1 / bstone::cgm::get_magnitude(x);
	return x * r_magnitude;
}

// ==========================================================================

using Vec2F = Vec<2, float>;
using Vec3F = Vec<3, float>;
using Vec4F = Vec<4, float>;

using Vec2D = Vec<2, double>;
using Vec3D = Vec<3, double>;
using Vec4D = Vec<4, double>;

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_VEC_INCLUDED
