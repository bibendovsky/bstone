/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// Project's version.
//


#include "bstone_version.h"


namespace bstone
{


const std::string& Version::get_string()
{
	static const auto result = std::string
	{
#include "bstone_version_.h"
	};

	return result;
}


} // bstone


