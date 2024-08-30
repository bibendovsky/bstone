/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// A wrapper for std::ostringstream to produce a formatted string.
//


#include "bstone_format_string.h"


namespace bstone
{


FormatString::FormatString()
	:
	stream_{}
{
}

FormatString::operator std::string()
{
	return stream_.str();
}

std::string FormatString::to_string() const
{
	return stream_.str();
}


} // bstone
