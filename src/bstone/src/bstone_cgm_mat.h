/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Computer Graphics Math - Column-major matrix.

/*
Notes:
  - Only integral and floating-point types are supported.
*/

#ifndef BSTONE_CGM_MAT_INCLUDED
#define BSTONE_CGM_MAT_INCLUDED

#include <cstdint>

#include <type_traits>
#include <utility>

#include "bstone_assert.h"
#include "bstone_utility.h"

namespace bstone {
namespace cgm {

template<std::intptr_t M, std::intptr_t N, typename T>
class Mat;

// ==========================================================================

namespace detail {

template<std::intptr_t M, std::intptr_t N, typename T>
class MatBase
{
	static_assert(M >= 2 && M <= 4 && N >= 2 && N <= 4, "Unsupported dimensions.");

	static_assert(
		std::is_integral<T>::value || std::is_floating_point<T>::value,
		"Expected integer or floating-point type.");

public:
	static constexpr auto row_count = std::intptr_t{M};
	static constexpr auto column_count = std::intptr_t{N};
	static constexpr auto item_count = row_count * column_count;

public:
	using Item = T;

public:
	template<typename... TArgs>
	constexpr MatBase(TArgs&&... args) noexcept
		:
		m_{std::forward<TArgs>(args)...}
	{}

	constexpr const Item& operator[](std::intptr_t index) const noexcept
	{
		BSTONE_ASSERT(index >= 0 && index < item_count);
		return m_[index];
	}

	constexpr Item& operator[](std::intptr_t index) noexcept
	{
		return bstone::as_mutable(bstone::as_const(*this)[index]);
	}

protected:
	using Items = Item[item_count];

	Items m_{};
};

} // namespace detail

template<typename T>
class Mat<4, 4, T> : public detail::MatBase<4, 4, T>
{
public:
	using Base = detail::MatBase<4, 4, T>;
	using typename Base::Item;

public:
	Mat() = default;

	constexpr Mat(
		Item m11, Item m21, Item m31, Item m41,
		Item m12, Item m22, Item m32, Item m42,
		Item m13, Item m23, Item m33, Item m43,
		Item m14, Item m24, Item m34, Item m44)
		:
		Base{m11, m21, m31, m41, m12, m22, m32, m42, m13, m23, m33, m43, m14, m24, m34, m44}
	{}

	static constexpr Mat get_identity()
	{
		return Mat
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
	}
};

// ==========================================================================

template<typename T>
inline constexpr Mat<4, 4, T> operator*(const Mat<4, 4, T>& a, const Mat<4, 4, T>& b)
{
	return Mat<4, 4, T>
	{
		a[ 0] * b[ 0] + a[ 4] * b[ 1] + a[ 8] * b[ 2] + a[12] * b[ 3],
		a[ 1] * b[ 0] + a[ 5] * b[ 1] + a[ 9] * b[ 2] + a[13] * b[ 3],
		a[ 2] * b[ 0] + a[ 6] * b[ 1] + a[10] * b[ 2] + a[14] * b[ 3],
		a[ 3] * b[ 0] + a[ 7] * b[ 1] + a[11] * b[ 2] + a[15] * b[ 3],

		a[ 0] * b[ 4] + a[ 4] * b[ 5] + a[ 8] * b[ 6] + a[12] * b[ 7],
		a[ 1] * b[ 4] + a[ 5] * b[ 5] + a[ 9] * b[ 6] + a[13] * b[ 7],
		a[ 2] * b[ 4] + a[ 6] * b[ 5] + a[10] * b[ 6] + a[14] * b[ 7],
		a[ 3] * b[ 4] + a[ 7] * b[ 5] + a[11] * b[ 6] + a[15] * b[ 7],

		a[ 0] * b[ 8] + a[ 4] * b[ 9] + a[ 8] * b[10] + a[12] * b[11],
		a[ 1] * b[ 8] + a[ 5] * b[ 9] + a[ 9] * b[10] + a[13] * b[11],
		a[ 2] * b[ 8] + a[ 6] * b[ 9] + a[10] * b[10] + a[14] * b[11],
		a[ 3] * b[ 8] + a[ 7] * b[ 9] + a[11] * b[10] + a[15] * b[11],

		a[ 0] * b[12] + a[ 4] * b[13] + a[ 8] * b[14] + a[12] * b[15],
		a[ 1] * b[12] + a[ 5] * b[13] + a[ 9] * b[14] + a[13] * b[15],
		a[ 2] * b[12] + a[ 6] * b[13] + a[10] * b[14] + a[14] * b[15],
		a[ 3] * b[12] + a[ 7] * b[13] + a[11] * b[14] + a[15] * b[15]
	};
}

// ==========================================================================

using Mat4F = Mat<4, 4, float>;

using Mat4D = Mat<4, 4, double>;

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_MAT_INCLUDED
