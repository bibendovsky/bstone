/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
Computer Graphics Math:
A vector
*/

#if !defined(BSTONE_CGM_VEC_INCLUDED)
#define BSTONE_CGM_VEC_INCLUDED

#include <cassert>
#include <cmath>
#include "bstone_int.h"
#include "bstone_utility.h"

namespace bstone {
namespace cgm {

template<IntP N, typename T>
class VecT;

// ==========================================================================

template<typename T>
class VecT<2, T>
{
public:
	static constexpr auto item_count = 2;

	using Item = T;

	constexpr VecT() = default;

	constexpr VecT(Item v0, Item v1)
		:
		v_{v0, v1}
	{}

	constexpr const Item& operator[](IntP index) const
	{
		assert(index >= 0 && index < item_count);
		return v_[index];
	}

	constexpr Item& operator[](IntP index)
	{
		return const_cast<Item&>(::bstone::as_const(*this)[index]);
	}

	constexpr Item get_magnitude() const
	{
		return std::sqrt((v_[0] * v_[0]) + (v_[1] * v_[1]));
	}

private:
	using Items = Item[item_count];

	Items v_{};
};

// ==========================================================================

template<typename T>
class VecT<3, T>
{
public:
	static constexpr auto item_count = 3;

	using Item = T;

	constexpr VecT() = default;

	constexpr VecT(Item v0, Item v1, Item v2)
		:
		v_{v0, v1, v2}
	{}

	constexpr const Item& operator[](IntP index) const
	{
		assert(index >= 0 && index < item_count);
		return v_[index];
	}

	constexpr Item& operator[](IntP index)
	{
		return const_cast<Item&>(::bstone::as_const(*this)[index]);
	}

	constexpr Item get_magnitude() const
	{
		return std::sqrt((v_[0] * v_[0]) + (v_[1] * v_[1]) + (v_[2] * v_[2]));
	}

private:
	using Items = Item[item_count];

	Items v_{};
};

// ==========================================================================

template<typename T>
class VecT<4, T>
{
public:
	static constexpr auto item_count = 4;

	using Item = T;

	constexpr VecT() = default;

	constexpr VecT(Item v0, Item v1, Item v2, Item v3)
		:
		v_{v0, v1, v2, v3}
	{}

	constexpr const Item& operator[](IntP index) const
	{
		assert(index >= 0 && index < item_count);
		return v_[index];
	}

	constexpr Item& operator[](IntP index)
	{
		return const_cast<Item&>(::bstone::as_const(*this)[index]);
	}

	constexpr Item get_magnitude() const
	{
		return std::sqrt((v_[0] * v_[0]) + (v_[1] * v_[1]) + (v_[2] * v_[2]) + (v_[3] * v_[3]));
	}

private:
	using Items = Item[item_count];

	Items v_{};
};

// ==========================================================================

template<typename T>
inline constexpr bool operator==(const VecT<2, T>& a, const VecT<2, T>& b)
{
	return a[0] == b[0] && a[1] == b[1];
}

template<typename T>
inline constexpr bool operator==(const VecT<3, T>& a, const VecT<3, T>& b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

template<typename T>
inline constexpr bool operator==(const VecT<4, T>& a, const VecT<4, T>& b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

// ==========================================================================

template<IntP N, typename T>
inline constexpr bool operator!=(const VecT<N, T>& a, const VecT<N, T>& b)
{
	return !(a == b);
}

// ==========================================================================

template<typename T>
inline constexpr VecT<2, T> operator-(const VecT<2, T>& a)
{
	return VecT<2, T>{-a[0], -a[1]};
}

template<typename T>
inline constexpr VecT<3, T> operator-(const VecT<3, T>& a)
{
	return VecT<3, T>{-a[0], -a[1], -a[2]};
}

template<typename T>
inline constexpr VecT<4, T> operator-(const VecT<4, T>& a)
{
	return VecT<4, T>{-a[0], -a[1], -a[2], -a[3]};
}

// ==========================================================================

template<typename T>
inline constexpr VecT<2, T> operator+(const VecT<2, T>& a, const VecT<2, T>& b)
{
	return VecT<2, T>{a[0] + b[0], a[1] + b[1]};
}

template<typename T>
inline constexpr VecT<3, T> operator+(const VecT<3, T>& a, const VecT<3, T>& b)
{
	return VecT<3, T>{a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

template<typename T>
inline constexpr VecT<4, T> operator+(const VecT<4, T>& a, const VecT<4, T>& b)
{
	return VecT<4, T>{a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
}

// ==========================================================================

template<typename T>
inline constexpr VecT<2, T> operator-(const VecT<2, T>& a, const VecT<2, T>& b)
{
	return VecT<2, T>{a[0] - b[0], a[1] - b[1]};
}

template<typename T>
inline constexpr VecT<3, T> operator-(const VecT<3, T>& a, const VecT<3, T>& b)
{
	return VecT<3, T>{a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

template<typename T>
inline constexpr VecT<4, T> operator-(const VecT<4, T>& a, const VecT<4, T>& b)
{
	return VecT<4, T>{a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]};
}

// ==========================================================================

template<typename T>
inline constexpr VecT<2, T> operator*(const VecT<2, T>& a, T s)
{
	return VecT<2, T>{a[0] * s, a[1] * s};
}

template<typename T>
inline constexpr VecT<3, T> operator*(const VecT<3, T>& a, T s)
{
	return VecT<3, T>{a[0] * s, a[1] * s, a[2] * s};
}

template<typename T>
inline constexpr VecT<4, T> operator*(const VecT<4, T>& a, T s)
{
	return VecT<4, T>{a[0] * s, a[1] * s, a[2] * s, a[3] * s};
}

// ==========================================================================

template<IntP N, typename T>
inline constexpr VecT<N, T> operator*(T s, const VecT<N, T>& a)
{
	return a * s;
}

// ==========================================================================

template<typename T>
inline constexpr T dot(const VecT<2, T>& a, const VecT<2, T>& b)
{
	return (a[0] * b[0]) + (a[1] * b[1]);
}

template<typename T>
inline constexpr T dot(const VecT<3, T>& a, const VecT<3, T>& b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

template<typename T>
inline constexpr T dot(const VecT<4, T>& a, const VecT<4, T>& b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}

// ==========================================================================

template<IntP N, typename T>
inline constexpr VecT<N, T> normalize(const VecT<N, T>& a)
{
	const auto r_magnitude = 1 / a.get_magnitude();
	return a * r_magnitude;
}

// ==========================================================================

using Vec2F = VecT<2, float>;
using Vec3F = VecT<3, float>;
using Vec4F = VecT<4, float>;

using Vec2D = VecT<2, double>;
using Vec3D = VecT<3, double>;
using Vec4D = VecT<4, double>;

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_VEC_INCLUDED
