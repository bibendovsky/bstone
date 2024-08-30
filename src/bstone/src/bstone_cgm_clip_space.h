/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Computer Graphics Math - Clip space transformations.

/*
Notes:
  - Only floating-point types are supported.
*/

#ifndef BSTONE_CGM_CLIP_SPACE_INCLUDED
#define BSTONE_CGM_CLIP_SPACE_INCLUDED

#include <cmath>

#include <type_traits>

#include "bstone_assert.h"
#include "bstone_cgm_mat.h"

namespace bstone {
namespace cgm {

/*
Creates a matrix for an orthographic parallel viewing volume,
using right-handed coordinates.

The near and far clip planes correspond to z normalized device
coordinates of -1 and +1 respectively.

Params:
  - l: Left point of near clipping plane.
  - r: Right point of near clipping plane.
  - b: Bottom point of near clipping plane.
  - t: Top point of near clipping plane.
  - n: The distance from the eye to the near clipping plane.
  - f: The distance from the eye to the far clipping plane.

Returns:
  A matrix that produces parallel projection.
*/
template<typename T>
inline constexpr Mat<4, 4, T> make_ortho_rh_n1p1(T l, T r, T b, T t, T n, T f) noexcept
{
	static_assert(std::is_floating_point<T>::value, "Expected floating-point type.");

	BSTONE_ASSERT(l != r);
	BSTONE_ASSERT(b != t);
	BSTONE_ASSERT(n != f);

	const auto r_rml = 1 / (r - l);
	const auto r_tmb = 1 / (t - b);
	const auto r_fmn = 1 / (f - n);

	const auto _11 = 2 * r_rml;
	const auto _14 = -(r + l) * r_rml;

	const auto _22 = 2 * r_tmb;
	const auto _24 = -(t + b) * r_tmb;

	const auto _33 = -2 * r_fmn;
	const auto _34 = -(f + n) * r_fmn;

	return Mat<4, 4, T>
	{
		_11,   0,   0, 0,
		  0, _22,   0, 0,
		  0,   0, _33, 0,
		_14, _24, _34, 1
	};
}

/*
Creates a matrix for a right handed, symmetric perspective-view frustum.

The near and far clip planes correspond to z normalized device coordinates
of -1 and +1 respectively.

Params:
  - v: Vertical field of view angle expressed in radians.
  - w: Width of the viewport.
  - h: Height of the viewport.
  - n: Positive distance from the eye to the near clipping plane.
  - f: Positive distance from the eye to the far clipping plane.
*/
template<typename T>
inline Mat<4, 4, T> make_perspective_vfov_rh_n1p1(T v, T w, T h, T n, T f) noexcept
{
	static_assert(std::is_floating_point<T>::value, "Expected floating-point type.");

	BSTONE_ASSERT(n > 0);
	BSTONE_ASSERT(f > 0);
	BSTONE_ASSERT(f > n);

	const auto half_v = v / 2;
	const auto height = std::cos(half_v) / std::sin(half_v);
	const auto width = height * h / w;

	const auto _11 = width;
	const auto _22 = height;
	const auto _33 = -(f + n) / (f - n);
	const auto _34 = -(2 * f * n) / (f - n);

	return Mat<4, 4, T>
	{
		_11, 0, 0,  0,
		0, _22, 0,  0,
		0, 0, _33, -1,
		0, 0, _34,  0,
	};
}

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_CLIP_SPACE_INCLUDED
