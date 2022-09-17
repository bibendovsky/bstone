/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_UN_VALUE_INCLUDED
#define BSTONE_UN_VALUE_INCLUDED


namespace bstone
{


//
// A wrapper to make a value uninitalized.
//
template<typename T>
class UnValue
{
public:
	UnValue()
	{
	}

	UnValue(
		const T& that)
		:
		value_{that}
	{
	}

	operator T&()
	{
		return value_;
	}


private:
	T value_;
}; // UnValue


} // bstone


#endif // !BSTONE_UN_VALUE_INCLUDED
