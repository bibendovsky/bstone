/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Project's version.
//


#ifndef BSTONE_VERSION_INCLUDED
#define BSTONE_VERSION_INCLUDED


#include <string>


namespace bstone
{


struct Version
{
	static const std::string& get_string();
}; // Version


} // bstone


#endif // !BSTONE_VERSION_INCLUDED
