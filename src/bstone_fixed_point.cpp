/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "bstone_fixed_point.h"


namespace bstone
{


FixedPoint::FixedPoint(
	const Value new_value)
	:
	value_{new_value}
{
}

FixedPoint::FixedPoint(
	const Value int_part,
	const Value frac_part)
	:
	value_{(int_part << frac_bits) | (frac_part & frac_mask)}
{
}

int FixedPoint::get_int() const
{
	return value_ >> frac_bits;
}

int FixedPoint::get_frac() const
{
	return value_ & frac_mask;
}

FixedPoint::Value& FixedPoint::get_value()
{
	return value_;
}

FixedPoint::Value FixedPoint::get_value() const
{
	return value_;
}


} // bstone


bstone::FixedPoint operator+(
	const bstone::FixedPoint& lhs,
	const bstone::FixedPoint& rhs)
{
	return lhs.get_value() + rhs.get_value();
}

bstone::FixedPoint& operator+=(
	bstone::FixedPoint& lhs,
	const bstone::FixedPoint& rhs)
{
	lhs.get_value() += rhs.get_value();

	return lhs;
}

bstone::FixedPoint operator/(
	const bstone::FixedPoint& lhs,
	const int rhs)
{
	return lhs.get_value() / rhs;
}

bstone::FixedPoint operator*(
	const bstone::FixedPoint& lhs,
	const int rhs)
{
	return lhs.get_value() * rhs;
}

bstone::FixedPoint operator*(
	const int lhs,
	const bstone::FixedPoint& rhs)
{
	return lhs * rhs.get_value();
}
