/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Fixed-point number.
//


#ifndef BSTONE_FIXED_POINT_INCLUDED
#define BSTONE_FIXED_POINT_INCLUDED


namespace bstone
{


class FixedPoint final
{
public:
	using Value = int;


	static constexpr auto frac_bits = Value{16};
	static constexpr auto max_frac = 1 << frac_bits;
	static constexpr auto frac_mask = max_frac - 1;


	explicit FixedPoint(
		const Value new_value = 0);

	FixedPoint(
		const Value int_part,
		const Value frac_part);


	Value get_int() const;

	Value get_frac() const;


	Value& get_value();

	Value get_value() const;


	float to_float() const;

	double to_double() const;


private:
	Value value_;
}; // FixedPoint


} // bstone


bstone::FixedPoint operator+(
	const bstone::FixedPoint& lhs,
	const bstone::FixedPoint& rhs);

bstone::FixedPoint& operator+=(
	bstone::FixedPoint& lhs,
	const bstone::FixedPoint& rhs);

bstone::FixedPoint operator/(
	const bstone::FixedPoint& lhs,
	const int rhs);

bstone::FixedPoint operator*(
	const bstone::FixedPoint& lhs,
	const int rhs);

bstone::FixedPoint operator*(
	const int lhs,
	const bstone::FixedPoint& rhs);


#endif // !BSTONE_FIXED_POINT_INCLUDED
