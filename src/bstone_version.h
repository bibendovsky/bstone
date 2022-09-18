/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
