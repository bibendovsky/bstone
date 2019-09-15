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


#ifndef BSTONE_RGB_COLOR_MODEL_INCLUDED
#define BSTONE_RGB_COLOR_MODEL_INCLUDED


#include <cstdint>

#include <type_traits>

#include "glm/glm.hpp"


namespace bstone
{


// ==========================================================================
// R8g8b8
//

class R8g8b8
{
public:
	std::uint8_t r_;
	std::uint8_t g_;
	std::uint8_t b_;


	R8g8b8();

	R8g8b8(
		const std::uint8_t r,
		const std::uint8_t g,
		const std::uint8_t b);


	std::uint8_t& get(
		const int index);

	const std::uint8_t& get(
		const int index) const;


	std::uint8_t& operator[](
		const int index);

	const std::uint8_t& operator[](
		const int index) const;
}; // R8g8b8

using R8g8b8Ptr = R8g8b8*;
using R8g8b8CPtr = const R8g8b8*;

//
// R8g8b8
// ==========================================================================


// ==========================================================================
// R8g8b8a8
//

class R8g8b8a8
{
public:
	std::uint8_t r_;
	std::uint8_t g_;
	std::uint8_t b_;
	std::uint8_t a_;


	R8g8b8a8();

	R8g8b8a8(
		const std::uint8_t r,
		const std::uint8_t g,
		const std::uint8_t b,
		const std::uint8_t a);


	std::uint8_t& get(
		const int index);

	const std::uint8_t& get(
		const int index) const;


	std::uint8_t& operator[](
		const int index);

	const std::uint8_t& operator[](
		const int index) const;


	void reset();


	// Averages two colors and premultiplies alpha.
	static R8g8b8a8 average_pa(
		const R8g8b8a8 color_0,
		const R8g8b8a8 color_1);

	// Averages four colors and premultiplies alpha.
	static R8g8b8a8 average_pa(
		const R8g8b8a8 color_0,
		const R8g8b8a8 color_1,
		const R8g8b8a8 color_2,
		const R8g8b8a8 color_3);
}; // R8g8b8a8

using R8g8b8a8Ptr = R8g8b8a8*;
using R8g8b8a8CPtr = const R8g8b8a8*;

//
// R8g8b8a8
// ==========================================================================


} // bstone


#endif // !BSTONE_RGB_COLOR_MODEL_INCLUDED
