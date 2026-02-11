/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// R8G8B8A8 color type.
//


#ifndef BSTONE_RGB8_INCLUDED
#define BSTONE_RGB8_INCLUDED


#include <cstdint>

#include <vector>


namespace bstone
{


// ==========================================================================
// Rgba8
//

class Rgba8
{
public:
	std::uint8_t r_;
	std::uint8_t g_;
	std::uint8_t b_;
	std::uint8_t a_;


	Rgba8();

	Rgba8(
		std::uint8_t r,
		std::uint8_t g,
		std::uint8_t b,
		std::uint8_t a);


	std::uint8_t& get(
		int index);

	const std::uint8_t& get(
		int index) const;


	std::uint8_t& operator[](
		int index);

	const std::uint8_t& operator[](
		int index) const;


	void reset();
}; // Rgba8


bool operator==(
	const Rgba8& lhs,
	const Rgba8& rhs);

bool operator!=(
	const Rgba8& lhs,
	const Rgba8& rhs);

//
// Rgba8
// ==========================================================================


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using Rgba8Buffer = std::vector<Rgba8>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_RGB8_INCLUDED
