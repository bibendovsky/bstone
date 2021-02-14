/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// R8G8B8A8 color type.
//


#include "bstone_rgb8.h"

#include <cassert>


namespace bstone
{


static_assert(sizeof(Rgba8) == 4, "Class size mismatch.");


// ==========================================================================
// Rgba8
//

Rgba8::Rgba8() noexcept
	:
	r_{},
	g_{},
	b_{},
	a_{}
{
}

Rgba8::Rgba8(
	const std::uint8_t r,
	const std::uint8_t g,
	const std::uint8_t b,
	const std::uint8_t a) noexcept
	:
	r_{r},
	g_{g},
	b_{b},
	a_{a}
{
}

std::uint8_t& Rgba8::get(
	const int index)
{
	assert(index >= 0 && index < 4);

	return reinterpret_cast<std::uint8_t*>(this)[index];
}

const std::uint8_t& Rgba8::get(
	const int index) const
{
	assert(index >= 0 && index < 4);

	return reinterpret_cast<const std::uint8_t*>(this)[index];
}

std::uint8_t& Rgba8::operator[](
	const int index)
{
	return get(index);
}

const std::uint8_t& Rgba8::operator[](
	const int index) const
{
	return get(index);
}

void Rgba8::reset() noexcept
{
	reinterpret_cast<std::uint32_t&>(*this) = 0;
}

bool operator==(
	const Rgba8& lhs,
	const Rgba8& rhs) noexcept
{
	return reinterpret_cast<const std::uint32_t&>(lhs) == reinterpret_cast<const std::uint32_t&>(rhs);
}

bool operator!=(
	const Rgba8& lhs,
	const Rgba8& rhs) noexcept
{
	return !(lhs == rhs);
}

//
// Rgba8
// ==========================================================================


} // bstone
