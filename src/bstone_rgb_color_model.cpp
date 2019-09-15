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
// Types of RGB color model.
//


#include "bstone_precompiled.h"
#include "bstone_rgb_color_model.h"

#include <cassert>


namespace bstone
{


static_assert(sizeof(R8g8b8a8) == 4, "Class size mismatch.");


// ==========================================================================
// R8g8b8
//

R8g8b8::R8g8b8()
	:
	r_{},
	g_{},
	b_{}
{
}

R8g8b8::R8g8b8(
	const std::uint8_t r,
	const std::uint8_t g,
	const std::uint8_t b)
	:
	r_{r},
	g_{g},
	b_{b}
{
}

std::uint8_t& R8g8b8::get(
	const int index)
{
	assert(index >= 0 && index < 3);

	return reinterpret_cast<std::uint8_t*>(this)[index];
}

const std::uint8_t& R8g8b8::get(
	const int index) const
{
	assert(index >= 0 && index < 3);

	return reinterpret_cast<const std::uint8_t*>(this)[index];
}

std::uint8_t& R8g8b8::operator[](
	const int index)
{
	return get(index);
}

const std::uint8_t& R8g8b8::operator[](
	const int index) const
{
	return get(index);
}

//
// R8g8b8
// ==========================================================================


// ==========================================================================
// R8g8b8a8
//

R8g8b8a8::R8g8b8a8()
	:
	r_{},
	g_{},
	b_{},
	a_{}
{
}

R8g8b8a8::R8g8b8a8(
	const std::uint8_t r,
	const std::uint8_t g,
	const std::uint8_t b,
	const std::uint8_t a)
	:
	r_{r},
	g_{g},
	b_{b},
	a_{a}
{
}

std::uint8_t& R8g8b8a8::get(
	const int index)
{
	assert(index >= 0 && index < 4);

	return reinterpret_cast<std::uint8_t*>(this)[index];
}

const std::uint8_t& R8g8b8a8::get(
	const int index) const
{
	assert(index >= 0 && index < 4);

	return reinterpret_cast<const std::uint8_t*>(this)[index];
}

std::uint8_t& R8g8b8a8::operator[](
	const int index)
{
	return get(index);
}

const std::uint8_t& R8g8b8a8::operator[](
	const int index) const
{
	return get(index);
}

void R8g8b8a8::reset()
{
	*reinterpret_cast<std::uint32_t*>(this) = 0;
}

R8g8b8a8 R8g8b8a8::average_pa(
	const R8g8b8a8 color_0,
	const R8g8b8a8 color_1)
{
	constexpr auto color_count = 2;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_;
	const auto r_sum = color_0.r_ + color_1.r_;
	const auto g_sum = color_0.g_ + color_1.g_;
	const auto b_sum = color_0.b_ + color_1.b_;

	return R8g8b8a8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

R8g8b8a8 R8g8b8a8::average_pa(
	const R8g8b8a8 color_0,
	const R8g8b8a8 color_1,
	const R8g8b8a8 color_2,
	const R8g8b8a8 color_3)
{
	constexpr auto color_count = 4;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_ + color_2.a_ + color_3.a_;
	const auto r_sum = color_0.r_ + color_1.r_ + color_2.r_ + color_3.r_;
	const auto g_sum = color_0.g_ + color_1.g_ + color_2.g_ + color_3.g_;
	const auto b_sum = color_0.b_ + color_1.b_ + color_2.b_ + color_3.b_;

	return R8g8b8a8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

//
// R8g8b8a8
// ==========================================================================


} // bstone
