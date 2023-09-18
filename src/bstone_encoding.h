/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// Encoding utils.
//


#ifndef BSTONE_ENCODING_INCLUDED
#define BSTONE_ENCODING_INCLUDED


#include <string>


namespace bstone
{


std::u16string u8_to_u16(
	const std::string& utf8_string);

std::string u16_to_u8(
	const std::u16string& utf16_string);


} // bstone


#endif // !BSTONE_ENCODING_INCLUDED
