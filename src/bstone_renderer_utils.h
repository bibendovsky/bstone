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
// Renderer utils.
//


#ifndef BSTONE_RENDERER_UTILS_INCLUDED
#define BSTONE_RENDERER_UTILS_INCLUDED


#include <cassert>
#include "bstone_matn.h"


namespace bstone
{


// ==========================================================================
// RendererUtils
//

struct RendererUtils
{
	template<typename T>
	using Mat4T = MatNT<4, T>;


	template<typename T>
	static constexpr auto pi_t = static_cast<T>(3.14159265358979323846);

	template<typename T>
	static constexpr auto pi_over_180_t = static_cast<T>(0.0174532925199432957692);


	template<typename T>
	static T deg_to_rad(
		const T angle_deg)
	{
		return angle_deg * pi_over_180_t<T>;
	}


	template<typename T>
	static Mat4T<T> build_rotation_about_z_t(
		const T angle_rad)
	{
		const auto cos_t = std::cos(angle_rad);
		const auto sin_t = std::sin(angle_rad);

		const auto m_11 = cos_t;
		const auto m_12 = -sin_t;
		const auto m_13 = static_cast<T>(0);
		const auto m_14 = static_cast<T>(0);

		const auto m_21 = sin_t;
		const auto m_22 = cos_t;
		const auto m_23 = static_cast<T>(0);
		const auto m_24 = static_cast<T>(0);

		const auto m_31 = static_cast<T>(0);
		const auto m_32 = static_cast<T>(0);
		const auto m_33 = static_cast<T>(1);
		const auto m_34 = static_cast<T>(0);

		const auto m_41 = static_cast<T>(0);
		const auto m_42 = static_cast<T>(0);
		const auto m_43 = static_cast<T>(0);
		const auto m_44 = static_cast<T>(1);

		return Mat4T<T>
		{
			m_11, m_12, m_13, m_14,
			m_21, m_22, m_23, m_24,
			m_31, m_32, m_33, m_34,
			m_41, m_42, m_43, m_44,
		};
	}


	template<typename T>
	static Mat4T<T> build_ortho_projection_t(
		const T l,
		const T r,
		const T b,
		const T t,
		const T n,
		const T f)
	{
		assert(n > static_cast<T>(0));
		assert(f > static_cast<T>(0));
		assert(l != r);
		assert(b != t);
		assert(n != f);

		const auto r_r_minus_l = static_cast<T>(1) / (r - l);
		const auto r_t_minus_b = static_cast<T>(1) / (t - b);
		const auto r_f_minus_n = static_cast<T>(1) / (f - n);

		const auto m_11 = static_cast<T>(2) * r_r_minus_l;
		const auto m_12 = static_cast<T>(0);
		const auto m_13 = static_cast<T>(0);
		const auto m_14 = -(r + l) * r_r_minus_l;

		const auto m_21 = static_cast<T>(0);
		const auto m_22 = static_cast<T>(2) * r_t_minus_b;
		const auto m_23 = static_cast<T>(0);
		const auto m_24 = -(t + b) * r_t_minus_b;

		const auto m_31 = static_cast<T>(0);
		const auto m_32 = static_cast<T>(0);
		const auto m_33 = static_cast<T>(2) * r_f_minus_n;
		const auto m_34 = -(f + n) * r_f_minus_n;

		const auto m_41 = static_cast<T>(0);
		const auto m_42 = static_cast<T>(0);
		const auto m_43 = static_cast<T>(2) * r_f_minus_n;
		const auto m_44 = static_cast<T>(1);

		return Mat4T<T>
		{
			m_11, m_12, m_13, m_14,
			m_21, m_22, m_23, m_24,
			m_31, m_32, m_33, m_34,
			m_41, m_42, m_43, m_44,
		};
	}

	template<typename T>
	static Mat4T<T> build_perspective_projection_t(
		const T l,
		const T r,
		const T b,
		const T t,
		const T n,
		const T f)
	{
		assert(n > static_cast<T>(0));
		assert(f > static_cast<T>(0));
		assert(l != r);
		assert(b != t);
		assert(n != f);

		const auto r_r_minus_l = static_cast<T>(1) / (r - l);
		const auto r_t_minus_b = static_cast<T>(1) / (t - b);
		const auto r_f_minus_n = static_cast<T>(1) / (f - n);

		const auto m_11 = static_cast<T>(2) * n * r_r_minus_l;
		const auto m_12 = static_cast<T>(0);
		const auto m_13 = (r + l) * r_r_minus_l;
		const auto m_14 = static_cast<T>(0);

		const auto m_21 = static_cast<T>(0);
		const auto m_22 = static_cast<T>(2) * n * r_t_minus_b;
		const auto m_23 = (t + b) * r_t_minus_b;
		const auto m_24 = static_cast<T>(0);

		const auto m_31 = static_cast<T>(0);
		const auto m_32 = static_cast<T>(0);
		const auto m_33 = -(f + n) * r_f_minus_n;
		const auto m_34 = -static_cast<T>(2) * f * n * r_f_minus_n;

		const auto m_41 = static_cast<T>(0);
		const auto m_42 = static_cast<T>(0);
		const auto m_43 = static_cast<T>(-1);
		const auto m_44 = static_cast<T>(0);

		return Mat4T<T>
		{
			m_11, m_12, m_13, m_14,
			m_21, m_22, m_23, m_24,
			m_31, m_32, m_33, m_34,
			m_41, m_42, m_43, m_44,
		};
	}
}; // RendererUtils

//
// RendererUtils
// ==========================================================================


} // bstone


#endif // !BSTONE_RENDERER_UTILS_INCLUDED
