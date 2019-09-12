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

//
// R8g8b8a8
// ==========================================================================


} // bstone
