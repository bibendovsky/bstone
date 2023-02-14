/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
Computer Graphics Math:
Common transformations
*/

#if !defined(BSTONE_CGM_TRANSFORM_INCLUDED)
#define BSTONE_CGM_TRANSFORM_INCLUDED

#if !defined(NDEBUG)
//#define BSTONE_CGM_TRANSFORM_REFERENCE
#endif

#include <cmath>
#include "bstone_cgm_mat.h"
#include "bstone_cgm_vec.h"

namespace bstone {
namespace cgm {

/*
Composes a translation matrix created from a three-component vector.

Params:
  - m: Input matrix.
  - v: A translation vector.

Returns:
	An input matrix multiplied by the translation one.
*/
template<typename T>
inline constexpr MatT<4, 4, T> translate(const MatT<4, 4, T>& m, const VecT<3, T>& v)
{
	const auto _14 = v[0];
	const auto _24 = v[1];
	const auto _34 = v[2];

#if defined(BSTONE_CGM_TRANSFORM_REFERENCE)
	return m * MatT<4, 4, T>
	{
		  1,   0,   0, 0,
		  0,   1,   0, 0,
		  0,   0,   1, 0,
		_14, _24, _34, 1,
	};
#else
	return MatT<4, 4, T>
	{
		m[ 0],
		m[ 1],
		m[ 2],
		m[ 3],

		m[ 4],
		m[ 5],
		m[ 6],
		m[ 7],

		m[ 8],
		m[ 9],
		m[10],
		m[11],

		(m[ 0] * _14) + (m[ 4] * _24) + (m[ 8] * _34) + m[12],
		(m[ 1] * _14) + (m[ 5] * _24) + (m[ 9] * _34) + m[13],
		(m[ 2] * _14) + (m[ 6] * _24) + (m[10] * _34) + m[14],
		(m[ 3] * _14) + (m[ 7] * _24) + (m[11] * _34) + m[15]
	};
#endif
}

/*
Composes a scale matrix created from three scalars.

Params:
  - m: Input matrix.
  - v: Scalars for each axis.

Returns:
	An input matrix multiplied by the scale one.
*/
template<typename T>
inline constexpr MatT<4, 4, T> scale(const MatT<4, 4, T>& m, const VecT<3, T>& v)
{
	const auto _11 = v[0];
	const auto _22 = v[1];
	const auto _33 = v[2];

#if defined(BSTONE_CGM_TRANSFORM_REFERENCE)
	return m * MatT<4, 4, T>
	{
		_11,   0,   0, 0,
		  0, _22,   0, 0,
		  0,   0, _33, 0,
		  0,   0,   0, 1,
	};
#else
	return MatT<4, 4, T>
	{
		m[ 0] * _11,
		m[ 1] * _11,
		m[ 2] * _11,
		m[ 3] * _11,

		m[ 4] * _22,
		m[ 5] * _22,
		m[ 6] * _22,
		m[ 7] * _22,

		m[ 8] * _33,
		m[ 9] * _33,
		m[10] * _33,
		m[11] * _33,

		m[12],
		m[13],
		m[14],
		m[15]
	};
#endif
}

/*
Composes a rotation matrix created from a unit vector and an angle.

Params:
  - m: Input matrix.
  - t: Rotation angle expressed in radians.
  - u: Normalized rotation axis.

Returns:
	An input matrix multiplied by the rotation one.
*/
template<typename T>
inline constexpr MatT<4, 4, T> rotate(const MatT<4, 4, T>& m, T t, const VecT<3, T>& u)
{
	const auto ct = std::cos(t);
	const auto oct = 1 - ct;
	const auto st = std::sin(t);

	const auto ux = u[0];
	const auto uy = u[1];
	const auto uz = u[2];

	const auto uxst = ux * st;
	const auto uyst = uy * st;
	const auto uzst = uz * st;

	const auto uxyoct = ux * uy * oct;
	const auto uxzoct = ux * uz * oct;
	const auto uyzoct = uy * uz * oct;

	const auto _11 = ct + (ux * ux * oct);
	const auto _12 = uxyoct - uzst;
	const auto _13 = uxzoct + uyst;

	const auto _21 = uxyoct + uzst;
	const auto _22 = ct + (uy * uy * oct);
	const auto _23 = uyzoct - uxst;

	const auto _31 = uxzoct - uyst;
	const auto _32 = uyzoct + uxst;
	const auto _33 = ct + (uz * uz * oct);

#if defined(BSTONE_CGM_TRANSFORM_REFERENCE)
	return m * MatT<4, 4, T>
	{
		_11, _21, _31, 0,
		_12, _22, _32, 0,
		_13, _23, _33, 0,
		  0,   0,   0, 1
	};
#else
	return MatT<4, 4, T>
	{
		(m[ 0] * _11) + (m[ 4] * _21) + (m[ 8] * _31),
		(m[ 1] * _11) + (m[ 5] * _21) + (m[ 9] * _31),
		(m[ 2] * _11) + (m[ 6] * _21) + (m[10] * _31),
		(m[ 3] * _11) + (m[ 7] * _21) + (m[11] * _31),

		(m[ 0] * _12) + (m[ 4] * _22) + (m[ 8] * _32),
		(m[ 1] * _12) + (m[ 5] * _22) + (m[ 9] * _32),
		(m[ 2] * _12) + (m[ 6] * _22) + (m[10] * _32),
		(m[ 3] * _12) + (m[ 7] * _22) + (m[11] * _32),

		(m[ 0] * _13) + (m[ 4] * _23) + (m[ 8] * _33),
		(m[ 1] * _13) + (m[ 5] * _23) + (m[ 9] * _33),
		(m[ 2] * _13) + (m[ 6] * _23) + (m[10] * _33),
		(m[ 3] * _13) + (m[ 7] * _23) + (m[11] * _33),

		m[12],
		m[13],
		m[14],
		m[15]
	};
#endif
}

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_TRANSFORM_INCLUDED
