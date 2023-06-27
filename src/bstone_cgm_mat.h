/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
Computer Graphics Math:
Column-major matrix
*/

#if !defined(BSTONE_CGM_MAT_INCLUDED)
#define BSTONE_CGM_MAT_INCLUDED

#include <cassert>
#include <type_traits>
#include "bstone_int.h"
#include "bstone_type_traits.h"

namespace bstone {
namespace cgm {

template<IntP M, IntP N, typename T>
class MatT;

template<typename T>
class MatT<4, 4, T>
{
public:
	static constexpr auto row_count = IntP{4};
	static constexpr auto column_count = IntP{4};
	static constexpr auto item_count = row_count * column_count;

	using Item = T;

	MatT() = default;

	constexpr MatT(
		Item m11, Item m21, Item m31, Item m41,
		Item m12, Item m22, Item m32, Item m42,
		Item m13, Item m23, Item m33, Item m43,
		Item m14, Item m24, Item m34, Item m44)
		:
		m_{m11, m21, m31, m41, m12, m22, m32, m42, m13, m23, m33, m43, m14, m24, m34, m44}
	{}

	const Item& operator[](IntP index) const
	{
		assert(index >= 0 && index < item_count);
		return m_[index];
	}

	Item& operator[](IntP index)
	{
		return const_cast<Item&>(type_traits::as_const(*this)[index]);
	}

private:
	using Items = Item[item_count];

	Items m_{};
};

// ==========================================================================

template<typename T>
inline constexpr MatT<4, 4, T> operator*(const MatT<4, 4, T>& a, const MatT<4, 4, T>& b)
{
	return MatT<4, 4, T>
	{
		(a[ 0] * b[ 0]) + (a[ 4] * b[ 1]) + (a[ 8] * b[ 2]) + (a[12] * b[ 3]),
		(a[ 1] * b[ 0]) + (a[ 5] * b[ 1]) + (a[ 9] * b[ 2]) + (a[13] * b[ 3]),
		(a[ 2] * b[ 0]) + (a[ 6] * b[ 1]) + (a[10] * b[ 2]) + (a[14] * b[ 3]),
		(a[ 3] * b[ 0]) + (a[ 7] * b[ 1]) + (a[11] * b[ 2]) + (a[15] * b[ 3]),

		(a[ 0] * b[ 4]) + (a[ 4] * b[ 5]) + (a[ 8] * b[ 6]) + (a[12] * b[ 7]),
		(a[ 1] * b[ 4]) + (a[ 5] * b[ 5]) + (a[ 9] * b[ 6]) + (a[13] * b[ 7]),
		(a[ 2] * b[ 4]) + (a[ 6] * b[ 5]) + (a[10] * b[ 6]) + (a[14] * b[ 7]),
		(a[ 3] * b[ 4]) + (a[ 7] * b[ 5]) + (a[11] * b[ 6]) + (a[15] * b[ 7]),

		(a[ 0] * b[ 8]) + (a[ 4] * b[ 9]) + (a[ 8] * b[10]) + (a[12] * b[11]),
		(a[ 1] * b[ 8]) + (a[ 5] * b[ 9]) + (a[ 9] * b[10]) + (a[13] * b[11]),
		(a[ 2] * b[ 8]) + (a[ 6] * b[ 9]) + (a[10] * b[10]) + (a[14] * b[11]),
		(a[ 3] * b[ 8]) + (a[ 7] * b[ 9]) + (a[11] * b[10]) + (a[15] * b[11]),

		(a[ 0] * b[12]) + (a[ 4] * b[13]) + (a[ 8] * b[14]) + (a[12] * b[15]),
		(a[ 1] * b[12]) + (a[ 5] * b[13]) + (a[ 9] * b[14]) + (a[13] * b[15]),
		(a[ 2] * b[12]) + (a[ 6] * b[13]) + (a[10] * b[14]) + (a[14] * b[15]),
		(a[ 3] * b[12]) + (a[ 7] * b[13]) + (a[11] * b[14]) + (a[15] * b[15])
	};
}

// ==========================================================================

namespace detail {

template<typename TMatrix>
struct MakeIdentity;

template<typename T>
struct MakeIdentity<MatT<4, 4, T>>
{
	static constexpr MatT<4, 4, T> invoke()
	{
		return MatT<4, 4, T>
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
	}
};

} // namespace detail

template<typename TMatrix>
inline constexpr auto make_identity()
{
	return detail::MakeIdentity<TMatrix>::invoke();
}

// ==========================================================================

using Mat4F = MatT<4, 4, float>;
using Mat4D = MatT<4, 4, double>;

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_MAT_INCLUDED
